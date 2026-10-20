# Weitere Rückentwicklung der OEM-Switch-Treiber

Ergänzung: Die neu belegte EXT1-Maskenkorrektur (Patch 333) und der Abgleich
des RD05-Init-Stubs mit dem SDK sind in
[Portmodus und WLAN-Schlüsselpfade](reverse-engineering/oem-kernels/INTERFACE_FINDINGS.md)
beschrieben; reproduzierbare Instruktionsbelege stehen in
`target/linux/realtek/rtl8197f/oem-interface-evidence.json`.

Der anschließende Paketlängenabgleich ergänzt mit Patch 334 die explizite
Auswahl von Profil 0 für aktive RD05-/RTL8367D-Ports. OEM-Registerbelege,
SDK-Abgleich und Grenzen stehen in
[Kalibrierung und MTU-Profile](reverse-engineering/oem-kernels/CALIBRATION_AND_MTU_FINDINGS.md).

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

## Weitere Board-Init-Rückentwicklung: AC23 und RD05 unterscheiden

Die folgenden Ergänzungen sind in
`target/linux/realtek/rtl8197f/oem-board-init-evidence.json` mit Kernel-Hash,
Funktionsbereichen/-Hashes und ausgewählten MIPS-Instruktionen festgehalten.
Die 32-Einträge-L2P-Tabellen sind statische SDK-Portzuordnungen; sie enthalten
keine individuellen MAC-Adressen oder Kalibrierwerte. Sie beweisen keine
Beschriftung oder Belegung der kupfernen Gehäuseanschlüsse.

### Reproduzierbarer statischer Auditor

`scripts/rtl8197f-oem-switch-audit.py` verwendet zuerst den vollständigen
Kallsyms-Validator. Danach erfasst er direkte JAL-Aufrufstellen und deren
Ziele in ausgewählten Switch-Funktionen. Für bekannte Registerschreiber
und RGMII-Delay-Aufrufe dekodiert er ausschließlich unmittelbar benachbarte
konstante Argumente, einschließlich des MIPS-Delay-Slots. Vorzeichenbehaftetes
`addiu`, vorzeichenloses `ori` und echte Null-Moves werden unterschieden.
Unbekannte Loads oder über globale Zeiger geladene Argumente werden nicht
als Konstanten geraten. Ein exakt erkannter `jr ra; return 0`-Stub wird
zusätzlich gekennzeichnet.

```sh
python3 scripts/rtl8197f-oem-switch-audit.py /pfad/ac23-kernel.raw \
  --output /tmp/ac23-switch-audit.json
# Einzelne Textfunktion statt der Standardauswahl:
python3 scripts/rtl8197f-oem-switch-audit.py /pfad/rd05-kernel.raw \
  --symbol dal_rtl8367d_switch_init --output /tmp/rd05-dal-init.json
```

Die Ausgabe wird nicht überschrieben. Standardmäßig wurden auf den drei
Originalkernels 5 MW5-, 7 AC23- und 11 RD05-Funktionen geprüft. Der Auditor
ist kein Disassembler für beliebige MIPS-Varianten, kein vollständiger
Kontrollflussanalysator und keine Firmware-Ausführung. **Eine vorhandene
Aufrufstelle beweist weder ihre Erreichbarkeit noch einen erfolgreichen
Registerzugriff.** Gerade der nachfolgende RD05-Pfad zeigt diese Grenze.

### AC23: eigene Schwellen und abschließender RGMII-Wert

`RTL83XX_init` bei `0x801e0610` fordert folgende externe Werte an:

| Register | MW5 | AC23 |
| --- | --- | --- |
| 0x1b03 | 0x0936 | 0x0222 |
| 0x121f / 0x1220 | 464 / 428 | 470 / 440 |
| 0x1221 / 0x1222 | 414 / 378 | 460 / 430 |
| 0x1223 / 0x1224 | 464 / 428 | 770 / 740 |
| 0x1225 / 0x1226 | 414 / 378 | 720 / 680 |

Damit ist ein pauschales Übertragen der acht MW5-Schwellen auf AC23
widerlegt. AC23-SSC, PHY-Patch und Flow-Control werden in dieser Änderung
nicht zusätzlich programmiert; eine live bestätigte Switch-ID/-Revision
und die Einordnung in unseren DSA-Pfad fehlen weiterhin.

Der AC23 setzt den logischen externen Port 16 zunächst bei `0x801e06ac`
auf TX0/RX2. Nach dem SSC-Aufruf bei `0x801e0750` folgt bei `0x801e0760`
ein weiterer Delay-Aufruf mit **TX0/RX5**. Der MW5 besitzt dieselbe
zweistufige Delay-Anforderung, aber eine andere Board-Registersequenz.

Der untersuchte C-Familien-HAL-Block liegt beim AC23 bei `0x806431c0`,
beim MW5 bei `0x805811b0`. Die SDK-L2P-Funktion liest das Wort an
`hal + 4 + logical_port * 4`: Port 16 wird auf physisch 6 abgebildet,
Port 17 auf physisch 7. Das gilt für diese ausgewählte Familientabelle;
es ersetzt keinen Laufzeitnachweis der konkreten AC23-Siliziumvariante.

Die Delay-Hilfsfunktion wählt für logischen Port 16 Register `0x1307` und
ändert nur dessen unteres Nibble: TX in Bit 3, RX in Bit 2..0. Der Linux-
DSA-Treiber verwendet 300-ps-Schritte für RX. Der AC23-Device-Tree enthielt
bislang den temporären Wert RX2 = 600 ps. Er fordert nun den abschließenden
OEM-Wert **RX5 = 1500 ps** an; TX bleibt 0. Die Boardprofile und deren
Validator halten diesen Sollwert fest. Das ist eine belegte Korrektur der
angeforderten Verzögerung, keine nachgewiesene Durchsatzverbesserung oder
vollständige Übernahme der vorausgehenden OEM-SSC-Konfiguration.

### RD05: Typ 4, Port 7 und ein Init-Erfolg ohne ASIC-Initialisierung

Die Zeigerkette ist nun bis zum tatsächlichen Ziel nachvollziehbar:

```text
switch_probe: ID 0x6642 → Chiptyp 4
hal_find_device → Typ-4-HAL bei 0x808520e0
dal_mgmt_attachDevice → dal_rtl8367d_mapper_get
  → Mapper bei 0x80852c40
rtk_switch_init → Mapper-Slot 0
  → dal_rtl8367d_switch_init bei 0x80315670
      jr ra
      move v0, zero  (Delay-Slot)
```

Dieser konkrete Init-Einstieg ist nur acht Byte lang, hat keine Aufrufe
und keine Registerzugriffe und liefert sofort Erfolg. Somit beweist ein
erfolgreiches OEM-`rtk_switch_init()` auf diesem RD05 **keine vollständige
ASIC-Kaltinitialisierung**. Ob Bootloader oder andere OEM-Pfade einen
vorausgesetzten Zustand herstellen, ist noch nicht vollständig geklärt.
Unser DSA-Treiber darf seine notwendigen Initialisierungen deshalb nicht
durch diesen Stub ersetzen.

Für Chiptyp 4 verzweigt `RTL83XX_init` bei `0x802f7a14` nach
`0x802f7ce4` und überschreibt den Board-Port auf logisch **17**. Die
Typ-4-L2P-Tabelle ordnet diesen physisch **7** zu. Das bestätigt unseren
RD05-CPU-Port 7; der anfängliche globale Wert 16 darf nicht isoliert als
CPU-Port 6 interpretiert werden. Auch hier werden TX0/RX2 vor SSC und
TX0/RX5 danach angefordert. Die Portnummer wird aus dem überschriebenen
globalen Wert geladen; der Auditor gibt sie deshalb nicht als lokale
Konstante aus. Die betreffende Instruktionsfolge ist separat belegt.

Besonders relevant ist der spätere Branch bei `0x802f7bcc`: Für Chiptyp 4
kehrt die Boardfunktion nach Duplex-/Downspeed-Konfiguration zum Epilog
`0x802f79b8` zurück. Die ebenfalls im Funktionskörper vorhandenen acht
C-Familien-Flow-Control-Schreiber und der nachfolgende `rtk_phyPatch_set`
werden auf diesem normalen Typ-4-Pfad **übersprungen**. Der statische
Auditor findet diese acht Stellen trotzdem. Sie sind kein Beleg dafür,
dass der RD05-D-Switch dieselben Schwellen erhalten muss wie AC23.

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
- Die gesamte Host-Regression umfasst jetzt 42 bestandene Tests. Die
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

Die vier neuen Auditor-Tests prüfen echte MIPS-Instruktionskodierungen:
beide Argumentreihenfolgen, Delay-Slots, drei RGMII-Argumente, Sign Extension,
unbekannte Loads, exakte Erfolg-Stubs sowie ungültige oder abgeschnittene
Funktionsbereiche. Der geänderte AC23-DTS wurde zu einem DTB kompiliert und
zurückgelesen: CPU-Port 6 enthält TX=0 und RX=1500 ps. Die bereits bestehenden
DTC-Warnungen zu doppelter Unit-Adresse und PCIe-Interrupt-Provider bleiben;
sie wurden durch diese Timingänderung nicht behoben. Unveränderte C-Treiber
wurden für diesen Analyse-/DTS-Schritt nicht erneut als MIPS-Objekte gebaut.

Das ist kein vollständiger OpenWrt-Firmware-/MODPOST-Build und kein
Hardwaretest. Nach einem solchen Build und Flashen: Gerät vollständig vom
Strom trennen, Startmeldung `restored MW5 OEM external flow-control thresholds
(6367/0060)` kontrollieren und die acht Register in
`/proc/mw5-rtl8367-flowctrl` prüfen. Anschließend bidirektionale LAN/WAN-
und WLAN-Last ohne laufendes `mw5-netdiag full` testen. Erst diese Prüfung
kann zeigen, ob die fehlende Boardinitialisierung zum Netzstillstand beitrug.

## Vollständige Kernel-Ablage und vertiefte NIC-Analyse

Die vollständigen entpackten OEM-Kernels, sämtliche 85.309 Symbole und die
wortweise geprüfte Disassemblierung aller Image-Bytes liegen nun unter
[`reverse-engineering/oem-kernels/`](reverse-engineering/oem-kernels/README.md).
Das Archiv enthält keine SPI-/Factory-/Umgebungspartitionen. Es bewahrt
auch Kernel-Daten und Code außerhalb der einfachen `_stext.._etext`-Grenze;
es ist keine vollständige semantische Rückentwicklung oder C-Dekompilierung.

Die [weitere NIC-Analyse](reverse-engineering/oem-kernels/NIC_FINDINGS.md)
bestätigt die 24-Byte-Ringe von MW5 **und AC23** direkt in den RX-/TX-
Init-Schleifen, gegenüber 32 Byte beim RD05. Der AC23-DTS setzt deshalb
jetzt explizit sechs Dwords, statt den Acht-Dword-Treiberstandard zu erben.
MW5 und RD05 erhalten ebenfalls explizite, zu ihrem bisherigen Verhalten
passende Angaben. Die CPUICR1-/Padding-Regression führt beide Formen aus.
Die neu erfasste OEM-Reset-Aufrufkette bindet auf allen drei Boards Tabellen-
Save/Restore, Re-Probe, NIC-Reinit und Betriebsmodus ein; sie wird nicht
als isolierte TX-Recovery in unseren laufenden Treiber kopiert.

Der aktuelle Host-Teststand umfasst 49 bestandene Tests. Alle drei
Kernel-Archive bestanden zusätzlich den unabhängigen wortweisen
Integritätstest. Ein vollständiger Firmwarebuild und Hardwarelasttest
werden dadurch weiterhin nicht ersetzt.
