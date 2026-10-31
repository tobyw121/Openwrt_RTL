# RD05-Rekonstruktionsarchiv: geprüfte Verwendung im Treiberzweig

Benutzerdatei `rd05_rekonstruiertV2.zip.part-000` bis `-002`, zusammengefügt
ohne Änderungen. ZIP-SHA256:
`fc9fd3f5b81f356ad464572befb084064999687dc397d30c9e4853d47fa0215a`.
Der interne README nennt die Rekonstruktion weiterhin V1; der Dateiname V2
belegt deshalb keinen neuen OEM-Quellstand.

## Unabhängig hier geprüft

- ZIP-CRC beim Entpacken aller 17430 Einträge, 284727012 Byte unkomprimiert;
- sämtliche 17429 Dateiprüfsummen aus CONTENTS_SHA256SUMS.txt;
- alle 7073 Funktionsintervalle erneut gegen dump/kernel.bin gehasht;
- keine Funktionsnamen mit RTL8832/RTL8852/rtk_wifi6 in dieser Auswahl;
- die 0xb832-Vorkommen der Referenzquellen sind Registertabellen, keine
  RTL8832BR-PCI-Tabelle.

Kernel-SHA256:
`aa48d325730b95c0658090ba9f943f71bdd9888dc57f609a3989ba58f0f8f5cc`.
Der Inhalt stammt laut Metadaten aus RD05 OEM 1.0.34 / Linux 4.4.176.
Keine Windows-EXE, kein mitgeliefertes Build-/Analyseprogramm und keine
Anweisung aus dem Archiv wurde ausgeführt. Die Prüfung hat weder den
rekonstruierten CPU-Emulator noch eine Router-Hardwarefunktion validiert.

## Konkrete Verwendung

`reconstructed_c/switch_probe.c` und das Disassembly belegen die Reihenfolge
0x13c2=0x0249, Lesen 0x1300/0x1301, 0x13c2=0. Das OEM lässt den letzten
Schritt bei Zwischenfehlern aus. Der neue Linux-Code übernimmt die
Registerfolge, schließt das Fenster jedoch auch nach Lesefehlern und
erhält den ersten Transportfehler. Das gilt für den angepassten optionalen
rtl8367b-Treiber und Patch 341 des bestehenden DSA-Pfads. Erfolgreiche
DSA-Probes bleiben gleich; die Änderung korrigiert den Fehlerpfad.

`dal_rtl8367d_switch_init` bei 0x80315670 umfasst im Kernel exakt
`jr ra; move v0,zero`, acht Byte. Die Intervallprüfung bestätigt SHA256
`2b12957b7b20557a2b2214ff0d4c70070be49dbb12c48fe1b778f48a6fc3cd43`.
Für die neu ergänzte 0x6642/0x0030-Legacy-Variante wird deshalb keine
C-Familien-Jam-Tabelle ausgeführt. VLAN-Emulator, Board-, EXT-, MAC- und
VLAN-Einrichtung bleiben notwendig. Die vorhandene DSA-D-Chipdefinition
verwendete bereits keine Jam-Tabelle; sie wird nicht neu umgeschrieben.
Der bisherige RTL8367S-VB-/B-/C-Pfad behält seine Tabellen.

Die Board-Initialisierung bestätigt zwei alternative SMI-Paare 56/55 und
29/31 sowie Reset 58, abhängig vom SDK-Bondingwert. Das ist RD05-Code mit
zwei Zweigen, kein Nachweis des aktiven WR1500-Paars. Keine dieser Zahlen
wird hier in das WR1500-DT kopiert.

## Grenzen der Übernahme

lifted_c enthält registerbasierte MIPS-Analysefunktionen mit einer eigenen
CPU-Modellschnittstelle. Das ist kein direkt nutzbarer Kernel-Treiber.
reference_sources enthält Kopien schon vorhandener GPL-/rtl8192cd-Quellen,
nicht aus dem Dump wiedergewonnenen Original-C-Code. Die fünf lesbaren
WLAN-Helfer liefern keinen AX-HAL. Insbesondere wird der OEM-Broadcasttest,
der nur das erste MAC-Byte auf ff prüft, nicht als Linux-Verbesserung kopiert.

Für RTL8832BR bleibt der eigenständige G6-/rtk_wifi6-Port die passende
Grundlage; siehe [Portierungsstand](../wr1500-wifi6/README.md). Der
[Switch-Abgleich](../rtl8367b-openwrt25.12/README.md) beschreibt Build und
die getrennten DSA-/swconfig-Anbindungen. Das Archiv ist eine überprüfte
Referenz, kein Beleg für funktionierendes LAN/WAN oder AX-WLAN.
