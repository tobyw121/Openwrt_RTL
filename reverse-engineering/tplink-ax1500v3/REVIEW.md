# TP-Link AX1500v3 original GPL review — 2026-11-20

Source: https://static.tp-link.com/upload/gpl-code/2026/202610/20261022/GPL_ax1500v3.tar.gz
SHA256: 6d133598b134b6878ce802a53b3061d9f2efb3636ad0d31ec4cb3bcbdf438f2a
Downloaded to /tmp/GPL_ax1500v3.tar.gz (about2.1GiB), extracted G6 at
/workspace/tplink-g6. This matches the archive already pinned by the package.

## Implemented WLAN correction

G6 platform/mips_97f/rtw_pltfm_preconf.h enables RTW_CUSTOM_PARA_DIR but does
not define REALTEK_CONFIG_PATH. phl/phl_types.h falls back to an empty root;
hal_g6/flash/hal_flash.c initializes the RF directory from HAL_FILE_CONFIG_PATH.
hal_g6/hal_ld_file.c then constructs root + IC + RFE + optional FEM + filename.
Our package installs files under /etc/conf/rtl8832bre/RFE50 and AP_5G.
Patch104 therefore sets the package's Linux97F default root to /etc/conf/.
No OEM NVM write API, shared password, regulation bypass or acceleration is
introduced. Existing explicit platform overrides retain precedence.

The empty `rtw_load_phy_file_path` diagnostic has a second, distinct cause:
include/rtw_nvm.h defines rtw_nvm_fill_full_para_path as an empty macro when
CONFIG_RTW_DRV_HAS_NVM is absent. The actual PHL RF reader constructs its
own filenames, independently of that diagnostic buffer. Merely populating
the diagnostic string would not repair actual file I/O. Check PHL's resolved
paths and successful parameter parsing on hardware rather than that log alone.
Missing file behavior and internal-table fallback are unchanged.

## Error-unwind findings

os_dep/linux/pci_intf.c still frees the primary adapter through
rtw_free_drv_sw without the same stop sequence used in ordinary removal.
The existing IRQ preflight does not resolve all possible IRQ failures.
No blind timer cancellation was added: partially initialized timers and
callbacks which can rearm must be audited before claiming UAF is fixed.
Manual probe remains the package policy. eFuse access is likewise unresolved.

## Switch scope

The active tree already contains original-SDK-based CVLAN busy polling,
unconditional commands and verified VLAN writes. Do not reimport OEM
swconfig, proprietary bridge/NAT shortcuts or an all-port VLAN bootstrap
on top of Linux DSA. No switch timing, DMA or port map was changed here.

## Validation and limits

All five package patches apply to freshly extracted GPL with zero fuzz.
Prepared platform header preprocesses with the expected RF root and
RTW_CUSTOM_PARA_DIR, and installed root/AP_5G files exist. Production DMA,
regulatory/key tests are also run against prepared GPL code. No configured
MIPS cross-toolchain is available in this environment; full module/image
build and actual RF-file loading are not claimed.

Build on PC:
```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make package/kernel/rtl8832br-wr1500/clean
make package/kernel/rtl8832br-wr1500/compile -j1 V=s
make -j"$(nproc)" V=s
```
After installation, UART38400 router commands individually:
```sh
wr1500-wifi6-diag --archive
cat /sys/bus/pci/devices/0000:00:00.0/irq
```
Only if IRQ is neither0 nor255, once with UART recovery available:
```sh
modprobe rtk_wifi6
dmesg | tail -100
iw dev
```
Still capable of hitting the unresolved failed-probe cleanup; do not loop.
