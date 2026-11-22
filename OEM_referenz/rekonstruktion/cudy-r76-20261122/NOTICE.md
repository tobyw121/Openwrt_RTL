# Herkunft und Lizenzhinweise

Die Eingangsfirmware `Cudy_AX1500.bin` und die bereits vorhandene Hersteller-GPL
`GPL_for_AX1500_rtl8197.tar.gz` wurden vom Nutzer bereitgestellt. Extrahierte
Binärdateien behalten ihre ursprüngliche Herkunft; eine Extraktion oder
Dekompilierung verleiht ihnen keine neue Lizenz.

`oem_reference/` enthält unveränderte Dateien aus dieser Hersteller-GPL. Ihre
Copyright- und Lizenzhinweise sind maßgeblich. Der beigefügte GPL-2.0-Text ist
eine Lesekopie. Das Verzeichnis ist ein gezielter Referenzauszug, kein vollständiges
OpenWrt-/Kernel-Bausystem. `source_manifest.json` nennt die übernommenen Dateien.

Die selektiv rekonstruierten C-Dateien behalten ihre jeweiligen SPDX- und
Herkunftshinweise. Ihre neuen, für Offlineprüfungen geeigneten APIs werden in den
Fachberichten beschrieben. Sie werden nicht als unveränderter Herstellerquellcode
ausgegeben.

`decompiled/` enthält automatische Ghidra-Ausgaben zur Prüfung. Diese sind kein
kompilierbarer C-Quellbaum. Typen, Signaturen, Aliase und Kontrollfluss müssen
anhand von ELF-Dateien, Relokationen und Disassemblaten geprüft werden.

Die mitgelieferten OEM-Startskripte sind Referenzmaterial. Die Analyse- und
Prüfwerkzeuge starten keine OEM-Programme, laden keine Module und greifen nicht
auf Routerhardware zu. MIPS-Befehle werden ausschließlich mit synthetischem
Speicher im dokumentierten Emulator ausgeführt.
