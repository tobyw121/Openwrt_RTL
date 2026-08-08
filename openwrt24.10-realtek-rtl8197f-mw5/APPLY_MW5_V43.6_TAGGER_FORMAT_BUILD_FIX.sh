#!/bin/sh
# Apply the complete v43.5 -> v43.6 source correction.
set -eu
TREE=${1:-.}
PATCH_FILE=${2:-MW5_V43.6_TAGGER_FORMAT_BUILD_FIX.patch}

case "$TREE" in
/*) : ;;
*) TREE="$(pwd)/$TREE" ;;
esac
case "$PATCH_FILE" in
/*) : ;;
*) PATCH_FILE="$(pwd)/$PATCH_FILE" ;;
esac

[ -d "$TREE/target/linux/realtek" ] || {
	echo "Not an OpenWrt Realtek tree: $TREE" >&2
	exit 1
}
[ -f "$PATCH_FILE" ] || {
	echo "Patch not found: $PATCH_FILE" >&2
	exit 1
}

cd "$TREE"
patch --dry-run --fuzz=0 -p1 < "$PATCH_FILE"
patch --fuzz=0 -p1 < "$PATCH_FILE"
echo "Applied MW5 v43.6 tagger format build correction."
