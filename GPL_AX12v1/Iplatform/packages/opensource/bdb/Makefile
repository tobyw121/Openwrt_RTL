#
# Copyright (C) 2005-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bdb
PKG_VERSION:=4.8.30
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

MAKE_PATH:=build_unix
CONFIGURE_PATH:=build_unix
CONFIGURE_CMD:=../dist/configure

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=
  TITLE:=Berkeley DB
  URL:=http://www.oracle.com/technology/software/products/berkeley-db/db/index.html
endef

define Package/$(PKG_NAME)/description
  This is Berkeley DB from Oracle.  To view release and installation documentation, 
  load the distribution file docs/index.html into your web browser.
endef

CONFIGURE_VARS += CFLAGS="-Os"

CONFIGURE_ARGS += \
	--prefix=$(PKG_INSTALL_DIR)/usr\
	--disable-cryptography \
	--disable-hash \
	--disable-queue \
	--disable-replication \
	--disable-statistics \
	--disable-verify \
	--disable-compat185 \
	--disable-cxx \
	--disable-diagnostic \
	--disable-dump185 \
	--disable-java \
	--disable-mingw \
	--disable-o_direct \
	--enable-posixmutexes \
	--disable-smallbuild \
	--disable-tcl \
	--disable-test \
	--disable-uimutexes \
	--enable-umrw \
	--enable-smallbuild \
	--disable-libtool-lock

TARGET_CFLAGS += $(FPIC)

#can't use default MAKE_FLAGS
MAKE_FLAGS = \
	CROSS="$(TARGET_CROSS)" \
	ARCH="$(ARCH)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
	$(call Build/Configure/Default)	
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/build_unix/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdb-4.8.so $(1)/usr/lib/libdb-4.8.so
ifneq (,$(findstring intel_grx350,$(BOARD)))
	echo "[bdb] intel will not strip it !!!"
else
	$(STRIP) $(1)/usr/lib/libdb-4.8.so
endif
	ln -sf libdb-4.8.so $(1)/usr/lib/libdb.so
	ln -sf libdb-4.8.so $(1)/usr/lib/libdb-4.so
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdb-4.8.a $(1)/usr/lib/libdb-4.8.a
endef
 
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdb-4.8.so $(1)/usr/lib/libdb-4.8.so
ifneq (,$(findstring intel_grx350,$(BOARD)))
	echo "[bdb] intel will not strip it !!!"
else
	$(STRIP) $(1)/usr/lib/libdb-4.8.so
endif
	ln -sf libdb-4.8.so $(1)/usr/lib/libdb.so
	ln -sf libdb-4.8.so $(1)/usr/lib/libdb-4.so
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
