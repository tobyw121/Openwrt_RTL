#!/usr/bin/env bash
set -euo pipefail

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

CONFIG_FILE=${1:-configs/rtl8197f_all_devices_defconfig}
JOBS=${JOBS:-1}

if [[ ! -f "$CONFIG_FILE" ]]; then
    echo "Missing config seed: $CONFIG_FILE" >&2
    exit 2
fi

./CHECK_SOURCE_TREE_V41.sh
./VERIFY_FULLFLASH_TEMPLATES_V41_1.sh
./scripts/feeds update -a
./scripts/feeds install -a
cp "$CONFIG_FILE" .config
make defconfig
./BOOTSTRAP_BUILD_ENV_V41_2.sh
make target/linux/compile -j1 V=s
make -j"$JOBS" V=sc RTL8197F_BUILD_FULLFLASH=1
