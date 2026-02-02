#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gd
PKG_VERSION:=2.1.1
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_MD5SUM:=BE0A6D326CD8567E736FBC75DF0A5C45

PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/gd
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=A lightweight C library that eases the writing of UNIX daemons
  URL:=http://0pointer.de/lennart/projects/libdaemon/
  DEPENDS:=+zlib +png +jpeg +tiff
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

#define Build/Configure
#	(cd $(PKG_BUILD_DIR); \
#		./configure CFLAGS="-I/home/tp/bcm4908_30.0/IPlatform/platform/staging_dir/target-arm-openwrt-linux-uclibc/usr/include \
#		-I/home/tp/bcm4908_30.0/IPlatform/platform/staging_dir/target-arm-openwrt-linux-uclibc/include \
#		-I/home/tp/bcm4908_30.0/IPlatform/platform/../../boards/model_brcm_bcm490x/toolchain/opt/toolchains/crosstools-arm-gcc-5.3-linux-4.1-glibc-2.22-binutils-2.25/usr/include ")
#		 CROSS="arm-buildroot-linux-gnueabi-" ARCH="arm64"
#endef
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgd.{a,so*} $(1)/usr/lib/
endef

define Package/gd/install
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgd.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,gd))