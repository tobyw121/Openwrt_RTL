# Lange Eingaben über die serielle Konsole

Für das RTL8197F-Target beträgt der BusyBox-Eingabepuffer jetzt 8192 statt
512 Byte. Das gilt auch für RD05, MW5 und AC23. Eine einzelne interaktive
Kommandozeile darf einschließlich aller Befehle, Anführungszeichen und
Argumente höchstens 8191 Byte umfassen; ein Byte bleibt für den Abschluss.
Andere Targets behalten ihren bisherigen Standard.

Der RTL8197F-UART mit seiner 16-Byte-FIFO löst den Empfangsinterrupt jetzt ab
einem statt acht Bytes aus. Dadurch bleibt mehr Platz für weitere Zeichen,
während der Interrupt auf Bearbeitung wartet. Die Einstellung wird bei
termios-Änderungen erneut gesetzt. Baudrate, Register-Aliasse, Sende-FIFO und
UARTs ohne den RTL8197F-Quirk behalten ihre bisherigen Einstellungen.
Das vergrößert nicht die physische FIFO und garantiert keine verlustfreie
Übertragung während beliebig langer Interruptsperren.

## RD05-Gerätelog r0-962e91478d

Die Meldung `ttyS ttyS0: input overrun(s)` belegt einen Empfangsüberlauf.
Das ist unabhängig vom 8192-Byte-Zeilenpuffer: Bereits verlorene UART-Zeichen
kann die Shell nicht wiederherstellen. Der WLAN-Treiber druckt erfolgreiche
Initialisierungsphasen deshalb standardmäßig nicht mehr synchron mit
`KERN_EMERG` auf die Konsole. Der letzte Zustand bleibt in den Modulparametern
`init_phase`, `init_rc` und `last_phase` erhalten; Fehler werden weiterhin
sofort ausgegeben. Ausführliche Phasen lassen sich bei Bedarf mit
`trace_init=1` beim Laden oder über
`/sys/module/rtl8192cd/parameters/trace_init` aktivieren. Währenddessen keine
großen Texte einfügen.

Diese Änderung reduziert eine belegte Quelle langer Konsolenausgaben. Sie
beweist keine maximal zulässige Interruptlatenz des gesamten Vendor-Treibers.
Bis zum Hardwaretest im Terminalprogramm 1–2 ms Sendeverzögerung pro Zeichen
verwenden; für größere Skripte eine Dateiübertragung nutzen.

## Aktualisieren

```sh
git pull --ff-only origin MW5_Stable
make defconfig
make package/utils/busybox/clean
make -j"$(nproc)"
```

Der Kernel-Patch und BusyBox müssen beide im neu geflashten Image enthalten
sein. `make defconfig` übernimmt den neuen Target-Standard. Bei ausdrücklich
aktivierter BusyBox-Anpassung (`CONFIG_BUSYBOX_CUSTOM=y`) bleibt der eigene
Wert erhalten; unter `Base system -> BusyBox -> Library tuning -> Maximum
length of input` gegebenenfalls ebenfalls 8192 einstellen.

Große Skripte am besten als Datei übertragen. Bei weiterhin verlorenen
Zeichen beim schnellen Einfügen kann das Terminalprogramm eine Verzögerung
von etwa 1 ms pro Zeichen verwenden. Das hilft gegen UART-Überläufe, hebt
aber das Zeilenlimit nicht auf. `stty cols` ändert nur die Darstellung.

## Prüfung

- Der echte Kconfig-Resolver wählt 8192 für RTL8197F und 512 für andere Targets.
- BusyBox 1.36.1 wurde mit dem 8192-Byte-Eingabepuffer als Hostprogramm gebaut.
  In einer interaktiven PTY-Sitzung blieben eingefügte Nutztexte mit 1024,
  4096 und 7800 Zeichen vollständig erhalten; die Shell prüfte jeweils die
  tatsächliche Variablenlänge. Das war kein physischer UART-Test.
- Die gepatchte `dw8250_set_termios()`-Funktion wurde mit allen 256 FCR-Werten
  mit und ohne RTL8197F-Quirk ausgeführt: Trigger und FIFO-Aktivierung ändern
  sich nur beim Quirk, die übrigen Bits bleiben erhalten.
- Der tatsächliche gepatchte `8250_dw.o` kompiliert für MIPS/Linux 6.6.114.
  Ein vollständiger Firmware-Build und ein Hardware-Pastetest stehen aus.
