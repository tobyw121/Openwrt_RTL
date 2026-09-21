#!/bin/sh
set -eu

CONFIG="${MW5_CONFIG:-configs/rtl8197f_mw5_defconfig}"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 1)}"

[ -f "$CONFIG" ] || { echo "missing config: $CONFIG" >&2; exit 1; }
cp "$CONFIG" .config

# The full tree ships feed working trees with project-specific local changes.
# Do not merge remote feed heads implicitly. Install the bundled feed state.
./scripts/feeds install -a
make defconfig
make -j"$JOBS" V=sc
