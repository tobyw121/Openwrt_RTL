#!/bin/sh
. /usr/lib/modem/usbmodem_log.sh
. /lib/functions.sh
. ../netifd-proto.sh
init_proto "$@"

proto_ncm_init_config() {
	no_device=1
	available=1
	proto_config_add_string "device"
	proto_config_add_string apn
	proto_config_add_string auth
	proto_config_add_string username
	proto_config_add_string password
	proto_config_add_string pincode
	proto_config_add_string delay
	proto_config_add_string mode
	#syslog $LOG_PROTO_INIT "proto=ncm"
}

proto_ncm_setup() {
	local interface="$1"
    local iface="$2"
    
	local manufacturer initialize setmode connect

	local device apn auth username password pincode delay mode
	json_get_vars device apn auth username password pincode delay mode
	
    syslog $LOG_PROTO_SETUP "proto=ncm device=$device apn=$apn auth=$auth username=$username password=$password pincode=$pincode delay=$delay mode=$mode"
	
	[ -n "$device" ] || {
		echo "ncm[$$]" "No control device specified" >>/dev/console
		proto_notify_error "$interface" NO_DEVICE
		proto_set_available "$interface" 0
		return 1
	}
	[ -e "$device" ] || {
		echo "ncm[$$]" "Control device not valid" >>/dev/console
		proto_set_available "$interface" 0
		return 1
	}
	[ -n "$apn" ] || {
		echo "ncm[$$]" "No APN specified" >>/dev/console
		proto_notify_error "$interface" NO_APN
		proto_set_available "$interface" 0
		return 1
	}

	[ -n "$delay" ] && sleep "$delay"
	manufacturer=`gcom -d "$device" -s /etc/gcom/getcardinfo.gcom | awk '/Manufacturer/ { print $2 }'`
	[ $? -ne 0 ] && {
		echo "ncm[$$]" "Failed to get modem information" >>/dev/console
		proto_notify_error "$interface" GETINFO_FAILED
		proto_set_available "$interface" 0
		return 1
	}

	json_load "$(cat /etc/gcom/ncm.json)"
	json_select "$manufacturer"
	[ $? -ne 0 ] && {
		echo "ncm[$$]" "Unsupported modem" >>/dev/console
		proto_notify_error "$interface" UNSUPPORTED_MODEM
		proto_set_available "$interface" 0
		return 1
	}
	json_get_values initialize initialize
	for i in $initialize; do
        echo "---initialize:$initialize" >>/dev/console
		eval COMMAND="$i" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
			echo "ncm[$$]" "Failed to initialize modem" >>/dev/console
			proto_notify_error "$interface" INITIALIZE_FAILED
			proto_set_available "$interface" 0
			return 1
		}
	done

	[ -n "$pincode" ] && {
		PINCODE="$pincode" gcom -d "$device" -s /etc/gcom/setpin.gcom || {
			echo "ncm[$$]" "Unable to verify PIN" >>/dev/console
			proto_notify_error "$interface" PIN_FAILED
			proto_block_restart "$interface"
			return 1
		}
	}
	[ -n "$mode" ] && {
		json_select modes
		json_get_var setmode "$mode"
		COMMAND="$setmode" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
			echo "ncm[$$]" "Failed to set operating mode" >>/dev/console
			proto_notify_error "$interface" SETMODE_FAILED
			proto_set_available "$interface" 0
			return 1
		}
		json_select ..
	}
	
	local sig_res=""
	sig_res=$(gcom -d ${device} -s /etc/gcom/getstrength.gcom 2>/dev/null)
	#echo "sig_res:$sig_res" >>/dev/console
	[ -z $sig_res] || {
		rssi=$(echo "$sig_res" | awk '{
		if($0 ~ /CSQ:/ )
		{
			split($0,arr_tmp,":")
			split(arr_tmp[2],arr,",")
			print(arr[1])
		}
		}')
		rssi=`echo $rssi | sed 's/ //g'`
		#echo "rssi:$rssi" >>/dev/console
		sig_flag=$(expr match $rssi "^[0-9]*$")
		if [ $sig_flag -gt 0 ];then
			uci set modem.modemisp.modem_signal="$rssi"
			uci commit modem
		fi
		#st=$(uci get modem.modemconf.modem_signal)
		#echo "modemconf st:$st*************************************" >>/dev/console
	}		
	syslog $LOG_GET_SIGNAL "gcom -d ${device} -s /etc/gcom/getstrength.gcom" "$sig_res" "$rssi"

    if [ $auth = "0" ];then
        json_get_var connect connectnoauth
    else
        json_get_var connect connect
    fi
	 
	eval COMMAND="$connect" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
		echo "ncm[$$]" "Failed to connect" >>/dev/console
		proto_notify_error "$interface" CONNECT_FAILED
		proto_set_available "$interface" 0
		return 1
	}
	[ -z $iface ] && iface=wwan0
	ifconfig $iface up

	proto_export "INTERFACE=$interface"
	proto_export "IFNAME=$iface"
	proto_run_command "$interface" udhcpc \
		-p /var/run/udhcpc-$iface.pid \
		-s /lib/netifd/dhcp.script \
		-O 33 -O 121 -O 249 \
		-f -R -a -t 0 -i "$iface" \
		${ipaddr:+-r $ipaddr} \
		${hostname:+-H $hostname} \
		${vendorid:+-V "$vendorid"} \
		$clientid $broadcast $dhcpopts
}

proto_ncm_teardown() {
	local interface="$1"

	local device 
	json_get_vars device 
	local manufacturer disconnect
	syslog $LOG_PROTO_TEARDOWN "proto=ncm $@"
	manufacturer=`gcom -d "$device" -s /etc/gcom/getcardinfo.gcom | awk '/Manufacturer/ { print $2 }'`
	[ $? -ne 0 ] && {
		echo "ncm[$$]" "Failed to get modem information" >>/dev/console
		proto_notify_error "$interface" GETINFO_FAILED
		return 1
	}
	json_load "$(cat /etc/gcom/ncm.json)"
	json_select "$manufacturer" || {
		echo "ncm[$$]" "Unsupported modem" >>/dev/console
		proto_notify_error "$interface" UNSUPPORTED_MODEM
		return 1
	}

	json_get_var disconnect disconnect
    
	COMMAND="$disconnect" gcom -d "$device" -s /etc/gcom/runcommand.gcom || {
		echo "ncm[$$]" "Failed to disconnect"
		proto_notify_error "$interface" DISCONNECT_FAILED
		return 1
	}
    
    proto_kill_command "$interface" 15
}

add_protocol ncm
