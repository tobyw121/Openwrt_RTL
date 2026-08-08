# RD05 native DSA v38.1 UART/SPI boot-safety correction

This tree is v38 plus two board-specific boot-safety changes:

1. Xiaomi RD05 UART0 remains the `serial0` block at physical `0x18147000`,
   115200 8N1, but its board DTS removes the unverified interrupt property and
   uses the reliable 8250 polling path for the recovery console.
2. The RTL8197F SPI-ROM driver no longer performs WRDI/WREN/RDSR calibration
   during early platform probe.  MTD is registered from the stable automatic
   read window first.  Non-destructive calibration runs lazily on the first
   bounded `rootfs_data` erase or page-program operation.

The observed stop after `RTL8197F SPI auto-map fixed` is therefore treated as an
SPI-controller early user-mode lockup, not as a wrong UART base or stdout-path.
