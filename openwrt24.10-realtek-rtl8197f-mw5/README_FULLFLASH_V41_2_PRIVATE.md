# RTL8197F v41.2 PRIVATE - automatic full-flash builds

This tree automatically builds board-matched full-flash images for Xiaomi RD05,
Tenda Nova MW5 and Tenda AC23/Lynx. The embedded templates contain private
factory, MAC and RF calibration data and must not be published or exchanged
between physical devices.

## Correct fresh-tree build order

A fresh OpenWrt source tree does not contain a MIPS cross compiler. v41.2 uses:

1. `make tools/install`
2. `make toolchain/install`
3. `make target/linux/compile`
4. `make` for the selected packages and images

The previous v41.1 helper entered `target/linux/compile` before steps 1 and 2,
which could produce `mipsel-openwrt-linux-musl-gcc: not found` and a Flex/M4
failure. All v41.2 helpers call `BOOTSTRAP_BUILD_ENV_V41_2.sh` first.

## Verify and build every device

```sh
./CHECK_SOURCE_TREE_V41.sh
./VERIFY_FULLFLASH_TEMPLATES_V41_1.sh
./BUILD_ALL_FULLFLASH_V41_2.sh
```

The old command remains supported and forwards to the corrected helper:

```sh
./BUILD_ALL_FULLFLASH_V41_1.sh
```

## Build one configuration

```sh
./BUILD_RTL8197F_V41_2.sh configs/rtl8197f_rd05_defconfig
./BUILD_RTL8197F_V41_2.sh configs/rtl8197f_mw5_defconfig
./BUILD_RTL8197F_V41_2.sh configs/rtl8197f_ac23_defconfig
./BUILD_RTL8197F_V41_2.sh configs/rtl8197f_all_devices_defconfig
```

Expected output suffixes:

```text
squashfs-spi-full.bin
squashfs-spi-full.bin.sha256
squashfs-spi-full.bin.spi-manifest.txt
```
