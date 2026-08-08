# Gesamtpaket: RTL8197F / RT8197F + RTL8367RB Plattform

Dieses gemeinsame ZIP fasst die beiden zuvor analysierten Quellen zusammen:

1. **AX12v1 GPL** aus `GPL_AX12v1.tar.zst`
2. **MR62X SDK** aus `sdk_GPL_MR62X.tar.zst`

Hinweis zur Bezeichnung: In beiden Quellbäumen heißt die Plattform überwiegend **RTL8197F**. Die Schreibweise **RT8197F** wurde als Kurz-/Tippvariante behandelt.

## Kernaussage über beide SDKs

Beide Pakete beschreiben eine Realtek-Routerplattform mit **RTL8197F/RTL8197F_VG** als MIPS-SoC und **RTL8367RB/RTL8367RB-VC** als externem Gigabit-Switch. Der SoC bindet den Switch über einen CPU-/Host-Port per **RGMII** an. Die Switch-Register werden über GPIO-bitbanged **SMI/MDC/MDIO** angesprochen. Die wichtigen Codepfade liegen in Bootloader/U-Boot, Kernel-Target-Configs, `rtknet`-Ethernet/Switch-Treibern, Realtek-Switch-APIs und WLAN-HAL-/Backports-Daten.

## Gemeinsame technische Merkmale

- SoC/Target: `RTL8197F`, oft `RTL8197F_VG`
- Architektur: MIPS 24K, 32-bit, Little Endian
- Kernel-Familie: Linux 4.4.x
- BusyBox: 1.24.1
- Toolchain-Familie: Realtek MSDK `msdk-6.4.1-mips-EL-4.4-u0.9.33-m32ut-190619`
- Flash-Ziele: insbesondere SPI-NAND-Targets
- Ethernet/Switch: Realtek `rtknet`, `rtl819x`, `rtl83xx`, `rtl83xx_v1dot4`, ältere `rtl8367r`-API
- Externer Switch: RTL8367R/RB/RB-VC; für die untersuchten Targets besonders `CONFIG_RTL_8367RB_VC=y`
- Switch-API: `CONFIG_RTL_83XX_API_V1_4=y`
- GPIO-MDIO-Pinning im Kernel-Ziel: `CONFIG_RTL_MDC_H0_MDIO_G7=y`

## Wie die Plattform funktioniert

1. **Bootloader/U-Boot** initialisiert DRAM, Flash, SoC-Bonding/Boardprofil und gegebenenfalls den externen RTL8367RB-Switch.
2. **Frühe Switch-Initialisierung** setzt Reset-GPIOs, MDC/MDIO-/SMI-GPIOs und den Host-/CPU-Port. Der Host-Port wird typischerweise auf RGMII, Link Up, 1000 Mbit/s, Full Duplex und Flow-Control gezwungen.
3. **Linux-Kernel 4.4** startet als Realtek-MIPS-Target mit `CONFIG_SOC_RTL8197F=y` und bindet Realtek-Netzwerk-/Switch-Code ein.
4. **`rtknet`/`rtl819x`** stellt den SoC-NIC, Fastpath-/NAT-/Bridge-Komponenten und die Switch-Anbindung bereit.
5. **`rtl83xx_v1dot4` / `rtl8367r`** setzen Portrollen, VLANs, RGMII-Delays, CPU-Port/Tagging und Switch-Register.
6. **RootFS-/Init-Skripte** starten je nach SDK klassische Realtek-Init-Pfade (`rcS`, `rcS_GW`, `startup.sh`) oder OpenWrt-ähnliche Base-Files.
7. **WLAN** wird über Realtek Backports/WiFi-Treiber und 8197F-/mips_97f-HAL-/PHY-/RF-Daten eingebunden.

## Wichtige Config-Flags über beide Quellen

- `CONFIG_SOC_RTL8197F=y`
- `CONFIG_RTL_8197F=y`
- `CONFIG_RTL_8197F_GW=y`
- `CONFIG_RTL_8197F_VG=y`
- `CONFIG_SPI_NAND_FLASH=y`
- `CONFIG_RTL_8367RB_VC=y`
- `CONFIG_RTL_83XX_SUPPORT=y`
- `CONFIG_RTL_83XX_API_V1_4=y`
- `CONFIG_RTL_MDC_H0_MDIO_G7=y`

## Ordnerstruktur dieses Gesamtpakets

- `AX12v1_GPL_Paket/` – fokussierte Sammlung aus `GPL_AX12v1.tar.zst`
  - `README.md`
  - `docs/KURZ_ZUSAMMENFASSUNG.md`
  - `reports/` mit Inventar, Trefferlisten und Belegstellen
  - `source_subset/` mit relevanten Bootloader-, Board-, Treiber-, WLAN- und RootFS-Dateien
- `MR62X_SDK_Paket/` – fokussierte Sammlung aus `sdk_GPL_MR62X.tar.zst`
  - `README.md`
  - `docs/funktionsweise.md`
  - `docs/wichtige_code_config_belegstellen.md`
  - `analysis/` mit Inventaren und Config-Matrizen
  - `selected_sources/` mit relevanten OpenWrt-/Realtek-SDK-Dateien
- `VERGLEICH_AX12v1_vs_MR62X.md` – kurzer Vergleich der beiden Quellen
- `GESAMT_INVENTAR.csv` – Dateiindex des gemeinsamen Pakets mit Größe und SHA-256

## Unterschiede zwischen den Quellen

- **AX12v1 GPL** wirkt wie ein klassischer Realtek-Board-/BSP-Baum mit `rtl8197`-Top-Level, Bootcode, Board-Configs, RootFS-Skripten und Kernel-/Backports-Subtrees.
- **MR62X SDK** ist stärker OpenWrt-21.02-strukturiert und legt relevante Inhalte unter `sdk/openwrt-21.02/...` ab.
- Beide enthalten die entscheidenden RTL8197F-/RTL8367RB-Pfade, aber mit unterschiedlicher Verzeichnisstruktur und teils anderen Defconfig-Namen.
- Für die untersuchte Zielplattform zeigen beide auf den modernen `RTL_8367RB_VC` + `RTL_83XX_API_V1_4`-Pfad; ältere `rtl8367r`-Treiber bleiben als Kompatibilitäts-/Referenzpfad relevant.

## Nicht gemacht

Die Quellen wurden statisch analysiert und zusammengestellt. Es wurde nichts kompiliert, nicht auf Hardware getestet und nicht gegen ein laufendes Gerät verifiziert.
