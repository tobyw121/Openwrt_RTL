# RTL8197F v42.5 changelog

## MW5 silent boot fix

The MW5 Realtek boot code uses a 16-byte `IMG_HEADER_T`. It copies the image
from flash offset `firmware + 0x10` to `startAddr` and jumps directly to that
address. v42.4 reproduced the stock image's 40-byte payload prefix by emitting
a 56-byte header. That shifted the OpenWrt LZMA loader, which is linked at
`0x80a00000`, to runtime address `0x80a00028` and caused a silent hang after
the bootloader recovery wait.

v42.5 changes MW5 to:

- `RTL8197F_HEADER_SIZE := 16`
- no `RTL8197F_LEN_INCLUDES_HEADER_PAD`
- executable loader bytes begin immediately at firmware offset `0x10`
- rootfs remains located at `0x10 + IMG_HEADER_T.len`

The AC23 60-byte image format is unchanged.

## Regression protection

The MW5 fullflash builder now rejects an OpenWrt firmware payload containing
40 zero bytes immediately after the logical header. The private stock template
is still accepted because it is parsed only as source material, not executed as
the newly generated OpenWrt payload.
