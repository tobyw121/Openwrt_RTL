#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=udpxy
PKG_VERSION:=1.0
PKG_RELEASE:=1
PKG_REV:=08bf9b8f378409e56df11a4dd796b2d309121ee5

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_PROTO:=git
#PKG_SOURCE_URL:=git://github.com/haibbo/udpxy.git
PKG_BRANCH:=OpenWRT/uClibc
PKG_SOURCE_VERSION:=$(PKG_REV)

include $(INCLUDE_DIR)/package.mk

define Package/udpxy
  SECTION:=net
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=Routing and Redirection
#  DEPENDS:=+libpthread
  TITLE:=UDP Proxy for IPTV
endef

define Package/udpxy/description
	Improxy is an IGMP/MLD Proxy daemon written in c for Linux. It is lightweight and do not depend on any thirdparty library.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/udpxy \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/udpxy
	$(CP) -u ./src/* $(PKG_BUILD_DIR)/udpxy
endef

define Package/udpxy/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/udpxy/udpxy $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/etc/init.d/udpxy.init $(1)/etc/init.d/udpxy
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_DATA) ./filesystem/etc/hotplug.d/iface/udpxy.hotplug $(1)/etc/hotplug.d/iface/50-udpxy
	$(INSTALL_DIR) $(1)/etc/hotplug.d/firewall
	$(INSTALL_DATA) ./filesystem/etc/hotplug.d/firewall/udpxy.hotplug $(1)/etc/hotplug.d/firewall/50-udpxy
endef

$(eval $(call BuildPackage,udpxy))
