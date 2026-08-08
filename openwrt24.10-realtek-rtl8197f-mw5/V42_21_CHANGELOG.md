# RTL8197F v42.21

## MW5 SPI auto-map coordinate fix

Hardware boot logs from v42.20 showed that the firmware parser path was reached,
but no `kernel` or `rootfs` subpartitions were created. The RTL8197F SPI-ROM
MTD driver reported raw bytes that exactly matched the private full-flash image
at the requested offsets. The MW5 DTS nevertheless configured
`realtek,read-shift = <1>`.

For a child `firmware` MTD partition this caused reads at relative offset zero
to reach master offset `0x30000`, after which the driver subtracted one and
returned bytes from `0x2ffff`. The parser therefore saw:

```
00 63 72 36 ...   instead of   63 72 36 63 ... (`cr6c`)
```

The same error changed the computed SquashFS read from `hsqs` to a one-byte
prefixed sequence. v42.21 sets the MW5 property explicitly to zero. The
RTL8197F firmware splitter from v42.20 is otherwise unchanged.

The setting is MW5-specific. RD05, AC23 and other RTL8197F boards retain their
existing values until their auto-map coordinates are confirmed on hardware.

## Regression coverage

`CHECK_SOURCE_TREE_V42.sh` now requires:

- `realtek,read-shift = <0>` in the MW5 DTS;
- no stale `<1>` override in that DTS;
- `spi_automap_read_shift: 0` in the MW5 board profile;
- the v42.20 firmware-node/mtdsplit checks.

## Hardware status

The supplied direct test image retains the v42.20 binary parser-path patch and
changes only the embedded MW5 DT property from one to zero. Physical rootfs
mount and userspace boot remain to be confirmed.
