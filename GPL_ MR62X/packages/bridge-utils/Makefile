# 
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=bridge-utils
PKG_RELEASE:=1
PKG_SOURCE_URL:=@SF/bridge
PKG_VERSION:=1.5
PKG_MD5SUM:=ec7b381160b340648dede58c31bb2238
PKG_FIXUP:=autoreconf
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

include $(INCLUDE_DIR)/package.mk

define Package/bridge
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=Ethernet bridging configuration utility
  URL:=http://bridge.sourceforge.net/
endef

define Package/bridge/description
 Manage ethernet bridging: a way to connect networks together to 
 form a larger network.
endef

define Package/bridge/config
	config PACKAGE_BRIDGE_BUILD_32BIT_COMPAT_KERNEL_64BIT
		bool "brctl ioctl support kernel 64bit & build it in 32bit"
		depends PACKAGE_bridge
		default n
		help
			if set to y,  support the application which build with 32bit compiler and the kernel which build with 64bit compiler.
endef

CONFIGURE_ARGS += \
	--with-linux-headers="$(LINUX_DIR)" \

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -u ./src/* $(PKG_BUILD_DIR)/
	( cd $(PKG_BUILD_DIR) ; \
		[ -f ./configure ] || { \
			ln -sf configure.in configure.ac ; \
			autoconf ; \
		} \
	)
endef

ifdef CONFIG_PACKAGE_BRIDGE_BUILD_32BIT_COMPAT_KERNEL_64BIT
  	TARGET_CFLAGS += -DIPF_BUILD_32BIT_COMPAT_KERNEL_BUILD_64BIT
endif

define Package/bridge/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brctl/brctl $(1)/usr/sbin
endef

$(eval $(call BuildPackage,bridge))
