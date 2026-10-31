# RTL8367B/C/D: OpenWrt-25.12-Abgleich für unseren 24.10-Tree

Verglichen am 31.10.2026 mit dem Commit und SHA256 aus
[source.json](source.json). Der große RTL8367D-Funktionsumfang war bereits
im 24.10-Tree enthalten. Es wird keine zweite Treiberkopie angelegt.

## Umsetzung

In `target/linux/generic/files/drivers/net/phy/rtl8367b.c`:

- 25.12-Allokationsprüfung für den D-Familien-VLAN-MC-Emulator: bei
  Speichermangel `-ENOMEM`, ohne weitere Registerinitialisierung.
- Freigabe dieses Speichers beim Entfernen des Treibers, wie in 25.12.
- Expliziter `platform_device.h`-Include.
- Zusätzliche Erkennung `0x6642/0x0030` als RTL8367D/RB-VC. 25.12 kennt bei
  ID `0x6642` nur Revision `0x0010` als RTL8367S-VB. `0x0030` ist im RD05
  gemessen; eine rohe WR1500-Revisionsmessung steht weiterhin aus.
- Eigener Enumwert und gemeinsame D-Familienprüfung für RGMII-Mux,
  EXT0/EXT1-Zuordnung, MAC-Force, PVID, VLAN/FID und Portstatus.
- Separates Compatible `realtek,rtl8367d-swconfig` für den optionalen
  Legacy-Pfad; `realtek,rtl8367d` bleibt dem DSA-Pfad vorbehalten.

Das anschließend eingereichte [RD05-Rekonstruktionsarchiv](../rd05-reconstructed-v2/README.md)
belegt die leere D-Initialisierungsroutine und die Magic-Probe-Sequenz.
Die neue D-Revision spielt deshalb keine C-Jam-Tabelle ab; der bisherige
S-VB-Pfad bleibt unverändert. Sowohl der Legacy-Treiber als auch der
DSA-Fehlerpfad (neuer Patch 341) schließen das ID-Fenster nach Lesefehlern.
Das Archiv liefert keinen Grund, die restliche Boardinitialisierung auszulassen.

Die 24.10-Plattformdaten-Anbindung und der `int remove()`-Callback bleiben
erhalten: Beide werden von Kernel 6.6 unterstützt. Die 25.12-Entfernung der
Plattformdaten und Verschiebung von `linux/rtl8367.h` sind nicht notwendig.
Das Paket `kmod-switch-rtl8367b` nennt jetzt auch C/D und ausdrücklich
swconfig. Kein Geräteprofil aktiviert es durch diese Änderung zusätzlich.

## Anbindung und Grenzen

| Pfad | Treiber | Board-Eigenschaften |
| --- | --- | --- |
| Vorhandenes DSA | rtl8365mb | mdc-/mdio-/reset-gpios, ports, CPU-Conduit |
| Optionales swconfig | rtl8367b | gpio-sck/gpio-sda oder mii-bus/phy-id; realtek,extif |

`realtek,extif` besteht aus zehn Zellen: CPU-Port, TX-Delay, RX-Delay,
Modus, Force, TX-Pause, RX-Pause, Link, Duplex und Speed-Enum. Für die
D-Familie ist CPU6=EXT0, CPU7=EXT1 die Treiberschnittstelle; das beweist
keine WR1500-Verdrahtung. Legacy-Reset verwendet einen Reset-Controller
namens `switch`, nicht automatisch die DSA-Eigenschaft `reset-gpios`.

Ein DSA-Knoten kann nicht durch bloßes Umbenennen mit swconfig laufen.
Beide Treiber dürfen nicht dieselbe Hardware gleichzeitig steuern. Der
RTL8197F-Conduit und die CPU-Tags fehlen durch einen Treiberwechsel nicht
plötzlich als Voraussetzung. Es werden keine WR1500-GPIOs geraten und
keine OEM-NAT-Shortcuts aktiviert. LAN/WAN und firewall4 sind später am
vollständigen Datenpfad zu prüfen.

## Prüfung und Build

Der vollständige geänderte C-Treiber kompiliert als MIPS-Little-Endian-
Objekt gegen Linux 6.6.114. Der Host-C-Test führt die echten Funktionen
mit SMI-Mocks aus: sechs alte/neue ID-/Revisionspaare, unbekannte
Revisionen, SMI-Lesefehler, Cleanup-Schreibfehler und fehlgeschlagene
VLAN-Allokation. Der bestehende DSA-Patchsatz einschließlich 341 wurde
zusätzlich angewandt und sein C-Treiber als MIPS-Objekt übersetzt. Kein
Hardwaretest, vollständiger Firmwarebau oder Modul-Link wird behauptet.

PC, separat ein IPK bauen; das bestehende WR1500- oder RD05-Profil behalten:

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
sed -i '/^CONFIG_PACKAGE_kmod-switch-rtl8367b=/d; /^# CONFIG_PACKAGE_kmod-switch-rtl8367b is not set$/d' .config
printf '%s\n' 'CONFIG_PACKAGE_kmod-switch-rtl8367b=m' >> .config
make defconfig
make target/linux/clean
make download -j8
make package/kernel/linux/compile -j"$(nproc)" V=s
```

`=m` baut das Paket separat, ohne es ins Image aufzunehmen. Reguläres Image
danach mit `make -j"$(nproc)" V=s`. Das allein aktiviert noch kein WR1500-LAN.

Router über UART, einzeln zur Bestandsaufnahme:

```sh
cat /etc/openwrt_release
```

```sh
ls /sys/bus/platform/drivers/rtl8367b
```

```sh
ls /sys/bus/platform/drivers/realtek-smi
```

```sh
ip -br link
```

Nicht vorhandene Treiberverzeichnisse sind bei ungeladenen Modulen normal.
Keine Registerschreib-/Reset-Kommandos oder paralleles Laden beider
Switch-Treiber sind Bestandteil dieser Bestandsaufnahme.
