# Cudy WR1500 / R76: OEM-Imageanalyse

Analysiert wurde `fullflash-templates/WR1500-R76-oem_Firmware_flash.bin`
aus Commit `5a0a0ff46fae351efdcdb26dc2a51bd8a5b25b22`.

- Dateigröße: **15.180.020 Bytes**, nicht 16 MiB.
- SHA256: `89f0a7a5a9b2bcb14ff98ac26f6989c3b48d83269b4fcf07ff26cd3248d5574a`.
- Ergebnis: **kombiniertes OEM-Upgrade-Image**, kein vollständiger SPI-Abzug.
- OEM-Version: **2.3.10**, Buildkennung `2.3.10-20251215-170019`.
- Betriebssystem: OpenWrt **21.02-SNAPSHOT**, Linux **4.4.176**.

Maschinenlesbare Befunde und Hashes: [evidence.json](evidence.json).
Die OEM-Programme wurden nicht ausgeführt. Alle regulären Rootfs-Dateien
wurden gelesen und die Module als ELF-Dateien untersucht. Dies ist eine
Container-, Daten- und ausgewählte Codeanalyse, keine vollständige semantische
Rekonstruktion aller Programme oder des ursprünglichen C-Quelltextes.

## Dateiaufbau und tatsächliche Flash-Adressen

| Dateioffset | Inhalt |
| --- | --- |
| `0x000000–0x020000` | 128-KiB-Bootloader-Anteil |
| `0x020000–0x030000` | OEM-Signaturblock mit Padding; keine cfg-Kopie |
| `0x030000` | uImage-Header, Name `R76` |
| `0x030040` | Realtek-Header `cr6c`, Burn-Adresse `0x40000`, Loaderadresse `0x80a00000` |
| `0x032858` | LZMA-Alone-Stream des Kernels |
| `0x2a0000` | SquashFS 4 mit XZ-Kompression, 256-KiB-Blöcken |
| Dateiende | OpenWrt-fwtool-Metadaten mit `supported_devices: ["R76"]` |

Header- und Daten-CRC32 des uImage stimmen. Der Kernel ließ sich vollständig
auf **6.718.440 Bytes** entpacken. Auch der Gzip-Anteil des Bootloaders wurde
vollständig dekodiert. Diese Prüfungen belegen Datenintegrität, aber keine
überprüfte OEM-Signatur oder Authentizität.

Die folgende **physische OEM-Partitionstabelle** steht im entpackten Kernel
ab Dateioffset `0x62bf98` als acht 32-Byte-`mtd_partition`-Einträge. Namen,
64-Bit-Größen und Offsets wurden direkt aus diesen Datensätzen gelesen:

| Partition | Physischer Offset | Größe |
| --- | --- | --- |
| `u-boot` | `0x000000` | `0x020000` |
| `cfg` | `0x020000` | `0x010000` |
| `bdinfo` | `0x030000` | `0x010000` |
| `firmware` | `0x040000` | `0xec0000` |
| `kernel` | `0x040000` | `0x270000` |
| `rootfs` | `0x2b0000` | `0xc50000` |
| `rootfs_data` | `0xf00000` | `0x0f0000` |
| `backup` | `0xff0000` | `0x010000` |

`firmware` überlappt hier absichtlich `kernel` und `rootfs`. Die Tabelle
beschreibt einen 16-MiB-Flashbereich; sie ersetzt noch keine Messung an einem
konkreten WR1500 oder einen Nachweis der SPI-Read-Mapping-Einstellung.

Die OEM-Datei `/lib/upgrade/platform.sh` überspringt bei diesem NOR-Format
**192 KiB (`0x30000`)** und schreibt den Rest in die Partition `firmware`,
die bei **`0x40000`** beginnt. Daher liegen Kernel und Rootfs in der Datei
jeweils **`0x10000` vor ihrer Zieladresse im Flash**. Der GPL-Imagebuilder
beschreibt dieselbe Kombination aus Bootloader, Signatur/Padding und Firmware.

Ein direktes Schreiben dieser Datei als vollständiges SPI-Abbild würde
`cfg` und `bdinfo` nicht erhalten und Kernel sowie Rootfs falsch platzieren.
Die Datei enthält auch keinen vollständigen Inhalt von `rootfs_data` oder
`backup`. Sie ist kein Ersatz für ein gerätespezifisches Flash-Backup.

## Bootloader, Kernel und UART

Obwohl die Partition `u-boot` heißt, enthält sie **Realtek-BSP-Bootcode**.
Ein Gzip-Stream bei `0x8d60` entpackt sich zu 141.344 Bytes. Darin stehen
unter anderem `Realtek ... boot code`, die Realtek-Kommandos `LOADADDR`,
`XMOD`, `J`, `FLI` und `FLW` sowie TFTP-Funktionen. Das sind Hinweise auf die
vorhandenen Transfer-/Diagnosefunktionen, keine getestete RAM-Bootanleitung.
Der Image-Bootloader trägt die Kennung **`OEM_BOOT 00000002`**.

Der Kernel meldet GCC 6.4.1 / Realtek MSDK Build 3055 und als kompiliertes
Builddatum den 15. Dezember 2025. Der uImage-Zeitstempel ist dagegen der
4. Dezember 2025; die Zeitfelder sollten deshalb nicht als einheitlicher
Erstellungsnachweis behandelt werden.

Die kompilierte Kernel-Kommandozeile lautet:

```text
console=ttyS0,38400 root=/dev/mtdblock5
```

Damit gibt es jetzt eine konkrete **OEM-Kernelreferenz für 38.400 Baud**.
Der vorhandene experimentelle OpenWrt-RAM-Test verwendet weiterhin 115.200
Baud; ein OEM-Bootlog und die tatsächliche Übergabe der Bootargumente sind
für den Hardwaretest noch nötig. Die installierte physische RAM-Größe ist
aus diesem Paket nicht verlässlich bestimmt.

Es wurden **21.521 kallsyms-Einträge** rekonstruiert. Der Decoder konnte bei
diesem Kernel keine Exporttabellen-Gegenprüfungen durchführen; die ausgewählten
P0-/Switch-Routinen wurden zusätzlich durch Disassembly und GPL-Abgleich
plausibilisiert. Die vorhandenen ELF-Module sind **MIPS32 little endian, o32,
MIPS32r2**. Die Bezeichnung `mips_24kec` in `/etc/openwrt_release` allein wäre
kein hinreichender Endianness-Nachweis gewesen.

## WLAN-Treiber, Firmware und Tabellen

Das Rootfs enthält 3.172 Inodes. Ausgelesen wurden **2.532 reguläre Dateien**
mit zusammen **41.581.540 Bytes**, außerdem 431 Symlink-Einträge. Symlinks
wurden inventarisiert, aber nicht als Links im privaten Extraktionsverzeichnis
angelegt. Es sind **19 Kernelmodule** enthalten, darunter:

| Modul | Befund |
| --- | --- |
| `rtk_wifi6.ko` | 7.494.584 Bytes; Realtek-Treiber mit PHL/HALRF, RTL8832BR-/8192XB-Code und cfg80211-Referenzen |
| `rtl8192cd.ko` | 2.689.132 Bytes; separater Legacy-Treiber |
| `cfg80211.ko`, `compat.ko` | Backports-Basis aus Linux 5.2.8 für Kernel 4.4.176 |
| `ther_ctrl.ko` | Thermik-Abhängigkeit beider WLAN-Treiber |

Die OEM-Skripte ordnen **2,4 GHz `wlan1` / rtl8192cd** und **5 GHz `wlan0`
/ rtk_wifi6** zu. Die 5-GHz-Konfiguration enthält `11anacax`; geladen wird
`rtk_wifi6` mit `rtw_ht_enable=2`, `rtw_vht_enable=2`, `rtw_he_enable=2` und
`rtw_phy_file_path=/etc/conf/`.

`rtk_wifi6.ko` enthält die PCI-Aliasse **10ec:b832**, **10ec:b814** und
**10ec:0192**. Das sind unterstützte IDs des Moduls, **keine Messung der im
konkreten WR1500 bestückten PCIe-ID**. Die RTL8832BRE/RFE50-Dateien und der
zugehörige Code sind eine wesentlich konkretere Referenz als die bisherige
GPL-Option `CONFIG_RTL8852AE_BACKPORTS=y`.

Das Modul enthält `rtw_cfg80211_ops`, `wiphy_register`- und cfg80211-Bezüge,
aber auch `rtw_handlers_def`; die OEM-Skripte verwenden weiterhin `iwpriv`
und `set_mib`. Das OEM-System ist daher keine vollständig WEXT-freie Vorlage.

Unter `/etc/conf/rtl8832bre/RFE50/` liegen **23 Tabellen-Dateien mit insgesamt
209.420 Bytes**: PHY/Register, Gain, Radio A/B, Power-by-Rate, Power-Limits,
RU-Limits und TSSI-Tracking. Einige Dateien unter `AP_2G`/`AP_5G` enthalten
nur einen Versionskommentar und den Terminator `0xffff 0xffff`; sie sind
keine zusätzlichen vollständigen Registertabellen. Die gemeinsamen Tabellen
enthalten dagegen umfangreiche konkrete Datensätze.

Zusätzlich konnten zwei eingebettete Firmware-Arrays anhand der ELF-Symbole
lokal extrahiert werden:

| Symbol | Modul-Dateioffset | Größe |
| --- | --- | --- |
| `array_8192xb_u1_ap` | `0x4b2ee4` | 385.464 Bytes |
| `array_8192xb_u2_ap` | `0x454ce0` | 385.536 Bytes |

Ihre Hashes stehen in `evidence.json`. Diese Firmware und Tabellen sind
nützliche Referenzen für eine spätere Portierung; **das 4.4-Modul kann nicht
direkt als Kernel-6.6-Treiber verwendet werden**. Vollständiger zugehöriger
WLAN-C-Quelltext wurde dadurch nicht wiederhergestellt.

Die Tabellen sind keine gerätespezifischen Messwerte. `flash` und `libapmib.so`
verwenden `/dev/mtdblock1` sowie virtuelle Dateien wie `/var/HWCONF`; beim
OEM-Start werden Hardware-MIBs geprüft und gegebenenfalls aus Defaults
initialisiert. Die echten `cfg`-/`bdinfo`-Inhalte und die am Chip ausgelesenen
eFuse-Werte fehlen in dieser Upgrade-Datei. Sie dürfen nicht durch die hier
enthaltenen allgemeinen Defaults ersetzt werden.

## Switch, CPU-Link und GPIO

Die OEM-Skripte konfigurieren für R76 LAN über die Portnummern **3, 0, 1**,
WAN über **2** und logische Host-Verbindungen **`6@eth0` / `5@eth1`**.
Das ist die OEM-Konfiguration; die Reihenfolge der beschrifteten Buchsen
ist noch nicht am Gerät geprüft. Die GPL wählt RTL8367RB-VC, der Kernel
enthält RTL8367C- und RTL8367D-SDK-Pfade mit Erkennung zur Laufzeit.

Ausgewählte Codebelege:

- [`init_8197f_p0.dis`](init_8197f_p0.dis): Funktion bei `0x802780d8`, 384 Bytes.
  Sie verwendet im kompilierten Pfad **SoC-TX-Delay 1 und RX-Delay 0** sowie
  OEM-CPU-Tag-/Router-Einstellungen. Diese unterscheiden sich von unseren
  DSA-Anforderungen und von den bisherigen RD05-Timingwerten.
- [`RTL83XX_init.dis`](RTL83XX_init.dis): Funktion bei `0x802a0cc8`, 1.424 Bytes,
  mit rekonstruierten Namen der direkten Aufrufziele. Zunächst wird
  `rtk_port_rgmiiDelayExt_set(ext_id=0x10, tx=0, rx=2)` aufgerufen. Wenn das
  anschließende Aktivieren von SSC erfolgreich ist, folgt `tx=1, rx=0`.
  Die SDK-Port-ID ist nicht ohne weitere HAL-Zuordnung eine physische Pinnummer.

Der RD05-OEM-Kernel hat an dieser Stelle andere Funktionsgrößen und anderen
Code. Der R76-Ablauf ist ein wertvoller zusätzlicher Vergleich, aber noch
kein belegter Fix für die CRC-Probleme des RD05.

Der kompilierte GPIO-Initialisierungscode und die GPL-Feldnamen ergeben
folgende BSP-Pinnummern:

| Funktion | BSP-GPIO |
| --- | --- |
| System-/Reset-LED | 33 |
| WPS-LED | 57 |
| Reset-Taster | 54 |
| WPS-Taster | 34 |
| Externer Switch-Reset | 58 |
| SMI MDC/MDIO bei Bond `97FN` | 56 / 55 |
| SMI MDC/MDIO bei anderen Bonds | 29 / 31 |

Die Auswahl erfolgt über `rtl819x_bond_option()`. Der reale Bond-Wert,
Polaritäten und die elektrische Funktion sind noch nicht gemessen. Daher
werden diese Kandidaten hier dokumentiert und nicht blind in den DTS aktiviert.

## Beschleunigung und Nutzen für unseren Zweig

Die OEM-Startscripte aktivieren `/proc/fast_nat` und `/proc/hw_nat`; zusätzlich
werden VLAN-/Workmode- und Bridge-Shortcut-Parameter gesetzt. Das bestätigt
die private SDK-Weiterleitung im OEM-System. Es ist kein Nachweis einer
korrekten firewall4-Integration für OpenWrt 24.10. Unsere bestehende
Offload-Policy bleibt unverändert.

Der Gewinn für die weitere Arbeit ist konkret: bekannte Container-Grenzen,
eine rekonstruierte OEM-Flashmap, die echte Wi-Fi-6-Binärreferenz samt
Firmware-/Registertabellen, ausgewählte Switch-/P0-Routinen und GPIO-Kandidaten.
Es wurden **keine Flash-, WLAN-, Switch- oder GPIO-Funktionen** im bisherigen
UART/RAM-Profil aufgrund dieser statischen Analyse freigeschaltet.

Als nächste unabhängige Hardwarebelege fehlen insbesondere OEM-UART-Bootlog,
tatsächliche PCI-ID, RAM-Ausstattung, Switch-ID/Bond-Wert sowie ein eigener
SPI-Abzug mit den ursprünglichen Factory-/Boarddaten.

Die vollständige private Extraktion liegt im Analyse-Workspace unter
`/workspace/rtl-build-validation/wr1500-r76/`: `kernel.raw`,
`bootloader-unpacked.bin`, `kernel-symbols.json`, `rootfs/`,
`rootfs-inventory-private.json` und `embedded-firmware/`. Rootfs-Binaries,
Zertifikatsschlüssel und sonstige OEM-Konfigurationsinhalte werden nicht
zusätzlich in diesem Analyseordner im Repository veröffentlicht.
