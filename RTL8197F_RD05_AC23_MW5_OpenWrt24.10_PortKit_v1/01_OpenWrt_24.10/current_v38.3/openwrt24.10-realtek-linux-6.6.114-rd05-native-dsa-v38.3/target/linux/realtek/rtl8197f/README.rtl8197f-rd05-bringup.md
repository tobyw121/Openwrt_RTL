# RTL8197F/RD05 bring-up notes

This subtarget is a conservative bring-up base for the Xiaomi R4/RD05 class of
RTL8197F-VG boards.

Implemented in this tree:

- `rtl8197f.dtsi` with RTL8197F physical MMIO ranges instead of the unrelated
  RTL838x switch-SoC layout.
- GPIO controller driver for pins A0..H7, numbered 0..63, with the Realtek BSP
  edge IRQ encoding.
- SHEIPA SPI-MEM controller driver for SPI-NOR probing and read access.
  Write/erase operations are disabled unless the SPI controller node explicitly
  contains `realtek,allow-writes`.
- `scripts/rtl8197f-image.py`, a non-secure `cs6c`/IMG_HEADER_T wrapper matching
  the Realtek boot-code checksum convention.
- RD05 image recipe output `kernel-cs6c.bin` for bootmiwifi/header experiments.
- Native rtl865x CPU-interface Ethernet driver (`CONFIG_RTL8197F_RTKNET`) for
  hardware RX/TX validation through Linux `net_device` + NAPI.
- The RD05 DTS now enables the external RTL8367RB switch node for the candidate
  H0/G7 SMI pins, no switch-reset GPIO until hardware proves the reset line, and rtl865x external-port TX mask (`0x40`).
- The RD05 SPI backup (`xiaomi_r4-rd05_spi_Backup.bin`) confirms the U-Boot
  `model=RD05` environment and the `ethaddr`/`ethaddr_wan` variables in the
  NVRAM/Bdata areas, so userspace reads those variables for LAN/WAN MACs.

Safety constraints:

- Do not flash the generated images without serial console and an external SPI
  recovery path.
- `kernel-cs6c.bin` only adds the Realtek/Xiaomi header. The bootloader still
  expects an executable payload at `startAddr`; stock images include a Realtek
  decompressor/loader payload. If the bootloader jumps directly to the payload,
  wrapping a normal OpenWrt `uImage` is not sufficient for final flash boot.
- WLAN and vendor NAT/fastpath offload are intentionally not enabled by this
  patch set.  Ethernet is native bring-up code and still requires hardware
  validation before it should be treated as production-ready.

First useful tests:

1. Build an initramfs image and test via UART/TFTP if the bootloader supports it.
2. Confirm UART output at 115200 8N1.
3. Confirm SPI-NOR auto-map or JEDEC probing and MTD partition discovery.
4. Bring up `eth0` and check rtl865x RX/TX counters with a directly connected
   peer or with a validated RTL8367RB DSA CPU port.
5. Only after SPI read-only probing works should write/erase be validated.

## RD05 userspace fast path

The RD05 target overlay now keeps boot moving to a serial console even while the
SPI AutoMap MTD path is read-only:

- `lib/preinit/81_rd05_force_ramoverlay` forces a tmpfs overlay instead of
  trying to finalize JFFS2 on the read-only flash map.
- `etc/rc.d/S00rd05-console` arms an emergency ttyS0 shell before the generic
  `S00sysfixtime` stage, so a blocking rcS script no longer leaves the board
  headless.
- `etc/init.d/boot` creates minimal RAM-backed `system`, `network` and
  `dropbear` configs, loads boot modules, and skips the slow/unsafe
  `config_generate`/WLAN/reload path during bring-up.
- `etc/init.d/done` marks `/tmp/rd05-login-ready`, stops the emergency shell
  watchdog and prints the final “press Enter for ttyS0 OpenWrt login” marker.
- `etc/inittab` uses `usr/libexec/rd05-login.sh` for ttyS0/console login so the
  serial log shows an explicit RD05 OpenWrt login marker before the normal
  OpenWrt root shell hand-off.

On a successful serial boot, the kernel command line must contain
`init=/etc/preinit PREINIT=1`; otherwise Linux starts `/sbin/init` directly and
OpenWrt skips the preinit/RAM-overlay path.  Because `/etc/preinit` is PID 1 in
this mode, the RD05 wrapper must `exec /sbin/init` after the preinit hooks; if it
returns normally, Linux panics with `Attempted to kill init`.  With the corrected
command line and hand-off, the expected RD05 markers are `RD05 preinit v41`,
`RD05 sysinit v40`, `RD05 boot v39`, `RD05 done v40`, followed by
procd's normal `askfirst` login on `ttyS0`. If the serial log stops after
OpenWrt's `/etc/modules-boot.d` kmodloader messages and the sysinit wrapper is
not reached, the preinit hand-off watchdog prints `RD05 preinit v41: procd
handoff watchdog running RD05 fast path`, reruns the RD05 fast-path boot/done
sequence, and starts a ttyS0 rescue shell so the board remains interactive.

## Loader and kmodloader status

- The RD05 flash kernel path uses OpenWrt's raw `rtl8197f-loader-kernel`
  wrapper, not the legacy vendor `target/linux/realtek/rtkload` build.  This is
  intentional for native Linux 6.6 bring-up: `rtkload` is kept as reference BSP
  material, while `scripts/rtl8197f-image.py` provides the Realtek `cs6c` header
  and checksum around the raw self-extracting LZMA payload.
- The LZMA loader is linked at the RD05 `bootmiwifi` start address
  `0x80cf0000`, decompresses the appended kernel to `0x80100000`, and now
  forwards the board `RTL8197F_KERNEL_CMDLINE` into the loader source build.
- The serial loader banner should read
  `OpenWrt kernel loader for Realtek RTL8197F/RTL8197FH`; if the old `AR7XXX/AR9XXX` banner is still visible, the flashed kernel payload was built from an older loader.
- `etc/inittab` now starts `usr/libexec/rd05-sysinit.sh`, which prints a post-`init: Console is alive` marker, bypasses the generic `rcS` sequence, runs only the RD05 fast-path `boot`/`done` scripts, and arms a ttyS0 rescue shell before the fast path can block. `/etc/preinit` also arms a delayed hand-off watchdog before `exec /sbin/init`, covering the observed case where procd reaches the generic kmodloader stage but never runs the RD05 sysinit wrapper.
- The same RD05 `inittab`, `boot`, `done`, login and sysinit wrappers are mirrored into `package/base-files/files` as a fallback, so images built from the generic base-files package still print the RD05 markers even if a target overlay is not applied or an incremental build reuses base-files.
- `etc/rc.d/S10boot` and `etc/rc.d/S95done` remain target-overlay symlinks to
  the RD05 fast-path init scripts, but the sysinit wrapper also calls those
  scripts directly so boot cannot stop at the generic OpenWrt
  `/etc/modules-boot.d` kmodloader stage on this read-only AutoMap bring-up.
- `etc/init.d/boot` runs `/sbin/kmodloader`, logs whether it completed or
  returned an error, applies the backup-derived MAC variables when present, and
  still continues to the serial login marker so a bad boot module cannot hide
  the console during RTL8197F/RTL8197FH bring-up.
- Static checks for this path are collected in
  `tools/rtl8197f-rd05/check-v39-loaders.sh`.
