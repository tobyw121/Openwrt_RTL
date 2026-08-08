# Realtek target driver status

## RD05 / RTL8197F

- platform, IRQ, GPIO, timer, UART, reset/watchdog: native
- SHEIPA SPI and protected SPI-NOR overlay: native
- PCIe host: native, compile-checked, hardware enumeration pending
- RTL8197F CPU-DMA Ethernet: native, CPURPDCR0 semantics added in v35
- RTL8367D: native Linux DSA variant
- RTL8367D user PHYs: phylib polling; EEE disabled during stabilization
- external RTL8812FE and integrated WLAN: identified, vendor source inventoried,
  native mac80211 implementation not claimed

## Other Realtek subtargets

The retained rtl838x, rtl839x, rtl930x and rtl931x sources remain separate
OpenWrt DSA/platform support.  The RD05 port does not route through those SoCs;
only shared Realtek infrastructure is retained.
