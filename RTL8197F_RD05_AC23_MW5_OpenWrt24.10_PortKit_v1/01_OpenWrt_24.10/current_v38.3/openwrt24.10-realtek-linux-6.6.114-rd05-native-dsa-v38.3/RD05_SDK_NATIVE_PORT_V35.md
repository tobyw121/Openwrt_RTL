# Xiaomi RD05: native RTL8197F / RTL8367D port for OpenWrt 24.10

## Scope

This tree is a source-audited Linux 6.6/OpenWrt port for the Xiaomi Mi WiFi R4
RD05.  The hardware path covered by the native port is:

```text
MIPS 24Kc / RTL8197FH-VG
        |
        +-- RTL8197F CPU DMA and switch-core port 0
        |          |
        |          +-- RGMII, 1 Gbit/s, full duplex
        |                    |
        |                    +-- RTL8367D EXT1 / logical CPU port 7
        |                              |
        |                              +-- UTP0..UTP4 -> lan1, lan2, wan,
        |                                  lan3, lan4 through Linux DSA
        |
        +-- SHEIPA SPI controller -> GD25Q128-compatible 16 MiB NOR
        |
        +-- RTL8197F PCIe root complex -> PCI 10ec:f812 / RTL8812FE
        |
        +-- GPIO, interrupt controller, UART, timer and reset/watchdog
```

The SDK archive used for this audit is `8197_all_SDKs.tar.zst`, SHA-256
`4a899dcd2768066354d6365ff3eb7818f76114aa5977a55b971198b841e2ccfb`.
The second supplied archive is byte-identical.  The extracted archive contains
38,617 regular source files from several generations.  The exact files used as
technical references and their individual SHA-256 hashes are listed in
`RD05_SDK_SOURCE_MAP.tsv`.

The implementation deliberately does **not** copy the legacy Realtek NAT,
FastPath, private netfilter, procfs control plane or parallel Linux-2.6/3.x
network stack into Linux 6.6.  Their hardware-relevant initialization and ring
semantics were translated into normal kernel subsystems.

## Source selection

The archive is not one coherent SDK.  It contains overlapping RTL819x,
RTL83xx, Linux 3.10, Linux 4.4, bootloader and application trees.  For the RD05,
the authoritative references were selected as follows:

1. `MR62X_SDK_Paket/.../target/linux/target/bsp` for the RTL8197F platform,
   interrupt, GPIO and PCIe host sequence.
2. `MR62X_SDK_Paket/.../target/linux/rtknet/drivers/net/rtl819x` for the
   RTL8197F CPU-DMA descriptor format and P0 switch-core setup.
3. `rtl83xx_v1dot4/dal/rtl8367d` for RTL8367D register definitions, CPU-tag,
   VLAN, L2, statistics and EXT1 behavior.
4. `package/uboot/realtek/generic/boot` for reset ordering, pin muxing and the
   SHEIPA SPI command convention.
5. `backports/src/drivers/net/wireless/realtek/rtl8192fe` only for WLAN
   inventory and chip identification.  This source is a vendor `rtl8192cd`
   FullMAC stack, not a Linux 6.6 mac80211 driver.

## RTL8197F platform operation

### CPU, memory and interrupt topology

The SoC exposes a little-endian MIPS 24Kc core with 128 MiB RAM on the RD05.
The native target provides the RTL8197F machine description, interrupt
controller, timer/clock setup and the standard 8250 UART at the documented
MMIO locations.  Device Tree, rather than board-file registration, now owns
resources and interrupt routing.

The Ethernet CPU-DMA interrupt remains available on the RTL interrupt
controller.  The driver also retains a bounded RX polling fallback because
hardware tests have shown that frames can be present in the switch path while
the CPU interrupt count remains unchanged.  Polling is a recovery mechanism,
not a replacement for DSA or phylink.

### GPIO, reset and pin muxing

The native GPIO driver exposes the RTL8197F GPIO banks through gpiolib.  RD05
board setup keeps the SDK wiring:

- GPIO H2 / GPIO58 deasserts the RTL8367D reset line.
- GPIO H0 and G7 form the low-level SMI clock/data pair.
- pin-mux state is established before the DSA switch probes.
- reboot uses the RTL8197F watchdog/software-reset sequence.

The old bootloader and BSP code performed these writes globally.  The port
confines them to a board-specific pre-initialization helper and Device Tree.

### SPI NOR and persistent overlay

The RD05 has a 16 MiB serial NOR layout:

```text
0x000000-0x020000  boot
0x020000-0x030000  nvram / CRC32 U-Boot environment
0x030000-0x040000  bdata
0x040000-0x050000  factory
0x050000-0x060000  crash
0x060000-0x350000  kernel
0x350000-0xe60000  rootfs
0xe60000-0x1000000 rootfs_data / OEM overlay
```

The controller's command FIFO accepts different access widths depending on the
BSP generation.  The native MTD driver therefore performs a non-destructive
WRDI/RDSR/WREN/RDSR calibration and accepts only a reproducible clear/set
transition of the write-enable status bit.  Page program and 64 KiB erase are
hard-limited to `0x00e60000..0x00ffffff`; all boot, NVRAM, factory, kernel and
SquashFS regions remain read-only.

The supplied v34 hardware log confirms that the calibrated command mode is
`dr8`, the write-enable transition is detected, and `/dev/mtdblock8` mounts as
a writable JFFS2 `/overlay`.

The U-Boot environment is parsed through the kernel NVMEM U-Boot-environment
provider.  It contains the board identity and permanent addresses:

```text
model=RD05
ethaddr=a4:ba:70:1b:d4:10
ethaddr_wan=a4:ba:70:29:36:a8
wl0_macaddr=a4:ba:70:1b:d4:12
wl1_macaddr=a4:ba:70:1b:d4:11
```

The Ethernet master consumes `ethaddr` through an NVMEM cell instead of using
a random address.  The WLAN cells are declared for a future native radio
implementation.

### Native PCIe host

The SDK board files describe a one-slot PCIe root complex.  The native driver
at `drivers/pci/controller/pcie-rtl8197f.c` replaces the MIPS-specific
`pci_controller` code with a generic `pci_host_bridge`.

Implemented hardware behavior includes:

- root-complex configuration window at SoC offset `0x00b00000`;
- directly connected endpoint window at `0x00b10000`;
- 2 MiB I/O aperture at physical `0x18c00000`;
- 16 MiB memory aperture at physical `0x19000000`;
- clock gates 12, 13 and 18, followed by active-controller gate 14;
- 40 MHz PHY MDIO values `0x0f=0x12f6`, `0x00=0x0071`,
  `0x06=0x1ac1`;
- RTL8197FH/VG MDIO register 8 value `0x3101`;
- PHY reset sequence `0x01` then `0x81`;
- endpoint PERST# asserted for 300 ms;
- link-state acceptance only at value `0x11`;
- root-complex and endpoint command value `0x00100007`;
- single-slot configuration access through IPCFG;
- the BSP's CPU IRQ 5 mapping.

The OEM log identifies the endpoint as PCI `10ec:f812` and the vendor driver
prints `found 8812F` and `Hardware type = RTL8812FE`.  The host bridge is
therefore enabled in the RD05 DTS with the 40 MHz and VG-specific properties.

## RTL8197F wired data path

### Descriptor model

The active `rtl8197f_rtknet` driver uses the SDK's new descriptor mode:

- 64 RX and 64 TX descriptors by default;
- eight 32-bit words, or 32 bytes, per descriptor;
- one RX packet-header ring selected by `CPURPDCR0`;
- one TX ring selected by `CPUTPDCR0`;
- 2,048-byte receive buffers;
- KSEG1/uncached descriptor visibility as required by the SoC DMA.

The CPU-DMA enable sequence is ordered after descriptor-format selection and
ring programming.  After the hardware start bit is asserted, the driver
re-applies `CPUICR1` and the ring pointers because the RTL8197F can otherwise
retain the old six-word format or clear the base register.  The v34 hardware
log confirms `CPUICR1=0x002081d3` and non-zero RX/TX ring bases.

### Authoritative receive pointer

A material behavior from `rtl819x_swNic.c` was missing from earlier ports.  The
SDK does not rely solely on a cached descriptor OWN bit.  `CPURPDCR0` is the
hardware's current receive descriptor pointer.  If the software consumer index
differs from the hardware index, a descriptor has been advanced by DMA and
must be inspected even when the uncached/cached OWN observation is stale.

The v35 driver implements this rule and exposes the following ethtool counters:

- `rx_cdp_reads`;
- `rx_cdp_advanced`;
- `rx_cdp_owned_advanced`;
- `rx_cdp_invalid`;
- `rx_cdp_last`;
- `rx_cdp_last_idx`.

This converts the SDK's current-descriptor-pointer semantics into the normal
Linux receive loop without importing the old private networking stack.

### P0/RGMII setup

The SDK `init_8197f_p0()` establishes the SoC side of the CPU link.  The native
implementation reproduces the hardware-relevant parts:

- physical RTL8197F port 0;
- forced 1 Gbit/s, full-duplex link;
- RGMII mode and external PHY identifier 5;
- P0 router mode;
- CPU-tag recognition on RX;
- CPU-tag capability on TX;
- hardware tag removal disabled so the Linux DSA tagger receives all eight
  bytes;
- TX delay 0;
- RTL8197FH/VG SoC RX delay 6;
- fixed IPG behavior and P0 pad-drive setup.

TX descriptors select direct destination-port bit 0.  The switch's port 7 is
not a SoC DMA destination bit; it is the remote RTL8367D CPU port reached over
P0/RGMII.

## RTL8367D operation and Linux DSA conversion

### Identification and transport

The native Realtek SMI driver uses the existing Linux GPIO-SMI/regmap transport.
The RTL8367D variant is selected by the SDK's probe identity:

- magic register `0x13c2` reports `0x0249`;
- chip register `0x1300` reports `0x6642`;
- the tested hardware revision is `0x0030`.

No second private SMI stack is registered.  The old DAL read/write helpers are
represented by regmap operations under the DSA switch driver.

### Port topology

The RD05 switch topology is fixed in Device Tree:

```text
RTL8367D port 0 -> lan1
RTL8367D port 1 -> lan2
RTL8367D port 2 -> wan
RTL8367D port 3 -> lan3
RTL8367D port 4 -> lan4
RTL8367D port 7 / EXT1 -> CPU port -> RTL8197F P0/RGMII
```

Port 5 and EXT0/port 6 are not exposed as user ports.

### CPU-tag contract

The SDK CPU DAL supports CPU-port selection, tag position and four/eight-byte
formats.  The RD05 native contract is:

- CPU port mask `0x80`, selecting port 7;
- tag after the source MAC;
- eight-byte Realtek tag;
- Linux DSA protocol `rtl8_4`;
- no hardware TX tag insertion on the RTL8197F master;
- no hardware RX tag removal on the RTL8197F master;
- DSA inserts/removes the tag exactly once.

The packet path is therefore:

```text
TX: DSA slave skb
    -> rtl8_4 tag inserted by Linux
    -> rtl8197f_rtknet TX descriptor, DP=0x01
    -> RTL8197F P0/RGMII
    -> RTL8367D EXT1/CPU7
    -> selected user port

RX: RTL8367D user port
    -> switch forwarding to CPU7
    -> 8-byte rtl8_4 tag added by RTL8367D
    -> RTL8197F P0/RGMII and CPU-DMA RX ring
    -> rtl8_4 tag decoded by Linux DSA
    -> correct lan/wan slave netdev
```

### Switch configuration represented by DSA

The native driver maps the hardware-relevant DAL operations to standard Linux
interfaces:

- phylink for CPU and user-port link state;
- DSA bridge join/leave and STP state;
- VLAN filtering, PVID and membership;
- FDB add/delete/dump and learning control;
- unknown unicast, multicast and broadcast flooding;
- per-port MIB statistics through ethtool;
- port enable/disable and PHY power state;
- MTU accounting for the 8-byte DSA tag.

The initial RD05 bootstrap permits user ports plus CPU7 in the expected flood
and isolation masks.  Normal bridge, VLAN and FDB ownership then belongs to
Linux DSA rather than the old DAL user ABI.

### RGMII timing

The SDK distinguishes the two directions:

- RTL8197F P0 receive sampling is a SoC-side delay;
- RTL8367D EXT1 receive sampling is a switch-side delay.

The production defaults in this tree use the SDK-derived SoC P0 delay and a
switch EXT1 baseline of TX0/RX5.  The earlier v34 hardware log used switch RX6
as an experiment and still reported CPU7 FCS errors.  v35 returns the native
production default to RX5 and adds direction-specific diagnostics instead of
silently treating an experimental tap as proven.

A new on-device traffic run is required to determine whether the remaining
FCS count is a stable timing issue, a transient caused by interface restart, or
an artifact of the old diagnostic sequence.

## Device Tree and OpenWrt integration

The RD05 DTS now describes:

- RTL8197F platform resources;
- bounded writable SPI window and fixed partitions;
- U-Boot environment NVMEM cells;
- native PCIe host with 40 MHz/VG PHY tuning;
- native RTL8197F Ethernet master;
- RTL8367D DSA switch with five user ports and CPU port 7;
- fixed 1 Gbit/s RGMII CPU link;
- GPIO/pinmux state and switch reset ordering;
- EEE disabled on the unstable external user PHYs during bring-up.

The `rtl8367d-compat` package is no longer a substitute switch stack.  It only
installs bounded diagnostics and compatibility helpers.  Release 7 contains
`rd05-v35-check` and the v35 network diagnostic script.

## WLAN status

The RD05 has two vendor WLAN interfaces:

- integrated RTL8197F 2.4 GHz MAC (`wl1` in the OEM runtime);
- external PCIe RTL8812FE (`wl0`), PCI `10ec:f812`.

The archive supplies a large `rtl8192cd` FullMAC tree with HALMAC, PHYDM,
private netlink, vendor cfg80211 wrappers and a backported 5.2 kernel API.  It
is not a native Linux 6.6/mac80211 driver.  Copying it into the target would
reintroduce a second wireless framework and would not satisfy the requested
native port.

Consequently, v35 ports the PCIe host and declares the NVMEM calibration/MAC
inputs, but does not mislabel the vendor FullMAC code as completed native WLAN
support.  Exact findings and the remaining rewrite boundary are documented in
`RD05_WLAN_NATIVE_STATUS_V35.md`.

## Removed legacy code

The following material is intentionally absent from the active target:

- the embedded copy of the complete legacy switch SDK;
- the copied Linux-4.4 `rtknet-vendor-4.4` tree;
- old RTL8325D/RTL8367R parallel switch stacks;
- stale vendor target dumps;
- inactive reference-only kernel patches;
- proprietary FastPath/NAT/netfilter hooks;
- private DAL ioctl/proc control planes.

Every source needed for audit remains identified by path and hash in the
source map, but the build tree contains only the native implementations.

## Validation performed

The final validation workflow checks:

1. every active Realtek kernel patch with both Git and GNU patch parsers;
2. the expected Linux 6.6 overlay sources, bindings and Device Tree files;
3. all required target Kconfig symbols;
4. DSA CPU-tag, descriptor, SPI protection and PCIe source markers;
5. shell syntax for all RD05 helper scripts;
6. absence of conflict markers, empty C sources and broken symlinks;
7. native PCIe-host compilation with warnings promoted to errors;
8. complete ZIP extraction and file/symlink comparison;
9. SHA-256 checksums for the delivered artifacts.

The source-level and archive checks cannot replace a real RD05 boot test.

## Required hardware validation

After building and booting v35, run:

```sh
rd05-v35-check
rd05-netdiag full
```

During `rd05-netdiag full`, a Linux host connected to LAN2 must run
`tools/rd05-pc-netdiag.py` so that external ARP, ICMP, UDP and raw Ethernet frames actually
enter the switch.  The decisive checks are:

- PCI endpoint `0000:00:00.0` enumerates as `10ec:f812`;
- `CPUICR1` and ring bases retain their post-start values;
- `rx_cdp_advanced` increases when the hardware pointer moves;
- `eth0` RX and `rd05_rx_external` increase;
- the frame appears on the correct DSA slave;
- CPU7 FCS/drop deltas remain zero under sustained bidirectional traffic;
- bridge/VLAN/FDB behavior works across all five user ports;
- `/overlay` remains writable after repeated reboot and erase cycles.

Until those tests are completed, the native code is source-audited and
compile/patch validated, but the remaining electrical timing and complete
switch-to-SoC receive path are not claimed as hardware-proven.
