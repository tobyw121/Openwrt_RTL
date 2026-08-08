ifeq ($(CONFIG_PLATFORM_RTL8197F), y)

$(info Building Realtek G6 wifi driver for Realtek RTL8197F...)

CONFIG_PHL_TEST_SUITE = y
CONFIG_RTW_SUPPORT_MBSSID_VAP = y

$(info Enabled loading PHY parameters from file)
CONFIG_LOAD_PHY_PARA_FROM_FILE = y

EXTRA_CFLAGS += -DCONFIG_LITTLE_ENDIAN -DCONFIG_PLATFORM_RTL8197F
EXTRA_CFLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT

EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_PRE_CONFIG
EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_POST_CONFIG
EXTRA_CFLAGS += -I$(src)/platform/mips_97f

ifeq ($(CONFIG_RTW_ENABLE_CUSTOM_PARA_PATH), y)
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=CONFIG_RTW_CUSTOM_PARA_PATH
else
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=\"/etc/conf/\"
endif

EXTRA_CFLAGS += -DCONFIG_FIRMWARE_PATH=\"/hw_setting/\"

DIR_LINUX ?= $(shell pwd)/../../../../../../backports-5.2.8-1
export DIR_LINUX
ARCH ?= mips
CROSS_COMPILE ?= /toolchain/msdk-4.8.5-mips-EB-4.4-u0.9.33-m32ut-180206/bin/msdk-linux- 
KSRC := $(DIR_LINUX)

# Suppress warnings for simple zero initialization "= {0}".
EXTRA_CFLAGS += -Wno-missing-braces

_PLATFORM_FILES :=

ifeq ($(CONFIG_PCI_HCI), y)
EXTRA_CFLAGS += -DCONFIG_PLATFORM_OPS
_PLATFORM_FILES += platform/platform_mips_97f_pci.o
endif

# For storage of calibration and configurations
ifeq ($(call check_config,RTW_DRV_HAS_NVM), y)
include $(src)/platform/rtk_ap/nvm.mk
endif

OBJS += $(_PLATFORM_FILES)

$(shell mkdir -p $(DIR_ROMFS)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8852ae $(DIR_ROMFS)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8852ce $(DIR_ROMFS)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8192xbe $(DIR_ROMFS)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8832bre $(DIR_ROMFS)/etc/conf)
endif
