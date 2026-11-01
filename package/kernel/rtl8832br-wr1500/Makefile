include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8832br-wr1500
PKG_VERSION:=2026.10.22
PKG_RELEASE:=4
PKG_LICENSE:=GPL-2.0-only
PKG_SOURCE:=GPL_ax1500v3.tar.gz
PKG_SOURCE_URL:=https://static.tp-link.com/upload/gpl-code/2026/202610/20261022/
PKG_HASH:=6d133598b134b6878ce802a53b3061d9f2efb3636ad0d31ec4cb3bcbdf438f2a
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=mac80211

include $(INCLUDE_DIR)/package.mk

WR1500_RFE50_SHA256_FILES := \
  820332547fe26b27b92492e99f8a42f62ff31938ddf47f33b6f1b7c9cbc65456:AP_5G/PHY_REG.txt \
  f50c63561e394216c447d49bc32cd52fd2889cf7991d9ad1c38a84bb6bb001f2:AP_5G/PHY_REG_GAIN.txt \
  54c4c30ce765826b1447d2fb955c80f5f68abfc74836afcfd22cf3fed700f3ea:AP_5G/RadioA.txt \
  54c4c30ce765826b1447d2fb955c80f5f68abfc74836afcfd22cf3fed700f3ea:AP_5G/RadioB.txt \
  636f5593c10879f4c68a6fd57d2e6de76d7d82bd6d2de1a4db4e572e40d53df6:AP_5G/TXPWR_ByRate.txt \
  28dc64f9d9453ac14652129a680e0b6ed8b94e11d0929150a5481424017e1830:AP_5G/TXPWR_LMT.txt \
  17716c458e18adecb2e85509eb5ad28e8ffbbcda7773031ee14464af426a47e2:AP_5G/TXPWR_LMT_RU.txt \
  b3238a9d0139c85c8a227aafa70c0eb52dcff4d191fe5e9abca8964b99ff9dd8:AP_5G/TXPWR_TrackTSSI.txt \
  7ac5d832e9542dd84e96b4cac88ea969598aa6219da85afdd73ae55fb0f89e0e:PHY_REG.txt \
  29a94695f044938778de739182d5ec7c1f3d5cb86b9682c79ae1e0d227b993cf:PHY_REG_GAIN.txt \
  3677ddb8a85a2eb714b7d697f704b4c26f4f8183a54e8c5ecc5a8d10d5757415:RadioA.txt \
  76f90cf32576d2df3aa5546cc1ed6083d7f234540df74cf39d0900ddfa6b7217:RadioB.txt \
  636f5593c10879f4c68a6fd57d2e6de76d7d82bd6d2de1a4db4e572e40d53df6:TXPWR_ByRate.txt \
  0335f1aef6c6b0d726f1070498ea592ecaa5b8f8b12edb95c0536275ea32cac6:TXPWR_LMT.txt \
  31134285c23d931de7389d5827f0d8d996add280440c40a2ac1949c783642ea9:TXPWR_LMT_RU.txt \
  8e5c10bfc228da5364796961ae599d3fa8cbe3aa61b21e25e3106dc4b1ea7e68:TXPWR_TrackTSSI.txt

define KernelPackage/rtl8832br-wr1500
  SUBMENU:=Wireless Drivers
  TITLE:=Cudy WR1500 R76 RTL8832BR 5GHz Wi-Fi 6 driver
  DEPENDS:=@TARGET_realtek_rtl8197f +kmod-cfg80211 +@DRIVER_11AC_SUPPORT +@DRIVER_11AX_SUPPORT +wifi-scripts +iw +wireless-regdb
  KCONFIG:=CONFIG_WIRELESS=y
  FILES:=$(PKG_BUILD_DIR)/rtk_wifi6.ko
  # Manual probe until IRQ registration and failed-probe teardown pass hardware tests.
endef

define KernelPackage/rtl8832br-wr1500/description
 Native Linux 6.6/OpenWrt cfg80211 port of the Realtek G6 RTL8832BR
 PCIe driver. Probe is restricted to Cudy WR1500 R76 and PCI 10ec:b832.
 Includes the observed OEM RFE50/AP_5G parameters; Linux bridge and
 netfilter own forwarding. Does not drive the integrated 2.4GHz radio.
endef

# Extract only the G6 subtree from the verified vendor GPL archive.
define Build/Prepare
	@set -eu; \
	  count=0; \
	  for item in $(WR1500_RFE50_SHA256_FILES); do \
	    expected=$${item%%:*}; relative=$${item#*:}; \
	    file="$(CURDIR)/files/RFE50/$$relative"; \
	    if [ ! -s "$$file" ]; then \
	      echo "WR1500 RFE50 ERROR: missing/empty active calibration: $$relative" >&2; \
	      echo "Restore package/kernel/rtl8832br-wr1500/files/RFE50 from this source tree." >&2; \
	      exit 1; \
	    fi; \
	    actual=$$(sha256sum "$$file"); actual=$${actual%% *}; \
	    if [ "$$actual" != "$$expected" ]; then \
	      echo "WR1500 RFE50 ERROR: wrong SHA256 for active calibration: $$relative" >&2; \
	      exit 1; \
	    fi; \
	    count=$$((count+1)); \
	  done; \
	  [ "$$count" -eq 16 ] || { echo "WR1500 RFE50 ERROR: expected 16 active entries" >&2; exit 1; }; \
	  echo "WR1500 RFE50: 16 active OEM calibration files verified (no external manifest needed)"
	$(RM) -r $(PKG_BUILD_DIR)
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	tar -xzf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR) --strip-components=8 \
		GPL_ax1500v3/rtl8198/linux-4.4.x/drivers/net/wireless/realtek/g6_wifi_driver
	$(SED) 's/\r//g' $(PKG_BUILD_DIR)/core/rtw_mib.c $(PKG_BUILD_DIR)/core/rtw_recv.c
	$(call Build/Patch)
endef

define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
		CONFIG_RTLWIFI6=m CONFIG_RTL8832BR=y CONFIG_SOC_RTL8197F=y \
		CONFIG_BR_EXT=n CONFIG_TXPWR_LIMIT=y CONFIG_TXPWR_LIMIT_EN=y \
		CONFIG_FILE_FWIMG=n CONFIG_EFUSE_CONFIG_FILE=n CONFIG_PROC_DEBUG=n CONFIG_APPEND_VENDOR_IE_ENABLE=n \
		USER_EXTRA_CFLAGS="-DCONFIG_RTL8832BR -DCONFIG_WLAN0_RFE_TYPE_50 -DCONFIG_WLAN0_AP_5G -Wno-error=address -Wno-error=implicit-fallthrough -Wno-error=enum-conversion -Wno-error=misleading-indentation" \
		NOSTDINC_FLAGS="$(KERNEL_NOSTDINC_FLAGS) \
		-I$(STAGING_DIR)/usr/include/mac80211-backport \
		-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
		-I$(STAGING_DIR)/usr/include/mac80211 \
		-I$(STAGING_DIR)/usr/include/mac80211/uapi \
		-include $(PKG_BUILD_DIR)/include/openwrt_g6.h -include backport/backport.h" \
		modules
endef

define KernelPackage/rtl8832br-wr1500/install
	$(INSTALL_DIR) $(1)/etc/conf/rtl8832bre/RFE50/AP_5G
	$(INSTALL_DATA) ./files/RFE50/*.txt $(1)/etc/conf/rtl8832bre/RFE50/
	$(INSTALL_DATA) ./files/RFE50/AP_5G/*.txt $(1)/etc/conf/rtl8832bre/RFE50/AP_5G/
endef

$(eval $(call KernelPackage,rtl8832br-wr1500))
