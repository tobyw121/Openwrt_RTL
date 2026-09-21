include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8197f-tenda-ac23-base
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/rtl8197f-tenda-ac23-base
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RTL8197F Tenda AC23/Lynx board-only userspace defaults
  DEPENDS:=@TARGET_realtek_rtl8197f
endef

define Package/rtl8197f-tenda-ac23-base/description
 Installs the network first-boot defaults specific to the Tenda AC23/Lynx
 RTL8197F board instead of carrying them in every Realtek target rootfs.
endef

define Build/Compile
endef

define Package/rtl8197f-tenda-ac23-base/install
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/98-tenda-ac23-flow-offload $(1)/etc/uci-defaults/98-tenda-ac23-flow-offload
	$(INSTALL_BIN) ./files/etc/uci-defaults/99-tenda-lynx-network $(1)/etc/uci-defaults/99-tenda-lynx-network
endef


$(eval $(call BuildPackage,rtl8197f-tenda-ac23-base))
