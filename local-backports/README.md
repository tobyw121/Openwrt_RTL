# Local build-fix backports

`rtl8192cd-linux6-buildfix-v42.1.patch` contains only the five source-level
corrections derived from the supplied Linux 6.6.114 compiler log. From the root
of a compatible tree, apply with:

```sh
patch -p1 < local-backports/rtl8192cd-linux6-buildfix-v42.1.patch
```

The complete v42.1 tree already includes these changes; do not apply it twice.
