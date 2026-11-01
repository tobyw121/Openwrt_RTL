# WR1500/R76: nativer Ethernet-/DSA-Teststand, 01.11.2026

Das R76-Geräteprofil aktiviert jetzt Ethernet, GPIO-SMI und den externen
RTL8367D als Linux-DSA-Switch. Das ist der erste Hardwareteststand dieser
Anbindung, noch kein am Gerät bestätigter LAN-/WAN-Funktionserfolg.
Der separate RTL8832BR-WLAN-Port und das beschreibbare Overlay bleiben offen.

## Umsetzung der OEM-Laufzeitbelege

| Bereich | Konfiguration |
| --- | --- |
| SMI/Reset | MDC GPIO56, MDIO GPIO55, Reset GPIO58 active-low; Pinmux-Maske passend zu OEM 0x33320000 |
| Buchsen | LAN1=P3, LAN2=P0, LAN3=P1, WAN=P2; Port4/5/6 unbenutzt |
| CPU-Verbindung | RTL8197F P0/RGMII zu RTL8367D MAC7/EXT1 |
| SoC-Timing | TX1/RX0, RGTXC0; P0-PCR 0x16972039, MACCR 0x804a1485, Pad 0xdbffff7f |
| Switch-Timing | TX1/RX0, Feintap4, SSC 0x2473/1/0x5fa/0xf; keine RD05-Parameterübernahme |
| DMA | Ring0 RX900/TX768, 8 DWORDs / 32 Byte; weitere OEM-Ringe nicht aktiviert |
| Diagnose | Treiber 1.8.48, ausschließlich lesendes /proc/cudy-r76-rtknet |

Die 128-MiB-RAM-Freigabe bleibt erhalten. RE1500/M1500 und die historischen
WR1500-RAM-Profile erhalten keine neue Switch-Anbindung.

## Bewusste Anpassung an Linux DSA

Der OEM hat P0-Routermodus und seinen 4-Byte-CPU-Tag-Parser aktiviert.
Diese beiden Komponenten werden nicht mit dem Linux-DSA-Tagger vermischt:
R76 verwendet den bestehenden `rtl8_4`-Tagger mit acht Bytes nach der
Quell-MAC. Der SoC transportiert diese Bytes unverändert; CPU-Tag-RX/TX
und P0-Routermodus sind dafür deaktiviert. Erwarteter SoC-Wert ist dadurch
`P0GMII=0x00037d50`, `MACCR1` Router-/Tagbits=0, statt OEM `0x06037d50/1`.
Das ist eine zu prüfende Linux-Anpassung, kein behaupteter OEM-Binärklon.

Der SoC-interne P0-Hostpfad erhält VID1/2 statt der RD05-VID9/8. Nur P0
ist dort Mitglied; externe Buchsen bleiben ausschließlich DSA-Ports.
SoC-Netif-Hardware-Routing und alle Realtek-NAT-/Shortcut-/Checksum-/SG-/
TSO-/QoS-Beschleuniger sind im R76-Pfad ausgeschaltet.

Der externe Switch startet mit Isolation jedes Nutzports ausschließlich
zum CPU-Port. CPU7 darf die vier Nutzports erreichen. Unbenutzte Ports
haben Isolationsmaske null. Die Matrix und CPU-Portmaske werden ausgelesen;
abweichende Werte oder SMI-Fehler brechen den Setup-Pfad ab. Ein gelernter
Zieleintrag darf die Isolation nicht übersteuern. Linux/DSA entscheidet
anschließend über die LAN-Bridge; WAN bleibt ein eigenständiges Interface.
firewall4, Conntrack und Linux-NAT behandeln gerouteten WAN-/LAN-Verkehr.
Der interne gemeinsame Switch-VID1 ist keine Freigabe einer LAN/WAN-Bridge.
Es wird kein paralleler swconfig-Treiber aktiviert.

## Prüfung

- Beide vollständigen C-Treiber gegen MIPS LE / Linux 6.6.114 übersetzt.
- DSA-Patchsatz einschließlich neuem Patch 342 angewandt; neue Patches
  ohne Fuzz. Ältere Patchkontexte behalten ihr bisheriges Verhalten.
- Produktionsfunktionen mit simuliertem MMIO/Regmap geprüft: gemessene
  R76-Werte, keine RD05-Kalibrierung im R76-Pfad, RD05/MW5-Regressionen,
  Isolationsmatrix und Abbruch bei Transport-/Readbackfehlern.
- Kompilierte DTs beider R76-Images: Pins, CPU-Port, physische Labels,
  Ringgeometrie und Flash-Lesegrenzen geprüft.

Kein vollständiger Firmwarebau, kein erfolgreicher Routerboot dieses
Treiberstands und kein Hardware-Isolationstest werden daraus abgeleitet.
Die gerätespezifische Factory-MAC-Anbindung ist noch nicht implementiert;
der native NIC kann deshalb eine temporäre lokale MAC verwenden.

## PC: neues Image bauen

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make download -j8
make -j"$(nproc)" V=s
```

Fullflash: `bin/targets/realtek/rtl8197f/openwrt-realtek-rtl8197f-cudy_ax1500_r76-squashfs-spi-full.bin`.
Die persönliche Vorlage `fullflash-templates/Cudy_AX1500.bin` ist weiterhin
erforderlich. Nicht das OEM-Webinterface oder sysupgrade zum Einspielen
verwenden; der vorhandene Fullflash-/RAM-Testweg bleibt maßgeblich.

## Router: nach dem Flashen über UART, einzeln

```sh
cat /etc/openwrt_release
```

```sh
ip -br link
```

```sh
dmesg | grep -Ei 'R76|8367|8365|rtknet|smi|failed|error'
```

```sh
cat /proc/cudy-r76-rtknet
```

Erwartet sind eth0 als CPU-Conduit sowie lan1/lan2/lan3/wan. Erst wenn sie
vorhanden sind, den Datenpfad prüfen. Bei fehlender Chipdetektion oder Probe-
Fehlern den vollständigen UART-Bootlog senden; keine RD05-Timingknöpfe setzen.

## PC: LAN1 mit unverändertem OpenWrt-Standard-LAN testen

Kabel an LAN1, Router-LAN 192.168.1.1, temporäre PC-Adresse:

```sh
sudo ip link set enx14ebb6300bb2 up
sudo ip addr replace 192.168.1.3/24 dev enx14ebb6300bb2
sudo ip neigh flush to 192.168.1.1 dev enx14ebb6300bb2
ping -I enx14ebb6300bb2 -c 10 192.168.1.1
ip neigh show dev enx14ebb6300bb2
```

Optional in einem zweiten PC-Terminal:

```sh
sudo tcpdump -ni enx14ebb6300bb2 -e 'arp or icmp or (udp port 67 or 68)'
```

Router nach dem Versuch, einzeln:

```sh
ethtool -S lan1
```

```sh
cat /proc/cudy-r76-rtknet
```

WAN erst nach erfolgreichem LAN-Test an die Fritzbox anschließen:

```sh
ifup wan
```

```sh
ifstatus wan
```

```sh
ip route
```

```sh
ping -c 3 1.1.1.1
```

Danach DHCP, Routing beider Richtungen, LAN/WAN-Isolation und Last gesondert
testen. Aus einem Link-Up oder einem bestandenen lokalen Ping folgt noch
kein erfolgreicher Firewall-/Internetfunktionstest.
