# Copyright(c) 2011-2019 Shenzhen TP-LINK Technologies Co.Ltd.
# file     Makefile
# brief    
# author   Zhu Haiming<zhuhaiming@tp-link.com.cn>
# version  1.0.0
# date     01Dec14
# history  arg 1.0.0, 07May2019, Zhu Haiming, Create the file. 

include $(TOPDIR)/rules.mk

PKG_NAME:=rpcap
PKG_VERSION:=4.1.3
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/rpcap
  SECTION:=net
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=Debug
  TITLE:=wireshark remote package tool rpcap
  URL:=http://tp-link.net
  DEPENDS:=libpthread
  MAINTAINER:=Zhu Haiming<zhuhaiming@tp-link.net>
endef

define Package/rpcap/Description
	wireshark remote package tool
endef

CONFIGURE_ARGS += --with-pcap=linux

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/rpcapd/ CC=$(TARGET_CC)
endef

define Package/rpcap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rpcapd/rpcapd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/etc/init.d/rpcap $(1)/etc/init.d/rpcap
endef

$(eval $(call BuildPackage,rpcap))
