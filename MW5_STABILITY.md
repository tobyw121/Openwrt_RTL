# MW5 software offload policy and WLAN RX changes

The current policy keeps both Linux nftables flow offloading and the custom
MW5 software flow cache disabled. This supersedes the software-offload defaults
in the older v44.65.21/v44.65.25 notes. The reported symptom is loss of forwarding
on all interfaces during a speed test; its hardware root cause is not yet proven.

## Changes

- The Ethernet driver's software-flow gate defaults off before userspace runs.
  Master/hardware activation no longer implicitly enables that gate.
- First boot, saved-config migration, firewall/port repairs, delayed WAN recovery
  and `mw5-accel apply` keep `flow_offloading` and `flow_offloading_hw` at `0`.
  `mw5-accel software on` returns an error while this policy is in force.
- `/etc/config/mw5_accel` now matches the UCI namespace used by the helper.
  The MW5 base package installs the S94 accelerator-policy service link in
  squashfs so it does not depend on a writable overlay to enable the service.
- MW5 WLAN packets received during NAPI are collected in a per-radio delivery
  queue and handed to GRO after the vendor RX/software-queue locks are released.
  This avoids a second per-packet enqueue to the CPU networking backlog and
  permits receive batching. GRO still traverses the normal Linux bridge,
  routing, firewall and conntrack; it does not bypass forwarding policy.
- Timer-driven/reorder delivery outside the poll retains `netif_rx()`.
  Polling with budget zero does not process RX or complete NAPI. Existing
  positive-budget accounting and periodic 5 GHz TX reclaim are preserved.
- The historical `mw5_netif_rx_pkt` counter counts stack handoffs, including
  GRO delivery; it is no longer a count of CPU backlog enqueues.

No RF power, regulatory/DFS rules, channel widths, calibration or flash partition
boundaries are changed. DMA/DSA hardware assists retain their existing policy.
Low-level diagnostic proc/module parameters are not a supported way to enable
software acceleration in this baseline.

## Host validation

From the repository root:

```sh
python3 -m unittest discover -s scripts/tests -v
python3 scripts/validate-mw5-classic-routing.py
```

The regression tests execute shell policy functions against fake UCI/proc
interfaces, including old configurations and repairs. They compile and run the
actual C NAPI/delivery functions against mocked kernel/HAL boundaries with
warnings as errors and undefined-behaviour instrumentation. They check zero,
partial and exhausted budgets, interrupt rearming, queued delivery outside vendor
locks, non-poll fallback, closed devices and periodic TX reclaim. These are host
logic tests, not a full kernel/module or firmware build.

The attempted `make prereq` did not pass in the provided cloud instance: GNU awk,
rsync and ncurses development files are missing, and the default umask must be
changed to 022 for OpenWrt builds. A prepared target toolchain/kernel and feeds
are also absent. No new firmware image or MIPS module has been validated here.

## Required MW5 hardware validation

Build the MW5 firmware in a prepared OpenWrt environment using
`configs/rtl8197f_mw5_defconfig`. After installing the resulting image, check:

```sh
mw5-accel apply
mw5-accel status
uci get firewall.@defaults[0].flow_offloading
uci get firewall.@defaults[0].flow_offloading_hw
uci get mw5_accel.core.software
```

All three UCI values must be `0`; `/proc/rd05-rtknet` must report `sw=0`.
Repeat after reboot, a port-mode change and a firewall repair. The source changes
alone do not modify a router already running older firmware.

Compare with the previous firmware with both software accelerators disabled:

1. Test LAN-to-WAN routing, then 2.4 GHz and 5 GHz WLAN-to-WAN separately.
   Exercise upload, download and simultaneous bidirectional traffic for at least
   ten minutes each. Record throughput, CPU/softirq load, latency and drops.
2. Keep an existing SSH session open and repeatedly establish new SSH sessions
   during each test. Ping alone is insufficient for the reported failure mode.
3. Use the existing `MW5-LIVE-AUDIT` collector to preserve driver/ring state and
   packet captures on the PC. Capture the kernel log over serial if available.
4. Test WLAN association, interface stop/start, reboot and traffic between
   stations to catch GRO/bridge and teardown regressions.

Throughput gains and resolution of the all-interface stall remain unmeasured
until these hardware tests pass. Do not re-enable flow offloading to compensate
for a regression without first isolating the failure.
