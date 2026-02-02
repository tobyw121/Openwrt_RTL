# Copyright (c) 2016 Shenzhen TP-LINK Technologies Co.Ltd.
#
# Author  : Wang Lian <wanglian@tp-link.net>
# Version : 1.0
# Date    : 2017.03.29
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=miniupnpc
PKG_RELEASE:=1
PKG_VERSION:=2.0

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib 
TARGET_LDFLAGS += -L$(STAGING_DIR)/lib 

define Package/$(PKG_NAME)/template
	SECTION:=TP-LINK	
	CATEGORY:=TP-LINK iplatform apps
    SUBMENU:=P2P
endef

define Package/$(PKG_NAME)
	$(call Package/$(PKG_NAME)/template)
	DEPENDS:=+libpthread
	TITLE:=miniupnpc
endef

define Package/libminiupnpc/description
	libminiupnpc
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/bin
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib/libminiupnpc.so  $(1)/usr/lib/
	ln -sf libminiupnpc.so $(1)/usr/lib/libminiupnpc.so.16
#	$(INSTALL_DIR) $(1)/usr/sbin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/usr/sbin/
#	$(CP) -rf ./filesystem/* $(1)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/miniupnpc $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
