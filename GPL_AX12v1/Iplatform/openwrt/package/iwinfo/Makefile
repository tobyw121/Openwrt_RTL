#
# Copyright (c) 2013 The Linux Foundation. All rights reserved.
# Copyright (C) 2010-2012 Jo-Philipp Wich <xm@subsignal.org>
#
# This is free software, licensed under the GPL 2 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libiwinfo
PKG_RELEASE:=36

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
PKG_CONFIG_DEPENDS := \
	CONFIG_PACKAGE_kmod-brcm-wl \
	CONFIG_PACKAGE_kmod-brcm-wl-mini \
	CONFIG_PACKAGE_kmod-brcm-wl-mimo \
	CONFIG_PACKAGE_kmod-madwifi \
	CONFIG_TARGET_ramips \
	CONFIG_PACKAGE_kmod-qca-wifi-perf \
	CONFIG_PACKAGE_kmod-qca-wifi-akronite-perf \
	CONFIG_PACKAGE_kmod-mac80211 \
	CONFIG_PACKAGE_kmod-qca-mac80211

PKG_BUILD_DEPENDS := \
      PACKAGE_kmod-qca-wifi-akronite-perf:kmod-qca-wifi-akronite-perf +PACKAGE_kmod-qca-wifi-perf:kmod-qca-wifi-perf

include $(INCLUDE_DIR)/package.mk


define Package/libiwinfo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Generalized Wireless Information Library (iwinfo)
  DEPENDS:=+PACKAGE_kmod-mac80211:libnl-tiny  +PACKAGE_kmod-qca-mac80211:libnl-tiny \
           +PACKAGE_kmod-qca-wifi-perf:kmod-qca-wifi-perf +CONFIG_PACKAGE_kmod-qca-wifi-akronite-perf:kmod-qca-wifi-akronite-perf
  MAINTAINER:=Jo-Philipp Wich <xm@subsignal.org>
endef

define Package/libiwinfo/config
	source "$(SOURCE)/Config.in"
endef

define Package/libiwinfo/description
  Wireless information library with consistent interface for proprietary Broadcom,
  madwifi, mtk, rtk, nl80211 and wext driver interfaces.
endef


define Package/libiwinfo-lua
  SUBMENU:=Lua
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=libiwinfo Lua binding
  DEPENDS:=+libiwinfo +liblua +lua
  MAINTAINER:=Jo-Philipp Wich <xm@subsignal.org>
endef

define Package/libiwinfo-lua/description
  This is the Lua binding for the iwinfo library. It provides access to all enabled
  backends.
endef


define Package/iwinfo
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Generalized Wireless Information utility
  DEPENDS:=+libiwinfo
  MAINTAINER:=Jo-Philipp Wich <xm@subsignal.org>
endef

define Package/iwinfo/description
  Command line frontend for the wireless information library.
endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

IWINFO_BACKENDS := \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl),wl) \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl-mini),wl) \
	$(if $(CONFIG_PACKAGE_kmod-brcm-wl-mimo),wl) \
	$(if $(CONFIG_PACKAGE_kmod-madwifi),madwifi) \
	$(if $(CONFIG_PACKAGE_kmod-ramips),mtk) \
	$(if $(CONFIG_PACKAGE_kmod-qca-wifi-perf),qcawifi) \
	$(if $(CONFIG_PACKAGE_kmod-qca-wifi-akronite-perf),qcawifi) \
	$(if $(CONFIG_PACKAGE_kmod-mac80211),nl80211) \
	$(if $(CONFIG_PACKAGE_kmod-qca-mac80211),nl80211) \
	$(subst ",,$(CONFIG_IWINFO_WIRELESS_TYPE))

ifeq ($(CONFIG_TARGET_BOARD),"model_intel_grx350")
IWINFO_BACKENDS += intelwifi
endif

ifeq ($(CONFIG_TARGET_BOARD),"board_mtk798x")
IWINFO_BACKENDS += mtk
endif

BCM_SDK := $(if $(CONFIG_BCM_SDK7X),7X)
BCM_SDK += $(if $(CONFIG_BCM_SDK_AX),AX)
BCM_SDK += $(if $(CONFIG_BCM_2G_AX),SUPPORT_BRCM_2G_AX)
BCM_SDK += $(if $(CONFIG_BCM_5G_AX),SUPPORT_BRCM_5G_AX)
BCM_SDK += $(if $(CONFIG_BCM_2G_BE),SUPPORT_BRCM_2G_BE)
BCM_SDK += $(if $(CONFIG_BCM_5G_BE),SUPPORT_BRCM_5G_BE)

ifeq ($(CONFIG_IWINFO_USE_QCA_NL80211_SUBCMD),y)
TARGET_CFLAGS += -DUSE_QCA_NL80211_SUBCMD=1
endif

ifeq ($(CONFIG_IWINFO_PRODUCT_WA3001),y)
TARGET_CFLAGS += -DATH_SUPPORT_IQUE
TARGET_CFLAGS += -DUMAC_SUPPORT_STA_STATS_ENHANCEMENT=1
TARGET_CFLAGS += -DDATH_SUPPORT_EXT_STAT=1
#TARGET_CFLAGS += -DNO_NL80211_SCAN=1
endif

ifneq ($(CONFIG_BCM_WL_STA_INFO_VER),)
TARGET_CFLAGS += -DBCM_WL_STA_INFO_VER=$(CONFIG_BCM_WL_STA_INFO_VER)
endif

ifneq ($(CONFIG_BCM_WL_STA_INFO_SUBVER),)
TARGET_CFLAGS += -DBCM_WL_STA_INFO_SUBVER=$(CONFIG_BCM_WL_STA_INFO_SUBVER)
endif

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include \
	-D_GNU_SOURCE

ifeq ($(CONFIG_IWINFO_DFS_CHANNEL_SUPPORT),y)
TARGET_CFLAGS += -DDFS_CHANNEL_SUPPORT
endif

ifeq ($(CONFIG_IWINFO_WIFI_TRIBAND),y)
TARGET_CFLAGS += -DWIFI_TRIBAND_SUPPORT
endif

ifeq ($(CONFIG_IWINFO_DOT11_EHT_BE),y)
TARGET_CFLAGS += -DDOT11_EHT_BE
endif

ifeq ($(CONFIG_USE_SDK_NL_TINY),y)
TARGET_CFLAGS += -I$(SDK_KERNEL_DIR)/include/uapi/linux/mtk_nl80211_inc
TARGET_CFLAGS += -I$(SDK_STAGING_DIR)/usr/include/libnl-tiny
TARGET_CFLAGS += -DMTK_NETLINK_SUPPORT
TARGET_LDFLAGS += -L$(SDK_STAGING_DIR)/usr/lib -lnl-tiny
else
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny
endif

MAKE_FLAGS += \
	FPIC="$(FPIC)" \
	CFLAGS="$(TARGET_CFLAGS) $(DELTACFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	BACKENDS="$(IWINFO_BACKENDS)" \
	BCMFLAGS="$(BCM_SDK)"	\
	DBDCCFLAGS="$(CONFIG_PACKAGE_WIFI_DBDC_CHIP_TYPE)"

TARGET_LDFLAGS += -luci -lubox


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/iwinfo
	$(CP) $(PKG_BUILD_DIR)/include/iwinfo.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/include/iwinfo/* $(1)/usr/include/iwinfo/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libiwinfo.so $(1)/usr/lib/libiwinfo.so
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwinfo.so $(1)/usr/lib/lua/iwinfo.so
endef

define Package/libiwinfo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libiwinfo.so $(1)/usr/lib/libiwinfo.so
endef

define Package/libiwinfo-lua/install
	$(INSTALL_DIR) $(1)/usr/lib/lua
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwinfo.so $(1)/usr/lib/lua/iwinfo.so
endef

define Package/iwinfo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/iwinfo $(1)/usr/bin/iwinfo
endef

$(eval $(call BuildPackage,libiwinfo))
$(eval $(call BuildPackage,libiwinfo-lua))
$(eval $(call BuildPackage,iwinfo))
