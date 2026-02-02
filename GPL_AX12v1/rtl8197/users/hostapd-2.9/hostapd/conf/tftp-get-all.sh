#!/bin/sh
#
# script file to start network
#
# Usage: init.sh {gw | ap} {all | bridge | wan}
#
cd /var
sleep 1
tftp 192.168.1.66 -r compat.ko -g
sleep 3
tftp 192.168.1.66 -r cfg80211.ko -g
sleep 3
tftp 192.168.1.66 -r 8852ae_98d.ko -g
sleep 3
mkdir rtl8852ae
sleep 1
cd ./rtl8852ae
sleep 1
tftp 192.168.1.66 -r PHY_REG.txt -g
sleep 3
tftp 192.168.1.66 -r RadioA.txt -g
sleep 3
tftp 192.168.1.66 -r RadioB.txt -g
sleep 3
tftp 192.168.1.66 -r TXPWR_ByRate.txt -g
sleep 3
tftp 192.168.1.66 -r TXPWR_LMT.txt -g
sleep 3
tftp 192.168.1.66 -r TXPWR_LMT_RU.txt -g
sleep 3
tftp 192.168.1.66 -r TXPWR_TrackTSSI.txt -g
sleep 3
tftp 192.168.1.66 -r RTL8852Afw.bin -g