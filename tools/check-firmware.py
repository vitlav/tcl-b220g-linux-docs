#!/usr/bin/env python3
"""Read-only comparison of a firmware directory with the TCL reference subset."""
import argparse
import hashlib
import json
import lzma
from pathlib import Path
import subprocess
import sys


def references(repo):
    rows = {}
    for name in ("firmware-manifest.json", "firmware-runtime-manifest.json"):
        manifest = json.loads((repo / "artifacts" / name).read_text())
        for item in manifest["files"]:
            path = item.get("firmware_relative_path")
            if path is None:
                continue
            rel = Path(path)
            if rel.is_absolute() or ".." in rel.parts:
                raise ValueError("Invalid manifest path: " + path)
            optional = rel.name.startswith(("bdwlan.", "bdwlanu."))
            row = {"path": path, "bytes": item["bytes"],
                   "sha256": item["sha256"], "optional_variant": optional}
            if path in rows and rows[path] != row:
                raise ValueError("Conflicting manifest entry: " + path)
            rows[path] = row
    return list(rows.values())


def inspect(root, ref):
    row = dict(ref)
    candidates = [root / (ref["path"] + suffix) for suffix in ("", ".zst", ".xz")]
    # Like the kernel file loader: decompression fallback only if the plain
    # pathname is absent, not when reading an existing file fails.
    selected = next((p for p in candidates if p.exists() or p.is_symlink()), None)
    row["alternatives_present"] = [str(p.relative_to(root)) for p in candidates
                                   if p.exists() or p.is_symlink()]
    if selected is None:
        row["status"] = "missing"
        return row
    row["selected"] = str(selected.relative_to(root))
    try:
        if selected == candidates[1]:
            data = subprocess.run(["zstd", "-q", "-d", "-c", "--", str(selected)],
                                  check=True, capture_output=True).stdout
        elif selected == candidates[2]:
            data = lzma.decompress(selected.read_bytes())
        else:
            data = selected.read_bytes()
        row["actual_bytes"] = len(data)
        row["actual_sha256"] = hashlib.sha256(data).hexdigest()
        row["status"] = ("match" if len(data) == ref["bytes"] and
                         row["actual_sha256"] == ref["sha256"] else "different")
    except (OSError, subprocess.CalledProcessError, lzma.LZMAError) as exc:
        row["status"] = "error"
        row["error"] = str(exc)
    return row


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("firmware_directory", type=Path)
    args = parser.parse_args()
    if not args.firmware_directory.is_dir():
        parser.error("firmware directory does not exist")
    repo = Path(__file__).resolve().parents[1]
    rows = [inspect(args.firmware_directory, ref) for ref in references(repo)]
    failed = [r for r in rows if r["status"] != "match" and not r["optional_variant"]]
    print(json.dumps({"scope": "Reference subset only; not a full boot readiness test",
                      "optional_variants_affect_exit_status": False,
                      "reference_subset_matches": not failed,
                      "files": rows}, indent=2))
    return bool(failed)


if __name__ == "__main__":
    sys.exit(main())
