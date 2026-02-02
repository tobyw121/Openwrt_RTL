#!/bin/sh
# dslite.sh - IPv4-in-IPv6 tunnel backend
# Copyright (c) 2013 OpenWrt.org

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/network.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

proto_dslite_setup() {
	local cfg="$1"
	local iface="$2"
	local link="dslt-$cfg"
	local tunlink="wanv6"
	local count=0
	local flag=0
	local addr_temp
	#local STDOUT=/dev/console
	local STDOUT=/dev/null

	local mtu ttl peeraddr ip6addr dns_server dns_pri dns_snd server
	json_get_vars ttl  peeraddr ip6addr

	echo dslite start:$cfg > /dev/console

	[ -z "$ip6addr" ] && { 
		local wanif="$tunlink" 
		if [ -z "$wanif" ] && ! network_find_wan6 wanif; then 
			proto_notify_error "$cfg" "NO_WAN_LINK" 
			sleep 5 
			echo "NO_WAN_LINK_ERROR" >/tmp/connecterror
			proto_setup_failed "$cfg" 
			return 
		fi 
        
		if ! network_get_ipaddr6 ip6addr "$wanif"; then 
			proto_notify_error "$cfg" "NO_WAN_LINK" 
			sleep 5 
			echo "NO_WAN_LINK_ERROR" >/tmp/connecterror
			proto_setup_failed "$cfg" 
			return 
		fi 
		network_get_dnsserver dns_server "$wanif"
	}

	echo "ip6addr : $ip6addr" > $STDOUT
	config_load /etc/config/network
	config_get dynamic_mode $cfg dynamic_mode
	[ $dynamic_mode -eq 1 ] && {
		config_load /etc/config/dsliteV6plus
	}
	config_get server $cfg AFTR_name
	
	dns_pri=`echo $dns_server|awk -F ' ' '{print $1}'`
	dns_snd=`echo $dns_server|awk -F ' ' '{print $2}'`
	echo dns_pri $dns_pri dns_snd $dns_snd > $STDOUT

	[ $dynamic_mode -eq 1 ] && [ -z "$peeraddr" ] && [ -z "$server" ] && {
		DSLITE_AFTR_LIST_PRIVATE=$(uci get protocol.dslite.AFTR_name_list_private)
		DSLITE_AFTR_LIST_PUBLIC=$(uci get protocol.dslite.AFTR_name_list_public)
		if [ -n "$DSLITE_AFTR_LIST_PRIVATE" -o -n "$DSLITE_AFTR_LIST_PUBLIC" ]; then
			for domain in $DSLITE_AFTR_LIST_PRIVATE; do
				dnslookup -6 -t 2 "$domain" $dns_pri || \
				dnslookup -6 -t 2 "$domain" $dns_snd && {
					uci_set dsliteV6plus wan AFTR_name "$domain"
					uci_commit dsliteV6plus
					server="$domain"
				}	
			done
			for domain in $DSLITE_AFTR_LIST_PUBLIC; do
				addr_ipv6=`dnslookup -6 -t 2 "$domain" $dns_pri`
				[ -z "$addr_ipv6" ] && addr_ipv6=`dnslookup -6 -t 2 "$domain" $dns_snd`
				for addr in $addr_ipv6; do
					ping -6 $addr -c 1 -W 3 && {
						uci_set dsliteV6plus wan AFTR_name "$domain"
						uci_commit dsliteV6plus
						server="$domain"
						peeraddr="$addr"
					}
				done
			done
		else
			uci_set dsliteV6plus wan AFTR_name "gw.transix.jp"
			uci_commit dsliteV6plus
			server="gw.transix.jp"
		fi
	}
	
	echo "server: $server" > $STDOUT
	echo "dns server : $dns_server" > $STDOUT
	[ -n "$server" ] && [ -z "$peeraddr" ] && {
		peeraddr=`dnslookup -6 -t 2 $server $dns_pri`
		echo "pridnslookup -6 -t 2 $server $dns_pri" > $STDOUT

		[ -z "$peeraddr" ] && {
			peeraddr=`dnslookup -6 -t 2 $server $dns_snd`
			echo "snddnslookup -6 -t 2 $server $dns_snd" > $STDOUT
		}

		[ -n "$peeraddr" ] && {
			for addr in $peeraddr; do
				local ping_success=`ping -6 $addr -c 1 -W 3|grep "bytes from"`
				count=`expr $count + 1`
				[ $count == 1 ] && addr_temp=$addr
				[ -n "$ping_success" ] && {
					flag=1
					echo "ping success,peeraddr=$addr" > $STDOUT
					peeraddr=$addr
					break
				}
			done
			[ $flag == 0 ] && peeraddr=$addr_temp
			echo $peeraddr > /tmp/dslt-dnsreplyUsed
		}

	}

	[ -z "$peeraddr" ] && {
		echo "peeraddr is null" > $STDOUT
		echo "MISSING_ADDRESS_ERROR" >/tmp/connecterror
		proto_notify_error "$cfg" "MISSING_ADDRESS"
		#proto_block_restart "$cfg"
		sleep 10
		proto_setup_failed "$cfg"
		return
	}
		
	echo "peer2: $peeraddr" > $STDOUT

	mtu=65000
	tunlink=${tunlink:-wanv6}
	/lib/netifd/dslite-up.sh $link $mtu $ip6addr $peeraddr $tunlink $cfg
	
	if [ -n "$server" ]; then
		proto_run_command "$cfg" /usr/sbin/dslite \
			-i "$link" \
			-m "$mtu" \
			-l "$ip6addr" \
			-6 "$tunlink" \
			-s "$server" \
			"$cfg"
	fi
	
	[ -e "/proc/dslite_fast_forward" ] && echo 1 0 > /proc/dslite_fast_forward
}

proto_dslite_teardown() {
	local cfg="$1"
	local link="dslt-$cfg"
	echo dslite teardown > /dev/console
	
	proto_kill_command $cfg
	
	[ -e "/proc/dslite_fast_forward" ] && echo 0 0 > /proc/dslite_fast_forward

	# delete dslite config in dsliteV6plus
	uci delete dsliteV6plus.wan.AFTR_name
	uci commit

}

proto_dslite_init_config() {            
	available=1
	proto_config_add_string "AFTR_name"
	proto_config_add_string "ip6addr"
	proto_config_add_string "peeraddr"
	proto_config_add_string "tunlink"
	proto_config_add_int "mtu"
	proto_config_add_int "ttl"
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol dslite
}
