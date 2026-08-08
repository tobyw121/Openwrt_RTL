# RTL8197F v42.25 changelog

## MW5 loader/kernel overlap fixed

A complete v42.24 OpenWrt build produced a `0x900d5b`-byte kernel including
the appended DTB. Loaded at `0x80100000`, it ended at `0x80a00d5b` and
overwrote `0xd5b` bytes of the self-extracting loader linked at
`0x80a00000`. The decoder could still print `done!` from I-cache, but the
following cache invalidation exposed overwritten instructions and the OEM
bootcode reported `Undefined Exception happen.`

v42.25 keeps the OEM firmware header/staging address at `0x80a00000`, but
links the MW5 loader at `0x80d00000`. The startup relocation therefore copies
the complete payload forward by three megabytes before decoding. This distance
is larger than the complete loader/LZMA payload and keeps the decompressed
kernel approximately three megabytes below the executing loader.
A new build-time check reads the LZMA-alone uncompressed-size field and fails
the image build before linking if the requested safety margin is violated.

The v42.23 UART RX and physical-P0/RGMII fixes, the v42.22 SquashFS fix, and
the v42.21 exact SPI auto-map coordinates are retained.
