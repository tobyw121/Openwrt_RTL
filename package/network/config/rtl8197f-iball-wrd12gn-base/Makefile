include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8197f-iball-wrd12gn-base
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/rtl8197f-iball-wrd12gn-base
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RTL8197F iBall WRD12GN board defaults
  DEPENDS:=@TARGET_realtek_rtl8197f
endef

define Package/rtl8197f-iball-wrd12gn-base/description
 Installs the safe accelerator and network defaults for the iBall WRD12GN.
endef

define Build/Compile
endef

define Package/rtl8197f-iball-wrd12gn-base/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/98-wrd12gn-flow-offload $(1)/etc/uci-defaults/98-wrd12gn-flow-offload
endef

$(eval $(call BuildPackage,rtl8197f-iball-wrd12gn-base))

