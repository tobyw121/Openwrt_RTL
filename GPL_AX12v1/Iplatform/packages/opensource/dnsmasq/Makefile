#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dnsmasq

ifeq ($(CONFIG_PACKAGE_DNSMASQ_VERSION),"2.83")
PKG_VERSION:=2.83
else
PKG_VERSION:=2.62
endif
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://thekelleys.org.uk/dnsmasq

ifeq ($(PKG_VERSION),2.62)
PKG_MD5SUM:=f47e5cb8f5bac6343f24b2dbe317ab40
else
PKG_MD5SUM:=c87d5af020d12984d2ab9fbf04e2dcca
endif

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/dnsmasq/Default
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=A lightweight DNS and DHCP server
  DEPENDS:= +PACKAGE_$(PKG_NAME)_UGLY_LOG:liblog +libubox +PACKAGE_ffs:libuci
  URL:=http://www.thekelleys.org.uk/dnsmasq/
endef

define Package/dnsmasq
$(call Package/dnsmasq/Default)
  VARIANT:=nodhcpv6
endef

define Package/dnsmasq-dhcpv6
$(call Package/dnsmasq/Default)
  TITLE += (with DHCPv6 support)
  DEPENDS:=@IPV6 +kmod-ipv6 +libubox +PACKAGE_ffs:libuci
  VARIANT:=dhcpv6
endef

define Package/dnsmasq/description
  It is intended to provide coupled DNS and DHCP service to a LAN.
endef

define Package/dnsmasq-dhcpv6/description
$(call Package/dnsmasq/description)

This is a variant with DHCPv6 support
endef

define Package/dnsmasq/config/Default
	config PACKAGE_$(PKG_NAME)_UGLY_LOG
		bool "Enable ugly log"
		default y
endef

Package/dnsmasq/config=$(call Package/dnsmasq/config/Default,dnsmasq)
Package/dnsmasq-dhcpv6/config=$(call Package/dnsmasq/config/Default,dnsmasq-dhcpv6)

define Package/dnsmasq/conffiles
/etc/config/dhcp
/etc/dnsmasq.conf
endef

Package/dnsmasq-dhcpv6/conffiles = $(Package/dnsmasq/conffiles)

TARGET_CFLAGS += \
	-ffunction-sections -fdata-sections \
	$(if $(CONFIG_IPV6),,-DNO_IPV6)\
	$(if $(CONFIG_PACKAGE_ffs), -DFFS_SUPPORT)

ifeq ($(BUILD_VARIANT),nodhcpv6)
  TARGET_CFLAGS += -DNO_DHCP6
endif

MAKE_FLAGS := \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS) $(if $(CONFIG_PACKAGE_$(PKG_NAME)_UGLY_LOG), -DCONFIG_UGLY_LOG=1) -I$(STAGING_DIR)/usr/include" \
	LDFLAGS="-Wl,--gc-sections $(if $(CONFIG_PACKAGE_$(PKG_NAME)_UGLY_LOG), -L$(STAGING_DIR)/usr/lib -llog) \
		$(if $(CONFIG_PACKAGE_ffs), -L$(STAGING_DIR)/usr/lib -luci)"\
	PREFIX="/usr"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
ifeq ($(PKG_VERSION),2.62)
	$(CP) -u ./src/* $(PKG_BUILD_DIR)/
else
	$(CP) -u ./src-2.83/* $(PKG_BUILD_DIR)/
endif
endef

define Package/dnsmasq/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_INSTALL_DIR)/usr/sbin/dnsmasq $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/config
ifeq ($(PKG_VERSION),2.62)
	$(INSTALL_DATA) ./filesystem/etc/config/dhcp.conf $(1)/etc/config/dhcp
	$(INSTALL_DATA) ./filesystem/etc/dnsmasq.conf $(1)/etc/dnsmasq.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/etc/init.d/dnsmasq.init $(1)/etc/init.d/dnsmasq
	$(INSTALL_DIR) $(1)/etc/hotplug.d/iface
	$(INSTALL_BIN) ./filesystem/etc/hotplug.d/iface/dnsmasq.hotplug $(1)/etc/hotplug.d/iface/60-dnsmasq
else
	$(INSTALL_DATA) ./filesystem-2.83/etc/config/dhcp.conf $(1)/etc/config/dhcp
	$(INSTALL_DATA) ./filesystem-2.83/etc/dnsmasq.conf $(1)/etc/dnsmasq.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem-2.83/etc/init.d/dnsmasq.init $(1)/etc/init.d/dnsmasq	
endif
endef

Package/dnsmasq-dhcpv6/install = $(Package/dnsmasq/install)

$(eval $(call BuildPackage,dnsmasq))
$(eval $(call BuildPackage,dnsmasq-dhcpv6))
