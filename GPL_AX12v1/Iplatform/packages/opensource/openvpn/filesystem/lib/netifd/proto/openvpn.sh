#!/bin/sh
[ -x /usr/sbin/openvpn ] || exit 0

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. /lib/functions/service.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

SERVICE_DAEMONIZE=1
SERVICE_WRITE_PID=1
SERVICE_PID_FILE="/var/run/openvpn-client.pid"
authfile="/etc/openvpn/openvpn.passwd"

proto_openvpn_init_config() {
	proto_config_add_string "username"
	proto_config_add_string "password"
	proto_config_add_string "connect"
	proto_config_add_string "disconnect"
	available=1
	no_device=1
}

proto_openvpn_setup() {
	local config="$1"
	local iface="$2"
	local parent=""
	local username=""
	local password=""
	local mtu=""

	config_load vpn
	config_get enabled client enabled
	config_get vpntype client vpntype
	if [ $enabled == "off" -o $vpntype == "none" ]; then
		echo "vpn client is off, exit" > /dev/console
	fi

	grep -wq "tun" /proc/modules || /sbin/insmod tun 2>&- >&-

	config_load network
	config_get parent $config parent
	config_get iface $parent ifname
	config_get username $config username
	config_get password $config password
	config_get des $config des

	service_stop /usr/sbin/openvpn

	if [ "$username" != ""  -a "$password" != "" ]; then
		rm -f $authfile
		echo $username >> $authfile
		echo $password >> $authfile
#temporarily remove --route-table option, cause openvpn doesn't recognize it
		service_start /usr/sbin/openvpn --ignore-unknown-option block-outside-dns register-dns --config "/etc/openvpn/$des.ovpn" --writepid $SERVICE_PID_FILE --script-security 2 --up "/lib/netifd/openvpn-up.sh" --down "/lib/netifd/openvpn-down.sh" --auth-nocache --auth-user-pass "$authfile"
	else
		service_start /usr/sbin/openvpn --ignore-unknown-option block-outside-dns register-dns --config "/etc/openvpn/$des.ovpn" --writepid $SERVICE_PID_FILE --script-security 2 --up "/lib/netifd/openvpn-up.sh" --down "/lib/netifd/openvpn-down.sh"
	fi
}

proto_openvpn_teardown() {
	local interface="$1"
	local openvpn_server_enabled="off"
	echo "openvpn_teardown: $@" > /dev/console

	service_stop /usr/sbin/openvpn

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
#proto_kill_command "$interface" 15

	config_load openvpn
	config_get openvpn_server_enabled "server" "enabled" "off"
	if [ $openvpn_server_enabled == "on" ]; then
		echo "openvpn server is running ,do not rmmode tun" > /dev/console
	else
		echo "rmmod openvpn tun in client" > /dev/console
		/sbin/rmmod tun 2>&- >&-
	fi

}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol openvpn
}

