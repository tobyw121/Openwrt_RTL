# MW5 cfg80211 bring-up checklist

1. Confirm `rtl8197f_soc` and `rtl8822b_pci` each probe once and bind to radio
   1/2.4 GHz and radio 0/5 GHz respectively.
2. Check dmesg for `board-data` and `board data applied`; verify RFE 5/6 and
   MAC base+1/base+4.
3. Check `/sys/kernel/debug/rtl8192cd/<device>/` for `radio_index`, `rfe_type`,
   `mac_from_nvmem`, `calibration_len` and `factory_len`.
4. `iw phy` must show two wiphys/radios and no setup step may require `iwpriv`.
5. Start and stop an AP through `mw5-wifi apply` / hostapd-nl80211 on each band; exercise channel,
   SSID, WPA2/CCMP key install and station removal.
6. Run sustained RX/TX while checking for NAPI stalls, RX interrupt storms,
   descriptor starvation and DMA warnings.
7. On 5 GHz, validate channel switch and DFS/CAC for the regulatory domain in
   use.
8. Repeat interface up/down, module remove/reload and reboot to catch lifetime,
   IRQ and workqueue teardown errors.
