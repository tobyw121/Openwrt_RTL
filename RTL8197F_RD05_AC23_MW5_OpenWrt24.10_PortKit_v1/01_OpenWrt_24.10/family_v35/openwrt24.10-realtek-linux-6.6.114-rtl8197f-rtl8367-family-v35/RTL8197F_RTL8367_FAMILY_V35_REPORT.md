# OpenWrt 24.10 / Linux 6.6.114 — RTL8197F + RTL8367 family v35

## Ziel

Der vorhandene RD05-v34-Baum wurde von einem einzigen RTL8367D-Sonderpfad zu
einer ID-geprüften RTL8367-Familienintegration erweitert. Schwerpunkt sind
RTL8197F/FH-VG mit dem nativen rtl865x-NIC sowie RTL8367D, RTL8367RB-VC und
RTL8367RB-VB. Alte B-Map-Varianten bleiben über die vorhandenen OpenWrt-
`swconfig`-Treiber baubar, werden aber nicht fälschlich mit C-/D-Registern
initialisiert.

## Wesentliche Änderungen

1. Neuer Kernelpatch `327-net-dsa-realtek-rtl8367-family-profiles.patch`:
   - C-Map-IDs `0x0276`, `0x0597`, `0x6367`;
   - D-Map-ID `0x6642`;
   - exakte Revisionen vor generischen Familienprofilen;
   - getrennte C-/D-SSC- und RGMII-Pfade;
   - Aliase für C, D, R, RB, RB-VB und RB-VC;
   - unbekannte alte B-Map-Chips bleiben fail-closed.
2. RTL8197F-NIC v35:
   - getrennte DSA- und Legacy-swconfig-Mastermodi;
   - direkte P0-DMA-Route;
   - konfigurierbare P0-TX/RX/RGTXC-Timings;
   - DSA-Tag-Passthrough ausschließlich im DSA-Modus;
   - allgemeine externe-Switch-Startsequenz statt RD05-only-Gates.
3. DTS/Bindings:
   - C/RB-VB- und D/RB-VC-Vorlagen;
   - abgesicherte Legacy-B-Map-Vorlage;
   - RD05 bleibt TX0/RX6/RGTXC3;
   - DSA und Legacy sind gegenseitig ausgeschlossen.
4. Dokumentation:
   - 42.708 abgeleitete B-/C-/D-Registersymbole;
   - 276 RTL8197F-NIC-Symbole;
   - Varianten- und Quellinventar;
   - klare Trennung zwischen exaktem, experimentellem, Legacy- und
     Inventory-only-Support.

## Wichtigste Varianten

| Kombination | Registerpfad | CPU-Link | Status |
|---|---|---|---|
| RTL8197F + RTL8367D | D, `0x6642:0x0030` | Switch 7/EXT1 ↔ SoC P0 | exaktes Profil, Hardwaretest weiter erforderlich |
| RTL8197F + RTL8367RB-VC | identisch zum D-DAL | Switch 7/EXT1 ↔ SoC P0 | exaktes Profil |
| RTL8197F + RTL8367RB-VB | C, `0x6367:0x0020` | typ. Switch 6/EXT0 ↔ SoC P0 | exaktes Profil, neue Vorlage |
| RTL8197F + alte RTL8367RB/R-VB | B | boardabhängig | optionaler Legacy-swconfig-Pfad |

## Keine unbelegte Vollständigkeitsbehauptung

Der Baum erweitert die Familienerkennung, den Basispfad, PHY/Link, STP, MTU,
Statistiken, CPU-Tag und die RTL8197F-DMA-/RGMII-Anbindung. Der Linux-6.6-
`rtl8365mb`-Treiber besitzt jedoch noch keine vollständigen DSA-Offload-Callbacks
für Bridge, VLAN, FDB, MDB, LAG, TC/ACL, QoS, Policing, Mirror und PTP. Diese
Funktionen sind im Vendor-SDK vorhanden, aber nicht automatisch sichere oder
lizenzneutrale Linux-Implementierungen. Software-Forwarding ist möglich, aber
kein Ersatz für vollständigen ASIC-Offload.

Ebenso fehlen reale Boot-/Traffic-Tests auf jeder PCB-/Siliziumvariante. Details,
Registertabellen und Testplan befinden sich unter
`target/linux/realtek/docs/rtl8197f-rtl8367/`.

## Reproduzierbare Validierung

Der komplette Linux-6.6.114-Patchstapel wurde in realer Reihenfolge auf eine
saubere Kernelquelle angewendet: 398 Backport-, 151 Pending-, 51 Hack- und 38
Realtek-Patches, insgesamt 638 Patches. Dabei wurde ein bereits in v34
vorhandener Kontextfehler in Patch 325 sowie ein Fuzz-abhaengiger Kontext in
Patch 326 gefunden und korrigiert. Ergebnis:
`MANUAL_KERNEL_PREPARE_PASS` und `RTL8367_FINAL_SOURCE_MATCH_PASS`.

Der Projektvalidator meldet 186 PASS, 7 bewusst dokumentierte WARN und 0 FAIL.
Der finale DSA-Switchquelltext sowie der finale RTL8197F-NIC-Treiber kompilieren
mit `W=1` und `-Werror` gegen installierte Linux-Header. Ein kompletter
OpenWrt-MIPS-Build konnte in dieser Umgebung nicht abgeschlossen werden, weil
die Host-Abhaengigkeiten ncurses und GNU awk fehlen. Physische Boardtests sind
weiterhin erforderlich. Der genaue Nachweis steht in
`RTL8197F_RTL8367_FAMILY_V35_VALIDATION.md` und unter `validation/`.
