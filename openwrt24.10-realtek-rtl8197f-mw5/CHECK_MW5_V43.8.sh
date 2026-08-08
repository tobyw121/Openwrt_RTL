#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# Compatibility entry point retained for users of the v43.8 tree.
ROOT="${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}"
exec "$ROOT/CHECK_MW5_V43.9.sh" "$ROOT"
