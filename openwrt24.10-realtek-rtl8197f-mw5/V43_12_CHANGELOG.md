# v43.12 changelog

- Correct the MW5 four-byte Realtek CPU-to-switch tag from the speculative
  `0x92xx` layout to protocol-9 plus destination-mask only (`0x90xx`).
- LAN port 1 now emits `0x9002`; WAN port 3 emits `0x9008`.
- Remove the unsupported TX learn-disable bit from `tag_rtl4_9.c`.
- Expand the TX destination field to the low byte, matching the Realtek rtl4a
  four-byte layout while preserving the MW5 port mapping.
- Preserve the observed switch-to-CPU RX format `0x0400 | source-port`.
- Preserve all v43.11 DMA/FCS/KSEG1/ring0 fixes unchanged.
- Keep the standard OpenWrt firewall unchanged.
- Update the built-in `mw5-netdiag` banner and log collection for v43.12 and
  print the expected LAN/WAN/RX tag words automatically.
- Add `CHECK_MW5_V43.12.sh` and update the generic MW5 WAN/LAN source checker.
