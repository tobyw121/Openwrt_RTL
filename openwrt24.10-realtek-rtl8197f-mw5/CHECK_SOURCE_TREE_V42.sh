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
[[ ! -e .config ]] || bad 'generated root .config present'

required=(
  target/linux/realtek/dts/rtl8197f_xiaomi_r4-rd05.dts
  target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts
  target/linux/realtek/dts/rtl8197f_tenda_ac23.dts
  target/linux/realtek/rtl8197f/board-profiles-v42.json
  target/linux/realtek/patches-6.6/328-net-dsa-realtek-rtl8365mb-add-mw5-rtl8363-family.patch
  package/kernel/rtl8192cd-rtl8197f/Makefile
  package/kernel/rtl8192cd-rtl8197f/src/rtl8197f_platform_glue.c
  package/kernel/rtl8192cd-rtl8197f/files/usr/sbin/rtl8197f-wlan-load
  package/kernel/rtl8192cd-rtl8197f/files/usr/sbin/rtl8197f-wlan-ap-test
  package/kernel/rtl8192cd-rtl8197f/WLAN_INIT_PHASES_V42.md
  MW5_WLAN_RECONSTRUCTION_V42.md
  V42_4_CHANGELOG.md
  V42_5_CHANGELOG.md
  V42_6_CHANGELOG.md
  V42_7_CHANGELOG.md
  V42_8_CHANGELOG.md
  V42_15_CHANGELOG.md
  V42_17_CHANGELOG.md
  V42_18_CHANGELOG.md
  V42_20_CHANGELOG.md
  V42_21_CHANGELOG.md
  V42_22_CHANGELOG.md
  V42_23_CHANGELOG.md
  V42_24_CHANGELOG.md
  V42_25_CHANGELOG.md
  V42_26_CHANGELOG.md
  V42_27_CHANGELOG.md
  RTL8197F_v42.4_VALIDATION.txt
  RTL8197F_v42.5_VALIDATION.txt
  RTL8197F_v42.6_VALIDATION.txt
  RTL8197F_v42.8_VALIDATION.txt
  RTL8197F_v42.15_VALIDATION.txt
  RTL8197F_v42.17_VALIDATION.txt
  RTL8197F_v42.17_DIAGNOSTIC.txt
  RTL8197F_v42.18_VALIDATION.txt
  RTL8197F_v42.18_DIAGNOSTIC.txt
  RTL8197F_v42.20_VALIDATION.txt
  RTL8197F_v42.21_VALIDATION.txt
  RTL8197F_v42.22_VALIDATION.txt
  RTL8197F_v42.22_DIAGNOSTIC.txt
  RTL8197F_v42.23_VALIDATION.txt
  RTL8197F_v42.23_DIAGNOSTIC.txt
  RTL8197F_v42.24_VALIDATION.txt
  RTL8197F_v42.24_DIAGNOSTIC.txt
  RTL8197F_v42.25_VALIDATION.txt
  RTL8197F_v42.25_DIAGNOSTIC.txt
  RTL8197F_v42.26_VALIDATION.txt
  RTL8197F_v42.26_DIAGNOSTIC.txt
  RTL8197F_v42.27_VALIDATION.txt
  RTL8197F_v42.27_DIAGNOSTIC.txt
  target/linux/realtek/patches-6.6/321-mips-rtl8197f-stop-watchdog-at-kernel-entry.patch
  target/linux/realtek/patches-6.6/322-serial-8250-dw-add-rtl8197fs-tx-alias.patch
  scripts/rtl8197f-tenda-mw5-fullflash.py
  scripts/rtl8197f-tenda-lynx-fullflash.py
  scripts/rtl8197f-fullflash-verify.py
  scripts/rtl8197f-firmware-budget.py
  scripts/rtl8197f-loader-overlap-check.py
  scripts/rtl8197f-spi-program-verify.sh
  scripts/check-rtl8192cd-sha256.sh
  fullflash-templates/xiaomi-r4-rd05-spi.bin
  fullflash-templates/tenda-nova-mw5-spi.bin
  fullflash-templates/tenda-ac23-spi.bin
)
for f in "${required[@]}"; do [[ -f "$f" ]] || bad "missing $f"; done

wlan_dirs=(
  package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/Data/8197F
  package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/Data/8822B
  package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/Data/8812F
  package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/Data/8814B
  package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/HalMac88XX/halmac_88xx/halmac_8822b
  package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/HalMac88XX/halmac_88xx/halmac_8812f
  package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/HalMac88XX/halmac_88xx_v1/halmac_8814b
)
for d in "${wlan_dirs[@]}"; do
  [[ -d "$d" ]] || bad "missing WLAN source/data directory $d"
done
for chip in 8197F 8822B 8812F 8814B; do
  count=$(find "package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/Data/$chip" -type f 2>/dev/null | wc -l)
  (( count > 0 )) || bad "empty WLAN data directory for $chip"
done

grep -q '^ARCH:=mips$' target/linux/realtek/Makefile || bad 'Realtek target architecture missing'
grep -q '^ARCH:=mipsel$' target/linux/realtek/rtl8197f/target.mk || bad 'RTL8197F mipsel subtarget missing'
grep -q 'partition@30000' target/linux/realtek/dts/rtl8197f_tenda_ac23.dts || bad 'AC23 firmware window missing'
grep -q 'reg = <0x030000 0x7b0000>' target/linux/realtek/dts/rtl8197f_tenda_ac23.dts || bad 'AC23 firmware size mismatch'
grep -q 'gpio0 33 GPIO_ACTIVE_LOW' target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts || bad 'MW5 status LED GPIO missing'
grep -q 'gpio0 54 GPIO_ACTIVE_LOW' target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts || bad 'MW5 reset GPIO missing'
grep -A12 '^&uart0' target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts | grep -q 'reg-io-width = <1>' || bad 'MW5 UART byte I/O override missing'
grep -A14 '^&uart0' target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts | grep -q 'realtek,rx-alias-offset = <0x24>' || bad 'MW5 RTL8197FS RX alias missing'
grep -A14 '^&uart0' target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts | grep -q 'realtek,tx-alias-offset = <0x24>' || bad 'MW5 RTL8197FS TX alias missing'
grep -q 'kmod-rtl8192cd-rtl8197f-mw5' target/linux/realtek/image/rtl8197f.mk || bad 'MW5 WLAN variant not selected'
grep -q 'BLOCKSIZE := 64k' target/linux/realtek/image/rtl8197f.mk || bad 'Tenda 64 KiB erase alignment missing'
grep -q 'rtl8197f-firmware-budget' target/linux/realtek/image/rtl8197f.mk || bad 'strict Tenda firmware budget gate missing'
grep -q -- '-odhcpd-ipv6only' target/linux/realtek/image/rtl8197f.mk || bad 'MW5 flash-safe package exclusions missing'
grep -q 'CONFIG_TARGET_SQUASHFS_BLOCK_SIZE=1024' configs/rtl8197f_mw5_defconfig || bad 'MW5 1 MiB SquashFS blocks missing'
grep -A45 '^define Device/tenda_nova_mw5$' target/linux/realtek/image/rtl8197f.mk | grep -q 'RTL8197F_HEADER_SIZE := 16' || bad 'MW5 logical 16-byte header missing'
if grep -A45 '^define Device/tenda_nova_mw5$' target/linux/realtek/image/rtl8197f.mk | grep -q 'RTL8197F_LEN_INCLUDES_HEADER_PAD'; then
  bad 'MW5 still emits the OEM 40-byte payload prefix'
fi
grep -q 'require_immediate_payload=True' scripts/rtl8197f-tenda-mw5-fullflash.py || bad 'MW5 shifted-loader guard missing'
grep -q 'BOOTLOADER_LENGTH_BIAS = 0x282' scripts/rtl8197f-rootfs-checksum.py || bad 'MW5 rootfs boot-length bias missing'
grep -q 'cache_size = lzma_outsize;' target/linux/realtek/image/lzma-loader/src/loader.c || bad 'post-LZMA cache-size reload missing'
! grep -q 'stream-dependent invalid cache range' target/linux/realtek/image/lzma-loader/src/loader.c || bad 'obsolete v42.24 cache diagnosis remains'
grep -q 'rootfs_bootloader_marker' scripts/rtl8197f-tenda-mw5-fullflash.py || bad 'MW5 fullflash rootfs marker validation missing'
grep -q 'kmod-rtl8192cd-rtl8197f-rd05' target/linux/realtek/image/rtl8197f.mk || bad 'RD05 WLAN variant not selected'
grep -q 'kmod-rtl8192cd-rtl8197f-ac23' target/linux/realtek/image/rtl8197f.mk || bad 'AC23 WLAN variant not selected'
grep -q 'VARIANT:=mw5' package/kernel/rtl8192cd-rtl8197f/Makefile || bad 'MW5 WLAN package variant missing'
grep -q 'VARIANT:=rd05' package/kernel/rtl8192cd-rtl8197f/Makefile || bad 'RD05 WLAN package variant missing'
grep -q 'VARIANT:=ac23' package/kernel/rtl8192cd-rtl8197f/Makefile || bad 'AC23 WLAN package variant missing'
grep -q 'RTL8197F_CPP_CONFIGS' package/kernel/rtl8192cd-rtl8197f/src/Makefile || bad 'Kbuild/C preprocessor synchronization missing'
grep -q 'pcie-driver-skipped' package/kernel/rtl8192cd-rtl8197f/src/8192cd_osdep.c || bad 'external-radio gate missing'
grep -q 'netdev-open-complete' package/kernel/rtl8192cd-rtl8197f/src/8192cd_osdep.c || bad 'WLAN open phase telemetry missing'
grep -q 'rf-calibration-complete' package/kernel/rtl8192cd-rtl8197f/src/8192cd_hw.c || bad 'WLAN RF phase telemetry missing'
grep -q 'RTL8822B(10ec:b822),wlan0,RFE6' package/kernel/rtl8192cd-rtl8197f/files/etc/rtl8197f-wlan-profiles.conf || bad 'MW5 OEM radio mapping missing'
grep -q '+wireless-tools' package/kernel/rtl8192cd-rtl8197f/Makefile || bad 'iwpriv dependency missing' 
grep -q '#include "typedef.h"' package/kernel/rtl8192cd-rtl8197f/src/sha256.c || bad 'self-contained SHA-256 helper missing'
grep -q 'rtl8192cd-objs += rtl8197f_platform_glue.o sha256.o' package/kernel/rtl8192cd-rtl8197f/src/Makefile || bad 'SHA-256 object not linked into rtl8192cd'

python3 - <<'PY' || bad 'WLAN variant/source validation'
from pathlib import Path

root = Path('.')
pkg = (root / 'package/kernel/rtl8192cd-rtl8197f/Makefile').read_text(errors='surrogateescape')
image = (root / 'target/linux/realtek/image/rtl8197f.mk').read_text(errors='surrogateescape')
srcmk = (root / 'package/kernel/rtl8192cd-rtl8197f/src/Makefile').read_text(errors='surrogateescape')
hal_path = root / 'package/kernel/rtl8192cd-rtl8197f/src/WlanHAL/RTL88XX/Hal88XXGen.c'
hal = hal_path.read_text(errors='surrogateescape')
sha = (root / 'package/kernel/rtl8192cd-rtl8197f/src/sha256.c').read_text(errors='surrogateescape')

variants = {
    'mw5': ('CONFIG_WLAN_HAL_8822BE=y', 'kmod-rtl8192cd-rtl8197f-mw5'),
    'rd05': ('CONFIG_WLAN_HAL_8812FE=y', 'kmod-rtl8192cd-rtl8197f-rd05'),
    'ac23': ('CONFIG_WLAN_HAL_8814BE=y', 'kmod-rtl8192cd-rtl8197f-ac23'),
}
for name, (symbol, package) in variants.items():
    if f'VARIANT:={name}' not in pkg or symbol not in pkg or package not in image:
        raise SystemExit(f'incomplete WLAN variant mapping: {name}')

for symbol in ('CONFIG_WLAN_HAL_8197F', 'CONFIG_WLAN_HAL_8822BE',
               'CONFIG_WLAN_HAL_8812FE', 'CONFIG_WLAN_HAL_8814BE'):
    if symbol not in srcmk:
        raise SystemExit(f'missing C preprocessor synchronization symbol: {symbol}')

case = hal.index('case HW_VAR_POWERLIMITFILE_SIZE:')
start = hal.index('#ifdef TXPWR_LMT_8812F', case)
end = hal.index('#ifdef TXPWR_LMT_8192F', start)
segment = hal[start:end]
if segment.count('{') != segment.count('}'):
    raise SystemExit('unbalanced RTL8812F power-limit block')
if 'dma_handle %pad' not in hal or '&dma_handle_amsdu' not in hal:
    raise SystemExit('dma_addr_t printk correction missing')
if '(HAL_RTL_R8(REG_RX_DRVINFO_SZ) | BIT_APP_MH_SHIFT_VAL) & ~BIT_WMAC_ENSHIFT' not in hal:
    raise SystemExit('RX driver-info mask precedence correction missing')
if '#if defined(CONFIG_IEEE80211W) || defined(CONFIG_IEEE80211R)' in sha:
    raise SystemExit('SHA-256 implementation is still feature-gated and may build empty')
for symbol in ('sha256_vector', 'hmac_sha256_vector', 'sha256_prf_bits', 'sha256_prf'):
    if symbol not in sha:
        raise SystemExit(f'missing local SHA-256 symbol: {symbol}')
print('PASS: WLAN variants, preprocessor symbols, RTL8812F block and local SHA-256 linkage')
PY

TMP_BUDGET=$(mktemp -d)
truncate -s $((0x5f8112)) "$TMP_BUDGET/oversize.bin"
if python3 scripts/rtl8197f-firmware-budget.py \
    --image "$TMP_BUDGET/oversize.bin" --limit 5696k --erase-size 64k \
    --label tenda_nova_mw5 >"$TMP_BUDGET/oversize.log" 2>&1; then
  bad 'MW5 budget checker accepted the v42.3 oversized image'
fi
grep -q 'over budget: 0x70000' "$TMP_BUDGET/oversize.log" || bad 'MW5 oversize calculation mismatch'
truncate -s $((0x588001)) "$TMP_BUDGET/fits.bin"
python3 scripts/rtl8197f-firmware-budget.py \
  --image "$TMP_BUDGET/fits.bin" --limit 5696k --erase-size 64k \
  --label tenda_nova_mw5 >"$TMP_BUDGET/fits.log" || bad 'MW5 fitting image rejected'
grep -q 'padded=0x590000' "$TMP_BUDGET/fits.log" || bad 'MW5 64 KiB padding calculation mismatch'
rm -rf "$TMP_BUDGET"

python3 - <<'PY' || bad 'MW5 header-layout regression'
from importlib.util import module_from_spec, spec_from_file_location
from pathlib import Path
import sys

path = Path('scripts/rtl8197f-tenda-mw5-fullflash.py')
spec = spec_from_file_location('mw5_fullflash', path)
mod = module_from_spec(spec)
sys.modules[spec.name] = mod
spec.loader.exec_module(mod)
spi = Path('fullflash-templates/tenda-nova-mw5-spi.bin').read_bytes()
stock = spi[mod.FIRMWARE_OFFSET:mod.FIRMWARE_OFFSET + mod.FIRMWARE_SIZE]
mod.parse_firmware(stock, 'stock template')
try:
    mod.parse_firmware(stock, 'generated OpenWrt firmware', require_immediate_payload=True)
except SystemExit as exc:
    if '0x80a00028' not in str(exc):
        raise
else:
    raise SystemExit('shifted MW5 payload was accepted')
print('PASS: MW5 stock template accepted only as template; shifted OpenWrt payload rejected')
PY

python3 - <<'PY' || bad 'MW5 rootfs boot-length marker regression'
from importlib.util import module_from_spec, spec_from_file_location
from pathlib import Path
import struct, sys

rootfs_path = Path('scripts/rtl8197f-rootfs-checksum.py')
spec = spec_from_file_location('rtl_rootfs', rootfs_path)
mod = module_from_spec(spec)
sys.modules[spec.name] = mod
spec.loader.exec_module(mod)

source = bytearray(0x12345)
source[:4] = b'hsqs'
struct.pack_into('<HH', source, 28, 4, 0)
struct.pack_into('<Q', source, 40, len(source))
built, info = mod.build_rootfs(bytes(source), 'synthetic rootfs')
checked = mod.check_rootfs(built, 'synthetic built rootfs', allow_trailing=False)
if checked != info:
    raise SystemExit('MW5 rootfs marker build/check metadata mismatch')
if info.bootloader_marker + 0x282 != info.total_size:
    raise SystemExit('MW5 rootfs marker does not reproduce checksum length')
if info.bytes_used != len(source) or info.original_bytes_used != len(source):
    raise SystemExit('SquashFS bytes_used was modified to include the OEM trailer')
if struct.unpack_from('<Q', built, 40)[0] != len(source):
    raise SystemExit('built SquashFS superblock bytes_used changed')
if info.checksum_offset != mod.align_up(len(source)):
    raise SystemExit('OEM checksum is not outside SquashFS at the aligned offset')

bad = bytearray(built)
struct.pack_into('>I', bad, 8, 0x6a6ca27e)
try:
    mod.check_rootfs(bytes(bad), 'timestamp marker', allow_trailing=False)
except ValueError as exc:
    if 'bootloader would check' not in str(exc):
        raise
else:
    raise SystemExit('timestamp-style MW5 rootfs marker was accepted')

spi = Path('fullflash-templates/tenda-nova-mw5-spi.bin').read_bytes()
fw = 0x30000
length = struct.unpack_from('>I', spi, fw + 12)[0]
root = fw + 16 + length
stock_marker = struct.unpack_from('>I', spi, root + 8)[0]
if stock_marker != 0x002f4d80 or stock_marker + 0x282 != 0x002f5002:
    raise SystemExit('stock MW5 rootfs marker no longer matches recovered bootloader rule')
print('PASS: MW5 rootfs boot-length marker generation, rejection and stock-template rule')
PY

python3 - <<'PY' || bad 'SquashFS bytes_used/OEM trailer regression'
from pathlib import Path

checksum = Path('scripts/rtl8197f-rootfs-checksum.py').read_text()
mw5 = Path('scripts/rtl8197f-tenda-mw5-fullflash.py').read_text()
lynx = Path('scripts/rtl8197f-tenda-lynx-fullflash.py').read_text()
image = Path('target/linux/realtek/image/rtl8197f.mk').read_text()

for stale in ('new_bytes_used = old_bytes_used + CHECKSUM_SIZE',
              'bytes_used - CHECKSUM_SIZE',
              'rootfs_bytes_used - 2',
              'original filesystem length plus two'):
    if stale in checksum + mw5 + lynx + image:
        raise SystemExit(f'stale bytes_used+2 trailer encoding remains: {stale}')
for token in ('checksum_offset = align_up(old_bytes_used)',
              'original_bytes_used = bytes_used',
              'if any(data[bytes_used:checksum_offset])'):
    if token not in checksum:
        raise SystemExit(f'missing standards-compliant rootfs token: {token}')
if '(rootfs_bytes_used + 0xFFF) & ~0xFFF' not in mw5:
    raise SystemExit('MW5 checksum offset is not derived from normal bytes_used')
if '(rootfs_bytes_used + 0xFFF) & ~0xFFF' not in lynx:
    raise SystemExit('AC23/Lynx checksum offset is not derived from normal bytes_used')
print('PASS: SquashFS bytes_used excludes the external Tenda checksum trailer')
PY

python3 - <<'PY' || bad 'patch validation'
from pathlib import Path
from collections import defaultdict
import hashlib, re
failed = False
for d in Path('.').rglob('patches'):
    if not d.is_dir():
        continue
    seen = defaultdict(list)
    for p in d.glob('*.patch'):
        seen[hashlib.sha256(p.read_bytes()).hexdigest()].append(p.name)
    for names in seen.values():
        if len(names) > 1:
            print(f"duplicate patch contents in {d}: {', '.join(sorted(names))}")
            failed = True
hunk_re = re.compile(r'^@@ -(\d+)(?:,(\d+))? \+(\d+)(?:,(\d+))? @@')
for p in Path('target/linux/realtek/patches-6.6').glob('*.patch'):
    lines = p.read_text(errors='surrogateescape').splitlines()
    i = 0
    while i < len(lines):
        m = hunk_re.match(lines[i])
        if not m:
            i += 1
            continue
        oe, ne = int(m.group(2) or 1), int(m.group(4) or 1)
        os = ns = 0
        i += 1
        while i < len(lines) and not lines[i].startswith('@@ '):
            line = lines[i]
            if line.startswith('diff --git ') or line.startswith('--- '):
                break
            if line.startswith('\\ No newline at end of file'):
                i += 1
                continue
            if line.startswith(' '): os += 1; ns += 1
            elif line.startswith('-'): os += 1
            elif line.startswith('+'): ns += 1
            else: break
            i += 1
        if (os, ns) != (oe, ne):
            print(f'malformed hunk in {p}: expected {oe}/{ne}, got {os}/{ns}')
            failed = True
if failed:
    raise SystemExit(1)
print('PASS: patch contents and hunk counts')
PY

for f in \
  CHECK_SOURCE_TREE_V42.sh \
  VERIFY_FULLFLASH_TEMPLATES_V42.sh \
  BUILD_RTL8197F_V42.sh \
  BUILD_ALL_FULLFLASH_V42.sh \
  scripts/rtl8197f-tenda-mw5-auto-fullflash.sh \
  scripts/rtl8197f-tenda-mw5-spi-build.sh \
  scripts/rtl8197f-spi-program-verify.sh \
  scripts/check-rtl8192cd-sha256.sh \
  target/linux/realtek/base-files/usr/bin/rtl8197f-board-check \
  package/kernel/rtl8192cd-rtl8197f/files/usr/sbin/rtl8197f-wlan-load \
  package/kernel/rtl8192cd-rtl8197f/files/usr/sbin/rtl8197f-wlan-ap-test \
  package/kernel/rtl8192cd-rtl8197f/files/usr/bin/rtl8197f-wlan-check; do
  bash -n "$f" || bad "shell syntax: $f"
done

while IFS= read -r -d '' f; do
  python3 - "$f" <<'PY' || bad "Python syntax: $f"
import ast, pathlib, sys
ast.parse(pathlib.Path(sys.argv[1]).read_text(encoding='utf-8', errors='surrogateescape'), filename=sys.argv[1])
PY
done < <(find scripts -maxdepth 2 -type f -name 'rtl8197f-*.py' -print0)


AP_DRYRUN=$(mktemp)
package/kernel/rtl8192cd-rtl8197f/files/usr/sbin/rtl8197f-wlan-ap-test \
  --profile mw5 --band 2g --ssid V42-CHECK --regdomain 1 --open --dry-run \
  >"$AP_DRYRUN"
grep -q 'iwpriv wlan1 set_mib rfe_type=5' "$AP_DRYRUN" || bad 'MW5 2.4 GHz AP dry-run RFE sequence'
grep -q 'iwpriv wlan1 set_mib band=11' "$AP_DRYRUN" || bad 'MW5 2.4 GHz AP dry-run band sequence'
rm -f "$AP_DRYRUN"

bash ./scripts/check-rtl8192cd-sha256.sh || bad 'rtl8192cd local SHA-256/PRF known-answer test'
./VERIFY_FULLFLASH_TEMPLATES_V42.sh || bad 'v42 fullflash/profile verification'
if find . -type f \( -name '*.rej' -o -name '*.orig' \) -print -quit | grep -q .; then
  bad 'reject or backup files present'
fi
(( fail == 0 )) || exit 1
ok 'v42 source tree static checks'

python3 - <<'PY' || bad 'RTL8197F early watchdog/UART regression'
from pathlib import Path
head = Path('target/linux/realtek/image/lzma-loader/src/head.S').read_text()
board = Path('target/linux/realtek/image/lzma-loader/src/board.c').read_text()
prom = Path('target/linux/realtek/files-6.6/arch/mips/rtl838x/prom.c').read_text()
if head.find('0xb800311c') < 0 or head.find('0xa5000000') < 0 or head.find('0x00800000') < 0:
    raise SystemExit('exact SDK kick-then-disable loader watchdog sequence missing')
if '0xa5f00000' in head or '0xa5f00000U' in board or '0xa5f00000U' in prom:
    raise SystemExit('incorrect watchdog value 0xa5f00000 remains')
if head.find('0xb800311c') > head.find('CP0_WATCHLO'):
    raise SystemExit('loader watchdog stop is not before CP0/cache setup')
for token in ('READREG8(UART_LSR_ADDR)', 'UART_THR_F_ADDR', 'UART_BASE + 0x24',
              'WRITEREG8(rtl8197f_uart_thr(), ch)', "board_putc('L')"):
    if token not in board:
        raise SystemExit(f'loader UART/marker token missing: {token}')
for token in ('RTL8197F_WDTCNR_KSEG1', 'RTL8197F_UART_THR_F',
              '__raw_readb', '__raw_writeb', 'rtl8197f_early_wdt_stop();',
              "rtl8197f_early_putc('P');"):
    if token not in prom:
        raise SystemExit(f'kernel early UART/watchdog token missing: {token}')
entry_patch = Path('target/linux/realtek/patches-6.6/321-mips-rtl8197f-stop-watchdog-at-kernel-entry.patch').read_text()
for token in ('rtl8197f_early_mark 0x4b', 'rtl8197f_early_mark 0x53',
              '0xa5000000', '0x00800000', '0xb8147014', '0xb8147000', 'addiu\tt0, t0, 0x24'):
    if token not in entry_patch:
        raise SystemExit(f'kernel-entry marker patch token missing: {token}')
if entry_patch.find('rtl8197f_early_mark 0x4b') > entry_patch.find('\tkernel_entry_setup'):
    raise SystemExit('K marker is not before kernel_entry_setup')
print('PASS: exact RTL8197F watchdog stop, MW5 UART alias and K/S/P early diagnostics')
PY



python3 - <<'PY' || bad 'RTL8197F firmware mtdsplit DTS regression'
from pathlib import Path
import re

for rel in (
    'target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts',
    'target/linux/realtek/dts/rtl8197f_tenda_ac23.dts',
):
    text = Path(rel).read_text()
    m = re.search(r'firmware:\s*partition@30000\s*\{(.*?)\n\s*\};', text, re.S)
    if not m:
        raise SystemExit(f'firmware partition block missing in {rel}')
    block = m.group(1)
    if 'label = "firmware";' not in block:
        raise SystemExit(f'firmware label missing in {rel}')
    if 'compatible =' in block:
        raise SystemExit(f'compatible property still suppresses OpenWrt mtdsplit in {rel}')

config = Path('target/linux/realtek/rtl8197f/config-6.6').read_text()
for token in ('CONFIG_MTD_SPLIT_FIRMWARE=y', 'CONFIG_MTD_SPLIT_RTL8197F_FW=y'):
    if token not in config:
        raise SystemExit(f'missing kernel split config: {token}')

parser = Path('target/linux/generic/files/drivers/mtd/mtdsplit/mtdsplit_rtl8197f.c').read_text()
for token in ('be32_to_cpu(hdr.length)', 'sizeof(hdr) + (u64)image_len',
              'mtd_check_rootfs_magic', 'KERNEL_PART_NAME', 'ROOTFS_PART_NAME'):
    if token not in parser:
        raise SystemExit(f'RTL8197F parser token missing: {token}')

core_patch = Path('target/linux/generic/pending-6.6/400-mtd-mtdsplit-support.patch').read_text()
if '!of_find_property(mtd_get_of_node(part), "compatible", NULL)' not in core_patch:
    raise SystemExit('OpenWrt firmware-compatible suppression rule changed; review DTS fix')
mw5 = Path('target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts').read_text()
if 'realtek,read-shift = <0>;' not in mw5:
    raise SystemExit('MW5 SPI auto-map read-shift is not zero')
if 'realtek,read-shift = <1>;' in mw5:
    raise SystemExit('stale MW5 one-byte read shift remains')
print('PASS: Tenda firmware nodes allow the RTL8197F mtdsplit parser to run')
print('PASS: MW5 SPI auto-map uses exact OEM coordinates (read_shift=0)')
PY


python3 - <<'PY' || bad 'RTL8197F loader/kernel overlap regression'
from pathlib import Path
import json, struct, subprocess, tempfile

mk = Path('target/linux/realtek/image/rtl8197f.mk').read_text()
if 'RTL8197F_LOADADDR := 0x80a00000' not in mk or 'RTL8197F_LZMA_TEXT_START := 0x80d00000' not in mk:
    raise SystemExit('MW5 OEM staging/link split is not 0x80a00000 -> 0x80d00000')
makefile = Path('target/linux/realtek/image/Makefile').read_text()
if 'rtl8197f-loader-overlap-check.py' not in makefile:
    raise SystemExit('loader overlap checker is not in the image pipeline')
loader = Path('target/linux/realtek/image/lzma-loader/src/loader.c').read_text()
if 'cache_size = lzma_outsize;' not in loader:
    raise SystemExit('normal post-decode cache size reload missing')
if 'stream-dependent invalid cache range' in loader:
    raise SystemExit('obsolete v42.24 register-corruption diagnosis remains')

profile = json.loads(Path('target/linux/realtek/rtl8197f/board-profiles-v42.json').read_text())
mw5 = profile['boards']['tenda,nova-mw5']
for key, value in {
    'loader_staging_address': '0x80a00000',
    'loader_link_address': '0x80d00000',
    'kernel_load_address': '0x80100000',
}.items():
    if mw5.get(key) != value:
        raise SystemExit(f'MW5 loader profile mismatch: {key}={mw5.get(key)!r}')

with tempfile.TemporaryDirectory() as td:
    p = Path(td) / 'kernel.lzma'
    # Minimal LZMA-alone header with the physically observed world-build size.
    p.write_bytes(bytes.fromhex('6d00008000') + struct.pack('<Q', 0x900d5b))
    good = subprocess.run([
        'python3', 'scripts/rtl8197f-loader-overlap-check.py',
        '--lzma', str(p), '--kernel-loadaddr', '0x80100000',
        '--loader-start', '0x80d00000', '--safety-margin', '64k',
        '--label', 'mw5-v42.27'], capture_output=True, text=True)
    if good.returncode != 0:
        raise SystemExit(good.stdout + good.stderr)
    badrun = subprocess.run([
        'python3', 'scripts/rtl8197f-loader-overlap-check.py',
        '--lzma', str(p), '--kernel-loadaddr', '0x80100000',
        '--loader-start', '0x80a00000', '--safety-margin', '0',
        '--label', 'mw5-old'], capture_output=True, text=True)
    if badrun.returncode == 0 or 'excess=0xd5b' not in badrun.stderr:
        raise SystemExit('old 0x80a00000 overlap was not rejected exactly')
print('PASS: MW5 stages at 0x80a00000, relocates to 0x80d00000, and rejects old overlap')
PY

python3 - <<'PY' || bad 'RTL8197FS UART RX/TX alias and MW5 P0/RGMII regression'
from pathlib import Path
import json
patch = Path('target/linux/realtek/patches-6.6/322-serial-8250-dw-add-rtl8197fs-tx-alias.patch').read_text()
dts = Path('target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts').read_text()
drv = Path('target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c').read_text()
for token in ('rx_alias_offset', 'tx_alias_offset',
              'realtek,rx-alias-offset', 'realtek,tx-alias-offset',
              'offset == UART_RX', 'offset == UART_TX', 'UART_LCR_DLAB',
              'p->iotype != UPIO_MEM', 'readb(addr)', 'writeb(value, addr)'):
    if token not in patch:
        raise SystemExit(f'missing DW-8250 RX/TX-alias token: {token}')
if patch.find('offset == UART_RX') > patch.find('value = readb(addr)'):
    raise SystemExit('RX alias selection occurs after the data read')
if patch.find('UART_LCR_DLAB', patch.find('offset == UART_RX')) > patch.find('value = readb(addr)'):
    raise SystemExit('RX DLAB guard is not evaluated before data read')
for token in ('reg-io-width = <1>;',
              'realtek,rx-alias-offset = <0x24>;',
              'realtek,tx-alias-offset = <0x24>;',
              'realtek,tx-port-mask = <0x1>;',
              'realtek,tx-dp-ext = <0x0>;',
              'realtek,descriptor-dwords = <6>;',
              'realtek,p0-cpu-tag-pass-through;'):
    if token not in dts:
        raise SystemExit(f'missing MW5 DTS token: {token}')
for token in ('RTL_RTK_P0GMIICR_RX_DELAY_FS',
              'rx_delay = RTL_RTK_P0GMIICR_RX_DELAY_FS',
              'RTL_RTK_P0GMIICR_CPU_TAG_RX',
              'RTL_RTK_P0GMIICR_CPU_TAG_TX',
              'if (!mw5)',
              'rtl8197f_rtk_init_p0_rgmii(priv, "hw-start-mw5")',
              'priv->tx_port_mask = BIT(0)',
              'priv->tx_dp_ext = 0',
              'priv->tx_extspa = 0',
              'priv->desc_dwords = 6',
              'priv->desc_stride = 6 * sizeof(u32)',
              'idx * priv->desc_stride',
              '(priv->tx_ring_size - 1) * priv->desc_stride',
              'rtl8197f_rtk_desc_clear_padding',
              'rtl8197f poststart v43.6',
              'rtl8197f_rtk_p0_external_switch_board',
              'priv->legacy_rd05_pipeline = true',
              'const u8 *lan_mac = priv->ndev->dev_addr',
              'const u8 *cpu_mac = priv->ndev->dev_addr',
              'p0 cpu-pipeline v43.6'):
    if token not in drv:
        raise SystemExit(f'missing MW5 P0/RGMII driver token: {token}')
profile = json.loads(Path('target/linux/realtek/rtl8197f/board-profiles-v42.json').read_text())
if profile.get('version') != '42.27-development':
    raise SystemExit('board profile version is not 42.27-development')
mw5 = profile['boards']['tenda,nova-mw5']
for key, value in {
    'spi_automap_read_shift': 0,
    'uart_rbr_thr_alias_offset': 36,
    'soc_tx_port_mask': '0x1',
    'soc_tx_dp_ext': 0,
    'p0_rgmii_tx_delay': 0,
    'p0_rgmii_rx_delay': 5,
    'p0_cpu_tag_passthrough': True,
    'cpu_dma_descriptor_dwords': 6,
    'cpu_dma_descriptor_stride_bytes': 24,
    'soc_internal_p0_cpu_pipeline': True,
    'soc_internal_lan_vid': 9,
    'soc_internal_host_port': 0,
}.items():
    if mw5.get(key) != value:
        raise SystemExit(f'MW5 board profile mismatch: {key}={mw5.get(key)!r}')
print('PASS: RTL8197FS byte UART has DLAB-safe RX/TX aliases')
if 'realtek,legacy-rd05-sdk-pipeline;' not in dts:
    raise SystemExit('MW5 internal rtl865x P0-to-CPU pipeline DTS flag missing')
print('PASS: MW5 uses SDK physical-P0/RGMII and non-VG six-dword CPU-DMA descriptors')
print('PASS: MW5 enables OEM-derived internal P0-to-CPU VLAN/L2/netif/ACL pipeline with runtime MAC')
PY
python3 - <<'PY' || bad 'RTL8197F OEM switch-preinit watchdog regression'
from pathlib import Path
p = Path('target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rd05_oem_switch_preinit.c')
s = p.read_text()
for token in ('static void rtl8197f_stop_watchdog(void)',
              'v | RTL8197F_WDT_CLEAR',
              'writel(RTL8197F_WDT_STOP_PATTERN',
              'rtl8197f_stop_watchdog();'):
    if token not in s:
        raise SystemExit(f'missing OEM switch-preinit watchdog token: {token}')
if s.count('rtl8197f_stop_watchdog();') < 2:
    raise SystemExit('watchdog is not safely stopped both before and after switch reset')
if s.count('writel(RTL8197F_WDT_STOP_PATTERN') != 1:
    raise SystemExit('unsafe duplicate/direct watchdog stop-pattern write remains')
preinit = s[s.find('static int __init rtl8197f_oem_switch_preinit(void)'):]
first_call = preinit.find('rtl8197f_stop_watchdog();')
switch_call = preinit.find('rtl8197f_mw5_switch_gpio_init(gpio);')
if first_call < 0 or switch_call < 0 or first_call > switch_call:
    raise SystemExit('watchdog stop is not before the MW5 switch-reset routine')
print('PASS: OEM switch preinit uses safe kick-then-stop before and after MW5 reset delays')
PY
