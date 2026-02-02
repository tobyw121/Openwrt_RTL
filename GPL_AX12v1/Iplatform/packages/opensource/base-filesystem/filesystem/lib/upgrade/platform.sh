#!/bin/sh
RAMFS_COPY_DATA_PLAT="/lib/*.so* /usr/lib/liblua.so* /usr/lib/libxml2.so* /usr/lib/libifaddrs.so /etc/init.d/logd /etc/rc.common"
RAMFS_COPY_DIR_PLAT="/usr/lib/lua /etc/config /etc/profile.d /var /lib/functions"
RAMFS_COPY_BIN_PLAT="/usr/sbin/logd /usr/bin/lua /usr/sbin/logreset /sbin/start-stop-daemon /usr/bin/nvrammanager /usr/sbin/cloud_cleanFwInfo /sbin/getfirm  /usr/bin/ledcli /usr/bin/ledctrl"

platform_check_image() {
	echo "Platform image check OK ..."
	return 0
}

platform_upgrading_blink() {
	local product_name=$(uci get system.system.hostname)
	if [ "$product_name" == "Archer_Air_R5" ]; then
		[ -x "/usr/bin/ledcli" ] && ledcli WAN0_OFF
	fi

	local fw_led_support=$(uci get profile.@ledctrl[0].fw_led_support -c "/etc/profile.d" -q)
	if [ -n "$fw_led_support" -a "$fw_led_support" == "yes" ]; then
		[ -x "/usr/bin/ledcli" ] && ledcli FW_twinkle
	else
		[ -x "/usr/bin/ledcli" ] && ledcli STATUS_SAN
	fi

	[ -x "/usr/bin/lp5523ctrl" ] && ubus send leds '{"action" : "3","status" : "0"}'
	[ -x "/usr/bin/ledmatrix" ] && ubus call led_matrix event '{"firmware_update_begin":0}'
	[ -x "/usr/bin/screen_controller" ] && ubus call screen_controller event  '{"upgrade":1}'
}

platform_upgrading_hook() {
	echo "Start upgrading Status-LED blinking ..."
	platform_upgrading_blink
}

platform_do_upgrade() {
	if type 'platform_upgrading_hook' >/dev/null 2>/dev/null; then
		echo "Call platform upgrading hook ..."
		platform_upgrading_hook 
	fi
	echo "Perform nvrammanager upgrading ..."
	nvrammanager -u $@
}

platform_pre_upgrade() {
	echo "Platform do progress pre upgrade ..."

	#local hapd_flag=$(uci get profile.@wireless[0].wps_hostapd_support -c "/etc/profile.d" -q)
	local sdk_model=$(uci get profile.@global[0].model -c "/etc/profile.d" -q)

	# fix bug 341405, set hapd_enable=0 when update firmware for bcm product
	#if [ -n "$hapd_flag" -a "$hapd_flag" == "yes" ]; then
	#	nvram set hapd_enable=0
	#	nvram commit
	#fi

	# homecare tm-shn
	if [ -n "$sdk_model" -a "$sdk_model" == "INTEL_GRX350" -a -f "/tmp/tm-shn/setup.sh" ]; then
		/tmp/tm-shn/setup.sh stop eth1
		conntrack -F
	fi

	# amazon ffs
	[ -x /etc/init.d/ffs ] && {
		/etc/init.d/ffs stop
	}
}
