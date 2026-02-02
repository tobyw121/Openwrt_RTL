ifeq ($(CONFIG_PLATFORM_MIPS_PON), y)
EXTRA_CFLAGS += -DCONFIG_BIG_ENDIAN
EXTRA_CFLAGS += -DCONFIG_IOCTL_CFG80211 -DRTW_USE_CFG80211_STA_EVENT

EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_PRE_CONFIG
EXTRA_CFLAGS += -DCONFIG_RTW_HAS_PLATFORM_POST_CONFIG
EXTRA_CFLAGS += -I$(src)/platform/mips_pon

CONFIG_PHL_TEST_SUITE = y
CONFIG_RTW_SUPPORT_MBSSID_VAP = y

$(info Enabled loading PHY parameters from file)
CONFIG_LOAD_PHY_PARA_FROM_FILE = y

ifeq ($(CONFIG_RTW_ENABLE_CUSTOM_PARA_PATH), y)
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=CONFIG_RTW_CUSTOM_PARA_PATH
else
	EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=\"/etc/conf/\"
endif

EXTRA_CFLAGS += -DCONFIG_FIRMWARE_PATH=\"/etc/conf/\"

SUBARCH := $(shell uname -m | sed -e s/i.86/i386/)
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?=
KVER  := $(shell uname -r)
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/
INSTALL_PREFIX :=
STAGINGMODDIR := /lib/modules/$(KVER)/kernel/drivers/staging

_PLATFORM_FILES :=

ifeq ($(CONFIG_PCI_HCI), y)
EXTRA_CFLAGS += -DCONFIG_PLATFORM_OPS
_PLATFORM_FILES += platform/platform_linux_mips_pon_pci.o
endif

# For storage of calibration and configurations
ifeq ($(call check_config,RTW_DRV_HAS_NVM), y)
include $(src)/platform/rtk_ap/nvm.mk
endif

OBJS += $(_PLATFORM_FILES)

$(shell mkdir -p $(ROMFSDIR)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8852ae $(ROMFSDIR)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8852ce $(ROMFSDIR)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8192xbe $(ROMFSDIR)/etc/conf)
$(shell cp -rf $(src)/platform/mips_98d/rtl8832bre $(ROMFSDIR)/etc/conf)
endif
