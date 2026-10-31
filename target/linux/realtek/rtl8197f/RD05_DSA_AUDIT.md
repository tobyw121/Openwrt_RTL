# Adaptation of the uploaded RD05_DSA_Fix_v1.patch

Applied to MW5_Stable after the 1.8.46 coherent TX diagnostic. Driver version:
`1.8.47-rd05-dsa-auditfix`. This is a code/configuration audit update, not a
hardware-verified fix for missing wired RX or external CPU-port FCS errors.

## Adaptations and preserved behavior

- Use canonical package/ and target/ files. The uploaded duplicate feeds/base/
  copies do not exist in this checkout and are not created.
- Keep the coherent TX experiment and its default-off/reopen-only behavior.
  No descriptor fields, DMA address policy, ring size, GPIO or timing changes.
- Free skb on both failed linearization paths before NETDEV_TX_OK. This is a
  shared ownership bug fix, including the MW5 SG guard failure branch; successful
  MW5 processing and switch setup are unchanged. It does not explain failures
  of the small linear ARP packets in the recorded RD05 test.
- RD05 board.d always declares lan1+lan2 LAN and wan DHCP even if DSA is probing.
  eth0 is the conduit, not a LAN fallback. lan3/lan4 remain unbridged candidates.
  These labels remain a bring-up map, not a complete physical-jack proof.
- Remove the target-wide managed-switch bridge for RD05. Preserve factory
  identities through the existing board.d MAC providers: eth0 and LAN from
  ethaddr, WAN from ethaddr_wan, including Bdata fallbacks. config_generate
  consumes the network-device MAC fields. The test now explicitly checks these
  emitted MAC assignments as well as the RD05/MW5 topology separation.
- 99-rd05-dsa-network leaves an existing nonempty network configuration alone,
  generates one only when absent, and never deletes device/DHCP sections or
  replaces ULA/custom addresses. Restored old/wrong bridge layouts therefore
  remain and must be diagnosed, not silently rewritten. No reset is requested.
- Existing WAN broadcast settings are preserved. Fresh generated DHCP defaults
  no longer forcibly set broadcast=1. This option is not a fix for bad wire FCS.
  To restore that optional workaround explicitly on RD05 when needed:

```sh
uci set network.wan.broadcast='1'
uci commit network
ifup wan
```

- Patch 340 verifies CPU mask, active-port isolation and PVID after normal RD05
  bootstrap. Read errors propagate and mismatches return EIO; stopping CPU7 is
  attempted on failure and failure of that attempt is logged. The existing exact
  board/family-D gate remains. Diagnostic modes are reported as unverified.
  This is not a VLAN-table readback or physical isolation/packet-delivery proof.
- rtl8367dctl refuses bridge-init/VLAN/PVID writes on RD05 while retaining
  diagnostic display. Other-board VLAN argument validation and error propagation
  are improved. Multistep writes on those boards are not transactional.
- New rd05-dsa-check is display-only: topology, selected non-password UCI values,
  RD05 parameters, driver counters and boot messages. Exit 1 means topology
  warnings, not automatically bad hardware; exit 0 is not a hardware PASS.

## Verification

13 adapted regression tests from the supplied patch, plus 26 existing selected
RD05/MW5 regression tests pass (39 total). The duplicate-feed equality test is
removed because those files are not part of our tree. The supplied static test
result text is not imported as evidence. Ethernet and the switch with patch 340
compile for MIPS against Linux 6.6.114. Patch 340 applied without fuzz to the
prepared switch source. A complete OpenWrt firmware build and on-device test
have not been performed here.

```sh
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=scripts/tests python3 -m unittest test_rd05_dsa_auditfix test_rd05_tx_snapshot test_rd05_rgmii_sequence test_rd05_oem_switch_profile test_rd05_oem_dma test_rtl8197f_rd05_wan test_mw5_stability test_mw5_watchdog
```

## PC build

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git switch MW5_Stable
git pull --ff-only origin MW5_Stable
make defconfig
grep '^CONFIG_TARGET_realtek_rtl8197f_DEVICE_xiaomi_r4_rd05=y' .config
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

If the profile check is empty, select Realtek/RTL8197F/Xiaomi Mi WiFi R4 (RD05)
with make menuconfig before building. Diagnose a failure with make -j1 V=s.
Images are in bin/targets/realtek/rtl8197f/. Do not erase existing settings solely
to test this change; the checker can first reveal preserved topology problems.

## OpenWrt UART and PC test

Router, one line at a time, with PC at LAN2 and WAN temporarily unplugged:

```sh
ethtool -i eth0
rd05-dsa-check --ports
rd05-dsa-check > /tmp/rd05-dsa-check.txt 2>&1
ethtool -S lan2 > /tmp/rd05-audit-before.txt
```

PC terminal 1 (normal br-lan address 192.168.1.1):

```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
PCAP=$(mktemp "$HOME/rd05-audit-XXXXXX.pcap")
echo "$PCAP"
sudo tcpdump -U -ni enx14ebb6300bb2 -s 256 -w - 'arp or icmp' > "$PCAP"
```

PC terminal 2:

```sh
sudo ip neigh flush to 192.168.1.1 dev enx14ebb6300bb2
sudo arping -I enx14ebb6300bb2 -S 192.168.1.3 -c 10 192.168.1.1
ping -I 192.168.1.3 -c 5 192.168.1.1
```

Then router:

```sh
ethtool -S lan2 > /tmp/rd05-audit-after.txt
cat /tmp/rd05-dsa-check.txt
cat /tmp/rd05-audit-before.txt /tmp/rd05-audit-after.txt
```

Stop PC tcpdump with Ctrl+C and send PCAP plus UART output. Reconnect WAN and
check ifstatus wan and ip route. For the independent coherent-TX comparison,
follow RD05_TX_DMA_AB.md, including A-B-A rollback; do not combine unrelated
new knob changes. No OEM collector is involved in these OpenWrt checks.
