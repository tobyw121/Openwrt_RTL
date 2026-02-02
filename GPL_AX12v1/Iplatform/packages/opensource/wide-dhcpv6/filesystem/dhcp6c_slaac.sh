#!/bin/sh
. /lib/functions.sh
. /lib/netifd/netifd-proto.sh
. /usr/share/libubox/jshn.sh

config_load /etc/config/network
config_get wan_type wan wan_type

if [ "$wan_type" == "pppoeshare" ]; then
	config_get ip_config wan ip_config
	json_load "`ubus call network.interface.internet status`"
	json_get_var ifname "l3_device"
else
	config_get ip_config wanv6 ip_config
	json_load "`ubus call network.interface.wanv6 status`"
	json_get_var ifname "l3_device"
fi

mbit=`cat /proc/sys/net/ipv6/conf/$ifname/ndisc_mbit`

[ $mbit -eq 0 -a "$ip_config" == "auto" ] || [ "$ip_config" == "slaac" ] && {
		old_ip6addr=`cat /tmp/slaac_addr`
		new_ip6addr=`cat /proc/sys/net/ipv6/conf/$ifname/slaac_addr`

		if [ -n "$old_ip6addr" -a -n "$new_ip6addr" -a "$old_ip6addr" != "$new_ip6addr" ]; then
				echo "old_ip6addr: $old_ip6addr;    new_ip6addr: $new_ip6addr" > /dev/console
				echo SLAAC PREFIX change !!!!! > /dev/console
				echo $new_ip6addr > /tmp/slaac_addr
				
				if [ "$wan_type" == "pppoeshare" ]; then
					ubus call network.interface.internet down
					ubus call network.interface.internet up
				else
					ubus call network.interface.wanv6 down
					ubus call network.interface.wanv6 up
				fi
		fi
}

exit 0