# Kurz-Zusammenfassung: RTL8197F + RTL8367RB Plattform

Quelle: hochgeladenes Archiv `GPL_AX12v1.tar.zst`. Im Quellbaum wird durchgehend **RTL8197F** verwendet; die Schreibweise **RT8197F** scheint daher eine Kurz-/Tippvariante zu sein.

## Kernaussage

Die Plattform ist ein Realtek-Router/Gateway-SoC-Design mit **RTL8197F** als MIPS-basierter Host-CPU und **RTL8367RB / RTL8367RB-VC/VB** als externem Gigabit-Switch. Der Host hängt über eine erzwungene **RGMII-1-Gbit/s-Full-Duplex**-Verbindung am Switch. Die Switch-Steuerung läuft über eine SMI/MDC/MDIO-artige GPIO-Schnittstelle. Im Kernel-Zielprofil für `SPINAND_RTL8197F_VG_8832BR_8367R_KERN44_GW` sind die wichtigen Schalter gesetzt:

- `CONFIG_SOC_RTL8197F=y`
- `CONFIG_RTL_8197F=y`
- `CONFIG_RTL_8197F_GW=y`
- `CONFIG_RTL_8197F_VG=y`
- `CONFIG_RTL_8367RB_VC=y`
- `CONFIG_RTL_83XX_SUPPORT=y`
- `CONFIG_RTL_83XX_API_V1_4=y`
- `CONFIG_RTL_MDC_H0_MDIO_G7=y`

## Build-/Betriebsbasis

- Target/Board: `rtl8197F`
- Kernel: `linux-4.4`
- BusyBox: `busybox-1.24.1`
- Toolchain: `msdk-6.4.1-mips-EL-4.4-u0.9.33-m32ut-190619`
- CPU/Architektur laut Preconfig: MIPS, 32-bit, MIPS24K, Little Endian
- Hauptmodell im AX12v1-Profil: `SPINAND_RTL8197F_VG_8832BR_8367R_KERN44_GW`
- WLAN/Backports: `backports-5.2.8-1`, u. a. `CPTCFG_RTL8832BR=y` und `CPTCFG_RTL8852AE_BACKPORTS=y`; dazu Realtek-8197F PHY/RF/MAC-Parameterdateien.

## Boot- und Initialisierungsablauf

1. **Bootcode/U-Boot-artiger Realtek-Bootloader**
   - Auswahl über `bootcode/def-*8197f*8367rb*`.
   - Für VC/SPI-NAND ist exemplarisch `def-rtl8197fh_vg_8367rb_vc-spi_nand-config` relevant: `CONFIG_RTL8197F=y`, `CONFIG_RTL8197F_VG_SOC=y`, `CONFIG_SW_8367RB_VC=y`, `CONFIG_RTL_83XX_API_V1_4=y`, SPI-NAND-Boot aktiv.

2. **Switch-Vorinitialisierung im Bootloader**
   - `bootcode/boot/rtl8196x/swCore.c` setzt für RTL8197F + 8367R/83XX die Reset-GPIOs und die MDC/MDIO-GPIOs.
   - Wichtig ist die Reihenfolge: erst 819x/RGMII-Host-Port konfigurieren (`PCRP0`, `PITCR`, Force 1000M Full Duplex), danach `RTL83XX_INIT()` / Switch-Init. Der Quellkommentar sagt ausdrücklich, dass sonst ein erster Registerread `0x1202` fehlschlagen kann.

3. **Linux-Kernel / rtknet**
   - Das Kernelprofil aktiviert die Realtek-Switch-/Layered-Driver-Komponenten, Hardware NAT, IGMP/MLD Snooping, PPPoE-Beschleunigung und den 83xx-Switch-Support.
   - Bei `CONFIG_RTL_83XX_API_V1_4=y` wird `rtknet/drivers/net/rtl819x/rtl83xx_v1dot4` gebaut; die ältere `rtl8367r`-API bleibt im Paket relevant, weil ältere Bootcode- und Boardvarianten `CONFIG_SW_8367R` nutzen.

4. **Runtime / Rootfs-Skripte**
   - `inittab` startet `rcS`/`rcS_GW`/`rcS_AP` je nach Profil.
   - `rcS_GW` setzt `lo`, mountet `/proc` und ein RAMFS auf `/var`, legt Runtime-Verzeichnisse an, extrahiert Webpages per `flash extr /web`, startet `startup.sh`, `wlan_mod_utils.sh`, `init.sh gw all`, setzt Netzwerk-Sysctls, startet Watchdog/Webserver und `post_startup.sh`.
   - MP-Profile nutzen ein minimales `rcS`, starten u. a. `/bin/mp.sh`, `UDPserver` und `post_startup.sh`.

## Wichtige Treiber-/Source-Bereiche

| Bereich | Inhalt |
|---|---|
| `boards/rtl8197F/` | Boardauswahl, Kernel-/BusyBox-/Backports-Configs, Rootfs-Templates, `init.d`, BSP-Code (`gpio.c`, `setup.c`, `platform.c`, `mtd.c`, PCI/USB/I2S). |
| `bootcode/def-*8197f*8367rb*` | Bootloader-Defaultconfigs für RTL8197F + RTL8367RB-Varianten, SPI-Flash/SPI-NAND/DRAM-Varianten. |
| `bootcode/boot/rtl8196x/swCore.c` | frühe Host-Port-, RGMII-, Reset- und SMI/MDC/MDIO-Initialisierung. |
| `bootcode/boot/rtl8367r/` | ältere RTL8367RB-ASIC-API im Bootloader (`rtk_api.c`, `rtl8367b_asicdrv_*`, `smi.c`, `gpio.c`). |
| `bootcode/boot/rtl83xx_v1dot4/` | neuere 83xx API v1.4 im Bootloader, relevant für RTL8367RB-VC/VB-Auswahl. |
| `rtknet/drivers/net/rtl819x/rtl83xx_v1dot4/` | Kernel-Switch-DAL/API v1.4, eingebunden bei `CONFIG_RTL_83XX_API_V1_4`. |
| `rtknet/drivers/net/rtl819x/rtl8367r/` | ältere Kernel-API für RTL8367RB; relevant für ältere `CONFIG_RTL_8367R_SUPPORT`/`CONFIG_SW_8367R` Varianten. |
| `rtknet/drivers/net/rtl819x/common/smi.c` + `gpio.c` | GPIO-basierter Zugriffspfad für SMI/MDC/MDIO. |
| `backports-5.2.8-1/.../RTL8197F` und `.../Data/8197F` | WLAN-HAL, PHY/RF/MAC-Parameter und Kalibrierdaten für 8197F-basierte WLAN-Plattformen. |

## Wie der RTL8367RB angebunden ist

- Der Switch ist extern; der RTL8197F nutzt einen MAC/RGMII-Port als Host-/CPU-Verbindung.
- Der Bootcode setzt die Host-Verbindung in Force Mode: Link up, 1000M, Full Duplex, RX/TX Pause.
- In `rtl8367r/rtk_api.c` wird das externe Interface auf `MODE_EXT_RGMII` gesetzt, RGMII Delay konfiguriert, der CPU-Port aktiviert und CPU-Tagging auf `CPU_INSERT_TO_NONE` gesetzt.
- Die Steuerleitung ist je nach Boardbonding/Config unterschiedlich; das ausgewählte Kernelprofil nutzt `CONFIG_RTL_MDC_H0_MDIO_G7=y`. Andere Pfade im Code unterstützen C2/C3, G6/C3, D1/D7 und D5/D7.
- Reset erfolgt über GPIO, häufig H2 bzw. bei 97FN-Bonding F1.

## Wichtige Dateien im Paket

- `reports/inventory.csv`: kompletter Index des zusammengestellten Source-Subsets mit SHA-256.
- `reports/belegstellen_key_excerpts.txt`: zentrale Belegstellen/Code-Ausschnitte mit Zeilennummern.
- `reports/grep_hits_8197_8367_core.txt`: Suchtreffer für 8197F/8367R/8367RB im relevanten Quellbaum.
- `reports/archive_target_filelist_8197_8367.txt`: alle Archivpfade, deren Namen auf 8197F/8367R/8367RB passen.
- `source_subset/rtl8197/...`: kompakter relevanter Quellbaum.

## Nicht gemacht

Das Paket wurde statisch aus dem GPL-Quellarchiv analysiert und zusammengestellt. Es wurde **nicht kompiliert**, nicht auf Hardware getestet und nicht gegen ein laufendes Gerät verifiziert.
