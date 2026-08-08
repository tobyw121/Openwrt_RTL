#!/usr/bin/env bash
set -euo pipefail

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

DEVICE=${1:-all}
JOBS=${JOBS:-1}

case "$DEVICE" in
  rd05) CONFIG=configs/rtl8197f_rd05_defconfig ;;
  mw5)  CONFIG=configs/rtl8197f_mw5_defconfig ;;
  ac23) CONFIG=configs/rtl8197f_ac23_defconfig ;;
  all)  CONFIG=configs/rtl8197f_all_devices_defconfig ;;
  *) echo "Usage: $0 [rd05|mw5|ac23|all]" >&2; exit 2 ;;
esac

./VERIFY_FULLFLASH_TEMPLATES_V41_1.sh
cp "$CONFIG" .config
make defconfig
./BOOTSTRAP_BUILD_ENV_V41_2.sh
make -j"$JOBS" V=sc RTL8197F_BUILD_FULLFLASH=1
