# 
# Copyright (C) 2007-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libc-ares
PKG_VERSION:=1.15.0
PKG_RELEASE:=1

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=An asynchronous resolver library
endef

define Package/$(PKG_NAME)/description
 This is c-ares, an asynchronous resolver library.  It is intended for
 applications which need to perform DNS queries without blocking, or need to
 perform multiple DNS queries in parallel.  The primary examples of such
 applications are servers which communicate with multiple clients and programs
 with graphical user interfaces.
endef


TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS +=           \
	--disable-silent-rules  \
	--enable-debug          \
	--enable-optimize       \
	--enable-warnings       \
	--disable-werror        \
	--enable-curldebug      \
	--enable-dependency-tracking \
	--disable-code-coverage \
	--enable-nonblocking    
	
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef


define Build/Compile
	+$(MAKE) all $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CC="$(TARGET_CC)"
endef


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ares* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcares.{a,so*} $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libcares.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
