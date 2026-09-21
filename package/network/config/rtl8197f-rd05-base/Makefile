include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8197f-rd05-base
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/rtl8197f-rd05-base
  SECTION:=net
  CATEGORY:=Network
  TITLE:=RTL8197F Xiaomi R4/RD05 board-only userspace defaults
  DEPENDS:=@TARGET_realtek_rtl8197f
endef

define Package/rtl8197f-rd05-base/description
 Installs userspace helpers and first-boot defaults that are specific to the
 Xiaomi R4/RD05 RTL8197F board. Keeping these files out of the common Realtek
 base-files overlay avoids wasting the Tenda Nova MW5 fixed 0x590000 firmware
 window while preserving the exact RD05 runtime files on RD05 images.
endef

define Build/Compile
endef

define Package/rtl8197f-rd05-base/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/usr/sbin/rd05-ioctl $(1)/usr/sbin/rd05-ioctl
	$(INSTALL_DIR) $(1)/etc/profile.d
	$(INSTALL_BIN) ./files/etc/profile.d/99-rd05-uart-guard.sh $(1)/etc/profile.d/99-rd05-uart-guard.sh
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/etc/uci-defaults/98-rd05-firewall-flow-offload $(1)/etc/uci-defaults/98-rd05-firewall-flow-offload
	$(INSTALL_BIN) ./files/etc/uci-defaults/99-rd05-dsa-network $(1)/etc/uci-defaults/99-rd05-dsa-network
endef

$(eval $(call BuildPackage,rtl8197f-rd05-base))
