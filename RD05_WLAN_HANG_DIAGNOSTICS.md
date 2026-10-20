# RD05: Hänger während der WLAN-Initialisierung

## Gerätelog r0-5f845f4c12: UART aktiv, WLAN-TX gestört

UART meldet jetzt `irq=9`, `receive_mode=interrupt` und einen RX-Trigger von
1 Byte. Die übermittelten Diagnosebefehle kommen vollständig an; dieser Log
enthält keine UART-Overrun-Meldung. Das bestätigt diesen Befehlsblock, keinen
beliebig großen Paste-Test.

2,4 GHz erreicht `AP-ENABLED` und empfängt Open-System-Authentifizierungsanfragen,
aber die Stationstabellen bleiben leer. Die früher beobachtete Puffererschöpfung
ist in diesem kürzeren Log noch nicht sichtbar. 5 GHz bleibt in `HT_SCAN` und
meldet TX-DMA-Status `0x4400` und `0x6400`. Laut den RTL8812F-SDK-Definitionen:

| Bit | Bedeutung |
| --- | --- |
| 10 | `EP_QSEL_DIFF`: DMA-Kanal/Queue-Auswahl stimmen nicht überein |
| 14 | `PAYLOAD_UDN`: Payload-Unterlauf |
| 13, zusätzlich bei 0x6400 | `PAYLOAD_OVF`: Payload-Überlauf |

Damit ist der 5-GHz-AP-Start nicht abgeschlossen. Ein Passwortfehler oder
DHCP-Problem erklärt diese DMA-/Scan-Fehler nicht. Ein automatischer Neustart
von hostapd ist in diesem Log nicht belegt.

Ein konkreter Auswahlfehler wurde im Probe-Pfad gefunden: Die Präprozessor-
Bedingung vor `InitWlanBuffer88XX_V1()` berücksichtigte die CPU-Option
`CONFIG_RTL_8197G`, aber nicht die WLAN-HAL-Optionen `CONFIG_WLAN_HAL_8197G`
und `CONFIG_WLAN_HAL_8812FE`. Der RD05 hat eine RTL8197F-CPU und fiel dadurch
für beide Radios auf den alten generischen Buffer-Allocator zurück, obwohl
die anschließenden HAL-Funktionen chipspezifische Ring-Geometrien verwenden.
Die Auswahl berücksichtigt jetzt beide WLAN-HAL-Optionen. MW5/WRD12GN behalten
ihre bisherigen Zuordnungen, der AC23 verwendet bereits den V1-Pfad.

Die Linux6-Portierung benutzte kohärente TXBD/TXDESC-/Beacon-Descriptor-Speicher
bisher nur auf RTL8822B/8197F. RTL8812F/8197G erhalten jetzt ebenfalls diese
DMA-Allokationen. Für RTL8812F bleibt der vorhandene Queue-Code zuständig;
RTL8197G verwendet seinen eigenen Queue-Code und leitet DMA-Adressen aus den
jeweiligen Allokations-Handles plus geprüftem Offset ab. Der Beacon-Descriptor-
Count wird separat von der in Strides angegebenen Beacon-Tabelle behandelt.
Queue-Selektoren, Kanalbreiten und RF-Tabellen werden dadurch nicht umgestellt.
Diese Korrektur ersetzt den alten kmalloc-/Alias-Pfad, beweist aber noch nicht,
dass sämtliche beobachteten TX-Fehler behoben sind.

Zusätzlich behandelt cfg80211 Scan-SSIDs jetzt als längengeprüfte Bytefelder:
kein `strlen()` auf einer möglicherweise 32 Byte langen, nicht terminierten
SSID, keine P2P-Präfixprüfung ohne vorhandene SSID und keine alte gerichtete
SSID bei einem passiven Scan. Das gilt auch für Anfragen ohne Kanalliste.

`mw5-wifi diag` ergänzt vorhandene DMA-Basisadressen und Fehlerzähler beider
Radios. Nach dem Flashen erneut auf beiden Bändern verbinden und den Bericht
aufnehmen. Erwartet sind `AP-ENABLED` auch auf 5 GHz, ein erfolgreicher WPA-
Handshake und Einträge in den Stationstabellen. Das erfordert Hardwaretests.

Validierung: Die vier Treibervarianten kompilieren und linken gegen Linux
6.6.114/Backports 6.12.96. Native C-Tests prüfen kohärente DMA-Offsets bei
abweichenden CPU-/Busadressen, gültige DMA-Adresse 0, Speichergrenzen sowie
passive, binäre, maximale und ungültige Scan-SSIDs. Der echte Probe-Auswahlblock
wird zusätzlich mit den HAL-Konfigurationen aller vier Boards und einem
fehlgeschlagenen Allocator ausgeführt. Kein vollständiger
Firmware-Build und kein eigener Hardwaretest.

## Vollständigen Laufzeitbericht über UART erfassen

Nach einem fehlgeschlagenen Verbindungsversuch genügt ein kurzer Befehl:

```sh
mw5-wifi diag
```

Er sammelt Firmware-Version, UART-Status, WLAN-Interfaces, beide Stationstabellen,
verfügbare hostapd-Statusdaten sowie die begrenzten Authentifizierungs-, Kernel-
und Apply-Logs. Der Bericht erscheint auf der Konsole und in
`/tmp/mw5-webui/wlan-diag.log`. Es werden keine APs neu gestartet und keine
Passwort-/hostapd-Konfigurationsdateien ausgegeben.

Der zuletzt übermittelte abgeschnittene Gerätelog meldet noch
`r0-bf8862a476` und UART `irq:0`. Er enthält somit noch nicht den UART-Fix
`5ad14f0e32`; die mehrzeilige Befehlsfolge endet bereits bei `logread | grep -`.
Daraus lassen sich weder aktuelle Stationstabellen noch ein hostapd-Neustart
als Ursache für das zeitweise fehlende 5-GHz-Netz ablesen.

## Authentifizierung mit Firmware r0-962e91478d

Der neue Gerätelog bestätigt abgeschlossene Initialisierung beider Radios
und empfangene Open-System-Authentifizierungsanfragen. Auf 2,4 GHz scheitert
`issue_auth_resp()`, auf 5 GHz folgt auf die Antwortversuche keine belegte
Assoziierung. Beide Stationstabellen sind leer. `status=0` im bisherigen
`MW5-AUTH-RESP`-Log wurde vor dem Sendeaufruf ausgegeben und beweist keine
Übertragung. Das Ereignis heißt deshalb jetzt `MW5-AUTH-RESP-ATTEMPT`.

Die schnelle TX-Rückgewinnung für MGT/HI war bisher nur für RTL8822B Radio 0
aktiviert. Die generische HAL-Bedingung fordert mehr als zehn ausstehende
Deskriptoren; einzelne Management-Antworten können darunter liegen. Jetzt
aktivieren alle Linux6-HAL-Radios die passenden Completion-Interrupts und
planen darüber die bestehende Rückgewinnung ein. Registerbits und Queue-
Zuordnungen bleiben Sache der jeweiligen HAL. Es werden keine 8822B-
HI0-Umleitungen oder Descriptor-Layouts auf 8812F/8197G übertragen.

Verbleibende Authentifizierungsfehler nennen nun gedrosselt `AUTH-TX-FAIL`
mit Ursache (`management-buffer`, `header-buffer` oder `firetx`), Rückgabewert,
Queue und freien Puffern. Das unterscheidet Pool-Erschöpfung von TX-Ablehnung.
Es ist weiterhin ein Hardwaretest nötig, um die Authentifizierung zu bestätigen.

Das Startskript wartet auf ein neues `AP-ENABLED` von hostapd. `HT_SCAN`, ein
lebender Prozess, `type AP` und ein Beacon-Descriptor allein gelten nicht mehr
als erfolgreicher Start. Ein früheres `AP-ENABLED` wird nicht wiederverwendet.
RD05/AC23 warten außerdem nicht mehr bis zu 25 Sekunden auf den nur beim MW5
laufenden Auto-WAN-Dienst. Die MW5-Port-/MAC-Policy wird dort übersprungen;
das vorhandene boardspezifische RTL865x-Recovery bleibt erhalten.

Erfolgreiche Init-Phasen sind zur UART-Entlastung standardmäßig still; für
weitere Hängerdiagnosen `trace_init=1` aktivieren. Details und Grenzen der
UART-Änderung stehen in [UART_PASTE.md](UART_PASTE.md).

Validierung: Hosttests führen den echten HAL-Interrupt-Dispatcher mit
Management-/High-/Schwellwert-Ereignissen und die Shell-Startprüfung mit
aktuellen, fremden und veralteten hostapd-Ereignissen aus. Die vier WLAN-
Paketvarianten kompilieren und linken gegen Linux 6.6.114/Backports 6.12.96.
Das ersetzt weder einen Firmware-Build noch den Verbindungs- und UART-Test
auf dem RD05.

## Eingrenzung mit Firmware r0-66e44628ff

Der vollständige Gerätelog bestätigt: PCIe-Radio 0 erreicht Phase 190;
SoC-Radio 1 erreicht Phase 704, aber nicht 705. Der Hänger liegt damit in
`Wlan_HAL_Link()` des integrierten WLANs. NAPI und cfg80211 des PCIe-Radios
kehren vorher zurück. Das ist kein Nachweis für Funkbetrieb auf einem Band.

Der Abgleich mit den archivierten OEM-Kerneln zeigt eine falsche HAL-Auswahl:

| Board | Eigener OEM-Kernel | Korrigierte Paketwahl |
| --- | --- | --- |
| RD05 | `hal_Associate_8197G` bei `0x804f7630`, `InitPON8197G` bei `0x804f74a0` | `CONFIG_WLAN_HAL_8197G=y` |
| AC23 | `hal_Associate_8197G` bei `0x80394488`, `InitPON8197G` bei `0x803948a4` | `CONFIG_WLAN_HAL_8197G=y` |
| MW5 | `hal_Associate_8197F` bei `0x80362248`, `InitPON8197F` bei `0x80362628` | `CONFIG_WLAN_HAL_8197F=y` |

Die überprüften Symboltabellen liegen unter `reverse-engineering/oem-kernels`.
Beim RD05 setzt der OEM-Initialisierungspfad bei `0x803903d8` die interne
WLAN-Version auf `0x100f` (8197G), setzt bei `0x803903ec` die Gate-Bits `0x1f`
und ruft bei `0x803903f4` `Wlan_HAL_Link()` auf. Der AC23-Gerätetisch verwendet
wie MW5 weiterhin den integrierten MMIO-Bereich `0xb8640000`.
Die WLAN-HAL-Revision ist von der Bezeichnung des SoC-/CPU-Profils zu
unterscheiden; die CPU-Plattform wird hier nicht auf RTL8197G umgestellt.

Zusätzlich bricht `Wlan_HAL_Link()` jetzt bei fehlgeschlagener HAL-/HALMAC-
Anbindung sofort ab. Bisher konnte es anschließend nicht initialisierte
Callbacks aufrufen. Der integrierte Aufrufer wertet den Fehler nun aus;
der Fehlerpfad berücksichtigt teilweise angelegte HALMAC-Ressourcen.
Der optionale Surprise-Reset-Callback wird vor dem Aufruf geprüft: Die
8197G-Tabelle belegt ihn nicht, der reguläre StopHW-Pfad bleibt aktiv.
Der Factory-Import berücksichtigt nun auch `VERSION_8197G` als 2,4-GHz-Radio.

Diese Änderungen korrigieren belegte Zuordnungs- und Fehlerbehandlungsprobleme.
Eine Bestätigung, dass der RD05 damit startet und beide APs senden, steht aus.

## Frühere Diagnose und zusätzliche Phasen

Der Hardwarebericht mit Firmware `r0-65e596cca5` endet beim zweiten
`rtl8192cd_init_one()` nach Phase 106. Der erste Aufruf erreicht Phase 190.
Das belegt keinen erfolgreichen Funkbetrieb des ersten Radios. Die bisherigen
Phasen nennen auch nicht den Bus des betroffenen Aufrufs.

Die zusätzlichen Meldungen ändern keine RF-, IRQ-, DMA- oder Leistungsparameter
und sind **noch kein bestätigter Fix**. Vor jedem Attach erscheinen Radioindex,
Bus (`pci` oder `soc`), IRQ und MMIO-Ressourcenlänge direkt auf der seriellen
Konsole. Es werden keine Kalibrierwerte oder Zugangsdaten ausgegeben.

| Phase | Eintritt / Rückkehr |
| --- | --- |
| 716 / 717 | NAPI-Registrierung |
| 700 | gemeinsame Softwarestrukturen initialisiert |
| 701 / 702 | PCIe-HAL-Anbindung |
| 703 | SoC-Identifikation vor den Registerzugriffen |
| 704 / 705 | SoC-HAL-Anbindung |
| 706 / 707 | MAC-Softwareinitialisierung |
| 708 / 709 | cfg80211-Initialisierung |
| 710 / 711 | wiphy-Registrierung (Rückgabecode in Phase 711) |
| 712 / 713 | regulatorische wiphy-Konfiguration |
| 714 / 715 | Netzwerkschnittstellen-Registrierung |

Die Nummern sind Kennungen, keine aufsteigende Fortschrittsanzeige: etwa
710–713 liegen innerhalb von 708–709. Eine Eintrittsmeldung ohne passende
Rückkehrmeldung grenzt den Hänger ein, beweist aber allein keine Ursache.

Bereits auf der vorherigen Firmware kann unmittelbar nach dem Login, vor dem
automatischen WLAN-Start, die Konsolenausgabe erweitert werden:

```sh
dmesg -n 8
```

Anschließend die serielle Ausgabe vom Start bis zum Hänger vollständig erfassen.
Wenn ein neuer Diagnose-Build verwendet wird, insbesondere die letzte
`attach vendor core`-Zeile und die nachfolgenden Phasen mitschicken.
Während eines Hängers sind SSH und weitere Shellbefehle kein verlässlicher
Diagnoseweg. Es wurden keine Hardwaretests oder vollständigen Firmware-Builds
für diese Instrumentierung durchgeführt; der RD05-MIPS-Treiber wurde mit
Linux 6.6.114 und den OpenWrt-Backports kompiliert und gelinkt.
