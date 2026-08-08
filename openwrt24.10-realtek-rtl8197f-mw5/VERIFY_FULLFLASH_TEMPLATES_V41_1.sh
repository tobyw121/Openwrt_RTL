#!/usr/bin/env bash
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

python3 - <<'PY'
from pathlib import Path
import hashlib

root = Path('fullflash-templates')
items = [
    ('RD05', root/'xiaomi-r4-rd05-spi.bin', 0x1000000,
     '805469c49ecd6860a444f92ef19bcf4834ee912261b4a0205074d6095c516608',
     [(b'model=RD05', None), (b'cs6c', 0x60000), (b'hsqs', 0x350000)]),
    ('MW5', root/'tenda-nova-mw5-spi.bin', 0x800000,
     'b4b1b2caca98e20096a80519b9c8ff22399d051bc77f6aef4f29b6ef76a31428',
     [(b'sys.model=MW5', None), (b'Mw5;', None), (b'cr6c', 0x30000)]),
    ('AC23/Lynx', root/'tenda-ac23-spi.bin', 0x800000,
     '300c7216dbfa1d7d930a8db896ec755edb0c41bd3c1b3ab90e4befcd045aebf5',
     [(b'TENDA.', None), (b'Lynx_', None), (b'cr6c', 0x30000)]),
]
for name, path, size, expected_hash, checks in items:
    if not path.is_file():
        raise SystemExit(f'FAIL: {name} template missing: {path}')
    data = path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if len(data) != size:
        raise SystemExit(f'FAIL: {name} size 0x{len(data):x}, expected 0x{size:x}')
    if digest != expected_hash:
        raise SystemExit(f'FAIL: {name} SHA-256 {digest}, expected {expected_hash}')
    for marker, offset in checks:
        if offset is None:
            if marker not in data:
                raise SystemExit(f'FAIL: {name} marker missing: {marker!r}')
        elif data[offset:offset+len(marker)] != marker:
            got = data[offset:offset+len(marker)]
            raise SystemExit(f'FAIL: {name} marker at 0x{offset:x}: got {got!r}, expected {marker!r}')
    print(f'PASS: {name:10s} size=0x{size:x} sha256={digest}')
print('PASS: all private full-flash templates are board-matched and intact')
PY
