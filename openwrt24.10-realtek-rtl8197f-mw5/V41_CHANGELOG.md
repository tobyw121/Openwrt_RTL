# v41.0 changelog

## Clean-base reconstruction

- Rebuilt from the supplied original OpenWrt source archive.
- Removed all dependence on previously packaged host binaries and generated state.
- Preserved original mbedTLS, Dropbear and odhcpd package generations to prevent mixed patch series.
- Preserved the original toolchain and host-tool recipes.

## RTL8197F target

- Added `rtl8197f` Realtek subtarget and restored the target-root `ARCH:=mips` declaration required by OpenWrt target scanning.
- Kept the subtarget `ARCH:=mipsel`, CPU `24kc` and Linux 6.6.114 pin.
- Added RD05, MW5 and AC23 device/image profiles.
- Added four tested configuration seeds.
- Enabled per-device root filesystems for the multi-profile seed.

## Drivers and kernel patches

- Integrated RTL8197F GPIO, SPI-ROM, SPI, PCIe, USB, Ethernet and diagnostics work.
- Integrated RTL8367D and known RTL8367/RTL8363-family DSA extensions.
- Included corrected patch 328 context after patch 327.
- Included the corrected RTL8197F MTD splitter source include order.
- Retained the experimental MW5 rtl8192cd package and guarded duplicate `NETDEV_NO_PRIV` defines.

## Image safety

- Normal builds produce no personalized full-SPI image.
- Full-flash output is enabled only with `RTL8197F_BUILD_FULLFLASH=1` and a matching verified template.
- Removed RD05-specific target-wide static network/DHCP files that could contaminate other Realtek devices.

## Validation

- All config seeds pass target/profile selection.
- Linux 6.6.114 target prepare and all patches pass without rejects.
- No generated host/build/feed state is shipped.
