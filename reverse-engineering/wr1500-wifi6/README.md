# WR1500: Patchübernahme und Wi-Fi-6-Portierungsstand

## Ergänzung: G6-OS-Port und Rekonstruktionsarchiv, 31.10.2026

Der neue [OS-Service-Patch](linux66-os-service-api.patch) wird nach
`linux66-first-api.patch` auf den separat extrahierten G6-Quellbaum angewandt.
Er ist kein Patch für das gleichnamige oder ähnlich aufgebaute rtl8192cd-Paket.

Tatsächlich umgesetzt und als `os_dep/osdep_service_linux.o` für MIPS LE /
Linux 6.6.114 übersetzt:

- kernel_read/kernel_write-basierte Dateihelfer ohne get_fs/set_fs für
  Kernel ab 5.10; Fehler werden weitergegeben, neue Dateien mit 0600 angelegt;
- Größenprüfung für die int/u32-Schnittstellen der Dateihelfer;
- path_put nach erfolgreicher Verzeichnisauflösung;
- eth_hw_addr_set statt Schreiben in dev_addr;
- get_random_u32 statt entfernter prandom_u32-API;
- INIT_CSD statt Zugriff auf interne csd.flags für die SMP-Initialisierung.

Nach dieser Änderung wurde der vollständige Modulbau erneut versucht.
Die nächste tatsächlich beobachtete Sperre ist
`platform/platform_mips_97f_pci.c`: alte pci_dma_sync_single_*,
pci_map_single/pci_unmap_single und pci_alloc_consistent/pci_free_consistent.
Ein Namensaustausch allein reicht nicht: Der OEM nutzt auch ungeprüfte
DMA-Mappings in void-TX-Helfern und einen virt_to_phys-Fallback. Für einen
betriebsfähigen Port müssen Mappingfehler bis zu TX-Aufrufern durchgereicht,
bereits angelegte Mappings auf Fehlerpfaden abgebaut und die Ownership-/
Cache-Regeln der PHL-Ringe geprüft werden. Der weitere cfg80211- und
OpenWrt-Backports-Abgleich ist danach weiterhin nötig. Ein fertiges
rtk_wifi6.ko oder ein AX-AP-Start wird nicht behauptet.

### Was von rtl8192cd wiederverwendbar ist

| Bereich | Bewertung |
| --- | --- |
| Kernel-6.6-/Backports-Buildmuster | Als Vorlage nutzbar |
| netifd/hostapd/nl80211-Integration | Erfahrungen und ABI-Prüfungen nutzbar; die G6-cfg80211-Callbacks müssen eigene Adapterzustände verwenden |
| Board-/Factory-Validierung | Prinzip nutzbar; RD05-H601-Offsets/RFE nicht als WR1500-Daten übernehmen |
| PCIe-HAL und TX/RX-Deskriptoren | Kein direkter Ersatz: G6 hat eigene PHL-/HAL-Strukturen und AX-Hardwarepfade |
| Firmware und RFE50 | RTL8832BR-spezifische G6-/OEM-Daten erforderlich |
| Private Fast-NAT-/Bridge-Shortcuts | Nicht als OpenWrt-Datenpfad übernehmen |

Im G6-Pfad steht in `os_dep/linux/pci_intf.c` ausdrücklich
`PCI_DEVICE(0x10ec, 0xb832)` unter `CONFIG_RTL8832BR`. Im aktuellen
rtl8192cd-PCI-Wrapper fehlt diese Chipunterstützung. Die ID dort lediglich
einzutragen würde einen falschen HAL auf AX-Hardware starten.
Der richtige Ausbau besteht daher aus einem separaten G6-Backend/Modul mit
unseren OpenWrt-Anpassungsmustern, nicht dem Vermischen der AC- und AX-HALs.

Das neu eingereichte [Rekonstruktionsarchiv](../rd05-reconstructed-v2/README.md)
enthält keinen RTL8832BR-/G6-HAL und keine entsprechenden Funktionssymbole.
Es verbessert die RD05-Switch-/rtl8192cd-Referenz, ersetzt aber die G6-Quelle
nicht. `RTW_WKARD_PWRLIMIT_DISABLE`, private RX-/NAT-Hooks sowie Firmware- und
regulatorische Vorgaben bleiben vor jedem späteren RF-Betrieb zu bereinigen.

Die folgenden Abschnitte dokumentieren frühere Prüfstände. Insbesondere ist
die R76-RAM-Freigabe inzwischen mit MemTotal 120140 kB am Gerät bestätigt;
das belegt keine WLAN-Funktion.

## Bestätigte PCI-ID und fehlendes Diagnosepaket

Der Benutzer hat unter `r0-4608f15f18` für `0000:00:00.0` tatsächlich
vendor `0x10ec`, device `0xb832` und class `0x028000` gelesen. Es existiert
kein driver-Symlink: PCIe-Enumeration ist vorhanden, aber kein WLAN-Treiber
gebunden. Die PCI-Identität wird im Cudy-Profil jetzt als Runtime-Beleg
geführt. Sie ist kein Beleg für funktionierenden Funkbetrieb.

Die Benutzer-.config enthält das R76-Geräteprofil, aber keine aktive Auswahl
von `rtl8197f-cudy-ax1500-base`; auch opkg meldet kein installiertes Paket.
Warum die DEVICE_PACKAGES-Vorauswahl im konkreten Build nicht übernommen
wurde, ist ohne dessen generierte Metadaten nicht abschließend bewiesen.
Das Basispaket erhält deshalb zusätzlich einen ausdrücklichen Kconfig-
Default für die Cudy-Profile (Einzel- und Mehrgeräteauswahl). Eine vorhandene
explizite Abwahl wird dadurch nicht überschrieben. Der reale Installations-
Make-Schritt wurde auf sein ausführbares `/usr/sbin/cudy-ax1500-diag` geprüft.
Die Diagnose zeigt bei vorhandener PCI-Funktion nun auch `driver=unbound`.

PC, um die bereits gespeicherte Paketabwahl zu korrigieren:

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
rm -f tmp/.packageinfo tmp/.config-package.in
sed -i '/^CONFIG_PACKAGE_rtl8197f-cudy-ax1500-base=/d; /^# CONFIG_PACKAGE_rtl8197f-cudy-ax1500-base is not set$/d' .config
printf '%s\n' 'CONFIG_PACKAGE_rtl8197f-cudy-ax1500-base=y' >> .config
make defconfig
grep '^CONFIG_PACKAGE_rtl8197f-cudy-ax1500-base=' .config
make -j"$(nproc)" V=s
```

Nach dem nächsten regulären Firmwarewechsel auf dem Router einzeln:

```sh
opkg status rtl8197f-cudy-ax1500-base
cudy-ax1500-diag
```

Das behebt das fehlende Diagnosepaket. Es ersetzt weder die nachfolgend
beschriebene Wi-Fi-6-Portierung noch eine Ethernet-/Switch-Anbindung.

Stand 2026-10-31. Der Benutzerpatch
`Openwrt_RTL_MW5_WR1500_RD05_changes_20261031.patch` wird gezielt übernommen:
Die bereits integrierten RD05-Korrekturen (einschließlich identischem Patch
340 und rd05-dsa-check) werden nicht dupliziert. Das alte WR1500-RAM-Profil
erhält 38400 Baud in DT, UART-current-speed und Loader-Kommandozeile. Die
neuen R76-RAM-/Fullflash-Profile verwenden diese Baudrate bereits.

Die behaupteten 128 MiB werden ohne den ursprünglichen OEM-Messbeleg nicht
als nutzbarer Linux-Speicher eingetragen. Die 64-MiB-Testgrenze bleibt im
DT erhalten. Ein Loader-Platzierungslimit wäre kein Ersatz für diesen
Linux-Speicherbereich. Die abweichende rootfs_data-Endadresse 0xfe0000 wird
nicht übernommen; der vorliegende SPI-Kernel belegt 0xff0000.

## Neu gefundene passende Quellreferenz

Die TP-Link-GPL `GPL_ax1500v3.tar.gz`, SHA256
`6d133598b134b6878ce802a53b3061d9f2efb3636ad0d31ec4cb3bcbdf438f2a`,
enthält unter
`GPL_ax1500v3/rtl8198/linux-4.4.x/drivers/net/wireless/realtek/g6_wifi_driver/`
einen umfangreichen G6-Quellbaum (4.444 reguläre Dateien). Dieser enthält:

* `autoconf_rtl8832br_pci_linux.h`, RTL8832BR-PHL/HAL und Firmware;
* `platform/mips_97f.mk` und eine RTL8197F-Plattformanbindung;
* `platform/mips_98d/rtl8832bre/RFE50/`-Parameterdateien.

Dies ist eine konkrete Quelle für die weitere Portierung. Dass der
AX1500v3-Switch ein anderer ist, schließt die Verwendung seiner WLAN-
Quellreferenz nicht aus. Identische Revision oder Laufzeitfunktion auf
WR1500 ist damit nicht bewiesen.

## Tatsächlich geprüfter Portierungsschritt

[linux66-first-api.patch](linux66-first-api.patch) ist ein **separater,
unvollständiger Entwicklungspatch**, nicht auf den bestehenden MW5-/RD05-
Treiber anzuwenden. Er passt G6-Thread-Exit, zwei MAC-Adressen-Zuweisungen
und den normalen RX-Pfad ohne das private skb-Feld `from_dev` an.
Mit RTL8832BR-, RTL8197F- und cfg80211-Konfiguration kompiliert dadurch
`os_dep/linux/os_intfs.o` gegen den MIPS-Kernel 6.6.114.

Der vollständige Modulbuild wurde ebenfalls versucht und **schlägt weiterhin
fehl**: alte `get_fs/set_fs/mm_segment_t`-Dateizugriffe, entfernte
`pci_dma_sync_single_*`-/`pci_map_single`-APIs, `prandom_u32` und direkter
Zugriff auf interne SMP-Strukturen. Diese müssen korrekt portiert werden;
fehlende APIs werden nicht durch Dummy-Funktionen ersetzt. cfg80211 muss
anschließend gegen die OpenWrt-Backports gebaut werden. Das Herstellerprofil
definiert außerdem `RTW_WKARD_PWRLIMIT_DISABLE`; diese Policy ist vor einem
RF-Betrieb zu prüfen und darf nicht blind als OpenWrt-Default übernommen werden.

Privates Arbeitsverzeichnis:
`/workspace/rtl-build-validation/wr1500-wifi6/`.
Buildlogs: `wr1500-wifi6-97f-build.log` (Objekt bestanden) und
`wr1500-wifi6-module-build.log` (Modul fehlgeschlagen) im übergeordneten
Analyseverzeichnis. Der komplette Quellbaum wurde nicht ungeprüft in ein
automatisch gestartetes Kernelpaket übernommen.

**Kein vollständiges rtk_wifi6.ko für Kernel 6.6, kein WLAN-Autostart und
kein erfolgreicher Hardwareboot werden hier behauptet.** Die integrierte
2,4-GHz-Funkstrecke benötigt zusätzlich einen expliziten WR1500-Boardpfad
mit geprüfter H601-/Factory-Zuordnung. Ein RD05-Profil wird dafür nicht
ersatzweise geladen.

## Build und Gerätebelege

PC, vorhandenes Fullflash-Profil (noch ohne WLAN):

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git switch MW5_Stable
git pull --ff-only origin MW5_Stable
make menuconfig
# Realtek / RTL8197F / Cudy AX1500 R76 experimental; SquashFS
make defconfig
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

OEM-Router über UART, jeweils einzeln:

```sh
cat /proc/meminfo
cat /proc/bus/pci/devices
cat /proc/mtd
```

Diese Ausgaben ergänzen die statischen Referenzen. Sie reparieren weder
die noch offenen Kernel-APIs noch belegen sie einen WLAN-Start unseres Ports.
