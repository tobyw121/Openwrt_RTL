# v35 support status and limits

## What is implemented

- SMI and MDIO transport aliases for RTL8367C/D/R/RB/RB-VB/RB-VC names.
- Runtime chip-ID and revision matching, with exact profiles before family
  fallbacks.
- Eight-port correction for RTL8367S and RTL8367RB-VB.
- C-map EXT0/EXT1 and D-map EXT1 RGMII descriptions.
- Exact SSC enable and disable programming.
- Generic delay defaults plus explicit DTS delay override.
- D-map `TOP_CON0`, `SDS1_MISC0`, MAC7 force-link and EXT1 TXC handling.
- RD05-specific CPU7/EXT1 bootstrap remains opt-in and D-map-only.
- RTL8197F NIC DSA and legacy external-master modes.
- Optional build of the older swconfig drivers for B-map chips.
- Full machine-searchable B/C/D register symbol indices.

## What “all variants” means here

Every RTL8367 name found in the supplied SDK is classified in
`RTL8367_VARIANT_MATRIX.tsv`. It does **not** mean that an unknown product name
is allowed to write an arbitrary register map. C/D IDs are accepted by the DSA
driver; old B-map chips use their revision-specific legacy path; unresolved
marketing variants remain fail-closed.

## DSA capabilities present in the Linux 6.6 driver

The underlying `rtl8365mb` driver supplies probe/setup, PHY access, phylink MAC
configuration, link up/down, STP state, MTU handling, MIB/ethtool statistics and
the Realtek CPU-tag protocol. The RD05 branch additionally contains controlled
D-map VLAN/flood/isolation diagnostics and CPU-path bootstrap.

The driver does **not** currently expose complete standard DSA callbacks for
hardware bridge membership, VLAN add/delete, FDB add/delete/dump, MDB, LAG,
TC/ACL, QoS, policing, mirror or PTP offload. Linux can still forward many of
these functions in software through the CPU, but that is not equivalent to
complete ASIC offload. The huge vendor API was therefore not mislabeled as
“fully supported”.

## Hardware validation status

Validated statically/through host compilation:

- patch syntax and exact application;
- C/D profile code compiles with warnings promoted to errors;
- RTL8197F NIC compiles with warnings promoted to errors;
- YAML parses;
- DTS/property consistency and archive integrity checks.

Still requiring physical boards:

- boot and traffic on each chip ID/revision;
- SMI timing and reset polarity;
- switch- and SoC-side RGMII delay sweep with FCS counters;
- DSA CPU-tag RX/TX and source-port decode;
- bridge/VLAN isolation under netifd;
- suspend/reset/reprobe behavior;
- MTU and sustained bidirectional throughput;
- optional SSC-off mode;
- old B-map swconfig extif settings.

The supplied RD05 v34 report already states that full physical RX/DSA delivery
was not proven. v35 does not convert that unresolved hardware test into a claim.
