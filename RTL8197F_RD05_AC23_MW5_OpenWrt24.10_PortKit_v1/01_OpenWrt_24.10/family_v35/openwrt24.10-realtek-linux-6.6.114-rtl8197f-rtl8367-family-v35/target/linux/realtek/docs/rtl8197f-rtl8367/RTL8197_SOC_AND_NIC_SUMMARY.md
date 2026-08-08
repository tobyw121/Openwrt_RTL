# RTL8197 SoC variants and RTL8197F NIC integration

## RTL8197 variants encountered in the supplied material

The archive contains BSP, Ethernet or WLAN references for RTL8197D/DL/DN,
RTL8197F/FS/FH/FH-VG/FE, and RTL8197G/GE generations. These names do not imply
one interchangeable Ethernet block or one common board setup.

The OpenWrt target in this tree is actively built around **RTL8197F and
RTL8197FH-VG**. The Ethernet binding also lists RTL8197D and RTL8197G compatible
strings for code reuse, but this delivery does not claim that the complete
clock, reset, IRQ, flash, pinmux and board support for those SoCs has been
hardware-validated. RTL8197G references in the SDK are dominated by later WLAN
HAL/backport code rather than a completed Linux 6.6 SoC port.

## RTL8197F Ethernet architecture

The driver controls two coupled hardware domains:

1. the CPU-interface DMA descriptor/ring engine;
2. the internal rtl865x switch core and its physical P0 RGMII MAC.

An external RTL8367 is not reached through descriptor destination bit 6. The
vendor `CONFIG_RTL_83XX_SUPPORT` path routes it through **physical P0**, so TX
descriptors must use `DP=BIT(0)`, `DP_EXT=0`, and source extension zero.

### CPU-interface register window

| Offset | Register/function |
|---:|---|
| `0x000` | CPUICR: TX/RX enable, burst, CRC convention, DMA mode |
| `0x004` | CPURPDCR0: RX packet-header descriptor ring 0 |
| `0x01c` | CPURMDCR0: RX mbuf ring 0 |
| `0x020` | CPUTPDCR0: TX descriptor ring 0 |
| `0x028` | CPUIIMR: interrupt mask |
| `0x02c` | CPUIISR: interrupt status |
| `0x030/0x034/0x038` | queue-to-DMA controls |
| `0x03c/0x040/0x044` | DMA controls 0..2 |
| `0x060/0x064` | TX descriptor rings 2/3 |
| `0x068` | DMA control 3 |
| `0x078` | TXRINGCR |
| `0x080` | CPUIMCR |
| `0x0a0` | DMA control 4 |
| `0x0a4` | CPUICR1: descriptor stride/endian/LX/queue mapping |

v34/v35 retain the post-start rearm that programs eight dwords for RX and TX
descriptors and rewrites the ring bases after the interface opens.

### rtl865x switch-core registers relevant to the cascade

| Offset | Register/function |
|---:|---|
| `0x4000` | MACCR, including P0 gigabit-link indication |
| `0x4058` | MACCR1, P0 router mode and receive tag mode |
| `0x4100` | PITCR, P0 external-interface selection |
| `0x4104 + 4*p` | PCRP(p), force link/speed/duplex/pause/STP/reset |
| `0x414c` | P0GMIICR: CPU tag RX/TX, RGTXC, TX/RX delay, CONF_DONE |
| `0x4204` | SIRR/TRXRDY sideband |
| `0x4408/0x440c` | remark/ALE controls |
| `0x4500...` | switch buffer/queue controls |
| `0x4750...0x4778` | queue, remark and rate controls |
| `0x4800...` | per-queue rate controls |
| `0x4904...0x4910` | loopback/test controls |
| `0x5100` | MACCTRL1 |
| `0x5168...0x5170` | internal ingress-private controls |
| `0x5204` | IPv6 control |
| `0x6300` | table memory reset/control |

The complete 276-symbol driver-side index is in
`RTL8197F_NIC_REGISTER_MAP.tsv`.

## v35 external-master modes

### DSA mode

`realtek,rtl8367-dsa-master`:

- direct P0 descriptor destination;
- eight-dword descriptors;
- CPU-interface/SWCORE/TRXRDY start sidebands;
- protocol-4, eight-byte `rtl8_4` tag accepted but not stripped on P0 RX;
- SoC-internal VID9/netif/L2/ACL seed contains only P0;
- external user ports remain represented by DSA slave interfaces.

### Legacy swconfig mode

`realtek,rtl8367-legacy-master`:

- same physical P0/RGMII and DMA startup;
- CPU-tag recognition disabled;
- intended for the older `rtl8367`/`rtl8367b` drivers with B-map init tables;
- mutually exclusive with DSA mode.

### Timing properties

| Property | Range | Default |
|---|---:|---:|
| `realtek,p0-rgmii-tx-delay` | 0..1 | 0 |
| `realtek,p0-rgmii-rx-delay` | 0..7 | 5; RD05 6 |
| `realtek,p0-rgtxc` | 0..3 | 3 |

Switch-side and SoC-side delays are independent. Both must be recorded in a
board DTS; copying the RD05 RX6 value to another PCB without eye/FCS testing is
not justified.

## Combination profiles

### RTL8197F + NIC + RTL8367D

- switch ID/revision `0x6642:0x0030`;
- switch CPU port 7 / EXT1 / MAC7;
- D-map force-select and SDS1/TOP_CON0 routing;
- D EXT1 SSC enabled unless explicitly disabled;
- generic delays: switch TX0/RX5, SoC TX0/RX5;
- RD05 delays: switch TX0/RX6, SoC TX0/RX6.

### RTL8197F + NIC + RTL8367RB-VC

This is the same D-register-map implementation as RTL8367D in the supplied SDK.
Use `realtek,rtl8367rb-vc`; runtime probe must still return `0x6642:0x0030` for
the exact profile.

### RTL8197F + NIC + RTL8367RB-VB

- C-map `0x6367:0x0020`;
- usual CPU link physical port 6 / EXT0;
- C external-interface SSC enabled by the template;
- generic switch/SoC start point TX0/RX5;
- use `rtl8197f-rtl8367c-rb-vb-smi-template.dtsi` as a disabled board template.

### RTL8197F + old RTL8367RB/R-VB/B-map

Use the optional legacy swconfig modules plus
`rtl8197f-rtl8367b-legacy-swconfig-template.dtsi`. Confirm the exact extif
logical port and init-table revision before enabling the node. The DSA aliases
will reject the old silicon because its runtime ID is not a known C/D family ID.
