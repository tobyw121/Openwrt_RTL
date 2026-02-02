# Copyright (C) 2011-2014 TP-LINK

. /lib/functions.sh
. /lib/network/network_arch.sh

NetItfInitFlag=0

# additional wan has the highest priority
addlwanmask=0

##########
lagif=""
lagmask=0
lag1portmap=0
lag2portmap=0

# lan interface
lan_ifs=""

get_wan_portseq()
{
	echo "$WAN_PORT_SET"
}

get_lan_portseq()
{
	echo "$LAN_PORT_SET"
}

get_lan_portnum()
{
	local type=$1
	local portnum=0

	portnum=$(eval "echo \"${LAN_PORT_SET}\" | awk '{print NF}'")

	echo $portnum
}

port_is_lanagg()
{
	local lanport=$1
	local tmpbit

	if [ -n "$lanport" ]; then
		tmpbit=$((1<<$lanport))
		if [ $((${lagmask}&tmpbit)) -ne 0 ]; then
			echo "1"
		else
			echo "0"
		fi
	else
		echo "0"
	fi 
}

port_is_wanagg()
{
	local lanport=$1
	local tmpbit

	if [ -n "$lanport" ]; then
		tmpbit=$((1<<$lanport))
		if [ $((${addlwanmask}&tmpbit)) -ne 0 ]; then
			echo "1"
		else
			echo "0"
		fi
	else
		echo "0"
	fi 
}


get_landev_by_portindex()
{
	local index=$1
	local portset
	local dev=""

	if [ -n "$index" -a $index -gt 0 ]; then
		dev=$(eval "echo \"${LAN_PORT_DEVSET}\" | awk '{print \$${index}}'")
	fi

	echo "$dev"
}

get_landev_by_port()
{
	local port=$1
	local index=1
	local dev=""

	if [ -n "${LAN_PORT_SET}" ] ; then
		for i in ${LAN_PORT_SET} ; do
			if [ $port -eq $i ]; then
				dev="$(get_landev_by_portindex $index)"
				break
			fi
			index=$((index+1))
		done
	fi

	echo "$dev"
}

get_lan_aggmask()
{
	echo $lagmask
}

get_lan_aggifs()
{
	echo "$lagif"
}

get_wan_aggmask()
{
	echo $addlwanmask
}

get_lan_initifs()
{
	echo "$lan_ifs"
}

get_wan_initifs()
{
	echo "$WAN_PORT_DEVSET"
}

check_port_inseq()
{
	local portseq=$1
	local port=$2
	local ismember=0

	if [ -n "$portseq" -a -n "$port" ]; then
		for i in $portseq ; do
			if [ "$i" = "$port" ]; then
				ismember=1
				break
			fi
		done
	fi

	echo "$ismember"
}

#
# Get and check aggregation ports from config file
#
get_aggport_set ()
{
	local lagnum=0
	local inseq

	############## get wan aggregation port
	# wagg enable/port
	local add_wan_enable=$(uci get switch.addl_wan.addl_wan_enable)
	local add_wan_port=$(uci get switch.addl_wan.addl_wan_port)

	#check wan additional agg logic port
	addlwanmask=0
	if [ "${add_wan_enable}" = "1" -a -n "${WAN_PORT_SET}" -a -n "${add_wan_port}" ]; then
		for i in ${add_wan_port} ; do
			inseq=$(check_port_inseq "${WAN_PORT_SET}" "$i")
			if [ $inseq -eq 1 -a $((addlwanmask&(1<<$i))) -eq 0 ] ; then
				addlwanmask=$((addlwanmask|(1<<$i)))
			else
				addlwanmask=0
				break
			fi
		done
	fi

	############## get lan aggregation ports
	# LAG enable/ports
	local agg_enable=$(uci get switch.lan_agg.enable_agg)
	local agg_ports=$(uci get switch.lan_agg.lacpports)

	# get LAG Mask
	lagmask=0
	if [ "${agg_enable}" = "1" -a -n "${agg_ports}" -a -n "${LAN_PORT_SET}" ] ; then
		for i in ${agg_ports} ; do
			inseq=$(check_port_inseq "${LAN_PORT_SET}" "$i")
			if [ $inseq -eq 1 -a $((lagmask&(1<<$i))) -eq 0 ] ; then
				lagnum=$((lagnum+1))
				lagmask=$((lagmask|(1<<$i)))
			else
				lagnum=0
				lagmask=0
				break
			fi
		done

		if [ $lagnum -eq 0 ]; then
			echo "***err: invalid lan agg ports" >&2
		fi
	fi

	lag1portmap=${lagmask}
	lag2portmap=0
}

#
# Get lan interface by ports and one group of aggregation ports has one interface
#
get_agg_interface()
{
	local portIndex=1
	local tmpif=""
	local tmpbit=0

	lagif=""
	if [ -n "${LAN_PORT_SET}" -a $lagmask -ne 0 ]; then
		for i in ${LAN_PORT_SET} ; do
			if [ $((${lagmask}&(1<<$i))) -ne 0 ] ; then
				if [ -n "$lagif" ]; then
					lagif="${lagif}${i}"
				else
					lagif="ethlaga${i}"
				fi
			fi

			portIndex=$((portIndex+1))
		done
	fi
}

#
# Get lan interface by ports and one group of aggregation ports has one interface
#
get_lan_interface()
{
	local index
	local portIndex=1
	local tmpif=""
	local tmpbit=0

	if [ -n "${LAN_PORT_SET}" ]; then
		for i in ${LAN_PORT_SET} ; do
			tmpbit=$((1<<$i))

			if [ $((${lagmask}&tmpbit)) -eq 0 -a $((${addlwanmask}&tmpbit)) -eq 0 ]; then
				# each lan port with one lan interface in internal switch
				tmpif="$(get_landev_by_portindex $portIndex)"
				if [ -n "$tmpif" ]; then
					list_contains lan_ifs "${tmpif}" || append lan_ifs "${tmpif}"
				fi
			fi

			portIndex=$((portIndex+1))
		done

		#add agg interface
		if [ -n "${lagif}" ]; then
			append lan_ifs "${lagif}"
		fi
	fi
}

get_network_interface ()
{
	if [ $NetItfInitFlag -ne 0 ] ; then
		return
	fi

	# get the lan agg and wan agg ports
	get_aggport_set

	# get agg lan interface
	get_agg_interface

	# get init lan interface
	get_lan_interface

	echo "Agglanmask/aggwanmask	: \"${lagmask}/${addlwanmask}\""
	echo "LAN interface			: \"${lan_ifs}\""

	NetItfInitFlag=1
}
