# Tenda Nova MW5 rtl8192cd Linux-6 compatibility port v40

This tree is an experimental Linux-6 compatibility port of Realtek's GPL
`rtl8192cd` FullMAC/WEXT driver for the Tenda Nova MW5.

It is not a new native mac80211 driver.  The port keeps the original Realtek
HAL/PHYDM/HALRF architecture and adapts the kernel-facing parts required to
compile on Linux 6.x.  First hardware loading must be done manually with a
serial console and a complete SPI recovery image available.

## Hardware profiles

Default (`CONFIG_MW5_2G_ONLY`):

- integrated RTL8197F/RTL8197FS 2.4 GHz radio;
- fixed legacy MMIO fallback `0xb8640000` and IRQ 6, reconstructed from the OEM
  kernel; the long-term target is a DT/platform resource path;
- exact 56,080-byte MW5 firmware;
- board RFE type 5;
- exact recovered `PHY_REG_PG` Type1, Type5 and Type7 data.

Development dual-radio build:

- integrated RTL8197F 2.4 GHz;
- PCIe RTL8822B 5 GHz;
- PCI driver registration restored for the OpenWrt build path;
- exact 81,896-byte MW5 RTL8822B production firmware;
- exact 77,264-byte MP firmware for diagnostics only;
- exact recovered Type1, Type6 and Type13 power tables.

## Linux-6 compatibility work

- central modern timer wrapper preserving legacy callback-data semantics;
- current DMA mapping and synchronization wrappers;
- current `net_device` address handling;
- current time, procfs and PCI helper compatibility;
- removal/replacement of direct old-BSP cache and GPIO dependencies;
- cfg80211 enum compatibility for source compilation;
- deterministic build metadata;
- vendor `crc32` collision renamed;
- conservative Wireless Extensions/vendor private-ioctl control path;
- old vendor cfg80211 shim disabled;
- exact MW5 firmware and board tables embedded in the vendor HAL data objects.

## Proven by build

Both profiles compile and link as `rtl8192cd.ko` in a Linux-6.12 API smoke
build.  The default 2.4-GHz profile was rebuilt from this final source tree.
This proves source/API consistency only.  The x86 smoke modules are not usable
on the router.

## Not yet proven

- MIPS/OpenWrt cross-build of the complete image;
- loading on the MW5 hardware;
- correct MMIO/IRQ/DMA/coherency behavior;
- firmware boot and RF calibration;
- safe RF output power;
- AP, station, WDS, repeater or mesh operation;
- PCIe RTL8822B enumeration and interrupts;
- compatibility with standard OpenWrt `hostapd`/UCI wireless management.

Do not autoload the module for first bring-up.  Do not redistribute recovered
firmware or register-table files before reviewing their licensing.
