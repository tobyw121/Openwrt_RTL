#!/usr/bin/env python3
"""Read-only scanner for Realtek/Tenda flash signatures and common filesystems."""
from __future__ import annotations
import argparse, hashlib, pathlib, struct

SIGS = {
    b"cr6c": "Realtek kernel+rootfs header",
    b"cs6c": "Realtek kernel header",
    b"root": "Realtek rootfs header",
    b"hsqs": "SquashFS little-endian",
    b"sqsh": "SquashFS big-endian/legacy",
    b"nice": "Tenda-modified SquashFS magic",
    b"\x85\x19": "JFFS2 node magic",
    b"\xfd7zXZ\x00": "XZ stream",
    b"Tenda\x00": "Tenda-modified XZ marker",
}

def find_all(data: bytes, needle: bytes, limit: int = 32):
    start = 0
    out = []
    while len(out) < limit:
        pos = data.find(needle, start)
        if pos < 0:
            break
        out.append(pos)
        start = pos + 1
    return out

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("image", type=pathlib.Path)
    args = ap.parse_args()
    data = args.image.read_bytes()
    print(f"file={args.image}")
    print(f"size={len(data)}")
    print(f"sha256={hashlib.sha256(data).hexdigest()}")
    for sig, desc in SIGS.items():
        for pos in find_all(data, sig):
            print(f"0x{pos:08x} {desc}")
            if sig in (b"cr6c", b"cs6c", b"root") and pos + 16 <= len(data):
                magic, load, burn, length = struct.unpack_from(">4sIII", data, pos)
                print(f"  magic={magic.decode(errors='replace')} load=0x{load:08x} burn=0x{burn:08x} length=0x{length:x}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
