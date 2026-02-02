. /usr/share/libubox/jshn.sh

append() {
	local var="$1"
	local value="$2"
	local sep="${3:- }"

	eval "export -- \"$var=\${$var:+\${$var}\${value:+\$sep}}\$value\""
}

proto_config_add_generic() {
	json_add_array ""
	json_add_string "" "$1"
	json_add_int "" "$2"
	json_close_array
}

proto_config_add_int() {
	proto_config_add_generic "$1" 5
}

proto_config_add_string() {
	proto_config_add_generic "$1" 3
}

proto_config_add_boolean() {
	proto_config_add_generic "$1" 7
}

add_default_handler() {
	case "$(type $1 2>/dev/null)" in
		*function*) return;;
		*) eval "$1() { return; }"
	esac
}

_proto_do_teardown() {
	json_load "$data"
	eval "proto_$1_teardown \"$interface\" \"$ifname\""
}

_proto_do_setup() {
	json_load "$data"
	_EXPORT_VAR=0
	_EXPORT_VARS=
	eval "proto_$1_setup \"$interface\" \"$ifname\""
}

proto_init_update() {
	local ifname="$1"
	local up="$2"
	local external="$3"

	PROTO_KEEP=0
	PROTO_INIT=1
	PROTO_TUNNEL_OPEN=
	PROTO_IPADDR=
	PROTO_IP6ADDR=
	PROTO_ROUTE=
	PROTO_ROUTE6=
	PROTO_PREFIX6=
	PROTO_DNS=
	PROTO_DNS_SEARCH=
	json_init
	json_add_int action 0
	[ -n "$ifname" -a "*" != "$ifname" ] && json_add_string "ifname" "$ifname"
	json_add_boolean "link-up" "$up"
	[ -n "$3" ] && json_add_boolean "address-external" "$external"
}

proto_set_keep() {
	PROTO_KEEP="$1"
}

proto_close_nested() {
	[ -n "$PROTO_NESTED_OPEN" ] && json_close_object
	PROTO_NESTED_OPEN=
}

proto_add_nested() {
	PROTO_NESTED_OPEN=1
	json_add_object "$1"
}

proto_add_tunnel() {
	proto_add_nested "tunnel"
}

proto_close_tunnel() {
	proto_close_nested
}

proto_add_data() {
	proto_add_nested "data"
}

proto_close_data() {
	proto_close_nested
}

proto_add_dns_server() {
	local address="$1"

	append PROTO_DNS "$address"
}

proto_add_dns_search() {
	local address="$1"

	append PROTO_DNS_SEARCH "$address"
}

proto_add_ipv4_address() {
	local address="$1"
	local mask="$2"
	local broadcast="$3"
	local ptp="$4"

	append PROTO_IPADDR "$address/$mask/$broadcast/$ptp"
}

proto_add_ipv6_address() {
	local address="$1"
	local mask="$2"
	local preferred="$3"
	local valid="$4"
	local offlink="$5"

	append PROTO_IP6ADDR "$address/$mask/$preferred/$valid/$offlink"
}

proto_add_ipv4_route() {
	local target="$1"
	local mask="$2"
	local gw="$3"
	local route_pre="$4"

	if [ -n "$route_pre" ]; then
		append PROTO_ROUTE "$target/$mask/$gw///$route_pre//"
	else
		append PROTO_ROUTE "$target/$mask/$gw//"
	fi
}

proto_add_ipv6_route() {
	local target="$1"
	local mask="$2"
	local gw="$3"
	local metric="$4"
	local valid="$5"

	append PROTO_ROUTE6 "$target/$mask/$gw/$metric/$valid"
}

proto_add_ipv6_prefix() {
	local prefix="$1"
	local valid="$2"
	local preferred="$3"

	if [ -z "$valid" ]; then
		append PROTO_PREFIX6 "$prefix"
	else
		[ -z "$preferred" ] && preferred="$valid"
		append PROTO_PREFIX6 "$prefix,$valid,$preferred"
	fi
}

_proto_push_ipv4_addr() {
	local str="$1"
	local address mask broadcast ptp

	address="${str%%/*}"
	str="${str#*/}"
	mask="${str%%/*}"
	str="${str#*/}"
	broadcast="${str%%/*}"
	str="${str#*/}"
	ptp="$str"

	json_add_object ""
	json_add_string ipaddr "$address"
	[ -n "$mask" ] && json_add_string mask "$mask"
	[ -n "$broadcast" ] && json_add_string broadcast "$broadcast"
	[ -n "$ptp" ] && json_add_string ptp "$ptp"
	json_close_object
}

_proto_push_ipv6_addr() {
	local str="$1"
	local address mask preferred valid offlink

	address="${str%%/*}"
	str="${str#*/}"
	mask="${str%%/*}"
	str="${str#*/}"
	preferred="${str%%/*}"
	str="${str#*/}"
	valid="${str%%/*}"
	str="${str#*/}"
	offlink="${str%%/*}"

	json_add_object ""
	json_add_string ipaddr "$address"
	[ -n "$mask" ] && json_add_string mask "$mask"
	[ -n "$preferred" ] && json_add_int preferred "$preferred"
	[ -n "$valid" ] && json_add_int valid "$valid"
	[ -n "$offlink" ] && json_add_boolean offlink "$offlink"
	json_close_object
}

_proto_push_string() {
	json_add_string "" "$1"
}

_proto_push_route() {
	local str="$1";
	local target="${str%%/*}"
	str="${str#*/}"
	local mask="${str%%/*}"
	str="${str#*/}"
	local gw="${str%%/*}"
	str="${str#*/}"
	local metric="${str%%/*}"
	str="${str#*/}"
	local valid="${str%%/*}"
	str="${str#*/}"
	local route_pre="${str%%/*}"

	json_add_object ""
	json_add_string target "$target"
	json_add_string netmask "$mask"
	[ -n "$gw" ] && json_add_string gateway "$gw"
	[ -n "$metric" ] && json_add_int metric "$metric"
	[ -n "$valid" ] && json_add_int valid "$valid"
	[ -n "$route_pre" ] && json_add_boolean route_pre "$route_pre"
	json_close_object
}

_proto_push_array() {
	local name="$1"
	local val="$2"
	local cb="$3"

	[ -n "$val" ] || return 0
	json_add_array "$name"
	for item in $val; do
		eval "$cb \"\$item\""
	done
	json_close_array
}

_proto_notify() {
	local interface="$1"
	local options="$2"
	json_add_string "interface" "$interface"
	ubus $options call network.interface notify_proto "$(json_dump)"
}

proto_send_update() {
	local interface="$1"

	proto_close_nested
	json_add_boolean keep "$PROTO_KEEP"
	_proto_push_array "ipaddr" "$PROTO_IPADDR" _proto_push_ipv4_addr
	_proto_push_array "ip6addr" "$PROTO_IP6ADDR" _proto_push_ipv6_addr
	_proto_push_array "routes" "$PROTO_ROUTE" _proto_push_route
	_proto_push_array "routes6" "$PROTO_ROUTE6" _proto_push_route
	_proto_push_array "ip6prefix" "$PROTO_PREFIX6" _proto_push_string
	_proto_push_array "dns" "$PROTO_DNS" _proto_push_string
	_proto_push_array "dns_search" "$PROTO_DNS_SEARCH" _proto_push_string
	_proto_notify "$interface"
}

proto_export() {
	local var="VAR${_EXPORT_VAR}"
	_EXPORT_VAR="$(($_EXPORT_VAR + 1))"
	export -- "$var=$1"
	append _EXPORT_VARS "$var"
}

proto_run_command() {
	local interface="$1"; shift

	json_init
	json_add_int action 1
	json_add_array command
	while [ $# -gt 0 ]; do
		json_add_string "" "$1"
		shift
	done
	json_close_array
	[ -n "$_EXPORT_VARS" ] && {
		json_add_array env
		for var in $_EXPORT_VARS; do
			eval "json_add_string \"\" \"\${$var}\""
		done
		json_close_array
	}
	_proto_notify "$interface"
}

proto_kill_command() {
	local interface="$1"; shift

	json_init
	json_add_int action 2
	[ -n "$1" ] && json_add_int signal "$1"
	_proto_notify "$interface"
}

proto_notify_error() {
	local interface="$1"; shift

	json_init
	json_add_int action 3
	json_add_array error
	while [ $# -gt 0 ]; do
		json_add_string "" "$1"
		shift
	done
	json_close_array
	_proto_notify "$interface"
}

proto_block_restart() {
	local interface="$1"; shift

	json_init
	json_add_int action 4
	_proto_notify "$interface"
}

proto_set_available() {
	local interface="$1"
	local state="$2"
	json_init
	json_add_int action 5
	json_add_boolean available "$state"
	_proto_notify "$interface"
}

proto_add_host_dependency() {
	local interface="$1"
	local host="$2"
	local ifname="$3"

	# execute in subshell to not taint callers env
	# see tickets #11046, #11545, #11570
	(
		json_init
		json_add_int action 6
		json_add_string host "$host"
		[ -n "$ifname" ] && json_add_string ifname "$ifname"
		_proto_notify "$interface" -S
	)
}

proto_setup_failed() {
	local interface="$1"
	json_init
	json_add_int action 7
	_proto_notify "$interface"
}

proto_setup_ipv6_acl() {
	local lanv6_ifname
	local wanv6_ifname
	local lanv6_base_name
	local wanv6_base_name
	local ipv6_acl_existed
	local ipv6_proto
	local lanv6_proto

	config_load /etc/config/network
	config_get wanv6_ifname wanv6 ifname
	config_get lanv6_ifname wanv6 lanif
	config_get ipv6_proto wanv6 proto
	config_get lanv6_proto lanv6 proto

	if [ -n "${ipv6_proto}" -a "${ipv6_proto}" == "passthrough" ]; then
		json_init
		json_load "`ubus call network.interface.wan status`"
		json_get_var wanv6_base_name device
		json_load "`ubus call network.interface.lan status`"
		json_get_var lanv6_base_name device
	else
		lanv6_base_name=$(echo ${lanv6_ifname} | grep -Eo "br-lan")
		wanv6_base_name=$(echo ${wanv6_ifname} | grep -Eo "eth[0-9]")
	fi

	if [ -z "${lanv6_base_name}" ]; then
		lanv6_base_name="br-lan"
	fi

	if [ -z "${wanv6_base_name}" ]; then
		wanv6_base_name="eth1"
	fi

	ipv6_acl_existed=`cat /proc/rtl865x/soft_aclChains | grep -E "\-40003|\-40002"`
	if [ -n "${ipv6_acl_existed}" ]; then
		echo "unReg ${lanv6_base_name} -40003 in" > /proc/rtl865x/acl
		echo "unReg ${lanv6_base_name} -40002 in" > /proc/rtl865x/acl
		echo "unReg ${wanv6_base_name} -40003 in" > /proc/rtl865x/acl
		echo "unReg ${wanv6_base_name} -40002 in" > /proc/rtl865x/acl
		echo "0" > /proc/ipv6_fast_forward
	fi
	ipv6_acl_existed=`cat /proc/rtl865x/soft_aclChains | grep -E "\-40001|\-40000"`
	if [ -n "${ipv6_acl_existed}" ]; then
		echo "unReg ${lanv6_base_name} -40001 in" >/proc/rtl865x/acl
		echo "unReg ${lanv6_base_name} -40000 in" >/proc/rtl865x/acl
		echo "unReg ${wanv6_base_name} -40001 in" >/proc/rtl865x/acl
		echo "unReg ${wanv6_base_name} -40000 in" >/proc/rtl865x/acl
		echo "0" > /proc/ipv6_fast_forward
	fi

	echo "1" > /proc/ipv6_fast_forward
	if [ -n "${ipv6_proto}" -a "${ipv6_proto}" == "passthrough" ]; then
		echo "reg ${lanv6_base_name} -40001 in" > /proc/rtl865x/acl
		echo "add netif ${lanv6_base_name} chain -40001 act permit type srcfilter rule srcPhyport 100 proto l2" > /proc/rtl865x/acl
		#trap ipv6 to cpu
		echo "reg ${lanv6_base_name} -40000 in" >/proc/rtl865x/acl
		echo "add netif ${lanv6_base_name} chain -40000 act toCPU type mac rule ethType 86dd ffff" >/proc/rtl865x/acl
		#permit packets rx from cpu
		echo "reg ${wanv6_base_name} -40001 in" > /proc/rtl865x/acl
		echo "add netif ${wanv6_base_name} chain -40001 act permit type srcfilter rule srcPhyport 100 proto l2" > /proc/rtl865x/acl
		#trap ipv6 to cpu
		echo "reg ${wanv6_base_name} -40000 in" >/proc/rtl865x/acl
		echo "add netif ${wanv6_base_name} chain -40000 act toCPU type mac rule ethType 86dd ffff" >/proc/rtl865x/acl
		return 0
	fi

	echo "reg ${wanv6_base_name} -40003 in" > /proc/rtl865x/acl
	#permit packets from cpu port
	echo "add netif ${wanv6_base_name} chain -40003 act permit type srcfilter rule srcPhyport 100 proto l2" > /proc/rtl865x/acl

	echo "reg ${wanv6_base_name} -40002 in" > /proc/rtl865x/acl
	if [ -n "${lanv6_proto}" -a "${lanv6_proto}" == "ndp" ]; then
		#trap all ipv6 packets from wan to CPU when use ndproxy
		echo "add netif ${wanv6_base_name} chain -40002 act toCPU layer l3_l4 type ipv6 rule nheader 00 00" > /proc/rtl865x/acl
	else
		#trap all ipv6 udp packets to cpu
		echo "add netif ${wanv6_base_name} chain -40002 act toCPU layer l3_l4 type ipv6 rule nheader 11 FF" > /proc/rtl865x/acl
		#trap icmp packets to cpu
		echo "add netif ${wanv6_base_name} chain -40002 act toCPU layer l3_l4 type ipv6 rule nheader 3A FF" > /proc/rtl865x/acl
		#trap tcp sync+ack to cpu
		echo "add netif ${wanv6_base_name} chain -40002 act toCPU layer l3_l4 type tcp rule flags 2 2" > /proc/rtl865x/acl
	fi

	echo "reg ${lanv6_base_name} -40003 in" > /proc/rtl865x/acl
	#permit packets from cpu port
	echo "add netif ${lanv6_base_name} chain -40003 act permit type srcfilter rule srcPhyport 100 proto l2" > /proc/rtl865x/acl

	echo "reg ${lanv6_base_name} -40002 in" > /proc/rtl865x/acl
	if [ -n "${lanv6_proto}" -a "${lanv6_proto}" == "ndp" ]; then
		#trap all ipv6 packets from wan to CPU when use ndproxy
		echo "add netif ${lanv6_base_name} chain -40002 act toCPU layer l3_l4 type ipv6 rule nheader 00 00" > /proc/rtl865x/acl
	else
		#trap all ipv6 udp packets to cpu
		echo "add netif ${lanv6_base_name} chain -40002 act toCPU layer l3_l4 type ipv6 rule nheader 11 FF" > /proc/rtl865x/acl
		#trap icmp packets to cpu
		echo "add netif ${lanv6_base_name} chain -40002 act toCPU layer l3_l4 type ipv6 rule nheader 3A FF" > /proc/rtl865x/acl
		#trap tcp sync to cpu
		echo "add netif ${lanv6_base_name} chain -40002 act toCPU layer l3_l4 type tcp rule flags 2 2" > /proc/rtl865x/acl
	fi
}

init_proto() {
	proto="$1"; shift
	cmd="$1"; shift

	case "$cmd" in
		dump)
			add_protocol() {
				no_device=0
				available=0

				add_default_handler "proto_$1_init_config"

				json_init
				json_add_string "name" "$1"
				json_add_array "config"
				eval "proto_$1_init_config"
				json_close_array
				json_add_boolean no-device "$no_device"
				json_add_boolean available "$available"
				json_dump
			}
		;;
		setup|teardown)
			interface="$1"; shift
			data="$1"; shift
			ifname="$1"; shift

			add_protocol() {
				[[ "$proto" == "$1" ]] || return 0

				case "$cmd" in
					setup) _proto_do_setup "$1";;
					teardown) _proto_do_teardown "$1" ;;
					*) return 1 ;;
				esac
			}
		;;
	esac
}
