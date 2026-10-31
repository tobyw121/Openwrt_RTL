include $(TOPDIR)/rules.mk

PKG_NAME:=rtl8197f-cudy-ax1500-base
PKG_RELEASE:=2
PKG_LICENSE:=GPL-2.0-or-later

include $(INCLUDE_DIR)/package.mk

define Package/rtl8197f-cudy-ax1500-base
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Cudy AX1500 UART/SPI/PCI bring-up diagnostics
  DEPENDS:=@TARGET_realtek_rtl8197f
endef

define Package/rtl8197f-cudy-ax1500-base/description
 Read-only diagnostics for experimental Cudy AX1500 RAM boot profiles.
 Reports PCI identity on the R76 SPI profile. Does not write flash or
 configure Ethernet, WLAN, LEDs or GPIOs.
endef

define Build/Compile
endef

define Package/rtl8197f-cudy-ax1500-base/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) ./files/cudy-ax1500-diag $(1)/usr/sbin/cudy-ax1500-diag
endef

$(eval $(call BuildPackage,rtl8197f-cudy-ax1500-base))
