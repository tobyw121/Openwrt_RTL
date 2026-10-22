include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8197f-flow-offload
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/rtl8197f-flow-offload
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RTL8197F firewall4 Linux software flowtable support
  DEPENDS:=@TARGET_realtek_rtl8197f @IPV6 +firewall4 +kmod-nft-offload
endef

define Package/rtl8197f-flow-offload/description
 Installs the standard Linux flowtable dependencies for rtl8197f-accel.
 Activation is explicit through rtl8197f-accel software on; this package
 does not enable software offload at boot or enable Realtek hardware NAT.
 MW5 remains locked pending stress validation and Cudy profiles are RAM-only.
endef

define Build/Compile
endef

define Package/rtl8197f-flow-offload/install
	$(INSTALL_DIR) $(1)/usr/share/rtl8197f-flow-offload
	$(INSTALL_DATA) ./files/README $(1)/usr/share/rtl8197f-flow-offload/README
endef

$(eval $(call BuildPackage,rtl8197f-flow-offload))
