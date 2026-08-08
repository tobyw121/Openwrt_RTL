# OpenWrt 24.10 RD05 Native-DSA v38.3

## Purpose

v38.3 is a calibration-safety and diagnostics update for the boot-proven v38.2
RTL8197F + RTL8367D/RB-VC tree. It does not change the kernel Ethernet, CPU-DMA,
DSA tagger, switch register programming, Device Tree, UART or SPI-NOR drivers.

## Hardware evidence from v38.2

The router reaches OpenWrt userspace and initializes:

- UART0/ttyS0;
- RTL8197F SPI-ROM and all fixed MTD partitions;
- RTL8367D/RB-VC ID 0x6642, revision 0x0030;
- RTL8197F P0/RGMII master and RTL8367D CPU7/EXT1 DSA link;
- 8-dword CPU-DMA descriptor format and valid ring bases;
- persistent JFFS2 overlay after lazy SPI WEL calibration.

The paired PC test still receives no router frame. On the router, LAN2 and
RTL8367D port-7 output counters increase, but eth0 RX, `rx_cdp_advanced`,
`rd05_rx_external` and `rd05_rx_arp` remain zero. CPU7 FCS/drop counters also
increase in the opposite direction.

## v38.2 calibration defect

The v38.2 helper set `APPLY_SETTLE=0.2` and called `sleep 0.2`. The target
BusyBox rejects fractional sleep values. Because the first candidate had already
been programmed when the shell exited under `set -e`, repeated TX and RX sweep
attempts left the runtime link at:

```text
SOC_TX=0
SOC_RX=0
SW_TX=0
SW_TXC=0
SW_RX=0
SSC=0
```

This invalidated all traffic measurements collected afterward.

## v38.3 changes

`rd05-rgmii-calibrate` now:

1. uses integer milliseconds;
2. prefers `usleep`, then `busybox usleep`, then rounds up to whole-second
   `sleep` when no microsecond applet exists;
3. snapshots all six active parameters before TX/RX sweeps;
4. installs EXIT, HUP, INT and TERM rollback handlers;
5. restores the exact pre-sweep profile on errors and Ctrl+C;
6. returns nonzero after signal interruption;
7. verifies sysfs readback after every profile application;
8. saves only a completed winning profile;
9. provides `restore` as an alias of `sdk`;
10. warns when the runtime profile is all zero.

`rd05-v38-check` now accepts the actual v38.2/v38.x kernel markers, detects the
all-zero profile and describes IRQ 15 correctly: RX polling remains active even
when the interrupt count is zero.

`rd05-netdiag` also accepts v38.x markers and prints a critical warning before
interpreting traffic collected with an all-zero profile.

Package release is increased from 10 to 11.

## Immediate recovery on a running v38.2 image

Before any more traffic tests:

```sh
rd05-rgmii-calibrate sdk
rd05-rgmii-calibrate status
```

Expected:

```text
SOC_TX=0
SOC_RX=6
SW_TX=0
SW_TXC=0
SW_RX=5
SSC=1
```

Do not run the v38.2 `tx-auto` or `rx-auto` commands again; they contain the
fractional-sleep bug. Upgrade the helper or firmware to v38.3 first.

## v38.3 test sequence

```sh
rd05-v38-check
rd05-rgmii-calibrate tx-auto save
```

For RX calibration, run continuous traffic on the PC:

```sh
sudo ./tools/rd05-pc-netdiag.py \
  --interface enx00e04c5562c0 \
  --configure-ip \
  --calibration \
  --duration 300
```

While that runs on the PC:

```sh
rd05-rgmii-calibrate rx-auto save
```

Then:

```sh
rd05-v38-check
rd05-netdiag full
```

Success requires a real router frame at the PC and nonzero RTL8197F RX progress.

## Validation

- project validator: 228 PASS, 5 WARN, 0 FAIL;
- POSIX `sh -n`: PASS;
- BusyBox ash syntax: PASS;
- forced full-sweep error rollback: PASS;
- forced TERM interruption rollback: PASS, exit status 143;
- no fractional sleep remains;
- v38.2-to-v38.3 patch roundtrip: exact;
- application script: exact and idempotent;
- full ZIP and TAR.ZST roundtrips are verified separately.

The remaining Ethernet fault is not claimed fixed until a complete v38.3 timing
matrix is collected. The current evidence still points to the physical
P0/EXT1 sampling path before DSA receive decoding.
