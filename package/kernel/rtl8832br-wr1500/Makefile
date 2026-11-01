include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8832br-wr1500
PKG_VERSION:=2026.10.22
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0-only
PKG_SOURCE:=GPL_ax1500v3.tar.gz
PKG_SOURCE_URL:=https://static.tp-link.com/upload/gpl-code/2026/202610/20261022/
PKG_HASH:=6d133598b134b6878ce802a53b3061d9f2efb3636ad0d31ec4cb3bcbdf438f2a
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=mac80211

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8832br-wr1500
  SUBMENU:=Wireless Drivers
  TITLE:=Cudy WR1500 R76 RTL8832BR 5GHz Wi-Fi 6 driver
  DEPENDS:=@TARGET_realtek_rtl8197f +kmod-cfg80211 +@DRIVER_11AC_SUPPORT +@DRIVER_11AX_SUPPORT +wifi-scripts +iw +wireless-regdb
  KCONFIG:=CONFIG_WIRELESS=y
  FILES:=$(PKG_BUILD_DIR)/rtk_wifi6.ko
  AUTOLOAD:=$(call AutoLoad,60,rtk_wifi6)
endef

define KernelPackage/rtl8832br-wr1500/description
 Native Linux 6.6/OpenWrt cfg80211 port of the Realtek G6 RTL8832BR
 PCIe driver. Probe is restricted to Cudy WR1500 R76 and PCI 10ec:b832.
 Includes the observed OEM RFE50/AP_5G parameters; Linux bridge and
 netfilter own forwarding. Does not drive the integrated 2.4GHz radio.
endef

# Extract only the G6 subtree from the verified vendor GPL archive.
define Build/Prepare
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
