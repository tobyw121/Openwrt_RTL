# RTL8197F build configurations

Copy exactly one seed file to `.config`, then run `make defconfig`.

```sh
cp configs/rtl8197f_mw5_defconfig .config
make defconfig
```

Available seeds:

- `rtl8197f_rd05_defconfig`: Xiaomi R4 / RD05.
- `rtl8197f_mw5_defconfig`: Tenda Nova MW5.
- `rtl8197f_ac23_defconfig`: Tenda AC23 / Lynx 8000.
- `rtl8197f_all_devices_defconfig`: all three profiles, using per-device root filesystems.

The root of the source tree deliberately contains no generated `.config`.
