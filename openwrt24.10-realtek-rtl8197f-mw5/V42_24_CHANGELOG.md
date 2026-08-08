# RTL8197F v42.24 changelog

## Hardware result that triggered this release

The v42.23 direct image decompressed successfully but reset with the OEM
bootloader message `Undefined Exception happen.` before the loader printed
`Starting kernel at 80100000`.  This locates the failure in the loader
`flush_cache()` interval, before Linux executes its first instruction.

## Loader hardening

`lzma_outsize` is now reloaded through a volatile memory access after the old
SDK 4.40 decoder returns and before `flush_cache()`.  A compiler memory barrier
prevents the output size from remaining only in a decoder-lived register.
The direct v42.24 diagnostic image additionally fixes the exact aligned cache
end to `0x809fec80` for its verified 0x8fec87-byte kernel and uses a different
SDK-compatible LZMA stream (`nice=100`, `mf=bt3`).

## Retained v42.23 changes

- RTL8197FS RBR/THR byte alias at UART `+0x24`, with DLL retained at `+0x00`
  while DLAB is set.
- MW5 SoC DMA target P0 rather than external-switch port bit 6.
- RTL8197FS P0/RGMII TX0/RX5, non-VG pad configuration and CPU-tag pass-through.
- v42.22 MTD, SquashFS and OpenWrt userspace boot fixes.

## Validation status

Static source checks, patch application, SDK-4.40 decompression, checksums and
private full-flash preservation pass.  Physical v42.24 UART and LAN/WAN tests
remain required.
