#!/usr/bin/env bash
set -euo pipefail

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$ROOT"

BOOTSTRAP_JOBS=${BOOTSTRAP_JOBS:-1}
HOST_M4="$ROOT/staging_dir/host/bin/m4"

if [[ ! -f Makefile || ! -x scripts/feeds ]]; then
    echo "ERROR: not an OpenWrt source root: $ROOT" >&2
    exit 2
fi

if [[ ! -f .config ]]; then
    echo "ERROR: .config is missing. Copy a configs/rtl8197f_*_defconfig first and run make defconfig." >&2
    exit 2
fi

# A prematurely started target/linux build can leave a missing or broken M4 path.
# Remove only the M4 host-tool state when it is unusable; preserve all other tools.
if [[ -L "$HOST_M4" || -e "$HOST_M4" ]]; then
    if [[ ! -x "$HOST_M4" ]] || ! "$HOST_M4" --version >/dev/null 2>&1; then
        echo "Repairing incomplete host m4 state..."
        rm -f "$HOST_M4"
        rm -rf "$ROOT"/build_dir/host/m4-*
        if [[ -d "$ROOT/staging_dir/host/stamp" ]]; then
            find "$ROOT/staging_dir/host/stamp" -maxdepth 1 -type f \
                \( -name '.m4*' -o -name '*m4*installed*' \) -delete
        fi
    fi
fi

echo "==> Building OpenWrt host tools first"
make -j"$BOOTSTRAP_JOBS" V=s tools/install

if [[ ! -x "$HOST_M4" ]] || ! "$HOST_M4" --version >/dev/null 2>&1; then
    echo "ERROR: OpenWrt host m4 is still unavailable after tools/install: $HOST_M4" >&2
    exit 1
fi

echo "==> Building and installing the MIPS musl cross-toolchain"
make -j"$BOOTSTRAP_JOBS" V=s toolchain/install

CROSS_GCC=""
for candidate in "$ROOT"/staging_dir/toolchain-*/bin/mipsel-openwrt-linux-musl-gcc; do
    if [[ -x "$candidate" ]]; then
        CROSS_GCC=$candidate
        break
    fi
done

if [[ -z "$CROSS_GCC" ]]; then
    echo "ERROR: mipsel-openwrt-linux-musl-gcc was not produced by toolchain/install." >&2
    exit 1
fi

"$CROSS_GCC" --version >/dev/null

echo "PASS: host m4: $HOST_M4"
echo "PASS: cross gcc: $CROSS_GCC"
