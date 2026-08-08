#!/usr/bin/env bash
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"
JOBS=${JOBS:-1}
./CHECK_SOURCE_TREE_V42.sh
./VERIFY_FULLFLASH_TEMPLATES_V42.sh
./scripts/feeds update -a
./scripts/feeds install -a
cp configs/rtl8197f_all_devices_defconfig .config
make defconfig
./BOOTSTRAP_BUILD_ENV_V41_2.sh
make target/linux/compile -j1 V=s
make -j"$JOBS" V=sc RTL8197F_BUILD_FULLFLASH=1
find bin/targets/realtek/rtl8197f -maxdepth 1 -type f \
  \( -name '*rd05*squashfs-spi-full.bin' \
     -o -name '*mw5*squashfs-spi-full.bin' \
     -o -name '*ac23*squashfs-spi-full.bin' \) \
  -print -exec stat -c '  size=%s' {} \; -exec sha256sum {} \;
