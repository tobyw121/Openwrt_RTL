# Vollständig archivierte OEM-Kernels: MW5, AC23 und RD05

Diese Ablage enthält die **vollständigen entpackten Kernel-Images** aus den
drei bekannten SPI-Referenzdumps, alle wiederhergestellten Kallsyms-Symbole
und eine wortweise vollständige, annotierte MIPS32R2-Little-Endian-
Disassemblierung. Stand der Ablage: 20.10.2026.

| Verzeichnis | OEM-Kernel | Image-Bytes | Symbole | Disassembly-Teile |
| --- | --- | ---: | ---: | ---: |
| `mw5/` | Linux 3.10.90 | 6.890.804 | 29.629 | 7 |
| `ac23/` | Linux 3.10.90 | 7.526.476 | 31.553 | 8 |
| `rd05/` | Linux 4.4.176 | 9.780.680 | 24.127 | 10 |

Insgesamt sind 24.197.960 Image-Bytes beziehungsweise 6.049.490 Wörter
erhalten; die gesamte Ablage belegt komprimiert rund 37 MiB. Sie benötigt
für weitere Analysen weder die früheren `/tmp`-Dateien noch einen erneuten
Download der SPI-Dumps.

Das ist eine vollständige **technische Zerlegung des gespeicherten Images**,
keine vollständig abgeschlossene semantische Rückentwicklung und keine
Wiederherstellung der ursprünglichen C-Quellen. Nicht jedes als Instruktion
angezeigte Wort ist tatsächlich ausführbarer Code: Die vollständige Ansicht
erfasst absichtlich auch Daten, Tabellen, Strings und Padding. Laufzeitpfade,
indirekte Aufrufe und Registerbedeutungen müssen separat untersucht werden.

## Inhalt pro Gerät

- `manifest.json`: Herkunft, SPI-/Kernel-SHA256, LZMA-Position und -Länge,
  Image-Basis, Kallsyms-Prüfergebnis, Werkzeugversion, Datenschutzprüfungen,
  Prüfsummen und Größen sämtlicher Artefakte sowie lückenlose Teilbereiche.
- `kernel.raw.xz`: verlustfrei komprimiertes, vollständiges Kernel-Image ab
  Ladebasis `0x80000000`. Byteidentisch zum entpackten Referenzkernel.
- `symbols.tsv.xz`: sämtliche Symboladressen, Typen und Namen, einschließlich
  Aliasnamen, Datensymbolen und im Image nicht gespeicherten BSS-Adressen.
- `text-symbol-spans.tsv.xz`: Adressintervalle mit Textsymbolen, Größen und
  SHA256. Die Grenzen folgen benachbarten Symboladressen und sind keine
  garantierten ursprünglichen C-Funktionsgrößen. Linker-/Init-Marker sind
  ebenfalls Textsymbole und nicht automatisch Funktionen.
- `linker-markers.json`: vorhandene Text-, Daten-, Init- und BSS-Marken.
  Das sind rekonstruierte Linkermarken, keine ursprüngliche ELF-Sektionstabelle.
- `disassembly/START-STOP.dis.xz`: komprimierte Textansichten in maximal
  1-MiB-Image-Intervallen; STOP ist exklusiv. Jede Zeile enthält Adresse,
  ursprüngliches 32-Bit-Wort und GNU-Objdump-Dekodierung. Ausgerichtete
  Symbole und direkte Sprungziele werden annotiert. Auch Nullwörter bleiben
  vollständig erhalten. Nicht ausgerichtete Datensymbole stehen weiterhin
  bytegenau in der vollständigen Symboltabelle.

Die BSS-Laufzeitinhalte befinden sich nicht im gespeicherten Kernel und
werden nicht erfunden. Separate Rootfs-Dateien und nachgeladene `.ko`-Module
gehören nicht zum archivierten Kernel-Image. Die ursprünglichen SPI-Images,
Bootloader, U-Boot-Umgebung, Bdata, Factory-/RF-Kalibrierpartitionen und
Benutzereinstellungen werden hier nicht abgelegt. Im Kernel einkompilierte
generische OEM-Daten und Firmwaretabellen bleiben unverändert enthalten.

## Suchen und erneut analysieren

Aus dem Repository-Hauptverzeichnis:

```sh
# Sämtliche Namen/Adressen, zum Beispiel WLAN- und NIC-Einstiegspunkte:
xzcat reverse-engineering/oem-kernels/ac23/symbols.tsv.xz |
  rg 'New_swNic|8192cd|PHYDM|phydm|rtl8367'

# In der vollständigen annotierten Disassemblierung suchen:
xzgrep -n '<New_swNic_init>:' reverse-engineering/oem-kernels/ac23/disassembly/*.xz

# Kernel für die bestehenden Analysewerkzeuge wiederherstellen:
xz -dc reverse-engineering/oem-kernels/rd05/kernel.raw.xz > /tmp/rd05-kernel.raw
python3 scripts/rtl8197f-oem-switch-audit.py /tmp/rd05-kernel.raw \
  --symbol rtl865x_reinitSwitchCore --output /tmp/rd05-reset-audit.json
```

Gerade beim AC23 darf die Analyse **nicht bei `_etext` enden**:
`_etext = 0x804dc108`, aber beispielsweise `alloc_rx_buf = 0x804dd234`,
`New_swNic_send_tso_sg = 0x804df150` und `New_swNic_send = 0x804df18c`
liegen dahinter; `_sdata = 0x804f5364`. Die vollständige Image-Zerlegung
enthält auch diese Bereiche. Welche ursprünglichen ELF-Sektionen sie
bildeten, wird daraus nicht ungeprüft abgeleitet.

## Integrität prüfen

Der unabhängige Prüfer benötigt nur Python mit Standardbibliothek:

```sh
python3 scripts/verify-oem-kernel-archives.py \
  reverse-engineering/oem-kernels/mw5 \
  reverse-engineering/oem-kernels/ac23 \
  reverse-engineering/oem-kernels/rd05
```

Er prüft komprimierte und entpackte Größen/Hashes, Kernel-Identität,
Symbolanzahl, alle Textsymbol-Spannen und **jedes einzelne Wort** aller
Disassembly-Teile gegen das archivierte Image. Lücken, Überlappungen,
vertauschte Endianness und abgeschnittene Teile führen zum Fehler.
Ein bestandener Integritätstest beweist keine korrekte Interpretation des
OEM-Verhaltens oder Funktion unserer OpenWrt-Treiber auf Hardware.

## Aus dem Referenzdump reproduzieren

Quelle ist `tobyw121/Openwrt_RTL`, Branch `RT8197F`, Commit
`e6d68e9067f54b695022f61ed52e50dc56c5a7b7`, Verzeichnis
`openwrt24.10-realtek-rtl8197f-mw5/fullflash-templates`.
Die genauen SPI-Fingerprints stehen in jedem Manifest und im Archivierer.

```sh
python3 scripts/rtl8197f-archive-oem-kernel.py \
  --board mw5 --dump /pfad/tenda-nova-mw5-spi.bin \
  --output /tmp/neue-oem-analyse/mw5 --objdump mips-linux-gnu-objdump
```

Für die anderen Geräte `--board ac23` beziehungsweise `--board rd05` und
deren Referenzdatei verwenden. Das Ausgabe-Verzeichnis darf noch nicht
existieren. Benötigt werden Python mit `lzma` und GNU-MIPS-Objdump; dessen
Version ist im Manifest erfasst. Andere Objdump-Versionen können andere
Textdarstellungen erzeugen; die Originalwörter müssen identisch bleiben.

Der Archivierer akzeptiert ausschließlich die drei bekannten SPI-Hashes,
extrahiert genau einen begrenzten LZMA-Kernelstream und prüft dessen
aufgezeichneten SHA256 sowie vollständige Kallsyms-Struktur. Vor dem
Archivieren werden bekannte Factory-/Umgebungskennungen und binäre MAC-
Formen auf Übereinstimmungen im Kernel sowie PEM-Private-Key-Marker geprüft.
Diese Prüfung fand keine Treffer; sie ist kein allgemeiner Geheimnisscanner.
Generische einkompilierte OEM-Defaults sind Teil des unveränderten Images.
Keine Routerprogramme wurden bei der Erstellung ausgeführt.

## Bisherige fachliche Ergebnisse

Siehe [NIC-Geometrie und Resetpfade](NIC_FINDINGS.md),
[Switch-Rückentwicklung](../../OEM_SWITCH_REVERSE.md) und
[Factory-/WLAN-Integration](../../OEM_DUMP_INTEGRATION.md).
