# RD05 OEM-Zustandsbericht ohne MTD-Dumps

`rd05-oem-collect.sh` ist ein eigenständiges Shellskript für die vorhandene
BusyBox-OEM-Firmware. Es benötigt keinen OpenWrt-Neubau und installiert nichts.
Es setzt weder `head -c`, `mktemp`, `timeout` noch `flock` voraus; Datei-Lesegrenzen
werden mit begrenztem `dd` umgesetzt, temporäre Verzeichnisse mit atomarem `mkdir`.
Es wurde anhand des hochgeladenen `collector-script.sh` auf unseren Diagnosefall
zugeschnitten. Dessen standardmäßig aktiviertes `HASH_MTD=1`, Upload-/TFTP-Dienste,
Rootfs-Kopien, Prozessumgebungen und breite Debugfs-Suche wurden nicht übernommen.

## Enthaltene Informationen (Version 1.1)

- CPU, Speicher, Kernel, Module, Speicheradressbereiche und UART-Statistik.
- Alle von Linux enumerierten PCI-Geräte mit IDs, Treiberzuordnung, IRQ,
  Ressourcenadressen und verfügbaren Linkeigenschaften. Keine BAR-Inhalte oder
  PCI-Konfigurationsraum-Dumps.
- Plattform-, SPI- und MDIO-Inventar sowie ausgewählte Ethernet-/UART-/GPIO-
  Plattformdaten. Ein integrierter SoC-Block muss kein PCI-Gerät sein.
- Registrierte GPIO-Controller und schon exportierte GPIOs mit Richtung und
  aktuellem Wert; vorhandene GPIO-/Pinmux-Debugübersichten. Kein Exportieren,
  Umstellen oder Testschalten von Pins. Nicht registrierte Pins oder unbekannte
  elektrische Boardverbindungen werden dadurch nicht automatisch erkannt.
- Verfügbare Temperatur-, LED-, CPU-Frequenz- und Taktinformationen.
- Netzwerkgeräte, Adressen, Routen, Nachbarn, Bridges/VLANs, lesbare Firewallregeln
  und verfügbare WLAN-Fähigkeiten. Keine UCI-/NVRAM-/WLAN-Schlüsseldumps.
- Realtek-TX-/RX-Ringe, bekannte Diagnose-/ASIC-Zähler sowie Netz-/Interruptzähler
  vor, während und nach einer Beobachtungsphase. Fehlende Werkzeuge werden protokolliert.
- Mit `--deep`: zusätzlich entdeckte Realtek-/RTW-/WLAN-proc-Dateien, vorhandene
  Realtek-/ieee80211-Debugfs-Zustände und Realtek-Modulparameter. Die Liste
  `inventory/realtek-decisions.tsv` zeigt jeden gefundenen Pfad und die Entscheidung.
  Es wird dabei auch unbekannter Vendor-Zustand gelesen; Namensfilter schließen
  Befehls-/Test-/Reset- und erkennbare Schlüssel-/Flash-Schnittstellen aus. Das ist
  keine Quellcodeprüfung jedes unbekannten Lesecallbacks.
- Optional: die bereits am RD05 verwendeten, festen MMIO-/RTL8367-Leseaufträge.
  Die Antworten stehen in `commands/rd05-register-readback.txt` und im dmesg.
- Optional: ein kurzer ARP-/ICMP-/DHCP-Mitschnitt, wenn OEM-tcpdump vorhanden ist.
  Es werden keine Pakete durch das Skript erzeugt.

Es werden **keine MTD-Partitionen gelesen, gehasht oder gesichert** und keine
NVMEM-/eFuse-/EEPROM-Inhalte, RAM-Abzüge oder Rootfs-Binärkopien angefertigt.
Das ist ein umfangreiches Inventar mit ausgewählten Laufzeitdaten, keine
vollständige semantische Analyse sämtlicher Hardware. Insbesondere macht ein
nachträglicher OEM-Ringdump alte Paketpuffer nicht wieder gültig.

## Grenzen und Verhalten

Die Basiserfassung liest eine feste Auswahl; `--deep` erweitert sie um entdeckte
Realtek-Zustandsdateien. Sie liest keine
beliebigen Vendor-`test`-/`reset`-/`gpio_ctrl`-Dateien. Es werden keine Module,
Netzwerkeinstellungen oder Firewallregeln geändert, keine Debug-Dateisysteme
gemountet und keine Upload-/TFTP-/HTTP-Dienste gestartet.

Normale Abfragen haben vier bis fünf Sekunden Zeit, die Pfadsuche zehn Sekunden.
Neue Abfragen enden spätestens nach
300 Sekunden zuzüglich der angeforderten Beobachtungsdauer oder bei 20.000
Abfragen beziehungsweise ungefähr 12 MiB Verzeichnisgröße zuzüglich einer Abfrage.
Bis zu 2.048 zusätzliche Zustandspfade werden ausgewählt; weitere Pfade erscheinen
als `node-limit` im Inventar. Der Ressourcenrahmen bleibt ausdrücklich begrenzt.
Archiv und Arbeitsverzeichnis bleiben bestehen; zusammen kann der Platzbedarf
etwa doppelt so groß sein. Virtuelle Dateien sind auf 512 KiB begrenzt, sonstige
Befehlsausgaben auf höchstens 1 MiB. Größenlimits können Daten abschneiden.
`status.tsv` dokumentiert fehlende/fehlgeschlagene Abfragen und Zeitüberschreitungen.

Ein Shell-Watchdog kann einen im Kernel ununterbrechbar hängenden Zugriff nicht
garantiert beheben. Vendor-Leseabfragen können Zähler verriegeln oder Timing
beeinflussen; dies ist kein störungsfreier Durchsatzbenchmark. Ohne lesbare
GPIO-/Pinmux-Schnittstelle gibt es keine vollständige Pinbelegungsrekonstruktion.

`--rd05-registers` ist ausdrücklich nur für den hier untersuchten Xiaomi RD05:
Es schreibt **Leseaufträge** in die bekannten `/proc/rtl865x`-Befehlsschnittstellen,
aber keine Registerwerte. Die Kernel-Konsolenschwelle wird kurz reduziert und bei
normalem Ende oder behandelbaren Signalen wiederhergestellt. Keine Speicher-
Adresssuche und keine Abfrage alter Paketpufferadressen.

Das Archiv enthält MAC-/IP-Adressen, Kernelmeldungen, Firewallregeln und bei PCAP
gegebenenfalls Hostnamen und Paketdaten. Vor öffentlicher Veröffentlichung prüfen.
Es wird nicht automatisch übertragen. Nur einen Lauf gleichzeitig
starten; ein nach Stromverlust verbleibendes Lock wird nicht blind entfernt.

## Mindestens eine Minute Zeitverlauf

Live-Läufe dauern mindestens 60 Sekunden; `--duration` akzeptiert 60 bis 600.
Der Standard ist 60 Sekunden mit einer Zielpause von fünf Sekunden zwischen den
Snapshots. Die Messdauer verwendet die monotone Kernel-Uptime, damit NTP-Sprünge
sie nicht verkürzen. Es gibt einen Anfangs- und einen Snapshot ab dem Ende des
angeforderten Zeitfensters. Aufwendige Reads verlängern Lauf und Abstände.
Im Deep-Modus wird die Kernel-Konsolenschwelle vorübergehend reduziert und danach
wiederhergestellt; die Meldungen bleiben im dmesg erfasst. Das reduziert die
UART-Ausgabelast während der wiederholten Vendor-Abfragen.

`live/timeline.tsv` und `live/window.txt` enthalten tatsächliche Zeiten. Jeder
Snapshot liegt als `live/0000.tar.gz`, `live/0001.tar.gz` usw. vor. Seine Rohdateien
werden erst nach erfolgreichem Packen und Prüfen gelöscht. So bleiben die vielen
Ring-Dumps im RAM handhabbar. Detaillierte Zusatzpfade werden pro Snapshot in
`realtek-state.txt.gz` zusammengefasst: aufeinanderfolgende gzip-Mitglieder mit
`STATE path=... bytes=... uptime=...` und anschließend den erfassten Bytes.
GNU gzip bzw. Python `gzip.decompress()` lesen diese Mitglieder gemeinsam.
`status.tsv` kennzeichnet pro Snapshot Fehler, Zeitüberschreitungen und Limits.

Der optionale OEM-PCAP läuft parallel zum Zeitfenster, kann aber bei 4.096 Paketen
oder Erreichen der Dateigrößengrenze früher enden. Für durchgehenden Paketverkehr
ist der externe PC-Mitschnitt vorzuziehen. Unbekannte Vendor-Ausgaben sind nicht
inhaltlich geschwärzt; das Deep-Archiv zunächst privat halten.

## PC: Skript übertragen

OEM-Firmware starten, PC an die nachweislich funktionierende LAN2-Buchse.
Für den kontrollierten LAN-Vergleich Fritzbox vom WAN trennen.

```bash
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
scp -O -o HostKeyAlgorithms=+ssh-rsa \
  scripts/rd05-oem-collect.sh root@192.168.31.1:/tmp/rd05-oem-collect.sh
```

Optional in einem zweiten PC-Terminal während der Erfassung:

```bash
ping -I enx14ebb6300bb2 -c 300 192.168.31.1
```

Es muss eine gültige PC-Adresse im OEM-Netz vorhanden sein. Diese Befehle ändern
die vorhandene IP-Konfiguration nicht. Der externe PC-Mitschnitt bleibt nützlich,
wenn OEM-tcpdump fehlt:

```bash
PCAP=$(mktemp "$HOME/rd05-oem-collector-XXXXXX.pcap")
echo "$PCAP"
sudo tcpdump -U -ni enx14ebb6300bb2 -s 256 -w - 'arp or icmp' > "$PCAP"
```

## OEM-Router: Erfassung starten

Über UART oder vorhandenes SSH, eine Zeile:

```sh
sh /tmp/rd05-oem-collect.sh --deep --rd05-registers --duration 60 --interval 5
```

Ohne die zusätzlichen Register-Leseaufträge:

```sh
sh /tmp/rd05-oem-collect.sh --deep --duration 60 --interval 5
```

Wenn `tcpdump` auf der OEM-Firmware vorhanden ist, optional stattdessen:

```sh
sh /tmp/rd05-oem-collect.sh --deep --rd05-registers --duration 60 --interval 5 --capture eth1
```

`eth1` ist die bisher im RD05-OEM-LAN beobachtete Schnittstelle. Im Zweifel
vorher `ip addr` prüfen. Ohne OEM-tcpdump wird der PCAP-Teil als nicht verfügbar
protokolliert; die restliche Sammlung bleibt erhalten. Kein Nachinstallieren nötig.

Am Ende erscheinen `ARCHIVE=/tmp/rd05-oem-collect.XXXXXX.tar.gz` und der
Arbeitsverzeichnispfad. Bei einem Abbruch bleibt die Teilsammlung erhalten.

## Vergleich Leerlauf gegen LAN-Verkehr

Für einen eindeutigen Vergleich zwei beschriftete Läufe ausführen. Zunächst auf
OEM ohne absichtlich erzeugten PC-Ping (Hintergrundverkehr kann weiterlaufen):

```sh
sh /tmp/rd05-oem-collect.sh --deep --label idle --rd05-registers --duration 60 --interval 5
```

Danach auf dem PC:

```bash
ping -I enx14ebb6300bb2 -c 300 192.168.31.1
```

Während dieses Pings auf OEM:

```sh
sh /tmp/rd05-oem-collect.sh --deep --label ping --rd05-registers --duration 60 --interval 5 --capture eth1
```

Die Dateinamen enthalten `idle` bzw. `ping`. Beide Archive senden. Der optionale
externe PC-Mitschnitt kann während des zweiten Laufs parallel laufen.

## PC: Archiv herunterladen

Nach Ende der Sammlung:

```bash
scp -O -o HostKeyAlgorithms=+ssh-rsa \
  'root@192.168.31.1:/tmp/rd05-oem-collect.*.tar.gz' .
ls -lt rd05-oem-collect.*.tar.gz
```

Der Aufruf holt alle zu diesem Namen passenden Collector-Archive; bei mehreren
Läufen das gewünschte neue Archiv senden. Externen tcpdump mit Strg+C stoppen
und gegebenenfalls dessen PCAP ebenfalls senden. Den Ping stoppen und WAN
anschließend wieder verbinden. **Kein Firmware-Build oder Flashen erforderlich.**

## Lokale Tests

```sh
sh -n scripts/rd05-oem-collect.sh
PYTHONDONTWRITEBYTECODE=1 PYTHONPATH=scripts/tests \
  python3 -m unittest test_rd05_oem_collector
```

Die Tests benutzen eine Offline-proc/sys-Fixture: PCI/GPIO-Daten, vollständiger
379-KiB-RX-Ring, Ausschluss von Flash/EEPROM/Schlüsseln, unveränderte Quelldateien,
Ablehnung von Live-Optionen im Offline-Modus und ein simulierter hängender Leser.
Zusätzlich prüft eine simulierte monotone Uhr die vollständige Minutenabdeckung
mit schnellen und langsamen Reads; ein Deep-Test prüft Inventar, Ausschlüsse und
wiederholte komprimierte Snapshots. Die Tests ersetzen keinen OEM-Hardwarelauf.
Die Fixture- und Timeout-Tests wurden unter der Host-Shell und einer separat
gebauten BusyBox-1.36.1-ash geprüft; die konkrete OEM-BusyBox ist noch nicht getestet.
