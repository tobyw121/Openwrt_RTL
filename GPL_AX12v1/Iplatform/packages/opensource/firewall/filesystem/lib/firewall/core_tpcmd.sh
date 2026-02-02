# tpsdk专有功能 by huluyao 20140423

_print_server_rule() {
	local guest_ifname_2g=$(uci get profile.@wireless[0].wireless_guest_ifname_2g -c "/etc/profile.d" -q)
	local guest_ifname_5g=$(uci get profile.@wireless[0].wireless_guest_ifname_5g -c "/etc/profile.d" -q)
	local guest_ifname_5g_2=$(uci get profile.@wireless[0].wireless_guest_ifname_5g_2 -c "/etc/profile.d" -q)

	local guest_rule_24g=
	local guest_rule_5g=
	local guest_rule_52g=
	if [ x"$guest_ifname_2g" == "x" ]; then
	    guest_rule_24g="-m physdev --physdev-in wl0.1"
	else
		guest_rule_24g="-m physdev --physdev-in ${guest_ifname_2g}"
	fi

	if [ x"$guest_ifname_5g" == "x" ]; then
	    guest_rule_5g="-m physdev --physdev-in wl1.1"
	else
		guest_rule_5g="-m physdev --physdev-in ${guest_ifname_5g}"
	fi

    local support_triband=$(uci get profile.@wireless[0].support_triband -c "/etc/profile.d")
    if [ x"$support_triband" == x"yes" ]; then
		if [ x"$guest_ifname_5g_2" == "x" ]; then
			guest_rule_52g="-m physdev --physdev-in wl2.1"
		else
			guest_rule_52g="-m physdev --physdev-in ${guest_ifname_5g_2}"
		fi
    fi

    if [ "$1" == "add" ]; then
        fw add 4 f block_entry_list_printer DROP { "$guest_rule_24g" }
        fw add 4 f block_entry_list_printer DROP { "$guest_rule_5g" }
        if [ x"$support_triband" == x"yes" ]; then
            fw add 4 f block_entry_list_printer DROP { "$guest_rule_52g" }
        fi
	elif [ "$1" == "del" ]; then
        fw del 4 f block_entry_list_printer DROP { "$guest_rule_24g" }
        fw del 4 f block_entry_list_printer DROP { "$guest_rule_5g" }
        if [ x"$support_triband" == x"yes" ]; then
            fw del 4 f block_entry_list_printer DROP { "$guest_rule_52g" }
        fi
    fi
}

fw_load_tpcmd() {    
    # for guest_network
    local model=$(uci get profile.@global[0].model -c "/etc/profile.d")
    # FW Replaced by br-filter control, fix 427182
    [ "$model" = "QCA_IPQ50XX" -o "$model" = "QCA_IPQ95XX" ] || {
    local rule="-p tcp -m multiport --ports 80,443,22,20002,139,445,20,21,8200"
    local udp_rule="-p udp -m multiport --dports 137,138"
    local print_rule="-p tcp --dport 20005"
    local sftp_support="$(uci -q -c /etc/profile.d get profile.@sftp[0].sftp_support)"    
    fw add 4 f block_entry_list
    fw add 4 f zone_lan block_entry_list 1 { "$rule" }
    fw add 4 f block_entry_list_udp
    fw add 4 f zone_lan block_entry_list_udp 1 { "$udp_rule" }
    fw add 4 f block_entry_list_printer
    fw add 4 f zone_lan block_entry_list_printer 1 { "$print_rule" }
    _print_server_rule add
    }

    fw add 4 f ftp_access
    fw add 4 f zone_wan ftp_access 1
    fw add 4 f ftp_access_lan
    fw add 4 f zone_lan ftp_access_lan 1
    if [ "$sftp_support" == "yes" ]; then
    	fw add 4 f sftp_access
        fw add 4 f zone_wan sftp_access 1
        fw add 4 f sftp_access_lan
        fw add 4 f zone_lan sftp_access_lan 1
    fi

    # for local management
    fw add i f local_mgnt
    fw add i f local_mgnt DROP

    # for remote management
    fw add 4 f lan_remote_mgnt
    fw add 4 f input_lan lan_remote_mgnt
}

