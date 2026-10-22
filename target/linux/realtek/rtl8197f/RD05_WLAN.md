# RD05: WLAN in LuCI und netifd

Die bisherige Datei `/etc/config/wireless` enthielt nur einen Abschnitt vom
Typ `mw5_cfg80211`. Die APs wurden separat durch `mw5-wifi` gestartet. Daher
konnte WLAN funktionieren, obwohl LuCI keine Funkgeräte oder WLAN-Netzwerke
anzeigte: netifd hatte weder `wifi-device`- noch `wifi-iface`-Abschnitte.

Das RD05-Paket installiert jetzt einen netifd-Adapter und die Abhängigkeiten
`wifi-scripts`, `iwinfo` und `rpcd-mod-iwinfo`. Die vorhandene
mac80211-Konfigurationsschnittstelle delegiert ausschließlich auf dem RD05
an diesen Adapter. Andere Geräte verwenden weiterhin ihre bisherige Verwaltung.
Der Typ `mac80211` ist hier die LuCI-/UCI-Kompatibilitätsschnittstelle;
der eigentliche Treiber bleibt der vendorbasierte rtl8192cd-cfg80211-Port.
Dies stellt ihn nicht auf den Kernel-mac80211-Datenpfad um.

## Konfiguration und Lebenszyklus

| UCI-Gerät | Schnittstelle | Band |
| --- | --- | --- |
| `radio0` / `phy0` | `wlan0` | 5 GHz |
| `radio1` / `phy1` | `wlan1` | 2,4 GHz |

Bei der ersten Migration werden SSIDs, Schlüssel, Kanäle, Kanalbreiten,
Ländercode und Aktivierungszustand aus `mw5wifi` übernommen. Beide APs werden
dem logischen Netzwerk `lan` zugeordnet. Bereits vorhandene reguläre
`wifi-device`-Abschnitte verhindern eine automatische Neuanlage; LuCI-Änderungen
werden nicht durch `mw5-wifi init` oder dessen Statusspeicherung überschrieben.
Bei solchen vorhandenen Konfigurationen müssen `phy`, `band` und die festen
Interface-Namen zur Tabelle passen.

netifd wird der alleinige Besitzer der APs. Der bisherige Bootdienst lädt
die Konfiguration neu, statt zusätzlich eigene hostapd-Prozesse zu starten.
Auch `mw5-wifi apply` und `mw5-wifi stop` leiten auf die reguläre
WLAN-Verwaltung weiter. AP-Einstellungen künftig in **Netzwerk → WLAN** oder
über `uci ... wireless` ändern; die alten `mw5wifi`-SSID-/Schlüsseleinstellungen
sind nach der Migration nicht mehr maßgeblich.

Der Adapter verwendet die vorhandene, auf dem RD05 erprobte hostapd-Ansteuerung
mit `driver=nl80211`. Er registriert die Root-Interfaces und überwacht die
hostapd-Prozesse über netifd. Start/Stop beider Radios werden durch eine
gemeinsame Sperre serialisiert; die vorhandenen L2-Reparaturschritte und
Radio-Lifecycle-Einstellungen bleiben erhalten. Ein fehlgeschlagener Start
wird nicht als aktives Funkgerät gemeldet.

Die Sperre verwendet atomisches `mkdir` und benötigt kein zusätzliches
`flock`-Paket. Nach höchstens 20 Sekunden Wartezeit wird abgebrochen, ohne
eine fremde Sperre zu entfernen. Normaler Abschluss und HUP/INT/TERM räumen
die eigene Sperre auf. Nach einem SIGKILL kann sie bis zum Neustart unter
`/var/run/rd05-wlan-apply.lock.d` bestehen bleiben.

Unterstützt werden zunächst **ein AP pro Radio**, feste Kanäle, HT20/HT40
und bei 5 GHz VHT20/VHT40/VHT80, offene Netze und WPA/WPA2-PSK mit
8–63 Zeichen langer Passphrase. WPA2-CCMP ist die empfohlene Einstellung.
Versteckte SSID und AP-Client-Isolation werden an hostapd übergeben.
STA, Mesh, WDS, PMF/WPA3, zusätzliche VAPs, automatische Kanalwahl und ein
manueller Sendeleistungswert werden ausdrücklich abgelehnt. Die LuCI-Oberfläche
kann weitere Optionen anbieten; der Adapter macht daraus keine zugesagte
Treiberfunktion. Weitere Spezialoptionen sind hier noch nicht angebunden.

## Bauen und prüfen

Nach `git pull --ff-only origin MW5_Stable` zunächst `make defconfig` ausführen,
damit die neuen Paketabhängigkeiten in die bestehende Build-Konfiguration
übernommen werden. Anschließend das RD05-Image neu bauen und flashen.
Der erste Boot migriert die Konfiguration automatisch.

In LuCI **Netzwerk → WLAN** öffnen und beide Radios prüfen. Falls nötig über
UART den Zustand abfragen:

```sh
wifi status
iw dev
logread | tail -60
tail -60 /tmp/mw5-webui/netifd-wlan.log
```

SSID oder Passwort ändern, speichern/anwenden, beide Bänder mit einem Client
verbinden und den lokalen Routerzugriff prüfen. Anschließend ein Radio
deaktivieren: das andere muss weiterlaufen. Nach erneutem Aktivieren und
Neustart müssen die Einstellungen erhalten bleiben, sofern ein persistentes
Overlay verfügbar ist. Der letzte Benutzerlog meldete ein tmpfs-Overlay;
damit gehen Änderungen beim Neustart verloren, unabhängig von LuCI.

Der LAN-/WAN-Fehler mit FCS-Fehlern am Switch-CPU-Port ist hiervon getrennt.
Diese Änderung behebt dessen elektrische beziehungsweise Frame-Verarbeitung
nicht. Auf echter Hardware sind die neue LuCI-/netifd-Anbindung und das
vollständige Firmware-Image noch zu prüfen; Hosttests ersetzen dies nicht.

## Autostartfehler auf `00eadaa10e`: INVALID_SSID

Der Benutzerlog zeigt beide Radios korrekt konfiguriert, aktiviert und mit
`autostart=true`, aber mit `retry_setup_failed=true`. Beide Starts scheitern
vor dem Laden des WLAN-Treibers mit `INVALID_SSID`. Es ist kein fehlender
Autostart-Eintrag: Die SSID-Prüfung lehnt gültige Namen ab.

Die Prüfung verwendete `tr -d '[:cntrl:]'`. OpenWrt deaktiviert standardmäßig
`BUSYBOX_CONFIG_FEATURE_TR_CLASSES`. Ohne diese Option interpretiert BusyBox
den Ausdruck als einzelne zu löschende Zeichen, einschließlich `n`, `t` und
`r`. Dadurch verändern sich auch die beiden Standard-SSIDs und werden als
ungültig bewertet. GNU-tr auf dem Entwicklungssystem zeigte diesen Fehler
in den bisherigen Hosttests nicht.

Die Korrektur verwendet stattdessen den grundlegenden oktalen Bytebereich
`tr -d '\000-\037\177'`. Sie benötigt keine Zeichenklassen-Unterstützung,
erhält normale SSIDs und weist Zeilenumbrüche, CR und weitere Steuerbytes
weiterhin zurück. Ein zusätzlicher Test mit BusyBox 1.36.1 ohne TR_CLASSES
reproduziert die alte Ablehnung und prüft die korrigierte Funktion unter ash.
Der Test lässt sich mit `RTL8197F_BUSYBOX_NO_TR_CLASSES=/pfad/zu/busybox`
zusätzlich zur normalen Hosttestsuite ausführen.
