#!/usr/bin/env python3
"""Exercise the selected kernel's unmodified earlycpio.c on a host with small header shims."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path
import shutil
import subprocess
import tempfile

p = argparse.ArgumentParser(description=__doc__)
p.add_argument('--kernel', type=Path, required=True)
p.add_argument('--archive', type=Path, required=True)
p.add_argument('--aml', type=Path, required=True)
a = p.parse_args()
source = a.kernel / 'lib/earlycpio.c'
results = {'earlycpio_sha256': hashlib.sha256(source.read_bytes()).hexdigest(), 'tests': []}
with tempfile.TemporaryDirectory(prefix='tcl-earlycpio-check-') as directory:
    d = Path(directory)
    (d / 'linux').mkdir()
    shutil.copy2(a.kernel / 'include/linux/earlycpio.h', d / 'linux/earlycpio.h')
    (d / 'linux/types.h').write_text('#include <stddef.h>\n')
    (d / 'linux/kernel.h').write_text('''#include <stdint.h>
#include <stdio.h>
#define PTR_ALIGN(p,a) ((__typeof__(p))(((uintptr_t)(p)+(a)-1)&~((uintptr_t)(a)-1)))
#define pr_warn(...) fprintf(stderr, __VA_ARGS__)
''')
    (d / 'linux/string.h').write_text('''#include <string.h>
static inline long strscpy(char *dst, const char *src, size_t size) {
    size_t n = strlen(src);
    if (!size) return -1;
    if (n >= size) n = size - 1;
    memcpy(dst, src, n); dst[n] = 0; return n;
}
''')
    (d / 'main.c').write_text('''#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/earlycpio.h>
static void *read_file(const char *name, size_t *size) {
    FILE *f = fopen(name, "rb"); if (!f) exit(2);
    if (fseek(f, 0, SEEK_END)) exit(2);
    long len = ftell(f); if (len < 0) exit(2);
    rewind(f); *size = len;
    void *data = malloc(*size + 1); if (!data) exit(2);
    if (fread(data, 1, *size, f) != *size) exit(2);
    fclose(f); return data;
}
int main(int argc, char **argv) {
    if (argc != 4) return 2;
    size_t len, alen; void *data = read_file(argv[1], &len);
    void *aml = read_file(argv[2], &alen); long offset = 0;
    struct cpio_data cd = find_cpio_data("kernel/firmware/acpi/", data, len, &offset);
    int found = cd.data != NULL;
    if (found && (cd.size != alen || memcmp(cd.data, aml, alen) || strcmp(cd.name, "scm-cca0.aml"))) return 3;
    printf("found=%d bytes=%zu name=%s next_offset=%ld\\n", found, cd.size, cd.name, offset);
    if (found) {
        struct cpio_data next = find_cpio_data("kernel/firmware/acpi/", (char *)data + offset, len - offset, NULL);
        if (next.data) return 4;
    }
    free(data); free(aml); return found != atoi(argv[3]);
}
''')
    subprocess.run(['cc', '-Wall', '-Wextra', '-Werror', '-I', str(d), str(source),
                    str(d / 'main.c'), '-o', str(d / 'check')], check=True)
    raw = a.archive.read_bytes()
    # A compressed copy is sufficient to test early-parser visibility. This is NOT a bootable initramfs.
    zipped = gzip.compress(raw, mtime=0)
    for name, blob, expected in [('early', raw, 1), ('early-before-gzip', raw + zipped, 1),
                                 ('gzip-only', zipped, 0), ('gzip-before-early', zipped + raw, 0)]:
        f = d / name
        f.write_bytes(blob)
        run = subprocess.run([str(d / 'check'), str(f), str(a.aml), str(expected)],
                             capture_output=True, text=True, check=True)
        results['tests'].append({'case': name, 'expected_found': expected, 'result': run.stdout.strip()})
print(json.dumps(results, indent=2))
