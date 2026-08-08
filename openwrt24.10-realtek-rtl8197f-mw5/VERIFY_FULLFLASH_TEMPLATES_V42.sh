#!/usr/bin/env bash
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

(cd fullflash-templates && sha256sum -c SHA256SUMS)
python3 scripts/rtl8197f-fullflash-verify.py --board rd05 \
  --template fullflash-templates/xiaomi-r4-rd05-spi.bin
python3 scripts/rtl8197f-fullflash-verify.py --board mw5 \
  --template fullflash-templates/tenda-nova-mw5-spi.bin
python3 scripts/rtl8197f-fullflash-verify.py --board ac23 \
  --template fullflash-templates/tenda-ac23-spi.bin
python3 scripts/rtl8197f-validate-board-profiles.py --tree "$ROOT"

echo 'PASS: v42 private templates, headers, checksums and board profiles'
