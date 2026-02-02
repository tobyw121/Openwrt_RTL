# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

ARCH:=mips
CPU_TYPE:=4kec
BOARD:=realtek
BOARDNAME:=Realtek MIPS
DEVICE_TYPE:=basic
FEATURES:=ramdisk squashfs
SUBTARGETS:=generic

KERNEL_PATCHVER:=4.4

define Target/Description
	Build firmware images for Realtek RTL819x based boards.
endef

include $(INCLUDE_DIR)/target.mk

FEATURES := $(filter-out mips16,$(FEATURES))

DEFAULT_PACKAGES += uboot-envtools ethtool kmod-gpio-button-hotplug \
	dnsmasq firewall ip6tables iptables odhcp6c odhcpd-ipv6only \
	ip-full ip-bridge tc

DIR_BOARD := $(TOPDIR)/target/linux/target
DIR_ROMFS := $(TOPDIR)/target/linux/target/romfs
DIR_TMPFS := $(TOPDIR)/target/linux/target/tmpfs
DIR_IMAGE := $(TOPDIR)/target/linux/target/image
DIR_TOOLS := $(TOPDIR)/target/linux/target/tools
DIR_ROOT  := $(TOPDIR)/target/linux
DIR_USERS :=
DIR_BZBOX :=
DIR_LINUX := $(TOPDIR)/target/linux/linux-$(KERNEL_PATCHVER)
WIFI_DRV_PATH := drivers/net/wireless/realtek/rtl8192cd

export DIR_BOARD DIR_ROMFS DIR_TMPFS DIR_IMAGE DIR_TOOLS DIR_LINUX WIFI_DRV_PATH

$(eval $(call BuildTarget))
