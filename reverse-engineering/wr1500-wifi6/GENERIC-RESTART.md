# SoC-based restart and selective Gesamtpatch integration

The watchdog/software-reset handler now registers whenever the DT exposes
`realtek,rtl8197f-syscon`, using that node's mapped register resource. No board
name is needed. This covers the supported RTL8197F-family DT interface; it is
not evidence that unrelated RTL8197D/G register layouts are identical. New
SoCs require a verified compatible register interface before using this path.
RD05/MW5 switch GPIO initialization remains board-specific and unchanged.
Handler registration now survives an external-switch GPIO mapping failure.

The Gesamtpatch's R76-only DMA-burst property is available as an optional
experiment. The DTS is deliberately not changed to 64 bytes: absence retains
128 bytes. This is not an Ethernet corruption fix. Existing IRQ/RFE50 changes
are preserved, including IRQ255 rejection and exact request_irq errors.
Unverified 2.4GHz HAL activation and duplicate reference archives are omitted.
WLAN failed-probe timer cleanup and eFuse/RF-path issues remain unresolved.

Validation: restart source compiled as built-in MIPS Linux6.6.114 object;
production init function exercised with R76, an unknown future board, RD05,
MW5 and absent SoC node. Hardware restart still needs UART confirmation. A standalone Ethernet-object
compile against plain Linux6.6 headers is blocked by the pre-existing
OpenWrt flow_rule_has_control_flags backport dependency; no full image build
is claimed.

PC build:

```sh
cd ~/Downloads/openwrt/github/Openwrt_RTL
git pull --ff-only origin MW5_Stable
cp configs/rtl8197f_ax1500_defconfig .config
make defconfig
make target/linux/clean
make -j"$(nproc)" V=s
```

Router commands, individually over UART38400 on WR1500:

```sh
dmesg | grep 'rtl8197f-restart'
```

```sh
sync
```

```sh
reboot
```

Expected registration followed by assertion message and bootloader output.
Repeat three times if successful. Use the existing image installation method;
this change does not add flash writes or persistent overlay support.
