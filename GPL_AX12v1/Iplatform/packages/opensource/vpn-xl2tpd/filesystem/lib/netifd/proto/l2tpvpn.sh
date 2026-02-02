#!/bin/sh
[ -x /usr/sbin/xl2tpd ] || exit 0

conffile="/tmp/l2tpvpn/l2tpvpn.conf"
controlfile="/tmp/l2tpvpn/l2tpvpn-control"
pidfile="/tmp/l2tpvpn/l2tpvpn.pid"
l2tpdir="/tmp/l2tpvpn"
ipsec_conf="/etc/ipsec.conf"
ipsec_conf_common="/etc/ipsec.conf.common"
ipsec_conf_temp="/etc/ipsec.conf.client"
ipsec_conf_vpns="/etc/ipsec.conf.server"
ipsec_secrets="/etc/ipsec.secrets"
ipsec_secrets_temp="/etc/ipsec.secrets.client"
ipsec_secrets_vpns="/etc/ipsec.secrets.server"

localport=1702

L2TP_VPN_SERVER_OPEN=no
L2TP_VPN_SERVER_RUNNING=no

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}


get_l2tp_vpn_server_status() {
	config_load  l2tpoveripsec
	config_get enabled l2tpoveripsec enabled

	pid=$(pidof charon)

	if [ $enabled = "on" ]; then
		L2TP_VPN_SERVER_OPEN=yes
		if [ ! -z $pid ]; then
			L2TP_VPN_SERVER_RUNNING=yes
		fi
	fi
}

proto_l2tpvpn_init_config() {
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "keepalive"
	proto_config_add_string "pppd_options"
	proto_config_add_boolean "ipv6"
	proto_config_add_int "mru"
	proto_config_add_int "demand"
	proto_config_add_string "server"
	available=1
	no_device=1
}

ipsec_init_config_in_tear_down(){
	cp ${ipsec_conf_common} ${ipsec_conf}
	cat ${ipsec_conf_vpns} >> ${ipsec_conf}

	cat ${ipsec_secrets_vpns} > ${ipsec_secrets}
}

ipsec_init_config() {
	local psk
	local localip
	local serverip="$1"
	local nexthop
	local iface
	local parent

	config_load network
	config_get psk vpn psk
	config_get parent vpn parent
	
	if [ -z "$serverip" ]; then
		echo "serverip= ?" > /dev/console
		return 1
	fi

	if [ ! $parent ]; then
		echo "parent= ?" > /dev/console
		return 1
	fi
	
	while true; do
		config_load network
		config_get psk vpn psk
		config_get parent vpn parent
		json_load "`ubus call network.interface.${parent} status`"
		json_get_var iface l3_device
		json_select ipv4-address
		json_select 1
		json_get_var localip address
		json_load "`ubus call network.interface.${parent} status`"
		json_select route
		json_select 1
		json_get_var nexthop nexthop

		if [ ! $iface ]; then
			echo "iface= ?, wait 5s" > /dev/console
			sleep 5
			continue
			#return 1
		fi
		# dslite: wan has no ipv4 address
		if [ "$iface" = "dslt-wan" ]; then
			json_load "`ubus call network.interface.lan status`"
			json_select ipv4-address
			json_select 1
			json_get_var localip address
		fi
		if [ ! $localip ]; then
			echo "localip= ?, wait 5s" > /dev/console
			sleep 5
			continue
			#return 1
		fi
		break
	done
	#record serverip
	uci set vpn.client.ipsecServerIP=$serverip
	uci commit

	#cp $CONFIG_TEMP $CONFIG
	#cp $SECRETS_TEMP $SECRETS
	
	: > ${ipsec_conf_temp}
#	echo "# /etc/ipsec.conf - IPsec configuration file" >> ${ipsec_conf_temp}
#	echo "# Include non-UCI connections here" >> ${ipsec_conf_temp}
#	echo "# They will be preserved across restarts/upgrades" >> ${ipsec_conf_temp}
#	echo "	" >> ${ipsec_conf_temp}
#	echo "config setup" >> ${ipsec_conf_temp}
#	echo "	charondebug=\"ike 4, knl 1, cfg 4\"" >> ${ipsec_conf_temp}
#	echo "	" >> ${ipsec_conf_temp}
#	echo "conn %default" >> ${ipsec_conf_temp}
#	echo "	ikelifetime=60m" >> ${ipsec_conf_temp}
#	echo "	keylife=20m" >> ${ipsec_conf_temp}
#	echo "	rekeymargin=3m" >> ${ipsec_conf_temp}
#	echo "	keyingtries=1" >> ${ipsec_conf_temp}
#	echo "	keyexchange=ikev1" >> ${ipsec_conf_temp}
#	echo "	authby=secret" >> ${ipsec_conf_temp}
	echo "conn L2TP-VPNC-PSK" >> ${ipsec_conf_temp}
	echo "	keyingtries=%forever" >> ${ipsec_conf_temp}
	echo "	auto=add" >> ${ipsec_conf_temp}
	echo "	type=transport" >> ${ipsec_conf_temp}
	echo "	left=$localip" >> ${ipsec_conf_temp}
	echo "	leftprotoport=17/$localport" >> ${ipsec_conf_temp}
	echo "	leftfirewall=yes" >> ${ipsec_conf_temp}
	echo "	right=$serverip" >> ${ipsec_conf_temp}
	echo "	rightprotoport=17/1701" >> ${ipsec_conf_temp}
#echo "	ike=3des-sha1-modp1024,aes-sha1-modp2048,aes-sha256-modp2048,aes-sha384-modp2048,aes-sha512-modp2048,aes-sha1-modp3072,aes-sha256-modp3072,aes-sha384-modp3072,aes-sha512-modp3072!"  >> ${ipsec_conf_temp}
#echo "	esp=3des-sha1,aes-sha1,aes-sha256,aes-sha384,aes-sha512,aes-sha1,aes-sha256,aes-sha384,aes-sha512!"  >> ${ipsec_conf_temp}
	echo "	ike=aes-sha1-modp2048,aes-sha256-modp2048,aes-sha384-modp2048,aes-sha512-modp2048,aes-sha1-modp3072,aes-sha256-modp3072,aes-sha384-modp3072,aes-sha512-modp3072!"  >> ${ipsec_conf_temp}
	echo "	esp=aes-sha1,aes-sha256,aes-sha384,aes-sha512,aes-sha1,aes-sha256,aes-sha384,aes-sha512!"  >> ${ipsec_conf_temp}
#echo "	ike=3des-md5-modp1024,3des-sha1-modp1024" >> ${ipsec_conf_temp}
#echo "	esp=3des-md5,3des-sha1,des-md5,des-sha1" >> ${ipsec_conf_temp}
	echo "	dpddelay=5" >> ${ipsec_conf_temp}
	echo "	dpdtimeout=20" >> ${ipsec_conf_temp}
	echo "	" >> ${ipsec_conf_temp}

	cp ${ipsec_conf_common} ${ipsec_conf}
	cat ${ipsec_conf_temp} >> ${ipsec_conf}

	: > ${ipsec_secrets_temp}
	echo "$localip $serverip : PSK \"$psk\"" >> ${ipsec_secrets_temp}
	cp ${ipsec_secrets_temp} ${ipsec_secrets}


	if [ $L2TP_VPN_SERVER_RUNNING = "yes" ]; then
		echo "l2tpvpn server is running. append its conf" > /dev/console
		cat ${ipsec_conf_vpns} >> ${ipsec_conf}
		cat ${ipsec_secrets_vpns} >> ${ipsec_secrets}
	else
		echo "l2tpvpn server not running. drop its conf" > /dev/console
	fi

	return 0
}


proto_l2tpvpn_setup() {
	local config="$1"
	local iface="$2"
	local parent=""
	local optfile="/tmp/l2tpvpn/options.${config}"
	local psk
	local server
	local server_ip
	local ipsec
	local model=$(uci get profile.@global[0].model -c /etc/profile.d -q)
	json_get_var server server

	mkdir -p /tmp/l2tpvpn
	config_load vpn
	config_get ipsec client ipsec
	config_get enabled client enabled
	
	if [ $enabled != "on" ]; then
		echo "vpn client is off, exit" > /dev/console
		exit 1
	fi
	config_load network
	config_get psk $config psk
	config_get parent $config parent
	config_get iface $parent ifname

	echo proto_l2tpvpn_setup parent_iface $iface > /dev/console
	#json_get_var server server && {
	config_get server $config server && {
		for ip in $(resolveip -4 -t 10 "$server"); do
			( proto_add_host_dependency "$config" "$ip" "$parent" )
			echo "$ip" >> /tmp/server.l2tp-${config}
			serv_addr=1
			# if resolveip get two address, only set the first one to server_ip
			[ -z "$server_ip" ] && server_ip=${ip}
		done
	}
	echo "l2tpvpn $server $iface $psk $parent $ipsec" > /dev/console

	[ -n "$serv_addr" ] || {
		echo "Could not resolve server address" > /dev/console
		sleep 5
		proto_setup_failed "$config"
		exit 1
	}

#	if [ ! -p /var/run/xl2tpd/l2tp-control ]; then
#		/etc/init.d/xl2tpd start
#	fi

	json_get_vars ipv6 demand keepalive username password pppd_options
	[ "$ipv6" = 1 ] || ipv6=""
#	if [ "${demand:-0}" -gt 0 ]; then
#		demand="precompiled-active-filter /etc/ppp/filter demand idle $demand"
#	else
		demand="persist"
#	fi

	[ -n "$mru" ] || json_get_var mru mru
	
#	local interval="${keepalive##*[, ]}"
#	[ "$interval" != "$keepalive" ] || interval=5


	echo ipsec $ipsec > /dev/console
	if [ "$ipsec" = "1" ]; then	
		#/etc/init.d/ipsec stop
		if [ "$psk" != "" ]; then
			get_l2tp_vpn_server_status
			ipsec_init_config "$server_ip"
			if [ $? -neq 0 ]; then 
				echo "ipsec_init_config error" > /dev/console
				exit 1
			fi

			if [ $L2TP_VPN_SERVER_RUNNING = "yes" ]; then
				echo "setup:update ipsec in client" > /dev/console
				ipsec reload
				ipsec update
                ipsec rereadsecrets
			else
				echo "setup:start ipsec in client" > /dev/console
				ipsec start
				[ "$model" = "RTL_8197" ] && sysctl -w net.core.dev_weight=16
			fi
		fi
	else
		echo "FBI warning: ipsec == 0. check it!" >  /dev/console
	fi
		
	# make sure pppd killed before setup (Bug 73649) 
	#ps | grep -v grep | grep pppd | grep l2tp | sed 's/  */ /g' | grep -o '[^ ].*' | cut -d " " -f1 | xargs kill -9

	#sleep 2
	if [ "$psk" != "" -a "$ipsec" = "1" ]; then
		if [ $L2TP_VPN_SERVER_RUNNING != "yes" ]; then
			# check ipsec startup status once per second for a maximum of 10 seconds
			for i in `seq 10`; do
				sleep 1
				if [ -n "$(ipsec status)" ]; then
					echo "ipsec startup finished" > /dev/console
					break
				fi
			done
		fi
		echo "start ipsec connection" > /dev/console
		ipsec stroke up-nb L2TP-VPNC-PSK
	fi
}

proto_l2tpvpn_teardown() {
	local interface="$1"
	local optfile="/tmp/l2tpvpn/options.${interface}"
	local model=$(uci get profile.@global[0].model -c /etc/profile.d -q)

	rm -f /tmp/server.l2tp-${interface}
	echo "no_server" > /tmp/connecterror
	case "$ERROR" in
		1)
			echo "auth_failed" > /tmp/connecterror
		;;
		11|19)
			proto_notify_error "$interface" AUTH_FAILED
			proto_block_restart "$interface"
		;;
		2)
			proto_notify_error "$interface" INVALID_OPTIONS
			proto_block_restart "$interface"
		;;
	esac

#	xl2tpd-control disconnect l2tp-${interface}
	# Wait for interface to go down
#       while [ -d /sys/class/net/l2tp-${interface} ]; do
#		sleep 1
#	done

#	xl2tpd-control remove l2tp-${interface}
#rm -f ${optfile}
	ipsec stroke down-nb L2TP-VPNC-PSK

	#decide whether to kill or update ipsec

	get_l2tp_vpn_server_status

	if [ $L2TP_VPN_SERVER_OPEN = "yes" ]; then
		echo "teardown: l2tpvpn server is open, update ipsec" > /dev/console
		ipsec_init_config_in_tear_down
		ipsec reload
		ipsec update
        ipsec rereadsecrets
	else
		echo "teardown: l2tpoveripsec server not open, stop ipsec" > /dev/console
		: > ${ipsec_conf}
		: > ${ipsec_secrets}
		ipsec stop
		[ "$model" = "RTL_8197" ] && sysctl -w net.core.dev_weight=64
	fi
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol l2tpvpn
}
