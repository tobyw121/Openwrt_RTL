# RD05 TX DMA buffer A/B diagnostic (introduced in 1.8.46)

The prior 1.8.45 trace showed valid-looking software ARP/DHCP and matching
PH_LEN/M_LEN (+4 FCS), but an empty external PCAP and increasing external CPU-port
FCS errors. The trace copied bytes before streaming DMA mapping and does not
establish the actual DMA fetch contents.

`rd05_tx_coherent` (default 0, root-only sysfs) is latched by ndo_open. Setting
it on a running device does not replace or free live buffers. RD05 reuses the
existing coherent TX pool allocator, single-descriptor copy, OWN/software-busy
reuse guards, memory barriers, and completion/free paths. Coherent slots have
no streaming mapping handle and are not passed to dma_unmap_single/page.
Allocation failure fails open; there is no silent streaming fallback.

An exact xiaomi,r4-rd05 board check and !is_mw5 gate select the new policy.
MW5 keeps its previous allocation policy. RX, RGMII, CRC lengths, destination
fields, ring sizes, factory data and firewall settings are unchanged. Existing
packet-address encoding still applies. Coherent buffers differ in address and
alignment as well as cache ownership; success does not uniquely prove one cache
bug. This is a diagnostic, not a demonstrated Ethernet fix or accelerator.

The request and latched policy are exposed as:

```
rd05_tx_buffer requested=1 active=1 copies=... stride=2048
```

Only compare while eth0 is running. `copies` is cumulative across reopens; use
deltas. Parameter changes require eth0 down/up, briefly interrupting Ethernet.
Use UART. Keep WAN disconnected during the comparison and do not change other
DMA/timing knobs. An A-B-A comparison controls for recovery caused by reopening
eth0 itself. Do not persist this experimental parameter in startup scripts yet.

## Build on PC

Use the existing configured RD05 tree:

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

If the profile check is empty select Realtek/RTL8197F/Xiaomi Mi WiFi R4 (RD05)
with `make menuconfig` first. For a build failure use `make -j1 V=s`.
Images: `bin/targets/realtek/rtl8197f/`. After flashing, `ethtool -i eth0`
must identify `1.8.47-rd05-dsa-auditfix` in the audit-fix source package.
The diagnostic policy is unchanged from 1.8.46; this version adds ownership
fixes and DSA policy checks, not a demonstrated DMA/FCS repair.

## PC capture (PC attached to LAN2)

```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
PCAP=$(mktemp "$HOME/rd05-dma-ab-XXXXXX.pcap")
echo "$PCAP"
sudo tcpdump -U -ni enx14ebb6300bb2 -s 256 -w - 'arp or icmp' > "$PCAP"
```

## OpenWrt router over UART

Check br-lan=192.168.1.1/24 first. Enter lines individually. For A:

```sh
ethtool -i eth0
ip -br address show br-lan
read saved_lvl rest < /proc/sys/kernel/printk
dmesg -n 1
echo 0 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_coherent
ip link set eth0 down
ip link set eth0 up
sleep 2
grep rd05_tx_buffer /proc/rd05-rtknet
ethtool -S lan2 > /tmp/rd05-A-before.txt
ip neigh flush to 192.168.1.3 dev br-lan
echo 8 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_snapshot_budget
ping -I br-lan -c 5 192.168.1.3
echo 0 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_snapshot_budget
ethtool -S lan2 > /tmp/rd05-A-after.txt
cat /proc/rd05-rtknet > /tmp/rd05-A-state.txt
dmesg > /tmp/rd05-A-dmesg.txt
```

Expect requested=0 active=0 for A. Then B:

```sh
echo 1 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_coherent
ip link set eth0 down
ip link set eth0 up
sleep 2
grep rd05_tx_buffer /proc/rd05-rtknet
ethtool -S lan2 > /tmp/rd05-B-before.txt
ip neigh flush to 192.168.1.3 dev br-lan
echo 8 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_snapshot_budget
ping -I br-lan -c 5 192.168.1.3
echo 0 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_snapshot_budget
ethtool -S lan2 > /tmp/rd05-B-after.txt
cat /proc/rd05-rtknet > /tmp/rd05-B-state.txt
dmesg > /tmp/rd05-B-dmesg.txt
```

B requires requested=1 active=1, nonzero stride, and increasing copies.
If reopening fails, save dmesg and restore A; do not infer a data-plane result
from a test where B did not allocate/open successfully.

Return to A and retest:

```sh
echo 0 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_coherent
ip link set eth0 down
ip link set eth0 up
sleep 2
grep rd05_tx_buffer /proc/rd05-rtknet
ethtool -S lan2 > /tmp/rd05-A2-before.txt
ip neigh flush to 192.168.1.3 dev br-lan
ping -I br-lan -c 5 192.168.1.3
ethtool -S lan2 > /tmp/rd05-A2-after.txt
cat /proc/rd05-rtknet > /tmp/rd05-A2-state.txt
dmesg -n "$saved_lvl"
```

Display results for UART transfer:

```sh
cat /tmp/rd05-A-before.txt /tmp/rd05-A-after.txt /tmp/rd05-A-state.txt
cat /tmp/rd05-B-before.txt /tmp/rd05-B-after.txt /tmp/rd05-B-state.txt
cat /tmp/rd05-A2-before.txt /tmp/rd05-A2-after.txt /tmp/rd05-A2-state.txt
grep -E 'RD05-TX-SNAPSHOT|RD05-TX-BYTES|coherent TX|failed' /tmp/rd05-B-dmesg.txt
```

Stop PC tcpdump with Ctrl+C and send its PCAP and UART log. Restore the console
loglevel even if a ping fails. Reconnect WAN after the experiment; `ifup wan`
starts DHCP again if needed. No OEM collector or module unload is involved.

Validation: native policy matrix exercises RD05 on/off, MW5 and other boards,
and verifies that changing the request does not itself change a latched active
value. Existing TX snapshot, DMA address/FIFO, RGMII, RD05 WAN and MW5 stability
regressions pass; the Ethernet object compiles for MIPS/Linux 6.6.114. No full
firmware build or physical A/B validation was performed in this workspace.
