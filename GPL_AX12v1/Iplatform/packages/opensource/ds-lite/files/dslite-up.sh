#!/bin/sh
. /lib/netifd/netifd-proto.sh

link=$1
mtu=$2
local_addr=$3
remote_addr=$4
tunlink=$5
cfg=$6

proto_init_update "$link" 1
proto_add_tunnel
json_add_string mode ipip6
json_add_int mtu "${mtu:-1280}"
json_add_int ttl "${ttl:-64}"
json_add_string local "$local_addr"
json_add_string remote "$remote_addr"
[ -n "$tunlink" ] && json_add_string link "$tunlink"
proto_close_tunnel
proto_send_update "$cfg"

proto_init_update "$link" 1
proto_set_keep 1
proto_add_ipv4_route "0.0.0.0" 0
proto_add_dns_server "8.8.8.8"
proto_add_dns_server "8.8.4.4"
proto_send_update "$cfg"
ifconfig $link mtu $mtu
#echo "[dslite-up.sh]:disable dslt-wan tso" > /dev/console
#ethtool -K $link tso off
#enable dslite hwnat
#MT762x
[ -x "/usr/bin/hw_nat" ] && hw_nat -W 0
#MT7986
[ -w "/sys/kernel/debug/hnat/mape_toggle" ] && echo '0'>/sys/kernel/debug/hnat/mape_toggle
