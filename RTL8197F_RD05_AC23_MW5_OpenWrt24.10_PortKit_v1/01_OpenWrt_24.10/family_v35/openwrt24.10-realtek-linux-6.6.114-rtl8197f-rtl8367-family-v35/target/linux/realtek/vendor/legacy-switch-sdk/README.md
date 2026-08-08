# Realtek-Switch-SDK: Portierungs- und Build-Regeln

Dieses Verzeichnis enthält das **vollständige, unveränderte, vom Benutzer bereitgestellte Realtek-Switch-SDK** als Referenz für die Linux-6.6-Portierung. Die Originaldateien liegen unter `source/`; Prüfsummen und eine Datei-für-Datei-Zuordnung liegen daneben.

## Warum die Originalmodule nicht direkt gebaut werden

Der SDK-Stand stammt aus der Linux-2.6.32-Ära und enthält proprietäre RTK/DAL/HAL-Schnittstellen, direkte Registerzugriffe sowie alte Kernelmodule (`rtcore`, `rtdrv`, `rtk`, `rtnic`). Diese Module verwenden unter anderem entfernte `.ioctl`-Callbacks, alte Task-/Scheduler- und IRQ-APIs und einen parallelen Switch-Netzwerkstack. Sie werden deshalb **nicht** als zweite, konkurrierende Treiberfamilie in Linux 6.6 geladen.

Die Hardwarefunktionen werden stattdessen in den zugehörigen Linux-Subsystemen bereitgestellt:

| SDK-Bereich | Linux-6.6/OpenWrt-Ersatz |
|---|---|
| BSP, IRQ, Timer, Clock | MIPS-Realtek-Plattform, `irqchip`, `REALTEK_OTTO_TIMER`, RTL83xx-Clock-Treiber |
| NIC/DMA | `NET_RTL838X`, NAPI und DSA-Master |
| Switch-Core/DAL/RTK | `NET_DSA_RTL83XX`, bridge/switchdev, tc, ethtool, devlink und Netlink |
| SMI/MDIO | `MDIO_REALTEK_OTTO_AUX` und Linux-MDIO |
| PHY/SerDes | `REALTEK_SOC_PHY` (`rtl83xx-phy.c`) |
| RTL8231 | `MFD_RTL8231`, `PINCTRL_RTL8231`, GPIO und `LEDS_RTL8231` |
| GPIO | `GPIO_REALTEK_OTTO` beziehungsweise `GPIO_RTL8197F` |
| Watchdog | `REALTEK_OTTO_WDT` |
| UART | 8250/OF-Serielltreiber |
| Flash | SPI-NOR/MTD und RTL8197F-SPI-ROM-Automap |
| RTL9300-I²C | `I2C_RTL9300` und `I2C_MUX_RTL9300` |

## Vollständige Inventarisierung

`PORT_MANIFEST.tsv` enthält für **jede reguläre Datei und jeden symbolischen Link** unter `source/`:

- Pfad, Typ, Größe und SHA-256,
- logische Komponente,
- Portierungsstatus,
- nativen Linux-6.6-Ersatz,
- Build-Richtlinie und Begründung.

`PORT_SUMMARY.tsv` fasst die Statusklassen zusammen. `SOURCE_SHA256SUMS` ermöglicht die Integritätsprüfung der regulären Quelldateien.

## Bewusst offene Hardwarelücken

Die SDK-spezifischen Implementierungen für **RTL8328**, **RTL8389** sowie die speziellen Initialisierungsfolgen für **RTL8208/RTL8212** besitzen in diesem Tree keinen nachgewiesenen, vollständigen Linux-6.6-DSA/Ethernet-/PHY-Ersatz. Die Quellen bleiben daher als `legacy-*-gap-retained` erhalten, werden aber nicht irreführend als funktionsfähig gebaut.

Das ist eine Sicherheits- und Korrektheitsentscheidung: bloßes Kompilieren mittels Stubs würde weder Paketverkehr noch Registerbelegung oder Interruptverhalten beweisen und könnte Hardware beschädigen oder den Bootvorgang blockieren.

## Lizenz

Die SDK-Dateien enthalten eigene Realtek-Hinweise, die die Nutzung, Änderung und Weitergabe auf autorisierte Lizenzen beschränken. Dieses Verzeichnis ändert oder ersetzt diese Lizenztexte nicht. Vor Weitergabe oder Veröffentlichung muss der Nutzer seine Berechtigung prüfen.
