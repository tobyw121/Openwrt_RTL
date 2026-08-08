# v43.10 changelog

- Follow RTL8197F OEM `USE_SWITCH_RX_CDP` completion semantics on MW5.
- Follow RTL8197F OEM `USE_SWITCH_TX_CDP` reclamation semantics on MW5.
- Clear stale TX OWN state when a descriptor is proven complete by CPUTPDCR0.
- Keep the proven v43.9 `0x8899 / 0x0400+source-port` RX DSA decoder.
- Add bounded malformed-RX descriptor/payload snapshots.
- Add RX/TX CDP counters to `/proc/rd05-rtknet`.
- Update built-in `mw5-netdiag`/help markers to v43.10.
- Keep standard OpenWrt firewall4 configuration unchanged.
- Keep physical LAN=RTL8367 port 1 and WAN=port 3 mapping unchanged.
