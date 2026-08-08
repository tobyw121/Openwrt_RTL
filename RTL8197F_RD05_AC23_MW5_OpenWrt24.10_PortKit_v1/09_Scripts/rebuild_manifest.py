#!/usr/bin/env python3
from __future__ import annotations
import csv, hashlib, pathlib, sys
root = pathlib.Path(sys.argv[1] if len(sys.argv) > 1 else ".").resolve()
out = root / "08_Indexes" / "MANIFEST.tsv"
rows=[]
for p in sorted(root.rglob("*")):
    if not p.is_file() or p == out:
        continue
    h=hashlib.sha256()
    with p.open("rb") as f:
        for chunk in iter(lambda:f.read(1024*1024), b""):
            h.update(chunk)
    rows.append((p.relative_to(root).as_posix(), p.stat().st_size, h.hexdigest()))
out.parent.mkdir(parents=True, exist_ok=True)
with out.open("w", newline="", encoding="utf-8") as f:
    w=csv.writer(f, delimiter="\t")
    w.writerow(("path","size_bytes","sha256"))
    w.writerows(rows)
print(f"wrote {out} with {len(rows)} files")
