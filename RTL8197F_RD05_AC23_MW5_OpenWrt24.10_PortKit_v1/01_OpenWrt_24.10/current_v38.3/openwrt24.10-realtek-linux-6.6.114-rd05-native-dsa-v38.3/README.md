# OpenWrt 24.10 native RTL8197F / RTL8367D tree for Xiaomi RD05 — v38.1

This tree targets the Xiaomi Mi WiFi R4 RD05 with Linux 6.6.114 and contains a
source-audited native port of the wired and platform hardware found in
`8197_all_SDKs.tar.zst` and the multi-SDK `entpackt.7z` collection.

## Implemented natively

- RTL8197F/RTL8197FH-VG MIPS platform support
- dual-bank interrupt controller, timer, UART0/UART1/UART2, GPIO and reset/watchdog
- SHEIPA SPI controller and bounded writable SPI-NOR MTD path
- U-Boot environment NVMEM parsing and permanent Ethernet MAC
- RTL8197F PCIe root complex
- RTL8197F 8-dword CPU-DMA Ethernet master with early P0/RGMII and VG IPG setup
- RTL8197F-specific IRQ parent routing plus the second 32-input IRQ bank
- SDK-compatible CPURPDCR0 receive-pointer handling
- RTL8197F P0/RGMII CPU-tag pass-through
- RTL8367C/R/RB/S/SB/D/RB-VC identification, SMI/regmap and Linux DSA support
- RTL8367D coarse and family-D fine RGMII timing calibration
- five DSA user ports plus CPU7/EXT1
- bridge, VLAN, FDB, STP, phylink and per-port MIB integration
- native RTL8197F USB2 host glue for generic EHCI/OHCI (board-disabled by default)
- persistent JFFS2 `rootfs_data` overlay
- RD05-specific router diagnostics and `tools/rd05-pc-netdiag.py`

## Important WLAN boundary

The OEM hardware is identified as an integrated RTL8197F radio plus external
PCIe RTL8812FE (`10ec:f812`).  The supplied wireless source is the legacy
`rtl8192cd` FullMAC stack with private APIs; it is inventoried but intentionally
not presented as a native Linux 6.6/mac80211 port.

See:

- `RD05_SDK_NATIVE_PORT_V35.md` for the full technical analysis;
- `RD05_SDK_SOURCE_MAP.tsv` for exact SDK source paths and hashes;
- `RD05_SDK_COMPONENT_INVENTORY.tsv` for audited component sizes and dispositions;
- `RD05_WLAN_NATIVE_STATUS_V35.md` for the WLAN boundary;
- `REALTEK_DRIVER_PORT_REPORT.md` for build and validation status.

## Build

```sh
make defconfig
make -j1 target/linux/compile V=s
make -j$(nproc)
```

After booting on an RD05:

```sh
rd05-v38-check
rd05-netdiag full
```

Keep serial recovery and a full flash backup available.  The final
switch-to-SoC RX path, RGMII timing, PCI enumeration and multi-port DSA
forwarding still require validation on the physical router.

See `RD05_NATIVE_DSA_V38.md` for the latest SDK-derived extensions: early
RTL8197F P0/VG-IPG programming, 64-input IRQ routing, all three UART nodes,
native USB2 glue, versioned RGMII profiles and expanded RTL8367-family IDs.


See `RD05_NATIVE_DSA_V38_1.md` for the UART polling and deferred SPI calibration boot-safety correction.

## RD05 v38.3 calibration safety update

See `RD05_NATIVE_DSA_V38_3.md` for the BusyBox fractional-sleep fix, atomic
RGMII sweep rollback and the hardware interpretation of the v38.2 LAN test.
