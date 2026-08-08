# RTL8197F v42.7

## MW5 early boot fix

- Stop the RTL8197F watchdog at the first LZMA-loader instructions, before cache/BSS setup.
- Stop it again in `board_init()` and Linux `prom_init()`.
- Correct the loader UART to vendor byte accesses.
- Select the RTL8197F/FS THR alias at `UART0 + 0x24` and the FH/VG register at `UART0 + 0x00` from the chip ID.
- Emit `[LDR]` before the normal loader banner.
- Correct Linux early `prom_init()` UART access in the same way.
- Add a direct, checksum-corrected v42.7 MW5 Full-Flash test image derived from the user's v42.6 image without changing rootfs or private partitions.
