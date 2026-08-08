# RTL8197F RD05 / AC23 / MW5 OpenWrt 24.10 PortKit v1

Dieses Paket ist eine kuratierte Arbeitsgrundlage für die Portierung folgender Geräte auf OpenWrt 24.10 / Linux 6.6:

| Gerät | SoC | externer Switch | 5-GHz-WLAN | Flash |
|---|---|---|---|---|
| Xiaomi Router 4 / RD05 | RTL8197FH-VG | RTL8367D | RTL8812FE | 16 MiB GD25Q128 |
| Tenda AC23 / Lynx 8000 | RTL8197FH | RTL8367RB | RTL8814BR | 8 MiB BH25Q64 |
| Tenda Nova MW5 | RTL8197FS | RTL8363NB | RTL8812BRH | 8 MiB W25Q64JV |

## Wichtigste Verzeichnisse

- `01_OpenWrt_24.10/`: aktueller RD05-DSA-Stand v38.3 sowie RTL8197F/RTL8367-Familienstand v35.
- `02_RTL8197F_SoC_NIC/`: RTL8197F/FH/FS, RLX/MIPS-Plattform, RTL865x/RTL819x-NIC, DMA, Register und Header.
- `03_Switches/`: RTL8367D/RB/R/C, SMI, CPU-Port/CPU-Tag, DAL/Mapper, VLAN, FDB, STP, PHY, MIB und Linux-DSA-Referenzen.
- `04_WLAN_INIC/`: vollständige relevante rtl8192fe-/rtl8192cd-Zweige, RTL8197F, RTL8812F/FE, RTL8814A/B, HALMAC, PHYDM, RF/AGC/Tx-Power und Firmwareblobs.
- `05_Boot_Board_ImageTools/`: RTL8197F-Bootcode, Boardprofile sowie `cvimg`, `mgbin`, LZMA-Loader und Realtek-Imagewerkzeuge.
- `06_Toolchain_Changes_Only/`: nur OpenWrt-Toolchain-Rezepte, Konfigurationen und Patches; keine vorkompilierte vollständige Cross-Toolchain.
- `07_Device_Evidence_Sanitized/`: Hardware- und Flashübersichten sowie bereinigte `/proc`-/Bootlog-Auszüge.
- `08_Indexes/`: Dateimanifest, Quellherkunft, Prüfsummen und Statistiken.
- `09_Scripts/`: reproduzierbare Analyse- und Manifestskripte.

## Nicht enthalten

Die vollständigen SPI-Dumps, Factory-/NVRAM-Partitionen, WLAN-Passwörter, individuelle MAC-Adressen, Seriennummern und RF-Kalibrierungsdaten wurden absichtlich nicht dupliziert. Die Originaldateien müssen lokal als private Sicherung aufbewahrt werden.

## Portierungspriorität

1. RTL8197F Clock/Reset/IRQ/UART/GPIO/SPI/PCIe und Device Tree stabilisieren.
2. RTL8197F-NIC auf ein einzelnes Linux-Master-Netdevice reduzieren.
3. RTL8367D/RB über SMI anbinden und CPU-Port/RGMII initialisieren.
4. Realtek-8-Byte-CPU-Tag verifizieren und DSA-Portdemultiplexing aktivieren.
5. VLAN/FDB/STP/MIB/Bridge-Offload ergänzen.
6. WLAN getrennt portieren; Vendorcode ist eine Referenz, kein direkt moderner mac80211-Treiber.

## Lizenzhinweis

Viele Vendorquellen tragen proprietäre oder uneinheitliche Lizenztexte. Registerwissen und Hardwareverhalten können zur Neuimplementierung verwendet werden; Code darf nicht automatisch unverändert in einen GPL-Upstream-Treiber übernommen werden. Vor Veröffentlichung jede Datei einzeln lizenzrechtlich prüfen.
