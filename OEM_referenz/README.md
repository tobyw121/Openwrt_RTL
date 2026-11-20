# OEM_referenz — ausgewählte Originaldateien aus beiden GPLs

Die Unterordner enthalten unveränderte Textdateien mit ursprünglichen Pfaden.
`manifest.json` nennt Herkunft, Archiv-SHA256, Dateigröße und Datei-SHA256.
Herstellerhinweise und Lizenzheader bleiben erhalten; OpenWrt-Lizenztexte sind
unter google_ax1500/openwrt/21.02/LICENSES abgelegt. Es gilt die jeweilige
Dateilizenz, nicht pauschal eine neue Lizenz für die gesamte Sammlung.

| Ordner | Inhalt / Verwendung |
|---|---|
| google_ax1500 | RTL8367D-DAL samt Registerdefinitionen, VLAN/Port/SMI-Quellen; BSP-CPU/IRQ/Takt/PCI/Timer/Reset; Kernelkonfigurationen für8832BR/8367R; WLAN-Skript und rtlwifi-Patches |
| tplink_ax1500v3 | G6-PCI/IRQ/DMA-Anbindung, Linux-Adapter-Lebenszyklus, RF-Dateilader, NVM-Schnittstelle, Plattformkonfiguration und Build-Makefiles |

Die Google-GPL enthält keinen G6-WLAN-Treiber. Die TP-Link-Sammlung stammt
hier aus dem G6-Unterbaum; eine zusätzliche BSP-/RTL8367D-Dateisuche fand in
den verwendeten Archivpfaden keine Dateien. Nicht vorhandene Teile werden
nicht aus der jeweils anderen GPL unter falscher Herkunft ergänzt.
Keine Firmwarebinärdateien, Toolchains oder ausgeführten OEM-Buildwerkzeuge.
Die Referenzdateien sind keine Build-Eingaben unserer Firmware; insbesondere
keine OEM-Konfigurationen unbesehen in OpenWrt24.10 übernehmen. Proprietäre
NAT-/Bridge-Abkürzungen bleiben deaktiviert; Linux DSA/nftables sind maßgeblich.

## Bereits portiert / neue Ergänzung

- Switch-CVLAN-Kommandos und Rückleseprüfung: GPL-DAL bestätigt Encoding,
  VID/PVID und Busy-/Readback-Register (Commit4467b72cdb).
- WLAN-RF-Wurzel `/etc/conf/`: aus G6-HAL-Dateipfadlogik abgeleitet
  (Commit09621e038e); Test des echten vorverarbeiteten Plattformheaders.
- Neu: Watchdog-only-Fallback im SoC-Neustarthandler, falls der vorherige
  Software-Reset zurückkehrt. Entspricht Google-BSP `plat_machine_restart`:
  WDTCNR=0. Keine schlafenden Netzwerkoperationen im atomaren Resetpfad.
  Diese Ergänzung gilt für die registrierte RTL8197F-SoC-Schnittstelle,
  nicht ungeprüft für jede Realtek-Chipfamilie. Hardwaretest noch erforderlich.

Offen bleiben G6-IRQ-/Timer-Fehlerbereinigung, eFuse/MAC, tatsächliches Laden
der RF-Dateien und Hardwarevalidierung der WAN/LAN-Trennung. Referenzmaterial
allein ist kein Nachweis funktionierender APs oder Ethernet-Datenübertragung.

## Reproduzieren und prüfen

```sh
python3 OEM_referenz/collect.py --google /pfad/GPL_for_AX1500_rtl8197 --tplink /pfad/GPL_ax1500v3
PYTHONPATH=scripts/tests python3 -m unittest test_oem_referenz test_rtl8197f_generic_restart
```
Die Auswahlskript-Ausgabe nennt Umfang und Bytes. Es werden nur vorhandene
reguläre Textdateien kopiert; die Originale bleiben unverändert.

## PC: WR1500 bauen

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make -j"$(nproc)" V=s
```

Nach dem Einspielen am Router über UART38400 Befehle einzeln:

```sh
dmesg | grep rtl8197f-restart
sync
reboot
```

Erwartet sind Resetmeldung und Bootloader-Ausgabe. Falls der Software-Reset
zurückkehrt, erscheint die neue Watchdog-Fallback-Meldung. Wiederholte
Kalt-/Warmstarts und Ethernet/WLAN danach prüfen. Der tmpfs-Overlay-Status
wird nicht verändert. Kein vollständiger Kernel-/Imagebau in dieser Umgebung.
