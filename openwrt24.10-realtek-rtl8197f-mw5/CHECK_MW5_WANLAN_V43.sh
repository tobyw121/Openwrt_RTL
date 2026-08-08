#!/bin/sh
# Static consistency checks for the MW5 WAN/LAN SDK port.
set -u

ROOT=${1:-$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)}
DRIVER="$ROOT/target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
DTS="$ROOT/target/linux/realtek/dts/rtl8197f_tenda_nova-mw5.dts"
NETWORK="$ROOT/target/linux/realtek/base-files/etc/board.d/02_network"
TAGGER="$ROOT/target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c"
MW5_NET_DEFAULTS="$ROOT/target/linux/realtek/base-files/etc/uci-defaults/99-mw5-dsa-network"
PATCH="$ROOT/target/linux/realtek/patches-6.6/329-net-dsa-realtek-add-rtl4-protocol9-for-mw5.patch"

pass=0
fail=0

ok() {
	printf 'PASS: %s\n' "$1"
	pass=$((pass + 1))
}

bad() {
	printf 'FAIL: %s\n' "$1" >&2
	fail=$((fail + 1))
}

require_file() {
	if [ -f "$1" ]; then
		ok "file exists: ${1#$ROOT/}"
	else
		bad "missing file: ${1#$ROOT/}"
	fi
}

require_grep() {
	file=$1
	pattern=$2
	desc=$3
	if grep -Eq "$pattern" "$file" 2>/dev/null; then
		ok "$desc"
	else
		bad "$desc"
	fi
}

reject_grep() {
	file=$1
	pattern=$2
	desc=$3
	if grep -Eq "$pattern" "$file" 2>/dev/null; then
		bad "$desc"
	else
		ok "$desc"
	fi
}

for file in "$DRIVER" "$DTS" "$NETWORK" "$TAGGER" "$PATCH"; do
	require_file "$file"
done

[ "$fail" -eq 0 ] || exit 1

require_grep "$DRIVER" 'DRV_VERSION[[:space:]]+"1\.4\.11-sdk-mw5-fcs-uncached-v43\.11"' \
	'driver core remains validated MW5 WAN/LAN v43.11'
require_grep "$DRIVER" 'rtl8197f_rtk_program_tx_ring_geometry' \
	'TX ring geometry helper is present'
require_grep "$DRIVER" 'expected_dma_cr1[[:space:]]*=[[:space:]]*\(priv->tx_ring_size[[:space:]]*-[[:space:]]*1\)[[:space:]]*\*[[:space:]]*priv->desc_stride' \
	'DMA_CR1 is derived from ring count and descriptor stride'
require_grep "$DRIVER" 'RTL_RTK_DMA_CR4_TX_RING0_TAIL_AWARE' \
	'TX ring 0 tail-aware mode is programmed'
require_grep "$DRIVER" 'RTL_RTK_TXRINGCR_TX_RING1_EN[[:space:]]*\|' \
	'MW5 single-ring path explicitly clears TX ring1-3 enables'
require_grep "$DRIVER" 'of_machine_is_compatible\("tenda,nova-mw5"\)' \
	'MW5 has an explicit driver branch'
require_grep "$DRIVER" 'priv->rd05_sdk_newdesc_rx[[:space:]]*=[[:space:]]*true;' \
	'MW5/New-Descriptor RX path is enabled'
require_grep "$DRIVER" 'priv->desc_dwords[[:space:]]*=[[:space:]]*6;' \
	'MW5 native descriptor size is six DWORD'
require_grep "$DRIVER" '"rtl4_9/4byte"[[:space:]]*:[[:space:]]*"rtl8_4/8byte"' \
	'MW5 and RD05 tag protocols remain separated'
require_grep "$DRIVER" 'if[[:space:]]*\(!mw5\)[[:space:]]*\{' \
	'MW5 leaves hardware CPU-tag parser/generator disabled for DSA wire pass-through'
require_grep "$DRIVER" 'if[[:space:]]*\(!mw5\)[[:space:]]*$' \
	'MW5 hardware TX CPU-tag generation is conditionally disabled'
require_grep "$DRIVER" 'gmii[[:space:]]*\|=[[:space:]]*RTL_RTK_P0GMIICR_CPU_TAG_TX;' \
	'RD05 retains its existing hardware TX CPU-tag setting'

require_grep "$DTS" 'realtek,cpu-tag-4bytes-proto9;' \
	'MW5 DTS requests four-byte protocol-9 tags'
require_grep "$DTS" 'port@1' 'MW5 LAN physical port 1 is present'
require_grep "$DTS" 'label[[:space:]]*=[[:space:]]*"lan";' 'MW5 LAN label is present'
require_grep "$DTS" 'port@3' 'MW5 WAN physical port 3 is present'
require_grep "$DTS" 'label[[:space:]]*=[[:space:]]*"wan";' 'MW5 WAN label is present'
require_grep "$DTS" 'port@6' 'MW5 CPU physical port 6 is present'
require_grep "$DTS" 'realtek,descriptor-dwords[[:space:]]*=[[:space:]]*<6>;' \
	'MW5 DTS selects six-DWORD descriptors'

require_grep "$TAGGER" 'RTL4_9_TX_PROTOCOL_VALUE[[:space:]]+0x9' \
	'new DSA tagger uses protocol 0x9'
require_grep "$TAGGER" 'htons\(ETH_P_REALTEK\)' \
	'new DSA tagger emits EtherType 0x8899'
require_grep "$TAGGER" 'u32[[:space:]]+port_mask;' \
	'TX tag stores the destination mask in a fixed-width type'
require_grep "$TAGGER" 'port_mask[[:space:]]*=[[:space:]]*BIT\(dp->index\);' \
	'TX tag derives the destination port mask once'
require_grep "$TAGGER" 'FIELD_PREP\(RTL4_9_TX_PORT,[[:space:]]*port_mask\)' \
	'TX tag uses the fixed-width destination port mask'
reject_grep "$TAGGER" 'RTL4_9_TX_LEARN_DIS|BIT\(9\)' \
	'protocol-9 four-byte TX tag carries no speculative learn-disable bit'
require_grep "$TAGGER" 'RTL4_9_TX_PORT[[:space:]]+GENMASK\(7,[[:space:]]*0\)' \
	'protocol-9 TX destination mask uses the rtl4a-compatible low byte'
require_grep "$TAGGER" 'dsa_master_find_slave\(master,[[:space:]]*0,[[:space:]]*encoded_port\)' \
	'RX accepts a direct source-port encoding'
require_grep "$TAGGER" 'source_port[[:space:]]*=[[:space:]]*__ffs\(encoded_port\)' \
	'RX also accepts a one-hot source-port mask'
require_grep "$TAGGER" 'DSA_TAG_PROTO_RTL4_9' \
	'tagger registers the new DSA protocol'
require_grep "$TAGGER" 'rtl4_9 v43\.12 RX unknown tag' \
	'tagger rate-limits unknown RX-tag diagnostics through its local counter'
reject_grep "$TAGGER" 'netdev_err_ratelimited' \
	'tagger does not use the unavailable netdev_err_ratelimited helper'
reject_grep "$TAGGER" 'mask=0x%x[^\n]*BIT\(dp->index\)' \
	'tagger does not pass unsigned long BIT() directly to %x'

require_grep "$PATCH" 'DSA_TAG_PROTO_RTL4_9_VALUE[[:space:]]+28' \
	'patch adds a unique protocol ID'
require_grep "$PATCH" 'config NET_DSA_TAG_RTL4_9' \
	'patch adds tagger Kconfig'
require_grep "$PATCH" 'tristate "Realtek RTL8365MB switch subdriver"' \
	'patch targets the Linux 6.6 Realtek subdriver Kconfig layout'
require_grep "$PATCH" 'imply NET_DSA_REALTEK_SMI' \
	'patch Kconfig context includes the Linux 6.6 SMI imply line'
require_grep "$PATCH" 'imply NET_DSA_REALTEK_MDIO' \
	'patch Kconfig context includes the Linux 6.6 MDIO imply line'
reject_grep "$PATCH" 'tristate "Realtek RTL8365MB switch driver"' \
	'patch no longer assumes the newer converted-driver Kconfig layout'
require_grep "$PATCH" 'obj-\$\(CONFIG_NET_DSA_TAG_RTL4_9\).*tag_rtl4_9\.o' \
	'patch adds tagger Makefile entry'
require_grep "$PATCH" 'case DSA_TAG_PROTO_RTL4_9:' \
	'RTL8365MB change_tag_protocol handles protocol 9'
require_grep "$PATCH" 'cpu->format[[:space:]]*=[[:space:]]*RTL8365MB_CPU_FORMAT_4BYTES' \
	'RTL8365MB is switched to four-byte CPU tags'
require_grep "$PATCH" 'realtek,cpu-tag-4bytes-proto9' \
	'RTL8365MB selection is limited by the MW5 DTS property'

require_grep "$NETWORK" 'tenda,nova-mw5\)' 'MW5 board cases are present'
require_grep "$NETWORK" 'ucidef_set_interfaces_lan_wan[[:space:]]+"\$lan_list"[[:space:]]+"\$wan_list"' \
	'MW5 uses normal LAN/WAN DSA interfaces'
require_grep "$NETWORK" 'wan_mac=\$\(macaddr_add[[:space:]]+"\$lan_mac"[[:space:]]+7\)' \
	'MW5 WAN MAC uses OEM base+7'
require_grep "$NETWORK" 'No target-wide bridge object' \
	'MW5 suppresses the global bridge MAC object'

if sh -n "$NETWORK"; then
	ok '02_network shell syntax is valid'
else
	bad '02_network shell syntax is invalid'
fi

# Check the exact MW5 topology block, not unrelated target-wide defaults.
MW5_BLOCK=$(sed -n '/^case \$board in$/,/^esac$/p' "$NETWORK" | sed -n '/^tenda,nova-mw5)/,/^[[:space:]]*;;/p' | head -30)
if printf '%s\n' "$MW5_BLOCK" | grep -q 'ucidef_set_bridge_device'; then
	bad 'MW5 topology block must not create a global switch bridge'
else
	ok 'MW5 topology block does not create a global switch bridge'
fi

require_grep "$MW5_NET_DEFAULTS" "add_list network.br_lan.ports='lan'" \
	'MW5 uci-defaults forces the physical LAN port into br-lan'
require_grep "$MW5_NET_DEFAULTS" "set network.lan.ipaddr='192.168.1.1'" \
	'MW5 uci-defaults assigns the recovery LAN address'
require_grep "$MW5_NET_DEFAULTS" "set network.wan.device='wan'" \
	'MW5 uci-defaults keeps WAN as a routed DSA port'
require_grep "$MW5_NET_DEFAULTS" "uci -q delete dhcp.lan.ignore" \
	'MW5 uci-defaults enables LAN DHCP'

printf '\nResult: %d PASS, %d FAIL\n' "$pass" "$fail"
[ "$fail" -eq 0 ] || exit 1
