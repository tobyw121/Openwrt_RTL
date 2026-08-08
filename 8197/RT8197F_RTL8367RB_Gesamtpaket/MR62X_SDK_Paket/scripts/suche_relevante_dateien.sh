#!/usr/bin/env bash
set -euo pipefail
ROOT=${1:-selected_sources/sdk/openwrt-21.02}
rg -n -S 'RTL8197F|8197F|8367RB|8367R|RTL_8367|RTL_83XX|MDC|MDIO|SMI|RGMII|rtknet|SPI_NAND|SPINAND|mips_97f' "$ROOT"
