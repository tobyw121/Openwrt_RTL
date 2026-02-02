#!/bin/sh

[ -x /usr/sbin/pppd ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh

	config_load /etc/config/sysmode
	config_get mode sysmode mode "router"
	[ "$mode" = "ap" ] && exit 0

	init_proto "$@"
}

pppv6_generic_init_config() {
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "keepalive"
	proto_config_add_string "pppd_options"
	proto_config_add_boolean "authfail"
	proto_config_add_int "mru"
	proto_config_add_boolean "ipv4"
	proto_config_add_string "lanif"
}

pppv6_generic_setup() {
	local config="$1"; shift


	json_get_vars ipv4 keepalive username password pppd_options ip_mode ip_config dns_mode ifname
	[ -n "$mru" ] || json_get_var mru mru

	local ipv4arg=""
	[ "$ipv4" == "1" ] && ipv4arg="ipv4"

	local interval="${keepalive##*[, ]}"
	[ "$interval" != "$keepalive" ] || interval=5

	[ "$ip_config" == "specified" ] && json_get_var ipaddr ipaddr	
	local dnsarg=""
#	[ "$dns_mode" == "static" ] && dnsarg=""

	echo "0" > /proc/sys/net/ipv6/conf/$ifname/accept_ra

	local model=$(uci get profile.@global[0].model -c /etc/profile.d -q)
	if [ "$model" = "RTL_8197" ]; then
		# set rtl8197 ipv6 acl rules
		proto_setup_ipv6_acl
	fi

	proto_run_command "$config" /usr/sbin/pppd \
		nodetach ifname "${proto:-ppp}-$config" \
		ipparam "$config" \
		${keepalive:+lcp-echo-interval $interval lcp-echo-failure ${keepalive%%[, ]*}} \
		defaultroute noaccomp nopcomp noip ipv6 \
		${dnsarg:+"$dnsarg"} \
		${ipv4arg:+"$ipv4arg"} \
		maxfail 1 ${ipaddr:+"$ipaddr:"} \
		${username:+user "$username"} \
		${password:+password "$password"} \
		ip-up-script /lib/netifd/pppv6-up \
		ipv6-up-script /lib/netifd/pppv6-up \
		ip-down-script /lib/netifd/ppp-down \
		ipv6-down-script /lib/netifd/ppp-down \
		${mru:+mtu $mru mru $mru} \
		$pppd_options "$@"
}

pppv6_generic_teardown() {
	local interface="$1"
	local ifname="$2"
	local dhcp6cdir="/tmp/dhcp6c"
#	local pidfile="$dhcp6cdir/dhcp6c.pid"

	case "$ERROR" in
		11|19)
			proto_notify_error "$interface" AUTH_FAILED
			json_get_var authfail authfail
			if [ "${authfail:-0}" -gt 0 ]; then
				proto_block_restart "$interface"
			fi
		;;
		2)
			proto_notify_error "$interface" INVALID_OPTIONS
			proto_block_restart "$interface"
		;;
	esac
	
	crontab -l | grep -v "/lib/netifd/dhcp6c_slaac.sh" | crontab -
	kill -9 `ps |grep dhcp6c_slaac.sh |grep -v grep | awk '{print $1}'`
	rm /tmp/slaac_addr
	
	[ -d "$dhcp6cdir" ] && {
		killall dhcp6c
		sleep 1
		local count=`ps | grep dhcp6c | grep -v "grep" | wc -l`
        if [ "0" != "$count" ];then                        
            killall -9 dhcp6c && rm -rf $dhcp6cdir
        fi    
	}
	sleep 1	
	proto_kill_command "$interface" 15
}

proto_pppoev6_init_config() {
	pppv6_generic_init_config
	proto_config_add_string "ac"
	proto_config_add_string "service"
	proto_config_add_string "dns"
	proto_config_add_string "ip_mode"
	proto_config_add_string "ip_config"
	proto_config_add_string "dns_mode"
	proto_config_add_string "ipaddr"
	proto_config_add_string "ip6addr"
}

proto_pppoev6_setup() {
	local config="$1"
	local iface="$2"

	for module in slhc ppp_generic pppox pppoe; do
		/sbin/insmod $module 2>&- >&-
	done

	json_get_var ac ac
	json_get_var service service

	pppv6_generic_setup "$config" \
		pppoe unit 1 "nic-$iface" \
		${ac:+rp_pppoe_ac "$ac"} \
		${service:+rp_pppoe_service "$service"}
}

proto_pppoev6_teardown() {
	pppv6_generic_teardown "$@"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol pppoev6
}
