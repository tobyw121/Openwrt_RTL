# OpenWrt 24.10 RTL8197F / RTL8367 integration v41.0

This source tree was rebuilt from the supplied clean `openwrt-24.10.0.zip` tree.
It does not contain a copied `build_dir`, `staging_dir`, feed checkout, generated
`.config`, host binary, or target output from another computer.

The user's `RTL8917F` wording is treated as `RTL8197F` throughout this port.

## Main design rule

The original OpenWrt host tools, toolchain recipes and base packages remain
untouched. Only target-specific Realtek additions are integrated. In particular,
the original coherent package directories for mbedTLS, Dropbear and odhcpd are
preserved. This avoids the mixed-version and duplicate-patch failures seen in the
older assembled trees.

The RTL8197F target remains pinned to Linux 6.6.114 because this is the kernel
version for which the imported RTL8197F, RTL8367 and experimental rtl8192cd
changes were developed and previously reached a successful module/package build.

## Device configurations

Copy one seed to `.config`, then run `make defconfig`:

```sh
cp configs/rtl8197f_rd05_defconfig .config
make defconfig
```

Available configurations:

- `configs/rtl8197f_rd05_defconfig` — Xiaomi Mi WiFi R4 / RD05
- `configs/rtl8197f_mw5_defconfig` — Tenda Nova MW5
- `configs/rtl8197f_ac23_defconfig` — Tenda AC23 / Lynx 8000
- `configs/rtl8197f_all_devices_defconfig` — all three profiles with per-device root filesystems

The all-device seed enables `CONFIG_TARGET_PER_DEVICE_ROOTFS=y`, preventing the
experimental MW5 WLAN package from being placed into RD05 and AC23 images.

## Normal build

Install the normal OpenWrt 24.10 build dependencies on the host first. Then:

```sh
./scripts/feeds update -a
./scripts/feeds install -a
cp configs/rtl8197f_mw5_defconfig .config
make defconfig
make target/linux/compile -j1 V=s
make -j1 V=sc
```

A helper performs the same sequence and stops immediately if feed update fails:

```sh
./BUILD_RTL8197F_V41.sh configs/rtl8197f_mw5_defconfig
```

Use a serial first full build. Only increase `JOBS` after one successful build:

```sh
JOBS=8 ./BUILD_RTL8197F_V41.sh configs/rtl8197f_mw5_defconfig
```

## Private full-flash images

Normal builds do not need or include factory/SPI dumps. Full-flash generation is
opt-in because those images preserve device-specific calibration, MAC addresses,
factory data and bootloader partitions.

```sh
./BUILD_PRIVATE_FULLFLASH_V41.sh rd05 /path/to/verified-16MiB-dump.bin
./BUILD_PRIVATE_FULLFLASH_V41.sh ac23 /path/to/verified-8MiB-dump.bin
./BUILD_PRIVATE_FULLFLASH_V41.sh mw5 /path/to/verified-8MiB-dump.bin
```

Never flash a full-SPI image without a verified backup and serial/SPI recovery.

## Integrated target work

- RTL8197F little-endian MIPS 24Kc subtarget
- RD05, MW5 and AC23 DTS/image profiles
- RTL8197F GPIO, SPI, SPI-ROM map, PCIe and USB host glue
- native RTL8197F/rtl865x CPU-interface Ethernet development driver
- RTL8367-family Realtek DSA extensions
- RTL8367D chip ID `0x6642`, revision family `0x003x`
- legacy RTL8367R/RB IDs and RTL8367C/R/RB/SB common family paths
- RTL8363SC-VB / RTL8363NB / RTL8364B class IDs used by MW5-family work
- corrected MTD firmware splitter include ordering
- corrected DSA patch ordering and patch 328 context
- optional userspace RTL8367D compatibility/diagnostic package
- experimental MW5 RTL8197F `rtl8192cd` Linux 6 module package
- safe opt-in RD05, MW5 and AC23 full-flash construction helpers

## Validation performed

- source tree static validator passed
- no duplicate patch contents
- all Realtek unified-diff hunk line counts verified
- no `.rej`, `.orig`, `build_dir`, `staging_dir`, feeds, target `bin`, root `.config` or Python bytecode in the delivered source
- mbedTLS, Dropbear, odhcpd, `toolchain/` and original `tools/` contents preserved from the clean source, except for the separately added RD05 diagnostic helper
- all four configuration seeds were expanded successfully with `make defconfig`
- `make target/linux/prepare -j1 V=s` completed successfully against Linux 6.6.114 and applied the complete generic and Realtek patch stack without rejects

## Current engineering status and limits

This is a development tree, not a claim of production-ready hardware support.

- The MW5 `rtl8192cd.ko` and its IPK were previously built successfully for Linux 6.6.114, but radio runtime, calibration, stability and regulatory behavior still require serial-console hardware testing.
- RD05 native Ethernet/DSA code is present, but the earlier hardware test still reported that the attached PC did not receive router frames. This tree does not claim that issue is solved.
- AC23 DTS, image layout and wired RTL8197F/RTL8367 integration are reconstructed development support. Its exact switch port map, RGMII delays, flash layout and WLAN hardware have not been verified on an AC23 board.
- Not every RTL8367 marketing variant is register-compatible. The code contains explicit known-ID paths and fallbacks; unknown IDs must not be assumed supported without register-level validation.
- A complete `make world` was not executed in the packaging environment. The delivered tree passed source, configuration and kernel-prepare validation only.

Run `./CHECK_SOURCE_TREE_V41.sh` before building or redistributing the tree.

## v41.1 private automatic full-flash mode

In the v41.1 PRIVATE archive, full-flash output is enabled by default for RD05,
MW5 and AC23. See `README_FULLFLASH_V41_1_PRIVATE.md`. The embedded templates and
all resulting full-flash files are private and must not be published.

## v41.2 fresh-tree bootstrap

Use `BUILD_RTL8197F_V41_2.sh` or `BUILD_ALL_FULLFLASH_V41_2.sh`. These helpers
build OpenWrt host tools and the MIPS cross-toolchain before compiling the
kernel. See `README_FULLFLASH_V41_2_PRIVATE.md`.
