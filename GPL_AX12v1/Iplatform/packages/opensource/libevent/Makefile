# Copyright (c) 2016 Shenzhen TP-LINK Technologies Co.Ltd.
#
# Author  : Wang Lian <wanglian@tp-link.net>
# Version : 1.0
# Date    : 2017.03.23
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libevent
PKG_RELEASE:=1
PKG_VERSION:=2.1.8

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
    
include $(INCLUDE_DIR)/package.mk

CFLAGS += -I$(STAGING_DIR)/usr/include
LDFLAGS += -L$(STAGING_DIR)/usr/lib 
LDFLAGS += -L$(STAGING_DIR)/lib

#CFLAGS += -Wno-error=unused-but-set-variable -Wno-error=unused-result

define Package/$(PKG_NAME)    
	SECTION:=TP-LINK
    SUBMENU:=P2P
	DEPENDS:=+libpthread +libopenssl
	CATEGORY:=TP-LINK iplatform apps
	TITLE:=libevent
endef

define Package/$(PKG_NAME)/description
	libevent
endef

define Build/Prepare	
    mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure	
	(cd $(PKG_BUILD_DIR) && ./configure --prefix=/usr \
		CC=$(CC) \
		CFLAGS='$(CFLAGS)' \
		LDFLAGS='$(LDFLAGS)' )
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/.libs/libevent.so $(1)/usr/lib/
	ln -sf libevent.so $(1)/usr/lib/libevent-2.1.so.6
	ln -sf libevent.so $(1)/usr/lib/libevent-2.1.so.6.0.2
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/.libs/libevent*.{a,so*} $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
