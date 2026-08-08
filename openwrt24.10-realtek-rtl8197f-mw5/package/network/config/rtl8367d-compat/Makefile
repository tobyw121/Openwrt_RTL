# SPDX-License-Identifier: GPL-2.0-or-later
#
# Mainline-friendly compatibility helpers for RTL8197F/RD05 + RTL8367D.
# This package replaces Realtek swconfig/private-ioctl/FastPath/ACL workflows
# with standard Linux DSA, bridge, ethtool, tc and nftables commands.

include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8367d-compat
PKG_VERSION:=2026.07.26
PKG_RELEASE:=11

include $(INCLUDE_DIR)/package.mk

define Package/rtl8367d-compat
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RTL8197F/RD05 and RTL8367-family DSA compatibility helpers
  DEPENDS:=+ip-bridge +tc +ethtool +nftables +firewall4 +kmod-nft-offload
endef

define Package/rtl8367d-compat/description
  User-space compatibility layer for Realtek SDK/swconfig/private-ioctl style
  switch workflows on RTL8197F/RD05 and RTL8367-family switches.  It uses DSA, bridge VLANs,
  ethtool, tc flower and nftables flowtables instead of legacy kernel hooks.
  It also installs rd05-raw for bounded AF_PACKET/PCAP and Magic-frame tests.
endef

define Package/rtl8367d-compat/conffiles
/etc/rd05-rgmii.conf
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/rd05-ioctl $(CURDIR)/src/rd05-ioctl.c
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/rd05-raw $(CURDIR)/src/rd05-raw.c
endef

define Package/rtl8367d-compat/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/rtl8367dctl $(1)/usr/sbin/rtl8367dctl
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rd05-ioctl $(1)/usr/sbin/rd05-ioctl.bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/rd05-raw $(1)/usr/sbin/rd05-raw
	$(INSTALL_BIN) ./files/rd05-rgmii-calibrate $(1)/usr/sbin/rd05-rgmii-calibrate
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_CONF) ./files/rd05-rgmii.conf $(1)/etc/rd05-rgmii.conf
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/rd05-rgmii.init $(1)/etc/init.d/rd05-rgmii
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./files/swconfig-rtl8367d $(1)/sbin/swconfig-rtl8367d
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/99-rtl8367d-compat $(1)/etc/uci-defaults/99-rtl8367d-compat
endef

$(eval $(call BuildPackage,rtl8367d-compat))
