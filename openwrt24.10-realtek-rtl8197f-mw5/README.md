# MW5 v43.10 current development note

For Tenda Nova MW5, v43.10 is documented in `MW5_V43.10_OEM_CDP_COMPLETION.md`
and `V43_10_CHANGELOG.md`. It retains the hardware-proven asymmetric CPU-tag
handling from v43.9 and changes the RTL8197F 24-byte RX/TX ring completion
policy to the OEM New_swNic CDP model. CPURPDCR0/CPUTPDCR0 are used as the
authoritative current-descriptor cursors, with OWN retained for RX runout and
fallback diagnostics. Standard OpenWrt firewall4 policy remains unchanged.

# OpenWrt RTL8197F RD05 / MW5 / AC23 v42.27 PRIVATE


**Critical v42.27 MW5 P0-to-CPU pipeline fix:** the v42.26 hardware log proves that LAN/WAN frames reach the RTL8363 user ports while `eth0` RX remains zero. v42.27 enables the missing RTL8197FS internal rtl865x PVID/VLAN/netif/L2/unknown-to-CPU/ACL seed for MW5, uses the live device MAC, and repeats it after SWCORE stabilizes. See `V42_27_CHANGELOG.md`.
**Critical v42.25 MW5 loader relocation:** a full world build exceeded the old `0x80100000..0x80a00000` decompression window by `0xd5b` bytes. The MW5 loader is now linked/headered at `0x80d00000`, and the image pipeline rejects future kernel/loader overlap before linking. See `V42_25_CHANGELOG.md`.
**Critical v42.26 MW5 CPU-DMA descriptor fix:** the non-VG RTL8197FS SDK uses 6-dword/24-byte new descriptors. v42.25 incorrectly used the RTL8197F-VG 8-dword/32-byte cache-aligned layout, while hardware readback retained zero stride fields. v42.26 selects 24 bytes for MW5, retains 32 bytes for VG/RD05, and rearms CPUICR1 plus both ring bases after CPUIF start. See `V42_26_CHANGELOG.md`.
**Critical MW5 loader/console/network correction:** v42.24 hardens the in-place LZMA cache flush and completes the RTL8197FS UART quirk by routing both RBR and THR through the byte alias at `+0x24` while preserving DLL at `+0x00` under DLAB. It also follows the Realtek GPL SDK topology for RTL8197F + external RTL8367: the SoC DMA targets physical P0, and P0 is configured as the 1 Gbit/s RGMII CPU link with RTL8197FS TX0/RX5 timing and CPU-tag pass-through. See `V42_24_CHANGELOG.md`.

v42.24 retains the hardware-proven v42.22 firmware split, SPI auto-map and SquashFS/OEM-checksum fixes.

![OpenWrt logo](include/logo.png)

OpenWrt Project is a Linux operating system targeting embedded devices. Instead
of trying to create a single, static firmware, OpenWrt provides a fully
writable filesystem with package management. This frees you from the
application selection and configuration provided by the vendor and allows you
to customize the device through the use of packages to suit any application.
For developers, OpenWrt is the framework to build an application without having
to build a complete firmware around it; for users this means the ability for
full customization, to use the device in ways never envisioned.

Sunshine!

## Download

Built firmware images are available for many architectures and come with a
package selection to be used as WiFi home router. To quickly find a factory
image usable to migrate from a vendor stock firmware to OpenWrt, try the
*Firmware Selector*.

* [OpenWrt Firmware Selector](https://firmware-selector.openwrt.org/)

If your device is supported, please follow the **Info** link to see install
instructions or consult the support resources listed below.

## 

An advanced user may require additional or specific package. (Toolchain, SDK, ...) For everything else than simple firmware download, try the wiki download page:

* [OpenWrt Wiki Download](https://openwrt.org/downloads)

## Development

To build your own firmware you need a GNU/Linux, BSD or macOS system (case
sensitive filesystem required). Cygwin is unsupported because of the lack of a
case sensitive file system.

### Requirements

You need the following tools to compile OpenWrt, the package names vary between
distributions. A complete list with distribution specific packages is found in
the [Build System Setup](https://openwrt.org/docs/guide-developer/build-system/install-buildsystem)
documentation.

```
binutils bzip2 diff find flex gawk gcc-6+ getopt grep install libc-dev libz-dev
make4.1+ perl python3.7+ rsync subversion unzip which
```

### Quickstart

1. Run `./scripts/feeds update -a` to obtain all the latest package definitions
   defined in feeds.conf / feeds.conf.default

2. Run `./scripts/feeds install -a` to install symlinks for all obtained
   packages into package/feeds/

3. Run `make menuconfig` to select your preferred configuration for the
   toolchain, target system & firmware packages.

4. Run `make` to build your firmware. This will download all sources, build the
   cross-compile toolchain and then cross-compile the GNU/Linux kernel & all chosen
   applications for your target system.

### Related Repositories

The main repository uses multiple sub-repositories to manage packages of
different categories. All packages are installed via the OpenWrt package
manager called `opkg`. If you're looking to develop the web interface or port
packages to OpenWrt, please find the fitting repository below.

* [LuCI Web Interface](https://github.com/openwrt/luci): Modern and modular
  interface to control the device via a web browser.

* [OpenWrt Packages](https://github.com/openwrt/packages): Community repository
  of ported packages.

* [OpenWrt Routing](https://github.com/openwrt/routing): Packages specifically
  focused on (mesh) routing.

* [OpenWrt Video](https://github.com/openwrt/video): Packages specifically
  focused on display servers and clients (Xorg and Wayland).

## Support Information

For a list of supported devices see the [OpenWrt Hardware Database](https://openwrt.org/supported_devices)

### Documentation

* [Quick Start Guide](https://openwrt.org/docs/guide-quick-start/start)
* [User Guide](https://openwrt.org/docs/guide-user/start)
* [Developer Documentation](https://openwrt.org/docs/guide-developer/start)
* [Technical Reference](https://openwrt.org/docs/techref/start)

### Support Community

* [Forum](https://forum.openwrt.org): For usage, projects, discussions and hardware advise.
* [Support Chat](https://webchat.oftc.net/#openwrt): Channel `#openwrt` on **oftc.net**.

### Developer Community

* [Bug Reports](https://bugs.openwrt.org): Report bugs in OpenWrt
* [Dev Mailing List](https://lists.openwrt.org/mailman/listinfo/openwrt-devel): Send patches
* [Dev Chat](https://webchat.oftc.net/#openwrt-devel): Channel `#openwrt-devel` on **oftc.net**.

## License

OpenWrt is licensed under GPL-2.0

## RTL8197F development integration

The RD05, Tenda Nova MW5 and Tenda AC23 development port in this tree is
documented in [`README_RTL8197F_RD05_MW5_AC23_V41.md`](README_RTL8197F_RD05_MW5_AC23_V41.md).

## RTL8197F v41.2 private full-flash build

See [`README_FULLFLASH_V41_2_PRIVATE.md`](README_FULLFLASH_V41_2_PRIVATE.md).
The v41.2 build helpers bootstrap OpenWrt host tools and the MIPS cross-toolchain
before entering `target/linux/compile`.

## v42.5 MW5 flash budget

The MW5 OEM KernelFS partition is fixed at 0x590000 bytes. v42.5 does not
borrow space from CFM, CFM_BACKUP, LOG or ENV. The profile uses 64 KiB image
alignment, 1 MiB SquashFS blocks and a reduced immutable package set while
retaining IPv4 DHCP/firewall and both WLAN paths. `rtl8197f-firmware-budget.py`
now fails immediately with raw, padded, limit and overrun sizes instead of
allowing generic `check-size` to delete the intermediate image.
