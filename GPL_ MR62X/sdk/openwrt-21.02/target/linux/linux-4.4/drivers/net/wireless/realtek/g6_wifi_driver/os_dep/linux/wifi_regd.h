/******************************************************************************
 *
 * Copyright(c) 2009-2010 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

#ifndef __WIFI_REGD_H__
#define __WIFI_REGD_H__

void rtw_regd_apply_flags(struct wiphy *wiphy);
int rtw_regd_init(struct wiphy *wiphy);

#ifdef CONFIG_CFG80211_CRDA_SUPPORT
#define RTL819x_2GHZ_CH01_11    REG_RULE(2412-10, 2462+10, 40, 0, 20, 0)
#define RTL819x_2GHZ_CH12_13    REG_RULE(2467-30, 2472+10, 40, 0, 20, 0)
#define RTL819x_2GHZ_CH14       REG_RULE(2484-10, 2484+10, 20, 0, 20, 0)
/* 5G */
#define RTL819x_5GHZ_5150_5350  REG_RULE(5150-10, 5350+10, 160, 0, 30, 0)
#define RTL819x_5GHZ_5470_5850  REG_RULE(5470-10, 5850+10, 160, 0, 30, 0)

#define RTL_2GHZ_ALL            RTL819x_2GHZ_CH01_11, \
                                RTL819x_2GHZ_CH12_13, \
                                RTL819x_2GHZ_CH14

#define RTL_5GHZ_ALL            RTL819x_5GHZ_5150_5350, \
                                RTL819x_5GHZ_5470_5850


static const struct ieee80211_regdomain rtl_regdom_all = {
        .n_reg_rules = 5,
        .alpha2 =  "00",
        .reg_rules = {
                RTL_2GHZ_ALL,
                RTL_5GHZ_ALL,
        }
};
#endif


#endif /* __WIFI_REGD_H__ */
