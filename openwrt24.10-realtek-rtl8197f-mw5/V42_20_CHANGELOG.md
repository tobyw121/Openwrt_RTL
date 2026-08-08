# RTL8197F v42.20

## MW5/AC23 MTD firmware split fix

The physical MW5 v42.19 boot reached the normal Linux console, registered the
8 MiB SPI auto-map device and created the seven fixed OEM partitions.  It did
not create the dynamic `kernel` and `rootfs` partitions, and the root mount
ended in the diagnostic panic marker `!`.

The RTL8197F firmware parser was already built in and enabled through
`CONFIG_MTD_SPLIT_RTL8197F_FW=y`.  Its `cr6c` calculation is correct:

```
rootfs_offset = sizeof(IMG_HEADER_T) + be32(header.length)
```

The actual blocker was the OpenWrt mtdsplit core.  For a partition named
`firmware`, `mtd_partition_split()` only starts firmware parsers when the DT
partition node has no `compatible` property.  The MW5 and AC23 DTS files used:

```
compatible = "realtek,rtl8197f-firmware";
```

That property therefore suppressed the parser it appeared to select.

v42.20 removes the property from both Tenda firmware nodes and keeps the
partition name `firmware`.  OpenWrt can now run all firmware parsers; the
RTL8197F parser recognizes `cr6c`, validates the SquashFS magic and creates:

- `kernel`: firmware-relative offset `0`, size `0x10 + header.length`
- `rootfs`: immediately after the Realtek kernel/checksum area

The dynamically created `rootfs` name also lets `CONFIG_MTD_ROOTFS_ROOT_DEV`
select it as the root block device.

## Direct MW5 diagnostic image

The supplied v42.20 direct image additionally patches the already-built
v42.19 kernel at the single `mtd_partition_split()` branch so the parser runs
with the old embedded DTB.  This is only needed for the direct test image; a
normal build from this tree uses the corrected DTS and does not require that
binary branch patch.

Expected new boot messages include:

```
rtl8197f-fw: firmware split kernel=0x... rootfs=0x...
2 rtl8197f-fw partitions found on MTD device firmware
... : "kernel"
... : "rootfs"
VFS: Mounted root (squashfs filesystem) readonly ...
```

## Preserved fixes

v42.20 retains the verified 16-byte MW5 Realtek header, proprietary rootfs
boot-length marker, SDK-compatible rootfs checksum, loader LZMA window check,
RTL8197F watchdog handling, RTL8197FS byte UART/TX alias and safe MW5 OEM
switch-preinit watchdog sequence.

## Validation limit

The source tree and direct test image were statically and structurally
validated.  A fresh complete OpenWrt world build and the physical v42.20 boot
remain required.  Ethernet traffic, DSA forwarding, WLAN RF operation and
sysupgrade are not claimed by this change.
