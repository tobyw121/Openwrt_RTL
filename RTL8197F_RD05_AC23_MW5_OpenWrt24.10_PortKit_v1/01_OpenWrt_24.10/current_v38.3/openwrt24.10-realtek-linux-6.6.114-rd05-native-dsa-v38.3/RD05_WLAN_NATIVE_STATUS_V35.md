# RD05 WLAN native-port status (v35)

## Proven hardware identity

The OEM RD05 boot record and the supplied Realtek source establish two radios:

1. the integrated RTL8197F wireless MAC, exposed by the OEM runtime as `wl1`;
2. a PCIe endpoint with vendor/device ID `10ec:f812`, for which the OEM driver
   prints `found 8812F` and `Hardware type = RTL8812FE`, exposed as `wl0`.

The OEM kernel calls the module `RTK-8192FE-wlan-driver` and the source root is
named `rtl8192fe`, but the detected external device is RTL8812FE.  The module
name is therefore a family/legacy name, not a reliable chip identifier.

## Source architecture in 8197_all_SDKs

The relevant directory is:

```text
backports/src/drivers/net/wireless/realtek/rtl8192fe
```

It contains approximately 2,300 files and combines:

- `rtl8192cd` FullMAC/core code;
- RTL8197F integrated-radio support;
- RTL8812FE PCIe HAL generation code;
- HALMAC 8812F PCIe transactions;
- PHYDM 8812F calibration and RF code;
- private generic-netlink interfaces;
- Realtek-specific VAP, repeater, thermal and MiWiFi hooks;
- a backported cfg80211 compatibility layer based on Linux 5.2-era APIs.

Kconfig explicitly provides `WLAN_HAL_8197F`, `USE_PCIE_SLOT_0` and
`SLOT_0_8812FE`.  This confirms that the archive contains the correct vendor
hardware support, but not that it is a native Linux 6.6 driver.

## Why it is not enabled in this native tree

A direct build of this source would add a parallel wireless stack with old
kernel APIs, vendor-owned station/VAP management, private netlink commands and
non-standard lifetime rules.  It would not be a normal mac80211/modern
cfg80211 driver and would undermine the native-port requirement.

The v35 tree therefore does not copy or enable the vendor WLAN driver.  It does
port the prerequisites that can be represented natively:

- RTL8197F PCIe root complex;
- one-slot config-space access and IRQ routing;
- RD05 40 MHz/VG PCIe PHY sequence;
- endpoint enumeration expected as `10ec:f812`;
- U-Boot-environment NVMEM cells for `wl0_macaddr` and `wl1_macaddr`;
- the disabled integrated-WMAC Device Tree resource for future work.

## Work required for a genuinely native WLAN driver

A complete native implementation needs two separate kernel drivers or one
shared modern framework:

### External RTL8812FE

- PCI probe and power sequencing;
- firmware and efuse/NVMEM loading;
- DMA rings and interrupt handling;
- HALMAC register programming;
- PHYDM/RF calibration translated into a maintained PHY layer;
- mac80211 TX/RX status, rate control, AMPDU and beacon handling;
- cfg80211 regulatory, scan, station, AP and monitor operations;
- runtime PM, suspend/resume and recovery.

### Integrated RTL8197F radio

- native platform-bus probe;
- SoC radio clocks/reset and IRQ;
- internal DMA and firmware interface;
- RF/BB calibration and board RFE description;
- the same mac80211/cfg80211 contract as above.

The SDK source map records the exact RTL8812FE HALMAC, PHYDM and PCI glue files
that must be used as behavioral references.  No claim is made that this
multi-layer rewrite has been completed in v35.

## Safe test boundary

With v35, `lspci -nn` or `/sys/bus/pci/devices/0000:00:00.0` should prove the
native host and endpoint path.  Absence of a `wlan` interface is expected until
a genuine Linux 6.6 wireless driver is added.  Loading the old vendor module
against this tree is outside the validated configuration.
