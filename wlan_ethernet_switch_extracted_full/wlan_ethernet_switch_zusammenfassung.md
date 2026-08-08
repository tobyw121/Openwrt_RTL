# WLAN- und Ethernet/Switch-Treiberinventar
Quelle: `GPL_AX12v1.tar.zst` und `sdk_GPL_MR62X.tar.zst` (lokal analysierte Archivlisten).
## Kurzfazit
- **GPL_AX12v1** enthält primär den Realtek-Backports-WLAN-Treiber `rtl8192fe/rtl8192cd` inkl. HAL/PHYDM/WPA3 und viele RF-/Firmware-Daten sowie den Realtek-`rtknet`-Ethernet-/Switch-Baum für RTL819x/RTL83xx/RTL8367/RTL865x.
- **sdk_GPL_MR62X** enthält zusätzlich zum `rtl8192cd`/Backports-Baum einen großen **Realtek G6 WiFi 6**-Treiberbaum mit RTL8852/RTL8832-Bezügen und praktisch denselben `rtknet`-Switch-/Ethernet-Baum plus U-Boot-Switchsupport.
- Board-/Build-Daten nennen u. a. Varianten mit **RTL8197F**, **RTL8812/8814/8832**, **RTL8852** und externem **RTL8367R/RB** Switch.
## Mengenübersicht
| Archiv | Bereich | Dateien | Größe MiB | Häufigste Klassen |
|---|---:|---:|---:|---|
| GPL_AX12v1 | Ethernet/Switch | 1003 | 28.0 | Realtek RTL83xx/RTL8367/RTL8325 switch driver data (462), Bootloader switch support (323), Realtek RTL865x/fastpath Ethernet driver data (96), Realtek RTL819x Ethernet/Switch SDK driver (67) |
| GPL_AX12v1 | Ethernet/Switch + WLAN | 115 | 0.8 | Board/build configuration (115) |
| GPL_AX12v1 | WLAN | 4830 | 111.2 | Realtek rtl8192fe/rtl8192cd backports driver (2355), WLAN subsystem/build support (1691), Linux staging Realtek WLAN driver (283), Linux rtlwifi driver family (262) |
| sdk_GPL_MR62X | Ethernet/Switch | 1020 | 29.0 | Realtek RTL83xx/RTL8367/RTL8325 switch driver data (462), Bootloader switch support (323), Realtek RTL865x/fastpath Ethernet driver data (107), Realtek RTL819x Ethernet/Switch SDK driver (65) |
| sdk_GPL_MR62X | Ethernet/Switch + WLAN | 117 | 0.9 | Board/build configuration (117) |
| sdk_GPL_MR62X | WLAN | 11479 | 372.3 | Realtek G6 WiFi 6 driver (2801), WLAN subsystem/build support (2752), Realtek rtl8192fe/rtl8192cd backports driver (2357), Realtek rtl8192cd WLAN driver (2356) |

## Wichtigste Komponenten
### GPL_AX12v1
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/realtek/rtl8192fe/WlanHAL/Data` — 723 Dateien, 5.81 MiB; Chips/Keywords: RTL8192FE, RTL8881A, RTL8822C, RTL8812, RTL8192F, RTL8814B, RTL8814A, RTL8192E, RTL8197F, RTL8198F
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/realtek/rtl8192fe/wpa3/src_mbedtls` — 222 Dateien, 3.41 MiB; Chips/Keywords: RTL8192FE, WPA3
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/realtek/rtl8192fe/phydm/halrf` — 217 Dateien, 8.92 MiB; Chips/Keywords: RTL8192FE, RTL8812, RTL8822C, RTL8814B, RTL8197G, RTL8198F, RTL8814A, RTL8197F, RTL8192F, RTL8821C
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/realtek/rtl8192fe/WlanHAL/RTL88XX` — 206 Dateien, 2.69 MiB; Chips/Keywords: RTL8192FE, RTL8197F, RTL8881A, RTL8197G, RTL8198F, RTL8814B, RTL8812, RTL8192E, RTL8822C, RTL8821C
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/realtek/rtl8192fe/WlanHAL/HalMac88XX` — 201 Dateien, 22.86 MiB; Chips/Keywords: RTL8192FE, RTL8821C, RTL8822C, RTL8812, RTL8814B, RTL8197F
- **Ethernet/Switch** – Realtek RTL83xx/RTL8367/RTL8325 switch driver data: `GPL_AX12v1/rtl8197/rtknet/drivers/net/rtl819x/rtl83xx_v1dot4/dal` — 170 Dateien, 4.63 MiB; Chips/Keywords: RTL819X, RTL83XX, RTL8367C, RTL8367D
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/intel/iwlwifi` — 162 Dateien, 3.49 MiB; Chips/Keywords: MAC80211
- **WLAN** – madwifi WLAN package/patches: `GPL_AX12v1/Iplatform/openwrt/package/madwifi` — 161 Dateien, 0.75 MiB; Chips/Keywords: -
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/broadcom/brcm80211` — 117 Dateien, 2.97 MiB; Chips/Keywords: CFG80211, MAC80211
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/mediatek/mt76` — 113 Dateien, 0.8 MiB; Chips/Keywords: MAC80211
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/ath/ath9k` — 111 Dateien, 2.6 MiB; Chips/Keywords: -
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/backports-5.2.8-1/net/mac80211` — 80 Dateien, 1.81 MiB; Chips/Keywords: MAC80211
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/Iplatform/openwrt/package/mac80211` — 77 Dateien, 0.31 MiB; Chips/Keywords: MAC80211
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/linux-4.4/net/mac80211` — 77 Dateien, 1.62 MiB; Chips/Keywords: MAC80211
- **WLAN** – Linux staging Realtek WLAN driver: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/staging/rtl8723bs/include` — 71 Dateien, 0.55 MiB; Chips/Keywords: RTL8723B, RTL8192C, CFG80211
- **WLAN** – Linux staging Realtek WLAN driver: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/staging/rtl8723bs/hal` — 64 Dateien, 1.13 MiB; Chips/Keywords: RTL8723B
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/ath/ath10k` — 62 Dateien, 2.1 MiB; Chips/Keywords: -
- **WLAN** – WLAN subsystem/build support: `GPL_AX12v1/rtl8197/backports-5.2.8-1/drivers/net/wireless/broadcom/b43` — 62 Dateien, 2.02 MiB; Chips/Keywords: -
### sdk_GPL_MR62X
- **WLAN** – Realtek G6 WiFi 6 driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/g6_wifi_driver/phl/hal_g6` — 1536 Dateien, 138.77 MiB; Chips/Keywords: RTL8852B, RTL8852A, RTL8852C, RTL8192XB, RTL8832BR
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/realtek/rtl8192fe/WlanHAL/Data` — 723 Dateien, 5.81 MiB; Chips/Keywords: RTL8192FE, RTL8881A, RTL8812, RTL8822C, RTL8192F, RTL8814B, RTL8814A, RTL8192E, RTL8197F, RTL8198F
- **WLAN** – Realtek rtl8192cd WLAN driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/rtl8192cd/WlanHAL/Data` — 723 Dateien, 5.81 MiB; Chips/Keywords: RTL8192CD, RTL8881A, RTL8812, RTL8822C, RTL8192F, RTL8814B, RTL8814A, RTL8192E, RTL8198F, RTL8197F
- **WLAN** – Realtek G6 WiFi 6 driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/g6_wifi_driver/platform/mips_98d` — 413 Dateien, 3.23 MiB; Chips/Keywords: RTL8852A, RTL8852C, RTL8192XB, RTL8832BR
- **WLAN** – Realtek G6 WiFi 6 driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/g6_wifi_driver/platform/mips_97f` — 258 Dateien, 1.66 MiB; Chips/Keywords: RTL8852A, RTL8192XB, RTL8832BR
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/realtek/rtl8192fe/wpa3/src_mbedtls` — 222 Dateien, 3.41 MiB; Chips/Keywords: RTL8192FE, WPA3
- **WLAN** – Realtek rtl8192cd WLAN driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/rtl8192cd/wpa3/src_mbedtls` — 222 Dateien, 3.41 MiB; Chips/Keywords: RTL8192CD, WPA3
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/realtek/rtl8192fe/phydm/halrf` — 217 Dateien, 8.92 MiB; Chips/Keywords: RTL8192FE, RTL8812, RTL8822C, RTL8814B, RTL8197G, RTL8198F, RTL8814A, RTL8197F, RTL8821C, RTL8192F
- **WLAN** – Realtek rtl8192cd WLAN driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/rtl8192cd/phydm/halrf` — 217 Dateien, 8.98 MiB; Chips/Keywords: RTL8192CD, RTL8812, RTL8822C, RTL8197G, RTL8814B, RTL8814A, RTL8198F, RTL8197F, RTL8821C, RTL8192F
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/realtek/rtl8192fe/WlanHAL/RTL88XX` — 206 Dateien, 2.69 MiB; Chips/Keywords: RTL8192FE, RTL8197F, RTL8198F, RTL8881A, RTL8197G, RTL8814B, RTL8812, RTL8192E, RTL8821C, RTL8822C
- **WLAN** – Realtek rtl8192cd WLAN driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/rtl8192cd/WlanHAL/RTL88XX` — 204 Dateien, 2.66 MiB; Chips/Keywords: RTL8192CD, RTL8197F, RTL8198F, RTL8197G, RTL8881A, RTL8814B, RTL8812, RTL8192E, RTL8821C, RTL8822C
- **WLAN** – Realtek rtl8192fe/rtl8192cd backports driver: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/realtek/rtl8192fe/WlanHAL/HalMac88XX` — 201 Dateien, 22.86 MiB; Chips/Keywords: RTL8192FE, RTL8821C, RTL8812, RTL8822C, RTL8814B, RTL8197F
- **WLAN** – Realtek rtl8192cd WLAN driver: `sdk/openwrt-21.02/target/linux/linux-4.4/drivers/net/wireless/realtek/rtl8192cd/WlanHAL/HalMac88XX` — 201 Dateien, 22.86 MiB; Chips/Keywords: RTL8192CD, RTL8821C, RTL8822C, RTL8812, RTL8814B, RTL8197F
- **Ethernet/Switch** – Realtek RTL83xx/RTL8367/RTL8325 switch driver data: `sdk/openwrt-21.02/target/linux/rtknet/drivers/net/rtl819x/rtl83xx_v1dot4/dal` — 170 Dateien, 4.62 MiB; Chips/Keywords: RTL819X, RTL83XX, RTL8367C, RTL8367D
- **WLAN** – WLAN subsystem/build support: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/intel/iwlwifi` — 162 Dateien, 3.49 MiB; Chips/Keywords: MAC80211
- **WLAN** – WLAN subsystem/build support: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/broadcom/brcm80211` — 117 Dateien, 2.97 MiB; Chips/Keywords: CFG80211, MAC80211
- **WLAN** – WLAN subsystem/build support: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/mediatek/mt76` — 113 Dateien, 0.8 MiB; Chips/Keywords: MAC80211
- **WLAN** – WLAN subsystem/build support: `sdk/openwrt-21.02/package/backports/src/drivers/net/wireless/ath/ath9k` — 111 Dateien, 2.6 MiB; Chips/Keywords: -

## Hinweise zur Nutzung
- Die Datei `wlan_ethernet_switch_manifest.csv` enthält eine zeilenweise Pfadliste aller erkannten Dateien.
- Die `*_paths.txt`-Dateien enthalten exakte Tar-Pfade für eine spätere Extraktion.
- `extract_wlan_ethernet_switch_sources.sh` ist ein Hilfsskript, um die gelisteten Dateien aus den Originalarchiven zu extrahieren.
