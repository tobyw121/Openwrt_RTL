# WR1500: Patchübernahme und Wi-Fi-6-Portierungsstand

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
# Realtek / RTL8197F / Cudy AX1500 R76 experimental full SPI image; SquashFS
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
