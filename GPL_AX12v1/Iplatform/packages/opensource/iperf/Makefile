#
# Copyright (c) 2014 The Linux Foundation. All rights reserved.
# Copyright (C) 2010-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iperf
PKG_VERSION:=2.0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/iperf
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=iperf
endef

define Package/iperf/description
	This is Iperf v2.0.0, a tool for measuring Internet bandwidth performance.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/iperf
	$(CP) -u ./src/* $(PKG_BUILD_DIR)/iperf
endef

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		./iperf/configure --host=arm-linux\
	)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"
endef

define Package/iperf/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/iperf $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,iperf))
