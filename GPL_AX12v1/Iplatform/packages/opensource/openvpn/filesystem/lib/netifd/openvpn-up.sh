#!/bin/sh
PPP_IPPARAM="$6"
. /lib/netifd/netifd-proto.sh
#env | xargs echo > /dev/console
IFNAME=$1
IPLOCAL=$4
IPREMOTE=$5
proto_init_update "$IFNAME" 1 1
proto_set_keep 1
local dns1
local dns2
[ -n "$PPP_IPPARAM" ] && {
	[ -n "$IPLOCAL" ] && proto_add_ipv4_address "$IPLOCAL" 32
	[ -n "$IPREMOTE" ] && proto_add_ipv4_route_noextra "$IPREMOTE" 32 0.0.0.0
	[ -n "$foreign_option_1" ] && {
		local param="$foreign_option_1"
		local opt=${param% *}
		dns1=${param##* }
		[ "$opt" == "dhcp-option DNS" -a -n "$dns1" ] && proto_add_dns_server "$dns1"
	}
	[ -n "$foreign_option_2" ] && {
		local param="$foreign_option_2"
		local opt=${param% *}
		dns2=${param##* }
		[ "$opt" == "dhcp-option DNS" -a -n "$dns2" -a "$dns1" != "$dns2" ] && proto_add_dns_server "$dns2"
	}
}
proto_send_update "vpn" 

