# RD05: Hänger während der WLAN-Initialisierung

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
