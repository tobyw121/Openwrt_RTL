# Realtek-Beschleunigung unter OpenWrt 24.10 / Linux 6.6.114

Portierungsstand: Ein optionaler, firewall4-gesteuerter Linux-Software-Offload-
Modus und bestehende Hardware-Paketassists sind eingebunden. Das autonome
Realtek-Hardware-Routing/NAT ist noch nicht vollständig portiert und bleibt
gesperrt. Details und Gerätegrenzen stehen unter „Bedienung und Prüfung“.

## Umfang und verbindliche Policy

Diese Bestandsaufnahme betrifft die RTL8197F/FS/FH-, RTL865x- und RTL83xx-
Mechanismen im gelieferten Cudy-GPL-Paket und unserem `MW5_Stable`-Zweig.
Sie ist kein vollständiger Katalog aller Realtek-Produktfamilien. Die 15
GPL-Quellgruppen mit überprüften Pfaden und SHA256 stehen in
[acceleration-inventory.json](../../../../reverse-engineering/gpl-ax1500/acceleration-inventory.json).
Die weiteren WLAN-Einträge beziehen sich auf den bereits vorhandenen Treiber;
ein zusätzlicher vollständiger WLAN-Quellbaum ist in dieser GPL nicht enthalten.

**Strikte Standardpolicy:** Gerouteter Verkehr soll den regulären Linux-/Netfilter-
Pfad behalten. Kein privater Realtek-Cache und keine autonome Hardware-NAT-
Tabelle darf die Weiterleitungsentscheidung übernehmen. DMA-, Queue- und
Paketverarbeitungshilfen sind davon getrennt. firewall4 verwaltet weiterhin
die nftables-Regeln, Conntrack den Verbindungszustand und NAT die Übersetzung.
Die normalen Linux-NAT-Chains treffen ihre NAT-Entscheidung dabei wie üblich
am ersten Paket; diese Dokumentation verspricht keine NAT-Regelauswertung
für jedes einzelne Paket einer Verbindung.

Standard-OpenWrt-Flow-Offloading ist nicht grundsätzlich eine unsichere
Firewallumgehung: Die Aufnahme einer Verbindung erfolgt nach Firewallprüfung.
Folgepakete nutzen jedoch einen früheren Flowtable-Pfad und durchlaufen
nicht mehr alle normalen Forward-Hooks. Das verändert unter anderem die
Beobachtung durch Zähler, Logging und bestimmte Paketregeln. Wegen der hier
geforderten vollständigen regulären Firewallverarbeitung wird auch dieses
Offloading in der strikten Standardkonfiguration abgeschaltet. Der nachfolgend
beschriebene optionale Software-Modus nutzt stattdessen bewusst die normale
OpenWrt-Flowtable-Semantik mit Firewallprüfung bei der Aufnahme einer Verbindung.
Die proprietären Treiber-Fastpaths bleiben in beiden Modi gesperrt.

## Software-Weiterleitung und private Hooks

| Mechanismus | Funktion / GPL-Referenz | Anpassung für unseren Zweig |
| --- | --- | --- |
| Realtek IPv4 FastPath / Software-NAPT | `net/rtl/fastpath/core/fastpath_core.c`: eigener Verbindungs-/NAT-Cache, verkürzte Ausgabe | Nicht importiert; Linux-Routing, Conntrack und nftables verwenden |
| Realtek IPv6 FastPath | `fast_ipv6_core.c`: beschleunigte IPv6-Weiterleitung | Nicht importiert; IPv6, soweit im Geräteimage enthalten, bleibt im normalen Stack |
| Bridge Shortcut | `fast_br.c`, außerdem `CONFIG_RTL_NO_BR_SHORTCUT` / `RTL_BRSHORTCUT_LINUX_VLAN_CTL` im BSP | Kein privater LAN/WLAN/WAN-Shortcut; normale Linux-Bridge-/DSA-Steuerung |
| PPPoE FastPath | `fast_pppoe_core.c`: PPPoE-spezifische Weiterleitung | Normale Linux-PPP-/PPPoE-Verarbeitung, kein privater Cache |
| PPTP / L2TP FastPath | `fast_pptp_core.c`, `fast_l2tp_core.c` | Normale Tunnel-/Routing-Verarbeitung; kein paralleler proprietärer Tunnelpfad |
| Realtek Filter-/Conntrack-/NAT-Hooks | `rtl_ps_hooks.c`, `filter_ipv4.c`, `filter_ipv6.c`, `filter_v2.c` | Keine Übernahme von Linux-4.4-Conntrack-Interna oder iptables-Tabellenaufrufen in firewall4 |
| Unser MW5 Exact-Flow-Cache | RX-Cache mit NAT-/MAC-Umschreibung und direktem `dev_queue_xmit()` | Unter strikter Policy im Kernel gesperrt, auch bei alten Aktivierungsargumenten |
| Linux Software-Flowtable | Standard `nf_flow_table`, nicht Realtek-spezifisch | Standard aus; expliziter, firewall4-gesteuerter Software-Modus mit Vorprüfung und Rückkehr zum strikten Modus |

Der GPL-Fastpath ruft stellenweise `rtl_ipt_do_table()` und private Hook-
Hilfen auf (`fastpath_core.c`, u. a. um Zeile 1163). Das bildet weder die
heutigen nftables-Regeln noch deren Prioritäten, Familien, Sets, Flow-Lebensdauer
und vollständige Hook-Reihenfolge nach. Eine reine API-/Compiler-Portierung
dieses Codes wäre daher keine ausreichende Firewallintegration.

Im vorhandenen WLAN-Treiber entfernt der Linux-6-Build bereits die alten
Bridge-, Fastbridge-, Extension-Port- und WLAN-HWNAT-Defines am Ende von
`8192cd_cfg.h`. Linklokales TX-/RX-Deskriptor-Reuse ist davon getrennt.
Der zusätzliche RX-Aufruf unseres eigenen MW5-Caches wird nun außerdem an
seinem exportierten Kernel-Einstieg abgewiesen.

## Hardware-Weiterleitung, Switch und Klassifizierung

| Mechanismus | Funktion | Entscheidung / Linux-6.6-Schnittstelle |
| --- | --- | --- |
| RTL865x L3/L4 Routing und Hardware-NAPT | ASIC-NAT-/Routing-/Next-Hop-/IP-Tabellen; Referenzen `rtl865x_nat.c`, `rtl865x_asicL4.c` | Strikt gesperrt. Der historische exakte `TC_SETUP_FT`-Encoder bleibt als Referenz im Quellcode, kann aber keine neue Regel installieren |
| RTL865x L2 FDB, VLAN, Netif, ACL zur CPU-Anbindung | Transport zwischen physischem Host-Port, CPU-DMA und externem Switch | Bestehende Boardinitialisierung bleibt erhalten. CPU-Traps und Portzuordnung sind keine eigenständige NAT-Beschleunigung |
| RTL836x L2 Switching / FDB / VLAN / STP | Switchinterne Weiterleitung zwischen Bridge-Ports | Linux DSA/Bridge muss Eigentümer bleiben; keine autonome WAN-LAN-Brücke einrichten |
| CPU-Tag / Extension Ports / WFO | Zuordnung externer Ports und direkte WLAN-/Switch-Verknüpfung | DSA-Tagger für regulären CPU-Transport; alte WFO-/WLAN-Extension-Weiterleitung bleibt abgeschaltet |
| Hardware ACL / Trap / Mirror | Frühzeitiges Filtern, CPU-Traps, Spiegeln | Vorhandene tc-flower-Anbindung meldet nicht unterstützte Hardware-Programmierung explizit. Keine private ACL als Ersatz für nftables |
| Multicast / IGMP / MLD Snooping | Lokale Gruppenverwaltung und Switch-Replikation | Linux-Bridge-/DSA-Semantik und VLAN-Grenzen beachten; kein ungeprüftes geroutetes Multicast-Offloading |
| Hardware QoS / Queues / Prioritäten / Rate Limits | ASIC-Prioritäts- und Queue-Verarbeitung | Bestehende Diagnosefunktionen bleiben gerätespezifisch und standardmäßig aus; keine automatische Übersetzung von SQM/CAKE in SDK-Register |

L2-Verkehr zwischen LAN-Ports derselben Bridge durchläuft nicht automatisch
die gerouteten `inet`-Forward-Regeln. Das ist auch ohne Realtek-Fastpath die
normale Trennung zwischen Bridge und Router. Wer Regeln für jedes lokale
Ethernet-Frame benötigt, muss die Bridge-Firewall und ein mögliches DSA-L2-
Offloading separat prüfen. Die hier implementierte strikte Policy betrifft
die eigenen gerouteten Weiterleitungsbeschleuniger; sie ersetzt keine
VLAN-/Portisolationsprüfung und keine Prüfung fremder eBPF-/tc-Programme.

Ein TX-Deskriptor darf außerdem nach einer bereits erfolgten Linux-
Weiterleitungsentscheidung einen normalen L2-Egress-Lookup verwenden. Das ist
von einem autonomen ASIC-Ingress-L3/L4-Lookup zu unterscheiden. Deshalb werden
nicht blind alle Register und Bits mit dem Namen `HWLOOKUP` gelöscht.

## Hilfen, die keine Routingentscheidung ersetzen

| Mechanismus | Nutzen | Status und Grenze |
| --- | --- | --- |
| Coherent-/Uncached-DMA, Deskriptorringe | DMA-Eigentümerschaft, weniger Cache-Alias-Probleme | Vorhandene boardbezogene Implementierung; MW5 hat einen eigenen coherent RX-Pfad. Keine zusätzliche Aktivierung durch diese Änderung |
| `page_pool` / RX-Recycling | Weniger Speicherallokationen | Vorhandener optionaler Pfad, kein neues Standardfeature. Cache-/DMA-Synchronisation benötigt Hardwaretests |
| NAPI / Poll-Budget / IRQ-Mitigation | Paketverarbeitung bündeln, IRQ-Last begrenzen | Linux-NAPI beibehalten; Ring-/Timerwerte sind gerätespezifisch. Übermäßiges IRQ-Sperren kann UART und Netzwerk stören |
| BQL / TX-Completion | Begrenzung gepufferter TX-Bytes | Normale Linux-Netdev-Buchführung, keine Firewallabkürzung |
| TX-/RX-Checksum-Offload | Prüfsummen berechnen bzw. Metadaten bereitstellen | Vorhandene Netdev-Funktionen; nur korrekt beschriebene Protokolle und validierte RX-Metadaten akzeptieren. MW5-Stabilitätsvorgaben bleiben aus |
| Scatter/Gather | Mehrere skb-Fragmente per DMA statt Kopieren | Vorhandener opt-in mit Fragmentgrenzen/Fallback; benötigt DMA-/Cache-Tests |
| TSO / GSO / GRO / LRO | Segmentieren bzw. Zusammenfassen | Linux-GSO/GRO-Semantik verwenden; proprietäres LRO nicht pauschal portieren. Paketgrößen, Zähler und SQM-Verhalten müssen separat geprüft werden |
| WLAN A-MPDU / A-MSDU / Reordering / Rate Control | Effizientere Funkübertragung | Linklokale Treiberfunktionen, kein gerouteter NAT-Cache. Keine neuen Chips oder Funktionen durch dieses GPL-Paket nachgewiesen |
| WLAN TX-/RX-Shortcut | Kann nur Treiberarbeit sparen oder aber den Netzwerkstack überspringen | Pro Pfad prüfen: Deskriptor-/Header-Reuse ist anders als WLAN→WAN-Direktweiterleitung. Unser exportierter MW5-RX-Fastpath nimmt unter strikter Policy kein skb entgegen |
| WLAN Hardware-Verschlüsselung | Funkdaten verschlüsseln | Vorhandene Funkchip-/Key-Implementierung; nicht durch falsche Factory- oder Schlüssel-Tabellen anderer Boards ersetzen |
| Crypto-/IPsec-Engine | Im GPL-Bootcode enthaltene `crypto_engine/rtl_ipsec.c`-Referenz | Kein neu portierter Linux-Crypto-/XFRM-Treiber. Eine spätere Crypto-API-Portierung muss Schlüssel-/DMA-Lebensdauer prüfen und darf keine autonome Tunnel-Weiterleitung einschalten |

Diese Funktionen sind nicht automatisch auf jeder Variante vorhanden oder
stabil. Ein im SDK vorhandener Schalter ist kein Hardware-Nachweis. Auch
ein im Code vorhandener optionaler Pfad ist noch kein erfolgreicher Port.

## Implementierte Schutzmaßnahmen

Ab Treiberversion `1.8.40-strict-firewall` gilt:

1. Eine zentrale, nicht zur Laufzeit abschaltbare Policy sperrt eigene
   geroutete Fastpaths. Es gibt keinen Schalter `strict=0`.
2. `mw5_accel_sw_enable=1` und `rtl8197f_hw_napt_enable=1` werden abgewiesen;
   Ausschalten bleibt zulässig. Alte positive Modulargumente müssen entfernt
   werden, insbesondere vor einem experimentellen Modulbuild.
3. Die privaten Proc-Befehle `accel-sw-on` und `napt-on` werden abgewiesen.
4. Der Software-Cache kann keine Flows installieren und kein RX-skb
   übernehmen. Das originale skb bleibt für den normalen Linux-Pfad erhalten.
5. Eigene Hardware-Flowtable-Bindings werden mit `-EOPNOTSUPP` abgewiesen.
   Die ASIC-L3/L4-Aktivierungsfunktion hält L3/L4 aus, auch bei einem
   unerwarteten internen Aktivierungsversuch. Cleanup bleibt vorhanden.
6. `/proc/rd05-rtknet` meldet
   `firewall_policy mode=strict routed_shortcuts=0 tc_flowtable_hw=0 dma_path=linux`.
7. Eine neue UCI-Defaults-Datei setzt auf den unterstützten Routerprofilen
   beide firewall4-Flow-Offload-Optionen auf null. Sie ersetzt keine Zonen,
   Weiterleitungen, NAT-Regeln oder Benutzerregeln.

Die bisherigen `mw5-accel hardware on`-/`hw=1`-Bezeichnungen betreffen in
diesem Modus Paketverarbeitungshilfen, **kein aktiviertes Hardware-NAT**.
Checksum/SG/TSO/QoS werden dadurch nicht pauschal freigeschaltet.

## Bedienung und Prüfung

### Strikter Standardmodus

Nach einem Build mit diesen Änderungen:

```sh
rtl8197f-accel status
```

Eine vorhandene Installation ausdrücklich auf die strikte UCI-Policy setzen:

```sh
rtl8197f-accel enforce
rtl8197f-accel verify
```

`enforce` ändert nur die beiden Flow-Offload-Optionen, speichert sie und lädt
die normale Firewall neu. `defaults` speichert dieselben Optionen ohne Reload
und ist für den ersten Boot vorgesehen. Das Werkzeug schaltet keinen
Beschleuniger ein und verändert weder WAN/LAN-Zuordnung noch Funkparameter.

`verify` prüft die UCI-Werte und einen vorhandenen Forward-Filter-Hook in
`table inet fw4`, sucht im aktiven nftables-Regelwerk nach
`flow add @...`/`flow offload @...` und verlangt die neue Treiber-Policyzeile.
Eine nicht abfragbare nftables-Konfiguration oder ein alter/nicht geladener
Treiber ist kein PASS. Eine unbenutzte Flowtable allein gilt nicht als aktiver
Flow-Offload-Aufruf. Selbst angelegte Flow-Regeln werden gemeldet und nicht
ungefragt aus fremden Tabellen entfernt.

Ein PASS ist ein Konfigurationsnachweis, kein Beweis für funktionierende
Verkabelung, VLAN-Isolation oder Internetzugang. Auf den Cudy-UART-Profilen
ist der Ethernet-Treiber deaktiviert; dort ist ein vollständiges PASS noch
nicht zu erwarten.

### Optionaler Software-Flowtable-Modus

Die Portierung des Software-Weiterleitungspfads verwendet die vorhandene
Linux-6.6-Implementierung statt eines zweiten Realtek-NAT-Caches. Das optionale
Paket `rtl8197f-flow-offload` zieht `firewall4` und `kmod-nft-offload` mit deren
Kernel-Abhängigkeiten ein. Es installiert keine eigenen Firewallregeln und
aktiviert beim Installieren keinen Beschleuniger. OpenWrts `kmod-nft-offload`
setzt das IPv6-Buildfeature voraus; das bedeutet nicht, dass WAN-IPv6 aktiviert
wird. Auf kleinen Flash-Profilen ist das Paket nicht standardmäßig ausgewählt.

Auf einem unterstützten, bereits funktionierenden Router mit dem neuen Treiber
und geladenen Flowtable-Modulen:

```sh
rtl8197f-accel software on
rtl8197f-accel verify
```

Zurück zur vollständigen regulären Paketverarbeitung:

```sh
rtl8197f-accel software off
```

Die Aktivierung verlangt einen aktiven firewall4-Forward-Hook, den neuen
Treiber-Policynachweis, geladene `nf_flow_table_inet`-/`nft_flow_offload`-Module
und ein als `up` gemeldetes WAN. Sie setzt `flow_offloading=1`, hält
`flow_offloading_hw=0` und prüft das Ergebnis mit `fw4 check` vor dem Reload.
Bei einem Konfigurations-/Prüf-/Reload-Fehler werden die bisherigen beiden
Offload-Optionen wiederhergestellt; nach einem Reload-Fehler wird zusätzlich
ein Reload mit den alten Optionen versucht. Ein Fehler bleibt ein Fehler und
wird nicht als erfolgreicher Rollback oder erfolgreicher Offload ausgegeben.

firewall4 generiert die Flowtable-Regeln; Conntrack/NAT/Flow-Lebensdauer bleiben
im Standard-Linux-Backend. `verify` unterscheidet diesen Modus vom strikten
Modus und verlangt hier vorhandene Flow-Regeln in `table inet fw4` sowie
weiterhin erfüllte Geräte-/Backend-Voraussetzungen. Eine fremde Flow-Regel
allein reicht nicht aus. Ein PASS bedeutet nicht,
dass schon Pakete beschleunigt wurden oder die Durchsatz-/Sicherheitstests
bestanden sind. Flow-Admission kann nicht durch unsere alten Proc-NAT-Befehle
oder durch einen parallelen privaten Cache ersetzt werden.

**Gerätegrenzen:** MW5 bleibt wegen der bisherigen Speedtest-/Lastprobleme
gesperrt. Die Cudy-UART-Profile haben noch keinen Ethernet-Datenpfad. Beim RD05
verhindert das fehlende WAN die Aktivierung. Für Regeln, die jedes Paket sehen
müssen, sowie ungeprüfte SQM-/CAKE- oder Spezialkonfigurationen bleibt der
strikte Modus die passende Einstellung. Dieser Software-Modus ist kein
portierter proprietärer Realtek-Fastpath und kein Hardware-NAT.

### Hardware-Port: konkreter Stand und fehlende Teile

DMA, NAPI/BQL, die vorhandenen Netdev-Paketassists und die TC-Anbindung sind
bereits Linux-6.6-Code im Zweig. Der historische Hardware-NAPT-Encoder ist
jedoch **kein vollständiger funktionsfähiger Hardware-Routing-Port**. Neu ist
seine strengere Übersetzung der von Linux-6.6-Flowtable erzeugten Aktionen:

- TCP-/UDP-Portedits akzeptieren ausschließlich die zwei vollständigen,
  ausgerichteten Portmasken von `nf_flow_table_offload.c`; unbekannte Masken,
  unpassende Offsets und Werte in beizubehaltenden Bits werden abgewiesen.
- IPv4-Adressedits verlangen einen vollständigen Austausch ohne Teilmaske.
- Ethernet-MAC-Edits werden nicht mehr ignoriert. Ohne vollständigen ASIC-
  Next-Hop-/MAC-Encoder wird die Regel abgewiesen und bleibt im Linux-Pfad.

Deshalb gibt es weiterhin keine Hardware-Aktivierung. Noch erforderlich sind
insbesondere vollständige ARP-/Next-Hop-/Netif-/MAC-Abbildung, verifizierte
Hash-/Byteordnungs-/Kollisionsbehandlung am Chip, Aktivitäts-/Timeoutabgleich,
Löschung bei Conntrack-/Route-/Nachbar-/Firewall-/Netdev-Änderungen und eine
nachgewiesene WAN/LAN-/DSA-Isolation. Erst dann darf eine Regel erfolgreich als
Hardware-offloaded bestätigt werden. Die bestehende Lösch-/Flush-Infrastruktur
und Native-Tests sind dafür Grundlagen, kein Ersatz für den Hardware-Nachweis.

`rtl8197f-accel hardware on` wird daher ausdrücklich mit einem Fehler beendet.
Kein Schalter wird angeboten, der diese fehlende Portierung nur übergeht.

## Hardware-Abnahme vor weiterer Leistungsoptimierung

Für MW5, AC23, RD05 und WRD12GN getrennt prüfen: LAN/WLAN→WAN und Antworten,
DHCP/ARP/DNS, TCP/UDP sowie unterstütztes IPv6, Portweiterleitungen und
verbotene WAN→LAN-Verbindungen. Mit temporären eigenen Testregeln müssen
Zähler/Logging und ein Regelwechsel während einer Verbindung überprüft
werden. Conntrack-Löschung, Firewall-Neuladen, Linkwechsel und MTU-/Fragment-
Fälle gehören ebenso dazu wie Last bei gleichzeitig laufender UART-Konsole.
Keine belastende Aktivierung auf Verdacht und keine Abschaltung von CRC-
Prüfungen, um einen kaputten Datenpfad zu verdecken.

Der RD05 hat weiterhin einen ungeklärten CPU-Link-/FCS-Fehler. Diese Änderung
behauptet weder dessen Behebung noch eine gemessene Durchsatzsteigerung.
Ein vollständiger Firmwarebuild, das MW5-Imagebudget und Hardwaretests müssen
noch geprüft werden. Bereits abgeschaltete instabile Fastpaths bleiben aus.
