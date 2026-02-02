#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=wireguard
PKG_RELEASE:=1.0
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)

define KernelPackage/wireguard
  SUBMENU:=Other modules
  TITLE:=wireguard
  FILES:=$(PKG_BUILD_DIR)/wireguard.ko
  AUTOLOAD:=$(call AutoLoad,80,wireguard)
  KCONFIG:=
endef

define KernelPackage/wireguard/description
  Kernel module to establish WireGuard VPN
endef

EXTRA_KCONFIG:= \
	CONFIG_WIREGUARD=m

EXTRA_CFLAGS:= \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=m,%,$(filter %=m,$(EXTRA_KCONFIG)))) \
	$(patsubst CONFIG_%, -DCONFIG_%=1, $(patsubst %=y,%,$(filter %=y,$(EXTRA_KCONFIG)))) \
#for h files are not under kernel dir sometimes
ifdef CONFIG_LINUX_KERNEL_INCLUDE_PATH
	EXTRA_CFLAGS += -I$(LINUX_DIR)/../$(CONFIG_LINUX_KERNEL_INCLUDE_PATH)
endif

ifneq (,$(findstring rtl8197,$(BOARD)))
	EXTRA_CFLAGS += -DNO_KERNEL_COMPAT
endif

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	M="$(PKG_BUILD_DIR)" \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	$(EXTRA_KCONFIG)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		$(MAKE_OPTS) \
		modules
endef

define KernelPackage/wireguard/install
	
endef

$(eval $(call KernelPackage,wireguard))
