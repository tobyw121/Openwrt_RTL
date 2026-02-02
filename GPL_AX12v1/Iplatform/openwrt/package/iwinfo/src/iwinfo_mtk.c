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

#include "iwinfo/mtk.h"
#include "iwinfo/wext.h"

#ifdef WIFI_TRIBAND_SUPPORT
#include <unl.h>
#include <linux/nl80211.h>
#include <netlink/attr.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include "mtk_vendor_nl80211.h"
#endif

struct survey_table {
	long channel;
	long strength;
	char mode[12];
	char ssid[66];
	char bssid[18];
	char enc[16];
	char crypto[16];
};

#ifdef WIFI_TRIBAND_SUPPORT
char* wifidev_to_wifiname[3] = {
	"ra0",
	"rai0",
	"rax0"
};
#else
char* wifidev_to_wifiname[2] = {
	"ra0",
#ifdef PACKAGE_WIFI_DBDC_MODE
	"rax0"
#else
	"rai0"
#endif
};
#endif

#define COUNTRY_CODE_MAX_LEN 2
#define COUNTRY_CA "CA"
#define COUNTRY_DE "DE"
#define COUNTRY_JP "JP"
#define COUNTRY_RU "RU"
#define COUNTRY_US "US"
#define COUNTRY_KR "KR"
#define COUNTRY_SG "SG"
#define COUNTRY_TW "TW"



/* EU */
CH_FREQ_MAP CH_HZ_OF_EU_5G[] = 
{
	{36, 5180},
	{40, 5200},
	{44, 5220},
	{48, 5240},
#ifdef DFS_CHANNEL_SUPPORT
	/* band 2 */
	{52, 5260},
	{56, 5280},
	{60, 5300},
	{64, 5320},
	/* band 3*/
	{100, 5500},
	{104, 5520},
	{108, 5540},
	{112, 5560},
	{116, 5580},
	{120, 5600},
	{124, 5620},
	{128, 5640},
	{132, 5660},
	{136, 5680},
	{140, 5700},
#endif
};
CH_FREQ_MAP CH_HZ_OF_EU_2G[] = 
{
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
};

/* JP */
CH_FREQ_MAP CH_HZ_OF_JP_5G[] = 
{
	{36, 5180},
	{40, 5200},
	{44, 5220},
	{48, 5240},
#ifdef DFS_CHANNEL_SUPPORT
	/* band 2 */
	{52, 5260},
	{56, 5280},
	{60, 5300},
	{64, 5320},
	/* band 3*/
	{100, 5500},
	{104, 5520},
	{108, 5540},
	{112, 5560},
	{116, 5580},
	{120, 5600},
	{124, 5620},
	{128, 5640},
	{132, 5660},
	{136, 5680},
	{140, 5700},
	{144, 5720},
#endif
};
CH_FREQ_MAP CH_HZ_OF_JP_2G[] = 
{
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
};

/* RU */
CH_FREQ_MAP CH_HZ_OF_RU_5G[] = 
{
	{36, 5180},
	{40, 5200},
	{44, 5220},
	{48, 5240},
	/* band 2 */
	{52, 5260},
	{56, 5280},
	{60, 5300},
	{64, 5320},
	/* band 4 */
	{149, 5745},
	{153, 5765},
	{157, 5785},
	{161, 5805},
	{165, 5825},
};
CH_FREQ_MAP CH_HZ_OF_RU_2G[] = 
{
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
	{12, 2467},
	{13, 2472},
};

/* US */
#ifdef WIFI_TRIBAND_SUPPORT
CH_FREQ_MAP CH_HZ_OF_US_6G[] = 
{
	{1, 5955},
	{5, 5975},
	{9, 5995},
	{13, 6015},
	{17, 6035},
	{21, 6055},
	{25, 6075},
	{29, 6095},
	{33, 6115},
	{37, 6135},
	{41, 6155},
	{45, 6175},
	{49, 6195},
	{53, 6215},
	{57, 6235},
	{61, 6255},
	{65, 6275},
	{69, 6295},
	{73, 6315},
	{77, 6335},
	{81, 6355},
	{85, 6375},
	{89, 6395},
	{93, 6415},
	{97, 6435},
	{101, 6455},
	{105, 6475},
	{109, 6495},
	{113, 6515},
	{117, 6535},
	{121, 6555},
	{125, 6575},
	{129, 6595},
	{133, 6615},
	{137, 6635},
	{141, 6655},
	{145, 6675},
	{149, 6695},
	{153, 6715},
	{157, 6735},
	{161, 6755},
	{165, 6775},
	{169, 6795},
	{173, 6815},
	{177, 6835},
	{181, 6855},
	{185, 6875},
	{189, 6895},
	{193, 6915},
	{197, 6935},
	{201, 6955},
	{205, 6975},
	{209, 6995},
	{213, 7015},
	{217, 7035},
	{221, 7055},
	{225, 7075},
	{229, 7095},
	{233, 7115}
};
#endif

CH_FREQ_MAP CH_HZ_OF_US_5G[] = 
{
	{36, 5180},
	{40, 5200},
	{44, 5220},
	{48, 5240},
#ifdef DFS_CHANNEL_SUPPORT
	/* band 2 */
	{52, 5260},
	{56, 5280},
	{60, 5300},
	{64, 5320},
	/* band 3*/
	{100, 5500},
	{104, 5520},
	{108, 5540},
	{112, 5560},
	{116, 5580},
	{120, 5600},
	{124, 5620},
	{128, 5640},
	{132, 5660},
	{136, 5680},
	{140, 5700},
	{144, 5720},
#endif
	/* band 4 */
	{149, 5745},
	{153, 5765},
	{157, 5785},
	{161, 5805},
	{165, 5825},
};
CH_FREQ_MAP CH_HZ_OF_US_2G[] = 
{
	{1, 2412},
	{2, 2417},
	{3, 2422},
	{4, 2427},
	{5, 2432},
	{6, 2437},
	{7, 2442},
	{8, 2447},
	{9, 2452},
	{10, 2457},
	{11, 2462},
};

/* KR */
CH_FREQ_MAP CH_HZ_OF_KR_5G[] =
{
        {36, 5180},
        {40, 5200},
        {44, 5220},
        {48, 5240},
        /* band 4 */
        {149, 5745},
        {153, 5765},
        {157, 5785},
        {161, 5805},
        {165, 5825},
};

CH_FREQ_MAP CH_HZ_OF_KR_2G[] =
{
        {1, 2412},
        {2, 2417},
        {3, 2422},
        {4, 2427},
        {5, 2432},
        {6, 2437},
        {7, 2442},
        {8, 2447},
        {9, 2452},
        {10, 2457},
        {11, 2462},
        {12, 2467},
        {13, 2472},
};

const char* _devname_to_ifname(const char *ifname)
{
	if (!strncmp(ifname, "wifi0", 5))
	{
		//printf("call with ifname %s\n", wifidev_to_wifiname[0]);
		return wifidev_to_wifiname[0];
	}
	else if (!strncmp(ifname, "wifi1", 5))
	{
		//printf("call with ifname %s\n", wifidev_to_wifiname[1]);
		return wifidev_to_wifiname[1];
	}
#ifdef WIFI_TRIBAND_SUPPORT
	else if (!strncmp(ifname, "wifi2", 5))
	{
		//printf("call with ifname %s\n", wifidev_to_wifiname[1]);
		return wifidev_to_wifiname[2];
	}
#endif
	else
	{
		//printf("call with ifname %s\n", ifname);
		return ifname;
	}
}

/**/
static int __mtk_if_status(const char *ifname)
{
	char cmd[256];
	char buf[1024];
	FILE *fp = NULL;

	memset(cmd, 0, sizeof(cmd));
	memset(buf, 0, sizeof(buf));
	sprintf(cmd, "ifconfig %s | grep UP", _devname_to_ifname(ifname));
	fp = popen(cmd, "r");
	fscanf(fp, "%s\n", buf);
	pclose(fp);
	if (buf[0]=='\0')
	{	
		//printf("tqj-->iwinfo: interface down, return -1\n");
		return -1;
	}
	else
	{
		//printf("tqj-->iwinfo: interface up, return 0\n");
		return 0;
	}
}


#ifdef MTK_NETLINK_SUPPORT

static int get_bssinfo_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int err = 0;
	char* nl_data;

	struct nlattr *sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX + 1];

	if (!data)
		return -1;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(sub_tb, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSSINFO]) {
			nl_data = nla_data(sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSSINFO]);
			memcpy(data, nl_data, sizeof(WLAN_BSS_INFO));
		}		
	} else
		printf("%s(), no any show rsp string from driver\n", __func__);

	return 0;
}

static int get_stainfo_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int err = 0;
	char* nl_data;

	struct nlattr *sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX + 1];

	if (!data)
		return -1;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(sub_tb, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_STAINFO]) {
			nl_data = nla_data(sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_STAINFO]);
			memcpy(data, nl_data, sizeof(WLAN_STA_INFO_TABLE));
		}		
	} else
		printf("%s(), no any show rsp string from driver\n", __func__);

	return 0;
}

static int get_scaninfo_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int err = 0;
	char* nl_data;

	struct nlattr *sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX + 1];

	if (!data)
		return -1;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(sub_tb, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SCANINFO]) {
			nl_data = nla_data(sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SCANINFO]);
			memcpy(data, nl_data, sizeof(SCAN_BSS_TABLE));
		}		
	} else
		printf("%s(), no any show rsp string from driver\n", __func__);

	return 0;
}

static int get_chanlist_callback(struct nl_msg *msg, void *data)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int err = 0;
	char* nl_data;

	struct nlattr *sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX + 1];

	if (!data)
		return -1;

	err = nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
			  genlmsg_attrlen(gnlh, 0), NULL);
	if (err < 0)
		return err;

	if (tb[NL80211_ATTR_VENDOR_DATA]) {
		err = nla_parse_nested(sub_tb, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_MAX,
			tb[NL80211_ATTR_VENDOR_DATA], NULL);
		if (err < 0)
			return err;

		if (sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANLIST]) {
			nl_data = nla_data(sub_tb[MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANLIST]);
			memcpy(data, nl_data, sizeof(struct MTK_CHANINFO));
		}		
	} else
		printf("%s(), no any show rsp string from driver\n", __func__);

	return 0;
}


int (*registered_handler)(struct nl_msg *, void *);
void *registered_handler_data;

static void register_handler(int (*handler)(struct nl_msg *, void *), void *data)
{
	registered_handler = handler;
	registered_handler_data = data;
}

static int valid_handler(struct nl_msg *msg, void *arg)
{
	if (registered_handler)
		return registered_handler(msg, registered_handler_data);

	return 0;
}

static int handle_common_command(struct nl_msg *msg,
			   unsigned short sub_cmd_id, char *data, size_t len)
{
	void *attr;

	attr = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!attr)
		return -1;

	if (nla_put(msg, sub_cmd_id, len, data))
		return -1;

	nla_nest_end(msg, attr);

	return 0;
}

static int mtk_get80211priv(const char *ifname, unsigned short sub_cmd_id, int (*handler)(struct nl_msg *, void *), void *data, size_t len)
{
	struct nl_msg *msg = NULL;
	struct unl unl;
	int if_idx;
	int ret = 0;

	int cmm_len = 0;
	const char cmm_data[8] = "Any";
	char *p_cmm_data = NULL;

	//printf("%s(%d), sub_cmd_id:%d\n", __func__, __LINE__, sub_cmd_id);
	if( __mtk_if_status(ifname) < 0)
		return -1;

	if_idx = if_nametoindex(ifname);
	if (unl_genl_init(&unl, "nl80211") < 0) {
		printf("Failed to connect to nl80211\n");
		ret = -1;
		goto out;
	}

	msg = unl_genl_msg(&unl, NL80211_CMD_VENDOR, false);
	if (!msg) {
		printf("Failed to connect to unl_genl_msg()\n");
		ret = -1;
		goto out;
	}

	if (nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_idx) ||
	    nla_put_u32(msg, NL80211_ATTR_VENDOR_ID, MTK_NL80211_VENDOR_ID) ||
	    nla_put_u32(msg, NL80211_ATTR_VENDOR_SUBCMD, MTK_NL80211_VENDOR_SUBCMD_GET_RUNTIME_INFO))
	{
		printf("Nla put error\n");
		ret = -1;
		nlmsg_free(msg);
		goto out;
	}

	register_handler(handler, data);

	/*
	 参考 nla_reserve()@dl/libnl-tiny-/attr.c
	 tlen = NLMSG_ALIGN(msg->nm_nlh->nlmsg_len) + nla_total_size(attrlen);
	 if ((tlen + msg->nm_nlh->nlmsg_len) > msg->nm_size)
		return NULL;
	 (Bug?? msg->nm_nlh->nlmsg_len used 2 times)

	 unl_genl_msg() 分配的msg大小( nm_size)是4096。一些sub_cmd_id的data太长了，在进行到 
	 handle_common_command-nla_put-nla_reserve时，判断msg剩余长度不够用时，就会报错退出。
	 其实nla_put传入的data和len只要不为空就行，driver并不会使用这些传回数据，因此这里对于
	 这种情况直接替换为较短的buf。
	*/
	if (sub_cmd_id == MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_STAINFO ||
		sub_cmd_id == MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SCANINFO)
	{
		//printf("sub_cmd_id:%d, nm_size:%d, nlmsg_len:%d/%d, u32/nla_u32:%d/%d\n", sub_cmd_id, msg->nm_size, 
		//	msg->nm_nlh->nlmsg_len, NLMSG_ALIGN(msg->nm_nlh->nlmsg_len),
		//	sizeof(if_idx), nla_total_size(sizeof(if_idx)));
		p_cmm_data = cmm_data;
		cmm_len = strlen(cmm_data);
	}
	else
	{
		p_cmm_data = data;
		cmm_len = len;
	}

	if (handle_common_command(msg, sub_cmd_id, p_cmm_data, cmm_len))
	{
		printf("Failed to handle_common_command()\n");
		ret = -1;
		nlmsg_free(msg);
		goto out;
	}

	ret = unl_genl_request(&unl, msg, valid_handler, NULL);
	if (ret)
	{
		printf("Failed to unl_genl_request(): %d\n", ret);
		ret = -1;
		//nlmsg_free(msg);  //Mustn't nlmsg_free, because unl_genl_request has excuted
		goto out;
	}

out:
	unl_free(&unl);

	return ret;
}

static int handle_set_command(struct nl_msg *msg, char *cmd_str, int attr)
{
	void *data;
	size_t len = 0;

	data = nla_nest_start(msg, NL80211_ATTR_VENDOR_DATA);
	if (!data)
		return -1;

	len = strlen(cmd_str);
	if (len) {
		if (nla_put_string(msg, attr, cmd_str))
			return -1;
	}

	nla_nest_end(msg, data);

	return 0;
}

static int mtk_set80211priv(const char *ifname, void *cmd_str)
{
	struct nl_msg *msg = NULL;
	struct unl unl;
	int if_idx;
	int ret = 0;

	if( __mtk_if_status(ifname) < 0)
		return -1;

	if_idx = if_nametoindex(ifname);
	if (unl_genl_init(&unl, "nl80211") < 0) {
		printf("Failed to connect to nl80211\n");
		ret = -1;
		goto out;
	}

	msg = unl_genl_msg(&unl, NL80211_CMD_VENDOR, false);
	if (!msg) {
		printf("Failed to connect to unl_genl_msg()\n");
		ret = -1;
		goto out;
	}

	if (nla_put_u32(msg, NL80211_ATTR_IFINDEX, if_idx) ||
	    nla_put_u32(msg, NL80211_ATTR_VENDOR_ID, MTK_NL80211_VENDOR_ID) ||
	    nla_put_u32(msg, NL80211_ATTR_VENDOR_SUBCMD, MTK_NL80211_VENDOR_SUBCMD_VENDOR_SET))
	{
		printf("Nla put error\n");
		ret = -1;
		nlmsg_free(msg);
		goto out;
	}

	if (handle_set_command(msg, cmd_str, MTK_NL80211_VENDOR_ATTR_VENDOR_SET_CMD_STR))
	{
		printf("Failed to handle_set_command()\n");
		ret = -1;
		nlmsg_free(msg);
		goto out;
	}

	ret = unl_genl_request(&unl, msg, valid_handler, NULL);
	if (ret)
	{
		printf("Failed to unl_genl_request(): %d\n", ret);
		ret = -1;
		//nlmsg_free(msg);  //Mustn't nlmsg_free, because unl_genl_request has excuted
		goto out;
	}

out:
	unl_free(&unl);

	return ret;
}

#else
static int mtk_wrq(struct iwreq *wrq, const char *ifname, int cmd, void *data, size_t len)
{
	strncpy(wrq->ifr_name, ifname, IFNAMSIZ);

	if( data != NULL )
	{
		if( len < IFNAMSIZ )
		{
			memcpy(wrq->u.name, data, len);
		}
		else
		{
			wrq->u.data.pointer = data;
			wrq->u.data.length = len;
		}
	}

	return iwinfo_ioctl(cmd, wrq);
}

static int mtk_get80211priv(const char *ifname, int op, void *data, size_t len)
{
	struct iwreq iwr;

	if( __mtk_if_status(ifname) < 0 || mtk_wrq(&iwr, ifname, op, data, len) < 0)
		return -1;

	return iwr.u.data.length;
}
#endif

static int mtk_isap(const char *ifname)
{
	int ret=0;

	if( strlen(ifname) <= 7 )
	{
		static char wifiname[IFNAMSIZ];
		snprintf(wifiname, sizeof(wifiname), "%s", ifname);
#ifdef WIFI_TRIBAND_SUPPORT
	if( !strncmp(wifiname, "ra", 2) || !strncmp(wifiname, "rai", 3) || !strncmp(wifiname, "rax", 3) || !strncmp(wifiname, "wifi", 4) ) ret=1;
#else
#ifdef PACKAGE_WIFI_DBDC_MODE
		if( !strncmp(wifiname, "ra", 2) || !strncmp(wifiname, "rax", 3) || !strncmp(wifiname, "wifi", 4) ) ret=1;
#else
		if( !strncmp(wifiname, "ra", 2) || !strncmp(wifiname, "rai", 3) || !strncmp(wifiname, "wifi", 4) ) ret=1;
#endif
#endif
	}

	return ret;
}

int is_cli_iface(const char *ifname)
{
	int ret = 0;
	if(strlen(ifname) <= 7)
	{
		static char wifiname[IFNAMSIZ];
		snprintf(wifiname, sizeof(wifiname), "%s", ifname);
		if( !strncmp(wifiname, "apcli", 5)) 
			ret=1;
	}
	return ret;
}

static int mtk_iscli(const char *ifname)
{
	int ret=0;
	char cmd[256];
	char buf[1024];
	FILE *fp = NULL;
	char *pstr;
	memset(cmd, 0, sizeof(cmd));
	
	if( strlen(ifname) <= 7 )
	{
		static char wifiname[IFNAMSIZ];
		snprintf(wifiname, sizeof(wifiname), "%s", ifname);
		if( !strncmp(wifiname, "apcli", 5) || !strncmp(wifiname, "apclii", 6) ) 
		{	
			sprintf(cmd, "ifconfig %s | grep UP", _devname_to_ifname(ifname));
			fp = popen(cmd, "r");
			fscanf(fp, "%s\n", buf);
			pclose(fp);
			pstr = strstr(buf, "UP");
			ret = pstr? 1 : 0;
		}
	}

	return ret;
}

int mtk_probe(const char *ifname)
{
	return ( mtk_isap(ifname) || mtk_iscli(ifname) );
}

void mtk_close(void)
{
	/* Nop */
}

char *GetAuthModeStr(UINT32 authMode)
{
	if (IS_AKM_OPEN(authMode))
		return "OPEN";
	else if (IS_AKM_SHARED(authMode))
		return "SHARED";
	else if (IS_AKM_AUTOSWITCH(authMode))
		return "WEPAUTO";
	else if (IS_AKM_WPANONE(authMode))
		return "WPANONE";
	else if (IS_AKM_WPA1(authMode) && IS_AKM_WPA2(authMode))
		return "WPA1WPA2";
	else if (IS_AKM_WPA1PSK(authMode) && IS_AKM_WPA2PSK(authMode))
		return "WPAPSKWPA2PSK";
#if 0
	else if (IS_AKM_WPA2PSK(authMode) && IS_AKM_WPA3PSK(authMode))
		return "WPA2PSKWPA3PSK";
	else if (IS_AKM_WPA3PSK(authMode))
		return "WPA3PSK";
#endif
	else if (IS_AKM_WPA1(authMode))
		return "WPA";
	else if (IS_AKM_WPA1PSK(authMode))
		return "WPAPSK";
	else if (IS_AKM_FT_WPA2(authMode))
		return "FT-WPA2";
	else if (IS_AKM_FT_WPA2PSK(authMode))
		return "FT-WPA2PSK";
	else if (IS_AKM_WPA3(authMode)) /* WPA3 will be always accompanied by WPA2, so it should put before the WPA2 */
		return "WPA3";
	else if (IS_AKM_WPA2(authMode))
		return "WPA2";
	else if (IS_AKM_WPA2PSK(authMode))
		return "WPA2PSK";
#if 0
	else if (IS_AKM_WPA3_192BIT(authMode))
		return "WPA3-192";
#endif
	else if (IS_AKM_OWE(authMode))
		return "OWE";
	else
		return "UNKNOW";
}

char *GetEncryModeStr(UINT32 encryMode)
{
	if (IS_CIPHER_NONE(encryMode))
		return "NONE";
	else if (IS_CIPHER_WEP(encryMode))
		return "WEP";
	else if (IS_CIPHER_TKIP(encryMode) && IS_CIPHER_CCMP128(encryMode))
		return "TKIPAES";
	else if (IS_CIPHER_TKIP(encryMode))
		return "TKIP";
	else if (IS_CIPHER_CCMP128(encryMode))
		return "AES";
	else if (IS_CIPHER_CCMP256(encryMode))
		return "CCMP256";
	else if (IS_CIPHER_GCMP128(encryMode))
		return "GCMP128";
	else if (IS_CIPHER_GCMP256(encryMode))
		return "GCMP256";
	else if (IS_CIPHER_BIP_CMAC128(encryMode))
		return "BIP-CMAC128";
	else if (IS_CIPHER_BIP_CMAC256(encryMode))
		return "BIP-CMAC256";
	else if (IS_CIPHER_BIP_GMAC128(encryMode))
		return "BIP-GMAC128";
	else if (IS_CIPHER_BIP_GMAC256(encryMode))
		return "BIP-GMAC256";
	else
		return "UNKNOW";
}



int mtk_get_mode(const char *ifname, int *buf)
{
	if( mtk_isap(_devname_to_ifname(ifname)) ) *buf = IWINFO_OPMODE_MASTER;
	else if( mtk_iscli(_devname_to_ifname(ifname)) ) *buf = IWINFO_OPMODE_CLIENT;
	else *buf = IWINFO_OPMODE_UNKNOWN;
	return 0;
}

int mtk_get_vapinfo(const char *ifname, WLAN_BSS_INFO *bss)
{
#ifdef MTK_NETLINK_SUPPORT
	if (mtk_get80211priv(ifname, MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_BSSINFO, get_bssinfo_callback,
		bss, sizeof(WLAN_BSS_INFO)) < 0)
#else
	if (mtk_get80211priv(ifname, RTPRIV_IOCTL_GBSSINFO, bss, sizeof(WLAN_BSS_INFO)) < 0)
#endif
	{
		/*can not get bss info when interface is down, so give defalut value*/
		printf("error: cannot get bssinfo from driver\n");

		bss->securityEnable = 0;

#ifdef WIFI_TRIBAND_SUPPORT
		if (!strncmp(ifname, "rai", 3))
#else  /* WIFI_TRIBAND_SUPPORT */
#ifdef PACKAGE_WIFI_DBDC_MODE
		if (!strncmp(ifname, "rax", 3))
#else
		if (!strncmp(ifname, "rai", 3))
#endif
#endif /* WIFI_TRIBAND_SUPPORT */
		{
			bss->channel = 36;
#ifdef DOT11_EHT_BE
			bss->phymode = PHY_11BE_5G;
#elif defined(PACKAGE_WIFI_HE_AX)
			bss->phymode = PHY_11AX_5G;
#else
			bss->phymode = PHY_11VHT_N_A_MIXED;
#endif
		}
#ifdef WIFI_TRIBAND_SUPPORT
		else if (!strncmp(ifname, "rax", 3))
		{
			bss->channel = 37;
#ifdef DOT11_EHT_BE
			bss->phymode = PHY_11BE_6G;
#elif defined(PACKAGE_WIFI_HE_AX)
			bss->phymode = PHY_11AX_6G;
#else
			bss->phymode = PHY_11VHT_N_A_MIXED;
#endif
		}
#endif /* WIFI_TRIBAND_SUPPORT */
		else
		{
			bss->channel = 1;
#ifdef DOT11_EHT_BE
			bss->phymode = PHY_11BE_24G;
#elif defined(PACKAGE_WIFI_HE_AX)
			bss->phymode = PHY_11AX_24G;
#else
			bss->phymode = PHY_11BGN_MIXED;
#endif
		}
		bss->rssi = -1;
		bss->authMode = SEC_AKM_OPEN;
		bss->encrypType = SEC_CIPHER_NONE;

#ifdef WIFI_TRIBAND_SUPPORT
		if (!strncmp(ifname, "rax", 3))
		{
			bss->authMode = SEC_AKM_OWE;
			bss->encrypType = SEC_CIPHER_NONE;
		}
#endif

		return -1;	
	}
	return 0;
}

int mtk_get_ssid(const char *ifname, char *buf)
{
	return wext_ops.ssid(_devname_to_ifname(ifname), buf);
}

int mtk_get_bssid(const char *ifname, char *buf)
{
	char cmd[256];
	FILE *fp = NULL;

	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd, "ifconfig %s | grep UP", _devname_to_ifname(ifname));
	fp = popen(cmd, "r");
	fscanf(fp, "%s\n", buf);
	pclose(fp);

	return wext_ops.bssid(ifname, buf);
}

int mtk_get_bitrate(const char *ifname, int *buf)
{
	return wext_ops.bitrate(ifname, buf);
}

int mtk_get_channel(const char *ifname, int *buf)
{
	WLAN_BSS_INFO bss;
	memset(&bss, 0, sizeof(WLAN_BSS_INFO));
	mtk_get_vapinfo(_devname_to_ifname(ifname), &bss);
	*buf = bss.channel;
	return 0;
	/*
	return wext_ops.channel(_devname_to_ifname(ifname), buf);
	*/
}

int mtk_get_frequency(const char *ifname, int *buf)
{
	return wext_ops.frequency(_devname_to_ifname(ifname), buf);
}

int mtk_get_txpower(const char *ifname, int *buf)
{
	return -1;
}

int mtk_get_signal(const char *ifname, int *buf)
{
	return -1;
}

int mtk_get_noise(const char *ifname, int *buf)
{
	return -1;
}

int mtk_get_quality(const char *ifname, int *buf)
{
	return -1;
}

int mtk_get_quality_max(const char *ifname, int *buf)
{
	return -1;
}

static int mtk_get_rate(MACHTTRANSMIT_SETTING HTSetting)

{
	int MCSMappingRateTable[] =
	{2,  4,   11,  22, /* CCK*/
	12, 18,   24,  36, 48, 72, 96, 108, /* OFDM*/
	13, 26,   39,  52,  78, 104, 117, 130, 26,  52,  78, 104, 156, 208, 234, 260, /* 20MHz, 800ns GI, MCS: 0 ~ 15*/
	39, 78,  117, 156, 234, 312, 351, 390,										  /* 20MHz, 800ns GI, MCS: 16 ~ 23*/
	27, 54,   81, 108, 162, 216, 243, 270, 54, 108, 162, 216, 324, 432, 486, 540, /* 40MHz, 800ns GI, MCS: 0 ~ 15*/
	81, 162, 243, 324, 486, 648, 729, 810,										  /* 40MHz, 800ns GI, MCS: 16 ~ 23*/
	14, 29,   43,  57,  87, 115, 130, 144, 29, 59,   87, 115, 173, 230, 260, 288, /* 20MHz, 400ns GI, MCS: 0 ~ 15*/
	43, 87,  130, 173, 260, 317, 390, 433,										  /* 20MHz, 400ns GI, MCS: 16 ~ 23*/
	30, 60,   90, 120, 180, 240, 270, 300, 60, 120, 180, 240, 360, 480, 540, 600, /* 40MHz, 400ns GI, MCS: 0 ~ 15*/
	90, 180, 270, 360, 540, 720, 810, 900};

	int rate_count = sizeof(MCSMappingRateTable)/sizeof(int);
	int rate_index = 0;
	int value = 0;

	if (HTSetting.field.MODE >= MODE_HTMIX)
	{
    		rate_index = 12 + ((UCHAR)HTSetting.field.BW *24) + ((UCHAR)HTSetting.field.ShortGI *48) + ((UCHAR)HTSetting.field.MCS);
	}
	else if (HTSetting.field.MODE == MODE_OFDM)
		rate_index = (UCHAR)(HTSetting.field.MCS) + 4;
	else if (HTSetting.field.MODE == MODE_CCK)   
		rate_index = (UCHAR)(HTSetting.field.MCS);

	if (rate_index < 0)
		rate_index = 0;
    
	if (rate_index >= rate_count)
		rate_index = rate_count-1;

	value = (MCSMappingRateTable[rate_index] * 5)/10;

	return value;
}

int mtk_get_assoclist(const char *ifname, char *buf, int *len)
{
	struct iwinfo_assoclist_entry entry;
	WLAN_STA_INFO_TABLE staInfoTab;
	WLAN_STA_INFO *pStaInfo;
	int i, j;

#ifdef MTK_NETLINK_SUPPORT
	if (mtk_get80211priv(_devname_to_ifname(ifname), MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_STAINFO, get_stainfo_callback,
		&staInfoTab, sizeof(WLAN_STA_INFO_TABLE)) < 0)
		return -1;
#else
	if (mtk_get80211priv(_devname_to_ifname(ifname), RTPRIV_IOCTL_GSTAINFO, &staInfoTab, sizeof(WLAN_STA_INFO_TABLE)) > 0)
#endif
	{
		j = 0;
		for (i = 0; i < staInfoTab.Num; i++)
		{
			memset(&entry, 0, sizeof(entry));
			memcpy(entry.mac, &staInfoTab.Entry[i].addr, 6);
			if(staInfoTab.Entry[i].avgRssi0 > staInfoTab.Entry[i].avgRssi1)
				entry.signal = staInfoTab.Entry[i].avgRssi0;
			else
				entry.signal = staInfoTab.Entry[i].avgRssi1;

			entry.noise  = -95;
			entry.inactive =staInfoTab.Entry[i].connectedTime * 1000;
			
			entry.tx_packets = staInfoTab.Entry[i].txPackets;
			entry.rx_packets = staInfoTab.Entry[i].rxPackets;

			entry.tx_rate.rate = staInfoTab.Entry[i].lastTxRate * 1000;
			entry.rx_rate.rate = staInfoTab.Entry[i].lastRxRate * 1000;	

			/*no need to set this filed*/
			entry.tx_rate.mcs = 0;
			entry.tx_rate.is_40mhz = 1;
			entry.tx_rate.is_short_gi = 1;
			
			entry.rx_rate.mcs = 0;
			entry.rx_rate.is_40mhz = 1;
			entry.rx_rate.is_short_gi = 1;
			
			memcpy(&buf[j], &entry, sizeof(struct iwinfo_assoclist_entry));
			j += sizeof(struct iwinfo_assoclist_entry);
		}
		*len = j;
		return 0;
	}
	
	return -1;
}


#if 0
int mtk_get_assoclist(const char *ifname, char *buf, int *len)
{
	struct iwinfo_assoclist_entry entry;
	RT_802_11_MAC_TABLE *mt;
	MACHTTRANSMIT_SETTING rxrate;
	char raname[IFNAMSIZ],raidx[IFNAMSIZ],raiidx[IFNAMSIZ];
	int mtlen=sizeof(RT_802_11_MAC_TABLE);
	int i,j;

	if ((mt = (RT_802_11_MAC_TABLE *) malloc(mtlen)) == NULL)
	{
		return -1;
	}

	memset(mt, 0, mtlen);
	snprintf(raname, sizeof(raname), "%s", _devname_to_ifname(ifname));

	if (mtk_get80211priv(_devname_to_ifname(ifname), RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, mt, mtlen) > 0)
	{
		j = 0;

		for (i = 0; i < mt->Num && i < MAX_NUMBER_OF_MAC; i++)
		{
			snprintf(raidx, sizeof(raidx), "ra%d", mt->Entry[i].ApIdx);
			snprintf(raiidx, sizeof(raiidx), "rai%d", mt->Entry[i].ApIdx);

			if( strncmp(raname, raidx, IFNAMSIZ) && strncmp(raname, raiidx, IFNAMSIZ) ) continue;

			memset(&entry, 0, sizeof(entry));

			memcpy(entry.mac, &mt->Entry[i].Addr, 6);

			if(mt->Entry[i].AvgRssi0 > mt->Entry[i].AvgRssi1)
				entry.signal = mt->Entry[i].AvgRssi0;
			else
				entry.signal = mt->Entry[i].AvgRssi1;

			entry.noise  = -95;

			entry.inactive = mt->Entry[i].ConnectedTime * 1000;

			rxrate.word = mt->Entry[i].LastRxRate;
			entry.tx_rate.rate = mtk_get_rate(mt->Entry[i].TxRate) * 1000;
			entry.rx_rate.rate = mtk_get_rate(rxrate) * 1000;

			entry.tx_rate.mcs = mt->Entry[i].TxRate.field.MCS;
			entry.rx_rate.mcs = rxrate.field.MCS;

			entry.tx_packets = 0;
			entry.rx_packets = 0;

			if(mt->Entry[i].TxRate.field.BW) entry.tx_rate.is_40mhz = 1;
			if(mt->Entry[i].TxRate.field.ShortGI) entry.tx_rate.is_short_gi = 1;
			if(rxrate.field.BW) entry.rx_rate.is_40mhz = 1;
			if(rxrate.field.ShortGI) entry.rx_rate.is_short_gi = 1;

			memcpy(&buf[j], &entry, sizeof(struct iwinfo_assoclist_entry));
			j += sizeof(struct iwinfo_assoclist_entry);
		}
		*len = j;
		free(mt);
		return 0;
	}

	free(mt);
	return -1;
}
#endif 

int mtk_get_txpwrlist(const char *ifname, char *buf, int *len)
{
	return -1;
}

static int ascii2num(char ascii)
{
	int num;
	if ((ascii >= '0') && (ascii <= '9'))
		num=ascii - 48;
	else if ((ascii >= 'a') && (ascii <= 'f'))
		num=ascii - 'a' + 10;
        else if ((ascii >= 'A') && (ascii <= 'F'))
		num=ascii - 'A' + 10;
	else
		num = 0;
	return num;
}

static void next_field(char **line, char *output, int n, int m) {
	while (**line == ' ') (*line)++;
	char *sep = strchr(*line, ' ');

	while (m-- >0) {
		if (*(*line+m) != ' ') {
			sep= *line+m+1;
			break;
		}
	}

	if (sep) {
		*sep = '\0';
		sep++;
	}

	strncpy(output, *line, n);

	/* Update the line token for the next call */
	*line = sep;
}

static int mtk_get_scan(const char *ifname, struct survey_table *st)
{
	int survey_count = 0;
	char ss[64] = "SiteSurvey=1";
	char *p;
	SCAN_BSS_TABLE scan_table;    

#ifdef MTK_NETLINK_SUPPORT
	if( mtk_set80211priv(_devname_to_ifname(ifname), ss) < 0 )
		return -1;
#else
	if( mtk_get80211priv(ifname, RTPRIV_IOCTL_SET, ss, sizeof(ss)) < 0 )
		return -1;
#endif

	sleep(3);

#ifdef MTK_NETLINK_SUPPORT
	if (mtk_get80211priv(_devname_to_ifname(ifname), MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_SCANINFO, get_scaninfo_callback,
		&scan_table, sizeof(SCAN_BSS_TABLE)) < 0)
		return -1;
#else
	if( mtk_get80211priv(ifname, RTPRIV_IOCTL_GSITESURVEY, &scan_table, sizeof(scan_table)) < 1 )
		return -1;
#endif
	for (survey_count = 0; survey_count < scan_table.BssNr; survey_count++)
	{
		/* channel */
		st[survey_count].channel = scan_table.ScanTbl[survey_count].channel;
		/* ssid */
		strncpy(st[survey_count].ssid, scan_table.ScanTbl[survey_count].ssid, MAX_LEN_OF_SSID);
		/* bssid */
		sprintf(st[survey_count].bssid, "%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(scan_table.ScanTbl[survey_count].bssid));
		/* crypto&enc */
		sprintf(st[survey_count].enc, "%s\n", GetAuthModeStr(scan_table.ScanTbl[survey_count].authMode));
		sprintf(st[survey_count].crypto, "%s\n", GetEncryModeStr(scan_table.ScanTbl[survey_count].encrypType));
		//printf("tqj->ssid %s, get enc %d, %s, crypto %d, %s\n",scan_table.ScanTbl[survey_count].ssid, scan_table.ScanTbl[survey_count].authMode, GetAuthModeStr(scan_table.ScanTbl[survey_count].authMode), scan_table.ScanTbl[survey_count].encrypType, GetEncryModeStr(scan_table.ScanTbl[survey_count].encrypType));
		/* strength */
		st[survey_count].strength = scan_table.ScanTbl[survey_count].rssi;
		/* wireless mode */
		/*Since iwinfo do not care about wireless mode, do not support it so far --tqj*/
		sprintf(st[survey_count].mode, "%s\n", "None");
	}
	return scan_table.BssNr;
}

int mtk_get_scanlist(const char *ifname, char *buf, int *len)
{
	int sc,i,j=0,h;
	char *p;
	struct survey_table stl[MAX_NUM_OF_SURVEY_CNT];
	struct iwinfo_scanlist_entry sce;

	sc = mtk_get_scan(_devname_to_ifname(ifname), stl);
	if ( sc < 1)
		return -1;
	
	for (i = 0; i < sc; i++)
	{
		memset(&sce, 0, sizeof(sce));

		for (h = 0; h < 6; h++)
			sce.mac[h] = (uint8_t)(ascii2num(stl[i].bssid[h*3]) * 16 + ascii2num(stl[i].bssid[h*3+1]));

		memcpy(sce.ssid, stl[i].ssid, sizeof(sce.ssid));
		sce.ssid[33] = '\0';
		sce.channel=(uint8_t) stl[i].channel;
		sce.quality = (uint8_t) stl[i].strength;
		sce.quality_max = 100;
		sce.signal = 155 + sce.quality;
		sce.mode=1;
		if (strncmp(stl[i].enc,"OPEN", 4))
		{
			sce.crypto.enabled=1;
			if (!strncmp(stl[i].enc,"WPAPSKWPA2PSK", 13))
			{
				sce.crypto.wpa_version = 3;
				sce.crypto.auth_suites = IWINFO_KMGMT_PSK;
			}
			else if (!strncmp(stl[i].enc,"WPA2PSK", 7))
			{
				sce.crypto.wpa_version = 2;
				sce.crypto.auth_suites = IWINFO_KMGMT_PSK;
			}
			else if (!strncmp(stl[i].enc,"WPAPSK", 6))
			{
				sce.crypto.wpa_version = 1;
				sce.crypto.auth_suites = IWINFO_KMGMT_PSK;
			}
			else if (!strncmp(stl[i].enc,"WPA1WPA2", 8))
			{
				sce.crypto.wpa_version = 3;
				sce.crypto.auth_suites = IWINFO_KMGMT_8021x;
			}

			if (!strncmp(stl[i].crypto,"TKIPAES", 7))
				sce.crypto.pair_ciphers = (IWINFO_CIPHER_TKIP | IWINFO_CIPHER_CCMP);
			else if (!strncmp(stl[i].crypto,"AES", 3))
				sce.crypto.pair_ciphers = IWINFO_CIPHER_CCMP;
			else if (!strncmp(stl[i].crypto,"TKIP", 4))
				sce.crypto.pair_ciphers = IWINFO_CIPHER_TKIP;
		}
		else
		{
			if (!strncmp(stl[i].crypto,"WEP", 3))
			{
				sce.crypto.enabled=1;
				sce.crypto.wpa_version = 0;
				sce.crypto.auth_algs = (IWINFO_AUTH_OPEN | IWINFO_AUTH_SHARED);
				sce.crypto.pair_ciphers = (IWINFO_CIPHER_WEP104 | IWINFO_CIPHER_WEP40);
				sce.crypto.auth_suites = IWINFO_KMGMT_NONE;
			}
			else
				sce.crypto.enabled=0;
		}
		
		memcpy(&buf[j], &sce, sizeof(struct iwinfo_scanlist_entry));
		j += sizeof(struct iwinfo_scanlist_entry);
	}
	
	*len = j;
	return 0;
}

int mtk_get_freqlist(const char *ifname, char *buf, int *len)
{
	struct MTK_CHANINFO chans;
	struct iwinfo_freqlist_entry entry;
	int i, bl = 0;
	int chan_num;
	CH_FREQ_MAP *pChHzMap = NULL;

	int 			fd;
	unsigned char	country[COUNTRY_CODE_MAX_LEN+1] = COUNTRY_DE;	

#ifdef MTK_NETLINK_SUPPORT
	if (mtk_get80211priv(_devname_to_ifname(ifname), MTK_NL80211_VENDOR_ATTR_GET_RUNTIME_INFO_GET_CHANLIST, get_chanlist_callback,
		&chans, sizeof(struct MTK_CHANINFO)) < 0 || chans.mtk_nchans==0)
#else
	/*for MT7613 ioctl call will success even though interface was down*/
	if (mtk_get80211priv(_devname_to_ifname(ifname), RTPRIV_IOCTL_GCHANLIST, &chans, sizeof(struct MTK_CHANINFO)) < 0 || chans.mtk_nchans==0)
#endif
	{
		printf("fail to get chanlist from driver, use default\n");

		fd = popen("echo $(getfirm COUNTRY) | tr -d \"\\n\"", "r");
		if (fd)
		{
			memset(country, 0, COUNTRY_CODE_MAX_LEN+1);
			fread(country, sizeof(char), COUNTRY_CODE_MAX_LEN, fd);
			pclose(fd);

			country[COUNTRY_CODE_MAX_LEN] = '\0';
			printf("Got country is: %s\n", country);
		}
		//printf("Used country is: %s\n", country);

#ifdef WIFI_TRIBAND_SUPPORT
		if (!strncmp(ifname, "rai", 3))
#else
#ifdef PACKAGE_WIFI_DBDC_MODE
		if (!strncmp(ifname, "rax", 3))
#else
		if (!strncmp(ifname, "rai", 3))
#endif
#endif
		{
			if (!strncmp(country, COUNTRY_DE, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_EU_5G;
				chan_num = sizeof(CH_HZ_OF_EU_5G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_JP, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_JP_5G;
				chan_num = sizeof(CH_HZ_OF_JP_5G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_RU, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_RU_5G;
				chan_num = sizeof(CH_HZ_OF_RU_5G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_US, COUNTRY_CODE_MAX_LEN) ||
				!strncmp(country, COUNTRY_CA, COUNTRY_CODE_MAX_LEN) ||
				!strncmp(country, COUNTRY_TW, COUNTRY_CODE_MAX_LEN) ||
				!strncmp(country, COUNTRY_SG, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_US_5G;
				chan_num = sizeof(CH_HZ_OF_US_5G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_KR, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_KR_5G;
				chan_num = sizeof(CH_HZ_OF_KR_5G) / sizeof(CH_FREQ_MAP);
			}
			else
			{
				pChHzMap = &CH_HZ_OF_EU_5G;
				chan_num = sizeof(CH_HZ_OF_EU_5G) / sizeof(CH_FREQ_MAP);
			}
		}
#ifdef WIFI_TRIBAND_SUPPORT
		else if (!strncmp(ifname, "rax", 3))
		{
			//TODO for other countries
			if (!strncmp(country, COUNTRY_US, COUNTRY_CODE_MAX_LEN) ||
				!strncmp(country, COUNTRY_CA, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_US_6G;
				chan_num = sizeof(CH_HZ_OF_US_6G) / sizeof(CH_FREQ_MAP);
			}
		}
#endif
		else
		{
			if (!strncmp(country, COUNTRY_DE, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_EU_2G;
				chan_num = sizeof(CH_HZ_OF_EU_2G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_JP, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_JP_2G;
				chan_num = sizeof(CH_HZ_OF_JP_2G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_RU, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_RU_2G;
				chan_num = sizeof(CH_HZ_OF_RU_2G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_US, COUNTRY_CODE_MAX_LEN) ||
					!strncmp(country, COUNTRY_CA, COUNTRY_CODE_MAX_LEN) ||
					!strncmp(country, COUNTRY_TW, COUNTRY_CODE_MAX_LEN) ||
					!strncmp(country, COUNTRY_SG, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_US_2G;
				chan_num = sizeof(CH_HZ_OF_US_2G) / sizeof(CH_FREQ_MAP);
			}
			else if (!strncmp(country, COUNTRY_KR, COUNTRY_CODE_MAX_LEN))
			{
				pChHzMap = &CH_HZ_OF_KR_2G;
				chan_num = sizeof(CH_HZ_OF_KR_2G) / sizeof(CH_FREQ_MAP);
			}
			else
			{
				pChHzMap = &CH_HZ_OF_EU_2G;
				chan_num = sizeof(CH_HZ_OF_EU_2G) / sizeof(CH_FREQ_MAP);
			}
		}
 
		//printf("tqj-->chan num is %d\n", chan_num);
		for (i = 0; i < chan_num; i++)
		{
			entry.mhz = (pChHzMap + i)->freqMHz;
			entry.channel = (pChHzMap + i)->channel;
			entry.restricted = entry.channel <= CH_MAX_2G_CHANNEL ? 0x80 : 0;
			memcpy(&buf[bl], &entry, sizeof(struct iwinfo_freqlist_entry));
			bl += sizeof(struct iwinfo_freqlist_entry);
		}
		
		*len = bl;

		return 0;
	}
	for (i = 0; i < chans.mtk_nchans; i++)
	{	
		entry.mhz        = chans.mtk_chans[i].ic_freq / 1000;
		entry.channel    = chans.mtk_chans[i].ic_ieee;
		entry.restricted = entry.channel <= CH_MAX_2G_CHANNEL ? 0x80 : 0;

		memcpy(&buf[bl], &entry, sizeof(struct iwinfo_freqlist_entry));
		bl += sizeof(struct iwinfo_freqlist_entry);
	}
	*len = bl;

	return 0;
}

int mtk_get_country(const char *ifname, char *buf)
{
	sprintf(buf, "00");
	return 0;
}

int mtk_get_countrylist(const char *ifname, char *buf, int *len)
{
	/* Stub */
	return -1;
}

int mtk_get_hwmodelist(const char *ifname, int *buf)
{

	WLAN_BSS_INFO *bss;
	if ((bss = (WLAN_BSS_INFO *) malloc(sizeof(WLAN_BSS_INFO))) == NULL)
	{
		return -1;
	}
	memset(bss, 0, sizeof(WLAN_BSS_INFO));
	mtk_get_vapinfo(_devname_to_ifname(ifname), bss);
	if (bss->phymode < 0 || bss->phymode >= PHY_MODE_MAX)
	{
		free(bss);
		return -1;
	}
	*buf = CFG_WMODE_MAP[2*(bss->phymode) + 1];
	free(bss);
	return 0;
}

int mtk_get_encryption(const char *ifname, char *buf)
{
	struct iwinfo_crypto_entry *crypto = (struct iwinfo_crypto_entry *)buf;
	WLAN_BSS_INFO bss;
	memset(&bss, 0, sizeof(WLAN_BSS_INFO));
	mtk_get_vapinfo(_devname_to_ifname(ifname), &bss);
/* to do: need to figure out the detail encrypt*/
	if (bss.securityEnable)
	{
		crypto->enabled=1;
	}
	else
	{
		crypto->enabled=0;
	}
	
	return 0;
}

int mtk_get_phyname(const char *ifname, char *buf)
{
	/* No suitable api in mtk */
	strcpy(buf, ifname);
	return 0;
}

int mtk_get_mbssid_support(const char *ifname, int *buf)
{
	return -1;
}

int mtk_get_hardware_id(const char *ifname, char *buf)
{
	return -1;
}

int mtk_get_hardware_name(const char *ifname, char *buf)
{
	sprintf(buf, "Generic Mediatek/Ralink");
	return 0;
}

int mtk_get_txpower_offset(const char *ifname, int *buf)
{
	/* Stub */
	*buf = 0;
	return -1;
}

int mtk_get_frequency_offset(const char *ifname, int *buf)
{
	/* Stub */
	*buf = 0;
	return -1;
}

const struct iwinfo_ops mtk_ops = {
	.name             = "mtk",
	.probe            = mtk_probe,

	.channel          = mtk_get_channel,
	.frequency        = mtk_get_frequency,
	.frequency_offset = mtk_get_frequency_offset,
	.txpower          = mtk_get_txpower,
	.txpower_offset   = mtk_get_txpower_offset,
	.bitrate          = mtk_get_bitrate,
	.signal           = mtk_get_signal,
	.noise            = mtk_get_noise,
	.quality          = mtk_get_quality,
	.quality_max      = mtk_get_quality_max,
	.mbssid_support   = mtk_get_mbssid_support,
	.hwmodelist       = mtk_get_hwmodelist,
	.mode             = mtk_get_mode,
	.ssid             = mtk_get_ssid,
	.bssid            = mtk_get_bssid,
	.country          = mtk_get_country,
	.hardware_id      = mtk_get_hardware_id,
	.hardware_name    = mtk_get_hardware_name,
	.encryption       = mtk_get_encryption,
	.assoclist        = mtk_get_assoclist,
	.txpwrlist        = mtk_get_txpwrlist,
	.scanlist         = mtk_get_scanlist,
	.freqlist         = mtk_get_freqlist,
	.countrylist      = mtk_get_countrylist,
	.close            = mtk_close
};

