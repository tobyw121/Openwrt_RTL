#!/bin/bash
#
# Test Down/Up Rmmod/insomd Driver and switch to Ping AP Test
#****************************************************************
#
# ************************          You must frist to check "Edit" and the end "main Test Item".    *********************************************
#
#############################################################
#read -p "please input Driver Module Name(exp:8712u) :" dmodule
#read -p "please input wlan Interface Number(exp:0~9):" wNum
#read -p "please input Wlan'sIP:" ip
#read -p "please input Ping Destination's IP:" destip
#************************* Edit Config*******************************************************************************

### Driver Module ###
dmodule="8188eu.ko"
### Wlan interface Number###
wNum=2
###your Wlan IP Address##
ip="192.168.10.102"
###you Want To Ping Dest IP Address##
destip="192.168.10.10"
ConnetBreakCount=3
PingBreakCount=3
LogFile=EventLog
#************************* Edit connect AP setting SSID**************************************************
####1.OPEN: ######
ap[1]="sd4_open"

####2.WPA/WPA2: ######
ap[2]="sd4_wpa"

####3.WPA/WPA2: ######
ap[3]="sd4_wpa2"

####4.WEP: #######
ap[4]="" #"sd4_wep"

####5.OPEN:5G  #######
ap[5]="" #"sd4_open_5g"

####6.WPA/WPA2:5G #######
ap[6]="" #"sd4_wpa2_5g"

#########################################################################
#### wpa_supplicant in Background #############
wpa_supplicant -g/var/run/wpa_supplicant-global -B
###############################################
function LOG(){
	echo -e $1 	
	echo `date`: $1>> $LogFile
}
function fail_exit(){
	LOG "At `date` Module-$dmodule wlan$wNum AutoTest FAILED"
	exit 0
}

function wptap(){
		wpa_cli -iwlan$wNum disconnect
		ssid=$1
		scstatus=`wpa_cli -iwlan$wNum scan`
		LOG "wpa_cli scan status: $scstatus"
		if [ "$scstatus" != "OK" ]; then
				LOG "Can't scan : $scstatus  "
		fi
		sleep 3
		wpa_cli -iwlan$wNum scan_results
		listNet=`wpa_cli -iwlan$wNum list_networks|grep 0`
		if [ "$listNet" = "" ]; then 
			LOG "No wpa_cli old profile :$pro"
		else
			LOG "remove wpa_cli old profile :$pro "
			wpa_cli -iwlan$wNum disable_network $pro
			wpa_cli -iwlan$wNum remove_network $pro
		fi
	LOG "Connect to SSID: $ssid"
	wpa_cli -iwlan$wNum reconnect		
	## Try to Connect AP
	concount=0
	while :
	do
		LOG ">>>Try To associate count: $concount"
		##add wpacli profile
		pro=`wpa_cli -iwlan$wNum add_network`
		LOG ">>> Wpa_cli Profile number: $pro"
		LOG ">>> Wpa_cli Set Profile "
		wpa_cli -iwlan$wNum set_network $pro ssid \"$ssid\"		
		#open
    if [ "$ssid" == "${ap[1]}" ]; then		
			wpa_cli -iwlan$wNum set_network $pro key_mgmt NONE
		#tkip AES
		elif [ "$ssid" == "${ap[2]}" ] || [ "$ssid" == "${ap[3]}" ]; then 
			wpa_cli -iwlan$wNum set_network $pro psk \"12345678\"
		elif [ "$ssid" == "${ap[5]}" ]; then 
			wpa_cli -iwlan$wNum set_network $pro key_mgmt NONE
		elif [ "$ssid" == "${ap[6]}" ]; then 
			wpa_cli -iwlan$wNum set_network $pro psk \"12345678\"
		#WEP
		elif [ "$ssid" == "${ap[4]}" ]; then
			LOG ">>> Wpa_cli Set Profile WEP key"		
			wpa_cli -iwlan$wNum set_network $pro wep_key0 '1234567890'
			LOG ">>> Wpa_cli Set Profile WEP key index "	
			wpa_cli -iwlan$wNum set_network $pro wep_tx_keyidx 0
			LOG ">>> Wpa_cli Set Profile WEP None Mgmt "
			wpa_cli -iwlan$wNum set_network $pro key_mgmt NONE
		fi		
		LOG ">>> Wpa_cli Enable Profile "
		wpa_cli -iwlan$wNum enable_network $pro
		LOG ">>> Wait for connect "
		sleep 10
		COMPLETED=`wpa_cli -iwlan$wNum status|grep wpa_state=COMPLETED`	
		#COMPLETED=`iwconfig wlan$wNum |grep unassociated`
		##Check Wlan connect
		if [ "$COMPLETED" = "wpa_state=COMPLETED" ]; then
	       	   	 LOG ">>> wlan associated to $1. $COMPLETED "
			 break   	
		else		
			LOG ">>> wpa_cli unassociated.>> $COMPLETED "
     			LOG ">>> Re site survey $1 "		   	
			wpa_cli -iwlan$wNum disable_network $pro
			wpa_cli -iwlan$wNum remove_network $pro
			if [ "$concount" == "$ConnetBreakCount" ]; then
				LOG "@@@ Connect Fail to exit @@@"
				echo `date` > ConnectFailFinish
				fail_exit
			fi		
		fi
		concount=$(($concount+1))		
	done
}
function Pap(){
	pcount=0
	arp -d $1
	## For DMP platform ping
	if [ "$platform" != "" ]; then
		while :
		do
			alive=""
			alive=`ping $1 |grep alive`
 	
			if [ "$alive" != "" ];then
			      LOG "DMP Ping OK!!! "
			      break
			else
			    LOG "Ping Failed!!! "
			    sleep 2
			    if [ "$pcount" == "$PingBreakCount" ]; then
				LOG "@@@ Ping Fail to exit @@@"
				echo `date` > PingTimeOutfinish
				fail_exit
			    fi
			    pcount=$(($pcount+1))	
			fi
			
		done
	## For Normal Linux ping
	else 	
		while :
		do
			result=`ping $1 -c 20 |grep ttl`
			if [ "$result" != "" ]; then
				LOG "Wlan Ping success !!! "
				break
			else
				LOG "Wlan Ping Failed !!! "
				sleep 1
				if [ "$pcount" == "$PingBreakCount" ]; then
					LOG "@@@ Ping Fail to exit @@@"
					LOG `date` > PingTimeOutfinish
					fail_exit
				fi
				pcount=$(($pcount+1))
			fi
			
		done
	fi
		
}
function s3(){
		LOG " s3 sleep :$1 !!! "
		sleep 1
		rtcwake -m mem -s $1
	
}
################### Device UP/Down ########################### 
function wlanClosed(){
	#LOG "wlanClosed rmmod module $dmodule wlan$wNum"
	ifconfig wlan$wNum down
	sleep 1
	wpa_cli -g/var/run/wpa_supplicant-global interface_remove wlan$wNum
	#killall wpa_supplicant
	rmmod $dmodule
}
function wlanClosedForce(){
	LOG "wlanClosedForce rmmod module $dmodule wlan$wNum"
	ifconfig wlan$wNum down
	sleep 1
	#wpa_cli -g/var/run/wpa_supplicant-global interface_remove wlan$wNum
	killall wpa_supplicant
	rmmod $dmodule
	exit 0
}

trap wlanClosedForce SIGHUP SIGINT SIGTERM

function wlanDU(){
	wlanClosed
	insmod ./$dmodule rtw_antdiv_cfg=0 rtw_power_mgnt=1 rtw_ips_mode=1 rtw_enusbss=0 rtw_hwpwrp_detect=0 rtw_hwpdn_mode=0 2> t.txt
        insresult=`grep "cannot insert" t.txt`
        if [ "$insresult" != "" ]; then
                        LOG "@@@ insert module Fail to exit @@@"
                        fail_exit
        fi
        sleep 2
        ifconfig wlan$wNum up 2> t2.txt
        upresult=`grep "No such device" t2.txt`
        if [ "$upresult" != "" ]; then
                        LOG "@@@ Device up Fail to exit @@@"
                        fail_exit
        fi
	sleep 3
}

function netifDU(){
	ifconfig wlan$wNum $ip
	wpa_cli -g/var/run/wpa_supplicant-global interface_add wlan$wNum "" nl80211 /var/run/wpa_supplicant
}
############### RandScan ##############
function RandScan(){
ScanMAXCOUNT=6
count=1
while [ "$count" -le $ScanMAXCOUNT ]
do
        number=$[ $RANDOM % $1 ]
        LOG "Now at RandScan $count"th" sleep $number"
				sleep $number
        let "count += 1"
        iwlist wlan$wNum scan
done
}
############## switch Ping AP############
function switchPAP(){
for apname in ${ap[@]}; do	
		LOG "Now connect to $apname "	
		wptap $apname	
		LOG "connected to Ping $apname "		
		Pap $destip			
		wpa_cli -iwlan$wNum disconnect
		sleep 6
	done
}

############******************************* Main********************************************************
rm $LogFile
LOG "Begin Auto_swAPV5.sh Test"

module_name=""
module_no=0

if [ $# -eq 0 ]; then
	module_name=$dmodule
	module_no=$wNum
	test_cnt=0
	echo "You have selected default configuration - WFI:$module_name wlan$module_no counters:$test_cnt"
else
	module_name=$1
	module_no=$2
	test_cnt=$3
	dmodule=$module_name
	wNum=$module_no
	echo "You have selected test - WIFI:$module_name wlan$module_no counters:$test_cnt"
fi

#wlanDU
contc=1
if [ $test_cnt != 0 ]; then
	test_cnt=$(($test_cnt+1))
fi

#while :
until [ "$contc" == "$test_cnt" ]
do	
	LOG "-----------------------------------------"		
	LOG "Now at `date` Module-$dmodule wlan$wNum Test Count: $contc"	
	######## ******************************** Test Item Config**********************
	wlanDU
	netifDU
	switchPAP
	RandScan 6
	#s3 10
	##################################	
	contc=$(($contc+1))
	if [ "$test_cnt" == "$contc" ]; then
		LOG "At `date` Module-$dmodule wlan$wNum AutoTest SUCCESS"
	fi
done
wlanClosed
