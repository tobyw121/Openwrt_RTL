/*
 * Copyright (c) 2004-2011 Atheros Communications Inc.
 * Copyright (c) 2011-2012 Qualcomm Atheros, Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/unistd.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/delay.h>
#endif

#include "./8192cd_cfg.h"

#ifdef RTK_NL80211

#include <linux/kernel.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/cfg80211.h>
#include <net/mac80211.h>
#include <net/ieee80211_radiotap.h>
#include <linux/wireless.h>
#include <linux/device.h>
#include <linux/if_ether.h>
#include <linux/nl80211.h>
#include <linux/ieee80211.h>

#ifdef __LINUX_2_6__
#include <linux/initrd.h>
//#include <linux/syscalls.h>
#endif

#include "./8192cd.h"
#include "./8192cd_debug.h"
#include "./8192cd_cfg80211.h"
#include "./8192cd_headers.h"
#include "./8192cd_p2p.h"

#include <net80211/ieee80211.h>
#include <net80211/ieee80211_crypto.h>
#include <net80211/ieee80211_ioctl.h>
#include "./8192cd_net80211.h"

//#define DEBUG_NL80211_02
#ifdef DEBUG_NL80211_02
//#define NLENTER	{}
//#define NLEXIT {}
#define NLENTER	printk("[%s][%s] +++ \n", priv->dev->name, (char *)__func__)
#define NLEXIT  printk("[%s][%s] --- \n\n", priv->dev->name, (char *)__func__)
#define NLINFO(fmt, args...) printk("[%s %d]"fmt, __func__,__LINE__, ## args)
#define NLNOT   printk("[%s %d] !!! NOT implement YET !!!\n", (char *)__func__,__LINE__)
#else
#define NLENTER
#define NLEXIT
#define NLINFO(fmt, args...) do{}while(0)
#define NLNOT
#endif

#define CATEGORY_IS_NON_ROBUST(cat) \
	(cat == _PUBLIC_CATEGORY_ID_ \
	|| cat == _VENDOR_ACTION_ID_)

#define CATEGORY_IS_ROBUST(cat) !CATEGORY_IS_NON_ROBUST(cat)

void void_printk_nl80211(const char *fmt, ...)
{
	;
}


#ifdef DEBUG_NL80211_02
#define NLMSG	panic_printk
#else
#define NLMSG	void_printk_nl80211
#endif

//#define	SIGNAL_TYPE_UNSPEC
u8 hostapd_acs_scaning = 0;
u8 wpa_oui[] = {0x00, 0x50, 0xf2, 0x01};
u8 dpp_oui_str[3]={0x50, 0x6f, 0x9a};
/*HAPD_OWE*/
u8 owe_transition_oui[] = {0x50, 0x6f, 0x9a, 0x1c};
extern unsigned char WFA_OUI_PLUS_TYPE[];

#define RTK_MAX_WIFI_PHY 2
static int rtk_phy_idx=0;
struct rtknl *rtk_phy[RTK_MAX_WIFI_PHY];
static dev_t rtk_wifi_dev[RTK_MAX_WIFI_PHY];
#if (defined(CONFIG_WLAN_HAL_8192FE)|| defined(CONFIG_WLAN_HAL_8197G)) && defined(CONFIG_AX1500)
#ifdef CONFIG_2G_ON_WLAN0
static char *rtk_dev_name[RTK_MAX_WIFI_PHY]={"wlan0","wlan1"};
#else
static char *rtk_dev_name[RTK_MAX_WIFI_PHY]={"wlan1","wlan0"};
#endif
#else
static char *rtk_dev_name[RTK_MAX_WIFI_PHY]={"RTKWiFi0","RTKWiFi1"};
#endif
char rtk_fake_addr[6]={0x00,0xe0,0x4c,0xcc,0xdd,0x01}; //mark_dual , FIXME if wlan_mac readable

#define MAX_5G_DIFF_NUM		14
#define PIN_LEN					8
#define SIGNATURE_LEN			4
#if 0
#ifdef CONFIG_RTL_HW_SETTING_OFFSET
#define HW_SETTING_OFFSET		CONFIG_RTL_HW_SETTING_OFFSET
#else
#define HW_SETTING_OFFSET		0x6000
#endif
#endif
extern unsigned int HW_SETTING_OFFSET; //mark_hw,from rtl819x_flash.c
#define HW_WLAN_SETTING_OFFSET	13

__PACK struct hw_wlan_setting {
	unsigned char macAddr[6] ;
	unsigned char macAddr1[6] ;
	unsigned char macAddr2[6] ;
	unsigned char macAddr3[6] ;
	unsigned char macAddr4[6] ;
	unsigned char macAddr5[6] ;
	unsigned char macAddr6[6] ;
	unsigned char macAddr7[6] ;
	unsigned char pwrlevelCCK_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrlevelCCK_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrlevelHT40_1S_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrlevelHT40_1S_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiffHT40_2S[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiffHT20[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiffOFDM[MAX_2G_CHANNEL_NUM] ;
	unsigned char regDomain ;
	unsigned char rfType ;
	unsigned char ledType ; // LED type, see LED_TYPE_T for definition
	unsigned char xCap ;
	unsigned char TSSI1 ;
	unsigned char TSSI2 ;
	unsigned char Ther ;
	unsigned char Reserved1 ;
	unsigned char Reserved2 ;
	unsigned char Reserved3 ;
	unsigned char Reserved4 ;
	unsigned char Reserved5 ;
	unsigned char Reserved6 ;
	unsigned char Reserved7 ;
	unsigned char Reserved8 ;
	unsigned char Reserved9 ;
	unsigned char Reserved10 ;
	unsigned char pwrlevel5GHT40_1S_A[MAX_5G_CHANNEL_NUM] ;
	unsigned char pwrlevel5GHT40_1S_B[MAX_5G_CHANNEL_NUM] ;
	unsigned char pwrdiff5GHT40_2S[MAX_5G_CHANNEL_NUM] ;
	unsigned char pwrdiff5GHT20[MAX_5G_CHANNEL_NUM] ;
	unsigned char pwrdiff5GOFDM[MAX_5G_CHANNEL_NUM] ;
	unsigned char wscPin[PIN_LEN+1] ;
#ifdef RTK_AC_SUPPORT
	unsigned char pwrdiff_20BW1S_OFDM1T_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW2S_20BW2S_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM2T_CCK2T_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW3S_20BW3S_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_4OFDM3T_CCK3T_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW4S_20BW4S_A[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM4T_CCK4T_A[MAX_2G_CHANNEL_NUM] ;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_A[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_A[MAX_5G_DIFF_NUM] ;


	unsigned char pwrdiff_20BW1S_OFDM1T_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW2S_20BW2S_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM2T_CCK2T_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW3S_20BW3S_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM3T_CCK3T_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_40BW4S_20BW4S_B[MAX_2G_CHANNEL_NUM] ;
	unsigned char pwrdiff_OFDM4T_CCK4T_B[MAX_2G_CHANNEL_NUM] ;

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW2S_20BW2S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW3S_20BW3S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_40BW4S_20BW4S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_RSVD_OFDM4T_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW1S_160BW1S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW2S_160BW2S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW3S_160BW3S_B[MAX_5G_DIFF_NUM] ;
	unsigned char pwrdiff_5G_80BW4S_160BW4S_B[MAX_5G_DIFF_NUM] ;

	unsigned char pwrdiff_20BW1S_OFDM1T_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_C[MAX_2G_CHANNEL_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_C[MAX_5G_CHANNEL_NUM];

	unsigned char pwrdiff_20BW1S_OFDM1T_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_40BW2S_20BW2S_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_OFDM2T_CCK2T_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_40BW3S_20BW3S_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_4OFDM3T_CCK3T_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_40BW4S_20BW4S_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrdiff_OFDM4T_CCK4T_D[MAX_2G_CHANNEL_NUM];

	unsigned char pwrdiff_5G_20BW1S_OFDM1T_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_40BW2S_20BW2S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_40BW3S_20BW3S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_40BW4S_20BW4S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_RSVD_OFDM4T_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW1S_160BW1S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW2S_160BW2S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW3S_160BW3S_D[MAX_5G_CHANNEL_NUM];
	unsigned char pwrdiff_5G_80BW4S_160BW4S_D[MAX_5G_CHANNEL_NUM];

	unsigned char pwrlevelCCK_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevelCCK_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevelHT40_1S_C[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevelHT40_1S_D[MAX_2G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_C[MAX_5G_CHANNEL_NUM];
	unsigned char pwrlevel5GHT40_1S_D[MAX_5G_CHANNEL_NUM];
#endif
}__WLAN_ATTRIB_PACK__;

__PACK struct param_header {
	unsigned char signature[SIGNATURE_LEN];  // Tag + version
	unsigned short len ;
} __WLAN_ATTRIB_PACK__;

unsigned char is_WRT_scan_iface(const char* if_name)
{
#ifdef CONFIG_OPENWRT_SDK
	if((strcmp(if_name, "tmp.wlan0")==0) || (strcmp(if_name, "tmp.wlan1")==0))
		return 1;
	else
#endif
		return 0;
}

int is_zero_mac(const unsigned char *mac)
{
	return !(mac[0] | mac[1] | mac[2] | mac[3] | mac[4] | mac[5]| mac[6]| mac[7]);
}

void dump_mac(struct rtl8192cd_priv *priv, unsigned char *mac)
{
	if(mac && !is_zero_mac(mac)){
		NDEBUG(" %pm\n", mac);
    }
}

void rtk_cfg80211_rx_mgmt(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo)
{
	struct wireless_dev *wdev;
	u8 *pframe;
	int freq = 0;
	int channel;
	pframe = get_pframe(pfrinfo);
	wdev = (&priv->wdev);
	channel = (int)priv->pmib->dot11RFEntry.dot11channel;
	if(priv->pshare->curr_band == BAND_2G)
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_2GHZ);
	else
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_5GHZ);

	cfg80211_rx_mgmt(wdev, freq, 0, pframe, pfrinfo->pktlen, GFP_ATOMIC);
}
#if defined(P2P_SUPPORT)
// function declaration
int realtek_remain_on_channel(struct wiphy *wiphy,
    struct wireless_dev *wdev,
	struct ieee80211_channel *channel,
	unsigned int duration,
	u64 *cookie);

static int realtek_cancel_remain_on_channel(struct wiphy *wiphy,
			struct wireless_dev *wdev,	u64 cookie);

#endif //CONFIG_P2P

#if defined(VAP_MAC_DRV_READ_FLASH)
int read_flash_hw_mac_vap(unsigned char *mac, int vap_idx)
{
	unsigned int offset;

	//NLENTER;

	if(!mac)
		return -1;

	vap_idx +=1;

	if(vap_idx > 7)
		return -1;

	offset = HW_SETTING_OFFSET+ sizeof(struct param_header)+ HW_WLAN_SETTING_OFFSET + sizeof(struct hw_wlan_setting) * (rtk_phy_idx-1);
	offset += (vap_idx*ETH_ALEN);
	offset |= 0xbd000000;
	memcpy(mac,(unsigned char *)offset,ETH_ALEN);

	if(is_zero_mac(mac))
		return -1;

	DEBUG_INFO("VAP[%d][%d]=%pm\n", (rtk_phy_idx-1), vap_idx, mac);

	return 0;
}
#endif

//brian, get MAC address from utility, rtk_tx_calr
#if 0
void read_flash_hw_mac( unsigned char *mac,int idx)
{
	unsigned int offset;
	if(!mac)
		return;
	offset = HW_SETTING_OFFSET+ sizeof(struct param_header)+ HW_WLAN_SETTING_OFFSET + sizeof(struct hw_wlan_setting) * idx;
	offset |= 0xbd000000;
	memcpy(mac,(unsigned char *)offset,ETH_ALEN);
}
#endif
//#define CPTCFG_CFG80211_MODULE 1 // mark_com

#if 1 //_eric_nl event

struct rtl8192cd_priv* realtek_get_priv(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);

	return (dev) ? (GET_DEV_PRIV(dev)) : (rtk->priv);
}

struct ieee80211_channel *rtk_get_iee80211_channel(struct wiphy *wiphy, unsigned int channel)
{
	unsigned int  freq = 0;

	if(channel >= 34)
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_5GHZ);
	else
		freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_2GHZ);

	return ieee80211_get_channel(wiphy, freq);

}

int translate_ss_result_rssi(int rssi)
{

#if !defined(SIGNAL_TYPE_UNSPEC)
	//reference libiwinfo/iwinfo_nl80211.c - scan_cb
	return ((rssi-100)*100);
#else
	return rssi;
#endif
}

void realtek_cfg80211_inform_bss(struct rtl8192cd_priv *priv)
{
	struct wiphy *wiphy = priv->rtk->wiphy;
	struct ieee80211_channel *notify_channel=NULL;
	struct cfg80211_bss *bss = NULL;
	u8 *pbuf=NULL;
	size_t pbuflen=0;
	s32 notify_signal=0;

	notify_channel = rtk_get_iee80211_channel(wiphy, priv->pmib->dot11Bss.channel);
	if(notify_channel == NULL){
		NDEBUG2("CHK!!\n\n");
		return;
	}
	pbuf = priv->pmib->dot11Bss.WholeFrame;
	pbuflen = priv->pmib->dot11Bss.WholeFrameLength;
	notify_signal = translate_ss_result_rssi(priv->pmib->dot11Bss.rssi);

	bss = cfg80211_inform_bss_frame(wiphy, notify_channel,
		(struct ieee80211_mgmt *)pbuf, pbuflen, notify_signal, GFP_ATOMIC);

	if (bss)
		cfg80211_put_bss(wiphy,bss);
	else 
		NDEBUG2("CHK!!\n\n");

}

void realtek_cfg80211_inform_ss_result(struct rtl8192cd_priv *priv)
{
	int i;
	struct wiphy *wiphy = priv->rtk->wiphy;
	struct ieee80211_channel *notify_channel=NULL;
	struct cfg80211_bss *bss = NULL;
	u8 *pbuf=NULL;
	size_t pbuflen=0;
	s32 notify_signal=0;

	NLMSG("SiteSurvey Count=%d\n", priv->site_survey->count);

	if (!priv->scan_req) {
		/*No scan_req, No report to NL80211*/
		NDEBUG2("CHK!!");
		return;
	}

	/*AP mode skip auto 20/40M force to 40M band width
	   skip scan , report nope scan result to hapd */
#if 1
	if ((OPMODE & WIFI_AP_STATE)
		&& priv->nl80211_state != NL80211_OPEN && priv->hapd_ap_2040m_scan == 0) {
		NDEBUG2("\n skip cfg80211 inform result\n\n");
		return;
	}
#endif
	NDEBUG2("\n");

	for (i = 0; i < priv->site_survey->count; i++) {
		notify_channel = rtk_get_iee80211_channel(wiphy, priv->site_survey->bss[i].channel);
		if (notify_channel == NULL) {
			NDEBUG2("\n	CHK!\n");
			continue;
		}
		pbuf = priv->site_survey->bss[i].WholeFrame;
		pbuflen = priv->site_survey->bss[i].WholeFrameLength;
		notify_signal = translate_ss_result_rssi(priv->site_survey->bss[i].rssi);
		bss = cfg80211_inform_bss_frame(
			wiphy, notify_channel, (struct ieee80211_mgmt *)pbuf, pbuflen, notify_signal, GFP_ATOMIC);

		//dump_hex("inform frame",pbuf,pbuflen);
		if (bss){
			cfg80211_put_bss(wiphy,bss);
			//NDEBUG2("cfg80211_put_bss\n");
		}else 
			NDEBUG2("\n	CHK!\n");

	}
}

// static void realtek_cfg80211_sscan_disable(struct rtl8192cd_priv *priv) rename to easy understand name rtk_abort_scan
static void rtk_abort_scan(struct rtl8192cd_priv *priv, enum scan_abort_case abort_case)
{
/*
	struct rtl8192cd_priv *priv_root = GET_ROOT(priv);
	struct cfg80211_scan_request *scan_req = priv_root->scan_req;
*/

	NLMSG("[%s] rtk_abort_scan [%p]+++ \n", priv->dev->name, GET_ROOT(priv)->scan_req);

#if 0
	if(priv_root->scan_req)
	{
		priv_root->ss_req_ongoing = 0;
		priv_root->scan_req = NULL;
	}

	if(priv->scan_req) //VXD can also do scan
	{
		priv->ss_req_ongoing = 0;
		priv->scan_req = NULL;
	}

	//event_indicate_cfg80211(priv, NULL, CFG80211_SCAN_ABORDED, NULL);
	//cfg80211_sched_scan_stopped(wiphy);
#else
	event_indicate_cfg80211(priv, NULL, CFG80211_SCAN_ABORTED, NULL);
#ifdef UNIVERSAL_REPEATER
	if (IS_ROOT_INTERFACE(priv))
	{
		struct rtl8192cd_priv *priv_vxd = GET_VXD_PRIV(priv);

		if (IS_DRV_OPEN(priv_vxd))
			event_indicate_cfg80211(priv_vxd, NULL, CFG80211_SCAN_ABORTED, NULL);
	}
#endif /* UNIVERSAL_REPEATER */
#endif
}



#define HAPD_READY_RX_EVENT	5

void event_to_name(int event, char *event_name)
{

	switch (event) {
	case CFG80211_CONNECT_RESULT:
		strcpy(event_name, "CFG80211_CONNECT_RESULT");
		break;
	case CFG80211_ROAMED:
		strcpy(event_name, "CFG80211_ROAMED");
		break;
	case CFG80211_DISCONNECTED:
		strcpy(event_name, "CFG80211_DISCONNECTED");
		break;
	case CFG80211_IBSS_JOINED:
		strcpy(event_name, "CFG80211_IBSS_JOINED");
		break;
    case CFG80211_NEW_STA:
        strcpy(event_name, "CFG80211_NEW_STA");
        break;
	case CFG80211_SCAN_DONE:
		strcpy(event_name, "CFG80211_SCAN_DONE");
		break;
	case CFG80211_SCAN_ABORTED:
		strcpy(event_name, "CFG80211_SCAN_ABORTED");
		break;
	case CFG80211_DEL_STA:
		strcpy(event_name, "CFG80211_DEL_STA");
		break;
	case CFG80211_RADAR_CAC_FINISHED:
		strcpy(event_name, "CFG80211_RADAR_CAC_FINISHED");
		break;
	case CFG80211_RADAR_DETECTED:
		strcpy(event_name, "CFG80211_RADAR_DETECTED");
		break;
	case CFG80211_RADAR_CAC_ABORTED:
		strcpy(event_name, "CFG80211_RADAR_CAC_ABORTED");
		break;
	default:
		strcpy(event_name, "UNKNOWN EVENT");
		break;
	}

}

#define ASSOC_REQ_BUF_LEN (256*2)
//u8 assoc_req_ies_buf[ASSOC_REQ_BUF_LEN];

int event_indicate_cfg80211(struct rtl8192cd_priv *priv, unsigned char *mac, int event, unsigned char *extra)
{
//	struct net_device	*dev = (struct net_device *)priv->dev;
	struct stat_info	*pstat = NULL;
	//struct station_info sinfo;
	struct wiphy *wiphy = priv->rtk->wiphy;
	unsigned short reason = 0;
#ifdef SMP_SYNC
	unsigned long flags;
#endif
	//NLENTER;
	{
		char event_name[32];
		event_to_name(event, event_name);
		NLMSG("EVENT [%s][%s=%d]\n", priv->dev->name, event_name, event);
	}

    /*cfg p2p 2014-0330 , report CFG80211_NEW_STA , ASAP*/
	if( (event != CFG80211_SCAN_ABORTED) && (event != CFG80211_SCAN_DONE)  && (event != CFG80211_NEW_STA) && (event != CFG80211_DEL_STA) && (event != CFG80211_RADAR_CAC_FINISHED)
		&& (event != CFG80211_RADAR_DETECTED) && (event != CFG80211_RADAR_CAC_ABORTED)){ //eric-bb
    	if( (OPMODE & WIFI_AP_STATE) && (priv->up_time <= HAPD_READY_RX_EVENT) )
    	{
    		NLMSG("ignore cfg event,up_time[%d],event[%d]\n", priv->up_time,event);
    		return -1;
    	}
    }

	if(mac)
		pstat = get_stainfo(priv, mac);

	SMP_LOCK_CFG80211(flags);

	switch(event) {
		case CFG80211_CONNECT_RESULT:
			{
				NDEBUG2("cfg80211_event [CFG80211_CONNECT_RESULT][%d]\n", event);
				struct cfg80211_bss *bss = NULL;
				struct ieee80211_channel *channel = NULL;
				channel = rtk_get_iee80211_channel(wiphy, priv->pmib->dot11Bss.channel);

				if(priv->receive_connect_cmd == 0)
				{
					NDEBUG2("Not received connect cmd yet !! No report CFG80211_CONNECT_RESULT\n");
					break;
				}
#if 1
				bss = cfg80211_get_bss(wiphy,
						channel, priv->pmib->dot11Bss.bssid,
						priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen,
						WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

				if(bss==NULL){
					NDEBUG2("report this bss\n");
					realtek_cfg80211_inform_bss(priv);
				}
#endif
				reason = (*(unsigned short *)(extra));
				if(reason){
					wpaslog("reason=[%d]",reason);
				}
				cfg80211_connect_result(priv->dev, BSSID,
						priv->rtk->clnt_info.assoc_req, priv->rtk->clnt_info.assoc_req_len,
						priv->rtk->clnt_info.assoc_rsp, priv->rtk->clnt_info.assoc_rsp_len,
						reason, GFP_ATOMIC);
			}
			break;
		case CFG80211_ROAMED:
			{
				NDEBUG2("cfg80211_event [CFG80211_ROAMED][%d]\n", event);
				break;
			}
			break;
		case CFG80211_DISCONNECTED:
			{
				//_eric_nl ?? disconnect event no mac, for station mode only ??
				NDEBUG2("cfg80211_event [CFG80211_DISCONNECTED][%d]\n", event);
#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0))
				if(extra)
					cfg80211_disconnected(priv->dev, le16_to_cpu(*(unsigned short *)(extra)), NULL, 0, 1, GFP_ATOMIC);
				else
					cfg80211_disconnected(priv->dev, 0, NULL, 0, 1, GFP_ATOMIC);
#else
				if(extra)
					cfg80211_disconnected(priv->dev, le16_to_cpu(*(unsigned short *)(extra)), NULL, 0, GFP_KERNEL);
				else
					cfg80211_disconnected(priv->dev, 0, NULL, 0, GFP_ATOMIC);
#endif
				break;
			}
			break;
		case CFG80211_IBSS_JOINED:
			{
				struct cfg80211_bss *bss = NULL;
				struct ieee80211_channel *channel = NULL;

				channel = rtk_get_iee80211_channel(wiphy, priv->pmib->dot11Bss.channel);

				bss = cfg80211_get_bss(wiphy,
						channel, priv->pmib->dot11Bss.bssid,
						priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen,
						WLAN_CAPABILITY_ESS, WLAN_CAPABILITY_ESS);

				if(bss==NULL)
				{
					printk("report this bss\n");
					realtek_cfg80211_inform_bss(priv);
				}


#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE >= KERNEL_VERSION(3, 15, 0))
				cfg80211_ibss_joined(priv->dev, BSSID, channel, GFP_ATOMIC);
#else
				cfg80211_ibss_joined(priv->dev, BSSID, GFP_ATOMIC);
#endif
			}
			break;
		case CFG80211_NEW_STA:
			{
#if 0
				NDEBUG2("cfg80211_event [CFG80211_NEW_STA][%d]\n", event);
				/* send event to application */
				memset(&sinfo, 0, sizeof(struct station_info));
				memset(assoc_req_ies_buf, 0, ASSOC_REQ_BUF_LEN);
		        sinfo.assoc_req_ies = assoc_req_ies_buf;

				if(pstat == NULL)
				{
					NDEBUG2("chk pstat, mac=[%pm]\n",mac);

					if(extra == NULL){
						NDEBUG2("NO PSTA for CFG80211_NEW_STA\n");
						break;
					} else
						pstat = (struct stat_info	*)extra;
				}

				/* TODO: sinfo.generation ???*/
				if(pstat->wpa_ie[1] > 0){
					sinfo.assoc_req_ies_len += pstat->wpa_ie[1]+2;
		            memcpy(ie_pos,pstat->wpa_ie, pstat->wpa_ie[1]+2);
		            ie_pos+=pstat->wpa_ie[1]+2;
		        }

				/*for wrt-wps*/
				if(pstat->wps_ie[1] > 0){
					sinfo.assoc_req_ies_len += pstat->wps_ie[1]+2;
		            memcpy(ie_pos,pstat->wps_ie, pstat->wps_ie[1]+2);
		            ie_pos+=pstat->wps_ie[1]+2;
				}

#if defined(P2P_SUPPORT)
		        /*p2p support , cfg p2p , 2014 0330 , report p2p_ie included in assoc_req*/
				if(pstat->p2p_ie[1] > 0)
				{
					sinfo.assoc_req_ies_len += pstat->p2p_ie[1]+2;
		            memcpy(ie_pos,pstat->p2p_ie, pstat->p2p_ie[1]+2);
		            ie_pos += (pstat->p2p_ie[1]+2);
				}
		        /*p2p support , cfg p2p , 2014 0330 , report p2p_ie included in assoc_req*/
#endif

#if defined(CONFIG_IEEE80211R)
				if (pstat->md_ie[1] > 0) {
					sinfo.assoc_req_ies_len += pstat->md_ie[1]+2;
					memcpy(ie_pos, pstat->md_ie, pstat->md_ie[1]+2);
					ie_pos += (pstat->md_ie[1]+2);
				}

				if (pstat->ft_ie[1] > 0) {
					sinfo.assoc_req_ies_len += pstat->ft_ie[1]+2;
					memcpy(ie_pos, pstat->ft_ie, pstat->ft_ie[1]+2);
					ie_pos += (pstat->ft_ie[1]+2);
				}
#endif

#if /*!defined(OPENWRT_CC) && */(LINUX_VERSION_CODE < KERNEL_VERSION(4, 0, 0))
				if(sinfo.assoc_req_ies_len)
					sinfo.filled |= STATION_INFO_ASSOC_REQ_IES;
#endif
				if(sinfo.assoc_req_ies_len>ASSOC_REQ_BUF_LEN){
					NDEBUG("!!!assoc_req_ies_len too long!!!\n");
				}
				SMP_UNLOCK_CFG80211(flags);
				//NDEBUG2("cfg80211_new_sta assoc req,[idx=%d] Rx assoc_req_ies_len = %d\n", priv->dev->ifindex, sinfo.assoc_req_ies_len);
				cfg80211_new_sta(priv->dev, mac, &sinfo, GFP_ATOMIC);
				//NDEBUG2("cfg80211_new_sta ,STA[%02x:%02x:%02x:%02x%:02x:%02x]\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
				netif_wake_queue(priv->dev); //wrt-vap
				SMP_LOCK_CFG80211(flags);
#else
				/*pass up to hapd at OnAssocReq()*/
				log("You shouldn't see this msg...");
#endif
			}

			break;
		case CFG80211_SCAN_ABORTED:
			{
				NDEBUG2("cfg80211_event [CFG80211_SCAN_ABORTED][%d]\n", event);
				priv->ss_req_ongoing = 0;
#ifdef USE_OUT_SRC
				priv->pshare->bScanInProcess = FALSE;
#endif
				if (priv->scan_req) {
					#ifdef CFG80211_FOR_BACKPORT
					struct cfg80211_scan_info info = {
						.aborted = true,
					};
					cfg80211_scan_done((struct cfg80211_scan_request *)priv->scan_req, &info);
					#else
					SMP_UNLOCK_CFG80211(flags);
					cfg80211_scan_done((struct cfg80211_scan_request *)priv->scan_req, true);
					SMP_LOCK_CFG80211(flags);
					#endif
					priv->scan_req = NULL;
				}

				/* RollBack to original channel */
				GET_ROOT(priv)->pshare->CurrentChannelBW = GET_ROOT(priv)->pshare->is_40m_bw;
				SwBWMode(GET_ROOT(priv), GET_ROOT(priv)->pshare->CurrentChannelBW, GET_ROOT(priv)->pshare->offset_2nd_chan);
				SwChnl(GET_ROOT(priv), GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel, GET_ROOT(priv)->pshare->offset_2nd_chan);
				RTL_W8(TXPAUSE, RTL_R8(TXPAUSE) & ~STOP_BCN);
			}
			break;
		case CFG80211_SCAN_DONE:
			{
				NDEBUG2("cfg80211_event [CFG80211_SCAN_DONE][%d]\n", event);
				priv->ss_req_ongoing = 0;
#ifdef USE_OUT_SRC
				priv->pshare->bScanInProcess = FALSE;
#endif
				priv->site_survey->count_backup = priv->site_survey->count;
				memcpy(priv->site_survey->bss_backup, priv->site_survey->bss, sizeof(struct bss_desc)*priv->site_survey->count);
#if defined(P2P_SUPPORT)
		        if(rtk_p2p_is_enabled(priv)==CFG80211_P2P){
		            rtk_p2p_set_role(priv,priv->p2pPtr->pre_p2p_role);
		            rtk_p2p_set_state(priv,priv->p2pPtr->pre_p2p_state);

		            NDEBUG2("role[%d]\n",rtk_p2p_get_role(priv));
		            NDEBUG2("state[%d]\n",rtk_p2p_get_state(priv));
		        }
#endif
				if (priv->scan_req) {
					#ifdef CFG80211_FOR_BACKPORT
					struct cfg80211_scan_info info = {
						.aborted = false,
					};
					cfg80211_scan_done((struct cfg80211_scan_request *)priv->scan_req, &info);
					#else
					SMP_UNLOCK_CFG80211(flags);
					cfg80211_scan_done(priv->scan_req, false);
					SMP_LOCK_CFG80211(flags);
					#endif
					priv->scan_req = NULL;
				}
			}
			break;
		case CFG80211_DEL_STA:
			NDEBUG("cfg80211_event [CFG80211_DEL_STA][%d]\n", event);
			cfg80211_del_sta(priv->dev, mac, GFP_ATOMIC);
			break;
		case CFG80211_RADAR_CAC_FINISHED:
			if (priv->pshare->dfs_chan_def && priv->wdev.cac_started) {
				NDEBUG("cfg80211_event [CFG80211_RADAR_CAC_FINISHED][%d]\n", event);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
				cfg80211_cac_event(priv->dev, (const struct cfg80211_chan_def *)priv->pshare->dfs_chan_def, NL80211_RADAR_CAC_FINISHED, GFP_ATOMIC);
#else
				cfg80211_cac_event(priv->dev, NL80211_RADAR_CAC_FINISHED, GFP_ATOMIC);
#endif
			}
			break;
		case CFG80211_RADAR_DETECTED:
			if (priv->pshare->dfs_chan_def) {
				NDEBUG("cfg80211_event [CFG80211_RADAR_DETECTED][%d]\n", event);
				cfg80211_radar_event(wiphy, (struct cfg80211_chan_def *)priv->pshare->dfs_chan_def, GFP_ATOMIC);
				if (priv->wdev.cac_started) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
					cfg80211_cac_event(priv->dev, (const struct cfg80211_chan_def *)priv->pshare->dfs_chan_def, NL80211_RADAR_CAC_ABORTED, GFP_ATOMIC);
#else
					cfg80211_cac_event(priv->dev, NL80211_RADAR_CAC_ABORTED, GFP_ATOMIC);
#endif
				}
			}
			priv->pmib->dot11DFSEntry.DFS_detected = 0;
			break;
		case CFG80211_RADAR_CAC_ABORTED:
			if (priv->pshare->dfs_chan_def && priv->wdev.cac_started) {
				NDEBUG("cfg80211_event [CFG80211_RADAR_CAC_ABORTED][%d]\n", event);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
				cfg80211_cac_event(priv->dev, (const struct cfg80211_chan_def *)priv->pshare->dfs_chan_def, NL80211_RADAR_CAC_ABORTED, GFP_ATOMIC);
#else
				cfg80211_cac_event(priv->dev, NL80211_RADAR_CAC_ABORTED, GFP_ATOMIC);
#endif
			}
			break;
		default:
			NDEBUG("cfg80211_event [Unknown Event !!][%d]\n", event);
    }

	SMP_UNLOCK_CFG80211(flags);

	return -1;
}


#endif

#if 0
void realtek_ap_calibration(struct rtl8192cd_priv	*priv)
{
	NLENTER;

#if 0
	unsigned char CCK_A[3] = {0x2a,0x2a,0x28};
	unsigned char CCK_B[3] = {0x2a,0x2a,0x28};
	unsigned char HT40_A[3] = {0x2b,0x2b,0x29};
	unsigned char HT40_B[3] = {0x2b,0x2b,0x29};
	unsigned char DIFF_HT40_2S[3] = {0x0,0x0,0x0};
	unsigned char DIFF_20[3] = {0x02,0x02,0x02};
	unsigned char DIFF_OFDM[3] = {0x04,0x04,0x04};
	unsigned int thermal = 0x19;
	unsigned int crystal = 32;
#else
	unsigned char CCK_A[3] = {0x2b,0x2a,0x29};
	unsigned char CCK_B[3] = {0x2b,0x2a,0x29};
	unsigned char HT40_A[3] = {0x2c,0x2b,0x2a};
	unsigned char HT40_B[3] = {0x2c,0x2b,0x2a};
	unsigned char DIFF_HT40_2S[3] = {0x0,0x0,0x0};
	unsigned char DIFF_20[3] = {0x02,0x02,0x02};
	unsigned char DIFF_OFDM[3] = {0x04,0x04,0x04};
	unsigned int thermal = 0x16;
	unsigned int crystal = 32;
#endif

	int tmp = 0;
	int tmp2 = 0;

	for(tmp = 0; tmp <=13; tmp ++)
	{
		if(tmp < 3)
			tmp2 = 0;
		else if(tmp < 9)
			tmp2 = 1;
		else
			tmp2 = 2;

		priv->pmib->dot11RFEntry.pwrlevelCCK_A[tmp] = CCK_A[tmp2];
		priv->pmib->dot11RFEntry.pwrlevelCCK_B[tmp] = CCK_B[tmp2];
		priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[tmp] = HT40_A[tmp2];
		priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[tmp] = HT40_B[tmp2];
		priv->pmib->dot11RFEntry.pwrdiffHT40_2S[tmp] = DIFF_HT40_2S[tmp2];
		priv->pmib->dot11RFEntry.pwrdiffHT20[tmp] = DIFF_20[tmp2];
		priv->pmib->dot11RFEntry.pwrdiffOFDM[tmp] = DIFF_OFDM[tmp2];
	}

	priv->pmib->dot11RFEntry.ther = thermal;
	priv->pmib->dot11RFEntry.xcap = crystal;

	NLEXIT;
}
#endif

/*
//mark_swc
static void rtk_set_phy_channel(struct rtl8192cd_priv *priv,unsigned int channel,unsigned int bandwidth,unsigned int chan_offset)
{
    NDEBUG2("ch[%d]bw[%d]offset[%d]\n",channel,bandwidth,chan_offset);
	//priv , share  part
	priv->pshare->CurrentChannelBW = bandwidth;
	priv->pshare->offset_2nd_chan =chan_offset ;

	// wifi chanel  hw settting  API
	SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	SwChnl(priv, channel, priv->pshare->offset_2nd_chan);
	//printk("rtk_set_phy_channel end !!!\n  chan=%d \n",channel );

}
*/
static void rtk_get_band_capa(struct rtl8192cd_priv *priv,BOOLEAN *band_2gig ,BOOLEAN *band_5gig)
{
	//default register as 2.4GHz
	*band_2gig = true;
	*band_5gig = false;

	if ((GET_CHIP_VER(priv) == VERSION_8812E) || (GET_CHIP_VER(priv) == VERSION_8814A) || (GET_CHIP_VER(priv) == VERSION_8822B))
	{
#if defined(CONFIG_WLAN_HAL_8814AE)
		if(priv->pshare->is5g)
#endif
		{
			*band_2gig = false;
			*band_5gig = true;
		}
	}
	else if (GET_CHIP_VER(priv) == VERSION_8192D)
	{
		*band_2gig = false;
		*band_5gig = true;
	}
	else if (GET_CHIP_VER(priv) == VERSION_8881A)
	{
#if defined(CONFIG_RTL_8881A_SELECTIVE)
		//8881A selective mode
		*band_2gig = true;
		*band_5gig = true;
#else
		//use pcie slot 0 for 2.4G 88E/92E, 8881A is 5G now
		*band_2gig = false;
		*band_5gig = true;
#endif
	}
	//mark_sel
	//if 881a , then it is possible to  *band_2gig = true ,*band_5gig = true in selective mode(FLAG?)
	//FIXME
}

void realtek_ap_default_config(struct rtl8192cd_priv *priv)
{
#if 0//!defined(RTK_129X_PLATFORM) && !defined(CONFIG_RTL8672)
	//short GI default
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = 1;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = 1;
    priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = 1;
	//APMDU
	priv->pmib->dot11nConfigEntry.dot11nAMPDU = 1;
#endif

#ifdef MBSSID
	if(IS_ROOT_INTERFACE(priv))
	{
		priv->pmib->miscEntry.vap_enable = 1; //eric-vap //eric-brsc
	}
	else
	{
#if 0
		if(IS_VAP_INTERFACE(priv))
		{
			struct rtl8192cd_priv *priv_root = GET_ROOT(priv);
			struct rtl8192cd_priv *priv_vxd = GET_VXD_PRIV(priv_root);
			unsigned char is_vxd_running = 0;

			if(priv_vxd)
				is_vxd_running = netif_running(priv_vxd->dev);

			if(priv_root->pmib->miscEntry.vap_enable == 0)
			{
				priv_root->pmib->miscEntry.vap_enable = 1;

				if(is_vxd_running)
				rtl8192cd_close(priv_vxd->dev);

				rtl8192cd_close(priv_root->dev);
				rtl8192cd_open(priv_root->dev);

				if(is_vxd_running)
				rtl8192cd_open(priv_vxd->dev);

			}
		}
#endif
		//vif copy settings from root
		//priv->pmib->dot11BssType.net_work_type = GET_ROOT(priv)->pmib->dot11BssType.net_work_type;
		priv->pmib->dot11RFEntry.phyBandSelect = GET_ROOT(priv)->pmib->dot11RFEntry.phyBandSelect;
		priv->pmib->dot11RFEntry.dot11channel = GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel;
		priv->pmib->dot11nConfigEntry.dot11nUse40M = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nUse40M;
		priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11n2ndChOffset;

		priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11FragmentationThreshold;
		priv->pmib->dot11OperationEntry.dot11RTSThreshold = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11RTSThreshold;
		priv->pmib->dot11OperationEntry.dot11ShortRetryLimit = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11ShortRetryLimit;
		priv->pmib->dot11OperationEntry.dot11LongRetryLimit = GET_ROOT(priv)->pmib->dot11OperationEntry.dot11LongRetryLimit;
	}
#endif

}

//mark_priv
#define RTK_PRIV_BW_5M 1
#define RTK_PRIV_BW_10M 2
#define RTK_PRIV_BW_80M_MINUS 3
#define RTK_PRIV_BW_80M_PLUS 4

static inline int is_hw_vht_support(struct rtl8192cd_priv *priv)
{
	int support=0;

	if (GET_CHIP_VER(priv) == VERSION_8812E)
		support=1;
	else if (GET_CHIP_VER(priv) == VERSION_8881A)
		support=1;

	return support;
}
//priv low bandwidth
static inline int is_hw_lbw_support(struct rtl8192cd_priv *priv)
{
	int support=0;

	if (GET_CHIP_VER(priv) == VERSION_8812E)
		support=1;
#if defined(CONFIG_WLAN_HAL_8192EE)
	if ((GET_CHIP_VER(priv) == VERSION_8192E) && (_GET_HAL_DATA(priv)->cutVersion == ODM_CUT_C))
		support=1;
#endif
	if(!support)
		printk("This IC NOT support 5M10M !! \n");

	return support;
}

static inline int convert_privBW(char *str_bw) //mark_priv
{
	int priv_bw=0;

    if(!strcmp(str_bw,"5M"))
		priv_bw = RTK_PRIV_BW_5M;
    else if(!strcmp(str_bw,"10M"))
		priv_bw = RTK_PRIV_BW_10M;
    //future 160M

    return priv_bw;
}

int check_5M10M_config(struct rtl8192cd_priv *priv)
{

	int priv_bw=0;
	int ret = 0;

	priv_bw = convert_privBW(priv->pshare->rf_ft_var.rtk_uci_PrivBandwidth);
	//printk("rtk_set_channel_mode , priv_band= %s , val=%d \n", priv->pshare->rf_ft_var.rtk_uci_PrivBandwidth, priv_bw);

	//first check if priv_band is set
	if(priv_bw)
	{
		//check 5/10M
		if( (priv_bw == RTK_PRIV_BW_10M) && is_hw_lbw_support(priv))
		{
			priv->pmib->dot11nConfigEntry.dot11nUse40M = CHANNEL_WIDTH_10;
			priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_DONTCARE;
			ret = 1;
			NDEBUG("Force config bandwidth=10M\n");
		}
		else if( (priv_bw == RTK_PRIV_BW_5M) && is_hw_lbw_support(priv))
		{
			priv->pmib->dot11nConfigEntry.dot11nUse40M = CHANNEL_WIDTH_5;
			priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_DONTCARE;
			ret = 1;
			NDEBUG("Force config bandwidth=5M\n");
		}
		else
			NDEBUG("No such priv channel type !!!\n");

	}

	return ret;

}

int is_support_ac(struct rtl8192cd_priv *priv)
{
	int ret=0;

	switch(GET_CHIP_VER(priv)) {
		case VERSION_8812E:
		case VERSION_8881A:
		case VERSION_8822B:
			ret=1;
			break;
#if defined(CONFIG_WLAN_HAL_8814AE)
		case VERSION_8814A:
			if(priv->pshare->is5g)
				ret=1;
#endif
	}

	return ret;
}
#if defined(DFS)
static void rtk_set_band_mode(struct rtl8192cd_priv *priv,enum ieee80211_band band ,enum nl80211_chan_width channel_width)
{
    NLENTER;

	if(band == IEEE80211_BAND_2GHZ)
	{
		priv->pmib->dot11BssType.net_work_type = WIRELESS_11B|WIRELESS_11G;
		priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_2G;
	}
	else if(band == IEEE80211_BAND_5GHZ)
	{
		priv->pmib->dot11BssType.net_work_type = WIRELESS_11A;
		priv->pmib->dot11RFEntry.phyBandSelect = PHY_BAND_5G;
	}

	if(channel_width != NL80211_CHAN_WIDTH_20_NOHT) {
		priv->pmib->dot11BssType.net_work_type |= WIRELESS_11N;
	} else {
		if((OPMODE & WIFI_AP_STATE) && !under_apmode_repeater(priv))
			priv->rtk->keep_legacy = 1;
	}

	if((channel_width == NL80211_CHAN_WIDTH_80 || is_support_ac(priv)) && channel_width != NL80211_CHAN_WIDTH_20_NOHT) {
		priv->pmib->dot11BssType.net_work_type |= WIRELESS_11AC;
	}

#ifdef UNIVERSAL_REPEATER
	if(IS_ROOT_INTERFACE(priv) && priv->pvxd_priv)
	{
		priv->pvxd_priv->pmib->dot11BssType.net_work_type = priv->pmib->dot11BssType.net_work_type;
		priv->pvxd_priv->pmib->dot11RFEntry.phyBandSelect = priv->pmib->dot11RFEntry.phyBandSelect;
	}
#endif
}
#endif
static void rtk_set_channel_mode(struct rtl8192cd_priv *priv, struct cfg80211_chan_def *chandef)
{
	int config_BW5m10m=0;

	config_BW5m10m = check_5M10M_config(priv);

	//printk("[%s]rtk_set_channel_mode , priv_band= %s , val=%d \n", priv->dev->name, priv->pshare->rf_ft_var.rtk_uci_PrivBandwidth);

	//first check if priv_band is set
	if(!config_BW5m10m)
	{
		//normal channel setup path from cfg80211
		if(chandef->width == NL80211_CHAN_WIDTH_40) {
			//printk("NL80211_CHAN_WIDTH_40\n");
			priv->pshare->is_40m_bw = CHANNEL_WIDTH_40;
			priv->pshare->CurrentChannelBW = CHANNEL_WIDTH_40;
			priv->pmib->dot11nConfigEntry.dot11nUse40M = CHANNEL_WIDTH_40;
			if (chandef->center_freq1 > chandef->chan->center_freq) {
				//printk("NL80211_CHAN_WIDTH_40-PLUS\n");
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE; //above
			} else {
				//printk("NL80211_CHAN_WIDTH_40-MINUS\n");
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW; //below
			}
		} else if(chandef->width == NL80211_CHAN_WIDTH_80) {
			//printk("NL80211_CHAN_WIDTH_80\n");
			priv->pshare->is_40m_bw = CHANNEL_WIDTH_80;
			priv->pshare->CurrentChannelBW = CHANNEL_WIDTH_80;
			priv->pmib->dot11nConfigEntry.dot11nUse40M = CHANNEL_WIDTH_80;

			if (chandef->center_freq1 > chandef->chan->center_freq) {
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_ABOVE; //dontcare
			} else {
				priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_BELOW; //dontcare
			}
		} else {
#if 0
			if(chandef->width == NL80211_CHAN_WIDTH_20 || chandef->width == NL80211_CHAN_WIDTH_20_NOHT)
				printk("NL80211_CHAN_WIDTH_20\/NL80211_CHAN_WIDTH_20_NOHT\n");
            else
                printk("Unknown bandwidth: %d, use 20Mhz be default\n", chandef->width);
#endif
			priv->pshare->is_40m_bw = CHANNEL_WIDTH_20;
			priv->pshare->CurrentChannelBW = CHANNEL_WIDTH_20;
			priv->pmib->dot11nConfigEntry.dot11nUse40M = CHANNEL_WIDTH_20;
			priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = HT_2NDCH_OFFSET_DONTCARE;
		}

#ifdef UNIVERSAL_REPEATER
		if(IS_ROOT_INTERFACE(priv) && priv->pvxd_priv)
		{
			priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nUse40M = priv->pmib->dot11nConfigEntry.dot11nUse40M;
			priv->pvxd_priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
		}
#endif

	}
}

void realtek_ap_config_apply(struct rtl8192cd_priv	*priv)
{
    #if	0	//def P2P_SUPPORT
    int keep_go_state=0;

    if(priv->pmib->p2p_mib.p2p_enabled==CFG80211_P2P){
        keep_go_state=1;
    }
    #endif

	NLENTER;

	if(under_apmode_repeater(priv) && (GET_VXD_PRIV(priv)->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE)) {
		NLINFO("Repeater! STA is alive, skip down-up\n");
	} else {

		closeopenlog("[%s][down up]\n",priv->dev->name);
#ifndef RTK_129X_PLATFORM
		priv->dev->flags &= ~IFF_UP;
#endif
		rtl8192cd_close(priv->dev);
		priv->dev->flags |= IFF_UP;
		rtl8192cd_open(priv->dev);
	}

}

int realtek_cfg80211_ready(struct rtl8192cd_priv	*priv)
{

	if (netif_running(priv->dev) && (priv->drv_state & DRV_STATE_OPEN))
		return 1;
	else
		return 0;
}

void realtek_reset_security(struct rtl8192cd_priv *priv)
{
	NLENTER;
	priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
	priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPACipher = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
	priv->pmib->dot11GroupKeysTable.dot11Privacy = 0;
	priv->pmib->dot11RsnIE.rsnielen = 0;	// reset RSN IE length
#ifdef CONFIG_IEEE80211W
	priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = 0;
	priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 0;
#endif
}

void realtek_auth_wep(struct rtl8192cd_priv *priv, int cipher)
{
	//_eric_nl ?? wep auto/shared/open ??
	NLENTER;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = cipher;
	priv->pmib->dot1180211AuthEntry.dot11EnablePSK = 0;
	priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPACipher = 0;
	priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = 0;
	NLEXIT;
}
void realtek_auth_wpa(struct rtl8192cd_priv *priv, int wpa, int psk, int cipher)
{
	int wpa_cipher = 0;

	if (cipher & BIT(_TKIP_PRIVACY_))
		wpa_cipher |= BIT(1);
	if (cipher & BIT(_CCMP_PRIVACY_))
		wpa_cipher |= BIT(3);

	NLENTER;
	NDEBUG2("%s wpa[%d] psk[%d] cipher[0x%x] wpa_cipher[0x%x]\n", priv->dev->name, wpa, psk, cipher, wpa_cipher);
	priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
	priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;

	if (psk)
		priv->pmib->dot1180211AuthEntry.dot11EnablePSK |= wpa;

	priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = 1;

	if (wpa & PSK_WPA)
		priv->pmib->dot1180211AuthEntry.dot11WPACipher = wpa_cipher;
	if ((wpa & PSK_WPA2) || (wpa & PSK_WPA3))
		priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = wpa_cipher;

	if (cipher & BIT(_TKIP_PRIVACY_))
		priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_TKIP;
	else
		priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_CCMP; //wpa2/wpa3

	NLEXIT;
}


/*
target_bss_idx add for STA mode to choose the index at
AP mode don't care it.
*/
void realtek_get_security(
	struct rtl8192cd_priv *priv,
	struct rtknl *rtk, struct cfg80211_crypto_settings crypto
	, int target_bss_idx)
{
	int i = 0;
	rtk->cipher = rtk->wpa = rtk->psk = rtk->sha256 = 0;

	NDEBUG2("AKM cnt[%d], uni_cipher_cnt=[%d]\n", crypto.n_akm_suites, crypto.n_ciphers_pairwise);

	/*parsing AKM(Auth Key Managment) suite */
	for (i = 0; i < crypto.n_akm_suites; i++) {
		switch (crypto.akm_suites[i]) {
		case WLAN_AKM_SUITE_8021X:
			NDEBUG2("WLAN_AKM_SUITE_8021X\n");
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				rtk->wpa |= PSK_WPA;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				rtk->wpa |= PSK_WPA2;
			break;
		case WLAN_AKM_SUITE_PSK:
			NDEBUG2("WLAN_AKM_SUITE_PSK\n");
			rtk->psk = 1;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				rtk->wpa |= PSK_WPA;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				rtk->wpa |= PSK_WPA2;
			break;
			/*HAPD 11R */
		case WLAN_AKM_SUITE_FT_PSK:
			NDEBUG2("WLAN_AKM_SUITE_FT_PSK\n");
			rtk->psk = 1;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				rtk->wpa |= PSK_WPA;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				rtk->wpa |= PSK_WPA2;
			break;
		case WLAN_AKM_SUITE_FT_8021X:
			NDEBUG2("WLAN_AKM_SUITE_FT_8021X\n");
			rtk->psk = 0;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				rtk->wpa |= PSK_WPA;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				rtk->wpa |= PSK_WPA2;
			break;
			/*HAPD 11R */
		case WLAN_AKM_SUITE_PSK_SHA256:
			NDEBUG2("WLAN_AKM_SUITE_PSK_SHA256\n");
			rtk->psk = 1;
			rtk->sha256 = 1;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				rtk->wpa |= PSK_WPA;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				rtk->wpa |= PSK_WPA2;
			break;
			/* 20180202 */
		case WLAN_AKM_SUITE_8021X_SHA256:
			NDEBUG2("WLAN_AKM_SUITE_8021X_SHA256\n");
			rtk->psk = 0;
			rtk->sha256 = 1;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_1)
				rtk->wpa |= PSK_WPA;
			if (crypto.wpa_versions & NL80211_WPA_VERSION_2)
				rtk->wpa |= PSK_WPA2;
			break;
		case WLAN_AKM_SUITE_SAE:
				if(OPMODE & WIFI_STATION_STATE){
			NDEBUG2("WLAN_AKM_SUITE_SAE,i[%d]\n",i);
			rtk->psk = 1;
			rtk->sha256 = 1;
			rtk->wpa |= PSK_WPA3;

			if(target_bss_idx>=0 && target_bss_idx<MAX_BSS_NUM){
				priv->site_survey->bss_target[target_bss_idx].t_stamp[1] |= BIT(11);
				wpaslog("peer[%pm]",
				priv->site_survey->bss_target[target_bss_idx].bssid);
				wpaslog("set t_stamp_1 to [%04X]",
				priv->site_survey->bss_target[target_bss_idx].t_stamp[1]);
			}
				}
			break;
		default:
			NDEBUG2("[akm_suites]0x[%04X] not support!!\n", crypto.akm_suites[i]);
			break;
		}
	}

	//_eric_nl ?? multiple ciphers ??
	for (i = 0; i < crypto.n_ciphers_pairwise; i++) {
		switch (crypto.ciphers_pairwise[i]) {
		case WLAN_CIPHER_SUITE_WEP40:
			rtk->cipher = BIT(_WEP_40_PRIVACY_);
			NDEBUG2("WEP40[idx=%d]\n", i);
			break;
		case WLAN_CIPHER_SUITE_WEP104:
			rtk->cipher = BIT(_WEP_104_PRIVACY_);
			NDEBUG2("WEP104[idx=%d]\n", i);
			break;
		case WLAN_CIPHER_SUITE_TKIP:
			rtk->cipher |= BIT(_TKIP_PRIVACY_);
			NDEBUG2("TKIP[idx=%d]\n", i);
			break;
		case WLAN_CIPHER_SUITE_CCMP:
			rtk->cipher |= BIT(_CCMP_PRIVACY_);
            NDEBUG2("CCMP[idx=%d]\n", i);
			break;
		default:
			NDEBUG2("[ciphers_pairwise]=0x%08x not support\n", crypto.ciphers_pairwise[i]);
			break;
		}
	}

	switch (crypto.cipher_group) {
	case WLAN_CIPHER_SUITE_WEP40:
		rtk->cipher = BIT(_WEP_40_PRIVACY_);
		NDEBUG2("WEP GROUP\n");
		break;
	case WLAN_CIPHER_SUITE_WEP104:
		rtk->cipher = BIT(_WEP_104_PRIVACY_);
		NDEBUG2("WEP GROUP\n");
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		NDEBUG2("TKIP GROUP\n");
		break;
	case WLAN_CIPHER_SUITE_CCMP:
    	NDEBUG2("CCMP GROUP\n");
		break;
	case WLAN_CIPHER_SUITE_SMS4:
		NDEBUG2("WAPI GROUP\n");
		break;
	default:
		NDEBUG2("NONE GROUP\n");
		break;
	}
}

void realtek_set_security(struct rtl8192cd_priv *priv, struct rtknl *rtk)
{
	if (rtk->wpa)
		realtek_auth_wpa(priv, rtk->wpa, rtk->psk, rtk->cipher);
	else {
		if (rtk->cipher & BIT(_WEP_40_PRIVACY_))
			realtek_auth_wep(priv, _WEP_40_PRIVACY_);
		else if (rtk->cipher & BIT(_WEP_104_PRIVACY_))
			realtek_auth_wep(priv, _WEP_104_PRIVACY_);
	}
}

void realtek_set_security_ap(struct rtl8192cd_priv *priv, struct rtknl *rtk, struct cfg80211_crypto_settings crypto)
{
    realtek_get_security(priv,rtk, crypto,0);

#ifdef CONFIG_IEEE80211W
	/*
	 *  hostapd don't set 11w in akm_suites, workaround here
	 *  check dot11IEEE80211W @realtek_set_band
	 */
    if ((priv->pmib->dot1180211AuthEntry.dot11IEEE80211W != NO_MGMT_FRAME_PROTECTION) &&
		!(priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA3)) {
		rtk->psk = 1;
		rtk->wpa = PSK_WPA2;
	} else if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK & PSK_WPA3) {
		rtk->wpa |= PSK_WPA3;
		rtk->psk = 1;
	}
#endif				/* CONFIG_IEEE80211W */
	realtek_set_security(priv, rtk);

}

void realtek_set_security_cli(
	struct rtl8192cd_priv *priv, struct rtknl *rtk,
	struct cfg80211_connect_params *sme , int target_bss_idx)
{

	realtek_get_security(priv,rtk, sme->crypto , target_bss_idx);

#ifdef CONFIG_IEEE80211W_CLI
	if (sme->mfp == NL80211_MFP_REQUIRED) {
		wpaslog("NL80211_MFP_REQUIRED");
		if (rtk->sha256 == true)
			priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 1;
		else
			priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 0;

		priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = MGMT_FRAME_PROTECTION_OPTIONAL;
	}else if(sme->mfp == NL80211_MFP_OPTIONAL){
		wpaslog("NL80211_MFP_OPTIONAL");
	}else{
		wpaslog("NL80211_MFP_NO");
	}
#endif				/* CONFIG_IEEE80211W_CLI */

	realtek_set_security(priv, rtk);

}

void realtek_get_key_from_sta(struct rtl8192cd_priv *priv, struct stat_info *pstat, struct key_params *params, u8 key_index)
{
	unsigned int cipher = 0;
	struct Dot11EncryptKey *pEncryptKey;

	//_eric_cfg ?? key len data seq for get_key ??
	if(pstat == NULL)
	{
		if (key_index == priv->default_mgmt_key_idx)
		{
			//NDEBUG("get default mgnt key\n");
			cipher = priv->pmib->dot11IGTKTable.dot11Privacy;
			pEncryptKey = &priv->pmib->dot11IGTKTable.dot11EncryptKey;
		}
		else
		{
			cipher = priv->pmib->dot11GroupKeysTable.dot11Privacy;
			pEncryptKey = &priv->pmib->dot11GroupKeysTable.dot11EncryptKey;
		}
	}
	else
	{
		cipher = pstat->dot11KeyMapping.dot11Privacy;
		pEncryptKey = &pstat->dot11KeyMapping.dot11EncryptKey;
	}

	switch (cipher) {
	case DOT11_ENC_WEP40:
		params->cipher = WLAN_CIPHER_SUITE_WEP40;
		params->key_len = 5;
		memcpy((void *)params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
		break;
	case DOT11_ENC_WEP104:
		params->cipher = WLAN_CIPHER_SUITE_WEP104;
		params->key_len = 10;
		memcpy((void *)params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
		break;
	case DOT11_ENC_CCMP:
		params->cipher = WLAN_CIPHER_SUITE_CCMP;/*eric refine*/
		params->key_len = 32;
		memcpy((void *)params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
		memcpy((void *)(params->key+16), pEncryptKey->dot11TMicKey1.skey, pEncryptKey->dot11TMicKeyLen);
		break;
	case DOT11_ENC_TKIP:
		params->cipher = WLAN_CIPHER_SUITE_TKIP;/*eric refine*/
		params->key_len = 32;
		memcpy((void *)params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
		memcpy((void *)(params->key+16), pEncryptKey->dot11TMicKey1.skey, pEncryptKey->dot11TMicKeyLen);
		memcpy((void *)(params->key+24), pEncryptKey->dot11TMicKey2.skey, pEncryptKey->dot11TMicKeyLen);
		break;
	case DOT11_ENC_BIP:
		params->cipher = WLAN_CIPHER_SUITE_AES_CMAC;
		params->key_len = 32;
		memcpy((void *)params->key, pEncryptKey->dot11TTKey.skey, pEncryptKey->dot11TTKeyLen);
		memcpy((void *)(params->key+16), pEncryptKey->dot11TMicKey1.skey, pEncryptKey->dot11TMicKeyLen);
		break;
	default:
		NDEBUG("cipher(%d) not support!!\n", cipher);
	}
}

void clear_wps_ies(struct rtl8192cd_priv *priv)
{
	priv->pmib->wscEntry.wsc_enable = 0;

	priv->pmib->wscEntry.beacon_ielen = 0;
	priv->pmib->wscEntry.probe_rsp_ielen = 0;
	priv->pmib->wscEntry.probe_req_ielen = 0;
	priv->pmib->wscEntry.assoc_ielen = 0;
}


/*void copy_wps_ie(struct rtl8192cd_priv *priv, unsigned char *wps_ie, unsigned char mgmt_type) move to 8192cd_util.c*/
/*the function can be replaced by rtk_cfg80211_set_wps_p2p_ie*/
//void rtk_set_ie(struct rtl8192cd_priv *priv, unsigned char *pies, unsigned int ies_len, unsigned char mgmt_type)


//static int rtw_cfg80211_set_probe_req_wpsp2pie(struct rtl8192cd_priv *priv, char *buf, int len)
int rtk_cfg80211_set_wps_p2p_ie(struct rtl8192cd_priv *priv, const char *buf, int len, int mgmt_type)
{
	int wps_ielen = 0;
	u8 *wps_ie;
	//u32   wfd_ielen = 0;
	//u8 *wfd_ie;
#if defined(P2P_SUPPORT)
	u32 p2p_ielen = 0;
	u8 *p2p_ie;
	u8 *p2p_ie_listen_tag_ptr = NULL;
	int p2p_ie_listen_tag;
#endif

	if (len <= 0)
		return -1;

	//NDEBUG2("mgmt_type=[%d]\n", mgmt_type);

	/*set WPS IE */
	if ((wps_ie = rtk_get_wps_ie((u8 *) buf, len, NULL, &wps_ielen))) {
		copy_wps_ie(priv, wps_ie, mgmt_type);
	}
#if defined(P2P_SUPPORT)
	/*set P2P IE */
	if ((p2p_ie = rtk_get_p2p_ie(buf, len, NULL, &p2p_ielen))) {
		copy_p2p_ie(priv, p2p_ie, mgmt_type);

		if (mgmt_type == MGMT_PROBEREQ) {
			/*check if  listen channel from cfg80211 equl my keep */
			p2p_ie_listen_tag_ptr = p2p_search_tag(p2p_ie, p2p_ielen, TAG_LISTEN_CHANNEL, &p2p_ie_listen_tag);
			if (p2p_ie_listen_tag_ptr && p2p_ie_listen_tag == 5) {
				if (priv->pmib->p2p_mib.p2p_listen_channel != p2p_ie_listen_tag_ptr[4]) {
					NDEBUG("listen ch no equl\n");
					priv->pmib->p2p_mib.p2p_listen_channel = p2p_ie_listen_tag_ptr[4];
				}
			}
		}

	}
#endif
	/*set WFD IE to do */
	return len;
}


/*BC FANG*/
#define HS20_INDICATION_OUI_TYPE 16
#define HS20_PPS_MO_ID_PRESENT 0x02
//#define HS20_ANQP_DOMAIN_ID_PRESENT 0x04
#define HS20_ANQP_DOMAIN_ID_PRESENT 	BIT(4)
#define HS20_DGAF_DISABLED BIT(0)
#define HS20_VERSION 0x10 /* Release 2 */

/**/
#define HS20_IE_INDICATE_MFPR BIT(6)
#define HS20_IE_INDICATE_MFPC BIT(7)

unsigned char *parsing_specific_ie(unsigned char *pbuf, int index, unsigned char oui, int *len, int limit)
{
	unsigned char *ie=NULL;

	ie = get_ie(pbuf, index, len, limit);

	if(ie) {
		if((*(ie+5) == oui) && (oui!=0) && limit > 5)
			return ie;
		else if(oui == 0)
			return ie;
		else {
			limit -= (*len+2);
			return parsing_specific_ie(ie+*len+2, index, oui, len, limit);
		}
	}
	else
		return NULL;
}


/*BC FANG add*/
extern unsigned char OSEN_OUI[];
extern unsigned char HS2_OUI[];
extern unsigned char OSEN_GROUP_CIPHER_SUITE[];
extern unsigned char WPA_CCMP_CIPHER_SUITE[];
extern unsigned char OSEN_AKM_SUITE[];
extern unsigned char PSK_SHA256_AKM_SUITE[];
extern unsigned char PSK_WPA3_AKM_SUITE[];

extern unsigned char OWE_AKM_SUITE[];

int rtk_cfg80211_syn_beacon_ies(
	struct rtl8192cd_priv *priv,
	const unsigned char *BeaconBuf,
	const  size_t BeaconLen)
{
	unsigned char *TAG_IE=NULL, *tmp_IE_ptr=NULL;
	size_t TAG_LEN=0, tmp_IE_Len=0 ;
#ifdef CONFIG_IEEE80211W
	DOT11_RSN_CAPABILITY *RSN_Cap_Ptr;
	int PMF_IEEE80211W=0;
	int SHA256_VAL=0;
#endif
	int count,i;
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite;
	DOT11_RSN_IE_HEADER *pDot11RSNIEHeader=NULL;
#ifdef HS2_SUPPORT
	DOT11_RSN_IE_COUNT_SUITE *cipher_suite_ptr;
	unsigned char *ExtCap_IE_Ptr=NULL;
	unsigned char *OSEN_IE=NULL;
	size_t ExtCap_IE_LEN=0;
	unsigned char* HS2_Content=0;
	unsigned short cipher_count=0;
	unsigned char osen_found=0;
	unsigned char hs2_found=0;
#endif

	//dump_hex("BeaconBuf", BeaconBuf, BeaconLen);
	//log("size of struct ss_res[%d][0x%x]",sizeof(struct ss_res),sizeof(struct ss_res));

#ifdef HAPD_11V
	/*parsing 11V MOBILITY_DOMAIN*/
	TAG_IE = get_ie((unsigned char *)BeaconBuf, _EXTENDED_CAP_IE_, &TAG_LEN, BeaconLen);
	if(TAG_IE) {
		//dump_hex("ext cap", TAG_IE, TAG_LEN+2);
		if(TAG_LEN!=4){
			NDEBUG2("len!=4\n");
			NDEBUG2("WNM no supported\n");
			priv->hapd_11v_enable = 0;
		}else{
			NDEBUG2("Ext Cap byte2=0x[%02X]\n",TAG_IE[4]);
			if(TAG_IE[4] & _BSS_TRANSITION_){
				NDEBUG2("WNM enabled(11V)\n");
				priv->hapd_11v_enable = 1;
			}
		}
	}
	else {
		NDEBUG2("WNM no supported\n");
		priv->hapd_11v_enable = 0;
	}
#endif
#ifdef HAPD_11K
	/*parsing 11K MOBILITY_DOMAIN ; fi found priv->hapd_11k_ie include TLV*/
	TAG_IE = get_ie((unsigned char *)BeaconBuf, WLAN_EID_RRM_ENABLED_CAPABILITIES, &TAG_LEN, BeaconLen);
	if(TAG_IE) {
		if(TAG_LEN!=5){
			NDEBUG2("[11K]IE should be 5,chk\n");
			priv->hapd_11k_ie_len = 0;
		}else{
			memcpy(priv->hapd_11k_ie,TAG_IE,TAG_LEN+2);
			priv->hapd_11k_ie_len = TAG_LEN+2;
			NDEBUG2("11K enabled\n");
		}
	}
	else {
		priv->hapd_11k_ie_len = 0;
		//NDEBUG("[11K]not found\n");
	}
#endif
#ifdef HAPD__IEEE80211R
	/*parsing 11R MOBILITY_DOMAIN*/
	TAG_IE = get_ie(BeaconBuf, WLAN_EID_MOBILITY_DOMAIN, &TAG_LEN, BeaconLen);
	if(TAG_IE) {
		FT_ENABLE = 1;
		//memcpy(MDID, TAG_IE+2, 2);/*MDID len fixed = 5*/
		struct priv_shared_info *pshare = priv->pshare;
		memcpy(pshare->hapd_FT_MDIE, TAG_IE, 5);/*MDID len fixed = 5*/
		pshare->hapd_FT_MDIE_len=5;

		if(TAG_IE[4]&BIT(0)){
			//priv->pmib->dot11FTEntry.dot11FTOverDSEnabled=1;
			NDEBUG("[11R]over DS = 1\n");
		}else{
			NDEBUG("[11R]over DS = 0\n");
		}
		if(TAG_IE[4]&BIT(1)){
			//priv->pmib->dot11FTEntry.dot11FTResourceRequestSupported=1;
			NDEBUG("[11R]FT Resource Request Supported = 1\n");
		}else{
			NDEBUG("[11R]FT Resource Request Supported = 0\n");
		}
		NDEBUG("[11R]MDID IE found\n");
	}
	else {
		NDEBUG("[11R]MDID IE not found\n");
		FT_ENABLE = 0;
		priv->pmib->dot11OperationEntry.keep_rsnie=0;
	}
#endif

#ifdef DIRECT_HAPD_RSN_IE /*directly use RSN IE & WPA IE from hostapd*/
	/*GET RNS IE*/
	TAG_IE = get_ie((unsigned char *)BeaconBuf, _RSN_IE_2_, &TAG_LEN, BeaconLen);
	if(TAG_IE) {
		memcpy(priv->pmib->dot11RsnIE.rsnie, TAG_IE, TAG_LEN+2);
		priv->pmib->dot11RsnIE.rsnielen=TAG_LEN+2;
		priv->pmib->dot11OperationEntry.keep_rsnie=1;
		NDEBUG("[RSN] len[%d]\n",priv->pmib->dot11RsnIE.rsnielen);
	}else{
		//NDEBUG("no included[RSN_IE]\n");
	}

	/*GET WPA IE*/
	tmp_IE_ptr = (unsigned char *)BeaconBuf;
	tmp_IE_Len = BeaconLen;

	for(;;){
		TAG_IE = get_ie(tmp_IE_ptr, _RSN_IE_1_, &TAG_LEN, tmp_IE_Len);
		if (!TAG_IE) break;

		tmp_IE_ptr = TAG_IE+TAG_LEN+2;
		tmp_IE_Len = BeaconLen - (size_t)(tmp_IE_ptr - BeaconBuf);

		pDot11RSNIEHeader = (DOT11_RSN_IE_HEADER *)TAG_IE;

		if (memcmp(pDot11RSNIEHeader->OUI, wpa_oui, sizeof(wpa_oui)))
			continue;
		/*copy WPA IE from hostapd*/
		NDEBUG("[WPA IE found] len[%d]\n",TAG_LEN);
		memcpy((priv->pmib->dot11RsnIE.rsnie + priv->pmib->dot11RsnIE.rsnielen), TAG_IE, TAG_LEN+2);
		priv->pmib->dot11RsnIE.rsnielen += TAG_LEN+2;
		priv->pmib->dot11OperationEntry.keep_rsnie=1;
	}
	NDEBUG("[RSN/WPA] len[%d]\n",priv->pmib->dot11RsnIE.rsnielen);
#endif

	/*parsing country code*/
	TAG_IE = get_ie((unsigned char *)BeaconBuf, _COUNTRY_IE_, &TAG_LEN, BeaconLen);
	if(TAG_IE) {
		NDEBUG2("[Country code]found\n");
		COUNTRY_CODE_ENABLED = 1;
		memcpy(priv->pmib->dot11dCountry.dot11CountryString, TAG_IE+2, 3);
		#if (defined(DOT11D) || defined(DOT11H) || defined(DOT11K))
		check_country_channel_table(priv);
		#endif
	} else {
		COUNTRY_CODE_ENABLED = 0;
	}


	/*parsing PMF/WPA3*/
	TAG_IE = get_ie((unsigned char *)BeaconBuf, _RSN_IE_2_, &TAG_LEN, BeaconLen);
	if (TAG_IE) {
		/* header */
		TAG_IE += sizeof(DOT11_WPA2_IE_HEADER);

		/* group chipher */
		TAG_IE += sizeof(DOT11_RSN_IE_SUITE);

		/* pairwise chiper */
		pDot11RSNIECountSuite = (DOT11_RSN_IE_COUNT_SUITE *)TAG_IE;
		count = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);

		TAG_IE += (2 + count * sizeof(DOT11_RSN_IE_SUITE));

		/* check AKM AuthKeyMgnt*/
		pDot11RSNIECountSuite = (DOT11_RSN_IE_COUNT_SUITE *)TAG_IE;
		count = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);

		for (i = 0 ; i < count ; ++i){
			if (!memcmp(&pDot11RSNIECountSuite->dot11RSNIESuite[i],
				PSK_WPA3_AKM_SUITE, OUI_LEN)){
				priv->pmib->dot1180211AuthEntry.dot11EnablePSK |= PSK_WPA3;
				//priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = 1;
				NDEBUG("WPA3 = 1\n");
			}
			#ifdef HAPD_OWE
			if (!memcmp(&pDot11RSNIECountSuite->dot11RSNIESuite[i],
				OWE_AKM_SUITE, OUI_LEN)){
				priv->pmib->dot1180211AuthEntry.dot11EnablePSK |= OWE_WPA3;
				owelog("OWE = 1\n");
			}
			#endif
		}
		TAG_IE += (2 + count * sizeof(DOT11_RSN_IE_SUITE));
#ifdef CONFIG_IEEE80211W
		/*RSN cap , len=2*/
		RSN_Cap_Ptr = (DOT11_RSN_CAPABILITY *)TAG_IE;

		if (RSN_Cap_Ptr->field.MFPC && RSN_Cap_Ptr->field.MFPR) {
			PMF_IEEE80211W = MGMT_FRAME_PROTECTION_REQUIRED;
			SHA256_VAL=1;
			NDEBUG("11W=2 , SHA256=1\n");
		} else if (RSN_Cap_Ptr->field.MFPC) {
			PMF_IEEE80211W = MGMT_FRAME_PROTECTION_OPTIONAL;
			SHA256_VAL=1;
			NDEBUG("11W=1, SHA256=1\n");
		} else {
			PMF_IEEE80211W = NO_MGMT_FRAME_PROTECTION;
			SHA256_VAL=0;
			NDEBUG("11W=0,  SHA256=0\n");
		}

		//priv->pmib->dot1180211AuthEntry.dot11ieee8021x = 1;
		priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = PMF_IEEE80211W;
		priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = SHA256_VAL;
#endif

	}

#ifdef HS2_SUPPORT
	/*OSEN ; OSU Server-only authenticated layer 2 Encryption Network element*/
	/*Start of parsing OSEN IE , */
	tmp_IE_ptr = (unsigned char *)BeaconBuf;
	tmp_IE_Len = BeaconLen;
	for(;;) {
		TAG_IE = get_ie(tmp_IE_ptr, _VENDOR_SPEC_IE_, &TAG_LEN, tmp_IE_Len);
		if(TAG_IE) {
			if(memcmp(TAG_IE+2,OSEN_OUI,OUI_LEN)==0) {
				osen_found=1;
				NDEBUG("[OSEN]present\n");
				memcpy(priv->pmib->hs2Entry.osen_ie,TAG_IE,TAG_LEN+2);
				priv->pmib->hs2Entry.osen_ie_len=TAG_LEN+2;

				OSEN_IE	= TAG_IE + 	2 + sizeof(DOT11_RSN_IE_SUITE);/*T,L And OSEN OUI */
				/*
				if(memcmp(OSEN_IE , OSEN_GROUP_CIPHER_SUITE , OUI_LEN)==0) {
					NDEBUG("[OSEN] OSEN_GROUP_CIPHER_SUITE found\n");
				}
				*/

				/*pairwise cipher suite*/
				OSEN_IE += sizeof(DOT11_RSN_IE_SUITE);
				cipher_suite_ptr=(struct _DOT11_RSN_IE_COUNT_SUITE *)OSEN_IE;
				cipher_count = le16_to_cpu(cipher_suite_ptr->SuiteCount);
				/*
				//NDEBUG("[OSEN] Pairwise cipher count[%d]\n",cipher_count);
				if(memcmp(cipher_suite_ptr->dot11RSNIESuite,WPA_CCMP_CIPHER_SUITE,OUI_LEN)==0) {
					NDEBUG("[OSEN] Pairwise cipher [%02X,%02X,%02X,%02X]\n",
						   cipher_suite_ptr->dot11RSNIESuite[0].OUI[0],cipher_suite_ptr->dot11RSNIESuite[0].OUI[1],
						   cipher_suite_ptr->dot11RSNIESuite[0].OUI[2],cipher_suite_ptr->dot11RSNIESuite[0].Type);
				}
				*/

				/*AKM suite*/
				OSEN_IE += ( 2  + sizeof(DOT11_RSN_IE_SUITE)*cipher_count);
				cipher_suite_ptr=(struct _DOT11_RSN_IE_COUNT_SUITE *)OSEN_IE;
				cipher_count = le16_to_cpu(cipher_suite_ptr->SuiteCount);
				/*
				NDEBUG("[OSEN] AKM suite count[%d]\n",cipher_count);
				if(memcmp(cipher_suite_ptr->dot11RSNIESuite,OSEN_AKM_SUITE,OUI_LEN)==0) {
					NDEBUG("[OSEN] AKM suite [%02X,%02X,%02X,%02X]\n",
						   cipher_suite_ptr->dot11RSNIESuite[0].OUI[0],cipher_suite_ptr->dot11RSNIESuite[0].OUI[1],
						   cipher_suite_ptr->dot11RSNIESuite[0].OUI[2],cipher_suite_ptr->dot11RSNIESuite[0].Type);
				}
				*/

			}
			else {
				/* no break; search untill last OSEN IE , hostapd will bring mulit OSEN, use last one*/
			}
			tmp_IE_ptr = TAG_IE+TAG_LEN+2;
			tmp_IE_Len = BeaconLen - (size_t)(tmp_IE_ptr - BeaconBuf);
			/*search on going*/
		}
		else{
			break;
		}
	}
	if(osen_found==0){
		priv->pmib->hs2Entry.osen_ie_len=0;
	}
	/*end of parsing OSEN IE*/


	/*parsing  HS2 IE, 20180208*/
	tmp_IE_ptr = (unsigned char *)BeaconBuf;
	tmp_IE_Len = BeaconLen;
	for(;;) {	/*threr are multi 221ID*/
		TAG_IE = get_ie(tmp_IE_ptr, _VENDOR_SPEC_IE_, &TAG_LEN, tmp_IE_Len);
		if(TAG_IE) {
			if(memcmp(TAG_IE+2,HS2_OUI,OUI_LEN)==0) {
				hs2_found=1;
				NDEBUG("[HS2] IE found,LEN[%u]\n",TAG_LEN);
				HS2_Content=TAG_IE+6;

				/*HS2 IE & LEN*/
				memcpy(priv->pmib->hs2Entry.hs2_ie, TAG_IE+2, TAG_LEN);
				priv->pmib->hs2Entry.hs2_ielen = TAG_LEN;
				priv->pmib->hs2Entry.hs_enable=1;/* 20180206*/

				/*proxy arp handled in wifi drv,hostapd wont config this parameter,when HS2 enabled auto let it enabled*/
				priv->proxy_arp=1;

				NDEBUG("[HS2]cap[%02X]\n",HS2_Content[0]);

				if(HS2_Content[0] & HS20_ANQP_DOMAIN_ID_PRESENT) {
					NDEBUG("[ANQP Domain]present\n");
				} else {
					NDEBUG("[ANQP Domain]not present\n");
				}
				priv->dgaf_disable = (HS2_Content[0] & HS20_DGAF_DISABLED);
				NDEBUG("[dgaf disable] = [%s]\n",(priv->dgaf_disable)? "0":"1");
				NDEBUG("[HS2 ver] [%02X]\n",(HS2_Content[0]&0xf0)>>4);

				break;/*Found break for loop*/
			} else {
				tmp_IE_ptr = TAG_IE+TAG_LEN+2;
				tmp_IE_Len = BeaconLen - (size_t)(tmp_IE_ptr - BeaconBuf);
				/*search on going*/
			}
		} else {
			break;
		}
	}

	if(hs2_found==0){
		/*HS2 IE & LEN*/
		priv->pmib->hs2Entry.hs2_ielen = 0;
		priv->pmib->hs2Entry.hs_enable=0;
	}


	/*Sync Extend capability*/
	ExtCap_IE_Ptr = parsing_specific_ie((unsigned char *)BeaconBuf, _EXTENDED_CAP_IE_, 0, &ExtCap_IE_LEN, BeaconLen);

	if(ExtCap_IE_Ptr) {
		unsigned char* contend = ExtCap_IE_Ptr+2;
		NDEBUG("[Extend Capability] IE Found\n");

		if(ExtCap_IE_LEN > 1) {
			priv->proxy_arp = (contend[1] & _PROXY_ARP_);
			NDEBUG("[Proxy ARP][%s]\n",(priv->proxy_arp)? "enabled":"disabled");
		}

		if(ExtCap_IE_LEN > 2) {
			/* Bit 17 - WNM-Sleep Mode */
			if(contend[2] & BIT(1))	{
				NDEBUG("WNM-Sleep supported\n");
			} else {
				NDEBUG("WNM-Sleep not supported\n");
			}

			priv->pmib->wnmEntry.dot11vBssTransEnable = (contend[2] & _BSS_TRANSITION_);

			NDEBUG("BSS Transition [%s]\n",(priv->pmib->wnmEntry.dot11vBssTransEnable)? "enabled":"disabled");

		}

		if(ExtCap_IE_LEN > 3) {
			if(contend[3] & BIT(1)) {	/* Bit 25 - SSID List */
				NDEBUG("[SSID list] supported\n");
			}

			if(priv->pmib->hs2Entry.hs_enable){
				priv->pmib->hs2Entry.advt_proto_ie[0]=0x7f;
				priv->pmib->hs2Entry.advt_proto_ie[1]=0x00;
				priv->pmib->hs2Entry.advt_proto_ielen = 2;
				NDEBUG("HS2 is enabled force [ADVT]\n");
			}

			if(contend[3] & _INTERWORKING_SUPPORT_) {
				unsigned char *interworking_ie=NULL;
				int	interworking_ie_len=0;

				interworking_ie = parsing_specific_ie((unsigned char *)BeaconBuf, _INTERWORKING_IE_, 0, &interworking_ie_len, BeaconLen);

				if(interworking_ie) {
					memcpy(priv->pmib->hs2Entry.interworking_ie, interworking_ie+2, interworking_ie_len);
					priv->pmib->hs2Entry.interworking_ielen = interworking_ie_len;
					NDEBUG("[Interworking]present\n");
				} else {
					priv->pmib->hs2Entry.interworking_ielen=0;
					NDEBUG("[Interworking]not found\n");
				}
			} else {
				priv->pmib->hs2Entry.interworking_ielen=0;
				NDEBUG("[Interworking]not found\n");
			}
		}

		/* 20180206 ,  Qos Map should not config from here
			should call by hostapd's ops set_qos_map -> nl80211_set_qos_map
			right now use ioctl method , in wifi drv handle by 	QOS_MAP_SET_T */


		if(ExtCap_IE_LEN > 5) {
			/* Bit 46 - WNM-Notification */
			if(contend[5] & BIT(6)) {
				NDEBUG("[WNM-Notification]found\n");
			} else {
				NDEBUG("[WNM-Notification]no found\n");
			}
		}
	}
	else {
		NDEBUG("[Ext Cap] not found\n");
	}

	/*Roaming Consortium;ID 111,0x6F*/
	TAG_IE = parsing_specific_ie((unsigned char *)BeaconBuf, _ROAM_IE_, 0, &TAG_LEN, BeaconLen);
	if(TAG_IE) {
		memcpy(priv->pmib->hs2Entry.roam_ie, TAG_IE+2, TAG_LEN);
		priv->pmib->hs2Entry.roam_ielen = TAG_LEN;
		NDEBUG("Roaming IE found\n");
	} else {
		priv->pmib->hs2Entry.roam_ielen = 0;
		NDEBUG("Roaming IE not found\n");
	}

	/*Time Advt ID 69*/
	TAG_IE = parsing_specific_ie((unsigned char *)BeaconBuf, _TIMEADVT_IE_, 0, &TAG_LEN, BeaconLen);
	if(TAG_IE) {
		memcpy(priv->pmib->hs2Entry.timeadvt_ie, TAG_IE+2, TAG_LEN);
		priv->pmib->hs2Entry.timeadvt_ielen = TAG_LEN;
		NDEBUG2("Time ADV IE found\n");
	} else {
		priv->pmib->hs2Entry.timeadvt_ielen = 0;
		NDEBUG("Time ADV IE not found\n");
	}
#endif


	/*Sync Interworking IE */
	TAG_IE = get_ie((unsigned char *)BeaconBuf, _INTERWORKING_IE_, &TAG_LEN, BeaconLen);
	if (TAG_IE) {
		if (TAG_LEN != 1){
			priv->hapd_interworking_ie_len = 0;
			NDEBUG("[Interworking]IE should be 1\n");
		} else {
			memcpy(priv->hapd_interworking_ie, TAG_IE, TAG_LEN + 2);
			priv->hapd_interworking_ie_len = TAG_LEN + 2;
			NDEBUG("[Interworking] enabled\n");
		}
	} else {
		priv->hapd_interworking_ie_len = 0;
		//NDEBUG("[Interworking]not found\n");
	}

	/*Sync ANQP IE */
	TAG_IE = get_ie((unsigned char *)BeaconBuf, _ADVT_PROTO_IE_, &TAG_LEN, BeaconLen);
	if (TAG_IE) {
		if (TAG_LEN != 2){
			priv->hapd_anqp_ie_len= 0;
			NDEBUG("[ANQP]IE should be 2\n");
		} else {
			memcpy(priv->hapd_anqp_ie, TAG_IE, TAG_LEN + 2);
			priv->hapd_anqp_ie_len= TAG_LEN + 2;
			NDEBUG("[ANQP] enabled\n");
		}
	} else {
		priv->hapd_anqp_ie_len = 0;
		//NDEBUG("[ANQP]not found\n");
	}
#ifdef HAPD_OWE
	/*GET OWE transition mode info*/
	tmp_IE_ptr = (unsigned char *)BeaconBuf;
	tmp_IE_Len = BeaconLen;
	priv->owe_transition_ie_len=0;
	for(;;){
		TAG_IE = get_ie(tmp_IE_ptr, _RSN_IE_1_, &TAG_LEN, tmp_IE_Len);
		if (!TAG_IE) break;
		tmp_IE_ptr = TAG_IE+TAG_LEN+2;
		tmp_IE_Len = BeaconLen - (size_t)(tmp_IE_ptr - BeaconBuf);
		pDot11RSNIEHeader = (DOT11_RSN_IE_HEADER *)TAG_IE;
		if (memcmp(pDot11RSNIEHeader->OUI, owe_transition_oui, sizeof(owe_transition_oui))){
			continue;
		}else{
			owelog("[%s][owe_transition] len[%d]",priv->dev->name,TAG_LEN);
			memcpy(priv->owe_transition_ie, TAG_IE, TAG_LEN+2);
			priv->owe_transition_ie_len = TAG_LEN+2;
			break;
		}
	}
#endif
	/*Sync DPP IE */
	TAG_IE = get_oui((unsigned char *)BeaconBuf, (unsigned char *)&dpp_oui_str[0], &TAG_LEN, BeaconLen);
	if (TAG_IE) {
		if (TAG_LEN != 4){
			priv->hapd_dpp_ie_len= 0;
			NDEBUG("[DPP]IE should be 4\n");
		} else {
			memcpy(priv->hapd_dpp_ie, TAG_IE, TAG_LEN + 2);
			priv->hapd_dpp_ie_len= TAG_LEN + 2;
			NDEBUG("[DPP] enabled\n");
		}
	} else {
		priv->hapd_dpp_ie_len = 0;
		NDEBUG("[DPP]not found\n");
	}

	return 0;
}

void dump_ies(struct rtl8192cd_priv *priv,
					unsigned char *pies, unsigned int ies_len, unsigned char mgmt_type)
{
	unsigned char *pie = pies;
	unsigned int len, total_len = 0;
#if 0
	int i = 0;
#endif

	while(1)
	{
		len = pie[1];

		total_len += (len+2);
		if(total_len > ies_len)
		{
			printk("Exceed !!\n");
			break;
		}

		if(pie[0] == _WPS_IE_)
			copy_wps_ie(priv, pie, mgmt_type);

		//printk("[Tag=0x%02x Len=%d(0x%x)]\n", pie[0], len, len);
		pie+=2;

#if 0
		for(i=0; i<len; i++)
		{
			if((i%10) == 9)
				printk("\n");

			printk("%02x ", pie[i]);
		}

		printk("\n");
#endif

		pie+=len;

		if(total_len == ies_len)
		{
			//printk("Done \n");
			break;
		}

	}

}

void realtek_set_ies_apmode(struct rtl8192cd_priv *priv, struct cfg80211_beacon_data *info)
{
	unsigned char *beacon_buf=NULL;
	size_t beacon_len=0;

	NLENTER;
	/*BC FANG*/
	//20180130

	beacon_len = info->head_len + info->tail_len - WLAN_HDR_A4_QOS_HT_LEN ;
	beacon_buf = (unsigned char *)kmalloc(beacon_len+10, GFP_KERNEL);
    if (beacon_buf != NULL)
       if(beacon_len+10 > 4095 )
           printk(KERN_CRIT"%s %d %d \n",__func__,__LINE__,beacon_len+10);
	clear_wps_ies(priv);

	if(info->beacon_ies) {
		NDEBUG2("beacon_ies_len[%d]\n", info->beacon_ies_len);
		rtk_cfg80211_set_wps_p2p_ie(priv, info->beacon_ies, info->beacon_ies_len, MGMT_BEACON);
	}
	if(info->proberesp_ies) {

		/*Sync TIME ZONE IE , BC FANG*/
		unsigned char *ie = NULL;
		int ie_len=0; //20180130

		NDEBUG2("proberesp_ies_len[%d]\n", info->proberesp_ies_len);
		rtk_cfg80211_set_wps_p2p_ie(priv, info->proberesp_ies, info->proberesp_ies_len, MGMT_PROBERSP);
	#if 1
		/*Sync TIME ZONE IE , BC FANG*/
		ie = parsing_specific_ie((unsigned char *)info->proberesp_ies, _TIMEZONE_IE_, 0, &ie_len, info->proberesp_ies_len);

		if(ie) {
			memcpy(priv->pmib->hs2Entry.timezone_ie, ie+2, ie_len);
			priv->pmib->hs2Entry.timezone_ielen = ie_len;
			NDEBUG2("[Time Zone]\n");
		} else {
			/*Time Zone IE not found*/
		}
		/*Sync TIME ZONE IE , BC FANG*/
	#endif
	}
	if(info->assocresp_ies) {
		NDEBUG2("assocresp_ies_len[%d]\n", info->assocresp_ies_len);
		rtk_cfg80211_set_wps_p2p_ie(priv, info->assocresp_ies, info->assocresp_ies_len, MGMT_ASSOCRSP);
	}


	/*parsing some info from Beacon, BC FANG*/
#if 1
	if(beacon_buf) {
		//unsigned char beacon_str[500]={0};

		if(info->head)
			memcpy(beacon_buf, info->head+WLAN_HDR_A4_QOS_HT_LEN, info->head_len-WLAN_HDR_A4_QOS_HT_LEN);

		if(info->tail)
			memcpy(beacon_buf+info->head_len-WLAN_HDR_A4_QOS_HT_LEN, info->tail, info->tail_len);



		rtk_cfg80211_syn_beacon_ies(priv, beacon_buf, beacon_len);

		// MARK it
		//init_beacon(priv);


		kfree(beacon_buf);
	}
#endif //20180130
	/*parsing some info from Beacon, BC FANG*/

}

static int realtek_set_bss(struct rtl8192cd_priv *priv, struct cfg80211_ap_settings *info)
{

	memset(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID,0,33);
	memcpy(SSID, info->ssid, info->ssid_len);
	SSID_LEN = info->ssid_len;
	NDEBUG2("SSID[%s]\n", SSID);

	switch(info->hidden_ssid)
	{
		case NL80211_HIDDEN_SSID_NOT_IN_USE:
			HIDDEN_AP=0;
			break;
		case NL80211_HIDDEN_SSID_ZERO_CONTENTS:
			HIDDEN_AP=1;
			break;
		case NL80211_HIDDEN_SSID_ZERO_LEN:
			HIDDEN_AP=2;
            break;
		default:
			NDEBUG("fail, unknown hidden SSID option[%d]\n", info->hidden_ssid);
			return -EOPNOTSUPP;
	}

	priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod = info->beacon_interval;
	priv->pmib->dot11StationConfigEntry.dot11DTIMPeriod = info->dtim_period;

	return 0;
}

static int realtek_set_auth_type(struct rtl8192cd_priv *priv, enum nl80211_auth_type auth_type)
{
	//NDEBUG2("auth_type[0x%02X]\n", auth_type);

	switch (auth_type) {
	case NL80211_AUTHTYPE_OPEN_SYSTEM:
    	NDEBUG2("NL80211_AUTHTYPE_OPEN_SYSTEM\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = _AUTH_ALGM_OPEN_;
		break;
	case NL80211_AUTHTYPE_SHARED_KEY:
    	NDEBUG2("NL80211_AUTHTYPE_SHARED_KEY\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = _AUTH_ALGM_SHARED_;
		break;
	case NL80211_AUTHTYPE_NETWORK_EAP:
    	NDEBUG2("NL80211_AUTHTYPE_NETWORK_EAP\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = NL80211_AUTHTYPE_NETWORK_EAP;
		break;
	case NL80211_AUTHTYPE_AUTOMATIC:
    	NDEBUG2("NL80211_AUTHTYPE_AUTOMATIC\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
		break;
	case NL80211_AUTHTYPE_SAE:
	NDEBUG2("NL80211_AUTHTYPE_SAE\n");
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = _AUTH_ALGM_SAE_;
		break;
	default:
    	NDEBUG("no support auth type[0x%02x]\n",auth_type);
		return -ENOTSUPP;
	}

	return 0;
}

static void config_ssid_from_beacon(struct rtl8192cd_priv *priv, struct cfg80211_beacon_data *beacon)
{
	u8 *p, *skip_hdr, *ssid;
	s32 len, remain_len;
	skip_hdr = (u8 *)(beacon->head + (WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_));
	remain_len = beacon->head_len - (WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_);
	p = get_ie(skip_hdr, _SSID_IE_, &len, remain_len);
	if (p){
		ssid = p + 2;

		/* ssid changed */
		if ((len != SSID_LEN) || memcmp(ssid, SSID, len))
		{
			memset(SSID, 0, sizeof(SSID));
			memcpy(SSID, ssid, len);
			SSID_LEN = len;
			init_beacon(priv);
		}
	}

}


static void config_security_from_beacon(struct rtl8192cd_priv *priv,
	struct cfg80211_beacon_data *beacon, struct rtknl *rtk)
{
	u8 *p;
	s32 len;
	u16 cap_info;
	bool privacy_enable = false;

	u8 dot11EnablePSK = 0, dot11WPACipher = 0, dot11WPA2Cipher = 0, dot11PrivacyAlgrthm = 0, dot118021xAlgrthm = 0;
#ifdef CONFIG_IEEE80211W
	u8 dot11IEEE80211W = 0;
	bool dot11EnableSHA256 = false;
#endif
	size_t offset = 0;
	u16 count, i;

	DOT11_RSN_IE_HEADER *pDot11RSNIEHeader;
#ifdef CONFIG_IEEE80211W
	DOT11_RSN_IE_SUITE *pDot11RSNIESuite;
#endif
	DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite;
#ifdef CONFIG_IEEE80211W
	DOT11_RSN_CAPABILITY *pDot11RSNCapability;
#endif

	if (beacon->head)
	{
		/* get capability info */
		p = (u8 *)(beacon->head + _BEACON_CAP_OFFSET_);
		cap_info = le16_to_cpu(*((u16 *)p));
		privacy_enable = (cap_info & BIT4) ? true : false;
	}

	if (privacy_enable && beacon->tail_len)
	{
		/* parse wpa2 */
		p = get_ie((unsigned char *)beacon->tail, _RSN_IE_2_, &len, beacon->tail_len);
		if (p)
		{

#ifdef DIRECT_HAPD_RSN_IE /*directly use RSN IE from hostapd*/
			memcpy(priv->pmib->dot11RsnIE.rsnie, p, len+2);
			priv->pmib->dot11RsnIE.rsnielen=len+2;
			priv->pmib->dot11OperationEntry.keep_rsnie=1;
			NDEBUG("[_RSN_IE_2_] IE found,use it as DUT's RNS IE len:%d\n",priv->pmib->dot11RsnIE.rsnielen);
#endif
			/* header */
			p += sizeof(DOT11_WPA2_IE_HEADER);
#ifdef CONFIG_IEEE80211W
			/* group chipher */
			pDot11RSNIESuite = (DOT11_RSN_IE_SUITE *)p;
			if (pDot11RSNIESuite->Type == DOT11_ENC_TKIP)
				priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_TKIP;
			else if (pDot11RSNIESuite->Type == DOT11_ENC_CCMP)
				priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_CCMP;
			p += sizeof(DOT11_RSN_IE_SUITE);
#endif
			/* pairwise chiper */
			pDot11RSNIECountSuite = (DOT11_RSN_IE_COUNT_SUITE *)p;
			count = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);

			for (i = 0 ; i < count ; ++i)
			{
				if (pDot11RSNIECountSuite->dot11RSNIESuite[i].Type == DOT11_ENC_TKIP)
					dot11WPA2Cipher |= BIT1;
				else if (pDot11RSNIECountSuite->dot11RSNIESuite[i].Type == DOT11_ENC_CCMP)
					dot11WPA2Cipher |= BIT3;
			}

			p += (2 + count * sizeof(DOT11_RSN_IE_SUITE));

#ifdef CONFIG_IEEE80211W
			/* check akm */
			pDot11RSNIECountSuite = (DOT11_RSN_IE_COUNT_SUITE *)p;
			count = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);
			for (i = 0 ; i < count ; ++i)
				if (pDot11RSNIECountSuite->dot11RSNIESuite[i].Type == DOT11_ENC_BIP)
					dot11EnableSHA256 = true;
				else if(pDot11RSNIECountSuite->dot11RSNIESuite[i].Type == DOT11_ENC_GCMP)
				{
					dot11EnableSHA256 = true;
					dot11EnablePSK |= PSK_WPA3;
				}
			p += (2 + count * sizeof(DOT11_RSN_IE_SUITE));

			/* rsn cap */
			pDot11RSNCapability = (DOT11_RSN_CAPABILITY *)p;
			if (pDot11RSNCapability->field.MFPC && pDot11RSNCapability->field.MFPR)
				dot11IEEE80211W = MGMT_FRAME_PROTECTION_REQUIRED;
			else if (pDot11RSNCapability->field.MFPC)
				dot11IEEE80211W = MGMT_FRAME_PROTECTION_OPTIONAL;
			else
				dot11IEEE80211W = NO_MGMT_FRAME_PROTECTION;
#endif
		}

		/* parse wpa */
		do
		{
			p = get_ie((unsigned char *)(beacon->tail + offset), _RSN_IE_1_, &len, beacon->tail_len - offset);
			if (!p) break;

			offset = (p - beacon->tail) + 2 + len;
			pDot11RSNIEHeader = (DOT11_RSN_IE_HEADER *)p;

			if (memcmp(pDot11RSNIEHeader->OUI, wpa_oui, sizeof(wpa_oui)))
				continue;
#ifdef DIRECT_HAPD_RSN_IE /*directly use RSN IE from hostapd*/
			memcpy((priv->pmib->dot11RsnIE.rsnie + priv->pmib->dot11RsnIE.rsnielen), p, len+2);
			priv->pmib->dot11RsnIE.rsnielen += len+2;
			priv->pmib->dot11OperationEntry.keep_rsnie=1;
			NDEBUG("[_RSN_IE_2_] IE found,use it as DUT's RNS IE TAG_LEN: %d\n",priv->pmib->dot11RsnIE.rsnielen);
#endif

			p += sizeof(DOT11_RSN_IE_HEADER);

			/* group cipher */
#ifdef CONFIG_IEEE80211W
			pDot11RSNIESuite = (DOT11_RSN_IE_SUITE *)p;
			if (pDot11RSNIESuite->Type == DOT11_ENC_TKIP)
				priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_TKIP;
			else if (pDot11RSNIESuite->Type == DOT11_ENC_CCMP)
				priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_CCMP;
			p += sizeof(DOT11_RSN_IE_SUITE);
#endif

			/* unicast cipher */
			pDot11RSNIECountSuite = (DOT11_RSN_IE_COUNT_SUITE *)p;
			count = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);
			for (i = 0 ; i != count ; ++i)
			{
				if (pDot11RSNIECountSuite->dot11RSNIESuite[i].Type == DOT11_ENC_TKIP)
					dot11WPACipher |= BIT1;
				else if (pDot11RSNIECountSuite->dot11RSNIESuite[i].Type == DOT11_ENC_CCMP)
					dot11WPACipher |= BIT3;
			}
		} while (p);

		if (dot11WPACipher) dot11EnablePSK |= PSK_WPA;
		if (dot11WPA2Cipher) dot11EnablePSK |= PSK_WPA2;

		if (dot11EnablePSK)
		{
			if ((dot11WPACipher & BIT3) || (dot11WPA2Cipher & BIT3))
				dot11PrivacyAlgrthm = _CCMP_PRIVACY_;
			else
				dot11PrivacyAlgrthm = _TKIP_PRIVACY_;

			dot118021xAlgrthm = 1;

#ifdef CONFIG_IEEE80211W
			if (!(dot11EnablePSK & (PSK_WPA2|PSK_WPA3))
				|| !(dot11WPA2Cipher & BIT3))
			{
				dot11IEEE80211W = 0;
				dot11EnableSHA256 = 0;
			}
#endif
		}
		else if (privacy_enable) /* wep encryption */
		{
			dot11PrivacyAlgrthm = rtk->cipher;
			NDEBUG("wep encryption, need to check dot11PrivacyAlgrthm\n");
		}

		NDEBUG("dot11PrivacyAlgrthm=%d, dot11EnablePSK=%d, dot11WPACipher=%d, dot11WPA2Cipher=%d, dot118021xAlgrthm=%d\n",
			dot11PrivacyAlgrthm, dot11EnablePSK, dot11WPACipher, dot11WPA2Cipher, dot118021xAlgrthm);
#ifdef CONFIG_IEEE80211W
		NDEBUG("dot11IEEE80211W=%d, dot11EnableSHA256=%d\n", dot11IEEE80211W, dot11EnableSHA256);
#endif

		/* apply setting */
		priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
		priv->pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = dot11PrivacyAlgrthm;
		priv->pmib->dot1180211AuthEntry.dot11EnablePSK = dot11EnablePSK;
		priv->pmib->dot1180211AuthEntry.dot11WPACipher = dot11WPACipher;
		priv->pmib->dot1180211AuthEntry.dot11WPA2Cipher = dot11WPA2Cipher;
		priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm = dot118021xAlgrthm;
#ifdef CONFIG_IEEE80211W
		priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = dot11IEEE80211W;
		priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = dot11EnableSHA256;
#endif

		if (dot11EnablePSK)
			psk_init(priv);
	}
}

static int realtek_change_beacon(struct wiphy *wiphy, struct net_device *dev,
				struct cfg80211_beacon_data *beacon)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

	if ((OPMODE & WIFI_AP_STATE) == 0)
		return -EOPNOTSUPP;

	realtek_reset_security(priv);
	realtek_set_ies_apmode(priv, beacon);/*cfg p2p*/
	config_ssid_from_beacon(priv, beacon);
	config_security_from_beacon(priv, beacon, rtk);

	NLEXIT;

	return 0;
}

static int realtek_cfg80211_del_beacon(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
#ifdef MBSSID
	unsigned int i=0;
#endif
	NLENTER;

	if(priv->pmib->p2p_mib.p2p_enabled){
		OPMODE_VAL(WIFI_STATION_STATE);
	    priv->pmib->p2p_mib.p2p_enabled=0;
	}

	if ((OPMODE & WIFI_AP_STATE) == 0)
		return -EOPNOTSUPP;

	/*2021_12_28 note : root will be call first here and help to others vap call rtl8192cd_close*/
	closeopenlog("[%s]rtl8192cd_close\n",dev->name);
	rtl8192cd_close(dev);

#ifdef MBSSID
	if (IS_ROOT_INTERFACE(priv) && (dev->flags &(IFF_PROMISC|IFF_ALLMULTI))==0){
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			closeopenlog("[%s] => dev_close",priv->pvap_priv[i]->dev->name);
			dev_close(priv->pvap_priv[i]->dev);
		}
	}
#endif
#if !(defined(RTK_129X_PLATFORM) || defined(CONFIG_RTL8672))
	//priv->dev->flags &= ~IFF_UP;
#endif

	NLEXIT;
	return 0;
}


static int realtek_stop_ap(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int ret = 0;

	NLENTER;

	if(under_apmode_repeater(priv)) {
		NLINFO("Repeater! Do nothing\n");
	} else {
		ret = realtek_cfg80211_del_beacon(wiphy, dev);
		priv->nl80211_state = NL80211_CLOSE;
	}

	NLEXIT;
	return ret;
}

#if 0
static int realtek_cfg80211_add_beacon(struct wiphy *wiphy, struct net_device *dev,
				struct beacon_parameters *info)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	realtek_ap_beacon(wiphy, dev, info, true);
	realtek_ap_config_apply(priv);

	NLEXIT;
	return 0;
}

//_eric_nl ?? what's the diff between st & add beacon??
static int realtek_cfg80211_set_beacon(struct wiphy *wiphy, struct net_device *dev,
				struct beacon_parameters *info)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	realtek_ap_beacon(wiphy, dev, info, false);

	NLEXIT;
	return 0;

}

//_eric_nl ?? what's the purpose of del_beacon ??
static int realtek_cfg80211_del_beacon(struct wiphy *wiphy, struct net_device *dev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	if (OPMODE & WIFI_AP_STATE == 0)
		return -EOPNOTSUPP;
	if (priv->assoc_num == 0)
		return -ENOTCONN;

	rtl8192cd_close(priv->dev);

	NLEXIT;
	return 0;
}
#endif


static int realtek_cfg80211_set_channel(struct wiphy *wiphy, struct net_device *dev,
			      struct cfg80211_chan_def *chandef)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int channel = 0;
	/*Let Vap intereface just follow root iunterface's channel and 2ndchoffset*/
	if(IS_VAP_INTERFACE(priv)){
		priv->pmib->dot11RFEntry.dot11channel = GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel;
		priv->pmib->dot11nConfigEntry.dot11nUse40M = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11nUse40M;
		priv->pmib->dot11nConfigEntry.dot11n2ndChOffset = GET_ROOT(priv)->pmib->dot11nConfigEntry.dot11n2ndChOffset;
		return 0;
	}
	NLENTER;

	channel = ieee80211_frequency_to_channel(chandef->chan->center_freq);

	NDEBUG2("[%s]center_freq=[%u] channel=[%d] hw_value=[%u] bandwidth=[%d]\n", priv->dev->name,
		chandef->chan->center_freq, channel, chandef->chan->hw_value, chandef->width);
	hostapd_acs_scaning = 0;

	if(IS_ROOT_INTERFACE(priv))
		priv->auto_channel = priv->pshare->auto_channel = priv->pmib->miscEntry.auto_channel;
	if(!priv->pshare->auto_channel)
		priv->pmib->dot11RFEntry.dot11channel = channel;

	rtk_set_channel_mode(priv,chandef);

//	realtek_ap_default_config(priv);
//	realtek_ap_config_apply(priv);

	NLEXIT;
	return 0;
}


//Not in ath6k
static int realtek_cfg80211_change_bss(struct wiphy *wiphy,
				struct net_device *dev,
				struct bss_parameters *params)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	unsigned char dot11_rate_table[]={2,4,11,22,12,18,24,36,48,72,96,108,0};

	NLENTER;

#if 0
if (params->use_cts_prot >= 0) {
	sdata->vif.bss_conf.use_cts_prot = params->use_cts_prot;
	changed |= BSS_CHANGED_ERP_CTS_PROT;
}
#endif

	priv->pmib->dot11RFEntry.shortpreamble = params->use_short_preamble;
#ifdef UNIVERSAL_REPEATER
	changePreamble(priv, params->use_short_preamble);
#endif
#if 0
if (params->use_short_slot_time >= 0) {
	sdata->vif.bss_conf.use_short_slot =
		params->use_short_slot_time;
	changed |= BSS_CHANGED_ERP_SLOT;
}
#endif

	if (params->basic_rates) {
		int i, j;
		u32 rates = 0;

		//printk("rate = ");
		for (i = 0; i < params->basic_rates_len; i++) {
			int rate = params->basic_rates[i];
			//printk("%d ", rate);

			for (j = 0; j < 13; j++) {
				if ((dot11_rate_table[j]) == rate)
				{
					//printk("BIT(%d) ", j);
					rates |= BIT(j);
				}

			}
		}
		//printk("\n");
		priv->pmib->dot11StationConfigEntry.dot11BasicRates = rates;
	}

	priv->pmib->dot11OperationEntry.block_relay = params->ap_isolate;
	NDEBUG("AP isolate (block relay) is set to %d\n", priv->pmib->dot11OperationEntry.block_relay);

	NLEXIT;
	return 0;
}

void set_pairwise_key_for_ibss(struct rtl8192cd_priv *priv, union iwreq_data *wrqu)
{
	int i = 0;
	struct stat_info *pstat = NULL;
	struct ieee80211req_key *wk = (struct ieee80211req_key *)wrqu->data.pointer;

	printk("set_pairwise_key_for_ibss +++ \n");

	for(i=0; i<NUM_STAT; i++)
	{
		if(priv->pshare->aidarray[i] && (priv->pshare->aidarray[i]->used == TRUE))
		{
			pstat = get_stainfo(priv, priv->pshare->aidarray[i]->station.cmn_info.mac_addr);

			if(pstat)
			{
				memcpy(wk->ik_macaddr, priv->pshare->aidarray[i]->station.cmn_info.mac_addr, ETH_ALEN);
				rtl_net80211_setkey(priv->dev, NULL, wrqu, NULL);
			}
		}
	}
}

//#define TOTAL_CAM_ENTRY (priv->pshare->total_cam_entry)
/*add for HW_ENC_FOR_GROUP_CIPHER export to Halxx88Gen.c*/
void _rtl_net80211_setkey(struct net_device *dev, void *info, void *wrqu, char *extra)
{
	int ret=0;
	union iwreq_data *iw_ptr=(union iwreq_data *)wrqu;
	if(iw_ptr && iw_ptr->data.pointer){
		ret=rtl_net80211_setkey(dev, info, (union iwreq_data *)wrqu, extra);
		if(ret)
			hwgrouplog("ret=[%d]",ret);
	}
}

static int realtek_cfg80211_add_key(struct wiphy *wiphy, struct net_device *dev,
				   u8 key_index, bool pairwise,
				   const u8 *mac_addr,
				   struct key_params *params)
{

	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	union iwreq_data wrqu;
	struct ieee80211req_key wk;
#ifdef CONFIG_IEEE80211R
	struct stat_info *pstat =NULL;
#endif
	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

#ifdef CONFIG_IEEE80211R
	if (FT_ENABLE && mac_addr) {
		pstat = get_stainfo(priv, (u8 *)mac_addr);
		if (pstat == NULL) {
			pstat = alloc_stainfo(priv, (u8 *)mac_addr, -1);
			if (pstat == NULL) {
				FTDEBUG("Exceed the upper limit of supported clients...\n");
				return -1;
			}

			FTDEBUG("alloc ft-sta peer[%pm]\n",mac_addr);

			pstat->state = WIFI_AUTH_SUCCESS;
			pstat->auth_seq = 0;
			#if    defined( CONFIG_IEEE80211R ) && !defined( RTK_NL80211 )
			if (priv->pmib->dot11FTEntry.dot11FTReassociationDeadline)
				pstat->expire_to = priv->pmib->dot11FTEntry.dot11FTReassociationDeadline;
			else
				pstat->expire_to = MAX_FTREASSOC_DEADLINE;
			#else
				pstat->expire_to = priv->assoc_to;
			#endif
			pstat->AuthAlgrthm = 2;
			pstat->ft_state = state_ft_auth;
			#if defined( CONFIG_IEEE80211R ) && !defined( RTK_NL80211 )
			pstat->ft_auth_expire_to = 3;
			#endif
			pstat->tpcache_mgt = 0xffff;
			auth_list_add(priv, pstat);
		} else {
			FTDEBUG("state=%x, auth_seq=%d, ft_state=%d\n", pstat->state, pstat->auth_seq, pstat->ft_state);
			pstat->ft_state = state_ft_auth;
		}
	}
#endif

#if 0
	if (key_index > TOTAL_CAM_ENTRY) {
		NDEBUG("key index [%d] out of bounds\n", key_index);
		return -ENOENT;
	}

	if(mac_addr == NULL) {
		printk("NO MAC Address !!\n");
		return -ENOENT;;
	}
#endif

	memset(&wk, 0, sizeof(struct ieee80211req_key));

	wk.ik_keyix = key_index;

	if(mac_addr != NULL)
		memcpy(wk.ik_macaddr, mac_addr, ETH_ALEN);
	else
		memset(wk.ik_macaddr, 0, ETH_ALEN);

	#if 1
	/*in rtl_net80211_setkey(), group identification is by mac address*/
	if (!pairwise){
		memcpy(wk.ik_macaddr, BROADCAST_MAC, ETH_ALEN);
	}
	#endif

	switch (params->cipher) {
	case WLAN_CIPHER_SUITE_WEP40:
		rtk->cipher = _WEP_40_PRIVACY_;
		wk.ik_type = IEEE80211_CIPHER_WEP;
		break;
	case WLAN_CIPHER_SUITE_WEP104:
		rtk->cipher = _WEP_104_PRIVACY_;
		wk.ik_type = IEEE80211_CIPHER_WEP;
		//NDEBUG("WEP\n");
		break;
	case WLAN_CIPHER_SUITE_TKIP:
		wk.ik_type = IEEE80211_CIPHER_TKIP;
		//NDEBUG("TKIP\n");
		break;
	case WLAN_CIPHER_SUITE_CCMP:
		wk.ik_type = IEEE80211_CIPHER_AES_CCM;
		//NDEBUG("CCMP\n");
		break;
#ifdef CONFIG_IEEE80211W
	case WLAN_CIPHER_SUITE_AES_CMAC:
		wk.ik_type = IEEE80211_CIPHER_AES_CMAC;
		//NDEBUG("CMAC\n");
		break;
#endif
	default:
		NDEBUG("cipher(0x%08x) not support\n", params->cipher);
		return -EINVAL;
	}


	wk.ik_keylen = params->key_len;
	memcpy(wk.ik_keydata, params->key, params->key_len);

#if 0
	{
	int tmp = 0;
	printk("keylen = %d: ", wk.ik_keylen);
	for(tmp = 0; tmp < wk.ik_keylen; tmp ++)
		printk("%02x ", wk.ik_keydata[tmp]);
	printk("\n");
	}
	//_eric_cfg ?? key seq is not used ??

	printk("[%s] add keyid = %d, mac = %pm\n",priv->dev->name , wk.ik_keyix, wk.ik_macaddr);
	printk("type = 0x%x, flags = 0x%x, keylen = 0x%x \n", wk.ik_type, wk.ik_flags, wk.ik_keylen);
#endif


	wrqu.data.pointer = &wk;
	rtl_net80211_setkey(priv->dev, NULL, &wrqu, NULL);

#ifdef HW_ENC_FOR_GROUP_CIPHER
	/*for support HW_ENC_FOR_GROUP_CIPHER*/
	if(!pairwise && (wk.ik_type >= IEEE80211_CIPHER_WEP && wk.ik_type <= IEEE80211_CIPHER_AES_CCM) ){
		memcpy((void *)&priv->group_key_cache,(void *)&wrqu,sizeof(union iwreq_data));
		if(sizeof(struct ieee80211req_key)<=64){
			hwgrouplog("group_key_backup_done\n");
			memcpy((void *)priv->wk_bk,(void *)&wk,sizeof(struct ieee80211req_key));
			priv->group_key_cache.data.pointer = priv->wk_bk;
		}else{
			printk("[%s %d] Err chk!!",__func__,__LINE__);
		}
	}
#endif

	#if 1 //wrt-adhoc
	if(OPMODE & WIFI_ADHOC_STATE){
		if(!pairwise)
			set_pairwise_key_for_ibss(priv, &wrqu);	//or need to apply set_default_key
	}
	#endif

	NLEXIT;
	return 0;

}

static int realtek_cfg80211_del_key(struct wiphy *wiphy, struct net_device *dev,
				   u8 key_index, bool pairwise,
				   const u8 *mac_addr)
{

	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	union iwreq_data wrqu;
	struct ieee80211req_del_key wk;
	int ret=0;

	NLENTER;

	rtk->cipher = _NO_PRIVACY_;
	if(!pairwise)
	{
		// NDEBUG2("No need to delete Groupe key !!\n");
		goto realtek_cfg80211_del_key_end;
	}


	if (!realtek_cfg80211_ready(priv)){
		NDEBUG("No realtek_cfg80211_ready !!\n");
		ret = -EIO;
		goto realtek_cfg80211_del_key_end;
    }

#if 0
	if (key_index > TOTAL_CAM_ENTRY) {
		NDEBUG("key index %d out of bounds\n" ,key_index);
		return -ENOENT;
	}
#endif

 	memset(&wk, 0, sizeof(struct ieee80211req_del_key));

	wk.idk_keyix = key_index;

	if(mac_addr != NULL)
		memcpy(wk.idk_macaddr, mac_addr, ETH_ALEN);
	else
		memset(wk.idk_macaddr, 0, ETH_ALEN);


	wrqu.data.pointer = &wk;

	rtl_net80211_delkey(priv->dev, NULL, &wrqu, NULL);

realtek_cfg80211_del_key_end:
	NLEXIT;
	return ret;

}



static int realtek_cfg80211_get_key(struct wiphy *wiphy, struct net_device *dev,
				   u8 key_index, bool pairwise,
				   const u8 *mac_addr, void *cookie,
				   void (*callback) (void *cookie,
						     struct key_params *))
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct key_params params;
	struct stat_info	*pstat = NULL;
	u8 key[64] = {0};

	NLENTER;

	if(mac_addr)
		pstat = get_stainfo(priv, (unsigned char *)mac_addr);

	//NDEBUG2("key_index [%d]\n",  key_index);

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

#if 0
	if (key_index > TOTAL_CAM_ENTRY) {
		NDEBUG("key index [%d] out of bounds\n" ,  key_index);
		return -ENOENT;
	}
#endif

    #if 0
	if(pairwise)
	{
		pstat = get_stainfo(priv, mac_addr);
		if (pstat == NULL)
			return -ENOENT;
	}
    #endif

	memset(&params, 0, sizeof(params));
	params.key = key;
	realtek_get_key_from_sta(priv, pstat, &params, key_index);

	//_eric_cfg ?? key seq is not used ??
    #if 0
	params.seq_len = key->seq_len;
	params.seq = key->seq;
    #endif

	callback(cookie, &params);

	NLEXIT;

	return 0;
}

static int realtek_cfg80211_set_default_key(struct wiphy *wiphy,
					   struct net_device *dev,
					   u8 key_index, bool unicast,
					   bool multicast)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	NDEBUG2("default key_index[%d] unicast[%d] multicast[%d] \n", key_index, unicast, multicast);
	priv->pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = key_index;
	NLEXIT;
	return 0;
}

static int realtek_cfg80211_set_default_mgmt_key(struct wiphy *wiphy,
					     struct net_device *dev,
					     u8 key_idx)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	NDEBUG2("default mgmt key_index[%d]\n", key_idx);
	priv->default_mgmt_key_idx = key_idx;
	return 0;
}
/*
//not in ath6k
static int realtek_cfg80211_auth(struct wiphy *wiphy, struct net_device *dev,
			  struct cfg80211_auth_request *req)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

static int realtek_cfg80211_assoc(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_assoc_request *req)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

static int realtek_cfg80211_deauth(struct wiphy *wiphy, struct net_device *dev,
			    struct cfg80211_deauth_request *req,
			    void *cookie)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}

static int realtek_cfg80211_disassoc(struct wiphy *wiphy, struct net_device *dev,
			      struct cfg80211_disassoc_request *req,
			      void *cookie)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;
	return 0;
}
*/

//Not in ath6k
static int realtek_cfg80211_add_station(struct wiphy *wiphy, struct net_device *dev,
				 const u8 *mac, struct station_parameters *params)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
*/

	NLENTER;

	NLEXIT;
	return 0;
}


void realtek_del_station(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{
	unsigned long	flags;
	if(pstat->state & WIFI_ASOC_STATE){
		// if we even received dis_assoc from this STA don't send dis_assoc to it again
		issue_deauth(priv, pstat->cmn_info.mac_addr, _RSON_AUTH_NO_LONGER_VALID_);
	}
	flags = 0;
	SAVE_INT_AND_CLI(flags);

	if (!SWCRYPTO && pstat->dot11KeyMapping.keyInCam) {
		if (CamDeleteOneEntry(priv, pstat->cmn_info.mac_addr, 0, 0)) {
			pstat->dot11KeyMapping.keyInCam = FALSE;
			pstat->tmp_rmv_key = TRUE;
			priv->pshare->CamEntryOccupied--;
		}
	}

	if (asoc_list_del(priv, pstat))
	{
#ifdef CONFIG_RTK_WLAN_MANAGER
		if (OPMODE & WIFI_AP_STATE)
			rtw_netlink_send_del_sta_msg(priv, pstat->cmn_info.mac_addr);
#endif
		if (pstat->expire_to > 0)
		{
			cnt_assoc_num(priv, pstat, DECREASE, (char *)__FUNCTION__);
			check_sta_characteristic(priv, pstat, DECREASE);
			LOG_MSG("nl80211 ask del_station,peer[%pm]\n",pstat->cmn_info.mac_addr);
		}
	}
	free_stainfo(priv, pstat);
	RESTORE_INT(flags);

}

//eric ?? can apply to disconnect ??
#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE > KERNEL_VERSION(3,19,0))
static int realtek_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, struct station_del_parameters *params)
#else
static int realtek_cfg80211_del_station(struct wiphy *wiphy, struct net_device *dev, u8 *mac)
#endif
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct stat_info	*pstat;
	int ret=0;
#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE > KERNEL_VERSION(3,19,0))
	const u8 *mac = params->mac;
#endif

	NLENTER;

	if(!IS_DRV_OPEN(priv)) {
		NLMSG("[%s]%s is not open\n", __func__, priv->dev->name);
		goto realtek_cfg80211_del_station_end;
	}

	pstat = get_stainfo(priv, (unsigned char *)mac);

	if (pstat == NULL){
		goto realtek_cfg80211_del_station_end;
	}

	log2("peer:%pm isPMF[%d]",mac,pstat->isPMF);
	realtek_del_station(priv, pstat);

realtek_cfg80211_del_station_end:
	NLEXIT;
	return ret;
}

static int realtek_cfg80211_change_station(struct wiphy *wiphy,
					   struct net_device *dev, const u8 * mac, struct station_parameters *params)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct stat_info *pstat = NULL;
	union iwreq_data wrqu;
	struct ieee80211req_mlme mlme;
	int err = 0;
	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

	if (mac) {
		//dump_mac(priv, mac);
		pstat = get_stainfo(priv, (unsigned char *)mac);
	}

	if (pstat == NULL)
		goto realtek_cfg80211_change_station_end;

#if 0
	if ((OPMODE & WIFI_AP_STATE) == 0) {
		return -EOPNOTSUPP;
	}
#endif

	memcpy(mlme.im_macaddr, mac, ETH_ALEN);

	#if 0	/*G6 driver seem just mesh mode need this API; ignore it for now*/
	err = cfg80211_check_station_change(wiphy, params, CFG80211_STA_AP_MLME_CLIENT);
	if (err) {
		NDEBUG("cfg80211_check_station_change error(%d)!!\n", err);
		goto realtek_cfg80211_change_station_end;
	}
	#endif


	if (params->sta_flags_set & BIT(NL80211_STA_FLAG_AUTHORIZED))
		mlme.im_op = IEEE80211_MLME_AUTHORIZE;
	else
		mlme.im_op = IEEE80211_MLME_UNAUTHORIZE;

	wrqu.data.pointer = &mlme;



	if (mlme.im_op == IEEE80211_MLME_AUTHORIZE) {
		NDEBUG2("peer[%pm]	IEEE80211_MLME_AUTHORIZE\n",mac);
		//Wifi Logo FFD fragment attack, clear fragment list after 4-way handshake.
		free_sta_frag_list(priv, pstat);
#ifdef CONFIG_IEEE80211W
		if (OPMODE & WIFI_AP_STATE) {
			pstat->isPMF = (pstat->wpa_sta_info->mgmt_frame_prot) ? 1 : 0;
			PMFDEBUG("peer[%pm] isPMF[%d]\n" , mac, pstat->isPMF);
		}
#if 0//def CONFIG_IEEE80211W_CLI
		else if (OPMODE & WIFI_STATION_STATE)
			pstat->isPMF = (priv->support_pmf) ? 1 : 0;
#endif
#endif
	} else {
		NDEBUG2("IEEE80211_MLME_UNAUTHORIZE(clean port)\n");
	}

	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK || priv->pmib->dot118021xAuthEntry.dot118021xAlgrthm)	//OPENWRT_RADIUS
		rtl_net80211_setmlme(priv->dev, NULL, &wrqu, NULL);


realtek_cfg80211_change_station_end:
	NLEXIT;
	return err;
}


static void realtek_cfg80211_set_rate_info(struct rate_info *r_info, unsigned int rate, unsigned char bw, unsigned char isSgi)
{
	r_info->flags = 0;

#ifdef RTK_AC_SUPPORT
	if (is_VHT_rate(rate))
	{
		r_info->flags |= RATE_INFO_FLAGS_VHT_MCS;
		r_info->mcs = (rate - VHT_RATE_ID) % 10;
		r_info->nss = (rate - VHT_RATE_ID) / 10 + 1;
	}
	else
#endif
	if (is_MCS_rate(rate))
	{
		r_info->flags |= RATE_INFO_FLAGS_MCS;
		r_info->mcs = (rate - HT_RATE_ID);
	}
	else
		r_info->legacy = (rate&0x7f) / 2;

	if(isSgi)
		r_info->flags |= RATE_INFO_FLAGS_SHORT_GI;

	switch(bw) {
#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
		case CHANNEL_WIDTH_160:
			r_info->bw = RATE_INFO_BW_160;
			break;
		case CHANNEL_WIDTH_80:
			r_info->bw = RATE_INFO_BW_80;
			break;
		case CHANNEL_WIDTH_40:
			r_info->bw = RATE_INFO_BW_40;
			break;
		case CHANNEL_WIDTH_20:
			r_info->bw = RATE_INFO_BW_20;
			break;
		case CHANNEL_WIDTH_10:
			r_info->bw = RATE_INFO_BW_10;
			break;
		case CHANNEL_WIDTH_5:
			r_info->bw = RATE_INFO_BW_5;
			break;
		default:
			NDEBUG2("Unknown bw(=%d)\n", bw);
			r_info->bw = RATE_INFO_BW_20;
			break;
#else
		case CHANNEL_WIDTH_160:
			r_info->flags |= RATE_INFO_FLAGS_160_MHZ_WIDTH;
			break;
		case CHANNEL_WIDTH_80:
			r_info->flags |= RATE_INFO_FLAGS_80_MHZ_WIDTH;
			break;
		case CHANNEL_WIDTH_40:
			r_info->flags |= RATE_INFO_FLAGS_40_MHZ_WIDTH;
			break;
#endif
	}

}

static int realtek_cfg80211_get_station(struct wiphy *wiphy, struct net_device *dev,
				 const u8 *mac, struct station_info *sinfo)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct stat_info *pstat = NULL;

	unsigned int tx_rate, rx_rate;
	unsigned char tx_bw, rx_bw, tx_sgi, rx_sgi;

	if(mac)
		pstat = get_stainfo(priv, (unsigned char *)mac);

	if(pstat==NULL)
		return -ENOENT;

	tx_rate = pstat->current_tx_rate;
	rx_rate = pstat->rx_rate;
	tx_bw = pstat->tx_bw;
	rx_bw = pstat->rx_bw;
	tx_sgi = (pstat->ht_current_tx_info&BIT(1))?TRUE:FALSE;
	rx_sgi = (pstat->rx_splcp)?TRUE:FALSE;


	sinfo->filled = 0;

#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	sinfo->filled = BIT(NL80211_STA_INFO_INACTIVE_TIME) |
					BIT(NL80211_STA_INFO_CONNECTED_TIME)|
					BIT(NL80211_STA_INFO_RX_BYTES64)	|
					BIT(NL80211_STA_INFO_RX_PACKETS)	|
					BIT(NL80211_STA_INFO_TX_BYTES64)	|
					BIT(NL80211_STA_INFO_TX_PACKETS)	|
					BIT(NL80211_STA_INFO_SIGNAL)		|
					BIT(NL80211_STA_INFO_TX_BITRATE)	|
					BIT(NL80211_STA_INFO_RX_BITRATE)	|
					0;
#else
	sinfo->filled = STATION_INFO_INACTIVE_TIME	|
					STATION_INFO_CONNECTED_TIME	|
					STATION_INFO_RX_BYTES64		|
					STATION_INFO_RX_PACKETS		|
					STATION_INFO_TX_BYTES64		|
					STATION_INFO_TX_PACKETS		|
					STATION_INFO_SIGNAL			|
					STATION_INFO_TX_BITRATE		|
					STATION_INFO_RX_BITRATE		|
					0;
#endif

	sinfo->inactive_time = (priv->expire_to - pstat->expire_to)*1000;
	sinfo->connected_time = pstat->link_time;
	sinfo->rx_bytes = pstat->rx_bytes;
	sinfo->rx_packets = pstat->rx_pkts;
	sinfo->tx_bytes = pstat->tx_bytes;
	sinfo->tx_packets = pstat->tx_pkts;
#if defined(SIGNAL_TYPE_UNSPEC)
	sinfo->signal = pstat->rssi;
#else
	if(pstat->rssi > 100)
		sinfo->signal = -20;
	else
		sinfo->signal = pstat->rssi-100;
#endif

	realtek_cfg80211_set_rate_info(&sinfo->txrate, tx_rate, tx_bw, tx_sgi);
	realtek_cfg80211_set_rate_info(&sinfo->rxrate, rx_rate, rx_bw, rx_sgi);

#if 0 //_eric_nl ?? sinfo->bss_param ??
	if(OPMODE & WIFI_STATION_STATE)
	{
		sinfo->filled |= STATION_INFO_BSS_PARAM;
		sinfo->bss_param.flags = 0;
		sinfo->bss_param.dtim_period = priv->pmib->dot11Bss.dtim_prd;
		sinfo->bss_param.beacon_interval = priv->pmib->dot11StationConfigEntry.dot11BeaconPeriod;
	}
#endif

	//NLEXIT;
	return 0;
}


static int realtek_cfg80211_dump_station(struct wiphy *wiphy, struct net_device *dev,
				 int idx, u8 *mac, struct station_info *sinfo)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int num = 0;
	struct list_head *phead, *plist;
	struct stat_info *pstat;
	int ret = -ENOENT;
#ifdef SMP_SYNC
    unsigned long flags = 0;
#endif

	//NDEBUG2("\n");
	//printk("try dump sta[%d]\n", idx);

	if(idx >= priv->assoc_num)
		return -ENOENT;

	phead = &priv->asoc_list;
	if (!(priv->drv_state & DRV_STATE_OPEN) || list_empty(phead)) {
		return -ENOENT;
	}

    SMP_LOCK_ASOC_LIST(flags);

	plist = phead->next;
	while (plist != phead) {
		if(num == idx){
            pstat = list_entry(plist, struct stat_info, asoc_list);
			if(mac)
				memcpy(mac, pstat->cmn_info.mac_addr, ETH_ALEN);
			else
				mac = pstat->cmn_info.mac_addr;

			ret = realtek_cfg80211_get_station(wiphy, dev, pstat->cmn_info.mac_addr, sinfo);
			break;
		}
		num++;
		plist = plist->next;
	}

    SMP_UNLOCK_ASOC_LIST(flags);

	//NLEXIT;
	return ret;
}

#if 0
//not in ath6k
static int realtek_cfg80211_set_txq_params(struct wiphy *wiphy,
				    struct ieee80211_txq_params *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;
	NLNOT;

	printk("queue = %d\n", params->queue);

	return 0;

}
#endif

static int realtek_cfg80211_set_wiphy_params(struct wiphy *wiphy, u32 changed)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;

	if (changed & WIPHY_PARAM_FRAG_THRESHOLD)
		priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = wiphy->frag_threshold;
	if (changed & WIPHY_PARAM_RTS_THRESHOLD)
		priv->pmib->dot11OperationEntry.dot11RTSThreshold = wiphy->rts_threshold;
	if (changed & WIPHY_PARAM_RETRY_SHORT)
		priv->pmib->dot11OperationEntry.dot11ShortRetryLimit = wiphy->retry_short;
	if (changed & WIPHY_PARAM_RETRY_LONG)
		priv->pmib->dot11OperationEntry.dot11LongRetryLimit = wiphy->retry_long;

	if(under_apmode_repeater(priv)) {
		priv = GET_VXD_PRIV(priv);

		if (changed & WIPHY_PARAM_FRAG_THRESHOLD)
			priv->pmib->dot11OperationEntry.dot11FragmentationThreshold = wiphy->frag_threshold;
		if (changed & WIPHY_PARAM_RTS_THRESHOLD)
			priv->pmib->dot11OperationEntry.dot11RTSThreshold = wiphy->rts_threshold;
		if (changed & WIPHY_PARAM_RETRY_SHORT)
			priv->pmib->dot11OperationEntry.dot11ShortRetryLimit = wiphy->retry_short;
		if (changed & WIPHY_PARAM_RETRY_LONG)
			priv->pmib->dot11OperationEntry.dot11LongRetryLimit = wiphy->retry_long;

		NLMSG("Apply advanced settings to VXD(%s)\n",priv->dev->name);
	}

	NLEXIT;
	return 0;
}

static int realtek_cfg80211_set_ap_chanwidth(struct wiphy *wiphy,
				      struct net_device *dev,
				      struct cfg80211_chan_def *chandef)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
*/
	NLENTER;
	return realtek_cfg80211_set_channel(wiphy, dev, chandef);
}

static int realtek_cfg80211_set_monitor_channel(struct wiphy *wiphy,
					 struct cfg80211_chan_def *chandef)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);

	NLENTER;
	return realtek_cfg80211_set_channel(wiphy, priv->dev, chandef);
}

#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196

#define MAX_2G_POWER_dBm 20 //defined by OpenWrt Webpage
#define MAX_5G_POWER_dBm 20 //defined by OpenWrt Webpage


unsigned int get_max_power(struct rtl8192cd_priv *priv)
{
	int max_power = 0;

#ifdef TXPWR_LMT
	if(!priv->pshare->rf_ft_var.disable_txpwrlmt) {
		if((priv->pshare->txpwr_lmt_HT1S)
			&& (priv->pshare->txpwr_lmt_HT1S <= priv->pshare->tgpwr_HT1S_new[RF_PATH_A]))
			max_power = priv->pshare->txpwr_lmt_HT1S;
		else
			max_power = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
	}
	else
		max_power = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
#else
	max_power = priv->pshare->tgpwr_HT1S_new[RF_PATH_A];
#endif

	max_power = (max_power/2);

	//panic_printk("[%s][%s][%d] max_power=%d dBm \n", priv->dev->name, __FUNCTION__, __LINE__, max_power);

	return max_power;
}

static int realtek_cfg80211_set_tx_power(struct wiphy *wiphy,
				  struct wireless_dev *wdev,
				  enum nl80211_tx_power_setting type, int mbm)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
	int dbm = MBM_TO_DBM(mbm);
	int max_pwr=0, rfuoput=0, new_rfuoput=0, i;

	NLENTER;

	max_pwr = get_max_power(priv);

	if(max_pwr == 0) {
		if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
			max_pwr=MAX_5G_POWER_dBm;
		else
			max_pwr=MAX_2G_POWER_dBm;
	}

	//panic_printk("### max_pwr=%d dbm=%d(mbm=%d) \n", max_pwr, dbm, mbm);

	rtk->pwr_set_dbm = dbm;

	if(dbm >= max_pwr)
	{
		rfuoput = 0;
		rtk->pwr_rate = 100;
	}
#if defined(DEC_PWR_BY_PERCENTAGE)
	else if(dbm >= ((max_pwr*70)/100))
	{
		rfuoput = 1;
		rtk->pwr_rate = 70;
	}
	else if(dbm >= ((max_pwr*50)/100))
	{
		rfuoput = 2;
		rtk->pwr_rate = 50;
	}
	else if(dbm >= ((max_pwr*35)/100))
	{
		rfuoput = 3;
		rtk->pwr_rate = 35;
	}
	else
	{
		rfuoput = 4;
		rtk->pwr_rate = 15;
	}
	//panic_printk("### rfuoput idx=%d rtk->pwr_rate=%d(percent) \n", rfuoput, rtk->pwr_rate);

	if(rfuoput == 1)
		rfuoput = -3;
	else if(rfuoput == 2)
		rfuoput = -6;
	else if(rfuoput == 3)
		rfuoput = -9;
	else if(rfuoput == 4)
		rfuoput = -17;
#else
	else{
		rfuoput = (dbm-max_pwr)*2;
	}
#endif
	new_rfuoput = rfuoput;

	//panic_printk("### from cur_pwr=%d to rfuoput=%d \n",rtk->pwr_cur, rfuoput);

	rfuoput = rfuoput - rtk->pwr_cur;
	rtk->pwr_cur = new_rfuoput;

	//panic_printk("### adjust power=%d\n", rfuoput);

	if(priv->pmib->dot11RFEntry.phyBandSelect & PHY_BAND_2G) {
		for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++) {
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_A[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_B[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_C[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelCCK_D[i] = 1;
			}

			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] = 1;
			}
		}
	} else {
		for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++) {
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] = 1;
			}
			if(priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] != 0){
				if ((priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] + rfuoput) >= 1)
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] += rfuoput;
				else
					priv->pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] = 1;
			}
		}
	}

	//Apply config immediately for AP mode
	if(OPMODE & WIFI_AP_STATE)
	{
		if(priv->pmib->dot11RFEntry.dot11channel)
			SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pmib->dot11nConfigEntry.dot11n2ndChOffset);
	}

	return 0;
}

struct rtl8192cd_priv* get_priv_from_wdev(struct rtknl *rtk, struct wireless_dev *wdev)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(wdev == &(rtk->rtk_iface[tmp].priv->wdev))
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}
	return priv;
}


static int realtek_cfg80211_get_tx_power(struct wiphy *wiphy,
				  struct wireless_dev *wdev, int *dbm)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
/*
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
*/
	//NLENTER;

	if(rtk->pwr_set_dbm)
		*dbm = rtk->pwr_set_dbm;
	else
		*dbm = 13;

	//NLEXIT;
	return 0;

}


#if 1

//_eric_nl ?? suspend/resume use open/close ??
static int realtek_cfg80211_suspend(struct wiphy *wiphy, struct cfg80211_wowlan *wow)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
#if defined(RTK_129X_PLATFORM) && defined(CONFIG_OPENWRT_SDK)
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
#endif

	NLENTER;
#if defined(RTK_129X_PLATFORM) && defined(CONFIG_OPENWRT_SDK)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
	kobject_uevent(&priv->dev->dev.kobj, KOBJ_OFFLINE);
#else
	kobject_hotplug(&priv->dev->class_dev.kobj, KOBJ_OFFLINE);
#endif //(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
#else
	NLNOT;
#endif //RTK_129X_PLATFORM && CONFIG_OPENWRT_SDK

	return 0;
}

static int realtek_cfg80211_resume(struct wiphy *wiphy)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
#if defined(RTK_129X_PLATFORM) && defined(CONFIG_OPENWRT_SDK)
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
#endif

	NLENTER;
#if defined(RTK_129X_PLATFORM) && defined(CONFIG_OPENWRT_SDK)
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
	kobject_uevent(&priv->dev->dev.kobj, KOBJ_ONLINE);
#else
	kobject_hotplug(&priv->dev->class_dev.kobj, KOBJ_ONLINE);
#endif //(LINUX_VERSION_CODE > KERNEL_VERSION(2,6,12))
#else
	NLNOT;
#endif //RTK_129X_PLATFORM && CONFIG_OPENWRT_SDK

	return 0;
}

/*cfg p2p*/
void realtek_cfg80211_fill_available_channel(struct rtl8192cd_priv *priv,
			  struct cfg80211_scan_request *request)
{
	int idx=0;
    //NDEBUG("\n");
	if (request->n_channels > 0) {
		//request should scan only one band, apply band here
		// rtk_set_band_mode(priv, request->channels[0]->band, request->scan_width);

		if(IS_HAL_CHIP(priv) && ((int)request->channels[0]->band == (int)IEEE80211_BAND_5GHZ) && !(RTL_R8(0x454) & BIT(7))) {
			//To prevent treated as 2.4GHz at CheckBand88XX_AC(), Hal88XXPhyCfg.c
			//0x454 is configured at rtl8192cd_init_hw_PCI(), rtl8192cd_hw.c
			RTL_W8(0x454, RTL_R8(0x454) | BIT(7));
		}

		if(request->n_channels == 3 &&
			request->channels[0]->hw_value == 1 &&
			request->channels[1]->hw_value == 6 &&
			request->channels[2]->hw_value == 11
		){
			NDEBUG2("social_channel from cfg80211\n");
		}

		priv->available_chnl_num = request->n_channels;
		//NDEBUG2("n_channels[%d]\n",n_channels);
		if (request->n_channels==1) {
			NDEBUG2("n_channels[%d],ch[%d]\n", request->n_channels,ieee80211_frequency_to_channel(request->channels[0]->center_freq));
		}
		for (idx = 0; idx < request->n_channels; idx++){
			priv->available_chnl[idx] = ieee80211_frequency_to_channel(request->channels[idx]->center_freq);
		}
	}
}

static int realtek_cfg80211_scan_abort(struct rtl8192cd_priv *priv)
{
	if (TRUE == priv->pshare->bScanInProcess) {
		NLMSG("scan disable (bScanInProcess %s)\n", priv->dev->name);
		return -EBUSY;
	}
	if (!IS_DRV_OPEN(priv)) {
		NLMSG("scan disable (interface %s is not opened)\n", priv->dev->name);
		return -ENETDOWN;
	}

	if (!netif_running(priv->dev)) {
		NLMSG("scan disable (!netif_running %s)\n", priv->dev->name);
		return -ENETDOWN;
	}

	if (priv->pshare->rtk_remain_on_channel) {
		NLMSG("scan disable (rtk_remain_on_channel %s)\n", priv->dev->name);
		return -EBUSY;
	}

	if (priv->ss_req_ongoing) {
        NLMSG("scan disable (ss_req_ongoing %s)\n", priv->dev->name);
        return -EBUSY;
    }

	if (priv->scan_req) {
        NLMSG("scan disable (scan_req %s)\n", priv->dev->name);
        return -EBUSY;
    }

#if defined(DFS)
	if (timer_pending(&GET_ROOT(priv)->ch_avail_chk_timer)) {
		NLMSG("%s ch_avail_chk_timer pending\n", priv->dev->name);
		return -EBUSY;
	}
#endif

#ifdef UNIVERSAL_REPEATER
	if (IS_VXD_INTERFACE(priv)) {
		struct rtl8192cd_priv *priv_root = GET_ROOT(priv);

		if (!priv_root) {
			NLMSG("scan disable (priv_root = NULL %s)\n", priv_root->dev->name);
			return -ENETDOWN;
		}

		if (!netif_running(priv_root->dev)) {
			NLMSG("scan disable (!netif_running %s)\n", priv_root->dev->name);
			return -ENETDOWN;
		}

		if (priv_root->nl80211_state != NL80211_OPEN) {
			NLMSG("%s root interface not open complete\n", priv_root->dev->name);
			return -EBUSY;
		}
	}
#endif /* UNIVERSAL_REPEATER */

	return 0;
}

static int realtek_cfg80211_scan(struct wiphy *wiphy, struct cfg80211_scan_request *request)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, request->wdev);	//realtek_get_priv(wiphy, NULL);/*eric refine 23390*/
	struct cfg80211_ssid *ssids = request->ssids;
	int ret;

	NLENTER;
	NLMSG("request->flags = 0x%x \n", request->flags);

	if (is_WRT_scan_iface(priv->dev->name))
		priv = GET_ROOT(priv);

	ret = realtek_cfg80211_scan_abort(priv);
	if (ret) {
		return ret;
	}

	if (request->n_channels == 0) {
		NDEBUG2("start_scan\n");
		goto start_scan;
	}
#if 0
	{
		unsigned char idx;
		if (ssids->ssid != NULL) {
			NDEBUG("p2pssid=[%s]\n", ssids->ssid);
		}

		if (request->ie) {
			printk("request->ie = ");
			for (idx = 0; idx < request->ie_len; idx++) {
				printk(" %02x", request->ie);
			}
			printk("\n");
		}

		printk("\n");

		if (request->n_channels > 0) {
			unsigned char n_channels = 0;
			n_channels = request->n_channels;
			for (idx = 0; idx < n_channels; idx++) {
				NDEBUG("channel[%d]=%d\n", idx,
				       ieee80211_frequency_to_channel(request->channels[idx]->center_freq));
			}
		}
	}
#endif

	//rtk_abort_scan(priv);

	priv->ss_ssidlen = 0;
	memset(priv->ss_ssid, 0, sizeof(priv->ss_ssid));

#if defined(P2P_SUPPORT)
	if (ssids->ssid != NULL && !memcmp(ssids->ssid, "DIRECT-", 7)
	    && rtk_get_p2p_ie((u8 *) request->ie, request->ie_len, NULL, NULL)) {
		NDEBUG("Ssid=[%s],len[%d]...\n", ssids->ssid, ssids->ssid_len);

		priv->ss_ssidlen = (ssids->ssid_len > 32) ? 32 : ssids->ssid_len;
		memcpy(priv->ss_ssid, ssids->ssid, priv->ss_ssidlen);

		if (!rtk_p2p_is_enabled(priv)) {
			NDEBUG2("==>rtk_p2p_enable(CFG80211_P2P)\n");
			rtk_p2p_enable(priv, P2P_DEVICE, CFG80211_P2P);
		}

		priv->p2pPtr->pre_p2p_role = rtk_p2p_get_role(priv);
		priv->p2pPtr->pre_p2p_state = rtk_p2p_get_state(priv);

		rtk_p2p_set_role(priv, P2P_DEVICE);
		rtk_p2p_set_state(priv, P2P_S_SEARCH);
		//GET_ROOT(priv)->site_survey_times = SS_COUNT-2;   // pre-channel just scan twice
	} else
#endif
	if (request!=NULL && ssids !=NULL ) {
		if(strlen(ssids->ssid) && ssids->ssid_len){
			NDEBUG2("Ssid=[%s],len[%d]...\n", ssids->ssid, ssids->ssid_len);
			priv->ss_ssidlen = (ssids->ssid_len > 32) ? 32 : ssids->ssid_len;
			memcpy(priv->ss_ssid, ssids->ssid, priv->ss_ssidlen);
		}
	}
#if defined(P2P_SUPPORT)
	/*set WPS P2P IE to probe_req */
	if (request->ie && request->ie_len > 0) {
		rtk_cfg80211_set_wps_p2p_ie(priv, (u8 *) request->ie, request->ie_len, MGMT_PROBEREQ);
	}
#endif
#ifdef CUSTOMIZE_SCAN_HIDDEN_AP
	//scan for HiddenAP
	if (request->n_ssids && request->ssids[0].ssid_len) {
		priv->ss_ssidlen = request->ssids[0].ssid_len;
		memcpy(priv->ss_ssid, request->ssids[0].ssid, request->ssids[0].ssid_len);
	}
#endif

#if 0				//def WIFI_SIMPLE_CONFIG
	if (len == 2)
		priv->ss_req_ongoing = 2;	// WiFi-Simple-Config scan-req
	else
#endif

start_scan:
	if (IS_VXD_INTERFACE(priv))
		priv->ss_req_ongoing = SSFROM_REPEATER_VXD;
	else
		priv->ss_req_ongoing = SSFROM_WEB;

	wpaslog("...START SCAN...");
	priv->scan_req = request;
	if (request->n_channels > 0 && !priv->pmib->miscEntry.auto_channel) {
		/*use channels from cfg80211 parameter */
		realtek_cfg80211_fill_available_channel(priv, request);
	} else {
		/*use rtk default available channels */
		get_available_channel(priv);
	}

#if 1
	if ((OPMODE & WIFI_AP_STATE) && !priv->pmib->miscEntry.auto_channel){
		hapdlog("apmode acs");
		priv->ss_req_ongoing = 0;
		priv->ss_ssidlen = 0;
		priv->auto_channel = 1;
		priv->autoch_only_ss = 1;
		priv->autoch_only_ss_clear_ch = 0;
		priv->autoch_only_ss_2ndch_backup = priv->pshare->offset_2nd_chan;
	}
	else if((OPMODE & WIFI_AP_STATE) && priv->pmib->miscEntry.auto_channel){
		priv->auto_channel = 1; //using driver acs
		hostapd_acs_scaning++;
	}
#endif
	start_clnt_ss(priv);

	NLEXIT;
	return 0;
}

static void realtek_set_band(struct rtl8192cd_priv *priv, struct cfg80211_beacon_data *beacon, struct cfg80211_chan_def *chandef)
{
	enum ieee80211_band band = chandef->chan->band;
	enum nl80211_chan_width channel_width = chandef->width;
#ifdef CONFIG_IEEE80211W
	enum mfp_options dot11IEEE80211W = NO_MGMT_FRAME_PROTECTION;
#endif
	bool is_b_only = true, is_ht_enable = false, is_vht_enable = false;
	bool dot11nShortGIfor20M = false, dot11nShortGIfor40M = false, dot11nShortGIfor80M = false, dot11nAMPDU = false;
	bool dot11nTxSTBC = false;
#ifdef WIFI_11N_2040_COEXIST
	bool dot11n2040Coexist = false;
#endif
	u8 net_work_type = WIRELESS_11B;
	u8 phyBandSelect = PHY_BAND_2G;
#ifdef WIFI_WMM
	bool wmm_support = false;
#ifdef WMM_APSD
	bool uapsd_support = false;
#endif
#endif
#ifdef CONFIG_IEEE80211W
	bool dot11EnableSHA256 = false;
#endif
#ifdef CONFIG_IEEE80211R
	int dot11FastBSSTransitionEnabled = false, dot11FTOverDSEnabled = false, dot11FTResourceRequestSupported = false;
	u8 dot11FTMobilityDomainID[2] = {0};
#endif
	u8 *str;
	NLENTER;

	if (beacon->tail_len)
	{
		unsigned char *p;
		unsigned int len;

		/* after TIM IE */
		// mem_dump("beacon tail", beacon->tail, beacon->tail_len);

		p = get_ie((unsigned char *)beacon->tail, _EXT_SUPPORTEDRATES_IE_, &len, beacon->tail_len);
		if (p)
			is_b_only = false;

		p = get_ie((unsigned char *)beacon->tail, _HT_CAP_, &len, beacon->tail_len);
		if (p)
		{
			struct ht_cap_elmt *ht_cap = (struct ht_cap_elmt *)(p + 2);

			is_ht_enable = true;
			dot11nAMPDU = true;

			if (ht_cap->ht_cap_info & cpu_to_le16(HT_CAP_INFO_SHORT_GI20MHZ))
				dot11nShortGIfor20M = true;

			if (ht_cap->ht_cap_info & cpu_to_le16(HT_CAP_INFO_SHORT_GI40MHZ))
				dot11nShortGIfor40M = true;

			if (ht_cap->ht_cap_info & cpu_to_le16(HT_CAP_INFO_TX_STBC))
			{
				dot11nTxSTBC = true;
				NDEBUG(" Tx STBC is supported\n");
			}
		}

#ifdef WIFI_11N_2040_COEXIST
		if (is_ht_enable)
		{
			p = get_ie((unsigned char *)beacon->tail, _EXTENDED_CAP_IE_, &len, beacon->tail_len);

			if (p)
			{
				if (*(p+2) & _2040_COEXIST_SUPPORT_)
				{
					dot11n2040Coexist = true;
					NDEBUG(" HT coexist is supported\n");
				}
			}
		}
#endif

#ifdef RTK_AC_SUPPORT
		p = get_ie((unsigned char *)beacon->tail, EID_VHTCapability, &len, beacon->tail_len);
		if (p)
		{
			struct vht_cap_elmt	*vht_cap = (p + 2);

			is_vht_enable = true;

			if (vht_cap->vht_cap_info & cpu_to_le32(VHT_CAP_SHORT_GI_80))
				dot11nShortGIfor80M = true;
		}
#endif

#ifdef WIFI_WMM
		/* Check if WMM is set */
		p = (unsigned char *)beacon->tail;
		for (;;)
		{
			p = get_ie(p, _RSN_IE_1_, &len, beacon->tail_len);
			if (p)
			{
				if (!memcmp(p+2, WMM_PARA_IE, 6))
				{
					wmm_support = true;
					NDEBUG(" WMM is supported\n");
#ifdef WMM_APSD
					if (*(p+8) & BIT(7))
					{
						uapsd_support = true;
						NDEBUG(" U-APSD is supported\n");
					}
#endif
					break;
				}
			}
			else
			{
				NDEBUG("_RSN_IE_1_ cannot be found\n");
				break;
			}
			p = p + len + 2;
		}
#endif

#ifdef CONFIG_IEEE80211W
		/* check 11w configuration */
		p = get_ie((unsigned char *)beacon->tail, _RSN_IE_2_, &len, beacon->tail_len);
		if (p)
		{
			unsigned short count, i;
			signed short valid_len = len + 2;
			DOT11_RSN_IE_COUNT_SUITE *pDot11RSNIECountSuite;
			DOT11_RSN_CAPABILITY *pDot11RSNCapability;
			unsigned char oui[4] = {0x00, 0x0f, 0xac, 0x06};

			/* skip hdr and group chipher */
			p += sizeof(DOT11_WPA2_IE_HEADER) + sizeof(DOT11_RSN_IE_SUITE);
			valid_len -= sizeof(DOT11_WPA2_IE_HEADER) + sizeof(DOT11_RSN_IE_SUITE);
			if (valid_len < 0) goto check_end;

			/* skip pairwise chiper */
			pDot11RSNIECountSuite = (DOT11_RSN_IE_COUNT_SUITE *)p;
			count = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);
			p += (2 + count * sizeof(DOT11_RSN_IE_SUITE));
			valid_len -= (2 + count * sizeof(DOT11_RSN_IE_SUITE));
			if (valid_len < 0) goto check_end;

#ifdef CONFIG_IEEE80211W
			/* check akm */
			pDot11RSNIECountSuite = (DOT11_RSN_IE_COUNT_SUITE *)p;
			count = le16_to_cpu(pDot11RSNIECountSuite->SuiteCount);
			for (i = 0 ; i < count ; ++i)
				if (!memcmp(&pDot11RSNIECountSuite->dot11RSNIESuite[i], oui, sizeof(DOT11_RSN_IE_SUITE)))
					dot11EnableSHA256 = true;
			p += (2 + count * sizeof(DOT11_RSN_IE_SUITE));
			valid_len -= (2 + count * sizeof(DOT11_RSN_IE_SUITE));
			if (valid_len < 0) goto check_end;

			/* rsn cap */
			pDot11RSNCapability = (DOT11_RSN_CAPABILITY *)p;
			if (pDot11RSNCapability->field.MFPC && pDot11RSNCapability->field.MFPR)
				dot11IEEE80211W = MGMT_FRAME_PROTECTION_REQUIRED;
			else if (pDot11RSNCapability->field.MFPC)
				dot11IEEE80211W = MGMT_FRAME_PROTECTION_OPTIONAL;
			else
				dot11IEEE80211W = NO_MGMT_FRAME_PROTECTION;
#endif //CONFIG_IEEE80211W
		}
check_end:
		;
#endif
#ifdef CONFIG_IEEE80211R
		p = get_ie((u8 *)beacon->tail, _MOBILITY_DOMAIN_IE_, &len, beacon->tail_len);
		if (p) {
			dot11FastBSSTransitionEnabled = true;
			memcpy(dot11FTMobilityDomainID, GetFTMDID(p), 2);
			dot11FTOverDSEnabled = GetFTOverDS(p);
			dot11FTResourceRequestSupported = GetFTResReq(p);
		}
#endif

		/* sync countrystr */
		p = get_ie((unsigned char *)beacon->tail, _COUNTRY_IE_, &len, beacon->tail_len);
		if (p){
			NDEBUG2("sync country_str");
			str = (p + 2);
			memcpy(priv->pmib->dot11dCountry.dot11CountryString, str, 2);
			COUNTRY_CODE_ENABLED = 1;
		}

	}

	switch(band)
	{
	case IEEE80211_BAND_2GHZ:
		phyBandSelect = PHY_BAND_2G;
		net_work_type = WIRELESS_11B;
		if (!is_b_only)
		{
			if (!is_ht_enable)
				net_work_type |= WIRELESS_11G;
			else
				net_work_type |= (WIRELESS_11G | WIRELESS_11N);
		}
		break;
	case IEEE80211_BAND_5GHZ:
		phyBandSelect = PHY_BAND_5G;
		net_work_type = WIRELESS_11A;
		if (is_ht_enable)
			net_work_type |= WIRELESS_11N;

		if (is_vht_enable)
			net_work_type |= WIRELESS_11AC;
		break;
	case IEEE80211_BAND_60GHZ:
	default:
		NDEBUG("band=%d not support\n", band);
		break;
	}

	NDEBUG(" net_work_type=%d\n", net_work_type);
	NDEBUG(" sgi 20(%d), 40(%d), 80(%d)\n", dot11nShortGIfor20M, dot11nShortGIfor40M, dot11nShortGIfor80M);
#ifdef CONFIG_IEEE80211W
	NDEBUG(" 11w(%d), sha256(%d)\n", dot11IEEE80211W, dot11EnableSHA256);
#endif
#ifdef CONFIG_IEEE80211R
	NDEBUG("11r(%d), ft_overds(%d), ft_req(%d)\n", dot11FastBSSTransitionEnabled, dot11FTOverDSEnabled, dot11FTResourceRequestSupported);
	NDEBUG("ft_id=%x%x\n", dot11FTMobilityDomainID[0], dot11FTMobilityDomainID[1]);
#endif

	/* apply setting */
	priv->pmib->dot11RFEntry.phyBandSelect = phyBandSelect;
	priv->pmib->dot11BssType.net_work_type = net_work_type;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = dot11nShortGIfor20M;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = dot11nShortGIfor40M;
	priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = dot11nShortGIfor80M;
	priv->pmib->dot11nConfigEntry.dot11nSTBC = dot11nTxSTBC;
#ifdef WIFI_11N_2040_COEXIST
	priv->pmib->dot11nConfigEntry.dot11nCoexist = dot11n2040Coexist;
#endif
	priv->pmib->dot11nConfigEntry.dot11nAMPDU = dot11nAMPDU;
#ifdef WIFI_WMM
	priv->pmib->dot11QosEntry.dot11QosEnable = wmm_support;
#ifdef WMM_APSD
	priv->pmib->dot11QosEntry.dot11QosAPSD = uapsd_support;
#endif
#endif
#ifdef CONFIG_IEEE80211W
	priv->pmib->dot1180211AuthEntry.dot11IEEE80211W = dot11IEEE80211W;
	priv->pmib->dot1180211AuthEntry.dot11EnableSHA256 = dot11EnableSHA256;
#endif
#ifdef CONFIG_IEEE80211R
	priv->pmib->dot11FTEntry.dot11FastBSSTransitionEnabled = dot11FastBSSTransitionEnabled;
	memcpy(priv->pmib->dot11FTEntry.dot11FTMobilityDomainID, dot11FTMobilityDomainID, 2);
	priv->pmib->dot11FTEntry.dot11FTOverDSEnabled = dot11FTOverDSEnabled;
	priv->pmib->dot11FTEntry.dot11FTResourceRequestSupported = dot11FTResourceRequestSupported;
#endif

	/* copy from rtk_set_band_mode */
	if(channel_width == NL80211_CHAN_WIDTH_20_NOHT)
		if((OPMODE & WIFI_AP_STATE) && !under_apmode_repeater(priv))
			priv->rtk->keep_legacy = 1;

#ifdef CONFIG_WLAN_HAL_8814AE
	if(GET_CHIP_VER(priv) == VERSION_8814A)
		priv->pmib->dot11nConfigEntry.dot11nAMSDU = 2;
#endif

#ifdef UNIVERSAL_REPEATER
	if(IS_ROOT_INTERFACE(priv) && priv->pvxd_priv)
	{
		priv->pvxd_priv->pmib->dot11BssType.net_work_type = priv->pmib->dot11BssType.net_work_type;
		priv->pvxd_priv->pmib->dot11RFEntry.phyBandSelect = priv->pmib->dot11RFEntry.phyBandSelect;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nAMPDU = priv->pmib->dot11nConfigEntry.dot11nAMPDU;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nSTBC = priv->pmib->dot11nConfigEntry.dot11nSTBC;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M = priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M = priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M;
		priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M = priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M;
#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv) == VERSION_8814A)
			priv->pvxd_priv->pmib->dot11nConfigEntry.dot11nAMSDU = priv->pmib->dot11nConfigEntry.dot11nAMSDU;
#endif
	}
#endif
}

static int realtek_start_ap(struct wiphy *wiphy, struct net_device *dev,
			   struct cfg80211_ap_settings *info)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
//	struct cfg80211_scan_request *scan_req_pending = NULL;
	int ret = 0;

	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

	if ((OPMODE & (WIFI_AP_STATE | WIFI_ADHOC_STATE)) == 0) //wrt-adhoc
		return -EOPNOTSUPP;

	if (info->ssid == NULL)
		return -EINVAL;

	//scan_req_pending = priv->scan_req;

	//if(IS_ROOT_INTERFACE(priv))
	rtk_abort_scan(priv, SCAN_ABORT_START_AP);

	/*fixme, should not enable carrier here.
	 Under mac80211 architecture will be invoked by compatible-wireless */
	netif_carrier_on(priv->dev);

	realtek_reset_security(priv);

	realtek_set_band(priv, &info->beacon, &info->chandef);

	realtek_cfg80211_set_channel(wiphy, dev, &info->chandef);

	realtek_set_ies_apmode(priv, &info->beacon);

	ret = realtek_set_bss(priv, info);

	ret = realtek_set_auth_type(priv, info->auth_type);

	realtek_set_security_ap(priv, rtk, info->crypto);

	realtek_ap_default_config(priv);

	realtek_ap_config_apply(priv);

	if(priv->pshare->auto_channel)
	schedule_work(&GET_ROOT(priv)->cfg80211_cmd_queue);

	priv->pmib->miscEntry.func_off = 0;

#if defined(DFS)
	if((OPMODE&WIFI_AP_STATE) && info->chandef.chan->dfs_state == NL80211_DFS_AVAILABLE) {
		printk("*** [%s]Under DFS channel, radar detection is active ***\n",priv->dev->name);
		/* DFS activated after 1 sec; prevent switching channel due to DFS false alarm */
		mod_timer(&priv->DFS_timer, jiffies + RTL_SECONDS_TO_JIFFIES(1));
		mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

		DFS_SetReg(priv);
		RTL_W8(TXPAUSE, 0x00);
	}
#endif
	//if(scan_req_pending)
	//	realtek_cfg80211_scan(wiphy, scan_req_pending);

    if(ret){
        NDEBUG("fail[%d]\n",ret);
	}

	priv->nl80211_state = NL80211_OPEN;

	NLEXIT;

	return ret;

}


static int realtek_cfg80211_join_ibss(struct wiphy *wiphy, struct net_device *dev,
			       struct cfg80211_ibss_params *ibss_param)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
*/
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);

	NLENTER;

	if (!realtek_cfg80211_ready(priv))
		return -EIO;

	if ((OPMODE & WIFI_ADHOC_STATE) == 0)
		return -EOPNOTSUPP;

	//printk("Ad-Hoc join [%s] \n", ibss_param->ssid);

	SSID_LEN = (ibss_param->ssid_len > 32) ? 32: ibss_param->ssid_len;
	memcpy(SSID, ibss_param->ssid, SSID_LEN);

	realtek_reset_security(priv);

	if (ibss_param->privacy)
	{
		realtek_auth_wep(priv, _WEP_40_PRIVACY_);
	}

#if 0
	if (ibss_param->chandef.chan)
	{
		realtek_cfg80211_set_channel(wiphy, dev, ibss_param->chandef.chan, ibss_param->channel_type);
	}
#endif

#if 0

	if (ibss_param->channel_fixed) {
		/*
		 * TODO: channel_fixed: The channel should be fixed, do not
		 * search for IBSSs to join on other channels. Target
		 * firmware does not support this feature, needs to be
		 * updated.
		 */
		return -EOPNOTSUPP;
	}

	memset(vif->req_bssid, 0, sizeof(vif->req_bssid));
	if (ibss_param->bssid && !is_broadcast_ether_addr(ibss_param->bssid))
		memcpy(vif->req_bssid, ibss_param->bssid,
			sizeof(vif->req_bssid));
#endif

#ifdef UNIVERSAL_REPEATER
	if(IS_VXD_INTERFACE(priv))
	{
		//printk("launch vxd_ibss_beacon timer !!\n");
		construct_ibss_beacon(priv);
		issue_beacon_ibss_vxd((long unsigned int)priv);
	}
#endif

	priv->join_res = STATE_Sta_No_Bss;
	start_clnt_lookup(priv, 1);

	NLEXIT;

	return 0;
}

static int realtek_cfg80211_leave_ibss(struct wiphy *wiphy, struct net_device *dev)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
*/

	NLENTER;
	return 0;
}


static int realtek_cfg80211_set_wds_peer(struct wiphy *wiphy, struct net_device *dev,
				  const u8 *addr)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
*/
	NLENTER;
	return 0;
}

static void realtek_cfg80211_rfkill_poll(struct wiphy *wiphy)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, NULL);
*/
	NLENTER;
	return;
}


static int realtek_cfg80211_set_power_mgmt(struct wiphy *wiphy, struct net_device *dev,
				    bool enabled, int timeout)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
*/
	NLENTER;
	return 0;
}

//not in ath6k
static int realtek_cfg80211_set_bitrate_mask(struct wiphy *wiphy,
				      struct net_device *dev,
				      const u8 *addr,
				      const struct cfg80211_bitrate_mask *mask)
{
/*
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
*/
	NLENTER;
	NLNOT;

	//printk("fixed=%d, maxrate=%d\n", mask->fixed, mask->maxrate);  //mark_com

	return 0;
}
#endif

static int apply_acl_rules(struct rtl8192cd_priv *priv)
{
	unsigned int i=0;

	for (i=0; i<priv->pmib->dot11StationConfigEntry.dot11AclNum; i++)
	{
		struct list_head *pnewlist;
		struct wlan_acl_node *paclnode;

		pnewlist = priv->wlan_aclpolllist.next;
		list_del_init(pnewlist);

		paclnode = list_entry(pnewlist,	struct wlan_acl_node, list);
		memcpy((void *)paclnode->addr, priv->pmib->dot11StationConfigEntry.dot11AclAddr[i], MACADDRLEN);
		paclnode->mode = (unsigned char)priv->pmib->dot11StationConfigEntry.dot11AclMode;
		NDEBUG("[Drv]Sync MAC ACL entry[%d]: %pm,%s from MIB\n",i,paclnode->addr,(paclnode->mode&1U)? "Allowed":"Denied");
		list_add_tail(pnewlist, &priv->wlan_acl_list);
	}

	return 0;
}

static int realtek_reset_mac_acl(struct rtl8192cd_priv *priv)
{
	int i, cnt = 0;
	struct list_head *phead, *plist;
	struct wlan_acl_node *paclnode;
#ifdef SMP_SYNC
	unsigned long flags;
	flags = 0;
#endif

	for(i = 0; i < priv->pmib->dot11StationConfigEntry.dot11AclNum ; i++) {
		NDEBUG("Reset MAC ACL entry[%d]: %pm\n",i,priv->pmib->dot11StationConfigEntry.dot11AclAddr[i]);
		memset(priv->pmib->dot11StationConfigEntry.dot11AclAddr[i],0,MACADDRLEN);
	}

	// clear list
	phead = &priv->wlan_acl_list;

	if (list_empty(phead)) // nothing to remove
		goto exit;

	SMP_LOCK_ACL(flags);

	plist = phead->next;

	while(plist != phead)
	{
		paclnode = list_entry(plist, struct wlan_acl_node, list);
		plist = plist->next;
		list_del_init(&paclnode->list);
		list_add_tail(&paclnode->list, &priv->wlan_aclpolllist);
		++cnt;
	}

	SMP_UNLOCK_ACL(flags);

exit:

	if (cnt != priv->pmib->dot11StationConfigEntry.dot11AclNum)
		NDEBUG("len(wlan_aclpolllist)=%d, dot11AclNum=%d\n", cnt, priv->pmib->dot11StationConfigEntry.dot11AclNum);

	priv->pmib->dot11StationConfigEntry.dot11AclNum = 0;
	priv->pmib->dot11StationConfigEntry.dot11AclMode = 0;

	return 0;
}

static int realtek_set_mac_acl(struct wiphy *wiphy, struct net_device *dev,
			       const struct cfg80211_acl_data *params)
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int i;

	NLENTER;

	realtek_reset_mac_acl(priv);

	NDEBUG("MAC ACL mode:%s configured\n",(params->acl_policy&NL80211_ACL_POLICY_DENY_UNLESS_LISTED)? "Allow":"Deny");
	priv->pmib->dot11StationConfigEntry.dot11AclMode = (params->acl_policy&NL80211_ACL_POLICY_DENY_UNLESS_LISTED)? 1:2;

	for(i = 0; i < params->n_acl_entries ; i++) {
		priv->pmib->dot11StationConfigEntry.dot11AclNum++;
		memcpy(priv->pmib->dot11StationConfigEntry.dot11AclAddr[i],params->mac_addrs[i].addr,MACADDRLEN);
		NDEBUG("Append MAC ACL entry[%d]: %pm\n",i,priv->pmib->dot11StationConfigEntry.dot11AclAddr[i]);
	}
	NDEBUG("MAC ACL total entry number:%d\n", priv->pmib->dot11StationConfigEntry.dot11AclNum);

	apply_acl_rules(priv);

	NLEXIT;
	return 0;
}

#if defined(DFS)
static int realtek_start_radar_detection (struct wiphy *wiphy,
					 struct net_device *dev,
					 struct cfg80211_chan_def *chandef,
					 u32 cac_time_ms)
{
	int ret=0;
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	int channel = 0;
	enum nl80211_dfs_regions dfs_region;

	NLENTER;

	if(priv->pshare->dfs_chan_def) {
		if(priv->pshare->dfs_chan_def->chan)
			kfree(priv->pshare->dfs_chan_def->chan);

		kfree(priv->pshare->dfs_chan_def);
	}

	//backup chandef for DFS report
	priv->pshare->dfs_chan_def = (struct cfg80211_chan_def *)kzalloc(sizeof(struct cfg80211_chan_def), GFP_KERNEL);
	if(priv->pshare->dfs_chan_def) {
		NDEBUG("dfs_chan_def=%p\n", priv->pshare->dfs_chan_def);
		memcpy(priv->pshare->dfs_chan_def, chandef, sizeof(struct cfg80211_chan_def));
		priv->pshare->dfs_chan_def->chan = 0;
	}
    else
		return -ENOMEM;

	priv->pshare->dfs_chan_def->chan = (struct ieee80211_channel *)kzalloc(sizeof(struct ieee80211_channel), GFP_KERNEL);
	if(priv->pshare->dfs_chan_def->chan)
		memcpy(priv->pshare->dfs_chan_def->chan, chandef->chan, sizeof(struct ieee80211_channel));
	else
		return -ENOMEM;

	channel = ieee80211_frequency_to_channel(chandef->chan->center_freq);

	NDEBUG2("center_freq=[%u] channel=[%d] hw_value=[%u] bandwidth=[%d]\n",
		chandef->chan->center_freq, channel, chandef->chan->hw_value, chandef->width);

	EXTERN enum nl80211_dfs_regions reg_get_dfs_region(struct wiphy *wiphy);
	dfs_region = reg_get_dfs_region(wiphy);
	switch(dfs_region) {
		case NL80211_DFS_ETSI:
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_ETSI;
			break;
		case NL80211_DFS_FCC:
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_FCC;
			break;
		case NL80211_DFS_JP:
			priv->pmib->dot11StationConfigEntry.dot11RegDomain = DOMAIN_MKK;
			break;
		default:
			printk("%s %d Unset region, keep \"World Wide\"\n",__func__,__LINE__);
			break;
	}

	priv->pmib->dot11RFEntry.dot11channel = channel;

	rtk_set_band_mode(priv,chandef->chan->band , chandef->width);
	rtk_set_channel_mode(priv,chandef);
	SwBWMode(priv, priv->pmib->dot11nConfigEntry.dot11nUse40M, priv->pmib->dot11nConfigEntry.dot11n2ndChOffset);
	SwChnl(priv, channel, priv->pmib->dot11nConfigEntry.dot11n2ndChOffset);

	mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(cac_time_ms));
	printk("*** [%s]Activate DFS-CAC for %d miliseconds ***\n",priv->dev->name,cac_time_ms);

	/* DFS activated after 200 ms; prevent switching channel due to DFS false alarm */
	mod_timer(&priv->DFS_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
	mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

	DFS_SetReg(priv);

	if (!priv->pmib->dot11DFSEntry.CAC_enable) {
		mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
	}

	priv->pmib->dot11DFSEntry.disable_tx = 1;

	return ret;
}
#endif

int realtek_update_ft_ies(struct wiphy *wiphy, struct net_device *dev,
					 struct cfg80211_update_ft_ies_params *ftie)
{
/*
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
*/

	NLENTER;
	//NDEBUG("md=%d, ie_len=%d\n", ftie->md, ftie->ie_len);
	mem_dump((unsigned char *)__func__, (unsigned char *)ftie->ie, ftie->ie_len);

	return 0;
}

int get_bss_by_bssid(struct rtl8192cd_priv *priv, unsigned char* bssid, unsigned int bssdb_count, struct bss_desc *bssdb)
{
	int ix = 0, found = 0;

	STADEBUG("count = %d %pm\n", bssdb_count, bssid);
	//dump_mac(priv, bssid);

	for(ix = 0 ; ix < bssdb_count ; ix++){//_Eric ?? will bss_backup be cleaned?? -> Not found in  codes
		STADEBUG("[%d]Match %pm with %pm\n",ix,bssdb[ix].bssid,bssid);
		if(!memcmp(bssdb[ix].bssid , bssid, 6)){
			found = 1;
			break;
		}
	}

	if(found == 0)
	{
		STADEBUG("%s BSSID NOT Found !!\n",__func__);
		return -EINVAL;
	}
	else
		return ix;

}


int get_bss_by_ssid(struct rtl8192cd_priv *priv, const char* ssid, int ssid_len, unsigned int bssdb_count, struct bss_desc *bssdb)
{
	int ix = 0, found = 0;

	STADEBUG("count[%d] ssid[%s]\n", bssdb_count, ssid);

	for(ix = 0 ; ix < bssdb_count ; ix++){ //_Eric ?? will bss_backup be cleaned?? -> Not found in  codes
		STADEBUG("[%d]Match %s to %s\n",ix,bssdb[ix].ssid, ssid);
		if((ssid_len == bssdb[ix].ssidlen) && !memcmp(bssdb[ix].ssid , ssid, ssid_len))	{
			found = 1;
			break;
		}
	}

	if(found == 0){
		STADEBUG("SSID NOT Found !!\n");
		return -EINVAL;
	}
	else
		return ix;

}

void dump_ie(unsigned char *name, const char *ies, unsigned int ies_len, struct Dot11VariableIEs *ie)
{
	unsigned int i = 0;

	if (ies && ies_len > 0) {
		panic_printk("[%s] ies total len: %u (%#02X)\n", name, ies_len, ies_len);

		for (i = 0; i < ies_len; i++) {
			panic_printk("%02X ", (unsigned char)ies[i]);
			if (i % 16 == 15)
				panic_printk("\n");
		}
		panic_printk("\n");
	} else if (ie && ie->length > 0) {
		panic_printk("[%s] ie id: %u (%#02X), len: %u (%#02X), value:\n", name, ie->elementID, ie->elementID, ie->length, ie->length);

		for (i = 0; i < ie->length; i++) {
			panic_printk("%02X ", (unsigned char)ie->data[i]);
			if (i % 16 == 15)
				panic_printk("\n");
		}
		panic_printk("\n");
	} else {
		panic_printk("[%s] error, ies:%p, ies_len: %u, ie: %p, ie_len: %u\n", name, ies, ies_len, ie, ie->length);
	}
}

static void realtek_connect_ie_handler(struct rtl8192cd_priv *priv, const char *ies, int ies_len)
{
	struct Dot11VariableIEs *p;
	u32 i;

	if (!ies || (ies_len == 0)) {
		panic_printk("wpa_supplicant_not_include IEs\n");
		return;
	}

	//NDEBUG("[IEs from wpa_supplicant;len[%d]\n", ies_len);
	//dump_hex("[IEs]", ies, ies_len);
	dump_ie("IEs from wpa_supplicant", ies, ies_len, NULL);

	memset(&priv->pmib->dot11SuppReguClassesIE, 0, sizeof(struct Dot11SuppReguClassesIE));

	for (i = 0; i < ies_len;) {
		p = (struct Dot11VariableIEs *)(ies + i);

		switch (p->elementID)
		{
#ifdef DIRECT_HAPD_RSN_IE	/*directly use all IEs(include RNS/WPA) from wpa_supplicant */
		case _RSN_IE_2_:
			//dump_ie("RSN_IE", NULL, 0, p);
			memcpy(priv->pmib->dot11RsnIE.rsnie, p, p->length + 2);
			priv->pmib->dot11RsnIE.rsnielen = p->length + 2;
			priv->pmib->dot11OperationEntry.keep_rsnie = 1;
			break;
#endif

		case _SUPPORTED_REGULATORY_CLASSES_IE_:
			/*
			 * wpa_supplicant comment:
			 * The value of the Length field of the Supported
			 * Operating Classes element is between 2 and 253.
			 * Silently skip invalid elements to avoid interop
			 * issues when trying to use the value.
			 */
			if (p->length >= 2 && p->length <= 253) {
				//dump_ie("SUPPORTED_REGULATORY_CLASSES_IE", NULL, 0, p);
				memcpy(priv->pmib->dot11SuppReguClassesIE.ie, p, p->length + 2);
				priv->pmib->dot11SuppReguClassesIE.ielen = p->length + 2;
			}
			break;

		case _VENDOR_SPECIFIC_IE_:
			/* WPS and P2P process in rtk_cfg80211_set_wps_p2p_ie, may be integrated here */

#ifdef DIRECT_HAPD_RSN_IE
			if (!memcmp(p->data, wpa_oui, sizeof(wpa_oui))) {
				//dump_ie("WPA_IE", NULL, 0, p);
				memcpy((priv->pmib->dot11RsnIE.rsnie + priv->pmib->dot11RsnIE.rsnielen), p, p->length + 2);
				priv->pmib->dot11RsnIE.rsnielen += p->length + 2;
				priv->pmib->dot11OperationEntry.keep_rsnie = 1;
			}
#endif
			break;

		default:
			panic_printk("[%s %d][%s] ignored unknown element (id=%d elen=%d)\n", __FUNCTION__, __LINE__, priv->dev->name,
				p->elementID, p->length);
			break;
		}

		i += (p->length + 2);
	}
}

static int realtek_cfg80211_connect(
	struct wiphy *wiphy, struct net_device *dev, struct cfg80211_connect_params *sme)
{

	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	unsigned int bssdb_count = 0;
	struct bss_desc *bssdb = NULL;
	int bss_num = -1;
	int ret = 0;

	realtek_change_iftype(priv, NL80211_IFTYPE_STATION);

	NLENTER;

	if (dev) {
		//dump_mac(priv, GET_MY_HWADDR);
	}

	/*remove let vap inetrface can as client mode*/

	if (!realtek_cfg80211_ready(priv)) {
		NDEBUG2("return!\n");
		return -EIO;
	}
	#if 1//wrt_clnt
	if ((OPMODE & WIFI_STATION_STATE) == 0) {
		printk("NOT in Client Mode, can NOT Associate !!!\n");
		return -1;
	}
	#endif

	//rtk_abort_scan(priv);
	priv->receive_connect_cmd = 1;

#if 1				/*WPAS STA mode*/
	priv->pmib->wscEntry.assoc_ielen = 0;
	priv->pmib->wscEntry.wsc_enable = 0;

	if (sme->ie && (sme->ie_len > 0)) {
		/*use IEs from wpa_supplicant*/
		//dump_hex("ie from wpa_supplicant", (unsigned char *)sme->ie, sme->ie_len);
		/*set WPS P2P IE to Assoc_Req */
		rtk_cfg80211_set_wps_p2p_ie(priv, sme->ie, sme->ie_len, MGMT_ASSOCREQ);
	}

	if (priv->pmib->wscEntry.wsc_enable)
		priv->wps_issue_join_req = 1;
#endif

//=== check parameters
	if ((sme->bssid == NULL) && (sme->ssid == NULL)) {
		NDEBUG("No bssid&ssid from request !!!\n");
		return -1;
	}

	if (OPMODE & WIFI_STATION_STATE) {
		bssdb_count = priv->site_survey->count_target;
		bssdb = priv->site_survey->bss_target;
	} else {
		bssdb_count = priv->site_survey->count_backup;
		bssdb = priv->site_survey->bss_backup;
	}

	if (sme->bssid) {
		bss_num = get_bss_by_bssid(priv, (unsigned char *)sme->bssid, bssdb_count, bssdb);
	} else if (sme->ssid) {	//?? channel parameter check ??
		bss_num = get_bss_by_ssid(priv, sme->ssid, sme->ssid_len, bssdb_count, bssdb);
	} else {
		NDEBUG("Unknown rule to search BSS!!\n");
		return -1;
	}

	if (bss_num < 0) {
		NDEBUG("Can not found this bss from SiteSurvey result!!\n");
		return -1;
	} else {
		//panic_printk("\n\n");
		wpaslog("target bss num[%d]", bss_num);
	}

	priv->ss_req_ongoing = 0;	//found bss, no need to scan ...

//=== set security
	realtek_reset_security(priv);

	realtek_set_security_cli(priv, rtk, sme , bss_num);

	//dump_security_mib(priv);

	realtek_connect_ie_handler(priv, sme->ie, sme->ie_len);

#ifndef DIRECT_HAPD_RSN_IE
	if (priv->pmib->dot1180211AuthEntry.dot11EnablePSK)
		psk_init(priv);

	syncMulticastCipher(priv, &bssdb[bss_num]);	/*eric refine 23277 */
#endif

	//=== set key (for wep only)
	if (sme->key_len && (rtk->cipher & (BIT(_WEP_40_PRIVACY_) | BIT(_WEP_104_PRIVACY_)))) {
		NDEBUG2("Set wep key to connect ! \n");

		if (rtk->cipher & BIT(_WEP_40_PRIVACY_)) {
			priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_WEP40;

		} else if (rtk->cipher & BIT(_WEP_104_PRIVACY_)) {
			priv->pmib->dot11GroupKeysTable.dot11Privacy = DOT11_ENC_WEP104;

		}

		memcpy(&priv->pmib->dot11DefaultKeysTable.keytype[sme->key_idx].skey[0], sme->key, sme->key_len);

		priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKeyLen = sme->key_len;
		memcpy(&priv->pmib->dot11GroupKeysTable.dot11EncryptKey.dot11TTKey.skey[0], sme->key, sme->key_len);

		if (sme->auth_type == NL80211_AUTHTYPE_OPEN_SYSTEM)
			priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
		else if (sme->auth_type == NL80211_AUTHTYPE_SHARED_KEY)
			priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 1;
		else
			priv->pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 2;
	}

	if ((OPMODE & (WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE)) == (WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE)) {
		NDEBUG2("try issue deauth to...\n");
		if (memcmp(priv->pmib->dot11StationConfigEntry.dot11Bssid, bssdb[bss_num].bssid, 6) == 0) {
			NDEBUG2("issue deauth to...\n");
			//dump_mac(priv,priv->site_survey->bss_target[bss_num].bssid);
			issue_deauth(priv, priv->site_survey->bss_target[bss_num].bssid, _RSON_DEAUTH_STA_LEAVING_);
			OPMODE_VAL(OPMODE & (~(WIFI_AUTH_SUCCESS | WIFI_ASOC_STATE)));
		}
	}
	//=== connect
	ret = rtl_wpas_join(priv, bss_num);

	NLEXIT;
	return ret;
}



static int realtek_cfg80211_disconnect(struct wiphy *wiphy,
						  struct net_device *dev, u16 reason_code)
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	struct list_head *phead, *plist;
	struct stat_info *pstat;

	NLENTER;
	//rtk_abort_scan(priv);

	event_indicate_cfg80211(priv, NULL, CFG80211_DISCONNECTED, NULL);

	if(OPMODE & WIFI_STATION_STATE){
		phead = &priv->asoc_list;
		plist = phead->next;
		while(plist != phead){
			pstat = list_entry(plist, struct stat_info, asoc_list);
			plist = plist->next;
			issue_deauth(priv,pstat->cmn_info.mac_addr,reason_code);
			NDEBUG("===[issue_deauth] DA [%pm]\n",pstat->cmn_info.mac_addr);
		}
	}

#if !(defined(RTK_129X_PLATFORM) || defined(CONFIG_RTL8672) || defined(CONFIG_RTL_819X))
	if(IS_VXD_INTERFACE(priv))
		rtl8192cd_close(priv->dev);
#endif

	return 0;
}

static int realtek_cfg80211_channel_switch(struct wiphy *wiphy,
			struct net_device *dev, struct cfg80211_csa_settings *params)

{
/*
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
*/

	NLENTER;
	return 0;
}

static void realtek_mgmt_frame_register(struct wiphy *wiphy,
				       struct  wireless_dev *wdev,
				       u16 frame_type, bool reg)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);

	switch (frame_type) {
	case IEEE80211_STYPE_AUTH: /* 0x00B0 */
		if (reg > 0){
			wpaslog("add type 0x[%x]",frame_type);
			SET_CFG80211_REPORT_MGMT(priv, IEEE80211_STYPE_AUTH);
		}else{
			wpaslog("rm type 0x[%x]",frame_type);
			CLR_CFG80211_REPORT_MGMT(priv, IEEE80211_STYPE_AUTH);
		}
		break;
		default:
		break;
	}
}

static struct device_type wiphy_type = {
	.name	= "wlan",
};


int register_netdevice_name_rtk(struct net_device *dev)
{
	int err;

	if (strchr(dev->name, '%')) {
		err = dev_alloc_name(dev, dev->name);
		if (err < 0)
			return err;
	}

	return register_netdevice(dev);
}

#if 1 //wrt-vap

/*static int realtek_nliftype_to_drv_iftype(enum nl80211_iftype type, u8 *nw_type)
{
	switch (type) {
	case NL80211_IFTYPE_STATION:
	//case NL80211_IFTYPE_P2P_CLIENT:
		*nw_type = INFRA_NETWORK;
		break;
	case NL80211_IFTYPE_ADHOC:
		*nw_type = ADHOC_NETWORK;
		break;
	case NL80211_IFTYPE_AP:
	//case NL80211_IFTYPE_P2P_GO:
		*nw_type = AP_NETWORK;
		break;
	default:
		printk("invalid interface type %u\n", type);
		return -ENOTSUPP;
	}

	return 0;
}

static BOOLEAN realtek_is_valid_iftype(struct rtknl *rtk, enum nl80211_iftype type,
				   u8 *if_idx, u8 *nw_type)
{
	if (realtek_nliftype_to_drv_iftype(type, nw_type))
		return false;

	if (  type == NL80211_IFTYPE_AP
        || type == NL80211_IFTYPE_STATION
        || type == NL80211_IFTYPE_ADHOC
#if defined(P2P_SUPPORT)
        || type == 	NL80211_IFTYPE_P2P_CLIENT
        || type == 	NL80211_IFTYPE_P2P_GO
        || type == 	NL80211_IFTYPE_P2P_DEVICE
#endif
        ) //wrt-adhoc
		return true;

	return false;
}
*/
char check_vif_existed(struct rtl8192cd_priv *priv, struct rtknl *rtk, unsigned char *name)
{
	int tmp = 0;

	for (tmp = 0; tmp < VIF_NUM; tmp++)
	{
		if(!strcmp(name, rtk->ndev_name[tmp]))
		{
			printk("%s = %s, existed in vif[%d]\n", name, rtk->ndev_name[tmp], tmp);
			return 1;
		}
	}

	return 0;
}

unsigned char check_vif_type_match(struct rtl8192cd_priv *priv, unsigned char is_vxd)
{
	unsigned char ret = 0;

	NDEBUG2("priv[%p],(root=%d vxd=%d vap=%d)\n",	priv, IS_ROOT_INTERFACE(priv), IS_VXD_INTERFACE(priv), IS_VAP_INTERFACE(priv));
#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
	NDEBUG2("proot_priv[%p],vap_id[%d]\n", priv->proot_priv, priv->vap_id);
#endif

	if(is_vxd && IS_VXD_INTERFACE(priv))
		ret = 1;

	if((!is_vxd) && IS_VAP_INTERFACE(priv))
		ret = 1;

	if(ret){
		NDEBUG2("is_vxd[%d],type OK \n", is_vxd);
	}else{
		NDEBUG2("is_vxd[%d],type NOT match \n", is_vxd);
	}

	return ret;
}

extern int dev_change_name(struct net_device *dev, const char *newname);
void rtk_change_netdev_name(struct rtl8192cd_priv *priv, unsigned char *name)
{
#if 0
	printk("rtk_change_netdev_name for priv = 0x%x (root=%d vxd=%d vap=%d) +++ \n",
		priv, IS_ROOT_INTERFACE(priv), IS_VXD_INTERFACE(priv), IS_VAP_INTERFACE(priv));
	printk("from %s to %s \n", priv->dev->name, name);
#endif
#if defined(_INCLUDE_PROC_FS_) && defined(__KERNEL__)
	rtl8192cd_proc_remove(priv->dev);
#endif
	dev_change_name(priv->dev, name); //Need to modify kernel code to export this API
#if defined(_INCLUDE_PROC_FS_) && defined(__KERNEL__)
	rtl8192cd_proc_init(priv->dev);
#endif

}

struct rtl8192cd_priv* get_priv_vxd_from_rtk(struct rtknl *rtk)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(IS_VXD_INTERFACE(rtk->rtk_iface[tmp].priv))
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}

	//printk("name = %s priv_vxd = 0x%x \n", priv->dev->name, priv);

	return priv;
}

struct rtl8192cd_priv* get_priv_from_rtk(struct rtknl *rtk, unsigned char *name)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(!strcmp(rtk->rtk_iface[tmp].priv->dev->name, name))
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}

#if 0
	if(priv) //rtk_vap
	printk("get_priv_from_rtk name = %s priv = 0x%x %s\n", name, priv, priv->dev->name);
	else
		printk("get_priv_from_rtk = NULL !!\n");
#endif

	return priv;
}


struct rtl8192cd_priv* get_priv_from_ndev(struct rtknl *rtk, struct net_device *ndev)
{
	struct rtl8192cd_priv *priv = NULL;
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv)
		if(ndev == rtk->rtk_iface[tmp].priv->dev)
		{
			priv = rtk->rtk_iface[tmp].priv;
			break;
		}
	}

	//printk("ndev = 0x%x priv = 0x%x \n", ndev, priv);

	return priv;
}

void rtk_add_priv(struct rtl8192cd_priv *priv_add, struct rtknl *rtk)
{
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv == NULL)
		{
			rtk->rtk_iface[tmp].priv = priv_add;
			strcpy(rtk->rtk_iface[tmp].ndev_name, priv_add->dev->name); /*eric refine 23390*/
			break;
		}
	}
}

void rtk_del_priv(struct rtl8192cd_priv *priv_del, struct rtknl *rtk)
{
	int tmp = 0;

	for(tmp = 0; tmp<(IF_NUM); tmp++)
	{
		if(rtk->rtk_iface[tmp].priv == priv_del)
		{
			rtk->rtk_iface[tmp].priv = NULL;
			memset(rtk->rtk_iface[tmp].ndev_name, 0, 32);/*eric refine 23390*/
			break;
		}
	}
}

unsigned char find_ava_vif_idx(struct rtknl *rtk)
{
	unsigned char idx = 0;

	for(idx = 0; idx < VIF_NUM; idx ++)
	{
		if(rtk->ndev_name[idx][0] == 0)
			return idx;
	}

	return -1;
}

unsigned char get_vif_idx(struct rtknl *rtk, unsigned char *name)
{
	unsigned char idx = 0;

	for(idx = 0; idx < VIF_NUM; idx ++)
	{
		if(rtk->ndev_name[idx][0] != 0)
		if(strcmp(name, rtk->ndev_name[idx])==0)
			return idx;
	}

	return -1;
}


void realtek_create_vap_iface(struct rtknl *rtk, unsigned char *name)
{
	struct rtl8192cd_priv *priv = NULL;

	if (!rtk || !name) {
		WARN_ON(1);
		return;
	}

	priv = rtk->priv;

	if (check_vif_existed(priv, rtk, name))
		NDEBUG("vif interface already existed !! \n");

	if (rtk->num_vif == VIF_NUM) {
		NDEBUG("Reached maximum number of supported vif\n");
		return;
	}

	rtk->idx_vif = find_ava_vif_idx(rtk);

	NDEBUG("rtk->idx_vif = %d\n", rtk->idx_vif);

	if(rtk->idx_vif < 0)
		NDEBUG("rtk->idx_vif < 0 \n");

		if(dev_valid_name(name))
			strcpy(rtk->ndev_name[rtk->idx_vif], name);

	rtl8192cd_init_one_cfg80211(rtk);
	rtk->num_vif++;
}

#endif

int realtek_interface_add(struct rtl8192cd_priv *priv,
					  struct rtknl *rtk, const char *name,
					  enum nl80211_iftype type,
					  u8 fw_vif_idx, u8 nw_type)
{
 	struct net_device *ndev;

	NLENTER;
	//NDEBUG("type[%d]\n", type);
	ndev = priv->dev;
	//dump_mac(priv, ndev->dev_addr);
	if (!ndev){
		NDEBUG("ndev = NULL !!\n");
		free_netdev(ndev);
		return -1;
	}

	strcpy(ndev->name, name);
	realtek_change_iftype(priv, type);

	dev_net_set(ndev, wiphy_net(rtk->wiphy));

	rtk->wiphy->flags |= WIPHY_FLAG_4ADDR_AP;
	rtk->wiphy->flags |= WIPHY_FLAG_4ADDR_STATION;

	priv->wdev.wiphy = rtk->wiphy;

	ndev->ieee80211_ptr = &priv->wdev;

	SET_NETDEV_DEV(ndev, wiphy_dev(rtk->wiphy));

	priv->wdev.netdev = ndev;
	priv->wdev.iftype = type;

	SET_NETDEV_DEVTYPE(ndev, &wiphy_type);

	priv->cfg80211_interface_add = FALSE;

	register_netdev(ndev);

#if 0
	if(IS_ROOT_INTERFACE(priv))
		register_netdev(ndev);
#ifdef UNIVERSAL_REPEATER //wrt-vxd
	else if(IS_VXD_INTERFACE(priv))
		register_netdev(ndev);
#endif
	else
		register_netdevice_name_rtk(ndev);
#endif

	rtk->ndev_add = ndev;

	//NDEBUG2("add priv=[%p] wdev=[%p] ndev=[%p] rtk=[%p]\n", priv, &priv->wdev, ndev, rtk);
	rtk_add_priv(priv, rtk);

	NLEXIT;

	return 0;

}

#ifdef SUPPORT_MONITOR
void rtk_enable_monitor_mode(struct rtl8192cd_priv *priv)
{
	//if(priv->pmib->miscEntry.scan_enable)
		priv->chan_num = 0;

	priv->is_monitor_mode = TRUE;
	RTL_W32(RCR, RCR_APP_FCS | RCR_APP_MIC | RCR_APP_ICV | RCR_APP_PHYSTS | RCR_HTC_LOC_CTRL
		  | RCR_AMF | RCR_ADF | RCR_AICV | RCR_ACRC32 | RCR_CBSSID_ADHOC | RCR_AB | RCR_AM | RCR_APM | RCR_AAP);

	init_timer(&priv->chan_switch_timer);
	priv->chan_switch_timer.data = (unsigned long) priv;
	priv->chan_switch_timer.function = rtl8192cd_chan_switch_timer;
	mod_timer(&priv->chan_switch_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pmib->miscEntry.chan_switch_time));
}

void rtk_disable_monitor_mode(struct rtl8192cd_priv *priv)
{
	priv->is_monitor_mode = FALSE;
	if (timer_pending(&priv->chan_switch_timer))
		del_timer(&priv->chan_switch_timer);
}
#endif

void realtek_change_iftype(struct rtl8192cd_priv *priv ,enum nl80211_iftype type)
{
	int a4_enable = priv->pmib->miscEntry.a4_enable;
	//OPMODE &= ~(WIFI_STATION_STATE|WIFI_ADHOC_STATE|WIFI_AP_STATE);
#if defined(P2P_SUPPORT)
	if(IS_ROOT_INTERFACE(priv) && IS_DRV_OPEN(priv)){
		if(type != NL80211_IFTYPE_P2P_CLIENT)
		rtl8192cd_close(priv->dev);
	}
#endif
	switch (type) {
	case NL80211_IFTYPE_STATION:
		if((OPMODE & WIFI_STATION_STATE)==0 || !a4_enable)
			OPMODE_VAL(WIFI_STATION_STATE);
		priv->pmib->p2p_mib.p2p_enabled=0;
		priv->wdev.iftype = type;
		NDEBUG2("switch to [NL80211_IFTYPE_STATION]\n");
		break;
	case NL80211_IFTYPE_ADHOC:
		if((OPMODE & WIFI_ADHOC_STATE)==0 || !a4_enable)
			OPMODE_VAL(WIFI_ADHOC_STATE);
		priv->wdev.iftype = type;
        priv->pmib->p2p_mib.p2p_enabled=0;
		NDEBUG2("switch to [NL80211_IFTYPE_ADHOC]\n");
		break;
	case NL80211_IFTYPE_AP:
		if((OPMODE & WIFI_AP_STATE)==0 || !a4_enable)
			OPMODE_VAL(WIFI_AP_STATE);
		priv->wdev.beacon_interval = 0;
		priv->pmib->miscEntry.func_off = 1;
		priv->pmib->p2p_mib.p2p_enabled=0;
		priv->wdev.iftype = type;
#ifdef SUPPORT_MONITOR
		rtk_disable_monitor_mode(priv);
#endif
#if defined(DFS)
		/*fixme, should not disable carrier here.
		  Under mac80211 architecture will be invoked by compatible-wireless */
		netif_carrier_off(priv->dev);
#endif
		NDEBUG2("switch to [NL80211_IFTYPE_AP]\n");
		break;
#if defined(P2P_SUPPORT)
	case NL80211_IFTYPE_P2P_CLIENT:
		if((OPMODE & WIFI_STATION_STATE)==0 || !a4_enable)
		OPMODE_VAL(WIFI_STATION_STATE);
        rtk_p2p_set_role(priv,P2P_DEVICE);
        priv->pmib->p2p_mib.p2p_enabled=CFG80211_P2P;
		priv->wdev.iftype = type;
		NDEBUG2("switch to [NL80211_IFTYPE_P2P_CLIENT]\n");
		break;
	case NL80211_IFTYPE_P2P_GO:
		if((OPMODE & WIFI_AP_STATE)==0 || !a4_enable)
			OPMODE_VAL(WIFI_AP_STATE);
        rtk_p2p_set_role(priv,P2P_TMP_GO);
        priv->pmib->p2p_mib.p2p_enabled=CFG80211_P2P;
		priv->wdev.iftype = type;
		NDEBUG2("switch to [NL80211_IFTYPE_P2P_GO]\n");
		break;

    case NL80211_IFTYPE_P2P_DEVICE:
        if((OPMODE & WIFI_STATION_STATE)==0 || !a4_enable)
                OPMODE_VAL(WIFI_STATION_STATE);
        rtk_p2p_set_role(priv,P2P_DEVICE);
        priv->pmib->p2p_mib.p2p_enabled=CFG80211_P2P;
        priv->wdev.iftype = type;
		NDEBUG2("switch to [NL80211_IFTYPE_P2P_DEVICE]\n");
        break;
#endif
#ifdef SUPPORT_MONITOR
    case NL80211_IFTYPE_MONITOR:
		if((OPMODE & WIFI_SITE_MONITOR)==0 || !a4_enable)
			OPMODE_VAL(WIFI_SITE_MONITOR);
		priv->wdev.iftype = type;
		priv->pmib->p2p_mib.p2p_enabled=0;
		rtk_enable_monitor_mode(priv);
		NDEBUG2("switch to [NL80211_IFTYPE_MONITOR]\n");
        break;
#endif
	default:
		NDEBUG("invalid interface type [%d]\n", type);
		if((OPMODE & WIFI_AP_STATE)==0 || !a4_enable)
			OPMODE_VAL(WIFI_AP_STATE);
	}

}

void type_to_name(enum nl80211_iftype type, unsigned char* type_name)
{
	switch (type) {
	case NL80211_IFTYPE_STATION:
		strcpy(type_name, "NL80211_IFTYPE_STATION");
		break;
	case NL80211_IFTYPE_ADHOC:
		strcpy(type_name, "NL80211_IFTYPE_ADHOC");
		break;
	case NL80211_IFTYPE_AP:
		strcpy(type_name, "NL80211_IFTYPE_AP");
		break;
    case NL80211_IFTYPE_P2P_CLIENT:
        strcpy(type_name, "NL80211_IFTYPE_P2P_CLIENT");
        break;
	case NL80211_IFTYPE_P2P_GO:
		strcpy(type_name, "NL80211_IFTYPE_P2P_GO");
		break;
	case NL80211_IFTYPE_MONITOR:
		strcpy(type_name, "NL80211_IFTYPE_MONITOR");
		break;
	default:
		strcpy(type_name, "NOT SUPPORT TYPE");
	}
}

static struct wireless_dev *realtek_cfg80211_add_iface(struct wiphy *wiphy,
						      const char *name,
#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0))
						      unsigned char name_assign_type,
#endif
						      enum nl80211_iftype type,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)) && !defined(CPTCFG_VERSION)
						      u32 *flags,
#endif
						      struct vif_params *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy); //return &wiphy->priv;
	struct rtl8192cd_priv	*priv = rtk->priv;
	struct rtl8192cd_priv *priv_add = NULL;
	unsigned char type_name[32];

	NLENTER;

	type_to_name(type, type_name);
	//NLMSG("ADD [%s][%s=%d]\n", name, type_name, type);

#if 0//def WDS
	if(params)
	{
		printk("use_4addr = %d \n", params->use_4addr);
	}
#endif

	if((strcmp(name, "wlan0")==0) || (strcmp(name, "wlan1")==0))
	{
		//NLMSG("Root interface, just change type\n");
		priv->cfg80211_interface_add = TRUE;
		realtek_change_iftype(priv, type);
		return &rtk->priv->wdev;
	}

	priv_add = get_priv_from_rtk(rtk, (unsigned char *)name);

	if(priv_add)
	{
		unsigned char type_match = 0;
		unsigned char is_vxd = 0;

		if(is_WRT_scan_iface(name))
		{
			//printk("Add Scan interface, do nothing\n");
			return &priv_add->wdev;
		}

		if(type == NL80211_IFTYPE_AP)
		{
			is_vxd = 0;
			rtk->num_vap ++ ;
		}
		else
		{
			is_vxd = 1;
			rtk->num_vxd = 1;
		}

		type_match = check_vif_type_match(priv_add, is_vxd);

		if(!type_match)
		{
			unsigned char name_vxd[32];
			unsigned char name_vap[32];
			unsigned char name_tmp[32];
			struct rtl8192cd_priv *priv_vxd = NULL;
			struct rtl8192cd_priv *priv_vap = NULL;

			printk("Type NOT Match !!! need to change name\n");

			if(is_vxd)
			{
				priv_vap = priv_add;
				priv_vxd = get_priv_vxd_from_rtk(rtk);
			}
			else
			{
				snprintf(name_vap, sizeof(name_vap), "%s-%d", rtk->priv->dev->name, (RTL8192CD_NUM_VWLAN));
				priv_vap = get_priv_from_rtk(rtk, name_vap);
				priv_vxd = priv_add;
			}

			snprintf(name_tmp, sizeof(name_tmp), "%s-%d", rtk->priv->dev->name, (RTL8192CD_NUM_VWLAN+10));

			strncpy(name_vap, priv_vap->dev->name, sizeof(name_vap)-1);
			name_vap[sizeof(name_vap)-1] = '\0';
			strncpy(name_vxd, priv_vxd->dev->name, sizeof(name_vxd)-1);
			name_vxd[sizeof(name_vxd)-1] = '\0';

#if 0
			printk(" [BEFORE] +++ \n");
			printk("VAP = 0x%x(0x%x) name=%s \n", priv_vap, priv_vap->dev, priv_vap->dev->name);
			printk("VXD = 0x%x(0x%x) name=%s \n", priv_vxd, priv_vxd->dev, priv_vxd->dev->name);
#endif

			rtk_change_netdev_name(priv_vap, name_tmp);
			rtk_change_netdev_name(priv_vxd, name_vap);
			rtk_change_netdev_name(priv_vap, name_vxd);

#if 0
			printk(" [AFTER] --- \n");
			printk("VAP = 0x%x(0x%x) name=%s \n", priv_vap, priv_vap->dev, priv_vap->dev->name);
			printk("VXD = 0x%x(0x%x) name=%s \n", priv_vxd, priv_vxd->dev, priv_vxd->dev->name);
#endif


			if(is_vxd)
			{
#if 1 //wrt-adhoc
				{
					NDEBUG("\n\nVXD change type to %d \n\n", type);
					realtek_change_iftype(priv_vxd, type);
				}
#endif
				priv_vxd->cfg80211_interface_add = TRUE;
				return &priv_vxd->wdev;
			}
			else {
				priv_vap->cfg80211_interface_add = TRUE;
				return &priv_vap->wdev;
			}

		}
		else
		{
			//printk("Type OK, do nothing\n");

#if 1 //wrt-adhoc
			if(is_vxd)
			{
    			NDEBUG("\n\nVXD change type to %d \n\n", type);
				realtek_change_iftype(priv_add, type);
			}
#endif
			priv_add->cfg80211_interface_add = TRUE;
			return &priv_add->wdev;
		}

	}
	else
	{
		printk("Can not find correspinding priv for %s !!\n", name);
		return NULL;
	}

	NLEXIT;

	return &rtk->priv->wdev;

}


void close_vxd_vap(struct rtl8192cd_priv *priv_root)
{
	int i = 0;

//#ifdef UNIVERSAL_REPEATER
#if 0	//prevent drop vxd connection
	if(IS_DRV_OPEN(priv_root->pvxd_priv))
		rtl8192cd_close(priv_root->pvxd_priv->dev);
#endif

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if(IS_DRV_OPEN(priv_root->pvap_priv[i]))
			rtl8192cd_close(priv_root->pvap_priv[i]->dev);
	}
#endif

}

static int realtek_cfg80211_del_iface(struct wiphy *wiphy,
				     struct wireless_dev *wdev)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv	*priv = get_priv_from_wdev(rtk, wdev);
	unsigned char *name = NULL;

	NLENTER;

	if(priv)
	{
		name = priv->dev->name;
		NDEBUG2("del_iface:name[%s] priv[%p] wdev[%p]\n", name, priv, wdev);
	}
	else
	{
		NDEBUG2("Can NOT find priv from wdev[%p]", wdev);
		return -1;
	}

	/*remove netif_stop_queue(priv->dev) to avoid virtual interface ask queue packet*/
	/*openwrt bug fixing*/
	if(priv->pmib->dot11StationConfigEntry.dot11AclMode)
		realtek_reset_mac_acl(priv);
	if(priv->cfg80211_interface_add == FALSE) {
		NDEBUG2("del_iface:name[%s] is already closed\n", name);
		return 0;
	}
#ifdef MBSSID
	if(IS_ROOT_INTERFACE(priv))
	{
		close_vxd_vap(priv);
	}

#ifdef UNIVERSAL_REPEATER
	if(IS_VXD_INTERFACE(priv))
		rtk->num_vxd = 0;
#endif

	if(IS_VAP_INTERFACE(priv))
		rtk->num_vap --;
#endif
	priv->cfg80211_interface_add = FALSE;

	priv->receive_connect_cmd = 0;

	rtk_abort_scan(priv, SCAN_ABORT_DEL_IFACE);

	if(IS_ROOT_INTERFACE(priv)){
		closeopenlog("[%s]rtl8192cd_close\n",priv->dev->name);
		rtl8192cd_close(priv->dev);
	}

	NLEXIT;

	return 0;
}

//survey_dump
static int realtek_dump_survey(struct wiphy *wiphy,
								struct net_device *dev,
								int idx,
								struct survey_info *survey)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = rtk->priv;
	struct ieee80211_supported_band *sband=NULL;
	int freq, band=0;
	u32 time_clm_tmp = 0;
	u32 time_nhm_tmp = 0;
	//NLENTER;
	for (band = 0; band < IEEE80211_NUM_BANDS; band++) {
		sband = wiphy->bands[band];

		if (sband)
			break;;
	}

	if(!sband) {
		NDEBUG("%s under unknown band!!\n",dev->name);
        return -1;
	} else {
		//this ops will be invoked several times, until statistic of all channels reported
		if(idx > sband->n_channels-1)
		{
			NDEBUG2("Exceed maximum:%d, statistic of all channels were reported \n", sband->n_channels-1);
			return -1;
		}
	}

	if((int)sband->band == (int)IEEE80211_BAND_2GHZ)
		freq = ieee80211_channel_to_frequency(priv->rtk->survey_info[idx].channel, IEEE80211_BAND_2GHZ);
	else
		freq = ieee80211_channel_to_frequency(priv->rtk->survey_info[idx].channel, IEEE80211_BAND_5GHZ);

	survey->channel = ieee80211_get_channel(wiphy, freq);

	survey->noise = (priv->pshare->clm_nhm_rtp[idx].nhm_noise_pwr - 100); //report dbm

#if /*defined(OPENWRT_CC) || */(LINUX_VERSION_CODE >= KERNEL_VERSION(4, 0, 0))
	survey->time = SS_AUTO_CHNL_NHM_TO_VALUE-10;/*unit ms*/
	time_clm_tmp = priv->pshare->clm_nhm_rtp[idx].clm_ratio;
	//time_clm_tmp = time_clm_tmp>>2;
	time_nhm_tmp = priv->pshare->clm_nhm_rtp[idx].nhm_ratio;
	//time_nhm_tmp = time_nhm_tmp>>4;

	survey->time_busy = 
		((time_clm_tmp + time_nhm_tmp) *	(SS_AUTO_CHNL_NHM_TO_VALUE-10))/100;
	survey->time_rx = priv->rtk->survey_info[idx].rx_time;
	survey->time_tx = priv->rtk->survey_info[idx].tx_time;
	survey->filled = SURVEY_INFO_NOISE_DBM|SURVEY_INFO_TIME|SURVEY_INFO_TIME_BUSY;//|SURVEY_INFO_TIME_RX|SURVEY_INFO_TIME_TX;
#else
	survey->channel_time = 1000;
	survey->channel_time_busy = priv->rtk->survey_info[idx].chbusytime;
	survey->channel_time_rx = priv->rtk->survey_info[idx].rx_time;
	survey->channel_time_tx = priv->rtk->survey_info[idx].tx_time;
	survey->filled = SURVEY_INFO_NOISE_DBM|SURVEY_INFO_CHANNEL_TIME|SURVEY_INFO_CHANNEL_TIME_BUSY|SURVEY_INFO_CHANNEL_TIME_RX|SURVEY_INFO_CHANNEL_TIME_TX;
#endif
	//survey->time_busy = priv->ss_channel_info[idx].utilization;

	log2("ch[%u] time_busy[%llu/%llu],clm[%d],nhm[%d] noise[%d]dbm",
		priv->rtk->survey_info[idx].channel,
		survey->time_busy,
		survey->time,
		time_clm_tmp,
		time_nhm_tmp,
		priv->pshare->clm_nhm_rtp[idx].nhm_noise_pwr
		);
	//NLEXIT;

	return 0;
}


static int realtek_cfg80211_change_iface(struct wiphy *wiphy,
					struct net_device *ndev,
					enum nl80211_iftype type,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 12, 0)) && !defined(CPTCFG_VERSION)
					u32 *flags,
#endif
					struct vif_params *params)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv	*priv = get_priv_from_ndev(rtk, ndev); //rtk->priv;
	unsigned char type_name[32];

	NLENTER;

	type_to_name(type, type_name);
	NLMSG("CHANGE [%s][%s=%d]\n", priv->dev->name, type_name, type);
	realtek_change_iftype(priv, type);

	if (priv->scan_req)
		event_indicate_cfg80211(priv, NULL, CFG80211_SCAN_ABORTED, NULL);

	NLEXIT;
	return 0;
}


int realtek_remain_on_channel(struct wiphy *wiphy,
			      struct wireless_dev *wdev,
			      struct ieee80211_channel *channel, unsigned int duration, u64 * cookie)
{

	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
	u8 remain_ch = (u8) ieee80211_frequency_to_channel(channel->center_freq);
	NLENTER;
	NDEBUG2("\n");
	if (timer_pending(&priv->pshare->remain_on_ch_timer))
		del_timer(&priv->pshare->remain_on_ch_timer);

	memcpy(&priv->pshare->remain_on_ch_channel, channel, sizeof(struct ieee80211_channel));
	priv->pshare->remain_on_ch_cookie = *cookie;
	/*restore remain_on channel */
	priv->pshare->remain_on_ch_u8 = remain_ch;
	if( remain_ch != priv->pshare->working_channel2){
		NDEBUG2("cfg80211 request remain_on ch[%d] DUT's ch[%d], =>SwChnl\n",
			remain_ch,priv->pshare->working_channel2);
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, remain_ch, priv->pshare->offset_2nd_chan);
	}
#ifdef P2P_SUPPORT	
	priv->pmib->p2p_mib.p2p_listen_channel = remain_ch;	/*set listen channel to remain channel */
	priv->p2pPtr->pre_p2p_role = rtk_p2p_get_role(priv);
	priv->p2pPtr->pre_p2p_state = rtk_p2p_get_state(priv);
	if (!rtk_p2p_is_enabled(priv)) {
		NDEBUG2("==>rtk_p2p_enable(CFG80211_P2P)\n");
		rtk_p2p_enable(priv, P2P_DEVICE, CFG80211_P2P);
	}
	rtk_p2p_set_role(priv, P2P_DEVICE);	// role to device
	P2P_listen(priv, NULL);	//state to listen
	NDEBUG2("role[%d] state[%d]\n", rtk_p2p_get_role(priv),rtk_p2p_get_state(priv));
#endif

	/*lock channel switch; include site survey , ss_timer*/
	priv->pshare->rtk_remain_on_channel = 1;
	cfg80211_ready_on_channel(wdev, *cookie, channel, duration, GFP_ATOMIC);

	if (duration < 400)
		duration = duration * 3;	//extend from exper. unit ms
	mod_timer(&priv->pshare->remain_on_ch_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(duration));

	return 0;
}

static int realtek_cancel_remain_on_channel(struct wiphy *wiphy, struct wireless_dev *wdev, u64 cookie)
{

	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
	NLENTER;
	NDEBUG2("\n");
	if (timer_pending(&priv->pshare->remain_on_ch_timer)) {
		del_timer(&priv->pshare->remain_on_ch_timer);
	}
	/*unlock [channel switch] ; unlock site survey , ss_timer*/
	priv->pshare->rtk_remain_on_channel = 0;
	if(priv->pshare->remain_on_ch_u8 != GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel){
		NDEBUG2("remain_on_ch_u8[%d], dot11channel[%d], =>SwChnl\n",
			priv->pshare->remain_on_ch_u8,GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel);
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv,GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
		priv->pshare->remain_on_ch_u8 = GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel;
	}
#ifdef P2P_SUPPORT
	rtk_p2p_set_role(priv, priv->p2pPtr->pre_p2p_role);
	rtk_p2p_set_state(priv, priv->p2pPtr->pre_p2p_state);
#endif
	return 0;
}

void realtek_cfg80211_remain_on_ch_expire(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct wireless_dev *wdev = &priv->wdev;
	NDEBUG2("\n");
#ifdef P2P_SUPPORT
	rtk_p2p_set_role(priv, priv->p2pPtr->pre_p2p_role);
	rtk_p2p_set_state(priv, priv->p2pPtr->pre_p2p_state);
#endif
	cfg80211_remain_on_channel_expired(wdev,priv->pshare->remain_on_ch_cookie,
		&priv->pshare->remain_on_ch_channel,GFP_ATOMIC);

	/*unlock [channel switch] ; unlock site survey , ss_timer*/
	priv->pshare->rtk_remain_on_channel = 0;

	if(priv->pshare->remain_on_ch_u8 != GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel){
		NDEBUG2("remain_on_ch_u8[%d], dot11channel[%d], =>SwChnl\n",
			priv->pshare->remain_on_ch_u8,GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel);
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv,GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
		priv->pshare->remain_on_ch_u8 = GET_ROOT(priv)->pmib->dot11RFEntry.dot11channel;
	}

	return;
}

void realtek_cfg80211_ch_switch_notify(struct work_struct *work)
{
	//u8 ret = SUCCESS;
	struct rtl8192cd_priv *priv = container_of(work, struct rtl8192cd_priv, cfg80211_cmd_queue);
	int freq;
	unsigned char dot11channel = priv->pmib->dot11RFEntry.dot11channel;
	int centerchannel = get_center_channel(priv, dot11channel, priv->pshare->offset_2nd_chan, 0);
	u8 ht = 0;
	u8 bw = priv->pshare->is_40m_bw;
	if(priv->ht_ie_len > 0)
		ht = 1;

	struct cfg80211_chan_def chdef;

	if(centerchannel == 0 || dot11channel == 0)
		return;

	if (bw == CHANNEL_WIDTH_20)
		chdef.width = ht ? NL80211_CHAN_WIDTH_20 : NL80211_CHAN_WIDTH_20_NOHT;
	else if (bw == CHANNEL_WIDTH_40)
		chdef.width = NL80211_CHAN_WIDTH_40;
	else if (bw == CHANNEL_WIDTH_80)
		chdef.width = NL80211_CHAN_WIDTH_80;
	else if (bw == CHANNEL_WIDTH_160)
		chdef.width = NL80211_CHAN_WIDTH_160;
	else if (bw == CHANNEL_WIDTH_5)
		chdef.width = NL80211_CHAN_WIDTH_5;
	else if (bw == CHANNEL_WIDTH_10)
		chdef.width = NL80211_CHAN_WIDTH_10;
	else {
		printk("	chk , unknow width!!! \n\n");
		return;
	}


	freq = ieee80211_channel_to_frequency(dot11channel, (dot11channel < 14) ? IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ);
	chdef.chan = ieee80211_get_channel(priv->rtk->wiphy, freq);
	chdef.center_freq1 = ieee80211_channel_to_frequency(centerchannel, (centerchannel < 14) ? IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ);
	chdef.center_freq2 = 0;

	if (OPMODE & WIFI_STATION_STATE) {
		cfg80211_ch_switch_started_notify(priv->dev, &chdef, 0);
	}
	else
		{
			if(!priv->auto_channel && IS_DRV_OPEN(priv))
			{
				if((hostapd_acs_scaning == 0 || !priv->pmib->miscEntry.auto_channel))
				{
					cfg80211_ch_switch_notify(priv->dev, &chdef);
				}
			}
		}
}

#ifdef CONFIG_IEEE80211R
static void set_ft_ie(WPA_STA_INFO *sta_info, u8 *pbuf, int limit)
{
	u8 *p;

	p = get_ie(pbuf, _RSN_IE_2_, &sta_info->rsn_ie_len, limit);
	if (p) {
		memcpy(sta_info->rsn_ie, p + 2, sta_info->rsn_ie_len);
	}

	p = get_ie(pbuf, _MOBILITY_DOMAIN_IE_, &sta_info->md_ie_len, limit);
	if (p) {
		memcpy(sta_info->md_ie, p + 2, sta_info->md_ie_len);
	}

	p = get_ie(pbuf, _FAST_BSS_TRANSITION_IE_, &sta_info->ft_ie_len, limit);
	if (p) {
		memcpy(sta_info->ft_ie, p + 2, sta_info->ft_ie_len);
	}
}
#endif
/*END of P2P_SUPPORT*/

int rtw_sae_preprocess(
	struct rtl8192cd_priv *priv,
	 const u8 *buf, u32 len,
	 u8 tx)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)buf;
	const u8 *frame_body = buf + (ieee80211_hdrlen(mgmt->frame_control));
	u16 alg;
	u16 seq;
	u16 status;
	int ret = 0;

	alg = RTW_GET_LE16(frame_body);
	if (alg != WLAN_AUTH_SAE)
		return ret;

	seq = RTW_GET_LE16(frame_body + 2);
	status = RTW_GET_LE16(frame_body + 4);
	wpaslog("	%s : AUTH alg[0x%02x] seq[0x%02x] status[0x%02x] mesg[%s]",
		(tx == TRUE) ? "Tx" : "Rx", alg, seq, status,
		(seq == 1) ? "Commit" : "Confirm");

	ret = 1;
	if (tx && (seq == 2) && (status == 0) && (OPMODE&WIFI_AP_STATE)) {
			/*AP mode; queue confirm frame until external auth status update */
			ret = 2;
	}

	return ret;
}

int rtk_cfg80211_mgmt_tx(struct rtl8192cd_priv *priv,
			 int tx_ch, const u8 * mgmt_buf_from_cfg, int mgmt_buf_from_cfg_len)
{
	unsigned char *pbuf;
	unsigned char *frame_after_wlan_hrd = NULL;
	int frame_after_wlan_hrd_len = 0;
	int sizeof_mgmt_wlan_hrd = 0;
	u8 category, action;
#ifdef P2P_SUPPORT
	u8 OUI_Subtype;
	u8 dialogToken;
#endif
#ifdef CONFIG_IEEE80211W
	struct ieee80211_mgmt_hrd *mgmt_frame_hrd = (struct ieee80211_mgmt_hrd *)mgmt_buf_from_cfg;
#endif
	struct stat_info *psta = NULL;
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)mgmt_buf_from_cfg;
	u8 *frame_body = NULL;
	unsigned short packet_type_filter = 0;
#if defined(HS2_SUPPORT) || defined(CONFIG_MBO)
	u8 *dump_buf = NULL;
	int dump_len = 0;
#endif
#if defined(WIFI_WMM)
	int ret = 0;
#endif

	/*put action type condition at top of mgmt*/
	if (ieee80211_is_action(mgmt->frame_control)) {
		packet_type_filter = CFG_MGT_TX_TYPE_ACTION;
	}else
	if (ieee80211_is_mgmt(mgmt->frame_control)) {
		packet_type_filter = CFG_MGT_TX_TYPE_MGT;
	}

	DECLARE_TXINSN(txinsn);

	if (priv->pshare->working_channel2 != tx_ch) {

		NDEBUG2("	chk , our working ch != assigned by cfg\n\n");
		priv->pshare->CurrentChannelBW = HT_CHANNEL_WIDTH_20;
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, tx_ch, priv->pshare->offset_2nd_chan);
	}
	//rtk_set_scan_deny(priv,300);   // deny channel switch for 300 ms
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);

	if (txinsn.phdr == NULL)
		goto fail;

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	if(priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_5G)
		txinsn.tx_rate = _6M_RATE_;
	else
		txinsn.tx_rate = _1M_RATE_;
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif
	txinsn.fixed_rate = true;
	txinsn.need_ack = true;
	txinsn.retry = true;

	psta = get_stainfo(priv, mgmt_frame_hrd->da);
#ifdef CONFIG_IEEE80211W	/* 20180205 */
	if (!psta) {
		PMFDEBUG("not found[%pm]\n",mgmt_frame_hrd->da);
	}else{
		PMFDEBUG("peer[%pm] isPMF[%d]\n",mgmt_frame_hrd->da,psta->isPMF);
	}
	if (psta)
		txinsn.isPMF = psta->isPMF;
	else
		txinsn.isPMF = 0;
#endif

	sizeof_mgmt_wlan_hrd = sizeof(struct ieee80211_mgmt_hrd);
	frame_after_wlan_hrd = (unsigned char *)(mgmt_buf_from_cfg + sizeof_mgmt_wlan_hrd);
	frame_after_wlan_hrd_len = mgmt_buf_from_cfg_len - sizeof_mgmt_wlan_hrd;

	/*-------------parsing	category and action so far just display---------------*/
	category = frame_after_wlan_hrd[0];
	action = frame_after_wlan_hrd[1];

	/*
	if(packet_type_filter == CFG_MGT_TX_TYPE_ACTION){
		NDEBUG2("action frame ; category[%d]; action[%d]\n",category,action);
	}
	*/

	if(packet_type_filter == CFG_MGT_TX_TYPE_MGT){
		if(ieee80211_is_auth(mgmt->frame_control)){
			NDEBUG2("Auth\n");
		}
		else if(ieee80211_is_reassoc_resp(mgmt->frame_control)){
			NDEBUG2("reAssocResp\n");
		}
		else if (ieee80211_is_assoc_resp(mgmt->frame_control)){
			NDEBUG2("AssocResp\n");
		}
		else if(ieee80211_is_probe_resp(mgmt->frame_control)){
			NDEBUG2("ProbeResp\n");
		}
		else if(ieee80211_is_disassoc(mgmt->frame_control)){
			NDEBUG2("DisAssoc\n");
		}
		else if(ieee80211_is_deauth(mgmt->frame_control)){
			NDEBUG2("DeAuth\n");
		}
		else if(ieee80211_is_reassoc_req(mgmt->frame_control)){
			NDEBUG2("ReAssocReq\n");
		}
		else if(ieee80211_is_assoc_req(mgmt->frame_control)){
			NDEBUG2("AssocReq\n");
		}
		else if(ieee80211_is_probe_req(mgmt->frame_control)){
			NDEBUG2("ProbeReq\n");
		}else{
			NDEBUG2("frame control 0x[%02X]\n",mgmt->frame_control);
		}

		if(!ieee80211_is_disassoc(mgmt->frame_control) && !ieee80211_is_deauth(mgmt->frame_control) && !ieee80211_is_action(mgmt->frame_control)) {
			txinsn.isPMF = 0;
		}
		else if(ieee80211_is_action(mgmt->frame_control)) {
			if(CATEGORY_IS_NON_ROBUST(category))
				txinsn.isPMF = 0;
		}
	}
	else if (category == WLAN_ACTION_DLS) {
		switch (action) {
		case _DLS_REQ_ACTION_ID_:
			NDEBUG2("tx DLS_REQ_ACTION_ID_\n");
			break;
		case _DLS_RSP_ACTION_ID_:
			NDEBUG2("tx DLS_RSP_ACTION_ID_\n");
			break;
		default:
			NDEBUG2("unknown action frame[%d]\n", action);
			break;
		}
	}
	else if (category == WLAN_CATEGORY_PUBLIC || category == WLAN_CATEGORY_PROTECTED_DUAL_OF_ACTION) {
		/*all Public Action frame , ID = 4, don't need PMF */
#ifdef CONFIG_IEEE80211W
		if(category != WLAN_CATEGORY_PROTECTED_DUAL_OF_ACTION)
			txinsn.isPMF = 0;
#endif
#if defined(HS2_SUPPORT) || defined(CONFIG_MBO) /* 20180205 */
		switch (action) {
		case WLAN_PA_GAS_INITIAL_REQ:
			NDEBUG2("tx GAS INIT REQ\n");
			break;
		case WLAN_PA_GAS_INITIAL_RESP:
			dump_buf = frame_after_wlan_hrd;
			dump_len = frame_after_wlan_hrd_len;
			gas_init_req_check_frames_tx(priv, &dump_buf, &dump_len);
			if (dump_len) {
				frame_after_wlan_hrd = dump_buf;
				frame_after_wlan_hrd_len = dump_len;
			}
			NDEBUG2("tx GAS INIT RSP\n");
			break;
		case WLAN_PA_GAS_COMEBACK_REQ:
			NDEBUG2("tx GAS COMEBACK REQ\n");
			break;
		case WLAN_PA_GAS_COMEBACK_RESP:
			NDEBUG2("tx GAS COMEBACK RESP\n");
			break;
		default:
			NDEBUG2("unknown action frame[%d]\n", action);
			break;
		}
#endif
#ifdef P2P_SUPPORT
		if (action == _P2P_PUBLIC_ACTION_FIELD_ && !memcmp(frame_after_wlan_hrd + 2, WFA_OUI_PLUS_TYPE, 4)) {
			OUI_Subtype = frame_after_wlan_hrd[6];
			dialogToken = frame_after_wlan_hrd[7];
			switch (OUI_Subtype) {
			case P2P_GO_NEG_REQ:
				NDEBUG2("P2P_GO_NEG_REQ ,dialog[%d]\n", dialogToken);
				break;
			case P2P_GO_NEG_RESP:
				NDEBUG2("P2P_GO_NEG_RESP \n");
				break;
			case P2P_GO_NEG_CONF:
				NDEBUG2("P2P_GO_NEG_CONF\n");
				break;
			case P2P_INVITATION_REQ:
				NDEBUG2("P2P_INVITATION_REQ,dialog[%d]\n", dialogToken);
				break;
			case P2P_INVITATION_RESP:
				NDEBUG2("P2P_INVITATION_RESP,dialog[%d]\n", dialogToken);
				break;
			case P2P_DEV_DISC_REQ:
				NDEBUG2("P2P_INVITATION_RESP,dialog[%d]\n", dialogToken);
				rtk_p2p_set_state(priv, P2P_S_IDLE);
				break;
			case P2P_DEV_DISC_RESP:
				NDEBUG2("P2P_DEV_DISC_RESP,dialog[%d]\n", dialogToken);
				rtk_p2p_set_state(priv, P2P_S_IDLE);
				break;
			case P2P_PROV_DISC_REQ:
				NDEBUG2("P2P_PROV_DISC_REQ,dialog[%d]\n", dialogToken);
				break;
			case P2P_PROV_DISC_RSP:
				NDEBUG2("P2P_PROV_DISC_RSP,dialog[%d]\n", dialogToken);
				break;
			default:
				NDEBUG2("unknown [%d]\n", dialogToken);
				break;
			}
		}
#endif

	}
	else if (category == WLAN_CATEGORY_VENDOR_SPECIFIC) {
		/*0x7F action frame Vendor Spec ID 127(0x7F)*/
#ifdef CONFIG_IEEE80211W
		/*all VENDOR_SPECIFIC Action frame , ID = 127, don't need PMF */
		txinsn.isPMF = 0;
#endif
		switch (action) {
		default:
			NDEBUG2("unknown,action[%d]\n", action);
			break;
		}
#ifdef P2P_SUPPORT
		OUI_Subtype = frame_after_wlan_hrd[5];
		dialogToken = frame_after_wlan_hrd[6];
		switch (OUI_Subtype) {
		case P2P_NOA:
			NDEBUG2("P2P_NOA,dialog[%d]\n", dialogToken);
			break;
		case P2P_PRESENCE_REQ:
			NDEBUG2("P2P_PRESENCE_REQ,dialog[%d]\n", dialogToken);
			break;
		case P2P_PRESENCE_RSP:
			NDEBUG2("P2P_PRESENCE_RSP,dialog[%d]\n", dialogToken);
			break;
		case P2P_GO_DISCOVERY:
			NDEBUG2("P2P_GO_DISCOVERY,dialog[%d]\n", dialogToken);
			break;
		default:
			NDEBUG2("unknown,dialog[%d]\n", dialogToken);
			break;
		}
#endif

	}
	else if (category == WLAN_ACTION_QOS) {
		/* QOS ID 1 */
		switch (action) {
		case _QOS_MAP_CONFIGURE_ID_:
			/*replcae orig issue_QoS_MAP_Configure() here */
			NDEBUG2("TX QoS_MAP_Configure\n");
			break;
		default:
			NDEBUG2("unknown,dialog[%d]\n", action);
			break;
		}
	}
/*HAPD__IEEE80211R*/
#ifdef CONFIG_IEEE80211R 
	else if (category == WLAN_ACTION_FT) {
		/* Fast BSS trans ID 6 */
		switch (action) {
		case WLAN_FT_ACTION_REQ:
			FTDEBUG("TX FT-Action Req");
			break;
		case WLAN_FT_ACTION_RESP:
			FTDEBUG("TX FT-Action Resp");
			break;
		case WLAN_FT_ACTION_CONFIRM:
			FTDEBUG("TX FT-Action Confirm");
			break;
		case WLAN_FT_ACTION_ACK:
			FTDEBUG("TX FT-Action Ack");
			break;
		default:
			FTDEBUG("unknown,dialog[%d]\n", action);
			break;
		}
		FTDEBUG("to peer[%pm]",mgmt_frame_hrd->da);
	}
#endif
	else if (category == WLAN_ACTION_UNPROTECTED_WNM) {
#ifdef CONFIG_IEEE80211W
		/*all UNPROTECTED_WNM Action frame , ID = 11, don't need PMF */
		txinsn.isPMF = 0;
#endif
		NDEBUG2("WLAN_ACTION_UNPROTECTED_WNM , action[%d]\n",action);

	} else if (category == WLAN_ACTION_WNM) {
		/* WNM ID 10 */
		switch (action) {
		case WNM_NOTIFICATION_REQ:
			/* action type 26 */
			NDEBUG2("WNM_NOTIFICATION_REQ\n");
			if (frame_after_wlan_hrd[9] == 0) {
				NDEBUG2("TX WNM NOTIFICATION REQ\n");
			} else if (frame_after_wlan_hrd[9] == 1) {
				//issue_WNM_Deauth_Req
				NDEBUG2("TX WNM Deauth Req\n");
			}
			break;
		case WNM_NOTIFICATION_RESP:
			/*type 27 */
			NDEBUG2("WNM_NOTIFICATION_RESP\n");
			break;
		case WNM_BSS_TRANS_MGMT_REQ:
			/*type 7 */
			/*replace issue_BSS_TSM_req in hostapd case */
			NDEBUG2("WNM_BSS_TRANS_MGMT_REQ\n");
			break;
		case WNM_BSS_TRANS_MGMT_RESP:
			/*type 8 */
			NDEBUG2("WNM_BSS_TRANS_MGMT_RESP\n");
			break;
		default:
			NDEBUG2("action[%d]\n", action);
			break;
		}
	} 
	else if(category == WLAN_CATEGORY_RADIO_MEASUREMENT){
		/* 11K category 5 */
		switch (action) {
		case WLAN_RRM_RADIO_MEASUREMENT_REQUEST:
				NDEBUG2("RRM_RADIO_MEASUREMENT_REQUEST\n");
#ifdef HAPD_11K
				hapd_rm_check_mgmt_tx(priv, mgmt_buf_from_cfg, mgmt_buf_from_cfg_len);
#endif
			break;
		case WLAN_RRM_RADIO_MEASUREMENT_REPORT:
				NDEBUG2("RRM_RADIO_MEASUREMENT_REPORT\n");
			break;
		case WLAN_RRM_LINK_MEASUREMENT_REQUEST:
				NDEBUG2("RRM_LINK_MEASUREMENT_REQUEST\n");
			break;
		case WLAN_RRM_LINK_MEASUREMENT_REPORT:
				NDEBUG2("RRM_LINK_MEASUREMENT_REPORT\n");
			break;
		case WLAN_RRM_NEIGHBOR_REPORT_REQUEST:
				NDEBUG2("RRM_NEIGHBOR_REPORT_REQUEST\n");
			break;
		case WLAN_RRM_NEIGHBOR_REPORT_RESPONSE:
				frame_body = (u8 *)mgmt_buf_from_cfg + sizeof(struct ieee80211_mgmt_hrd);
				//unsigned char dialog_token = frame_body[2];
				//unsigned char tag_number = frame_body[3];
				frame_body[16] = priv->pmib->dot11RFEntry.dot11channel;
				NDEBUG2("RRM_NEIGHBOR_REPORT_RESPONSE\n");
			break;
		default:
			NDEBUG2("unknown,dialog[%d]\n", action);
			break;
		}
	}
	else if(category == WLAN_ACTION_SA_QUERY){
		PMFDEBUG("SA_QUERY\n");
	}
	else {
		NDEBUG2("unknown category=[%d]\n", category);
	}
	/*-------------parsing	category and action so far just display---------------*/

	/*clean && fill wlan head */
	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));
	memcpy((void *)(txinsn.phdr), mgmt_buf_from_cfg, sizeof(struct ieee80211_mgmt_hrd));

#ifdef CONFIG_IEEE80211W
	if (txinsn.isPMF)
		*(unsigned char *)(txinsn.phdr + 1) |= BIT(6);	// enable privacy
#endif

	/*fill frame content */
	memcpy((void *)pbuf, frame_after_wlan_hrd, frame_after_wlan_hrd_len);

#if defined(HS2_SUPPORT) || defined(CONFIG_MBO)
	if (dump_len != 0) {
		kfree(dump_buf);
		dump_len = 0;
	}
#endif

	txinsn.fr_len += frame_after_wlan_hrd_len;

#if defined(WIFI_WMM)
	if (psta)
		ret = check_dz_mgmt(priv, psta, &txinsn);
	if (ret < 0)
		goto fail;
	else if (ret==1)
		return SUCCESS;
	else
#endif
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS) {
		return SUCCESS;
	} else {
		NDEBUG2("TX action fail\n");
	}

fail:
	NDEBUG2("fail !!!\n");
	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
#if defined(HS2_SUPPORT) || defined(CONFIG_MBO)
	if (dump_len != 0) {
		kfree(dump_buf);
		dump_len = 0;
	}
#endif
	return FAIL;

}

static int realtek_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
           struct cfg80211_mgmt_tx_params *params,
           u64 *cookie)
{
	struct rtknl *rtk = wiphy_priv(wiphy);
	struct rtl8192cd_priv *priv = get_priv_from_wdev(rtk, wdev);
	struct ieee80211_mgmt *mgmt;
	struct stat_info *pstat = NULL;
	u8 *pbuf;
	u16 status = 0;
	int limit;
	WPA_STA_INFO *sta_info;
	unsigned short packet_type_filter=0;
	int ie_len=0;
	u8 *ptr_tmp=NULL;
	int ret = 0;
	unsigned short tx_ch=0;
	#ifdef HAPD_OWE
	u8 *pmkid_ptr = NULL;
	u8	pmkid_cnt = 0;
	#endif

	NLENTER;
	if(params==NULL){
		NDEBUG2("param is NULL CHK!!\n");
		return -EPERM;
	}
	if(params->chan && (ieee80211_frequency_to_channel(params->chan->center_freq)
				== priv->pmib->dot11RFEntry.dot11channel)){

		tx_ch = ieee80211_frequency_to_channel(params->chan->center_freq);
		//NDEBUG2("tx_ch [%d]\n",tx_ch);
	}else{
		NDEBUG2("use current channel[%d]\n",priv->pmib->dot11RFEntry.dot11channel);
		tx_ch = priv->pmib->dot11RFEntry.dot11channel;
		//return -EPERM;
	}

	*cookie = priv->mgmt_tx_cookie++;
	mgmt = (struct ieee80211_mgmt *)params->buf;

	//NDEBUG2("frame control=0x[%x]\n", le16_to_cpu(mgmt->frame_control));

	if(ieee80211_is_reassoc_resp(mgmt->frame_control) || ieee80211_is_assoc_resp(mgmt->frame_control)){
		/*assoc resp/reassoc resp don't call back to hapd,
		it will trigger hadp 4-1 again(twice) cause 4-ways fail*/
		//log2("don't call back");
	}else{
		cfg80211_mgmt_tx_status(wdev, *cookie, params->buf, params->len, TRUE, GFP_ATOMIC);//GFP_ATOMIC
	}

	/*put action type condition at top of mgmt*/
	if (ieee80211_is_action(mgmt->frame_control)) {
		packet_type_filter = CFG_MGT_TX_TYPE_ACTION;
	}else
	if (ieee80211_is_mgmt(mgmt->frame_control)) {
		packet_type_filter = CFG_MGT_TX_TYPE_MGT;
	}else{
		NDEBUG("not support mgt_tx fc_type[%x]\n",le16_to_cpu(mgmt->frame_control));
		return -EPERM;
	}


	pstat = get_stainfo(priv, mgmt->da);
	if (!pstat) {

		if (ieee80211_is_probe_resp(mgmt->frame_control)){
			;/*go on*/
		}else if (packet_type_filter == CFG_MGT_TX_TYPE_ACTION && \
			mgmt->u.action.category == WLAN_ACTION_PUBLIC) {
			;/*go on*/
		}else
		{
			NDEBUG("sta = NULL\n");
			return -EPERM;
		}
	}
	if(packet_type_filter==CFG_MGT_TX_TYPE_MGT){
		if (ieee80211_is_auth(mgmt->frame_control)) {
#ifdef CONFIG_IEEE80211R
			if (pstat->ft_state && le16_to_cpu(mgmt->u.auth.auth_alg) == _AUTH_ALGM_FT_) {
				FTDEBUG("tx ft-auth-resp; ft_state[%d]",pstat->ft_state);
	
				sta_info = pstat->wpa_sta_info;
	
				limit = params->len - (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.auth));
				pbuf = (u8 *) (params->buf + (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.auth)));
				set_ft_ie(sta_info, pbuf, limit);
	
				pstat->auth_seq = le16_to_cpu(mgmt->u.auth.auth_transaction);
				status = le16_to_cpu(mgmt->u.auth.status_code);
				issue_auth_resp(priv, pstat, status);
			}
#endif /* CONFIG_IEEE80211R */

			if(le16_to_cpu(mgmt->u.auth.auth_alg) == _AUTH_ALGM_SAE_){
				if(OPMODE & WIFI_AP_STATE){
					sta_info = pstat->wpa_sta_info;
					pstat->auth_seq = le16_to_cpu(mgmt->u.auth.auth_transaction);
					status = le16_to_cpu(mgmt->u.auth.status_code);
					limit = params->len - (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.auth));
					pbuf = (u8 *)(params->buf + (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.auth)));
					memcpy(sta_info->sae_ie, pbuf, limit);
					sta_info->sae_ie_len = limit;
					pstat->is_sae_sta = 1;
					hapdlog("auth_resp from hapd");
					issue_auth_resp(priv, pstat, status);
				}else if(OPMODE & WIFI_STATION_STATE){
					panic_printk("\n");
					wpaslog("STA mode handle AUTH from WPAS");
					ret = rtw_sae_preprocess(priv,params->buf,params->len,TRUE);
					wpaslog("ret=[%d]",ret);
					rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
				}else{

				}
			}
		}
		else if (ieee80211_is_reassoc_resp(mgmt->frame_control)) {
				if(params->len < (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.reassoc_resp))){
					//dump_hex("assoc_resp from hapd",params->buf,params->len);
					NDEBUG2("CHK!!!");
				}
				/*enabled here for [all assoc_resp_reply_by_hostapd 20210225]*/
				pbuf = (u8 *)params->buf + (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.reassoc_resp));
				limit = params->len - (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.reassoc_resp));
				#ifdef CONFIG_IEEE80211R
				if (pstat->ft_state == state_ft_assoc) {
					FTDEBUG("set_ft_ie");
					sta_info = pstat->wpa_sta_info;
					set_ft_ie(sta_info, pbuf, limit);
				}
				#endif
#ifdef CONFIG_IEEE80211W
				ptr_tmp = get_ie(pbuf, EID_TIMEOUT_INTERVAL, &ie_len, limit);
				if (ptr_tmp) {
					/*assoc_resp from hapd include 11W timeout send here*/
					log2("[11w timeout]");
					memcpy(pstat->pmf_timeout_ie,ptr_tmp,ie_len+2);
					pstat->pmf_timeout_ie_len=ie_len+2;
				}
#endif
#ifdef HAPD_OWE
				ptr_tmp = get_ie(pbuf, _OWE_DH_IE_, &ie_len, limit);
				if (ptr_tmp && ptr_tmp[2]==0x20) {
					/*assoc_resp from hapd include OWE DH group IE*/
					owelog("owe_dh_ie");
					memcpy(pstat->owe_dh_ie,ptr_tmp,OWE_DH_IE_LEN);
					pstat->owe_dh_ie_len = OWE_DH_IE_LEN;
				}
				ptr_tmp = get_ie(pbuf, _RSN_IE_2_, &ie_len, limit);
				if (ptr_tmp) {
					/*assoc_resp inlcude RSN means it include PMKID send here*/
					ret = wpa2_ie_check(priv ,ptr_tmp,ptr_tmp[1]+2, &pmkid_ptr, &pmkid_cnt);
					if(ret)
						owelog("ret=[%d]",ret);
					memcpy(pstat->owe_rsn_resp,ptr_tmp,ptr_tmp[1]+2);
					pstat->owe_rsn_resp_len = ptr_tmp[1]+2;

					if(pmkid_cnt){
						owelog("pmkid_cnt[%d",pmkid_cnt);
						owelog("clean owe_dh");
						pstat->owe_dh_ie_len = 0;
					}
				}
#endif
				status = le16_to_cpu(mgmt->u.reassoc_resp.status_code);
				log2("re_assoc_resp,status=%d",status);
				#ifdef REPLY_ASSOC_BY_HAPD
				issue_asocrsp(priv, status, pstat, WIFI_REASSOCRSP,0);
				#endif

		}
		else if (ieee80211_is_assoc_resp(mgmt->frame_control)) {
			sta_info = pstat->wpa_sta_info;
			if(params->len < (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.assoc_resp))){
				//dump_hex("assoc_resp from hapd",params->buf,params->len);
				NDEBUG2("CHK!!!");
			}
			pbuf = (u8 *)params->buf + (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.assoc_resp));
			limit = params->len - (ieee80211_hdrlen(mgmt->frame_control) + sizeof(mgmt->u.assoc_resp));
			#ifdef CONFIG_IEEE80211R
			if (pstat->ft_state == state_imd_assoc) {
				FTDEBUG("set_ft_ie");
				set_ft_ie(sta_info, pbuf, limit);
			}
			#endif
#ifdef CONFIG_IEEE80211W
			ptr_tmp = get_ie(pbuf, EID_TIMEOUT_INTERVAL, &ie_len, limit);
			if (ptr_tmp) {
				/*assoc_resp from hapd include 11W timeout send here*/
				log2("[11w timeout]");
				memcpy(pstat->pmf_timeout_ie,ptr_tmp,ie_len+2);
				pstat->pmf_timeout_ie_len=ie_len+2;
			}
			#endif
#ifdef HAPD_OWE
			ptr_tmp = get_ie(pbuf, _OWE_DH_IE_, &ie_len, limit);
			if (ptr_tmp && ptr_tmp[2]==0x20) {
				/*assoc_resp from hapd include OWE DH group IE*/
				owelog("owe_dh_ie");
				memcpy(pstat->owe_dh_ie,ptr_tmp,OWE_DH_IE_LEN);
				pstat->owe_dh_ie_len = OWE_DH_IE_LEN;
			}
			ptr_tmp = get_ie(pbuf, _RSN_IE_2_, &ie_len, limit);
			if (ptr_tmp) {
				/*assoc_resp inlcude RSN means it include PMKID send here*/
				ret = wpa2_ie_check(priv ,ptr_tmp,ptr_tmp[1]+2, &pmkid_ptr, &pmkid_cnt);
				if(ret)
					owelog("ret=[%d]",ret);
				memcpy(pstat->owe_rsn_resp,ptr_tmp,ptr_tmp[1]+2);
				pstat->owe_rsn_resp_len = ptr_tmp[1]+2;

				if(pmkid_cnt){
					owelog("pmkid_cnt[%d",pmkid_cnt);
					owelog("clean owe_dh");
					pstat->owe_dh_ie_len = 0;
				}
			}
#endif
			status = le16_to_cpu(mgmt->u.reassoc_resp.status_code);
			#ifdef REPLY_ASSOC_BY_HAPD
			log2("assoc_resp,status=%d",status);
			issue_asocrsp(priv, status, pstat, pstat->assoc_type,0);
			#endif
		}
		else
		if (ieee80211_is_probe_resp(mgmt->frame_control)) {
			//NDEBUG("probe_resp");
			//dump_hex("probe_resp",params->buf, params->len);
			//rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
		}else{
			NDEBUG("mgt type 0x[%x]no support yet\n",cpu_to_le16(mgmt->frame_control));
		}
	}
	else if(packet_type_filter==CFG_MGT_TX_TYPE_ACTION){
#ifdef CONFIG_IEEE80211R
		//old define => _FAST_BSS_TRANSITION_CATEGORY_ID_
		if (mgmt->u.action.category == WLAN_ACTION_FT) {
			/* Standly(SD9)
			action_ie = params->buf + ieee80211_hdrlen(mgmt->frame_control);
			action_ie_len = params->len - ieee80211_hdrlen(mgmt->frame_control);
			FTDEBUG("issue ft-action packet\n");
			issue_ft_action(priv, pstat, action_ie, action_ie_len);
			*/
			rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
		}else
#endif
#ifdef HAPD_11K
		if (mgmt->u.action.category == WLAN_ACTION_RADIO_MEASUREMENT) {
			NDEBUG2("WLAN_ACTION_RADIO_MEASUREMENT\n");
			rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
		}else
#endif
#ifdef	CONFIG_IEEE80211W
		if (mgmt->u.action.category == WLAN_ACTION_SA_QUERY) {
			log2("SA_QUERY from hapd");
			rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
		}else
#endif
		if (mgmt->u.action.category == WLAN_ACTION_WNM) {
			NDEBUG2("WLAN_ACTION_WNM\n");
			rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
		}else
		if (mgmt->u.action.category == WLAN_ACTION_PUBLIC || 
			mgmt->u.action.category == WLAN_CATEGORY_PROTECTED_DUAL_OF_ACTION) {
			NDEBUG2("WLAN_ACTION_PUBLIC\n");
#if defined(HAPD_DPP) || defined(REPLY_HS2_BY_HAPD)
			switch (mgmt->u.action.u.chan_switch.action_code) {
				case _DPP_ACTION_FIELD_:
				case _GAS_INIT_REQ_ACTION_ID_:
				case _GAS_INIT_RSP_ACTION_ID_:
				case _GAS_COMBACK_REQ_ACTION_ID_:
				case _GAS_COMBACK_RSP_ACTION_ID_:
					rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
					break;
				default:
					break;
			}
#endif
#ifdef REPLY_HS2_BY_HAPD
		}else
		if (mgmt->u.action.category == WLAN_ACTION_DLS) {
			switch (mgmt->u.action.u.chan_switch.action_code) {
			case _DLS_REQ_ACTION_ID_:
			case _DLS_RSP_ACTION_ID_:
				rtk_cfg80211_mgmt_tx(priv,tx_ch,params->buf, params->len);
				break;
			default:
				break;
			}
#endif
		}else{
			NDEBUG2("action type 0x[%x] not support yet\n",mgmt->u.action.category);
		}
	}
	else {
		NDEBUG2("not support frame control=0x%x\n", le16_to_cpu(mgmt->frame_control));
		return -EPERM;
	}

	return 0;
}
int realtek_cfg80211_external_auth(struct wiphy *wiphy, struct net_device *dev,
	struct cfg80211_external_auth_params *params)
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, dev);
	/*when interfaces has root,va0,va1...etc, hapd will call this func by root interface
	even peer(STA) connect with va0*/
	if((OPMODE&WIFI_AP_STATE)){
		/*do not thing*/
		return 0;
	}
	//wpaslog("action type[%d]",params->action);
	if (params->status != WLAN_STATUS_SUCCESS){
		wpaslog("SAE fail");
		return 0;
	}
	if (params->status == WLAN_STATUS_SUCCESS) {
		panic_printk("SAE Success");
		wpaslog("bssid:[%pm]",params->bssid);
		wpaslog("ssid:[%s]",((params->ssid.ssid_len == 0) ? "" : (char *)params->ssid.ssid));
		//wpaslog("suite: 0x%08x", params->key_mgmt_suite);
		//dump_hex("pmkid", params->pmkid,LEN_PMKID);
	}
	return 0;
}


static int realtek_cfg80211_set_pmksa(struct wiphy *wiphy,
	struct net_device *ndev,
	struct cfg80211_pmksa *pmksa)
{
	struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, ndev);
	wpaslog("start assoc...\n");
	start_clnt_assoc(priv);
	return 0;
}


static int realtek_cfg80211_del_pmksa(struct wiphy *wiphy,
				struct net_device *ndev,
				struct cfg80211_pmksa *pmksa)
{
	//struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, ndev);
	wpaslog("bssid[%pm]",pmksa->bssid);
	return 0;
}

static int realtek_cfg80211_flush_pmksa(struct wiphy *wiphy,
					struct net_device *ndev)
{
	//struct rtl8192cd_priv *priv = realtek_get_priv(wiphy, ndev);
	wpaslog("\n");
	return 0;
}

int realtek_cfg80211_get_channel(struct wiphy *wiphy,
			       struct wireless_dev *wdev,
			       struct cfg80211_chan_def *chandef)
{
	struct rtknl *rtk = NULL;
	struct rtl8192cd_priv *priv = NULL;
	unsigned int dot11nUse40M;
	unsigned int dot11channel;
	unsigned int dot11n2ndChOffset;
	unsigned int centerchannel;
	int freq;
	rtk = wiphy_priv(wiphy);

	if(rtk==NULL){
		NDEBUG2("chk!\n");
		return -EPERM;
	}
	priv = get_priv_from_wdev(rtk, wdev);
	if(priv==NULL){
		NDEBUG2("chk!\n");
		return -EPERM;
	}

	dot11nUse40M = priv->pmib->dot11nConfigEntry.dot11nUse40M;
	dot11channel = priv->pmib->dot11RFEntry.dot11channel;
	dot11n2ndChOffset = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
	centerchannel = get_center_channel(priv, dot11channel, dot11n2ndChOffset, 0);
	

	if (!IS_DRV_OPEN(priv))
		return -EPERM;

	freq = ieee80211_channel_to_frequency(dot11channel, (dot11channel < 14) ? IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ);;
	chandef->chan = ieee80211_get_channel(wiphy, freq);
	chandef->center_freq1 = ieee80211_channel_to_frequency(centerchannel, (centerchannel < 14) ? IEEE80211_BAND_2GHZ : IEEE80211_BAND_5GHZ);
	chandef->center_freq2 = 0;

#ifdef CONFIG_OPENWRT_SDK
	switch (dot11nUse40M) {
	case 0:
		chandef->width = NL80211_CHAN_WIDTH_20;
		break;
	case 1:
		chandef->width = NL80211_CHAN_WIDTH_40;
		break;
	case 2:
		chandef->width = NL80211_CHAN_WIDTH_80;
		break;
	case 3:
		chandef->width = NL80211_CHAN_WIDTH_160;
		break;
	case 4:
		chandef->width = NL80211_CHAN_WIDTH_10;
		break;
	case 5:
		chandef->width = NL80211_CHAN_WIDTH_5;
		break;
	}
#else
	switch (dot11nUse40M) {
	case HT_CHANNEL_WIDTH_20:
		chandef->width = NL80211_CHAN_WIDTH_20;
		break;
	case HT_CHANNEL_WIDTH_20_40:
		chandef->width = NL80211_CHAN_WIDTH_40;
		break;
	case HT_CHANNEL_WIDTH_80:
		chandef->width = NL80211_CHAN_WIDTH_80;
		break;
	case HT_CHANNEL_WIDTH_160:
		chandef->width = NL80211_CHAN_WIDTH_160;
		break;
	case HT_CHANNEL_WIDTH_10:
		chandef->width = NL80211_CHAN_WIDTH_10;
		break;
	case HT_CHANNEL_WIDTH_5:
		chandef->width = NL80211_CHAN_WIDTH_5;
		break;
	}
#endif

	if (!chandef->chan || !cfg80211_chandef_valid(chandef))
		return -EINVAL;

	return 0;
}

static struct ieee80211_channel realtek_2ghz_channels[] = {
  CHAN2G(1, 2412, IEEE80211_CHAN_NO_HT40MINUS),
  CHAN2G(2, 2417, IEEE80211_CHAN_NO_HT40MINUS),
  CHAN2G(3, 2422, IEEE80211_CHAN_NO_HT40MINUS),
  CHAN2G(4, 2427, IEEE80211_CHAN_NO_HT40MINUS),
  CHAN2G(5, 2432, 0),
  CHAN2G(6, 2437, 0),
  CHAN2G(7, 2442, 0),
  CHAN2G(8, 2447, 0),
  CHAN2G(9, 2452, 0),
  CHAN2G(10, 2457, IEEE80211_CHAN_NO_HT40PLUS),
  CHAN2G(11, 2462, IEEE80211_CHAN_NO_HT40PLUS),
  CHAN2G(12, 2467, IEEE80211_CHAN_NO_HT40PLUS),
  CHAN2G(13, 2472, IEEE80211_CHAN_NO_HT40PLUS),
  CHAN2G(14, 2484, IEEE80211_CHAN_NO_HT40PLUS | IEEE80211_CHAN_NO_HT40MINUS | IEEE80211_CHAN_NO_OFDM)
};

static struct ieee80211_rate realtek_rates[] = {
	RATETAB_ENT(10, 0x1, 0),
	RATETAB_ENT(20, 0x2, 0),
	RATETAB_ENT(55, 0x4, 0),
	RATETAB_ENT(110, 0x8, 0),
	RATETAB_ENT(60, 0x10, 0),
	RATETAB_ENT(90, 0x20, 0),
	RATETAB_ENT(120, 0x40, 0),
	RATETAB_ENT(180, 0x80, 0),
	RATETAB_ENT(240, 0x100, 0),
	RATETAB_ENT(360, 0x200, 0),
	RATETAB_ENT(480, 0x400, 0),
	RATETAB_ENT(540, 0x800, 0),
};

static const struct ieee80211_regdomain *_rtl_regdomain_select(
					   unsigned int dot11RegDomain)
{
   switch (dot11RegDomain) {
   case DOMAIN_FCC:
   case DOMAIN_NCC:
	   return &rtl_world_regdom_1;
   case DOMAIN_ETSI:
	   return &rtl_world_regdom_3;
   case DOMAIN_MKK:
	   return &rtl_world_regdom_6;
   case DOMAIN_ISRAEL:
	   return &rtl_world_regdom_7;
   case DOMAIN_RUSSIAN:
	   return &rtl_world_regdom_12;
   case DOMAIN_CN:
	   return &rtl_world_regdom_13;
   case DOMAIN_TEST:
	   return &rtl_world_regdom_all;
   default:
	   return &rtl_regdom_no_midband;
   }
}

static struct ieee80211_supported_band realtek_band_2ghz = {
   .band = NL80211_BAND_2GHZ,
   .n_channels = ARRAY_SIZE(realtek_2ghz_channels),
   .channels = realtek_2ghz_channels,
   .n_bitrates = realtek_g_rates_size,
   .bitrates = realtek_g_rates,
   .ht_cap = {
	   .cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40 | 
			   IEEE80211_HT_CAP_TX_STBC | IEEE80211_HT_CAP_RX_STBC,
	   .ht_supported = true,
	   .ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
	   .ampdu_density = 7,
	   .mcs = {
		   .rx_mask = {0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0},
		   .rx_highest = cpu_to_le16(300),
		   .tx_params = IEEE80211_HT_MCS_TX_DEFINED
	   }
   }
};

static struct ieee80211_supported_band realtek_band_5ghz = {
	.band = NL80211_BAND_5GHZ,
	.n_channels = ARRAY_SIZE(realtek_5ghz_a_channels),
	.channels = realtek_5ghz_a_channels,
	.n_bitrates = realtek_a_rates_size,
	.bitrates = realtek_a_rates,
	.ht_cap = {
		.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40 |
				IEEE80211_HT_CAP_TX_STBC | IEEE80211_HT_CAP_RX_STBC,
		.ht_supported = true,
		.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		.ampdu_density = 7,
		.mcs = {
			.rx_mask = {0xff, 0xff, 0, 0, 0, 0, 0, 0, 0, 0},
			.rx_highest = cpu_to_le16(300),
			.tx_params = IEEE80211_HT_MCS_TX_DEFINED
		}
	},
	.vht_cap = {
		.vht_supported = true,
		.cap = IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_7991 | IEEE80211_VHT_CAP_SHORT_GI_80 | 
				IEEE80211_VHT_CAP_TXSTBC | IEEE80211_VHT_CAP_RXSTBC_1 |
#if (BEAMFORMING_SUPPORT == 1)
				IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE | IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE |
				(1 << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT) | (1 << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT) |
#endif
				IEEE80211_VHT_CAP_HTC_VHT | (7 << IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_SHIFT), 
		.vht_mcs = {
			.rx_mcs_map = cpu_to_le16(0xfffa),
			.tx_mcs_map = cpu_to_le16(0xfffa),
			.rx_highest = cpu_to_le16(780),
			.tx_highest = cpu_to_le16(780),
		}
	}
};

void rtk_reg_notifier(
	struct wiphy *wiphy,struct regulatory_request *request)
{
	struct rtknl *rtk = wiphy_priv(wiphy); //return &wiphy->priv;
	struct rtl8192cd_priv   *priv = rtk->priv;
	struct ieee80211_regdomain *realtek_regdom=NULL;

	NDEBUG2("country code [%s]",request->alpha2);
	strcpy(priv->pmib->dot11dCountry.dot11CountryString, request->alpha2);
#if (defined(DOT11D) || defined(DOT11H) || defined(DOT11K))
	map_country_to_reg(GET_ROOT(priv));
#endif
	//wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
	wiphy->regulatory_flags = REGULATORY_WIPHY_SELF_MANAGED;
	realtek_regdom = (struct ieee80211_regdomain *)_rtl_regdomain_select(GET_ROOT(priv)->pmib->dot11StationConfigEntry.dot11RegDomain);
	//wiphy_apply_custom_regulatory(wiphy, realtek_regdom);
	if (rtnl_is_locked())
		regulatory_set_wiphy_regd_sync_rtnl(wiphy, realtek_regdom);
	else {
		rtnl_lock();
		regulatory_set_wiphy_regd_sync_rtnl(wiphy, realtek_regdom);
		rtnl_unlock();
	}
}

struct cfg80211_ops realtek_cfg80211_ops = {
	.add_virtual_intf = realtek_cfg80211_add_iface,
	.del_virtual_intf = realtek_cfg80211_del_iface,
	.change_virtual_intf = realtek_cfg80211_change_iface,
	.add_key = realtek_cfg80211_add_key,
	.del_key = realtek_cfg80211_del_key,
	.get_key = realtek_cfg80211_get_key,
	.set_default_key = realtek_cfg80211_set_default_key,
	.set_default_mgmt_key = realtek_cfg80211_set_default_mgmt_key,
	//.add_beacon = realtek_cfg80211_add_beacon,
	//.set_beacon = realtek_cfg80211_set_beacon,
	//.del_beacon = realtek_cfg80211_del_beacon,
	.add_station = realtek_cfg80211_add_station,
	.del_station = realtek_cfg80211_del_station,
	.change_station = realtek_cfg80211_change_station,
	.get_station = realtek_cfg80211_get_station,
	.dump_station = realtek_cfg80211_dump_station,
#if 0//def CONFIG_MAC80211_MESH
		.add_mpath = realtek_cfg80211_add_mpath,
		.del_mpath = realtek_cfg80211_del_mpath,
		.change_mpath = realtek_cfg80211_change_mpath,
		.get_mpath = realtek_cfg80211_get_mpath,
		.dump_mpath = realtek_cfg80211_dump_mpath,
		.set_mesh_params = realtek_cfg80211_set_mesh_params,
		.get_mesh_params = realtek_cfg80211_get_mesh_params,
#endif
	.change_bss = realtek_cfg80211_change_bss,
	//.set_txq_params = realtek_cfg80211_set_txq_params,
	//.set_channel = realtek_cfg80211_set_channel,
	.suspend = realtek_cfg80211_suspend,
	.resume = realtek_cfg80211_resume,
	.scan = realtek_cfg80211_scan,
#if 0
	.auth = realtek_cfg80211_auth,
	.assoc = realtek_cfg80211_assoc,
	.deauth = realtek_cfg80211_deauth,
	.disassoc = realtek_cfg80211_disassoc,
#endif
	.join_ibss = realtek_cfg80211_join_ibss,
	.leave_ibss = realtek_cfg80211_leave_ibss,
	.set_wiphy_params = realtek_cfg80211_set_wiphy_params,
	.set_ap_chanwidth = realtek_cfg80211_set_ap_chanwidth,
	.set_monitor_channel = realtek_cfg80211_set_monitor_channel,
	.set_tx_power = realtek_cfg80211_set_tx_power,
	.get_tx_power = realtek_cfg80211_get_tx_power,
	.set_power_mgmt = realtek_cfg80211_set_power_mgmt,
	.set_wds_peer = realtek_cfg80211_set_wds_peer,
	.rfkill_poll = realtek_cfg80211_rfkill_poll,
	//CFG80211_TESTMODE_CMD(ieee80211_testmode_cmd)
	.set_bitrate_mask = realtek_cfg80211_set_bitrate_mask,
	.connect = realtek_cfg80211_connect,
	.disconnect = realtek_cfg80211_disconnect,
	.remain_on_channel = realtek_remain_on_channel,
	.cancel_remain_on_channel = realtek_cancel_remain_on_channel,
	.mgmt_tx = realtek_mgmt_tx,
	.mgmt_frame_register = realtek_mgmt_frame_register,
	.channel_switch = realtek_cfg80211_channel_switch,
	.dump_survey = realtek_dump_survey,//survey_dump
	.start_ap = realtek_start_ap,
	.change_beacon = realtek_change_beacon,
	.stop_ap = realtek_stop_ap,
#if 0
	.sched_scan_start = realtek_cfg80211_sscan_start,
	.sched_scan_stop = realtek_cfg80211_sscan_stop,
	.set_bitrate_mask = realtek_cfg80211_set_bitrate,
	.set_cqm_txe_config = realtek_cfg80211_set_txe_config,
#endif
	.set_mac_acl = realtek_set_mac_acl,
#if defined(DFS)
	.start_radar_detection = realtek_start_radar_detection,
#endif
	.get_channel = realtek_cfg80211_get_channel,
	.update_ft_ies = realtek_update_ft_ies,
	.external_auth = realtek_cfg80211_external_auth,
	.set_pmksa = realtek_cfg80211_set_pmksa,
	.del_pmksa = realtek_cfg80211_del_pmksa,
	.flush_pmksa = realtek_cfg80211_flush_pmksa,
};

#ifdef RTK_129X_PLATFORM
static void  rtk_create_dev(struct rtknl *rtk, struct device *dev, int idx)
#else
static void  rtk_create_dev(struct rtknl *rtk,int idx)
#endif
{
	/* define class here */
#ifdef RTK_129X_PLATFORM
	rtk->dev = dev;
#else
    rtk->cl = class_create(THIS_MODULE, rtk_dev_name[idx]);

    /* create first device */
    rtk->dev = device_create(rtk->cl, NULL, rtk_wifi_dev[idx], NULL, rtk_dev_name[idx]);

 	dev_set_name(rtk->dev, rtk_dev_name[idx]);
  	printk("Device Name = %s \n", dev_name(rtk->dev));
#endif
	printk("VIF_NUM=%d\n", VIF_NUM);
	memset(rtk->ndev_name, 0, VIF_NUM*VIF_NAME_SIZE);

	//init rtk phy root name
	#if (defined(CONFIG_WLAN_HAL_8192FE)|| defined(CONFIG_WLAN_HAL_8197G)) && defined(CONFIG_AX1500)
	#ifdef CONFIG_2G_ON_WLAN0
	snprintf(rtk->root_ifname, sizeof(rtk->root_ifname), "wlan0");
	#else
	snprintf(rtk->root_ifname, sizeof(rtk->root_ifname), "wlan1");
	#endif
	#else
	snprintf(rtk->root_ifname, sizeof(rtk->root_ifname), "wlan%d", idx);
	#endif
	//mark_dual ,init with fake mac for diff phy
	rtk_fake_addr[3] += ((unsigned char)idx) ;
	memcpy(rtk->root_mac, rtk_fake_addr, ETH_ALEN);
}

#if	defined(CONFIG_WIRELESS_LAN_MODULE) || defined(RTK_129X_PLATFORM)
void rtk_unregister_wiphy(struct rtknl *rtk)
{
	if(rtk->wiphy_registered){
		wiphy_unregister(rtk->wiphy);
		rtk->wiphy_registered = false;
	}
}

void rtk_free_wiphy(struct rtknl *rtk)
{
	rtk_phy_idx--;

	NDEBUG2("free wiphy[%d]\n", rtk_phy_idx);

	rtk_phy[rtk_phy_idx] = NULL;

	if(rtk->wiphy) {
		wiphy_free(rtk->wiphy);
		rtk->wiphy = NULL;
	}
}
#endif

void rtk_remove_dev(struct rtknl *rtk,int idx)
{
	NDEBUG2("remove cfg80211 device[%d]\n", idx);
	//remove device
	if(rtk->dev)
	{
#if !defined(RTK_129X_PLATFORM)
		device_destroy(rtk->cl, rtk_wifi_dev[idx]);
#endif
		rtk->dev = NULL;
	}

	//remove class
	if(rtk->cl)
	{
		class_destroy(rtk->cl);
		rtk->cl = NULL;
	}
}


//struct rtknl *realtek_cfg80211_create(struct rtl8192cd_priv *priv)
#ifdef RTK_129X_PLATFORM
struct rtknl *realtek_cfg80211_create(struct device *dev)
#else
struct rtknl *realtek_cfg80211_create(void)
#endif
{
	struct wiphy *wiphy;
	struct rtknl *rtk;

	//NLENTER;

	/* create a new wiphy for use with cfg80211 */
	wiphy = wiphy_new(&realtek_cfg80211_ops, sizeof(struct rtknl));

	if (!wiphy) {
		printk("couldn't allocate wiphy device\n");
		return NULL;
	}

	//printk("NUM_TX_DESC[%d] NUM_RX_DESC[%d] NUM_RX_DESC_2G[%d]\n",NUM_TX_DESC,NUM_RX_DESC,NUM_RX_DESC_2G);

	rtk = wiphy_priv(wiphy);
	rtk->wiphy = wiphy;
	//rtk->priv = priv;	 //mark_dual2

	//sync to global rtk_phy
	if(rtk_phy_idx >= RTK_MAX_WIFI_PHY)
	{
		printk("ERROR!! rtk_phy_idx >=  RTK_MAX_WIFI_PHY\n");
		wiphy_free(wiphy);
		return NULL;
	}
#ifdef RTK_129X_PLATFORM
	rtk_create_dev(rtk,dev,rtk_phy_idx);
#else
	rtk_create_dev(rtk,rtk_phy_idx);
#endif
	rtk_phy[rtk_phy_idx] = rtk;
	rtk_phy_idx++;

	//priv->rtk = rtk ; //mark_dual2

	//NLEXIT;
	return rtk;
}

static const struct ieee80211_txrx_stypes rtw_cfg80211_default_mgmt_stypes[NUM_NL80211_IFTYPES] = {
	[NL80211_IFTYPE_ADHOC] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_STATION] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_AP] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
	[NL80211_IFTYPE_AP_VLAN] = {
		/* copy AP */
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
#if defined(P2P_SUPPORT)
	[NL80211_IFTYPE_P2P_CLIENT] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ACTION >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4)
	},
	[NL80211_IFTYPE_P2P_GO] = {
		.tx = 0xffff,
		.rx = BIT(IEEE80211_STYPE_ASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_REASSOC_REQ >> 4) |
		BIT(IEEE80211_STYPE_PROBE_REQ >> 4) |
		BIT(IEEE80211_STYPE_DISASSOC >> 4) |
		BIT(IEEE80211_STYPE_AUTH >> 4) |
		BIT(IEEE80211_STYPE_DEAUTH >> 4) |
		BIT(IEEE80211_STYPE_ACTION >> 4)
	},
#endif
};


#define MAX_REMAIN_ON_CHANNEL_DURATION 5000 //ms
#define SCAN_IE_LEN_MAX                2304
//#define SSID_SCAN_AMOUNT               1 // for WEXT_CSCAN_AMOUNT 9
#define SSID_SCAN_AMOUNT	9

#define MAX_NUM_PMKIDS                 32
#define	NL_MAX_INTF						9

static const struct ieee80211_iface_limit rtk_if_limits[] = {
	{ .max = 8,	.types = BIT(NL80211_IFTYPE_AP) },
	{ .max = 1,	.types = BIT(NL80211_IFTYPE_STATION) },
};

static const struct ieee80211_iface_combination rtk_2g_comb = {
	.limits = rtk_if_limits,
	.n_limits = ARRAY_SIZE(rtk_if_limits),
	.max_interfaces = NL_MAX_INTF,
	.num_different_channels = 1,
};

static const struct ieee80211_iface_combination rtk_5g_comb = {
	.limits = rtk_if_limits,
	.n_limits = ARRAY_SIZE(rtk_if_limits),
	.max_interfaces = NL_MAX_INTF,
	.num_different_channels = 1,
	.radar_detect_widths =	BIT(NL80211_CHAN_WIDTH_20_NOHT) |
					BIT(NL80211_CHAN_WIDTH_20) |
					BIT(NL80211_CHAN_WIDTH_40) |
					BIT(NL80211_CHAN_WIDTH_80),
};

/**
 * enum wiphy_flags - wiphy capability flags
 *
 * @WIPHY_FLAG_CUSTOM_REGULATORY:  tells us the driver for this device
 * 	has its own custom regulatory domain and cannot identify the
 * 	ISO / IEC 3166 alpha2 it belongs to. When this is enabled
 * 	we will disregard the first regulatory hint (when the
 * 	initiator is %REGDOM_SET_BY_CORE).

 * @WIPHY_FLAG_STRICT_REGULATORY: tells us the driver for this device will
 *	ignore regulatory domain settings until it gets its own regulatory
 *	domain via its regulatory_hint() unless the regulatory hint is
 *	from a country IE. After its gets its own regulatory domain it will
 *	only allow further regulatory domain settings to further enhance
 *	compliance. For example if channel 13 and 14 are disabled by this
 *	regulatory domain no user regulatory domain can enable these channels
 *	at a later time. This can be used for devices which do not have
 *	calibration information guaranteed for frequencies or settings
 *	outside of its regulatory domain. If used in combination with
 *	WIPHY_FLAG_CUSTOM_REGULATORY the inspected country IE power settings
 *	will be followed.

 * @WIPHY_FLAG_DISABLE_BEACON_HINTS: enable this if your driver needs to ensure
 *	that passive scan flags and beaconing flags may not be lifted by
 *	cfg80211 due to regulatory beacon hints. For more information on beacon
 *	hints read the documenation for regulatory_hint_found_beacon()

 * @WIPHY_FLAG_NETNS_OK: if not set, do not allow changing the netns of this
 *	wiphy at all

 * @WIPHY_FLAG_PS_ON_BY_DEFAULT: if set to true, powersave will be enabled
 *	by default -- this flag will be set depending on the kernel's default
 *	on wiphy_new(), but can be changed by the driver if it has a good
 *	reason to override the default

 * @WIPHY_FLAG_4ADDR_AP: supports 4addr mode even on AP (with a single station
 *	on a VLAN interface)

 * @WIPHY_FLAG_4ADDR_STATION: supports 4addr mode even as a station

 * @WIPHY_FLAG_CONTROL_PORT_PROTOCOL: This device supports setting the
 *	control port protocol ethertype. The device also honours the
 *	control_port_no_encrypt flag.

 * @WIPHY_FLAG_IBSS_RSN: The device supports IBSS RSN.

 * @WIPHY_FLAG_MESH_AUTH: The device supports mesh authentication by routing
 *	auth frames to userspace. See @NL80211_MESH_SETUP_USERSPACE_AUTH.

 * @WIPHY_FLAG_SUPPORTS_SCHED_SCAN: The device supports scheduled scans.

 * @WIPHY_FLAG_SUPPORTS_FW_ROAM: The device supports roaming feature in the 	firmware.
 * @WIPHY_FLAG_AP_UAPSD: The device supports uapsd on AP.
 * @WIPHY_FLAG_SUPPORTS_TDLS: The device supports TDLS (802.11z) operation.
 * @WIPHY_FLAG_TDLS_EXTERNAL_SETUP: The device does not handle TDLS (802.11z)
 *	link setup/discovery operations internally. Setup, discovery and
 *	teardown packets should be sent through the @NL80211_CMD_TDLS_MGMT
 *	command. When this flag is not set, @NL80211_CMD_TDLS_OPER should be
 *	used for asking the driver/firmware to perform a TDLS operation.
 * @WIPHY_FLAG_HAVE_AP_SME: device integrates AP SME
 * @WIPHY_FLAG_REPORTS_OBSS: the device will report beacons from other BSSes
 *	when there are virtual interfaces in AP mode by calling
 *	cfg80211_report_obss_beacon().
 * @WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD: When operating as an AP, the device
 *	responds to probe-requests in hardware.
 * @WIPHY_FLAG_OFFCHAN_TX: Device supports direct off-channel TX.
 * @WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL: Device supports remain-on-channel call.
 */
int realtek_cfg80211_init(struct rtknl *rtk,struct rtl8192cd_priv *priv)
{
	struct wiphy *wiphy = rtk->wiphy;
	BOOLEAN band_2gig = false, band_5gig = false;
	int ret;
	const struct ieee80211_regdomain *realtek_regdom;
#ifdef EN_EFUSE
	char efusemac[ETH_ALEN];
#endif
#if defined(EN_EFUSE) && !defined(CUSTOMIZE_FLASH_EFUSE)
	char zero[ETH_ALEN] = {0,0,0,0,0,0};
#endif
#ifdef CONFIG_WLAN_HAL_8814AE
	unsigned char txbf_max_ant, txbf_sounding_dim;
#endif

	NLENTER;
	rtk->priv = priv;  //mark_dual

	rtk_get_band_capa(priv,&band_2gig,&band_5gig);

	//wiphy->mgmt_stypes = realtek_mgmt_stypes; //_eric_cfg ??
	wiphy->mgmt_stypes = rtw_cfg80211_default_mgmt_stypes; /*cfg p2p*/

#if defined(SIGNAL_TYPE_UNSPEC)
	wiphy->signal_type=CFG80211_SIGNAL_TYPE_UNSPEC;
#else
	wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM; //mark_priv
#endif
	/* max num of ssids that can be probed during scanning */
	//wiphy->max_scan_ssids = MAX_PROBED_SSIDS;
	wiphy->max_scan_ssids = SSID_SCAN_AMOUNT;

	/* max num of ssids that can be matched after scan */
	//wiphy->max_match_sets = MAX_PROBED_SSIDS;

	//wiphy->max_scan_ie_len = 1000; /* FIX: what is correct limit? */
	wiphy->max_scan_ie_len = SCAN_IE_LEN_MAX;
	wiphy->max_num_pmkids = MAX_NUM_PMKIDS;
	wiphy->max_remain_on_channel_duration = MAX_REMAIN_ON_CHANNEL_DURATION; /*cfg p2p p2p related*/

	switch(GET_CHIP_VER(priv)) {
		case VERSION_8188C:
		case VERSION_8188E:
		case VERSION_8881A:
			wiphy->available_antennas_tx = 0x1;
			wiphy->available_antennas_rx = 0x1;
		break;
		case VERSION_8814A:
			wiphy->available_antennas_tx = 0x7;
			wiphy->available_antennas_rx = 0xf;
			break;
		default:
			wiphy->available_antennas_tx = 0x3;
			wiphy->available_antennas_rx = 0x3;
	}

	 /*The device supports roaming feature in the firmware*/    	//_eric_cfg ?? support these features ??
	wiphy->flags |= WIPHY_FLAG_SUPPORTS_FW_ROAM;
	wiphy->flags |= WIPHY_FLAG_AP_UAPSD;
	/*device integrates AP SME*/
	wiphy->flags |= WIPHY_FLAG_HAVE_AP_SME;
	wiphy->flags |= WIPHY_FLAG_HAS_REMAIN_ON_CHANNEL;/*cfg p2p ; p2p must use it*/

	/*The device supports a4*/
#ifdef A4_STA
	wiphy->flags |= WIPHY_FLAG_4ADDR_AP;
	wiphy->flags |= WIPHY_FLAG_4ADDR_STATION;
#endif

    if(band_2gig){
		wiphy->interface_modes = BIT(NL80211_IFTYPE_AP)|
							BIT(NL80211_IFTYPE_STATION) | //_eric_cfg station mandatory ??
							BIT(NL80211_IFTYPE_ADHOC) |   //wrt-adhoc
#if defined(P2P_SUPPORT)
							BIT(NL80211_IFTYPE_P2P_CLIENT)|
							BIT(NL80211_IFTYPE_P2P_GO)|
							BIT(NL80211_IFTYPE_P2P_DEVICE)|
#endif
							0/*BIT(NL80211_IFTYPE_MONITOR)*/;
    }else{
        wiphy->interface_modes = BIT(NL80211_IFTYPE_AP)|
                                BIT(NL80211_IFTYPE_STATION) | //_eric_cfg station mandatory ??
                                BIT(NL80211_IFTYPE_ADHOC)|
								0/*BIT(NL80211_IFTYPE_MONITOR)*/;
    }

	wiphy->max_acl_mac_addrs = NUM_STAT;
	//printk("set_wiphy_dev +++ \n");
	set_wiphy_dev(wiphy, rtk->dev); //return wiphy->dev.parent;
	//printk("set_wiphy_dev --- \n");

#if defined(EN_EFUSE) && !defined(CUSTOMIZE_FLASH_EFUSE)
#ifdef CONFIG_WLAN_HAL_8881A
	if (GET_CHIP_VER(priv) != VERSION_8881A)
#endif
	{
		memset(efusemac,0,ETH_ALEN);
		extern void read_efuse_mac_address(struct rtl8192cd_priv * priv,char * efusemac);
		read_efuse_mac_address(priv,efusemac);
		if( memcmp(efusemac,zero,ETH_ALEN) && !IS_MCAST(efusemac))
			memcpy(rtk->root_mac,efusemac,ETH_ALEN);
	}
#endif
	memcpy(wiphy->perm_addr, rtk->root_mac, ETH_ALEN);
	wpaslog("wiphy->perm_addr[%pm]",wiphy->perm_addr);
	memcpy(priv->pmib->dot11Bss.bssid, wiphy->perm_addr, ETH_ALEN);

	/*
	 * Even if the fw has HT support, advertise HT cap only when
	 * the firmware has support to override RSN capability, otherwise
	 * 4-way handshake would fail.
	 */
	if (band_2gig)
	{
		wiphy->bands[IEEE80211_BAND_2GHZ] = &realtek_band_2ghz;
		switch (get_rf_mimo_mode(priv))
		{
		case RF_1T1R:
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(150);
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[1] = 0;
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[2] = 0;
			break;
		case RF_2T2R:
			/* skip default setting */
			break;
		case RF_3T3R:
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(450);
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[1] = 0xff;
			wiphy->bands[IEEE80211_BAND_2GHZ]->ht_cap.mcs.rx_mask[2] = 0xff;
			break;
		default:
			NDEBUG(" Waring, mimo(%d) not support\n", get_rf_mimo_mode(priv));
			break;
		}
		wiphy->iface_combinations = &rtk_2g_comb;
		wiphy->n_iface_combinations = 1;
	}

	if (band_5gig)
	{
		wiphy->bands[IEEE80211_BAND_5GHZ] = &realtek_band_5ghz;
		switch (get_rf_mimo_mode(priv))
		{
		case RF_1T1R:
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(150);
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[1] = 0;
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[2] = 0;
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xfffe);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xfffe);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_highest = cpu_to_le16(390);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_highest = cpu_to_le16(390);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap &= ~(IEEE80211_VHT_CAP_TXSTBC);
			break;
		case RF_2T2R:
			/* skip default setting */
			break;
		case RF_3T3R:
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_highest = cpu_to_le16(450);
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[0] = 0xff;
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[1] = 0xff;
			wiphy->bands[IEEE80211_BAND_5GHZ]->ht_cap.mcs.rx_mask[2] = 0xff;
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_mcs_map = cpu_to_le16(0xffea);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_mcs_map = cpu_to_le16(0xffea);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.rx_highest = cpu_to_le16(1170);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.vht_mcs.tx_highest = cpu_to_le16(1170);
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap |= (IEEE80211_VHT_CAP_RXLDPC);
			break;
		default:
			NDEBUG(" Waring, mimo(%d) not support\n", get_rf_mimo_mode(priv));
			break;
		}

#ifdef CONFIG_WLAN_HAL_8814AE
		if(GET_CHIP_VER(priv)==VERSION_8814A) {
			if (priv->pshare->rf_ft_var.bf_sup_val != 0) {
				txbf_max_ant = priv->pshare->rf_ft_var.bf_sup_val;
				txbf_sounding_dim = priv->pshare->rf_ft_var.bf_sup_val;
			} else if (get_rf_mimo_mode(priv) == RF_4T4R) {
				txbf_max_ant = 3;
				txbf_sounding_dim = 3;
			} else if (get_rf_mimo_mode(priv) == RF_3T3R) {
				txbf_max_ant = 2;
				txbf_sounding_dim = 3;
			} else if ((get_rf_mimo_mode(priv) == RF_2T4R) || (get_rf_mimo_mode(priv) == RF_2T2R)) {
				txbf_max_ant = 2;
				txbf_sounding_dim = 1;
			} else {
				txbf_max_ant = 1;
				txbf_sounding_dim = 1;
			}

			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap &= ~IEEE80211_VHT_CAP_BEAMFORMEE_STS_MASK;
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap |= (txbf_max_ant << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT);

			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap &= ~IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_MASK;
			wiphy->bands[IEEE80211_BAND_5GHZ]->vht_cap.cap |= (txbf_sounding_dim << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT);
		}
#endif

		wiphy->iface_combinations = &rtk_5g_comb;
		wiphy->n_iface_combinations = 1;
	}

#if 0 //def RTK_NL80211
		//wiphy->regulatory_flags |= REGULATORY_CUSTOM_REG;
	wiphy->regulatory_flags = REGULATORY_WIPHY_SELF_MANAGED;
	realtek_regdom = _rtl_regdomain_select(priv->pmib->dot11StationConfigEntry.dot11RegDomain);
		//wiphy_apply_custom_regulatory(wiphy, realtek_regdom);
	if (rtnl_is_locked())
		regulatory_set_wiphy_regd_sync_rtnl(wiphy, realtek_regdom);
	else {
		rtnl_lock();
		regulatory_set_wiphy_regd_sync_rtnl(wiphy, realtek_regdom);
		rtnl_unlock();
	}
#endif

	wiphy->cipher_suites = cipher_suites;
	wiphy->n_cipher_suites = ARRAY_SIZE(cipher_suites);
#if 0//def CONFIG_PM
	wiphy->wowlan.flags = WIPHY_WOWLAN_MAGIC_PKT |
			      WIPHY_WOWLAN_DISCONNECT |
			      WIPHY_WOWLAN_GTK_REKEY_FAILURE  |
			      WIPHY_WOWLAN_SUPPORTS_GTK_REKEY |
			      WIPHY_WOWLAN_EAP_IDENTITY_REQ   |
			      WIPHY_WOWLAN_4WAY_HANDSHAKE;
	wiphy->wowlan.n_patterns = WOW_MAX_FILTERS_PER_LIST;
	wiphy->wowlan.pattern_min_len = 1;
	wiphy->wowlan.pattern_max_len = WOW_PATTERN_SIZE;

	wiphy->max_sched_scan_ssids = MAX_PROBED_SSIDS;


	wiphy->features |= NL80211_FEATURE_INACTIVITY_TIMER;
	wiphy->probe_resp_offload =
		NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS |
		NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS2 |
		NL80211_PROBE_RESP_OFFLOAD_SUPPORT_P2P;
#endif
#ifdef CONFIG_OPENWRT_SDK
	wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;
#endif

	wpaslog("Client mode add SAE Support");
	wiphy->features |= NL80211_FEATURE_SAE;

	/*----------------------reg domain related-----------------------*/
	/*asssign assign reg_notifier; for hapd can via it change wlan drv's RegDoamin*/
	//NDEBUG2("assign wiphy->reg_notifier\n");
	wiphy->reg_notifier = rtk_reg_notifier;


	/*---we need default val here---*/
	NDEBUG2("use default countrystr [%s]\n",priv->pmib->dot11dCountry.dot11CountryString);
	NDEBUG2("use default regdomain  [%d]\n",priv->pmib->dot11StationConfigEntry.dot11RegDomain);

	wiphy->regulatory_flags = REGULATORY_WIPHY_SELF_MANAGED;
	realtek_regdom = _rtl_regdomain_select(priv->pmib->dot11StationConfigEntry.dot11RegDomain);


	ret = wiphy_register(wiphy);

	if (rtnl_is_locked())
		regulatory_set_wiphy_regd_sync_rtnl(wiphy, (struct ieee80211_regdomain *)realtek_regdom);
	else {
		rtnl_lock();
		regulatory_set_wiphy_regd_sync_rtnl(wiphy, (struct ieee80211_regdomain *)realtek_regdom);
		rtnl_unlock();
	}
	/*----------------------reg domain related-----------------------*/



	if (ret < 0) {
		printk("couldn't register wiphy device\n");
		return ret;
	}

	rtk->wiphy_registered = true;

	NLEXIT;
	return 0;
}

void realtek_process_cfg80211_event(unsigned long task_priv)
{
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;
	struct list_head *head = &priv->pshare->cfg80211_queue;
	unsigned long flags;
	struct CFG80211_RX_MGMT_IND *entry, *next;

	if (!IS_DRV_OPEN(priv))
		return;

	list_for_each_entry_safe(entry, next, head, list) {
		spin_lock_irqsave(&priv->pshare->cfg80211_queue_lock, flags);
		list_del_init(&entry->list);
		spin_unlock_irqrestore(&priv->pshare->cfg80211_queue_lock, flags);

		cfg80211_rx_mgmt(entry->wdev, entry->freq, 0, entry->buf, entry->len, entry->flags);
		kfree(entry);
	}
}

void realtek_enqueue_cfg80211_event(struct rtl8192cd_priv *priv, struct rx_frinfo *pfrinfo, u32 answered)
{
	unsigned long flags;
	unsigned int channel = priv->pmib->dot11RFEntry.dot11channel;
	unsigned char *pframe = get_pframe(pfrinfo);
	struct CFG80211_RX_MGMT_IND *ind = NULL;

	ind = (struct CFG80211_RX_MGMT_IND *)kmalloc(sizeof(struct CFG80211_RX_MGMT_IND), GFP_ATOMIC);
	memset(ind, 0, sizeof(struct CFG80211_RX_MGMT_IND));

	if (!ind)
		return;

	INIT_LIST_HEAD(&ind->list);
	ind->wdev = &priv->wdev;
	if (priv->pshare->curr_band == BAND_2G)
		ind->freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_2GHZ);
	else
		ind->freq = ieee80211_channel_to_frequency(channel, IEEE80211_BAND_5GHZ);
	memcpy(ind->buf, pframe, pfrinfo->pktlen);
	ind->len = pfrinfo->pktlen;
	ind->flags = answered;

	spin_lock_irqsave(&priv->pshare->cfg80211_queue_lock, flags);
	list_add_tail(&ind->list, &priv->pshare->cfg80211_queue);
	spin_unlock_irqrestore(&priv->pshare->cfg80211_queue_lock, flags);

	tasklet_schedule(&priv->pshare->cfg80211_tasklet);
}

void realtek_clear_cfg80211_event(struct rtl8192cd_priv *priv)
{
	struct list_head *head = &priv->pshare->cfg80211_queue;
	struct CFG80211_RX_MGMT_IND *entry = NULL, *next = NULL;
	unsigned long flags;

	spin_lock_irqsave(&priv->pshare->cfg80211_queue_lock, flags);

	list_for_each_entry_safe(entry, next, head, list) {
		list_del_init(&entry->list);
		kfree(entry);
	}

	spin_unlock_irqrestore(&priv->pshare->cfg80211_queue_lock, flags);
}

#ifdef SUPPORT_WPAS_SAE
/*
Note by Ong-JingYing(pluswang@realtek.com)
for STA mode handle SAE-COMMIT case
when STA mode start join to WPA3 AP, issue_auth_req will go throught here
and pass event (cfg80211_external_auth_request) up to wpas
wpas prepare SAE-COMMIT content and driver help to send out
*/
void wpas_issue_sae_auth_commit( struct rtl8192cd_priv *priv)
{
	int ret=0;
	struct cfg80211_external_auth_params params;
	struct net_device *netdev = priv->dev;
	int freq = 0;
	memset(&params,0,sizeof(struct cfg80211_external_auth_params));

	/* rframe, in this case is null point */
	if(priv->pmib->dot11RFEntry.dot11channel >= 34)
		freq = ieee80211_channel_to_frequency(priv->pmib->dot11RFEntry.dot11channel, IEEE80211_BAND_5GHZ);
	else
		freq = ieee80211_channel_to_frequency(priv->pmib->dot11RFEntry.dot11channel, IEEE80211_BAND_2GHZ);

	wpaslog("freq=[%d] , peer[%pm]",freq , priv->pmib->dot11Bss.bssid);
	params.action = EXTERNAL_AUTH_START;
	memcpy(params.bssid, priv->pmib->dot11Bss.bssid, ETH_ALEN);
	params.ssid.ssid_len = priv->pmib->dot11Bss.ssidlen;
	memcpy(params.ssid.ssid, priv->pmib->dot11Bss.ssid, priv->pmib->dot11Bss.ssidlen);
	params.key_mgmt_suite = cpu_to_le32(0x8ac0f00);
	ret = cfg80211_external_auth_request(netdev,&params, GFP_ATOMIC);
	wpaslog("ret = [%d]\n",ret);

}

#endif

#endif //RTK_NL80211

