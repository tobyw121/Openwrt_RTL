# Prüfung des rekonstruiert.7z-Pakets

Stand: 2026-10-31. Bezug: MW5_Stable, Ausgangscommit efa9274cf6.
Die zwei hochgeladenen Teile wurden zusammengefügt und mit libarchive
vollständig entpackt: 12.759 Dateien, 754.411.475 Bytes Nutzdaten.
Archiv-SHA256 und Einzelvergleiche stehen in
[reconstructed-archive-review.json](reconstructed-archive-review.json).
Enthaltene Programme, Makefiles und Skripte wurden nicht ausgeführt.

## Ergebnis für den RD05

Das Paket ist als Referenzsammlung brauchbar. Es belegt noch keine Lösung
für den ausfallenden Ethernet-Datenpfad. Die originalen SDK-Quellen sind
wertvoller als die generierten Treibergerüste. Mehrere zentrale Dateien
sind mit der bereits untersuchten GPL identisch.

| Bestandteil | Nachprüfung und Nutzen |
| --- | --- |
| Blob-Paket | Alle 848 Einträge der enthaltenen SHA256SUMS stimmen. Dies prüft interne Konsistenz, nicht die Herkunft. |
| 18 Binärblobs | Alle 18 zugehörigen C-Arrays ergeben exakt die enthaltenen Binärdateien und stimmen mit Größe/Hash des Manifests überein. |
| Firmware-Neuheit | 11 von 18 haben bereits mindestens eine bytegleiche Datei im WLAN-Paket unseres Trees. Die übrigen sind Vergleichskandidaten, keine automatisch geeigneten Upgrades. |
| 148 Texttabellen | 130 haben bereits eine bytegleiche Datei im WLAN-Paket. Die übrigen müssen nach Chip, RFE und Tabellenformat geprüft werden. |
| RTL8367D-DAL | Originale Register-/Portfunktionen für gezielte Vergleiche vorhanden; keine vollständige RTL8367D-Kaltstart-/Jam-Tabelle im rekonstruierten Blob-Paket nachgewiesen. |
| Funktionskataloge/Disassembly | Einstiegspunkte und Vergleichsmaterial. Katalogeinträge sind keine implementierten Funktionen. |
| Generierte NIC-/Switch-Gerüste | Wesentliche Funktionen fehlen; kein fertiger Linux-6.6-Treiber. |

## Konkreter Quellenvergleich

Basis für den GPL-Vergleich ist das bereits lokal untersuchte
`GPL_for_AX1500_rtl8197/openwrt/21.02/target/linux/realtek/files-4.4/drivers/net/rtl819x`.
Die folgenden Pfade beziehen sich auf die Wurzel des hochgeladenen Pakets.

* `02_RTL8197F_SoC_NIC/combined_vendor_reference/MR62X_SDK_Paket/selected_sources/sdk/openwrt-21.02/target/linux/rtknet/drivers/net/rtl819x/rtl819x_swNic.c`:
  Gegenüber der bekannten GPL genau ein zusätzlicher Block mit 31 Zeilen,
  durch `#if 0` deaktivierte TX-Debugausgabe. Keine neue aktive DMA-Logik.
* `rtl865xc_swNic.c` im selben Verzeichnis ist bytegleich mit der bekannten
  GPL (SHA256 `d4eb0a0e8e01130b551a92f0691fac47401b90393bc1cde7b46e3b84e9f424d3`).
* Die AX12v1-Kopien von `dal_rtl8367d_port.c` und
  `dal_rtl8367d_switch.c` sind ebenfalls bytegleich mit der bekannten GPL.
  Neun enthaltene Kopien je Dateiname entsprechen nur drei verschiedenen
  Inhalten: Kopien aus Kernel, Bootcode und mehreren Paketordnern dürfen
  nicht als unabhängige Bestätigungen gezählt werden.
* MR62X `dal_rtl8367d_port.c`, Zeilen 3170–3255: RGMII-TX-Delay ist Bit 3,
  RX-Delay sind Bits 2:0 in EXT1_RGMXF. Der Kommentar verlangt Delay-Setup
  vor `rtk_port_macForceLinkExt_set()`. Das bestätigt unsere bereits
  eingeführte Reihenfolge. Neue SerDes-/Fiber-/2,5-Gbit-Tabellen in dieser
  Variante sind kein Beleg für ihre Anwendung auf dem RD05-RGMII-Link.

## Grenzen der generierten C-Rekonstruktion

`all_most_SDKS_c_reconstruction/src/ethernet/rtl865x_swnic_rewrite.c`:
`open()` setzt lediglich den Softwarezustand; `poll_rx()` gibt unabhängig
vom Budget null zurück; `clean_tx()` meldet Erfolg ohne Ringbereinigung.
Damit ist weder Empfang noch vollständige TX-Verarbeitung implementiert.

`all_most_SDKS_c_reconstruction/src/switch/rtl8367_rewrite.c`:
Default CPU-Port 6 und Tag-Protokoll 9 sind kein RD05-Profil. Unser RD05
verwendet CPU-Port 7 und den rtl8_4-Pfad. Portfreigabe, PVID und CPU-Tag-
Konfiguration liefern in diesem Gerüst `SDK_RE_ENOSYS`. Diese Funktionen
dürfen nicht als funktionierende Switch-Implementierung importiert werden.

Die im Paket beworbenen 25 RTL8367R/RB-ChipData-Tabellen sind keine
nachgewiesene RTL8367D-Jam-Tabelle. Ebenso fehlt dort laut Paketbericht
ein vollständiger eigenständiger RTL8363NB-Treibersatz. Tabellen und
Firmware verschiedener Chipvarianten ersetzen keine Boardkalibrierung.

## Konsequenz für unseren Tree

Keine Änderung von DMA, Timing, CPU-Tags oder Firmware allein aus diesem
Paket ableiten. Die aktuelle RD05-TX-Coherent-A/B/A-Diagnose bleibt der
nächste Hardwarevergleich; siehe
[RD05_TX_DMA_AB.md](../../target/linux/realtek/rtl8197f/RD05_TX_DMA_AB.md).
Die dortigen Vorher-/Nachher-Zähler sollen zeigen, ob die CPU-Port-FCS-
Fehler vom TX-Puffer-/Cachepfad abhängen. Dieser Archivvergleich ersetzt
den Test nicht. MW5-Hardwareeinstellungen bleiben unverändert.

Geprüft wurden Archivlesbarkeit, Prüfsummen, Blob-Arrays und die genannten
Quellpfade. Keine Aussage über vollständige semantische Prüfung aller
12.759 Dateien, Hardwarefunktion oder erfolgreichen Firmware-Build.
Die umfangreichen Rohdaten bleiben im lokalen Analyseverzeichnis;
im Repository werden Bewertung und Prüfergebnisse festgehalten.
