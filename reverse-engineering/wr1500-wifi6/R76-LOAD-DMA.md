# R76 load failure: coherent packet buffers — 2026-11-21

User log from boot r0-3f1e722980: normal ping still responds, SSH/Internet
fail during traffic. No demonstrated complete switch crash. Before load:
699 RX /410 TX, no drops. Later3270 RX completions,2469 delivered,801 drops:
798 bad_tag,3 bad_l3. Ring_full, TX timeouts, DMA descriptor errors and runouts
remain zero. Both coherent_rx/coherent_tx are0; acceleration is disabled.
The current log does not establish the exact cause of malformed headers.

R76 only now selects the existing persistent coherent RX pool and per-slot
coherent TX buffers at probe. Page pool is disabled for that board. This
avoids cacheline-sharing / streaming skb DMA ownership in this failure path.
Descriptor geometry, CPU-tag format, port map, checksums, firewall and switch
VLANs stay unchanged. No automatic live ring reset or MAC-based tag repair.
MW5 policy/helpers and RD05 buffer selection are unchanged.

RD05/R76 rejected frames retain one bounded24-byte header snapshot and length
in the existing proc output (`routed_last_bad`). This is not an unbounded
packet logger and does not relax invalid-tag drops. If drops remain after
coherent buffers, compare that header with the expected rtl8_4 layout before
changing the tag parser or PHY timings. Invalid-tag counters alone do not
prove cache corruption, and this candidate is NOT a hardware-validated fix.

Tests: existing production parser UBSan tests and unchanged-MW5 comparison,
RD05 DMA/snapshot regressions. No MIPS build or sustained hardware test in
this environment. Coherent allocation failure follows existing error unwind;
no silent fallback to a partially initialized streaming path is introduced.

PC build:
```sh
cd ~/Schreibtisch/openwrt_git_RTL/Openwrt_RTL
git pull --ff-only origin MW5_Stable
make target/linux/clean
make -j"$(nproc)" V=s
```

After installing image, router via UART38400, commands individually:
```sh
cat /proc/cudy-r76-rtknet
ifstatus wan
ip route
```
Confirm coherent_rx=1 coherent_tx=1 page_pool=0. Do not activate WLAN during
this Ethernet test (the independent G6 cfg80211 blocking bug remains open).
PC on LAN, Fritzbox on WAN:
```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
for n in 56 74 75 128 512 1400; do ping -I 192.168.1.3 -c 10 -W 1 -s "$n" 192.168.1.1; done
curl -I --max-time 10 http://192.168.1.1/
ssh -o ConnectTimeout=10 -o ServerAliveInterval=5 -o ServerAliveCountMax=3 root@192.168.1.1
```
After failure or sustained traffic use UART to read proc again, including
routed_guard/routed_last_bad and TX/RX counters. These are read-only operations
and do not depend on the nl80211/RTNL requests that hung in the WLAN case.

Follow-up: R76-TCP-BURST64.md documents a later hardware candidate overriding
only R76 CPU-DMA burst to64; coherent buffers alone did not cure load failures.
