#!/bin/sh

. /lib/functions.sh

IPSEC_CLIENT_TUNNEL=L2TP-VPNC-PSK
DORMANT_TIME=60
CONSOLE=/dev/console
ipmark="peer-ip:"
connmark="ESTABLISHED"
connection="L2TP-VPNC-PSK"


while true; do

	sleep $DORMANT_TIME

	config_load vpn
	config_get enabled client enabled
	config_get vpntype client vpntype
	config_get ipsec client ipsec
	config_get ipsecServerIP client ipsecServerIP

	if [ "$enabled" != "on" -o "$vpntype" != "l2tpvpn" -o "$ipsec" != "1" ]; then
		echo "l2tp/ipsec vpn client not running" > $CONSOLE
		continue;
	fi

#config_load network

	status=$(lua -e 'inetm = require "luci.model.internet"; inet=inetm.Internet(); print(inet:conn_state("vpn"))')

	if [ "$status" != "connected" ];then
		ipsec_info=$(ipsec status $connection|grep $connmark|grep $ipsecServerIP)
		words=$(expr length "$ipsec_info")
		echo $0 ="$ipsec_info"= ="$words"= > $CONSOLE

		#ugly
		if [ $words -eq 0 -o "$word" = "0" ]; then
			echo "$ipsecServerIP haven't got ipsec tunnel with us, disconnect" > $CONSOLE
			echo "manually bring up tunnel $IPSEC_CLIENT_TUNNEL" > $CONSOLE
			ipsec stroke up-nb $IPSEC_CLIENT_TUNNEL
		else
			echo "$ipsecServerIP have got ipsec tunnel with us" ="$ipsec_info"=  ="$words"= > $CONSOLE
			echo "redial l2tp connection" > $CONSOLE
			source /etc/hotplug.d/ipsec/01-user && start_xl2tpd_client
		fi
	fi
done
