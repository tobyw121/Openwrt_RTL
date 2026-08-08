# Abschlussbericht: Realtek-Treiberport für OpenWrt 24.10

**Zielstand:** OpenWrt 24.10, Realtek MIPS, Linux **6.6.114**  
**Ausgangsgerät:** Xiaomi RD05 / RTL8197F / RTL8367D  
**Bearbeitungsdatum:** 24. Juli 2026

## 1. Ergebnis

Der gelieferte, zuvor reduzierte OpenWrt-Tree wurde wieder um den vollständigen Realtek-Targetumfang ergänzt. Die native Linux-6.6-Treiberfamilie für RTL8197F, RTL838x, RTL839x, RTL930x und RTL931x ist im Targetbaum vorhanden; alle 37 vorgesehenen Kernelpatches liegen aktiv unter `target/linux/realtek/patches-6.6/`.

Die angeforderte Version „Linux 6.5.114“ wurde nicht verwendet, weil diese Versionsnummer nicht existiert. Der gelieferte Tree ist auf `KERNEL_PATCHVER:=6.6` und `LINUX_VERSION-6.6 = .114` eingestellt; der offizielle Stable-Tag `v6.6.114` bezeichnet den Commit `4a243110dc884d8e1fe69eecbc2daef10d8e75d7`.

Die ursprüngliche RD05-Auswahl in `.config` ist bytegenau erhalten:

- `CONFIG_TARGET_realtek=y`
- `CONFIG_TARGET_realtek_rtl8197f=y`
- `CONFIG_TARGET_realtek_rtl8197f_DEVICE_xiaomi_r4_rd05=y`
- `CONFIG_LINUX_6_6=y`

## 2. Tatsächliche Portierungsänderungen

### Aktivierte Plattformen

`target/linux/realtek/Makefile` enthält:

```make
SUBTARGETS:=rtl8197f rtl838x rtl839x rtl930x rtl931x
KERNEL_PATCHVER:=6.6
```

### Aktivierter Kernelpatchsatz

20 zuvor separat als deaktiviert abgelegte RTL83xx-Patches wurden in die aktive 6.6-Serie übernommen. Zusammen mit den 17 bereits aktiven RTL8197F/RD05-Patches enthält der Tree jetzt 37 aktive Patches.

Die Patchserie bindet unter anderem ein:

- MIPS-Realtek-Plattform und IRQ-Chip,
- Otto-Timer und RTL83xx-Clocks,
- RTL838x/RTL839x/RTL930x/RTL931x Ethernet und DSA,
- RTL83xx-PHY/SerDes,
- MDIO-SMBus, SFP-SMBus und Otto-Aux-MDIO,
- RTL8231-MFD, GPIO/Pinctrl und LED-Scan-Matrix,
- RTL9300-I²C und I²C-Mux,
- RTL8197F-GPIO, SPI, MTD und Ethernet,
- RD05-OEM-Switch-Vorinitialisierung,
- native RTL8367D-Unterstützung im RTL8365MB-DSA-Pfad.

### Behobene Konfigurationslücke

Mehrere RTL838x/RTL839x-Device-Trees enthalten `realtek,rtl8231-leds`, während der LED-Treiber deaktiviert war. Deshalb wurde in beiden Unterzielen gesetzt:

```text
CONFIG_LEDS_RTL8231=y
```

Die übrigen benötigten RTL8231-Komponenten (`MFD_RTL8231`, `PINCTRL_RTL8231`, `GPIO_REALTEK_OTTO`, `MDIO_REALTEK_OTTO_AUX`) waren bereits aktiviert.

## 3. Umgang mit dem gelieferten SDK

Das SDK enthält 962 reguläre Dateien und 14 symbolische Links. Es stammt aus der Linux-2.6.32-Ära und umfasst BSP-, NIC-, GPIO-, IRQ-, SMI-, RTL8231-, Switch-Core-, LED-, UART-, Watchdog-, PHY-, DAL-, HAL-, RTK- und Diagnosecode.

Die kompletten Originalquellen sind unverändert eingebettet unter:

```text
target/linux/realtek/vendor/legacy-switch-sdk/source/
```

Sie werden nicht automatisch gebaut. Für jede der 976 Inventareinheiten existiert ein Eintrag in:

```text
target/linux/realtek/vendor/legacy-switch-sdk/PORT_MANIFEST.tsv
```

Das Manifest enthält Pfad, Typ, Größe, SHA-256, Komponente, Portierungsstatus, Linux-6.6-Ersatz und Build-Richtlinie.

### Statusverteilung des SDK-Inventars

| Status | Einträge |
|---|---:|
| Native Integration | 12 |
| Native Integration, Hardwarevalidierung nötig | 51 |
| Native semantische Ersetzung | 58 |
| Generischer Kernelersatz | 5 |
| Generischer Kernelersatz, Hardwarevalidierung nötig | 2 |
| Ersetztes Linux-2.6-BSP | 85 |
| Ersetzte Legacy-Kernelmodule | 52 |
| Ersetzte Legacy-Kernelglue | 45 |
| Erhaltene proprietäre Steuer-API | 161 |
| Erhaltene Legacy-Supportbibliothek | 13 |
| Erhaltene Chip-Lücke | 168 |
| Erhaltene PHY-Lücke | 6 |
| Build-Unterstützung/Referenz/Test | 317 |
| Manuelle Restprüfung | 1 |
| **Gesamt** | **976** |

### Warum die alten Kernelmodule nicht direkt geladen werden

`rtcore`, `rtdrv`, `rtk` und `rtnic` verwenden entfernte Linux-2.6-Schnittstellen und bilden einen parallelen proprietären Netzwerk-/Switch-Stack. Ein Build über Kompatibilitätsstubs würde nur Kompilierbarkeit vortäuschen und weder DMA-, IRQ-, Register- noch Paketpfadkorrektheit beweisen.

Stattdessen sind die Funktionen den Linux-6.6-Subsystemen zugeordnet:

| Legacy-Funktion | Nativer Ersatz |
|---|---|
| BSP/IRQ | MIPS-Realtek-Plattform und Linux-IRQ-Chip |
| NIC/DMA | `NET_RTL838X`, NAPI, DSA-Master |
| Switch-Core/DAL/RTK | DSA, switchdev, bridge, tc, ethtool, devlink, Netlink |
| SMI/MDIO | Linux-MDIO und `MDIO_REALTEK_OTTO_AUX` |
| PHY/SerDes | `REALTEK_SOC_PHY` |
| RTL8231 | MFD, GPIO/Pinctrl und LED-Klasse |
| GPIO | gpiolib/pinctrl |
| Watchdog | Linux-Watchdog-Framework |
| UART | 8250/OF |
| Flash | SPI-NOR/MTD |

## 4. Explizit nicht als funktionsfähig behauptete Teile

### RTL8328

Das SDK enthält umfangreiche RTL8328-Registertabellen, NIC-, BSP-, Watchdog-, PHY- und DAL-Implementierungen. Im Linux-6.6-Tree ist jedoch kein vollständiger, nachgewiesener RTL8328-DSA/Ethernet-Datenpfad vorhanden. Die Quelle bleibt erhalten, wird aber nicht als funktionierender Kernelport ausgegeben.

### RTL8389

Der Tree kennt eine RTL8389-Familienkonstante, besitzt aber keinen verifizierten vollständigen DSA/Ethernet-Datenpfad für die SDK-spezifische RTL8389-Implementierung. Auch diese Quelle ist vollständig inventarisiert und bewusst nicht aktiviert.

### RTL8208/RTL8212

Die speziellen SDK-Initialisierungsfolgen für diese Multiport-PHYs sind nicht als exakter Linux-6.6-Port nachgewiesen. Grundlegendes Clause-22-Probing ist kein Ersatz für einen geprüften Initialisierungsablauf.

Diese drei Punkte sind echte Portierungslücken und keine versteckten „Erfolge“.

## 5. Validierung

### Statische Portprüfung

Ausgeführt wurde:

```sh
./scripts/validate-realtek-driver-port.sh
```

Ergebnis nach Patchfix 1:

```text
PASS=134
WARN=4
FAIL=0
```

Geprüft wurden unter anderem:

- Kernelversion und ausgewähltes Gerät,
- alle fünf Realtek-Unterziele,
- Anzahl und Namen aller 37 Patches,
- strikte Unified-Diff-Analyse aller Patches mit `git apply --numstat`,
- Syntaxanalyse aller Patches mit GNU `patch`,
- erwartete Kernel-Overlaytreiber,
- notwendige Kconfig-Symbole pro Unterziel,
- RTL8231-LED-DTS-Nutzung,
- vollständiges SDK-Inventar und SHA-256-Prüfsummen,
- reproduzierbare Manifestgenerierung,
- Konfliktmarker und leere C-/Headerdateien.

### Patchfix 1: korrigierte Hunk-Zähler

Beim ersten realen Kernel-Prepare-Lauf wurde sichtbar, dass die in diesem Port
zusätzlich eingefügten SoC-Abhängigkeiten in den Patches 720 und 723 nicht in
deren Hunk-Zählern berücksichtigt waren. Dadurch brach GNU `patch` mit
`malformed patch` ab. Die Hunk-Zähler und Diffstat-Angaben wurden korrigiert.
Fünf ältere, von GNU `patch` tolerierte, aber formal inkonsistente Hunks wurden
ebenfalls normalisiert.

Patches 720 und 723 wurden anschließend mit `--fuzz=0` gegen den relevanten
Linux-6.6.114-Dateistand nach den betroffenen OpenWrt-Generic-Transformationen
angewendet. Beide Tests waren erfolgreich. Der Validator enthält jetzt zwei
strikte Parserprüfungen, sodass falsche Hunk-Zähler künftig als Fehler erkannt
werden. Details stehen in `README_PATCHFIX_720_723.md`.

### OpenWrt-Metadatenprüfung

`make defconfig` wurde mit einem ausschließlich temporären POSIX-`asort()`-Fallback ausgeführt, weil der Prüfhost nur `mawk` statt GNU `awk` besitzt.

Ergebnis:

```text
exit_code=0
errors=0
warnings=39
```

RD05 und Linux 6.6 blieben ausgewählt. Die 39 Warnungen betreffen 14 bereits im reduzierten Ausgangsbaum fehlende Feed-/Paketabhängigkeiten:

`cgi-io`, `csstidy/host`, `kmod-phy-bcm-ns-usb2`, `kmod-phy-bcm-ns-usb3`, `libintl-full`, `liblucihttp-ucode`, `libnetfilter-conntrack`, `libnettle`, `libopenssl`, `libpam`, `libtirpc`, `libwolfssl`, `luasrcdiet/host`, `ubi-utils`.

Die ursprüngliche `.config` und `include/scan.awk` wurden danach exakt wiederhergestellt.

### Kernel-Prepare-Test

OpenWrt forderte korrekt:

```text
linux-6.6.114.tar.xz
SHA-256: ca4175a03ce2943ae192d77ad91e37ee292f1f1bb7b2954b062b0ef7eb0cb97c
```

Der Lauf wurde vor dem Treibercompile beendet, weil der Container `cdn.kernel.org` und `mirrors.mit.edu` nicht per DNS auflösen konnte. Der externe Binärdownload über das Dateitool war ebenfalls durch dessen MIME-Filter gesperrt.

## 6. Nicht durchgeführte Nachweise

Folgende Nachweise sind weiterhin erforderlich:

1. Vollständiger OpenWrt-Build mit vollständigen Feeds und lokal verfügbarem Kernelarchiv.
2. Patchanwendung gegen einen kompletten lokalen Linux-6.6.114-Upstreambaum.
3. Boot auf RD05 und auf repräsentativen RTL838x/RTL839x/RTL930x/RTL931x-Geräten.
4. Prüfung von IRQ, Timer, Clock, GPIO, SPI/MTD, UART und Watchdog.
5. DMA-/NAPI-/Paketpfadtest des Ethernet-Treibers.
6. DSA-, VLAN-, FDB-, STP-, LAG-, QoS-, SFP- und PHY/SerDes-Tests.
7. Gesonderte Neuentwicklung oder belastbare Hardwarevalidierung für RTL8328, RTL8389 und RTL8208/RTL8212.

## 7. Reproduzierbare Prüfkommandos

Aus dem Wurzelverzeichnis:

```sh
./scripts/validate-realtek-driver-port.sh
make defconfig
make -j1 target/linux/prepare V=s
make -j1 target/linux/compile V=s
```

Für die letzten drei Befehle werden GNU `awk`, die OpenWrt-Hostabhängigkeiten, vollständige Feeds sowie das Kernelarchiv benötigt.

## 8. Eingangsprüfsummen

```text
bc4e680ff19b5d7483b62ae8a16a2a048d69d7b75c2881ee88686cc7afee4c89  sdk.tar.zst
2d4f9deedb82843deac206162d0b22a237c4c5e37d7353803a4a6aa9f66497ac  openwrt24.10-rtl8197f-rtl8367d-clean.tar.zst
```

## 9. Tree-Statistik

- 4.077 reguläre Dateien
- 17 symbolische Links
- 832 Verzeichnisse
- ca. 103 MB logische Nutzdaten
- 37 aktive Kernelpatches
- 680 Kernel-Overlaydateien
- 77 DTS-/DTSI-Dateien
