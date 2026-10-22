# SPDX-License-Identifier: GPL-2.0-or-later

rtl8197f_is_cudy_ram() {
	case "$(cat /tmp/sysinfo/board_name 2>/dev/null)" in
		cudy,wr1500|cudy,re1500|cudy,m1500) return 0 ;;
	esac
	return 1
}

rtl8197f_is_mw5() {
	case "$(cat /tmp/sysinfo/board_name 2>/dev/null)" in
		tenda,nova-mw5|nova-mw5) return 0 ;;
		?*) return 1 ;;
	esac
	case "$(tr '\0' ' ' </proc/device-tree/compatible 2>/dev/null)" in
		*"tenda,nova-mw5 "*) return 0 ;;
	esac
	return 1
}
