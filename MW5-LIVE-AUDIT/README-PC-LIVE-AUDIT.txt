MW5 Live Audit Interactive v2

1. PC mit zwei dedizierten Ethernet-Adaptern verkabeln:
   NIC 1 -> MW5 LAN
   NIC 2 -> MW5 WAN

2. Optional dritte NIC/WLAN für normales Internet/Management benutzen.

3. Pakete unter Debian/Ubuntu:
   sudo apt update
   sudo apt install iproute2 tcpdump iperf3 dnsmasq-base ethtool \
       openssh-client isc-dhcp-client iputils-ping arping tshark python3

4. Beide Dateien im selben Ordner lassen:
   mw5-pc-audit-interactive.sh
   mw5-router-audit.sh

5. Start:
   chmod +x *.sh
   sudo ./mw5-pc-audit-interactive.sh

Das PC-Skript:
- listet physische Adapter inkl. MAC, Link, Speed, IPv4;
- markiert die aktuelle Default-/Management-NIC;
- lässt LAN- und WAN-Adapter per Nummer auswählen;
- fragt Kabelbelegung und Testparameter ab;
- nimmt die Adapter automatisch aus NetworkManager;
- erstellt getrennte Linux Network Namespaces;
- simuliert am PC-WAN 192.168.178.1 samt DHCP/DNS;
- testet den LAN-DHCP des MW5;
- startet PCAPs auf LAN und WAN;
- lädt den read-only Router-Collector nach /tmp;
- erstellt Router-Snapshots vor/nach jedem Test;
- führt mehrere iperf3 Download-/Upload-Zyklen aus;
- führt anschließend UDP-Tests aus;
- erkennt automatisch: MW5-LAN erreichbar, aber WAN-Forwarding tot;
- erstellt sofort einen FAIL-Snapshot;
- holt die Routerdaten zurück;
- setzt die PC-NICs nach Ende wieder zurück.

Ergebnisse:
  ./mw5-audit-results/mw5-YYYYMMDD-HHMMSS/
  ./mw5-audit-results/mw5-YYYYMMDD-HHMMSS.tar.gz

v44.65.25 / audit v7 additions:
- physical NICs remain visible via temporary macvlan children;
- per-run SSH known_hosts handles the MW5 per-boot host key safely;
- unsafe debugfs regmap register scans remain disabled;
- any iperf3/TCP failure creates an immediate FAIL snapshot even if ICMP works.

Live Audit v7 - sofortige Snapshot-Sicherung
--------------------------------------------
Jeder über router_snapshot ausgelöste PRE/POST/FAIL-Snapshot wird unmittelbar
mit snapshot-stream als eigenes tar.gz nach router/live-snapshots/ auf den PC
übertragen und zusätzlich unter router/live-extracted/ entpackt. Das finale
Gesamtarchiv bleibt erhalten, ist aber nicht mehr der einzige Sicherungspunkt.
Wenn TCP/SSH später zusammenbricht, bleiben alle zuvor übertragenen Snapshots
lokal verfügbar.
