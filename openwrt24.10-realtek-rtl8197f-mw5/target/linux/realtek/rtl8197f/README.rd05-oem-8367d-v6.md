# Xiaomi RD05 / RTL8197FH-VG / RTL8367D OEM-aligned v6 notes

This tree incorporates the data collected from the stock Xiaomi RD05 firmware
shell and boot log:

* SoC: RTL8197FH-VG
* External switch: OEM reports `switchChip = 8367D`
* OEM switch-management platform devices:
  * `rtl819x_8367r_i2c_pin.1` -> exported GPIO55
  * `rtl819x_8367r_i2c_pin.2` -> exported GPIO56
  * `rtl819x_8367r_reset_pin.0` -> exported GPIO58
* Reset is deasserted high in the OEM runtime (`gpio58 value=1`), so it is
  treated as active-low in the DTS and diagnostics.
* OEM port mapping:
  * port 0 / mask `0x1`  -> LAN, OEM `eth0`, PVID 9
  * port 1 / mask `0x2`  -> LAN, OEM `eth1`, PVID 9
  * port 2 / mask `0x4`  -> WAN, OEM `eth2`, PVID 8
  * port 3 / mask `0x8`  -> LAN, OEM `eth3`, PVID 9
  * port 4 / mask `0x10` -> LAN, OEM `eth4`, PVID 9
* OEM CPU/extension side appears in the rtl865x tables as extension/port 8,
  represented by mask bit `0x20000`.  This is not the same as blindly using a
  normal DSA CPU port 7.
* Flash partitioning follows the OEM layout:
  * rootfs: `0x00350000-0x00e60000`
  * OpenWrt `rootfs_data`: `0x00e60000-0x01000000` (OEM calls this `overlay`)

The DSA `realtek-smi` switch node is deliberately left `disabled` in v6.  The
node documents the discovered hardware facts, but upstream `rtl8365mb` does not
currently model the RD05's OEM 8367D + rtl865x extension-port topology well
enough to claim a production switch configuration.

The built-in `rtl8197f_ethdiag` driver is now v36 and only probes the OEM pins:

* SMI candidate 1: MDC GPIO56, MDIO GPIO55
* SMI candidate 2: MDC GPIO55, MDIO GPIO56
* reset candidate: GPIO58 active-low
* bounded rtl865x internal MDCIO reads before and after the GPIO58 reset pulse

Useful boot-log filter:

```sh
dmesg | grep -i -E 'v36|8367D|OEM reset|GPIO-SMI|MDIO level|internal MDCIO|CHIP_NUMBER|CHIP_VER|extension'
```
