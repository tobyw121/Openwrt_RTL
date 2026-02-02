
include $(TOPDIR)/rules.mk

PKG_NAME:=modeswitch-data
PKG_VERSION:=1.0

include $(INCLUDE_DIR)/package.mk

define Package/modeswitch-data
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=3G/4G Support
  DEPENDS:=
  TITLE:=mode-switch-data for usb_modeswitch
endef

define Package/modeswitch-data/Description
	This package contains the data needed to switch a modem.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -u ./src/* $(PKG_BUILD_DIR)/
endef

define Package/modeswitch-data/install
	$(INSTALL_DIR) $(1)/etc/modeswitch_data
	$(CP) ./usb_modeswitch.d/* $(1)/etc/modeswitch_data/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
