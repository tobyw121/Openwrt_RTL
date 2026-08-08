# RTL819x rtl865x/rtknet vendor source staged for Linux 6.6 porting

This directory contains the Realtek GPL SDK `target/linux/rtknet` Ethernet,
rtl865x switch-core, fastpath and RTL8367 support sources that were extracted
from the vendor OpenWrt-21.02/Linux-4.4 tree.

Status in this OpenWrt 24.10 / Linux 6.6.114 tree:

- The source is staged as a reference/porting base and is **not wired into the
  kernel Makefile by default**.
- The original driver uses Linux-4.4-era APIs, global Realtek Kconfig symbols
  such as `RTL_819X`, `RTL_8197F`, `RTL_8367R_SUPPORT`, `OPENWRT_SDK`, legacy
  swconfig paths, Realtek fastpath hooks and direct MMIO assumptions.
- Building it as-is against Linux 6.6 is not expected to succeed.
- The maintainable Linux-6.6 path is to use the in-kernel DSA Realtek switch
  drivers for RTL8367* where possible, then port the RTL8197F/RTL8197FH rtl865x
  MAC DMA path separately as a modern `net_device` + NAPI + phylink/DSA
  CPU-port driver.  This tree now carries that native CPU-interface driver as
  `drivers/net/ethernet/rtl8197f_rtknet.c` behind `CONFIG_RTL8197F_RTKNET`.

The companion patches in `target/linux/realtek/patches-6.6` add RTL8367-family
compatible strings to the existing Linux DSA Realtek drivers and expose an
`RTL8197F_RTKNET_REFERENCE` Kconfig guard documenting this staged code.
