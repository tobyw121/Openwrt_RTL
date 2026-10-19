# MW5 software offload policy and WLAN RX changes

The current policy keeps both Linux nftables flow offloading and the custom
MW5 software flow cache disabled. This supersedes the software-offload defaults
in the older v44.65.21/v44.65.25 notes. The reported symptom is loss of forwarding
on all interfaces during a speed test; its hardware root cause is not yet proven.

## Network liveness without diagnostic readers

Driver `1.8.35-mw5-independent-watchdog-v44.66.13` addresses two concrete gaps
relevant to a live serial console but stalled network that appears stable only
while `mw5-netdiag full` runs:

- MW5 IRQ-mask and RX/TX doorbell writes now have a same-register readback to
  complete posted MMIO writes in the control path itself. Correctness must not
  depend on a later diagnostic register read.
- The existing timer now checks MW5 RX and TX progress even after recent RX IRQ
  activity. Previously recent RX IRQs skipped this pass entirely, and TX-only
  lost completion IRQs were not checked. TX readiness requires a valid CDP in
  the pending ring interval **and** a hardware-cleared OWN bit, under `tx_lock`.
  The timer schedules normal NAPI cleanup; it never unmaps/reuses DMA buffers,
  rewinds rings or reprograms forwarding tables itself.
- An RX publication wait discovered during the timer's current readiness check
  arms the 1 ms retry immediately, instead of waiting a full watchdog period.
- Ethernet NAPI budget-zero calls reclaim TX only, without consuming RX recovery
  state or completing NAPI. TX cleanup remains included in diagnostic totals.

`rx_poll ... tx_fallback=N` in `/proc/rd05-rtknet` counts watchdog-scheduled polls
with a confirmed TX completion. `mw5-netdiag full` includes its delta as
`rd05_tx_watchdog`. Idle and hardware-owned TX rings do not trigger extra polls;
the existing watchdog period and the retired software-flow policy are unchanged.

The compiled host regression reproduces missed RX/TX scheduling with recent RX
activity, posted control writes, ring wrap, invalid CDPs, OWN/CDP disagreement,
publication waits, stopped interfaces, and zero-budget NAPI. It runs the actual
driver functions against mocked MMIO/DMA, with no userspace diagnostic reader.
This verifies those logic defects, **not** the hardware root cause of the report.
No full target kernel build or physical MW5 reproduction has been completed in
this cloud instance.

For hardware acceptance, cold-boot the new image and first run the previously
failing speed test **without starting netdiag, ethtool dumps or register polling**.
Keep only the serial console open; check new SSH/TCP connections from a client
while testing LAN, 2.4 GHz and 5 GHz separately. Repeat after idle and reboot.
Capture counters/logs afterward, or immediately over serial if forwarding fails.
Do not treat stability observed only during `netdiag full` as passing this test.

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

The AP writer now explicitly enables the Short Guard Interval capabilities
already advertised by the two MW5 wiphys: SGI20 for HT20, SGI20/40 for wider HT
channels, and SGI80 for the 5 GHz VHT80 profile. Previously the generated hostapd
configuration omitted these flags, and the driver's beacon parser then cleared
its SGI settings. WMM and A-MPDU remain enabled for the HT/VHT profiles.

At the same modulation, stream count and channel width, Short GI can raise the
PHY rate by about 11%; this is not a measured throughput improvement. Actual
benefit depends on client negotiation, RF conditions and CPU load. The change
does not force a higher modulation, fixed bitrate, transmit power or wider
channel. The explicit `g-ht20` profile now also keeps HT20 when a saved width is
40. Legacy b/g profiles do not gain HT/VHT capabilities.

After deploying a build containing these changes, use the normal apply command
to regenerate hostapd configuration. Both per-band SGI settings default to `1`
when absent from an older configuration. For an A/B comparison, disable SGI:

```sh
mw5-wifi save --2g-short-gi 0 --5g-short-gi 0
mw5-wifi apply
```

Re-enable it:

```sh
mw5-wifi save --2g-short-gi 1 --5g-short-gi 1
mw5-wifi apply
```

The UCI options are `mw5wifi.radio2g.short_gi` and `mw5wifi.radio5g.short_gi`.
Applying WLAN settings interrupts associations; run from wired LAN or serial.
Configuration persistence still depends on the image's writable storage/overlay.

`mw5-netdiag full` also reads the per-radio debugfs `gro_packets`, `gro_merged`
and `backlog_packets` counters when available. `gro_packets` counts frames handed
to GRO; `gro_merged` counts `GRO_MERGED`/`GRO_MERGED_FREE` results; `backlog_packets`
counts delivery outside NAPI. These are separate from the legacy `/proc` handoff
counter. Missing legacy counters and incomplete monitor summaries now report
`NA`, rather than inventing a zero measurement. The new snapshot reads only
named diagnostic counters, not MMIO/register dumps.

Compare each band separately with the same client, position, channel and width,
using an iperf server on a wired PC rather than on the MW5. Record both traffic
directions, simultaneous traffic, actual negotiated rates, RSSI and new SSH/TCP
connection success. The uploaded log had a 2.4 GHz client at -74 dBm and no 5 GHz
client, so it cannot establish a 5 GHz performance baseline or a throughput gain
from these new settings.

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
