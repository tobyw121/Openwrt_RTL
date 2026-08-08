#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PATCH="$HERE/MW5_V43.11_FCS_UNCACHED_RING0_FIX.patch"
ROOT=${1:-.}

if [ ! -f "$PATCH" ]; then
	echo "Missing patch: $PATCH" >&2
	exit 1
fi

cd "$ROOT"
patch --dry-run --fuzz=0 -p1 < "$PATCH"
patch --fuzz=0 -p1 < "$PATCH"

if [ -x ./CHECK_MW5_V43.11.sh ]; then
	./CHECK_MW5_V43.11.sh .
fi

echo "MW5 v43.11 patch applied successfully."
