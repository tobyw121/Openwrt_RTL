#!/bin/sh
# Temporary on-device validation for MW5 v43.2.
# It rewrites the intended OpenWrt DSA topology and clears RTL8197F
# P0 CFG_TX_CPUC_TAG (bit 26) after netifd has reopened the port.
# All changes are lost after reboot when the device uses a tmpfs overlay.

set -eu

case "$(cat /tmp/sysinfo/board_name 2>/dev/null || true)" in
	tenda,nova-mw5|*nova-mw5*) ;;
	*) echo "WARNING: board_name is not tenda,nova-mw5" >&2 ;;
esac

while uci -q delete network.@device[0]; do :; done

uci -q batch <<'UCI'
delete network.br_lan
delete network.lan
delete network.wan_dev
delete network.wan
delete network.wan6

set network.br_lan='device'
set network.br_lan.name='br-lan'
set network.br_lan.type='bridge'
set network.br_lan.stp='0'
set network.br_lan.igmp_snooping='0'
add_list network.br_lan.ports='lan'

set network.lan='interface'
set network.lan.device='br-lan'
set network.lan.proto='static'
set network.lan.ipaddr='192.168.1.1'
set network.lan.netmask='255.255.255.0'
set network.lan.ip6assign='60'

set network.wan_dev='device'
set network.wan_dev.name='wan'
set network.wan_dev.ipv6='1'

set network.wan='interface'
set network.wan.device='wan'
set network.wan.proto='dhcp'

set network.wan6='interface'
set network.wan6.device='@wan'
set network.wan6.proto='dhcpv6'
UCI
uci -q commit network

uci -q set dhcp.lan='dhcp'
uci -q set dhcp.lan.interface='lan'
uci -q set dhcp.lan.start='100'
uci -q set dhcp.lan.limit='150'
uci -q set dhcp.lan.leasetime='12h'
uci -q delete dhcp.lan.ignore
uci -q commit dhcp

/etc/init.d/network restart
/etc/init.d/dnsmasq restart
sleep 6

# SWCORE_BASE 0x1b800000 + P0GMIICR 0x414c.
P0GMIICR=0x1b80414c
if command -v devmem >/dev/null 2>&1; then
	old="$(devmem "$P0GMIICR" 32)"
	new="$(printf '0x%08x' $((old & ~0x04000000)))"
	devmem "$P0GMIICR" 32 "$new" >/dev/null
	readback="$(devmem "$P0GMIICR" 32)"
	echo "P0GMIICR: $old -> $readback (TX CPU-tag bit cleared)"
else
	echo "devmem is unavailable; the v43.2 TX-tag bit could not be changed" >&2
fi

ip link set dev lan up 2>/dev/null || true
ip link set dev br-lan up 2>/dev/null || true

echo
echo '--- UCI network ---'
uci show network
echo
echo '--- link state ---'
ip -d link show dev lan 2>/dev/null || true
ip -d link show dev br-lan 2>/dev/null || true
echo
echo '--- bridge state ---'
bridge link show dev lan 2>/dev/null || true
echo
echo '--- interface status ---'
ubus call network.interface.lan status 2>/dev/null || true
echo
echo '--- counters ---'
cat /proc/net/dev
