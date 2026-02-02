#!/bin/sh

[ -f /lib/vpn/vpn_fc.sh ] && . /lib/vpn/vpn_fc.sh

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

	proto_run_command "$config" /bin/nice -n -20 /usr/sbin/pppd \
		nodetach ifname "pptp-$config" \
		ipparam "$config" \
		${keepalive:+lcp-echo-interval $interval lcp-echo-failure ${keepalive%%[, ]*}} \
		${ipv6:++ipv6} \
		noaccomp nopcomp \
		$dnsarg \
		${ipaddr:+"$ipaddr:"} \
		$demand maxfail 1 \
		${username:+user "$username"} \
		${password:+password "$password"} \
		${connect:+connect "$connect"} \
		${disconnect:+disconnect "$disconnect"} \
		ip-up-script /lib/netifd/pppvpn-up \
		ip-down-script /lib/netifd/pppvpn-down \
		${mru:+mtu $mru mru $mru} \
		$pppd_options "$@"
}

ppp_generic_teardown() {
	local interface="$1"
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

proto_pptpvpn_init_config() {
	ppp_generic_init_config
	proto_config_add_string "server"
	available=1
	no_device=1
}

proto_pptpvpn_setup() {
	local config="$1"
	local iface="$2"
	local parent=""
	local encryption_type="0"
	local up_flag=""

	config_load vpn
	config_get enabled client enabled
	if [ $enabled != "on" ]; then
		echo "vpn client is off, exit" > /dev/console
		exit 0
	fi

	config_load network
	config_get parent $config parent
	config_get encryption_type $config encryption
	
	while true; do
		config_get iface $parent ifname
		config_get up_flag $parent up
		if [ -n "$iface" ] && [ -n "$up_flag" ];then
			break
		fi
		echo "$0 iface=?, wait 10 second" > /dev/console
		sleep 10
		config_load network
		config_get parent $config parent
		config_get encryption_type $config encryption
	done

	local ip serv_addr server

#json_get_var server server && {
	config_load protocol
	config_get server pptpvpn server && {
	        for ip in $(resolveip -4 -t 10 "$server"); do
	             ( proto_add_host_dependency "$config" "$ip" "$parent" ) # add pptp server host route.
	              serv_addr=1
	        done
	}

#serv_addr=1
	[ -n "$serv_addr" ] || {
		echo "Could not resolve server address" > /dev/console
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
	
	if [ -d /proc/fcache/ ] ; then
		update_pptp_fc_gre_status
	fi	

	if [ "$encryption_type" == "max" ]; then
		ppp_generic_setup "$config" \
			pptp pptp_server $server \
			default-asyncmap nobsdcomp \
			nodeflate novj mppe required,no40,no56,stateless refuse-eap \
			nic-ifname $iface
	elif [ "$encryption_type" == "auto" ]; then
		ppp_generic_setup "$config" \
			pptp pptp_server $server \
			default-asyncmap nobsdcomp \
			nodeflate novj mppe no40,no56,stateless refuse-eap \
			nic-ifname $iface
	elif [ "$encryption_type" == "none" ]; then
		ppp_generic_setup "$config" \
			pptp pptp_server $server \
			default-asyncmap nobsdcomp \
			nodeflate novj refuse-eap \
			nic-ifname $iface
	else
		#error in conf, make life struggling
		ppp_generic_setup "$config" \
			pptp pptp_server $server \
			default-asyncmap nobsdcomp \
			nodeflate novj mppe required,no40,no56,stateless refuse-eap \
			nic-ifname $iface
	fi
}

proto_pptpvpn_teardown() {
	if [ -d /proc/fcache/ ] ; then
		update_pptp_fc_gre_status
	fi	
	ppp_generic_teardown "$@"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol pptpvpn
}

