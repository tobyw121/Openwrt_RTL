# RT8197F / RTL8197F + RTL8367RB Plattformpaket

Dieses ZIP wurde aus `GPL_AX12v1.tar.zst` erstellt und enthält eine kompakte Sammlung der relevanten Daten für die Realtek-RTL8197F/RTL8367RB-Plattform.

## Inhalt

- `docs/KURZ_ZUSAMMENFASSUNG.md` – kurze technische Zusammenfassung auf Deutsch.
- `reports/inventory.csv` – Dateiindex mit Kategorie, Größe und SHA-256.
- `reports/inventory_by_category.txt` – Statistik nach Kategorien.
- `reports/belegstellen_key_excerpts.txt` – wichtigste Code-/Config-Ausschnitte mit Zeilennummern.
- `reports/grep_hits_8197_8367_core.txt` – Suchtreffer zu 8197F/8367R/8367RB.
- `reports/archive_target_filelist_8197_8367.txt` – passende Dateinamen aus dem Gesamtarchiv.
- `source_subset/rtl8197/` – relevante Source-Dateien und Verzeichnisse:
  - Board/BSP/Rootfs: `boards/rtl8197F/`
  - Bootloader-Configs/Init: `bootcode/`
  - Kernel-Netzwerk/Switch: `rtknet/drivers/net/rtl819x/`
  - WLAN-HAL/Kalibrierdaten: `backports-5.2.8-1/.../rtl8192fe/.../8197F`

## Hinweis zur Bezeichnung

Im Quellcode heißt die Plattform `RTL8197F`; `RT8197F` wurde als vermutlich verkürzte Schreibweise behandelt.
