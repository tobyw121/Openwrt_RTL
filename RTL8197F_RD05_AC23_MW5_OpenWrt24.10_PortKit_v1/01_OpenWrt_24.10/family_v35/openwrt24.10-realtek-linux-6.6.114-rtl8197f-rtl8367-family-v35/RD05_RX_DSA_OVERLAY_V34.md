# Xiaomi RD05 v34: calibrated SPI-NOR, post-start rings and RGMII timing

## Evidence from the v33 hardware run

The post-start descriptor correction worked: CPUICR1 read back as `0x002081d3`
and the hardware retained both RX and TX ring addresses. The test window did not
contain peer-generated LAN2 traffic, so zero CPU RX packets in that run are not
a valid negative RX verdict.

The RTL8367D CPU7 counters did record FCS errors for frames sent by the SoC. v34
changes only the EXT1 receive sampling tap from 5 to 6. DSA protocol-4 tagging,
CPU7 selection, VLAN state and the RTL8197F receive-tag preservation remain
unchanged.

## SPI-NOR calibration

The v33 trace repeatedly returned raw RDSR `0x00000004`, with no observable WEL
under the assumed byte/bit mapping. v34 does not guess that mapping. Before the
first rootfs_data operation it proves a WEL clear/set/clear/set transition using
WRDI and WREN, testing command FIFO widths in this order:

1. 32-bit write with opcode in bits 31:24 (Realtek GPL SDK convention)
2. 8-bit write
3. 16-bit write
4. 32-bit write with opcode in bits 7:0

The detected status byte lane and bit position are normalized for WEL/WIP polls.
If calibration fails, erase/program is refused. The writable window remains
strictly limited to `0x00e60000..0x00ffffff`.

## Verification

Run after boot:

```sh
rd05-v34-check
rd05-netdiag full
```

`rd05-v34-check` brings up LAN and waits for the post-start marker before
reporting, avoiding the false early failures seen in the v33 log. Real hardware
confirmation of v34 is still required.
