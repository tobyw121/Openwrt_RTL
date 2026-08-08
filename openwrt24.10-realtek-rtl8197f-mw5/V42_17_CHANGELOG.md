# RTL8197F v42.17

## MW5 post-scheduler watchdog reset fix

Physical v42.16 UART diagnostics reached `cpu_startup_entry()` and created PID 1 and kthreadd before the OEM bootloader reported another watchdog timeout. Binary inspection found one later reachable WDTCNR write in `rtl8197f_oem_switch_preinit()`: after two one-second switch-reset delays it wrote `0xa5000000` directly.

The Realtek RTL8197F watchdog driver explicitly warns that a bare stop-pattern write can reset the SoC immediately. It requires a counter reload first. v42.17 therefore:

- adds `rtl8197f_stop_watchdog()` to the board-local switch preinit driver;
- reads WDTCNR, sets `WDT_CLEAR` bit 23, writes it back, then writes the exact `0xa5000000` stop pattern;
- performs MMIO readbacks after both writes;
- stops the watchdog immediately after mapping system registers, before the MW5 2 x 1 second switch-reset delays;
- repeats the safe sequence after GPIO reset/pinmux setup;
- removes the unsafe direct stop-pattern write.

A direct v42.17 diagnostic image adds `kernel_init`, `do_initcalls` and OEM switch-preinit UART markers.

## Remaining limitations

A full OpenWrt world build and physical v42.17 test are still required. Ethernet, DSA, WLAN RF calibration, sysupgrade and long-term stability are not claimed from static validation alone.
