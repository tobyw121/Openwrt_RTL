#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=wifidog
PKG_KMOD_NAME:=br_wifidog

PKG_VERSION:=1.1.1
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

# PKG_SOURCE_URL:=http://swupdate.openvpn.net/community/releases

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib 
TARGET_LDFLAGS += -L$(STAGING_DIR)/lib 

define Package/$(PKG_NAME)
  TITLE:=Open source wifidog solution using $(2)
  CATEGORY:=TP-LINK iplatform apps
  URL:=http://wifidog.net
  SUBMENU:=wifidog
  MENU:=1
  DEPENDS:=+libpthread
  MENU:=1
endef

define KernelPackage/$(PKG_KMOD_NAME)/Default
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  MAINTAINER:=Wang Hao <wanghao@tp-link.com.cn>
endef

define KernelPackage/$(PKG_KMOD_NAME)
  $(call KernelPackage/$(PKG_KMOD_NAME)/Default)
   SUBMENU:=wifidog
   TITLE:=wifidog handler in bridge 
   FILES:=$(PKG_BUILD_DIR)/kernel/$(PKG_KMOD_NAME).ko
   AUTOLOAD:=
endef

define Build/Configure
	$(call Build/Configure/Default)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef


EXTRA_KCONFIG:= \
	CONFIG_BR_FILTER=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)/kernel" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	CFLAGS="$(TARGET_CFLAGS) -Wno-error=unused-variable"	\
	$(EXTRA_KCONFIG)

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/wifidog
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/wifidog/locale

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifidog $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wdctl $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/wifidog/* $(1)/usr/include/
	
	# $(CP) filesystem/etc/wifidog/* $(1)/etc/wifidog/
	$(CP) filesystem/* $(1)/
	$(INSTALL_BIN) filesystem/etc/init.d/wifidog $(1)/etc/init.d/wifidog
	
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call KernelPackage,$(PKG_KMOD_NAME)))

