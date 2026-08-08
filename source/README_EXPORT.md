# Realtek-Treiberexport aus `sdk_GPL_MR62X.tar(3).zst`

Dieser Export enthält die aus dem SDK extrahierten Realtek-Treiber, Board-/Kernel-/Userland-Configs und Bootcode-Bestandteile für die angefragten Plattformen und Komponenten.

## Umfang des Exports

| Bereich | Exportierter Pfad | Zweck |
|---|---|---|
| RTL8197F / RTL8197FH / RTL8197F_VG Plattform | `sdk/openwrt-21.02/target/linux/target/*RTL8197F*`, `sdk/openwrt-21.02/target/linux/realtek/`, `sdk/openwrt-21.02/package/uboot/realtek/generic/def-rtl8197*` | OpenWrt Target-Konfigurationen, Board-/RAMFS-Templates, U-Boot-Defconfigs für F/FH/FS/VG-Varianten. |
| Bootcode | `sdk/openwrt-21.02/package/uboot/realtek/generic/boot/`, `btcode/`, `btcode_vg/`, `bsp/`, `config/`, `romboot_support/` | Startcode, DDR-/eFuse-Initialisierung, Flash/NAND/SD-Bootpfade, gzip/LZMA-Image-Loader, U-Boot-Board-/Switch-Unterstützung. |
| Ethernet MAC / rtl865x | `sdk/openwrt-21.02/target/linux/rtknet/` | Realtek-819x/865x Ethernet-, Switch-, L2/L3/L4-, NAT-, VLAN-, FDB-, IGMP-/MLD- und Fastpath-Code. |
| RTL8367R/RB/RB-VC-Familie | `sdk/openwrt-21.02/target/linux/rtknet/drivers/net/rtl819x/rtl8367r/`, `rtl83xx/`, `rtl83xx_v1dot4/`, sowie U-Boot `boot/rtl8367r`, `boot/rtl83xx*` | Switch-API/ASIC-Treiber für Ports, VLAN, LUT/FDB, QoS, ACL, MIB, IGMP, EEE, PHY, CPU-Tagging und SMI/MDC/MDIO-Anbindung. |
| RTL8197F/G WLAN | `.../rtl8192fe/WlanHAL/RTL88XX/RTL8197F`, `.../RTL8197G`, `.../Data/8197F`, `.../Data/8197G`, `.../phydm/rtl8197f`, `.../phydm/rtl8197g` und identische Kernel-Tree-Kopie unter `target/linux/linux-4.4/.../rtl8192cd/` | WLAN-HAL, Firmware-/HW-Image-Tabellen, PHYDM/RF-Kalibrierung, Power-Sequencing, Rx/Tx-Descriptoren. |
| RTL8812F / RTL8812FE / RTL8812FS | `.../WlanHAL/RTL88XX/RTL8812F`, `.../Data/8812F`, `.../HalMac88XX/halmac_88xx/halmac_8812f`, `.../phydm/rtl8812f`, `.../phydm/halrf/rtl8812f` | 8812F WLAN-HAL, HALMAC, PHY/RF/DPK-Kalibrierung, MAC/BB/RF-Tabellen, PCIe/USB/SDIO-HCI-Unterstützung. |
| PCIe für RTL8812F | `.../HalMac88XX/halmac_88xx/halmac_8812f/halmac_pcie_8812f.[ch]`, `.../Kconfig`, `.../Makefile` | PCIe-HCI-Auswahl über `CONFIG_PCI_HCI` und Slot-Optionen `SLOT_0_8812FE` / `SLOT_1_8812FE`; bindet den 8812F-HALMAC-PCIe-Code in `rtl8192cd-objs` ein. |
| rtl8192fe / RTK-8192FE-wlan-driver | `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/realtek/rtl8192fe/` plus Kernel-Kopie `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/rtl8192cd/` | OpenWrt/backports-Treiberpaket und Kernel-Tree-Kopie des Realtek-AP-WLAN-Treibers. |

## Funktionsweise, kurz zusammengefasst

### Boot-/Plattformpfad RTL8197F/FH/VG

Die U-Boot-Realtek-Quelle trennt normalen Bootcode und VG-Bootcode über `CONFIG_RTL_8197F_VG`: im Makefile wird dann `BTCODE_DIR=btcode_vg`, sonst `BTCODE_DIR=btcode`. Die Defconfigs aktivieren die RTL8197F/VG-SoC-Auswahl, Flash-Typ, Image-Offets, Kompression, Debug und je nach Board den externen RTL8367/83xx-Switch. Der Bootcode kompiliert `start.S`, `start_c.c`, Cache-Code, DDR-/eFuse-Code und je nach Konfiguration NAND-/SPI-NAND-/SD-Hilfen. GZIP- oder LZMA-komprimierte Images werden als `.initrd` in den Startcode eingebettet, im RAM dekomprimiert und anschließend an die Zieladresse gesprungen.

### Ethernet MAC / rtl865x / rtknet

`rtknet/drivers/net/rtl819x` enthält den Realtek-SoC-Netzwerkpfad. Der Kern bindet u. a. `rtl865xc_swNic.o`, `rtl_nic.o` und bei `CONFIG_RTL_8197F` auch `rtl819x_swNic.o`. Darunter liegen Layered-ASIC-Treiber (`AsicDriver`), L2-Treiber (`fdb`, `vlan`, `stp`, QoS), L3/L4-Treiber (`arp`, `route`, `nat`, `ppp`, multicast), IGMP-Snooping sowie Fastpath-/Feature-Hooks. Die Kconfig schaltet Hardware-NAPT, Hardware-Multicast, VLAN/Netif-Mapping, Multiple-WAN und MIPS16-Optionen über Realtek-SoC-Symbole.

### RTL8367R/RB/RB-VC Switch

Die RTL8367-Familie wird im Linux-Netzwerkbaum und im U-Boot-Bootbaum exportiert. Die Linux-Seite `rtl8367r/Makefile` baut `rtk_api.o` und viele `rtl8367b_asicdrv_*` Module für ACL, CPU-Tag, Dot1x, EEE, Flow-Control, Green Ethernet, IGMP, Bandwidth-Control, Interrupt, LED, LUT/FDB, MIB, Mirror, PHY, Port, QoS, RMA, Scheduling, Storm-Control, SVLAN, Trunking und VLAN. VC/RB-Varianten erscheinen in U-Boot-Defconfigs als `CONFIG_SW_8367RB_VC=y` zusammen mit `CONFIG_SW_83XX=y` und `CONFIG_RTL_83XX_API_V1_4=y`.

### WLAN-HAL: RTL8197F/G, RTL8192FE, RTL8812F

Der Realtek-WLAN-Treiber wird als `RTL8192CD`-Treiber konfiguriert. `Kconfig` aktiviert SoC-WLAN über `WLAN_HAL_8197F`/`WLAN_HAL_8197G`, PCIe-WLAN über `USE_PCIE_SLOT_0/1` und Slot-Auswahl wie `SLOT_0_8192FE`, `SLOT_0_8812FE`, `SLOT_1_8192FE`, `SLOT_1_8812FE`. Die HAL bindet chipabhängig Firmware, Power-Sequencing, PHY-Konfiguration, Tx/Rx-Descriptoren, HALMAC, PHYDM und HALRF ein. Die `WlanHAL/Data/*`-Ordner enthalten MAC/BB/RF-/Power-Limit-Tabellen, die beim Build in C-Quellen umgewandelt werden.

### PCIe für RTL8812F

Für RTL8812F/FE schaltet `Kconfig` `SLOT_0_8812FE` bzw. `SLOT_1_8812FE` und damit `WLAN_HAL_8812FE`. Im Makefile werden dann `WlanHAL/RTL88XX/RTL8812F/*`, `RTL8812FE/Hal8812FEGen.o` und HALMAC-Module einschließlich `halmac_pcie_8812f.o` eingebunden. Die RFE-Typen im Kconfig steuern interne/externe PA/LNA-Varianten, eFuse-Optionen und Beamforming-Pfade.

## Hinweise

- Der exakte String `RTL8812FR` wurde im Archiv nicht gefunden. Exportiert sind die vorhandenen RTL8812F-Varianten `RTL8812FE` und `RTL8812FS` sowie die gemeinsamen RTL8812F/HALMAC/PHYDM/HALRF-Dateien.
- `RTL8197FH` erscheint in diesem SDK hauptsächlich als U-Boot-/Board-Defconfig-Variante (`def-rtl8197fh*`) und nutzt die gemeinsamen RTL8197F-Treiber/HAL-Dateien.
- Es wurden Quellen, Konfigurationsdateien und Bootcode exportiert; keine neuen Kernelmodule oder Images wurden kompiliert.

## Dateien in diesem Paket

- `source/` – extrahierte Originalpfade aus dem SDK.
- `MANIFEST.txt` – vollständige Dateiliste des Exports.
- `EXPORT_PATHS_FROM_ORIGINAL_ARCHIVE.txt` – Pfade, die aus dem Originalarchiv selektiert wurden.
- `README_EXPORT.md` – diese Übersicht.
