# R76 RX payload appearing at Ethernet offset0 — 2026-11-21

Latest user counters: rx_bad_desc=0, dma_errors=0, timeouts=0, ring_full=0,
but bad_tag3616, bad_port1, bad_l3=19. WAN and CPU FCS counters0.
RX/TX counters continue increasing. A rejected161-byte frame starts:
63636570743a202a2f2a0d0a436f6e6e656374696f6e3a20
ASCII: `ccept: */*\r\nConnection: `. These are HTTP payload bytes rather than
Ethernet destination/source/tag. This is stronger evidence than merely a
missing DSA tag, but does NOT distinguish receive-buffer offset, premature
completion, overwrite or wrong descriptor association. Do not guess an ingress
port or prepend invented MAC/DSA headers to such data.

Scoped RD05/R76 diagnostics now retain the failing frame's descriptor words,
ring index, CPURPDCR0 sample and expected DMA buffer pointer alongside the
existing header/length snapshot. The descriptor is read in NAPI before recycle,
not obtained later from a potentially reused slot. The proc view is a best-
effort diagnostic while live traffic continues, not a synchronized packet trace.
No ring reset, switch-table rewrite, parser relaxation or MW5-path change.
This is instrumentation for the unresolved failure, NOT a claimed traffic fix.
Eight existing parser/DMA/snapshot regressions pass; no full MIPS build or
hardware verification of the added fields in this environment.

PC rebuild:
```sh
cd ~/Schreibtisch/openwrt_git_RTL/Openwrt_RTL
git pull --ff-only origin MW5_Stable
make target/linux/clean
make -j"$(nproc)" V=s
```

After installation: WLAN remains disabled. UART38400, first capture baseline:
```sh
cat /proc/cudy-r76-rtknet
```
PC reproduce with the SAME previously failing LAN-to-WAN workload, then:
```sh
ping -c 5 192.168.1.1
curl -I --max-time 5 http://192.168.1.1/
```
UART after failure:
```sh
cat /proc/cudy-r76-rtknet
ethtool -S wan
```
Send routed_last_bad and routed_bad_dma together plus counters. Do not reboot
before obtaining them. No need to repeat unchanged captures on the old image:
that version cannot report the added descriptor context.
