# RTL8367 register maps and driver families

## 1. Identity probing common to the supplied SDKs

The v1.4 API first unlocks/identifies the switch by writing the Realtek magic
value `0x0249` to register `0x13c2`, then reads:

| Register | Meaning |
|---|---|
| `0x1300` | chip number / family ID |
| `0x1301` | silicon/version word |
| `0x1322` | software/hardware reset control |
| `0x13c2` | Realtek magic-ID access register |

The product name alone is not sufficient. In particular, **RB-VB is C-map**,
whereas **RB-VC is D-map**. The v35 driver therefore treats the runtime chip ID
and revision as authoritative and does not let a compatible string force an
unknown B-map chip through C/D register writes.

## 2. B register map

The older B-era API covers RTL8367, RTL8367B, RTL8367R/RB and several related
marketing variants. Its canonical register header in the supplied SDK contains
13,622 indexed definitions. It is coupled to large, revision-specific
`ChipDataXX` initialization arrays; these arrays are the main reason a generic
DSA fallback is unsafe.

Important B-map blocks used by the OpenWrt legacy driver include:

| Address/block | Function |
|---|---|
| `0x000e + 0x20 * port` | per-port miscellaneous/egress mode |
| `0x0500`, `0x0501`, `0x0510...`, `0x0520...` | table-access engine |
| `0x0700...` | port PVID controls |
| `0x0728 + 4*n` | VLAN member configuration table |
| `0x07a8`, `0x07a9` | VLAN enable and ingress control |
| `0x08a2 + port` | port isolation masks |
| `0x1000...` | MIB counter window |
| `0x1200` | switch global control / maximum frame length |
| `0x1300`, `0x1301`, `0x1302` | chip number, version and mode |
| `0x1303`, `0x1304`, `0x13e2` | pad/drive/debug controls |
| `0x1305`, `0x13c3` | RGMII delay/interface controls |
| `0x1312 + ext` | external MAC force-link control |
| `0x1322` | chip reset |
| `0x1352 + port` | port status |
| `0x1f00...0x1f04` | indirect PHY/SDS access |
| `0x2000 + 32*phy + reg` | internal PHY register aperture |

Safe Linux path in v35: build the existing `kmod-switch-rtl8367` and
`kmod-switch-rtl8367b` modules as optional modules, use
`realtek,rtl8367-legacy-master` on the RTL8197F NIC, and provide the exact old
`realtek,extif` tuple for the board. These modules are not installed into the
RD05 image by default.

## 3. C register map

The C-map v1.4 header contains 18,679 indexed definitions. SDK probing groups
chip IDs `0x0276`, `0x0597`, and `0x6367` into this register family. Known exact
Linux profiles are:

- `0x6367:0x0020` — RTL8367RB-VB;
- `0x6367:0x0040` — RTL8365MB-VC;
- `0x6367:0x00a0` — RTL8367S.

Typical RTL8197F cascade: switch physical port 6 / EXT0 to SoC physical P0.
Some C-map parts also expose physical port 7 / EXT1.

Key C-map registers:

| Address | Function |
|---|---|
| `0x0500`, `0x0501` | table access control/address |
| `0x0510...`, `0x0520...` | table write/read data |
| `0x0700...0x0705` | PVID controls |
| `0x07a8`, `0x07a9` | VLAN enable and ingress filtering |
| `0x0890`, `0x0891`, `0x0892` | unknown unicast, multicast and broadcast flood masks |
| `0x08a2...0x08ac` | port isolation masks |
| `0x09c0` | unknown-unicast behavior |
| `0x0a40...` | storm-control masks/meters |
| `0x1219`, `0x121a` | CPU-port mask and CPU-tag control |
| `0x1300`, `0x1301` | chip number/version |
| `0x1306`, `0x1307` | EXT0/EXT1 RGMII delay fields |
| `0x1312...0x1319` | MAC0...MAC7 force-link controls |
| `0x1322` | chip reset |
| `0x13f9` | external TX clock delay selection |
| `0x1d52...0x1d55` | EXT0 SSC block |
| `0x1d59...0x1d5c` | EXT1 SSC block |
| `0x1d70` | top-level SerDes/external-interface control |

C-map SSC sequences implemented by v35:

| State | seed | divider | polynomial | control |
|---|---:|---:|---:|---:|
| enabled | `0x0001` | `0x000f` | `0x05fa` | `0x2473` |
| disabled | `0x8000` | `0x000f` | `0x0008` | `0x2411` |

When SSC is enabled, the SDK default RGMII delay is TX tap 0 / RX tap 5. When it
is explicitly disabled, the conservative C-map default in v35 is TX0/RX2.
Explicit DTS delay properties always override these defaults.

## 4. D register map

The D-map v1.4 header contains 10,407 indexed definitions. The family ID is
`0x6642`. The supplied SDK maps RTL8367RB-VC onto the RTL8367D DAL. The exact
RD05 chip is `0x6642:0x0030` and uses physical port 7 / EXT1 / MAC7 as RGMII CPU
link.

Key D-map registers:

| Address | Function |
|---|---|
| `0x0500`, `0x0501`, `0x0510...`, `0x0520...` | table access |
| `0x0700...0x0707` | direct 12-bit PVID per port |
| `0x07a8`, `0x07a9`, `0x07aa` | VLAN/ingress/filter control |
| `0x0890`, `0x0891`, `0x0892` | unknown/broadcast flood masks |
| `0x08a2...0x08a9` | 8-port isolation masks |
| `0x08c5` | source-port permit mask |
| `0x08c9`, `0x08cb`, `0x08cd` | unknown IPv4/IPv6/L2 multicast masks |
| `0x09c0` | unknown-unicast behavior |
| `0x09c1`, `0x09da`, `0x121c`, `0x12fb` | mirror/monitor controls |
| `0x0a10` | isolation-action behavior |
| `0x1219`, `0x121a` | CPU-port mask and CPU-tag control |
| `0x12c0...0x12c7` | MAC0...MAC7 force status |
| `0x12c8...0x12cf` | MAC force-select enables |
| `0x12d0 + port` | D-family port status |
| `0x1300`, `0x1301` | chip number/version |
| `0x1303`, `0x1304` | external pad drive/debug controls |
| `0x1306`, `0x1307` | EXT0/EXT1 RGMII delay fields |
| `0x1322` | chip reset |
| `0x13f9` | EXT1 TX-clock delay field |
| `0x1d52...0x1d55` | D EXT1 SSC block |
| `0x1d70` | TOP_CON0: EXT1-to-MAC7 routing |
| `0x1d78` | SDS1 mode/power/force control |

The D-family SSC enable/disable values are identical to the C-family values,
but the active block for RB-VC/D is EXT1 at `0x1d52...0x1d55`. Clearing the
registers to zero is not the SDK disable operation; v35 uses the exact
`0x8000/0x000f/0x0008/0x2411` sequence.

Generic D-map timing is TX0/RX5. RD05 is deliberately TX0/RX6 because the v33
hardware trace showed FCS errors at switch CPU7 when the RTL8197F transmitted
into EXT1 with RX tap 5.

## 5. Machine-readable maps

The `register-maps` directory contains every one-line or continued `#define`
that can be indexed from the canonical supplied B/C/D headers:

- `rtl8367b-register-map.tsv` — 13,622 symbols;
- `rtl8367c-register-map.tsv` — 18,679 symbols;
- `rtl8367d-register-map.tsv` — 10,407 symbols;
- `REGISTER_MAP_SOURCES.tsv` — source line counts and SHA-256 hashes.

The full vendor APIs are much larger than the active Linux driver and include
QoS, ACL, SVLAN, IGMP/MLD, EEE, RLDP, OAM, mirror, rate-limit, storm-control,
MIB and PHY calibration code. Presence in the map does not mean the Linux DSA
API currently offloads that feature.
