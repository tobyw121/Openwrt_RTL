# RTL8197F v42.18

## MW5 runtime console and root-mount diagnostics

The physical v42.17 boot reached PID 1, completed `do_initcalls()` and
`wait_for_initramfs()`, then reset before `kernel_init_freeable()` returned.
The last direct marker stream was:

```
IFDW1723uwEdc
```

This places the remaining failure in `console_on_rootfs()`, `/init` probing or
`prepare_namespace()`/root mounting. The earlier direct diagnostic UART worked
because it used the RTL8197FS byte aliases, while the regular DW-8250 console
still used the common 32-bit register description and standard TX offset zero.

v42.18 adds:

- a DW-8250 `realtek,tx-alias-offset` quirk;
- byte-wide MW5 UART I/O with the proven TX alias at `UART0 + 0x24`;
- DLAB protection so divisor-latch writes at offset zero are never redirected;
- a direct diagnostic image that patches `serial8250_console_putchar()` to poll
  byte LSR `0xb8147014` and transmit at byte alias `0xb8147024`;
- focused markers through `/dev/console`, `/init`, device-probe completion,
  `mount_root()`, `/dev/root`, squashfs/jffs2 attempts, root move/chroot and the
  final panic path.

The direct image retains the v42.17 safe OEM switch-preinit watchdog sequence.
It does not claim that the root filesystem mounts successfully; the physical
UART result is still required.

## Validation scope

The exact Linux 6.6.114 patch applies cleanly, the source checker validates the
MW5 DTS and DLAB-safe alias logic, and the direct image passes the old LZMA SDK
4.40 decoder, Realtek checksums and full-flash preservation checks.

A complete OpenWrt world build and physical v42.18 boot remain required.
Ethernet, DSA, WLAN RF, sysupgrade and long-term stability are not claimed.
