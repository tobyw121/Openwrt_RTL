# RD05/R76 Ethernet parser hardening — MW5 excluded

Scope is explicitly !is_mw5 AND (xiaomi,r4-rd05 OR cudy,ax1500-r76).
Existing MW5 tag repair, destination-MAC fallback, acceleration parser and
TX-timeout branch remain unchanged. The previously identified TCP/UDP checks
in the MW5-only accelerator are deliberately NOT modified under this scope.
No acceleration is enabled, no forwarding decision or conntrack bypass added.

Changes:
- Existing OWN/stable descriptor/address/length/FIRST/LAST/buffer backend
  checks retained; scoped DMA read barrier and bad-descriptor counter added.
- RD05/R76 copy up to160 header bytes through skb_copy_bits, handling
  non-linear skbs without unguarded direct header reads. Validate rtl8_4
  protocol and source physical port (RD05 0..4, R76 0..3), no DA guessing.
- Recognize VLAN/QinQ (two levels), PPPoE and unknown EtherTypes; retain
  Linux processing for IPv6, ARP, PPPoE, further encapsulations and fragments.
  No application-layer DHCP/DNS/ARP parser or proprietary forwarding added.
- IPv4 header/total length, TCP data offset and UDP length checked before
  delivering malformed frames. Ethernet padding remains legal. No TCP/UDP
  checksum or IP fragment reassembly in the driver.
- Scoped counters in existing proc output: bad_desc/bad_tag/bad_port/bad_l3,
  vlan/pppoe and TX retries. Unsupported protocols are not classified as bad.
- On RD05/R76 timeout: stop queue, reclaim confirmed TX completions, re-kick
  pending TX, schedule NAPI, wake only with adequate space. No live RX-ring,
  VLAN, L2/ACL table reset. This is not a guarantee of recovering a hard hang.

Validation: compiled production parser with UBSan exercises short headers,
invalid ports, IP/UDP lengths, TCP offsets, QinQ, PPPoE, fragments and IPv6.
Board guard tested including a forced MW5 flag with matching RD05/R76 DT.
Existing MW5 helper definitions compared to pre-change commit0df222f142.
No configured MIPS kernel build available; full firmware/hardware validation
is still required. Ring layout and port mapping are not changed by this work.

PC build WR1500:
```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make -j"$(nproc)" V=s
```
For RD05 instead copy configs/rtl8197f_rd05_defconfig before make defconfig.
Router UART: WR1500 38400, RD05 115200; commands individually:
```sh
cat /proc/cudy-r76-rtknet
```
RD05 uses /proc/rd05-rtknet instead.
```sh
ifstatus wan
ethtool -S wan
```
PC wired to LAN, Fritzbox at WAN:
```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
sudo arping -I enx14ebb6300bb2 -S 192.168.1.3 -c 10 192.168.1.1
ping -I 192.168.1.3 -c 10 192.168.1.1
```
Compare proc counters before/after. Bad-tag/port counters increasing on known
valid traffic require wire/descriptor inspection, not disabling validation.
