# WR1500/R76: natives RTL8832BR-Paket, 01.11.2026

Das neue `package/kernel/rtl8832br-wr1500` baut `rtk_wifi6.ko` aus der G6-GPL
und installiert die bestätigten RFE50-Basis- und AP_5G-Parameter. Das R76-
Basispaket zieht den Kernel-Treiber automatisch hinzu; auch die AX1500-
defconfig und der vorhandene Defaults-Helfer wählen ihn aus. Autoload hat
Priorität 60. Ein R76-Firmwarebau benötigt dafür keine menuconfig-Auswahl.

Paketrevision 2 behebt zusätzlich die private Header-Auflösung im echten
OpenWrt-Staging: `Build/InstallDev` von mac80211 stellt auch interne Header
wie `sta_info.h` bereit. `drv_types.h` bindet deshalb den Realtek-Stationsheader
relativ zu seinem eigenen Verzeichnis ein. Die Linux-/Backports-Suchreihenfolge
bleibt erhalten, damit das mitgelieferte, ausschließlich für Userspace-Werkzeuge
gedachte `include/linux/wireless.h` keine Kernel-Header überschattet.

## Quelle und Reproduzierbarkeit

Quelle: TP-Link `GPL_ax1500v3.tar.gz`, SHA256
`6d133598b134b6878ce802a53b3061d9f2efb3636ad0d31ec4cb3bcbdf438f2a`.
Der erste Download ist etwa 2,1 GiB groß und wird in `dl/` zwischengespeichert.
Es wird nur der G6-Unterbaum extrahiert, nicht das ganze Hersteller-BSP gebaut.
Zwei Dateien mit CR-Zeichen werden vor dem Paketpatch normalisiert.
Der Patch enthält die früheren OS-Service-Anpassungen und die weiteren
Linux-6.6-/cfg80211-Änderungen; die alten Entwicklungspatches nicht zusätzlich
auf das Paket anwenden. Kein vorbereitetes `.ko` muss manuell kopiert werden.

## Anbindung und Steuerung

- Probe akzeptiert nur `cudy,ax1500-r76` und PCI `10ec:b832`.
- PCI aktiviert und reserviert ausschließlich MMIO-Ressourcen. BAR0/I/O wird
  nicht benötigt; der vorhandene OEM-Befund ist BAR2/MMIO bei 0x19000000.
- RX/TX verwenden Linux-DMA-APIs mit geprüften Mappings; partielle TX-Mappings
  werden zurückgerollt, RX-Mappingfehler nicht als fertige Deskriptoren publiziert.
  Es gibt keinen virt_to_phys-Ersatz für ein fehlgeschlagenes aktives Mapping.
- Tasklet-Kontext, Dateioperationen und cfg80211-Callbacks sind angepasst.
  Der Treiber wird gegen die OpenWrt-Backports statt native 6.6-WLAN-Header gebaut.
- WEXT-/private ioctl- und OEM-Vendor-Command-Schnittstellen sowie die private
  Proc-Debug-Oberfläche werden nicht registriert. Steuerung erfolgt über
  nl80211, netifd und hostapd; LuCI nutzt diese normale OpenWrt-Anbindung.
- Private Bridge-/NAT-/FleetConntrack-Pfade sind nicht aktiviert. Die
  Linux-Bridge und firewall4 bleiben für den Systemdatenpfad zuständig.
  Der optionale WLAN-TX-Deskriptorcache ist für den ersten Test ausgeschaltet.
- cfg80211-Kanalsperren, NO_IR und RADAR werden nicht zugunsten des OEM-
  Kanalplans gelöscht. Die Herstelleroption zum Abschalten von Leistungsgrenzen
  ist entfernt; die ByRate-, LMT- und RU-Tabellen bleiben aktiv.
- Schlüsselentfernung ruft den Hardware-Key-Delete-Pfad auf. Fehler werden
  gemeldet, betroffene Pairwise-Stationen gesperrt und gelöschte Schlüssel aus
  dem Softwarezustand entfernt. Der OEM hatte hier auch bei WPA-Schlüsseln
  ohne Hardwareentfernung Erfolg zurückgegeben.

Die Parameter werden aus `/etc/conf/rtl8832bre/RFE50` und `AP_5G` geladen.
Sie stimmen mit dem geprüften OEM-Archiv überein, einschließlich der beiden
OEM-ByRate-Abweichungen. Die Firmware stammt aus dem eingebetteten G6-Firmware-
Array; ihre tatsächliche Startversion muss der erste OpenWrt-Bootlog bestätigen.

## Grenzen und tatsächlich ausgeführte Prüfungen

Vollständiger Treiberbau mit MIPS LE und Linux 6.6.114; cfg80211/compat aus
Backports 6.12.96 ebenfalls gebaut. Modpost verwendet deren echte Exporte
und die Kernel-Symboltabelle; keine künstlichen Symbole oder Modpost-Warn-
Umgehung. Im Referenzkernel werden die ersetzten nativen cfg80211/mac80211-
Exporte beim Backports-Abgleich ausgeschlossen. Der vollständige Paketpatch
ist ohne Fuzz anwendbar; ein frischer GPL-Unterbaum plus Patch baut ebenfalls
das Modul. Der Testkernel ist ein Referenzbuild, kein fertiges R76-Image.

Host-C-Tests mit UBSan prüfen produktive Funktionen für DMA-Fehler, gültige
DMA-Adresse null, Rückabwicklung partieller TX-Mappings, Schlüsselentfernung
und Erhalt der cfg80211-Kanalsperren. Die Paketdaten werden gegen die
OEM-Dateiprüfsummen geprüft. Vorhandene R76-/RD05-/MW5-Tests laufen zusätzlich.
Eine zusätzliche Regression verwendet die echte Paket-Builddefinition mit
nachgebildetem OpenWrt-Staging: Der alte Include scheitert am mac80211-
Stationsheader, der lokale Include baut; Linux-Header behalten Vorrang vor
den Userspace-Werkzeugheadern des Herstellers. Der vollständige Modulbau
wurde außerdem mit dem durch Build/InstallDev erzeugten Headerlayout geprüft.
Einige ältere Herstellerwarnungen sind als Warnungen belassen; inkompatible
API-Typen und implizite Funktionsdeklarationen bleiben Buildfehler.

Nicht ausgeführt: kompletter OpenWrt-Firmwarebau, RF-Hardwaretest,
Durchsatz-/Langzeit-/Thermaltest, WPA3-/PMF- oder DFS-Funktionstest. Der
integrierte 2,4-GHz-8197G benötigt einen eigenen Treiber-/Factory-Port.
Manuelle feste/begrenzte Sendeleistung und eine echte Messwertabfrage sind
noch nicht implementiert und liefern EOPNOTSUPP statt eines erfundenen Werts.
Zunächst WPA2-CCMP, Kanal 36 und automatische Sendeleistung testen.

## PC: WR1500 bauen

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

Bei Fehlern nur den Treiber einzeln mit ausführlichem Log bauen:

```sh
make package/kernel/rtl8832br-wr1500/clean
make package/kernel/rtl8832br-wr1500/compile V=s -j1
```

R76 baut weiterhin die bisherigen RAM-/Fullflash-Artefakte. Für Fullflash
bleibt die persönliche `fullflash-templates/Cudy_AX1500.bin` erforderlich;
der vorhandene Einspielweg und die Flash-Leseschutzregeln ändern sich nicht.

## Router: erster Boot über UART 38400, Befehle einzeln

```sh
cat /etc/openwrt_release
```

```sh
lsmod | grep -E 'rtk_wifi6|cfg80211|compat'
```

```sh
readlink /sys/bus/pci/devices/0000:00:00.0/driver
```

```sh
iw dev
```

```sh
dmesg | grep -Ei 'rtk|8832|firmware|efuse|RFE|PHL|failed|error' | tail -100
```

Bei funktionierendem Probe erwartet: PCI-Treiber `rtk_wifi6` und ein
5-GHz-Wiphy. Ein Modul-Eintrag allein beweist noch keinen gestarteten AP.
SSID, WPA2-CCMP-Passwort und das tatsächliche Aufstellland anschließend in
LuCI konfigurieren. OpenWrts sichere Erstkonfiguration wird beibehalten;
es wird kein offenes Funknetz oder gemeinsames Standardpasswort angelegt.

## PC: nach Einrichtung des 5-GHz-Funknetzes

```sh
nmcli --ask device wifi connect 'DEINE-SSID' ifname wlp41s0
iw dev wlp41s0 link
ping -I wlp41s0 -c 20 192.168.1.1
```

Danach auf dem Router den tatsächlich von `iw dev` ausgegebenen AP-
Interfacenamen verwenden (netifd kann etwa `phy0-ap0` vergeben):

```sh
iw dev phy0-ap0 station dump
```

Vollständigen Bootlog und diese Ausgaben senden. WLAN-Schlüssel nicht
mitsenden; insbesondere kein ungefiltertes `wifi status` oder `uci show wireless`.
