EXTRA_CFLAGS += -O1
#EXTRA_CFLAGS += -O3
#EXTRA_CFLAGS += -Wall
#EXTRA_CFLAGS += -Wextra
src:=$(srctree)/drivers/net/wireless/realtek/g6_wifi_driver
DRV_PATH:=$(src)
EXTRA_CFLAGS += -Werror
EXTRA_CFLAGS += -Wvla
EXTRA_CFLAGS += -Wframe-larger-than=2048
#EXTRA_CFLAGS += -pedantic
#EXTRA_CFLAGS += -Wshadow -Wpointer-arith -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes

EXTRA_CFLAGS += -Wno-unused-variable
#EXTRA_CFLAGS += -Wno-unused-value
EXTRA_CFLAGS += -Wno-unused-label
#EXTRA_CFLAGS += -Wno-unused-parameter
#EXTRA_CFLAGS += -Wno-unused-function
EXTRA_CFLAGS += -Wno-unused
#EXTRA_CFLAGS += -Wno-uninitialized

GCC_VER_49 := $(shell echo `$(CC) -dumpversion | cut -f1-2 -d.` \>= 4.9 | bc )
ifeq ($(GCC_VER_49),1)
EXTRA_CFLAGS += -Wno-date-time	# Fix compile error && warning on gcc 4.9 and later
endif

EXTRA_CFLAGS += -g -I$(src)/include

EXTRA_LDFLAGS += --strip-debug

CONFIG_AUTOCFG_CP = y

check_config = $(findstring y,$(value CONFIG_$(1))$(value CONFIG_$(1)))

########################## WIFI IC ############################
ifeq ($(call check_config,RTL8852AE), y)
CONFIG_RTL8852A = y
else
CONFIG_RTL8852A = n
endif

ifeq ($(call check_config,RTL8852BE), y)
CONFIG_RTL8852B = y
else
CONFIG_RTL8852B = n
endif

ifeq ($(call check_config,RTL8852CE), y)
CONFIG_RTL8852C = y
else
CONFIG_RTL8852C = n
endif

ifeq ($(call check_config,RTL8832BR), y)
CONFIG_RTL8832BR = y
else
CONFIG_RTL8832BR = n
endif

ifeq ($(call check_config,RTL8192XB), y)
CONFIG_RTL8192XB = y
else
CONFIG_RTL8192XB = n
endif

######################### Interface ###########################
CONFIG_USB_HCI = n
CONFIG_PCI_HCI = y
CONFIG_SDIO_HCI = n
CONFIG_GSPI_HCI = n
########################## Features ###########################
#CONFIG_MP_INCLUDED = n
CONFIG_RTW_SUPPORT_MBSSID_VAP = y
CONFIG_CONCURRENT_MODE = y
CONFIG_POWER_SAVING = n
CONFIG_IPS_MODE = default
CONFIG_LPS_MODE = default
CONFIG_BTC = n
CONFIG_WAPI_SUPPORT = n
CONFIG_EFUSE_CONFIG_FILE = y
CONFIG_EXT_CLK = n
CONFIG_TRAFFIC_PROTECT = n
CONFIG_LOAD_PHY_PARA_FROM_FILE = n
CONFIG_FILE_FWIMG = y
CONFIG_TXPWR_BY_RATE = n
CONFIG_TXPWR_BY_RATE_EN = auto
CONFIG_TXPWR_LIMIT = n
CONFIG_TXPWR_LIMIT_EN = auto
CONFIG_RTW_CHPLAN = 0xFFFF
CONFIG_RTW_ADAPTIVITY_EN = disable
CONFIG_RTW_ADAPTIVITY_MODE = normal
CONFIG_SIGNAL_SCALE_MAPPING = n
CONFIG_80211W = y
CONFIG_REDUCE_TX_CPU_LOADING = n
CONFIG_BR_EXT = y
CONFIG_TDLS = n
CONFIG_WIFI_MONITOR = n
CONFIG_MCC_MODE = n
CONFIG_APPEND_VENDOR_IE_ENABLE = y
CONFIG_WLAN_EVENT_INDICATE_GENL = n
CONFIG_RTW_NAPI = n
CONFIG_RTW_GRO = n
CONFIG_RTW_NETIF_SG = n
CONFIG_RTW_IPCAM_APPLICATION = n
CONFIG_ICMP_VOQ = n
CONFIG_IP_R_MONITOR = n #arp VOQ and high rate
# user priority mapping rule : tos, dscp
CONFIG_RTW_UP_MAPPING_RULE = tos
CONFIG_FORCE_QOS_SUPPORT = y

CONFIG_PHL_ARCH = y
CONFIG_HWSIM = n
CONFIG_FSM = n
CONFIG_CMD_DISP = y

#CONFIG_PHL_TEST_SUITE = n
CONFIG_WIFI_6 = y

RTW_PHL_RX = y
RTW_PHL_TX = y
RTW_PHL_BCN = y
DIRTY_FOR_WORK = y

CONFIG_DRV_FAKE_AP = n

CONFIG_DBG_AX_CAM = y

USE_TRUE_PHY = y

########################## Android ###########################
# CONFIG_RTW_ANDROID - 0: no Android, 4/5/6/7/8/9/10 : Android version
CONFIG_RTW_ANDROID = 0

ifeq ($(shell test $(CONFIG_RTW_ANDROID) -gt 0; echo $$?), 0)
EXTRA_CFLAGS += -DCONFIG_RTW_ANDROID=$(CONFIG_RTW_ANDROID)
endif

########################## Debug ###########################
CONFIG_RTW_DEBUG = y
# default log level is _DRV_INFO_ = 4,
# please refer to "How_to_set_driver_debug_log_level.doc" to set the available level.
CONFIG_RTW_LOG_LEVEL = 2

# enable /proc/net/rtlxxxx/ debug interfaces
CONFIG_PROC_DEBUG = y

#enable BB debug message. Ex: dbg 30
# CONFIG_BB_DBG_TRACE_SUPPORT = y

# enable mirror dump debug
CONFIG_RTW_MIRROR_DUMP = n

######################## Wake On Lan ##########################
CONFIG_WOWLAN = n
#bit2: deauth, bit1: unicast, bit0: magic pkt.
CONFIG_WAKEUP_TYPE = 0x7
CONFIG_WOW_LPS_MODE = default
#bit0: disBBRF off, #bit1: Wireless remote controller (WRC)
CONFIG_SUSPEND_TYPE = 0
CONFIG_WOW_STA_MIX = n
CONFIG_GPIO_WAKEUP = n
# Please contact with RTK support team first. After getting the agreement from RTK support team, 
# you are just able to modify the CONFIG_WAKEUP_GPIO_IDX with customized requirement.
CONFIG_WAKEUP_GPIO_IDX = default
CONFIG_HIGH_ACTIVE_DEV2HST = n
######### only for USB #########
CONFIG_ONE_PIN_GPIO = n
CONFIG_HIGH_ACTIVE_HST2DEV = n
CONFIG_PNO_SUPPORT = n
CONFIG_PNO_SET_DEBUG = n
CONFIG_AP_WOWLAN = n
######### Notify SDIO Host Keep Power During Syspend ##########
CONFIG_RTW_SDIO_PM_KEEP_POWER = y
###################### MP HW TX MODE FOR VHT #######################
CONFIG_MP_VHT_HW_TX_MODE = n

###################### Platform Related #######################
CONFIG_PLATFORM_I386_PC = n
CONFIG_PLATFORM_RTL8198D = n
CONFIG_PLATFORM_MIPS_PON = n
CONFIG_PLATFORM_MIPS_XDSL = n
CONFIG_PLATFORM_ANDROID_X86 = n
CONFIG_PLATFORM_ANDROID_INTEL_X86 = n
CONFIG_PLATFORM_NV_TK1 = n
CONFIG_PLATFORM_NV_TK1_UBUNTU = n
CONFIG_PLATFORM_ARM_SUNxI = n

########### RTK_WFO ################################
ifeq ($(CONFIG_WFO_OFFLOAD_2G), y)
WFO_OFFLOAD_2G=y
else ifeq ($(CONFIG_WFO_OFFLOAD_2G), y)
WFO_OFFLOAD_2G=y
else
WFO_OFFLOAD_2G=n
endif

ifeq ($(CONFIG_WFO_OFFLOAD_5G), y)
WFO_OFFLOAD_5G=y
else ifeq ($(CONFIG_WFO_OFFLOAD_5G), y)
WFO_OFFLOAD_5G=y
else
WFO_OFFLOAD_5G=n
endif

ifeq ($(WFO_OFFLOAD_2G), y)
RTK_WFO=y
else ifeq ($(WFO_OFFLOAD_5G), y)
RTK_WFO=y
else
RTK_WFO=n
endif

ifeq ($(CONFIG_WFO_VIRT), m)
WFO_VIRT=m
else ifeq ($(CONFIG_WFO_VIRT), m)
WFO_VIRT=m
else
WFO_VIRT=n
endif

ifeq ($(RTK_WFO), y)
ifeq ($(WFO_VIRT), m)
  EXTRA_CFLAGS += -DWFO_VIRT_MODULE
  ifeq ($(CONFIG_WFO_VIRT_SAME_CPU), y)
  EXTRA_CFLAGS += -DWFO_RADIO_SENDER
  EXTRA_CFLAGS += -DWFO_VIRT_RECEIVER
  endif
else
  EXTRA_CFLAGS += -DCONFIG_RTK_WFO_NO_VIRT
  EXTRA_CFLAGS += -DWFO_RADIO_SENDER
  EXTRA_CFLAGS += -DWFO_VIRT_RECEIVER
  EXTRA_CFLAGS += -DWFO_VIRT_SENDER
  EXTRA_CFLAGS += -DWFO_RADIO_RECEIVER
endif # CONFIG_WFO_VIRT
endif # CONFIG_RTK_WFO

########### CUSTOMER ################################

CONFIG_DRVEXT_MODULE = n

ifeq ($(src),)
export TopDIR ?= $(shell pwd)
export src ?= $(TopDIR)
else
export TopDIR ?= $(src)
endif

# Auto select the right platform if supported
include $(src)/platform.mk

########### COMMON  #################################
check_config = $(findstring y,$(value CONFIG_$(1))$(value CONFIG_$(1)))

ifeq ($(CONFIG_GSPI_HCI), y)
HCI_NAME = gspi
endif

ifeq ($(CONFIG_SDIO_HCI), y)
HCI_NAME = sdio
endif

ifeq ($(CONFIG_USB_HCI), y)
HCI_NAME = usb
endif

ifeq ($(CONFIG_PCI_HCI), y)
HCI_NAME = pci
endif

ifeq ($(CONFIG_HWSIM), y)
	HAL = hal_sim
else
	HAL = phl
endif

########### HAL_RTL8852A #################################
ifeq ($(CONFIG_RTL8852A), y)
IC_NAME := rtl8852a
ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8852au
endif
ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8852ae
endif
ifeq ($(CONFIG_SDIO_HCI), y)
MODULE_NAME = 8852as
endif

endif

########### HAL_RTL8852B #################################
ifeq ($(CONFIG_RTL8852B), y)
IC_NAME := rtl8852b
ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8852bu
endif
ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8852be
endif
ifeq ($(CONFIG_SDIO_HCI), y)
MODULE_NAME = 8852bs
endif

endif

########### HAL_RTL8852C #################################
ifeq ($(CONFIG_RTL8852C), y)
IC_NAME := rtl8852c
ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8852cu
endif
ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8852ce
endif
ifeq ($(CONFIG_SDIO_HCI), y)
MODULE_NAME = 8852cs
endif

endif

########### HAL_RTL8832BR #################################
ifeq ($(CONFIG_RTL8832BR), y)
IC_NAME := rtl8832br
#EXTRA_CFLAGS += -DRTW_WKARD_UPDATE_DMAC_KEYID
ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8832bu
endif
ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8832br
endif
ifeq ($(CONFIG_SDIO_HCI), y)
MODULE_NAME = 8832bs
endif

endif

########### HAL_RTL8192XB #################################
ifeq ($(CONFIG_RTL8192XB), y)
IC_NAME := rtl8192xb
#EXTRA_CFLAGS += -DRTW_WKARD_UPDATE_DMAC_KEYID
ifeq ($(CONFIG_USB_HCI), y)
MODULE_NAME = 8192xbu
endif
ifeq ($(CONFIG_PCI_HCI), y)
MODULE_NAME = 8192xbr
endif
ifeq ($(CONFIG_SDIO_HCI), y)
MODULE_NAME = 8192xbs
endif

endif
########### FIX MODULE NAME ###############################
MODULE_NAME := rtk_wifi6

########### AUTO_CFG  #################################

ifeq ($(CONFIG_AUTOCFG_CP), y)
$(shell cp $(DRV_PATH)/autoconf_$(IC_NAME)_$(HCI_NAME)_linux.h $(DRV_PATH)/include/autoconf.h)
endif

########## PLATFORM MAKEFILE ##########################
# Import platform assigned KSRC and CROSS_COMPILE
include $(wildcard $(src)/platform/*.mk)

########### END OF PATH  #################################
ifeq ($(call check_config,PHL_TEST_SUITE), y)
EXTRA_CFLAGS += -DCONFIG_PHL_TEST_SUITE
endif

ifeq ($(call check_config,MP_INCLUDED), y)
#MODULE_NAME := $(MODULE_NAME)_mp
EXTRA_CFLAGS += -DCONFIG_MP_INCLUDED
EXTRA_CFLAGS += -DCONFIG_PHL_TEST_SUITE
EXTRA_CFLAGS += -DCONFIG_MP_CONFIG_FOR_AP
endif

ifeq ($(CONFIG_FSM), y)
EXTRA_CFLAGS += -DCONFIG_FSM
endif

ifeq ($(CONFIG_CMD_DISP), y)
EXTRA_CFLAGS += -DCONFIG_CMD_DISP
endif

ifeq ($(call check_config,SHARE_XSTAL), y)
EXTRA_CFLAGS += -DCONFIG_SHARE_XSTAL
endif

ifeq ($(call check_config,2G_25MHZ_XSTAL), y)
EXTRA_CFLAGS += -DCONFIG_2G_25MHZ_XSTAL
endif

ifeq ($(call check_config,2G_ON_PCIE_SLOT0), y)
EXTRA_CFLAGS += -DCONFIG_2G_ON_PCIE_SLOT0
endif

ifeq ($(CONFIG_RTW_SUPPORT_MBSSID_VAP), y)
EXTRA_CFLAGS += -DCONFIG_RTW_SUPPORT_MBSSID_VAP
endif

ifeq ($(CONFIG_CONCURRENT_MODE), y)
EXTRA_CFLAGS += -DCONFIG_CONCURRENT_MODE
endif

ifeq ($(CONFIG_POWER_SAVING), y)
ifneq ($(CONFIG_IPS_MODE), default)
EXTRA_CFLAGS += -DRTW_IPS_MODE=$(CONFIG_IPS_MODE)
endif
ifneq ($(CONFIG_LPS_MODE), default)
EXTRA_CFLAGS += -DRTW_LPS_MODE=$(CONFIG_LPS_MODE)
endif
ifneq ($(CONFIG_WOW_LPS_MODE), default)
EXTRA_CFLAGS += -DRTW_WOW_LPS_MODE=$(CONFIG_WOW_LPS_MODE)
endif
EXTRA_CFLAGS += -DCONFIG_POWER_SAVING
endif

ifeq ($(CONFIG_BTC), y)
EXTRA_CFLAGS += -DCONFIG_BTC
endif

ifeq ($(CONFIG_WAPI_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_WAPI_SUPPORT
endif

ifeq ($(CONFIG_RTL_CFG80211_WAPI_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_RTL_CFG80211_WAPI_SUPPORT
endif

ifeq ($(CONFIG_RTW_OPCLASS_CHANNEL_SCAN), y)
EXTRA_CFLAGS += -DCONFIG_RTW_OPCLASS_CHANNEL_SCAN
endif

ifeq ($(CONFIG_WIFI_6), y)
EXTRA_CFLAGS += -DCONFIG_WIFI_6
endif

ifeq ($(CONFIG_EFUSE_CONFIG_FILE), y)
EXTRA_CFLAGS += -DCONFIG_EFUSE_CONFIG_FILE

#EFUSE_MAP_PATH
USER_EFUSE_MAP_PATH ?=
ifneq ($(USER_EFUSE_MAP_PATH),)
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"$(USER_EFUSE_MAP_PATH)\"
else
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"/system/etc/wifi/wifi_efuse_$(MODULE_NAME).map\"
endif

#WIFIMAC_PATH
USER_WIFIMAC_PATH ?=
ifneq ($(USER_WIFIMAC_PATH),)
EXTRA_CFLAGS += -DWIFIMAC_PATH=\"$(USER_WIFIMAC_PATH)\"
else
EXTRA_CFLAGS += -DWIFIMAC_PATH=\"/data/wifimac.txt\"
endif

endif

ifeq ($(CONFIG_EXT_CLK), y)
EXTRA_CFLAGS += -DCONFIG_EXT_CLK
endif

ifeq ($(CONFIG_TRAFFIC_PROTECT), y)
EXTRA_CFLAGS += -DCONFIG_TRAFFIC_PROTECT
endif

ifeq ($(CONFIG_LOAD_PHY_PARA_FROM_FILE), y)
EXTRA_CFLAGS += -DCONFIG_LOAD_PHY_PARA_FROM_FILE
# Path is set in platform's Makefile
#EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH_WITH_IC_NAME_FOLDER
#EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=\"/lib/firmware/\"
endif

ifeq ($(CONFIG_FILE_FWIMG), y)
EXTRA_CFLAGS += -DCONFIG_FILE_FWIMG
# Path is set in platform's Makefile
# default 8852A firmware path fix to [REALTEK_CONFIG_PATH]rtl8852a[e/s/u]/rtl8852afw.bin
# and CONFIG_FIRMWARE_PATH is useless temporally from PHL formal_006
# EXTRA_CFLAGS += -DCONFIG_FIRMWARE_PATH=\"\"
# EXTRA_CFLAGS += -DCONFIG_FIRMWARE_PATH=\"/lib/firmware/\"
endif

ifeq ($(CONFIG_RTK_WLAN_EVENT_INDICATE), y)
EXTRA_CFLAGS += -DRTK_WLAN_EVENT_INDICATE
endif

ifeq ($(CONFIG_RTW_SBWC), y)
EXTRA_CFLAGS += -DSBWC
endif

ifeq ($(CONFIG_RTW_GBWC), y)
EXTRA_CFLAGS += -DGBWC
endif

ifeq ($(CONFIG_AP_NEIGHBOR_INFO), y)
EXTRA_CFLAGS += -DAP_NEIGHBOR_INFO
endif

ifneq ($findstring y,$(CONFIG_RTW_CURRENT_RATE_ACCOUNTING)$(CONFIG_RTW_CURRENT_RATE_ACCOUNTING),)
EXTRA_CFLAGS += -DRTW_CURRENT_RATE_ACCOUNTING
endif

ifeq ($(CONFIG_TXPWR_BY_RATE), n)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE=0
else ifeq ($(CONFIG_TXPWR_BY_RATE), y)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE=1
endif
ifeq ($(CONFIG_TXPWR_BY_RATE_EN), n)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE_EN=0
else ifeq ($(CONFIG_TXPWR_BY_RATE_EN), y)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE_EN=1
else ifeq ($(CONFIG_TXPWR_BY_RATE_EN), auto)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE_EN=2
endif

ifeq ($(CONFIG_TXPWR_LIMIT), n)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT=0
else ifeq ($(CONFIG_TXPWR_LIMIT), y)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT=1
endif
ifeq ($(CONFIG_TXPWR_LIMIT_EN), n)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT_EN=0
else ifeq ($(CONFIG_TXPWR_LIMIT_EN), y)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT_EN=1
else ifeq ($(CONFIG_TXPWR_LIMIT_EN), auto)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT_EN=2
endif

ifneq ($(CONFIG_RTW_CHPLAN), 0xFFFF)
EXTRA_CFLAGS += -DCONFIG_RTW_CHPLAN=$(CONFIG_RTW_CHPLAN)
endif

ifeq ($(CONFIG_CALIBRATE_TX_POWER_BY_REGULATORY), y)
EXTRA_CFLAGS += -DCONFIG_CALIBRATE_TX_POWER_BY_REGULATORY
endif

ifeq ($(CONFIG_CALIBRATE_TX_POWER_TO_MAX), y)
EXTRA_CFLAGS += -DCONFIG_CALIBRATE_TX_POWER_TO_MAX
endif

ifeq ($(CONFIG_RTW_ADAPTIVITY_EN), disable)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_EN=0
else ifeq ($(CONFIG_RTW_ADAPTIVITY_EN), enable)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_EN=1
endif

ifeq ($(CONFIG_RTW_ADAPTIVITY_MODE), normal)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_MODE=0
else ifeq ($(CONFIG_RTW_ADAPTIVITY_MODE), carrier_sense)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_MODE=1
endif

ifeq ($(CONFIG_SIGNAL_SCALE_MAPPING), y)
EXTRA_CFLAGS += -DCONFIG_SIGNAL_SCALE_MAPPING
endif

ifeq ($(call check_config,RTW_80211W_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_IEEE80211W
endif

ifeq ($(CONFIG_WOWLAN), y)
EXTRA_CFLAGS += -DCONFIG_WOWLAN -DRTW_WAKEUP_EVENT=$(CONFIG_WAKEUP_TYPE)
EXTRA_CFLAGS += -DRTW_SUSPEND_TYPE=$(CONFIG_SUSPEND_TYPE)
ifeq ($(CONFIG_WOW_STA_MIX), y)
EXTRA_CFLAGS += -DRTW_WOW_STA_MIX
endif
ifeq ($(CONFIG_SDIO_HCI), y)
EXTRA_CFLAGS += -DCONFIG_RTW_SDIO_PM_KEEP_POWER
endif
endif

ifeq ($(CONFIG_AP_WOWLAN), y)
EXTRA_CFLAGS += -DCONFIG_AP_WOWLAN
ifeq ($(CONFIG_SDIO_HCI), y)
EXTRA_CFLAGS += -DCONFIG_RTW_SDIO_PM_KEEP_POWER
endif
endif

ifeq ($(CONFIG_PNO_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_PNO_SUPPORT
ifeq ($(CONFIG_PNO_SET_DEBUG), y)
EXTRA_CFLAGS += -DCONFIG_PNO_SET_DEBUG
endif
endif

ifeq ($(CONFIG_GPIO_WAKEUP), y)
EXTRA_CFLAGS += -DCONFIG_GPIO_WAKEUP
ifeq ($(CONFIG_ONE_PIN_GPIO), y)
EXTRA_CFLAGS += -DCONFIG_RTW_ONE_PIN_GPIO
endif
ifeq ($(CONFIG_HIGH_ACTIVE_DEV2HST), y)
EXTRA_CFLAGS += -DHIGH_ACTIVE_DEV2HST=1
else
EXTRA_CFLAGS += -DHIGH_ACTIVE_DEV2HST=0
endif
endif

ifeq ($(CONFIG_HIGH_ACTIVE_HST2DEV), y)
EXTRA_CFLAGS += -DHIGH_ACTIVE_HST2DEV=1
else
EXTRA_CFLAGS += -DHIGH_ACTIVE_HST2DEV=0
endif

ifneq ($(CONFIG_WAKEUP_GPIO_IDX), default)
EXTRA_CFLAGS += -DWAKEUP_GPIO_IDX=$(CONFIG_WAKEUP_GPIO_IDX)
endif

ifeq ($(CONFIG_RTW_SDIO_PM_KEEP_POWER), y)
ifeq ($(CONFIG_SDIO_HCI), y)
EXTRA_CFLAGS += -DCONFIG_RTW_SDIO_PM_KEEP_POWER
endif
endif

ifeq ($(CONFIG_REDUCE_TX_CPU_LOADING), y)
EXTRA_CFLAGS += -DCONFIG_REDUCE_TX_CPU_LOADING
endif

ifeq ($(CONFIG_BR_EXT), y)
BR_NAME = br0
EXTRA_CFLAGS += -DCONFIG_BR_EXT
EXTRA_CFLAGS += '-DCONFIG_BR_EXT_BRNAME="'$(BR_NAME)'"'
endif


ifeq ($(CONFIG_TDLS), y)
EXTRA_CFLAGS += -DCONFIG_TDLS
endif

ifeq ($(CONFIG_WIFI_MONITOR), y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MONITOR
endif

ifeq ($(CONFIG_MCC_MODE), y)
EXTRA_CFLAGS += -DCONFIG_MCC_MODE
endif

ifeq ($(CONFIG_RTW_NAPI), y)
EXTRA_CFLAGS += -DCONFIG_RTW_NAPI
endif

ifeq ($(CONFIG_RTW_GRO), y)
EXTRA_CFLAGS += -DCONFIG_RTW_GRO
endif

ifeq ($(CONFIG_RTW_IPCAM_APPLICATION), y)
EXTRA_CFLAGS += -DCONFIG_RTW_IPCAM_APPLICATION
ifeq ($(CONFIG_WIFI_MONITOR), n)
EXTRA_CFLAGS += -DCONFIG_WIFI_MONITOR
endif
endif

ifeq ($(CONFIG_RTW_NETIF_SG), y)
EXTRA_CFLAGS += -DCONFIG_RTW_NETIF_SG
endif

ifeq ($(CONFIG_ICMP_VOQ), y)
EXTRA_CFLAGS += -DCONFIG_ICMP_VOQ
endif

ifeq ($(CONFIG_IP_R_MONITOR), y)
EXTRA_CFLAGS += -DCONFIG_IP_R_MONITOR
endif

ifeq ($(CONFIG_MP_VHT_HW_TX_MODE), y)
EXTRA_CFLAGS += -DCONFIG_MP_VHT_HW_TX_MODE
ifeq ($(CONFIG_PLATFORM_I386_PC), y)
## For I386 X86 ToolChain use Hardware FLOATING
EXTRA_CFLAGS += -mhard-float
else
## For ARM ToolChain use Hardware FLOATING
EXTRA_CFLAGS += -mfloat-abi=hard
endif
endif

ifeq ($(CONFIG_APPEND_VENDOR_IE_ENABLE), y)
EXTRA_CFLAGS += -DCONFIG_APPEND_VENDOR_IE_ENABLE
endif

ifeq ($(CONFIG_WLAN_EVENT_INDICATE_GENL), y)
EXTRA_CFLAGS += -DCONFIG_WLAN_EVENT_INDICATE_GENL
endif

ifeq ($(CONFIG_RTW_DEBUG), y)
EXTRA_CFLAGS += -DCONFIG_RTW_DEBUG
EXTRA_CFLAGS += -DRTW_LOG_LEVEL=$(CONFIG_RTW_LOG_LEVEL)
endif

ifeq ($(CONFIG_PROC_DEBUG), y)
EXTRA_CFLAGS += -DCONFIG_PROC_DEBUG
endif

ifeq ($(CONFIG_RTW_MIRROR_DUMP), y)
EXTRA_CFLAGS += -DCONFIG_RTW_MIRROR_DUMP
endif

ifeq ($(CONFIG_BB_DBG_TRACE_SUPPORT), y)
EXTRA_CFLAGS += -DDRV_BB_DBG_TRACE_SUPPORT
endif

ifeq ($(CONFIG_RTW_UP_MAPPING_RULE), dscp)
EXTRA_CFLAGS += -DCONFIG_RTW_UP_MAPPING_RULE=1
else
EXTRA_CFLAGS += -DCONFIG_RTW_UP_MAPPING_RULE=0
endif

ifeq ($(CONFIG_FORCE_QOS_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_FORCE_QOS_SUPPORT
endif

EXTRA_CFLAGS += -DPLATFORM_LINUX

ifeq ($(USE_TRUE_PHY), y)
EXTRA_CFLAGS += -DUSE_TRUE_PHY
endif

ifeq ($(CONFIG_HWSIM), y)
EXTRA_CFLAGS += -DCONFIG_HWSIM

# To use pure sw beacon
EXTRA_CFLAGS += -DCONFIG_SWTIMER_BASED_TXBCN
EXTRA_CFLAGS += -DCONFIG_SUPPORT_MULTI_BCN
endif

ifeq ($(CONFIG_DRV_FAKE_AP), y)
EXTRA_CFLAGS += -DCONFIG_DRV_FAKE_AP
OBJS += core/rtw_fake_ap.o
endif

ifeq ($(CONFIG_DBG_AX_CAM), y)
EXTRA_CFLAGS += -DCONFIG_DBG_AX_CAM
endif

########### PLATFORM OPS  ##########################
EXTRA_CFLAGS += -I$(src)/platform

ifeq ($(CONFIG_PROFILING), y)
ifeq ($(ARCH), mips)
OBJS += profiling/m1004kperf.o
else ifeq ($(ARCH), arm64)
OBJS += profiling/pmu_perf.o
endif
endif

########### CUSTOMER ################################
USER_MODULE_NAME ?=
ifneq ($(USER_MODULE_NAME),)
MODULE_NAME := $(USER_MODULE_NAME)
endif

EXTRA_LDFLAGS += -Map=$(obj)/$(MODULE_NAME).map

ifneq ($(KERNELRELEASE),)
########### COMMON #################################
include $(src)/common.mk

EXTRA_CFLAGS += -DPHL_PLATFORM_LINUX
EXTRA_CFLAGS += -DCONFIG_PHL_ARCH

ifeq ($(RTW_PHL_RX), y)
EXTRA_CFLAGS += -DRTW_PHL_RX
endif

ifeq ($(RTW_PHL_TX), y)
EXTRA_CFLAGS += -DRTW_PHL_TX
endif

ifeq ($(RTW_PHL_BCN), y)
EXTRA_CFLAGS += -DRTW_PHL_BCN
endif

ifeq ($(DIRTY_FOR_WORK), y)
EXTRA_CFLAGS += -DDIRTY_FOR_WORK
endif

########### Ther Control  ##########################
ifeq ($(CONFIG_THER_CTRL), y)
THER_CTRL=y
EXTRA_CFLAGS += -I$(srctree)/drivers/net/wireless/realtek/ther_ctrl/
else ifeq ($(CONFIG_THER_CTRL), y)
THER_CTRL=y
ifeq ($(CONFIG_RTL_8197F_VG),y)
EXTRA_CFLAGS += -I$(srctree)/drivers/net/wireless/realtek/ther_ctrl/
else
EXTRA_CFLAGS += -I$(srctree)/drivers/net/wireless/realtek/ther_ctrl/
endif
else
THER_CTRL=n
endif

ifeq ($(THER_CTRL), y)
EXTRA_CFLAGS += -DWIFI6_THER_CTRL -DRTW_STA_BWC
endif
####################################################

ifneq (,$(wildcard $(DRV_PATH)/phl/gen_git_info.sh))
$(shell $(DRV_PATH)/phl/gen_git_info.sh )
endif
include $(src)/phl/phl.mk


obj-$(CONFIG_RTLWIFI6) := $(MODULE_NAME).o
obj-$(CONFIG_RTLWIFI6) := $(MODULE_NAME).o
$(MODULE_NAME)-y = $(OBJS)

else

export CONFIG_RTLWIFI6 = m


all: modules

modules:
#REMOVE BEGIN
ifneq (,$(wildcard ./phl/gen_git_info.sh))
	#./phl/gen_git_info.sh $(DRV_PATH)
endif
#REMOVE END
	#rm -f .symvers.$(MODULE_NAME)

	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(shell pwd)  modules

	#cp Module.symvers .symvers.$(MODULE_NAME)

strip:
	$(CROSS_COMPILE)strip $(MODULE_NAME).ko --strip-unneeded

install:
	install -p -m 644 $(MODULE_NAME).ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

uninstall:
	rm -f $(MODDESTDIR)/$(MODULE_NAME).ko
	/sbin/depmod -a ${KVER}

backup_rtlwifi:
	@echo "Making backup rtlwifi drivers"
ifneq (,$(wildcard $(STAGINGMODDIR)/rtl*))
	@tar cPf $(wildcard $(STAGINGMODDIR))/backup_rtlwifi_driver.tar $(wildcard $(STAGINGMODDIR)/rtl*)
	@rm -rf $(wildcard $(STAGINGMODDIR)/rtl*)
endif
ifneq (,$(wildcard $(MODDESTDIR)realtek))
	@tar cPf $(MODDESTDIR)backup_rtlwifi_driver.tar $(MODDESTDIR)realtek
	@rm -fr $(MODDESTDIR)realtek
endif
ifneq (,$(wildcard $(MODDESTDIR)rtl*))
	@tar cPf $(MODDESTDIR)../backup_rtlwifi_driver.tar $(wildcard $(MODDESTDIR)rtl*)
	@rm -fr $(wildcard $(MODDESTDIR)rtl*)
endif
	@/sbin/depmod -a ${KVER}
	@echo "Please reboot your system"

restore_rtlwifi:
	@echo "Restoring backups"
ifneq (,$(wildcard $(STAGINGMODDIR)/backup_rtlwifi_driver.tar))
	@tar xPf $(STAGINGMODDIR)/backup_rtlwifi_driver.tar
	@rm $(STAGINGMODDIR)/backup_rtlwifi_driver.tar
endif
ifneq (,$(wildcard $(MODDESTDIR)backup_rtlwifi_driver.tar))
	@tar xPf $(MODDESTDIR)backup_rtlwifi_driver.tar
	@rm $(MODDESTDIR)backup_rtlwifi_driver.tar
endif
ifneq (,$(wildcard $(MODDESTDIR)../backup_rtlwifi_driver.tar))
	@tar xPf $(MODDESTDIR)../backup_rtlwifi_driver.tar
	@rm $(MODDESTDIR)../backup_rtlwifi_driver.tar
endif
	@/sbin/depmod -a ${KVER}
	@echo "Please reboot your system"

config_r:
	@echo "make config"
	/bin/bash script/Configure script/config.in

#REMOVE BEGIN
cov:
	make -f coverity.mk target=all
smatch:
	make CHECK="smatch $(SMATCH_OPT)" CC="cgcc" 
.PHONY: checkpatch checkpatch2
checkpatch:
	@repo=`svn info | grep "^Repository Root" | cut -d " " -f3` && \
	svn diff $$DIFFX | filterdiff --clean $$FILTER > checkpatch.diff && \
	svn export -q --force $$repo/trunk/checkpatch/ && \
	PATH=$$PWD/checkpatch:$$PATH pre-commit.py -l checkpatch.diff && /bin/bash checkpatch/genchkid.sh
checkpatch2:
	@svn diff $$DIFFX | filterdiff --clean $$FILTER > checkpatch2.diff && \
	PATH=$$PWD/checkpatch:$$PATH pre-commit.py -l checkpatch2.diff && /bin/bash checkpatch/genchkid.sh
checkpatch3:
	@repo=`svn info | grep "^Repository Root" | cut -d " " -f3` && \
	svn diff $$DIFFX | filterdiff --clean $$FILTER > checkpatch.diff && \
	svn export -q --force $$repo/trunk/checkpatch/ && \
	PATH=$$PWD/checkpatch:$$PATH checkpatch.pl -q -no-tree -patch -show-types -root . -strict -ignore=",$$IGNORE" checkpatch.diff && /bin/bash checkpatch/genchkid.sh
#REMOVE END

.PHONY: modules clean

clean:
	#$(MAKE) -C $(KSRC) M=$(shell pwd) clean
	cd $(HAL) ; rm -fr */*/*/*.mod.c */*/*/*.mod */*/*/*.o */*/*/.*.cmd */*/*/*.ko
	cd $(HAL) ; rm -fr */*/*.mod.c */*/*.mod */*/*.o */*/.*.cmd */*/*.ko
	cd $(HAL) ; rm -fr */*.mod.c */*.mod */*.o */.*.cmd */*.ko
	cd $(HAL) ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd core ; rm -fr */*.mod.c */*.mod */*.o */.*.cmd */*.ko
	cd core ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep/linux ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep/linux/hwsim ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd os_dep ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	cd platform ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	rm -fr Module.symvers ; rm -fr Module.markers ; rm -fr modules.order
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm -fr .tmp_versions
endif

