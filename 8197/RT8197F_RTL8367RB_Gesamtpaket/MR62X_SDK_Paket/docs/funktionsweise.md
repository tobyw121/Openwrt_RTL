# Funktionsweise der RTL8197F + RTL8367RB Plattform

## Architektur

Die Plattform kombiniert einen Realtek RTL8197F/RTL8197F_VG MIPS-SoC mit einem externen Realtek RTL8367R/RB/RB-VC Ethernet-Switch. Der SoC stellt CPU, Speichercontroller, Flash-Zugriff, Netzwerk-MAC/NIC und WLAN-Integration bereit. Der externe Switch übernimmt PHY-/Switching-Funktionen, Port-Isolation, VLANs, QoS, ACL/MIB/LED- und Link-Management.

## Boot- und Initialisierungspfad

1. U-Boot wird mit einer boardabhängigen `def-*8197f*8367*config` gebaut.
2. U-Boot initialisiert Flash, DRAM, Realtek-SoC und optional den externen 8367-Switch.
3. Kernel 4.4 startet als Realtek-MIPS-Kernel mit `SOC_RTL8197F`.
4. `rtknet` baut den SoC-NIC und die Realtek-Switch-API ein.
5. Der Switch wird über `smi_init()`, `smi_read()` und `smi_write()` per GPIO-MDC/MDIO/SMI konfiguriert.
6. `rtk_api.c` setzt Default-Portrollen, WAN/LAN-VLANs, RGMII-Delay und weitere Switch-Parameter.
7. Userland/RootFS startet über `etc.* / init.d / rcS` bzw. OpenWrt-Base-Files.

## Switch-Anbindung

- Kernel-Target: `CONFIG_RTL_MDC_H0_MDIO_G7=y`.
- U-Boot hat mehrere Varianten: `CONFIG_RTL_MDC_H0_MDIO_G7=y` oder `CONFIG_RTL_MDC_D1_MDIO_D7=y`, abhängig von Board/Flash/VC-Variante.
- `common/smi.c` implementiert GPIO-basierte SMI-/MDC-MDIO-Registerzugriffe.
- `rtl83xx_v1dot4/rtk_api.c` ist der zentrale High-Level-API-Code für den Switch und enthält RTL8367RB-spezifische Fallbacks.

## Relevante Treiberfamilien

- `rtl819x_swNic.o`, `rtl_nic.o`, `rtl865xc_swNic.o`: SoC Ethernet/NIC.
- `rtl83xx_v1dot4`: neue Realtek unmanaged switch API v1.4, für `RTL_8367RB_VC` relevant.
- `rtl83xx`: ältere 83xx API/Implementierung.
- `rtl8367r`: ältere RTL8367B/R API-Implementierung.
- `common/smi.c`: gemeinsamer Zugriff auf externe Switch-Register.
- `l2Driver`, `l3Driver`, `l4Driver`, `fastpath`: Realtek Bridge/NAT/Fastpath-Offload-Komponenten.

## WLAN

Das MR62X-SDK enthält mehrere Realtek-WLAN-Bäume. Für das gefundene Target sind besonders relevant:

- Backports/WiFi6: `CPTCFG_RTL8832BR=y`, `CPTCFG_RTL8852AE_BACKPORTS=y`.
- Plattformdaten: `g6_wifi_driver/platform/mips_97f`.
- 8197F-HAL/PHY/RF-Daten: `WlanHAL/RTL88XX/RTL8197F` und `WlanHAL/Data/8197F`.

## Abweichungen / Stolperstellen

- In Kernel-Configs ist der 8367RB-VC-Pfad klar: `RTL_8367RB_VC=y` + `RTL_83XX_API_V1_4=y`.
- In manchen Backports-Configs steht `RTL_8367R_NEW_SUPPORT=y`; EasyMesh-Backports-Configs wechseln auf `RTL_8367RB_VC=y` + API v1.4.
- Bootloader-Pinning und Kernel-Pinning sind nicht in allen U-Boot-Defconfigs identisch. Beim Portieren müssen GPIO-MDIO-Pins zwischen U-Boot, Kernel und Boardlayout abgeglichen werden.
- `RTL8367RB` und `RTL8367R/RB-VC` werden im Baum teils über ältere `rtl8367r`-Ordner und teils über `rtl83xx_v1dot4` abgebildet.
