# Cudy AX1500 / WR1500 R76: Ethernet, LAN/WAN und CPU-DMA

Stand: 22.11.2026. Untersucht wurde der angehängte 16-MiB-Dump `Cudy_AX1500.bin`.

## Ergebnis und tatsächlich gelieferte Rekonstruktion

Der aktive Ethernet-Datenpfad lässt sich wesentlich genauer bestimmen als durch eine bloße Suche nach `swNic_*`-Symbolen. Die Registrierung der Netzwerkfunktionen, der Aufruf aus `re865x_open`, die TX-/RX-Tasklets und die Paketfunktionen führen zum **neuen RTL8197F-Descriptorpfad**. Mehrere entscheidende Paketfunktionen liegen in einem zusätzlichen Codebereich nach `_etext` und waren in der zuvor vorhandenen Symboltabelle nicht benannt. Ihre Adressen und ihre tatsächlich kompilierten Verzweigungen wurden hier zurückgewonnen.

Das Paket liefert:

- `r76_desc_reference.c/.h`: kompilierbare C-Rekonstruktion der TX-Descriptorbildung, RX-Metadaten, der vollständigen RX-Vorverarbeitung sowie der Ring-/OWN-Entscheidungen. Der zusätzliche CDP-Plausibilitätscheck ist ausdrücklich als neue Funktion markiert.
- `hotcode_functions.tsv`: zehn nachträglich identifizierte Paket-/Taskletfunktionen mit Adresse, Größe, Zuordnungssicherheit und Beleg.
- `ethernet_evidence.json`: Default-Ringgrößen, Registertabellen, VLAN-/Netif-Defaults, rekonstruierte globale Zustandsadressen und Callgraph.
- `disassembly/`: adressierte Originalinstruktionen der ausgewerteten Funktionen und der relevanten Registrierungsstellen.
- `verify_against_mips.py`, `verification-fast.json`: Vergleich der C-Ergebnisse und Ringentscheidungen mit ausgeführten Original-MIPS-Routinen in einer isolierten Emulation.
- `unicorn_branch_regression.py` und zwei Versionsprotokolle: reproduzierbarer Nachweis einer Emulatorabweichung, wegen der die geprüfte Version festgelegt ist.

Diese C-Bibliothek ist **kein vollständiger Linux-Netzwerktreiber**. Sie enthält keine Linux-Geräteregistrierung, keinen DMA-Allocator, keine Interruptanbindung, kein NAPI/BQL und keine DSA-Anbindung. Die übergeordnete Treiberimplementierung liegt zusätzlich als OEM-GPL-Referenz im Gesamtpaket. Eine Übernahme in unseren OpenWrt-Treiber muss diese Kernelabhängigkeiten an den Zielkernel anpassen und anschließend am Router geprüft werden.

## 1. Identität der Binärquelle

| Bestandteil | Wert |
|---|---|
| SHA256 des Flash-Dumps | `f50f3147274a82fcbf4fef7a0c7500aeb7868b60e4e17975faea588237abd188` |
| SHA256 des entpackten Kernels | `7d7e41a50d6659b5c84c524205bcca25306e2e98a5949c61032810ff84fd003e` |
| Entpackter Kernel | 6.743.460 Bytes |
| Kernel | Linux 4.4.176, gebaut am 21.01.2025 |
| Ausgewertete Instruktionen | MIPS32, Little Endian |
| Adressbezug | Kernel-Dateioffset = virtuelle Adresse − `0x80000000` |

Der entpackte Kernel stimmt bytegenau mit dem zuvor symbolisierten R76-Kernel überein. Der Vergleich ist über SHA256 erfolgt. Die Werkzeuge in diesem Verzeichnis verweigern feste Offset-Zuordnungen bei einer anderen Kernel-Prüfsumme.

## 2. Aktiver Callgraph: Nachweis bis zur Netzwerkregistrierung

Die bloße Existenz von `swNic_send` und `New_swNic_init` reicht für die Zuordnung des aktiven Pfades nicht. In diesem Dump ist die Kette aber statisch geschlossen:

| Belegadresse | Nachgewiesene Beziehung |
|---|---|
| `0x8065979c`, `0x80659878–0x8065987c` | `re865x_probe` installiert die Operationsstruktur bei `0x8054d518` in das Netzwerkgerät. |
| `0x8054d520`, `0x8054d524`, `0x8054d528` | Diese Struktur enthält `re865x_open`, `re865x_close` und den TX-Einstieg `0x805288c0`. |
| `0x8024bde0` | `re865x_open` ruft `New_swNic_init` auf. |
| `0x8024bf54` | `tasklet_init` registriert den RX-Callback `0x805291b8`. Die Adresse entsteht aus `lui 0x8053` und `addiu -0x6e48`. |
| `0x8024bf68` | `tasklet_init` registriert den TX-Callback `0x805283f8`. |
| `0x80528cf0` | `re865x_start_xmit` ruft `New_swNic_send`. |
| `0x80528cd0`, `0x80528400` | TX-Wiederholung und TX-Tasklet rufen `New_swNic_txDone`. |
| `0x8052922c` | RX-Tasklet ruft `New_swNic_receive`. |
| `0x8052aa2c` | `New_swNic_send` ruft den eigentlichen Sender `_New_swNic_send`. |

Die symbolisierten älteren Funktionen sind ebenfalls vorhanden:

- `swNic_send` bei `0x80245750` bearbeitet Packetheader und mbuf-Strukturen sowie einen separaten Pointer-/OWN-Ring.
- `swNic_txDone` bei `0x80245a14` wartet auf das OWN-Bit in diesem älteren Ring.

**Diese älteren Funktionen beschreiben nicht die oben registrierte NEW-Paketkette.** Ihr 32-Byte-Packetheader darf nicht mit einem NEW-DMA-Descriptor verwechselt werden. Für den tatsächlich registrierten NEW-Pfad sind Größe und Felder unabhängig davon direkt an den Instruktionen bestätigt.

### Zurückgewonnene Paketfunktionen

| Rekonstruierter Name | Adresse | Größe |
|---|---:|---:|
| `interrupt_dsr_tx` | `0x805283f8` | `0x74` |
| `re865x_start_xmit` | `0x805288c0` | `0x8f8` |
| `interrupt_dsr_rx` | `0x805291b8` | `0xf54` |
| `New_rtl8651_rxPktPreprocess` | `0x8052a234` | `0xb8` |
| `rx_desc_not_empty` | `0x8052a2ec` | `0x7c` |
| `New_swNic_receive` | `0x8052a368` | `0x3a0` |
| `_New_swNic_send` | `0x8052a708` | `0x310` |
| `New_swNic_send` | `0x8052aa18` | `0x40` |
| `New_swNic_txDone` | `0x8052aa58` | `0x138` |
| `New_swNic_freeTxRing` | `0x8052ab90` | `0xe4` |

Diese Namen sind analytische Zuordnungen zum bereitgestellten OEM-GPL-Code, keine neu entdeckten Debug-Symbole. Die Implementierungsmuster, globalen Ringadressen und Aufrufer stimmen mit den jeweiligen Funktionen überein.

## 3. Descriptorformat und Ringgrößen

### Ringkonfiguration aus den tatsächlichen Init-Argumenten

`re865x_open` übergibt folgende Werte an `New_swNic_init`:

| Größe | Statische Quelle | Werte |
|---|---|---|
| TX-Einträge | Tabelle `0x8063369c` | `[768, 2, 2, 2]` |
| RX-Einträge | Tabelle `0x806336ac` | `[900, 2, 2, 2, 2, 2]` |
| RX-Puffergröße | Instruktion `0x8024bdd8` | `0x6a4` = 1700 Bytes |
| TX-Stride | u. a. `0x8052a784`, `0x8052aad0` | 32 Bytes |
| RX-Stride | u. a. `0x8052a404`, `0x8052a324` | 32 Bytes |

Die CPU reserviert und überträgt **acht 32-Bit-Wörter pro Ringplatz**, obwohl die OEM-Proc-Ausgabe nur sechs Nutzwörter ausgibt. Die zusätzlichen zwei Wörter sind die Ausrichtungserweiterung. Bei der TX-Vorbereitung werden alle 32 Bytes genullt; die ausgewertete Senderoutine befüllt die ersten sechs Wörter.

### TX-Felder der kompilierten Routine

`DW0` bezeichnet das erste 32-Bit-Wort; ein Wort wird im Speicher Little Endian abgelegt.

| Wort | Bits | Bedeutung / tatsächlich verwendete Belegung |
|---|---|---|
| DW0 | 0 | OWN, zuletzt über uncached Alias gesetzt |
| DW0 | 1 | Ringende; Ringinitialisierung setzt dieses Bit am letzten Eintrag |
| DW0 | 2, 3 | Last/First; Sender setzt beide |
| DW0 | 4, 5 | Hardware-Lookup, Bridging; aus `nicTx.flags` `0x20`/`0x40` |
| DW0 | 6–22 | Paketlänge einschließlich CRC-Platz: `len < 60 ? 64 : len + 4`, auf 17 Bits begrenzt |
| DW0 | 28 | VLAN-Header vorhanden, bei EtherType `0x8100` |
| DW1 | 0–31 | DMA-Datenadresse aus dem Senderargument |
| DW2 | 0–8 | VLAN-Tag-Portmaske; aktiver Helferwert wird auf `0x3f` begrenzt |
| DW2 | 15–31 | Pufferlänge, derselbe 17-Bit-Längenwert |
| DW3 | 0–11 | VLAN-ID |
| DW3 | 22–24 | Priorität aus dem OEM-Helfer bzw. VLAN-Prioritätszweig |
| DW4 | 24–30 | Ziel-Portmaske, sieben Bits |
| DW5 | 30–31 | Bei Hardware-Lookup setzt das OEM-Binary den Wert **3**. |
| DW6, DW7 | vollständig | In dieser Senderoutine null |

Der verwendete Wert 3 im letzten Feld ist binär belegt. Ein abweichender allgemeiner Kommentar in einer Headerdatei wäre kein Grund, ihn stillschweigend auf 0 zu ändern.

Die hier ausgewertete `_New_swNic_send`-Variante setzt keine L3-/L4-Checksum-Offload- oder TSO-Bits. Die entsprechenden bedingten GPL-Pfade dürfen daher nicht ungeprüft als aktiv vorausgesetzt werden.

### RX-Felder

| Wort | Bits | Bedeutung |
|---|---|---|
| DW0 | 0, 1 | OWN, Ringende |
| DW0 | 16–31 | Pufferkapazität bei Initialisierung |
| DW1 | 0–31 | empfangene Datenadresse; wird dem alten skb zugeordnet |
| DW2 | 0–13 | empfangene Länge einschließlich vier CRC-Bytes |
| DW2 | 20–23 | Ziel-Extension-Portmaske |
| DW2 | 24–25 | Quell-Extension-Port |
| DW4 | 13–15 | Quellportkennung |
| DW4 | 16–27 | VLAN-ID |
| DW4 | 28 | L2-Aktionskennzeichen |
| DW5 | 19–21 | Paketpriorität |
| DW5 | 30, 31 | L4-/L3-Checksum-OK |

Der normale RX-Initwert lautet bei 1700 Bytes `0x06a40001`, am letzten Ringplatz `0x06a40003`. Im Empfangspfad setzt die OEM-Routine beim Rearm nur OWN wieder auf 1 und lässt die anderen Bits des Wortes stehen. Ein später beobachtetes Wort `0x06a4000d` kann daher die durch Empfang gesetzten First-/Last-Bits enthalten; es ist kein Beweis, dass die Initialisierung mit `0x0d` erfolgte.

## 4. TX-Publikation und Completion

Die tatsächliche kompilierten Abfolge ist:

1. Ringvollprüfung: `next(producer) == consumer` beendet die Funktion mit `-1`, ohne einen Descriptor zu veröffentlichen.
2. Direkte Vorbereitung des **cached** Ringdescriptors und Speicherung des zugehörigen skb-Pointers.
3. Cache-Writeback über Funktionspointer `0x80672648` für genau **32 Bytes** (`0x8052a8f8–0x8052a904`).
4. MIPS-`sync` bei `0x8052a908`.
5. Wechsel zum uncached Alias durch OR mit `0x20000000`; anschließend OWN-Bit setzen (`0x8052a90c–0x8052a92c`).
6. Software-Producerindex aktualisieren.
7. `CPUICR` bei `0xb8010000` um TXFD, **Bit 23**, ergänzen.

**Die lokale temporäre TX-Descriptor-Kopie aus einem anderen GPL-Präprozessorzweig ist hier nicht aktiv.** Die Binärdatei wählt den direkten cached Zugriff mit vollständiger 32-Byte-Cachebehandlung. Der übergeordnete TX-Einstieg schreibt außerdem den eigentlichen Paketpuffer in den Cache zurück (`0x80528ca4–0x80528cb8`), bevor er `New_swNic_send` aufruft.

`New_swNic_txDone` liest den Hardware-CDP einmal, berechnet `(CDP - uncached_ring_base) >> 5` und gibt skb-Einträge vom Consumerindex bis zu diesem Index frei. Ein OWN-Test und eine zusätzliche Begrenzung am Producerindex sind in dieser Routine nicht vorhanden. Das unterscheidet sie klar vom älteren `swNic_txDone`.

Der neue `r76_checked_cdp_index` in der C-Referenz prüft Ausrichtung und Ringgrenzen und normalisiert physische/KSEG-Aliase. Er ist eine zusätzliche Schutzfunktion und **kein originaler OEM-Code**. Ein plausibler CDP allein beweist weder korrekte DMA-Sichtbarkeit noch richtige Freigabe bei beschädigtem Ringzustand.

### Register und Initialisierung

| Zweck | Adresse / Wert |
|---|---|
| CPUICR / TXFD | `0xb8010000`, Bit 23 |
| RX-CDP 0–5 | `0xb8010004`, `08`, `0c`, `10`, `14`, `18` |
| TX-CDP 0–3 | `0xb8010020`, `0xb8010024`, **`0xb8010060`, `0xb8010064`** |
| CPUIIMR | `0xb8010028` |
| CPUIISR | `0xb801002c` |
| DMA_CR1 | `0xb8010040`, im Init `(768−1) × 32 = 0x5fe0` |
| DMA_CR4 | `0xb80100a0`, im Init Wert 1 |
| CPUICR1 | `0xb80100a4`; TX-Descriptorgrößenfeld 8 und RX-Descriptorgrößenfeld 8 |

Die TX-CDP-Register sind **nicht** für alle Ringe durch `0xb8010020 + 4 × Ringnummer` darstellbar. Gerade Ring 2 und 3 benötigen die tatsächliche Tabelle. Alle vier TX-Ringbasen werden initialisiert; die Tail-Aware-Konfiguration dieses Builds aktiviert dabei den nachgewiesenen Wert 1.

## 5. Empfang, Extension-Ports und Cachebehandlung

`rx_desc_not_empty` behandelt zwei Fälle: Verschiedene Software-/Hardwareindizes bedeuten verfügbare Arbeit. Sind beide gleich, entscheidet OWN; OWN=0 kann einen vollständig gefüllten Ring anzeigen. Die gleiche Entscheidung ist im RX-Einstieg zu erkennen.

Der Paketempfang verwirft in der ausgewerteten Variante Descriptoren, bei denen nicht beide Checksum-OK-Bits gesetzt sind. Für einen angenommenen Frame wird zuerst ein neuer skb/Puffer bereitgestellt; danach werden Metadaten und Datenadresse des alten Frames übernommen und der neue Puffer in den Descriptor geschrieben. Vor OWN werden im beobachteten normalen Ersatzpufferpfad **1536 Bytes (`0x600`)** ab der neuen skb-Datenadresse invalidiert (`0x8052a5b4–0x8052a5f0`), obwohl die Allokationsgröße 1700 ist. Ob das für weitere Paketgrößen ausreicht, wurde hier nicht auf Hardware geprüft. Dieser konkrete Unterschied gehört in einen Portierungsabgleich.

Die vollständig rekonstruierte Vorverarbeitung unterscheidet normale und Extension-Quellen. Sie liest eine Portmaskentabelle `[5,6,7,5,8,5,5,5]` bei `0x8054d940` und verwendet die speziellen VLAN-Kennungen `0xa530` beziehungsweise `0xa533` für bestimmte bereits übersetzte Extension-Pakete. Solche Kennungen sind Softwaremarkierungen, keine frei einzurichtenden 802.1Q-VLANs. Quellkennung 7 in diesem Descriptorformat darf ebenfalls nicht ohne Betrachtung der jeweiligen Portdomäne mit einem externen Switchport gleichgesetzt werden.

## 6. LAN/WAN: gemeinsame Hardware, getrennte Tabellenrollen

LAN und WAN verwenden denselben CPU-DMA-Netzwerktreiber. Die Trennung erfolgt über VLAN-/Netif-Konfiguration und den Switch. Im Kernel liegt bei `0x806335e8` eine Tabelle mit 44 Bytes pro Eintrag:

| Tabellenname | Rolle | Typ | VID | FID | Mitglied-/Untag-Maske | MTU |
|---|---|---:|---:|---:|---:|---:|
| `br-lan` | LAN | 1 / Ethernet | 1 | 0 | `0x10b`: Bits 0, 1, 3, 8 | 1500 |
| `eth1` | WAN | 1 / Ethernet | 2 | 0 | `0x04`: Bit 2 | 1500 |
| `pppoe-wan` | WAN-Unterinterface | 2 / PPPoE | 2 | 0 | `0x04`: Bit 2 | 1500 |

Diese Werte sind **kompilierte Defaults**. Die User-Space-Konfiguration kann Tabellen im laufenden System ändern. Die dortigen MAC-Adressen sind Platzhalter und werden im Bericht nicht als Geräteidentität verwendet. Der Tabellenname `br-lan` ist eine Netif-Konfigurationsbezeichnung; daraus allein folgt noch kein Beweis, dass der DMA-Treiber ein Linux-Gerät genau dieses Namens registriert.

Die VLAN-Mitgliedermaske `0x10b` und die sieben Bit breite TX-Zielmaske sind verschiedene Felder mit unterschiedlichem Kontext. Die komplette VLAN-Maske darf nicht einfach in das TX-Destination-Feld kopiert werden. Externe RTL8367D-Portnummern und die Beschriftung LAN1/LAN2/LAN3/WAN sind zusätzlich anhand der Switch-Konfiguration zuzuordnen; siehe den Switch-Bericht des Gesamtpakets.

## 7. Verifikation und Grenzen der Emulation

Die C-Datei kompiliert mit `-std=c11 -O2 -Wall -Wextra -Werror`. Die Verifikation führt Original-MIPS-Routinen aus dem unveränderten Kernel in künstlichem RAM aus. Externe Dienste wie `memset`, VLAN-Helfer, Cache-Writeback und skb-Freigabe sind im Emulationsspeicher durch nachvollziehbare Stubs ersetzt. Die getesteten Paketfunktionen selbst sind nicht gepatcht.

| Vergleich | Fälle | Ergebnis |
|---|---:|---|
| C-TX-Descriptor gegen vollständiges `_New_swNic_send` | 1024 | PASS |
| Originaler Ring-voll-Pfad: keine Veröffentlichung/Änderung | 1024 | PASS |
| Vollständige RX-Preprocess-Routine, Port-/Masken-/L2-Kombinationen | 512 | PASS |
| Originale RX-Verfügbarkeitsentscheidung, sechs Ringe | 768 | PASS |
| Originale TX-Completion, Ringwrap und alle vier CDP-Register | 256 | PASS |
| Zusätzlicher neuer CDP-Guard | 7 | PASS |
| Gesamt | **3591** | PASS |

Der TX-Vergleich prüft auch die beobachtete Reihenfolge Cache-Writeback → `sync` → OWN → CPUICR und die Softwareindex-/skb-Zuordnung. Die vollständige `New_swNic_receive`-Routine und ihre Allokations-/Cacheabhängigkeiten wurden nicht in derselben Breite emuliert; ihre Felder und Abläufe wurden statisch ausgewertet. Der vollständige NEW-Ringinitializer wurde ebenfalls statisch ausgewertet.

Emulation beweist keine reale Cachekohärenz, keine korrekte elektrischen RGMII-/PHY-Einstellungen, keinen fehlerfreien IRQ-Betrieb unter Last und keine tatsächliche Übertragung eines Pakets. Die bisherigen TCP-/Längengrenzenfehler des OpenWrt-Ports sind durch diese Arbeit **noch nicht als behoben nachgewiesen**.

### Konkrete Emulatorabweichung

Ein vollständiger originaler TX-Aufruf mit 100 Bytes und aktivem 802.1Q-Passthrough verursacht unter Unicorn 2.1.4 in dieser Umgebung `UC_ERR_EXCEPTION`. Die letzten Adressen sind im Protokoll `unicorn_2_1_4_regression.json` gespeichert; betroffen ist unter anderem die Kette `0x8052a9e4 → 0x8052a9a4 → 0x8052a8b8`. Mit exakt denselben OEM-Instruktionen, Eingaben und Service-Stubs liefert Unicorn **2.0.1.post1** den erwarteten 32-Byte-Descriptor und Returnwert 0. Das zweite Protokoll dokumentiert diesen Erfolg.

Die kurze isolierte Branchkette allein läuft in beiden Versionen. Der kleine Reproducer behält deshalb die umgebende ursprüngliche Senderoutine bei und behauptet keine weiter eingegrenzte Ursache innerhalb des Emulators. Kein fehlschlagender Fachtest wurde deaktiviert. Die Referenzsuite besteht vollständig mit der festgelegten Version. Deren Python-Modul meldet intern `2.0.1`; das installierte Paket heißt `2.0.1.post1`.

## 8. Reproduzieren auf Ubuntu

Im entpackten Projektverzeichnis:

```bash
sudo apt-get update
sudo apt-get install -y python3 python3-venv build-essential
bash ./ethernet/run_verify.sh
```

Das Skript erwartet `kernel/kernel.bin`, erzeugt eine lokale Python-Umgebung, installiert die festgelegten Prüfabhängigkeiten, kompiliert die C-Referenz und schreibt neue Ergebnisse nach `ethernet/verification_local.json` und `ethernet/unicorn_regression_local.json`. Bei einem anderen Speicherort kann der vorhandene Kernelpfad als erstes Argument übergeben werden.

**OpenWrt-Router:** Für diese Offline-Reproduktion ist dort kein Befehl und keine Änderung erforderlich. Ein echter Routertest gehört zur anschließenden Integration eines konkreten OpenWrt-Builds; dieses Paket enthält kein flashbares Ethernet-Testimage.

## 9. Konsequenzen für `rtl8197f_rtknet.c`

Der nächste konkrete Quellcodeabgleich kann jetzt die folgenden Punkte gegen nachgewiesene Binärwerte prüfen:

1. RX-/TX-Stride durchgehend 32 Bytes, passende Descriptorgrößenfelder und Ringadressen; sechs ausgegebene Nutzwörter sind keine 24-Byte-Ringgröße.
2. Byte- und Bitlayout wie oben, insbesondere 17-Bit-TX-Längen, RX-Länge in DW2, Zielportmaske in DW4 und RX-Checksumstatus in DW5.
3. Payload-Sichtbarkeit sowie vollständige Descriptor-Sichtbarkeit vor OWN; OWN erst nach der Barriere veröffentlichen.
4. TX-Freigabe am richtigen CDP-Register und korrektem Ringindex; robuste Grenzen zusätzlich sichern.
5. RX-Init und Rearm getrennt behandeln; die beobachteten First-/Last-Bits nicht mit einer Initialisierungsvorgabe verwechseln.
6. VLAN-/Netif-Defaults und Portmasken nicht zwischen interner NIC, Extension-Ports und externem Switch vermischen.
7. Die real kompilierten Pfade übernehmen, keine unpassende lokale-Descriptor-, Checksum- oder TSO-Variante aus anderen SDK-Konfigurationen voraussetzen.

Die neue C-Referenz und die Originalinstruktionen machen diesen Abgleich konkret prüfbar. Eine Behauptung, alle Linux-4.4-OEM-Funktionen seien damit automatisch für OpenWrt/Linux 6.6 portiert, wäre durch die vorliegenden Ergebnisse nicht gedeckt.
