# R76 TCP control/data divergence: 64-byte DMA-burst candidate

Latest PC capture uses192.168.1.3, correct R76 destination MAC. SYN74 bytes,
SYN-ACK74 and ACK66 succeed. HEAD request141 bytes (75TCP payload bytes) is
retransmitted; router ACK stays at1, so the payload is not accepted by TCP.
Router bad_tag increases and retained bytes contain HTTP payload rather than
Ethernet headers. This distinguishes the observed failure from a completely
stopped switch and from merely unresponsive HTTP userspace. It does not prove
which hardware block damaged the frame.

The earlier supplied DMA128/129 hypothesis is now a better controlled A/B:
R76 DTS overrides the existing board-scoped burst property to64 bytes. CPU
DMA burst field bits29:28 should become1 rather than2; no MTU, RGMII, VLAN,
ring-layout, tag parser or forwarding/firewall changes. RAM and Fullflash
inherit this property. RD05/MW5 retain their original128-byte policy.
Coherent RX/TX stay enabled. This is an unverified hardware candidate, NOT a
claimed final fix. Remove the property to return to the preceding image.

Validation: all four relevant board-DTS files preprocess, override appears in
both R76 variants and not RD05/MW5. Five selected regressions pass. No DTC
binary/configured cross-build present; actual DTB/image/hardware need build
and test on the normal development PC.

PC build:
```sh
cd ~/Schreibtisch/openwrt_git_RTL/Openwrt_RTL
git pull --ff-only origin MW5_Stable
make target/linux/clean
make -j"$(nproc)" V=s
```
After installing and booting, leave WLAN disabled. Router UART38400:
```sh
dmesg | grep 'CPU-DMA burst'
cat /proc/cudy-r76-rtknet
```
Expect burst64; CPUICR bits29:28=1, normally d4000000 with other current flags.

PC on LAN, use the correct explicit source for an unambiguous comparison:
```sh
sudo ip addr replace 192.168.1.3/24 dev enx00e04c5562c0
sudo ip link set dev enx00e04c5562c0 mtu 1500
sudo tcpdump -ni enx00e04c5562c0 -e -s 128 -c 40 'host 192.168.1.1 and tcp port 80'
```
Second PC terminal:
```sh
curl --interface 192.168.1.3 -I --max-time 5 http://192.168.1.1/
```
After reproducing the same routed workload, router UART:
```sh
cat /proc/cudy-r76-rtknet
ethtool -S wan
```
Successful HEAD should advance router ACK beyond1 for the75 bytes, rather
than repeated data segments without acknowledgement. Compare drop counters,
not just ping. Repeated small-packet success alone is insufficient.
