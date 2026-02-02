#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=comgt
PKG_VERSION:=0.32
PKG_RELEASE:=21

PKG_SOURCE:=$(PKG_NAME).$(PKG_VERSION).tgz
PKG_SOURCE_URL:=@SF/comgt
PKG_MD5SUM:=db2452680c3d953631299e331daf49ef

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME).$(PKG_VERSION)
PKG_CHECK_FORMAT_SECURITY:=0

include $(INCLUDE_DIR)/package.mk

define Package/comgt/Default
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
endef

define Package/comgt
$(call Package/comgt/Default)
  TITLE:=Option/Vodafone 3G/GPRS control tool
  DEPENDS:=+chat
  URL:=http://www.pharscape.org/comgt.html
endef

define Package/comgt/description
 comgt is a scripting language interpreter useful for establishing 
 communications on serial lines and through PCMCIA modems as well as GPRS 
 and 3G datacards.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="" \
		comgt
endef

define Package/comgt/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/comgt $(1)/usr/bin/
	ln -s comgt $(1)/usr/bin/gcom
	$(CP) ./filesystem/* $(1)/
#	$(INSTALL_DIR) $(1)/etc/chat
#	$(INSTALL_DATA) ./filesystem/etc/chat/3g.chat $(1)/etc/chat/3g.chat
#	$(INSTALL_DATA) ./filesystem/etc/chat/evdo.chat $(1)/etc/chat/evdo.chat
#	$(INSTALL_DIR) $(1)/etc/gcom
#	$(INSTALL_DATA) ./filesystem/etc/gcom/setpin.gcom $(1)/etc/gcom/setpin.gcom
#	$(INSTALL_DATA) ./filesystem/etc/gcom/setmode.gcom $(1)/etc/gcom/setmode.gcom
#	$(INSTALL_DATA) ./filesystem/etc/gcom/getcardinfo.gcom $(1)/etc/gcom/getcardinfo.gcom
#	$(INSTALL_DATA) ./filesystem/etc/gcom/getstrength.gcom $(1)/etc/gcom/getstrength.gcom
#	$(INSTALL_DATA) ./filesystem/etc/gcom/getcarrier.gcom $(1)/etc/gcom/getcarrier.gcom
#	$(INSTALL_DATA) ./filesystem/etc/gcom/getcnum.gcom $(1)/etc/gcom/getcnum.gcom
#	$(INSTALL_DATA) ./filesystem/etc/gcom/getimsi.gcom $(1)/etc/gcom/getimsi.gcom
#	$(INSTALL_DIR) $(1)/lib/netifd/proto
#	$(INSTALL_BIN) ./filesystem/lib/netifd/proto/3g.sh $(1)/lib/netifd/proto/3g.sh
#	$(INSTALL_BIN) ./filesystem/lib/netifd/proto/ncm.sh $(1)/lib/netifd/proto/ncm.sh
endef

$(eval $(call BuildPackage,comgt))
