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

Das ist eine Teilrekonstruktion der Aufruf- und Datenwege, keine vollständige
Rekonstruktion aller Timerbedingungen und Recovery-Schritte. Insbesondere
wird aus dem Funktionsnamen `one_sec_timer` keine ungeprüfte reale Zeitbasis
abgeleitet. Die genaue Behandlung von Watchdog-Grenzen, Deskriptoren und
Recovery benötigt weitere Untersuchung.

Die OEM-Firmware trennt Hardwarepolling, gecachte Userspace-Meldung und
TX-Fortschrittskontrolle. Ein weiterlaufender Watchdog oder das Lesen eines
gecachten proc-Werts beweist keine funktionierende Paketübertragung.
Der bestehende unabhängige Linux6-DMA/NAPI-Watchdog bleibt erhalten.

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

Das ist kein vollständiger OpenWrt-Firmware-/MODPOST-Build und kein
Hardwaretest. Nach einem solchen Build und Flashen: Gerät vollständig vom
Strom trennen, Startmeldung `restored MW5 OEM external flow-control thresholds
(6367/0060)` kontrollieren und die acht Register in
`/proc/mw5-rtl8367-flowctrl` prüfen. Anschließend bidirektionale LAN/WAN-
und WLAN-Last ohne laufendes `mw5-netdiag full` testen. Erst diese Prüfung
kann zeigen, ob die fehlende Boardinitialisierung zum Netzstillstand beitrug.
