# Weitere NIC-Erkenntnisse aus allen drei vollständigen Kernels

Die maschinenlesbaren Belege stehen in
`target/linux/realtek/rtl8197f/oem-nic-evidence.json`. Sie enthalten
Kernel-/Funktions-Hashes und die genauen MIPS-Wörter/Adressen der
Schleifenschritte und direkten Reset-Aufrufe. Originalcode und sämtliche
anderen Bereiche bleiben in den gerätespezifischen Archiven verfügbar.

## RX und TX verwenden nicht auf allen FH/VG-Boards dieselbe Geometrie

| Gerät | OEM `New_swNic_init` | TX-Schleifenschritt | RX-Schleifenschritt | Dwords |
| --- | --- | --- | --- | ---: |
| MW5 | `0x801a77c8` | `0x801a7884`: +24 | `0x801a79bc`: +24 | 6 |
| AC23 | `0x801a2370` | `0x801a242c`: +24 | `0x801a2564`, `0x801a25c0`: +24 | 6 |
| RD05 | `0x802b91ac` | `0x802b926c`: +32 | `0x802b957c`: +32 | 8 |

Das bestätigt die zuvor separat rückentwickelten OWN-Prüfungen nun direkt
an der Initialisierung beider Ringe. Beim AC23 berechnet die TX-Seite auch
das Ringende als `(count - 1) * 24` und setzt die Wrap-Markierung am letzten
Deskriptor. Seine RX-Seite setzt und bewegt die Deskriptoren ebenfalls in
24-Byte-Schritten. Eine 32-Byte-**Ausrichtung des Allokationsanfangs** ist
kein Beweis für einen 32-Byte-**Abstand zwischen Deskriptoren**.

Unser AC23-DTS enthielt keine `realtek,descriptor-dwords`-Angabe; dadurch
wählte der native Treiber bisher seinen Acht-Dword-Standard. Der DTS setzt
jetzt ausdrücklich **6 Dwords / 24 Byte**. MW5 erhält dieselbe explizite
Angabe und RD05 **8 Dwords / 32 Byte**; diese beiden Angaben entsprechen
dem bereits gewählten Treiberverhalten. Der Boardvalidator prüft nun auch
den Zusammenhang zwischen Dword-Angabe, Byteabstand und DTS.

Die bestehende CPUICR1-Konfiguration unterstützt beide Formen: sechs Dwords
werden durch gelöschte TXDESC-/RXDESC-Felder kodiert; acht Dwords durch den
Wert 8 an den jeweiligen Feldpositionen. Unsere Regression führt die
tatsächliche Treiberfunktion mit beiden Formen aus. Sie prüft außerdem,
dass die Padding-Hilfsfunktion bei sechs Dwords nicht die nachfolgenden
Wörter überschreibt, während sie bei acht Dwords das Padding löscht.

Die Änderung gleicht die angeforderte Geometrie an den konkreten OEM-Kernel
an. Sie beweist keine Hardwarekorrektur, vollständige AC23-Ingress-Parität
oder einen Durchsatzgewinn. Nach Build/Flash müssen CPUICR1, Ringfortschritt,
korrekte Paketdaten und bidirektionale LAN-/WLAN-Last geprüft werden.
Der MW5-Recovery-Pfad wird nicht auf AC23 oder RD05 ausgeweitet.

## Der OEM-Reset spielt weit mehr als den TX-Anstoß erneut ab

Die direkten Aufrufe von `rtl865x_reinitSwitchCore` lauten bei allen drei
Kernels in derselben fachlichen Reihenfolge:

```text
rtl819x_save_hw_tables
re865x_reProbe
New_swNic_reInit
rtl865x_reChangeOpMode
rtl819x_restore_hw_tables
```

AC23 und RD05 besitzen daneben einen direkten `__udelay`-Aufruf.
Beim MW5 liegt der Einstieg bei `0x80199454`, beim AC23 bei `0x80193560`,
beim RD05 bei `0x802a6d10`.

Im MW5-Code werden vor der Aufrufkette CPUIIMR (`0xb8010028`) und CPUICR
(`0xb8010000`) auf 0 gesetzt sowie SwitchCore-/Interruptzustände verändert.
Anschließend werden CPUICR, DMA-Parameter und Interruptzustände erneut
programmiert. Dieser OEM-Reset ist keine isolierte TX-Doorbell-Reparatur.

Die direkte Aufrufkette belegt die Beteiligung von Tabellen und NIC-Reinit,
aber noch nicht deren vollständigen Umfang, alle indirekten Aufrufe oder
die Sicherheit eines solchen Resets bei laufendem Linux-NAPI-/DSA-Betrieb.
Der vorhandene begrenzte MW5-TX-Wiederanstoß bleibt deshalb erhalten;
der OEM-Gesamtreset wird nicht in den laufenden Datenpfad kopiert.

## AC23-Code jenseits von `_etext`

Die archivierten Symboltabellen und Bytes zeigen TX-Wrapper sowie weitere
Netzwerkfunktionen hinter `_etext`; konkrete Adressen stehen im
[Archiv-README](README.md). `New_swNic_send` bei `0x804df18c` sperrt lokal
Interrupts, ruft `_New_swNic_send` bei `0x801a19b0` auf und stellt anschließend
den Interruptzustand wieder her. Der Wrapper ist also ausführbarer Code
und keine bloße Datenadresse hinter einer vermeintlichen Textgrenze.

Für spätere Analysen muss die Auswahl nach Symbolen und vollständigen
Image-Grenzen erfolgen. Die vollständige Archiv-Disassemblierung verliert
diese Bereiche nicht. Die ursprüngliche ELF-Sektionierung ist weiterhin
nicht rekonstruiert.
