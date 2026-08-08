# Vergleich: AX12v1 GPL vs. MR62X SDK für RTL8197F + RTL8367RB

## Gemeinsamkeiten

Beide Quellen enthalten relevante Daten für eine Plattform aus RTL8197F/RTL8197F_VG-SoC und externem RTL8367RB/RTL8367RB-VC-Switch. Beide nutzen Linux-4.4-basierte Realtek-MIPS-Targets, BusyBox 1.24.1, Realtek-MSDK-Toolchain und rtknet-/rtl819x-Netzwerktreiber. Die zentrale Switch-Anbindung erfolgt über RGMII als Datenpfad und über GPIO-bitbanged SMI/MDC/MDIO als Managementpfad.

## AX12v1 GPL

- Quelle: `GPL_AX12v1.tar.zst`
- Struktur: klassischer Realtek-`rtl8197`-Baum
- Wichtige Inhalte:
  - `boards/rtl8197F/`
  - `bootcode/`
  - `rtknet/drivers/net/rtl819x/`
  - `backports-5.2.8-1/.../8197F`
  - RootFS-/Init-Dateien wie `rcS`, `rcS_GW`, `startup.sh`
- Zentrales Profil: `SPINAND_RTL8197F_VG_8832BR_8367R_KERN44_GW`
- Relevante Flags: `CONFIG_SOC_RTL8197F`, `CONFIG_RTL_8367RB_VC`, `CONFIG_RTL_83XX_API_V1_4`, `CONFIG_RTL_MDC_H0_MDIO_G7`

## MR62X SDK

- Quelle: `sdk_GPL_MR62X.tar.zst`
- Struktur: OpenWrt-21.02-basierter Realtek-SDK-Baum
- Wichtige Inhalte:
  - `target/linux/target/`
  - `target/linux/rtknet/`
  - `package/uboot/realtek/generic/`
  - `target/linux/linux-4.4/drivers/net/wireless/realtek/...`
  - `package/backports/src/drivers/net/wireless/realtek/...`
- Zentrales Profil: `SPINAND_RTL8197F_VG_8832BR_8367R_KERN44_GW` und verwandte MR62X-/EasyMesh-Varianten
- Relevante Flags: `CONFIG_SOC_RTL8197F`, `CONFIG_RTL_8197F_VG_V672`, `CONFIG_SPI_NAND_FLASH`, `CONFIG_RTL_8367RB_VC`, `CONFIG_RTL_83XX_API_V1_4`, `CONFIG_RTL_MDC_H0_MDIO_G7`

## Praktische Bewertung

Für Portierung, Debugging oder Reverse Engineering sollte man beide Quellen parallel nutzen:

- **AX12v1 GPL** ist hilfreich für klassische Realtek-Init-Skripte, Board-BSP, Bootcode-Details und ältere Quellpfade.
- **MR62X SDK** ist hilfreich für OpenWrt-21.02-Struktur, modernere Target-Integration, Config-Matrizen und U-Boot-/Kernel-Abgleich.
- Bei Switch-Problemen sind besonders wichtig:
  - GPIO-Pinning für MDC/MDIO/SMI
  - Reset-GPIOs
  - RGMII-Delay-Werte
  - CPU-Port/Host-Port-Konfiguration
  - Abgleich zwischen Bootloader-Defconfig und Kernel-Config
  - Unterscheidung zwischen altem `rtl8367r`-Pfad und neuem `rtl83xx_v1dot4`-Pfad
