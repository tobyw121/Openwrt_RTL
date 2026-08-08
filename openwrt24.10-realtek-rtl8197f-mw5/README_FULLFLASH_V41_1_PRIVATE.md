# RTL8197F v41.1 PRIVATE – automatic full-flash builds

This tree automatically builds a personalized `squashfs-spi-full.bin` for every
selected RTL8197F device profile:

- Xiaomi R4 / RD05 – 16 MiB
- Tenda Nova MW5 – 8 MiB
- Tenda AC23 / Lynx – 8 MiB

The matching complete SPI templates are stored in `fullflash-templates/` and are
selected by fixed per-device paths. The builders validate size, model/factory
markers and firmware signatures before producing an output. They never use one
board's dump for another board.

## Privacy warning

This is a PRIVATE tree. The bundled dumps and every generated full-flash image
contain device-specific MAC addresses, serial numbers, factory/RF calibration,
configuration and possibly credentials. Do not publish them.

## Verify

```sh
./CHECK_SOURCE_TREE_V41.sh
./VERIFY_FULLFLASH_TEMPLATES_V41_1.sh
```

## Build all devices and all full-flash outputs

```sh
./BUILD_ALL_FULLFLASH_V41_1.sh
```

The normal device build helper also builds full-flash by default:

```sh
./BUILD_RTL8197F_V41.sh configs/rtl8197f_rd05_defconfig
./BUILD_RTL8197F_V41.sh configs/rtl8197f_mw5_defconfig
./BUILD_RTL8197F_V41.sh configs/rtl8197f_ac23_defconfig
./BUILD_RTL8197F_V41.sh configs/rtl8197f_all_devices_defconfig
```

Expected output suffix for each selected device:

```text
squashfs-spi-full.bin
squashfs-spi-full.bin.sha256
squashfs-spi-full.bin.spi-manifest.txt
```

To intentionally suppress private full-flash outputs for one manual build:

```sh
make -j1 V=sc RTL8197F_BUILD_FULLFLASH=0
```
