# RTL8197F OpenWrt 24.10 v42.4

## MW5 image-size and Full-SPI correction

The first complete v42.3 MW5 build proved that the WLAN module now links and
that the remaining failure is the fixed 0x590000-byte OEM KernelFS budget.
The combined image before generic padding was 0x5f8112 bytes; OpenWrt then
used its fallback 4/8/16/64/128/256 KiB padding sequence and rounded it to
0x600000, after which generic check-size deleted the file.

v42.4 changes the image path without extending the firmware partition:

- MW5 and AC23 explicitly use their measured 64 KiB SPI erase size.
- MW5 SquashFS uses 1024 KiB blocks for stronger XZ compression.
- The MW5 immutable rootfs keeps dnsmasq, firewall4, wireless-tools and the
  board-specific rtl8192cd module.
- IPv6 daemons, ip-full/ip-bridge, ethtool, uboot-envtools, procd-ujail and
  the HTTPS fetch stack are omitted from the 8 MiB factory image. They remain
  available as separately built packages.
- A strict pre-padding size gate predicts the final erase-aligned size and
  aborts without deleting the image.
- The OEM CFM, CFM_BACKUP, LOG and ENV ranges remain outside the writable
  firmware window and are preserved byte-for-byte by the fullflash builder.

No firmware partition enlargement or private-area overwrite is used.
