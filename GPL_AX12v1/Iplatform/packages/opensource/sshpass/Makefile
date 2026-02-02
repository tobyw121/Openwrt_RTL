#
# Copyright (C) 2006-2009 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sshpass
PKG_VERSION:=1.0.6
PKG_RELEASE:=1

include $(INCLUDE_DIR)/package.mk

define Package/sshpass
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=sshpass
endef

define Package/sshpass/description
  SSHpass offers you the ability to automatically offer a password via SSH when you are prompted for it.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-static \
	--enable-shared

define Package/sshpass/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/sshpass $(1)/usr/bin/
endef

$(eval $(call BuildPackage,sshpass))
