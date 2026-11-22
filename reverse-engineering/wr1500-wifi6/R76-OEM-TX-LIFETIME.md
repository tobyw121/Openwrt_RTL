# R76 OEM TX lifetime follow-up (2026-11-22)

OEM_Referenz_20261122_081547.zip parts000/001 passed ZIP CRC. All20
module binaries match original manifest SHA256/length. Original kallsyms
matches the supplied symbolized kernel at New_swNic_init=0x80262558.
INITIAL RX/TX stride is32; hardware CDP indices agree with software636/649.
The OEM allocates additional TX rings with distinct bases. Our driver only
allocates/programs ring0, but previously enabled ring1 via0x80090f13.

## Changes, R76 only

- Enable only allocated TX ring0, retaining the other existing TXRINGCR
  control bits. Readback checks ring-enable mask.
- Reclaim only before a valid, aligned CDP within the submitted circular
  interval AND after OWN clears. Invalid CDP, CDP beyond tx_head or OWN/CDP
  disagreement holds the buffer/slot for a later poll. No timeout-based free.
- Preserve completed descriptor bytes until submission, matching the existing
  OEM-style completion policy; retain prior packet-pointer alias changes.
- Proc exposes r76_tx_cdp guard=1 ring0_only=1 plus completion/defer counters.

MW5 retains its previous geometry and completion behavior; RD05 retains its
previous behavior. No VLAN, firewall, offload, WLAN or RX rearm changes here.
OEM CPU CRCAlignErr also grows while OEM traffic works, so it is not used as
an isolated reason to change RGMII timing. Snapshot metadata is insufficient
to safely replace Linux RX rearm flags with the observed0x06a4000d.

Validation: production C geometry and cleaner executed with host compiler
and UBSan. Exercises invalid/no-progress/out-of-window CDP, OWN disagreement,
ring wrap and successful completion on R76, MW5 and legacy board mocks.
Related RX/DMA/snapshot tests pass. Full MIPS build and hardware load testing
are unavailable here. This is a targeted lifetime correction, not a proven
cure for the observed TCP stall. The stricter guard may expose a real DMA
stall by retaining buffers rather than prematurely reusing them.

## Build (PC)

```
cd ~/Schreibtisch/openwrt_git_RTL/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make -j"$(nproc)" V=s
```

## Test after flashing (router UART38400; one command at a time)

```
ethtool -i eth0
cat /proc/cudy-r76-rtknet
ethtool -S wan
```

Expected driver1.8.49-r76-oem-tx-lifetime and r76_tx_cdp guard=1 ring0_only=1.
Keep WLAN disabled for this Ethernet test. On PC:

```
sudo ip link set dev enx00e04c5562c0 up
sudo ip addr replace 192.168.1.3/24 dev enx00e04c5562c0
sudo ip link set dev enx00e04c5562c0 mtu 1500
ping -I 192.168.1.3 -c 5 192.168.1.1
for n in $(seq 1 20); do curl --interface 192.168.1.3 -I --max-time 5 http://192.168.1.1/; done
```

Repeat PC checks under the same WAN workload that previously stalled TCP.
Then repeat router proc and ethtool counters. Capture bad_tag, TX completion,
invalid/defer counters before and after. Do not infer success from ping alone.
