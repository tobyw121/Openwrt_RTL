#!/bin/sh
[ -x /usr/bin/wg ] || exit 0

FW_LIBDIR=${FW_LIBDIR:-/lib/firewall}
. $FW_LIBDIR/fw.sh

[ -n "$INCLUDE_ONLY" ] || {
	. /lib/functions.sh
	. ../netifd-proto.sh
	init_proto "$@"
}

CONFIG_FILE="/var/etc/wireguard_client.conf"
ALLOWIPS=""

proto_wireguard_init_config() 
{
    proto_config_add_string "address"
    proto_config_add_string "private_key"
	proto_config_add_int    "listen_port"
    proto_config_add_string "public_key"
    proto_config_add_string "endpoint_address"
    proto_config_add_string "endpoint_port"
    proto_config_add_string "allowed_ips"
    proto_config_add_string "access"
    proto_config_add_string "preshared_key"
	proto_config_add_int    "persistent_keepalive"
	proto_config_add_int    "nat"

    available=1
	no_device=1
}

generate_wgconfig()
{
    local private_key
    local public_key
    local preshared_key
    local allowed_ips
    local persistent_keepalive
    local endpoint_address
    local endpoint_port

    config_get address $1 "address"
	config_get listen_port $1 "listen_port"
	config_get private_key $1 "private_key"
	config_get endpoint_address $1 "endpoint_address"
	config_get endpoint_port $1 "endpoint_port"
	config_get public_key $1 "public_key"
	config_get preshared_key $1 "preshared_key"
	config_get allowed_ips $1 "allowed_ips"
	config_get persistent_keepalive $1 "persistent_keepalive"
	config_get mtu $1 "mtu"

    [ -n "$listen_port" ] && echo -e "ListenPort = $listen_port" >>"$CONFIG_FILE"

	if [ "$private_key" != "" ];then
		echo -e "PrivateKey = $private_key\n" >>"$CONFIG_FILE"
	fi
	echo -e "[Peer]" >>"$CONFIG_FILE"
    [ -n "$public_key" ] && echo -e "PublicKey = $public_key" >>"$CONFIG_FILE"
	[ -n "$preshared_key" ] && echo -e "PresharedKey = $preshared_key" >>"$CONFIG_FILE"
	[ -n "$allowed_ips" ] && echo -e "AllowedIPs = $allowed_ips" >>"$CONFIG_FILE"
	ALLOWIPS=$allowed_ips
    if [ "$persistent_keepalive" == "" ];then
		echo -e "PersistentKeepalive = 25" >>"$CONFIG_FILE"
	else
		echo -e "PersistentKeepalive = $persistent_keepalive" >>"$CONFIG_FILE"
	fi

    [ -n "$endpoint_address" ] && echo -e "Endpoint = $endpoint_address:$endpoint_port" >>"$CONFIG_FILE"
}

init_config()
{
    local enabled
    local vpntype

    rm -rf $CONFIG_FILE
    config_load vpn
    config_get enabled "client" "enabled"
    config_get vpntype "client" "vpntype"

    if [ "$enabled" == "off" -o "$vpntype" != "wireguardvpn" ]; then
        echo "wireguard vpn if off, exit" > /dev/console
        exit 1
    fi

    ip link del dev wg_c 1>/dev/null 2>&1

    config_load network

    echo "before generate_wg_config" > /dev/console

    echo "[Interface]" >"${CONFIG_FILE}"
    generate_wgconfig vpn
}

proto_wireguard_setup() 
{
    echo "wireguard start setup" > /dev/console

    local address
    local listen_port
    local mtu
    
    echo "wireguard before init_config" > /dev/console

	init_config

    local load
	for module in wireguard; do
		grep -q "$module" /proc/modules && continue
		/sbin/insmod $module 2>&- >&-
		load=1
	done
	[ "$load" = "1" ] && sleep 1

    echo "wireguard before add wireguard interface" > /dev/console

	ip link add dev wg_c type wireguard
    ip addr add "$address" dev wg_c
	ip link set up dev wg_c

    if [ "$mtu" != "" ]; then
        ip link set mtu "$mtu" wg_c
    fi

    echo "wireguard before set wg_c " > /dev/console
    wg setconf wg_c $CONFIG_FILE
    runflag=`echo $?`
    if [ "$runflag" != 0 ]; then
        ip link del wg_c
        #rm -rf $CONFIG_FILE
        exit 1
    fi

    echo "wireguard before add route " > /dev/console

    if [ -n "$ALLOWIPS" ];then
        OLD_IFS="$IFS"
        IFS=','
        for var in $ALLOWIPS
        do
            ip route add "$var"  dev wg_c table vpn
        done
        IFS="$OLD_IFS"
    fi

    echo "wireguard client startup success, exit" > /dev/console
}

proto_wireguard_teardown() 
{
    echo "wireguard start teardown" > /dev/console

    ip link del wg_c
    rm -rf $CONFIG_FILE

    echo "wireguard client teardown success, exit" > /dev/console
}

[ -n "$INCLUDE_ONLY" ] || {
	add_protocol wireguard
}
