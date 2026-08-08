include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8192cd-rtl8197f
PKG_RELEASE:=6
PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=RTL8197F RD05/MW5/AC23 experimental port
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8192cd-rtl8197f/Default
  SUBMENU:=Wireless Drivers
  DEPENDS:=@TARGET_realtek_rtl8197f +wireless-tools
  FILES:=$(PKG_BUILD_DIR)/rtl8192cd.ko
  KCONFIG:= \
	CONFIG_WIRELESS=y \
	CONFIG_WIRELESS_EXT=y \
	CONFIG_WEXT_CORE=y \
	CONFIG_WEXT_PROC=y \
	CONFIG_WEXT_PRIV=y \
	CONFIG_WEXT_SPY=y
endef

define KernelPackage/rtl8192cd-rtl8197f-mw5
$(call KernelPackage/rtl8192cd-rtl8197f/Default)
  TITLE:=RTL8197FS + RTL8822B WLAN bring-up driver for Tenda MW5
  VARIANT:=mw5
  CONFLICTS:=kmod-rtl8192cd-rtl8197f-rd05 kmod-rtl8192cd-rtl8197f-ac23
endef

define KernelPackage/rtl8192cd-rtl8197f-rd05
$(call KernelPackage/rtl8192cd-rtl8197f/Default)
  TITLE:=RTL8197FH/VG + RTL8812F WLAN bring-up driver for Xiaomi RD05
  VARIANT:=rd05
  CONFLICTS:=kmod-rtl8192cd-rtl8197f-mw5 kmod-rtl8192cd-rtl8197f-ac23
endef

define KernelPackage/rtl8192cd-rtl8197f-ac23
$(call KernelPackage/rtl8192cd-rtl8197f/Default)
  TITLE:=RTL8197FH + RTL8814B WLAN bring-up driver for Tenda AC23
  VARIANT:=ac23
  CONFLICTS:=kmod-rtl8192cd-rtl8197f-mw5 kmod-rtl8192cd-rtl8197f-rd05
endef

define KernelPackage/rtl8192cd-rtl8197f-mw5/description
 Linux-6 compatibility port for the MW5 radio pair: integrated RTL8197FS
 2.4 GHz plus external PCIe RTL8822B 5 GHz. Manual, phase-instrumented load.
endef

define KernelPackage/rtl8192cd-rtl8197f-rd05/description
 Linux-6 compatibility port for the RD05 radio pair: integrated RTL8197FH/VG
 2.4 GHz plus external PCIe RTL8812F 5 GHz. Manual, phase-instrumented load.
endef

define KernelPackage/rtl8192cd-rtl8197f-ac23/description
 Linux-6 compatibility port for the AC23 radio pair: integrated RTL8197FH
 2.4 GHz plus external PCIe RTL8814B 5 GHz. Manual, phase-instrumented load.
endef

define Build/Prepare
	$(RM) -r $(PKG_BUILD_DIR)
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/. $(PKG_BUILD_DIR)/
	find $(PKG_BUILD_DIR) -maxdepth 1 -type f -name 'data_*.c' -exec touch {} +
endef

RTL8197F_COMMON_VARS:= \
	CONFIG_RTL8192CD=m \
	CONFIG_WIRELESS_LAN_MODULE=y \
	CONFIG_WIFI_HCI=y \
	CONFIG_PCI_HCI=y \
	CONFIG_NET_PCI=y \
	CONFIG_WLAN_HAL=y \
	CONFIG_WLAN_HAL_88XX=y \
	CONFIG_WLAN_HAL_8197F=y \
	CONFIG_RTL_8197F=y \
	CONFIG_RTL_8197F_VG=y \
	CONFIG_RTL_ODM_WLAN_DRIVER=y \
	CONFIG_WLAN_MACHAL_API=y \
	CONFIG_OPENWRT_SDK=y \
	CONFIG_MW5_LEGACY_WEXT=y

RTL8197F_VARIANT_VARS_mw5:= \
	CONFIG_WLAN_HAL_8822BE=y \
	CONFIG_WLAN_MACHAL_API_V0=y

RTL8197F_VARIANT_VARS_rd05:= \
	CONFIG_WLAN_HAL_8812FE=y \
	CONFIG_WLAN_MACHAL_API_V0=y

RTL8197F_VARIANT_VARS_ac23:= \
	CONFIG_WLAN_HAL_8814BE=y \
	CONFIG_WLAN_MACHAL_API_V1=y

define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
		$(RTL8197F_COMMON_VARS) \
		$(RTL8197F_VARIANT_VARS_$(BUILD_VARIANT)) \
		KCFLAGS="-I$(LINUX_DIR) -include $(PKG_BUILD_DIR)/rtl8197f_linux_compat.h -Wno-error -Wno-implicit-function-declaration -Wno-int-conversion -Wno-incompatible-pointer-types -Wno-address-of-packed-member" \
		modules
endef

define KernelPackage/rtl8192cd-rtl8197f/install-common
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/sbin $(1)/etc
	$(INSTALL_BIN) ./files/usr/bin/rtl8197f-wlan-check $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/sbin/rtl8197f-wlan-load $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/usr/sbin/rtl8197f-wlan-ap-test $(1)/usr/sbin/
	$(INSTALL_CONF) ./files/etc/rtl8197f-wlan-profiles.conf $(1)/etc/
endef

define KernelPackage/rtl8192cd-rtl8197f-mw5/install
$(call KernelPackage/rtl8192cd-rtl8197f/install-common,$(1))
endef

define KernelPackage/rtl8192cd-rtl8197f-rd05/install
$(call KernelPackage/rtl8192cd-rtl8197f/install-common,$(1))
endef

define KernelPackage/rtl8192cd-rtl8197f-ac23/install
$(call KernelPackage/rtl8192cd-rtl8197f/install-common,$(1))
endef

$(eval $(call KernelPackage,rtl8192cd-rtl8197f-mw5))
$(eval $(call KernelPackage,rtl8192cd-rtl8197f-rd05))
$(eval $(call KernelPackage,rtl8192cd-rtl8197f-ac23))
