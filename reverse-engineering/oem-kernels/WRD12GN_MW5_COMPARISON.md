# WRD12GN und MW5: Dump-Vergleich

Der WRD12GN ist beim WLAN sehr eng mit dem MW5 verwandt. Die beiden Geräte
haben in den untersuchten OEM-Kerneln dieselben WLAN-HALs, dieselbe statische
Radiogerätetabelle und viele bytegleiche RF-Tabellen. Ein identisches Board
oder ein austauschbares Flash-Image ist damit nicht belegt. Switchpfad,
Werksdatenformat und Flashaufteilung unterscheiden sich.

Die maschinenlesbaren Belege einschließlich SHA-256, Symboladressen und
Kalibrierungsfeld-Offsets stehen in
[wrd12gn-mw5-comparison.json](wrd12gn-mw5-comparison.json). Es werden keine
persönlichen Werkswerte, Zugangsdaten oder extrahierten OEM-Programme abgelegt.

## Untersuchte Eingaben

Neu aus Commit `9a6e6f7a6f5e907208b4bcd18bedeef5dab9e64e`:

- `iball-wrd12gn-firmware-backup.bin`: 8 MiB,
  SHA-256 `52dbee9e48c755bd9df78a5bb37b0958d6bb8dc1cbbcbfbaec280535712bcee8`.
- `wrd12gn_firmware_fixed.bin`: 8 MiB,
  SHA-256 `1af62630dcad6fb329c1991ab407e0a1298dac5c3148a122f8aa3e4518c2d169`.

Beide WRD12GN-Dateien sind bis einschließlich `0x78ffff` bytegleich.
39.589 Bytes unterscheiden sich ausschließlich ab `0x790000` bis zum Flashende.
Damit sind insbesondere Boot-/Factory-Bereich, komprimierter Kernel und
Root-Dateisystem gleich. Der Dateiname „fixed“ belegt keine Treiberkorrektur.
Die beiden Dateien sind keine unabhängigen Kernel-/Kalibrierungsreferenzen.

Die MW5-, AC23- und RD05-Kernel stammen aus den vorhandenen Archiven;
ihre gepackten und entpackten Hashes wurden gegen die Manifeste geprüft.

## WLAN: besonders nahe am MW5

| Merkmal | WRD12GN | MW5 | AC23 | RD05 |
| --- | --- | --- | --- | --- |
| Integrierte WLAN-HAL | RTL8197F | RTL8197F | RTL8197G | RTL8197G |
| Externe WLAN-HAL | RTL8822BE | RTL8822BE | RTL8814BE | RTL8812FE |
| OEM-Kernel | 3.10.90 | 3.10.90 | 3.10.90 | 4.4.176 |
| Externer RTL83xx-Switch-Treiber im untersuchten Kernel | nicht gefunden | vorhanden | vorhanden | vorhanden |

WRD12GN enthält `hal_Associate_8197F` bei `0x8036af88` und
`hal_Associate_8822BE` bei `0x8036cdb0`. Eine 8197G-/8812F-/8814B-HAL ist
in diesem Kernel nicht enthalten. Beim MW5 heißen die passenden Funktionen
genauso. Das belegt die Treiberfamilie, nicht die aufgedruckte Chipvariante.
Der MW5 verwendet laut bisherigen Gerätebelegen einen physischen RTL8812BRH
mit RTL8822B-HAL. Die genaue externe WRD12GN-Bestückung ist damit noch nicht
physisch identifiziert.

Die ersten zwei 20-Byte-Einträge von `wlan_device` sind vollständig bytegleich:

| Eintrag | Typwort | PCI-Konfiguration | MMIO | OEM-IRQ |
| --- | --- | --- | --- | --- |
| 0 / externes Radio | `0x106` | `0xb8b10000` | `0xb9000000` | 5 |
| 1 / integriertes Radio | `0x102` | 0 | `0xb8640000` | 6 |

Die Tabelle liegt beim WRD12GN bei `0x805c9900`, beim MW5 bei `0x805c8d70`.
OEM-IRQ-Nummern sind nicht mit den Linux-6.6-IRQ-Nummern gleichzusetzen.
Die Zuordnung externes Radio 0 / internes Radio 1 stimmt ebenfalls überein.

### Vergleich eingebundener RF-Tabellen

Die `data_*_start/end`-Symbole wurden als Zeiger auf tatsächliche Datenbereiche
aufgelöst. Verglichen wurden die vollständigen Bytes dieser Bereiche, nicht
nur Symbolnamen oder gleich lange Arrays.

| Tabelle | Größe in beiden Kerneln | Ergebnis |
| --- | ---: | --- |
| AGC_TAB_8197F | 4517 Byte | bytegleich |
| PHY_REG_8197F | 11180 Byte | bytegleich |
| RadioA_8197F | 1026 Byte | bytegleich |
| RadioB_8197F | 659 Byte | bytegleich |
| AGC_TAB_8822B | 4699 Byte | bytegleich |
| PHY_REG_8822B | 25573 Byte | bytegleich |
| RadioA_8822B | 5336 Byte | bytegleich |
| RadioB_8822B | 4096 Byte | bytegleich |

Von 71 gleichnamigen, auflösbaren Datenpaaren sind 61 bytegleich.
Darin sind auch zusätzliche Tabellen und Firmware-Blobs enthalten; das ist
keine Prozentangabe zur Gleichheit der Hardware oder aller Treiberfunktionen.

Die normale RTL8822B-Firmware unterscheidet sich:
WRD12GN 84064 Byte, MW5 81896 Byte. Die MP-Testfirmware ist dagegen mit
77264 Byte bytegleich. Eine Übernahme der normalen Firmware bedarf einer
separaten Kompatibilitätsprüfung.

WRD12GN-Kernel: 7104668 Byte, SHA-256
`a3daef16122e06ec89a16913ee318e6520ce7b0041adc97b1b04e7d01b648437`.
Die vorhandene kallsyms-Rekonstruktion liefert 28884 Symbole; 3657 exportierte
Symbol-/Namenszeiger wurden zusätzlich gegengeprüft. Der Kernel wurde am
4. Oktober 2018 mit Realtek MSDK GCC 4.4.7 gebaut. Das MW5-Image enthält einen
Build vom 9. September 2025. Das Builddatum allein belegt keine neuere
SDK- oder WLAN-Firmwareversion.

## Netzwerkpfad und Flash sind unterschiedlich

Der WRD12GN-Kernel enthält den internen RTL865x-/`swNic`-Pfad, aber keine
Symbole der geprüften externen RTL836x-/RTL83xx-Initialisierung. Auch im
extrahierten Root-Dateisystem wurde kein externer Switch-Treiber gefunden.
Das passt zu den bisherigen OEM-Laufzeitbelegen des WRD12GN. Das Fehlen eines
Symbols allein wäre kein endgültiger physischer Nachweis.

Beim MW5 sind dagegen 648 Symbole der geprüften externen Switch-Familien
vorhanden. Die bisherigen Gerätebelege identifizieren dort RTL8363NB und
seinen RGMII-CPU-Link. Diese Port-, VLAN-, RGMII- und DSA-Einstellungen dürfen
nicht auf den WRD12GN übertragen werden. Beide Geräte können RTL865x-Code
enthalten und trotzdem unterschiedliche physische Ethernetpfade besitzen.

| Bereich | WRD12GN-Dump | MW5-Dump |
| --- | --- | --- |
| Flashgröße | 8 MiB | 8 MiB |
| Realtek-Imageheader | `cr6c` bei `0x30000` | `cr6c` bei `0x30000` |
| LZMA-Kernelstream | `0x32818` | `0x32818` |
| OEM-SquashFS | `0x230000` | `0x228012` |
| Werksdaten | H601-Binärstruktur bei `0x20000` | anderes MW5-Werksdatenformat |
| WRD-Variantenunterschiede | nur `0x790000..0x7fffff` | nicht übertragbar |

Die bisherigen Boardprofile nennen 64 MiB RAM beim WRD12GN und 128 MiB beim
MW5 sowie unterschiedliche UART-Baudraten. Diese Angaben stammen aus früheren
Board-/Laufzeitbelegen, nicht aus einer RAM-Messung an den neuen Dumps.
Auch die genaue SoC-Gehäuse-/Bonding-Variante, GPIOs, PA/LNA und RFE-Verdrahtung
werden durch bytegleiche WLAN-Tabellen nicht bewiesen.

## WRD12GN-Kalibrierungsstruktur neu bestimmt

Alle regulären Dateien des WRD12GN-Rootfs wurden privat extrahiert: 679 Dateien,
13892812 Byte. Die beiden WLAN-Treiber sind in den Kernel eingebaut. Zu den
relevanten Userspace-Werkzeugen gehören `/bin/flash`, `/lib/libapmib.so`,
`/bin/wlanapp.sh` und `/bin/set_rx_gain_from_flash.sh`.

Die ELF-Dateien haben keine Section-Header mehr. Ihre dynamischen Symbole
lassen sich dennoch auslesen, beispielsweise mit `readelf -Ds`.
Die OEM-Bibliothek enthält:

- `hwmib_root_table` bei `0x267d4`, 136 Byte.
- `hwmib_table` bei `0x2685c`, 340 Byte.
- `hwmib_wlan_table` bei `0x1d4a4`, 4828 Byte.
- Tabellenzeilen mit 68 Byte; 70 benannte WLAN-Felder.

Aus diesen Tabellen folgen 3213 Byte HW_ROOT-Nutzdaten, darin WLAN_ROOT ab
Offset 13 mit 3200 Byte und zwei 1600-Byte-Radioblöcke. Der H601-Header hat
6 Byte. Seine Big-Endian-Längenangabe lautet 3214 Byte einschließlich der
abschließenden Prüfsumme; deren gesamte Byte-Summe ist modulo 256 null.
Alle 70 WLAN-Felder passen innerhalb eines 1600-Byte-Blocks.

| Struktur | Absoluter Flashbereich / Adresse |
| --- | --- |
| H601-Header | `0x20000..0x20005` |
| HW_ROOT | ab `0x20006`, 3213 Byte |
| WLAN-Record 0 | ab `0x20013`, 1600 Byte |
| WLAN-Record 1 | ab `0x20653`, 1600 Byte |
| XCAP innerhalb eines WLAN-Records | `+149`, 1 Byte |
| Thermal innerhalb eines WLAN-Records | `+152`, 1 Byte |
| 2,4-GHz-CCK-A-Tabelle | `+48`, 14 Byte |
| 5-GHz-HT40-1S-A-Tabelle | `+163`, 196 Byte |

Die einzelnen Kalibrierungswerte wurden nicht veröffentlicht. Ein Record-
Index aus der APMIB-Struktur ist nicht ohne weitere Prüfung mit einer
Linux-Radionummer gleichzusetzen.

Der RD05 besitzt ebenfalls H601, aber 6048 Byte einschließlich Prüfsumme
und eine andere Hardware-/Radiostruktur. Der derzeitige RD05-H601-Importer
ist daher keine passende WRD12GN-Kalibrierungslösung. Ebenso ersetzen die
bytegleichen allgemeinen RF-Tabellen keine gerätespezifische Kalibrierung.

Zusätzlich besteht Prüfbedarf bei den bisherigen WRD12GN-MAC-Zellen im DTS:
Die neue OEM-Tabelle beschreibt NIC0 bei HW_ROOT+1 und NIC1 bei HW_ROOT+7,
also ab Flash `0x20007` und `0x2000d`. Die bestehenden DTS-Zellen beginnen
bei Factory+8/+14. Diese Abweichung wurde hier dokumentiert, ohne ungeprüft
die Adresszuordnung oder den Treiber umzubauen.

## Konsequenz für unseren Treiber

Der WRD12GN ist von den verglichenen Geräten der passendste zusätzliche
Referenzkandidat für den WLAN-Pfad des MW5. Gemeinsame Fehler bei
RTL8197F-/RTL8822B-DMA, Management-TX und cfg80211 lassen sich dort sinnvoll
gegenprüfen. AC23 und RD05 verwenden abweichende WLAN-HALs.

Gemeinsam nutzbar sind HAL-/PHYDM-Code und nachweislich identische allgemeine
RF-Tabellen. Separat bleiben Werksdatenimport, RFE-/Antennenkonfiguration,
Ethernet-/Switchpfad, Partitionen, GPIOs und Bootparameter. Die Images sollten
weiterhin eigene Boardprofile behalten. Aus dieser Analyse wurden keine
RF-Werte kopiert oder laufenden Treiberparameter geändert.
