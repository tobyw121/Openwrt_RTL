# Realtek driver-port report: RD05 native DSA v35

## Result

The active OpenWrt target contains native Linux 6.6 implementations for the
RTL8197F platform, PCIe host, SPI/MTD, CPU-DMA Ethernet path and the RTL8367D
DSA switch.  Legacy copied SDK stacks have been removed from the build tree.

## Main changes from v34

- corrected the RTL8197F receive loop to use `CPURPDCR0` as the authoritative
  hardware consumer pointer;
- enabled both P0 CPU-tag RX and TX capability bits while preserving the
  complete eight-byte tag for Linux DSA;
- restored the SDK production baseline for RTL8367D EXT1 timing;
- added a generic Linux PCI host driver for the RTL8197F one-slot root complex;
- enabled RD05 PCIe 40 MHz/VG tuning and endpoint diagnostics;
- parsed permanent Ethernet/WLAN addresses from the CRC32 U-Boot environment;
- removed the inactive Linux-4.4 rtknet and embedded switch-SDK copies;
- documented the RTL8812FE FullMAC source without falsely enabling it as a
  native Linux 6.6 wireless driver.

## Active target patch set

The Realtek target uses 37 active Linux 6.6 patches.  Patch 312 adds the native
RTL8197F PCIe controller and patch 325 adds the RTL8367D DSA variant.  All patch
files are checked with both Git's patch parser and GNU patch syntax handling.

## Compile checks

The new PCIe host driver was compiled as an external module against the
available kernel headers with `W=1` and warnings promoted to errors.  The
project validator also checks all expected overlays, configuration symbols,
Device Tree markers, scripts, source-map entries and absence of stale vendor
stacks.

A full target/world build is attempted separately in the validation workflow.
If external source downloads or missing host packages prevent it, that is
reported rather than counted as a successful firmware build.

## Hardware status inherited from v34

The supplied RD05 log proves:

- the RTL8367D is detected and DSA tree setup completes;
- the 8-dword descriptor format and non-zero ring bases are accepted;
- the calibrated SPI-NOR write-enable sequence works;
- `rootfs_data` mounts persistently as JFFS2;
- LAN2 link and switch counters operate;
- CPU7 still accumulated FCS/drop counters in the tested timing setup;
- no valid external RX verdict was possible without coordinated peer traffic.

v35 addresses the missing SDK receive-pointer semantic and provides better
counter visibility.  A new physical test is still required.

## Deliverable status

Source audit: complete for RD05-relevant RTL8197F, RTL8367D, PCIe and WLAN
reference trees.

Native wired/platform port: implemented and statically validated.

Native WLAN: not claimed; the supplied code is a vendor FullMAC architecture
and requires a separate mac80211/cfg80211 rewrite.

Physical-router proof: pending for v35 PCI enumeration, RGMII error-free load,
switch-to-SoC RX delivery, DSA port isolation/VLAN/FDB and flash endurance.
