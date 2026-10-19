# RTL8197F: cfg80211-Port für OpenWrt 24.10 / Linux 6.6.114

Dieser Stand erweitert den gemeinsamen Linux-Bus-/cfg80211-Port. Er ist keine
Freigabe aller Boards für den produktiven Einsatz. Funk, Kalibrierung,
Switch-Kaltstart und Dauerlast müssen auf den jeweiligen Geräten geprüft werden.

## Hardware und SDK-Zuordnung

Referenz: Branch `RT8197F`, Commit
`e6d68e9067f54b695022f61ed52e50dc56c5a7b7`, insbesondere
`8197/backports/src/drivers/net/wireless/realtek/rtl8192fe/8192cd_osdep.c`
und dessen HAL-/PHYDM-Implementierungen. Vorhandene HAL-Quellen werden über die
Linux6-Bus-Anbindung verwendet; ein Kernel-4.4-Bustreiber wird nicht geladen.

| Board | 2,4 GHz | 5 GHz: Aufdruck / SDK-HAL | Erwartete PCI-ID | Switch / Flash |
| --- | --- | --- | --- | --- |
| Tenda NOVA MW5 | RTL8197FS | RTL8812BRH / 8822B | `10ec:b822` | RTL8363NB; 25Q64JVSIQ, 8 MiB |
| Tenda AC23 / Lynx 8000 | RTL8197FH | RTL8814BR / 8814B | `10ec:b814`, am Gerät zu bestätigen | Nutzerangabe „8637RB“, bisheriges Profil RTL8367RB-Familie; BH25Q64, 8 MiB |
| Xiaomi RD05 | RTL8197FH | RTL8812FR / 8812F | `10ec:f812`, OEM-Log vorhanden | Nutzerangabe RTL8367RB; vorhandenes OEM-Log RTL8367D, ID `0x6642`; vorhandenes 16-MiB-Profil |
| iBall WRD12GN, bestehendes Profil | RTL8197FH / OEM 97FN | RTL8812BRH / 8822B | `10ec:b822` | bestehendes Boardprofil, 8 MiB |

Ein Gehäuse-/Chipaufdruck ersetzt weder PCI-ID noch Switch-Register-ID.
Insbesondere wird RD05 nicht allein aufgrund der neuen Bestückungsangabe auf
einen anderen CPU-Port oder eine andere Switch-Initialisierung umgestellt.

## Änderungen

- Alle vier WLAN-Paketvarianten verwenden Linux6, cfg80211/nl80211 und die
  OpenWrt-Backports-Header. `wireless-tools`, private WEXT-Handler und der
  installierte `iwpriv`-AP-Test gehören nicht mehr zum Paketpfad. Die
  RTL8197F-Kernelkonfiguration fordert WEXT ebenfalls nicht mehr an.
- PCI-Probe und Vendor-Initialisierung wählen 8822B, 8812F oder 8814B anhand
  der echten PCI-ID und des jeweils einkompilierten HAL. Unbekannte Geräte
  werden abgewiesen. MMIO und Legacy-IRQ-Ressourcen verwaltet Linux.
- Der Standalone-Konfigurationsheader erzwingt im Linux6-Build keine MW5-
  Chips, HALMAC-Version oder Legacy-WEXT-Definition mehr. Die MW5-spezifische
  Einschaltsequenz und erzwungene 40-MHz-Korrektur verwenden einen eigenen
  Board-Schalter; die gemeinsame Gerätetabelle aktiviert diese nicht mehr.
- 8812F und 8814B melden das 5-GHz-Band und 802.11ac. 8814B meldet im
  4T4R-Modus vier HT-/VHT-Streams; Band-Kapazitäten werden pro Radio gespeichert.
- Drei TX-Deskriptor-IE-Zeiger im 8814B-fähigen HAL verwenden Byte-Abstände.
  Die bisherigen Struktur-Abstände konnten hinter den Deskriptor zeigen.
  Die ADDRCAM-Löschfunktion liefert bei Erfolg einen definierten Status.
- RTL8812F programmiert keine HT3S-/HT4S-Leistungswerte: Der Chip ist 2T2R.
  Beim 8814B verwenden die Vier-Pfad-Leistungsgrenzen den richtigen
  `CONFIG_WLAN_HAL_8814BE`-Schalter.
- Die 8812F-/8814B-HAL-Tabellen verwenden typkorrekte Callback-Adapter mit
  definierten Rückgaben. Absichtlich leere Legacy-Hooks für HALMAC/ADDRCAM
  und IE-Deskriptoren behalten ihre Aufgabe; der bislang unimplementierte
  8812F-Reserved-Page-Download meldet ausdrücklich einen Fehlschlag.
- Der 8814B-Duty-Cycle-Parser verwirft überlange Felder vor dem Firmware-Aufruf.
- RFE5/RFE6 bleiben MW5-Vorgaben. Ohne explizite Boardvorgabe überschreibt
  die neue Bus-Anbindung die Vendor-/eFuse-Auswahl nicht. Das allein beweist
  noch keine gültige Kalibrierung auf AC23 oder RD05.
- AC23 und RD05 beschreiben beide WLAN-Endpunkte, die ICTL-Quellen und die
  nicht nullbasierten PCI-Aperturen im DT. RD05 verwendet seine getrennten
  U-Boot-WLAN-MAC-Zellen ohne MW5-MAC-Inkrement. Der NVMEM-Leser akzeptiert
  gültige binäre und begrenzte ASCII-MAC-Zellen und erhält Probe-Fehler.
- Legacy-WDS/Mesh-Autointerfaces und WLAN-Bridge-/NAT-Abkürzungen sind im
  Linux6-Paketpfad deaktiviert. Die bestehende MW5-Abschaltung der
  Softwarebeschleuniger bleibt bestehen.

## Aus den OEM-Dumps übernommene Boarddaten

[OEM_DUMP_INTEGRATION.md](OEM_DUMP_INTEGRATION.md) beschreibt den neuen
Factory-Import und seine Grenzen. MW5 und AC23 lesen begrenzte Tenda-
Datensätze, RD05 seinen anhand der eigenen OEM-Bibliothek rekonstruierten
H601-Block einschließlich Prüfsumme. Alle drei DTS binden beide Radios an
den jeweiligen schreibgeschützten Factory-Bereich an. Einzelfelder werden
validiert; die individuellen Werte einer Referenzplatine werden nicht mehr
als MW5-Konstanten eingebaut. Die Daten gelten nach den Standardwerten und
bei jedem erneuten Hardwarestart. WEXT ist dafür nicht erforderlich.

Die neuen read-only Debugfs-Werte `factory_applied` und `factory_rejected`
zeigen die übernommenen bzw. verworfenen RF-Felder des letzten Imports.
Das bestätigt noch keine vollständige Kalibrierung oder bessere Reichweite.
Nicht zugeordnete AC-Leistungsdifferenzfelder, Regulatory-Verhalten,
Temperaturregelung und Hardwaretests bleiben offen.

## Reproduzierbare Prüfungen

Host-Regressionen und Board-Konsistenz:

```sh
PYTHONDONTWRITEBYTECODE=1 python3 -m unittest discover -s scripts/tests -v
python3 scripts/rtl8197f-validate-board-profiles.py
python3 scripts/validate-mw5-classic-routing.py
git diff --check
```

Für einen unabhängigen Übersetzungstest benötigt man einen vorbereiteten
Kernel-6.6.114-Buildbaum und einen MIPS-Compiler. Der Test liest die tatsächlichen
Common-/Varianten-Flags aus dem Paket-Makefile und kopiert die Quellen in
temporäre Verzeichnisse außerhalb des Checkouts:

```sh
python3 scripts/check-rtl8192cd-linux6-build.py \
  --kernel /pfad/kernel-build --kernel-source /pfad/linux-6.6.114 \
  --output /pfad/ausserhalb-des-checkouts \
  --cc mips-linux-gnu-gcc-14 --jobs 5
```

Optional `--backports /pfad/openwrt-backports` hinzufügen. Dafür müssen die
`build`- und `subsys`-Patches aus `package/kernel/mac80211/patches` angewendet
und dessen
`backport-include/backport/autoconf.h` passend zum Testkernel erzeugt sein.
Insbesondere ändert Patch 334 die Monitor-Callback-Signatur gegenüber dem
unveränderten Backports-Archiv; der Helfer weist ungepatchte Header zurück.
Der Test bildet das Header-Staging aus `mac80211/Build/InstallDev` nach,
einschließlich des nativen `linux/module.h`. Er prüft nach erfolgreichem Link
auch auf unaufgelöste WEXT-Symbole. Das Archiv wird mit dem im Repository
festgelegten SHA256 geprüft:
`816882e80cd6eb7f10d2851f634ca7964eb47a62f64eff4bf6f92ffdf8593650`.

Die Cloud-Prüfumgebung liegt unter `/workspace/rtl-build-validation`; lokale
Werkzeuge unter `/workspace/rtl-build-tools`. `env.sh` aktiviert diese Werkzeuge.
Der MIPS-Testkernel verwendet eine Malta-Konfiguration ohne WLAN-Alt-Treiber,
`WIRELESS_EXT`/`WEXT_*` und `CFG80211_WEXT`. Damit wird die CPU-/Kernel-API
geprüft, nicht der Bootpfad der realen RTL8197F-Plattform.

Ein gelinktes `rtl8192cd.o` ist kein ladbares Firmware-Artefakt: MODPOST gegen
den fertigen OpenWrt-Kernel, die vollständigen Plattformpatches, das gepatchte
mac80211-Paket sowie Imagegröße und Boot müssen im regulären OpenWrt-Build
zusätzlich geprüft werden. Pro Board einen eigenen Build mit passendem
`kmod-rtl8192cd-rtl8197f-*` durchführen.

## Verbleibende Integrations- und Hardwareprüfungen

Ausgeführt in der Cloud: alle vier Varianten als MIPS32R2/Little-Endian-Objekt
gegen Linux 6.6.114 und die gepatchten OpenWrt-Backports 6.12.96 übersetzt und
gelinkt; keine unaufgelösten WEXT-Symbole. Außerdem 38 Host-Regressionstests,
vier DTS-Kompilierungen, Boardprofil- und MW5-Routing-Prüfung bestanden.
Die SDK-Quellen erzeugen weiterhin Compilerwarnungen. Ein vollständiger
Firmware-/MODPOST-Build und Tests auf den Geräten wurden nicht durchgeführt.

1. Kernel-/Paket-Build und MODPOST auf dem vollständigen OpenWrt-Ziel abschließen.
   Auf dem Gerät `uname -r`, PCI-ID, BAR-Adressen, `iw phy` und IRQ-Zähler prüfen.
2. AC23-/RD05-RFE, PA/LNA, MAC und die vollständige OEM-Kalibrierung unabhängig
   vom MW5 validieren. Nicht mit fremden MW5-Factory-Daten senden.
3. Der vorhandene cfg80211-Adapter verwendet vorerzeugte `wlan0`/`wlan1`-
   Interfaces. Beliebige VIF-Namen, Multi-BSS und sämtliche netifd-/LuCI-
   Lebenszyklen sind noch nicht vollständig portiert. Die MW5-Steuerung
   `mw5-wifi` bleibt boardspezifisch. Sie wird nicht als AC23-/RD05-Steuerung
   installiert. Vorhandene Capability-Anzeigen sind keine Funktionsfreigabe.
4. Switch-Treiber bleiben Linux DSA/Phylink. Die weitere OEM-Rückentwicklung
   weist den Common-Init-Zweig und externe Flow-Control-Werte für die im
   MW5-Log beobachtete ID 0x6367/Revision 0x0060 nach. Patch 332 übernimmt
   acht belegte Schwellenwerte ausschließlich für diese MW5-Identität;
   siehe `OEM_SWITCH_REVERSE.md`. Vollständige Kaltstart-Parität bleibt
   auf Hardware zu prüfen. AC23-Portplan und RD05-Switchbezeichnung live bestätigen.
   Der MW5-TX-Watchdog beobachtet zusätzlich ausstehende OWN-Deskriptoren:
   Nach fünf Sekunden ohne CDP-/Freigabefortschritt stößt er TX begrenzt
   erneut an, ohne Ring-/RX-/Switch-Reset. `ethtool -S` zeigt
   `tx_watchdog_rekicks`; ein steigender Zähler ist Anlass zur Untersuchung,
   kein Beleg für einen behobenen Hardwarestillstand. Dieser neue Retry-Pfad
   ist für AC23/RD05 nicht aktiviert.
5. Je Band WPA2/CCMP-Verbindungsaufbau, DHCP, Bridging, Routing, Reconnect und
   Neustart testen. Danach parallelen LAN/WAN- und WLAN-Verkehr mit einem
   zweiten Rechner erzeugen; mindestens 30 Minuten bidirektional, anschließend
   längeren Dauertest. Dabei `mw5-netdiag full` ausgeschaltet lassen und
   serielle Konsole sowie IRQ-/Drop-/DMA-Zähler beobachten.
6. Erst danach DFS/CAC, Stationsmodus, Energiesparen und weitere APs freigeben.
   Der 8812F-Reserved-Page-Pfad für Firmware-Energiesparfunktionen bleibt
   unimplementiert; ein typkorrekter Callback macht diese Funktion nicht verfügbar.
   Ein höherer PHY-Modus beweist keinen höheren nutzbaren Durchsatz.

Eine vollständige, auf allen genannten Chips hardwarevalidierte Portierung
ist mit diesem Änderungssatz noch nicht erreicht.
