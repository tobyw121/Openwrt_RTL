# RTL8197F v41.3 - MW5 boot checksum and early UART fix

## Critical MW5 corrections

- Adds the Realtek/Tenda SquashFS trailer required by the RTL8197F bootloader:
  - increments SquashFS `bytes_used` by two;
  - pads the rootfs with zeroes to a 4 KiB boundary;
  - appends a big-endian 16-bit word that makes the complete rootfs sum zero.
- The algorithm was verified by reconstructing the original MW5 rootfs region
  byte-for-byte, including checksum word `0x326d` in the bundled stock template.
- The MW5 fullflash builder now rejects firmware without a valid rootfs trailer.
- Corrects the LZMA loader UART base for MW5 and AC23 from the inherited
  AR7xxx address `0xb8020000` to RTL8197F UART0 at `0xb8147000`.
- Keeps host tools and cross-toolchain bootstrap ordering from v41.2.

## Safety

MW5 fullflash images made by v41.1 or v41.2 must not be flashed again. Restore
an original board-matched SPI dump before testing a newly built v41.3 image.
The new source logic is statically and byte-for-byte validated, but hardware
boot remains experimental until confirmed on the target device.
