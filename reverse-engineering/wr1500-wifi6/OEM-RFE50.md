# WR1500 OEM RFE50: Dateivergleich, 01.11.2026

Untersucht wurde das Benutzerarchiv `wr1500-oem-rfe50.tar.gz`, erzeugt aus
`/etc/conf/rtl8832bre/RFE50` der WR1500-OEM-Firmware. Die vorangehenden
Laufzeitlogs belegen, dass rtk_wifi6 diese Basisdateien und AP_5G-Dateien
tatsächlich lädt; Firmwareversion V0.27.88.0 erreicht dabei Ready.

Das Archiv enthält 38 reguläre Dateien. Alle wurden ohne Ausführung und
unter Ausschluss absoluter Pfade, übergeordneter Pfade und Links untersucht.
Vergleichsgrundlage ist der bereits extrahierte G6-GPL-Quellbaum unter
`platform/mips_98d/rtl8832bre/RFE50`.

## Ergebnis

36 Dateien sind bytegenau identisch. Abweichend sind ausschließlich
`TXPWR_ByRate.txt` und `AP_5G/TXPWR_ByRate.txt`. Die OEM-Dateien dieser
beiden Pfade sind untereinander identisch. In jeder unterscheiden sich
fünf 5-GHz-Datenzeilen gegenüber der GPL: NSS1 OFDM 54M_24M, NSS1 MCS7_4,
NSS1 MCS11_8, NSS2 MCS7_4 und NSS2 MCS11_8. AP_5G hat zusätzlich einen
anderen Kommentar im Dateikopf. Beispielsweise lautet NSS1 MCS11_8 in der
GPL `15 15 17 18`, im OEM `17 18 19 19`. Diese Tabellenwerte sind ohne
Parser-/Skalierungsabgleich keine Aussage über gemessene EIRP.

PHY_REG, PHY_REG_GAIN, RadioA/B, TXPWR_TrackTSSI sowie die normalen und
RU-Leistungsgrenzen stimmen einschließlich AP_5G mit der GPL überein.
Die kleinen AP_5G/PHY_REG*, RadioA/B enthalten nur Kommentare und den
Endmarker `0xffff 0xffff`; sie sind keine vollständige zweite RF-Initialisierung.
Die geladenen Basistabellen bleiben erforderlich.

Prüfsummen aller Dateien und des Archivs:
[oem-rfe50-comparison.json](oem-rfe50-comparison.json).
Vollständige Unterschiede: [oem-rfe50-byrate.diff](oem-rfe50-byrate.diff).
Die OEM-Dateien selbst werden hier nicht als neues Firmwarepaket installiert.

## Bedeutung für die Portierung

Die Parameter passen weitgehend exakt zu dem vorhandenen G6-Quellstand.
RFE50 ist damit durch die OEM-Dateien und deren Laden zur Laufzeit belegt.
Es handelt sich um RTL8832BR-Parameter; die zusätzlich vorhandenen AP_2G-
und PON-Verzeichnisse belegen keine Nutzung durch den integrierten
rtl8192cd/8197G-Funkblock des WR1500. Im GPL-HAL
`phl/hal_g6/rtl8192xb/rtl8192xb_halinit.c` erhält CHIP_WIFI6_8832BR explizit
5-GHz-Unterstützung und 20/40/80 MHz.

Die Dateiparser und die Auswahl des Zusatzverzeichnisses existieren bereits
in `phl/hal_g6/hal_ld_file.c` und `phl/phl_init.c`. Ein späteres Paket muss
die Grundtabellen und den bestätigten AP_5G-Satz mit korrekter Fehlerbehandlung
bereitstellen. Die OEM-ByRate-Abweichungen müssen als Boarddaten erhalten
und zusammen mit eFuse, Länderauswahl, TSSI und den Leistungsgrenzen geprüft
werden; sie sind keine Freigabe, Leistungsgrenzen zu erhöhen oder abzuschalten.

Das Archiv enthält weder den ausführbaren Treiber noch den Firmwarecode,
noch einen vollständigen gerätespezifischen eFuse-/Factory-Datensatz.
Die bestehenden Linux-6.6-DMA-/cfg80211-Portierungsaufgaben bleiben offen.
Aus diesem Vergleich folgt kein erfolgreicher WLAN-Start unter OpenWrt.
