# Integration of supplied337–351 bundle, 2026-11-02

337–342 (seven files including both337 files) are byte-identical to this tree.
Do not reapply them. The new end state is consolidated into343 after342:
LAN ports0/1/3 VID1, WAN port2 VID2, CPU7 shared, stale VIDs8/9 cleared,
CPU7 exempt from VLAN ingress checks. Port isolation/readback is preserved.
This is a hardware candidate, not validated WAN traffic or proven isolation.

Adaptations to the supplied experiments:
- WAN ingress filtering stays ON by default. `rtl8365mb.r76_wan_ingress_bypass=1`
  is a setup-time, read-only parameter for isolated A/B tests only.
- Extra PHY2/OCP snapshots are OFF by default, enabled with
  `r76_phy_diagnostics=1`. Locked OCP snapshots retain map_lock and use
  map_nolock. PHY2 pre-command address read is preserved.
- Every CVLAN command is written explicitly, with busy polling. Repeated
  identical commands cannot disappear through regmap_update_bits optimization.
- Supplied345 final log context and349 PHY-read context did not match; rebuilt
  from actual prepared source. Do not install the original343–351 additionally.
- RD05/MW5 branches, RGMII, DMA, IRQ and firewall are untouched.
- User VLAN/bridge reconfiguration and bidirectional CPU-to-WAN tagging still
  require hardware validation; static membership is not a complete DSA VLAN API.

Validation: consolidated343 applies without fuzz to prepared342 source.
Earlier existing series required upstream-context fuzz when reproduced against
plain Linux6.6.114 rather than the complete OpenWrt kernel preparation.
Patched DSA driver compiled as a MIPS Linux6.6.114 object (reference build
supplied existing protocol28 constant; no new tagger protocol). This does not
constitute a full OpenWrt firmware build. Production functions tested with a
faulting regmap model, PVIDs, port isolation, ingress defaults/opt-in bypass.

PC build:
```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make -j"$(nproc)" V=s
```
Router via UART38400, commands individually after installing the new image:
```sh
dmesg | grep -E 'R76|CVLAN|OCP|PHY2'
ifstatus wan
ip route
ethtool -S wan
```
PC connected to LAN, Fritzbox at WAN; tcpdump in a separate PC terminal:
```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
sudo tcpdump -ni enx14ebb6300bb2 -e 'arp or (udp port 67 or 68)'
```
PC second terminal:
```sh
sudo arping -I enx14ebb6300bb2 -S 192.168.1.3 -c 10 192.168.1.1
ping -I 192.168.1.3 -c 10 192.168.1.1
```
Check LAN never receives Fritzbox DHCP replies, then router WAN obtains DHCP.
This single check is insufficient to prove isolation against tagged/injected
frames; test before using untrusted WAN. UART recovery remains necessary.
