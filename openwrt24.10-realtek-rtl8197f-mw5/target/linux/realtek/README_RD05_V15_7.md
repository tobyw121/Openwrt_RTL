# RD05 v15.7/v15.8 standard target boot

This tree uses the normal OpenWrt boot chain for Xiaomi R4/RD05:

```text
kernel -> /sbin/init -> procd -> /etc/preinit -> rcS -> askconsole login
```

No RD05 PID1 wrapper, preinit rescue shell, forced RAM overlay, or rc.local
marker is part of the runtime image.  Use
`tools/rtl8197f-rd05/purge-stale-rd05-bringup.sh .` before rebuilding a tree
that previously contained bring-up overlays.
