# OpenWrt

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

## MW5 v43.19 setup WebUI

This tree adds a small MW5-specific WebUI without full LuCI. It is served by
`uhttpd` at `http://192.168.1.1/` and provides:

* wired status and diagnostics,
* safe port mode switching with rollback,
* prepared WLAN AP settings for the staged rtl8192cd driver,
* prepared Mesh/Tenda Nova pairing settings.

Important commands:

```sh
mw5-portmode show
mw5-portmode apply router|setup|ap|swap
mw5-wifi status
mw5-netdiag
```

The proven Ethernet default from v43.16 remains unchanged: `tx_layout=2`.

## MW5 v43.19 SSH/WebUI access note

v43.19 keeps the v43.16 raw-untagged Ethernet default and adds a first-boot access fix:
Dropbear and uhttpd are included/enabled, both physical jacks are put into setup LAN mode for initial access, and `/usr/sbin/mw5-access-status` can be used from serial to verify TCP/22 and TCP/80 listeners.

SSH requires setting a root password first:

```sh
passwd
ssh root@192.168.1.1
```

WebUI:

```text
http://192.168.1.1/
```

## MW5 v43.19 WebUI/Port/WLAN notes

v43.19 keeps the proven raw-untagged Ethernet path and improves the minimal WebUI:

* `mw5-portmode force setup` keeps both physical jacks in `br-lan` on `192.168.1.1`.
* `mw5-portmode force router` returns to LAN+WAN routing mode.
* `mw5-wifi apply` now uses a stored/default WPA test key when the WebUI password field is left blank.
* Mesh/WDS source support is enabled in the experimental rtl8192cd build path, but Tenda-Nova pairing still needs two MW5 nodes for validation.
