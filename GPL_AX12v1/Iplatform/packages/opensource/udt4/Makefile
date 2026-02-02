# Copyright (c) 2016 Shenzhen TP-LINK Technologies Co.Ltd.
#
# Author  : Wang Lian <wanglian@tp-link.net>
# Version : 1.0
# Date    : 2017.03.02
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=udt4
PKG_RELEASE:=1
PKG_VERSION:=4.11

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

define Package/libudt
	$(call Package/$(PKG_NAME)/template)
	DEPENDS:=+libpthread +libstdcpp	
	TITLE:=libudt
endef

define Package/libudt/description
	UDT4: UDP-based Data Transfer Library - version 4
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/bin
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/libudt/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/libudt.so $(1)/usr/lib/
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/libudt.a $(1)/usr/lib/
endef

define Package/udtapp
	$(call Package/$(PKG_NAME)/template)
	DEPENDS:=+libpthread +libstdcpp	+libudt
	TITLE:=udtapp
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/src/udt.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/udt_c.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/libudt.so $(1)/usr/lib/
#	$(CP) $(PKG_BUILD_DIR)/src/libudt.a $(1)/usr/lib/
endef

define Package/udtapp/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/appserver $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/appclient $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/sendfile $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/recvfile $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/app/test $(1)/usr/bin/	
	$(CP) -rf ./filesystem/* $(1)/
endef

$(eval $(call BuildPackage,libudt))
$(eval $(call BuildPackage,udtapp))
