# OEM-Dumps: Integration für MW5, AC23 und RD05

Die vollständigen privaten SPI-Abbilder wurden offline untersucht. Kernel und
Dateisysteme lassen sich entpacken; beim AC23 musste eine Analysekopie von
`nice`/Tenda-XZ auf Standard-SquashFS/XZ normalisiert werden. Die Originale
wurden nicht verändert. Private Abbilder, Factory-Werte, MAC-Adressen und
extrahierte OEM-Binärdateien gehören nicht in Git.

Die drei untersuchten Dateien sind bytegenau identisch mit
[`RT8197F/openwrt24.10-realtek-rtl8197f-mw5/fullflash-templates`](https://github.com/tobyw121/Openwrt_RTL/tree/e6d68e9067f54b695022f61ed52e50dc56c5a7b7/openwrt24.10-realtek-rtl8197f-mw5/fullflash-templates).
Das wurde durch Vergleich der Git-Blob-IDs mit `git hash-object` geprüft.
Der Remote-Branch stand bei der Prüfung ebenfalls auf diesem Commit.

## Belege und Formate

| Board | Factory-Bereich | Format | Übernahme |
| --- | --- | --- | --- |
| MW5 | 0x20000, 64 KiB | Tenda `HW_WLAN0/1_*`-Datensätze | beide Radios getrennt, darunter 2,4-GHz-Leistungstabellen und Quarz-/Temperaturabgleich |
| AC23/Lynx | 0x20000, 64 KiB | Tenda `HW_WLAN0/1_*`-Datensätze | zusätzlich TSSI für vier 5-GHz-Pfade A/B/C/D und deren Temperaturreferenzen |
| RD05 | 0x40000, 64 KiB | binärer `H601`-Block | beide radiospezifischen Datensätze einschließlich Leistung, TSSI, Quarz und Temperatur |

Die Tenda-Werte sind zeilen-/NUL-getrennt. Skalare sind Dezimalzahlen,
Byte-Arrays kompakte Hex-Zeichenketten. Diese Unterscheidung ist relevant:
die Zahl `42` darf nicht als Hexzahl 0x42 in einen eFuse-Schatten geschrieben
werden. Der neue Import schreibt unmittelbar in die passenden RF-MIB-Felder.

Der RD05-Aufbau stammt aus **seiner eigenen** `/usr/lib/libapmib.so`:
`hwmib_root_table`, `hwmib_table` und `hwmib_wlan_table`. Die Strukturmetadaten
mit Pfad, Bibliotheks-Hash und ausgewählten Feld-Offsets stehen in
`target/linux/realtek/rtl8197f/oem-calibration-schema.json`. Keine Kalibrierwerte
sind enthalten. Das OEM-WLAN-Skript `/lib/wifi/miwifirealtek.sh` bestätigt die
Zuordnung zum 8812-Pfad und die Übernahme vor dem WLAN-Start.

Beim RD05 werden exakt diese Eigenschaften geprüft:

- Signatur `H601`, sechs Byte Header, Big-Endian-Längenfeld 6048;
- 6047 Byte HW_SETTING und ein zusätzliches additives Prüfsummenbyte;
- Summe der 6048 Nutz-/Prüfsummenbytes modulo 256 gleich null;
- WLAN_ROOT bei Nutzdaten+0x409, zwei Radio-Datensätze mit je 2507 Byte.

Ein anderes H601-Layout wird nicht als RD05 interpretiert. Insbesondere wird
das bestehende WRD12GN-Profil nicht automatisch an diesen Parser angebunden.

## Verhalten des Linux6-Treibers

`linux6/rtl8192cd_factory.c` wählt anhand des tatsächlichen Chips zwei bzw.
vier Funkpfade und das passende Band. Der begrenzte Parser in
`linux6/rtl8192cd_factory.h` erlaubt ausschließlich ausgewählte RF-Felder.
Der DT fordert das Format mit `realtek,factory-format = "tenda-kv"` bzw.
`"rd05-h601"` ausdrücklich an. Der Factory-Bereich bleibt schreibgeschützt.

Die Daten werden nach Vendor-/eFuse-Vorgaben und erneut vor der Hardware-
Initialisierung bei Open/Recovery angewendet. Kein WEXT-Aufruf und keine
OTP-/eFuse-Programmierung ist erforderlich. Die fest eingetragenen
MW5-Kalibrierkonstanten einer einzelnen Referenzplatine wurden entfernt.
Die MW5-RFE5/6-Vorgaben und der bisherige BSS-Grundzustand bleiben erhalten.

Ungültige Einzelfelder verändern ihr Ziel nicht. Abgewiesen werden unter
anderem doppelte oder unvollständige Datensätze, ungültige Hex-Zeichen,
unpassende Array-Längen, vollständig gelöschte Arrays und Skalare außerhalb
des zulässigen Bereichs. Ein ungültiger RD05-Header oder eine falsche
Prüfsumme verhindert jede Übernahme aus dem Block. Die Tenda-Container-
Prüfsumme wird bislang nicht geprüft; hier wird jedes unterstützte Feld
strukturell und auf seinen Wertebereich geprüft.

TSSI wird aus der Factory nur aktiviert, wenn alle für Band und Pfadzahl
benötigten TSSI-Tabellen vollständig eingelesen wurden. Alte MW5-5-GHz-
Basistabellen mit 177 Kanaleinträgen werden begrenzt übernommen; die übrigen
Einträge bleiben erhalten. Die inzwischen aus den MW5-/RD05-OEM-Programmen
und dem SDK rekonstruierte 14-Byte-AC-Gruppenzuordnung wird für acht
5-GHz-Differenztabellen der Pfade A/B übernommen; vier zusätzliche
2,4-GHz-Felder werden direkt eingelesen. Diese Gruppensätze sind **nicht**
mit vollständigen 196-Kanal-Tabellen gleichzusetzen. Details und Belege:
[AC-Kalibrierung und MTU-Profile](reverse-engineering/oem-kernels/CALIBRATION_AND_MTU_FINDINGS.md). Die
vollständige chipbezogene Power-Tracking-/Regulatory-Integration bleibt zu
validieren; dieser Stand behauptet keine vollständige OEM-RF-Parität.

Für Tenda-Boards werden vorhandene per-Radio-MAC-Datensätze bevorzugt.
Die Basis-MAC mit dem bisherigen DT-Inkrement bleibt der Ersatzweg. RD05
verwendet weiterhin seine separaten U-Boot-Umgebungszellen für MAC-Adressen.

## Befunde zur Netzwerkstabilität

Die MIPS-Disassemblierung des MW5-`phy_check.ko` zeigt einen aktivierbaren
Timerpfad, Portstatus-Abfragen über `switch_phy_status_get`, Link-Ereignisse
und `_rtl865x_clearHWL2Table_dynamic` im Down-Zweig. Die proc-Ausgaben dieses
Moduls lesen gespeicherte Zustände. Das ist kein Beweis, dass gerade diese
Dateien den stabilisierenden Effekt von `mw5-netdiag full` auslösen.

Das AC23-Modul verwendet einen anderen Ablauf mit verzögerter Arbeit und
Netlink-/Signal-Benachrichtigungen. Beide alten Module hängen an privaten
OEM-Schnittstellen. Sie werden nicht in Linux 6.6 geladen. Der vorhandene
DSA-/Phylink-Pfad wird nicht um unbewiesene Registerzugriffe oder eine
periodische pauschale Löschung von MAC-Tabellen erweitert.

Die weitere Rückentwicklung ist in [OEM_SWITCH_REVERSE.md](OEM_SWITCH_REVERSE.md)
dokumentiert. Inzwischen sind die Symboltabellen aller drei OEM-Kernels,
der MW5-Common-Init-Zweig für die geloggte ID 0x6367/0x0060 und acht externe
Flow-Control-Schreibzugriffe rekonstruiert. Patch 332 ergänzt diese acht
Werte gezielt für diese MW5-Identität. Vollständige Kaltstart-Parität und
die Ursache des Netzstillstands sind weiterhin nicht hardwarevalidiert.
Die MW5-Softwarebeschleuniger und RX-Batching bleiben standardmäßig aus.
Die OEM-3.10-FastNAT-/Fast-L2-Module des AC23 werden nicht übernommen.

## Prüfungen und Hardwareabnahme

Die tatsächlichen Parserfunktionen werden mit dem echten RF-MIB-Layout in
Hosttests kompiliert und mit UndefinedBehaviorSanitizer ausgeführt. Die
Tests behandeln beide Formate, getrennte Radios, zwei/vier Pfade, TSSI-
Vollständigkeit, beschädigte Daten und erhaltene Ausgangswerte. Zusätzlich
wurde der Parser lokal gegen die Originaldaten aller sechs Radios geprüft,
ohne individuelle Werte in Testergebnisse oder Git zu schreiben:

| Dump/Radio | angewendete RF-Felder | verworfene RF-Felder |
| --- | ---: | ---: |
| MW5 / 0, 5 GHz | 6 | 0 |
| MW5 / 1, 2,4 GHz | 11 | 0 |
| AC23 / 0, 5 GHz | 10 | 1 |
| AC23 / 1, 2,4 GHz | 9 | 0 |
| RD05 / 0, 5 GHz | 14 | 0 |
| RD05 / 1, 2,4 GHz | 18 | 0 |

Der abgewiesene AC23-Wert liegt außerhalb des 6-Bit-Quarzabgleichbereichs;
der vorhandene Standard bleibt erhalten. Die Zähler bezeichnen MIB-Felder,
nicht zwingend unterschiedliche Quellrecords: THER wird auch in thermal[0]
übernommen. Sie sind kein Beweis für bessere Sendeleistung oder Reichweite.

Auf dem Gerät stehen `factory_applied`, `factory_rejected` und `factory_len`
unter `/sys/kernel/debug/rtl8192cd/<Gerät>/` sowie die zusammengefasste
`factory RF:`-Kernelmeldung zur Verfügung. Nicht nur das Vorhandensein der
NVMEM-Zelle, sondern die tatsächliche Übernahme kontrollieren. Ein eigener
Factory-Bereich je Router und das passende Board-DTS sind erforderlich.

Nach einem vollständigen OpenWrt-Build/MODPOST und Flashen: Kaltstart,
WLAN-Start/Stop, Neustart, MAC-Adressen, Region, beide Bänder, Signal/Raten,
bidirektionale Last und thermisches Verhalten auf jedem Board prüfen.
MW5-Netzlast insbesondere ohne laufendes `mw5-netdiag full` testen.

Die vollständigen entpackten Kernel-Images aller drei Geräte, ihre gesamten
Symboltabellen und wortweise geprüften Disassemblierungen sind jetzt für
spätere Untersuchungen im Repository archiviert:
[`reverse-engineering/oem-kernels/`](reverse-engineering/oem-kernels/README.md).
Factory-/Umgebungspartitionen und Rootfs sind nicht Teil dieser Ablage.
