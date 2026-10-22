# Cudy AX1500 GPL: Bewertung für Openwrt_RTL

Stand: 2026-10-22, untersuchter Arbeitsstand `ddf39b1d2f` auf `MW5_Stable`.

Das Paket ist als zusätzliche Ethernet-/Switch-Referenz brauchbar. Es enthält
einen umfangreichen RTL8197F-BSP mit RTL83xx-API, aber keinen nachgewiesenen
neuen vollständigen WLAN-Treiber für MW5, AC23 oder RD05. Die Untersuchung
umfasst das Archivverzeichnis, die Boardkonfigurationen und gezielt die
NIC-, P0/RGMII- und RTL8367D-Pfade; sie ist keine vollständige Prüfung aller
enthaltenen Dateien. Es wurde kein Treiberverhalten aufgrund dieses Berichts
geändert und kein Hardwarefehler als behoben eingestuft.

## Herkunft und Wiederauffinden

- [Vom Benutzer angegebener Download](https://drive.usercontent.google.com/open?id=1sqW7x8sLHnSitW9ZXxgsrVN3lprFLp45&authuser=0)
- Archiv: `GPL_for_AX1500_rtl8197.tar.gz`, 146.030.155 Bytes.
- SHA256: `2413dbd852e6f039d402f8ffda62e20b9f7b88a38dde0e7d86a527078ed66b93`.
- 27.989 Archiveinträge; 25.265 reguläre Dateien mit zusammen 466.832.957 Bytes.
- `openwrt/Readme.txt` nennt Cudy WR1500, RE1500 und M1500.
- Quellwurzel im Archiv: `GPL_for_AX1500_rtl8197/openwrt/21.02/`.

Das Original und die extrahierten Dateien liegen in der Analyseumgebung unter
`/workspace/rtl-build-validation/gpl-review/`. Nur reguläre Dateien wurden
extrahiert; enthaltene Programme und Buildskripte wurden nicht ausgeführt.
Archiv, Toolchains, Factory-Abbild und Herstellerkonfigurationen werden nicht
in unseren Firmwarezweig importiert. [evidence.json](evidence.json) enthält
Dateihashes, Quellstellen und Vergleiche gegen einen festgehaltenen Git-Stand.

## Plattform und Übertragbarkeit

Die Buildbasis ist OpenWrt 21.02 mit Vendor-Linux **4.4.176**
(`target/linux/realtek/Makefile:19`). Ein direkter Austausch unseres
Linux-6.6.114-Treibers durch diesen Code wäre keine fertige Portierung.
Nützlich sind Registerdefinitionen, Initialisierungsfolgen und Deskriptorregeln.

| Profil | Produkt laut Readme | Kernelkonfiguration | Nutzen |
| --- | --- | --- | --- |
| R76 | WR1500 | RTL8197F, VG/V672, RTL8367RB-VC | Vergleich externer Switch am SoC |
| R101 | RE1500 | RTL8197F, VG/V672, RTL8367RB-VC ausgeschaltet | Gegenprobe ohne diesen externen Switch |
| R102 | M1500 | RTL8197F, VG/V672, RTL8367RB-VC | Zweites Profil für externe Switch-Anbindung |

Die tatsächlich beigelegten `rtl8197f/*/config-4.4` setzen
`CONFIG_CPU_LITTLE_ENDIAN=y` (Zeile 225). Eine abweichende übergeordnete
`CONFIG_BIG_ENDIAN`-Auswahl in `oemconfig/R76` ist daher kein Beleg für die
Byteordnung des ausgelieferten Kernels oder seiner DMA-Deskriptoren.
Produktname und ähnlich benannte Profile beweisen keine identische Platine,
Portzuordnung, RF-Beschaltung oder Kalibrierung unserer drei Geräte.

## Für den RD05 besonders relevant

Die folgenden Pfade beziehen sich auf
`target/linux/realtek/files-4.4/drivers/net/rtl819x/`.

1. **P0/RGMII und CPU-Tag-Modus:**
   `AsicDriver/rtl865x_asicL2.c:7183`, Funktion `init_8197f_p0`.
   Das SDK setzt CPU-Tag RX/TX, `PORT0_ROUTER_MODE` und den festen
   8-Byte-TX-IPG im selben `CONFIG_RTL_CPU_TAG`-Block. Der IPG wird zusätzlich
   durch `IS_8197F_VG()` begrenzt. Unser RD05-Code schaltet die drei
   Tag-/Router-Steuerungen für transparenten DSA-Transport ab, aktiviert
   den festen IPG bei Chip-ID-Maske `0x81970000` aber weiterhin.
   **Das ist eine konkrete Abweichung und ein Kandidat für einen isolierten
   Vergleichstest, kein bewiesener CRC-Fix.** Einfaches Auslassen der Writes
   reicht als Test nicht, wenn der Bootloader bereits feste IPG-Werte setzt.
   Zuerst müssen Normalmodus und Ausgangsregister unabhängig abgesichert werden.

2. **RGMII-Delay vor Force-Link:**
   `rtl83xx_v1dot4/dal/rtl8367d/dal_rtl8367d_port.c:3138` dokumentiert, dass
   `rgmiiDelayExt_set` vor `macForceLinkExt_set` aufzurufen ist. Der Delay liegt
   im unteren Nibble von `EXT1_RGMXF`: TX Bit 3 (0/2 ns), RX Bits 2:0 (acht
   Stufen). Das ist von SoC-RX-Delay und zusätzlichen TXC-Taps zu unterscheiden.
   Die komplette Reihenfolge einschließlich späterer Link-/Timing-Writes
   bleibt ein sinnvoller Prüfpunkt für unseren DSA-Treiber.

3. **SSC ist bekannt und bestätigt:**
   Dieselbe Datei, `macForceLinkExtSSC_set:4191`, schreibt für EXT1 bei SSC an
   `1d53=0001`, `1d55=000f`, `1d54=05fa`, `1d52=2473`; bei SSC aus
   `8000`, `000f`, `0008`, `2411`. Das entspricht unserem bereits untersuchten
   Ablauf. Das Paket liefert hier keine neue alternative Taktkonfiguration.

4. **TX-Längen enthalten vier FCS-Bytes:**
   `rtl819x_swNic.c:1260` setzt bei kurzen Frames die Deskriptorlänge auf 64,
   sonst auf Nutzlänge plus vier; die Hardware ergänzt die CRC.
   `tx_ph_len` und `tx_mlen` erhalten diese Länge. Damit ist eine pauschale
   Entfernung der vier Bytes aus unserem SDK-kompatiblen Deskriptorformat
   durch diese Quelle gerade nicht begründet. Die aktuelle Nutzdaten-/DMA-
   Behandlung muss trotzdem getrennt geprüft werden.

5. **Acht-Byte-CPU-Tag erfordert zwei Formatbits:**
   `rtl83xx_v1dot4/dal/rtl8367d/dal_rtl8367d_cpu.c:360` setzt für acht Bytes
   sowohl `CPU_TAG_FORMAT` als auch `CPU_TAG_FORMAT_PRI` auf null. Das ist
   ein brauchbarer Readback-Vergleich für den externen Switch. Es ersetzt
   nicht die Überprüfung, ob SoC und Switch den Frame tatsächlich unverändert
   und mit korrekter FCS übertragen.

Die bisherigen RD05-Messungen zeigen CPU-Port-FCS-Fehler bei ausbleibendem
SoC-RX und ohne Ausgangsverkehr an den Benutzerports. Das Paket stärkt die
Prüfung des gemeinsamen SoC/Switch-Pfads. Es beweist weder einen NAT-Fehler
noch dessen genaue elektrische oder softwareseitige Ursache. CRC-Prüfungen
abzuschalten würde diese Ursache nicht beheben.

## Was gegenüber unseren SDK-Auszügen wirklich hinzukommt

| Datei | Vergleich | Ergebnis |
| --- | --- | --- |
| `rtl819x_swNic.h` | MR62X | Bytegleich |
| `rtl865xc_swNic.c` | MR62X | Bytegleich |
| `rtl819x_swNic.c` | MR62X | Nur ein mit `#if 0` deaktivierter Debugblock entfernt; kein neuer aktiver TX-Fix |
| `AsicDriver/rtl865x_asicL2.c` | MR62X | Herstelleranpassungen, u. a. PHY-/Queue-/LED-/Flow-Control-Code; keine pauschal übertragbare Verbesserung |
| `dal_rtl8367d_port.c` | AX12 | Bytegleich |
| `dal_rtl8367d_cpu.c` | AX12 | Bytegleich; diese Datei fehlt im verglichenen MR62X-Auszug |

Der Gewinn ist vor allem der zusammenhängende Kernel- und Bootloader-BSP
einschließlich `rtl83xx_v1dot4`, Registerheadern und Boardkonfigurationen.
Ein anderer Dateihash allein bedeutet keinen neuen funktionalen Fix.

## WLAN, MW5 und AC23

R76 aktiviert `CONFIG_RTL8852AE_BACKPORTS=y` (`config-4.4:1436`). Im Archiv
sind außerdem Konfigurationsnamen für RTL8812F, RTL8814C und RTL8832BR vorhanden.
Die Dateiinventur findet jedoch keine Quellverzeichnisse für `rtl8192cd`,
RTL8852 oder RTL8832. Das vorhandene generische `rtl8812au-ct`-Paket ist ein
anderer Treiber und kein Ersatz für unsere SoC-/PCIe-Funkchips.
Profilnamen und Buildoptionen werden deshalb nicht als gelieferter vollständiger
WLAN-Quellcode oder als Nachweis neuer cfg80211-Unterstützung gewertet.

Für MW5 und AC23 sind die SoC-/DMA-Registerreferenzen ebenfalls interessant;
die RTL8367D-Werte dürfen aber nicht ungeprüft auf den MW5-Switch oder die
AC23-Platine übertragen werden. Keine fremden Factory-Kalibrierungen übernehmen.
Aus diesem Paket ist bislang kein belegter neuer WLAN-Leistungsfix abgeleitet.

## Nächste sinnvolle Entwicklungsschritte

1. Den IPG-/Tag-Modus als eigenständige Hypothese mit Register-Readback und
   identischen Testframes untersuchen, ohne gleichzeitig Pad/SSC/Delay zu ändern.
2. Die EXT1-Initialisierungsreihenfolge bis zum letzten Force-Link-Write mit
   dieser Referenz und dem RD05-OEM-Kernel vergleichen.
3. Anschließend SoC-TX, CPU-Port-Empfang/FCS, Kupferport-Ausgabe und SoC-RX
   gemeinsam messen. Erst ein DHCP-Lease und ARP-Verkehr in beiden Richtungen
   bestätigen einen funktionierenden Datenpfad.

Validierung dieser Analyse: Archiv inventarisiert, relevante Quellen gelesen,
sechs Kerndateien per SHA256 und Git-Inhalt verglichen. Kein Firmwarebuild und
kein Hardwaretest für dieses Paket durchgeführt.

Die darauf aufbauenden experimentellen Geräteprofile sind in
[CUDY_AX1500.md](../../target/linux/realtek/rtl8197f/CUDY_AX1500.md) dokumentiert.
Sie dienen zunächst ausschließlich dem UART-/RAM-Bring-up.

Die nachfolgende CPU-Clock-Portierung und die Bewertung weiterer Treiber sind
in [DRIVER_PORTS.md](DRIVER_PORTS.md) beschrieben.

Die Bestandsaufnahme der Software-/Hardware-Beschleuniger und ihre strikte
firewall4-Anpassung stehen in
[ACCELERATION.md](../../target/linux/realtek/rtl8197f/ACCELERATION.md), mit
Quellpfaden und Prüfsummen in [acceleration-inventory.json](acceleration-inventory.json).
