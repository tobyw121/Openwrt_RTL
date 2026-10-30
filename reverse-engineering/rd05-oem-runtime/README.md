# RD05: hochgeladene OEM-Laufzeitdaten

Ausgewertet: `OEM_Infos_rd05.txt`, `rd05-proc-rtl865x.tar.zst` und
`rd05-oem-hw-dump.tar.gz`. Dateigrößen, SHA256 und die aus den Ring-Dumps
ermittelten Werte stehen in [evidence.json](evidence.json).
Die Archive wurden ausschließlich als Daten gelesen; enthaltene Befehle
wurden nicht ausgeführt. Vollständige UCI-/Factory-Ausgaben bleiben privat.

## Was diese Daten zusätzlich belegen

Der Bootlog nennt Linux 4.4.176, `8197FH-VG`, `switchChip=8367D`, die PCI-ID
`f81210ec` (10ec:f812) und 128 MiB RAM. Der Hardware-Dump bezeichnet das
System als OpenWrt 18.06-SNAPSHOT. Die Version des WR1500-Images darf deshalb
nicht auf diesen RD05 übertragen werden.

Die OEM-Schnittstellen sind keine Linux-DSA-Conduit-/User-Port-Struktur.
Im Hardware-Dump steht beispielsweise `eth0` auf null RX/TX, während `eth1`
260 RX- und 117 TX-Pakete meldet. Das widerspricht nicht dem OEM-Verkehr:
Die Interface-Namen haben dort andere Aufgaben als in unserem Port.
Die unabhängig erfassten Dateien sind keine gemeinsame atomare Messung.

## DMA: tatsächliche Ringinhalte statt nur SDK-Annahmen

| Eigenschaft | OEM-Snapshot |
| --- | --- |
| RX-Ring 0 | 2.048 Einträge |
| weitere RX-Ringe 1–5 | je 2 Einträge |
| TX-Ring 0 | 768 Einträge |
| weitere TX-Ringe 1–3 | je 2 Einträge |
| Deskriptorabstand | durchgehend 32 Bytes |
| ausgegebener Deskriptorinhalt | 6 Datenwörter pro Eintrag |
| RX-Puffergrößenfeld | 1.700 Bytes |
| RX-Ring-0-CDP | `0xa4e4c380`, Index 1564 |
| TX-Ring-0-CDP | `0xa4e6dc40`, Index 738 |

Die CDP-Adressen passen zur Basis und zum 32-Byte-Abstand: RX
`0xa4e40000 + 1564 * 32`, TX `0xa4e68000 + 738 * 32`.
Der OEM gibt sechs Wörter aus, verwendet aber einen 32-Byte-Abstand.
Eine Umstellung unseres Treibers auf einen 24-Byte-Abstand wäre falsch.

In allen 768 TX-Ring-0-Einträgen sind FIRST/LAST gesetzt, HWLOOKUP/BRIDGE
gelöscht, DVID ist 9 und das rohe DP-Feld ist **0x2**. PH_LEN und M_LEN sind
in allen 774 ausgegebenen TX-Einträgen gleich. Ein Beispiel:

```text
0x00017b8c 0x84cbc8ce 0x02f70000 0x00000009 0x02000000 0x00000000
```

Mit den SDK-Feldpositionen ergibt das PH_LEN=M_LEN=1518. Die Angabe, dass
die Hardware FCS ergänzt, stammt weiterhin aus `_New_swNic_send`; die
Ring-Metadaten allein enthalten weder Nutzdaten noch eine unabhängige
Messung der ursprünglichen Paketlänge.

**Zwei konkrete Vergleichspunkte für den aktuellen Treiber:**

- Der RD05-DSA-Pfad verwendet DP=0x1 und transportiert das externe Ziel im
  CPU-Tag; der OEM-Ring zeigt DP=0x2. Im ebenfalls gelieferten HSA-Auszug
  stehen `dp=0x1` und `rmdp=0x2`. Rohes Descriptor-DP, remappter Port und
  physischer Host-Link dürfen daher nicht ohne OEM-Codeabgleich gleichgesetzt
  werden. Der HSA-Auszug muss nicht dasselbe Paket wie ein Ring-Eintrag zeigen.
- OEM-Pufferadressen im aktiven TX-Ring liegen bei `0x8...` (KSEG0), die
  CDP-Werte bei `0xa...` (KSEG1). Unser gemeinsamer DMA-Adresshelfer liefert
  bei `desc_addr_kseg1=1` auch für TX-Puffer `0xa...`. Das ist eine belegte
  Darstellungsabweichung, aber noch kein Nachweis, dass der DMA-Core diese
  Aliasse unterschiedlich behandelt. Cache-/Busadressierung muss geprüft
  werden, bevor die Linux-DMA-API-Verwendung geändert wird.

Mehr Deskriptoren oder ein anderer Pufferumfang allein erklären nicht, warum
unser Core überhaupt kein einziges Paket empfängt. Die OEM-Geometrie ist eine
Referenz, kein Anlass, die Ringgrößen auf Verdacht zu erhöhen.

## VLAN, Switch und Zählerdomänen

Der SoC-Snapshot zeigt VID9 mit Mitgliedern 0–4 und 8 (Maske 0x11f), VID8
mit Port 4 und FID1 sowie die LAN-Netif auf VID9. Der serielle Auszug zeigt
am externen Switch PVID8 für Port2 und PVID9 für Ports0/1/3/4 sowie eigene
FID-/Isolationsregeln. Diese SDK-Tabellen lassen sich nicht unverändert als
DSA-Portmasken übernehmen. Insbesondere sind SDK-Masken wie `0x20000`
nicht ohne HAL-Zuordnung ein physischer CPU-Port.

`stats.txt` enthält 1552 Software-Core-RX-Ereignisse und 3728 TX-Ereignisse.
`asicCounter.txt` zeigt am SoC-Port0 156048 Empfangs- und 2606706 Sendebytes
bei CRCAlignErr=0. Gleichzeitig meldet dessen **interner CPU-Port**
CRCAlignErr=2271. Damit darf dieser OEM-CPU-Zähler nicht als direktes Pendant
zu `cpu_fcs_errors` aus unserem externen RTL8367-DSA-Treiber behandelt werden.
Der OEM-Snapshot beweist weder eine kaputte physische CPU-Leitung noch, dass
man CRC-Prüfungen im OpenWrt-Port abschalten sollte.

## Was für einen nachgewiesenen Fix noch fehlt

Die seriellen `devmem`-Aufrufe haben in der gelieferten Datei keine numerischen
Registerausgaben; Fehler wurden mit `2>/dev/null` ausgeblendet. Das Archiv
enthält außerdem leere `memory.txt`, `phyReg.txt` und `mbufRing.txt` sowie
leere swconfig-Ausgaben. Konkrete OEM-RGMII-/SSC-Registerwerte und die
CPUICR/CPUICR1-/DMA-Register sind damit nicht erfasst.

Die Dateien sind eine deutliche Verbesserung der Beweislage: Jetzt lassen
sich Descriptor-Felder, Adressformen und Zielport-Remapping gegen einen
wirklich verwendeten OEM-Zustand vergleichen. Sie belegen jedoch noch
keine einzelne Ursache oder reparierten LAN-/WAN-Verkehr unter Linux 6.6.
Deshalb ändert dieser Befundbericht keine Laufzeit-, DSA- oder CRC-Defaults.
