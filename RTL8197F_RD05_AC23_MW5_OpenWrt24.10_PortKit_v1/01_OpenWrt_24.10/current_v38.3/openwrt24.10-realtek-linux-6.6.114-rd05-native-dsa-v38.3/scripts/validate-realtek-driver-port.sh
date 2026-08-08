#!/usr/bin/env bash
set -uo pipefail

ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)}"
TARGET="$ROOT/target/linux/realtek"
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

check_not_grep() {
    pattern=$1
    file=$2
    description=$3
    if grep -Eq "$pattern" "$file" 2>/dev/null; then fail "$description"; else pass "$description"; fi
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
    312-pci-controller-add-rtl8197f-host.patch
    314-irqchip-irq-realtek-rtl-add-VPE-support.patch
    315-irqchip-realtek-rtl8197f-fix-parent-routing.patch
    317-usb-host-add-rtl8197f-glue.patch
    318-add-rtl83xx-clk-support.patch
    320-harden-fw_init_cmdline.patch
    323-net-ethernet-add-rtl8197f-rtknet-native.patch
    324-net-ethernet-add-rd05-oem-switch-preinit.patch
    325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch
    326-rtl8197f-rtl8367d-mainstream-offload-notes.patch
    327-net-dsa-realtek-rd05-add-bootstage-markers.patch
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

check_grep '^CONFIG_NVMEM_U_BOOT_ENV=y$' "$TARGET/rtl8197f/config-6.6" 'RD05 U-Boot environment NVMEM parser is enabled'
check_grep 'compatible = "realtek,rtl8367d", "realtek,rtl8365mb";' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 declares RTL8367D explicitly with upstream fallback'
check_grep 'nvmem-cell-names = "mac-address";' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 Ethernet MAC is sourced through NVMEM'
check_grep '^CONFIG_PCIE_RTL8197F=y$' "$TARGET/rtl8197f/config-6.6" 'native RTL8197F PCIe host is enabled'
check_grep 'device_type = "pci";' "$TARGET/dts/rtl8197f.dtsi" 'RTL8197F DTSI describes a generic PCI host bridge'
check_grep 'realtek,refclk-40mhz;' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 selects the OEM 40 MHz PCIe reference clock'
check_grep 'realtek,rtl8197f-vg;' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 selects RTL8197FH/VG PCIe PHY tuning'
check_grep 'native host ready: endpoint=%04x:%04x' "$TARGET/files-6.6/drivers/pci/controller/pcie-rtl8197f.c" 'PCIe driver reports the enumerated endpoint identity'
check_grep 'RTL8197F_PCIE_COMMAND_ENABLE' "$TARGET/files-6.6/drivers/pci/controller/pcie-rtl8197f.c" 'PCIe driver ports the BSP command-register enable sequence'
check_grep 'RTL8197F_PCIE_EXT_IPCFG' "$TARGET/files-6.6/drivers/pci/controller/pcie-rtl8197f.c" 'PCIe driver implements the SDK endpoint config-window selector'
check_grep 'RTL_RTK_P0GMIICR_CPU_TAG_RX \|' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F enables both SDK CPU-tag gates on P0'
check_grep 'rx_cdp_owned_advanced' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F RX path implements SDK current-descriptor-pointer logic'
check_grep 'rd05_p0_rx_delay' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F exposes bounded P0 RGMII timing controls'
check_grep 'RTL8367D_BYPASS_LINE_RATE_REG' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D DSA port clears the SDK EXT1 bypass-line-rate bit'
check_grep 'RTL8367D_EXT_TXC_DLY_REG.*0x13f9' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D family-D 0x13f9 fine TX-clock register is defined'
check_grep 'rd05_ext1_txc_tap' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D exposes the family-D EXT1 fine TX-clock tap'
check_grep 'RTL8367D_SYNC_FIFO1_REG' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D reports synchronizer FIFO TX/RX error state'
check_grep 'RTL8367D_PORT7_CRC_SKIP' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D reports CPU7 CRC-skip state without masking errors'
check_grep 'cpu7-ext1-sdk-baseline-calibratable-v38' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D emits v38 fine-calibratable CPU-link marker'
check_grep 'RTL8367D RD05 RGMII v38 reapply:' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D runtime calibration logs register readback'
check_grep 'output_offset = 1' "$TARGET/patches-6.6/315-irqchip-realtek-rtl8197f-fix-parent-routing.patch" 'RTL8197F interrupt routing uses actual MIPS parent values'
if [ ! -e "$TARGET/patches-6.6/316-irqchip-realtek-rtl8197f-add-second-interrupt-bank.patch" ]; then pass 'unverified RTL8197F second IRQ bank patch is not active on RD05'; else fail 'unsafe second IRQ bank patch remains active'; fi
check_grep 'reg = <0x00003000 0x18>' "$TARGET/dts/rtl8197f.dtsi" 'RD05 uses the boot-proven bank-0 interrupt-controller window'
check_grep 'config USB_RTL8197F_HOST' "$TARGET/patches-6.6/317-usb-host-add-rtl8197f-glue.patch" 'RTL8197F USB2 host glue is wired into Kconfig'
check_grep 'RTL8197F USB2 host initialized:' "$TARGET/files-6.6/drivers/usb/host/rtl8197f-usb-host.c" 'RTL8197F USB2 glue reports BSP clock/PHY initialization'
check_grep 'uart0: serial@147000' "$TARGET/dts/rtl8197f.dtsi" 'RTL8197F UART0 remains at the proven 0x18147000 register block'
check_grep '/delete-property/ interrupts;' "$TARGET/dts/rtl8197f_xiaomi_r4-rd05.dts" 'RD05 board keeps UART0 recovery console in polling mode'
check_grep 'uart1: serial@147400' "$TARGET/dts/rtl8197f.dtsi" 'UART1 register block remains described but disabled'
check_grep 'uart2: serial@147800' "$TARGET/dts/rtl8197f.dtsi" 'UART2 register block remains described but disabled'
check_grep '^CONFIG_SERIAL_8250_NR_UARTS=3$' "$TARGET/rtl8197f/config-6.6" 'RTL8197F reserves three 8250 UART slots'
check_grep '^CONFIG_SERIAL_8250_RUNTIME_UARTS=3$' "$TARGET/rtl8197f/config-6.6" 'RTL8197F permits three runtime UARTs'
check_grep 'RTL_RTK_SWCORE_EXTPCR0' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F programs the VG EXTPCR0 TX-IPG field'
check_grep 'defer P0/RGMII programming to ndo_open' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F defers P0 programming to the boot-proven ndo_open sequence'
check_grep 'chip_id = 0x0276' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367 legacy family ID 0x0276 is covered'
check_grep 'chip_id = 0x0597' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367 legacy family ID 0x0597 is covered'
check_grep 'chip_id = 0x6367' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367C/R/RB/S family ID 0x6367 is covered'
check_grep 'chip_id = 0x6642' "$TARGET/patches-6.6/325-net-dsa-realtek-rtl8365mb-add-native-rtl8367d.patch" 'RTL8367D/RB-VC family ID 0x6642 is covered'
check_grep 'RTL8367 detect enter' "$TARGET/patches-6.6/327-net-dsa-realtek-rd05-add-bootstage-markers.patch" 'RTL8367D detection emits a pre-SMI bootstage marker'
if [ -d "$TARGET/files-6.6/drivers/net/ethernet/realtek/rtknet-vendor-4.4" ]; then
    fail 'inactive Linux 4.4 vendor rtknet source remains in native tree'
else
    pass 'inactive Linux 4.4 vendor rtknet source removed from native tree'
fi

expected_overlays=(
    arch/mips/rtl838x/prom.c
    drivers/clk/realtek/clk-rtl83xx.c
    drivers/clocksource/timer-rtl-otto.c
    drivers/gpio/gpio-rtl8197f.c
    drivers/i2c/busses/i2c-rtl9300.c
    drivers/i2c/muxes/i2c-mux-rtl9300.c
    drivers/mtd/maps/rtl8197f-spirom.c
    drivers/pci/controller/pcie-rtl8197f.c
    Documentation/devicetree/bindings/pci/realtek,rtl8197f-pcie.yaml
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
    drivers/usb/host/rtl8197f-usb-host.c
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
    CONFIG_PCI \
    CONFIG_PCIE_RTL8197F \
    CONFIG_USB_RTL8197F_HOST \
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
check_grep 'rd05 poststart v38\.2:' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F emits v38.2 post-start ring marker'
check_grep 'rtl8197f_rtk_rd05_wait_swcore_ready' "$TARGET/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c" 'RTL8197F waits for SWCORE reset defaults before table seeding'
check_grep 'SPI-NOR WEL calibration deferred until first bounded write/erase' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI-NOR avoids early SHEIPA user-mode commands and calibrates lazily'
check_not_grep 'cal_ret = rtl8197f_spirom_calibrate_write_enable' "$TARGET/files-6.6/drivers/mtd/maps/rtl8197f-spirom.c" 'SPI-NOR probe does not execute WREN/WRDI calibration'
check_grep 'RD05_NETDIAG_VERSION=38' "$TARGET/base-files/usr/bin/rd05-netdiag" 'RD05 diagnostic script is v38'
check_grep 'rd05_ensure_network' "$TARGET/base-files/usr/bin/rd05-netdiag" 'RD05 diagnostics start LAN before evaluating markers'
check_grep 'rx_cdp_' "$TARGET/base-files/usr/bin/rd05-netdiag" 'RD05 diagnostics expose SDK current-descriptor-pointer counters'
check_file "$TARGET/base-files/usr/bin/rd05-v38-check"
check_file "$TARGET/base-files/etc/rd05-v38-notes"
check_file "$ROOT/RD05_SDK_NATIVE_PORT_V35.md"
check_file "$ROOT/RD05_WLAN_NATIVE_STATUS_V35.md"
check_file "$ROOT/RD05_SDK_SOURCE_MAP.tsv"
check_file "$ROOT/RD05_SDK_COMPONENT_INVENTORY.tsv"
check_file "$ROOT/tools/rd05-pc-netdiag.py"
check_file "$ROOT/RD05_NATIVE_DSA_V38.md"
check_file "$ROOT/RD05_NATIVE_DSA_V38_3.md"
check_file "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate"
check_file "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii.conf"
check_file "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii.init"
check_grep 'RD05MAGIC-V31' "$ROOT/tools/rd05-pc-netdiag.py" 'PC companion uses the exact rd05-raw protocol token'
check_grep 'PACKET_OUTGOING' "$ROOT/tools/rd05-pc-netdiag.py" 'PC companion separates locally transmitted AF_PACKET frames'
check_grep '^PKG_RELEASE:=11$' "$ROOT/package/network/config/rtl8367d-compat/Makefile" 'RD05 compatibility package release contains v38.3 calibration safety fixes'
check_grep 'SW_TXC=' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii.conf" 'persistent RGMII profile stores the RTL8367D fine TX-clock tap'
check_grep '^PROFILE_VERSION=38$' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii.conf" 'persistent RGMII profile is versioned for v38'
check_grep '^SW_RX=5$' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii.conf" 'v38 default uses the RTL8367D SDK RX tap 5'
check_grep '^SSC=1$' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii.conf" 'v38 default enables the RTL8367D SDK SSC sequence'
check_grep 'apply SOC_TX SOC_RX SW_TX SW_TXC SW_RX SSC' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate" 'v38 calibration accepts all six independent timing parameters'
check_grep 'for sw_txc in 0 1 2 3 4 5 6 7' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate" 'RX calibration sweeps all eight family-D fine TX-clock taps'
check_grep '128 rows' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate" 'RX calibration documents the complete 128-profile matrix'
check_grep 'settle_ms' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate" 'RGMII calibration uses portable integer-millisecond settling'
check_grep 'rollback_exit' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate" 'RGMII calibration restores the pre-sweep profile on errors and signals'
check_grep 'verify_values' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate" 'RGMII calibration verifies sysfs readback after apply'
check_not_grep 'sleep 0\.2' "$ROOT/package/network/config/rtl8367d-compat/files/rd05-rgmii-calibrate" 'RGMII calibration contains no unsupported fractional BusyBox sleep'
check_grep 'runtime RGMII profile is all zero' "$TARGET/base-files/usr/bin/rd05-v38-check" 'RD05 checker detects the failed-v38.2 all-zero profile'

led_dts_count=$(grep -RIl 'realtek,rtl8231-leds' "$TARGET/dts" 2>/dev/null | wc -l | tr -d ' ')
if [ "$led_dts_count" -gt 0 ]; then
    pass "RTL8231 LED nodes are present in $led_dts_count DTS/DTSI files"
else
    fail 'no RTL8231 LED devicetree node found'
fi

if [ -d "$TARGET/vendor/legacy-switch-sdk" ]; then
    fail 'embedded legacy switch SDK remains in the native OpenWrt target'
else
    pass 'embedded legacy switch SDK removed from native OpenWrt target'
fi
if [ -d "$TARGET/rtl8197f/vendor-target" ]; then
    fail 'stale vendor-target configuration dump remains in native OpenWrt target'
else
    pass 'stale vendor-target configuration dump removed from native OpenWrt target'
fi
if [ -f "$TARGET/patches-6.6/322-net-ethernet-add-rtl8197f-rtknet-reference-guard.patch" ]; then
    fail 'inactive rtknet reference guard patch remains active'
else
    pass 'inactive rtknet reference guard patch removed'
fi

check_grep $'rtl8197f-cpudma\t' "$ROOT/RD05_SDK_SOURCE_MAP.tsv" 'source map records RTL8197F CPU-DMA references'
check_grep $'rtl8367d-dsa\t' "$ROOT/RD05_SDK_SOURCE_MAP.tsv" 'source map records RTL8367D DSA references'
check_grep $'spi-nor\t' "$ROOT/RD05_SDK_SOURCE_MAP.tsv" 'source map records SHEIPA/SPI-NOR references'
check_grep $'pcie-host\t' "$ROOT/RD05_SDK_SOURCE_MAP.tsv" 'source map records RTL8197F PCIe references'
check_grep $'rtl8812fe-wlan\t' "$ROOT/RD05_SDK_SOURCE_MAP.tsv" 'source map records RTL8812FE vendor sources'
check_grep '10ec:f812' "$ROOT/RD05_SDK_NATIVE_PORT_V35.md" 'port report records the OEM-proven PCI endpoint identity'
check_grep 'RTL8812FE' "$ROOT/RD05_WLAN_NATIVE_STATUS_V35.md" 'WLAN status report records the proven external radio identity'
check_grep 'Linux DSA' "$ROOT/RD05_SDK_NATIVE_PORT_V35.md" 'port report documents native Linux DSA conversion'

if grep -RInE '^(<<<<<<< .+|=======|>>>>>>> .+)$' \
    "$TARGET/files-6.6" "$TARGET/patches-6.6" "$TARGET"/*.md \
    "$ROOT/RD05_SDK_NATIVE_PORT_V35.md" "$ROOT/RD05_WLAN_NATIVE_STATUS_V35.md" "$ROOT/scripts/validate-realtek-driver-port.sh" >/dev/null 2>&1; then
    fail 'merge-conflict markers found in active port files'
else
    pass 'no merge-conflict markers in active port files'
fi

empty_sources=$(find "$TARGET/files-6.6" -type f \( -name '*.c' -o -name '*.h' \) -size 0 | wc -l | tr -d ' ')
if [ "$empty_sources" = 0 ]; then pass 'no zero-byte C/header files in active kernel overlay'; else fail "$empty_sources zero-byte C/header files in active kernel overlay"; fi

check_grep 'rd05-v38-check' "$ROOT/target/linux/realtek/base-files/usr/bin/rd05-v34-check" 'v34 check compatibility wrapper targets v38'
check_grep 'rd05-v38-check' "$ROOT/target/linux/realtek/base-files/usr/bin/rd05-v35-check" 'v35 check compatibility wrapper targets v38'
check_grep 'rd05-v38-check' "$ROOT/target/linux/realtek/base-files/usr/bin/rd05-v36-check" 'v36 check compatibility wrapper targets v38'
check_grep 'rd05-v38-check' "$ROOT/target/linux/realtek/base-files/usr/bin/rd05-v37-check" 'v37 check compatibility wrapper targets v38'

# These are deliberately warnings, not hidden successes.
warn 'OEM identity is proven as RTL8812FE (PCI 10ec:f812), but the supplied rtl8192cd/fullmac source is not a native Linux-6.6/mac80211 driver and is intentionally not enabled'
warn 'CPU7 RGMII timing and switch-to-SoC RX/DSA delivery require a new native-v38 on-device traffic test'
warn 'the native RTL8197F PCIe host is compile-checked against available headers but still requires RD05 enumeration testing'
warn 'full OpenWrt world compilation was not completed unless a later validation step reports it explicitly'
warn 'hardware boot, flash endurance, VLAN offload and multi-port forwarding tests remain required'

printf '\nSummary: PASS=%d WARN=%d FAIL=%d\n' "$PASS" "$WARN" "$FAIL"
if [ "$FAIL" -ne 0 ]; then
    exit 1
fi
