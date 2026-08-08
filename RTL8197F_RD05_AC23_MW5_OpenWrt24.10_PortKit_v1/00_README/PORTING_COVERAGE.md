# Portierungsabdeckung

## Enthaltene technische Bereiche

### RTL8197F/FH/FS

- RLX/MIPS-Plattformcode, Cache/DMA-Referenzen, IRQ, Timer, Clock, Reset, GPIO, Pinmux, SPI, UART und PCIe.
- RTL865x/RTL819x-NIC mit Descriptoren, RX/TX-Ringen, NAPI-/Interruptreferenzen, VLAN-/Portmaskenlogik, HW-NAT/Fastpath-Schnittstellen und Registerheadern.
- historische OpenWrt-Targets und Board-/Bootloader-Defconfigs.

### RTL8367D / RTL8367RB / RTL8367R/C

- Chip-ID-Erkennung, SMI, GPIO-MDC/MDIO, Registerzugriff, DAL/Mapper und API-Schichten.
- CPU-Port, CPU-Tag, RGMII-Force-Link und Delay-Konfiguration.
- VLAN/PVID, FDB/L2, STP, Port-Isolation, PHY, MIB, EEE, QoS, IGMP, ACL, Mirror, Storm Control und Interrupts.
- Linux-DSA-Referenzen aus `rtl8365mb` sowie aktueller OpenWrt-24.10-Arbeitsstand.

### WLAN / iNIC

- rtl8192fe-Backportszweig mit RTL8197F, RTL8812F/FE und RTL8814B.
- HALMAC, PHYDM, HALRF, RF-/AGC-/PHY-/Tx-Power-Tabellen und Firmwareblobs.
- älterer rtl8192cd-Zweig für RTL8197F/RTL8812-/RTL8814-Referenzen.

### Boot/Image/Toolchain

- Realtek-Bootcode für RTL8197F und RTL8367R/RB-nahe Boards.
- Realtek-Imageheader `cs6c`, `cr6c`, `root`, `cvimg`, `mgbin`, LZMA-Loader.
- OpenWrt-Toolchainrezepte und Quellpatches, jedoch keine vollständige vorkompilierte Toolchain.

## Bekannte Lücken

1. Für RTL8363NB existiert kein vollständig separater benannter Quellbaum; MW5 benötigt zusätzliche Register- und Laufzeitverifikation.
2. Die RTL8367D-Kaltstart-/Jam-Table ist nicht als vollständige eindeutige Sequenz vorhanden. In einem DAL-Zweig liefert `dal_rtl8367d_switch_init()` nur Erfolg zurück.
3. Das Xiaomi-`bootmiwifi`-Imageformat ist nicht vollständig durch die Realtek-Standardtools abgedeckt.
4. Die Vendor-WLAN-Treiber sind nicht direkt Linux-6.6-/mac80211-kompatibel.
5. Hardware-NAT/Fastpath ist nicht Bestandteil einer sauberen ersten DSA-Portierung.

## Empfohlene Verwendung

Vendorcode als Register- und Verhaltensreferenz behandeln. Für OpenWrt 24.10 neue, klar lizenzierte Linux-Treiber schreiben beziehungsweise vorhandene OpenWrt-/DSA-Treiber erweitern. Variantenspezifische Initialisierung nie ungeprüft zwischen RTL8367D, RB, R und C übernehmen.
