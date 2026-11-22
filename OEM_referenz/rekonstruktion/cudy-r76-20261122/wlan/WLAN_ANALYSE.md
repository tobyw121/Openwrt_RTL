# Cudy AX1500: WLAN aus dem SPI-Dump rekonstruiert

## Ergebnis und Grenzen

Aus dem Dump wurden **beide vollständigen OEM-WLAN-Module, ihre Symboltabellen und Relokationen, eingebettete Funkfirmware sowie die mitgelieferten RF-/PHY-/Leistungstabellen** zurückgewonnen. Die Module sind nicht gestrippt. Damit sind sämtliche ausführbaren Sektionen als benannte, mit Relokationen kommentierte MIPS-Assemblerdarstellung verfügbar. Zusätzlich sind mehrere genau abgegrenzte Funktionen in lesbares, kompilierbares C rekonstruiert und gegen den Originalmaschinencode geprüft worden.

Das ist eine belastbare Rekonstruktionsbasis. Es ist **kein vollständig portierter Linux-6.6-WLAN-Treiber**. Originale Typdefinitionen, viele Strukturdeklarationen, Makros und Kommentare sind in den ELF-Dateien nicht erhalten. Aus einem erfolgreichen Offlinevergleich folgt auch keine bestätigte Funk-, DMA- oder Interruptfunktion am Router.

Die Quelle ist `Cudy_AX1500.bin`, 16.777.216 Bytes, SHA-256:

`f50f3147274a82fcbf4fef7a0c7500aeb7868b60e4e17975faea588237abd188`.

Alle nachfolgenden Befunde beziehen sich auf genau diesen Dump. Die Quelle einzelner Tabellen ist über Modulhash, ELF-Sektion, Sektionsoffset, Dateioffset, Objektgröße und SHA-256 im jeweiligen Manifest verzeichnet.

## 1. Die beiden Funkpfade

| Merkmal | 2,4 GHz | 5 GHz |
|---|---|---|
| OEM-Modul | `rtl8192cd.ko` | `rtk_wifi6.ko` |
| Modulgröße | 2.685.988 Bytes | 7.490.636 Bytes |
| OEM-Basisschnittstelle | `wlan1` | `wlan0` |
| OEM-UCI-Treibername | `rtl8197f` | `rtl8192x` |
| Quell-/Familiennamen im Modul | `rtl8192fe`, `RTK-8192FE-wlan-driver`; HAL/Firmware `8197G` | `rtl8832br`, `rtl8832bre`; MAC/PHY-/Firmwarezweig `8192xb` |
| Funktionssymbole | 2.360 | 7.734 |
| Sämtliche ELF-Symbole | 7.194 | 25.722 |
| Relokationen | 68.645 | 196.513 |
| Ausführbare Bytes | 1.489.468 | 3.527.744 |
| Unaufgelöste Bytes beim Disassemblieren | 0 | 0 |
| Externe Imports | 190 | 244 |

Quellen: `recovered/*/module_summary.json`, `symbols.tsv`, `relocations.tsv`; `oem_payload/lib/wifi/rtl8197f.sh` und `rtl8192x.sh`.

**Die Namen sind nicht austauschbar.** Der Dateiname `rtl8192cd.ko` beweist keinen separat bestückten RTL8192CD-Chip. Der vorliegende Build verwendet einen `rtl8192fe`-Treiberzweig und enthält umfangreiche `8197G`-Initialisierung. Ebenso sind die `8192xb`-Firmwarearrays im 5-GHz-Modul kein Widerspruch zum enthaltenen `8832BR`-RF-Zweig. Physische Chipbestückung, Revision und tatsächlich gewählte RF-Konfiguration benötigen zusätzlich Live-Register/PCI-IDs oder Boardunterlagen.

Der 5-GHz-Treiber registriert laut `.modinfo` die PCI-IDs `10ec:b832`, `10ec:b814` und `10ec:0192`. Diese Liste beschreibt vom Modul akzeptierte IDs; sie beweist nicht, dass alle drei Geräte im Router vorhanden sind. Dateisystem und Modulquellpfade enthalten `WR1500V2/21.02`. Das ist ein Buildhinweis, keine nachträgliche Identifikation der aufgedruckten Hardwareversion.

## 2. Kernel-ABI und vollständige Abhängigkeiten

Beide WLAN-Module sind ELF32, little endian, MIPS O32 / MIPS32r2. Ihr Vermagic lautet wörtlich:

`4.4.176 mod_unload MIPS32_R2 32BIT `

Beide hängen laut `.modinfo` von `cfg80211`, `compat` und `ther_ctrl` ab. Die Backportskennung lautet `v5.2.8-1-0-ga33d4e95`, mit Linux-Basiskennung `v5.2.8-0-gd36a8d2fb62c`. Der 5-GHz-Treiber trägt zusätzlich die Versionszeichenfolge `v2.0.0.1 2021/09/22`. Diese Versionszeichenfolge ist nicht das Erstellungsdatum des gesamten Routerimages.

Im Kernelrohbild wurde eine konsistente Exporttabelle von **5.006 Einträgen** bei Dateioffset `0x5ef890` bis exklusiv `0x5f9500` ermittelt. 4.682 Name-/Adresspaare stimmen zusätzlich mit den rekonstruierten Kallsyms überein; die weiteren Einträge enthalten unter anderem Daten-/BSS-Symbole. Die Tabelle liegt als `abi/kernel_export_table.tsv` bei.

Alle Imports sämtlicher **20 OEM-Module** lassen sich auf diese Kernelexporte oder Exporte der übrigen mitgelieferten Module zurückführen. Es bleiben im Namensabgleich **keine unaufgelösten Imports**. Der vollständige Nachweis einschließlich Anbieter je Symbol liegt in `abi/module_abi_audit.json`.

Dies bestätigt die geschlossene Symbolbasis des extrahierten OEM-Satzes. Es prüft keine Strukturgrößen, Kernelkonfiguration oder Laufzeitverträglichkeit mit einem anderen Kernel. Insbesondere sind die Module dadurch nicht direkt auf Linux 6.6 oder ein beliebiges OpenWrt ladbar.

Vendor-Verknüpfungen umfassen beispielsweise `rtl_wlanFwdToEth`, `rtl_wifi_fdb_add_hook`, `rtl_wifi_fdb_del_hook` und `rtk_eventd_netlink_*`; der 2,4-GHz-Treiber nutzt außerdem `PCIE_reset_procedure_97F`, `RTLWIFINIC_GPIO_*`, `_dma_cache_wback`, `_dma_cache_inv` und `_dma_cache_wback_inv`. Diese konkreten Imports erklären, warum eine Portierung mehr als eine Anpassung von `vermagic` erfordert.

## 3. Firmware und Initialisierungstabellen

### Eingebettete Funkfirmware

| ELF-Objekt | Größe | SHA-256 |
|---|---:|---|
| `data_rtl8197Gfw` | 60.368 B | `247480a7df3d26ce7f96a1fbbb13b340db02d966a8956592241dce61c739aef2` |
| `array_8192xb_u1_ap` | 385.464 B | `e460f4fbea2ffab4fc38e79848a947c25be93e3e2bce3d30b808f4ff3d65a0fa` |
| `array_8192xb_u2_ap` | 385.536 B | `f9b4e66dba9b8ef505056493fa1a1f49e245049e89754b15894a66b29c4c96a2` |

Die Bytes liegen jeweils als `.bin` und als bytegenaues C-Array vor. `u1` und `u2` werden als ursprüngliche Objektnamen erhalten; ohne zusätzlichen Nachweis wird ihnen keine beliebige Chiprevision zugewiesen.

Insgesamt sind **232 benannte Firmware-/Register-/Power-Objekte** separat extrahiert: 201 aus dem 2,4-GHz-Modul und 31 aus dem 5-GHz-Modul, zusammen 1.193.115 Bytes. Dazu gehören `array_mp_8197g_*`, `array_mp_8832br_*`, `array_mp_8192xb_*`, originale Texttabellen, Trackingtabellen und Powersequenzen. Die vollständigen Originalmodule erhalten darüber hinaus alle weiteren, nicht separat klassifizierten Datenobjekte.

Bei Objekten mit Relokationen darf der extrahierte Rohdatenblock nicht als bereits geladene, gültige Pointerstruktur behandelt werden. Daher enthält das Manifest alle zugehörigen Relokationen. Für **acht TX-Power-Tabellen** wurden sämtliche Stringpointer anhand der ELF-Relokationen aufgelöst: **23.520 Einträge** sind als C-Stringlisten wiederhergestellt. Ihre Reihenfolge ist original; eine nicht belegte Zeilen-/Spaltenstruktur wird nicht erfunden.

### Originale RF-Dateien

Unter `oem_payload/etc/conf/rtl8832bre/RFE50/` sind **38 unveränderte Dateien** vorhanden. Sie umfassen unter anderem:

- `RadioA.txt`, `RadioB.txt`, `PHY_REG.txt` und `PHY_REG_GAIN.txt`;
- `TXPWR_ByRate.txt`, `TXPWR_LMT.txt`, `TXPWR_LMT_RU.txt` und `TXPWR_TrackTSSI.txt`;
- getrennte Profile unter `AP_2G`, `AP_5G`, `PON_2G` und `PON_5G`.

Die Präsenz mehrerer Profile beweist nicht deren gleichzeitige Laufzeitnutzung. Der Modulstarter setzt `rtw_phy_file_path=/etc/conf/`, die weitere Auswahl hängt von Treiber-/Board-/Kalibrierungsdaten ab.

### 2,4-GHz-Powersequenzen

Die Sequenzen wurden als 8-Byte-Einträge dekodiert. Quelle ist zusätzlich der Maschinenkode von `HalPwrSeqCmdParsing88XX` bei `.text+0x158480`.

| Sequenz | Allokierte Einträge | Einträge bis einschließlich END |
|---|---:|---:|
| `rtl8197G_card_enable_flow` | 41 | 22 |
| `rtl8197G_card_disable_flow` | 31 | 12 |
| `rtl8197G_enter_lps_flow` | 16 | 14 |
| `rtl8197G_leave_lps_flow` | 16 | 12 |

Die TSV-Dateien enthalten Registeroffset, Cut-/Fab-/Interfacefilter, Befehl, Maske und Wert. Im vorliegenden Interpreter sind Befehl 1 ein maskierter Bytezugriff, 2 Polling, 3 Verzögerung und 4 Ende. Das Befehlfeld ist das obere Nibble von Byte 4; Cutmasken stehen in Byte 2, Fab-/Interfacefilter in Byte 3. Der Pollingzähler ist sequenzweit und wird nicht für jeden Eintrag zurückgesetzt.

## 4. Lesbare C-Rekonstruktion und Prüfung

`reconstructed_c/cudy_wlan_recovered.c` enthält tatsächlich aus dem Binärcode abgeleitete Funktionen. Die API wurde für sichere Offlineverwendung neu geschrieben; sie ist keine Behauptung, die ursprünglichen C-Deklarationen zurückgewonnen zu haben.

| Rekonstruktion | Originalstelle | Prüfung |
|---|---|---|
| `cudy_8832br_txdesc_length` | `mac_txdesc_len_8192xb`, `.text+0x28ce4c`, 104 B | 1.000 Fälle gegen originalen MIPS-Code |
| `cudy_8832br_wd_checksum` | `mac_wd_checksum_8192xb`, `.text+0x28cff8`, 260 B | 1.000 Fälle gegen originalen MIPS-Code |
| `cudy_8197g_decode_rx_fields` | Felddekodierung in `QueryRxDesc88XX_V1`, `.text+0x158efc`, 584 B | 1.000 Fälle gegen vollständige Originalfunktion mit bereitstehendem Ring |
| `cudy_wlan_mask_merge` | mathematischer Kern von `hal_write32_mask`, `.text+0x1eefb0`, 176 B | statischer Instruktionsvergleich, C-Kompilierung |
| `cudy_8197g_run_power_sequence` | Steuerkern von `HalPwrSeqCmdParsing88XX`, `.text+0x158480`, 652 B | 200 Fälle mit originalem MIPS-Interpreter, darunter 20 Timeouts; vollständige Register-/Delay-Traces identisch |

Alle **3.000 Datenpfad-Differenzfälle stimmen überein**; zusätzlich stimmen **200 Powersequenzfälle** einschließlich 20 absichtlich ausgelöster Timeouts mit dem Originalinterpreter überein. Damit wurden insgesamt **3.200 WLAN-Fälle** geprüft. Die Originalbefehle wurden isoliert unter **Unicorn 2.0.1.post1** mit MIPS-24Kc-Modell ausgeführt. Der Versionspin ist absichtlich gewählt: Unicorn 2.1.4 löste bei gültigen komplexen MIPS-Branchfolgen eine reproduzierbare Emulatorausnahme aus; die Datenpfadvergleiche bestanden unter beiden Versionen, die Powersequenzvergleiche unter dem festgelegten Pin. Für den Call der Checksummenfunktion zur Längenfunktion wurde genau die notwendige ELF-HI16-/LO16-Relokation angewendet. Es wurden keine OEM-Binärdateien nativ ausgeführt.

Die RX-Prüfung deckt den Dekodierpfad bei bereitstehendem Ring ab; Cache-/DMA-Synchronisation, wartende Hardware, IRQs und tatsächliche Funkpakete bleiben außerhalb der Prüfung. Der C-Decoder verändert genau die im Original geschriebenen Ausgabebytes und erhält unbekannte Zwischenfelder. Die Powersequenz-API verwendet austauschbare Register-/Delaycallbacks; die plattformspezifische Taktsperre des Originals muss von einer späteren Hardwareanbindung berücksichtigt werden. Die 200 Powersequenzfälle emulieren einen durchgehend getakteten Registerblock und prüfen keine tatsächliche Kaltstartinitialisierung.

Die 5-GHz-TX-Deskriptorlänge ist nicht konstant: der rekonstruierte Code liefert je nach Metadaten **16, 32 oder 56 Bytes**. Der Checksummenkern löscht zunächst die unteren 16 Bits bei Deskriptoroffset 16, XOR-verknüpft die Halbwords der ermittelten Deskriptorlänge und schreibt das Ergebnis zurück. Dies betrifft WLAN-Deskriptoren; daraus folgt keine Aussage über den separaten Ethernet-DMA-Deskriptor.

## 5. OEM-Start und Kalibrierung

`wlan.preinit` hat `START=09` und ruft `/bin/startup.sh` auf. Das Skript initialisiert und prüft die Hardwarekonfiguration über `flash`. Es kann bei ungültigen Daten Standardwerte schreiben. `wlan.postinit` hat `START=11`, ruft `g6_drv_inst.sh` auf und überträgt anschließend mit `flash radio-cfg` Hardwareeinstellungen in die Treiber. `g6_drv_inst.sh` liest die tatsächliche Treiberkonfiguration aus `/proc/net/rtk_wifi6/drv_cfg` und erstellt gegebenenfalls persistente VAP-/Clientinterfaces. `wlan_thermal` startet den Temperaturdienst.

Diese Skripte sind als unveränderte Referenz beigefügt. Sie werden durch die Analysewerkzeuge **nicht ausgeführt**. Ein OpenWrt-Port muss Kalibrierungs-/eFuse-/Boarddaten gezielt übernehmen; das Kopieren von Standard-RF-Tabellen ersetzt die gerätespezifischen Daten nicht.

## 6. Was für den OpenWrt-Port noch konkret fehlt

Die rekonstruierte Basis erlaubt den Vergleich von Registerfolgen, Deskriptoren, Firmwareauswahl und Kalibrierpfaden. Für einen einsatzfähigen Quellport sind weiterhin vollständige Datenstrukturen und Aufrufketten zu prüfen, insbesondere PCI-/Plattformanbindung, DMA-/Cachebehandlung, IRQ-/Tasklet-/NAPI-Pfade, Queueverwaltung, cfg80211-/netdevice-API, Association-/Securitypfade, RF-Kalibrierung, DFS und Leistungsbegrenzung.

Der in diesem Auftrag zusätzlich inventarisierte frühere GPL-Tarball `GPL_for_AX1500_rtl8197.tar.gz` enthält 27.989 Mitglieder. In diesem konkreten Archiv wurden keine passenden C-Quellbäume für `rtl8192fe`, `rtk_wifi6`, `8832` oder `8197G` gefunden. Das ist ein Befund über dieses Archiv; es ist keine Aussage, dass passende Quellen grundsätzlich nirgends erhältlich wären. Die zusätzlich vom Hauptbericht bereitgestellte automatische Dekompilierung dient als Reviewhilfe, nicht als bereits kompilierbarer Originalquellbaum.

## 7. Paketinhalt und Reproduzierbarkeit

- `oem_payload/`: 20 originale Kernelmodule, WLAN-Start-/Adapter-/Thermalskripte, Flash-/Thermalprogramme und 38 RF-Texttabellen; Hashliste in `oem_payload_manifest.json`.
- `recovered/`: vollständige kommentierte Disassemblierungen beider Module, Symbol-/Relokationstabellen, Modulinfos, Firmware-/Konstantenobjekte und aufgelöste Stringpointertabellen.
- `decoded_power_sequences/`: decodierte 8197G-Powersequenzen.
- `abi/`: Kernelexporte und vollständiger Modulabhängigkeitsabgleich.
- `reconstructed_c/`: lesbare C-Rekonstruktionen, Makefile und reproduzierbare MIPS-Differenzprüfung.
- `extract_rootfs.py`: sichere SquashFS-Extraktion regulärer Dateien; Symlinks und Gerätedateien werden nur im Manifest dokumentiert.
- `analyze_wlan_modules.py`, `resolve_constant_pointers.py`, `audit_module_abi.py`: wiederholbare Analysewerkzeuge.

### Offlineprüfung auf dem Ubuntu-PC

Nach dem Entpacken des Gesamtpakets:

```bash
cd ~/Downloads/Cudy_AX1500_Rekonstruktion/wlan
python3 -m venv .venv
. .venv/bin/activate
python3 -m pip install -r requirements.txt
make -C reconstructed_c CC=gcc -j"$(nproc)" verify
python3 audit_module_abi.py oem_payload ../kernel/kernel.bin abi_recheck
```

Die Prüfung verwendet nur die mitgelieferten Originalmodule und den rekonstruierten C-Code. **Auf dem OpenWrt-Router ist für diese Offlineprüfung kein Befehl notwendig.** Die mitgelieferten OEM-Module sind kein Installationspaket für Linux 6.6. `verification.json` dokumentiert die durchgeführte Prüfung und ihre Reichweite.
