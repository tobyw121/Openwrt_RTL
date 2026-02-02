#
# Copyright (C) 2007-2021 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=uboot
PKG_VERSION:=1.0.0
PKG_BUILD_PARALLEL:=1

MAKE_ARGS:= CROSS="$(call qstrip,$(CONFIG_TOOLCHAIN_UBOOT_ROOT))/bin/$(TARGET_CROSS)"

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/uboot
  SECTION:=uboot
  CATEGORY:=uboot source
  TITLE:=uboot
  VERSION:=$(PKG_VERSION)
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/uboot/description
	bootloader
endef

define Build/Prepare	
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./$(BOARD)/$(SUBTARGET)/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./$(BOARD)/$(SUBTARGET)/$(BOARD)-$(SUBTARGET)-config $(PKG_BUILD_DIR)/.config
	$(MAKE) -C "$(PKG_BUILD_DIR)" $(MAKE_ARGS) oldconfig
	$(CP) $(PKG_BUILD_DIR)/autoconf.h $(PKG_BUILD_DIR)/boot/include/linux/autoconf2.h
endef

define Build/Compile
	$(MAKE) -C "$(PKG_BUILD_DIR)" $(MAKE_ARGS) -j`nproc` all
endef

define Package/uboot/install
	$(CP) $(PKG_BUILD_DIR)/uboot.bin $(KERNEL_BUILD_DIR)/
	$(CP) $(PKG_BUILD_DIR)/uboot_sig.bin $(KERNEL_BUILD_DIR)/
endef

$(eval $(call BuildPackage,uboot))
