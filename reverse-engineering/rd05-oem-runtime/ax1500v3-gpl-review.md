# TP-Link AX1500v3-GPL: Relevanz für RD05

Geprüft am 2026-10-31, Ausgangsstand MW5_Stable a6e2fade50.
Quelle: https://static.tp-link.com/upload/gpl-code/2026/202610/20261022/GPL_ax1500v3.tar.gz

Archiv: 2.206.787.638 Bytes; SHA256
`6d133598b134b6878ce802a53b3061d9f2efb3636ad0d31ec4cb3bcbdf438f2a`.
Das äußere TAR-Verzeichnis wurde vollständig gelesen: 289.738 Einträge,
darunter 268.247 reguläre Dateien mit zusammen 5.500.302.829 Bytes.
9.137 ausgewählte Dateien wurden zur statischen Prüfung abgelegt.
Enthaltene Buildskripte wurden nicht ausgeführt. Die 359 anhand ihrer
Endung erkannten inneren Archive wurden nicht vollständig rekursiv untersucht.

## Ergebnis

Als zusätzliche Referenz für RTL83xx-Portfunktionen brauchbar; kein
belegter Fix für den RD05-CPU-Link und kein passender Ersatz für seinen
RTL8197F-NIC-Treiber. Keine Treibereinstellungen aus dieser Prüfung geändert.

Die AX1500v3-Produktkonfiguration aktiviert RTL9607C, Big Endian und Linux
4.4.140. Der Pfadname `rtl8198` bezeichnet deshalb nicht hinreichend den
tatsächlich ausgewählten Chip. Insbesondere ist RTL8198F im Produktprofil
deaktiviert, obwohl entsprechende SDK-Quellen im Paket enthalten sind.

Alle folgenden Pfade sind relativ zu `GPL_ax1500v3/`:

| Quelle | Beleg |
| --- | --- |
| `Iplatform/build/Makefile` | `PRODUCT_NAME=ax1500v3` |
| `Iplatform/build/product_configs/ax1500v3/sdk_config/linux-4.4.x/.config` | Zeile 3: Linux/mips 4.4.140; 75: RTL9607C; 142: CPU_BIG_ENDIAN; 1354: RTL9607C_SERIES; 1358: RTL8198F_SERIES deaktiviert |
| `rtl8198/linux-4.4.x/drivers/net/ethernet/realtek/rtl83xx/API_Source/rtk_switch.h` | Zeilen 48–55: RTL8367C, RTL8370B, RTL8364B und RTL8363SC_VB; kein RTL8367D-Eintrag |
| `rtl8198/linux-4.4.x/drivers/net/ethernet/realtek/rtl83xx/API_Source/rtk_switch.c` | Zeilen 727–763: Dispatch der genannten Chipvarianten, kein RTL8367D-Initialisierungspfad |
| `rtl8198/linux-4.4.x/drivers/net/ethernet/realtek/rtl83xx/API_Source/rtl83xx_main.c` | Zeilen 161–192: Delay vor Force-Link zur Vermeidung eines RXC-Glitches; Beispielwerte TX=1/RX=3 für diesen Boardpfad |
| `rtl8198/linux-4.4.x/drivers/net/ethernet/realtek/rtl83xx/API_Source/port.c` | Zeilen 1385–1414: TX-Bit 3/RX-Bits 2:0, Auswahl des Registers abhängig vom logischen EXT-Port |
| `rtl8198/linux-4.4.x/drivers/net/ethernet/realtek/rtl86900/sdk/src/dal/rtl8198f/dal_rtl8198f_rtl83xx_api.c` | Weiterer SDK-Adapter für RTL8198F, kein RTL8197F-swNic-Ersatz |

## Was wir verwenden können

* Die ausdrücklich dokumentierte Reihenfolge Delay vor Force-Link bestätigt
  unsere bereits vorhandenen RD05-Patches 338/339. Daraus folgt keine neue
  Timingstufe für den RD05.
* Port-, CPU-Tag-, VLAN-, SMI- und MIB-Funktionen erlauben Vergleiche auf
  Registerebene, wenn vorher die jeweilige Chip-/Portzuordnung geprüft wird.
* Die umfangreiche Registerreferenz ist teilweise schon bekannt:
  `rtl8367c_reg.h` ist bytegleich mit einer Kopie in der zuvor untersuchten
  AX1500-RTL8197-GPL. SHA256:
  `8ce78067a7b57531b40d0c28a847b47590235098e191de7a1162421ae6371c7a`.
  Die verglichenen `port.c`, `rtk_switch.c` und `rtl8367c_asicdrv_port.c`
  sind dagegen nicht bytegleich mit deren vorhandenen Namensvettern;
  daraus allein folgt keine RD05-relevante Verbesserung.

## Was diese Quelle bisher nicht liefert

Im vollständigen äußeren Dateiverzeichnis wurden keine
`dal_rtl8367d_*`-Dateien und keine `*swNic*`-Dateien gefunden. In der
geprüften RTL83xx-API ist kein RTL8367D-/RB-VC-Dispatch nachgewiesen.
Eine vollständige passende RTL8367D-Kaltstart-/Jam-Tabelle wurde hier
nicht identifiziert. Diese Aussagen beziehen sich auf das sichtbare
Dateiverzeichnis und die geprüften Quellen, nicht auf sämtliche Inhalte
der nicht rekursiv untersuchten inneren Archive.

Der RD05 meldet Switch-ID 0x6642/Revision 0x0030 und verwendet unseren
RTL8367D/RB-VC-Pfad. AX1500v3-Timingwerte, Chip-Init-Sequenzen und
RTL9607C-/RTL8198F-DMA-Deskriptoren dürfen deshalb nicht ungeprüft in diesen
Pfad übernommen werden. CRC-Prüfung bleibt aktiv; es gibt aus dieser
Analyse keinen Beleg, dass CRC-Skip das LAN-/WAN-Problem beheben würde.

Die zuvor beobachteten CPU-Port-FCS-Fehler und der fehlende SoC-Empfang
sind durch diesen statischen Vergleich nicht erklärt. Der vorbereitete
[RD05-TX-Puffervergleich](../../target/linux/realtek/rtl8197f/RD05_TX_DMA_AB.md)
bleibt sinnvoll. Kein Hardwaretest oder Firmware-Build aus dieser Prüfung.
