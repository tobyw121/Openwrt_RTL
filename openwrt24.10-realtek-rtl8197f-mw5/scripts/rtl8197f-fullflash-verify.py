#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Offline structural verifier for RTL8197F private SPI templates/images."""
from __future__ import annotations
import argparse, hashlib, struct
from pathlib import Path

def s16(data: bytes) -> int:
    total=0
    for i in range(0,len(data)&~1,2): total=(total+((data[i]<<8)|data[i+1]))&0xffff
    if len(data)&1: total=(total+(data[-1]<<8))&0xffff
    return total

def sha(data: bytes) -> str: return hashlib.sha256(data).hexdigest()

def header(blob: bytes, off: int, load: int, burn: int) -> tuple[int,int]:
    sig, got_load, got_burn, length=struct.unpack_from('>4sIII',blob,off)
    if sig not in (b'cr6c',b'cs6c',b'csys'): raise ValueError(f'bad signature {sig!r}')
    if got_load != load or got_burn != burn: raise ValueError(f'bad load/burn 0x{got_load:x}/0x{got_burn:x}')
    end=off+16+length
    if end>len(blob): raise ValueError('header length outside image')
    if s16(blob[off+16:end]): raise ValueError('kernel checksum failed')
    return length,end

def main() -> int:
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--board',required=True,choices=('rd05','mw5','ac23'))
    ap.add_argument('--template',required=True,type=Path)
    ap.add_argument('--image',type=Path)
    args=ap.parse_args()
    t=args.template.read_bytes()
    sizes={'rd05':0x1000000,'mw5':0x800000,'ac23':0x800000}
    if len(t)!=sizes[args.board]: raise SystemExit(f'ERROR: template size 0x{len(t):x}')
    if args.board=='rd05':
        length,end=header(t,0x60000,0x80cf0000,0x60000)
        if t[0x350000:0x350004]!=b'hsqs': raise SystemExit('ERROR: RD05 fixed rootfs magic missing')
        preserved=((0,0x60000),(0xe60000,0x1000000))
    elif args.board=='mw5':
        length,end=header(t,0x30000,0x80a00000,0x30000)
        if t[end:end+4]!=b'hsqs': raise SystemExit('ERROR: MW5 header-derived rootfs magic missing')
        if not (b'sys.model=MW5' in t or b'Mw5;' in t): raise SystemExit('ERROR: MW5 marker missing')
        preserved=((0,0x30000),(0x5c0000,0x800000))
    else:
        length,end=header(t,0x30000,0x80a00000,0x30000)
        if not (b'TENDA.' in t and b'Lynx_' in t): raise SystemExit('ERROR: AC23 marker missing')
        if any(t[0x30010:0x3003c]) or t[0x3003c]==0: raise SystemExit('ERROR: AC23 60-byte header layout mismatch')
        preserved=((0,0x30000),(0x7e0000,0x800000))
    print(f'{args.board}: template OK size=0x{len(t):x} sha256={sha(t)} kernel_len=0x{length:x}')
    if args.image:
        i=args.image.read_bytes()
        if len(i)!=len(t): raise SystemExit('ERROR: image size mismatch')
        for start,stop in preserved:
            if i[start:stop]!=t[start:stop]: raise SystemExit(f'ERROR: preserved range 0x{start:x}..0x{stop:x} differs')
        print(f'{args.board}: fullflash preservation OK sha256={sha(i)}')
    return 0
if __name__=='__main__': raise SystemExit(main())
