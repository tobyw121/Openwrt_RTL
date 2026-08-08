# OpenWrt 24.10 / Linux 6.6.114 RTL8197F v42.27 development tree


**Critical v42.27 MW5 P0-to-CPU pipeline fix:** the v42.26 hardware log proves that LAN/WAN frames reach the RTL8363 user ports while `eth0` RX remains zero. v42.27 enables the missing RTL8197FS internal rtl865x PVID/VLAN/netif/L2/unknown-to-CPU/ACL seed for MW5, uses the live device MAC, and repeats it after SWCORE stabilizes. See `V42_27_CHANGELOG.md`.
**Critical v42.25 MW5 loader relocation:** a full world build exceeded the old `0x80100000..0x80a00000` decompression window by `0xd5b` bytes. The MW5 loader is now linked/headered at `0x80d00000`, and the image pipeline rejects future kernel/loader overlap before linking. See `V42_25_CHANGELOG.md`.
**Critical v42.26 MW5 CPU-DMA descriptor fix:** the non-VG RTL8197FS SDK uses 6-dword/24-byte new descriptors. v42.25 incorrectly used the RTL8197F-VG 8-dword/32-byte cache-aligned layout, while hardware readback retained zero stride fields. v42.26 selects 24 bytes for MW5, retains 32 bytes for VG/RD05, and rearms CPUICR1 plus both ring bases after CPUIF start. See `V42_26_CHANGELOG.md`.
See `V42_24_CHANGELOG.md` and `RTL8197F_v42.24_VALIDATION.txt` for the RTL8197FS UART-RX and MW5 P0/RGMII CPU-link corrections.

v42.24 retains the hardware-proven v42.22 rootfs mount and OpenWrt userspace boot. It adds the missing receive-side UART alias and replaces the incorrect extension-port descriptor route with the Realtek SDK physical-P0/RGMII topology for the external RTL8367-class switch.

This tree consolidates the recovered Realtek GPL SDK sources, OEM boot logs,
SPI dumps, `/proc`/sysfs collections and board inventories for:

- Xiaomi R4/RD05: RTL8197FH-VG + RTL8367D + RTL8812F PCIe
- Tenda Nova MW5: RTL8197FS + RTL8363NB class + integrated 2.4 GHz + external
  RTL8822B (`10ec:b822`) from OEM runtime; RTL8812BRH remains an inventory conflict
- Tenda AC23/Lynx: RTL8197FH + RTL8367RB class + RTL8814B/RTL8814BR candidate

## Recommended bring-up order

1. Build and UART-boot MW5 initramfs; do not write SPI first.
2. Verify RAM, timer, SPI read-only access and the full 8 MiB dump.
3. Validate RTL8197F TX/RX DMA before enabling DSA.
4. Capture RTL8363NB SMI registers immediately after reset and after OEM init.
5. Confirm LAN/WAN links, CPU-port RGMII timing and sustained traffic.
6. Load WLAN manually with `rtl8197f-wlan-load` in integrated-only mode.
7. Validate firmware, EFUSE/RFE, IRQ, DMA rings and low-power RF operation.
8. Enable the external PCIe radio only after PCI enumeration is stable.
9. Transfer the proven common platform fixes to RD05, then AC23.

## Fullflash commands

Run `VERIFY_FULLFLASH_TEMPLATES_V42.sh` before building. Full images are
personalized from the attached board-matched SPI dumps and contain private MAC,
calibration and configuration data. Never publish or flash them to another unit.

The MW5 builder preserves boot, CFG, CFM, CFM backup, log and ENV. The AC23
builder preserves `0x000000..0x030000` and `0x7e0000..0x800000`. Each builder
validates and reads back the generated file. This provides strong formal safety,
but successful hardware boot still has to be demonstrated through UART and a
post-write programmer readback.

## WLAN diagnostic commands

```sh
rtl8197f-wlan-load --probe
rtl8197f-wlan-load
rtl8197f-wlan-load --dual
rtl8197f-wlan-check /tmp/wlan-report.txt

# Print the reconstructed MW5 2.4 GHz AP sequence without applying it
rtl8197f-wlan-ap-test --profile mw5 --band 2g \
  --ssid MW5-LAB --regdomain 1 --open --dry-run
```

The first actual radio tests should use the integrated path only. `--dual`
allows PCIe registration for the external radio. The MW5 OEM data maps `wlan1`
to integrated RTL8197FS/RFE5 and `wlan0` to RTL8822B/RFE6. The AP test helper is
non-destructive unless `--apply` is specified. Phase 90 means only that module
initialization returned zero; phases through 390 expose later HAL/RF stages but
do not prove RF power, calibration, AP operation or regulatory correctness.
See `MW5_WLAN_RECONSTRUCTION_V42.md` and the package phase document.

## Known hard blockers

- Exact RTL8363NB cold-start/jam table is not present in the collected SDKs.
- AC23 RAM size, GPIOs, physical switch labels and RGMII delay are not confirmed
  by a live boot dump.
- The MW5 OEM runtime and PCI evidence identify RTL8822B; the physical target
  should still be checked because one board inventory names RTL8812BRH.
- The vendor WLAN source still uses compatibility warning suppressions and must
  be compiled and exercised on the target before it can be considered stable.

## Physical SPI programming safety

`scripts/rtl8197f-spi-program-verify.sh` provides the final external-programmer
workflow. It reads the chip twice before writing, requires both reads to match,
requires the live chip to match the personal template, invokes `flashrom -v`,
reads the complete chip again, compares every byte and reruns the board/layout
verifier. Writing additionally requires `--write --yes-i-understand`.

This verifies image construction and physical readback. It still cannot promise
that unverified GPIO, RGMII, switch or WLAN assumptions will boot and operate on
a specific router. Keep UART recovery and the original two backups.

## v42.2 WLAN compiler correction

The Linux 6.6.114 build log from the real OpenWrt toolchain reached
`WlanHAL/HalCommon.c` and stopped because the combined multi-chip build enabled
several switch cases that each declared `hci_type` in the same C scope. v42.2
uses chip-specific selector names and also removes the adjacent SDK merge
warnings for beamforming defaults, descriptor macro replacement, the duplicate
RTL8197F thermal-meter register and malformed power-sequence continuations.

Rebuild only the WLAN package first:

```sh
make package/kernel/rtl8192cd-rtl8197f/{clean,compile} V=sc -j1
```

A successful package compile removes only the source/API blocker. Do not load
the module automatically or write a full-flash image until the staged UART,
Ethernet, switch and WLAN checks in this document have been completed.

### MW5 boot-header requirement (v42.5)

The MW5 bootloader copies from `firmware + 0x10` to `0x80a00000` and jumps to
that address. OpenWrt MW5 images must therefore use a 16-byte header. Do not
prepend the 40-byte OEM payload prefix unless the loader was explicitly linked
for a `+0x28` runtime position.


v42.15 corrects the watchdog logic to the vendor SDK sequence: set WDT_CLEAR bit 23, then write the exact disable magic `0xa5000000`. The earlier `0xa5f00000` value was incorrect.


## v42.17 MW5 late watchdog correction

The board-local OEM switch preinit now uses the Realtek vendor-required sequence: reload WDTCNR by setting bit 23, then write exact `0xa5000000`, with readbacks. This is done before and after the MW5 two-second switch reset. The previous bare stop-pattern write could itself cause an immediate watchdog reset.


## v42.18 MW5 runtime console and root-mount diagnosis

MW5 UART0 now overrides `reg-io-width` to byte access and supplies `realtek,tx-alias-offset = <0x24>`. The DW-8250 patch redirects only TX writes while DLAB is clear, so divisor-latch programming remains at offset zero. The direct diagnostic image marks `/dev/console`, `/init`, device probing, root-device selection, squashfs/jffs2 mount attempts, panic and successful root move/chroot.

## v42.24 MW5 interactive UART and SDK P0/RGMII correction

A physical v42.22 boot reaches `procd`, mounts the SquashFS root and starts the
OpenWrt console.  Pressing Enter then generated continuous NUL bytes because
RTL8197FS RBR and THR both use the byte-only `+0x24` alias, while DLL remains at
`+0x00` under DLAB.  The DW-8250 quirk now has independent RX/TX aliases and
uses either alias only while DLAB is clear.

The external RTL8363/RTL8367 switch-side CPU port number is not an RTL8197F DMA
port number.  The Realtek GPL SDK connects that switch through RTL8197F physical
P0/RGMII.  MW5 therefore uses direct descriptor destination P0 (`DP=0x1`, no
`DP_EXT`), P0 forced 1 Gbit/s/full duplex, RTL8197FS TX0/RX5 RGMII timing,
non-VG pad drive and CPU-tag RX/TX pass-through without MAC tag removal.  DSA
continues to own the external switch ports and the on-wire rtl8_4 tag.

Hardware validation still requires an interactive UART shell plus bidirectional
LAN/WAN packet counters and ping/ARP traffic.  Link-up alone is not proof of a
working CPU data path.
