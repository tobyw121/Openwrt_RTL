#!/bin/sh
#
# script file to start network
#
# Usage: init.sh {gw | ap} {all | bridge | wan}
#
echo 0 > /proc/luna_watchdog/watchdog_flag
insmod compat.ko
sleep 2
insmod cfg80211.ko
sleep 2
insmod 8852ae_98d.ko rtw_phy_file_path=/var/ rtw_fw_file_path=/var/rtl8852ae/RTL8852Afw.bin rtw_vht_enable=2
#sleep 5
#hostapd /etc/conf/hostapd-backports.conf -B