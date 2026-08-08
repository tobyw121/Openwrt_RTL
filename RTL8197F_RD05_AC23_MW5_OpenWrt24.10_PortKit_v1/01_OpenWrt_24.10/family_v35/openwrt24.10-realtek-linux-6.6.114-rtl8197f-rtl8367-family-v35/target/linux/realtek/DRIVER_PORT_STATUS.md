# Realtek-Treiberport für OpenWrt 24.10 / Linux 6.6.114

## Versionsbasis

Der gelieferte OpenWrt-Tree nutzt `KERNEL_PATCHVER:=6.6` und `LINUX_VERSION-6.6 = .114`. Eine Kernelversion **6.5.114** existiert nicht; deshalb wird der technisch konsistente Zielstand **Linux 6.6.114** verwendet.

## Integrierte Zielplattformen

Der Realtek-Targetbaum enthält und aktiviert diese OpenWrt-Unterziele:

- `rtl8197f`
- `rtl838x`
- `rtl839x`
- `rtl930x`
- `rtl931x`

Die zuvor separat/deaktiviert abgelegten RTL83xx/RTL93xx-Patches sind unter `target/linux/realtek/patches-6.6/` aktiv. Der Baum enthält 37 Kernelpatches sowie die zugehörigen `files-6.6`-Overlays.

## Funktionsmatrix

| Komponente | Linux-6.6-Integration | Status |
|---|---|---|
| RTL8197F-MIPS-Plattform | Plattform-Kconfig, Little-Endian- und Boot-Anpassungen | integriert; Hardwaretest erforderlich |
| RTL8197F GPIO | `GPIO_RTL8197F` | integriert |
| RTL8197F SPI/Flash | Sheipa-SPI und SPI-ROM-Automap | integriert; Hardwaretest erforderlich |
| RTL8197F Ethernet | nativer `RTL8197F_RTKNET`-Pfad plus Diagnose | integriert; Hardwaretest erforderlich |
| RD05/RTL8367D | OEM-Vorinitialisierung und nativer RTL8367D-Pfad im RTL8365MB-DSA-Treiber | integriert; Hardwaretest erforderlich |
| RTL838x/RTL839x CPU-Port Ethernet | `NET_RTL838X` | integriert; Hardwaretest erforderlich |
| RTL838x/RTL839x/RTL93xx Switch | `NET_DSA_RTL83XX` | integriert; Hardwaretest erforderlich |
| RTL83xx PHY/SerDes | `REALTEK_SOC_PHY` | integriert; Firmware/Hardwaretest erforderlich |
| SMI/MDIO | `MDIO_REALTEK_OTTO_AUX`, MDIO-SMBus und SFP-SMBus | integriert |
| RTL8231 | MFD, Pinctrl/GPIO und LED-Scan-Matrix | integriert; LED-Symbol für rtl838x/rtl839x aktiviert |
| RTL9300 I²C/Mux | `I2C_RTL9300`, `I2C_MUX_RTL9300` | integriert |
| IRQ/Timer/Clock | Realtek IRQ-Chip, Otto-Timer und RTL83xx-Clock | integriert |
| GPIO/Watchdog/UART | gpiolib, Otto-Watchdog und 8250 | integriert/generisch ersetzt |
| SDK-RTK/DAL/ioctl-ABI | DSA, switchdev, bridge, tc, ethtool, devlink und Netlink | semantisch ersetzt; alte ABI nicht gebaut |
| RTL8328 | vollständige SDK-Quelle vorhanden | **kein verifizierter nativer Datenpfad** |
| RTL8389 | vollständige SDK-Quelle vorhanden | **kein verifizierter nativer Datenpfad** |
| RTL8208/RTL8212 | vollständige SDK-Quelle vorhanden | **spezielle Initialisierung nicht verifiziert** |

## Was „alle Treiber“ in diesem Tree bedeutet

Jede SDK-Datei ist im eingebetteten Legacy-Quellbaum erhalten und in `vendor/legacy-switch-sdk/PORT_MANIFEST.tsv` einem nativen Treiber, einem generischen Linux-Subsystem, einer Legacy-Steuer-API oder einer expliziten Hardwarelücke zugeordnet. Damit verschwindet keine Implementierung stillschweigend.

Eine proprietäre Linux-2.6-Steuer-API wird jedoch nicht durch Stubs als scheinbar funktionierendes Linux-6.6-Kernelmodul ausgegeben. Nur die nativen Subsystemtreiber sind für den Kernel-Build vorgesehen.

## Validierung und Grenzen

`scripts/validate-realtek-driver-port.sh` prüft Versionsstand, Subtargets, Patchsatz, Overlays, Kconfig-Symbole, SDK-Inventar, Prüfsummen, Manifestvollständigkeit und Konfliktmarker.

Nicht durchgeführt werden konnten:

- ein vollständiger OpenWrt-Firmwarebuild mit allen Feed-Quellen,
- ein exakter Patch-Dry-Run gegen einen lokal vorhandenen vollständigen Linux-6.6.114-Upstreamtree,
- Boot-, Interrupt-, DMA-, Switch- und PHY-Tests auf echter Hardware.

Diese Tests bleiben Voraussetzung, bevor Images auf Produktionshardware eingesetzt werden.
