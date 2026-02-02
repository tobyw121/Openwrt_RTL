#
# Copyright (C) 2007-2021 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=backports
PKG_VERSION:=5.2.8-1
PKG_RELEASE:=5.2.8-1
PKG_BUILD_PARALLEL:=1

MAKE_ARGS:= \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	KLIB_BUILD="$(LINUX_DIR)"

include $(INCLUDE_DIR)/package.mk

ifeq "$(wildcard $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8197g)" ""
RTL8192FE_SRC:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8192fe
else
RTL8192FE_SRC:= $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8197g
endif

define KernelPackage/backports
  SECTION:=kernel
  CATEGORY:=backports drivers
  TITLE:=backports
  VERSION:=$(LINUX_VERSION)+$(PKG_VERSION)
  FILES:= $(PKG_BUILD_DIR)/compat/compat.ko $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/g6_wifi_driver/rtk_wifi6.ko \
  $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8192fe/rtl8192cd.ko $(PKG_BUILD_DIR)/net/wireless/cfg80211.ko
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define KernelPackage/backports/description
	wireless drivers module
endef

define Build/Prepare	
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
	$(RM) $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/g6_wifi_driver
	$(CP) $(TOPDIR)/target/linux/linux-4.4/drivers/net/wireless/realtek/g6_wifi_driver $(PKG_BUILD_DIR)/drivers/net/wireless/realtek
	$(RM) $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/g6_wifi_driver_mp
	-$(CP) $(TOPDIR)/target/linux/linux-4.4/drivers/net/wireless/realtek/g6_wifi_driver_mp $(PKG_BUILD_DIR)/drivers/net/wireless/realtek
	$(RM) $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8723fu
	-$(CP) $(TOPDIR)/target/linux/linux-4.4/drivers/net/wireless/realtek/rtl8723fu $(PKG_BUILD_DIR)/drivers/net/wireless/realtek
	$(RM) $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/ther_ctrl
	$(CP) $(TOPDIR)/target/linux/linux-4.4/drivers/net/wireless/realtek/ther_ctrl $(PKG_BUILD_DIR)/drivers/net/wireless/realtek
	$(RM) $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/wfo_virt
	-$(CP) $(TOPDIR)/target/linux/linux-4.4/drivers/net/wireless/realtek/wfo_virt $(PKG_BUILD_DIR)/drivers/net/wireless/realtek
	if [ ! -d $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8723fu ]; then \
		sed -i '/realtek\/rtl8723fu/s/^/#/' $(PKG_BUILD_DIR)/drivers/net/wireless/Kconfig; \
	else \
		sed -i '/realtek\/rtl8723fu/s/^#+//' $(PKG_BUILD_DIR)/drivers/net/wireless/Kconfig; \
	fi
	if [ ! -d $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8192fe ]; then \
		sed -i '/realtek\/rtl8192fe/s/^/#/' $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/Kconfig; \
	else \
		sed -i '/realtek\/rtl8192fe/s/^#+//' $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/Kconfig; \
	fi
	if [ ! -d $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/rtl8197g ]; then \
		sed -i '/realtek\/rtl8197g/s/^/#/' $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/Kconfig; \
	else \
		sed -i '/realtek\/rtl8197g/s/^#+//' $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/Kconfig; \
	fi
endef

define Build/Configure
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./config/config.backports-5.2.8-1.RTL8197F_VG_8832BR_8367R_KERN44_GW $(PKG_BUILD_DIR)/.config
	make -C "$(PKG_BUILD_DIR)" $(MAKE_ARGS) backport-include/backport/autoconf.h
endef

define Build/Compile
	sed -i "/^[^#]*g6_wifi_driver_mp/ s/^/#/g" $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/Kconfig && \
	if [ ! -f $(PKG_BUILD_DIR)/.local-sym-updated ]; then \
		echo -n "Update backports local-symbols..."; \
		cp $(PKG_BUILD_DIR)/local-symbols $(PKG_BUILD_DIR)/local-symbols.old; \
		sed -n "/^[\t| ]*config/p" $(PKG_BUILD_DIR)/drivers/net/wireless/realtek/g6_wifi_driver/Kconfig > $(PKG_BUILD_DIR)/.kconf.tmp && \
		sed -i "s/^[\t| ]*config /CPTCFG_/" $(PKG_BUILD_DIR)/.kconf.tmp && \
		sed -i "s/$$$$/&=/g" $(PKG_BUILD_DIR)/.kconf.tmp && \
		cat $(PKG_BUILD_DIR)/.kconf.tmp >> $(PKG_BUILD_DIR)/local-symbols && \
		rm -f $(PKG_BUILD_DIR)/.kconf.tmp && touch $(PKG_BUILD_DIR)/.local-sym-updated;\
		echo "done."; \
	fi && \
	if [ -d $(RTL8192FE_SRC) ]; then \
		cp $(PKG_BUILD_DIR)/.config $(RTL8192FE_SRC)/config_8192cd_ax1500; \
		sed -i '/#/d' $(RTL8192FE_SRC)/config_8192cd_ax1500; \
		sed -i 's/CPTCFG_/CONFIG_/g' $(RTL8192FE_SRC)/config_8192cd_ax1500; \
		cp $(PKG_BUILD_DIR)/.config $(RTL8192FE_SRC)/config_8192cd.h; \
		truncate -s 0 $(RTL8192FE_SRC)/config_8192cd.h; \
	fi && \
	$(MAKE) -C "$(PKG_BUILD_DIR)" $(MAKE_ARGS) -j`nproc` $(if $(findstring s,$(OPENWRT_VERBOSE)),V=1,)
endef

$(eval $(call KernelPackage,backports))
