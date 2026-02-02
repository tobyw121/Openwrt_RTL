#
# Copyright (C) 2012-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=aria2
PKG_RELEASE:=1

PKG_CONFIG_DEPENDS := \
  ARIA2_NOSSL \
  ARIA2_OPENSSL \
  ARIA2_GNUTLS \
  ARIA2_BITTORRENT \
  ARIA2_METALINK

include $(INCLUDE_DIR)/package.mk

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

define Package/aria2/config
  source "$(SOURCE)/Config.in"
endef

define Package/aria2
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=lightweight download utility
  DEPENDS:=+zlib +ARIA2_METALINK:libxml2 +libstdcpp +ARIA2_OPENSSL:libopenssl +ARIA2_GNUTLS:libgnutls
endef

define Package/aria2/description
 aria2 is a lightweight multi-protocol & multi-source command-line download
 utility
endef

CONFIGURE_ARGS += \
	--disable-nls \
	$(if $(CONFIG_ARIA2_NOSSL),--disable,--enable)-ssl \
	$(if $(CONFIG_ARIA2_OPENSSL),--with,--without)-openssl \
	$(if $(CONFIG_ARIA2_GNUTLS),--with,--without)-gnutls \
	$(if $(CONFIG_ARIA2_BITTORRENT),--enable,--disable)-bittorrent \
	$(if $(CONFIG_ARIA2_METALINK),--enable,--disable)-metalink \
	$(if $(CONFIG_ARIA2_METALINK),--with,--without)-libxml2 \
	--without-libnettle \
	--without-libgmp \
	--without-libgcrypt \
	--without-libexpat \
	--without-libcares \
	--without-sqlite3 \
	--with-libz

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -u ./src/* $(PKG_BUILD_DIR)/
	# 以下文件存在依赖关系，原本压缩包中时间戳有先后，但源代码上传到git后时间戳都一致，会导致某些文件不必要的重新生成，需更新其时间戳以重建正确依赖关系
	cd $(PKG_BUILD_DIR) && touch aclocal.m4 Makefile.in config.h.in configure
	cd $(PKG_BUILD_DIR)/deps/wslay && touch aclocal.m4 Makefile.in config.h.in configure


endef

define Package/aria2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/aria2c $(1)/usr/bin/aria2c
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/etc/init.d/aria2 $(1)/etc/init.d/aria2
endef

$(eval $(call BuildPackage,aria2))
