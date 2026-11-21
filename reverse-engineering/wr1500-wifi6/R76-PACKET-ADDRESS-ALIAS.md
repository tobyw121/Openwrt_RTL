# R76 packet DMA pointer alias follow-up

64-byte burst is confirmed active (CPUICR=d4000000) but bad_tag689 persists.
It is not a validated fix. Original GPL rtl865xc_asicregs.h documents
DMA_CR0.HsbAddrMark (bits19:16): address[31:28] is replaced by
HsbAddrMark & address[31:28]. The VG SDK uses KSEG0 packet-buffer pointers;
our R76 rejection snapshot still showed KSEG1 (e.g.a2894000). Linux DMA
handles are physical allocation handles and are not the encoded OEM pointer.

Select the existing buffer-address alias helper for R76, as for RD05: packet
pointers become0x8..., ring/CDP addresses stay0xa..., original DMA handles
for mapping/allocation/free remain unchanged. No MAC guessing, tag repair,
parser relaxation, NAPT or firewall change. MW5 is explicitly excluded by
the helper and its existing policy is unchanged. This is a GPL-derived
hardware candidate and still requires validation; it is not a proven cure.
The preceding64-byte burst remains unchanged to isolate this address change.

PC rebuild:
```sh
cd ~/Schreibtisch/openwrt_git_RTL/Openwrt_RTL
git pull --ff-only origin MW5_Stable
make target/linux/clean
make -j"$(nproc)" V=s
```
Router UART38400 before/after the same load (WLAN disabled):
```sh
cat /proc/cudy-r76-rtknet
ethtool -S wan
```
If a rejected packet occurs, expected and descriptor DW1 should now use0x8...
while cdp/ring pointers remain0xa.... Send these values and drop counters.
PC current interface:
```sh
sudo ip addr replace 192.168.1.3/24 dev enx00e04c5562c0
curl --interface 192.168.1.3 -I --max-time 5 http://192.168.1.1/
```
Repeat after routing workload. No full MIPS build in this environment.
