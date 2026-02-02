#!/bin/sh

[ -x /usr/sbin/pppd ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

ppp_generic_init_config() {
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "keepalive"
	proto_config_add_int "demand"
	proto_config_add_string "pppd_options"
	proto_config_add_string "connect"
	proto_config_add_string "disconnect"
	proto_config_add_boolean "ipv6"
	proto_config_add_boolean "authfail"
	proto_config_add_int "mru"
	proto_config_add_string "ipaddr"
	proto_config_add_string "conn_mode"
}

ppp_generic_setup() {
	local config="$1"; shift

	json_get_vars ipv6 demand keepalive username password pppd_options ip_mode dns_mode 
	[ "$ipv6" = 1 ] || ipv6=""

	demand=""
#	if [ "${demand:-0}" -gt 0 ]; then
#		demand="precompiled-active-filter /etc/ppp/filter demand idle $demand"
#	else
#		demand="persist"
#	fi

	[ -n "$mru" ] || json_get_var mru mru

	local interval="${keepalive##*[, ]}"
	[ "$interval" != "$keepalive" ] || interval=5
	[ -n "$connect" ] || json_get_var connect connect
	[ -n "$disconnect" ] || json_get_var disconnect disconnect

	[ "$ip_mode" == "static" ] && json_get_var ipaddr ipaddr
	
	
	local dnsarg="usepeerdns"

	[ "$dns_mode" == "static" ] && dnsarg=""


	proto_run_command "$config" /usr/sbin/pppd \
		nodetach ifname "${proto:-ppp}-$config" \
		ipparam "$config" \
		${keepalive:+lcp-echo-interval $interval lcp-echo-failure ${keepalive%%[, ]*}} \
		${ipv6:++ipv6} \
		defaultroute noaccomp nopcomp \
		$dnsarg \
		${ipaddr:+"$ipaddr:"} \
		$demand maxfail 1 \
		${username:+user "$username"} \
		${password:+password "$password"} \
		${connect:+connect "$connect"} \
		${disconnect:+disconnect "$disconnect"} \
		ip-up-script /lib/netifd/ppp-up \
		ipv6-up-script /lib/netifd/ppp-up \
		ip-down-script /lib/netifd/ppp-down \
		ipv6-down-script /lib/netifd/ppp-down \
		${mru:+mtu $mru mru $mru} \
		$pppd_options "$@"
}

ppp_generic_teardown() {
	local interface="$1"
	local pptpfile="/tmp/server.pptp-${interface}"
	[ -f $pptpfile ] && rm -f $pptpfile
	echo "no_server" > /tmp/connecterror
	case "$ERROR" in
		11|19)
			proto_notify_error "$interface" AUTH_FAILED
			json_get_var authfail authfail
			if [ "${authfail:-0}" -gt 0 ]; then
				proto_block_restart "$interface"
			fi
			echo "auth_failed" > /tmp/connecterror
		;;
		2)
			proto_notify_error "$interface" INVALID_OPTIONS
			proto_block_restart "$interface"
		;;
	esac
	proto_kill_command "$interface" 15
}

# PPP on serial device

proto_ppp_init_config() {
	proto_config_add_string "device"
	ppp_generic_init_config
	no_device=1
	available=1
}

proto_ppp_setup() {
	local config="$1"

	json_get_var device device
	ppp_generic_setup "$config" "$device"
}

proto_ppp_teardown() {
	ppp_generic_teardown "$@"
}

proto_pppoe_init_config() {
	ppp_generic_init_config
	proto_config_add_string "ac"
	proto_config_add_string "service"
	proto_config_add_string "dns"
	proto_config_add_string "ip_mode"
	proto_config_add_string "dns_mode"
}

proto_pppoe_setup() {
	local config="$1"
	local iface="$2"

	for module in slhc ppp_generic pppox pppoe; do
		/sbin/insmod $module 2>&- >&-
	done

#	json_get_var mtu mtu
#	mtu="${mtu:-1492}"

	json_get_var ac ac
	json_get_var service service

	local model=$(uci get profile.@global[0].model -c /etc/profile.d -q)
	if [ "$model" = "RTL_8197" ]; then
		# disable fast l2tp
		echo "0" > /proc/fast_l2tp
		# diasble fast pptp
		echo "0" > /proc/fast_pptp
		# disable hw_nat of pptp/l2tp second-connection
		echo "8" > /proc/hw_nat
		# change ppp device name to pppoe-internet
		echo "0" > /proc/rtl865x/sw_netif
	fi

	ppp_generic_setup "$config"   \
		pppoe unit 0 \
		${ac:+rp_pppoe_ac "$ac"}  \
		${service:+rp_pppoe_service "$service"} \
		"nic-$iface"
}

proto_pppoe_teardown() {
	ppp_generic_teardown "$@"
}

proto_pptp_init_config() {
	ppp_generic_init_config
	proto_config_add_string "server"
	available=1
	no_device=1
}

proto_pptp_setup() {
	local config="$1"
	local iface="$2"
	local parent=""

	config_load network
	config_get parent $config parent
	config_get iface $parent ifname

	local model=$(uci get profile.@global[0].model -c /etc/profile.d -q)
	if [ "$model" = "RTL_8197" ]; then
		# disable fast l2tp
		echo "0" > /proc/fast_l2tp
		# enable fast pptp
		echo "1" > /proc/fast_pptp
		# enable hw_nat of pptp second-connection
		echo "84" > /proc/hw_nat
	fi

	#add for iptv bug when iptv mod did not write the real ifname of wan for us.
	[ -f /tmp/iptv_wan_iface ] && iface=`cat /tmp/iptv_wan_iface`

	local ip serv_addr server
	json_get_var server server && {
	        for ip in $(resolveip -4 -t 8 "$server"); do
	             ( proto_add_host_dependency "$config" "$ip" "$parent" ) # add pptp server host route.
	             echo "$ip" >> /tmp/server.pptp-${config}
	              serv_addr=1
	        done
	}

	# serv_addr=1
	[ -n "$serv_addr" ] || {
		echo "Could not resolve server address"
		sleep 5
		proto_setup_failed "$config"
		exit 1
	}

	local load
	for module in slhc ppp_generic ppp_async ppp_mppe ip_gre gre pptp; do
		grep -q "$module" /proc/modules && continue
		/sbin/insmod $module 2>&- >&-
		load=1
	done
	[ "$load" = "1" ] && sleep 1

	ppp_generic_setup "$config" \
		pptp pptp_server $server \
		default-asyncmap nobsdcomp \
		nodeflate noccp novj refuse-eap \
		nic-ifname $iface
	#Control PPTP dialing hardware acceleration
	[ -e /dev/hwnat0 ] && hw_nat -R 1
}

proto_pptp_teardown() {
	ppp_generic_teardown "$@"
	#Control PPTP dialing hardware acceleration
	[ -e /dev/hwnat0 ] && hw_nat -R 0
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol pppoe
	add_protocol pptp
	add_protocol ppp
}

