#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=pppd
PKG_VERSION:=2.4.3
PKG_RELEASE:=8

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=Network
  DEPENDS:=+kmod-ppp +kmod-pptp +kmod-pppoe +resolveip +PACKAGE_$(PKG_NAME)_UGLY_LOG:liblog
  TITLE:=PPP daemon (PPPoE/PPTP/L2TP plugin included)
endef

define Package/$(PKG_NAME)/description
This package contains the PPP (Point-to-Point Protocol) daemon for PPPoE, L2TP and PPTP.
endef

define Package/$(PKG_NAME)/config
	config PACKAGE_$(PKG_NAME)_UGLY_LOG
		bool "Enable ugly log"
		default y
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
$(call Build/Configure/Default,, \
	UNAME_S="Linux" \
	UNAME_R="$(LINUX_VERSION)" \
	UNAME_M="$(ARCH)" \
)
endef

ifeq ($(CONFIG_PPTP_VPN_RPS_OPTIMIZE),y)
	TARGET_CFLAGS+=-DPPTP_VPN_RPS_OPTIMIZE
endif

ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"9.2")
	export CONFIG_GCC_VERSION_92=y
endif

ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"10.3")
	export CONFIG_GCC_VERSION_92=y
endif

ifeq ($(CONFIG_LIBC_USE_MUSL),y)
	TARGET_CFLAGS += -DCONFIG_LIBC_USE_MUSL=y -I$(PKG_BUILD_DIR)/include
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/pppd $(TARGET_CONFIGURE_OPTS) \
		TARGET_CFLAGS="$(TARGET_CPPFLAGS) $(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include $(if $(CONFIG_PACKAGE_$(PKG_NAME)_UGLY_LOG),-DCONFIG_UGLY_LOG=1) -DQSDK=1" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		EXTRA_LIBS="$(if $(CONFIG_PACKAGE_$(PKG_NAME)_UGLY_LOG),-llog)"
endef

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR)/pppd install-devel PKG_INSTALL_DIR="$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/pppd $(1)/usr/include/
	
ifeq ($(CONFIG_LIBC_USE_MUSL),y)
	$(INSTALL_DIR) $(1)/usr/include/net
	$(CP) ./src/include/net/ppp_defs.h $(1)/usr/include/net
endif	
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/pppd/pppd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/ppp
	$(INSTALL_CONF) ./filesystem/etc/ppp/chap-secrets $(1)/etc/ppp/
	$(INSTALL_DATA) ./filesystem/etc/ppp/filter $(1)/etc/ppp/
	$(INSTALL_DATA) ./filesystem/etc/ppp/options $(1)/etc/ppp/
	ln -sf /tmp/resolv.conf.ppp $(1)/etc/ppp/resolv.conf
	$(INSTALL_DIR) $(1)/lib/netifd/proto
	$(INSTALL_BIN) ./filesystem/lib/netifd/proto/ppp.sh $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./filesystem/lib/netifd/proto/pppv6.sh $(1)/lib/netifd/proto/
	$(INSTALL_BIN) ./filesystem/lib/netifd/proto/pppshare.sh $(1)/lib/netifd/proto/
ifeq ($(CONFIG_PACKAGE_pptpvpn-client),y)
		$(INSTALL_BIN) ./filesystem/lib/netifd/proto/pptpvpn.sh $(1)/lib/netifd/proto/
		$(INSTALL_BIN) ./filesystem/lib/netifd/pppvpn-up $(1)/lib/netifd/
		$(INSTALL_BIN) ./filesystem/lib/netifd/pppvpn-down $(1)/lib/netifd/
endif
ifeq ($(CONFIG_PACKAGE_l2tpipsecvpn-server),y)
	$(INSTALL_BIN) ./filesystem/lib/netifd/l2tpvpn-server-up $(1)/lib/netifd/
	$(INSTALL_BIN) ./filesystem/lib/netifd/l2tpvpn-server-down $(1)/lib/netifd/
endif
	$(INSTALL_BIN) ./filesystem/lib/netifd/ppp-up $(1)/lib/netifd/
	$(INSTALL_BIN) ./filesystem/lib/netifd/pppv6-up $(1)/lib/netifd/
	$(INSTALL_BIN) ./filesystem/lib/netifd/pppshare-up $(1)/lib/netifd/
	$(INSTALL_BIN) ./filesystem/lib/netifd/ppp-dhcp6c.script $(1)/lib/netifd/
	$(INSTALL_BIN) ./filesystem/lib/netifd/pppshare-dhcp6c.script $(1)/lib/netifd/
	$(INSTALL_BIN) ./filesystem/lib/netifd/ppp-down $(1)/lib/netifd/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

