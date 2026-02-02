/*
 * iwinfo - Wireless Information Library - Linux Wireless Extension Backend
 *
 *   Copyright (C) 2009 Jo-Philipp Wich <xm@subsignal.org>
 *
 * The iwinfo library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * The iwinfo library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with the iwinfo library. If not, see http://www.gnu.org/licenses/.
 *
 * Parts of this code are derived from the Linux wireless tools, iwlib.c,
 * iwlist.c and iwconfig.c in particular.
 */

#include "iwinfo/rtk.h"
#include "iwinfo/nl80211.h"
#include <stdio.h>


#define COUNTRY_CODE_MAX_LEN 2
#define OUI_TPLINK 0x001D0F

enum tplink_vendor_subcmd{
	WIFI_TP_SUBCMD_GET_CHANNEL_UTILIZATION = 0x10,
};

struct wifi_if_name
{
    char *vifname;
    char *ifname;
};

struct ch_list_t
{
    unsigned char *len_ch;
};

struct country_ch_map_t
{
    char country[3];
    unsigned char chplan_24g;
    unsigned char chplan_5g;
};

extern int nl80211_init(void);
extern void nl80211_free(struct nl80211_msg_conveyor *cv);
extern struct nlattr ** nl80211_parse(struct nl_msg *msg);
extern struct nl80211_msg_conveyor * nl80211_msg(const char *ifname,
                                                 int cmd, int flags);
extern char * nl80211_phy2ifname(const char *ifname);
extern struct nl80211_msg_conveyor * nl80211_send(struct nl80211_msg_conveyor *cv, int (*cb_func)(struct nl_msg *, void *), void *cb_arg);

#define CH_LIST_ENT(_len, arg...) \
    {.len_ch = (unsigned char[_len + 1]) {_len, ##arg},}

#define CH_LIST_LEN(_ch_list) ((_ch_list)->len_ch[0])
#define CH_LIST_CH(_ch_list, _i) ((_ch_list)->len_ch[_i + 1])

#define COUNTRY_CHPLAN_ENT(_country, _chlist_24g, _chlist_5g) \
    {.country = (_country), .chplan_24g = (_chlist_24g), .chplan_5g = (_chlist_5g)}

static const struct ch_list_t ch_def_2g[] = {
    /* 0 */ CH_LIST_ENT(11, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11),
    /* 1 */ CH_LIST_ENT(13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13),
    /* 2 */ CH_LIST_ENT(14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14),
};

static const struct ch_list_t ch_def_5g[] = {
    /* 0 */ CH_LIST_ENT(4, 36, 40, 44, 48),
    /* 1 */ CH_LIST_ENT(9, 36, 40, 44, 48, 149, 153, 157, 161, 165),
    /* 2 */ CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165),
    /* 3 */ CH_LIST_ENT(20, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144),
    /* 4 */ CH_LIST_ENT(25, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165),
    /* 5 */ CH_LIST_ENT(19, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140),
    /* 6 */ CH_LIST_ENT(22, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 144, 149, 153, 157, 161, 165),
    /* 7 */ CH_LIST_ENT(8, 36, 40, 44, 48, 52, 56, 60, 64),
};

static const struct country_ch_map_t country_ch_map[] = {
    COUNTRY_CHPLAN_ENT("BR", 1, 1),
    COUNTRY_CHPLAN_ENT("DE", 1, 5),
    COUNTRY_CHPLAN_ENT("KR", 1, 1),
    COUNTRY_CHPLAN_ENT("RU", 1, 2),
    COUNTRY_CHPLAN_ENT("TW", 0, 1),
    COUNTRY_CHPLAN_ENT("US", 0, 4),
    COUNTRY_CHPLAN_ENT("JP", 1, 3),
    COUNTRY_CHPLAN_ENT("EG", 1, 7),   
    COUNTRY_CHPLAN_ENT("SG", 0, 1),
    COUNTRY_CHPLAN_ENT("CA", 0, 6),
};

struct wifi_if_name wifi_if_name_map[] = {
    {"wifi0",       "wlan0"},
    {"wifi1",       "wlan1"},

    {"wlan0_ap0",   "wlan0"},
    {"wlan0_ap1",   "wlan0-vap0"},
    {"wlan0_ap2",   "wlan0-vap1"},
    {"wlan0_ap3",   "wlan0-vap2"},
    {"wlan0_ap4",   "wlan0-vap3"},
    {"wlan0_ap5",   "wlan0-vap4"},
    {"wlan0_ap6",   "wlan0-vap5"},
    {"wlan0_sta",   "wlan0-vxd"},

    {"wlan1_ap0",   "wlan1"},
    {"wlan1_ap1",   "wlan1-vap0"},
    {"wlan1_ap2",   "wlan1-vap1"},
    {"wlan1_ap3",   "wlan1-vap2"},
    {"wlan1_ap4",   "wlan1-vap3"},
    {"wlan1_ap5",   "wlan1-vap4"},
    {"wlan1_ap6",   "wlan1-vap5"},
    {"wlan1_sta",   "wlan1-vxd"},
};

static int _exec_and_res(const char *cmd, char *outStr, int strLen)
{
    FILE *fp;
    int ret = -1;

    fp = popen(cmd, "r");
    if (fp == NULL)
        return -1;

    ret = fread(outStr, sizeof(char), strLen, fp);
    pclose(fp);

    return ret;
}


const char* real_ifname(const char *ifname)
{
    int i;

    for (i = 0; i < sizeof(wifi_if_name_map)/sizeof(wifi_if_name_map[0]); i++)
    {
        if (!strcmp(ifname, wifi_if_name_map[i].vifname))
        {
            return wifi_if_name_map[i].ifname;
        }
    }

    return ifname;
}

static int rtk_isap(const char *ifname)
{
    if (strstr(real_ifname(ifname), "wlan") != NULL)
        return 1;

    return 0;
}

static int rtk_is5g(const char *ifname)
{
    if (ifname && (strncmp(ifname, "wlan0", 5) == 0 || strncmp(ifname, "wifi0", 5) == 0))
        return 1;

    return 0;
}


static struct nl80211_msg_conveyor * rtk_vendor_msg(const char *ifname,
                                                 int cmd, int flags, uint32_t vendor_cmd)
{
    struct nl80211_msg_conveyor *cv;
    cv = nl80211_msg(ifname,cmd,flags);
    if (cv)
    {
        NLA_PUT_U32(cv->msg, NL80211_ATTR_VENDOR_ID, OUI_TPLINK);
        NLA_PUT_U32(cv->msg, NL80211_ATTR_VENDOR_SUBCMD, vendor_cmd);
        return cv;

    }
    return NULL;

nla_put_failure:
    /*NLA_PUT_U32宏用到此错误处理*/
    nl80211_free(cv);
    return NULL;
}

int rtk_probe(const char *ifname)
{
    return ( rtk_isap(ifname) );
}

void rtk_close(void)
{
    nl80211_close();
}

int rtk_get_mode(const char *ifname, int *buf)
{
    return nl80211_get_mode(ifname, buf);
}

int rtk_get_ssid(const char *ifname, char *buf)
{
    return nl80211_get_ssid(real_ifname(ifname), buf);
}

int rtk_get_bssid(const char *ifname, char *buf)
{
    return nl80211_get_bssid(real_ifname(ifname), buf);
}

int rtk_get_bitrate(const char *ifname, int *buf)
{
    return nl80211_get_bitrate(real_ifname(ifname), buf);
}

int rtk_get_channel(const char *ifname, int *buf)
{
    return nl80211_get_channel(real_ifname(ifname), buf);
}

int rtk_get_frequency(const char *ifname, int *buf)
{
    return nl80211_get_frequency(real_ifname(ifname), buf);
}

int rtk_get_txpower(const char *ifname, int *buf)
{
    return nl80211_get_txpower(real_ifname(ifname), buf);
}

int rtk_get_signal(const char *ifname, int *buf)
{
    return nl80211_get_signal(real_ifname(ifname), buf);
}

int rtk_get_noise(const char *ifname, int *buf)
{
    return nl80211_get_noise(real_ifname(ifname), buf);
}

int rtk_get_quality(const char *ifname, int *buf)
{
    return nl80211_get_quality(real_ifname(ifname), buf);
}

int rtk_get_quality_max(const char *ifname, int *buf)
{
    return nl80211_get_quality_max(real_ifname(ifname), buf);
}

int rtk_get_assoclist(const char *ifname, char *buf, int *len)
{
    return nl80211_get_assoclist(real_ifname(ifname), buf, len);
}

int rtk_get_txpwrlist(const char *ifname, char *buf, int *len)
{
    return nl80211_get_txpwrlist(real_ifname(ifname), buf, len);
}

int rtk_get_scanlist(const char *ifname, char *buf, int *len)
{
    int ret = nl80211_get_scanlist(real_ifname(ifname), buf, len);
    char *real_name = real_ifname(ifname);
    char cmd[128] = {0};
    char mode[4] = {0};
    char result = 0;

    /* it's error if ifname is real name */
    snprintf(cmd, sizeof(cmd), "uci get wireless.%s.mode", ifname);
    if (0 > _exec_and_res(cmd, mode, sizeof(mode)))
    {
        printf(" %s %d cmd exec err \n",__func__, __LINE__);
        return ret;
    }
    if (strncmp("sta", mode, 3) == 0)
    {
        snprintf(cmd, sizeof(cmd), "wpa_cli -i global interface | grep \"%s$\"  | wc -l", real_name);
    }
    else if  (strncmp("ap", mode, 2) == 0)
    {
        snprintf(cmd, sizeof(cmd), "hostapd_cli -i global interface | grep \"%s$\"  | wc -l", real_name);
    }
    else
    {
        printf(" %s warnning: it is real ifname: %s \n",__func__,ifname);
        return ret;
    }

    if (0 > _exec_and_res(cmd, &result, 1))
    {
        printf(" %s %d cmd exec err \n",__func__, __LINE__);
        return ret;
    }

    if (result == '0')
    {
        snprintf(cmd, sizeof(cmd), "ifconfig %s down", real_name);
        system(cmd);
    }

    return ret;
}

int rtk_get_freqlist(const char *ifname, char *buf, int *len)
{
    struct iwinfo_freqlist_entry *e = (struct iwinfo_freqlist_entry *)buf;
    int             i;
    FILE            *fd;
    unsigned char   country[COUNTRY_CODE_MAX_LEN + 1] = "DE";
    const struct ch_list_t *ch_list;
    const struct country_ch_map_t *ch_map = NULL;
    char *band;

    if (0 < _exec_and_res("echo $(getfirm COUNTRY) | tr -d \"\\n\"", country, COUNTRY_CODE_MAX_LEN))
    {
        country[COUNTRY_CODE_MAX_LEN] = '\0';
    }

    for (i = 0; i < ARRAY_SIZE(country_ch_map); i++)
    {
        if (!strncmp(country, country_ch_map[i].country, 2))
            ch_map = &country_ch_map[i];
    }

    if (ch_map == NULL)
        return -1;

    if (rtk_is5g(real_ifname(ifname)))
    {
        if (ch_map->chplan_5g >= ARRAY_SIZE(ch_def_5g))
            return -1;

        ch_list = &ch_def_5g[ch_map->chplan_5g];
        band = "a";
    }
    else
    {
        if (ch_map->chplan_24g >= ARRAY_SIZE(ch_def_2g))
            return -1;

        ch_list = &ch_def_2g[ch_map->chplan_24g];
        band = NULL;
    }

    for (i = 0; i < CH_LIST_LEN(ch_list); i++, e++)
    {
        e->channel = CH_LIST_CH(ch_list, i);
        e->mhz = iwinfo_channel2freq(e->channel, band);
        e->restricted = 0;
    }

    *len = CH_LIST_LEN(ch_list) * sizeof(struct iwinfo_freqlist_entry);

    return 0;

    //接口没启动时，国家码不生效，从内核获取到的信道列表是全集
    //return nl80211_get_freqlist(real_ifname(ifname), buf, len);
}

int rtk_get_country(const char *ifname, char *buf)
{
    return nl80211_get_country(real_ifname(ifname), buf);
}

int rtk_get_countrylist(const char *ifname, char *buf, int *len)
{
    return nl80211_get_countrylist(real_ifname(ifname), buf, len);
}

int rtk_get_hwmodelist(const char *ifname, int *buf)
{
    return nl80211_get_hwmodelist(real_ifname(ifname), buf);
}

int rtk_get_encryption(const char *ifname, char *buf)
{
    return nl80211_get_encryption(real_ifname(ifname), buf);
}

int rtk_get_mbssid_support(const char *ifname, int *buf)
{
    return nl80211_get_mbssid_support(real_ifname(ifname), buf);
}

int rtk_get_hardware_id(const char *ifname, char *buf)
{
    return nl80211_get_hardware_id(real_ifname(ifname), buf);
}

int rtk_get_hardware_name(const char *ifname, char *buf)
{
    return nl80211_get_hardware_name(real_ifname(ifname), buf);
}

int rtk_get_txpower_offset(const char *ifname, int *buf)
{
    return nl80211_get_txpower_offset(real_ifname(ifname), buf);
}

int rtk_get_frequency_offset(const char *ifname, int *buf)
{
    return nl80211_get_frequency_offset(real_ifname(ifname), buf);
}

int rtk_get_htmode(const char *ifname, int *buf)
{
    return nl80211_get_htmode(real_ifname(ifname), buf);
}

static int rtk_get_channel_utilization_cb(struct nl_msg *msg, void *arg)
{
	int *channel_utilization = arg;
	struct nlattr **tb = nl80211_parse(msg);

	if (tb[NL80211_ATTR_VENDOR_DATA])
		*channel_utilization = nla_get_u32(tb[NL80211_ATTR_VENDOR_DATA]);

	return NL_SKIP;
}

int rtk_get_channel_utilization(const char *ifname, int *buf)
{
	char *res, *channel;
	struct nl80211_msg_conveyor *req;

	res = nl80211_phy2ifname(real_ifname(ifname));
	*buf = -1;

	req = rtk_vendor_msg(res ? res : ifname, NL80211_CMD_VENDOR, 0, WIFI_TP_SUBCMD_GET_CHANNEL_UTILIZATION);
	if (req)
	{
		nl80211_send(req, rtk_get_channel_utilization_cb, buf);
		nl80211_free(req);
	}

	return (*buf < 0) ? -1 : 0;

}

const struct iwinfo_ops rtk_ops = {
    .name             = "rtk",
    .probe            = rtk_probe,

    .channel          = rtk_get_channel,
    .frequency        = rtk_get_frequency,
    .frequency_offset = rtk_get_frequency_offset,
    .txpower          = rtk_get_txpower,
    .txpower_offset   = rtk_get_txpower_offset,
    .bitrate          = rtk_get_bitrate,
    .signal           = rtk_get_signal,
    .noise            = rtk_get_noise,
    .quality          = rtk_get_quality,
    .quality_max      = rtk_get_quality_max,
    .mbssid_support   = rtk_get_mbssid_support,
    .hwmodelist       = rtk_get_hwmodelist,
    .mode             = rtk_get_mode,
    .ssid             = rtk_get_ssid,
    .bssid            = rtk_get_bssid,
    .country          = rtk_get_country,
    .hardware_id      = rtk_get_hardware_id,
    .hardware_name    = rtk_get_hardware_name,
    .encryption       = rtk_get_encryption,
    .assoclist        = rtk_get_assoclist,
    .txpwrlist        = rtk_get_txpwrlist,
    .scanlist         = rtk_get_scanlist,
    .freqlist         = rtk_get_freqlist,
    .countrylist      = rtk_get_countrylist,
    .close            = rtk_close,
    .channel_utilization  = rtk_get_channel_utilization,
    .htmode           = rtk_get_htmode,
};

