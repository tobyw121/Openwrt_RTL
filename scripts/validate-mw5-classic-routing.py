#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Static regression guard for MW5 classic OpenWrt LAN/WAN routing."""
from pathlib import Path
import sys

TOP = Path(__file__).resolve().parents[1]
RTK = TOP / "target/linux/realtek/files-6.6/drivers/net/ethernet/rtl8197f_rtknet.c"
NET = TOP / "target/linux/realtek/base-files/etc/uci-defaults/99-mw5-dsa-network"
FW = TOP / "target/linux/realtek/base-files/etc/uci-defaults/96-mw5-router-firewall"
TAG = TOP / "target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c"

errors = []
rtk = RTK.read_text()
net = NET.read_text()
fw = FW.read_text()
tag = TAG.read_text()

def need(text, needle, label):
    if needle not in text:
        errors.append(f"{label}: missing {needle!r}")

# The MW5 eth0 master is a pure DSA conduit. Never allow legacy descriptor
# HWLOOKUP to override DP=P0. Normal MW5 DSA traffic carries an RTL8367 wire CPU tag over P0.
need(rtk, "if (priv->is_mw5 && txm.hwlookup)", "rtknet DSA HWLOOKUP guard")
need(rtk, "txm.hwlookup = false;", "rtknet DSA HWLOOKUP guard")
need(rtk, "txm.bridge = false;", "rtknet DSA HWLOOKUP guard")
need(rtk, "priv->tx_port_mask = BIT(0);", "rtknet MW5 direct P0")
need(rtk, "priv->tx_dp_ext = 0;", "rtknet MW5 direct P0")

# MW5 uses the RTL8197F master only as the physical P0/RGMII conduit.
# External user-port selection belongs in the RTL8367 CPU-to-switch tag.
need(tag, "static unsigned int rtl4_9_tx_layout = RTL4_9_TX_LAYOUT_PROTO9;",
     "rtl4_9 protocol9 + P0 default")

# Classic OpenWrt router topology: LAN bridge and separate WAN DHCP client.
need(net, "add_list network.br_lan.ports='lan'", "MW5 LAN bridge")
need(net, "set network.lan.ipaddr='192.168.1.1'", "MW5 LAN address")
need(net, "set network.wan.device='wan'", "MW5 WAN device")
need(net, "set network.wan.proto='dhcp'", "MW5 WAN DHCP")
need(net, "set dhcp.lan.interface='lan'", "MW5 LAN DHCP server")
need(net, "set dhcp.wan.ignore='1'", "MW5 WAN DHCP-server isolation")

# Firewall/NAT must remain standard lan -> wan routing.
need(fw, "repair-firewall", "MW5 firewall repair")
# Firewall/conntrack remain authoritative. Software nft flow offload is allowed,
# while hardware flowtable/NAPT stays disabled until DMA/DSA is hardware-stable.
need(fw, "flow_offloading='1'", "MW5 software flow offload default")
need(fw, "flow_offloading_hw='0'", "MW5 hardware flow offload disabled")

if errors:
    print("MW5 classic routing validation FAILED", file=sys.stderr)
    for error in errors:
        print("  - " + error, file=sys.stderr)
    sys.exit(1)
print("MW5 classic routing validation OK")
