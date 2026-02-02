#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsodium
PKG_VERSION:=1.0.18
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libsodium
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libpthread
  TITLE:=The Sodium crypto library
endef

define Package/libsodium/description
  Sodium is a new, easy-to-use software library for encryption, decryption, signatures, password hashing and more.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared

define Package/libsodium/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/libsodium/.libs/libsodium.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsodium))
