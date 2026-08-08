# Gesamtanalyse von `entpackt(1).7z`

## Prüfumfang

- Archiv: `entpackt(1).7z`
- Größe: 445 MiB
- SHA-256: `87d7410a0349e8c4404f2ab708467415051cc10d5754a2e628f0e190efcc4cad`
- Entpackte reguläre Dateien: 309.890
- Verzeichnisse: ca. 25.377
- Logische Nutzdaten: ca. 4,06 GiB
- Vergleichsbasis: `8197_all_SDKs.tar(3).zst`
- Vergleichsmethode: vollständige Pfad- und SHA-256-Inhaltsprüfung aller regulären Dateien

## Kernergebnis

Das neue Archiv ist **wesentlich größer**, aber nicht einfach eine neuere oder vollständigere Ausgabe von `8197_all_SDKs`. Es besteht aus zwei Teilen:

1. Der Ordner `8197/` ist nur eine kleinere, neu geordnete Auswahl bereits bekannter Dateien. Sein gesamter Inhalt existiert bytegleich in der alten Sammlung.
2. Die wirklich neuen Inhalte sind zusätzliche, überwiegend ältere Realtek-Voll-SDKs, Bootloaderquellen, Legacy-Toolchains und ein Managed-Switch-SDK.

Das Archiv bringt **keinen neuen RTL8367D-Treiber**, keine neue RTL8367D-Initialisierungstabelle und keinen fertigen RD05-Port. Sein größter Nutzen ist die deutlich bessere historische und Low-Level-Dokumentation des RTL8197F/FS mit RTL8367R/RB.

## Mengenvergleich

| Bereich | Neues Archiv | Alte Sammlung |
|---|---:|---:|
| reguläre Dateien | 309.890 | 38.617 |
| identische Inhalts-Hashes | 10.589 | – |
| im neuen Archiv inhaltlich neu | 299.301 | – |
| Dateien unter `8197/` | 8.952 | 38.617 |
| gemeinsame Pfade unter `8197/` | 8.177 | – |
| nur im neuen `8197/` sichtbare Pfade | 775 | – |
| davon wirklich neuer Inhalt | **0** | – |
| RTL8367D-Dateien | 72 | 196 |
| neue RTL8367D-Dateien | **0** | – |

Die 775 nur unter neuen Pfaden sichtbaren Dateien in `8197/` sind verschobene oder umbenannte Duplikate. Die alte Sammlung besitzt zusätzlich 30.440 Pfade, die im neuen `8197/` fehlen.

## Wirklich neue Hauptpakete

| Paket | Dateien | ca. Größe | Bewertung |
|---|---:|---:|---|
| `rtk_openwrtSDK_v2.5_20160905` | 172.355 | 1.706,5 MiB | wichtig für RTL8197F-Boot, OpenWrt 14.07/Linux 3.10.49, RTL8367RB |
| `rtl819x-SDK-v3.4.11C-full-package` | 67.887 | 1.298,5 MiB | vollständiger Router-SDK-Zweig auf Linux 3.10.90 |
| `rtk-ms-uClinux-src-2.6.19-2.6.32.58-svn53590...` | 56.145 | 649,3 MiB | Managed-Switch-SDK, kaum direkt für RD05 |
| Legacy-MSDK/Toolchains | ca. 4.125 | ca. 204 MiB | Reproduktion alter Builds |
| `SDK_V2.1.4.53590_Patch` | 65 | 5,0 MiB | RTL833x/838x/839x-Patches, nicht RTL8367D |
| `8197/` | 8.952 | 279,9 MiB | vollständig bereits vorhanden |

## 1. Neues OpenWrt-SDK v2.5 von 2016

`rtk_openwrtSDK_v2.5_20160905` basiert auf:

- OpenWrt Barrier Breaker 14.07, Revision r42887
- Linux 3.10.49
- MIPS32r2/24Kec
- Realtek-Target `rtkmipsel/rtl8197f`

Relevante Konfigurationen enthalten unter anderem:

- `CONFIG_SOC_RTL8197F=y`
- `CONFIG_RTL_8367R_SUPPORT=y`
- `CONFIG_SPI_SHEIPA=y`
- `CONFIG_RTL8192CD=m`

### Besonders wertvolle neue Bootloaderprofile

Erstmals in dieser Sammlung vollständig vorhanden sind unter anderem:

- `def-rtl8197fb_8367rb_ddr128M-config`
- `def-rtl8197fs_8367rb-config`
- generische RTL8197F- und SPI-NAND-Konfigurationen

Außerdem liegen Referenz-Bootimages vor, beispielsweise:

- `boot_97FS_Giga.bin`
- `boot97DN_8367r_20150724.bin`
- `boot_8881AB+8367R.bin`

Diese Daten sind sehr hilfreich, um den frühen SoC-Start, DDR, Pinmux, SPI, RGMII, SMI und Switchreset zu rekonstruieren.

## 2. Neue und wichtige GPIO-Erkenntnis

Der ältere RTL8197F-Boot- und NIC-Code definiert für den externen RTL8367R/RB:

```c
#define GPIO_RESET 26 /* GPIO_H2 */
```

In der globalen Linux-GPIO-Nummerierung des OEM-Systems entspricht Bank H, Pin 2 sehr wahrscheinlich **GPIO 58**.

Die bereits bekannte neuere RTL8197FH-Konfiguration verwendet:

```text
CONFIG_RTL_MDC_H0_MDIO_G7=y
```

Bei derselben Nummerierung ergibt das:

- H0 = GPIO 56, wahrscheinlich SMI/MDC
- G7 = GPIO 55, wahrscheinlich SMI/MDIO
- H2 = GPIO 58, wahrscheinlich Switchreset

Das passt exakt zu den drei auf dem RD05 beobachteten GPIOs 55, 56 und 58. Damit ist die bislang nur vermutete Zuordnung jetzt stark belegt:

| GPIO | vermutete Funktion |
|---:|---|
| 55 / G7 | RTL8367D SMI-Datenleitung |
| 56 / H0 | RTL8367D SMI-Taktleitung |
| 58 / H2 | RTL8367D Reset |

Die endgültige Bestätigung sollte noch durch Laufzeitmessung oder gezielte GPIO-Tests erfolgen.

## 3. Vollständigeres RTL8197F/RTL8367RB-Bring-up

Die neuen alten Voll-SDKs enthalten deutlich vollständigere Abläufe für:

- RTL8197F/FS-Reset und Clocksetup
- RGMII-MAC-Konfiguration
- erzwungenen 1-Gbit/s-Full-Duplex-Link
- SMI-Initialisierung
- Reset des externen Switches
- Aufruf von `RTL8367R_init()`
- VLAN-Grundkonfiguration
- MIB-Reset
- optionales CPU-Tagging
- RTL865x-DMA/NIC und Portmasken

Für RTL8367R/RB sind große `ChipDataXX`-Registertabellen und eine echte `RTL8367R_init()`-Sequenz vorhanden. Das ist eine sehr gute Referenz für den Tenda AC23 mit RTL8367RB und für gemeinsame RTL8367-Familienfunktionen.

## 4. Was beim RTL8367D **nicht** neu ist

Das entscheidende negative Ergebnis:

- Alle 72 RTL8367D-Dateien im neuen Archiv sind bytegleich in `8197_all_SDKs` vorhanden.
- Die alte Sammlung besitzt weitere 124 RTL8367D-Dateien.
- Es gibt keine neue D-spezifische Jam-/Kaltstarttabelle.
- Es gibt keinen neuen D-spezifischen PHY-Patch.
- Es gibt keine neue, verifizierte CPU-Tag-Spezifikation für RTL8367D.
- Die Funktion bleibt leer:

```c
rtk_api_ret_t dal_rtl8367d_switch_init(void)
{
    return RT_ERR_OK;
}
```

Damit wird die zentrale Lücke für eine vollständig unabhängige RTL8367D-DSA-Initialisierung durch das neue Archiv nicht geschlossen.

## 5. Neues vollständiges RTL819x SDK v3.4.11C

Das Paket `rtl819x-SDK-v3.4.11C-full-package` ist ein vollständiger Realtek-11n-Router-SDK-Zweig auf Linux 3.10.90.

Es bringt zusätzlich:

- vollständigen RTL8197F/FS-Plattformcode
- RTL865x-NIC- und Fastpath-Quellen
- vollständige alte Switchanbindung
- Bootcode und Imagewerkzeuge
- integrierte WLAN-HAL-/PHYDM-Daten
- 8812-/8814A-nahe WLAN-Quellen
- mehr Referenzkonfigurationen für RTL8197F + RTL8367R/RB

Die Dokumentation nennt frühere Referenzimages wie:

- `fw_97F_8812BR_8367.bin`
- `97F_8812BR_8367_nfjrom`
- `boot_8197FS-8367RB.bin`

Die genannten Releasearchive selbst sind jedoch nicht vollständig Bestandteil des Uploads; vorhanden ist hauptsächlich der dazugehörige Quellbaum.

## 6. Imageformat und Bootloader

Neu beziehungsweise deutlich vollständiger vorhanden sind:

- `cvimg.c`
- `mgbin.c`
- Bootloader-`utility.c`
- TFTP-Flashinglogik
- Prüf- und Schreibcode für `cs6c`, `cr6c` und `root`

Damit lässt sich das Realtek/Tenda-Imageformat der beiden 8-MiB-Dumps sehr gut rekonstruieren. Insbesondere `cr6c` wird als Kernel-plus-Rootfs-Image direkt vom Bootloader geprüft und geschrieben.

Das löst jedoch nicht den Xiaomi-spezifischen RD05-Pfad:

- kein `bootmiwifi`
- kein RD05-Profil
- kein Xiaomi-Factory-Headerbuilder
- keine Xiaomi-spezifische Partitions-/Recoverylogik

## 7. WLAN

### Enthalten

- umfangreiche integrierte RTL8197F-WLAN-HAL-/PHYDM-Daten
- RTL8192CD-Vendortreiber
- 8812- und 8814A-nahe ältere Codezweige
- Firmware- und RF-/AGC-/Tx-Power-Tabellen

### Nicht neu oder nicht exakt vorhanden

- kein neuer RTL8812FE-Treiber gegenüber der alten Sammlung
- kein exakter neuer RTL8812BRH-Zweig
- kein vollständiger exakter RTL8814BR-Zweig; überwiegend RTL8814A beziehungsweise generische bedingte Pfade
- keine mac80211-/cfg80211-Portierung für Linux 6.6/6.12

Der WLAN-Teil bleibt damit wertvolle Vendorreferenz, aber kein direkt einsetzbarer OpenWrt-24.10/25.12-Treiber.

## 8. Managed-Switch-SDK

Das große Paket `rtk-ms-uClinux-src-2.6.19-2.6.32.58-svn53590` ist kein RTL8197F-Router-SDK. Es richtet sich hauptsächlich an RTL833x/838x/839x-Managed-Switches und enthält:

- Linux 2.6.19 und 2.6.32.58
- alte uClibc-/BusyBox-Varianten
- RTL8380/8390-nahe Plattform- und PHY-Komponenten
- Managed-Switch-CLI und SDK-Infrastruktur

Die zugehörigen Patches bringen unter anderem RTL8214FC-/RTL8218FB-, Fiber-, EEE-, RTCT-, L2-Notification- und RTL839x-NIC-Änderungen. Für RTL8197F + RTL8367D sind sie höchstens als Architekturvergleich nützlich.

## 9. Legacy-Toolchains

Neu enthalten sind mehrere alte Toolchains, beispielsweise:

- MSDK/GCC 4.3.6
- ältere MIPS-uClibc-Ketten
- GCC-3.4.x-/4.4.x-/4.6.x-nahe Umgebungen

Sie helfen beim reproduzierbaren Bauen historischer Vendorquellen. Für einen modernen OpenWrt-Build sollten sie nicht als Zieltoolchain verwendet werden.

## 10. Was weiterhin vollständig fehlt

Auch nach Einbeziehung des neuen Archivs fehlen:

1. ein RD05-Device-Tree
2. eine vollständige RTL8367D-Kaltstart-/Jam-Tabelle
3. eine auf Hardware bestätigte RTL8367D-CPU-Tag-Bitbelegung
4. ein moderner RTL8197F-Ethernettreiber ohne RTL865x-Vendor-Netif-Kopplung
5. ein Linux-6.6-/6.12-DSA-Treiber für RTL8367D
6. phylink-/DSA-/NVMEM-Integration
7. `bootmiwifi`-Quellcode und Xiaomi-Factory-Imageerzeugung
8. exakte RD05-LED-/Tastenbeschreibung
9. moderne RTL8197F- und RTL8812FE-WLAN-Treiber
10. ein fertiges OpenWrt-24.10- oder 25.12-Boardprofil

## Bedeutung für den RD05-Port

### Neu gewonnene Sicherheit

Das Archiv verbessert die Erfolgsaussicht vor allem bei:

- frühem RTL8197F-Boot
- GPIO-/Pinmux-Zuordnung
- Switchreset
- SMI/MDC/MDIO
- RGMII-Grundkonfiguration
- RTL8197F-NIC-DMA
- Realtek/Tenda-Imageformat
- Vergleich RTL8367RB gegen RTL8367D

Die GPIO-Kombination 55/56/58 ist der wichtigste neue konkrete Befund für das RD05.

### Nicht verbessert

Die größten DSA-Risiken bleiben:

- RTL8367D-Kaltstart
- D-spezifische PHY-Initialisierung
- genaue CPU-Tagfelder
- saubere Linux-6.x-NIC- und DSA-Integration

## Empfehlung zur Verwendung

Das neue Archiv sollte **nicht** die alte Sammlung ersetzen.

Empfohlene Rollenverteilung:

- `entpackt(1).7z`: Bootloader, RTL8197F/FS-Low-Level, RTL8367RB-Referenz, ältere Voll-SDKs, Imagewerkzeuge
- `8197_all_SDKs.tar.zst`: umfangreichere RTL8367D-DAL-Bestände und neuere Linux-4.4/OpenWrt-21.02-artige Vendorquellen
- RD05-SPI-Dump und OEM-Laufzeitdaten: exakte Boardparameter, Partitionen, GPIOs, RTL8367D-Zustand und Xiaomi-Bootpfad
- AC23-Dump: RTL8197FH + RTL8367RB + RTL8814BR-Vergleich
- MW5-Dump: RTL8197FS + RTL8363NB + RTL8812BRH-Vergleich

## Endurteil

Das neue Archiv ist **inhaltlich wertvoll, aber nicht technisch neuer** als die beste bereits vorhandene RTL8197F/Linux-4.4-Sammlung. Es erweitert die Quellenlage um vollständige ältere SDK-Generationen und liefert eine wesentlich bessere Rekonstruktion von Boot, GPIO, SMI, RGMII, NIC und RTL8367RB.

Für den RTL8367D selbst bringt es dagegen **keine neue Implementierung**. Der wichtigste neue praktische Fortschritt ist die sehr plausible RD05-Pinbelegung:

```text
GPIO 55 / G7 = SMI MDIO
GPIO 56 / H0 = SMI MDC
GPIO 58 / H2 = RTL8367D RESET
```

Damit wird ein kabelgebundener OpenWrt-Bring-up besser planbar. Eine vollständige OpenWrt-24.10-/25.12-Portierung bleibt möglich, benötigt aber weiterhin eigenständige Linux-6.x-Arbeit und Hardwaremessungen; sie lässt sich nicht direkt aus diesem SDK kompilieren.

## Einschränkung der Analyse

Die Bewertung basiert auf vollständiger statischer Archiv-, Pfad-, Hash-, Konfigurations- und Quellcodeanalyse. Die enthaltenen SDKs wurden in diesem Schritt nicht vollständig kompiliert und die neu gewonnenen GPIO- und Switchannahmen noch nicht auf einer realen Platine elektrisch verifiziert.
