#!/usr/bin/env python3
"""Generate derived RTL8367 register-map indices from a user-supplied Realtek SDK.

The script intentionally emits symbol/value/source-line indices and inventories,
not vendor implementation source. This keeps the OpenWrt driver auditable and
avoids mixing unknown/proprietary SDK licensing into GPL kernel code.
"""
from __future__ import annotations
import argparse, csv, hashlib, os, re
from pathlib import Path

DEFINE_RE = re.compile(r"^\s*#\s*define\s+([A-Za-z_][A-Za-z0-9_]*)(\([^)]*\))?\s*(.*?)\s*$")
SIMPLE_NUM_RE = re.compile(r"^\(?\s*(0[xX][0-9A-Fa-f]+|[0-9]+)[uUlL]*\s*\)?$")

def sha256(path: Path) -> str:
    h=hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda:f.read(1024*1024), b''):
            h.update(block)
    return h.hexdigest()

def logical_lines(path: Path):
    physical=path.read_text(errors='replace').splitlines()
    i=0
    while i < len(physical):
        start=i+1
        text=physical[i]
        while text.rstrip().endswith('\\') and i+1 < len(physical):
            text=text.rstrip()[:-1]+' '+physical[i+1].strip()
            i += 1
        yield start, text
        i += 1

def classify(name: str) -> str:
    n=name.upper()
    if '_REG_' in n or n.endswith('_REG') or n.startswith(('RTL8367B_REG','RTL8367C_REG','RTL8367D_REG')):
        return 'REGISTER'
    if n.endswith('_MASK') or '_MASK_' in n:
        return 'MASK'
    if n.endswith('_OFFSET') or '_OFFSET_' in n or n.endswith('_SHIFT') or '_SHIFT_' in n:
        return 'OFFSET_SHIFT'
    if 'PORT' in n:
        return 'PORT_TOPOLOGY'
    if 'CHIP' in n or 'MAGIC' in n:
        return 'IDENTITY_RESET'
    return 'OTHER'

def parse_header(family: str, path: Path, out: Path):
    rows=[]
    for line_no, line in logical_lines(path):
        m=DEFINE_RE.match(line)
        if not m:
            continue
        name,args,value=m.groups()
        value=re.sub(r'/\*.*?\*/', '', value).strip()
        value=re.sub(r'//.*$', '', value).strip()
        numeric=''
        mm=SIMPLE_NUM_RE.match(value)
        if mm:
            try: numeric=str(int(mm.group(1),0))
            except ValueError: pass
        rows.append((family,name,args or '',classify(name),value,numeric,line_no))
    with out.open('w',newline='',encoding='utf-8') as f:
        w=csv.writer(f,delimiter='\t',lineterminator='\n')
        w.writerow(['family','symbol','arguments','kind','raw_value','numeric_value_decimal','source_line'])
        w.writerows(rows)
    return len(rows)

def inventory(roots, out: Path):
    rows=[]
    for family,root in roots:
        for p in sorted(root.rglob('*')):
            if not p.is_file(): continue
            try:
                data=p.read_bytes()
                lines=data.count(b'\n') + (1 if data and not data.endswith(b'\n') else 0)
            except OSError:
                continue
            rows.append((family,str(p.relative_to(root)),p.suffix.lower(),len(data),lines,sha256(p)))
    with out.open('w',newline='',encoding='utf-8') as f:
        w=csv.writer(f,delimiter='\t',lineterminator='\n')
        w.writerow(['family','relative_path','suffix','bytes','lines','sha256'])
        w.writerows(rows)
    return len(rows)

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument('--sdk-root',type=Path,required=True)
    ap.add_argument('--output',type=Path,required=True)
    a=ap.parse_args()
    base=a.sdk_root/'8197/RT8197F_RTL8367RB_Gesamtpaket/AX12v1_GPL_Paket/source_subset/rtl8197/rtknet/drivers/net/rtl819x'
    headers={
      'B': a.sdk_root/'8197/rtl819x/rtl8367r/rtl8367b_reg.h',
      'C': base/'rtl83xx_v1dot4/dal/rtl8367c/rtl8367c_reg.h',
      'D': base/'rtl83xx_v1dot4/dal/rtl8367d/rtl8367d_reg.h',
    }
    roots=[
      ('B-map legacy API',a.sdk_root/'8197/rtl819x/rtl8367r'),
      ('C-map v1.4 DAL',base/'rtl83xx_v1dot4/dal/rtl8367c'),
      ('D-map v1.4 DAL',base/'rtl83xx_v1dot4/dal/rtl8367d'),
    ]
    a.output.mkdir(parents=True,exist_ok=True)
    meta=[]
    for family,path in headers.items():
        if not path.is_file(): raise SystemExit(f'missing header: {path}')
        n=parse_header(family,path,a.output/f'rtl8367{family.lower()}-register-map.tsv')
        meta.append((family,str(path),sum(1 for _ in path.open(errors='replace')),n,sha256(path)))
    inv=inventory(roots,a.output.parent/'RTL8367_DRIVER_SOURCE_INVENTORY.tsv')
    with (a.output/'REGISTER_MAP_SOURCES.tsv').open('w',newline='',encoding='utf-8') as f:
        w=csv.writer(f,delimiter='\t',lineterminator='\n')
        w.writerow(['family','source_path','source_lines','indexed_defines','source_sha256'])
        w.writerows(meta)
    print('register maps:', ', '.join(f'{x[0]}={x[3]}' for x in meta))
    print('inventory files:',inv)
if __name__=='__main__': main()
