# RTL8197F v42.23 changelog

## Hardware evidence from v42.22

- Linux mounts the corrected SquashFS root, runs `/sbin/init`, reaches `procd`
  and presents the OpenWrt console.
- Pressing Enter produces a continuous stream of NUL (`^@`) characters instead
  of an interactive shell.
- DSA and the external RTL8363/RTL8367-class switch register and report physical
  links, but LAN/WAN carry no usable traffic.

## RTL8197FS UART receive correction

The vendor SDK defines the non-VG RTL8197F/RTL8197FS UART0 data aliases as:

- RBR: base + `0x24`
- THR: base + `0x24`
- DLL: base + `0x00`
- LSR: base + `0x14`

v42.18 redirected only THR.  RX therefore read DLL at offset zero and returned
NUL bytes.  v42.23 adds `realtek,rx-alias-offset`, redirects UART_RX only while
DLAB is clear and preserves divisor-latch accesses at the standard location.

## MW5 external-switch CPU path correction

The RTL8363/RTL8367 switch-side CPU port 6 is not RTL8197F internal descriptor
port 6.  The Realtek GPL SDK `CONFIG_RTL_8197F + CONFIG_RTL_8367R_SUPPORT` path
uses physical RTL8197F P0/RGMII as the SoC endpoint.

MW5 now uses:

- descriptor destination `P0` (`tx-port-mask = 0x1`)
- no `DP_EXT`
- forced P0 1 Gbit/s/full-duplex RGMII
- RTL8197FS non-VG RGMII timing TX0/RX5
- non-VG pad drive value 6
- P0 CPU-tag recognition/transmission enabled
- MAC tag removal disabled so Linux DSA receives the complete tag
- no RTL8197FH-VG-only fixed-IPG or pad-mode bits

The external Realtek DSA driver remains responsible for LAN/WAN forwarding,
VLANs, FDB and rtl8_4 tagging.

## Validation status

Static source checks, Linux 6.6.114 patch dry-run, old Realtek LZMA 4.40 decode,
firmware checksums, deterministic image rebuild and protected full-flash range
comparison pass.  Interactive UART RX and actual LAN/WAN traffic still require
physical v42.23 testing.
