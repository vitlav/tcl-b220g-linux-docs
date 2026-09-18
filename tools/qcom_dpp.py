#!/usr/bin/env python3
"""Read Qualcomm DPP RWFS provisioning and build the TCL early ACPI override."""

from __future__ import annotations

import argparse
import hashlib
import json
import stat
import struct
import subprocess
from pathlib import Path


RWFS_MAGIC = b"RWFS"
RWFS_BLOB_SIZE = 0xF4
WLAN_FILE = "WLAN.PROVISION"
WLAN_FILE_SIZE = 27
WLAN_HEADER = b"\x01\x07\x01"


class DppError(ValueError):
    pass


def _u32(data: bytes, offset: int) -> int:
    if offset < 0 or offset + 4 > len(data):
        raise DppError(f"u32 at {offset:#x} lies outside the DPP image")
    return struct.unpack_from("<I", data, offset)[0]


def _utf16_name(raw: bytes) -> str:
    try:
        return raw.decode("utf-16le").split("\0", 1)[0]
    except UnicodeDecodeError as exc:
        raise DppError("invalid UTF-16LE RWFS directory entry") from exc


def read_rwfs_file(image: bytes, wanted: str) -> bytes:
    if len(image) < 0x60 or image[:4] != RWFS_MAGIC:
        raise DppError("not a Qualcomm DPP RWFS image")

    second = _u32(image, 0x08)
    if second + 0x40 > len(image) or image[second : second + 4] != RWFS_MAGIC:
        raise DppError("invalid RWFS second header")

    table_offset = _u32(image, second + 0x08)
    table_size = _u32(image, second + 0x0C)
    data_start = _u32(image, second + 0x14)
    table_end = table_offset + table_size
    if table_offset < second or table_end > len(image):
        raise DppError("RWFS directory lies outside the DPP image")

    for offset in range(table_offset, table_end, RWFS_BLOB_SIZE):
        if offset + RWFS_BLOB_SIZE > table_end:
            raise DppError("truncated RWFS directory entry")
        entry = image[offset : offset + RWFS_BLOB_SIZE]
        name = _utf16_name(entry[:98])
        region_len, data_len, present, relative = struct.unpack_from(
            "<IIII", entry, 0xC4
        )
        if not present:
            break
        if not name:
            raise DppError("present RWFS entry has no name")
        if data_len > region_len:
            raise DppError(f"{name}: data is larger than its RWFS region")
        start = data_start + relative
        end = start + data_len
        if start < data_start or end > len(image):
            raise DppError(f"{name}: data lies outside the DPP image")
        if name.casefold() == wanted.casefold():
            return image[start:end]

    raise DppError(f"{wanted} is absent from DPP")


def wlan_mac(image: bytes) -> bytes:
    provision = read_rwfs_file(image, WLAN_FILE)
    if len(provision) != WLAN_FILE_SIZE:
        raise DppError(
            f"unsupported {WLAN_FILE} size: {len(provision)}, expected {WLAN_FILE_SIZE}"
        )
    if provision[:3] != WLAN_HEADER:
        raise DppError(f"unsupported {WLAN_FILE} header: {provision[:3].hex()}")
    mac = provision[3:9]
    if mac == b"\0" * 6 or mac == b"\xff" * 6 or mac[0] & 0x03:
        raise DppError(f"invalid factory WLAN MAC: {mac.hex(':')}")
    return mac


def render_asl(template: str, mac: bytes) -> str:
    marker = "@WLAN_MAC_BYTES@"
    if template.count(marker) != 1:
        raise DppError(f"ASL template must contain exactly one {marker}")
    values = ", ".join(f"0x{byte:02X}" for byte in mac)
    return template.replace(marker, values)


def _newc_entry(name: str, mode: int, data: bytes, inode: int) -> bytes:
    encoded = name.encode() + b"\0"
    fields = [inode, mode, 0, 0, 1, 0, len(data), 0, 0, 0, 0, len(encoded), 0]
    result = bytearray(b"070701" + "".join(f"{value:08x}" for value in fields).encode())
    result.extend(encoded)
    result.extend(b"\0" * (-len(result) % 4))
    result.extend(data)
    result.extend(b"\0" * (-len(result) % 4))
    return bytes(result)


def build_early_cpio(aml: bytes) -> bytes:
    archive = bytearray()
    inode = 0
    for name in ("kernel", "kernel/firmware", "kernel/firmware/acpi"):
        inode += 1
        archive.extend(_newc_entry(name, stat.S_IFDIR | 0o755, b"", inode))
    inode += 1
    archive.extend(
        _newc_entry(
            "kernel/firmware/acpi/tcl-input.aml",
            stat.S_IFREG | 0o644,
            aml,
            inode,
        )
    )
    inode += 1
    archive.extend(_newc_entry("TRAILER!!!", 0, b"", inode))
    archive.extend(b"\0" * (-len(archive) % 512))
    return bytes(archive)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("dpp", type=Path, help="DPP partition or a read-only image")
    parser.add_argument("--template", type=Path, help="ASL template with MAC marker")
    parser.add_argument("--output-dir", type=Path, help="build ASL, AML and early CPIO here")
    args = parser.parse_args()

    image = args.dpp.read_bytes()
    mac = wlan_mac(image)
    result: dict[str, object] = {
        "dpp_sha256": hashlib.sha256(image).hexdigest(),
        "wlan_mac": mac.hex(":"),
        "source": WLAN_FILE,
    }

    if bool(args.template) != bool(args.output_dir):
        parser.error("--template and --output-dir must be used together")
    if args.template:
        args.output_dir.mkdir(parents=True, exist_ok=True)
        asl = args.output_dir / "tcl-input.asl"
        prefix = args.output_dir / "tcl-input"
        asl.write_text(render_asl(args.template.read_text(), mac))
        completed = subprocess.run(
            ["iasl", "-p", str(prefix), str(asl)],
            check=True,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
        )
        aml = prefix.with_suffix(".aml")
        aml_data = aml.read_bytes()
        if sum(aml_data) % 256:
            raise DppError("iasl produced AML with an invalid checksum")
        cpio = args.output_dir / "tcl-input-early.cpio"
        cpio.write_bytes(build_early_cpio(aml_data))
        result.update(
            {
                "aml_sha256": hashlib.sha256(aml_data).hexdigest(),
                "aml_size": len(aml_data),
                "cpio_sha256": hashlib.sha256(cpio.read_bytes()).hexdigest(),
                "iasl": completed.stdout.strip().splitlines()[-1],
            }
        )

    print(json.dumps(result, indent=2, sort_keys=True))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except (DppError, OSError, subprocess.CalledProcessError) as exc:
        raise SystemExit(f"qcom_dpp.py: {exc}") from exc
