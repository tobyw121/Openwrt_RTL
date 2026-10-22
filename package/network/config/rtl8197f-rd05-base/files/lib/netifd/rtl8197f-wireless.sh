#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
# LuCI/netifd adapter for RD05's fixed-interface vendor cfg80211 driver.
# mac80211 is the UCI/UI schema here; the packet path remains rtl8192cd.
. /lib/netifd/netifd-wireless.sh
init_wireless_driver "$@"
RD05_AP_LOCK=/var/run/rd05-wlan-apply.lock.d

rd05_ap_lock() {
	local waited=0
	# mkdir is atomic and available in the base system. Do not remove another
	# operation's lock on timeout, or reclaim a possibly still active lock.
	while ! mkdir "$RD05_AP_LOCK" 2>/dev/null; do
		[ "$waited" -lt 20 ] || {
			echo 'RD05 WLAN operation lock unavailable; retry or reboot after an interrupted operation' >&2
			return 1
		}
		sleep 1
		waited=$((waited + 1))
	done
	trap 'rmdir "$RD05_AP_LOCK"' EXIT
	trap 'exit 1' HUP INT TERM
}

drv_mac80211_cleanup() {
	:
}

drv_mac80211_init_device_config() {
	config_add_string phy country
	config_add_int txpower
}

drv_mac80211_init_iface_config() {
	config_add_string ifname
	config_add_boolean hidden isolate wds
	config_add_int ieee80211w
}

drv_mac80211_init_vlan_config() {
	:
}

drv_mac80211_init_station_config() {
	:
}

rd05_valid_text() {
	[ -n "$1" ] && [ "$1" = "$(printf '%s' "$1" | LC_ALL=C tr -d '[:cntrl:]')" ]
}

rd05_read_vif() {
	json_get_var network_bridge bridge
	json_select config
	json_get_vars mode ssid encryption key ifname hidden isolate wds ieee80211w
	json_select ..
	mode="${mode:-ap}"
	hidden="${hidden:-0}"; isolate="${isolate:-0}"
	case "$encryption" in
		none) rd05_enc=open ;;
		psk|psk+aes|psk+ccmp) rd05_enc=psk ;;
		psk-mixed|psk-mixed+aes|psk-mixed+ccmp) rd05_enc=mixed ;;
		psk2|psk2+aes|psk2+ccmp) rd05_enc=psk2 ;;
		*) rd05_error=UNSUPPORTED_ENCRYPTION; return 1 ;;
	esac
	[ "$mode" = ap ] && [ "${wds:-0}" = 0 ] && [ "${ieee80211w:-0}" = 0 ] || {
		rd05_error=UNSUPPORTED_MODE_OR_PMF; return 1
	}
	[ -z "$ifname" ] || [ "$ifname" = "$rd05_ifname" ] || {
		rd05_error=FIXED_ROOT_INTERFACE; return 1
	}
	rd05_valid_text "$ssid" && [ "${#ssid}" -le 32 ] || {
		rd05_error=INVALID_SSID; return 1
	}
	if [ "$rd05_enc" != open ]; then
		rd05_valid_text "$key" && [ "${#key}" -ge 8 ] && [ "${#key}" -le 63 ] || {
			rd05_error=INVALID_WPA2_PASSPHRASE; return 1
		}
	fi
	case "$hidden:$isolate" in 0:0|0:1|1:0|1:1) ;; *) rd05_error=INVALID_BOOLEAN; return 1 ;; esac
	# netifd supplies the actual bridge associated with the wifi-iface network.
	case "$network_bridge" in ''|*[!a-zA-Z0-9_.:-]*) rd05_error=BRIDGE_NETWORK_REQUIRED; return 1 ;; esac
	return 0
}

rd05_validate_vif() {
	rd05_count=$((rd05_count + 1))
	rd05_read_vif || return 1
}

rd05_start_vif() {
	local name="$1" pid
	rd05_read_vif || return 1
	# Do not source the AP front-end into netifd's JSON/global namespace.
	if ! (
		umask 077
		rd05_ap_lock || exit 1
		MW5_WIFI_LIBRARY=1 . /usr/sbin/mw5-wifi
		set_defaults
		load_radio_tuning
		REGDOMAIN="$rd05_country"; BRIDGE="$network_bridge"
		MW5_WIFI_NETIFD=1
		MW5_NETIFD_HIDDEN="$hidden"; MW5_NETIFD_ISOLATE="$isolate"
		stop_hostapd_band "$rd05_band"
		mw5_rtknet_reseed netifd-pre-driver
		probe_driver || exit 1
		mw5_rtknet_reseed netifd-post-driver
		iw reg set "$REGDOMAIN" >>"$LOG_FILE" 2>&1 || exit 1
		rm -f "$RUN_DIR/hostapd-$rd05_band.conf"
		start_radio "$rd05_band" "$rd05_ifname" 1 "$ssid" "$rd05_channel" "$rd05_width" "$rd05_enc" "$key" || exit 1
		mw5_rtknet_reseed netifd-post-ap
	) >>/tmp/mw5-webui/netifd-wlan.log 2>&1; then
		rd05_error=HOSTAPD_START_FAILED
		return 1
	fi
	pid="$(cat "/var/run/mw5-wifi/hostapd-$rd05_band.pid" 2>/dev/null)"
	case "$pid" in ''|*[!0-9]*) rd05_error=HOSTAPD_PID_MISSING; return 1 ;; esac
	wireless_add_vif "$name" "$rd05_ifname"
	wireless_add_process "$pid" /usr/sbin/hostapd 1
}

drv_mac80211_setup() {
	local phy country htmode channel band txpower
	local rd05_band rd05_ifname rd05_width rd05_channel rd05_country
	local rd05_error= rd05_count=0
	local mode ssid encryption key ifname hidden isolate wds ieee80211w rd05_enc
	json_select config
	json_get_vars phy country htmode channel band txpower
	json_select ..
	case "$phy:$band" in
		phy0:5g) rd05_band=5g; rd05_ifname=wlan0 ;;
		phy1:2g) rd05_band=2g; rd05_ifname=wlan1 ;;
		*) wireless_setup_failed INVALID_RADIO; return 1 ;;
	esac
	case "$rd05_band:$htmode" in
		2g:HT20|5g:HT20|5g:VHT20) rd05_width=20 ;;
		2g:HT40|5g:HT40|5g:VHT40) rd05_width=40 ;;
		5g:VHT80) rd05_width=80 ;;
		*) wireless_setup_failed UNSUPPORTED_HTMODE; return 1 ;;
	esac
	case "$channel" in ''|0|auto|*[!0-9]*) wireless_setup_failed FIXED_CHANNEL_REQUIRED; return 1 ;; esac
	[ "${#channel}" -le 3 ] || { wireless_setup_failed INVALID_CHANNEL; return 1; }
	rd05_channel="$channel"
	case "${country:-00}" in 00|[A-Z][A-Z]) rd05_country="${country:-00}" ;; *) wireless_setup_failed INVALID_COUNTRY; return 1 ;; esac
	# Do not silently accept a LuCI transmit-power override we cannot verify.
	[ -z "$txpower" ] || { wireless_setup_failed TXPOWER_OVERRIDE_UNSUPPORTED; return 1; }
	for_each_interface '' rd05_validate_vif
	[ "$rd05_count" = 1 ] || rd05_error=ONE_AP_PER_RADIO_REQUIRED
	if [ -n "$rd05_error" ]; then wireless_setup_failed "$rd05_error"; return 1; fi
	mkdir -p /tmp/mw5-webui
	for_each_interface '' rd05_start_vif
	if [ -n "$rd05_error" ]; then
		drv_mac80211_teardown
		wireless_setup_failed "$rd05_error"
		return 1
	fi
	wireless_set_data phy="$phy" band="$rd05_band"
	wireless_set_up
}

drv_mac80211_teardown() {
	local phy band
	json_select config
	json_get_vars phy band
	json_select ..
	case "$phy:$band" in phy0:5g|phy1:2g) ;; *) return 1 ;; esac
	if ! (
		rd05_ap_lock || exit 1
		MW5_WIFI_LIBRARY=1 . /usr/sbin/mw5-wifi
		stop_hostapd_band "$band"
		case "$band" in 5g) ip link set wlan0 down ;; 2g) ip link set wlan1 down ;; esac
	) >/dev/null 2>&1; then
		return 1
	fi
	wireless_process_kill_all
}

add_driver mac80211
