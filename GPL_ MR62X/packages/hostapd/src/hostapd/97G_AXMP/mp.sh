#!/bin/sh

echo ===== Start MP Mode =====

NAX_MPDAEMON=/usr/sbin/UDPserver
AX_MPDAEMON=/usr/sbin/AX_UDPs_97G


#kill hostapd_cli and hostapd
killall hostapd_cli > /dev/null 2>&1
killall hostapd > /dev/null 2>&1
sleep 1

#killall udhcpc and udhcpd
killall udhcpc > /dev/null 2>&1
killall udhcpd > /dev/null 2>&1
sleep 1

#killall UDPserver
killall UDPserver > /dev/null 2>&1
sleep 1

#killall AX_UDPserver
killall AX_UDPs_97G > /dev/null 2>&1
sleep 1

# set MP mode for g6_wifi_driver
# iwpriv wlan0 set_mp_mode
# iwpriv wlan1 set_mp_mode
# sleep 1

# set MP mode for 97FH
iwpriv wlan0 set_mib mp_specific=1
iwpriv wlan1 set_mib mp_specific=1
sleep 1

#UP wlan interface
ifconfig wlan0 up
sleep 3

ifconfig wlan1 up
sleep 3

iwpriv wlan0 mp_start
sleep 3

iwpriv wlan1 mp_start
sleep 3

#start udpserver
# if [ -f "$NAX_MPDAEMON" ]; then
#         $NAX_MPDAEMON &
# fi

# if [ -f "$AX_MPDAEMON" ]; then
#         $AX_MPDAEMON &
# fi

#start 2.4G & 5G udpserver with different socket
UDPserver wlan1 &
UDPserver wlan0 &

#disable debug message to reduce cpu loading
echo 0 0 > /proc/net/rtk_wifi6/wlan0/log_level
