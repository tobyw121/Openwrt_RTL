# Auswahl und Herkunft

## Primärquellen

- `8197_all_SDKs.tar.zst`: umfangreichster RTL8367D- und neuerer RTL8197F/WLAN-Bestand.
- `entpackt(1).7z`: zusätzliche vollständige historische SDKs, Bootcode, Boardprofile und Toolchainrezepte.
- OpenWrt-24.10-Arbeitsstände v38.3 und Familienstand v35 aus der Dateibibliothek.
- Private Geräteabbilder und OEM-Laufzeitdaten wurden nur analysiert und bereinigt zusammengefasst.

## Auswahlregeln

Enthalten wurden vollständige relevante Quellunterbäume, damit Abhängigkeiten zwischen `.c`, `.h`, Registermaps, DAL-Mappern, Firmwaretabellen und Builddateien erhalten bleiben. Entfernt wurden erzeugte Objekte (`*.o`, `*.ko`, `*.a`, `*.so`), Versionskontrollmetadaten und vollständige binäre Cross-Toolchains. Firmware-, RF-, AGC-, Tx-Power- und Microcode-Blobs wurden beibehalten.

## Besonderheiten

- Für RTL8363NB existiert im Material kein eigenständiger vollständig benannter Treiberbaum. Relevante gemeinsame SMI-, RTL83xx- und RTL8367-Familienpfade sind enthalten; MW5 benötigt zusätzliche Hardwareverifikation.
- Die RTL8367D-DAL-Initialisierungsfunktion ist in mindestens einem Vendorzweig leer. Eine vollständige Kaltstart-/Jam-Table muss weiterhin aus Bootloader, Laufzeitregisterdumps oder weiteren SDK-Ständen rekonstruiert werden.
- WLAN-Code ist umfangreich, aber für Linux 6.6 nicht direkt buildfähig. Er dient als Register-, Firmware-, PHYDM-, HALMAC- und Kalibrierungsreferenz.

## Entfernte Sicherheitsdaten

Zusätzlich wurden generische bzw. mitkopierte `passwd`, `shadow`, `smbpasswd` und lokale OpenWrt-Buildsignaturschlüssel entfernt. Diese Dateien sind für die Treiberportierung nicht erforderlich.
