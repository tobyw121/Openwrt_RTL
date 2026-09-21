# openwrt-feed_RTL8197f audit / v44.60

The supplied `openwrt-feed_RTL8197f.tar.zst` is an older RTL8197F/RTL8197FS
OpenWrt/Realtek SDK feed (Linux 4.14-era target `rtkmipsel`).  It was compared
path-by-path against the v44.59 Linux 6.6 driver tree before v44.60 was built.

## Coverage

- Feed files total: 1046
- Feed `rtl8192cd` files: 784
- Same relative `rtl8192cd` path already present in v44.59: 773
- Byte-identical common files: 487
- Common paths with later/current modifications: 286
- Feed-only `rtl8192cd` files: 11
- Current Linux-6.6 `rtl8192cd` files not present in the old feed: 1789

This means the current tree is not missing a wholesale vendor WLAN driver; it
already contains nearly the entire old driver plus the Linux-6.6/cfg80211/MW5
porting work.

## Safely integrated

1. Missing RTL8197F country power-limit source tables:
   - `WlanHAL/Data/8197F/Egypt/TXPWR_LMT_8197Fmp_Type0.txt`
   - `WlanHAL/Data/8197F/Iran/TXPWR_LMT_8197Fmp_Type0.txt`
   - `WlanHAL/Data/8197F/Moldova/TXPWR_LMT_8197Fmp_Type0.txt`
2. The feed's RTL8197FS bonding mapping was cross-checked against the current
   code.  Both map bond code `0xA` to `BSP_BOND_97FS = 3`; the current code
   already contains that logic, so no duplicate runtime change was required.
3. The legacy netifd/WEXT `rtl8192cd.sh` is retained as
   `rtl8192cd-legacy-netifd-wext.sh` for vendor MIB semantics (WPA2/CCMP, channel width, AP opmode), not
   installed into the firmware.
4. RTL8197FS/RTL83xx port-mask and bonding snippets are retained here as source
   references for future switch diagnostics.

## Deliberately not linked into Linux 6.6

- Linux 4.14 Realtek DMA patches.
- Old `rtl865x` fastpath/HWNAT implementation.
- Old proprietary network stack replacement.
- RTL8812F Tenda/D-Link power tables: the MW5 physical 5-GHz radio is
  RTL8812BRH and the vendor driver identifies it through the RTL8822B HAL path,
  so 8812F calibration tables are not interchangeable.
- `8192cd_common.h` and D-Link-specific regulatory headers that are unused by
  the current MW5 build.

Those components are useful for register/behavior comparison but transplanting
them would change DMA ownership, skb lifetime, DSA semantics or calibration and
would be higher risk than the existing Linux-6.6 port.
