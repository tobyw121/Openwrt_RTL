#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openvpn

ifeq ($(CONFIG_PACKAGE_OPENVPN_VERSION), "2.4.11")
PKG_VERSION:=2.4.11
PKG_RELEASE:=1
PKG_SOURCE_URL:=\
	https://build.openvpn.net/downloads/releases/ \
	https://swupdate.openvpn.net/community/releases/
PKG_HASH:=e579eff218ab1d765965e64a917927504d8324717afdfcd56850f6b83ba8441b
PKG_MAINTAINER:=Felix Fietkau <nbd@nbd.name>
PKG_BUILD_SRC=./src-2.4.11/*
PATCH_DIR:=./patches-2.4.11
else
PKG_VERSION:=2.3.8
PKG_RELEASE:=1
# PKG_SOURCE_URL:=http://swupdate.openvpn.net/community/releases
PKG_MD5SUM:=6ca03fe0fd093e0d01601abee808835c
PKG_BUILD_SRC=./src/*
endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)-multi/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_LICENSE:=GPL-2.0

include $(INCLUDE_DIR)/package.mk

define Package/openvpn/Default
  TITLE:=Open source VPN solution using $(2)
  SECTION:=net
  CATEGORY:=TP-LINK iplatform apps
  URL:=http://openvpn.net
  SUBMENU:=VPN
  MENU:=1
  DEPENDS:=+kmod-tun +OPENVPN_$(1)_ENABLE_LZO:liblzo +PACKAGE_$(PKG_NAME)_UGLY_LOG:liblog +OPENVPN_$(1)_ENABLE_IPROUTE2:ip $(3)
  VARIANT:=$(1)
  MAINTAINER:=Mirko Vogt <mirko@openwrt.org>
endef

Package/openvpn-openssl=$(call Package/openvpn/Default,openssl,OpenSSL,+libopenssl)
Package/openvpn-polarssl=$(call Package/openvpn/Default,polarssl,PolarSSL,+libpolarssl)
Package/openvpn-nossl=$(call Package/openvpn/Default,nossl,plaintext (no SSL))

ifeq ($(PKG_VERSION), "2.4.11")
define Package/openvpn/config/Default/source
	source "$(SOURCE)/Config-$(1)-2.4.11.in"
endef
else
define Package/openvpn/config/Default/source
	source "$(SOURCE)/Config-$(1).in"
endef
endif

define Package/openvpn/config/Default
	$(call Package/openvpn/config/Default/source,$(1))

	config PACKAGE_OPENVPN_VERSION
		string "openvpn compile version"
		depends PACKAGE_openvpn-openssl
		default "2.3.8"
		help
			choose which version u would like to build into ipf.
			as time pass by, we could add more newer versions to ipf. if no choose, default is 2.3.8

	config PACKAGE_$(PKG_NAME)_UGLY_LOG
		bool "Enable ugly log"
		default y
endef

Package/openvpn-openssl/config=$(call Package/openvpn/config/Default,openssl)
Package/openvpn-polarssl/config=$(call Package/openvpn/config/Default,polarssl)
Package/openvpn-nossl/config=$(call Package/openvpn/config/Default,nossl)

ifeq ($(BUILD_VARIANT),polarssl)
CONFIG_OPENVPN_POLARSSL:=y
endif
ifeq ($(BUILD_VARIANT),openssl)
CONFIG_OPENVPN_OPENSSL:=y
endif
ifeq ($(BUILD_VARIANT),nossl)
CONFIG_OPENVPN_NOSSL:=y
endif

CONFIGURE_VARS += \
	IFCONFIG=/sbin/ifconfig \
	ROUTE=/sbin/route \
	IPROUTE=/usr/sbin/ip \
	NETSTAT=/sbin/netstat

ifneq ($(CONFIG_PACKAGE_$(PKG_NAME)_UGLY_LOG),)
     TARGET_CFLAGS += -DCONFIG_UGLY_LOG=1
     EXTRA_LDFLAGS += -llog
endif

TARGET_CFLAGS += -Os
define Build/Configure
	$(call Build/Configure/Default, \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_SMALL),--enable-small) \
		--disable-selinux \
		--disable-systemd \
		--disable-plugins \
		--disable-debug \
		--disable-eurephia \
		--disable-pkcs11 \
		--enable-password-save \
		--enable-management \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_LZO),--enable,--disable)-lzo \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_X509_ALT_USERNAME),enable,disable-x509-alt-username)-ssl \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_SERVER),--enable,--disable)-server \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_SOCKS),--enable,--disable)-socks \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_HTTP),--enable,--disable)-http \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_FRAGMENT),--enable,--disable)-fragment \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_MULTIHOME),--enable,--disable)-multihome \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_IPROUTE2),--enable,--disable)-iproute2 \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_DEF_AUTH),--enable,--disable)-def-auth \
		$(if $(CONFIG_OPENVPN_$(BUILD_VARIANT)_ENABLE_PF),--enable,--disable)-pf \
		$(if $(CONFIG_OPENVPN_NOSSL),--disable-ssl --disable-crypto,--enable-ssl --enable-crypto) \
		$(if $(CONFIG_OPENVPN_OPENSSL),--with-crypto-library=openssl) \
		$(if $(CONFIG_OPENVPN_POLARSSL),--with-crypto-library=polarssl) \
	)
endef

define Package/openvpn-$(BUILD_VARIANT)/conffiles
/etc/config/openvpn
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(PKG_BUILD_SRC) $(PKG_BUILD_DIR)
ifeq ($(CONFIG_PACKAGE_OPENVPN_VERSION), "2.4.11")
	$(CP) ./src-2.4.11/.gitignore $(PKG_BUILD_DIR)
	$(CP) ./src-2.4.11/.gitattributes $(PKG_BUILD_DIR)
	$(Build/Patch)
else
	$(CP) ./src/.gitignore $(PKG_BUILD_DIR)
	$(CP) ./src/.gitattributes $(PKG_BUILD_DIR)
endif
	echo ------openvpn-$(PKG_VERSION)-------
endef

define Package/openvpn-$(BUILD_VARIANT)/install
	$(INSTALL_DIR) \
		$(1)/usr/sbin \
		$(1)/etc/init.d \
		$(1)/etc/config \
		$(1)/etc/openvpn \
		$(1)/lib/upgrade/keep.d

	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/openvpn \
		$(1)/usr/sbin/

	$(INSTALL_BIN) \
		filesystem/usr/sbin/checkpsw.sh \
		$(1)/usr/sbin/checkpsw.sh

	$(INSTALL_BIN) \
		filesystem/etc/init.d/openvpn \
		$(1)/etc/init.d/openvpn

	$(INSTALL_CONF) filesystem/etc/config/openvpn \
		$(1)/etc/config/openvpn

	$(INSTALL_DATA) \
		filesystem/lib/upgrade/keep.d/openvpn \
		$(1)/lib/upgrade/keep.d/openvpn
ifeq ($(CONFIG_PACKAGE_openvpn-client),y)
		$(INSTALL_DIR) $(1)/lib/netifd/proto
		$(INSTALL_BIN) ./filesystem/lib/netifd/proto/openvpn.sh $(1)/lib/netifd/proto
		$(INSTALL_BIN) ./filesystem/lib/netifd/openvpn-up.sh $(1)/lib/netifd/
		$(INSTALL_BIN) ./filesystem/lib/netifd/openvpn-down.sh $(1)/lib/netifd/
endif
endef

$(eval $(call BuildPackage,openvpn-openssl))
$(eval $(call BuildPackage,openvpn-polarssl))
$(eval $(call BuildPackage,openvpn-nossl))
