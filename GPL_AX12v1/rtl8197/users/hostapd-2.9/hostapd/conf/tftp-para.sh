#!/bin/sh
#
# script file to start network
#
# Usage: init.sh {gw | ap} {all | bridge | wan}

echo Please create RFE type folder [ex: RFE50]
read path

echo Please create FEM type folder [ex: SKY85333]
read fem

cd /var/config
sleep 1
mkdir rtl8852ae
sleep 1
cd ./rtl8852ae
mkdir $path
cd $path
sleep 1
tftp 192.168.1.66 -r PHY_REG.txt -g
sleep 1
tftp 192.168.1.66 -r RadioA.txt -g
sleep 1
tftp 192.168.1.66 -r RadioB.txt -g
sleep 1
tftp 192.168.1.66 -r TXPWR_ByRate.txt -g
sleep 1
tftp 192.168.1.66 -r TXPWR_LMT.txt -g
sleep 1
tftp 192.168.1.66 -r TXPWR_LMT_RU.txt -g
sleep 1
tftp 192.168.1.66 -r TXPWR_TrackTSSI.txt -g

if [ "$path" = "RFE51" ] || [ "$path" = "RFE52" ] || [ "$path" = "RFE53" ] || [ "$path" = "RFE54" ]
then
  mkdir $fem
  cd $fem
  tftp 192.168.1.66 -r $fem/PHY_REG.txt -g
  tftp 192.168.1.66 -r $fem/RadioA.txt -g
  tftp 192.168.1.66 -r $fem/RadioB.txt -g
  tftp 192.168.1.66 -r $fem/TXPWR_ByRate.txt -g
  tftp 192.168.1.66 -r $fem/TXPWR_LMT.txt -g
  tftp 192.168.1.66 -r $fem/TXPWR_LMT_RU.txt -g
  tftp 192.168.1.66 -r $fem/TXPWR_TrackTSSI.txt -g
fi
