# v43.7 - automatic wired test + no-firewall diagnostic image

Base: v43.6 tagger-format-buildfix (the build-confirmed source tree).

## Automatic one-command hardware test

`mw5-netdiag` with no arguments now runs the complete active wired test.
The operator only moves one Ethernet cable from the physical LAN jack to the
physical WAN jack when prompted.  No `ip`, `ping`, `uci`, `nft`, `ifup`, or
other commands need to be entered manually.

The test:

1. captures the original normal state;
2. actively tests normal LAN through `br-lan`, including DHCP/peer discovery and bridge/DSA counter deltas;
3. creates exactly one network/DHCP backup;
4. configures direct LAN with 192.168.1.1/24 and DHCP;
5. waits for LAN carrier and generates router-originated ARP/IP probes;
6. detects a DHCP lease or neighbour automatically and pings it when possible;
7. records LAN RX/TX deltas, eth0 drop deltas, `/proc/rd05-rtknet`, and relevant dmesg;
8. reconfigures direct WAN without overwriting the original backup and repeats the active test;
9. restores the exact original network/DHCP configuration;
10. while the cable remains on WAN, checks the normal routed WAN DHCP client and optional Internet ping;
11. writes a concise summary below `/tmp/mw5-diag`.

An EXIT/INT/TERM cleanup trap restores the original network configuration if
the active test is interrupted.

## Firewall deliberately removed from the datapath

This is a diagnostic image.  On Tenda Nova MW5 only:

- firewall4 is stopped;
- firewall4 is disabled at boot;
- `/etc/config/firewall` is replaced by an intentionally empty config;
- the nftables ruleset is flushed;
- no LAN/WAN firewall zones or forwarding policy remain active;
- the boot diagnostic service re-enforces the no-firewall state on every boot.

The previous firewall file is copied once to
`/etc/config/firewall.mw5-disabled-backup` for reference, but `mw5-netdiag
restore` intentionally does not re-enable it.

This change removes firewall policy as a variable only.  It does not swap the
verified DSA mapping and does not merge the routed network topology: physical
LAN remains DSA `lan`, physical WAN remains DSA `wan`, and direct tests still
select each port independently.

## Retained v43.6 fixes

- RTL4 protocol-9 tagger format build fix (`u32 port_mask`);
- RX encapsulated EtherType reconstruction and tracing;
- RTL8197F MAC-change hardware-table reseed;
- readable `/proc/rd05-rtknet` state;
- MW5 direct `lan|wan|auto` diagnostic modes.
