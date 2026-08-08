# Tenda Nova MW5: vollständige WAN/LAN-Portierung aus den RTL8197F/RTL8367-SDKs

Stand: v43.2, OpenWrt 24.10 / Linux 6.6.114

## Ziel

Diese Änderung korrigiert den gesamten kabelgebundenen Datenpfad des Tenda Nova MW5:

- RTL8367R/C-Familie: physischer Port 1 = LAN, Port 3 = WAN, Port 6 = CPU/RGMII
- RTL8197FS: physischer P0/RGMII als Host-Link zum externen Switch
- 4-Byte-Realtek-CPU-Tag mit EtherType `0x8899` und Protokoll `0x9`
- native RTL8197FS-New-Descriptor-Ringe mit 6 DWORD / 24 Byte
- korrekte DMA-Ringgeometrie und Non-VG-TXRINGCR-Konfiguration
- normale OpenWrt-DSA-Topologie: `br-lan` mit `lan`, direkt geroutetes `wan`
- OEM-konforme WAN-MAC: Basis-/LAN-MAC + 7

Die Änderungen wurden statisch gegen den gelieferten OpenWrt-Tree, die MW5-RTK-Collects und `all_most_SDKS` abgeglichen. Ein realer Pakettest auf dem MW5 ist weiterhin zwingend erforderlich.

## v43.2 Build-Korrektur

Linux 6.6.114 stellt in diesem Kontext keinen Helper `netdev_err_ratelimited()` bereit.
Der v43.1-Tagger brach deshalb bei `-Werror=implicit-function-declaration` ab.
v43.2 verwendet den bereits in derselben Datei vorhandenen und kompatiblen
`netdev_err()`-Helper. Es werden keine zusätzlichen Header oder API-Abhängigkeiten
eingeführt.

## Fehlerursachen im bisherigen v42.27-Tree

### 1. Falsches CPU-Tag zwischen RTL8367 und Linux DSA

Der MW5-SDK-Pfad aktiviert auf dem externen RTL8367:

```c
rtk_cpu_tagPort_set(r8367_cpu_port, CPU_INSERT_TO_ALL);
rtk_cpu_tagLength_set(CPU_LEN_4BYTES);
rtk_cpu_acceptLength_set(CPU_RX_64BYTES);
rtk_cpu_enable_set(enable);
```

Die SDK-Paketbeschreibung lautet:

```text
0x8899 + protocol 0x9 + priority + reserved + portmask/source-port
```

Der bisherige MW5-Tree verwendete dagegen den Linux-Tagger `rtl8_4` mit acht Byte. Damit stimmten Switch-Ausgabe und DSA-Demultiplexierung nicht überein. ARP, DHCP, DNS und normaler IP-Verkehr erreichten die DSA-Slaves `lan`/`wan` nicht.

### 2. RTL8197FS-New-Descriptor-Modus war nicht vollständig aktiviert

Der MW5 verwendet den SDK-Pfad `CONFIG_RTL_SWITCH_NEW_DESCRIPTOR`. Ohne Cacheline-Erweiterung bestehen RX- und TX-Deskriptoren aus jeweils sechs DWORD beziehungsweise 24 Byte. Der bisherige MW5-Zweig setzte die Ringgröße zwar teilweise auf 24 Byte, kennzeichnete den RX-Pfad intern aber nicht eindeutig als New-Descriptor-Pfad. Dadurch konnten Initialisierung, Ringzugriff und Re-Arm-Pfad auseinanderlaufen.

### 3. DMA-Ringgeometrie blieb teilweise vom Bootloader übrig

Der SDK-Code programmiert bei einem einzelnen TX-Ring:

```c
DMA_CR1 = (tx_ring_count - 1) * sizeof(struct tx_desc);
DMA_CR4 = TX_RING0_TAIL_AWARE;
```

Für 64 Deskriptoren mit 24 Byte ergibt das:

```text
(64 - 1) * 24 = 1512 = 0x000005e8
```

Im bisherigen Bootlog blieb dagegen `DMA_CR1=0x02ff02ff` erhalten. Der neue Treiber schreibt und prüft CPUTPDCR0, DMA_CR1 und DMA_CR4 sowohl vor als auch nach dem Aktivieren von CPUIF/TRXRDY.

Für RTL8197FS Non-VG werden TX-Ring 2 und 3 deaktiviert; Ring 0 und der für die Hardware-Zustandsmaschine benötigte Ring-1-Zustand bleiben aktiv. Der separate RD05-/RTL8197F-VG-Pfad bleibt unverändert erhalten.

### 4. Falsche OpenWrt-Netzwerktopologie

`02_network` erzeugte bisher für den gesamten Realtek-Target ein globales Bridge-Gerät `switch`. Daraus entstanden beim MW5:

```text
switch
switch.1
switch.2
```

Das ist für den zweipörtigen MW5-Router falsch. Der MW5 benötigt:

```text
br-lan
  `- lan

wan  (direkt gerouteter DSA-Port)
```

Auch `ucidef_set_bridge_mac` musste für den MW5 übersprungen werden, da dieser Aufruf allein bereits wieder das globale `bridge`-Objekt in `board.json` erzeugt.

### 5. WAN-MAC-Abstand

Die MW5-Collects zeigen den OEM-Adressplan:

- LAN/Basis: +0
- WLAN1: +1
- WLAN0: +4
- WAN: +7

Der bisherige Tree verwendete für WAN +2. Die Portierung stellt auf +7 um.

## Verwendete SDK-Referenzen

Die relevanten Quellen liegen in `all_most_SDKS` insbesondere unter:

```text
8197/RT8197F_RTL8367RB_Gesamtpaket/
  MR62X_SDK_Paket/selected_sources/sdk/openwrt-21.02/
    target/linux/rtknet/drivers/net/rtl819x/
```

Entscheidende Stellen:

- `rtl83xx_v1dot4/rtk_api.c::RTL83XX_cpu_tag()`
  - CPU-Port, `CPU_INSERT_TO_ALL`, `CPU_LEN_4BYTES`, 64-Byte-Accept-Length
- `AsicDriver/rtl865x_asicCom.h`
  - erzwingt `CONFIG_RTL_CPU_TAG` für `CONFIG_RTL_8367R_SUPPORT`
- `igmpsnooping/rtl865x_igmpsnooping.c::rtl_parseMacFrame()`
  - dokumentiert EtherType `0x8899`, Protokoll `0x9` und das 6-Bit-Portfeld
- `rtl819x_swNic.h`
  - native RX-/TX-New-Descriptor-Struktur mit sechs DWORD; DWORD 6/7 nur bei Cache-Align-Konfiguration
- `rtl819x_swNic.c::New_swNic_init()`
  - TX-CDP, `DMA_CR1`, `DMA_CR4`, RX-CDP und OWN/WRAP-Initialisierung
- `AsicDriver/rtl865x_asicL2.c::init_8197f_p0()`
  - P0/RGMII, TX-Delay 0, RX-Delay 5, CPU-Tag-Erkennung und Router-Mode
- `AsicDriver/rtl865x_asicL2.c`
  - Non-VG: TX-Ring 2/3 aus; VG: separate Multi-Ring-Konfiguration

Weitere gleichartige Kopien in AX12v1, SDK3, SDK7 und den allgemeinen RTL8197F-Quellen wurden zum Gegenvergleich herangezogen.

## Änderungen im Tree

### Kernel/DSA

- neuer Tagger: `target/linux/realtek/files-6.6/net/dsa/tag_rtl4_9.c`
- neues Protokoll: `DSA_TAG_PROTO_RTL4_9`
- RTL8365MB/RTL8367-Treiber wählt das Protokoll nur bei DTS-Eigenschaft:

```dts
realtek,cpu-tag-4bytes-proto9;
```

- TX: untere sechs Bits als Ziel-Portmaske
- RX: akzeptiert sowohl direkten Source-Port als auch One-Hot-Portmaske, da die SDK-Zweige das Feld unterschiedlich benennen
- Switch-CPU-Tagformat: 4 Byte, Position direkt nach Source-MAC

### RTL8197F-RTKNET

- MW5 explizit im New-Descriptor-RX-Pfad
- `desc_dwords=6`, `desc_stride=24`
- vollständige TX-Ringprogrammierung:
  - `CPUTPDCR0`
  - `DMA_CR1`
  - `DMA_CR4=TX_RING0_TAIL_AWARE`
  - MW5 Non-VG-TXRINGCR
- erneute Programmierung nach CPUIF/TRXRDY, da RTL8197F Schreibzugriffe im gestoppten Zustand teilweise verwirft
- erweiterte Readback-/Fehlerlogs
- MW5 und RD05 bleiben getrennte Pfade:
  - MW5: `rtl4_9`, 4 Byte, 6 DWORD
  - RD05: `rtl8_4`, 8 Byte, bisheriger VG-/RD05-Pfad

### DTS und OpenWrt-Userspace

- MW5-Switch erhält `realtek,cpu-tag-4bytes-proto9`
- Portmap bleibt:
  - Switch-Port 1: `lan`
  - Switch-Port 3: `wan`
  - Switch-Port 6: `cpu`, RGMII zu `ethernet0`
- kein globales `switch`-Bridgeobjekt für MW5
- kein globales `ucidef_set_bridge_mac` für MW5
- LAN wird automatisch als `br-lan` mit Port `lan` generiert
- WAN verwendet direkt das Gerät `wan` mit DHCP
- WAN-MAC = Factory-`HW_NIC0_ADDR` + 7

## Bauen

Aus dem Root des geänderten Trees:

```sh
./scripts/feeds update -a
./scripts/feeds install -a
make menuconfig
make target/linux/clean
make -j"$(nproc)" V=s
```

Bei bereits vollständig vorbereiteten Feeds und vorhandener `.config` genügen normalerweise:

```sh
make target/linux/clean
make -j"$(nproc)" V=s
```

## Flashen und Netzwerkkonfiguration neu erzeugen

Beim Testen keine alte v42.27-Netzwerkkonfiguration übernehmen. Für einen Sysupgrade ist deshalb ein Upgrade **ohne Beibehalten der Einstellungen** zu bevorzugen.

Bei einem bereits laufenden Testsystem kann ausschließlich die Netzwerkkonfiguration gesichert und neu erzeugt werden:

```sh
cp /etc/config/network /root/network.pre-v43 2>/dev/null || true
rm -f /etc/config/network
reboot
```

Beim nächsten Boot erzeugt `config_generate` die neue Topologie. Erwartet wird sinngemäß:

```text
config device
        option name 'br-lan'
        option type 'bridge'
        list ports 'lan'

config interface 'lan'
        option device 'br-lan'
        option proto 'static'

config interface 'wan'
        option device 'wan'
        option proto 'dhcp'
```

`switch`, `switch.1` und `switch.2` dürfen auf dem MW5 nicht mehr vorhanden sein.

## Erwartete Bootmeldungen

```sh
dmesg | grep -E 'rtl4_9|4-byte protocol|txring sdk v43|poststart v43|realtek-smi'
```

Erwartete Kernaussagen:

```text
MW5 SDK CPU tag: 4-byte protocol 0x9 after source MAC
MW5 DSA master: ... tag=rtl4_9/4byte
mw5 txring sdk v43.2: ... stride=24 dma_cr1=0x000005e8 ...
rtl8197f poststart v43.2: ... desc=6/24 ... dma_cr1=0x000005e8 ...
```

Es darf keine Meldung mit `rejected` für TX-Basis, DMA_CR1 oder Descriptorformat erscheinen.

## Hardwaretest

### Topologie

```sh
uci show network
ip -d link show br-lan
bridge link
bridge vlan show
```

Erwartet:

- `br-lan` enthält `lan`
- `wan` ist kein Mitglied von `br-lan`
- keine Geräte `switch.1` oder `switch.2`

### Zähler

```sh
ip -s link show eth0
ip -s link show lan
ip -s link show wan
cat /proc/net/dev
ethtool -S eth0 2>/dev/null
cat /proc/interrupts
```

Bei eingestecktem LAN-Kabel und ARP-/DHCP-Verkehr müssen RX-Zähler auf `eth0` und dem zugehörigen DSA-Slave steigen.

### ARP und DHCP

```sh
tcpdump -eni eth0 'ether proto 0x8899 or arp or (udp port 67 or 68)'
tcpdump -eni lan 'arp or (udp port 67 or 68)'
tcpdump -eni wan 'arp or (udp port 67 or 68)'
ubus call network.interface.lan status
ubus call network.interface.wan status
```

LAN-Test:

1. Client an LAN anschließen.
2. Client muss DHCP-Lease von OpenWrt erhalten.
3. ARP für die Routeradresse muss beantwortet werden.
4. Ping zur LAN-Adresse des MW5 testen.

WAN-Test:

1. DHCP-fähigen Upstream an WAN anschließen.
2. `network.interface.wan` muss eine Lease erhalten.
3. Upstream-Gateway muss per ARP aufgelöst werden.
4. Danach Routing, DNS-Auflösung und Internetzugriff testen.

## Diagnose, falls es weiterhin nicht funktioniert

### `eth0` RX bleibt 0

Dann liegt der Fehler weiterhin vor der DSA-Demultiplexierung. Sammeln:

```sh
dmesg
cat /proc/net/dev
cat /proc/interrupts
ethtool -S eth0 2>/dev/null
ip -s link show eth0
```

Zusätzlich die geloggten Werte für `CPURPDCR0`, `CPUICR1`, `DMA_CR1`, `DMA_CR4`, `TXRINGCR`, `CPUQDM0/2/4` und RX-Deskriptor 0 prüfen.

### `eth0` RX steigt, `lan`/`wan` bleiben 0

Dann kommen Frames im Master an, aber das CPU-Tag wird noch nicht korrekt auf einen Slave abgebildet. Rohdump sichern:

```sh
tcpdump -eni eth0 -XX -s 256 ether proto 0x8899
```

Benötigt werden die vier Bytes direkt nach der Source-MAC. Das zweite 16-Bit-Wort muss oben das Protokoll `0x9` tragen; das untere Feld muss Port 1 beziehungsweise Port 3 identifizieren.

### LAN funktioniert, WAN erhält keine Lease

Dann ist der gemeinsame CPU-/DSA-Pfad grundsätzlich funktionsfähig. Prüfen:

```sh
ip link show wan
ubus call network.interface.wan status
tcpdump -eni wan 'arp or (udp port 67 or 68)'
logread -e udhcpc
```

Zusätzlich Upstream-DHCP, Kabel, Linkstatus und MAC-Filterung des vorgeschalteten Geräts prüfen.

## Fullflash-Hinweis

Das zuvor verwendete private Fullflash-Template weicht in `cfm` und `cfm_backup` vom neuesten MW5-Dump ab. Ein Fullflash kann daher Gerätekonfiguration überschreiben. Für Tests bevorzugt das Sysupgrade-Image verwenden oder ausschließlich das neueste geräteeigene `mtd0-ALL.bin` als Template einsetzen.

## Build-Korrektur v43.2

Der erste v43.0-Patch 329 enthielt für `drivers/net/dsa/realtek/Kconfig`
einen Kontext aus einem neueren Kernelstand (`switch driver` und `depends on`).
OpenWrt 24.10 mit Linux 6.6.114 besitzt dort noch den ursprünglichen
`switch subdriver`-Block mit `imply NET_DSA_REALTEK_SMI` und
`imply NET_DSA_REALTEK_MDIO`. v43.2 fügt `select NET_DSA_TAG_RTL4_9`
in genau diesen Linux-6.6-Block ein. Dadurch wird der zuvor erzeugte
`drivers/net/dsa/realtek/Kconfig.rej` vermieden.

Nach einem fehlgeschlagenen Versuch muss der teilweise vorbereitete Kernelbaum
vor dem erneuten Build entfernt werden:

```sh
make target/linux/clean
make target/linux/compile V=s
```

## Validierungsstatus

Durchgeführt:

- SDK-Quellvergleich für CPU-Tag, P0/RGMII, Descriptoren und DMA
- statischer Vergleich zum v42.27-Tree
- Patch-329-Dry-Run mit `--fuzz=0` gegen den Linux-6.6-Realtek-Kconfig-Kontext
- Shell-Syntaxprüfung von `02_network`
- statische Konsistenzprüfung per `CHECK_MW5_WANLAN_V43.sh`: 43 PASS, 0 FAIL

Nicht durchgeführt:

- vollständiger OpenWrt-Cross-Build in dieser Umgebung
- Flash- und Pakettest auf realer MW5-Hardware

Die Portierung ist deshalb ein SDK-abgeleiteter, statisch validierter Hardware-Teststand und keine Garantie ohne Rückmeldung der Bootlogs und Paketmitschnitte des realen Geräts.
