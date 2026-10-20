# RD05 routed WAN DHCP

Observed failure: both WLAN bands associate and local LAN management works;
`wan` has a 1000/full carrier, but `ifstatus wan` stays pending with repeated
udhcpc discovers. Even a ping initiated by the router returns Network
unreachable. This precedes IP routing/NAT: no WAN DHCP lease/default route has
been installed. A link indication alone does not prove packet delivery.

The RD05 uses distinct factory LAN and WAN MAC addresses. The SoC's RTL865x
host-link seed previously installed CPU/static entries only for the master
address. The RD05 DTS now supplies `ethaddr_wan` to the Ethernet driver, which
installs the WAN identity as a CPU/static destination in both bootstrap FIDs.
The WAN entries use hash way 1, preserving existing LAN/broadcast entries in
way 0 even if their hashes collide. Linux DSA still selects the external
switch port; the SoC does not bridge WAN into LAN.

The U-Boot NVMEM driver converts `ethaddr` to six binary bytes but exposes
`ethaddr_wan` as ASCII. The named-MAC reader now distinguishes these formats
instead of treating the first six text characters as a binary MAC address.

RD05 defaults also request broadcast DHCP replies (`network.wan.broadcast=1`),
set the master to the factory LAN identity, and generate routed DSA interfaces
without the generic managed-switch `switch` bridge. Both generators use
lan1/lan2 for the LAN bridge and keep wan separate. The TX length check accounts
for the eight-byte DSA header on RD05 and AC23 as well as MW5.

## Follow-up: no wired RX on e00c3bf939

On-device retest of the preceding change still showed pending WAN DHCP.
The SoC reported 196 completed TX packets but zero RX packets, RX interrupts,
RX errors, or RX descriptor traces after thousands of RX polling passes.
This disproves completion of the WAN fix and places the remaining fault before
normal IP routing. TX DMA completion alone does not establish external egress.
A further capture showed nonzero, CRC-error-free ingress on WAN and LAN2,
but zero output octets on both ports. The external switch CPU MIB counters are needed to distinguish copper ingress,
CPU-port egress, and the RGMII/SoC receive path.

The RD05 P0 setup still enabled CPU_TAG_RX, CPU_TAG_TX, and
PORT0_ROUTER_MODE for the legacy SDK, while Linux uses the upstream rtl8_4
header tagger. That tagger needs the original switch ingress port in the tag
after SA. SoC parsing/generation can consume or rewrite that tag before Linux
sees it, and router mode can alter egress encapsulation.

P0 setup now explicitly keeps these three controls off for RD05, including
when an older DT still supplies `realtek,p0-cpu-tag-pass-through`. The RD05 DTS
no longer requests that legacy parser. The existing FH/VG RGMII delay, pad,
clock, and fixed IPG settings remain board-specific. The change does not infer
that all FS and FH electrical settings are interchangeable.

`/proc/rd05-rtknet` now includes a `dsa_host_link` row with live register values.
For this RD05 policy, expect `cpu_tag_rx=0 cpu_tag_tx=0 router_mode=0`.
The correction is a source-level tag-ownership fix; restored physical packet
flow remains to be established by hardware retest. If RX remains zero, inspect
the external switch CPU-port MIBs and RGMII timing rather than changing NAT.

## Validation

Native regression tests execute the actual MAC reader and table-selection
functions, including invalid addresses and LAN/WAN hash collisions. A native
MMIO test executes the complete P0 setup with old/new DT settings, checks
parser/router bits after repeated reseeds, and verifies that VG timing/IPG
settings are preserved. The
Ethernet object compiles for MIPS against Linux 6.6.114, and the RD05 DTS
compiles. These checks do not prove hardware packet delivery or replace a full
firmware build. The original DHCP failure still needs an on-device retest.

After flashing, keep the Fritzbox connected to WAN and check:

```sh
ifstatus wan
ip route
ping -c 3 1.1.1.1
```

Also inspect `cat /proc/rd05-rtknet`: the new driver version is
`1.8.38-rd05-transparent-dsa`, and RX packets must increase when a wired client
requests DHCP. Test both wired LAN jacks.

Expect `up: true`, a Fritzbox DHCP lease, and a default route via WAN. Then test
an Internet IP and DNS name from one LAN client and each WLAN band. Replies to
LAN-initiated connections use the existing stateful firewall/NAT policy.
Unsolicited inbound WAN connections still require explicit port forwarding.

For the broadcast-DHCP part alone on an existing image (not the kernel fix):

```sh
uci set network.wan.broadcast='1'
uci commit network
ifup wan
```

If DHCP still fails after the complete firmware update, collect `ifstatus wan`,
`ip -s link show wan`, the udhcpc log, and Ethernet/switch ethtool counters during
one renewal. A capture on the Fritzbox side can distinguish a lost discover
from a lost offer. Do not publish complete configuration files or credentials.

## CPU endpoint counters

`ethtool -S wan` and `ethtool -S lan2` also expose the RTL8367D CPU endpoint:

- `cpu_ifInOctets`: packets entering the switch from the SoC over RGMII.
- `cpu_ifOutOctets`: packets leaving the switch toward the SoC.
- `cpu_fcs_errors`: bad FCS received by the switch from the SoC.
- `cpu_stp_state` / `user_stp_state`: expected forwarding value is 3 on active ports.
- `cpu_isolation` / `user_isolation`: permitted destination masks. In the
  current bootstrap, CPU7 permits users 0..4 (`31`), users permit CPU7 (`128`).

These are live read-only observations. An unavailable/read-error value is
UINT64_MAX (18446744073709551615), not zero. Copper input with no CPU output
points to switch forwarding/classification. CPU output with no SoC RX points
to the RGMII/SoC/DMA receive path. CPU input or FCS counters help verify whether
completed SoC TX actually reached the external switch.

## RGMII timing probe after the c219e31a4f hardware log

The transparent configuration was confirmed active on hardware, but did not
restore traffic. The CPU endpoint counted 13941 input bytes, 12182 output
bytes and 73 FCS errors, while the SoC reported 73 TX completions and zero RX.
Active user/CPU STP was 3, isolation masks were 128/31. These observations
move the remaining investigation to the physical CPU/RGMII/FCS path; they do
not prove a specific delay or CRC-format correction.

Disabling EXT1 SSC did not obtain a WAN lease. A later sample had 51668 input
bytes, 95593 output bytes and 194 FCS errors. There was no immediate pre-test
snapshot, so the FCS delta attributable to disabling SSC is unknown.

`rd05-rgmii-test` now tests the eight legal switch-side EXT1 RX-delay steps.
It keeps the current SSC selection, CPU TX delay/fine tap, SoC timing and all
CRC checks. It renews DHCP through netifd and measures counter deltas over
three seconds per setting, after a one-second settling interval. It refuses
to run with WAN already up or without valid CPU counters. Settings are restored
on completion, command failures and INT/TERM; SIGKILL/power loss cannot run a
shell cleanup handler (reboot restores compiled defaults). A successful DHCP
observation is reported but never automatically persisted.

This is a diagnostic probe, not an automatic calibration or proof of stable
operation. `tx-clean-rx-missing` means only the measured SoC-to-switch direction
had traffic without new FCS errors. The opposite direction still failed.
`no-tx-sample` is inconclusive. Counter resets and missing/U64_MAX readings
must not be called clean results.

Run from UART or local WLAN while WAN and a wired client remain attached:

```sh
rd05-rgmii-test
cat /tmp/rd05-rgmii-test.log
```

The standalone script can also be copied to `/tmp` on existing c219e31a4f
firmware and run with `sh /tmp/rd05-rgmii-test`; a reflash is not needed for
this measurement. The package includes it in future RD05 builds.
