#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=hostapd
PKG_VERSION:=2.9
PKG_RELEASE:=1

# PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
# PKG_SOURCE_URL:=http://w1.fi/releases/
# PKG_MD5SUM:=F188FC53A495FE7AF3B6D77D3C31DEE8
# PKG_BUILD_PARALLEL:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/hostapd-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/json
TARGET_CFLAGS += -I../src
TARGET_CFLAGS += -I../src/utils
TARGET_CFLAGS += -DCONFIG_CONF_METHOD=1
ifneq ($(CONFIG_PACKAGE_network-roaming),)
	TARGET_CFLAGS += "-DTP_ROAMING_SUPPORT"
endif
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib 
TARGET_LDFLAGS += -L$(STAGING_DIR)/lib

define Package/hostapd
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=hostapd
  TITLE:=IEEE 802.1x Authenticator
  URL:=http://w1.fi/hostapd/
  DEPENDS:=+libnl
endef

define Package/hostapd/description
 This package contains a full featured IEEE 802.1x/WPA/EAP/RADIUS
 Authenticator.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/hostapd \
		CC="$(TARGET_CC)" \
		EXTRA_LDFLAGS="$(TARGET_LDFLAGS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/hostapd/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/hostapd_cli $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/hostapd/97G_AXMP/mp.sh $(1)/usr/sbin
	$(INSTALL_BIN) ./filesystem/etc/init.d/hostapd.init $(1)/etc/init.d/hostapd
endef

$(eval $(call BuildPackage,hostapd))

