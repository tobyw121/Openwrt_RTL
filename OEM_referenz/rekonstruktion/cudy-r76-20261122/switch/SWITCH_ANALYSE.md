# Cudy R76/AX1500: RTL8367D-Switchrekonstruktion

Stand: 22. November 2026. Untersucht wurde der angehängte Flashdump und zusätzlich das zuvor vom Nutzer bereitgestellte Herstellerarchiv `GPL_for_AX1500_rtl8197.tar.gz`. Die vorhandenen OEM-Quellen werden ausdrücklich als Herstellerreferenz bezeichnet. Sie wurden nicht aus Binärdaten als angeblich wiedergewonnener Originalquelltext ausgegeben.

## 1. Ergebnis und Belegstärke

Der externe Switch ist im Kernel mit der **Realtek-API `rtl83xx_v1dot4` einschließlich RTL8367D-DAL** implementiert. Die D-Funktionen lassen sich dem vorhandenen Herstellerquellbaum zuordnen. Die wesentlichen Board-, RGMII-, SSC-, CPU-Tag- und VLAN-Operationen wurden zusätzlich gegen die tatsächlichen MIPS-Instruktionen geprüft.

Die vorliegende Rekonstruktion enthält:

- eine Zuordnung von **2.045 Kernel-Symbolen zu Definitionen** in diesem SDK;
- ein Quellinventar mit **227 Dateien**, davon **109 C-Dateien und 116 Header**, zusammen 5.899.161 Byte;
- **17 bytegenau wiedergefundene Datentabellen** mit zusammen 609 Einträgen; mehrere Tabellen teilen Präfixe oder identische Inhalte, weshalb dies keine Anzahl eindeutiger Hardware-Schreibvorgänge ist;
- kommentierte Disassemblate der tatsächlich untersuchten Funktionen;
- die aus dem Kernel extrahierte D-HAL-Portzuordnung;
- eine Prüfung unveränderter OEM-Funktionen mit einem ausschließlich im Arbeitsspeicher simulierten SMI-Bus.
- eine selektive C-Rekonstruktion der belegten Registeroperationen mit austauschbaren Bus-Callbacks und Vergleich ihrer Schreibfolgen mit dem unveränderten OEM-Code.

Eine Symbol-/Definitionszuordnung allein beweist keine vollständige Funktionsgleichheit. Die 17 Tabellen sind ein stärkerer Beleg: Ihre gepackten Werte stimmen bytegenau mit dem Kernel überein. Hardwaretests, Paketübertragung, GPIO-Zeitverhalten und Linux-DSA-Integration wurden damit nicht geprüft.

**Eingangsdatei:** 16.777.216 Byte, SHA-256 `f50f3147274a82fcbf4fef7a0c7500aeb7868b60e4e17975faea588237abd188`.

**Untersuchter Kernel:** 6.743.460 Byte, SHA-256 `7d7e41a50d6659b5c84c524205bcca25306e2e98a5949c61032810ff84fd003e`; Linux 4.4.176 vom 21.01.2025. Er stimmt bytegenau mit dem dekomprimierten Kernel des früheren symbolisierten Analysearchivs überein. Deshalb sind dessen Symboladressen als Locator verwendbar. Alle unten genannten Kerneladressen beziehen sich auf Basis `0x80000000`.

## 2. Vier verschiedene Portnummerierungen

Die folgenden Nummern dürfen nicht vermischt werden:

| Ebene | Belegter Wert | Grundlage |
|---|---:|---|
| Externer RTL8367D: LAN-Reihenfolge der Boarddefinition | 3, 0, 1 | RootFS `/etc/board.d/02_network`, Fall `R76` |
| Externer RTL8367D: WAN | Port 2 | gleiche Boarddefinition und `RTL83XX_vlan_set` |
| SDK: logischer CPU-Port | 17 / `EXT_PORT1` | `RTL83XX_init`, Chiptyp-4-Zweig |
| Externer RTL8367D: physischer CPU-Port | 7 | D-HAL-Tabelle bei `0x80634d10`, `l2p_port[17] = 7` |
| OEM-Boarddefinition: zusätzliche Netif-Portangaben | `6@eth0`, `5@eth1` | RootFS-Definition; keine Aussage über den physischen RTL8367D-CPU-Port |
| OEM-Netzgeräte | LAN `eth0`, WAN `eth1` | RootFS-Definition |

Die Bestückung RTL8367D und die Verbindung des internen RTL8197F-Port0 mit dem externen EXT1/RGMII-Port waren bereits im Projektverlauf bestätigt. Der Flashdump selbst enthält keine aktuelle Messung eines Chip-ID-Registers. Er enthält den passenden Erkennungscode und die D-Konfiguration. `switch_probe` erkennt die D-Familie anhand `0x6642` in Register `0x1300` und wählt anschließend Chiptyp 4.

Die D-HAL-Struktur und ihre Prüfsumme stehen in `evidence/rtl8367d_hal_from_binary.json`. Sie erlaubt UTP0–4 und EXT0/EXT1; das generische UTP-Maskenbit für Port4 bedeutet nicht, dass am R76 ein fünfter RJ45-Anschluss existiert.

## 3. Der tatsächlich gebaute SMI-Pfad

Die Aufrufkette lautet:

`rtl8367d_set/getAsicReg` → `rtl8367c_smi_write/read` → `smi_write/read` → GPIO-Bitoperationen.

Das `c` im Transportnamen ist **kein Beleg für einen RTL8367C-Switch**. Der D-Header bildet seine Transportfunktionen ausdrücklich auf denselben Anschluss ab. Der Kernel benutzt `common/smi.c`; das generische Beispiel `dal/rtl8367d/rtl8367d_smi.c` mit leeren Portierungs-Makros steht nicht in der gebauten Objektliste.

Im Binärcode nachgewiesen:

- Schreibkommando `0xb8`, Lesekommando `0xb9`;
- 16-Bit-Registeradresse, Low-Byte zuerst;
- 16-Bit-Registerdaten, Low-Byte zuerst;
- bis zu fünf ACK-Abfragen pro Phase;
- Sperren und Wiederherstellen des lokalen Interruptzustands um die Transaktion.

Kerneladressen: `smi_read = 0x802738c4`, `smi_write = 0x80273a48`, D-Registerschreiben `0x802aceac`, D-Registerlesen `0x802aced4`. Beleg: `board_init_disassembly.txt`, `register_disassembly.txt` sowie `common/smi.c` und `rtl8367d_asicdrv.h` im Herstellerquellbaum.

Die GPIO-Auswahl ist durch SoC-/Bond-Abfragen verzweigt. Der Binärcode enthält unter anderem G6/C3, H0/G7 und C2/C3 als SCK/SDA-Kombinationen. Ohne den aktuellen Bond-/Chipregisterwert darf daraus keine universell richtige GPIO-Kombination abgeleitet werden. `init_83XX` enthält zudem Resetwiederholungen und Wartezeiten, nicht nur den SDK-Aufruf.

## 4. Initialisierung: Bootloader und Kernel unterscheiden sich

Die große Funktion `RTL83XX_init` ist die Boardkonfiguration. Die namensähnliche **`dal_rtl8367d_switch_init` enthält im Kernel lediglich einen erfolgreichen Rücksprung**, keine Registerschreibfolge:

```text
0x802b5cf0  jr ra
0x802b5cf4  move v0, zero
```

Das stimmt mit der D-Quelldatei überein. Die C-Variante enthält andere Initialisierungs- und Patchpfade. Bei Chiptyp 4 kehrt die Boardfunktion nach der D-Konfiguration zurück, bevor die C-bezogenen Registerfolgen `0x121f` bis `0x1226` und der nachfolgende PHY-Patch aufgerufen würden. Diese Werte dürfen deshalb nicht als R76-D-Kaltstarttabelle weiterverwendet werden.

Auch der Bootloader wurde geprüft. Der aus dem gzip-Bereich bei Flashoffset `0x8d60` extrahierte Code hat SHA-256 `df7179f0f0dd3c81cf928141588c5e9d32ede2bd75e1738d6c69da297deabd92`. Absolute Stringreferenzen und interne Sprungziele stützen eine Codebasis von `0x80000000`. Die Zuordnung wurde über die Mapperkette verfolgt:

| Bootloaderbestandteil | Adresse |
|---|---:|
| Boardfunktion `RTL83XX_init` | `0x8000d8b0` |
| `rtk_switch_init` | `0x8000e244` |
| DAL-Auswahl | `0x8000eaa0` |
| D-Mapper-Getter | `0x80014d90` |
| D-Mapper-Tabelle | `0x80021d50` |
| erster Funktionspointer: D-Switch-Init | `0x80015de0` |

Auch dieser letzte Funktionspointer führt zu `jr ra; move v0,zero`. Die Bootloader-Boardfunktion richtet den Anschluss hingegen tatsächlich ein. Sie verwendet TX-Delay0/RX-Delay2 und zunächst CPU-Tagmodus NONE. Der später ausgeführte Kernel verändert diese Einstellungen erneut.

Damit wurde in den geprüften D-Initpfaden **keine große bislang versteckte analoge PHY-Kaltstarttabelle gefunden**. Das beweist nicht, dass der Switch ohne Reset-, Strap-, ROM- oder Hardwareinitialisierung arbeitet. Ein Flashdump enthält keine zeitliche Aufzeichnung der tatsächlichen Registerantworten während eines Kaltstarts. Belege: `evidence/bootloader_switch.json` und `bootloader_board_init_disassembly.txt`.

## 5. RGMII, SSC und MAC-Force: konkrete D-Werte

`RTL83XX_init` ruft zunächst `rtk_port_rgmiiDelayExt_set(EXT1,0,2)` auf. Anschließend versucht es SSC einzuschalten. **Nur wenn dieser SSC-Aufruf erfolgreich ist**, folgt `rtk_port_rgmiiDelayExt_set(EXT1,1,0)`.

| Register | Maskierter Effekt im überprüften Kernelpfad |
|---|---|
| `0x1307` | Low-Nibble zuerst `0x2`; nach erfolgreichem SSC `0x8`; obere Bits bleiben erhalten |
| `0x1d53` | SSC-Ein: `0x0001` |
| `0x1d55` | SSC-Ein: `0x000f` |
| `0x1d54` | SSC-Ein: `0x05fa` |
| `0x1d52` | SSC-Ein: `0x2473` |
| `0x1303` | Bits 9, 6, 4 und 1 werden gesetzt, Maske `0x0252` |
| `0x1304` | DN-Feld Maske `0x7000` erhält 7; DP-Feld Maske `0x0700` erhält 5 |
| `0x1d70` | MAC7/EXT1-Mux: Bit13 setzen, sofern Bit12 für MAC4/EXT1 nicht gesetzt ist |
| `0x1305` | Bits7:4 erhalten Modus1 = RGMII |
| `0x1d78` | Bits4:0 erhalten `0x1f`, der konkurrierende SDS1-Modus wird deaktiviert |
| `0x12c7` | Force-Status für physischen Port7: `0x0076` |
| `0x12cf` | Force-Enable für physischen Port7: `0xffff` |

Die beiden letzten Werte stehen für den hier angeforderten 1-Gbit/s-Fullduplex-Link mit RX/TX-Pause und ohne Autonegotiation auf der CPU-MAC-Verbindung. **Das D-MAC-Force-Registerlayout unterscheidet sich von der älteren C-Variante**; ein ausschließlich auf `0x1311` basierender Portierungsversuch wäre damit nicht gleichwertig.

Im untersuchten RGMII-Kernelpfad wurde kein Schreibzugriff auf `0x13f9` gefunden. Ein entsprechender Wert aus einem anderen Treiberstand oder einer anderen Chipvariante darf nicht als im Dump gemessene Initialisierung ausgegeben werden.

Die Tabelle beschreibt beabsichtigte Registeroperationen bei erfolgreichen API-/SMI-Aufrufen. Sie ist keine Messung des aktuell eingeschalteten Routers. Belege: `register_disassembly.txt`, `mac_vlan_disassembly.txt`, Herstellerdatei `dal_rtl8367d_port.c`.

## 6. CPU-Tagging: NONE ist nur ein Zwischenzustand

Die vollständige erfolgreiche Aufruffolge ist entscheidend:

1. `init_83XX` ruft `RTL83XX_init` auf; dabei zunächst CPU-Funktion ein, Insertmodus NONE.
2. VLAN-Grundinitialisierung und Board-VLAN-Konfiguration folgen.
3. Bei `0x8027805c` folgt **`RTL83XX_cpu_tag(1)`**.
4. Diese Funktion setzt Insertmodus ALL, Tagformat4Byte, Accept-Length1 und CPU-Funktion ein.

Für EXT1 → physischen Port7 bedeutet dies im erfolgreichen Pfad:

- `CPU_PORT_MASK 0x1219 = 0x0080`;
- `CPU_CTRL 0x121a` erfüllt für die durch diese Aufrufe behandelten Bits **`value & 0x06bf == 0x02b9`**;
- die übrigen Bits werden nicht durch diese Ableitung bestimmt;
- es handelt sich um das im OEM verwendete vier Byte lange Realtek-CPU-Tagformat mit EtherType `0x8899`.

Der isolierte String `CPU_INSERT_TO_NONE` oder die erste Boardfunktion allein ergeben deshalb ein falsches Bild des Endzustands. Ein moderner DSA-Tagger muss zum tatsächlich gewählten Format passen; bloß dieselbe Taglänge ist kein ausreichender Kompatibilitätsnachweis. Belege: `tag_init_disassembly.txt`, `vlan_disassembly.txt`, `dal_rtl8367d_cpu.c`.

## 7. VLANs und Abgrenzung zum internen RTL865x

Der externe Switch verwendet in der untersuchten Boardlogik **WAN-PVID8 und LAN-PVID9**. Für WAN ist Port2 fest codiert. Nachgewiesene Stellen sind `0x802a1330` für VID8 und `0x802a1364` für VID9 in `RTL83XX_vlan_set`.

Dies widerspricht nicht automatisch internen RTL865x-/Netif-VLAN1/2-Einträgen: Das sind verschiedene Tabellen in verschiedenen Switch-/NIC-Ebenen. Eine Portierung muss diese Zuordnung vollständig verfolgen, statt die Nummern zwischen beiden Ebenen gleichzusetzen. Die abschließenden VLAN-/Isolationseinstellungen hängen außerdem vom Betriebsmodus und späteren Konfigurationsaufrufen ab.

Die D-Familie verwendet für PVID **ein Register pro physischem Port**:

`Register = 0x0700 + phys_port`, Maske `0x0fff`.

Eine PVID-Memberindex-Paarung nach einem älteren C-Layout wäre hier falsch. Die D-API akzeptiert an dieser Stelle nur Priorität0. Der D-VLAN4K-Eintrag wird als zwei 16-Bit-Wörter über `0x0510/0x0511` geschrieben, die VID nach `0x0501` und der maskierte Schreibbefehl `0x0b` nach `0x0500`. Die Bitbelegung enthält 8-Bit-Member-/Untag-Masken, 2-Bit-FID sowie IVL/SVL-Steuerbits.

Belege: `mac_vlan_disassembly.txt`, `register_disassembly.txt`, `dal_rtl8367d_vlan.c` und `rtl8367d_reg.h`.

## 8. Extrahierte Tabellen und Grenzen ihrer Verwendung

`evidence/exact_table_matches.json` nennt für jede Tabelle Quellpfad, Eintragszahl, Speicherbreite, Kerneloffset, virtuelle Adresse und SHA-256 der Rohbytes. Zu den größeren Treffern gehören:

| Tabelle | Einträge | Adresse | Einordnung |
|---|---:|---:|---|
| `PatchChipData` | 210 | `0x80550c10` | C-Familien-Patch; nicht als aktiver D-Kaltstartpatch behandeln |
| `SDSNctrlPatch` | 258 | `0x8054ec7c` | D-SerDes-Patch für entsprechende Portmodi |
| `Fiber1000M100MCfg` | 14 | `0x8054f594` | D-Fiber-Modus |
| `Fiber2P5GCfg` | 11 | `0x8054f48c` | D-Fiber-Modus |
| `HSGCfg`, `SGCfg` | je 9 | mehrere Präfixtreffer | HSGMII-/SGMII-Tabellen |

Die Anwesenheit von Fiber-/SerDes-Code beweist keine entsprechende Platinenverdrahtung. Der R76-Boardpfad wählt RGMII; die SerDes-Tabellen werden deshalb nicht als seine aktive Registerfolge angeboten.

## 9. Was tatsächlich geprüft wurde

`host_validation/run_validation.py` kompiliert fünf unveränderte C-Dateien aus dem OEM-D-SDK. Die Übertragungsfunktionen und Portzuordnung werden ausschließlich durch ein RAM-Modell ersetzt. Die Sollwerte wurden aus den oben genannten Instruktionsfolgen abgeleitet.

Erfolgreich geprüft wurden:

- leerer D-Switch-Init ohne Registerzugriffe;
- sämtliche 16 TX/RX-Delaykombinationen und Erhalt der oberen Registerbits;
- korrekte Ablehnung ungültiger Ports, Werte und fehlender Initialisierung;
- SSC-Ein-/Aus-Reihenfolge und Abbruch bei SMI-Schreibfehler;
- Übergang des CPU-Taggings vom Zwischenzustand NONE zu ALL/4Byte;
- D-PVID-Adressierung, VID-Grenze und Priorität0;
- VLAN-Tabellenformat aus zwei Wörtern und maskierter Befehl;
- D-MAC-Force-Register für physischen Port7.
- identische Registerschreibfolgen der neu geschriebenen C-Rekonstruktion und der unveränderten OEM-Funktionen bei vier verschiedenen anfänglichen Registermustern, einschließlich CPU-Disable und SMI-Fehlerabbruch.

`rtl8367d_reconstructed.c/.h` bildet die dokumentierten RGMII-Delay-, SSC-, PVID-, VLAN-, CPU-Tag- und MAC-Force-Operationen ab. Die Callback-Schnittstelle, physischen Portparameter und lokalen Fehlercodes sind ausdrücklich neue Anpassungen. Sie ersetzen weder die SDK-Initialisierungsprüfung noch die GPIO-/IRQ-Sperren des Betriebssystems. Die zwei ein Bit breiten VLAN-Flags akzeptieren hier nur 0/1; der OEM-Code lässt in seinen Grenzprüfungen zusätzlich den Enum-Endwert2 passieren. Ein vollständiger Board-Kaltstart wird von diesen Hilfsfunktionen nicht behauptet.

Das Ergebnis steht in `host_validation/result.txt`. Dies ist ein Nachweis für die reproduzierte Registerlogik, kein Nachweis für einen bootfähigen OpenWrt-Treiber für Linux 6.6. Für dessen Integration sind reale GPIO-Anbindung, Reset-/Taktdaten, Sperren, Phylink/DSA-Einbindung, Tagverarbeitung und Messungen nach einem echten Kaltstart weiterhin erforderlich.

## 10. Nachvollziehbare Dateien

| Datei | Inhalt |
|---|---|
| `evidence/source_files.json` | unveränderte OEM-Dateien mit Größe und Prüfsumme |
| `evidence/function_source_map.tsv` | Symboladresse, Funktionslänge bis zum nächsten Symbol, Bytehash, Quelldefinition |
| `evidence/verified_functions.json` | manuell geprüfte Funktionen mit präzisem Prüfumfang; keine pauschale Quellgleichheitsbehauptung |
| `evidence/exact_table_matches.json` und CSV-Dateien | bytegenaue Tabellenrekonstruktion |
| `evidence/rtl8367d_hal_from_binary.json` | rohe Portzuordnung und HAL-Limits |
| `evidence/bootloader_switch.json` | Bootloaderadressen und D-Mappernachweis |
| `*_disassembly.txt` | MIPS-Instruktionen der konkret untersuchten Funktionen |
| `host_validation/` | reproduzierbare Prüfungen an unverändertem OEM-Code, ohne Hardwarezugriff |
| `rtl8367d_register_contract.h` | belegte Konstanten, SSC-Sequenzen und reine Feldcodierung; kein Hardwaretreiber |
| `rtl8367d_reconstructed.c/.h` | neu formulierte, selektive Registerlogik mit Bus-Callbacks und dokumentierten Schnittstellenanpassungen |
| `rootfs_evidence/` | Boardnetzdefinition und Boarderkennung mit Herkunftsprüfsummen |

Die OEM-Quellen selbst liegen im gemeinsam gelieferten Herstellerreferenzbaum. Copyright- und Lizenztexte bleiben dort unverändert. Die Daten- und Funktionsmanifeste ersetzen weder einen vollständigen Compilervergleich noch einen Hardwaretest.
