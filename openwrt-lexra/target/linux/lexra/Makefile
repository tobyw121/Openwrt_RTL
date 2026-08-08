# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk

ARCH:=mips
BOARD:=Lexra
BOARDNAME:=Realtek Lexra MIPS
FEATURES:=pci ramdisk squashfs gpio
SUBTARGETS:=8197f

KERNEL_PATCHVER:=5.10

define Target/Description
	Build firmware images for Realtek Lexra based boards.
endef

include $(INCLUDE_DIR)/target.mk

FEATURES := $(filter-out mips16,$(FEATURES))

DEFAULT_PACKAGES += kmod-leds-gpio kmod-gpio-button-hotplug

$(eval $(call BuildTarget))
