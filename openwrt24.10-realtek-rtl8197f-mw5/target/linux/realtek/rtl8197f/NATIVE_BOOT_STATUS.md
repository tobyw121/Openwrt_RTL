# RTL8197F/RTL8197FH native boot status

Short answer: this tree can build a native OpenWrt 24.10/Linux 6.6.114
RTL8197F subtarget and contains native drivers for the core bring-up blocks, but
it is not yet a proven, fully complete RTL8197F/RTL8197FH + rtl865x + RTL8367RB
production port.

## What is native in this tree

- MIPS little-endian RTL8197F subtarget and boot path.
- UART0 console through the ns16550-compatible node in `rtl8197f.dtsi`.
- GPIO controller driver (`CONFIG_GPIO_RTL8197F`).
- SHEIPA SPI controller driver (`CONFIG_SPI_RTL8197F_SHEIPA`) and a conservative
  SPI auto-map MTD path (`CONFIG_MTD_RTL8197F_SPIROM`) for boards where active
  JEDEC probing is unsafe during bring-up.
- Native rtl865x CPU-interface Ethernet driver (`CONFIG_RTL8197F_RTKNET`) using
  `net_device` + NAPI rather than the legacy Realtek SDK fastpath/swconfig
  stack.  The v0.6 native driver exposes ethtool register dumps, ring sizing,
  IRQ/NAPI counters, descriptor error counters, DMA allocation counters, TX
  timeout recovery and RX runout restart counters for hardware bring-up.
- Linux DSA Realtek support for RTL8367-family compatibles, including
  `realtek,rtl8367rb`, through the in-kernel Realtek SMI/MDIO drivers.
- `rtl8197fh.dtsi`, a wrapper for RTL8197FH board DTS files that uses the same
  RTL8197F register map while exposing FH-specific compatible strings with F
  fallbacks.

## What is still board-validation work

- A real RTL8367RB board DTS must set the correct SMI MDC/MDIO GPIOs, reset GPIO
  and CPU port (usually EXT1/port 6 or EXT2/port 7 depending on the board).
- The rtl865x descriptor address format, destination-port mask and interrupt
  behaviour must be validated on hardware for each RTL8197F/RTL8197FH board.
- The RD05 DTS contains an enabled candidate RTL8367RB SMI attachment for H0/G7,
  but the current boot log still shows an SMI ACK timeout, so the pins/reset
  line/CPU port remain board-validation work.
- WLAN and vendor NAT/fastpath offload are not part of the native boot path.

## Expected answer for "complete native boot with rtl865x and RTL8367RB?"

No, not as a hardware-proven complete port yet.  The tree is now structured to
build and boot natively far enough to test UART, GPIO, flash and the native
rtl865x Ethernet path, and it has the DSA plumbing needed for RTL8367RB.  To
claim complete support, a board-specific DTS must enable RTL8367RB with verified
pins/CPU port and the native rtl865x driver must pass real packet RX/TX tests on
that hardware.  The Xiaomi RD05 userspace path is optimized to reach the serial
OpenWrt console first, using a RAM overlay and an emergency ttyS0 fallback while
flash write/erase and switch pins are still under validation.
