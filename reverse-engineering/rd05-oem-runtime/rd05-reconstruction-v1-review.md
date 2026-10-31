# RD05_Rekonstruktion V1: unabhängige Einordnung

Prüfung am 2026-10-31 des Uploads `rd05_rekonstruiert1.zip`, zusammengesetzt
aus drei Teilen. Archiv-SHA256:
`fc9fd3f5b81f356ad464572befb084064999687dc397d30c9e4853d47fa0215a`.
Die [maschinell erfassten Ergebnisse](rd05-reconstruction-v1-review.json)
nennen Größen, Grenzen und die Hashes der ausgewählten OEM-Funktionen.

## Mehrwert gegenüber der bisherigen Sammlung

Das Paket ordnet Funktionen und Daten gezielt dem RD05-OEM-Kernel 1.0.34
zu. Es ist damit für Rückentwicklung hilfreicher als eine Sammlung von
generischen SDK-Gerüsten. Der Kernel selbst ist nicht neu: Die enthaltenen
9.780.680 Bytes sind bytegleich mit unserem archivierten
`reverse-engineering/oem-kernels/rd05/kernel.raw.xz` nach Dekompression.

| Unabhängig geprüfter Gegenstand | Ergebnis |
| --- | --- |
| ZIP-Inhalt | Alle 17.430 Dateieinträge entpackt, ohne CRC-Fehler; 284.727.012 Bytes Nutzdaten |
| CONTENTS_SHA256SUMS | Alle 17.429 aufgeführten Dateien stimmen |
| Funktionsbereiche | Alle 7.073 angegebenen Intervall-Hashes passen zum Kernel |
| Datenzuordnungen | Alle 72 Einträge passen nach Größe, Hash und den angegebenen Kerneloffsets; 43 unterschiedliche Nutzlasten |
| GPL-Referenzen | Alle 611 Dateien bytegleich mit der bereits untersuchten AX1500-RTL8197-GPL |
| WLAN-Referenzport | 2.577 von 2.578 Dateien bytegleich mit unserem aktuellen Arbeitsbaum; eine abweichend, keine fehlend |

Der Arbeitsbaum enthält bereits andere, nicht durch diese Prüfung erzeugte
Änderungen. Die eine WLAN-Abweichung belegt deshalb keine neue Implementierung
im Upload. Referenzdateien nicht über den aktuellen Treiber kopieren.

## Konkrete Switch-Erkenntnisse

`dal_rtl8367d_switch_init` bei **0x80315670** ist in diesem Dump tatsächlich
acht Bytes lang:

```text
80315670: 03e00008  jr ra
80315674: 00001025  move v0, zero
```

Die zweite Instruktion ist der Delay-Slot des Rücksprungs. Die Funktion
gibt somit null zurück und schreibt keine Switchregister. Dies wurde anhand
der Kernelbytes geprüft, nicht nur aus dem generierten C übernommen. Auch
die mitgelieferte bekannte GPL-Funktion liefert lediglich `RT_ERR_OK`.
Eine vermeintlich fehlende Registertabelle in genau dieser Funktion erklärt
den LAN-/WAN-Ausfall damit nicht. Andere Initialisierung und Bootloader-
Vorbelegung sind dadurch keineswegs überflüssig.

`rtk_switch_init` bei **0x802ffa14** ruft `hal_find_device` und
`dal_mgmt_attachDevice` auf und führt anschließend einen indirekten
Backend-Aufruf aus. Ein leeres D-Backend darf nicht mit einem leeren
gesamten Initialisierungspfad gleichgesetzt werden.

Im geprüften Disassembly von `RTL83XX_init_switch_mode` bei **0x802f7fa0**:

* Chiptyp 4 wählt den logischen SDK-CPU-Port 17 über die globale Variable
  0x808520d8. Das ist keine externe Buchsennummer.
* Vor dem Force-Link-Aufruf wird `rtk_port_rgmiiDelayExt_set(cpu_port,0,2)`
  aufgerufen. Das sind API-Argumente; die spätere OEM-Laufzeitmessung RX=5
  wird dadurch nicht widerlegt. Eine direkte Änderung unseres Defaults auf
  RX=2 folgt daraus nicht.
* CPU-Tagging wird aktiviert und `rtk_cpu_tagPort_set(cpu_port,2)` aufgerufen.
  Der Zahlenwert darf nicht ungeprüft als DSA-Tag-Länge interpretiert werden.

Diese Pfade bestätigen und präzisieren die bereits vorhandene OEM-Analyse.
Keine daraus abgeleitete Änderung an MW5 oder RD05-Hardwareeinstellungen.

## Was die C-Dateien bedeuten

`lifted_c/` enthält 7.073 maschinennahe Übersetzungen mit einem künstlichen
Register-/Speichermodell. Das ist keine Wiederherstellung von Linux-Typen,
DMA-Ownership, Locking, Cachekohärenz oder einer funktionierenden Treiber-ABI.
Dass jedes Instruktionswort einem Decoderfall zugeordnet wurde, beweist
nicht dessen korrekte Semantik.

`reconstructed_c/` enthält laut Paket 19 lesbar rekonstruierte Funktionen:
12 Registerhelfer, Probe, den leeren D-Init-Pfad und fünf WLAN-Helfer.
Die vollständige TX-/RX-/DMA-Implementierung ist damit nicht rekonstruiert.
Die 6.250 Referenz-Namenszuordnungen sind keine 6.250 bewiesenen identischen
Implementierungen. Das Paket selbst kennzeichnet diese Grenzen korrekt.

Der 77.264-Byte-Firmwareblob bei **0x808a5c10** passt zu mehreren bereits
vorhandenen Namen (`rtl8812FfwMP`, `rtl8822BfwMP`, `rtl8822CfwMP`). Das ist
eine bestätigte Byteidentität, kein Nachweis für einen bestimmten aktiven
Funkchip oder für die Auswahl einer bestimmten RFE-/Kalibriertabelle.

## Konsequenz

Für die weitere Arbeit gezielt `New_swNic_init`, die tatsächlich aufgerufenen
TX-/RX-Pfade und ihre Cache-/Descriptor-Operationen mit unserem Treiber und
den OEM-Laufzeitdaten vergleichen. Die vorhandenen CPU-Port-FCS-Fehler sind
durch diese Zuordnungen allein noch nicht erklärt. Der vorbereitete
[RD05-TX-Puffervergleich](../../target/linux/realtek/rtl8197f/RD05_TX_DMA_AB.md)
bleibt ein unabhängiger Hardwaretest dieser Hypothese.

Hier wurden Hashes, Datenbereiche und ausgewählte Kontrollflüsse geprüft.
Die im Paket berichteten Syntax- und Differentialtests wurden nicht erneut
ausgeführt; ein Vergleich zweier dort erzeugter Modelle ist außerdem kein
unabhängiger Hardwarebeweis. Keine hochgeladenen Programme ausgeführt,
kein Kernel-/Firmware-Build, kein Routertest. Die vollständigen Rohdateien
bleiben im lokalen Analyseverzeichnis; im Tree stehen Bewertung und Belege.
