# RTL8197F v42.26 changelog

## MW5 LAN/WAN CPU-DMA correction

Physical v42.25 testing proves that the external RTL8363/RTL8367 switch and both
DSA user links reach 1 Gbit/s, but the RTL8197F CPU datapath does not move a
single frame: `eth0` RX remains zero, TX descriptors retain OWN, and NETDEV
WATCHDOG resets ring 0.

The Realtek SDK enables `CONFIG_RTL_TX_CACHE_ALIGN` and
`CONFIG_RTL_RX_CACHE_ALIGN` only for `CONFIG_RTL_8197F_VG`. Those options append
two reserved dwords and select an eight-dword/32-byte ring stride. The Tenda
Nova MW5 is an RTL8197FS non-VG device, so its native new-descriptor format is
six dwords/24 bytes and CPUICR1 leaves CF_TXDESC/CF_RXDESC at zero. v42.25 used
32-byte software rings while the hardware walked the 24-byte default.

v42.26 therefore:

- selects six dwords / 24 bytes for `tenda,nova-mw5`;
- keeps eight dwords / 32 bytes for VG/RD05 profiles;
- allocates, indexes, frees and audits rings with a runtime descriptor stride;
- programs DMA_CR1 with `(ring_entries - 1) * descriptor_stride`;
- avoids touching opts6/opts7 on six-dword descriptors;
- reapplies CPUICR1, CPURPDCR0 and CPUTPDCR0 only after TXCMD/RXCMD and TRXRDY
  are active, because the first v42.25 open read back zero ring bases;
- adds explicit `realtek,descriptor-dwords = <6>` to the MW5 DTS;
- adds source-tree regressions for the non-VG/VG split.

## Expected hardware readback

For MW5:

```text
registered eth0: ... desc=6/24 ...
rtl8197f poststart v42.26: hw-start-active desc=6/24 ...
cpuicr1=0x000001d3
dma_cr1=0x000005e8
cpurpdcr0=<non-zero> txbase=<non-zero>
```

For a 64-entry TX ring, `0x5e8 == 63 * 24`. TX completion and RX packet
counters must then advance before any further CPU-port or RGMII calibration is
considered.
