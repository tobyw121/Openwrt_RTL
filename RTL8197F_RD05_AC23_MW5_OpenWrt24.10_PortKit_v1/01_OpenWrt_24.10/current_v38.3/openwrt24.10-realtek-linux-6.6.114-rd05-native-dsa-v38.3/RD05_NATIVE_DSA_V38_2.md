# RD05 Native DSA v38.2

Recovery-oriented correction for the v38.1 early boot stop after MTD registration.

- keeps UART0/serial0 at 0x18147000 in polling-console mode;
- removes the unverified RTL8197F second interrupt-bank patch from the RD05 build;
- leaves UART1/UART2 disabled and without IRQ mappings;
- defers RTL8197F P0/RGMII register writes from probe to ndo_open, as in the boot-proven v37 sequence;
- adds bootstage markers around rtknet resource and IRQ mapping.

Ethernet/DSA functionality still requires an on-device v38.2 test.
