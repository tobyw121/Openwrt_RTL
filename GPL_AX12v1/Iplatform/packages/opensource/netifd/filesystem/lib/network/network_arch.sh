# Copyright (C) 2011-2014 TP-LINK

GMAC3_ENABLE=

# Set of lan ports: LAN1->1 LAN2->2 LAN3->3 LAN4->4, order of LAN port
LAN_PORT_SET="1 2 3 4"
# Set of lan phy ports: LAN1->phyport1 LAN2->phyport2 LAN3->phyport3 LAN4->phyport4
LAN_PHY_PORT_SET="1:1 2:2 3:3 4:4"
# Set of device for lan ports: LAN1->eth0.1 LAN2->eth0.2 LAN3->eth0.3 LAN4->eth0.4
LAN_PORT_DEVSET="eth0.2 eth0.2 eth0.2 eth0.2"
# Set of wan ports: WAN1->1
WAN_PORT_SET="1"
# Set of wan phy ports: WAN1->phyport5
WAN_PHY_PORT_SET="1:0"
# Set of device for lan ports: WAN1->1
WAN_PORT_DEVSET="eth1.1"
# Set of connecting CPU phy ports, more than one is possible
CPU1_PHY_PORT_SET="5"
# Set of another connecting CPU phy ports, more than one is possible
CPU2_PHY_PORT_SET="6 7"
# Set of WAN defualt VLAN ID
WAN_DEFUALT_VID="1"
# Set of LAN defualt VLAN ID
LAN_DEFUALT_VID="2 3 4 5"

get_lan_phyport_bylogic()
{
	local lport=$1
	local phyport=""
	local index=1

	if [ -n "$lport" ]; then
		for i in ${LAN_PORT_SET} ; do
			if [ $lport -eq $i ]; then
				phyport=$(eval "echo \"${LAN_PHY_PORT_SET}\" | awk '{print \$${index}}'")
				break
			fi
			index=$((index+1))
		done
	fi

	echo "${phyport##*:}"
}

get_wan_phyportseq()
{
	local wport=1
	local phyport=""
	local index=1
	
	if [ -n "$1" ]; then
		wport=$1
	fi

	if [ -n "$wport" ]; then
		for i in ${WAN_PORT_SET} ; do
			if [ $wport -eq $i ]; then
				phyport=$(eval "echo \"${WAN_PHY_PORT_SET}\" | awk '{print \$${index}}'")
				break
			fi
			index=$((index+1))
		done
	fi

	echo "${phyport##*:}"
}

get_phyport_byindex()
{
	local index=$1
	local phyport=""

	if [ -n "$index" ]; then
		phyport=$(eval "echo \"${LAN_PHY_PORT_SET}\" | awk '{print \$${index}}'")
	fi

	echo "$phyport"
}
