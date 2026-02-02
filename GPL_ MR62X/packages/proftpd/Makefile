#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=proftpd
PKG_VERSION:=1.3.4b
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=ftp://ftp.proftpd.org/distrib/source
PKG_MD5SUM:=0871e0b93c9c3c88ca950b6d9a04aed2

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/proftpd
#  SUBMENU:=File Transfer
  SECTION:=net
#  CATEGORY:=Network
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=USB Storage Sharing
  TITLE:=ProFTPD FTP server
  URL:=http://www.proftpd.org/
  MAINTAINER:=Cezary Jackiewicz <cezary@eko.one.pl>
  DEPENDS:=+libubox +libubus $(ICONV_DEPENDS) $(INTL_DEPENDS) +@BUILD_NLS +librt +liblog
endef

define Package/proftpd/conffiles
/etc/proftpd/proftpd.conf
endef

define Package/proftpd/config
source "$(SOURCE)/Config.in"
		config PACKAGE_$(PKG_NAME)_UGLY_LOG
		bool "Enable ugly log"
		default y
endef

TARGET_CPPFLAGS += \
	-I$(LINUX_DIR)/include \
	-I$(STAGING_DIR)/usr/include \
	-I$(ICONV_PREFIX)/include \
	-I$(INTL_PREFIX)/include

TARGET_LDFLAGS += \
	-L$(ICONV_PREFIX)/lib \
	-L$(INTL_PREFIX)/lib \
	-lubox -lubus -liconv -lintl -llog

MAKE_FLAGS += \
	INSTALL_USER=$(shell id -u) \
	INSTALL_GROUP=$(shell id -g)

CONFIGURE_ARGS += \
	--sysconfdir=/etc/proftpd \
	--disable-cap \
	--enable-devel \
	--enable-nls \
	--disable-ipv6 \
	ac_cv_header_curses_h=no \
	ac_cv_header_ncurses_h=no

ifeq ($(CONFIG_proftpd-in-fit), y)
	CONFIGURE_ARGS += --enable-fit \
	--disable-curses \
	--disable-ncurses \
	--disable-trace	
endif

ifeq ($(CONFIG_PACKAGE_$(PKG_NAME)_UGLY_LOG),y)
	TARGET_CPPFLAGS += -DCONFIG_UGLY_LOG=1
endif

ifeq ($(CONFIG_PACKAGE_proftpd_mod_sftp), y)
	CONFIGURE_ARGS += --enable-openssl --enable-ctrls\
	--with-modules=mod_sftp:mod_ban \
	--with-includes=/path/to/libsodium/include \
   	--with-libraries=/path/to/libsodium/lib
endif

ifeq ($(CONFIG_proftpd-limit-guest-ubus), y)
	CONFIGURE_ARGS += --enable-guest-limit
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/proftpd/install
	$(INSTALL_DIR) $(1)/etc $(1)/etc/proftpd
	$(if $(CONFIG_IPV6),,$(SED) 's/^UseIPv6/#UseIPv6/' $(PKG_INSTALL_DIR)/etc/proftpd/proftpd.conf)
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/proftpd/proftpd.conf $(1)/etc/proftpd/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/proftpd.init $(1)/etc/init.d/proftpd
	$(INSTALL_DATA) ./filesystem/proftpd.conf.orig $(1)/etc/proftpd/
	$(INSTALL_DATA) ./filesystem/sftp.conf.orig $(1)/etc/proftpd/
ifeq ($(CONFIG_PACKAGE_proftpd_mod_sftp), y)
	$(INSTALL_DATA) ./filesystem/limit_login.conf $(1)/etc/proftpd/
	$(INSTALL_DATA) ./filesystem/sftp.conf $(1)/etc/proftpd/
	$(INSTALL_DATA) ./filesystem/limit_dir.conf $(1)/etc/proftpd/
endif
	# $(INSTALL_DATA) ./filesystem/ftpd.passwd $(1)/etc/proftpd/
	# $(INSTALL_DATA) ./filesystem/ftpd.group  $(1)/etc/proftpd/
	$(INSTALL_DATA) ./filesystem/alias.conf $(1)/etc/proftpd/
	$(INSTALL_DIR) $(1)/usr/bin
	[ "$(CONFIG_proftpd-in-fit)" == "y" ] || \
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ftp{count,top,who,asswd} $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{ftpshut,proftpd} $(1)/usr/sbin/
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/in.proftpd $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,proftpd))
