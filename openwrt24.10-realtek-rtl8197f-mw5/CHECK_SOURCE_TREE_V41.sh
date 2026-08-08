#!/usr/bin/env bash
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

fail=0
bad() { echo "FAIL: $*" >&2; fail=1; }
ok() { echo "PASS: $*"; }

for d in build_dir staging_dir tmp bin feeds package/feeds; do
    [[ ! -e "$d" ]] || bad "generated directory present: $d"
done
[[ ! -e .config ]] || bad "generated root .config present"

for f in \
  target/linux/realtek/dts/rtl8197f_xiaomi_r4-rd05.dts \
  target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts \
  target/linux/realtek/dts/rtl8197f_tenda_ac23.dts \
  target/linux/realtek/patches-6.6/328-net-dsa-realtek-rtl8365mb-add-mw5-rtl8363-family.patch \
  target/linux/generic/files/drivers/mtd/mtdsplit/mtdsplit_rtl8197f.c \
  package/kernel/rtl8192cd-mw5/Makefile \
  fullflash-templates/xiaomi-r4-rd05-spi.bin \
  fullflash-templates/tenda-nova-mw5-spi.bin \
  fullflash-templates/tenda-ac23-spi.bin; do
    [[ -f "$f" ]] || bad "missing $f"
done

grep -q '^ARCH:=mips$' target/linux/realtek/Makefile || bad "Realtek target root architecture missing"
grep -q '^ARCH:=mipsel$' target/linux/realtek/rtl8197f/target.mk || bad "RTL8197F little-endian subtarget architecture missing"

# Keep the clean upstream package generations coherent. These filenames are
# valid in this source generation; the historical failures were caused by
# mixing them with files from other package generations.
grep -q '^PKG_VERSION:=3\.6\.6$' package/libs/mbedtls/Makefile || bad "unexpected mbedTLS package version"
grep -q '^PKG_RELEASE:=2$' package/libs/mbedtls/Makefile || bad "unexpected mbedTLS package release"
grep -q '^PKG_VERSION:=2024\.86$' package/network/services/dropbear/Makefile || bad "unexpected Dropbear package version"
grep -q '^PKG_RELEASE:=3$' package/network/services/dropbear/Makefile || bad "unexpected Dropbear package release"
grep -q '^PKG_SOURCE_VERSION:=b14cf98c914d8582f18c7b263814797366fa185d$' package/network/services/odhcpd/Makefile || bad "unexpected odhcpd source revision"
grep -q '^PKG_RELEASE:=3$' package/network/services/odhcpd/Makefile || bad "unexpected odhcpd package release"

python3 - <<'PY' || exit 1
from pathlib import Path
from collections import defaultdict
import hashlib, re, sys
root=Path('.')
failed=False
for d in root.rglob('patches'):
    if not d.is_dir():
        continue
    seen=defaultdict(list)
    for p in d.glob('*.patch'):
        seen[hashlib.sha256(p.read_bytes()).hexdigest()].append(p.name)
    for names in seen.values():
        if len(names)>1:
            print(f"FAIL: duplicate patch contents in {d}: {', '.join(sorted(names))}", file=sys.stderr)
            failed=True

# Verify unified-diff hunk line counts in the Realtek target patch series.
hunk_re=re.compile(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@')
for p in Path('target/linux/realtek/patches-6.6').glob('*.patch'):
    lines=p.read_text(errors='surrogateescape').splitlines()
    i=0
    while i < len(lines):
        m=hunk_re.match(lines[i])
        if not m:
            i += 1
            continue
        old_expected=int(m.group(2) or 1)
        new_expected=int(m.group(4) or 1)
        old_seen=new_seen=0
        i += 1
        while i < len(lines) and not lines[i].startswith('@@ '):
            line=lines[i]
            if line.startswith('diff --git ') or line.startswith('--- '):
                break
            if line.startswith('\\ No newline at end of file'):
                i += 1
                continue
            if line.startswith(' '):
                old_seen += 1; new_seen += 1
            elif line.startswith('-'):
                old_seen += 1
            elif line.startswith('+'):
                new_seen += 1
            else:
                # Mail headers or metadata terminate malformed hunks.
                break
            i += 1
        if (old_seen,new_seen)!=(old_expected,new_expected):
            print(f"FAIL: malformed hunk in {p}: expected {old_expected}/{new_expected}, got {old_seen}/{new_seen}", file=sys.stderr)
            failed=True
if failed:
    raise SystemExit(1)
print('PASS: no duplicate patch contents and Realtek patch hunks are well formed')
PY

while IFS= read -r -d '' f; do
    bash -n "$f" || bad "shell syntax: $f"
done < <(find . -maxdepth 1 -type f \( -name 'BUILD_*V41*.sh' -o -name 'CHECK_SOURCE_TREE_V41.sh' -o -name 'VERIFY_FULLFLASH_TEMPLATES_V41_1.sh' \) -print0; find scripts -maxdepth 1 -type f -name 'rtl8197f-*.sh' -print0)

while IFS= read -r -d '' f; do
    python3 - "$f" <<'PY_CHECK' || bad "Python syntax: $f"
import ast, pathlib, sys
ast.parse(pathlib.Path(sys.argv[1]).read_text(encoding="utf-8", errors="surrogateescape"), filename=sys.argv[1])
PY_CHECK
done < <(find scripts -maxdepth 2 -type f \( -name 'rtl8197f-*.py' -o -name 'realtek-sdk-port-manifest.py' -o -name 'rd05-pc-netdiag.py' \) -print0)


./VERIFY_FULLFLASH_TEMPLATES_V41_1.sh || bad "private full-flash template verification"

grep -q '^RTL8197F_BUILD_FULLFLASH ?= 1$' target/linux/realtek/image/Makefile || bad "full-flash default is not enabled"
grep -q 'squashfs-spi-full.bin' target/linux/realtek/image/rtl8197f.mk || bad "full-flash image definitions missing"

if find . -type f \( -name '*.rej' -o -name '*.orig' \) -print -quit | grep -q .; then
    bad "reject or backup files present"
fi

if (( fail )); then
    exit 1
fi
ok "source tree static checks"
