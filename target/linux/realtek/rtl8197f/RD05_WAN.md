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

## OEM chip-ID branch reconstruction (2026-10-21)

A direct examination of RD05 `init_8197f_p0` at `0x802cf41c` found a concrete
mismatch. Kernel SHA256 is
`aa48d325730b95c0658090ba9f943f71bdd9888dc57f609a3989ba58f0f8f5cc`;
the 400-byte function SHA256 is
`b147868cac2037ce7b6765d090a06104a56d8946fcddd8c3f3c7894cbe2ef200`.
The complete instructions are in the existing archived
`disassembly/80200000-80300000.dis.xz`.

The function reads system register `0xb8000000` and compares
`ID & 0xfffff000` to `0x81970000`. It does not select the branch based on the
router profile or the FH/VG name.

| Setting | ID matches 0x81970000 | Other IDs, including 0x8197f000 |
| --- | --- | --- |
| SoC RX delay | 6 | 5 |
| PAD preserve mask | 0x019fffff | 0x01bfffff |
| PAD OR value | 0xda600000 | 0xd8000000 |
| PITCR fixed-IPG field | clear field then set 0x1000 | preserve |
| EXTPCR0 IPG field | set 8 | preserve |

Branch sites are `0x802cf4bc` and `0x802cf4f8`; the distinct pad paths start
at `0x802cf500` and `0x802cf570`. The RX6 override is at `0x802cf590`.
The base RX5 setup is at `0x802cf480`. These instructions were checked against
the archived kernel bytes and interpreted with fake MMIO, including branch
delay slots, for several initial register values. This is static/host analysis,
not a trace of execution on the router.

Our driver previously forced the ID-matching branch's PAD bits, fixed IPG and
RX6 on RD05 unconditionally. Version `1.8.39-rd05-oem-chip-pad` reads the silicon
ID and follows the OEM branch. On the nonmatching path, the previously logged
PAD value `0xdbffff7f` becomes `0xd9bfff7f`; this clears the incorrectly forced
MODE/TX_E2 fields while preserving the other OEM-designated bits. RX becomes 5
unless a manual timing override was requested. Other board policies and the
Linux DSA opaque-tag policy remain unchanged.

The live ID is now reported as `soc_id` in `dsa_host_link`, and boot logs include
`RD05 OEM P0 chip_id=... old_id_branch=... fixed_ipg=... rx_delay=... pad=...`.
The source mismatch is established; restoring traffic and lowering FCS errors
still require a hardware retest. It is not sufficient to call this the sole
root cause. The OEM function may also be followed by later register writes.

`New_swNic_init` does use 32-byte RX/TX strides on RD05, so changing that known
geometry merely to match another router would not follow the OEM evidence.
The similarly named legacy `_swNic_send` is present too; its presence alone
does not establish that it is the active new-descriptor transmit path.

## Retest of 6afa2d01ed: actual RD05 ID matches the original branch

The next hardware log reports `soc_id=0x81970001`, `old_id_branch=1`,
`fixed_ipg=1`, RX6 and PAD `0xdbffff7f`. After masking revision bits, this is
exactly the OEM `0x81970000` branch. The adaptive-ID correction therefore does
not change P0 electrical settings on this particular device and did not fix
its outage. It must not be presented as an identified cause for this unit.
The native P0 test now includes the observed `0x81970001` revision value.

The same capture has 74 SoC TX completions and 74 switch CPU FCS errors,
14262 CPU input octets, 14935 CPU output octets, zero user output octets and
zero SoC RX. No new timing-probe result has been supplied yet. The existing
`rd05-rgmii-test` is the next measurement; another firmware build is not needed
for this measurement. A clean timing row would narrow the TX failure. Errors
at every RX-delay step would leave TX clock phase, frame/FCS generation and
the distinct SoC RX path under investigation, rather than proving any one of
them. CRC verification remains enabled throughout.

## First timing sweep: six CRC rows, two inconclusive rows

The v1 probe reported CRC errors at RX-delay steps 1, 2, 3, 4, 6 and 7.
Steps 0 and 5 had no increase in switch CPU input bytes during their measurement
windows and were marked `no-tx-sample`. They are not passing rows. Because v1
did not record SoC TX completions, these rows cannot distinguish absent DHCP
traffic from packets lost before the switch CPU MIB counted them. SoC RX
remained zero in every row. Delay 5 was restored, and SSC was enabled.

The v2 probe uses its own bounded DHCP client (`-t 1 -T 1`, four-second timeout)
with `/bin/true` as the event script and a private PID file. It does not install
an address, DNS or routes and does not overwrite netifd's client PID file. A
lease observed by this probe is reported separately from netifd WAN-up status.
It also samples SoC TX completions. `tx-missing-at-switch` now distinguishes a
SoC TX completion with no switch CPU input from a genuinely idle window. All
settings are restored; no timing row is persisted automatically. The field
order is printed in the report and is now five counters rather than four.

A fresh SDK source comparison of `_New_swNic_send` in
`8197/RT8197F_RTL8367RB_Gesamtpaket/MR62X_SDK_Paket/selected_sources/sdk/openwrt-21.02/target/linux/rtknet/drivers/net/rtl819x/rtl819x_swNic.c`
confirms that the new-descriptor routine sets both packet-header and buffer
length to payload length plus four (minimum 64), explicitly saying hardware
adds CRC. Our driver follows that rule. This does not prove that the complete
MAC/CRC configuration is correct, but it supplies no basis for blindly removing
those four bytes. The previous legacy `rtl865xc_swNic.c` comparison alone was
not sufficient to establish the new-descriptor rule.

## Timing probe preflight failure and v3 diagnostics

On firmware `26913f30ea`, a separate DHCP probe increased SoC TX from 140 to
151 and switch CPU FCS errors from 140 to 151. User-port output and SoC RX
remained zero. This is a sampled one-to-one correspondence, not proof of a
particular clock or CRC-generation defect.

A later report showed `rd05-rgmii-test` returning silently without creating
its report. The v2 script has silent preflight returns for missing `timeout`
and `udhcpc`. BusyBox's default configuration disables `timeout`; absence on
the actual flashed image must be checked with `command -v timeout` or
`sh -x /usr/sbin/rd05-rgmii-test`. The supplied `rd05-rgmii-boot.log` is a
different report, not a timing sweep: it has ten CPU-port FCS errors and
zero CPU-port output bytes, but does not contain a before/after sweep.

V3 reports missing prerequisites explicitly, saves preflight failures when
no other test owns the report, and leaves timing unchanged on those failures.
The RD05 package selects BusyBox's timeout applet for future builds. It also
accepts the older `p07_ifInOctets`, `p07_ifOutOctets`, and
`p07_dot3StatsFCSErrors` names as alternatives to the newer `cpu_*` counters.
This fixes the diagnostic, not the Ethernet datapath. Native tests cover
missing commands, old counter names, concurrent report preservation and
restoration after success, failure and termination. No new hardware result
or full firmware build is available for v3 yet.

## V4: Zeitbegrenzung ohne BusyBox-timeout

Auch das neu gebaute Image `7d25e5d6eb` meldet beim Benutzer weiterhin
`Required command missing: timeout`. Der vorherige Kconfig-Select lieferte
somit in diesem Image kein nutzbares Applet. V4 entfernt diese Voraussetzung:
Ein eigener Wächter beendet ausschließlich den privaten Vordergrund-DHCP-
Prozess nach vier Sekunden mit TERM und nötigenfalls eine Sekunde später
mit KILL. Nach Probeende wird der Wächter beendet. Auch beim Abbruch werden
die eigenen Prozesse beendet und die ursprüngliche RX-Verzögerung hergestellt.
Die netifd-DHCP-PID-Datei, Adressen und Routen werden nicht verändert.

Hosttests prüfen zusätzlich einen echten hängenden Testprozess, der TERM
ignoriert, sowie den gesamten Timing-Ablauf ohne verfügbares `timeout`.
Das ist eine Reparatur des Diagnosewerkzeugs, kein nachgewiesener RGMII-Fix.

## OEM-Laufzeitreferenz und erneuter Ausfall auf 7647623f0e

Die aktuellen Hardwareausgaben zeigen weiterhin WAN-DHCP pending, null
SoC-RX und null physische Ausgabe an WAN/LAN2. Der PC sendet ARP-Anfragen
auch mit passender Quelladresse 192.168.1.3, ohne eine Antwort zu erhalten.
Die externen CPU-Port-FCS-Zähler sind kumulativ; die einzelnen Aufnahmen
erlauben keinen neuen exakten Fehler-pro-TX-Vergleich.

Die anschließend gelieferten OEM-Archive enthalten vollständige RX-/TX-
Descriptor-Dumps, VLAN-/Netif-Tabellen und interne ASIC-Zähler. Analyse und
maschinenlesbare Belege liegen unter
[RD05 OEM runtime](../../../../reverse-engineering/rd05-oem-runtime/README.md).
Belegte Vergleichspunkte sind OEM-TX-DP=0x2 gegenüber DSA-DP=0x1 sowie
KSEG0-Pufferadressen gegenüber der KSEG1-Darstellung im aktuellen Treiber.
Ihre Bedeutung muss einschließlich OEM-Port-Remapping und DMA-Adressierung
geklärt werden. Der OEM-interne CPU-CRC-Zähler ist nicht derselbe Messpunkt
wie der externe DSA-CPU-Port-FCS-Zähler. Die Archive liefern keine numerischen
RGMII-/SSC-/CPUICR-Registerwerte und belegen noch keinen Ethernet-Fix.

## RD05-only implementation: 1.8.42

The uploaded runtime descriptors establish separate address representations:
packet pointers are KSEG0 (`0x8...`), while ring CDPs use KSEG1 (`0xa...`).
The RD05 now reproduces that distinction for TX/RX packet pointers, including
RX allocation, rearming and expected-address validation. Ring bases, original
Linux DMA handles, cache synchronization and unmapping remain unchanged.
This is an OEM-matching bring-up change; restored traffic is not yet measured.

The policy is selected once at probe and only for `xiaomi,r4-rd05`. The
read-only boot/load option `rd05_oem_buffer_alias=0` restores the previous
packet-address policy. For the built-in driver the kernel command-line option
is `rtl8197f_rtknet.rd05_oem_buffer_alias=0`. Changing it on a live RX ring is
deliberately unsupported. `/proc/rd05-rtknet` now includes a `rd05_oem_dma`
line with the selected alias, RX ring address, first RX buffer address and
TX descriptor diagnostic mode.

TX mode 13 adds an explicit RD05-only DP=2 comparison. It retains the DSA
wire tag, keeps HWLOOKUP/bridge shortcuts off, and does not import OEM VID9
metadata. Mode 0 remains the default; DP=2 is not a proven physical-port fix.

Patch 337 also requires the exact RD05 compatible before applying its private
external-switch CPU/VLAN/flood/isolation policy or publishing the calibration
target. Generic chip initialization remains separate. MW5 does not select the
new buffer policy or TX mode; its existing address results, DSA metadata,
RGMII settings and switch initialization are preserved.

Validation: native tests execute the actual address/metadata functions and
the switch board predicate across RD05, MW5, AC23 and WRD12GN. They cover
the legacy option, ring-address preservation, KSEG0/KSEG1/physical forms,
null chip information and diagnostic-mode isolation. Existing RD05 WAN,
MW5 stability and watchdog tests pass. Ethernet and patched switch objects
compile for MIPS against Linux 6.6.114. Patch 337 applies without fuzz to the
prepared switch tree. A complete firmware build and hardware validation are
still required.

After flashing, first collect with default TX mode 0:

```sh
cat /proc/rd05-rtknet
ethtool -S wan
ethtool -S lan2
ifstatus wan
```

The expected new line is `rd05_oem_dma buffer_alias=kseg0 ... txdesc_mode=0`;
the RX ring address should retain `0xa...`, the armed packet address should
be `0x8...`. The success criteria remain increasing SoC RX and physical
user-port output, followed by LAN ARP/DHCP and a WAN lease. Address readback
alone is not a passing Ethernet test.

## OEM register readback and RD05 FIFO correction: 1.8.43

Hardware retesting of 1.8.42 showed that KSEG0 packet addressing alone did
not restore traffic. In the DP13 test, LAN2 physical output and SoC RX stayed
zero; external CPU FCS errors rose from 207 to 222 across a window containing
both diagnostic and default mode. Mode 0 was restored. No successful DP2
egress or one-to-one per-packet explanation can be inferred from that window.

The next OEM UART capture finally contains clean repeated kernel-log register
readbacks. See [register-readback.json](../../../../reverse-engineering/rd05-oem-runtime/register-readback.json).

| Register | OEM | Previous port |
| --- | --- | --- |
| CPUICR | `0xe4000000` | `0xe4000000` |
| CPUICR1 | `0x002081d3` | `0x002081d3` |
| DMA_CR0 | `0x0003a0ce` | `0x000330d7` |

The GPL `rtl865x_asicCom.h` defines LOW_FIFO_MARK_VALUE=0xa0 and the
8197F-VG high watermark=0xce. Both `rtl865x_start()` and the NIC restart
path apply those values after writing the CPUICR burst size. This corroborates
the running RD05 measurement and establishes an actual policy mismatch.

Version 1.8.43 corrects only the RD05 DMA_CR0 watermark fields to 0xa0/0xce
at that existing point in startup. It preserves the upper register bits and
all other boards' 0x30/0xd7 policy, including MW5. It does not copy ring-size
registers: OEM DMA_CR1=0x5fe0 describes its 768-entry TX ring; our 64-entry
ring still requires 0x7e0. A native regression executes the production policy
across RD05/MW5/AC23/WRD12GN/generic and multiple starting register values.

This fixes a measured configuration mismatch, not a proven sole root cause.
LAN/WAN recovery still needs an on-device retest. Expect the RD05 startup
DMA_CR0 readback to end in `a0ce` (0x0003a0ce with the observed upper bits).

The MAC/P0 commands were corrupted by UART input overrun, so those values
remain missing. The supplied +0x834/+0x838 system values are not the RGMII
PAD_CTRL_1 register: our actual pad control is +0x850. For further OEM
capture, lower console logging temporarily, submit each read command
separately, then retrieve dmesg and restore the saved console level. The
proc `read` request outputs to the kernel log; plain `cat memory` is empty
by design. No MMIO write command is required for those readbacks.

## Complete OEM P0/MAC readback: 1.8.44

Suppressing console logging during the read requests produced intact OEM
values. See [p0-readback.json](../../../../reverse-engineering/rd05-oem-runtime/p0-readback.json).
P0 PCR=0x16972039, PITCR=0x1001, EXTPCR0=0x80000 and PAD_CTRL_1=0xdbffff7f
match the port. OEM P0GMIICR=0x060f7d46 differs from our 0x000f7d46 only in
the two CPU-tag bits, while MACCR1=1 differs in the router bit. Those three
controls remain deliberately disabled to preserve the Linux DSA wire tag.
Both paths use SoC RX delay 6 and TX delay 0; this does not measure the
external-switch EXT1 delay/SSC configuration.

MACCR differs: OEM=0x804a1485 versus port=0x804a1245. The difference is
entirely CF_FCDSC[10:4]: 0x48 (72 pages) versus 0x24 (36 pages).
The GPL `Setting_RTL8197F_PHY()` explicitly sets 0x48 in that field. Version
1.8.44 applies only that MAC flow-control tolerance field for RD05 in P0
initialization, including reseeds. It does not execute the OEM internal-PHY
calibration routine, overwrite the whole MACCR, or enable OEM router mode.
MW5 and other boards retain their existing CF_FCDSC values.

The native P0 MMIO test now checks exact 0x804a1245 -> 0x804a1485 conversion,
idempotence, preservation of unrelated bits and other-board values, alongside
the existing tag/timing checks. All 18 selected native tests pass and the
Ethernet object compiles for MIPS/Linux 6.6.114. A complete firmware build and
hardware retest are still needed: neither this field match nor the preceding
FIFO correction is proof of working Ethernet.

For the next hardware test, keep TX mode 0. Expect DMA_CR0 ending in a0ce,
MACCR=0x804a1485 with the observed initial bits, RX6/TX0, and opaque DSA
tag transport. Compare wired RX, physical LAN/WAN egress, FCS deltas, LAN
ARP/DHCP and the WAN lease against the previous failed capture.
