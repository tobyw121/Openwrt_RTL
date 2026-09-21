#!/bin/sh
# SPDX-License-Identifier: GPL-2.0-or-later
case "$(cat /tmp/sysinfo/board_name 2>/dev/null)" in
	*tenda,nova-mw5*|*nova-mw5*)
		if [ -n "${PS1:-}" ]; then
			echo "OpenWrt Realtek V212 v44.66.12 transparent P0/RGMII + rtl8_4 + VID0-untag dataplane:"
			echo "RX0=900 TX0=768, 6-DWORD/24-byte descriptors, coherent RX/TX, CDP+OWN completion,"
			echo "read-only RTL8367 register/MIB/port/flow-control audit, software flow offload."
			echo "Default: SAFE/isolated routed Internet. v44.66.10 uses rtl8_4 over physical P0 in normal/transparent mode with descriptor DVID disabled. The v44.66.9 router-mode DVID9/8 path is diagnostic-only via /proc/rd05-rtknet."
			echo "Ethernet reference: v43.20 unchanged. SSH uses a per-boot tmpfs host key bound to logical LAN. WLAN: MW5 AMF/ADF receive gates + RXBD DMA alignment; ICTL routing retained."
		fi
		;;
esac
