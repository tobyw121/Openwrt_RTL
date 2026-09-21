# Validation record: RTL8192CD MW5 Linux 6.6 port

## Inputs

- Vendor tree: `rtl8192cd-rtl8197f.tar.zst`.
- Kernel API reference: Linux `6.6.114` source supplied with the task.
- Intended integration: OpenWrt 24.10, Realtek RTL8197F MW5 target.

## What was validated here

1. Linux 6.6.114 cfg80211/platform/PCI/NAPI/NVMEM API signatures were checked
   directly against the supplied source tree.
2. The modified driver was compiled as an external module against the available
   x86_64 Linux 6.12.96 build headers. The build completed through C compilation,
   MODPOST and final `rtl8192cd.ko` link.
3. The linked module references cfg80211/NAPI/NVMEM and carries the new
   `rtl8197f_soc`/`rtl8822b_pci` attachment symbols.
4. Linux6 source/config guards remove the WEXT `ndo_do_ioctl`/wireless-handler
   userspace path for the MW5 variant.
5. DMA allocation in the Linux6 path is tied to the real platform/PCI
   `struct device`; bus backends own MMIO and IRQ-vector lifetime.

## What was not validated here

- No MIPS OpenWrt 24.10 target toolchain is installed in this runtime.
- The supplied Linux 6.6.114 tree cannot be fully configured here because the
  runtime lacks the matching Kconfig lexer/parser build dependencies (`flex`).
- No Tenda MW5 hardware is attached, so RF bring-up, IRQ polarity, BAR/MMIO
  values, NVMEM offsets, calibration payloads, DFS/CAC and throughput are not
  hardware-tested.
- The referenced OpenWrt image ZIP itself was not supplied; only its `.sha256`
  file is present. Therefore no board DTS from that ZIP could be inspected or
  patched and no replacement image checksum can be generated.

## Required target validation

Build inside the real OpenWrt 24.10 tree, merge the DT/NVMEM properties using
actual MW5 resource/partition values, then run the checklist in
`openwrt/MW5-CFG80211-CHECKLIST.md` before treating the port as production-ready.
