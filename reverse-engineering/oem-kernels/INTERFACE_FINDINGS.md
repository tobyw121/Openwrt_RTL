# Portmodus und WLAN-Schlüssellebensdauer

Die neuen Belege werden mit
`scripts/rtl8197f-oem-interface-evidence.py --output /tmp/interface-evidence.json`
aus den drei archivierten OEM-Kernels reproduziert. Das Werkzeug prüft
zunächst die komprimierten und entpackten Hashes von Kernel und Symboltabelle.
Es speichert Funktionsspannen, Hashes, direkte JAL-Aufrufstellen und kurze
Instruktionsbelege, keine Factory-Werte oder Schlüssel. Das Ergebnis steht in
`target/linux/realtek/rtl8197f/oem-interface-evidence.json`.

## Switch: EXT1 benötigt eine Vier-Bit-Maske

`rtl8367c_setAsicPortExtMode` berechnet in allen drei Images zunächst
`extension_id * 4`, lädt `0xf`, verschiebt diese Maske um den berechneten
Wert und übergibt sie an `rtl8367c_setAsicRegBits` für Register `0x1305`.

| Kernel | Beginn des Instruktionsbelegs | Aufruf von `setAsicRegBits` |
| --- | --- | --- |
| MW5 | `0x801e44e4` | `0x801e44f4` |
| AC23 | `0x801ec32c` | `0x801ec33c` |
| RD05 | `0x80342204` | `0x80342214` |

Die SDK-Datei `8197/API_Source/rtl8367c_asicdrv_port.c` und deren
Registerheader bestätigen: EXT0 liegt in Bits 3:0 von `0x1305`, EXT1 in
Bits 7:4 desselben Registers, EXT2 in Bits 3:0 von `0x13c3`.
Referenzcommit und Quellhash sind im JSON festgehalten.

Unser nativer DSA-Treiber verschob den **Wert** für EXT1 korrekt um vier,
die **Maske** jedoch nur um eins. Mit `mask=0x1e` und RGMII-Wert `0x10`
konnten somit Bits 3:1 des benachbarten EXT0 verändert werden, während
alte EXT1-Bits 7:5 stehen blieben. Beispielsweise wird aus einem alten
Registerwert `0x00ff` mit dieser Operation `0x00f1`, statt des korrekten
`0x001f`. Eine bereits passende Resetbelegung konnte den Fehler verdecken.

Patch 333 verwendet für Maske und Wert denselben vorhandenen Bitoffset.
Das betrifft insbesondere die CPU6/EXT1-Anbindung von MW5 und AC23.
Der eigene RTL8367D-EXT1-Pfad für RD05 nutzt bereits `GENMASK(7, 4)` und
erhält keine geänderte Registersequenz. Die C-Familienroutine im RD05-Image
beweist nicht, dass sie im D-Familien-Startpfad ausgeführt wird.

Die Regression kompiliert die tatsächlich gepatchten Makros und prüft für
alle 16 Moduswerte und alle 65.536 alten Registerwerte sowohl den gesetzten
Modus als auch den Erhalt der Nachbarbits.

## RD05: Init-Stub auch im SDK vorhanden

Die Funktion `dal_rtl8367d_switch_init` ist auch im verfügbaren AX12v1-SDK
ein unmittelbares `return RT_ERR_OK`. Das stimmt mit dem bereits gefundenen
Zwei-Instruktions-Stub im RD05-Kernel überein. Der Quellpfad und dessen Hash
stehen nun neben dem erneut geprüften Binärbeleg. Es handelt sich somit
nicht lediglich um eine durch Disassemblierung übersehene Init-Tabelle in
dieser Funktion. Welche frühen Initialisierungszustände aus Reset,
Bootloader und anderen Routinen stammen, bleibt separat zu rekonstruieren.

Auch `StopHW8812FE` ist im RD05-Image ein acht Byte langer Erfolgs-Stub.
Das beweist keinen fehlenden aktiven Stopppfad: Unsere RTL8812F-HAL-Tabelle
verwendet bereits den gemeinsamen `StopHW88XX` als `StopHWHandler`.

## WLAN: sichere Übergabe und gezieltes Löschen von Schlüsseln

Die vorhandenen SDK-Funktionen unterscheiden Broadcast-Gruppenschlüssel,
Clientschlüssel und PMF-Verwaltungsschlüssel. Ein Null-MAC ist im alten
Lösch-API hingegen ein globaler CAM-Reset. Der RD05-OEM-Kernel enthält
nachweislich sowohl `CamDeleteOneEntry`- als auch `CamResetAllEntry`-Aufrufe
in `rtl_net80211_delkey`. Die Hashes und Aufrufstellen sind archiviert;
die genaue Zweigsemantik wurde zusätzlich anhand unserer SDK-Quelle geprüft.

Der gemeinsame cfg80211-Pfad für alle drei Boards prüft jetzt vor der
Übergabe:

- Exakte Schlüssellängen: WEP40 5, WEP104 13, TKIP 32, CCMP und BIP-CMAC 16 Byte.
- Datenindizes 0..3, BIP-CMAC-Gruppenindizes 4/5 sowie gültige Unicastadresse
  und Index 0 für Clientschlüssel. Extended Key ID und MLO werden dadurch
  nicht neu unterstützt.
- Nicht unterstützte Link-IDs, fehlende Daten und ungültige Längen werden
  vor dem Kopieren oder Aufrufen der Vendor-Funktion zurückgewiesen.

Der Linux6-Löschpfad übergibt Gruppenschlüssel als Broadcast und behandelt
IGTK 4/5 separat. Er löscht den passenden Software-Schlüsselspeicher und
bei einem aktuellen Hardwareeintrag die CAM-Zuordnung. Einen alten GTK1
nach Installation von GTK2 zu entfernen darf dessen gemeinsamen VAP-CAM-Slot
nicht löschen; dieselbe Regel gilt umgekehrt. Ein alter IGTK-Index darf den
neuen Verwaltungsschlüssel ebenfalls nicht löschen. Schlüsselmaterial und
Paketnummern des gelöschten Softwareeintrags werden vollständig genullt.
Das Entfernen eines Clients setzt nicht mehr den gemeinsamen Cipherzustand
auf unverschlüsselt zurück. Fehlende Clients liefern `-ENOENT`.

Bei aktivierter Option `CONFIG_RTL_HW_ENC_GROUP_CIPHER` sichert der HAL
Gruppenschlüssel für spätere VAP-CAM-Umsortierungen. Der alte Puffer hatte
64 Byte, unsere Linux6-Kompatibilitätsstruktur jedoch 76 Byte. Der alte
cfg80211-Code kopierte zuerst die Zeigerstruktur und übersprang dann die
Datensicherung wegen der Größe. Damit blieb ein Zeiger auf die lokale
Stackvariable zurück. Der Puffer bietet jetzt 128 Byte; ein Buildcheck
erzwingt ausreichende Kapazität, und der gespeicherte Zeiger zeigt direkt
auf die dauerhafte Kopie im Interfaceobjekt. Beim Löschen des aktuellen
GTK werden Kopie und Zeiger entfernt. Die Option wird nicht eingeschaltet.

Die Hostregression führt die tatsächlichen Prüf-, Sicherungs- und
Löschfunktionen mit simulierten CAM-Grenzen aus: ungültige Eingaben,
Schlüsselwechsel in beiden Richtungen, IGTK-Wechsel, fehlende Clients,
mehrfaches Löschen und Lebensdauer der gesicherten Struktur.

## Grenzen und Hardwareprüfung

Validierung dieses Änderungssatzes: 53 Hosttests bestanden; Boardprofile,
26 MW5-Baseline-Prüfungen und klassische Routingregeln bestanden. Die drei
WLAN-Varianten wurden jeweils regulär und zusätzlich mit
`--extra-cflags=-DCONFIG_RTL_HW_ENC_GROUP_CIPHER` gegen Linux 6.6.114 und die
gepatchten OpenWrt-Backports 6.12.96 übersetzt und zu `rtl8192cd.o` gelinkt,
ohne unaufgelöste WEXT-Symbole. Der mit Patch 333 korrigierte
`rtl8365mb.o` übersetzt ebenfalls für MIPS. Alle drei Kernelarchive wurden
erneut vollständig gegen ihre Bytes, Symbole und Disassemblierung geprüft.
Das sind Objekt-/Regressionstests; ein vollständiger Firmwarebuild,
abschließendes MODPOST und Gerätetests gehören nicht zu diesen Ergebnissen.

Diese Änderungen vervollständigen noch nicht die Rückentwicklung. Offen
bleiben unter anderem Kaltstart-Parität der Switches, unbekannte kompakte
AC-Kalibriertabellen, allgemeine VIF-Lebensdauer, RTL8812F-Firmware-
Stromsparfunktionen sowie die Verifikation von DFS, Temperaturen und Last.
Die bestehenden Beschleunigerregeln und WEXT-Abschaltung bleiben erhalten.

Auf den Geräten sind Kaltstart und wiederholte Netzwerkneustarts, WPA2-
Anmeldung, GTK-Rekey mit mehreren Clients, Multicast/Broadcast, optional
PMF mit IGTK-Wechsel und bidirektionaler LAN-/WLAN-Verkehr zu prüfen.
Für MW5/AC23 muss zudem das EXT1-Feld in `0x1305` nach der Konfiguration
RGMII anzeigen und die Nachbarbits erhalten. Die Änderungen sind weder
ein gemessener Durchsatzgewinn noch ein Hardwarebeweis für die Behebung
des ursprünglichen MW5-Netzstillstands.
