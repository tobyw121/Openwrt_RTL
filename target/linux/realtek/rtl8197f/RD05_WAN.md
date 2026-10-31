# RD05 routed WAN DHCP

For a standalone OEM hardware/runtime inventory without MTD partition reads,
see [the bounded collector instructions](../../../../scripts/rd05-oem-collect.md).
This collector runs on the existing OEM image; no firmware build is required.

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

## External RTL8367D readback: OEM fine TX tap 4, profile v39

The OEM `8367read` proc requests succeeded. Repeated register values are
recorded in [external-switch-readback.json](../../../../reverse-engineering/rd05-oem-runtime/external-switch-readback.json).
The measured EXT1 mode is RGMII (0x1305=0x10), coarse TX0/RX5
(0x1307=5), fine TX tap **4** (0x13f9=0x0124, bits 5:3=4), and SSC
enabled with 0x2473/1/0x5fa/0xf at 0x1d52–0x1d55. Our previous fine tap
was zero (0x13f9=0x0104). The coarse timing and SSC already matched.

The old bootstrap diagnostic also showed debug0/debug1=0x784/0x4444,
TOP_CON0=0 and zero MAC7 force/enable state, whereas the OEM reports
0x7d6/0x7544, TOP_CON0=0x2000 and MAC7=0x76/enable=0xffff.
Patch 338 calls the existing EXT1 mux/debug/routing and force-link setup
before bootstrap CPU traffic. It requires both the RD05 board and D-family
chip and propagates setup failures before changing the private port tables.
The kernel selects the measured fine tap 4 on that board unless an explicit
module/runtime tap override was supplied; explicit zero remains supported.

The RD05 calibration package is release 12. Its v39 baseline keeps
SoC TX0/RX6, switch coarse TX0/RX5 and SSC=1, changing only SW_TXC to 4.
Older persistent profiles are backed up to `.pre-v39` and migrated so they
cannot silently restore the former zero tap. Current v39 saved profiles are
preserved. `sdk`/`restore` selects the measured tap as well. Mutating helper
commands now require the exact `xiaomi,r4-rd05` board; MW5 cannot migrate
the file or apply RD05 parameters.

OEM CPU_CTRL=0x2b9 versus our 0xb9 is the **4-byte versus 8-byte CPU-tag
format** bit. Linux rtl8_4 needs the existing 8-byte tag, so that difference
is intentional. CPU mask 0x80 agrees. OEM port7 misc=0x4800 and our former
0x4830 both leave CRC-skip disabled; unknown bits 0x30 are not copied blindly.
No CRC bypass, VLAN leak or OEM routing shortcut is introduced.

Validation covers old-profile backup/migration, preservation of a saved v39
profile, SDK restore, refusal to modify MW5, exact-board/chip initialization,
error propagation and preservation of explicit fine-tap overrides. All 23
selected native/shell tests pass. The patched switch object compiles for
MIPS/Linux 6.6.114; patch 338 applies without fuzz to the prepared tree.
Full-image build and physical Ethernet recovery still need verification.

After rebuilding, `rd05-rgmii-calibrate status` should show SW_TXC=4 and
`/tmp/rd05-rgmii-boot.log` should show the applied profile. The kernel reapply
log should report ext_txc=0x124 with the observed other EXT fields. Keep
TX descriptor mode 0 for the next LAN ARP/DHCP/WAN test.

## GP3000 family-D review: RGMII transaction ordering

The GP3000 tree at commit `5090fbcb6ba743dd7b1314811ef557bad0460147`
contains a second RTL8367D SDK implementation:
[dal_rtl8367d_port.c](https://github.com/jameywine/GPL-for-GP3000/blob/5090fbcb6ba743dd7b1314811ef557bad0460147/linux-5.10.x/drivers/net/ethernet/realtek/rtl8367/dal/rtl8367d/dal_rtl8367d_port.c).
Its `dal_rtl8367d_port_rgmiiDelayExt_set()` contract explicitly requires timing
configuration before `rtksw_port_macForceLinkExt_set()`. The associated register
header confirms EXT1 coarse TX/RX at 0x1307 and the separate fine TX tap at
0x13f9[5:3]. These match our existing definitions; no new timing constant is
inferred from the GP3000 board.

Patch 339 uses one RD05-only transaction for bootstrap and runtime calibration:
force CPU7 down, configure EXT1 mux/SSC/timing, apply the effective diagnostic
coarse timing, read back coarse and fine timing, then force the 1-Gbit full-duplex
link up. Bootstrap no longer rewrites SSC and timing after link enable. A read
error or mismatch prevents the final link enable; a mismatch logs
`RD05 EXT1 timing readback mismatch`. Failure to stop CPU7 aborts immediately.
This deliberate stop during reconfiguration is our robustness change based on
the SDK ordering requirement, not a claim that GP3000 uses identical board timing.

All new register operations require both `xiaomi,r4-rd05` and the D-family chip
flag. MW5 does not enter this transaction. CPU-tag format, VLAN/bridge policy,
CRC validation, firewall rules and acceleration defaults are unchanged.

Validation: native fault injection checks all six transaction boundaries,
readback mismatches, all supported coarse/fine timing combinations and board/chip
rejection. The patched switch object compiles for MIPS against Linux 6.6.114.
Neither check establishes that the RD05 FCS/RX failure is resolved; a full image
build and physical retest remain necessary.

For a bounded retest, connect the PC to LAN2, keep UART access to the router,
and enter router commands one line at a time. First record router counters:

```sh
cat /etc/openwrt_release
ethtool -S lan2 | grep -E 'ifInOctets|ifOutOctets|cpu_'
grep -E '^(rx packets|tx packets)' /proc/rd05-rtknet
```

On the PC, start a capture in terminal 1:

```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
sudo tcpdump -ni enx14ebb6300bb2 -e -vvv 'arp or icmp'
```

In PC terminal 2:

```sh
sudo ip neigh flush to 192.168.1.1 dev enx14ebb6300bb2
sudo arping -I enx14ebb6300bb2 -S 192.168.1.3 -c 10 192.168.1.1
ping -I 192.168.1.3 -c 3 192.168.1.1
```

Then on the router:

```sh
ethtool -S lan2 | grep -E 'ifInOctets|ifOutOctets|cpu_'
grep -E '^(rx packets|tx packets)' /proc/rd05-rtknet
dmesg | grep -E 'RD05 EXT1|RGMII|readback'
ifstatus wan
```

Stop PC tcpdump with Ctrl+C. Compare counter deltas, not historical totals.
A successful local test needs ARP replies and increasing SoC RX; WAN additionally
needs a DHCP lease from the Fritzbox. Do not disable CRC checks to obtain a pass.

## RD05 paired TX snapshots (1.8.45)

The manual OEM memory reads selected completed, CPU-owned descriptors 0..7
while the live TX position was 545. Reused/free packet buffers cannot be tied
to those old entries. Do not interpret those memory dumps as corrupt OEM frames.

`rd05_tx_snapshot_budget` now arms an opt-in capture of at most eight linear,
single-descriptor RD05 transmissions. Default is zero. The driver copies up to
96 packet bytes before streaming DMA mapping, saves the exact descriptor fields
before publishing OWN, and logs only local copies after releasing the TX lock.
`DW0` shows the intended published OWN value; it is not a completion readback.
No post-map packet-buffer CPU reads are introduced. Non-RD05 boards cannot
consume the quota or emit snapshots. No timing, CRC or forwarding policy changes.

This validates software publication, not the bytes actually fetched by DMA or
wire FCS. Hardware FCS counters and external packet capture remain separate
observations. Opt-in printk can perturb timing; keep console loglevel low during
the short capture. Packet prefixes may contain private traffic; isolate WAN
and use only controlled ARP/ping traffic when sharing logs.

Build on the PC in the already configured RD05 tree:

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git switch MW5_Stable
git pull --ff-only origin MW5_Stable
make defconfig
grep '^CONFIG_TARGET_realtek_rtl8197f_DEVICE_xiaomi_r4_rd05=y' .config
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

If the profile check is empty, select Realtek/RTL8197F/Xiaomi Mi WiFi R4 (RD05)
with `make menuconfig` before building. Use `make -j1 V=s` to diagnose a failed
parallel build. Images are in `bin/targets/realtek/rtl8197f/`.

After flashing the new OpenWrt image, disconnect WAN, attach PC to LAN2 and
access the router over UART. Router, one line at a time:

```sh
ethtool -i eth0
read saved_lvl rest < /proc/sys/kernel/printk
dmesg -n 1
ethtool -S lan2 > /tmp/rd05-tx-before.txt
echo 8 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_snapshot_budget
ping -I br-lan -c 5 192.168.1.3
ethtool -S lan2 > /tmp/rd05-tx-after.txt
cat /proc/rd05-rtknet > /tmp/rd05-tx-state.txt
echo 0 > /sys/module/rtl8197f_rtknet/parameters/rd05_tx_snapshot_budget
dmesg > /tmp/rd05-tx-dmesg.txt
dmesg -n "$saved_lvl"
tar -czf /tmp/rd05-tx-diag.tar.gz -C /tmp rd05-tx-before.txt rd05-tx-after.txt rd05-tx-state.txt rd05-tx-dmesg.txt
```

Before running those router commands, start the PC capture:

```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
PCAP=$(mktemp "$HOME/rd05-openwrt-tx-XXXXXX.pcap")
echo "$PCAP"
sudo tcpdump -U -ni enx14ebb6300bb2 -s 256 -w - 'arp or icmp' > "$PCAP"
```

Stop tcpdump with Ctrl+C after the router test. Send the PCAP and router log;
if wired SSH is still unavailable, display the following over UART:

```sh
grep -E 'RD05-TX-SNAPSHOT|RD05-TX-BYTES' /tmp/rd05-tx-dmesg.txt
cat /tmp/rd05-tx-before.txt
cat /tmp/rd05-tx-after.txt
cat /tmp/rd05-tx-state.txt
```

The quota may capture unrelated background traffic before ARP. If that happens,
inspect the recorded frames before deciding whether to rearm. No unlimited trace
is needed. Reconnect WAN after diagnostics. This change is not a claimed LAN/WAN
fix and has not been validated by a full image build or physical retest.

## Coherent TX diagnostic, 1.8.46

See [the RD05-only A-B-A DMA buffer test](RD05_TX_DMA_AB.md) for implementation
scope, build commands, the reopen-only parameter and complete PC/UART steps.
Streaming TX remains the RD05 default; this is not a proven hardware fix.


## 1.8.47 RD05 DSA audit fixes (2026-10-31)

See [adapted audit patch scope](RD05_DSA_AUDIT.md) for exact scope and tests.

- Free skb on both failed TX linearization paths before returning NETDEV_TX_OK.
- Use deterministic lan1/lan2 + wan defaults; never bridge the CPU conduit as
  a fallback while the switch is probing. Candidate lan3/lan4 stay unbridged.
- Preserve restored/custom UCI network and DHCP configuration. Do not delete
  every device section at package installation.
- Read back CPU mask, per-port isolation and PVID fields in normal RD05 mode;
  abort on mismatch/SMI error and attempt to stop CPU7. This is not a wire test.
- Refuse unsupported RD05 bridge/VLAN writes in rtl8367dctl rather than hide
  kernel errors. No dynamic VLAN/FDB/bridge-offload callbacks were added.
- Add rd05-dsa-check and offline source regression tests.

Physical jack mapping, external packet delivery, CPU-port FCS errors,
RGMII/DMA behaviour, tagged WAN and a full firmware build remain unverified.
No DMA defaults, ring sizes, tag format, RGMII delays or GPIO values changed.
