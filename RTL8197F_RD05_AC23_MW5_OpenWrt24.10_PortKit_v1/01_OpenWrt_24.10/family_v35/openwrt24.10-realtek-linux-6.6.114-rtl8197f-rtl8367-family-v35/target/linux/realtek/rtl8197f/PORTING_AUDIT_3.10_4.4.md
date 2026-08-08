# RTL8197 / RTL8367D 3.10/4.4-to-6.6 porting audit

This audit answers the four explicit review questions for the current
`openwrt-Realtek-openwrt-24.10` tree.  It is intentionally conservative: an
item is marked *ported* only when the Linux 6.6/OpenWrt 24.10 tree carries a
native driver, DTS binding, kernel config, or target patch that replaces the
legacy Kernel 3.10/4.4 Realtek mechanism with an upstream-style Linux API.

## 1. RTL8197D/F/FH/G register differences

| Variant/source | Legacy evidence checked | Linux 6.6 status | Result |
| --- | --- | --- | --- |
| RTL8197D / RTL819xD (Kernel 3.10) | `../openwrt-rtk819-main/target/linux/realtek/rtl819xd/kconfig/config_rtl8197DL` and `config_rtl8197DN_extPA` select an RLX5281, big-endian, `rtl819xd` platform with `CONFIG_RTL_8197D`, `CONFIG_RTL_819X_SWCORE`, layered L2/L3 driver symbols, and optional `CONFIG_RTL_8367R_SUPPORT`. | The active 6.6 subtarget is MIPS 24K little-endian RTL8197F, not RLX big-endian RTL8197D.  The native CPU-interface driver deliberately does not expose a `realtek,rtl8197d-rtknet` match because there is no RTL8197D DTSI/register map or hardware-validated board file. | **Not ported as a complete RTL8197D variant.** Treat the compatible as a future hook only. |
| RTL8197F / RTL8197F_VG / RTL8197FH-VG (Kernel 4.4) | `target/linux/realtek/rtl8197f/vendor-target/config.linux-4.4.*` and `preconfig_*Kernel4.4*` select MIPS24K little-endian, `CONFIG_RTL_8197F`, `CONFIG_RTL_8197F_GW`, `CONFIG_RTL_8197F_VG`, board revisions such as V355I/V612/V672/V622G, and SPI-NAND/RD05-style 8367R boards. | The 6.6 tree carries an RTL8197F subtarget, `rtl8197f.dtsi`, GPIO/SPIROM/UART bring-up, a native `rtl865x` CPU-interface Ethernet driver, and RD05 DTS wiring for the RTL8367D CPU path.  `rtl8197fh.dtsi` wraps the same register map with FH-specific compatibles and F fallbacks. | **Partially ported.** Core F/FH/RD05 addresses are represented, but board-revision-specific deltas remain hardware-validation items. |
| RTL8197G | Kernel 4.4 vendor backports configs contain `CPTCFG_WLAN_HAL_8197G`, but this audit did not find a native RTL8197G SoC DTSI/register-map port in the 6.6 target. | The native binding deliberately does not expose `realtek,rtl8197g-rtknet`: there is no RTL8197G SoC DTSI, clock/reset description, or validated Ethernet/SWCORE register delta. | **Not ported as a complete RTL8197G variant.** Add a compatible only after the RTL8197G register deltas are described and tested. |

Current native RTL8197F addresses/configuration that are covered:

- CPU Ethernet interface: `ethernet@10000`, `reg = <0x00010000 0x1000>`, IRQ
  `<15 3>`, RGMII, 64-entry rings, 2048-byte RX buffers, and KSEG1-style DMA
  address programming.
- Switchcore/syscon reference: `switchcore@1b800000`,
  `reg = <0x1b800000 0x10000>`, IRQ `<4>`.
- RD05 external-switch transmit metadata: `tx-port-mask = 0x40`,
  `tx-dp-ext = 0x2`, `tx-extspa = 0x0`.

## 2. Board configs from Kernel 3.10/4.4

Checked board/config groups:

- Kernel 3.10 Realtek configs:
  - `../openwrt-rtk819-main/target/linux/realtek/rtl819xd/config-3.10`
  - `../openwrt-rtk819-main/target/linux/realtek/rtl819xd/kconfig/config_rtl8197DL`
  - `../openwrt-rtk819-main/target/linux/realtek/rtl819xd/kconfig/config_rtl8197DN_extPA`
  - `../openwrt-rtk819-main/target/linux/realtek/rtl8881a/kconfig/config_rtl8881AB+8367R`
- Kernel 4.4 RTL8197F vendor configs staged in this tree:
  - `target/linux/realtek/rtl8197f/vendor-target/config.linux-4.4.*`
  - `target/linux/realtek/rtl8197f/vendor-target/preconfig_*Kernel4.4*`
  - `target/linux/realtek/rtl8197f/vendor-target/config.backports-5.2.8-1.*`
  - `target/linux/realtek/rtl8197f/vendor-target/config.users.*`

Result:

- The **Kernel 4.4 RTL8197F_VG + RTL8367R/RB board family** is the only family
  substantially mapped into the 6.6 target.
- The 6.6 config intentionally keeps the port small and native: it enables the
  RTL8197F platform, GPIO, MTD/SPIROM, DSA Realtek SMI/MDIO, `rtl8365mb`, and
  the native `CONFIG_RTL8197F_RTKNET` driver.
- The Kernel 3.10 RTL8197D/RLX board configs are **not converted** into DTS
  board files or 6.6 Kconfig fragments.  Their CPU endian/architecture and
  switch-core assumptions are different enough that they need a separate
  RTL8197D platform port rather than reuse of the RTL8197F DTSI.
- WLAN backports configs and board-specific WLAN HAL choices are **not** part of
  the native 6.6 Ethernet/switch port.

## 3. RTL8367D switch initialisation tables

Legacy facts checked:

- `target/linux/realtek/rtl8197f/vendor-target/SDK_PORTING_NOTES.v11` lists the
  already-extracted RTL8367D facts: chip ID `0x6642`, CPU mask/control registers
  `0x1219`/`0x121a`, unknown/broadcast flood masks `0x0890`/`0x0891`/`0x0892`,
  unknown multicast action registers `0x08c9`/`0x08cb`/`0x08cd`, unknown
  unicast action register `0x09c0`, and SDK EXT-port numbering.
- `config.linux-4.4.*` selects the external switch path with
  `CONFIG_RTL_8367RB_VC=y`, `CONFIG_RTL_83XX_SUPPORT=y`,
  `CONFIG_RTL_83XX_API_V1_4=y`, GPIO-SMI on H0/G7, and
  `CONFIG_RTL_EN_UNKNOWNUC_ONLY_TO_CPU=y`.

Linux 6.6 mapping:

- Patch `321` adds RTL8367-family compatibles to the in-kernel Realtek DSA
  SMI/MDIO drivers.
- Patch `325` adds the OEM RTL8367D chip ID `0x6642`.
- Patch `326` avoids applying an unsafe borrowed reset/jam sequence to RTL8367D.
- Patch `327` programs the RD05 CPU/user forwarding and unknown/broadcast flood
  behavior into the Linux DSA path.
- Patch `328` keeps switch detection table-driven while tolerating compatible
  `0x003x` RTL8367D steppings.

Result:

- **The minimal RD05/RTL8367D CPU-port bring-up facts are ported.**
- **The complete vendor RTL8367D init/jam table is not fully ported.**  The 6.6
  path deliberately avoids copying opaque Realtek init tables unless they can be
  mapped to documented DSA/phylink/PHY behavior or proven necessary on hardware.
  Remaining candidates include full LED programming, per-port PHY quirks,
  complete VLAN/STP defaults, EEE/LPI policy, MIB policy, mirror/trap tables,
  and any chip-specific analog/jam values not yet represented by Linux DSA.

## 4. Replaceable Fastpath/NAT functionality through modern APIs

Legacy facts checked:

- Kernel 3.10 RTL8197D configs carry layered L2/L3 driver symbols and optional
  Realtek fastpath-related hooks.
- Kernel 4.4 RTL8197F configs include standard netfilter NAT plus
  `CONFIG_RTL_HARDWARE_NAT` in at least the EasyMesh R2/WFA backports config.
- The staged vendor source contains Realtek private `rtl_ps_hooks`,
  `rtl865x_*nat*`, fastpath, hardware NAT, bridge/L2 refresh, and WLAN HWNAT
  hooks.

Linux 6.6 mapping decision:

- None of the private Realtek NAT/fastpath hooks are built by default.
- The native replacement direction is:
  1. keep basic switching/bridging in Linux DSA + bridge + switchdev,
  2. use normal Linux netfilter/nftables NAT in software first,
  3. ship OpenWrt `kmod-nf-flow` / `kmod-nft-offload` on RD05 so firewall4 can
     use the standard nftables software flowtable path,
  4. add hardware offload only through standard Linux flow offload callbacks if
     the rtl865x engine can be described safely without reintroducing vendor
     fastpath hooks.

Result:

- **Replaceable functionality is partially enabled through standard software
  flow offload, but not implemented as rtl865x hardware offload.**
  Mainstream-safe substitutes are DSA/switchdev for L2 and
  netfilter/nftables/flowtable for L3/NAT, but RTL8197F rtl865x hardware NAT
  offload still needs a separate design and hardware validation.

## Overall conclusion

The current tree has **not** completely ported every RTL8197D/F/FH/G register
variant, every Kernel 3.10/4.4 board config, every RTL8367D init table, or all
Realtek NAT/fastpath acceleration.  It has ported the maintainable subset needed
for native RTL8197F/FH RD05 bring-up:

- modern `net_device` + NAPI rtl865x CPU-interface driver,
- OpenWrt/DTS wiring for RTL8197F/FH and RD05,
- Linux DSA Realtek SMI/MDIO path for RTL8367D-family probing,
- RD05-specific RTL8367D CPU-port forwarding essentials.

The remaining work should be tracked as explicit per-board/per-chip follow-up
rather than hidden behind broad compatible strings.

## Follow-up guard added after this audit

The native `rtl8197f_rtknet` binding and OF match table now expose only the
validated `realtek,rtl8197f-rtknet` and `realtek,rtl8197fh-rtknet` compatibles
plus the generic `realtek,rtl865x-cpuif` fallback.  Unvalidated RTL8197D/G
strings are intentionally withheld so OpenWrt does not advertise a complete
port before those register maps and board configs are actually converted.

## RD05 management defaults added after this audit

The RD05 image now includes `kmod-nf-flow` and `kmod-nft-offload`, and the
first-boot firewall defaults enable nftables software flow offload while keeping
`flow_offloading_hw=0`.  This replaces the legacy Realtek fastpath/NAT hooks
with the standard OpenWrt/Linux flowtable path until a real rtl865x hardware
offload driver exists.  The RD05 DSA network defaults also bridge all exposed
RTL8367D user ports (`lan1`..`lan4`) so the RTL8197F CPU can manage them through
normal Linux DSA/netifd/bridge tooling.
