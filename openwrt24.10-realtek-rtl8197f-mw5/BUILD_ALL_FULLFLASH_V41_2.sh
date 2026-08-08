#!/usr/bin/env bash
set -euo pipefail
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
exec "$ROOT/BUILD_ALL_FULLFLASH_V41_3.sh" "$@"
