#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=netatalk
PKG_VERSION:=3.1.8
#PKG_RELEASE:=5

#PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)
#PKG_SOURCE_URL:=@SF/netatalk
#PKG_MD5SUM:=4a371025daf96adb73fa491efdbfef28

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
#PKG_FIXUP:=autoreconf
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/netatalk
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Filesystem
  DEPENDS:=+libgcrypt +libopenssl +bdb
  TITLE:=netatalk
  URL:=http://netatalk.sourceforge.net
  MAINTAINER:=W. Michael Petullo <mike@flyn.org>
endef

define Package/netatalk/decription
  Netatalk is a freely-available Open Source AFP fileserver.
  It also provides a kernel level implementation of the AppleTalk
  Protocol Suite.
endef

TARGET_CFLAGS += -std=gnu99
ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"10.3")
	TARGET_CFLAGS += -fcommon
endif
#TARGET_LDFLAGS += $(LIBRPC)

#CONFIGURE_VARS += ac_cv_path_LIBGCRYPT_CONFIG="$(STAGING_DIR)/usr/bin"

CONFIGURE_ARGS += \
	--disable-debugging \
	--with-cnid-default-backend=dbd \
	--with-bdb="$(STAGING_DIR)/usr" \
	--with-ssl-dir="$(STAGING_DIR)/usr" \
	--with-uams-path="/usr/lib/uams" \
	--without-ldap \
	--without-dtrace \
	--without-kerberos \
	--without-mysql-config

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/netatalk/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib/uams
	$(INSTALL_DIR) $(1)/etc/netatalk
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/afppasswd $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/afpd $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/cnid_dbd $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/cnid_metad $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/uams/*.so $(1)/usr/lib/uams/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libatalk.so.17.0.0 $(1)/usr/lib/
	ln -sf libatalk.so.17.0.0 $(1)/usr/lib/libatalk.so.17
	$(INSTALL_BIN) ./filesystem/etc/init.d/afpd.init $(1)/etc/init.d/afpd
endef

$(eval $(call BuildPackage,netatalk))
