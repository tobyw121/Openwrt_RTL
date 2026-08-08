RD05 native DSA v38.3 hotfix
============================

Target: unmodified v38.2 OpenWrt tree.

Apply:
  chmod +x apply-openwrt-rd05-native-dsa-v38.3.sh
  ./apply-openwrt-rd05-native-dsa-v38.3.sh /path/to/openwrt

The files/ directory mirrors every added or changed path.
The standalone rd05-rgmii-calibrate-v38.3 can be copied to a running test
system for temporary calibration testing.

Before any further test on a current v38.2 runtime that attempted tx-auto or
rx-auto, restore the SDK baseline:
  rd05-rgmii-calibrate sdk
