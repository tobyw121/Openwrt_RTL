#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later

case "$(cat /tmp/sysinfo/board_name 2>/dev/null)" in
	*tenda,nova-mw5*|*nova-mw5*)
		if [ -n "${PS1:-}" ]; then
			echo "MW5 v43.12: run 'mw5-netdiag' for the automatic LAN+WAN test."
			echo "Only move the Ethernet cable when prompted."
		fi
		;;
esac
