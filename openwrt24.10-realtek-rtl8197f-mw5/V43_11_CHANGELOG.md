# v43.11 changelog

- Enable the RTL8197F OEM CRC/FCS descriptor-length convention on Tenda Nova MW5.
- Add `realtek,sdk-crc-lengths` to the MW5 DTS and force the policy in the MW5 driver branch.
- Access MW5 six-DWORD/24-byte CPU DMA descriptors through a KSEG1 uncached CPU alias.
- Configure MW5 TXRINGCR as a true single-ring setup: only TX ring 0 enabled.
- Keep DMA_CR1 ring-0 length and DMA_CR4 ring-0 tail-aware mode.
- Omit TX EOR/WRAP on the MW5 tail-aware TX ring, matching New_swNic.
- Stop rewriting completed MW5 TX descriptors during CDP reclamation.
- Stop treating stale TX OWN as a reuse blocker after CPUTPDCR0/software tail has reclaimed the slot.
- Preserve the v43.9+ switch-to-CPU RX tag decoder (`0x0400 + source port`).
- Preserve TX protocol-9 tags (`0x9202` LAN, `0x9208` WAN).
- Extend `/proc/rd05-rtknet` with descriptor policy (`dwords`, `stride`, `kseg1`, `sdk_crc`, `txringcr`).
- Update `mw5-netdiag` to retain v43.10/v43.11 RX-bad and TX completion diagnostics.
- Keep standard OpenWrt firewall4 configuration unchanged.
