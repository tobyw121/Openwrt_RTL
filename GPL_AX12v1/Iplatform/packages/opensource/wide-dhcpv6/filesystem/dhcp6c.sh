#!/bin/sh

[ -x /usr/sbin/dhcp6c ] || exit 0

dhcp6cdir="/tmp/dhcp6c"
conffile="$dhcp6cdir/dhcp6c.conf"
dhcp6cpid="$dhcp6cdir/dhcp6c.pid"

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh

	config_load /etc/config/sysmode
	config_get mode sysmode mode "router"
	[ "$mode" = "repeater" -o "$mode" = "ap" -o "$mode" = "client" ] && exit 0

	init_proto "$@"
}

proto_dhcp6c_init_config() {
	proto_config_add_string "dns"
	proto_config_add_string "ip_mode"
	proto_config_add_string "ip_config"
	proto_config_add_string "ifname"
	proto_config_add_string "dns_mode"
	proto_config_add_string "lanif"
}

proto_dhcp6c_create_conffile(){
	local ip_mode="$1"
	local dns_mode="$2"
	local ifname="$3"
	local lanif="$4"
	local ip_config="$5"
	#Use the fixed iaid for ComCast IPv6
	#local id=`date +%s`
	#id=`expr $id % 1000000`
	#local id2=`expr $id + 1`
	local id=1
	local id2=$id

	rm -rf ${dhcp6cdir}
	mkdir -p ${dhcp6cdir}
	
	config_load /etc/config/network
	config_get wan_proto wan proto
	[ "$wan_proto" == "v6plus" -o "$wan_proto" == "dslite" ] && [ "$ip_config" == "dhcpv6" ] && ip_mode="prefix"
	
	[ "$ip_mode" == "prefix" ] && {
		echo "interface $ifname {" > "${conffile}" 
		[ "$ip_config" == "dhcpv6" ] && echo -e "\tsend ia-na $id;" >> "${conffile}"
		echo -e "\tsend ia-pd $id2;" >> "${conffile}"

		[ "$dns_mode" == "dynamic" ] && echo -e "\trequest domain-name-servers;" >> "${conffile}"
		echo -e "\tscript \"/lib/netifd/dhcp6c.script\";" >> "${conffile}"

		echo -e "};\n" >>  "${conffile}"

		echo -e "id-assoc na $id {};\n" >> "${conffile}"

		echo "id-assoc pd $id2 {" >> "${conffile}"
		echo -e "\tprefix-interface $lanif {" >> "${conffile}"
		echo -e "\t\tsla-id 1;" >> "${conffile}"
		echo -e "\t};" >>  "${conffile}"
		echo "};" >>  "${conffile}"
	}

	[ "$ip_mode" == "non_temp" ] && {
		echo "interface $ifname {" > "${conffile}"
		[ "$ip_config" == "dhcpv6" ] && echo -e "\tsend ia-na $id;" >> "${conffile}"

		if [ "$dns_mode" == "dynamic" ]; then
			echo -e "\trequest domain-name-servers;" >> "${conffile}"
			if [ "$ip_config" == "slaac" ]; then
				echo -e "\tinformation-only;" >> "${conffile}"
			fi
		fi
		echo -e "\tscript \"/lib/netifd/dhcp6c.script\";" >> "${conffile}"

		echo "};" >>  "${conffile}"

		echo "id-assoc na $id {};" >> "${conffile}"
	}
}

proto_dhcp6c_setup() {
	local config="$1"
	local iface="$2"
	local count=10
	local mbit=1

	json_get_vars dns_mode ip_mode ifname lanif ip_config

        if [ $ip_config == "slaac" -o $ip_config == "auto" ]; then
                echo "1"  > /proc/sys/net/ipv6/conf/$ifname/accept_ra_pinfo
        else
                echo "0"  > /proc/sys/net/ipv6/conf/$ifname/accept_ra_pinfo
        fi

	echo '-1' > /proc/sys/net/ipv6/conf/$ifname/ndisc_mbit 
	echo "2"  > /proc/sys/net/ipv6/conf/$ifname/accept_ra
	echo "1"  > /proc/sys/net/ipv6/conf/$ifname/sendrs
	# Wait for RA
	mbit=`cat /proc/sys/net/ipv6/conf/$ifname/ndisc_mbit`
	while [ $count -ne 0 -a $mbit -eq -1 -a "$ip_config" != "dhcpv6" ]; do
		sleep 1
		mbit=`cat /proc/sys/net/ipv6/conf/$ifname/ndisc_mbit`
		let "count=count - 1"
	done
	if [ $mbit -eq -1 -a "$ip_config" != "dhcpv6" ]; then
		sleep 3
		proto_setup_failed "$config"
		return
	fi
	if [ "$ip_config" == "auto" ]; then
		if [ $mbit -eq 0 ]; then
			ip_config="slaac"
		elif [ $mbit -eq 1 ]; then
                	ip_config="dhcpv6"
                else
			sleep 3
			proto_setup_failed "$config"
			return
		fi
	fi
	if [ "$ip_config" == "dhcpv6" ]; then
		ip_config="dhcpv6"
	
	elif [ "$ip_config" == "slaac" ]; then
		if [ $mbit -eq 1 ]; then
			sleep 3
			proto_setup_failed "$config"
			return
		else
			ip_config="slaac"
			
			# only set slaac address when do'not set slaac address in dhcp6c.script
			if [ "$ip_mode" == "non_temp" -a "$dns_mode" == "static" ]; then
				proto_init_update "$ifname" 1                                                      
				while read line                                                                    
				do                                                                                                                                  
						[ $ifname != ${line##* } ] && continue                                     
						[ "fe80" = ${line:0:4} ] && continue                                       
						element=${line:0:32}                                                       
																								   
						ip6addr=${element:0:4}:${element:4:4}:${element:8:4}:${element:12:4}       
						new_ip6addr=$ip6addr:${element:16:4}:${element:20:4}:${element:24:4}:${element:28:4}
																								  
				done </proc/net/if_inet6                                                           
																							   
				if [ -n "$new_ip6addr" ]; then
						[ ! -f "/tmp/slaac_addr" ] && echo $new_ip6addr > /tmp/slaac_addr
						proto_add_ipv6_address "$new_ip6addr" "64"                                 
						proto_send_update "$interface"                                                                                           
				fi
			fi
			echo "* * * * * sh /lib/netifd/dhcp6c_slaac.sh &" > /tmp/slaac_cron
			crontab -l | grep -v "/lib/netifd/dhcp6c_slaac.sh" | cat - "/tmp/slaac_cron" | crontab -
			rm -f /tmp/slaac_cron
		fi
	fi
	
	proto_dhcp6c_create_conffile "$ip_mode" \
		"$dns_mode" "$ifname" "$lanif" "$ip_config"

	local model=$(uci get profile.@global[0].model -c /etc/profile.d -q)
	if [ "$model" = "RTL_8197" ]; then
		# set rtl8197 ipv6 acl rules
		proto_setup_ipv6_acl
	fi

	if [ "$ip_config" == "dhcpv6" -o "$ip_mode" == "prefix" -o "$dns_mode" == "dynamic" ]; then
		proto_run_command "$config" /usr/sbin/dhcp6c -f \
		-p "$dhcp6cpid" \
		-c "$conffile" \
		-I "$ifname" \
		-t "$config" \
		"$ifname"
	fi
}

proto_dhcp6c_teardown() {
	local interface="$1"
	local ifname="$2"

	case "$ERROR" in
		11|19)
			proto_notify_error "$interface" AUTH_FAILED
			proto_block_restart "$interface"
		;;
		2)
			proto_notify_error "$interface" INVALID_OPTIONS
			proto_block_restart "$interface"
		;;
	esac
	
	crontab -l | grep -v "/lib/netifd/dhcp6c_slaac.sh" | crontab -
	kill -9 `ps |grep dhcp6c_slaac.sh |grep -v grep | awk '{print $1}'`
	rm /tmp/slaac_addr
	
	[ -n "$ifname" ] && {
		echo "1" > /proc/sys/net/ipv6/conf/$ifname/accept_ra
		ip -6 route del ::/0 dev "$ifname"
	}
	
	proto_kill_command  "$interface" 15
	sleep 1
	rm -rf ${dhcp6cdir}

}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol dhcp6c
}
