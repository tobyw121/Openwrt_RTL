# Lizenz- und Sicherheitshinweis

1. Vendorquellen können proprietäre Realtek-Hinweise enthalten. Nicht ungeprüft veröffentlichen oder upstreamen.
2. Keine Factory-, NVRAM-, BDATA- oder Kalibrierungsdaten aus privaten Dumps in Images einbauen.
3. Bootloader und Factory-Partitionen beim Testen zunächst schreibgeschützt behandeln.
4. Neue Images zuerst per RAM/Initramfs und serieller Konsole testen.
5. RTL8367-Registertabellen niemals zwischen D, RB, R und C ungeprüft austauschen.
