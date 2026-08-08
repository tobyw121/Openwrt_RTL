# SPDX-License-Identifier: GPL-2.0-only
#
# Makefile for the Linux Wireless network device drivers for Realtek units
#

obj-$(CPTCFG_RTL8180)		+= rtl818x/
obj-$(CPTCFG_RTL8187)		+= rtl818x/
obj-$(CPTCFG_RTLWIFI)		+= rtlwifi/
obj-$(CPTCFG_RTL8XXXU)		+= rtl8xxxu/
obj-$(CPTCFG_RTW88)		+= rtw88/
ifeq ($(CPTCFG_RTLWIFI6),m)
obj-$(CPTCFG_RTLWIFI6)      += g6_wifi_driver/
else
obj-$(CPTCFG_RTL8852AE)   += g6_wifi_driver/
endif
ifeq "$(wildcard $(BACKPORT_DIR)/drivers/net/wireless/realtek/rtl8197g)" ""
obj-$(CPTCFG_RTL8192CD)         += rtl8192fe/
else
obj-$(CPTCFG_RTL8192CD)         += rtl8197g/
endif
obj-$(CPTCFG_RTL8723FU)         += rtl8723fu/
obj-$(CPTCFG_WFO_VIRT)         += wfo_virt/
obj-$(CPTCFG_THER_CTRL)     += ther_ctrl/
