# RD05 Native DSA v38.3

Calibration-safety update based on the first complete v38.2 hardware boot and
paired LAN2/PC capture.

## Hardware findings

- v38.2 reaches userspace with UART0, MTD, RTL8367D detection, native DSA and
  the RTL8197F CPU DMA rings initialized.
- The PC capture contains no router frame (`router_mac=-`, `peer_frames=0`,
  `from_router=0`).
- RTL8367D port 7 transmits toward the SoC while `eth0` RX and the RTL8197F
  current RX descriptor pointer remain static.
- Router-to-switch traffic increments CPU7 FCS/drop counters.
- The v38.2 calibration helper used `sleep 0.2`. The target BusyBox rejects
  fractional sleep values, so both sweeps aborted after applying their first
  candidate and could leave the runtime profile at all zeros.

## v38.3 corrections

- uses integer millisecond timing with `usleep`, `busybox usleep`, or a safe
  whole-second fallback;
- never invokes fractional BusyBox `sleep`;
- snapshots all six RGMII parameters before a sweep;
- restores the snapshot automatically on errors, signals and Ctrl+C;
- verifies sysfs readback after every profile application;
- saves a winning profile only after a successful complete sweep;
- adds `rd05-rgmii-calibrate restore` as an alias for the SDK baseline;
- detects and reports an all-zero runtime profile;
- updates `rd05-v38-check` and `rd05-netdiag` to recognize v38.2/v38.x kernel
  markers and removes the stale 64-input-IRQ wording.

## Recovery from an aborted v38.2 sweep

Run before collecting any additional traffic data:

```sh
rd05-rgmii-calibrate sdk
rd05-rgmii-calibrate status
```

Expected baseline:

```text
SOC_TX=0
SOC_RX=6
SW_TX=0
SW_TXC=0
SW_RX=5
SSC=1
```

Then run TX calibration first. RX calibration requires continuous PC traffic.

```sh
rd05-rgmii-calibrate tx-auto save
```

PC:

```sh
sudo ./tools/rd05-pc-netdiag.py --interface IFACE --configure-ip \
  --calibration --duration 300
```

Router, while the PC command is running:

```sh
rd05-rgmii-calibrate rx-auto save
```

A failed or interrupted v38.3 sweep restores the exact pre-sweep profile.

## Scope

v38.3 deliberately does not change RTL8197F descriptor format, CPU-DMA start
order, RTL8367D DSA tagging or the boot-proven v38.2 platform code. The 8-dword
new-descriptor format matches the cache-aligned RTL8197F SDK build. A valid
RGMII matrix must be collected before making another kernel datapath change.
