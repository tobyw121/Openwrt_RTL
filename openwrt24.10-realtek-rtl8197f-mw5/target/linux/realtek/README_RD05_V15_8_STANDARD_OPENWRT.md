# RD05 v15.8 standard OpenWrt boot cleanup

v15.8 is a source-tree cleanup layer on top of v15.7.  The intended runtime
behaviour is the same as a normal OpenWrt target:

- kernel starts `/sbin/init`, not an RD05 PID1 wrapper
- procd runs the normal `/etc/preinit` and `/etc/init.d/rcS` flow
- no RD05 preinit shell, no forced RAM overlay, no rc.local marker
- serial login is provided through the standard `::askconsole` in `/etc/inittab`

The only RD05-specific pieces that remain are hardware support:

- UART0 is left without an interrupt in DTS, forcing the Linux 8250 polling
  fallback that works reliably on this board
- RTL8197F CP0 timer/GIMR2 bit15 setup remains
- RTL8367D switch uses the OEM-preinitialised state and skips destructive reset
  and jam-table writes

If an installed image still prints `RD05 preinit v36`, `forcing tmpfs RAM
overlay`, or `RD05 rc.local`, it was built from stale local overlay files.  Run:

```sh
tools/rtl8197f-rd05/purge-stale-rd05-bringup.sh .
```

Then rebuild after deleting rootfs/staging output.
