#!/bin/sh
. /lib/netifd/netifd-proto.sh
IFNAME=$1
proto_init_update "$IFNAME" 0
proto_send_update "vpn"

