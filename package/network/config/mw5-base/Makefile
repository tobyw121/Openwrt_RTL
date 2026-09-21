include $(TOPDIR)/rules.mk

PKG_NAME:=mw5-base
PKG_RELEASE:=6
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/mw5-base
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Tenda Nova MW5 network-core/Auto-WAN + volatile SSH baseline
  DEPENDS:=@TARGET_realtek_rtl8197f +dropbear +dnsmasq +firewall4
endef

define Package/mw5-base/description
 Installs the hardware-proven MW5 Ethernet/Auto-WAN baseline directly in squashfs.
 Also schedules the v44.31 volatile-hostkey LAN SSH repair service and the
 deterministic network-core, Auto-WAN, MW5 WiFi and management boot services.
endef

define Build/Compile
endef

define Package/mw5-base/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/etc/config/network $(1)/etc/config/network
	# v44.62.14: hostapd-mini always loads hostapd.uc, which imports
	# /usr/share/hostap/common.uc. The full generic wifi-scripts package stays
	# omitted on the MW5 for flash headroom; install only this required module.
	$(INSTALL_DIR) $(1)/usr/share/hostap
	$(INSTALL_DATA) ./files/usr/share/hostap/common.uc $(1)/usr/share/hostap/common.uc
	# MW5 has no writable overlay, so first-boot "enable" cannot reliably
	# create these rc.d links. Bake the board-only network services into
	# squashfs instead; their targets are supplied by realtek base-files.
	$(INSTALL_DIR) $(1)/etc/rc.d
	$(LN) ../init.d/mw5-network-core $(1)/etc/rc.d/S95mw5-network-core
	$(LN) ../init.d/mw5-webui $(1)/etc/rc.d/S96mw5-webui
	$(LN) ../init.d/mw5-auto-wan $(1)/etc/rc.d/S97mw5-auto-wan
	$(LN) ../init.d/mw5-wifi $(1)/etc/rc.d/S98mw5-wifi
	$(LN) ../init.d/mw5-access $(1)/etc/rc.d/S99mw5-access
endef

$(eval $(call BuildPackage,mw5-base))
