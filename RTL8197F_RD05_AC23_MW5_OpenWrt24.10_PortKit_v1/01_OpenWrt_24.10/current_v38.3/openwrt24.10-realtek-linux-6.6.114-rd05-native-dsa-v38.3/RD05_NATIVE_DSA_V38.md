# Xiaomi RD05 native DSA v38

This OpenWrt 24.10 / Linux 6.6.114 tree extends the v37 hardware-feedback port
with additional RTL8197F and RTL8367-family material audited from the supplied
multi-SDK archives.

## RTL8197F / RTL8197FH-VG

- 64-input, two-bank interrupt controller support.
- Correct BSP interrupt numbers for UART0 (9), UART1 (38), UART2 (39), USB host
  (13), SPI0 (23) and SWCORE/NIC (15).
- Three 8250/DW UART slots; UART1 and UART2 remain disabled in the generic board
  description until a board-specific pinmux is selected.
- P0/RGMII setup runs during NIC probe before DSA switch setup.
- RTL8197FH/VG PITCR and EXTPCR0 fixed-IPG fields are programmed to the BSP
  eight-byte values.
- Existing 8-dword CPU-DMA descriptors, ring re-arm, CPU-tag preservation and
  bounded diagnostics remain active.
- Native USB2 glue enables the SoC clock, host mode and both PHYs, applies the
  25/40 MHz BSP tuning sequences and then creates generic EHCI/OHCI children.
  The RD05 DTS deliberately leaves the block disabled by default.

## RTL8367 family

The DSA identification table now covers SDK auto-probe families represented by
IDs 0x0276, 0x0597, 0x6367 and the RTL8367D/RB-VC ID 0x6642. Known upstream
0x6367 revisions keep their more specific descriptors ahead of the fallback.
RTL8367D continues to use its own CPU7/EXT1, force-MAC, VLAN/PVID, MIB, SSC and
RGMII registers and does not import incompatible RTL8365MB reset/jam tables.

## Versioned RGMII profile

`/etc/rd05-rgmii.conf` uses `PROFILE_VERSION=38` and defaults to:

```text
SOC_TX=0
SOC_RX=6
SW_TX=0
SW_TXC=0
SW_RX=5
SSC=1
```

Apply it explicitly with:

```sh
rd05-rgmii-calibrate apply 0 6 0 0 5 1
```

Older v36/v37 profiles are preserved as `.pre-v38` and no longer silently
replace corrected kernel defaults.

## Validation boundary

Patch syntax, Linux 6.6.114 patch preparation, source compilation, script
syntax and archive round-trip tests are performed before distribution. These
checks do not replace an RD05 hardware test. The currently observed physical
link still has no confirmed router-to-PC reply and therefore requires the v38
TX/RX calibration and end-to-end test after flashing.
