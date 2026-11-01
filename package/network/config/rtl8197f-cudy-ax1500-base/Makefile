include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8197f-cudy-ax1500-base
PKG_RELEASE:=6
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/rtl8197f-cudy-ax1500-base
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Cudy AX1500 UART/SPI/PCI bring-up diagnostics
  DEPENDS:=@TARGET_realtek_rtl8197f
  DEFAULT:=y if TARGET_realtek_rtl8197f_DEVICE_cudy_ax1500_r76 || TARGET_DEVICE_realtek_rtl8197f_DEVICE_cudy_ax1500_r76 || TARGET_realtek_rtl8197f_DEVICE_cudy_re1500 || TARGET_DEVICE_realtek_rtl8197f_DEVICE_cudy_re1500 || TARGET_realtek_rtl8197f_DEVICE_cudy_m1500 || TARGET_DEVICE_realtek_rtl8197f_DEVICE_cudy_m1500
endef

define Package/rtl8197f-cudy-r76-base
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Cudy AX1500 WR1500 complete base package selection
  DEPENDS:=@TARGET_realtek_rtl8197f +rtl8197f-cudy-ax1500-base +ip-full +ip-bridge +ethtool +dnsmasq +odhcpd-ipv6only +firewall4 +dropbear +luci +uhttpd +uhttpd-mod-ubus +rpcd +kmod-cfg80211 +wifi-scripts +iw +iwinfo +rpcd-mod-iwinfo +wireless-regdb +hostapd-mini +tcpdump-mini +pciutils
  DEFAULT:=y if TARGET_realtek_rtl8197f_DEVICE_cudy_ax1500_r76 || TARGET_DEVICE_realtek_rtl8197f_DEVICE_cudy_ax1500_r76
endef

define Package/rtl8197f-cudy-r76-base/description
 Selects networking, SSH, firewall, LuCI, wireless userspace and diagnostics
 for WR1500/R76. Does not supply an unfinished WLAN driver or enable hardware.
endef

define Package/rtl8197f-cudy-ax1500-base/description
 Read-only diagnostics for experimental Cudy AX1500 RAM boot profiles.
 Reports PCI identity on the R76 SPI profile. Does not write flash or
 configure Ethernet, WLAN, LEDs or GPIOs.
endef

define Build/Compile
endef

define Package/rtl8197f-cudy-ax1500-base/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/cudy-ax1500-diag $(1)/usr/sbin/cudy-ax1500-diag
endef

$(eval $(call BuildPackage,rtl8197f-cudy-ax1500-base))

define Package/rtl8197f-cudy-r76-base/install
	$(INSTALL_DIR) $(1)/usr/share/cudy-r76
	echo 'Cudy R76 base packages; network hardware support is separate.' > $(1)/usr/share/cudy-r76/packages.info
	$(INSTALL_DATA) ./files/r76-hardware.json $(1)/usr/share/cudy-r76/hardware.json
endef

$(eval $(call BuildPackage,rtl8197f-cudy-r76-base))
