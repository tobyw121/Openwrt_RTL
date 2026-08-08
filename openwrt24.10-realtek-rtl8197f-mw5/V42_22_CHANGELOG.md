# RTL8197F v42.22

## MW5 SquashFS/OEM checksum separation

The v42.21 hardware log proved that the firmware parser and SPI auto-map now
work: the kernel created `kernel` and `rootfs`, selected `mtd5`, and read a
valid SquashFS superblock.  Mounting then failed with:

```
unable to read id index table
```

For a SquashFS image with one UID/GID entry, Linux 6.6 requires the final ID
index table to occupy exactly 8 bytes.  v42.21 changed the superblock
`bytes_used` from the real filesystem length to `bytes_used + 2` in order to
account for the Tenda checksum trailer.  This made:

```
bytes_used - id_table_start = 10
```

and the SquashFS sanity check rejected the filesystem.

v42.22 leaves `bytes_used` unchanged and keeps the Tenda BE16 additive
checksum after zero padding at the next 4 KiB boundary.  The proprietary
boot-length marker at superblock offset `+0x08` and the checksum coverage are
unchanged.  The resulting Linux invariant is again:

```
bytes_used - id_table_start = 8
```

Updated components:

- `scripts/rtl8197f-rootfs-checksum.py`
- `scripts/rtl8197f-tenda-mw5-fullflash.py`
- `scripts/rtl8197f-tenda-lynx-fullflash.py`
- Tenda image comments and source-tree regression tests

The fix applies to generated MW5 and AC23/Tenda rootfs trailers.  It does not
change the protected bootloader, factory, CFM, log or environment regions.
