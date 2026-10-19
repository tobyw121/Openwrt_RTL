# Weitere Rückentwicklung der OEM-Switch-Treiber

Stand: 20.10.2026. Grundlage sind dieselben drei SPI-Dumps aus RT8197F wie
in [OEM_DUMP_INTEGRATION.md](OEM_DUMP_INTEGRATION.md). Die Originale blieben
unverändert. Routerprogramme wurden nicht ausgeführt.

## Symbolnamen und Adressen wiederhergestellt

`scripts/rtl8197f-oem-kallsyms.py` rekonstruiert die komprimierten Symbolnamen
mit ihren absoluten Adressen aus den entpackten MIPS-Kernels. Es prüft
Adressreihenfolge, Symbolanzahl, die vollständigen Namen-/Tokenströme und
sämtliche 256-Symbol-Marker. Wo Daten-Symbole verfügbar sind, prüft es
zusätzlich die Exportdatensätze gegen Funktions- und Namenszeiger.

| OEM-Kernel | rekonstruierte Symbole | unabhängig geprüfte Exporte |
| --- | ---: | ---: |
| MW5, Linux 3.10.90 | 29.629 | 3.672 |
| AC23, Linux 3.10.90 | 31.553 | 3.803 |
| RD05, Linux 4.4.176 | 24.127 | 0 |

Beim RD05 standen für diesen zusätzlichen Exportabgleich keine passenden
Datensymbole zur Verfügung. Die vollständige Kallsyms-Strukturprüfung
bestand auch dort. Der Decoder unterstützt die absolute Little-Endian-
MIPS32-Darstellung dieser OEM-Kernels, keine beliebigen neuen Kernel-
Kallsyms-Formate und keine Binärdateien ohne erhaltene Symboltabelle.

Reproduzierbarer Aufruf mit einem bereits entpackten Kernel:

```sh
python3 scripts/rtl8197f-oem-kallsyms.py /pfad/mw5-kernel.raw \
  --output /tmp/mw5-switch-symbols.json
```

Der Helfer überschreibt keine bestehende Ausgabedatei. Standardmäßig gibt er
nur ausgewählte Symbolmetadaten aus. `--all-symbols` speichert alle Namen und
Adressen. Die maschinenlesbaren Ergebnisse für die relevanten Funktionen,
Kernel-Hashes und die nachfolgenden Registerbelege liegen in
`target/linux/realtek/rtl8197f/oem-switch-evidence.json`. Keine Zugangsdaten,
individuellen Factory-Werte oder vollständigen OEM-Binärdateien sind enthalten.

## MW5: tatsächliche Linkabfrage und Fortschrittskontrolle

Der Körper von `switch_phy_status_get` bei `0x80190e68` liest lediglich das
Bit der gewünschten Portnummer aus `curLinkPortMask` bei `0x806b0fdc`.
Die Funktion selbst führt **keinen** SMI-/MDIO-Zugriff aus.

Der Ablauf im OEM-Kernel lässt sich jetzt weiter verfolgen:

```text
one_sec_timer
  → rtl_get_8367r_port_link_status
      → rtk_port_macStatus_get für Ports 0..4
          → logischen Port auf physischen Port abbilden
          → rtl8367c_getAsicPortStatus
              → externes Statusregister 0x1352 + physischer Port lesen
  → Linkmaske speichern
  → rtl_check_swCore_tx_hang
      → interne SwitchCore-Ausgabezähler auswerten
      → New_check_tx_done_desc_swCore_own heranziehen
  → check_link_change
      → PHY-Reg. 1 abfragen und Änderungen zeitlich erfassen
  → Timer erneut planen

phy_check.ko
  → switch_phy_status_get
      → bereits gespeicherte Linkmaske auswerten
  → Link-Ereignisse melden
  → bei Link down dynamische RTL865x-Hardware-L2-Einträge leeren
```

Das ist eine Teilrekonstruktion der Aufruf- und Datenwege. Die unten
beschriebene Deskriptorprüfung ist inzwischen genauer rekonstruiert; nicht
alle Bedingungen und Folgen der OEM-Reinitialisierung sind geprüft.
Insbesondere wird aus dem Funktionsnamen `one_sec_timer` keine ungeprüfte
reale Zeitbasis abgeleitet.

Die OEM-Firmware trennt Hardwarepolling, gecachte Userspace-Meldung und
TX-Fortschrittskontrolle. Ein weiterlaufender Watchdog oder das Lesen eines
gecachten proc-Werts beweist keine funktionierende Paketübertragung.
Der bestehende unabhängige Linux6-DMA/NAPI-Watchdog bleibt erhalten.

### Erweiterung: stehenden MW5-TX-Ring erneut anstoßen

`New_check_tx_done_desc_swCore_own` bei `0x801a5e98` lädt den Software-
Done-Index aus `0x80a4e1c8` und den Ringzeiger aus `0x80a4e1a8`. Die
Adressrechnung `(index << 5) - (index << 3)` ergibt 24 Byte pro Deskriptor.
Nach Wechsel auf den uncached Alias liest die Funktion Wort 0 und testet
**Bit 0 (OWN)**. Bei gesetztem OWN speichert sie den Done-Index und liefert
0, andernfalls -1. Sie gibt selbst keinen Puffer frei und prüft hier nicht,
ob Done- und Current-Index gleich sind.

AC23 besitzt bei `0x801a091c` ebenfalls 24-Byte-Deskriptoren, RD05 bei
`0x802b9ad4` dagegen 32-Byte-Deskriptoren. Diese beiden Prüffunktionen
betrachten zusätzlich `done != current`. Die Funktionswörter, Adressen und
Hashes sind im Abschnitt `tx_progress_evidence` des JSON-Belegs festgehalten.
Ein gleichnamiger OEM-Helfer bedeutet also keine identische Ringgeometrie.

Der MW5-Aufrufer vergleicht zuvor fünf interne 64-Bit-Ausgabezähler und
verfolgt bei ausbleibender Ausgabe den Done-Index über mehrere Prüfungen.
Die im Kernel initialisierten Variablen stehen auf `flag=1`, `interval=30`
und `threshold=6`. Das sind OEM-Prüfbedingungen/Zähler, keine als
Millisekunden übernommenen Konstanten. Der Timer plant sich mit
`jiffies + 100` erneut und verzweigt bei gemeldetem Hang nach
`rtl865x_reinitSwitchCore`. Dessen gesamte Nebenwirkungen sind nicht
validiert. Dieser vollständige Reset wird deshalb nicht in unseren
laufenden MW5-Datenpfad übernommen.

Der Linux6-Treiber ergänzt stattdessen eine begrenzte TX-Wiederanstoßung:

- Der vorhandene unabhängige Timer prüft auch einen einzelnen ausstehenden
  TX-Deskriptor. Eine volle/gestoppte Linux-Sendequeue ist keine Voraussetzung.
- Nur bei gültigem CDP innerhalb des ausstehenden Ringbereichs und weiterhin
  gesetztem OWN wird eine Beobachtung gestartet. CDP-Fortschritt oder eine
  Änderung des monotonen Freigabezählers beginnt das Intervall neu. Damit
  bleiben vollständige Ringumläufe zwischen zwei Stichproben erkennbar.
  Neue Sendungen und RX-Interrupts verdecken keinen stehenden TX-Tail.
- Nach mindestens fünf Sekunden ohne diesen Fortschritt werden TRXRDY und
  TXFD erneut gesetzt, frühestens weitere fünf Sekunden später erneut.
  Fünf Sekunden sind unsere Linux-Retry-Regel, keine rekonstruierte OEM-Zeit.
  Die tatsächliche Reaktion erfolgt beim nächsten Watchdog-Durchlauf.
- Leerer Ring, ungültiger/außerhalb liegender CDP oder gelöschtes OWN beenden
  diese Beobachtung. Nur **CDP vorgerückt und OWN gelöscht** erlaubt weiterhin
  das Planen der TX-Freigabe durch NAPI. Zeitablauf ersetzt diesen Beleg nicht.
- Der Timer ändert keine Deskriptoren, DMA-Zuordnungen, Basis-/Ringzeiger,
  RX-Zustände oder DSA-/L2-Tabellen. Der bestehende explizite TX-Timeout
  verwendet nun denselben TX-Lock für seinen Anstoß und beginnt anschließend
  ein neues Beobachtungsintervall. Öffnen/Schließen verwirft alte Beobachtungen.

`tx_watchdog_rekicks` ist über `ethtool -S` sichtbar. Die bestehende
Treiber-proc-Ansicht meldet zusätzlich
`tx_progress enabled=… stall_ms=5000 armed=… rekicks=…`.
Diese Anzeigen starten keine Recovery. AC23/RD05 erhalten durch diese
Erweiterung keinen neuen automatischen Recovery-Pfad. Die auf MW5 bereits
deaktivierten Softwarebeschleuniger bleiben deaktiviert.
`mw5-accel status` zeigt die neue `tx_progress`-Zeile ebenfalls an.

Ein erneuter Anstoß kann eine ausbleibende TX-Aktivierung beheben, garantiert
aber keine Heilung eines tieferen Hardware-/DMA-Stillstands. Der ursprüngliche
Netzausfall ist damit weiterhin erst auf dem Router unter Last nachzuprüfen.

## MW5: Switch-Familie anhand der tatsächlichen ID

Das vorhandene MW5-Laufzeitlog meldet ID `0x6367`, Revision `0x0060`.
`rtk_switch_probe` bei `0x801fc824` ordnet `0x6367` dem RTL8367C-Selektor 0
zu. Das ist die für diesen Logbefund relevante Init-Verzweigung. Die
physische Bestückungsangabe RTL8363NB wird dadurch nicht in eine andere
Beschriftung umgedeutet.

Andere Zweige sind getrennt zu halten: `0x0801`/`0x6511` werden abhängig von
den Revisionsbits als RTL8364B oder RTL8363SC-VB behandelt. Selektor 1 gehört
zum RTL8370B-Zweig. Dort befindet sich `PatchChipData`, eine Tabelle mit
210 Register-/Wertpaaren. MW5- und AC23-Kernel enthalten dieselbe Tabelle;
Hash und Bereich sind dokumentiert, vollständige lokale Extraktionen wurden
separat aufbewahrt. Diese Tabelle wird **nicht** für den geloggten
MW5-Selektor 0 programmiert.

Damit ist die pauschale Suche nach einer unbekannten RTL8363NB-Tabelle für
den konkreten `0x6367/0x0060`-Logbefund präzisiert: Die OEM-Erkennung und der
relevante Common-Init-Zweig sind nun sichtbar. Das beweist weiterhin keine
vollständige Kaltstart-Parität unseres Ports. Reset, Boardnachlauf,
Analogeinstellungen und Plattformzustand müssen zusammen geprüft werden.

## Nachgewiesene externe Flow-Control-Sequenz

`RTL83XX_init` bei `0x801d8bc0` setzt zunächst den externen Switch zurück,
wartet, ruft `rtk_switch_init` auf und konfiguriert CPU-Anbindung und Board.
Nach `0x1b03 = 0x0936` folgen acht direkte Aufrufe von
`rtl8367c_setAsicReg`. Deren Transport ist nachweislich `smi_write`.
Die Argumente stehen jeweils in `a0` sowie im MIPS-Delay-Slot in `a1`.
Die jeweiligen Instruktionswörter und Call-Adressen sind im JSON-Beleg.

| Register | bisheriges LOG_MW5 | OEM-Wert | SDK-Feld |
| --- | --- | --- | --- |
| 0x121f | 0x01d6 = 470 | 0x01d0 = 464 | System ON |
| 0x1220 | 0x01b8 = 440 | 0x01ac = 428 | System OFF |
| 0x1221 | 0x01cc = 460 | 0x019e = 414 | Shared ON |
| 0x1222 | 0x01ae = 430 | 0x017a = 378 | Shared OFF |
| 0x1223 | 0x01d6 = 470 | 0x01d0 = 464 | FCOFF System ON |
| 0x1224 | 0x01b8 = 440 | 0x01ac = 428 | FCOFF System OFF |
| 0x1225 | 0x01cc = 460 | 0x019e = 414 | FCOFF Shared ON |
| 0x1226 | 0x01ae = 430 | 0x017a = 378 | FCOFF Shared OFF |

Die wiederholten Register-Snapshots im vorhandenen LOG_MW5 stimmen jeweils
überein und unterscheiden sich an allen acht Stellen vom OEM-Binärcode.
Damit ist eine konkrete Initialisierungsdifferenz belegt. Das Log ist keine
neue Messung mit Patch 332 und beweist keine Ursache des Netzstillstands.

Die Bedeutung der Register wird zusätzlich durch
`8197/API_Source/rtl8367c_reg.h` und `rtl8367c_asicdrv_fc.c` im Referenzbranch
bestätigt. Es handelt sich um Seitenschwellen des **externen** Switches.
Die ON-/OFF-Abstände bilden eine Hysterese. Sie sind nicht mit den früher
aus `/proc/rtl865x` erfassten internen SwitchCore-Parametern gleichzusetzen.

Patch `332-net-dsa-realtek-mw5-restore-oem-flow-control.patch` ergänzt diese
acht Werte mit drei Voraussetzungen: MW5-OEM-Init-Property, ID `0x6367`
und Revision `0x0060`. Die Reihenfolge entspricht dem OEM: RGMII-Register,
Flow-Control-Schwellen, danach die bereits vorhandenen SSC-Schreibzugriffe.
Der erste regmap-Fehler wird zurückgegeben. Andere Boards/Revisionen erhalten
keine zusätzlichen Flow-Control-Schreibzugriffe.

Dieser Befund ersetzt die bisherige Zurückhaltung für genau diesen externen
Block, dessen Herkunft zuvor nicht belegt war. Weitere SDK-Common-Einstellungen
wie alte Paketlängen-, ACL- oder Unknown-DA-Vorgaben werden nicht pauschal
kopiert; ihre Wirkung muss zum Linux-DSA-/Bridge-Modell passen.

## AC23 und RD05

Beim AC23 ist `switch_phy_status_get` ebenfalls ein Cache-Leser, hier mit
einem gespeicherten Wert pro Port. Sein OEM-Kernel enthält ähnliche
`RTL83XX_init`-/Common-Init-Wege. Der vorhandene MW5-Laufzeitnachweis für
Silizium-ID und Boardparameter gilt nicht automatisch für AC23. Deshalb
übernimmt der neue Patch keine MW5-Flow-Control-Werte auf AC23.

RD05 verwendet eine andere Init-Architektur:
`rtk_switch_init` → `hal_find_device` → `dal_mgmt_attachDevice` →
chipbezogener Funktionszeiger. Die alte direkte Initialisierung des MW5 ist
dafür keine passende Vorlage. Weitere DAL-/RTL8367D-Funktionen sind jetzt
gezielt adressierbar, aber noch nicht vollständig rekonstruiert.

## Nachweise und weitere Hardwareprüfung

- Decoder und Fehlerfälle werden mit synthetischen Kernel-Symboltabellen
  geprüft, einschließlich beschädigter Marker, Namen, Adressen und Exporte.
- Die tatsächliche gepatchte Flow-Control-Hilfsfunktion wird mit UBSan
  ausgeführt: acht Werte/Reihenfolge, Board-/ID-/Revisionssperren und
  Abbruch bei jedem möglichen regmap-Fehler.
- Die Realtek-Treiberteile aller sieben zugehörigen Repository-Patches
  werden auf die Linux-6.6.114-Testquelle angewendet. Der resultierende
  `rtl8365mb.o` kompiliert mit MIPS32R2, Little Endian und dem durch Patch 329
  angepassten DSA-Header. Andere DSA-Module sind damit nicht neu geprüft.
- Die gesamte Host-Regression umfasst jetzt 38 bestandene Tests. Die
  unveränderten WLAN-Varianten wurden in der vorigen Änderung bereits als
  MIPS-Objekte geprüft; sie wurden für diese Switch-Änderung nicht neu gebaut.

Für die TX-Erweiterung führt die Watchdog-Regression die tatsächlichen
C-Funktionen zusätzlich mit stehendem Einzelpaket, weiterlaufenden RX-IRQs,
neuen Sendungen, CDP-Fortschritt, kompletten Ringumläufen, fehlerhaftem CDP,
OWN/CDP-Widerspruch und überlaufenden jiffies aus. Sie prüft die
Fünf-Sekunden-Grenze und Wiederholungsbegrenzung, ausbleibende Ringänderungen
sowie die anschließende reguläre NAPI-Freigabe. OWN und Zeitkonstanten stammen
direkt aus dem Treiber. Das Ethernet-Objekt wird ebenfalls gegen den exakten
Linux-6.6.114-Kernel für MIPS32R2 Little Endian ohne WEXT gebaut.
Dabei wird der bereits im Repository vorhandene OpenWrt-Backport 703
(`flow_rule_has_control_flags`) auf eine separate Kopie von
`include/net/flow_offload.h` angewendet. Der Kernel-Testbaum bleibt unverändert.
Boardprofile, klassischer MW5-Routingpfad und der auf den aktuellen
coherent-DMA-/CLI-Stand angepasste MW5-Baseline-Prüfer bestehen ebenfalls.

Das ist kein vollständiger OpenWrt-Firmware-/MODPOST-Build und kein
Hardwaretest. Nach einem solchen Build und Flashen: Gerät vollständig vom
Strom trennen, Startmeldung `restored MW5 OEM external flow-control thresholds
(6367/0060)` kontrollieren und die acht Register in
`/proc/mw5-rtl8367-flowctrl` prüfen. Anschließend bidirektionale LAN/WAN-
und WLAN-Last ohne laufendes `mw5-netdiag full` testen. Erst diese Prüfung
kann zeigen, ob die fehlende Boardinitialisierung zum Netzstillstand beitrug.
