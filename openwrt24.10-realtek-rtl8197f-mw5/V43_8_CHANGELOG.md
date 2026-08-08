# v43.8 - MW5 RTL8197F RX descriptor to DSA integration

## Hardware evidence addressed

The v43.7 dual-port capture proved that valid frames reach the RTL8197F DMA
buffer without an on-wire 0x8899 header while the new-descriptor metadata
still carries the physical RTL8367 source port (opts4[15:13]: LAN=1, WAN=3).
It also showed repeated len=2046 garbage and a TX watchdog after the old
three-second delayed reseed rewound live ring pointers.

## Driver changes

- Driver version: `1.4.8-sdk-mw5-rxdesc-dsa-v43.8`.
- MW5 P0 no longer enables hardware CPU_TAG_RX or CPU_TAG_TX parsing/generation.
  Software DSA owns TX tagging and RX first attempts true 0x8899 wire pass-through.
- If a valid MW5 RX frame arrives without 0x8899, the master reconstructs the
  exact four-byte protocol-9 EtherType tag from opts4[15:13] before
  `eth_type_trans()`. Only proven source ports 1 (LAN) and 3 (WAN) are accepted.
- Frames already carrying 0x8899 are passed through unchanged; no double tag.
- Impossible MW5 receive lengths above the MTU/tag/VLAN/FCS envelope are
  rejected before entering the Linux networking stack.
- The automatic three-second MW5 delayed reseed is removed.
- Normal MW5 `reseed` and MAC-change reseeds update RTL865x tables only; they
  no longer rewind CPURPDCR0/CPUTPDCR0 or replay active DMA geometry.
- Explicit `rxstart` and true hardware-start/reset paths retain ring programming.
- First 64 MW5 TX packets are logged immediately before DMA mapping, including
  outer EtherType, protocol-9 tag word and encapsulated EtherType.
- `/proc/rd05-rtknet` reports RX DSA pass-through/synthesis/failure counters.

## DSA tagger

`tag_rtl4_9` remains the Linux DSA boundary. It now logs v43.8 markers and can
receive either the real RTL8367 0x8899 header or the identical header rebuilt by
the RTL8197F master from descriptor metadata.

## Firewall

v43.7's diagnostic no-firewall policy is removed. v43.8 retains the normal
OpenWrt firewall4 configuration and LAN/WAN zones. A first-boot migration helper
restores a saved v43.7 firewall backup when available, otherwise falls back to
the standard `/rom/etc/config/firewall`, and enables the firewall service.
`mw5-netdiag` no longer stops firewall4 or flushes nftables.

## Diagnostics

`mw5-netdiag` remains a one-command hardware test. BusyBox-incompatible `ip -s`
and `ip -6` forms were removed from snapshots. The test now records standard
firewall state rather than modifying it.
