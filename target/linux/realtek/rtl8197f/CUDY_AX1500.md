# Cudy WR1500 / RE1500 / M1500

These devices now have separate experimental **UART/RAM bring-up profiles**
in the RTL8197F target. They are not flashable router firmware support yet.
WR1500 is the first planned hardware test; RE1500 and M1500 remain untested.
Hardware revisions have not been identified, so support for every revision
sold under these names is not implied.

## Evidence and current integration

Source: [Cudy AX1500 GPL analysis](../../../../reverse-engineering/gpl-ax1500/README.md).
Archive SHA256:
`2413dbd852e6f039d402f8ffda62e20b9f7b88a38dde0e7d86a527078ed66b93`.
[cudy-ax1500-profiles.json](cudy-ax1500-profiles.json) records source hashes,
known build settings and unresolved hardware properties for each model.

| OpenWrt profile | GPL profile | External switch enabled by GPL | Current integration |
| --- | --- | --- | --- |
| `cudy_wr1500` | R76 | RTL8367RB-VC | UART and RAM-test image recipe |
| `cudy_re1500` | R101 | No RTL8367RB-VC | UART and RAM-test image recipe |
| `cudy_m1500` | R102 | RTL8367RB-VC | UART and RAM-test image recipe |

All three GPL kernel configs select RTL8197F-VG/V672, little-endian MIPS and
16 MiB SPI flash. Their configured kernel offset is `0x40000`, rootfs offset
`0x2b0000`. These values are evidence about the vendor build, not a complete
validated flash map. In particular, they do not identify all OEM settings,
signatures, factory data or the correct Linux auto-map byte shift.

The GPL enables `CONFIG_RTL8852AE_BACKPORTS`, but does not supply a corresponding
complete chip driver source tree. No physical PCI ID, firmware/calibration
mapping or working radio initialization has been confirmed on these units.
The RD05/MW5/AC23 radio profiles are therefore not assigned to the Cudy models.
The RE1500 must not inherit the WR1500 external-switch topology.

## Build the first WR1500 test profile

In `make menuconfig`, select Realtek / RTL8197F and the Cudy WR1500 profile
marked **experimental UART RAM boot**. Enable **Target Images -> ramdisk**.
The relevant resulting configuration includes:

```text
CONFIG_TARGET_realtek=y
CONFIG_TARGET_realtek_rtl8197f=y
CONFIG_TARGET_realtek_rtl8197f_DEVICE_cudy_wr1500=y
CONFIG_TARGET_ROOTFS_INITRAMFS=y
```

Then use the normal OpenWrt build workflow (`make defconfig`, `make`).
The profile recipe produces an initramfs artifact ending in
`cudy_wr1500-initramfs-ram-loader.bin`; the other profiles use their respective
model names. No factory, sysupgrade or full-SPI image is defined. A full
OpenWrt firmware build of these new profiles has not yet been completed here.

The artifact is a raw Realtek self-extracting RAM loader, **not a U-Boot
uImage or OEM upgrade file**. It is linked for RAM address `0x82000000` and
decompresses the kernel to `0x80100000`. The existing loader overlap check
reserves a 64 KiB margin before the loader, and the output size is capped
at 31 MiB, leaving at least 1 MiB for BSS, stack and decoder workspace below
the end of the selected diagnostic memory window.

That window is capped at **64 MiB**, a bring-up limit rather than a measurement
of installed RAM. Before attempting to load the artifact, confirm from the
OEM boot log that at least 64 MiB of contiguous RAM exists and determine the
bootloader's supported RAM transfer/jump commands. No command to write flash,
replace the bootloader, or run an unverified TFTP/bootm procedure is supplied.
Board-specific clocks and UART routing are inherited from our RTL8197F-VG
platform implementation and still need validation on Cudy hardware.

## Runtime isolation and diagnostics

Flash controllers, Ethernet, switch diagnostics, GPIO, WLAN, PCIe and USB are
disabled in the Cudy DTS. UART0 is enabled at 115200 8N1. There are no invented
SMI pins, LED polarities, MAC addresses, port labels or calibration offsets.
The common WLAN and WebUI startup scripts skip these RAM profiles, and the
network board script does not create a LAN/WAN topology for them.

The upgrade platform rejects these boards both during image checking and in
the upgrade handler. Forcing past the image check still does not select a
flash upgrade path. This is specific to these experimental profiles.

Once an independently verified RAM-loading method reaches the shell, run:

```sh
cudy-ax1500-diag
```

It reads kernel, CPU, memory and interrupt information without probing raw
flash or changing hardware/network settings. Linux `MemTotal` reflects the
64 MiB cap; it cannot establish the installed physical memory size.

## Next hardware evidence and implementation stages

1. Record WR1500 case revision, full OEM UART boot log, actual RAM capacity
   and RAM-loading commands; retain an original per-device SPI backup.
2. Validate this UART/RAM profile, watchdog handling and timer frequency.
3. Establish the stock partition map and byte mapping before enabling
   read-only flash access; locate each device's own factory/calibration data.
4. Verify SMI wiring, external-switch ID and physical port order for WR1500
   and M1500; reconstruct the separate RE1500 Ethernet path. Only then enable
   Ethernet and test DHCP/ARP in both directions.
5. Identify both radio paths and port the required firmware/calibration and
   cfg80211 integration; do not substitute an unrelated 8812/8822 profile.
6. Add a validated firmware wrapper and upgrade path after those checks.

Validation so far: all three DTS files compile against Linux 6.6.114 headers;
GNU make template tests verify that inherited flash image outputs are cleared;
shell tests verify upgrade refusal and startup isolation. These checks are
not a successful device boot or proof of working Ethernet/WLAN.

The raw loader was additionally cross-compiled for all three board macros with
a synthetic LZMA payload. The Cudy macros select the Realtek UART and early
watchdog path in both `head.S` and `board.c`, rather than the inherited Atheros
UART path. This compile check used stock MIPS GCC 14 with explicit little-endian
output, Linux assembly headers and freestanding linking; the OpenWrt-only
`-fhonour-copts` flag was omitted in temporary validation copies. No synthetic
payload is distributed as firmware, and this is not a full image build.
