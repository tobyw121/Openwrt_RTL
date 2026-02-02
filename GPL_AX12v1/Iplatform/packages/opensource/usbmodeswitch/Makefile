include $(TOPDIR)/rules.mk

PKG_NAME:=usb-modeswitch
PKG_VERSION:=2.2.1


PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.draisberghof.de/usb_modeswitch
PKG_MD5SUM:=46cd7fa937655ab2df616c1d5dfb121e

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)


include $(INCLUDE_DIR)/package.mk

define Download/data
  FILE:=$(PKG_SOURCE)
  URL:=$(PKG_SOURCE_URL)
  MD5SUM:=46cd7fa937655ab2df616c1d5dfb121e
endef
$(eval $(call Download,data))


define Package/usb-modeswitch
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  DEPENDS:=+libubox +libblobmsg-json +libusb-1.0
  TITLE:=USB mode switching utility(community)
endef


define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) \
		TARGET_CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/usb-modeswitch/install
	$(CP) ./filesystem/* $(1)/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/usb_modeswitch $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,usb-modeswitch))
