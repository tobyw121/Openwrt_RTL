MW5 Live Audit v7.8 - immediate PC-side evidence preservation

Purpose
-------
This audit is designed for the MW5 failure mode where ICMP can still work while
new SSH/TCP connections already fail. The important evidence is therefore moved
to the PC before and during the failure instead of relying on one final router
archive.

Key behaviour
-------------
- Physical Ethernet adapters stay visible in the host namespace. Temporary
  macvlan children are placed in isolated mw5lan/mw5wan namespaces.
- PRE, POST and FAIL snapshots use `snapshot-stream`: the router creates one
  snapshot and immediately streams the tar.gz over the existing management
  path to the PC. A local tar integrity check is required before it is accepted.
- v7.8 establishes an SSH ControlMaster and also opens one persistent SSH telemetry connection before the
  stress test. `/proc/rd05-rtknet` OEM ring/CDP/OWN state and interface counters
  are appended directly to `router/live-stream-1s.log` on the PC every second.
  If the router later refuses new TCP sessions, the already-open stream can keep
  delivering data until that connection itself fails.
- A fresh per-run SSH known_hosts file is used. A host-key change during the
  same run remains an error and therefore exposes an unexpected reboot.
- The collector never scans `/sys/kernel/debug/regmap/*/registers`; sequential
  debugfs MMIO reads are unsafe on this RTL8197F target.

Run
---
    sudo ./mw5-pc-audit-interactive.sh

Important output
----------------
- `router/live-stream-1s.log`: continuously PC-persisted driver state
- `router/live-snapshots/`: validated PRE/POST/FAIL tarballs
- `router/live-extracted/`: immediately extracted snapshots
- `pc/reachability-1s.log`: independent ICMP liveness
- `timeline.tsv`: test and failure markers
- `iperf/`: traffic-test results

Before a normal stress run v7.8 requires the RTL8367 CPU controller to report format8b=1. Production DSA uses the upstream rtl8_4 eight-byte tag over RTL8197F P0/RGMII; tag_rtl4_9 tx_layout is diagnostic-only.

The script is diagnostic only. It does not enable HW-NAPT, HWLOOKUP or guessed
RTL8367 flow-control register writes.


v44.66.11 transparent-P0 LAN egress gate
------------------------
The PCAP comparator explicitly reports `vlan_before_rtl8_leaks`, `vlan_before_rtl8_vids`, legacy
`vlan0_before_rtl8_leaks`, and `realtek_cpu_tag_on_user_wire`.  A successful MW5 LAN run requires both values
to stay at zero on the physical user links.  v44.66.8 hardware captured
`8100 0000 8899 0400 ...` on the LAN cable; that exact failure signature is now
a first-class audit result.
