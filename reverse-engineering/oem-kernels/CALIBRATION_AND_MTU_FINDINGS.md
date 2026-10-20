# Kompakte AC-Kalibrierung und RD05-Paketlängenprofile

## Zwei OEM-Programme bestätigen die Kanalzuordnung

Die SDK-Funktion `assign_diff_AC` in
`rtl819x/users/boa/utils/flash.c` expandiert 14 Factory-Bytes in eine
kanalweise MIB-Tabelle. Der jetzt geprüfte Code von `/bin/flash` aus dem
MW5-Dump und `/usr/sbin/flash` aus dem RD05-Dump stimmt damit überein:

| Datei | Funktionsbereich im ELF |
| --- | --- |
| MW5 `/bin/flash` | `0x00403308` bis ausschließlich `0x0040341c` |
| RD05 `/usr/sbin/flash` | `0x00010c48` bis ausschließlich `0x00010dcc` |

Der MW5-Compiler ersetzt das erste `memset` durch fünf einzelne Byte-
Schreibzugriffe. RD05 verwendet 14 indirekte `memset`-Aufrufe; dessen
MIPS-Dynamiksymboltabelle und GOT ordnen den verwendeten Slot `0x2e63c`
tatsächlich dem importierten `memset` zu. Der letzte Aufruf ist jeweils
ein Tailcall. Delay-Slots gehören zur jeweiligen Aufrufvorbereitung.

Die 15 Grenzen der 14 Bereiche sind **nullbasierte MIB-Indizes**:

```text
35, 40, 48, 56, 64, 104, 112, 120, 128, 136, 144, 153, 161, 169, 177
```

Für jeden Eintrag gilt `[Grenze[i], Grenze[i+1])`. Damit füllt das erste
Byte die Indizes 35..39, das fünfte 64..103 und das letzte 169..176.
Die Indizes 0..34 und 177..195 werden nicht geschrieben. Dies ist nicht
die eFuse-Kanalgruppentabelle für Basisleistungen und auch keine Liste
freizugebender Funkkanäle. Die Bytes enthalten gepackte Leistungsdifferenzen;
ihre beiden Nibbles bleiben unverändert. Ein 14-Byte-Gruppensatz darf deshalb
weder als 196-Byte-Kanaltabelle noch als 14 absolute Leistungswerte gelten.

`scripts/rtl8197f-oem-ac-calibration.py` akzeptiert nur die bekannten
Dateihashes. Es ordnet ELF-Adressen auf dateigestützte LOAD-Segmente ab,
rekonstruiert die RD05-APMIB-Metadaten und modelliert die beiden kurzen
MIPS-Routinen mit synthetischen Puffern. Das Modell erlaubt nur die dort
benötigten Instruktionen, begrenzte Stack-/Pufferzugriffe und bekannte
`memset`-Ziele. Unbekannte Instruktionen oder Ziele brechen die Analyse ab.
OEM-Programme werden weder nativ noch mit einem Systememulator gestartet.

Die exportierten Belege liegen in
`target/linux/realtek/rtl8197f/oem-ac-calibration-evidence.json`: Quell- und
Dateihashes, Funktionswörter, Zuordnung und strukturelle APMIB-Felder.
Keine vollständigen Userspace-Binärdateien oder individuellen Factory-Werte
werden neu ins Repository aufgenommen.

Reproduktion mit bereits statisch extrahierten OEM-Dateien:

```sh
python3 scripts/rtl8197f-oem-ac-calibration.py \
  --mw5-flash /privat/mw5/bin/flash \
  --rd05-flash /privat/rd05/usr/sbin/flash \
  --rd05-apmib /privat/rd05/usr/lib/libapmib.so \
  --output /tmp/ac-evidence.json
```

Die Ausgabe muss neu sein; das Werkzeug überschreibt keine bestehende Datei.

## Import in den gemeinsamen Linux6-WLAN-Treiber

Der Importer kennt jetzt für die RF-Pfade A/B jeweils:

- 2,4 GHz: `TX_POWER_DIFF_20BW1S_OFDM1T_*` und
  `TX_POWER_DIFF_40BW2S_20BW2S_*`, direkt je 14 Kanaleinträge.
- 5 GHz: `TX_POWER_DIFF_5G_20BW1S_OFDM1T_*`,
  `TX_POWER_DIFF_5G_40BW2S_20BW2S_*`,
  `TX_POWER_DIFF_5G_80BW1S_160BW1S_*` und
  `TX_POWER_DIFF_5G_80BW2S_160BW2S_*`, expandiert nach obiger Zuordnung.

Für RD05 wurden die Offsets aus dessen eigener `libapmib.so` gelesen:
Symbol `hwmib_wlan_table`, ELF-Adresse `0x1f880`, Größe 5916 Byte,
Datensatzabstand 68 Byte. Alle zwölf neuen Felder haben Typ BYTE_ARRAY
und Länge 14. Die Offsets liegen innerhalb des bekannten Radioabstands
von 2507 Byte; sie stammen nicht aus einem anders gepackten SDK-C-Struct.
Die bestehenden Header- und Prüfsummenprüfungen bleiben Voraussetzung.

Der MW5-Factory-Store enthält acht passende 5-GHz-Gruppensätze. Sie wurden
bisher übersprungen und können jetzt aus der jeweiligen Platine übernommen
werden. Im untersuchten AC23-Factory-Store fehlen diese kompakten Felder.
Dort verändert diese Erweiterung die bisherigen Werte nicht. Vorhandene
Basis-/TSSI- und Thermalimporte bleiben aktiv; es werden keine fremden
Kalibrierwerte ergänzt. Eine vollständige AC23-Vier-Pfad-RF-Parität ist
damit nicht belegt.

Die Expansion erfolgt erst nach vollständiger Prüfung des einzelnen
Datensatzes. Falsche Länge, doppelte KV-Schlüssel, ungültige Hex-Zeichen
und vollständig gelöschte Arrays verändern ihr Ziel nicht. Für die neuen
5-GHz-Gruppenfelder wird genau die belegte Länge 14 akzeptiert; eine
196-Byte-Tabelle unter demselben Schlüssel wird abgewiesen. Band- und
Radiozuordnung bleiben wirksam. Die nicht abgedeckten Kanaleinträge bleiben
aus den bisherigen HAL-/eFuse-Vorgaben erhalten.

## Switch: Profil 0 tatsächlich auswählen

Der RD05-OEM-Wrapper `rtk_switch_maxPktLen_set` bei `0x802f77d8`
programmiert Profil 0 und versucht anschließend in einer Schleife die
Portauswahl für die Gigabit-Geschwindigkeit auf Profil 0 zu setzen.
Der DAL-Code bei `0x80315678` berechnet die Bitposition als
`physical_port + speed * 8` und ruft den Bitsetter für `0x0883` auf.
Der Profilsetter bei `0x803157bc` schreibt `0x088c + cfgId` mit Maske
`0x3fff`. Dass dieselben Profilregister auch in den C-Familien-Kernels
vorhanden sind, ist ebenfalls im erweiterten Interface-Beleg festgehalten.

Die passende D-SDK-Quelle
`8197/RT8197F_RTL8367RB_Gesamtpaket/AX12v1_GPL_Paket/source_subset/rtl8197/rtknet/drivers/net/rtl819x/rtl83xx_v1dot4/dal/rtl8367d/dal_rtl8367d_switch.c`
und `rtl8367d_reg.h` bestätigen:

| Register | Bedeutung |
| --- | --- |
| `0x0883`, Bits 7:0 | Profilauswahl für physische Ports 0..7 bei 10/100 Mbit/s |
| `0x0883`, Bits 15:8 | dieselbe Auswahl bei Gigabit |
| `0x088c`, Bits 13:0 | Paketlängenprofil 0 |
| `0x088d`, Bits 13:0 | Paketlängenprofil 1 |

Unser DSA-Treiber programmierte bisher lediglich Profil 0. Patch 334
wählt es jetzt zusätzlich für die aktiven RTL8367D-User-, CPU- und
DSA-Ports bei beiden Geschwindigkeiten aus. Das passt zur bereits
vorhandenen DSA-Regel eines globalen Längenlimits. Auswahlbits ungenutzter
Ports bleiben unverändert. Fehler beim Schreiben der Länge verhindern
die Folgeoperation; Fehler der Profilauswahl werden zurückgegeben.
Die zusätzliche Registeroperation ist auf die D-Familie begrenzt.
Der C-Familien-Pfad von MW5 und AC23 bleibt unverändert.

Dies beseitigt die Abhängigkeit von einer möglicherweise geerbten
Profilauswahl. Es ist keine Messung, dass beim untersuchten Gerät zuvor
Profil 1 aktiv war oder dass dieser Zustand den Netzausfall verursachte.
Der Patch hebt keine Ethernet-/MTU-Grenzen an und aktiviert keine Beschleuniger.

## Validierung und verbleibende Arbeit

Für diesen Änderungssatz bestanden 57 Hosttests, die vier Boardprofile,
26 MW5-Baseline-Prüfungen und die klassische Routingprüfung. Alle drei
WLAN-Varianten wurden gegen Linux 6.6.114 mit den gepatchten OpenWrt-
Backports 6.12.96 zu `rtl8192cd.o` übersetzt und gelinkt, ohne unaufgelöste
WEXT-Symbole. Der gepatchte `rtl8365mb.o` übersetzt ebenfalls für MIPS.
Das WLAN-Paket trägt Release 100. Dies ersetzt weder einen vollständigen
Firmware-/MODPOST-Build noch Hardwaretests.

Die neuen Regressionen vergleichen die OEM-Routinen mit 256 synthetischen
Gruppensätzen, die gesamte 196-Byte-Ausgabe und die unveränderten Ränder.
Der tatsächliche C-Importer wird für alle acht Gruppenfelder über Tenda-KV
und RD05-H601 ausgeführt. Fehlerfälle prüfen die Unverändertheit des
vollständigen RF-MIB. Der tatsächliche DSA-Code wird mit simuliertem regmap
auf Registerreihenfolge, beide Geschwindigkeiten, ungenutzte Ports,
C-/D-Auswahl und Fehlerweitergabe geprüft. Die MIPS-Belege werden gegen
die vollständigen archivierten Kernelbytes abgeglichen.

Weiterhin erforderlich sind Geräteprüfungen: je Band Verbindung und
Wiederverbindung, Kanäle/Bandbreiten, WPA2/PMF, Multicast/Broadcast,
Temperaturregelung und bidirektionale Last; beim RD05 zusätzlich 100- und
1000-Mbit-Linkwechsel sowie verschiedene zulässige MTUs. Vollständige
Switch-Kaltstart-Parität, AC23-C/D-Pfadkalibrierung und weitere WLAN-
Lebenszyklen sind weiterhin offen. Diese Änderung beansprucht weder
vollständige Rückentwicklung noch einen gemessenen Durchsatzgewinn.
