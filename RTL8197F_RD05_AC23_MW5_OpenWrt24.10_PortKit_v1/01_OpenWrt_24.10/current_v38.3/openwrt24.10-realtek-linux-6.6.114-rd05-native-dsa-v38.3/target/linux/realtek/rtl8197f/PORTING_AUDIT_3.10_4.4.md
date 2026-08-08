# RTL8197F 3.10/4.4 SDK audit for the Linux 6.6 target

The archive contains multiple mutually incompatible vendor trees.  The active
port uses the MR62X Linux-4.4 BSP/rtknet sources only as hardware references
and implements their behavior in normal Linux 6.6 subsystems.

## Mapped behavior

| SDK area | Native Linux 6.6 destination |
|---|---|
| BSP machine/prom/setup | MIPS platform patches and Device Tree |
| BSP IRQ | irqchip/Device Tree routing |
| BSP GPIO | gpiolib driver and pinctrl state |
| SHEIPA SPI | native SPI/MTD driver with bounded writes |
| BSP PCIe | generic PCI host bridge driver |
| `rtl819x_swNic.c` | native netdev CPU-DMA rings |
| `rtl865x_asicL2.c` P0 setup | RTL8197F Ethernet master setup |
| `rtl83xx_v1dot4` RTL8367D DAL | Linux DSA, phylink, bridge, VLAN and FDB |
| private FastPath/NAT | not imported |
| private ioctl/proc DAL | not imported |
| `rtl8192cd` WLAN | inventoried only; not a native 6.6 driver |

The authoritative source paths and hashes are in the tree-root
`RD05_SDK_SOURCE_MAP.tsv`.
