include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=rtl8192cd-rtl8197f
PKG_RELEASE:=94
PKG_LICENSE:=GPL-2.0-only
PKG_MAINTAINER:=RTL8197F Linux 6.6 cfg80211 port
PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/rtl8192cd-rtl8197f/Default
  SUBMENU:=Wireless Drivers
  DEPENDS:=@TARGET_realtek_rtl8197f
  FILES:=$(PKG_BUILD_DIR)/rtl8192cd.ko
  KCONFIG:=CONFIG_WIRELESS=y
endef

define KernelPackage/rtl8192cd-rtl8197f-mw5
$(call KernelPackage/rtl8192cd-rtl8197f/Default)
  TITLE:=RTL8197FS + RTL8812BRH WLAN bring-up driver for Tenda MW5
  VARIANT:=mw5
  CONFLICTS:=kmod-rtl8192cd-mw5
  DEPENDS+=+kmod-cfg80211 +@DRIVER_11AC_SUPPORT
endef

define KernelPackage/rtl8192cd-rtl8197f-rd05
$(call KernelPackage/rtl8192cd-rtl8197f/Default)
  TITLE:=RTL8197FH/VG + RTL8812F WLAN bring-up driver for Xiaomi RD05
  VARIANT:=rd05
  DEPENDS+=+wireless-tools
  KCONFIG+=CONFIG_WIRELESS_EXT=y CONFIG_WEXT_CORE=y CONFIG_WEXT_PRIV=y
endef

define KernelPackage/rtl8192cd-rtl8197f-ac23
$(call KernelPackage/rtl8192cd-rtl8197f/Default)
  TITLE:=RTL8197FH + RTL8814B WLAN bring-up driver for Tenda AC23
  VARIANT:=ac23
  DEPENDS+=+wireless-tools
  KCONFIG+=CONFIG_WIRELESS_EXT=y CONFIG_WEXT_CORE=y CONFIG_WEXT_PRIV=y
endef

define KernelPackage/rtl8192cd-rtl8197f-wrd12gn
$(call KernelPackage/rtl8192cd-rtl8197f/Default)
  TITLE:=RTL8197FH + RTL8812BRH (8822B/HALMAC path) WLAN driver for iBall WRD12GN
  VARIANT:=wrd12gn
  DEPENDS+=+kmod-cfg80211 +@DRIVER_11AC_SUPPORT
endef

define KernelPackage/rtl8192cd-rtl8197f-mw5/description
 Linux 6.6.114 cfg80211 port for the MW5 radio pair: integrated RTL8197FS
 2.4 GHz plus physical RTL8812BRH PCIe 5 GHz radio (PCI 10ec:b822,
 using the recovered vendor 8822B-compatible HAL path), with DT/NVMEM board data.
endef

define KernelPackage/rtl8192cd-rtl8197f-rd05/description
 Linux-6 compatibility port for the RD05 radio pair: integrated RTL8197FH/VG
 2.4 GHz plus external PCIe RTL8812F 5 GHz. Manual, phase-instrumented load.
endef

define KernelPackage/rtl8192cd-rtl8197f-ac23/description
 Linux-6 compatibility port for the AC23 radio pair: integrated RTL8197FH
 2.4 GHz plus external PCIe RTL8814B 5 GHz. Manual, phase-instrumented load.
endef

define KernelPackage/rtl8192cd-rtl8197f-wrd12gn/description
 Linux 6.6 cfg80211 port for the WRD12GN radio pair: physical RTL8197FH
 integrated 2.4 GHz plus RTL8812BRH PCIe 5 GHz radio (PCI 10ec:b822). The
 recovered OEM SDK identifies its WLAN bonding profile as 97FN/PKG2 and drives
 the external radio through the 8822B/HALMAC-compatible path; RFE type 0 and
 explicit H601 NVMEM board data are retained.
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
	CONFIG_RTL_WDS_SUPPORT=y \
	CONFIG_RTL_MESH_SUPPORT=y \
	CONFIG_RTL_MESH_SINGLE_IFACE=y

RTL8197F_VARIANT_VARS_mw5:= \
	CONFIG_RTL_MESH_SUPPORT=n \
	CONFIG_RTL_MESH_SINGLE_IFACE=n \
	CONFIG_WLAN_HAL_8822BE=y \
	CONFIG_RTL_8822_SUPPORT=y \
	CONFIG_RTL_92F_SUPPORT=n \
	CONFIG_MW5_OEM_WLAN_ORDER=y \
	CONFIG_MW5_FORCE_WLAN_DEVICE_TABLE=y \
	CONFIG_BAND_5G_ON_WLAN0=y \
	CONFIG_RTL_5G_SLOT_1=y \
	CONFIG_SOC_WIFI=y \
	CONFIG_SOC_RFE_TYPE_5=y \
	CONFIG_USE_PCIE_SLOT_1=y \
	CONFIG_SLOT_1_8822BE=y \
	CONFIG_SLOT_1_RFE_TYPE_6=y \
	HIGH_POWER_EXT_PA=y \
	HIGH_POWER_EXT_LNA=y \
	CONFIG_SOC_EXT_PA=y \
	CONFIG_SOC_EXT_LNA=y \
	CONFIG_RTL8192CD_LINUX6_BUS=y \
	CONFIG_MW5_ACCEL=y \
	CONFIG_RG_WLAN_HWNAT_ACCELERATION=n \
	CONFIG_RTL_EXT_PORT_SUPPORT=n \
	CONFIG_RTW_BRSC=n \
	CONFIG_RTK_BR_SHORTCUT=n \
	CONFIG_RTL_FASTBRIDGE=n \
	CONFIG_HWNAT_FLEETCONTRACK=n \
	CONFIG_RTK_FC_WLAN_HWNAT_ACCELERATION=n \
	CONFIG_RTK_L34_FC_KERNEL_MODULE=n \
	CONFIG_PHY_EAT_40MHZ=y \
	CONFIG_WLAN_MACHAL_API_V0=y

RTL8197F_VARIANT_VARS_rd05:= \
	CONFIG_MW5_LEGACY_WEXT=y \
	CONFIG_RTL_COMAPI_WLTOOLS=y \
	CONFIG_WLAN_HAL_8812FE=y \
	CONFIG_RTL_8812F_SUPPORT=y \
	CONFIG_RTL_92F_SUPPORT=n \
	CONFIG_WLAN_MACHAL_API_V0=y

RTL8197F_VARIANT_VARS_ac23:= \
	CONFIG_MW5_LEGACY_WEXT=y \
	CONFIG_RTL_COMAPI_WLTOOLS=y \
	CONFIG_WLAN_HAL_8814BE=y \
	CONFIG_RTL_8814B_SUPPORT=y \
	CONFIG_RTL_92F_SUPPORT=n \
	CONFIG_WLAN_MACHAL_API_V1=y

RTL8197F_VARIANT_VARS_wrd12gn:= \
	CONFIG_RTL_MESH_SUPPORT=n \
	CONFIG_RTL_MESH_SINGLE_IFACE=n \
	CONFIG_WLAN_HAL_8822BE=y \
	CONFIG_RTL_8822_SUPPORT=y \
	CONFIG_RTL_92F_SUPPORT=n \
	CONFIG_MW5_OEM_WLAN_ORDER=y \
	CONFIG_MW5_FORCE_WLAN_DEVICE_TABLE=y \
	CONFIG_BAND_5G_ON_WLAN0=y \
	CONFIG_RTL_5G_SLOT_1=y \
	CONFIG_SOC_WIFI=y \
	CONFIG_SOC_RFE_TYPE_0=y \
	CONFIG_USE_PCIE_SLOT_1=y \
	CONFIG_SLOT_1_8822BE=y \
	CONFIG_SLOT_1_RFE_TYPE_0=y \
	CONFIG_RTL8192CD_LINUX6_BUS=y \
	CONFIG_PHY_EAT_40MHZ=y \
	CONFIG_WLAN_MACHAL_API_V0=y

# OpenWrt 24.10 supplies cfg80211 from the mac80211/backports tree.  The MW5
# module must compile against those headers rather than the native 6.6 cfg80211
# headers; the backport API level in this tree is 6.12.96.
RTL8197F_KBUILD_FLAGS_mw5:= \
	NOSTDINC_FLAGS="$(KERNEL_NOSTDINC_FLAGS) \
	-I$(PKG_BUILD_DIR) \
	-I$(STAGING_DIR)/usr/include/mac80211-backport \
	-I$(STAGING_DIR)/usr/include/mac80211-backport/uapi \
	-I$(STAGING_DIR)/usr/include/mac80211 \
	-I$(STAGING_DIR)/usr/include/mac80211/uapi \
	-include backport/backport.h \
	-DRTL8192CD_CFG80211_BACKPORT_612"

RTL8197F_KBUILD_FLAGS_wrd12gn:=$(RTL8197F_KBUILD_FLAGS_mw5)

define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)" \
		$(RTL8197F_COMMON_VARS) \
		$(RTL8197F_VARIANT_VARS_$(BUILD_VARIANT)) \
		$(RTL8197F_KBUILD_FLAGS_$(BUILD_VARIANT)) \
		KCFLAGS="-I$(LINUX_DIR) -include $(PKG_BUILD_DIR)/rtl8197f_linux_compat.h -Wno-error -Wno-implicit-function-declaration -Wno-int-conversion -Wno-address-of-packed-member" \
		modules
endef

define KernelPackage/rtl8192cd-rtl8197f/install-linux6
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./files/usr/bin/rtl8197f-wlan-check $(1)/usr/bin/
endef

define KernelPackage/rtl8192cd-rtl8197f/install-legacy
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/sbin $(1)/etc
	$(INSTALL_BIN) ./files/usr/bin/rtl8197f-wlan-check $(1)/usr/bin/
	$(INSTALL_BIN) ./files/usr/sbin/rtl8197f-wlan-load $(1)/usr/sbin/
	$(INSTALL_BIN) ./files/usr/sbin/rtl8197f-wlan-ap-test $(1)/usr/sbin/
	$(INSTALL_CONF) ./files/etc/rtl8197f-wlan-profiles.conf $(1)/etc/
endef

define KernelPackage/rtl8192cd-rtl8197f-mw5/install
$(call KernelPackage/rtl8192cd-rtl8197f/install-linux6,$(1))
endef

define KernelPackage/rtl8192cd-rtl8197f-rd05/install
$(call KernelPackage/rtl8192cd-rtl8197f/install-legacy,$(1))
endef

define KernelPackage/rtl8192cd-rtl8197f-ac23/install
$(call KernelPackage/rtl8192cd-rtl8197f/install-legacy,$(1))
endef

define KernelPackage/rtl8192cd-rtl8197f-wrd12gn/install
$(call KernelPackage/rtl8192cd-rtl8197f/install-linux6,$(1))
endef

$(eval $(call KernelPackage,rtl8192cd-rtl8197f-mw5))
$(eval $(call KernelPackage,rtl8192cd-rtl8197f-rd05))
$(eval $(call KernelPackage,rtl8192cd-rtl8197f-ac23))
$(eval $(call KernelPackage,rtl8192cd-rtl8197f-wrd12gn))
