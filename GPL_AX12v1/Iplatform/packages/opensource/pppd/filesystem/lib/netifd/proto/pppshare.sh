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

pppshare_generic_init_config() {
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "keepalive"
	proto_config_add_string "pppd_options"
	proto_config_add_boolean "authfail"
	proto_config_add_int "mru"
	proto_config_add_boolean "ipv4"
	proto_config_add_string "lanif"
	proto_config_add_string "ip_mode"
	proto_config_add_string "ipaddr"
}

pppshare_generic_setup() {
	local config="$1"; shift
	json_get_vars ipv4 keepalive username password pppd_options ip_mode ip_config dns_mode ifname
	
	echo pppshare setup begin: $ipv4 : $keepalive : $username : $password : $pppd_options : $ifname : $config > /dev/console

	[ -n "$mru" ] || json_get_var mru mru

	local ipv4arg=""
	[ "$ipv4" == "1" ] && ipv4arg="ipv4"

	local interval="${keepalive##*[, ]}"
	[ "$interval" != "$keepalive" ] || interval=5

#	[ "$ip_config" == "specified" ] && json_get_var ipaddr ipaddr
	[ "$ip_mode" == "static" ] && json_get_var ipaddr ipaddr
	
	local dnsarg="usepeerdns"
	[ "$dns_mode" == "static" ] && dnsarg=""

	echo "0" > /proc/sys/net/ipv6/conf/$ifname/accept_ra

	local model=$(uci get profile.@global[0].model -c /etc/profile.d -q)
	if [ "$model" = "RTL_8197" ]; then
		# change ppp device name to share-internet
		echo "3" > /proc/rtl865x/sw_netif

		# set rtl8197 ipv6 acl rules
		proto_setup_ipv6_acl
	fi

	proto_run_command "$config" /usr/sbin/pppd \
		nodetach ifname "share-$config" \
		ipparam "$config" \
		${keepalive:+lcp-echo-interval $interval lcp-echo-failure ${keepalive%%[, ]*}} \
		defaultroute noaccomp nopcomp ipv6 \
		${dnsarg:+"$dnsarg"} \
		${ipv4arg:+"$ipv4arg"} \
		${ipaddr:+"$ipaddr:"} \
		${username:+user "$username"} \
		${password:+password "$password"} \
		ip-up-script /lib/netifd/ppp-up \
		ipv6-up-script /lib/netifd/pppshare-up \
		ip-down-script /lib/netifd/ppp-down \
		ipv6-down-script /lib/netifd/ppp-down \
		${mru:+mtu $mru mru $mru} \
		$pppd_options "$@"
}

pppshare_generic_teardown() {
	local interface="$1"
	local ifname="$2"
	local dhcp6cdir="/tmp/dhcp6c"
#	local pidfile="$dhcp6cdir/dhcp6c.pid"
	echo pppshare teardown : $interface > /dev/console
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

proto_pppoeshare_init_config() {
	pppshare_generic_init_config
	proto_config_add_string "ac"
	proto_config_add_string "service"
	proto_config_add_string "dns"
	proto_config_add_string "ip_mode"
	proto_config_add_string "ip_config"
	proto_config_add_string "dns_mode"
	proto_config_add_string "ipaddr"
	proto_config_add_string "ip6addr"
}

proto_pppoeshare_setup() {
	local config="$1"
	local iface="$2"

	for module in slhc ppp_generic pppox pppoe; do
		/sbin/insmod $module 2>&- >&-
	done

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
	fi

	pppshare_generic_setup "$config" \
		pppoe unit 1  \
		${ac:+rp_pppoe_ac "$ac"} \
		${service:+rp_pppoe_service "$service"} \
		"nic-$iface" 
}

proto_pppoeshare_teardown() {
	pppshare_generic_teardown "$@"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol pppoeshare
}
