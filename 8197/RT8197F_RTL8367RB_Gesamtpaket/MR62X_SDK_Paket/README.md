# Kurz-Zusammenfassung: RTL8197F / RTL8367RB aus sdk_GPL_MR62X

Quelle: `sdk_GPL_MR62X.tar.zst`  
Analysedatum: 2026-05-31 11:36:23 UTC

Hinweis zur Bezeichnung: In den Quellen heißt die SoC-/Board-Plattform überwiegend `RTL8197F`; die Benennung `RT8197F` wird hier als Kurz-/Tippvariante behandelt.

## Kernbefund

Das MR62X-SDK enthält eine OpenWrt-21.02-basierte Realtek-Plattform mit Linux `4.4.176`, MIPS 24K Little Endian, BusyBox `1.24.1`, Realtek MSDK Toolchain `msdk-6.4.1-mips-EL-4.4-u0.9.33-m32ut-190619`, Board-Modellen für `RTL8197F_VG/8832BR+8367R` und Varianten für SPI-NAND, EasyMesh und Dualstack.

Die wichtigste Kernel-Zielkonfiguration ist:

- `CONFIG_SOC_RTL8197F=y`
- `CONFIG_RTL_8197F=y`, `CONFIG_RTL_8197F_GW=y`, `CONFIG_RTL_8197F_VG=y`
- `CONFIG_RTL_8197F_VG_V672=y`, `CONFIG_RTL_8197F_VG_GPIO_V672=y`
- `CONFIG_SPI_NAND_FLASH=y` bei den SPINAND-Targets
- `CONFIG_RTL_8367RB_VC=y`
- `CONFIG_RTL_83XX_SUPPORT=y`
- `CONFIG_RTL_83XX_API_V1_4=y`
- `CONFIG_RTL_MDC_H0_MDIO_G7=y` für das Kernel-MDIO/GPIO-Pinning

Wichtig: In einigen Backports-Konfigurationen steht statt `RTL_8367RB_VC` noch `RTL_8367R_NEW_SUPPORT=y`; die Linux-4.4-Kernel-Configs für die 8367RB-Ziele zeigen jedoch konsistent `RTL_8367RB_VC=y` plus `RTL_83XX_API_V1_4=y`. Die EasyMesh-Backports-Configs sind ebenfalls auf `RTL_8367RB_VC`/API v1.4 gesetzt.

## Wie die Plattform grob funktioniert

1. **Bootloader/U-Boot** initialisiert RTL8197F/RTL8197F_VG, DRAM, SPI-NOR oder SPI-NAND und den externen Switch. Für RTL8367RB-VC existieren eigene Defconfigs wie `def-rtl8197fs_vg_8367rb_vc-spi_nand-config`.
2. **Kernel 4.4** startet als Realtek-MIPS-Target (`CONFIG_REALTEK`, `CONFIG_SOC_RTL8197F`). Die Ziel-RootFS-Konfiguration nutzt SquashFS und MTD-/NAND-Treiber für Realtek-Flash-Mapping.
3. **Ethernet/Switch** liegt unter `target/linux/rtknet/drivers/net/rtl819x`. Der interne SoC-NIC wird über `rtl819x_swNic.o` eingebunden; der externe RTL8367RB wird über Realtek-RTK-API angesprochen.
4. **Switch-Steuerung** läuft über `rtl83xx_v1dot4` bzw. ältere `rtl8367r`-Treiber. Die API v1.4 wird als `Realtek_Unmanaged_Switch_API_V1.4.0_20200611` bezeichnet.
5. **Management-Bus** zum Switch ist GPIO-bitbanged SMI/MDC/MDIO. Für das Kernel-Target ist `H0/G7` aktiv; mehrere U-Boot-Varianten nutzen je nach Board `H0/G7` oder `D1/D7`.
6. **Port-/VLAN-Logik** ist in `rtk_api.c` beschrieben: Für den 8367RB-Zweig werden Host-Port, WAN/LAN-Portmasken, VLAN IDs und RGMII Delay-Werte gesetzt. Im Code sind `WAN_VID=1`, `LAN_VID=2`; WAN ist im Default-Zweig Port 4, wenn keine Portmasken getauscht werden.
7. **WLAN** wird über Realtek Backports/WiFi-Treiber eingebunden. Die MR62X-Konfiguration setzt `CPTCFG_RTL8832BR=y`, `CPTCFG_RTL8852AE_BACKPORTS=y`, `CPTCFG_WLAN_8192FE=y`; 8197F-HAL-Daten liegen zusätzlich im Baum, werden in den Backports-Configs aber als `WLAN_HAL_8197F` nicht aktiv gesetzt.

## Wichtige Ordner im ZIP

- `selected_sources/sdk/openwrt-21.02/target/linux/target/` – Board-, Kernel-, BusyBox-, Userland- und RootFS-Configs.
- `selected_sources/sdk/openwrt-21.02/target/linux/rtknet/` – Realtek Ethernet-/Switch-/Fastpath-/NAT-Treiber.
- `selected_sources/sdk/openwrt-21.02/package/uboot/realtek/generic/` – U-Boot/Bootloader-Configs und Switch-Initialisierungscode.
- `selected_sources/sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/...` – Kernel-WLAN-HAL, PHY/RF-Tabellen, `mips_97f` Plattformdaten.
- `selected_sources/sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/realtek/...` – Backports-WLAN-Quellen und 8197F-Daten.
- `analysis/` – CSV-Inventar, Config-Matrix, U-Boot-Matrix, Dateibaum und Roh-Treffer.

## Besonders relevante Dateien

- `target/linux/target/board-configuration.in`
- `target/linux/target/preconfig_SPINAND_RTL8197F_VG_8832BR_8367R_Kernel4.4_GW`
- `target/linux/target/config.linux-4.4.SPINAND_RTL8197F_VG_8832BR_8367R_KERN44_GW`
- `target/linux/target/config.backports-5.2.8-1.SPINAND_RTL8197F_VG_8832BR_8367R_KERN44_GW_EASYMESH`
- `package/uboot/realtek/generic/def-rtl8197fs_vg_8367rb_vc-spi_nand-config`
- `target/linux/rtknet/drivers/net/rtl819x/Kconfig`
- `target/linux/rtknet/drivers/net/rtl819x/Makefile`
- `target/linux/rtknet/drivers/net/rtl819x/common/smi.c`
- `target/linux/rtknet/drivers/net/rtl819x/rtl83xx_v1dot4/rtk_api.c`
- `target/linux/rtknet/drivers/net/rtl819x/rtl8367r/rtl8367b_asicdrv.c`

## Inhalt dieses Pakets

Dieses ZIP enthält nicht das komplette SDK, sondern einen fokussierten Extrakt: relevante Quellen, Treiber, Configs, RootFS-Init-Dateien, U-Boot-Defconfigs, WLAN-HAL-/PHY-Daten und Analyse-Dateien für die RTL8197F + RTL8367RB/8367RB-VC Plattform.
