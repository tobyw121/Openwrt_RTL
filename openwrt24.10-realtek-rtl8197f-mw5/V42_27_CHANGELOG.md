# v42.27 MW5 RTL8363/RTL8197FS P0-to-CPU pipeline fix

## Hardware finding

The v42.26 MW5 hardware log proves that the external RTL8363/RTL8367-class switch receives traffic on LAN and WAN while the RTL8197FS DSA master `eth0` receives zero packets. The physical links and DSA user ports are up, the six-dword/24-byte CPU-DMA format is active, and the RX descriptor base is programmed. The remaining missing block is the RTL8197FS internal rtl865x/SWCORE classification path between physical P0/RGMII and the CPU DMA ring.

The Realtek GPL SDK performs two different operations for this topology:

1. configure the external RTL8367R/RTL8363 CPU MAC on `EXT_PORT_1` / MAC port 6;
2. initialize the RTL8197F internal VLAN/netif/L2/ACL path and send unknown VID / selected protocols to the CPU.

v42.26 implemented the first operation but left `legacy-pipeline=0` on MW5.

## Changes

- enable `realtek,legacy-rd05-sdk-pipeline` for `tenda,nova-mw5`;
- enable the same path unconditionally for MW5 in the rtknet board setup;
- seed internal rtl865x VLAN 9 and VLAN 8 with physical P0 as the only host-link member;
- set P0 PVID to VLAN 9 and port-to-netif index 0;
- create netif0 with the actual runtime `eth0` MAC, not the RD05 hard-coded MAC;
- create CPU-MAC and broadcast L2 entries with the actual MW5 MAC;
- enable unknown VID, unknown unicast and unknown multicast delivery to CPU;
- retain the OEM-derived DHCPv4, DHCPv6 and PPPoE ACL table seed;
- repeat the seed three seconds after interface open, after SWCORE has reached stable defaults;
- expose `/proc/rd05-rtknet` on MW5 as a temporary `reseed`/`rxstart` diagnostic trigger;
- add a single `mw5 p0 cpu-pipeline v42.27` summary line with all table-programming counters;
- keep the MW5 switch CPU endpoint at port 6 / EXT_PORT_1. Chip ID `0x6367` and the RTL8367R SDK both identify this endpoint; port 7 belongs to other RTL8367D mappings and is not selected here.

## Deliberately unchanged

- external RTL8363/RTL8367 CPU port remains DSA port 6;
- LAN remains physical switch port 1;
- WAN remains physical switch port 3;
- P0/RGMII timing remains TX delay 0 / RX delay 5;
- CPU-DMA descriptor format remains six dwords / 24 bytes for RTL8197FS non-VG;
- OpenWrt bridge VLAN configuration remains `switch.1` for LAN and `switch.2` for WAN;
- private factory, MAC, RF, CFM, log and environment partitions are not modified by this source-only release.

## Expected hardware proof

The first boot should show:

```text
registered eth0: ... desc=6/24 ... legacy-pipeline=1 ...
mw5 rtl865x l2cpu v42.27: ... cpu-mac=cc:2d:21:9e:70:c0
mw5 rtl865x pipe v42.27: ... mac=cc:2d:21:9e:70:c0
mw5 p0 cpu-pipeline v42.27: l2_ok=... pipe_ok=... pvid_ok=... netif_ok=... acl_ok=...
```

After client DHCP renewal, `eth0` RX must increase and the client must receive a lease without a static address. Physical hardware validation is still required.
