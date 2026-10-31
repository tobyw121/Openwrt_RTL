# Cudy AX1500 / WR1500 R76: SPI-Analyse und Geräteprofil

## Ein Profil für RAM und Fullflash

Neu: **`cudy_ax1500_r76`** erzeugt
`openwrt-realtek-rtl8197f-cudy_ax1500_r76-squashfs-spi-full.bin`
mit exakt 16 MiB sowie SHA256 und `.spi-manifest.json`.
Im Menü das Profil mit **experimental** auswählen.
Dasselbe Profil baut auch das Initramfs-RAM-Image, wenn ramdisk aktiviert ist.
WR1500-UART- und Fullflash-Varianten sind keine separaten Menüeinträge mehr.
RE1500 und M1500 bleiben eigenständige Geräteprofile.

Der Builder `scripts/rtl8197f-cudy-fullflash.py` prüft den Originaldump über
uImage-CRCs und die kompilierte OEM-Partitionstabelle. Er ersetzt nur
0x40000–0xf00000. Bootloader, cfg, bdinfo sowie der gesamte Bereich ab
0xf00000 (OEM-rootfs_data und backup) bleiben bytegleich erhalten.
`CUDY_R76_SPI_TEMPLATE` kann beim Build auf den eigenen Originaldump zeigen.
Das Ergebnis enthält dessen private Gerätedaten und darf nicht als generische
Firmware für andere Geräte verteilt werden.

Für Linux 6.6 gilt ein **neues OpenWrt-Layout**: Kernel bei 0x40000 mit
4 MiB Budget, Rootfs bei **0x440000** mit 0xac0000 Bytes. Der separate
Fullflash-DT und die Signaturprobes benutzen dieselben Offsets. Die Rootfs
ist MTD-Partition 6. Ein neuer R76-uImage/cr6c-Container erhält korrekte
CRC32- und additive 16-Bit-Prüfsummen sowie RAM-Einstieg 0x82000000 passend
zum OpenWrt-Loader. Bei zu großem Kernel oder Rootfs bricht der Build ab.

PC-Build:

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git switch MW5_Stable
git pull --ff-only origin MW5_Stable
make menuconfig
# Realtek / RTL8197F / Cudy AX1500 / WR1500 R76, experimental
make defconfig
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

Das RAM-Image verwendet den OEM-DT mit Rootfs bei 0x2b0000 und rdinit;
das Flash-Image verwendet den OpenWrt-DT mit Rootfs bei 0x440000.
Die Auswahl von DT und Kommandozeile erfolgt pro Image im selben Profil.
Das neue Fullflash-Profil ist weiterhin experimentell: Ethernet/WLAN sind
nicht freigeschaltet. SPI ist lesend; Einstellungen sind ohne nutzbare
beschreibbare Overlay-Partition nicht persistent. Der erhaltene OEM-Overlay
wird absichtlich nicht als OpenWrt-Overlay eingebunden. Sysupgrade bleibt
gesperrt; dies ist keine Datei für das OEM-Webinterface. Ein bestätigter
Boot mit dem neuen Bootcontainer fehlt noch. Kein Flash-Befehl wird als
getestet ausgegeben. Auf dem OEM-Router für die nächste Prüfung einzeln:

```sh
cat /proc/mtd
cat /proc/meminfo
```

Hosttests prüfen Originalbereich-Erhalt, beide Containerprüfsummen,
gerade/ungerade Payloads und das Abweisen ungültiger/zu großer Eingaben.
Der Fullflash-DT wurde kompiliert und seine MTD-Reihenfolge kontrolliert.
Das ist kein vollständiger OpenWrt-Build oder Hardware-Funktionstest.

Quelle: `fullflash-templates/Cudy_AX1500.bin` aus Commit
`b2d07b498939386caf8e45eae3162ddbfe47e36a`, 16.777.216 Bytes.
SHA256: `f50f3147274a82fcbf4fef7a0c7500aeb7868b60e4e17975faea588237abd188`.
Die Antwort des Benutzers bestätigt die Zuordnung zu WR1500/R76; eine
konkrete Hardware-Revisionsnummer liegt weiterhin nicht vor.

## Befunde

Das Abbild entspricht einem vollständigen 16-MiB-SPI-Layout und enthält
nichtleere cfg-/bdinfo-Bereiche. Diese gerätespezifischen Daten werden nicht
als Defaults übernommen. Es handelt sich nicht um die bisher untersuchte
R76-Upgrade-Datei: Dort liegt der Firmwareinhalt um 0x10000 früher.

Das neue Abbild enthält OpenWrt 21.02-SNAPSHOT **2.3.0**, Kernel **4.4.176**,
Buildkennung im Kernel vom 21. Januar 2025. Der kompilierte Console-Parameter
ist `console=ttyS0,38400 root=/dev/mtdblock5`. Das zuvor analysierte
Upgrade-Image enthält 2.3.10 und einen anderen Kernel-Hash.

Ein uImage-Header bei **0x40000** nennt **R76**. Header- und Daten-CRC32
sind korrekt. Der darin liegende Realtek-Header `cr6c` bei **0x40040**
nennt Burn-Adresse **0x40000** und RAM-Einstieg **0x80a00000**. Der
LZMA-Alone-Stream bei **0x42858** entpackt vollständig zu 6.743.460 Bytes.
CRC32 belegt Konsistenz, keine verifizierte OEM-Authentizität.

Die folgende physische Partitionstabelle wurde aus acht aufeinanderfolgenden
32-Byte-Datensätzen im Kernel ab **0x631f98** gelesen. Die jeweiligen
Namenszeiger, Größen und Offsets wurden geprüft:

| OEM-Partition | Offset | Größe |
| --- | --- | --- |
| u-boot | 0x000000 | 0x020000 |
| cfg | 0x020000 | 0x010000 |
| bdinfo | 0x030000 | 0x010000 |
| firmware | 0x040000 | 0xec0000 |
| kernel | 0x040000 | 0x270000 |
| rootfs | 0x2b0000 | 0xc50000 |
| rootfs_data | 0xf00000 | 0x0f0000 |
| backup | 0xff0000 | 0x010000 |

`firmware` ist ein überlappender OEM-Sammelbereich. SquashFS 4/XZ liegt
tatsächlich bei 0x2b0000, verwendet 12.538.076 Bytes und nennt 2.996 Inodes.
Die OEM-Netzwerkskripte ordnen R76 LAN die SDK-Ports 3/0/1 und WAN Port 2
zu. Die zugehörigen OEM-Hostverbindungen 6@eth0/5@eth1 sind keine direkt
übertragbare Linux-DSA-Topologie oder vermessene Buchsenreihenfolge.

Das Rootfs wurde ausschließlich als Daten gelesen. Es enthält 2.474
reguläre Dateien, darunter `rtl8192cd.ko` und `rtk_wifi6.ko` für 4.4.176.
Letzteres ist 7.490.636 Bytes groß, SHA256
`b17226990e0f07e6efc3c04cdf0ea786aa9b301f314ecb777130be99139e3afb`.
Die PCI-Aliasse nennen 10ec:b832, 10ec:b814 und 10ec:0192. Das sind
Treiberfähigkeiten, keine ausgelesene PCI-ID dieses Geräts. RFE50-Dateien
sind enthalten; die reale Auswahl und die eFuse-/Factory-Kalibrierung sind
nicht dadurch bewiesen. Das Modul wird nicht auf Linux 6.6 übernommen.

Reproduzierbare, geräteprivate Daten nicht exportierende Header-/Kernelprüfung:

```sh
python3 scripts/rtl8197f-cudy-r76-inspect.py \
  fullflash-templates/Cudy_AX1500.bin \
  --report /tmp/cudy-r76-evidence.json
```

Die zu diesem Dump geprüften Ergebnisse stehen in [evidence.json](evidence.json).

## Neues Profil im bestehenden Realtek-Target

Profil: **`cudy_ax1500_r76`**, Menüname **Cudy AX1500 / WR1500 R76**,
Device Tree `rtl8197f_cudy_ax1500-r76.dts`, Boardkennung `cudy,ax1500-r76`.
Dies ist ein experimentelles kombiniertes RAM-/Fullflash-Profil; die
Hardwarefunktion ist weiterhin unbestätigt. RE1500 und M1500 bleiben als andere Geräte erhalten. Die neue Variante ergänzt:

* UART0 bei **38400 Baud, 8N1**, passend zur OEM-Kernelreferenz.
* Den bisherigen strapbasierten CPU-Taktdecoder und eine auf **64 MiB**
  begrenzte RAM-Testumgebung. Dies ist keine gemessene RAM-Ausstattung.
* Lesenden Zugriff auf den vorhandenen SPI-Automap, ohne JEDEC/SFDP-,
  WREN-, Schreib- oder Löschbefehle. Die Partitionen heißen `oem-*`, damit
  weder OEM-Rootfs noch OEM-Overlay automatisch für OpenWrt verwendet werden.
* Prüfung von uImage und SquashFS an den bekannten Offsets. Beide müssen
  denselben eindeutigen Byteversatz 0..3 bestätigen. Bei fehlender oder
  mehrdeutiger Übereinstimmung wird MTD nicht registriert. Bei Versatz >0
  können die ersten entsprechend vielen Bytes des gesamten Automap-Fensters
  nicht adressiert werden; solche Leseanforderungen werden zurückgewiesen.
  Die Erkennung setzt die unveränderten OEM-Signaturen voraus.
* PCIe-Enumeration und Ausgabe der tatsächlichen IDs über sysfs. Deren
  Funktion muss noch am Gerät geprüft werden. Kein Fremd-WLAN-Profil und
  keine statische Radio-ID werden zugewiesen.

Ethernet/DSA, WLAN, GPIO und LEDs bleiben deaktiviert. Die neue Variante
überspringt MW5-WLAN-/WebUI-Autostart und erhält keine erfundene LAN/WAN-
Topologie. Beide Sysupgrade-Handler verweigern Flash-Upgrades auch nach
einem erzwungenen Imagecheck. Der vorhandene SPI-Treiber verhält sich für
Boards ohne die neue optionale Probe-Eigenschaft wie zuvor; RD05/MW5
bekommen keine neuen Timings, Partitionen oder Hardware-Probes.

## Build auf dem Linux-PC

Im eigenen Checkout zunächst den Stand aktualisieren:

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git switch MW5_Stable
git pull --ff-only origin MW5_Stable
make menuconfig
```

Auswahl: **Target System → Realtek**, **Subtarget → RTL8197F**, **Target
Profile → Cudy AX1500 / WR1500 R76**. Unter **Target Images** **SquashFS** und **ramdisk** aktivieren, damit
beide Varianten entstehen. Die resultierenden Einträge sind:

```text
CONFIG_TARGET_realtek=y
CONFIG_TARGET_realtek_rtl8197f=y
CONFIG_TARGET_realtek_rtl8197f_DEVICE_cudy_ax1500_r76=y
CONFIG_TARGET_ROOTFS_INITRAMFS=y
```

Danach:

```sh
make defconfig
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

Bei einem Fehler den Build mit `make -j1 V=s` erneut starten.
Erwartetes Artefakt unter `bin/targets/realtek/rtl8197f/`:
`openwrt-realtek-rtl8197f-cudy_ax1500_r76-initramfs-ram-loader.bin`.
Zusätzlich wird das oben beschriebene Full-SPI-Image erzeugt.
Ein OEM-Factory-Upgrade oder Sysupgrade wird weiterhin nicht unterstützt. Der RAM-Loader liegt bei 0x82000000 und entpackt nach 0x80100000;
die bestehende Überlappungsprüfung bleibt aktiv. Es ist kein `bootm`-uImage
und kein OEM-Upgradecontainer.

## Erste Geräteprüfung

Tabby auf **38400 / 8N1**, Flusssteuerung aus. Vor einem RAM-Test fehlen
noch der vollständige OEM-Bootlog, die bestätigte zusammenhängende
RAM-Größe und die tatsächlich verfügbaren Bootloader-Transfer-/Sprungbefehle.
Ein ungeprüfter Flash-/TFTP-/Sprungbefehl wird daraus nicht erfunden.

Auf dem noch laufenden OEM-Router einzeln ausführen:

```sh
uname -a
cat /proc/cpuinfo
cat /proc/meminfo
cat /proc/mtd
cat /proc/bus/pci/devices
```

Nach einem anhand des Bootlogs verifizierten RAM-Start des neuen Profils:

```sh
cudy-ax1500-diag
```

Das sammelt Board-, Kernel-, Takt-/Speicher-, MTD- und PCI-Identitätsdaten,
ohne Konfiguration oder Flash zu ändern. Die MTD-Probe kann bei einem
abweichenden Read-Mapping fehlschlagen; der UART-/RAM-Test bleibt dafür
auswertbar. Erst mit diesen Live-Daten lassen sich das richtige Switch-
Profil und die erforderliche Wi-Fi-6-Portierung belastbar fortsetzen.

## Validierung

50 Python-/Host-C-/Shell-/Make-/DT-Prüfungen einschließlich der bestehenden
Cudy-, RD05- und MW5-Checks erfolgreich. Die neue echte C-Auswahlfunktion
wurde für alle vier Versätze, fehlende und mehrdeutige Signaturen getestet.
Der SPI-Treiber kompiliert für MIPS mit Linux 6.6.114. Der Loader mit der
neuen Boardkennung und Console-Zeile wurde mit synthetischer LZMA-Nutzlast
als Little-Endian-MIPS-Programm gebaut. Dazu wurden nur in der lokalen
Prüfkopie die Toolchainoption `-fhonour-copts` entfernt und Stock-GCC-
Link-/Endianoptionen gesetzt. Kein synthetisches Image wird ausgeliefert.

Diese Prüfungen sind kein vollständiger OpenWrt-Firmware-Build und kein
erfolgreicher Hardwareboot, Netzwerk- oder WLAN-Funktionstest.
