# RTL8197F v42.15

## Exact RTL8197F watchdog disable

- Correct the erroneous `0xa5f00000` watchdog value introduced in v42.8.
- Match the Realtek RTL819x SDK driver exactly: read WDTCNR, set `WDT_CLEAR` bit 23 and write it back, then write the exact disable magic `0xa5000000`.
- Apply the sequence in the LZMA-loader entry, loader board initialization, generic MIPS kernel entry and RTL8197F `prom_init()`.
- Keep MW5 UART diagnostics on 8-bit LSR `0xb8147014` and TX alias `0xb8147024`.
- Add a direct v42.15 diagnostic image whose post-`P` markers are UART-only and never feed the watchdog, allowing physical verification that the watchdog remains disabled.

The direct binary is structurally validated but still requires the attached MW5 UART test.
