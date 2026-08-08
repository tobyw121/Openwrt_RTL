
## v42.6 MW5 rootfs boot-length marker

- Reconstructed the OEM second-stage rootfs verification routine.
- Added the big-endian marker at SquashFS `+0x08`; the bootloader adds `0x282`.
- Recalculates and validates the rootfs trailer checksum after changing the marker.
- Rejects timestamp-style markers that would make the bootloader appear to hang.
- Added stock-template and synthetic regression tests.

# RTL8197F OpenWrt 24.10 development tree v42.0

## Board evidence and separation

- Added `board-profiles-v42.json` with source, confidence and unresolved fields
  for Xiaomi RD05, Tenda Nova MW5 and Tenda AC23/Lynx.
- Added a validator which checks DTS and image constants against the database.
- Added an on-device `rtl8197f-board-check` diagnostic command.
- MW5 now uses the OEM-observed GPIO33 active-low red status LED and GPIO54
  active-low reset key.
- AC23 is restricted to the RTL8367RB/RB-VC switch family instead of falling
  back to the unrelated RTL8367D initialization path.

## Full SPI flash safety

- MW5 uses the exact OEM MTD map and replaces only `0x030000..0x5c0000`.
- AC23 uses one combined firmware window `0x030000..0x7e0000`; the unsupported
  fixed rootfs offset from v41 was removed.
- AC23 `IMG_HEADER_T` payload alignment is 60 bytes (`0x3c`), derived from the
  matched stock dump. Kernel length and checksum are read from the header.
- Both Tenda builders validate kernel checksum, header-derived SquashFS offset,
  SquashFS v4 metadata and the Realtek/Tenda rootfs checksum trailer.
- Builders erase the unused firmware tail, preserve private regions byte for
  byte, write the result, read it back and compare it with the generated buffer.
- Added a common offline verifier for template identity and protected ranges.
- MW5 stock firmware can be reconstructed byte-for-byte into the original SPI
  dump. AC23 passes a complete synthetic combined-firmware insertion test while
  preserving boot/factory/config regions.

## Ethernet and switches

- Retained the verified RD05 RTL8367D map and MW5 port 1 LAN / port 3 WAN / port
  6 CPU topology.
- The MW5 RTL8363NB-class patch no longer applies the unrelated RTL8365MB-VC
  cold-start table. Only the recovered RTL8363/RTL8364 sparse sequence and
  board-specific OEM post-initialization registers are applied.
- The missing exact RTL8363NB cold-start jam table remains an explicit blocker.

## WLAN reconstruction

- Replaced the MW5-only package with `kmod-rtl8192cd-rtl8197f` for all boards.
- Included recovered RTL8197F, RTL8822B, RTL8812F and RTL8814B firmware, HAL,
  PHYDM, HALRF and power tables from the SDK collections.
- Added DT/profile selection for RTL8197FS (MW5) and RTL8197FH/VG (RD05/AC23).
- Added module telemetry for initialization phases and failure return codes.
- Extended telemetry through netdevice allocation, MIB construction, IRQ,
  HCI-DMA, power-on, MAC, firmware, BB/PHY, RF tables, LCK/IQK calibration and
  runtime queue activation.
- External PCIe registration can be disabled for an integrated-radio-only test;
  the PCI descriptors are skipped instead of being misclassified as integrated.
- OEM `/proc/wlan*/mib_all` data resolves MW5 `wlan1` as integrated RTL8197FS
  2.4 GHz RFE5 and `wlan0` as PCIe RTL8822B RFE6 (`10ec:b822`). The older
  RTL8812BRH inventory entry is retained as a conflicting, weaker source.
- Added controlled load, diagnostic and dry-run-first AP bring-up tools. The AP
  helper reconstructs the OEM private-MIB ordering but requires `--apply` and
  an explicit regulatory-domain value before changing an interface. Automatic
  module/radio loading remains disabled until IRQ, DMA, EFUSE, calibration and
  RF output are verified.

## Validation status

- Static tree, JSON, Python, shell and patch-hunk checks: passed.
- MW5 reconstructed AP private-MIB dry-run sequence: passed.
- Private SPI template SHA-256 and structural checks: passed for all boards.
- MW5 exact fullflash round-trip: passed.
- AC23 combined-firmware fullflash preservation/readback test: passed.
- Full OpenWrt compile was not completed in the analysis environment because
  GNU awk and ncurses development headers were unavailable.
- No hardware flash/boot/RF test is claimed. A formal image verifier cannot
  replace UART boot and SPI readback validation on the physical device.

- Added `scripts/rtl8197f-spi-program-verify.sh`: two identical pre-write chip
  reads, template provenance check, guarded flashrom write, full-chip readback,
  byte comparison and post-write board/protected-region validation.

- Made `sha256.c` self-contained instead of including all rtl8192cd headers;
  this removes a needless dependency on Linux bridge-private headers from the
  first helper object and improves Linux-6 build diagnostics.


## v42.3

See `V42_3_CHANGELOG.md`. Fixes the MW5 module-local SHA-256/PRF linkage at modpost.

## v42.4

See `V42_4_CHANGELOG.md`. Fixes the fixed MW5 0x590000-byte firmware budget without borrowing CFM/LOG/ENV space.

## v42.5 - MW5 16-byte boot header correction

MW5 no longer copies the OEM payload's 40-byte NOP prefix in front of the
OpenWrt LZMA loader. The bootloader-visible header is 16 bytes and the first
loader instruction is now placed at RAM address 0x80a00000 as linked.

## v42.7

See `V42_7_CHANGELOG.md`: early RTL8197F watchdog stop and MW5 UART alias fix.

## v42.8

See `V42_8_CHANGELOG.md`: exact RTL8197F watchdog stop at raw MIPS kernel entry and K/S/P early UART diagnostics.

## v42.15

Correct the RTL8197F watchdog stop sequence to vendor SDK behavior: kick with bit 23, then exact `0xa5000000`. Post-P diagnostic markers no longer touch the watchdog.

## v42.17

- Fix the late MW5 watchdog reset in `rtl8197f_oem_switch_preinit()`.
- Replace the unsafe bare WDTCNR stop-pattern write with the vendor-required kick-then-stop sequence and MMIO readbacks.
- Stop the watchdog before and after the two one-second MW5 switch-reset delays.


## v42.18

- Add an RTL8197FS DW-8250 byte-TX alias quirk and MW5 DTS override.
- Preserve UART DLL programming by using the alias only when DLAB is clear.
- Add direct runtime-console and root-mount diagnostics after the physical v42.17 boot completed initcalls and initramfs waiting.
- Patch the direct diagnostic `serial8250_console_putchar()` to the proven byte LSR/TX addresses so normal VFS/panic output can become visible.

## v42.20

- Enable the existing RTL8197F `cr6c` firmware splitter on Tenda MW5 and AC23.
- Remove the firmware-node `compatible` property that made OpenWrt
  `mtd_partition_split()` skip all firmware parsers.
- Add a regression check for the DTS/mtdsplit interaction.
- The direct MW5 test image keeps the v42.19 console/root-mount diagnostics and
  forces the parser branch in the already-built kernel so the old embedded DTB
  can be tested without another world build.

## v42.21

- Correct MW5 SPI auto-map reads to use exact flash offsets (`read_shift=0`).
- Preserve v42.20 RTL8197F firmware/rootfs splitter enablement.
- Add source-tree regression checks for `cr6c`/`hsqs` coordinate integrity.

## v42.22

- Preserve the standards-compliant SquashFS `bytes_used` field.
- Keep the Tenda BE16 rootfs checksum as an external 4 KiB-aligned trailer.
- Fix Linux 6.6 `unable to read id index table` caused by the former `bytes_used + 2` encoding.
- Add source-tree regressions for the 8-byte final ID index table invariant.


## v42.23

- Complete the RTL8197FS DW-8250 quirk with DLAB-safe RX and TX aliases at
  `+0x24`; DLL remains at `+0x00`.
- Correct MW5 descriptor routing from the external-switch CPU port number to
  RTL8197F physical P0.
- Apply the Realtek SDK P0/RGMII settings for RTL8197FS: TX0/RX5, non-VG pad
  drive, 1 Gbit/s/full duplex and CPU-tag pass-through for Linux DSA.
- Add regressions for UART input, direct P0 descriptor routing and MW5-specific
  RGMII configuration.

## v42.24

See `V42_24_CHANGELOG.md`: harden the post-LZMA cache-size reload while retaining the RTL8197FS UART RX and MW5 P0/RGMII fixes.
