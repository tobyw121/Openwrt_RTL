# RTL8197F OpenWrt 24.10 development tree v42.6

## MW5 bootloader rootfs-length fix

The MW5 OEM bootloader does not use the normal SquashFS-4 `bytes_used` field
when deciding how much rootfs data to checksum. Its recovered routine reads a
big-endian 32-bit marker from SquashFS superblock offset `+0x08`, adds `0x282`,
and checks that resulting number of bytes.

OpenWrt normally stores the SquashFS creation timestamp at `+0x08`. In the
v42.5 image this timestamp was `0x7ea26c6a` when interpreted by the bootloader,
causing a requested checksum length of `0x7ea26eec` bytes. The bootloader thus
appeared to hang after `irq:0x00008080` before printing the kernel jump message.

v42.6 now:

- writes `checksum_end - 0x282` as a big-endian marker at rootfs `+0x08`;
- preserves the normal little-endian SquashFS-4 `bytes_used` field at `+0x28`;
- recalculates the final big-endian 16-bit rootfs checksum word;
- validates the proprietary marker in the MW5 fullflash builder;
- rejects images whose marker would make the bootloader scan beyond the real
  rootfs checksum trailer;
- includes a regression test against the board-matched stock MW5 SPI image.

For the supplied v42.5-derived image:

- rootfs offset: `0x2e3122`
- bootloader marker: `0x0025cd80`
- bootloader check length: `0x25d002`
- rootfs checksum word: `0xa472`
- rootfs additive sum: `0x0000`

No bootloader, CFG, CFM, CFM_BACKUP, LOG or ENV bytes are modified.
