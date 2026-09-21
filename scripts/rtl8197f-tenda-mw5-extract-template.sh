#!/usr/bin/env bash
# SPDX-License-Identifier: GPL-2.0-or-later
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
    echo "Usage: $0 RTK-COLLECT-ARCHIVE.tar.gz [OUTPUT.bin]" >&2
    exit 2
fi

ARCHIVE="$1"
OUTPUT="${2:-tenda-mw5-spi-template.bin}"

if [[ ! -f "$ARCHIVE" ]]; then
    echo "ERROR: archive not found: $ARCHIVE" >&2
    exit 1
fi

ENTRY="$(tar -tzf "$ARCHIVE" | awk '/\/private\/mtd\/mtd0-ALL\.bin$/ { print; exit }')"
if [[ -z "$ENTRY" ]]; then
    echo "ERROR: no private/mtd/mtd0-ALL.bin found in $ARCHIVE" >&2
    exit 1
fi

tar -xOf "$ARCHIVE" "$ENTRY" > "$OUTPUT"

python3 - "$OUTPUT" <<'PY'
from pathlib import Path
import hashlib, struct, sys
p=Path(sys.argv[1])
b=p.read_bytes()
if len(b) != 0x800000:
    raise SystemExit(f"ERROR: extracted dump has {len(b)} bytes; expected 8388608")
sig=b[0x30000:0x30004]
if sig not in (b"cr6c", b"cs6c", b"csys"):
    raise SystemExit(f"ERROR: unsupported firmware signature at 0x30000: {sig!r}")
load,burn,length=struct.unpack_from(">III",b,0x30004)
if load != 0x80a00000 or burn != 0x30000:
    raise SystemExit(
        f"ERROR: not an MW5-style image: load=0x{load:08x} burn=0x{burn:08x}"
    )
rootfs=0x30000+0x10+length
if b[rootfs:rootfs+4] != b"hsqs":
    raise SystemExit(f"ERROR: no SquashFS at absolute offset 0x{rootfs:x}")
if b"sys.model=MW5" not in b and b"Mw5;" not in b:
    raise SystemExit("ERROR: dump lacks MW5 model markers; possible foreign Tenda image")
print(f"template={p}")
print(f"bytes={len(b)}")
print(f"sha256={hashlib.sha256(b).hexdigest()}")
print(f"signature={sig.decode()}")
print(f"load=0x{load:08x}")
print(f"burn=0x{burn:08x}")
print(f"rootfs_absolute=0x{rootfs:x}")
PY

sha256sum "$OUTPUT" > "${OUTPUT}.sha256"
printf 'Extracted private template:\n  %s\n  %s\n' "$OUTPUT" "${OUTPUT}.sha256"
