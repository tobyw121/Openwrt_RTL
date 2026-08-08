# Flashlayouts

## Xiaomi RD05 – 16 MiB

| Offset | Ende | Name | Größe |
|---:|---:|---|---:|
| 0x000000 | 0x020000 | boot | 128 KiB |
| 0x020000 | 0x030000 | nvram | 64 KiB |
| 0x030000 | 0x040000 | bdata | 64 KiB |
| 0x040000 | 0x050000 | factory | 64 KiB |
| 0x050000 | 0x060000 | crash | 64 KiB |
| 0x060000 | 0x350000 | kernel | 3008 KiB |
| 0x350000 | 0xe60000 | rootfs | 11328 KiB |
| 0xe60000 | 0x1000000 | overlay | 1664 KiB |

## Tenda MW5 und AC23 – 8 MiB

- Bootloader/Factory bis ungefähr `0x30000`.
- Realtek-Firmware beginnt bei `0x30000` mit `cr6c`.
- Header enthält Signatur, Load-Adresse, Burn-Adresse und Kernel-Länge.
- Kernel ist LZMA-komprimiert; Rootfs folgt unmittelbar.
- AC23 verwendet ein Tenda-modifiziertes SquashFS/XZ-Magic (`nice`/`Tenda`) statt Verschlüsselung.
