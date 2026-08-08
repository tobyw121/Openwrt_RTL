# Xiaomi RD05 native DSA v37

V37 is the source-audited follow-up to the v36 hardware run on the Xiaomi
Mi WiFi R4 RD05 (RTL8197F/RTL8197FH-VG plus RTL8367D).

## What the v36 hardware log proves

The native Linux/DSA control plane is present: RTL8367D is detected, CPU7/EXT1
is linked to RTL8197F P0/RGMII, the 8-byte `rtl8_4` tag path is selected, both
RTL8197F CPU-tag gates are enabled and 32-byte CPU-DMA descriptors/ring bases
are accepted.

The physical CPU link is not yet calibrated:

- RTL8367D CPU7 records almost every SoC-originated frame as an FCS error/drop;
- LAN2 ingress and CPU7 output counters grow, but RTL8197F `CPURPDCR0` stays on
  descriptor zero, OWN stays set and all Linux RX counters remain zero;
- no `tx-auto` or `rx-auto` matrix was actually executed in the supplied log;
  the device remained on SoC TX0/RX6, switch TX0/RX2, SSC off.

The log also exposes two independent startup defects:

- the first rtl865x/SWCORE register seed races reset defaults and six writes
  read back as zero before the delayed replay succeeds;
- OpenWrt preinit tries the overlay before lazy SPI-NOR WEL calibration, so the
  first mount may fall back to tmpfs even though a later JFFS2 mount succeeds.

Finally, the SWCORE interrupt counter stays at zero. RTL8197F writes actual
MIPS CPU IRQ values into IRR nibbles, while the generic Realtek multi-output
irqchip assumed output index plus one. V37 offsets RTL8197F output numbering so
DTS selector 3 maps IRR value 4 to the CPU IRQ4 parent used by the BSP.

## Exact RTL8367D family-D timing register

The complete RTL8367D register map in the supplied SDK defines:

- `0x1307`: public coarse EXT1 TX delay and EXT1 RX delay;
- `0x13f9[5:3]`: a separate three-bit EXT1 RGMII fine TX-clock delay;
- `0x13f7[10:8]/[7:0]`: synchronizer FIFO TX/RX error state;
- CPU7 miscellaneous register `0x00ee[8]`: CRC_SKIP.

V36 intentionally avoided `0x13f9` after treating it as another-family field.
The exact RTL8367D header proves that assumption wrong. V37 programs only the
EXT1 field in `0x13f9`, preserves the other EXT fields, exposes it as
`rd05_ext1_txc_tap=0..7`, reports synchronizer state and reports CRC_SKIP while
leaving CRC validation enabled.

## Complete directional calibration

Router/SoC to switch direction:

```sh
rd05-rgmii-calibrate tx-auto save
```

This sweeps SoC TX delay, switch RX tap and SSC. It scores CPU7 ingress against
FCS/drop deltas.

Switch to SoC direction requires continuous PC traffic. Start on the PC:

```sh
sudo ./tools/rd05-pc-netdiag.py \
  --interface enx00e04c5562c0 \
  --configure-ip \
  --calibration \
  --duration 240
```

At the same time on the router:

```sh
rd05-rgmii-calibrate rx-auto save
```

V37 tests all 128 combinations of switch coarse TX delay, all eight RTL8367D
`0x13f9` fine TX-clock taps and all eight RTL8197F P0 RX taps. A profile is
accepted only when CPU7 is emitting traffic and the RTL8197F RX ring or Linux
RX counters actually advance. If no candidate works, the previous profile is
restored instead of persisting a false result.

Saved profile format:

```text
SOC_TX=0
SOC_RX=6
SW_TX=0
SW_TXC=4
SW_RX=2
SSC=0
```

## Other v37 corrections

- non-destructive WEL command/status calibration runs before MTD partition
  registration, so the first OpenWrt preinit attempt can mount `rootfs_data`;
- rtl865x/SWCORE table writes wait for a nonzero reset-default PVCR0 and retry
  later instead of permanently accumulating startup failures;
- the RTL8197F-specific irqchip parent offset is part of the active patch set;
- legacy `rd05-v34-check`, `rd05-v35-check` and `rd05-v36-check` commands forward
  to the v37 checker.

## Validation sequence

```sh
rd05-v37-check
rd05-rgmii-calibrate tx-auto save
# Run PC calibration traffic, then:
rd05-rgmii-calibrate rx-auto save
rd05-v37-check
rd05-netdiag full
```

A successful result requires zero new CPU7 FCS/drop errors, a moving RX current
pointer, nonzero `eth0` RX, nonzero `rd05_rx_external` or `rd05_rx_arp`, correct
DSA-slave delivery, a nonzero `eth0` interrupt count under traffic, and a
persistent JFFS2 `/overlay` immediately after boot.
