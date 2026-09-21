# RTL8192CD MW5 Linux 6.6 / OpenWrt 24.10 port

This tree keeps the Realtek RTL8197FS/RTL8822B HAL, PHYDM, RF tables,
IQK/calibration algorithms, descriptors, firmware, eFuse/RFE parsers and the
hardware-specific TX/RX routines.  The MW5 variant replaces the Linux-facing
integration with Linux 6.6 APIs and makes cfg80211 the only userspace wireless
control ABI.

## Architecture

- `src/linux6/rtl8192cd_core.c` is the common Linux attachment layer. It feeds
  bus resources and board data into the existing vendor core without taking
  ownership away from the Linux bus driver.
- `src/linux6/rtl8197f_soc.c` is the platform backend for the integrated
  RTL8197FS. MMIO and IRQ come from DT; DMA uses the real platform `struct
  device`.
- `src/linux6/rtl8822b_pci.c` is the PCI backend for 10ec:b822. BAR mapping,
  DMA setup and IRQ-vector allocation are PCI-owned.
- `src/linux6/rtl8192cd_board.c` reads radio index, RFE type, base MAC and the
  optional raw calibration data plus the MW5 factory key/value blob from firmware properties/NVMEM.
- RX delivery is scheduled through NAPI. The descriptor/refill/HAL RX loop is
  intentionally retained and budgeted rather than rewritten.
- cfg80211 event delivery uses a workqueue for the Linux-6 MW5 path. The
  driver-private net80211 vocabulary remains only as an internal adapter for
  old vendor helpers; it is not a WEXT userspace ABI.

## cfg80211 contract on Linux 6.6

The port uses the Linux 6.6 callback signatures, including link-aware key
callbacks, `start_ap`, `stop_ap`, `del_station`,
`update_mgmt_frame_registrations`, `set_ap_chanwidth` and `get_channel`.
Compatibility guards cover the signature changes needed by the host 6.12
smoke build without changing the 6.6 contract.

For `CONFIG_RTL8192CD_LINUX6_BUS`, legacy `ndo_do_ioctl`,
`wireless_handlers`, private write-register ioctl/proc controls and the old
hostapd private WEXT configuration path are disabled. RD05/AC23 keep their
legacy WEXT build variant for now; this port changes the MW5 variant only.

Linux 6.6 treats `net_device::dev_addr` as read-only driver state. The MW5
path therefore updates interface addresses with `eth_hw_addr_set()` rather
than writing the pointer returned by `dev_addr` directly.

## DT / NVMEM board data

Required MW5 defaults are encoded in the bus backends but may be overridden by
DT firmware properties:

- integrated RTL8197FS: radio index 1, RFE 5, factory MAC increment +1;
- external RTL8822B: radio index 0, RFE 6, factory MAC increment +4.

The final OpenWrt MW5 DTS exposes the complete 64 KiB Tenda `factory` MTD
partition as an NVMEM cell to both radios. That partition is **not** a raw
eFuse dump: it is a NUL-separated `KEY=value` store. The Linux-6 board layer
reads the base `HW_NIC0_ADDR` and applies the DT `realtek,mac-address-increment`.
The eFuse shadow update then selects only `HW_WLAN1_*` records for radio 1 and
`HW_WLAN0_*` records for radio 0, maps recognized names through the original
Realtek eFuse command table, and updates the in-memory shadow map. Unknown keys
are ignored and no OTP/eFuse programming operation is performed.

The generic optional `calibration` NVMEM path is retained for other boards. It
accepts only a complete logical/shadow eFuse map whose size exactly equals the
chip-reported `EfuseMapLen`; otherwise the original hardware-eFuse path is used.

The board DTS now contains the real MW5 topology: the existing SoC WLAN resource
node for RTL8197FS and the PCI child `10ec:b822` for RTL8822B both reference the
factory NVMEM cell. No synthetic MMIO, IRQ or flash addresses are introduced.

## OpenWrt package integration

Use the top-level package `Makefile` as the package definition. The MW5 variant depends on OpenWrt `kmod-cfg80211`, builds against the
mac80211/backports 6.12.96 headers on kernel 6.6, and selects
`CONFIG_RTL8192CD_LINUX6_BUS`; it intentionally does not select WEXT. Put the package directory under the OpenWrt tree, add the board DT
NVMEM wiring, then build the image/package in the normal OpenWrt 24.10 kernel
build so it uses the target MIPS toolchain and the target `.config`.

## Validation status

The Linux 6.6.114 source supplied with this task was used as the API reference.
A complete exact-6.6 target image build cannot be performed in the supplied runtime
because the OpenWrt host prerequisites/toolchain are incomplete (notably GNU awk and ncurses development files). The
modified objects have instead been smoke-compiled against the available Linux
6.12.96 headers, including MODPOST and final `rtl8192cd.ko` link, with version
guards retaining the verified Linux 6.6 signatures. The host module is not a
deployable MW5 binary (wrong architecture/vermagic); it is compile validation,
not hardware validation.

One deliberate compatibility boundary remains: the old vendor code precreates
some VAP/VXD netdevs and historically renamed them dynamically. The Linux-6
cfg80211 path does not call the non-exported `dev_change_name()` helper; a
request that would require renaming a precreated interface to a mismatching
type/name is rejected with `-EOPNOTSUPP`. Root AP operation does not depend on
that legacy rename path, but multi-VAP/VXD deployments must be exercised on
the target and can be converted to fully dynamic cfg80211 netdev allocation in
a follow-up if the board image uses them.

Before deployment, validate on MW5 hardware at minimum: both probes/removes,
NVMEM factory MAC/RFE values, eFuse fallback and factory shadow-map overlay, 2.4/5 GHz AP
start/stop, WPA2/WPA3 key install, station add/delete, RX NAPI under load, IRQ
recovery, DFS/CAC on 5 GHz where applicable, suspend/reboot/remove, and repeated
interface up/down cycles.
