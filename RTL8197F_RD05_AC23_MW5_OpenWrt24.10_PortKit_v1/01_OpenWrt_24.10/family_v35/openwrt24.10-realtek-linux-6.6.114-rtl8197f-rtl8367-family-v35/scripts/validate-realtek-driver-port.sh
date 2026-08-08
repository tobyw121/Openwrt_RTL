#!/usr/bin/env bash
set -uo pipefail

ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
TARGET="$ROOT/target/linux/realtek"
VENDOR="$TARGET/vendor/legacy-switch-sdk"
PASS=0
FAIL=0
WARN=0

pass() { printf 'PASS  %s\n' "$*"; PASS=$((PASS + 1)); }
fail() { printf 'FAIL  %s\n' "$*"; FAIL=$((FAIL + 1)); }
warn() { printf 'WARN  %s\n' "$*"; WARN=$((WARN + 1)); }

check_file() {
    if [ -f "$1" ]; then pass "file exists: ${1#$ROOT/}"; else fail "missing file: ${1#$ROOT/}"; fi
}

check_dir() {
    if [ -d "$1" ]; then pass "directory exists: ${1#$ROOT/}"; else fail "missing directory: ${1#$ROOT/}"; fi
}

check_grep() {
    pattern=$1
    file=$2
    description=$3
    if grep -Eq "$pattern" "$file" 2>/dev/null; then pass "$description"; else fail "$description"; fi
}

printf 'Realtek Linux 6.6.114 driver-port validation\n'
printf 'root: %s\n\n' "$ROOT"

check_grep '^KERNEL_PATCHVER:=6\.6$' "$TARGET/Makefile" 'target kernel patch line is 6.6'
check_grep '^LINUX_VERSION-6\.6 = \.114$' "$ROOT/include/kernel-6.6" 'kernel release is exactly 6.6.114'
check_grep '^SUBTARGETS:=rtl8197f rtl838x rtl839x rtl930x rtl931x$' "$TARGET/Makefile" 'all five Realtek subtargets are enabled'
check_grep '^CONFIG_TARGET_realtek=y$' "$ROOT/.config" 'OpenWrt target remains Realtek'
check_grep '^CONFIG_TARGET_realtek_rtl8197f=y$' "$ROOT/.config" 'selected build remains rtl8197f'
check_grep '^CONFIG_TARGET_realtek_rtl8197f_DEVICE_xiaomi_r4_rd05=y$' "$ROOT/.config" 'selected device remains Xiaomi RD05'
check_grep '^CONFIG_LINUX_6_6=y$' "$ROOT/.config" 'OpenWrt configuration selects Linux 6.6'

for subtarget in rtl8197f rtl838x rtl839x rtl930x rtl931x; do
    check_dir "$TARGET/$subtarget"
    check_file "$TARGET/$subtarget/config-6.6"
done

expected_patches=(
    300-mips-add-rtl838x-platform.patch
    301-mips-add-rtl8197f-platform-kconfig.patch
    302-clocksource-add-otto-driver.patch
    303-mips-realtek-rtl8197f-force-little-endian.patch
    304-timer-otto-fix-stall-after-restart.patch
    305-gpio-add-rtl8197f-gpio-driver.patch
    306-spi-add-rtl8197f-sheipa-driver.patch
    307-mips-rtl8197f-force-r4k-timer-selection.patch
    308-mtd-add-rtl8197f-spirom-automap.patch
    309-net-ethernet-add-rtl8197f-ethdiag.patch
    310-add-i2c-rtl9300-support.patch
    311-add-i2c-mux-rtl9300-support.patch
    314-irqchip-irq-realtek-rtl-add-VPE-support.patch
    318-add-rtl83xx-clk-support.patch
    320-harden-fw_init_cmdline.patch
    322-net-ethernet-add-rtl8197f-rtknet-reference-guard.patch
    323-net-ethernet-add-rtl8197f-rtknet-native.patch
    324-net-ethernet-add-rd05-oem-switch-preinit.patch
    325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch
    326-rtl8197f-rtl8367d-mainstream-offload-notes.patch
    327-net-dsa-realtek-rtl8367-family-profiles.patch
    700-net-dsa-increase-dsa-max-ports-for-rtl838x.patch
    702-include-linux-add-phy-hsgmii-mode.patch
    704-include-linux-phy-increase-phy-address-number-for-rtl839x.patch
    706-include-linux-add-phy-ops-for-rtl838x.patch
    708-drivers-net-phy-eee-support-for-rtl838x.patch
    710-net-phy-sfp-re-probe-modules-on-DEV_UP-event.patch
    712-net-phy-add-an-MDIO-SMBus-library.patch
    714-net-phy-sfp-add-support-for-SMBus.patch
    716-net-ethernet-add-support-for-rtl838x-ethernet.patch
    718-net-dsa-add-support-for-rtl838x-switch.patch
    720-add-rtl-phy.patch
    722-net-dsa-add-rtl838x-support-for-tag-trailer.patch
    723-net-mdio-Add-Realtek-Otto-auxiliary-controller.patch
    800-gpio-regmap-Bypass-cache-for-shadowed-outputs.patch
    802-mfd-Add-RTL8231-core-device.patch
    803-pinctrl-Add-RTL8231-pin-control-and-GPIO-support.patch
    804-leds-Add-support-for-RTL8231-LED-scan-matrix.patch
)

actual_patch_count=$(find "$TARGET/patches-6.6" -maxdepth 1 -type f -name '*.patch' | wc -l | tr -d ' ')
if [ "$actual_patch_count" = "${#expected_patches[@]}" ]; then
    pass "active patch count is ${#expected_patches[@]}"
else
    fail "active patch count is $actual_patch_count, expected ${#expected_patches[@]}"
fi

for patch_name in "${expected_patches[@]}"; do
    patch_file="$TARGET/patches-6.6/$patch_name"
    if [ ! -f "$patch_file" ]; then
        fail "missing active patch: $patch_name"
        continue
    fi
    if grep -Eq '^diff --git |^--- (a/|/dev/null)' "$patch_file" && grep -Eq '^\+\+\+ (b/|/dev/null)' "$patch_file"; then
        pass "patch has unified-diff structure: $patch_name"
    else
        fail "patch lacks expected unified-diff headers: $patch_name"
    fi
done

# Header presence alone is not enough: a wrong hunk line count is accepted by
# simple grep checks but aborts OpenWrt's kernel preparation as a malformed patch.
strict_parse_errors=()
if command -v git >/dev/null 2>&1; then
    for patch_name in "${expected_patches[@]}"; do
        patch_file="$TARGET/patches-6.6/$patch_name"
        [ -f "$patch_file" ] || continue
        if ! git apply --numstat "$patch_file" >/dev/null 2>&1; then
            strict_parse_errors+=("$patch_name")
        fi
    done
    if [ "${#strict_parse_errors[@]}" -eq 0 ]; then
        pass 'all active patches pass strict git unified-diff parsing'
    else
        fail "strict git patch parsing failed: ${strict_parse_errors[*]}"
    fi
else
    fail 'git is unavailable; strict unified-diff parsing was not executed'
fi

gnu_parse_errors=()
if command -v patch >/dev/null 2>&1; then
    patch_parse_tmp=$(mktemp -d)
    for patch_name in "${expected_patches[@]}"; do
        patch_file="$TARGET/patches-6.6/$patch_name"
        [ -f "$patch_file" ] || continue
        patch_parse_output=$(patch --dry-run --batch -p1 -d "$patch_parse_tmp" < "$patch_file" 2>&1 || true)
        if grep -q 'malformed patch' <<<"$patch_parse_output"; then
            gnu_parse_errors+=("$patch_name")
        fi
    done
    rm -rf "$patch_parse_tmp"
    if [ "${#gnu_parse_errors[@]}" -eq 0 ]; then
        pass 'all active patches pass GNU patch syntax parsing'
    else
        fail "GNU patch syntax parsing failed: ${gnu_parse_errors[*]}"
    fi
else
    fail 'GNU patch is unavailable; patch syntax parsing was not executed'
fi

if find "$TARGET" -maxdepth 2 -type d -iname '*disabled*' | grep -q .; then
    fail 'disabled patch directories remain in target tree'
else
    pass 'no disabled patch directory remains'
fi

expected_overlays=(
    arch/mips/rtl838x/prom.c
    drivers/clk/realtek/clk-rtl83xx.c
    drivers/clocksource/timer-rtl-otto.c
    drivers/gpio/gpio-rtl8197f.c
    drivers/i2c/busses/i2c-rtl9300.c
    drivers/i2c/muxes/i2c-mux-rtl9300.c
    drivers/mtd/maps/rtl8197f-spirom.c
    drivers/net/dsa/rtl83xx/common.c
    drivers/net/dsa/rtl83xx/rtl838x.c
    drivers/net/dsa/rtl83xx/rtl839x.c
    drivers/net/dsa/rtl83xx/rtl930x.c
    drivers/net/dsa/rtl83xx/rtl931x.c
    drivers/net/ethernet/rtl8197f_rtknet.c
    drivers/net/ethernet/rtl8197f_rd05_oem_switch_preinit.c
    drivers/net/ethernet/rtl838x_eth.c
    drivers/net/phy/rtl83xx-phy.c
    drivers/spi/spi-rtl8197f-sheipa.c
)
for overlay in "${expected_overlays[@]}"; do
    check_file "$TARGET/files-6.6/$overlay"
done

for subtarget in rtl838x rtl839x; do
    cfg="$TARGET/$subtarget/config-6.6"
    for symbol in \
        CONFIG_NET_RTL838X \
        CONFIG_NET_DSA_RTL83XX \
        CONFIG_REALTEK_SOC_PHY \
        CONFIG_MDIO_REALTEK_OTTO_AUX \
        CONFIG_MFD_RTL8231 \
        CONFIG_PINCTRL_RTL8231 \
        CONFIG_LEDS_RTL8231 \
        CONFIG_GPIO_REALTEK_OTTO \
        CONFIG_REALTEK_OTTO_TIMER \
        CONFIG_REALTEK_OTTO_WDT; do
        check_grep "^${symbol}=y$" "$cfg" "$subtarget enables $symbol"
    done
done

for subtarget in rtl930x rtl931x; do
    cfg="$TARGET/$subtarget/config-6.6"
    for symbol in \
        CONFIG_NET_RTL838X \
        CONFIG_NET_DSA_RTL83XX \
        CONFIG_REALTEK_SOC_PHY \
        CONFIG_I2C_RTL9300 \
        CONFIG_I2C_MUX_RTL9300 \
        CONFIG_GPIO_REALTEK_OTTO \
        CONFIG_REALTEK_OTTO_WDT; do
        check_grep "^${symbol}=y$" "$cfg" "$subtarget enables $symbol"
    done
done

for symbol in \
    CONFIG_RTL8197F \
    CONFIG_GPIO_RTL8197F \
    CONFIG_MTD_RTL8197F_SPIROM \
    CONFIG_RTL8197F_RTKNET \
    CONFIG_REALTEK_OTTO_TIMER \
    CONFIG_REALTEK_OTTO_WDT \
    CONFIG_SERIAL_8250; do
    check_grep "^${symbol}=y$" "$TARGET/rtl8197f/config-6.6" "rtl8197f enables $symbol"
done

check_grep 'realtek,p0-cpu-tag-pass-through;' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 enables RTL8197F P0 CPU-tag RX pass-through'
check_grep 'realtek,writable-offset = <0x00e60000>;' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 writable flash window starts at rootfs_data'
check_grep 'realtek,writable-size = <0x001a0000>;' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 writable flash window is limited to rootfs_data'
check_grep 'RTL_RTK_P0GMIICR_CPU_TAG_RX' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F driver programs CPU-tag RX recognition'
check_grep 'RTL_RTK_MACCR1_RMD_TAG_MASK' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F driver preserves the DSA CPU tag'
check_grep 'RTL8197F_SPINOR_PP' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI auto-map MTD implements bounded page programming'
check_grep 'rtl8197f_spirom_in_writable_window' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI auto-map MTD enforces its writable range'
check_grep 'rtl8197f_rtk_rd05_poststart_rearm' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F re-arms descriptor stride and ring pointers after CPU DMA start'
check_grep 'RTL8197F_SPIC_FLUSH_FIFO' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI auto-map MTD flushes stale SHEIPA FIFO state'
check_grep 'calibrated SPI-NOR WEL:' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI auto-map MTD verifies WREN through RDSR'
check_grep 'preferred_modes' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI-NOR calibration tests command FIFO widths'
check_grep 'RTL8197F_CMD_DR32_HIGH' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI-NOR calibration includes GPL SDK high-byte command mode'
check_grep 'rd05 poststart v34:' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F emits v34 post-start ring marker'
check_grep 'RD05_NETDIAG_VERSION=34' "$TARGET/base-files/usr/bin/rd05-netdiag" 'RD05 diagnostic script is v34'
check_grep 'rd05_ensure_network' "$TARGET/base-files/usr/bin/rd05-netdiag" 'RD05 diagnostics start LAN before evaluating markers'
check_file "$TARGET/base-files/usr/bin/rd05-v34-check"
check_file "$ROOT/RD05_RX_DSA_OVERLAY_V34.md"

check_file "$ROOT/RTL8197F_RTL8367_FAMILY_V35_REPORT.md"
check_file "$TARGET/dts/rtl8197f-rtl8367c-rb-vb-smi-template.dtsi"
check_file "$TARGET/dts/rtl8197f-rtl8367d-rb-vc-smi-template.dtsi"
check_file "$TARGET/dts/rtl8197f-rtl8367b-legacy-swconfig-template.dtsi"
check_file "$TARGET/base-files/usr/bin/rtl8367-family-check"
check_file "$TARGET/base-files/etc/rtl8367-family-v35-notes"

check_grep '0x0276' "$TARGET/patches-6.6/327-net-dsa-realtek-rtl8367-family-profiles.patch" 'v35 DSA patch identifies C-map ID 0x0276'
check_grep '0x0597' "$TARGET/patches-6.6/327-net-dsa-realtek-rtl8367-family-profiles.patch" 'v35 DSA patch identifies C-map ID 0x0597'
check_grep '0x6367' "$TARGET/patches-6.6/327-net-dsa-realtek-rtl8367-family-profiles.patch" 'v35 DSA patch identifies C-map ID 0x6367'
check_grep '0x6642' "$TARGET/patches-6.6/327-net-dsa-realtek-rtl8367-family-profiles.patch" 'v35 DSA patch identifies D-map ID 0x6642'
check_grep 'RTL8367D_EXT1_SSC_SEED_DISABLE.*0x8000' "$TARGET/patches-6.6/327-net-dsa-realtek-rtl8367-family-profiles.patch" 'D-map SSC uses exact disable seed'
check_grep 'RTL8367D_EXT1_SSC_CTRL_DISABLE.*0x2411' "$TARGET/patches-6.6/327-net-dsa-realtek-rtl8367-family-profiles.patch" 'D-map SSC uses exact disable control'
check_grep 'realtek,rtl8367-legacy-master' "$TARGET/files-6.6/Documentation/devicetree/bindings/net/realtek,rtl8197f-rtknet.yaml" 'NIC binding documents legacy RTL8367 master mode'
check_grep 'realtek,p0-rgmii-rx-delay' "$TARGET/files-6.6/Documentation/devicetree/bindings/net/realtek,rtl8197f-rtknet.yaml" 'NIC binding documents P0 RGMII RX timing'
check_grep 'rtl8367_external_master' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'NIC generalizes DSA and legacy external-switch startup'
check_grep 'RTL8367 .* master v35' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'NIC emits v35 external-master marker'
check_grep 'realtek,p0-rgmii-rx-delay = <6>;' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 retains SoC-side RX tap 6'
check_grep 'realtek,p0-rgmii-rx-delay = <5>;' "$TARGET/dts/rtl8197f-rtl8367c-rb-vb-smi-template.dtsi" 'generic C template uses SoC-side RX tap 5'
check_grep 'realtek,p0-rgmii-rx-delay = <5>;' "$TARGET/dts/rtl8197f-rtl8367d-rb-vc-smi-template.dtsi" 'generic D template uses SoC-side RX tap 5'
check_grep '^CONFIG_PACKAGE_kmod-swconfig=m$' "$ROOT/.config" 'legacy swconfig core is built as optional module'
check_grep '^CONFIG_PACKAGE_kmod-switch-rtl8367=m$' "$ROOT/.config" 'legacy rtl8367 module is optional'
check_grep '^CONFIG_PACKAGE_kmod-switch-rtl8367b=m$' "$ROOT/.config" 'legacy rtl8367b module is optional'

RTL8367_DOC="$TARGET/docs/rtl8197f-rtl8367"
check_file "$RTL8367_DOC/RTL8367_VARIANT_MATRIX.tsv"
check_file "$RTL8367_DOC/RTL8367_REGISTER_MAP_SUMMARY.md"
check_file "$RTL8367_DOC/RTL8197_SOC_AND_NIC_SUMMARY.md"
check_file "$RTL8367_DOC/RTL8367_SUPPORT_STATUS.md"
check_file "$RTL8367_DOC/RTL8197F_NIC_REGISTER_MAP.tsv"
check_file "$RTL8367_DOC/RTL8367_DRIVER_SOURCE_INVENTORY.tsv"
check_file "$RTL8367_DOC/register-maps/rtl8367b-register-map.tsv"
check_file "$RTL8367_DOC/register-maps/rtl8367c-register-map.tsv"
check_file "$RTL8367_DOC/register-maps/rtl8367d-register-map.tsv"

b_defs=$(( $(wc -l < "$RTL8367_DOC/register-maps/rtl8367b-register-map.tsv") - 1 ))
c_defs=$(( $(wc -l < "$RTL8367_DOC/register-maps/rtl8367c-register-map.tsv") - 1 ))
d_defs=$(( $(wc -l < "$RTL8367_DOC/register-maps/rtl8367d-register-map.tsv") - 1 ))
if [ "$b_defs" = 13622 ]; then pass 'B-map index contains 13622 definitions'; else fail "B-map index contains $b_defs definitions"; fi
if [ "$c_defs" = 18679 ]; then pass 'C-map index contains 18679 definitions'; else fail "C-map index contains $c_defs definitions"; fi
if [ "$d_defs" = 10407 ]; then pass 'D-map index contains 10407 definitions'; else fail "D-map index contains $d_defs definitions"; fi

led_dts_count=$(grep -RIl 'realtek,rtl8231-leds' "$TARGET/dts" 2>/dev/null | wc -l | tr -d ' ')
if [ "$led_dts_count" -gt 0 ]; then
    pass "RTL8231 LED nodes are present in $led_dts_count DTS/DTSI files"
else
    fail 'no RTL8231 LED devicetree node found'
fi

check_dir "$VENDOR/source"
check_file "$VENDOR/PORT_MANIFEST.tsv"
check_file "$VENDOR/PORT_SUMMARY.tsv"
check_file "$VENDOR/SOURCE_SHA256SUMS"
check_file "$VENDOR/INPUT_ARCHIVES.sha256"
check_file "$VENDOR/README.md"

sdk_files=$(find "$VENDOR/source" -type f | wc -l | tr -d ' ')
sdk_links=$(find "$VENDOR/source" -type l | wc -l | tr -d ' ')
manifest_rows=$(awk 'END { print NR - 1 }' "$VENDOR/PORT_MANIFEST.tsv")
if [ "$sdk_files" = 962 ]; then pass 'embedded SDK has all 962 regular files'; else fail "embedded SDK has $sdk_files regular files, expected 962"; fi
if [ "$sdk_links" = 14 ]; then pass 'embedded SDK has all 14 symbolic links'; else fail "embedded SDK has $sdk_links symbolic links, expected 14"; fi
if [ "$manifest_rows" = $((sdk_files + sdk_links)) ]; then pass "manifest covers all $manifest_rows SDK entries"; else fail "manifest rows $manifest_rows do not match inventory $((sdk_files + sdk_links))"; fi

if (cd "$VENDOR/source" && sha256sum -c "$VENDOR/SOURCE_SHA256SUMS" >/dev/null); then
    pass 'all embedded regular SDK files match SOURCE_SHA256SUMS'
else
    fail 'embedded SDK source checksum verification failed'
fi

manifest_tmp=$(mktemp)
summary_tmp=$(mktemp)
manifest_log=$(mktemp)
if "$ROOT/scripts/realtek-sdk-port-manifest.py" "$VENDOR/source" "$manifest_tmp" "$summary_tmp" >"$manifest_log" 2>&1; then
    if cmp -s "$manifest_tmp" "$VENDOR/PORT_MANIFEST.tsv" && cmp -s "$summary_tmp" "$VENDOR/PORT_SUMMARY.tsv"; then
        pass 'port manifest is reproducible from current SDK source'
    else
        fail 'port manifest differs from regenerated inventory'
    fi
else
    fail 'port manifest generator failed'
fi
rm -f "$manifest_tmp" "$summary_tmp" "$manifest_log"

check_grep $'\tlegacy-chip-gap-retained\t' "$VENDOR/PORT_MANIFEST.tsv" 'manifest explicitly records unsupported chip-specific gaps'
check_grep 'RTL8328' "$TARGET/DRIVER_PORT_STATUS.md" 'status document records RTL8328 limitation'
check_grep 'RTL8389' "$TARGET/DRIVER_PORT_STATUS.md" 'status document records RTL8389 limitation'
check_grep 'RTL8208/RTL8212' "$TARGET/DRIVER_PORT_STATUS.md" 'status document records RTL8208/RTL8212 limitation'

if grep -RInE '^(<<<<<<< .+|=======|>>>>>>> .+)$' \
    "$TARGET/files-6.6" "$TARGET/patches-6.6" "$TARGET"/*.md "$ROOT/scripts/realtek-sdk-port-manifest.py" "$ROOT/scripts/validate-realtek-driver-port.sh" >/dev/null 2>&1; then
    fail 'merge-conflict markers found in active port files'
else
    pass 'no merge-conflict markers in active port files'
fi

empty_sources=$(find "$TARGET/files-6.6" -type f \( -name '*.c' -o -name '*.h' \) -size 0 | wc -l | tr -d ' ')
if [ "$empty_sources" = 0 ]; then pass 'no zero-byte C/header files in active kernel overlay'; else fail "$empty_sources zero-byte C/header files in active kernel overlay"; fi

# These are deliberately warnings, not hidden successes.
warn 'RTL8328 native DSA/Ethernet datapath is not verified; legacy source is retained only'
warn 'RTL8389 native DSA/Ethernet datapath is not verified; legacy source is retained only'
warn 'RTL8208/RTL8212 SDK-specific PHY initialization is not verified'
warn 'old RTL8367 B-map silicon remains on the optional swconfig path; exact extif/init revision must be verified'
warn 'family fallback profiles for non-exact C/D revisions require board hardware validation'
warn 'standard DSA bridge/VLAN/FDB/MDB/LAG/TC offload callbacks are not complete'
warn 'full OpenWrt compile and on-device boot/DMA/switch tests are still required'

printf '\nSummary: PASS=%d WARN=%d FAIL=%d\n' "$PASS" "$WARN" "$FAIL"
if [ "$FAIL" -ne 0 ]; then
    exit 1
fi
