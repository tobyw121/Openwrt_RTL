include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wireguard-tools
PKG_RELEASE:=1.0

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_RELEASE)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
	SECTION:=TP-LINK
	CATEGORY:=TP-LINK iplatform apps
	URL:=http://www.tp-link.com/ 
	DEPENDS:=+liblog 
	TITLE:=WireGuard-tools 
endef

define Package/$(PKG_NAME)/description
	Config tools for WireGuard VPN
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
	$(TARGET_CONFIGURE_OPTS) LDFLAGS="$(TARGET_LDFLAGS)" CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/$(PKG_NAME)/conffiles
/etc/config/wireguard
endef

#######
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) \
		$(1)/usr/bin \
		$(1)/etc/init.d \
		$(1)/etc/config \
		$(1)/usr/sbin \
		$(1)/lib/netifd \
		$(1)/lib/netifd/proto

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/wg $(1)/usr/bin/

	$(INSTALL_BIN) \
		filesystem/usr/sbin/wireguard_watchdog \
		$(1)/usr/sbin/wireguard_watchdog

	$(INSTALL_BIN) \
		filesystem/etc/init.d/wireguard \
		$(1)/etc/init.d/wireguard

	$(INSTALL_BIN) \
		filesystem/lib/netifd/proto/wireguard.sh \
		$(1)/lib/netifd/proto/wireguard.sh \
    
	$(INSTALL_DATA) ./filesystem/etc/config/wireguard $(1)/etc/config/wireguard
endef



$(eval $(call BuildPackage,$(PKG_NAME)))
