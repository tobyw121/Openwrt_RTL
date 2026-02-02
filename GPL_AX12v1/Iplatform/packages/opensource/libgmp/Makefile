#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libgmp
PKG_VERSION:=6.1.2
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/libgmp/Default/description
 Openswan is an IPsec implementation for Linux.
endef

define Package/$(PKG_NAME)
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=libgmp
  URL:=http://www.openswan.org/
endef

define Package/$(PKG_NAME)/description
GMP is a free library for arbitrary precision arithmetic, 
operating on signed integers, rational numbers, and floating-point numbers. 
There is no practical limit to the precision except the ones implied by the 
available memory in the machine GMP runs on. GMP has a rich set of functions, 
and the functions have a regular interface. 
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Configure
$(call Build/Configure/Default)
endef

define Build/Compile
    $(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS) CFLAGS+=-fPIC
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/gmp*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/.libs/libgmp.{a,so*} $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/.libs/libgmp.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,libgmp))
