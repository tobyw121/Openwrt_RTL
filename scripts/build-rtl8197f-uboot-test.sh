#!/usr/bin/env bash

set -euo pipefail

TOPDIR="$(cd "$(dirname "$0")/.." && pwd)"
PKG_DIR="${TOPDIR}/package/boot/realtek/generic"
OUT_DIR="${TOPDIR}/bin/targets/realtek/rtl8197f"
DEFCONFIG="${PKG_DIR}/def-rtl8197f-config"

if [[ ! -d "${PKG_DIR}" ]]; then
        echo "realtek bootloader sources not found at ${PKG_DIR}" >&2
        exit 1
fi

if [[ ! -f "${DEFCONFIG}" ]]; then
        echo "Default RTL8197F defconfig missing at ${DEFCONFIG}" >&2
        exit 1
fi

if [[ -z "${CROSS_COMPILE:-}" ]]; then
        compiler_path=$(find "${TOPDIR}/staging_dir" -maxdepth 4 -type f \( -name 'mips-openwrt-linux-*-gcc' -o -name 'mips-linux-gcc' \) | head -n1 || true)
        if [[ -z "${compiler_path}" ]]; then
                echo "No MIPS toolchain found. Run 'make toolchain/install' first or set CROSS_COMPILE manually." >&2
                exit 1
        fi

        CROSS_COMPILE="${compiler_path%gcc}"
fi

export ARCH=mips
export CROSS_COMPILE
export CROSS="${CROSS_COMPILE}"

pushd "${PKG_DIR}" >/dev/null

cp "${DEFCONFIG}" .config
make clean >/dev/null
make all

mkdir -p "${OUT_DIR}"
cp boot/boot.bin "${OUT_DIR}/u-boot-realtek_rtl8197f_uboot_test.bin"

echo "U-Boot image copied to ${OUT_DIR}/u-boot-realtek_rtl8197f_uboot_test.bin"

popd >/dev/null
