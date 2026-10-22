# GPL-Treiberportierungen für Linux 6.6.114

## Neu implementiert: RTL8197F CPU-Clock-Provider

`target/linux/realtek/files-6.6/drivers/clk/clk-rtl8197f.c` stellt die beim
Start konfigurierte CPU-Frequenz über das Linux Common Clock Framework bereit.
Er liest ausschließlich das Strap-Register an `0x18000008`. Er programmiert
weder PLL noch Taktteiler und bietet keine Übertaktungsfunktion.

Die vollständige Referenz steht unterhalb der GPL-Quellwurzel
`GPL_for_AX1500_rtl8197/openwrt/21.02/`:

- `target/linux/realtek/files-4.4/arch/mips/bsp/timer.c:27`: 16 Frequenzen
  von 450 bis 1200 MHz in 50-MHz-Schritten; Teiler 1, 2, 4 und 8.
- Dieselbe Datei ab Zeile 145: Bit 14 wählt stattdessen 200 MHz; Bits 18:15
  wählen die Tabellenfrequenz, Bits 20:19 den Teiler. CP0 Count läuft mit CPU/2.
- `target/linux/realtek/files-4.4/arch/mips/bsp/bspchip.h:324`:
  `BSP_HW_STRAP = SYSTEM_REG_BASE + 0x8`.

Der ebenfalls enthaltene `clk.c` verwendet eine verkürzte 600/800/1000-MHz-
Zuordnung und ignoriert Teiler und den 200-MHz-Modus. Diese verkürzte Funktion
wird deshalb nicht als vollständiger CPU-Taktdecoder übernommen.
Dateihashes und Portierungsumfang stehen in [driver-ports.json](driver-ports.json).

## Einbindung und Verhalten

- Kernel-Kconfig/Makefile-Anbindung über Patch 337 und
  `CONFIG_COMMON_CLK_RTL8197F=y` im RTL8197F-Subtarget.
- Early-Clock-Provider für den neuen DT-Compatible
  `realtek,rtl8197f-cpu-clock`; ein `reg`-Eintrag beschreibt genau das vier
  Byte große Strap-Register, `#clock-cells = <0>`.
- Nur die experimentellen Cudy-Profile WR1500, RE1500 und M1500 referenzieren
  den neuen Provider als CPU-Takt.
- Der CP0-Timer dieser Profile übernimmt CPU/2. Ein fehlender CPU-Clock-Provider
  führt dort zu einer expliziten Bootfehlermeldung statt zu einem geratenen Takt.
- MW5, AC23 und RD05 behalten den bisherigen festen 500-MHz-CP0-Takt. Eine
  spätere Umstellung benötigt unabhängige Register-/Hardwaremessungen.

Bei einem Cudy-RAM-Boot ist die erwartete neue Diagnose beispielsweise
`RTL8197F CPU strap=0x........ rate=1000000000 Hz (read-only)`. Der tatsächliche
Wert ist von der Startkonfiguration abhängig und noch nicht auf einem Cudy-
Gerät gemessen. Ein Hardwaretest oder vollständiger Firmwarebuild steht aus.

## Weitere GPL-Komponenten

| Komponente | Befund und Behandlung |
| --- | --- |
| RTL8197F Ethernet / RTL865x SwitchCore | Vollständige Quellreferenz vorhanden; Kernel-6.6-Treiber bereits im Zweig. Deskriptor-/FCS-Regeln werden als Referenz genutzt, kein alter Linux-4.4-Treiber darüberkopiert. |
| RTL8367D Port- und CPU-Tag-Treiber | Gegenüber unserem AX12-Auszug bytegleich. Bestätigung vorhandener Referenzen, kein neu gelieferter Fix für den RD05-Datenpfad. |
| Weitere RTL83xx-SDK-Familien | Quellcode ist vorhanden; zusätzliche DSA-Unterstützung erfordert passende Chip-ID, Registermodell und einen überprüften Boardanschluss. Nicht allein durch Kopieren der SDK-API unterstützt. |
| GPIO, PCIe, USB, SPI | BSP-Referenzen vorhanden; bereits moderne Plattformtreiber im Zweig. Einzelne Änderungen müssen pro Hardwarepfad geprüft werden. |
| Hardware-Watchdog | Register-/Timeout-Code im BSP vorhanden; eine zusätzliche Portierung müsste die Linux-Watchdog-API verwenden. Der RTL8197F-Block ist nicht einfach der vorhandene Otto-Watchdog. Nicht neu aktiviert. |
| Neue WLAN-Treiber | Kein vollständiger zusätzlicher RTL8192CD/RTL8852/RTL8832-Quellbaum im Archiv gefunden. Die Buildoption `RTL8852AE_BACKPORTS` allein liefert keinen Treiber. |

## Validierung

Native Tests führen den produktiven Decoder für alle 128 Kombinationen aus
Frequenzwahl, Teiler und 200-MHz-Modus aus. Zusätzlich prüfen sie die echte
`plat_time_init()`-Funktion für alle drei Cudy- und bisherigen Routerprofile,
den Fehlerpfad bei fehlendem Takt und den unveränderten generischen Timerpfad.
Der neue Clock-Provider wurde als Built-in-Objekt für Little-Endian-MIPS gegen
Linux 6.6.114 kompiliert. Patch 337 lässt sich auf die Kernelquellen anwenden;
die Cudy-DTS-Dateien werden erneut kompiliert. Das bestätigt keine reale
Taktfrequenz auf einer konkreten Platine und behebt nicht die offenen RD05-
Ethernet-CRC-Fehler.
