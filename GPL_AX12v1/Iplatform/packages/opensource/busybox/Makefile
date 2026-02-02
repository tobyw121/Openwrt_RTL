# 
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=busybox
PKG_VERSION:=1.19.4
PKG_RELEASE:=6
PKG_FLAGS:=essential

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.busybox.net/downloads \
		http://distfiles.gentoo.org/distfiles/
PKG_MD5SUM:=9c0cae5a0379228e7b55e5b29528df8e

PKG_BUILD_DEPENDS:=BUSYBOX_CONFIG_FEATURE_HAVE_RPC:librpc
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) grep '^CONFIG_BUSYBOX_' $(TOPDIR)/.config | md5s)
endif

ifeq ($(CONFIG_PACKAGE_lp5523ctrl),y)
	LP5523_SUPPORT="y"
endif

ifeq ($(CONFIG_PACKAGE_wireless-schedule),y)
	WIRELESS_SCHEDULE_SUPPORT="y"
endif

ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  BB_MAKE_VERBOSE := V=1
else
  BB_MAKE_VERBOSE :=
endif

init-y :=
init-$(CONFIG_BUSYBOX_CONFIG_HTTPD) += httpd
init-$(CONFIG_BUSYBOX_CONFIG_CROND) += cron
init-$(CONFIG_BUSYBOX_CONFIG_TELNETD) += telnet

define Package/busybox
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  MAINTAINER:=Nicolas Thill <nico@openwrt.org>
  TITLE:=Core utilities for embedded Linux
  URL:=http://busybox.net/
  DEPENDS:=+BUSYBOX_CONFIG_FEATURE_HAVE_RPC:librpc +common +PACKAGE_$(PKG_NAME)_UGLY_LOG:liblog
  MENU:=1
endef

define Package/busybox/description
 The Swiss Army Knife of embedded Linux.
 It slices, it dices, it makes Julian Fries.
endef

define Package/busybox/config
	source "$(SOURCE)/Config.in"

	config PACKAGE_$(PKG_NAME)_UGLY_LOG
		bool "Enable ugly log"
		default y
endef

define Build/Configure
	rm -f $(PKG_BUILD_DIR)/.configured*
	grep 'CONFIG_BUSYBOX_' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_BUSYBOX_\\(.*\\),\\1\\2,g" > $(PKG_BUILD_DIR)/.config
	yes '' | $(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		ARCH="$(ARCH)" \
		$(BB_MAKE_VERBOSE) \
		oldconfig
endef

ifdef CONFIG_GCC_VERSION_LLVM
  TARGET_CFLAGS += -fnested-functions
endif

LDLIBS:=m crypt
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
export LDFLAGS=$(TARGET_LDFLAGS)
ifdef CONFIG_BUSYBOX_CONFIG_FEATURE_HAVE_RPC
  LDLIBS += rpc
endif
ifneq ($(CONFIG_PACKAGE_$(PKG_NAME)_UGLY_LOG),)
     TARGET_CFLAGS += -DCONFIG_UGLY_LOG=1
     LDLIBS += log
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		KBUILD_HAVE_NLS=no \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		SKIP_STRIP=y \
		LDLIBS="$(LDLIBS)" \
		$(BB_MAKE_VERBOSE) \
		LP5523_SUPPORT="$(LP5523_SUPPORT)" \
		WIRELESS_SCHEDULE_SUPPORT="$(WIRELESS_SCHEDULE_SUPPORT)" \
		BUSYBOX_DHCP_OPTION66="$(CONFIG_BUSYBOX_DHCP_OPTION66)" \
		all
	rm -rf $(PKG_INSTALL_DIR)
	$(FIND) $(PKG_BUILD_DIR) -lname "*busybox" -exec rm \{\} \;
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		EXTRA_CFLAGS="$(TARGET_CFLAGS)" \
		ARCH="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		LDLIBS="$(LDLIBS)" \
		$(BB_MAKE_VERBOSE) \
		LP5523_SUPPORT="$(LP5523_SUPPORT)" \
		WIRELESS_SCHEDULE_SUPPORT="$(WIRELESS_SCHEDULE_SUPPORT)" \
		BUSYBOX_DHCP_OPTION66="$(CONFIG_BUSYBOX_DHCP_OPTION66)" \
		install
endef

define Package/busybox/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
	$(INSTALL_DIR) $(1)/etc/crontabs
	touch $(1)/etc/crontabs/root
	for tmp in $(init-y); do \
		$(INSTALL_BIN) ./filesystem/$$$$tmp $(1)/etc/init.d/$$$$tmp; \
	done
	-rm -rf $(1)/lib64
endef

$(eval $(call BuildPackage,busybox))
