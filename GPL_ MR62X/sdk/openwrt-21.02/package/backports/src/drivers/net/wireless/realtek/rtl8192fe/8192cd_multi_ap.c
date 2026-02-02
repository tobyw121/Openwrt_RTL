/*
 *  Multi-AP routines
 *
 */

#define _8192CD_MULTI_AP_C_

#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/timer.h>
#include "8192cd.h"
#include "8192cd_util.h"
#include "8192cd_headers.h"
#include "ieee802_mib.h"
#include "8192cd_netlink.h"

#define NETLINK_RTK 31
#define MAX_PAYLOAD 2040

#define TLV_TYPE_AP_CAPABILITY                        (161) // 0xA1
#define TLV_TYPE_AP_RADIO_BASIC_CAPABILITIES          (133) // 0x85
#define TLV_TYPE_AP_HT_CAPABILITIES                   (134) // 0x86
#define TLV_TYPE_AP_VHT_CAPABILITIES                  (135) // 0x87
#define TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION      (191)	// 0xBF
#define TLV_TYPE_TUNNELED                             (194) // 0xC2

#if (EASYMESH_VERSION >= MULTI_AP_VERSION(3,0,0))
#define TLV_TYPE_SERVICE_PRIORITIZATION_RULE			(185) // 0xB9
#define TLV_TYPE_DSCP_MAPPING_TABLE         			(186) // 0xBA
#endif

#define MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE 0x00
#define MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN 0x80
#define MASK_BTM_RESPONSE_EVENT 0x40

#define MAP_CHANNEL_CHANGE_NOTIFICATION_MESSAGE      (0xFE)
#define MAP_FRAGMENTIZED_DATA						(240) // 0xF0
#define MAP_GENERAL_NETLINK_MESSAGE                  (0xFF)

#define MAP_MAX_NETLINK_MSG_SIZE						4096

/*Multi-AP Triggered Events*/
//Received Beacon Metrics Response
//Received BTM Report

static struct sock *rtk_multi_ap_nl = NULL;
#if !defined(RTK_COMMON_NETLINK)
static int            msg_dropcounter       = 0;
#endif
static int            rtk_multi_ap_user_pid = 0;
static unsigned char *rtk_multi_ap_prefix   = "rtk_multi_ap";


static inline void _I2B(unsigned short int *memory_pointer, unsigned char **packet_ppointer)
{
#ifdef _LITTLE_ENDIAN_
    **packet_ppointer = *(((unsigned char *)memory_pointer)+1); (*packet_ppointer)++;
    **packet_ppointer = *(((unsigned char *)memory_pointer)+0); (*packet_ppointer)++;
#else
    **packet_ppointer = *(((unsigned char *)memory_pointer)+0); (*packet_ppointer)++;
    **packet_ppointer = *(((unsigned char *)memory_pointer)+1); (*packet_ppointer)++;
#endif
}

//Only for association request and association response
unsigned char *construct_multiap_ie(unsigned char *pbuf, unsigned int *frlen, unsigned char profile, unsigned char bss_type, unsigned short vlan_id) {

	unsigned char oui[4] = {0x50, 0x6f, 0x9a, 0x1B};
	unsigned char temp[128] = {0};
	unsigned char subelement_val = 0;
	unsigned char ie_len = 0;
	unsigned char *p;
	unsigned short vid = cpu_to_le16(vlan_id);

	subelement_val = bss_type;

	p = temp;
	memcpy(temp, oui, 4);
	p += 4;
	ie_len += 4;

	//Multi-ap extension subelement
	*p = 0x06;
	p++;
	*p = 1;
	p++;
	*p = subelement_val;
	p++;
	ie_len += 3;

	//Multi-ap profile subelement
	if (profile >= MAP_PROFILE_TWO) {
		*p = _MAP_PROFILE_SUBIE_ID_;
		p++;
		*p = 1;
		p++;
		*p = profile;
		p++;
		ie_len += 3;
		// If backhaul BSS and vid is configured, include traffic separation policy ie
		if ((bss_type & BIT6) && vid) {
			// 4 bytes
			*p = _MAP_VLAN_ID_SUBIE_ID_;
			p++;
			*p = 2;
			p++;
			memcpy(p, &vid, 2);
			p += 2;
			ie_len += 4;
		}
	}

	pbuf = set_ie(pbuf, _VENDOR_SPECIFIC_IE_, ie_len, temp, frlen);
	return pbuf;
}

unsigned char rssiToRcpi(unsigned char rssi)
{
	//convert per 100 to per 220
	return ( 2 * (10 + rssi ));
}

void rtk_multi_ap_nl_rcv(struct sk_buff *skb)
{
	struct nlmsghdr *nlh = NULL;

	if (skb == NULL) {
		panic_printk(KERN_INFO "%s: skb is NULL\n", __FUNCTION__);
		return;
	}

	nlh = (struct nlmsghdr *)skb->data;

	if (0 == memcmp(NLMSG_DATA(nlh), rtk_multi_ap_prefix, strlen(rtk_multi_ap_prefix))) {
		rtk_multi_ap_user_pid = nlh->nlmsg_pid;
	}

}

void _rtk_multi_ap_nl_send_fragment(char *data, int data_len)
{
#if defined(RTK_COMMON_NETLINK)
	rtk_netlink_send(data, data_len, NL_COMM_MAP);
#else
	struct nlmsghdr *nlh;
	struct sk_buff * skb = NULL;
	const char *     fn;
	int              err;

	skb    = alloc_skb(NLMSG_SPACE(data_len), GFP_ATOMIC);

	if (!skb) {
		err = -ENOBUFS;
		fn  = "alloc_skb";
		goto msg_fail;
	}

	nlh = nlmsg_put(skb, 0, 0, 0, data_len, 0);

	if (!nlh) {
		err = -ENOBUFS;
		fn  = "nlmsg_put";
		goto msg_fail_skb;
	}

#if defined(__LINUX_3_10__)
	NETLINK_CB(skb).portid = 0; //from kernel
#else
	NETLINK_CB(skb).pid = 0; //from kernel
#endif
	NETLINK_CB(skb).dst_group = 0; //unicast

	memcpy(NLMSG_DATA(nlh), data, data_len);

	if(rtk_multi_ap_nl)
		err = netlink_unicast(rtk_multi_ap_nl, skb, rtk_multi_ap_user_pid, MSG_DONTWAIT);
	else {
		printk("[%s %u]rtk_multi_ap_nl is NULL\n", __FUNCTION__, __LINE__);
		goto msg_fail_skb;
	}

	if (err < 0) {
		fn = "nlmsg_unicast";
		goto msg_fail; //nlmsg_unicast already kfree_skb
	}

	return;

msg_fail_skb:
	if (skb)
		kfree_skb(skb);

msg_fail:
	if (msg_dropcounter < 3) {
		msg_dropcounter++;
		panic_printk("[%s] drop netlink msg: pid=%d msglen=%d %s: err=%d\n", __FUNCTION__, rtk_multi_ap_user_pid, data_len, fn, err);
	}
	return;
#endif
}

static int message_id = 0;
#define FRAGMENTIZED_PACKET_LEN    MAX_PAYLOAD - 4

void rtk_multi_ap_nl_send(char *data, int data_len)
{
	int fragment_sequence_nr = 0;
	int data_sent = 0;
	unsigned char packet[MAX_PAYLOAD];

	if (data_len > MAP_MAX_NETLINK_MSG_SIZE) {
		panic_printk("Drop message size more than %u\n", MAP_MAX_NETLINK_MSG_SIZE);
		return;
	}
	if (data_len <= MAX_PAYLOAD) {
		_rtk_multi_ap_nl_send_fragment(data, data_len);
		return;
	}
	//fragmentation support
	//add 4 bytes of header to the packet to support the implementation of fragmentation
	packet[0] = MAP_FRAGMENTIZED_DATA;
	packet[1] = message_id;
	while (data_sent < data_len) {
		packet[3] = fragment_sequence_nr;
		fragment_sequence_nr++;
		if (data_len - data_sent <= FRAGMENTIZED_PACKET_LEN) {
			packet[2] = 1; //last fragment
			memcpy(packet + 4, data + data_sent, data_len-data_sent);
			_rtk_multi_ap_nl_send_fragment(packet, data_len - data_sent + 4);
			data_sent += data_len - data_sent;
		} else {
			packet[2] = 0; //not the last fragment
			memcpy(packet + 4, data + data_sent, FRAGMENTIZED_PACKET_LEN);
			_rtk_multi_ap_nl_send_fragment(packet, MAX_PAYLOAD);
			data_sent += FRAGMENTIZED_PACKET_LEN;
		}
	}
	message_id++;
	return;
}

void update_unassoc_MAC_RSSI_Entry(struct rtl8192cd_priv *priv,unsigned char* addr, unsigned char rssi, unsigned char status, struct sta_mac_rssi *EntryDB){
	int i, idx=-1, idx2 =0; 
	unsigned char rssi_input;
#if 0
	printk("%s Unassoc STA %02x%02x%02x%02x%02x%02x \n",
		__FUNCTION__, addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	for (i=0; i<priv->multiap_unassocStaEntryOccupied; i++) {
		if(EntryDB[i].used)
			printk("%s Target STA %02x%02x%02x%02x%02x%02x \n",
	__FUNCTION__, EntryDB[i].addr[0], EntryDB[i].addr[1], EntryDB[i].addr[2], EntryDB[i].addr[3], EntryDB[i].addr[4], EntryDB[i].addr[5]);
	}
	//return;
#endif
	for (i=0; i<priv->multiap_unassocStaEntryOccupied && i < MAX_PROBE_REQ_STA; i++) {
		if (EntryDB[i].used && memcmp(EntryDB[i].addr, addr, MACADDRLEN) == 0) {
			idx2 = i;
			rssi_input = (EntryDB[idx2].rssi)?(((EntryDB[idx2].rssi * 7)+(rssi * 3)) / 10):(rssi);			
			EntryDB[idx2].rssi = rssi_input;
			EntryDB[idx].status = status;
			// printk("Updated Unassoc STA %02x%02x%02x%02x%02x%02x - rssi %d (%d)\n",
			// 	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], rssi_input, rssi);
			return;
		}
	}
	return;
}

void add_unassoc_MAC_RSSI_Entry(struct rtl8192cd_priv *priv,unsigned char* addr, unsigned char status, struct sta_mac_rssi *EntryDB, unsigned int *EntryOccupied, unsigned int *EntryNum)
{
	int i, idx=-1;
	
	for (i=0; i<priv->multiap_unassocStaEntryOccupied; i++) {
		if(memcmp(addr, priv->multiap_unassoc_sta[i].addr, MACADDRLEN) == 0){
			// printk("%s - Skip duplicate %02x%02x%02x%02x%02x%02x\n", __FUNCTION__,
			// 	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
			return;
		}
	}
	for (i=0; i<MAX_PROBE_REQ_STA; i++) {
		if (EntryDB[i].used == 0) {
			if (idx < 0){
				idx = i; //search for empty entry
				EntryDB[idx].used = 1;
				memcpy(EntryDB[idx].addr, addr, MACADDRLEN);
				EntryDB[idx].Entry = idx;//check which entry is the probe sta recorded
				EntryDB[idx].status = status;
				(*EntryOccupied)++;
				// printk("%s -Add  %02x%02x%02x%02x%02x%02x\n", __FUNCTION__,
				// addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
				return;
			}
		}
	}

	return;
/*	
	if ((*EntryOccupied) == MAX_PROBE_REQ_STA) {// sta list full, need to replace sta
		idx = *EntryNum;	
		for (i=0; i<MAX_PROBE_REQ_STA; i++) {
			if (!memcmp(EntryDB[i].addr, addr, MACADDRLEN))					
				return;		// check if it is already in the list			
		}
		memcpy(EntryDB[idx].addr, addr, MACADDRLEN);		
		EntryDB[idx].used = 1;
		EntryDB[idx].Entry = idx;		
		EntryDB[idx].status = status;
		(*EntryNum)++;			
		if( (*EntryNum) == MAX_PROBE_REQ_STA)	
			*EntryNum = 0; // Reset entry counter;
		return;
	}
*/
}

void apCapability_notify(struct rtl8192cd_priv *priv){
	unsigned char send_buf[MAX_PAYLOAD];
	unsigned int data_len = 0;

	send_buf[0] = TLV_TYPE_AP_CAPABILITY;
	send_buf[1] = 1; //number of AP_HT_CAPABILITY
	send_buf[2] = 0;
	send_buf[3] = TLV_TYPE_AP_HT_CAPABILITIES;
	send_buf[4] = 1; //number of AP_HT_CAPABILITY
	send_buf[5] = 0;

	data_len += 1;
	
	rtk_multi_ap_nl_send(send_buf, data_len);
}

void clear_client_notify(struct rtl8192cd_priv *priv){
	unsigned char send_buf[2 + MACADDRLEN];

	send_buf[0] = 0x91;
	send_buf[1] = 0;
	memcpy(&send_buf[2], BSSID, MACADDRLEN);

	rtk_multi_ap_nl_send(send_buf, 2);
}

void clientCapability_notify(unsigned char *mac, unsigned char *bssid, unsigned char frameLength, unsigned char *frameData)
{
	struct clientCapabilityEntry clientCapability;
	unsigned int data_len = 1 + 1 + sizeof(struct clientCapabilityEntry);
	unsigned char send_buf[data_len];

	send_buf[0] = 0x91;
	send_buf[1] = 1;
	
	memcpy(clientCapability.macaddr, mac, MACADDRLEN);
	memcpy(clientCapability.bssid, bssid, MACADDRLEN);
	clientCapability.frameLength = frameLength;
	memcpy(clientCapability.frameBody, frameData, clientCapability.frameLength);
	
	memcpy(&send_buf[2], &clientCapability, sizeof(struct clientCapabilityEntry));

	rtk_multi_ap_nl_send(send_buf, data_len);
}

void client_notify(unsigned char event, unsigned char mac[MACADDRLEN], unsigned char bssid[MACADDRLEN])
{
	int           data_len                                                  = 0;
	unsigned char send_buf[1 + MACADDRLEN + MACADDRLEN + sizeof(unsigned char)] = { 0 };

	send_buf[0] = MAP_GENERAL_NETLINK_MESSAGE;
	data_len += 1;

	//  The MAC address of the client.
	memcpy(send_buf + data_len, mac, MACADDRLEN);
	data_len += MACADDRLEN;

	//  The BSSID of the BSS
	memcpy(send_buf + data_len, bssid, MACADDRLEN);
	data_len += MACADDRLEN;

	send_buf[data_len] = event;
	data_len += sizeof(unsigned char);

	rtk_multi_ap_nl_send(send_buf, data_len);
}

void client_join_notify(unsigned char bssid[MACADDRLEN], unsigned char mac[MACADDRLEN])
{
//	printk("[Multi-AP] client join event: MAC %02x:%02x:%02x:%02x:%02x:%02x BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
//	       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
	client_notify(MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_JOIN, mac, bssid);
}

void client_leave_notify(unsigned char bssid[MACADDRLEN], unsigned char mac[MACADDRLEN])
{
//	printk("[Multi-AP] client leave event: MAC %02x:%02x:%02x:%02x:%02x:%02x BSSID %02x:%02x:%02x:%02x:%02x:%02x\n",
//	       mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], bssid[0], bssid[1], bssid[2], bssid[3], bssid[4], bssid[5]);
	client_notify(MASK_CLIENT_ASSOCIATION_EVENT_ASSOCIATION_EVENT_LEAVE, mac, bssid);
}

void bss_transition_response_notify(unsigned char bssid[MACADDRLEN], unsigned char mac[MACADDRLEN], unsigned char target_bssid[MACADDRLEN], unsigned char status)
{
	int 		  data_len													= 0;
	unsigned char send_buf[1 + MACADDRLEN + MACADDRLEN + sizeof(unsigned char) + MACADDRLEN + sizeof(unsigned char)] = { 0 };

	send_buf[0] = MAP_GENERAL_NETLINK_MESSAGE;
	data_len += 1;

	//	The MAC address of the client.
	memcpy(send_buf + data_len, mac, MACADDRLEN);
	data_len += MACADDRLEN;

	//	The BSSID of the BSS
	memcpy(send_buf + data_len, bssid, MACADDRLEN);
	data_len += MACADDRLEN;

	send_buf[data_len] = MASK_BTM_RESPONSE_EVENT;
	data_len += sizeof(unsigned char);

	memcpy(send_buf + data_len, target_bssid, MACADDRLEN);
	data_len += MACADDRLEN;

	send_buf[data_len] = status;
	data_len += sizeof(unsigned char);

	rtk_multi_ap_nl_send(send_buf, data_len);
}

int rtk_multi_ap_init(void)
{

	#if defined(__LINUX_3_10__)
	struct netlink_kernel_cfg cfg = {
		.input = rtk_multi_ap_nl_rcv,
	};

	rtk_multi_ap_nl = netlink_kernel_create(&init_net, NETLINK_RTK, &cfg);
	#else
	rtk_multi_ap_nl     = netlink_kernel_create(&init_net, NETLINK_RTK, 0, rtk_multi_ap_nl_rcv, NULL, THIS_MODULE);
	#endif

	if (!rtk_multi_ap_nl) {
		panic_printk(KERN_ERR "rtk_multi_ap_nl: Cannot create netlink socket");
		return -ENOMEM;
	}

	return 0;
}

void rtk_multi_ap_exit(void)
{
	netlink_kernel_release(rtk_multi_ap_nl);
	rtk_multi_ap_user_pid = 0;
}

void rtk_multi_ap_switch_channel(struct rtl8192cd_priv *priv)
{
	int i, chan;

	if (!IS_ROOT_INTERFACE(priv)) {
		panic_printk("Must issue command in root interface !\n");
		return;
	}

	if (!(OPMODE & WIFI_AP_STATE)) {
		panic_printk("root interface must be AP !\n");
		return;
	}

	chan = priv->pmib->dot11RFEntry.dot11channel;

	for (i=0; i<priv->available_chnl_num; i++) {
		if (chan == priv->available_chnl[i])
			break;
		}

	if (i == priv->available_chnl_num) {
		panic_printk("invalid chan [%d] !\n", chan);
		return;
	}

	panic_printk("%s switch channel to ch %d\n", priv->dev->name, priv->pmib->dot11RFEntry.dot11channel);

#ifdef DFS
	if (!priv->ss_req_ongoing)
	{
		#if defined(UNIVERSAL_REPEATER) || defined(MBSSID)
		if (IS_ROOT_INTERFACE(priv))
		#endif
		{
			if (!priv->pmib->dot11DFSEntry.disable_DFS
				&& is_DFS_channel(priv, priv->pmib->dot11RFEntry.dot11channel) && (OPMODE & WIFI_AP_STATE)) {

				// no effect ?
				//if (!is_DFS_channel(priv, priv->pmib->dot11RFEntry.dot11channel))
				//	mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));

#ifdef RTK_REPEATER_MODE_DFS_REFINE
				if (priv->pmib->dot11DFSEntry.CAC_enable)
#endif
				{
#ifdef RTK_REPEATER_MODE_DFS_REFINE
					if (under_apmode_repeater(priv))
						mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
					else
#endif
					if ((priv->pmib->dot11StationConfigEntry.dot11RegDomain == DOMAIN_ETSI) &&
						(IS_METEOROLOGY_CHANNEL(priv->pmib->dot11RFEntry.dot11channel)))
						mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO_CE);
					else
						mod_timer(&priv->ch_avail_chk_timer, jiffies + CH_AVAIL_CHK_TO);

					/* DFS activated after 200 ms; prevent switching channel due to DFS false alarm */
					mod_timer(&priv->DFS_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
					mod_timer(&priv->dfs_det_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(priv->pshare->rf_ft_var.dfs_det_period*10));

					DFS_SetReg(priv);

					if (!priv->pmib->dot11DFSEntry.CAC_enable) {
						mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
					}
				}
			}

			/* disable all of the transmissions during channel availability check */
			priv->pmib->dot11DFSEntry.disable_tx = 0;
			if (!priv->pmib->dot11DFSEntry.disable_DFS &&
#ifdef RTK_REPEATER_MODE_DFS_REFINE
				(priv->pmib->dot11DFSEntry.CAC_enable) &&
#endif
				is_DFS_channel(priv, priv->pmib->dot11RFEntry.dot11channel) && (OPMODE & WIFI_AP_STATE)) {
				priv->pmib->dot11DFSEntry.disable_tx = 1;
			}
		}
	}
#endif /* DFS */

	priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw;
	if(!priv->ss_req_ongoing){
#if defined(CONFIG_RTL_SIMPLE_CONFIG)
	if(!rtk_sc_is_channel_fixed(priv))
#endif
	{
		priv->pshare->CurrentChannelBW = priv->pshare->is_40m_bw = priv->pmib->dot11nConfigEntry.dot11nUse40M;
		priv->pshare->offset_2nd_chan = priv->pmib->dot11nConfigEntry.dot11n2ndChOffset;
		SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
		SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
	}

	#ifdef AUTO_CHANNEL_TIMEOUT
		if (priv->pmib->miscEntry.autoch_timeout && priv->pshare->autoch_trigger_by_timeout)
		{
			// Update channel.
			unsigned char *pbuf = (unsigned char *)priv->beaconbuf + priv->timoffset - 3;

			if (*pbuf == _DSSET_IE_&& *(pbuf+1) == 1) {
				*(pbuf + 2) = priv->pmib->dot11RFEntry.dot11channel;
			}
			priv->ht_cap_len = 0;
	#ifdef MBSSID
			if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
				for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
					priv->pvap_priv[i]->pmib->dot11RFEntry.dot11channel = priv->pmib->dot11RFEntry.dot11channel;
					priv->pvap_priv[i]->ht_cap_len = 0; // re-construct HT IE

					if (IS_DRV_OPEN(priv->pvap_priv[i])) {
						pbuf = (unsigned char *)priv->pvap_priv[i]->beaconbuf + priv->timoffset - 3;
						if (*pbuf == _DSSET_IE_&& *(pbuf+1) == 1) {
							*(pbuf + 2) = priv->pmib->dot11RFEntry.dot11channel;
						}
					}
				}
			}
	#endif
			priv->pshare->autoch_trigger_by_timeout = 0;
			printk("===> complete select channel curr = %d, 2ch = %d\n",
				priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);
		}
		else
	#endif
		{
	priv->ht_cap_len = 0;	// re-construct HT IE
	init_beacon(priv);
#ifdef SIMPLE_CH_UNI_PROTOCOL
	STADEBUG("scan finish, sw ch to (#%d), init beacon\n", priv->pmib->dot11RFEntry.dot11channel);
#endif
#ifdef MBSSID
	if (GET_ROOT(priv)->pmib->miscEntry.vap_enable) {
		for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
			priv->pvap_priv[i]->pmib->dot11RFEntry.dot11channel = priv->pmib->dot11RFEntry.dot11channel;
			priv->pvap_priv[i]->ht_cap_len = 0;	// re-construct HT IE

			if (IS_DRV_OPEN(priv->pvap_priv[i]))
				init_beacon(priv->pvap_priv[i]);
		}
	}
#endif
		}
	}
#ifdef CLIENT_MODE
#ifdef HS2_CLIENT_TEST
	JOIN_RES = STATE_Sta_Ibss_Idle;
#else
	if (JOIN_RES == STATE_Sta_Ibss_Idle) {
		RTL_W32(CR, (RTL_R32(CR) & ~(NETYPE_Mask << NETYPE_SHIFT)) | ((NETYPE_ADHOC & NETYPE_Mask) << NETYPE_SHIFT));
		mod_timer(&priv->idle_timer, jiffies + RTL_SECONDS_TO_JIFFIES(5));
	}
#endif
#endif

	if (priv->ss_req_ongoing) {
		priv->site_survey->count_backup = priv->site_survey->count;
		memcpy(priv->site_survey->bss_backup, priv->site_survey->bss, sizeof(struct bss_desc)*priv->site_survey->count);
		priv->ss_req_ongoing = 0;
	}

#if defined(CONFIG_RTL_NEW_AUTOCH) && defined(SS_CH_LOAD_PROC)
	record_SS_report(priv);
#endif

	priv->pmib->multi_ap.multiap_change_channel = 0;

	// chan = priv->pmib->dot11RFEntry.dot11channel;

	// if (!IS_ROOT_INTERFACE(priv)) {
	// 	panic_printk("Must issue command in root interface !\n");
	// 	return;
	// }
	// if (!(OPMODE & WIFI_AP_STATE)) {
	// 	panic_printk("root interface must be AP !\n");
	// 	return;
	// }

	// for (i=0; i<priv->available_chnl_num; i++) {
	// 	if (chan == priv->available_chnl[i])
	// 		break;
	// }
	// if (i == priv->available_chnl_num) {
	// 	panic_printk("invalid chan [%d] !\n", chan);
	// 	return;
	// }

	// SwBWMode(priv, priv->pshare->CurrentChannelBW, priv->pshare->offset_2nd_chan);
	// SwChnl(priv, priv->pmib->dot11RFEntry.dot11channel, priv->pshare->offset_2nd_chan);

	// priv->ht_cap_len = 0;
	// update_beacon(priv);

	// #ifdef MBSSID
	// if (priv->pmib->miscEntry.vap_enable) {
	// 	int i;
	// 	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
	// 		if (IS_DRV_OPEN(priv->pvap_priv[i])) {
	// 			priv->pvap_priv[i]->pmib->dot11RFEntry.dot11channel = priv->pmib->dot11RFEntry.dot11channel;
	// 			priv->pvap_priv[i]->ht_cap_len = 0;
	// 			update_beacon(priv->pvap_priv[i]);
	// 		}
	// 	}
	// }
	// #endif

	return;
}

void getApCapability(struct rtl8192cd_priv *priv, unsigned char *result_buf, unsigned int *len){

	int offset=0;
	
	result_buf[0] = 0xA1;
	offset++;

	result_buf[offset] = 1;
	offset++;

	//support unassociated STA Link Metric on channel BSS operating on
	//if() 
	//result_buf[offset] |= BIT7;
	result_buf[offset] = 128;
	//printk("%s - AP Capability - %d\n", __FUNCTION__, *(result_buf+offset));
	offset++;

#if 0	
		//support unassociated STA Link Metric report on channel BSS not operating on
		if()
			*result_buf2 |= BIT6;
		//support agent-initated RSSI based steering
		if()
			*result_buf2 |= BIT5;
#endif
	
	*len += offset;

	//return len;
}

void getApBasicCapability(struct rtl8192cd_priv *priv, unsigned char *result_buf, unsigned int *len){

}

void getHTApCapability(struct rtl8192cd_priv *priv, unsigned char *result_buf, unsigned int *len){

	unsigned char val = 0;
	unsigned int mimo_mode=0, offset = 0;

	if(!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11N)){
		*(result_buf+offset) = 0x86;
		offset++;

		*(result_buf+offset) = 0;
		offset++;

		*len += offset;

		return;
	}

	mimo_mode = get_rf_mimo_mode(priv);

	//TX spatial stream support
	if(mimo_mode == MIMO_2T2R || mimo_mode == MIMO_2T3R || mimo_mode == MIMO_2T4R || mimo_mode == MIMO_4T4R)
		val |= BIT6;
	if(mimo_mode == MIMO_3T3R || mimo_mode == MIMO_3T4R || mimo_mode == MIMO_4T4R)
		val |= BIT7;

	//RX spatial stream support
	if(mimo_mode == MIMO_1T2R || mimo_mode == MIMO_2T2R || mimo_mode == MIMO_2T4R || mimo_mode == MIMO_3T4R || mimo_mode == MIMO_4T4R)
		val |= BIT4;
	if(mimo_mode == MIMO_3T3R || mimo_mode == MIMO_2T3R || mimo_mode == MIMO_4T4R)
		val |= BIT5;
	
	//Short GI Support for 20MHz
	if(priv->pmib->dot11nConfigEntry.dot11nShortGIfor20M)
		val |= BIT3;

	//Short GI Support for 40MHz
	if(	(priv->pshare->is_40m_bw == 1) || (priv->pshare->is_40m_bw == 2) ||
		(priv->pmib->dot11nConfigEntry.dot11nUse40M==1) || (priv->pmib->dot11nConfigEntry.dot11nUse40M==2)){
		if(priv->pmib->dot11nConfigEntry.dot11nShortGIfor40M)
			val |= BIT2;
	}
	
	//HT support for 40MHz
	if( ((priv->pshare->is_40m_bw == 1) || (priv->pshare->is_40m_bw == 2)) && priv->pmib->dot11nConfigEntry.dot11nUse40M)
		val |= BIT1;

	*(result_buf+offset) = 0x86;
	offset++;

	*(result_buf+offset) = 7;
	offset++;
	
	memcpy(result_buf+offset, GET_MY_HWADDR, MACADDRLEN);
	offset += MACADDRLEN;

	*(result_buf+offset) = val;
	offset++;
	
	*len += offset;

}

void getVHTApCapability(struct rtl8192cd_priv *priv, unsigned char *result_buf, unsigned int *len){
	//int len = -1;
	unsigned char val = 0, val2 = 0;
	unsigned int mimo_mode=0, offset=0;
#ifdef RTK_AC_SUPPORT
	struct vht_cap_elmt	*vht_cap;
#endif

	if(!(priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)||
		priv->pmib->dot11RFEntry.phyBandSelect == PHY_BAND_2G){
		*(result_buf+offset) =  0x87;
		offset++;

		*(result_buf+offset) = 0;
		offset++;

		*len += offset;

		return;
	}

	mimo_mode = get_rf_mimo_mode(priv);

	//TX spatial stream support
	if(mimo_mode == MIMO_2T2R || mimo_mode == MIMO_2T3R || mimo_mode == MIMO_2T4R || mimo_mode == MIMO_4T4R)
		val |= BIT5;
	if(mimo_mode == MIMO_3T3R || mimo_mode == MIMO_3T4R || mimo_mode == MIMO_4T4R)
		val |= BIT6;

	//RX spatial stream support
	if(mimo_mode == MIMO_1T2R || mimo_mode == MIMO_2T2R || mimo_mode == MIMO_2T4R || mimo_mode == MIMO_3T4R || mimo_mode == MIMO_4T4R)
		val |= BIT2;
	if(mimo_mode == MIMO_3T3R || mimo_mode == MIMO_2T3R || mimo_mode == MIMO_4T4R)
		val |= BIT3;

#ifdef RTK_AC_SUPPORT
	//Short GI Support for 80MHz
	if(priv->pshare->is_40m_bw == HT_CHANNEL_WIDTH_AC_80 && priv->pmib->dot11nConfigEntry.dot11nShortGIfor80M)
		val |= BIT1;

	//Short GI Support for 160MHz and 80+80 MHz
	//	val |= BIT0;


	//VHT Support for 80+80 MHz
	//val2 |= BIT7;

	//VHT Support for 160 MHz
	if(priv->pshare->CurrentChannelBW == HT_CHANNEL_WIDTH_AC_160)
		val2 |= BIT6;
#endif

	//SU Beamformer Capable
#if (BEAMFORMING_SUPPORT == 1)
	if(priv->pmib->dot11RFEntry.txbf == 1 && priv->pmib->dot11RFEntry.txbfer == 1)
		val2 |= BIT5;
#endif

	//MU Beamformer Capable
#if (MU_BEAMFORMING_SUPPORT == 1)
	if(priv->pmib->dot11RFEntry.txbf_mu == 1 && priv->pmib->dot11RFEntry.txbfer == 1)
		val2 |= BIT4;
#endif
	*(result_buf+offset) = 0x87;
	offset++;

	*(result_buf+offset) = 12;
	offset++;

	memcpy(result_buf+offset, GET_MY_HWADDR, MACADDRLEN);
	offset += MACADDRLEN;

#ifdef RTK_AC_SUPPORT
	vht_cap = &priv->vht_cap_buf;
	
	//vht tx MCS
	memcpy(result_buf+offset, &vht_cap->vht_support_mcs[1], 2);
	offset += 2;

	//vht rx MCS
	memcpy(result_buf+offset, &vht_cap->vht_support_mcs[0], 2);
	offset += 2;
#else
	//vht tx MCS
	memcpy(result_buf+offset, 0, 2);
	offset += 2;

	//vht rx MCS
	memcpy(result_buf+offset, 0, 2);
	offset += 2;
#endif

	memcpy(result_buf+offset, &val, 1);
	offset++;
	
	memcpy(result_buf+offset, &val2, 1);
	offset++;

	*len += offset;
	
//	return len;
}

int rtk_multi_ap_getApCapability(struct rtl8192cd_priv *priv, unsigned char* result_buf2){

	int len = 0;
	unsigned char buf[256] = {0};
	//unsigned char *result_buf;

	if(*result_buf2 == 0)
		getApCapability(priv, buf, &len);
	else if(*result_buf2 == 1)
		getHTApCapability(priv, buf, &len);
	else if(*result_buf2 == 2)
		getVHTApCapability(priv, buf, &len);
	else
		return len;
	
	memcpy(result_buf2, buf, len);

	return len;
}


#if (EASYMESH_VERSION >= MULTI_AP_VERSION(3,0,0))
int rtk_multi_ap_set_service_prioritization_rule(struct rtl8192cd_priv *priv, unsigned char *tlv_buffer, const unsigned short limit, int *processed_bytes)
{
	unsigned short *tlv_length;
	unsigned char   add_remove;
	unsigned char   output;
	unsigned char   always_match;

	struct rtl8192cd_priv *root_priv;

	if ((*processed_bytes + 3) > limit)
		goto abort;
	tlv_length = (unsigned short *)(tlv_buffer + 1);
	if (ntohs(*tlv_length) > limit)
		goto abort;
	*processed_bytes += 3;

	if ((*processed_bytes + 5) > limit)
		goto abort;
	add_remove = (*(tlv_buffer + 7)) & BIT7;
	*processed_bytes += 5;
	if ((*processed_bytes + 2) > limit)
		goto abort;
	output = *(tlv_buffer + 9);
	*processed_bytes += 2;

	if ((*processed_bytes + 1) > limit)
		goto abort;
	always_match = (*(tlv_buffer + 10)) & BIT7;
	*processed_bytes += 1;

	root_priv = GET_ROOT(priv);

	if (__rtk_multi_ap_set_service_prioritization_rule_apply(root_priv, add_remove, output, always_match))
		return -1;
	return 0;
abort:
	printk("[ERROR] 8192fe abort service prioritization tlv set. \n");
	return -ENOMEM;
}

int __rtk_multi_ap_set_service_prioritization_rule_apply(struct rtl8192cd_priv *priv, unsigned char add_remove, unsigned char output, unsigned char always_match)
{
	if (add_remove) {
		if (always_match) {
			priv->service_priority_enabled = 1;
			priv->service_priority_output  = output;
			printk("[8192fe] Service prioritization set successfully, output = %x. \n", priv->service_priority_output);
		}
	} else {
		if (always_match) {
			priv->service_priority_enabled = 0;
			//lazy execution for output.
			printk("[8192fe] Service prioritization removed. \n");
		}
	}
	return 0;
}

#define DSCP_TABLE_LENGTH 64
int rtk_multi_ap_set_dscp_mapping_table(struct rtl8192cd_priv *priv, unsigned char *tlv_buffer, const unsigned short limit, int *processed_bytes)
{
	unsigned short *tlv_length;
	struct rtl8192cd_priv *root_priv;

	if ((*processed_bytes + 3) > limit)
		goto abort;
	tlv_length = (unsigned short *)(tlv_buffer + 1);
	if (ntohs(*tlv_length) > limit)
		goto abort;
	*processed_bytes += 3;

	if ((*processed_bytes + 64) > limit)
		goto abort;
	root_priv = GET_ROOT(priv);;
	memcpy(root_priv->dscp_pcp_table, tlv_buffer + 3, DSCP_TABLE_LENGTH);
	*processed_bytes += 64;
	root_priv->dscp_pcp_table_enabled = 1;

	return 0;

abort:
	printk("[ERROR] abort dscp mapping table tlv set. \n");
	return -ENOMEM;
}
#endif // (EASYMESH_VERSION >= MULTI_AP_VERSION(3,0,0))

int rtk_multi_ap_get_general(struct rtl8192cd_priv *priv, unsigned char *tlv_buffer, const unsigned short limit, int *processed_bytes)
{
	int ret = -1;
	// switch(*tlv_buffer) {
	// case xxx:
	//	ret = rtk_multi_ap_get_xxx(struct rtl8192cd_priv *priv, unsigned char *tlv_buffer, const unsigned short limit, int *processed_bytes);
	// 	break;
	// }

	return ret;
}

int rtk_multi_ap_set_general(struct rtl8192cd_priv *priv, unsigned char *tlv_buffer, const unsigned short limit, int *processed_bytes)
{
	int ret = -1;
	switch (*tlv_buffer) {
#ifdef RTK_MULTI_AP_R3
	case TLV_TYPE_SERVICE_PRIORITIZATION_RULE:
		ret = rtk_multi_ap_set_service_prioritization_rule(priv, tlv_buffer, limit, processed_bytes);
		break;
	case TLV_TYPE_DSCP_MAPPING_TABLE:
		ret = rtk_multi_ap_set_dscp_mapping_table(priv, tlv_buffer, limit, processed_bytes);
		break;
#endif
	default:
		printk("[Error] 8192fe driver does not implement map set general id: %u \n", *tlv_buffer);
		break;
	}
	return ret;
}

/*
 *	current format is:
 *INPUT
 *		Field:          			Length:				Value:
 *		payload length				2 octets			payload length
 *		payload						6 octets			station mac
 *OUTPUT
 *		Field:          			Length:				Value:
 *		payload length				2 octets			k
 *		payload         			6 octets			interface mac address
 *		payload         			6 octets			neighbor interface mac address
 *		payload         			4 octets			measure window
 *		payload         			4 octets			tx packet ok
 *		payload         			4 octets			tx packet errors
 *		payload         			2 octets			tx max xput
 *		payload         			2 octets			tx phy rate
 *		payload         			2 octets			tx link availability
 *		payload         			4 octets			rx packet ok
 *		payload         			4 octets			rx packet errors
 *		payload         			1 octets			rx rssi
 *		Total size: 2+6+6+4+4+4+2+2+2+4+4+1 = 41
 */
int rtk_multi_ap_get_link_metric(struct rtl8192cd_priv *priv, unsigned char *tlv_buffer, const unsigned short limit, int *processed_bytes)
{
	int ret = -1;
	u8 sta_mac[6];
	u16 offset       = 0;
	u16 payload_len  = 0;
	u16 u16temp      = 0;
	u32 u32temp      = 0;
	struct stat_info *psta = NULL;

	memcpy(sta_mac, &tlv_buffer[2], 6);
	psta = get_stainfo(priv, sta_mac);

	memset(tlv_buffer, 0, limit);
	offset += 2; //reserve for payload len

	if (psta) {
		memcpy((tlv_buffer + offset), GET_MY_HWADDR, MACADDRLEN);
		offset += 6;

		memcpy((tlv_buffer + offset), sta_mac, MACADDRLEN);
		offset += 6;

		u32temp = 0;
		memcpy((tlv_buffer + offset), &u32temp, 4);
		offset += 4;

		memcpy((tlv_buffer + offset), &psta->tx_pkts, 4);
		offset += 4;

		memcpy((tlv_buffer + offset), &psta->tx_fail, 4);
		offset += 4;

		u16temp = ((psta->current_tx_rate > 0xffff) ? 0xffff : (psta->current_tx_rate & 0xffff));
		memcpy((tlv_buffer + offset), &u16temp, 2);
		offset += 2;
		memcpy((tlv_buffer + offset), &u16temp, 2);
		offset += 2;

		u16temp = 100;
		memcpy((tlv_buffer + offset), &u16temp, 2);
		offset += 2;

		memcpy((tlv_buffer + offset), &psta->rx_pkts, 4);
		offset += 4;

		u32temp = 0;
		memcpy((tlv_buffer + offset), &u32temp, 4);
		offset += 4;

		*(tlv_buffer + offset) = psta->rssi;
		offset++;
	}

	payload_len = offset -2;
	memcpy(tlv_buffer, &payload_len, 2);
	*processed_bytes = offset;
	ret = 0;
/*
	if(payload_len) {
		for(i = 0; i < 41; i++) {
			printk("%02x ", tlv_buffer[i]);
		}
	}
*/
	return ret;
}

int rtk_multi_ap_getApHECapability(struct rtl8192cd_priv *priv, unsigned char** result_buf2){
	int len = -1;

	/*
	802.11ax no support for high efficiency
	*/
	
	return len;
}

int rtk_multi_ap_getClientCapability(struct rtl8192cd_priv *priv, unsigned char* result){

	int len = 0;
	struct stat_info *pstat;
	unsigned char macaddr[6];
	unsigned short u8_max_len = 256;

	memcpy(macaddr, result, MACADDRLEN);
	pstat = get_stainfo(priv, macaddr);	

	if (pstat) {
		result[0] = 0;
		if (pstat->assoc_req_length > u8_max_len) {
			result[1] = 0;
			memcpy(&result[2], &pstat->assoc_req_length, sizeof(pstat->assoc_req_length));
			memcpy(&result[4], pstat->assoc_req_body, pstat->assoc_req_length);
			len = (pstat->assoc_req_length + 4);
		} else {
			result[1] = pstat->assoc_req_length;
			memcpy(&result[2], pstat->assoc_req_body, pstat->assoc_req_length);
			len = (pstat->assoc_req_length + 2);
		}
	} else {
		result[0] = 1;
		result[1] = 0;
		len = 2;
	}
	return len;
}


#define TLV_TYPE_AP_METRICS                           (148) // 0x94

const int MCS_DATA_RATEFloat_2[2][2][16] =
{
	{{6.5, 13, 19.5, 26, 39, 52, 58.5, 65, 13, 26, 39, 52, 78, 104, 117, 130},						  // Long GI, 20MHz
	 {7.2, 14.4, 21.7, 28.9, 43.3, 57.8, 65, 72.2, 14.4, 28.9, 43.3, 57.8, 86.7, 115.6, 130, 144.5}}, // Short GI, 20MHz
	{{13.5, 27, 40.5, 54, 81, 108, 121.5, 135, 27, 54, 81, 108, 162, 216, 243, 270},                  // Long GI, 40MHz
	 {15, 30, 45, 60, 90, 120, 135, 150, 30, 60, 90, 120, 180, 240, 270, 300}}                        // Short GI, 40MHz
};


void getAPMetric(struct rtl8192cd_priv *priv, unsigned char *buf, int *len){

	int data_len = 25;
	unsigned short tlv_len = 22;
	unsigned short sta_nr = 0;
	unsigned char esp_ie = 0;
	unsigned char *p;

	p = buf;

	//TLV Type
	*p = TLV_TYPE_AP_METRICS;
	p++;

	//Length
	memcpy(p, &tlv_len, sizeof(unsigned short));
	p+= 2;

	//The BSSID of the BSS
	
	memcpy(p, BSSID, MACADDRLEN);
	p += 6;

	// The Channel utilization
	*p = GET_ROOT(priv)->ext_stats.ch_utilization;
	p++;
	
	//The Number of STAs current associated with this BSS
	sta_nr = priv->assoc_num;
	memcpy(p, &sta_nr, sizeof(unsigned short));
	p+= 2;

	//Estimated Service Parameter Information Field
	esp_ie |= BIT7;

	*p = esp_ie;
	p++;

	//ESP AC=BE
	memset(p, 0, 3);
	p += 3;

	//ESP AC=BK
	memset(p, 0, 3);
	p += 3;	

	//ESP AC=VO
	memset(p, 0, 3);
	p += 3;	

	//ESP AC=VI
	memset(p, 0, 3);
	p += 3;	

	*len = data_len;
}

#define TLV_TYPE_ASSOCIATED_STA_LINK_METRICS          (150) // 0x96

void getAssocStaLinkMetric(struct rtl8192cd_priv *priv, unsigned char *buf, int *len){

	int data_len = 0;
	unsigned short tlv_len = 0;
	unsigned char sta_mac[6] = {0};
	unsigned char *p;
	int tx_rate=0, rx_rate=0;

	struct stat_info *pstat;

	memcpy(sta_mac, &buf[1], MACADDRLEN);
	//printk("%s - %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
	pstat = get_stainfo(priv, sta_mac);
	
	if(pstat)
	{
		p = buf;
		
		*p = TLV_TYPE_ASSOCIATED_STA_LINK_METRICS;
		
		p += 3; //Offset for TLV Type and Length

		memcpy(p, pstat->cmn_info.mac_addr, MACADDRLEN);
		p += 6;

		*p = 1; //Number of bssid reported for this STA
		p++;

		memcpy(p, BSSID, MACADDRLEN);
		p += 6;

		memset(p, 0, 4); //time delta
		p += 4;

#ifdef RTK_AC_SUPPORT
		if(pstat->current_tx_rate >= 0x90)
		{
			tx_rate = query_vht_rate(pstat);
			rx_rate = query_vht_rate(pstat);
		}
		else
#endif
		if (is_MCS_rate(pstat->current_tx_rate)) 
			tx_rate = MCS_DATA_RATEFloat_2[(pstat->ht_current_tx_info&BIT(0))?1:0][(pstat->ht_current_tx_info&BIT(1))?1:0][pstat->current_tx_rate&0xf];			
		else
			tx_rate = pstat->current_tx_rate/2;
			
		//rx rate
#ifdef RTK_AC_SUPPORT
		if(pstat->rx_rate >= 0x90)
			rx_rate = query_vht_rate(pstat);
		else
#endif
		if (is_MCS_rate(pstat->rx_rate))			
			rx_rate = MCS_DATA_RATEFloat_2[pstat->rx_bw&0x01][pstat->rx_splcp&0x01][pstat->rx_rate&0xf];
		else		
			rx_rate = pstat->rx_rate/2;
					
		memcpy(p, &tx_rate, 4); //tx rate
		p += 4;

		memcpy(p, &rx_rate, 4); //rx rate
		p += 4;

		*p = rssiToRcpi(pstat->rssi); //rssi converted to rcpi
		p++;

		data_len = 26;
		tlv_len = data_len;
		
		memcpy(&buf[1], &tlv_len, sizeof(unsigned short));

		*len = (data_len + 3); //For TLV and length
	}
	else{
		printk("%s - STA %02x%02x%02x%02x%02x%02x not found\n", __FUNCTION__,
			sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
		memset(buf, 0, 29);
		*len = 29;
	}
}

#define TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS         (162) //0xA2

void getAssocStaTrafficStats(struct rtl8192cd_priv *priv, unsigned char *buf, int *len){
		int data_len = 0;
		unsigned short tlv_len = 0;
		unsigned char sta_mac[6] = {0};
		unsigned char *p;
		struct stat_info *pstat;
	
		memcpy(sta_mac, &buf[1], MACADDRLEN);
		//printk("%s - %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
		pstat = get_stainfo(priv, sta_mac);
		if (pstat)
		{
			p = buf;
		
			*p = TLV_TYPE_ASSOCIATED_STA_TRAFFIC_STATS;
		
			p += 3;	//Offset for TLV Type and Length
	
			memcpy(p, pstat->cmn_info.mac_addr, MACADDRLEN);
			p += 6;
	
			memcpy(p, &pstat->tx_bytes, 4); //tx bytes
			p+=4;
	
			memcpy(p, &pstat->rx_bytes, 4); //rx bytes
			p+=4;
	
			memcpy(p, &pstat->tx_pkts, 4); //tx pkts
			p+=4;
	
			memcpy(p, &pstat->rx_pkts, 4); //rx pkts
			p+=4;
	
			memcpy(p, &pstat->tx_fail, 4); //tx fail
			p+=4;
	
			memset(p, 0x00, 4);		 //rx fail
			p+=4;
	
			memset(p, 0x00, 4);		 //retransmission
			p+=4;
		
			data_len = 34;
			tlv_len = data_len;
			
			memcpy(&buf[1], &tlv_len, sizeof(unsigned short));
	
			*len = (data_len + 3); //For TLV and length

		}
		else{
			memset(buf, 0, 37);
			*len = 37;
		}

}

#define TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE (152) // 0x98

void rtk_multi_ap_trigger_unassocStaMetric(unsigned long task_priv){
	
	//send the bssid of the triggerred ch util
	int i, j, sta_nr = 0, data_len = 0;
	unsigned short tlv_len = 0;
	unsigned char send_buf[1024] = { 0 };
	unsigned char channel_number;
	unsigned char *p;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		priv->multiap_unassocSta_ongoing = 0;
		return;
	}

	channel_number = priv->multiap_measure_channel;

	send_buf[0] = TLV_TYPE_UNASSOCIATED_STA_LINK_METRICS_RESPONSE;
	//send_buf[1] = 0;
	send_buf[3] = priv->multiap_measure_opClass; 
	send_buf[4] = 0; //sta number

	data_len = 2;
	
	p = &send_buf[5];

	for (i=0; i < priv->multiap_unassocStaEntryOccupied && i<MAX_PROBE_REQ_STA; i++) {
		if (priv->multiap_unassoc_sta[i].used && priv->multiap_unassoc_sta[i].rssi != 0) {
			memcpy(p, priv->multiap_unassoc_sta[i].addr, MACADDRLEN);
			p += 6;

			*p = channel_number;
			p++;

			memset(p, 0 , 4);
			p+= 4;
			
			*p = rssiToRcpi(priv->multiap_unassoc_sta[i].rssi); //(priv->multiap_unassoc_sta[i].rssi << 1);
			p++;

			sta_nr++;
			data_len += 12;
		} else if(priv->multiap_unassoc_sta[i].used) {
			for (j=0; j<priv->ProbeReqEntryOccupied; j++) {
				if(memcmp(priv->multiap_unassoc_sta[i].addr, priv->probe_sta[j].addr, MACADDRLEN) == 0){
					memcpy(p, priv->multiap_unassoc_sta[i].addr, MACADDRLEN);
					p += 6;

					*p = channel_number;
					p++;

					memset(p, 0 , 4);
					p+= 4;
					
					*p = (priv->probe_sta[j].rssi << 1);
					p++;

					sta_nr++;
					data_len += 12;
					break;
				}
			}
		}
	}

	priv->multiap_unassocStaEntryNum = 0;
	priv->multiap_unassocStaEntryOccupied = 0;
	memset(priv->multiap_unassoc_sta, 0, (sizeof(struct sta_mac_rssi)* MAX_PROBE_REQ_STA));

	tlv_len = data_len;
	memcpy(&send_buf[1], &tlv_len, sizeof(unsigned short));
	//printk("%s - sta nr: %d\n", __FUNCTION__, sta_nr);
	send_buf[4] = sta_nr;

	data_len += 3;

	rtk_multi_ap_nl_send(send_buf, data_len);
	priv->multiap_unassocSta_ongoing = 0;
	// RTL_W32(RCR, RTL_R32(RCR) & ~RCR_AAP); // Disable Accept Destination Address packets

}

int rtk_multi_ap_recordUnassocStaMetric(struct rtl8192cd_priv *priv, unsigned char *buf){

	int i, sta_nr;
	unsigned char *p;

	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		return 1;
	}

	if(priv->multiap_unassocSta_ongoing){
		printk("%s - measurement still ongoing\n", __FUNCTION__);
		return 1;
	}

	// RTL_W32(RCR, RTL_R32(RCR) | RCR_AAP); //Accept Destination Address packets

	priv->multiap_measure_opClass = buf[0];
	priv->multiap_measure_channel = buf[1];
	sta_nr = buf[2];

	if(sta_nr > 0)
		p = &buf[3];

	//if op class and channel is same
	if(priv->multiap_measure_channel == 0 || (priv->pmib->dot11RFEntry.dot11channel == priv->multiap_measure_channel)){
		//add into the unassoc monitor list
		for(i = 0; i < sta_nr; i++){
			add_unassoc_MAC_RSSI_Entry(priv, p, 0, priv->multiap_unassoc_sta, &priv->multiap_unassocStaEntryOccupied, &priv->multiap_unassocStaEntryNum);
			p += 6;
		}
	}

	//set timer to trigger update
	if(priv->multiap_unassoc_sta_timer.function != NULL){
		mod_timer(&priv->multiap_unassoc_sta_timer ,jiffies + RTL_SECONDS_TO_JIFFIES(7));
		priv->multiap_unassocSta_ongoing = 1;
	}
	return 1;
}

#define TLV_TYPE_BEACON_METRICS_RESPONSE              (154)

#if defined(DOT11K) ||defined(HAPD_11K)
void rtk_multi_ap_beaconMetricsResponseNotify(struct stat_info *pstat){

	int data_len = 0;
	unsigned short tlv_len = 0;
	//unsigned char send_buf[2048] = { 0 };
	unsigned char* send_buf;
	unsigned char i, bm_report_info_size, beacon_report_nr;
	unsigned char *pbuf;

	send_buf = (unsigned char*)kmalloc(2048*sizeof(unsigned char),GFP_ATOMIC);
	if(send_buf == NULL){
		printk("[%s:%d]kmalloc failed!\n", __FUNCTION__,__LINE__);
		return;
	}
	memset(send_buf, 0, 2048*sizeof(unsigned char));

	send_buf[0] = TLV_TYPE_BEACON_METRICS_RESPONSE;

	bm_report_info_size = sizeof(struct dot11k_beacon_measurement_report_info);
	beacon_report_nr = pstat->rm.beacon_report_num - pstat->rm.beacon_report_start_index;

	memcpy(&send_buf[3], pstat->cmn_info.mac_addr, MACADDRLEN);
	
	send_buf[9] = 0; //Reserved

	send_buf[10] = beacon_report_nr;
	
	data_len += 8;

	pbuf = &send_buf[11];
	
	for(i = pstat->rm.beacon_report_start_index; i < pstat->rm.beacon_report_num; i++){
		if(data_len + 5 + bm_report_info_size > 2048)
			break;
		*pbuf = _MEASUREMENT_REPORT_IE_;
		pbuf++;
		data_len++;

		*pbuf = pstat->rm.beacon_report_len[i];
		pbuf++;
		data_len++;

		*pbuf = pstat->rm.beacon_measurement_token[i];
		pbuf++;
		data_len++;

		*pbuf = pstat->rm.beacon_report_mode[i];
		pbuf++;
		data_len++;

		*pbuf = MEASUREMENT_TYPE_BEACON; //BEACON MEASUREMENT TYPE
		pbuf++;
		data_len++;

		if(pstat->rm.measure_result == MEASUREMENT_INCAPABLE || pstat->rm.measure_result == MEASUREMENT_REFUSED){
			break;
		}

		memcpy(pbuf, &(pstat->rm.beacon_report[i].info), bm_report_info_size);
		pbuf += bm_report_info_size;
		data_len += bm_report_info_size;

		if(pstat->rm.beacon_report_len[i] > 29
			&& data_len + pstat->rm.beacon_report[i].subelements_len <= 2048){
			memcpy(pbuf, pstat->rm.beacon_report[i].subelements, pstat->rm.beacon_report[i].subelements_len);
			pbuf += pstat->rm.beacon_report[i].subelements_len;
			data_len += pstat->rm.beacon_report[i].subelements_len;
		}
/*
		*pbuf = pstat->rm.beacon_report[i].subelements_len;
		pbuf++;
		data_len++;

		if(pstat->rm.beacon_report[i].subelements_len > 0){
			memcpy(pbuf, &(pstat->rm.beacon_report[i].subelements[2]), (pstat->rm.beacon_report[i].subelements_len-2));
			pbuf += pstat->rm.beacon_report[i].subelements_len;
			data_len += pstat->rm.beacon_report[i].subelements_len;
		}
*/		
	}

	tlv_len = data_len;
	memcpy(&send_buf[1], &tlv_len, sizeof(unsigned short));

	data_len += 3;

	rtk_multi_ap_nl_send(send_buf, data_len);

	if(pstat->rm.beacon_report_num >= MAX_BEACON_REPORT) {
		pstat->rm.beacon_report_num = 0;
	}
	pstat->rm.beacon_report_start_index = pstat->rm.beacon_report_num;
	kfree((void*)send_buf);
}
#endif

#define TLV_TYPE_BACKHAUL_STEERING_RESPONSE           (159) // 0x9F

void rtk_multi_ap_updateBackhaulSteerResults(unsigned long task_priv){
	
	//send the backhaul results
	int data_len = 0, result_code = 0;
	unsigned short tlv_len = 0;
	unsigned char send_buf[256] = { 0 };
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	struct stat_info *pstat;
	struct list_head *phead, *plist;
	unsigned long flags=0;

	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		return;
	}

	if(!IS_VXD_INTERFACE(priv) || priv->assoc_num < 1){
		//Not vxd interface && not associated
		printk("%s - %s interface not connected!\n", __FUNCTION__, priv->dev->name);
		priv->multiap_bssSteer_scan = 0;
		return;
	}
	priv->multiap_bssSteer_scan = 0;
	priv->multiap_bssSteer_channel = 0;
	memcpy(&send_buf[3], GET_MY_HWADDR, MACADDRLEN);

	SAVE_INT_AND_CLI(flags);
    SMP_LOCK_ASOC_LIST(flags);
	phead = &priv->asoc_list;
    plist = phead->next;
	pstat = list_entry(plist, struct stat_info, asoc_list);
	
	memcpy(&send_buf[9], priv->multiap_target_bssid, MACADDRLEN);

	if(memcmp(pstat->cmn_info.mac_addr, priv->multiap_target_bssid, MACADDRLEN) == 0)
		result_code = 0x00;
	else
		result_code = 0x01;
	
	SMP_UNLOCK_ASOC_LIST(flags);
    RESTORE_INT(flags);

	send_buf[0] = TLV_TYPE_BACKHAUL_STEERING_RESPONSE; //backhaul steer update id
	send_buf[15] = result_code;
	
	data_len = 13;
	tlv_len = data_len;
	memcpy(&send_buf[1], &tlv_len, sizeof(unsigned short));

	data_len += 3;

	rtk_multi_ap_nl_send(send_buf, data_len);

}

#ifdef CONFIG_IEEE80211V
int rtk_multi_ap_doBackhaulSteer(struct rtl8192cd_priv *priv, unsigned char *tmpbuf){

	unsigned char target_bssid[6] = {0}, backhaul_bss[6] = {0};
	unsigned char op_class, channel;
	struct stat_info *pstat;
	int i = 0, ret = 0;
	
	struct list_head *phead, *plist;
	unsigned long flags=0;

	if(!IS_VXD_INTERFACE(priv) || priv->assoc_num < 1 || priv->multiap_bssSteer_scan){
		//Not vxd interface && not associated
		return 0;
	}
	
	memcpy(backhaul_bss, tmpbuf, MACADDRLEN);
	memcpy(target_bssid, tmpbuf+6, MACADDRLEN);
	
	if(memcmp(backhaul_bss, GET_MY_HWADDR, MACADDRLEN) != 0){
		printk("Wrong bss - %02x%02x%02x%02x%02x%02x\n", backhaul_bss[0], backhaul_bss[1], backhaul_bss[2],
			backhaul_bss[3], backhaul_bss[4], backhaul_bss[5]);
		printk("Own bss - %02x%02x%02x%02x%02x%02x\n", priv->pmib->dot11OperationEntry.hwaddr[0], priv->pmib->dot11OperationEntry.hwaddr[1], priv->pmib->dot11OperationEntry.hwaddr[2],
			priv->pmib->dot11OperationEntry.hwaddr[3], priv->pmib->dot11OperationEntry.hwaddr[4], priv->pmib->dot11OperationEntry.hwaddr[5]);
		goto error;
	}

	memset(priv->multiap_target_bssid, 0, MACADDRLEN);

	op_class = tmpbuf[12];
	channel = tmpbuf[13];

	//check for valid channel
	for (i=0; i<priv->available_chnl_num; i++){
		//if (priv->pmib->dot11RFEntry.dot11channel == priv->available_chnl[i]){
		if (channel == priv->available_chnl[i]){
			ret = 1;
			break;
		}
	}

	if(ret < 1){
		printk("Not valid channel\n");
		goto error;
	}

	//TODO: maybe we can add a check on last roaming done to prevent to frequent roaming

	priv->multiap_bssSteer_scan = 1;
	priv->multiap_bssSteer_channel = channel;

	//save the target bssid
	memcpy(priv->multiap_target_bssid, target_bssid , MACADDRLEN);
	
	// set the roaming target
	memcpy(priv->pmib->dot11StationConfigEntry.dot11DesiredBssid , target_bssid, MACADDRLEN);
	
	start_clnt_lookup(priv, RESCAN_ROAMING);

    SAVE_INT_AND_CLI(flags);
    SMP_LOCK_ASOC_LIST(flags);
	phead = &priv->asoc_list;
    plist = phead->next;
	pstat = list_entry(plist, struct stat_info, asoc_list);
	SMP_UNLOCK_ASOC_LIST(flags);
    RESTORE_INT(flags);
	
#if 0
	//This is under assumption that vxd interface will only have 1 sta_info (remote AP)
    while (plist != phead) {  
		plist = plist->next;
    }
#endif

	if(pstat){
		if(!memcmp(pstat->cmn_info.mac_addr, target_bssid, MACADDRLEN)){
			//already associated with target, do nothing
			printk("Already associated with target bssid\n");
		}
		else{
			del_station(priv, pstat, DEL_STA_SEND_DISASSOC, _RSON_DISASSOC_DUE_BSS_TRANSITION);
#if 0			
			for(i=priv->join_index+1; i<priv->site_survey->count_target; i++) {
				if(!memcmp(target_bssid, priv->site_survey->bss_target[i].bssid, MACADDRLEN)) {
					
					// diassoc the present connecting AP
					issue_disassoc(priv, pstat->cmn_info.mac_addr, _RSON_DISASSOC_DUE_BSS_TRANSITION);
					del_station(priv, pstat, 0);

					printk("Found target bssid in ss list\n");
					ret = 1;

					break;
				}
			}
			if(ret < 1){
				//cannot find target
				printk("Target bssid not found, attempt manual\n");
				issue_disassoc(priv, pstat->cmn_info.mac_addr, _RSON_DISASSOC_DUE_BSS_TRANSITION);
				del_station(priv, pstat, 0);
			}
#endif			
		}
	}
	else{
		//not associated to any remote AP
		printk("Not associated to any remote AP\n");
	}
	
error:
	ret = 1;
	mod_timer(&priv->multiap_backhaul_steer_timer,jiffies + RTL_SECONDS_TO_JIFFIES(10));
	return ret;	
}
#endif

#define TLV_TYPE_METRIC_REPORT_POLICY                 (138) // 0x8A

void rtk_multi_ap_ch_util_trigger(struct rtl8192cd_priv *priv){

	//send the bssid of the triggerred ch util
	int data_len = 0;
	unsigned char send_buf[256] = { 0 };
	unsigned char ch_utilization;
	unsigned char ch_threshold;

	ch_utilization = ODMPTR->dm_ccx_info.clm_ratio;
	if(ch_utilization >= 100)
		ch_utilization = 255;
	else
		ch_utilization = ch_utilization*255/100;
	ch_threshold = priv->pmib->multi_ap.multiap_cu_threshold;

	//If the latest channel utilization has crossed the ch utilization threshold with respect to last measured
	if(((priv->multiap_last_cu < ch_threshold) && (ch_utilization >= ch_threshold)) ||
		((priv->multiap_last_cu >= ch_threshold) && (ch_utilization < ch_threshold)) ||
		(ch_threshold != 0 && ch_utilization > (priv->multiap_last_cu + (10 * ch_threshold)))){
		send_buf[0] = TLV_TYPE_METRIC_REPORT_POLICY;
		send_buf[1] = 3;	//ch util trigger
		memcpy(&send_buf[2], BSSID, MACADDRLEN);

		data_len += 8;

		rtk_multi_ap_nl_send(send_buf, data_len);
		priv->multiap_last_cu = ch_utilization;
	}
}

void rtk_multi_ap_sta_rssi_trigger(struct rtl8192cd_priv *priv, struct stat_info *pstat)
{

	int           data_len      = 0;
	unsigned char send_buf[256] = { 0 };
	unsigned char current_rssi;
	unsigned char rssi_threshold, hysteresis_margin;

	current_rssi = pstat->rssi;
	if (priv->pmib->multi_ap.multiap_report_rcpi_threshold < 20) {
		rssi_threshold = 0;
	} else {
		rssi_threshold = (priv->pmib->multi_ap.multiap_report_rcpi_threshold / 2 - 10);
	}

	hysteresis_margin = priv->pmib->multi_ap.multiap_report_rcpi_hysteris_margin;

	//If hysteresis margin is not set (0), use value no higher than 5 as recommended by spec
	if (hysteresis_margin == 0) {
		hysteresis_margin = 2;
	}

	if (rssi_threshold) {
		//If the latest station rssi has crossed the rssi threshold with respect to last measured including hysteresis margin
		if (((pstat->last_rssi <= rssi_threshold) && (current_rssi > rssi_threshold) && (current_rssi - pstat->last_rssi >= hysteresis_margin)) || ((pstat->last_rssi >= rssi_threshold) && (current_rssi < rssi_threshold) && (pstat->last_rssi - current_rssi >= hysteresis_margin))) {

			send_buf[0] = TLV_TYPE_METRIC_REPORT_POLICY; //unassoc rssi trigger
			send_buf[1] = 1;

			memcpy(&send_buf[2], pstat->cmn_info.mac_addr, MACADDRLEN);

			data_len = 8;

			rtk_multi_ap_nl_send(send_buf, data_len);
		}
	}

	pstat->last_rssi = pstat->rssi;
}

int rtk_multi_ap_updatePolicy(struct rtl8192cd_priv *priv, unsigned char *tmpbuf)
{

	unsigned char rcpi_threshold, rcpi_hysteris_margin, chUtil_threshold;

	rcpi_threshold = tmpbuf[0];

	rcpi_hysteris_margin = tmpbuf[1];

	chUtil_threshold = tmpbuf[2];

	priv->pmib->multi_ap.multiap_report_rcpi_threshold = rcpi_threshold;

	priv->pmib->multi_ap.multiap_report_rcpi_hysteris_margin = rcpi_hysteris_margin;

	priv->pmib->multi_ap.multiap_cu_threshold = chUtil_threshold;

	return 1;
}

int update_agent_steering_policy(struct rtl8192cd_priv *priv, unsigned char *tmpbuf)
{
	unsigned char steering_policy, chUtil_threshold, rcpi_threshold;

	steering_policy = tmpbuf[0];

	chUtil_threshold = tmpbuf[1];

	rcpi_threshold = tmpbuf[2];

	priv->pmib->multi_ap.multiap_steering_policy = steering_policy;

	priv->pmib->multi_ap.multiap_cu_threshold = chUtil_threshold;

	priv->pmib->multi_ap.multiap_steering_rcpi_threshold = rcpi_threshold;

	return 0;
}

#ifdef CONFIG_IEEE80211V
void rtk_multi_ap_agent_steering_trigger(struct rtl8192cd_priv *priv)
{
	struct stat_info *pstat;
	struct list_head *phead, *plist;
	phead = &priv->asoc_list;
	plist = phead;
	while ((plist = asoc_list_get_next(priv, plist)) != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat->bssTransTriggered != TRUE && (pstat->rssi << 1) < priv->pmib->multi_ap.multiap_steering_rcpi_threshold) {
			send_bss_trans_event(priv, pstat, -1);
		}
		if (plist == plist->next) {
			break;
		}
	}
}
#endif

int rtk_multi_ap_terminateBackhaulApConnection(struct rtl8192cd_priv *priv)
{

	struct stat_info *pstat;
	struct list_head *phead, *plist;
#ifdef SMP_SYNC
	// unsigned long flags;
#endif
	phead = &priv->asoc_list;
	if (list_empty(phead)) {
		return 0;
	}
	printk("%s - Terminate %d connection of backhaul AP!\n", priv->dev->name, priv->assoc_num);

	// SMP_LOCK_ASOC_LIST(flags);
	plist = phead->next;
	while (plist != phead) {
		pstat = list_entry(plist, struct stat_info, asoc_list);
		if (pstat) {
			del_station(priv, pstat, DEL_STA_SEND_DISASSOC, _RSON_UNSPECIFIED_);
		}
		if (plist == plist->next)
			break;
		plist = plist->next;
	}
	// SMP_UNLOCK_ASOC_LIST(flags);

	return 0;
}

int rtk_multi_ap_vxd_send_disassoc(struct rtl8192cd_priv *priv)
{

	struct stat_info *pstat;
	struct list_head *phead, *plist;
#ifdef SMP_SYNC
	unsigned long flags;
	flags = 0;
#endif
	phead = &priv->asoc_list;
	if (list_empty(phead)) {
		return 0;
	}

	// SMP_LOCK_ASOC_LIST(flags);
	plist = phead->next;
	pstat = list_entry(plist, struct stat_info, asoc_list);
	//This is under the assumption there vxd will only have 1 pstat
	if (pstat) {
		del_station(priv, pstat, DEL_STA_SEND_DISASSOC, _RSON_DEAUTH_STA_LEAVING_);
		printk("[MULTI-AP] vxd disassoc sent!\n");
	}

	return 0;
}

int rtk_multi_ap_terminateBackhaulStaConnection(struct rtl8192cd_priv *priv)
{

	struct stat_info *pstat;
	struct list_head *phead, *plist;
#ifdef SMP_SYNC
	unsigned long flags;
	flags = 0;
#endif
	phead = &priv->asoc_list;
	if (list_empty(phead)) {
		return 0;
	}

	// SMP_LOCK_ASOC_LIST(flags);
	plist = phead->next;
	pstat = list_entry(plist, struct stat_info, asoc_list);
	//This is under the assumption there vxd will only have 1 pstat
	if (pstat) {
		del_station(priv, pstat, DEL_STA_SEND_DISASSOC, _RSON_UNSPECIFIED_);
		printk("Terminate connection of backhaul STA!\n");
	}
	// SMP_UNLOCK_ASOC_LIST(flags);

	return 0;
}

void issue_probereq_map(struct rtl8192cd_priv *priv, unsigned char *ssid, int ssid_len, unsigned char *da) {
    //UINT8           meshiearray[32];	// mesh IE buffer (Max byte is mesh_ie_MeshID)
    struct wifi_mib *pmib;
    unsigned char	*hwaddr, *pbuf;
    unsigned char	*pbssrate=NULL;
    int		bssrate_len;
#ifdef DOT11K
    unsigned char val8;
#endif
    DECLARE_TXINSN(txinsn);

#ifdef MP_TEST
    if (priv->pshare->rf_ft_var.mp_specific)
        return;
#endif

    txinsn.retry = priv->pmib->dot11OperationEntry.dot11ShortRetryLimit;
    pmib = GET_MIB(priv);
    hwaddr = pmib->dot11OperationEntry.hwaddr;
    txinsn.q_num = MANAGE_QUE_NUM;
    txinsn.fr_type = _PRE_ALLOCMEM_;

#ifdef P2P_SUPPORT	/*cfg p2p cfg p2p*/
    if(rtk_p2p_is_enabled(priv)){
        txinsn.tx_rate = _6M_RATE_;
    }else
#endif    
        txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
    txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif	
    txinsn.fixed_rate = 1;
    pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);

    if (pbuf == NULL)
        goto issue_probereq_fail;

    txinsn.phdr = get_wlanhdr_from_poll(priv);

    if (txinsn.phdr == NULL)
        goto issue_probereq_fail;

    memset((void *)(txinsn.phdr), 0, sizeof (struct	wlan_hdr));

#ifdef HS2_SUPPORT
#ifdef HS2_CLIENT_TEST
	if ((priv->pshare->rf_ft_var.swq_dbg == 30) || (priv->pshare->rf_ft_var.swq_dbg == 31))
    {
		printk("Probe Request to SSID [Hotspot 2.0]\n");
        strcpy(ssid, "Hotspot 2.0");
        ssid[11] = '\0';
        ssid_len = strlen(ssid);
        pbuf = set_ie(pbuf, _SSID_IE_, ssid_len, ssid, &txinsn.fr_len);
    }
    else if ((priv->pshare->rf_ft_var.swq_dbg == 32) || (priv->pshare->rf_ft_var.swq_dbg == 33) || (priv->pshare->rf_ft_var.swq_dbg == 34) || (priv->pshare->rf_ft_var.swq_dbg == 35) || (priv->pshare->rf_ft_var.swq_dbg == 36) || (priv->pshare->rf_ft_var.swq_dbg == 37) || (priv->pshare->rf_ft_var.swq_dbg == 38) || (priv->pshare->rf_ft_var.swq_dbg == 39))
    {
        pbuf = set_ie(pbuf, _SSID_IE_, 0, ssid, &txinsn.fr_len);
    }
    else
#endif
#endif
	pbuf = set_ie(pbuf, _SSID_IE_, ssid_len, ssid, &txinsn.fr_len);
    /*fill supported rates*/ 

#ifdef P2P_SUPPORT
    if(rtk_p2p_is_enabled(priv)){
        get_bssrate_set(priv, _SUPPORTED_RATES_NO_CCK_, &pbssrate, &bssrate_len);	
    }else
#endif
    {
        get_bssrate_set(priv, _SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len);	
    }


    pbuf = set_ie(pbuf, _SUPPORTEDRATES_IE_ , bssrate_len , pbssrate, &txinsn.fr_len);

    if (get_bssrate_set(priv, _EXT_SUPPORTEDRATES_IE_, &pbssrate, &bssrate_len))
        pbuf = set_ie(pbuf, _EXT_SUPPORTEDRATES_IE_ , bssrate_len , pbssrate, &txinsn.fr_len);

#ifdef P2P_SUPPORT		
	if (rtk_p2p_is_enabled(priv) )
	{		/*add wsc ie*/
		if(pmib->wscEntry.probe_req_ielen){
			memcpy(pbuf, pmib->wscEntry.probe_req_ie, pmib->wscEntry.probe_req_ielen);
			pbuf += pmib->wscEntry.probe_req_ielen;
			txinsn.fr_len += pmib->wscEntry.probe_req_ielen;
		}
	}
	else
#endif
/*

#ifdef WIFI_SIMPLE_CONFIG
	{
		if (pmib->wscEntry.wsc_enable && pmib->wscEntry.probe_req_ielen) {
			memcpy(pbuf, pmib->wscEntry.probe_req_ie, pmib->wscEntry.probe_req_ielen);
			pbuf += pmib->wscEntry.probe_req_ielen;
			txinsn.fr_len += pmib->wscEntry.probe_req_ielen;
		}
	}
#endif


#ifdef P2P_SUPPORT
	if (rtk_p2p_is_enabled(priv) )
	{
		if(priv->p2pPtr->p2p_probe_req_ie_len){
			memcpy(pbuf, priv->p2pPtr->p2p_probe_req_ie, priv->p2pPtr->p2p_probe_req_ie_len);			
			pbuf += priv->p2pPtr->p2p_probe_req_ie_len ;
			txinsn.fr_len += priv->p2pPtr->p2p_probe_req_ie_len ;			
		}
	}	
#endif


#ifdef CONFIG_IEEE80211V_CLI
		if(WNM_ENABLE) {
			extended_cap_ie[2] |= _WNM_BSS_TRANS_SUPPORT_ ;
			pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 8, extended_cap_ie, &txinsn.fr_len);
		 }
#endif
*/
#ifdef HS2_SUPPORT
#ifdef HS2_CLIENT_TEST
	if (priv->pshare->rf_ft_var.swq_dbg == 30)
    {
		// HS2.0 AP does not transmit a probe response frame
		// HESSID is wrong.
		
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x02,0x50,0x6F,0x9A,0x00,0x00,0x01};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 31)
    {
		// HS2.0 AP does not transmit a probe response frame
		// HESSID is wrong.
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x00,0x50,0x6F,0x9A,0x00,0x00,0x01};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 32)
    {
		// APUT transmits Probe Response Message
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x33,0x44,0x55,0x66,0x77}; // HESSID = redir_mac, please refer to next line

		memcpy(&tmp[1], priv->pmib->hs2Entry.redir_mac, 6);

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

		frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 33)
    {
		// APUT transmits Probe Response Message
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x00,0x00,0x33,0x44,0x55,0x66,0x77}; // HESSID = redir_mac, please refer to next line

		memcpy(&tmp[3], priv->pmib->hs2Entry.redir_mac, 6);

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 34)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x03}; // HESSID is not present 

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 1, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 35)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x03,0x00,0x00}; // HESSID is not present

		temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 3, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 36)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x02,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 37)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x02,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
	else if (priv->pshare->rf_ft_var.swq_dbg == 38)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x0f,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 7, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
    else if (priv->pshare->rf_ft_var.swq_dbg == 39)
    {
        unsigned int temp_buf32, buf32 = _INTERWORKING_SUPPORT_BY_DW_, frlen=0;
        unsigned char tmp[]={0x0f,0x00,0x00,0xff,0xff,0xff,0xff,0xff,0xff};

        temp_buf32 = cpu_to_le32(buf32);
        pbuf = set_ie(pbuf, _EXTENDED_CAP_IE_, 4, &temp_buf32, &frlen);
        txinsn.fr_len += frlen;

        frlen = 0;
        pbuf = set_ie(pbuf, _INTERWORKING_IE_, 9, tmp, &frlen);
        txinsn.fr_len += frlen;
    }
#endif
#endif

/*
#ifdef A4_STA
    if(priv->pmib->miscEntry.a4_enable == 2) {
        pbuf = construct_ecm_tvm_ie(priv, pbuf, &txinsn.fr_len, BIT0);
    }
#endif
*/
	SetFrameSubType(txinsn.phdr, WIFI_PROBEREQ);

	//if (da)
	//	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), da, MACADDRLEN); // unicast
	//else
		memset((void *)GetAddr1Ptr((txinsn.phdr)), 0xff, MACADDRLEN); // broadcast
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), hwaddr, MACADDRLEN);
	//nctu note
	// spec define ProbeREQ Address 3 is BSSID or wildcard) (Refer: Draft 1.06, Page 12, 7.2.3, Line 27~28)
	memset((void *)GetAddr3Ptr((txinsn.phdr)), 0xff, MACADDRLEN);

	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
        return;
	
issue_probereq_fail:
    NDEBUG("tx probe_req fail!!\n");    
    if(RTL_R8(TXPAUSE)){
        NDEBUG("!!!tx_pause_val[%X]\n",RTL_R8(TXPAUSE));
    } 

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);
}

#if defined(RTK_MULTI_AP) && (EASYMESH_VERSION >= MULTI_AP_VERSION(2,0,0))

#define TLV_TYPE_AP_EXTENDED_METRICS			(199) // 0xc7

void getExtendedAPMetric(struct rtl8192cd_priv *priv, unsigned char *buf, int *len)
{
	int data_len = 33;
	unsigned short tlv_len = 30;
	unsigned char *p;

	p = buf;

	//TLV Type
	*p = TLV_TYPE_AP_EXTENDED_METRICS;
	p++;

	//Length
	memcpy(p, &tlv_len, sizeof(unsigned short));
	p+= 2;

	//The BSSID of the BSS
	memcpy(p, BSSID, MACADDRLEN);
	p += 6;

	// UnicastBytesSent
	memset(p, 0, 4);
	p += 4;

	// UnicastBytesReceived
	memset(p, 0, 4);
	p += 4;

	// MulticastBytesSent
	memset(p, 0, 4);
	p += 4;

	// MulticastBytesReceived
	memset(p, 0, 4);
	p += 4;

	// BroadcastBytesSent
	memset(p, 0, 4);
	p += 4;

	// BroadcastBytesReceived
	memset(p, 0, 4);
	p += 4;

	*len = data_len;
}

#define TLV_TYPE_EXTENDED_ASSOCIATED_STA_LINK_METRIC        (200) //0xC8

void getExtendedAssocStaLinkMetric(struct rtl8192cd_priv *priv, unsigned char *buf, int *len)
{
		int data_len = 0;
		unsigned short tlv_len = 0;
		unsigned char sta_mac[6] = {0};
		unsigned char *p;
		struct stat_info *pstat;

		memcpy(sta_mac, &buf[1], MACADDRLEN);
		//printk("%s - %02x%02x%02x%02x%02x%02x\n", __FUNCTION__, sta_mac[0], sta_mac[1], sta_mac[2], sta_mac[3], sta_mac[4], sta_mac[5]);
		pstat = get_stainfo(priv, sta_mac);
		if (pstat)
		{
			p = buf;

			*p = TLV_TYPE_EXTENDED_ASSOCIATED_STA_LINK_METRIC;

			p += 3;	//Offset for TLV Type and Length

			memcpy(p, pstat->cmn_info.mac_addr, MACADDRLEN);
			p += 6;

			*p = 1; //Number of BSSIDs reported for this STA
			p+=1;

			memcpy(p, BSSID, MACADDRLEN);
			p+=6;

			memcpy(p, &pstat->tx_bytes, 4); //Last data downlink rate
			p+=4;

			memcpy(p, &pstat->rx_bytes, 4); //Last data uplink rate
			p+=4;

			memcpy(p, &pstat->tx_pkts, 4); //utilization receive
			p+=4;

			memcpy(p, &pstat->rx_pkts, 4); //utilization transmit
			p+=4;

			data_len = 29;
			tlv_len = data_len;

			memcpy(&buf[1], &tlv_len, sizeof(unsigned short));

			*len = (data_len + 3); //For TLV and length
		}
		else {
			p = buf;

			*p = TLV_TYPE_EXTENDED_ASSOCIATED_STA_LINK_METRIC;

			data_len = 29;
			tlv_len = data_len;

			memcpy(&buf[1], &tlv_len, sizeof(unsigned short));

			*len = (data_len + 3); //For TLV and length
		}
}

#define TLV_TYPE_CHANNEL_SCAN_RESULT           (167) // 0xA7

void rtk_multi_ap_update_channel_scan_results(struct rtl8192cd_priv *priv)
{
	unsigned char send_buf[MAX_PAYLOAD] = {0};

	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		return;
	}

	unsigned short int data_len = 0;

	//TODO:error (incomplete scan) handling

	unsigned char i, j;

	data_len += 1; //TLV_TYPE_CHANNEL_SCAN_RESULT
	data_len += 2; //data_len
	data_len += 1; //band
	data_len += 1; //channel_nr
	for (i = 0; i < priv->multiap_scan_result->channel_nr; i++) {
		data_len += 1; //channel
		data_len += 1; //scan_status
		data_len += 31; //timestamp
		data_len += 1; //channel_utilization
		data_len += 1; //noise
		data_len += 2; //neighbor_nr
		for (j = 0; j < priv->multiap_scan_result->channels[i].neighbor_nr; j++) {
			data_len += 6; //bssid
			data_len += 1; //ssid_length
			data_len += priv->multiap_scan_result->channels[i].neighbors[j].ssid_length; //ssid
			data_len += 1; //signal strength
			data_len += 1; //channel_band_width
			data_len += 1; //channel_utilization
			data_len += 2; //station_count
		}
	}

	if(data_len > MAX_PAYLOAD) {
		printk("channel scan length %d is too long!!\n", data_len);
		goto free_scan_result;
	}
	//write into buffer
	unsigned int buf_pt_offset	= 0;

	send_buf[buf_pt_offset++]	= TLV_TYPE_CHANNEL_SCAN_RESULT;
	memcpy(&send_buf[buf_pt_offset], &data_len, 2); //data_len
	buf_pt_offset += 2;
	if (NULL != strstr(priv->dev->name, "wlan")) {
		send_buf[buf_pt_offset++]	= priv->dev->name[4] - '0';
	} else {
		send_buf[buf_pt_offset++]	= 0xFF;	//error
	}
	send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channel_nr;
	for (i = 0; i < priv->multiap_scan_result->channel_nr; i++) {
		send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].channel; //channel
		send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].scan_status; //scan_status
		memcpy(&send_buf[buf_pt_offset], priv->multiap_scan_result->channels[i].timestamp, 31);//timestamp
		buf_pt_offset += 31;
		if(priv->multiap_scan_result->channels[i].channel_utilization < 1) {
			priv->multiap_scan_result->channels[i].channel_utilization = 1;
		}
		send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].channel_utilization; //channel_utilization
		send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].noise; //noise
		memcpy(&send_buf[buf_pt_offset], &priv->multiap_scan_result->channels[i].neighbor_nr, 2);//neighbor_nr
		buf_pt_offset += 2;
		for (j = 0; j < priv->multiap_scan_result->channels[i].neighbor_nr; j++) {
			memcpy(&send_buf[buf_pt_offset], priv->multiap_scan_result->channels[i].neighbors[j].bssid, 6); //neighbor_nr
			buf_pt_offset += 6;
			send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].neighbors[j].ssid_length; //ssid_length
			memcpy(&send_buf[buf_pt_offset], priv->multiap_scan_result->channels[i].neighbors[j].ssid, priv->multiap_scan_result->channels[i].neighbors[j].ssid_length); //ssid
			buf_pt_offset += priv->multiap_scan_result->channels[i].neighbors[j].ssid_length;
			send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].neighbors[j].signal_strength; //signal_strength
			send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].neighbors[j].channel_band_width; //channel_band_width
			send_buf[buf_pt_offset++]	= priv->multiap_scan_result->channels[i].neighbors[j].channel_utilization; //channel_utilization
			memcpy(&send_buf[buf_pt_offset], &priv->multiap_scan_result->channels[i].neighbors[j].station_count, 2); //station_count
			buf_pt_offset += 2;
		}
	}

	rtk_multi_ap_nl_send(send_buf, data_len);

free_scan_result:
	//free memory
	if (priv->multiap_scan_result) {
		if (priv->multiap_scan_result->channels) {
			for (i = 0; i < priv->multiap_scan_result->channel_nr; i++) {
				if (priv->multiap_scan_result->channels[i].neighbors) {
					kfree(priv->multiap_scan_result->channels[i].neighbors);
				}
			}
			kfree(priv->multiap_scan_result->channels);
		}
		kfree(priv->multiap_scan_result);
		priv->multiap_scan_result = NULL;
	}
}

#define TLV_TYPE_CAC_STATUS_REPORT				(177) // 0xB1
#define TLV_TYPE_CAC_COMPLETION_REPORT			(175) // 0xAF

void rtk_multi_ap_update_cac_results(unsigned long task_priv)
{

	//unsigned char* send_buf_status_report;
	unsigned char* send_buf_completion_report;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	//unsigned short int data_len_status_report 		= 0;
	unsigned short int data_len_completion_report	= 0;

	unsigned char i, j;
	unsigned char *p;

	// //----for testing. need to change for real scenario. fill in dummy values for tesint purpose--

	struct CACCompletionReportTLV cac_completion_tlv;
	cac_completion_tlv.tlv_type				= TLV_TYPE_CAC_COMPLETION_REPORT;
	cac_completion_tlv.radio_nr				= priv->multiap_cac_channel_nr;
	cac_completion_tlv.radios			= (struct CACCompletionReportRadio*) kmalloc(cac_completion_tlv.radio_nr * sizeof(struct CACCompletionReportRadio), GFP_ATOMIC);
	for(i = 0; i < cac_completion_tlv.radio_nr; i++){
		memcpy(cac_completion_tlv.radios[i].radio_unique_identifier, GET_MY_HWADDR, 6);
		cac_completion_tlv.radios[i].op_class			= priv->multiap_cac_op_class[i];
		cac_completion_tlv.radios[i].channel			= priv->multiap_cac_channels[i];
		cac_completion_tlv.radios[i].flags				= 0;
		cac_completion_tlv.radios[i].pairs_nr			= 0;
		cac_completion_tlv.radios[i].pairs	= (struct CACCompletionReportClassChannelPairs*) kmalloc(cac_completion_tlv.radios[i].pairs_nr * sizeof(struct CACCompletionReportClassChannelPairs), GFP_ATOMIC);
		for(j = 0; j < cac_completion_tlv.radios[i].pairs_nr; j++){
			cac_completion_tlv.radios[i].pairs[j].pairs_op_class			= 0;
			cac_completion_tlv.radios[i].pairs[j].pairs_channel				= 0;
		}
	}
	// //-----------------------------------------------------------------------------------

	data_len_completion_report += 1; //radio_nr
	for (i = 0; i < cac_completion_tlv.radio_nr; i++) {
		data_len_completion_report += (6 + 1 + 1 + 1 + 1 ); //radio_unique_identifier + op_class + channel + flags + pairs_nr
		for (j = 0; j < cac_completion_tlv.radios[i].pairs_nr; j++) {
			data_len_completion_report += (1 + 1); //pairs_op_class + pairs_channel
		}
	}

	send_buf_completion_report = (unsigned char*)kmalloc((data_len_completion_report + 3) * sizeof(unsigned char), GFP_ATOMIC);
	unsigned int buf_cp_offset	= 0;
	send_buf_completion_report[buf_cp_offset++]	= TLV_TYPE_CAC_COMPLETION_REPORT;
	p = &send_buf_completion_report[buf_cp_offset];
	_I2B(&data_len_completion_report, &p); //length: data_len_completion_report
	buf_cp_offset += 2;
	send_buf_completion_report[buf_cp_offset++]	= cac_completion_tlv.radio_nr;
	for (i = 0; i < cac_completion_tlv.radio_nr; i++) {
		memcpy(&send_buf_completion_report[buf_cp_offset], cac_completion_tlv.radios[i].radio_unique_identifier, 6);//radio_unique_identifier
		buf_cp_offset += 6;
		send_buf_completion_report[buf_cp_offset++]	= cac_completion_tlv.radios[i].op_class; //op_class
		send_buf_completion_report[buf_cp_offset++]	= cac_completion_tlv.radios[i].channel; //channel
		send_buf_completion_report[buf_cp_offset++]	= cac_completion_tlv.radios[i].flags; //flags
		send_buf_completion_report[buf_cp_offset++]	= cac_completion_tlv.radios[i].pairs_nr; //pairs_nr
		for (j = 0; j < cac_completion_tlv.radios[i].pairs_nr; j++) {
			send_buf_completion_report[buf_cp_offset++]	= cac_completion_tlv.radios[i].pairs[j].pairs_op_class; //pairs_op_class
			send_buf_completion_report[buf_cp_offset++]	= cac_completion_tlv.radios[i].pairs[j].pairs_channel; //pairs_channel
		}
	}

	rtk_multi_ap_nl_send(send_buf_completion_report, data_len_completion_report + 3);

	kfree(send_buf_completion_report);

	for (i = 0; i < cac_completion_tlv.radio_nr; i++) {
		if (cac_completion_tlv.radios[i].pairs_nr){
			kfree(cac_completion_tlv.radios[i].pairs);
		}
	}

	if (cac_completion_tlv.radio_nr) {
		kfree(cac_completion_tlv.radios);
	}
}

void rtk_multi_ap_channel_scan_trigger_ss(struct rtl8192cd_priv *priv)
{
	priv->ss_req_ongoing = 1;//SSFROM_PASSIVE
	priv->multiap_channel_scan = 1;

	if(!priv->pmib->miscEntry.func_off)
		start_clnt_ss(priv);
}

int rtk_multi_ap_do_channel_scan(struct rtl8192cd_priv *priv, unsigned char *tmpbuf)
{
	//handle free multiap_requeste
	if (priv->multiap_requested_channels) {
		kfree(priv->multiap_requested_channels);
		priv->multiap_requested_channel_nr = 0;
	}

	int i = 0, j = 0, ret = 0;

	memcpy(&priv->multiap_requested_channel_nr, tmpbuf, 1);
	priv->multiap_requested_channels = (unsigned char*)kmalloc(priv->multiap_requested_channel_nr, GFP_ATOMIC);
	memcpy(priv->multiap_requested_channels, tmpbuf+1, priv->multiap_requested_channel_nr);

	//initialize the struct to stall results
	priv->multiap_scan_result	= (struct map_channel_scan_result_per_radio*)kmalloc(sizeof(struct map_channel_scan_result_per_radio), GFP_ATOMIC);
	priv->multiap_scan_result->channel_nr	= priv->multiap_requested_channel_nr;
	priv->multiap_scan_result->channels		= (struct map_channel_scan_result_per_channel*)kmalloc(priv->multiap_requested_channel_nr * sizeof(struct map_channel_scan_result_per_channel), GFP_ATOMIC);
	for (i = 0; i < priv->multiap_scan_result->channel_nr; i++) {
		priv->multiap_scan_result->channels[i].channel = priv->multiap_requested_channels[i];
		for (j = 0; j < priv->available_chnl_num; j++) {
			if (priv->multiap_scan_result->channels[i].channel == priv->available_chnl[j]) {
				break;
			}
		}
		if (j == priv->available_chnl_num) {
			priv->multiap_scan_result->channels[i].scan_status		= 1;
		} else {
			priv->multiap_scan_result->channels[i].scan_status      = 0;
		}
		memset(priv->multiap_scan_result->channels[i].timestamp, 0, 31);
		priv->multiap_scan_result->channels[i].channel_utilization	= 0;
		priv->multiap_scan_result->channels[i].noise				= 0;
		priv->multiap_scan_result->channels[i].neighbor_nr			= 0;
		priv->multiap_scan_result->channels[i].neighbors			= NULL;
	}

	rtk_multi_ap_channel_scan_trigger_ss(priv);

	ret = 1;
	return ret;
}

void rtk_multi_ap_update_cac_status(struct rtl8192cd_priv *priv)
{
	unsigned char send_buf_status_report[MAX_PAYLOAD] = {0};

	unsigned short int data_len_status_report 		= 0;

	unsigned char *p;

	data_len_status_report += 6; //radio_id
	data_len_status_report += 1; //status
	data_len_status_report += 1; //channel
	data_len_status_report += 1; //op_class

	unsigned int buf_sr_offset	= 0;
	send_buf_status_report[buf_sr_offset++]	= TLV_TYPE_CAC_STATUS_REPORT;
	p = &send_buf_status_report[buf_sr_offset];
	//_I2B(&data_len_status_report, &p); //length: data_len_status_report
	memcpy(p, &data_len_status_report, 2);
	buf_sr_offset += 2;
	p = &send_buf_status_report[buf_sr_offset];
	memcpy(p, priv->pmib->dot11OperationEntry.hwaddr, 6);
	buf_sr_offset += 6;
	// 0: ongoing CAC channel, 1: available channel, 2: non-occupancy channel
	if (priv->pmib->dot11DFSEntry.DFS_detected) {
		send_buf_status_report[buf_sr_offset++]	= 0;
	} else {
		send_buf_status_report[buf_sr_offset++]	= 1;
	}
	send_buf_status_report[buf_sr_offset++]	= priv->multiap_cac_channels[priv->multiap_cac_current_channel_idx];
	send_buf_status_report[buf_sr_offset++]	= priv->multiap_cac_op_class[priv->multiap_cac_current_channel_idx];

	rtk_multi_ap_nl_send(send_buf_status_report, data_len_status_report + 3);
}

#define CAC_COMPL_ACTION_MASK	0b00011000
#define CAC_COMPL_ACTION_RETURN 0b00001000
#define CAC_COMPL_ACTION_REMAIN 0b00000000
int rtk_multi_ap_do_cac(struct rtl8192cd_priv *priv, unsigned char *tmpbuf)
{
	int i = 0;
	unsigned char type			= tmpbuf[0];
	unsigned char channel_nr	= tmpbuf[1];

	unsigned char second_cac_flag = 0;

	int delay_start = 100;

	unsigned char need_return = 0;

	unsigned long		flags;

	//printk("Type       : %d\n", tmpbuf[0]);
	//printk("Channel_nr : %d\n", tmpbuf[1]);

	if (0 == type) {	//CAC Request
		if (priv->multiap_cac_current_channel_idx) {
			del_timer_sync(&priv->multiap_cac_process_timer);
		}
		SMP_LOCK_CAC_CHANNEL(flags);
		if(priv->multiap_cac_current_channel_idx) {
			if(priv->multiap_cac_channel_nr){
				kfree(priv->multiap_cac_channels);
				kfree(priv->multiap_cac_op_class);
			}
			priv->multiap_cac_channel_nr        = 0;
			priv->multiap_cac_current_channel_idx	= 0;
			second_cac_flag						= 1;
			delay_start                         = 5000;
		}
		priv->multiap_cac_channel_nr = tmpbuf[1];
		priv->multiap_cac_channels = (unsigned char*)kmalloc(priv->multiap_cac_channel_nr, GFP_ATOMIC);
		priv->multiap_cac_op_class = (unsigned char*)kmalloc(priv->multiap_cac_channel_nr, GFP_ATOMIC);
		for(i = 0; i < channel_nr; i++) {
			unsigned char cac_remain_flag = tmpbuf[i * 3 + 3];
			//printk("Request Channel %d: %d \n", i, tmpbuf[i * 3 + 2]);
			priv->multiap_cac_channels[i] = tmpbuf[i * 3 + 2];
			//printk("Request Flags %d: %d \n", i, tmpbuf[i * 3 + 3]);
			//printk("Request Op Class %d: %d \n", i, tmpbuf[i * 3 + 4]);
			priv->multiap_cac_op_class[i] = tmpbuf[i * 3 + 4];

			// if(CAC_COMPL_ACTION_REMAIN == (cac_remain_flag & CAC_COMPL_ACTION_MASK)) {
			// 	priv->multiap_cac_channel_hold = tmpbuf[i * 3 + 2];
			// } else if (CAC_COMPL_ACTION_RETURN == (cac_remain_flag & CAC_COMPL_ACTION_MASK) && !priv->multiap_cac_channel_hold ){
				// if (!second_cac_flag) {
				// 	need_return = 1;
				// }
			// }
			if (CAC_COMPL_ACTION_RETURN == (cac_remain_flag & CAC_COMPL_ACTION_MASK)){
				if (!second_cac_flag) {
					need_return = 1;
				}
			} else {
				priv->multiap_cac_channel_hold = 0;
			}
		}
		if(need_return) {
			priv->multiap_cac_channel_hold = priv->pmib->dot11RFEntry.dot11channel;
		}
		SMP_UNLOCK_CAC_CHANNEL(flags);
		//set the timer which triggers the call back function after 5 sec
		if(priv->multiap_cac_process_timer.function != NULL) {
			mod_timer(&priv->multiap_cac_process_timer,jiffies + RTL_MILISECONDS_TO_JIFFIES(delay_start));
		}
	} else if (1 == type) {	//CAC Termination
		for(i = 0; i < channel_nr; i++) {
			printk("Termination Channel %d: %d \n", i, tmpbuf[i * 2 + 2]);
			printk("Termination Op Class %d: %d \n", i, tmpbuf[i * 2 + 3]);
		}
		//cancel the timer if termination is received
		if (timer_pending(&priv->multiap_cac_process_timer)){
			del_timer_sync(&priv->multiap_cac_process_timer);
		}

		if(priv->multiap_cac_channel_nr){
			kfree(priv->multiap_cac_channels);
			kfree(priv->multiap_cac_op_class);
		}
		priv->multiap_cac_channel_nr      = 0;
		priv->multiap_cac_current_channel_idx = 0;

#ifdef DFS
		del_timer_sync(&priv->ch_avail_chk_timer);
		if(priv->ch_avail_chk_timer.function != NULL) {
			mod_timer(&priv->ch_avail_chk_timer, jiffies + RTL_MILISECONDS_TO_JIFFIES(200));
		}
#endif

		// Return to previous operation channel after CAC
		if (priv->pmib->dot11RFEntry.dot11channel != priv->multiap_cac_channel_hold && 0 != priv->multiap_cac_channel_hold) {
			priv->pmib->dot11RFEntry.dot11channel = priv->multiap_cac_channel_hold;
			rtk_multi_ap_switch_channel(priv);
		}
	} else {	//error
		printk("Error: Cannot identify CAC Packet! \n");
	}

	return 0;
}

void rtk_multi_ap_cac_channel_switch(unsigned long task_priv)
{
	unsigned long flags;
	struct rtl8192cd_priv *priv = (struct rtl8192cd_priv *)task_priv;

	if (!(priv->drv_state & DRV_STATE_OPEN)) {
		return;
	}

	SMP_LOCK_CAC_CHANNEL(flags);
	if (!priv->multiap_cac_channel_nr) {
		priv->multiap_cac_current_channel_idx = 0;
		SMP_UNLOCK_CAC_CHANNEL(flags);
		return;
	} else if(priv->multiap_cac_channel_nr == priv->multiap_cac_current_channel_idx) {
		rtk_multi_ap_update_cac_results(task_priv);
		if(priv->multiap_cac_channel_nr){
			kfree(priv->multiap_cac_channels);
			kfree(priv->multiap_cac_op_class);
		}
		priv->multiap_cac_channel_nr = 0;
		//switch to channel_hold
		if(priv->multiap_cac_channel_hold) {
			priv->pmib->dot11RFEntry.dot11channel = priv->multiap_cac_channel_hold;
		}
	} else {
		//get channel switch to from stored channel list
		//set the channel obtained to this variable
		priv->pmib->dot11RFEntry.dot11channel = priv->multiap_cac_channels[priv->multiap_cac_current_channel_idx];
		printk("Switch channel %d: %d\n", priv->multiap_cac_current_channel_idx, priv->multiap_cac_channels[priv->multiap_cac_current_channel_idx]);
		rtk_multi_ap_update_cac_status(priv);
		priv->multiap_cac_current_channel_idx++;
	}
	SMP_UNLOCK_CAC_CHANNEL(flags);

	//switch to the next channel in the list
	rtk_multi_ap_switch_channel(priv);
	//call this function itself after CAC_TEST_TIME
	if(priv->multiap_cac_process_timer.function != NULL) {
		// mod_timer(&priv->multiap_cac_process_timer,jiffies + RTL_MILISECONDS_TO_JIFFIES(5000));
		mod_timer(&priv->multiap_cac_process_timer,jiffies + RTL_SECONDS_TO_JIFFIES(10));
	}

	return;
}

void rtk_multi_ap_trigger_association_status_notification(unsigned char *bssid, unsigned char association_allowance_status)
{
	unsigned char send_buf[MAX_PAYLOAD] = {0};
	unsigned short data_len = 0;

	send_buf[0] = TLV_TYPE_ASSOCIATION_STATUS_NOTIFICATION;

	memcpy(&send_buf[3], bssid, 6);
	send_buf[9] = association_allowance_status;
	data_len = (6 + 1);
	memcpy(&send_buf[1], &data_len, sizeof(unsigned short));
	data_len += 3;

	rtk_multi_ap_nl_send(send_buf, data_len);
}

void rtk_multi_ap_trigger_tunneled_message(unsigned char* mac_address, unsigned char message_type, unsigned short payload_length, unsigned char *payload)
{
	unsigned char send_buf[MAX_PAYLOAD] = {0};
	unsigned short payload_len = 0;
	unsigned short data_len = 0;

	payload_len = payload_length;
	send_buf[0] = TLV_TYPE_TUNNELED;
	memcpy(&send_buf[3], mac_address, 6);
	send_buf[9] = message_type;
	memcpy(&send_buf[10], &payload_len, sizeof(unsigned short));
	memcpy(&send_buf[12], payload, payload_len);
	data_len = (6 + 1 + 2 + payload_len);
	memcpy(&send_buf[1], &data_len, sizeof(unsigned short));
	data_len += 3;
	rtk_multi_ap_nl_send(send_buf, data_len);
}

void rtk_multi_ap_trigger_fail_connection_message(unsigned char* mac_address, unsigned short status_code, unsigned short reason_code)
{
	unsigned char send_buf[MAX_PAYLOAD] = {0};
	unsigned char ptr = 0;
	unsigned short value = 0, data_len = 0;

	send_buf[ptr] = 0x12;
	ptr += 3;

	memcpy(&send_buf[ptr], mac_address, MACADDRLEN);
	ptr += 6;

	value = status_code;
	memcpy(&send_buf[ptr], &value, 2);
	ptr += 2;

	value = reason_code;
	memcpy(&send_buf[ptr], &value, 2);
	ptr += 2;

	data_len = 10;
	memcpy(&send_buf[1], &data_len, sizeof(unsigned short));

	data_len += 3;
	rtk_multi_ap_nl_send(send_buf, data_len);
}

void rtk_multi_ap_trigger_disassoc_stats_message(struct stat_info *pstat, unsigned short reason_code)
{
	unsigned char send_buf[MAX_PAYLOAD] = {0};
	unsigned char ptr = 0;
	unsigned short data_len = 0, value = 0, tlv_len = 0;

	send_buf[ptr] = 0x13;
	ptr += 3;

// STA MAC Address TLV
	// send_buf[ptr] = 0x95; //0x95
	// ptr += 1;
	// tlv_len = 6; //6
	// memcpy(&send_buf[ptr], &tlv_len, 2);
	// ptr += 2;
	memcpy(&send_buf[ptr], pstat->cmn_info.mac_addr, MACADDRLEN); // Mac address
	ptr += 6;

	data_len += 6; //9

// Reason code TLV
	// send_buf[ptr]  = 0xca; // 0xca
	// ptr += 1;
	// tlv_len = 2; // 2
	// memcpy(&send_buf[ptr], &tlv_len, 2);
	// ptr += 2;
	value = reason_code; // Reason code
	memcpy(&send_buf[ptr], &value, 2);
	ptr += 2;

	data_len += 2; //5

// Associated STA Traffic Stats TLV
	send_buf[ptr]  = 0xa2; // 0xa2
	ptr += 1;
	tlv_len = 34; // 34
	memcpy(&send_buf[ptr], &tlv_len, 2);
	ptr += 2;
	memcpy(&send_buf[ptr], pstat->cmn_info.mac_addr, MACADDRLEN); // 6
	ptr += 6;
	memcpy(&send_buf[ptr], &pstat->tx_bytes, 4); //Bytes Sent
	ptr += 4;
	memcpy(&send_buf[ptr], &pstat->rx_bytes, 4); //Bytes Received
	ptr += 4;
	memcpy(&send_buf[ptr], &pstat->tx_pkts, 4); //Packets Sent
	ptr += 4;
	memcpy(&send_buf[ptr], &pstat->rx_pkts, 4); //Packets Received
	ptr += 4;
	memcpy(&send_buf[ptr], &pstat->tx_fail, 4); //TxPacketsError
	ptr += 4;
	memset(&send_buf[ptr], 0, 4);  //RxPacketsError
	ptr += 4;
	memset(&send_buf[ptr], 0, 4); //RetrasmissionCount
	ptr += 4;

	data_len += 37;
	
	memcpy(&send_buf[1], &data_len, sizeof(unsigned short));

	data_len += 3; //For message type + overall length

	rtk_multi_ap_nl_send(send_buf, data_len);
}

#endif


int rtk_multi_ap_getMetric(struct rtl8192cd_priv *priv, unsigned char *buf){

	int len = 0;
#if 0
	int i;
#endif

	if(*buf == 0)
		getAPMetric(priv, buf, &len);
	else if(*buf == 1)
		getAssocStaLinkMetric(priv, buf, &len);
	else if(*buf == 2)
		getAssocStaTrafficStats(priv, buf, &len);
#if defined(RTK_MULTI_AP) && (EASYMESH_VERSION >= MULTI_AP_VERSION(2,0,0))
	else if(*buf == 3)
		getExtendedAPMetric(priv, buf, &len);
	else if(*buf == 4)
		getExtendedAssocStaLinkMetric(priv, buf, &len);
#endif

#if 0
	printk("Len(%d):", len);
	for(i = 0; i < len; i++)
		printk("%02x", buf[i]);
	printk("\n");
#endif
	return len;
}

int rtk_multi_ap_getAssocStaMetric(struct rtl8192cd_priv *priv, unsigned char *buf){

	int len = 0;

	if(*buf == 0)
		getAssocStaLinkMetric(priv, buf, &len);
	else if(*buf == 1)
		getAssocStaTrafficStats(priv, buf, &len);
#if defined(RTK_MULTI_AP) && (EASYMESH_VERSION >= MULTI_AP_VERSION(2,0,0))
	else if(*buf == 2)
		getExtendedAssocStaLinkMetric(priv, buf, &len);
#endif

	return len;
}

int rtk_multi_ap_get_available_channels(struct rtl8192cd_priv *priv, unsigned char *buf)
{
	int len = 0, i = 0;

	// printk("[%s] Available channels number: %d\n", priv->dev->name, priv->available_chnl_num);
	// for(i = 0; i < priv->available_chnl_num; i++) {
	// 	printk("[%s] Available channels %d: %d\n", priv->dev->name, i, priv->available_chnl[i]);
	// }

	buf[len] = (unsigned char)priv->available_chnl_num;
	len += 1;
	for(i = 0; i < priv->available_chnl_num; i++) {
		buf[len] = (unsigned char)priv->available_chnl[i];
		len += 1;
	}

	return len;
}

void rtk_multi_ap_channel_change_notify()
{
	int           data_len    = 0;
	unsigned char send_buf[1] = { 0 };

	send_buf[0] = MAP_CHANNEL_CHANGE_NOTIFICATION_MESSAGE;
	data_len += 1;

	rtk_multi_ap_nl_send(send_buf, data_len);
}

int rtk_multi_ap_set_tx_power(struct rtl8192cd_priv *priv, unsigned char *buf)
{
#if defined(CONFIG_WLAN_HAL_8814BE) || defined(CONFIG_WLAN_HAL_8812FE)
	int  rate;
	char max_power_offset          = 0;
	char max_power_assigned        = buf[0];
	char max_power_real            = 0;
	char tgpwr_HT1S_NEW_offset_tmp = 0;
#endif

	if (!IS_ROOT_INTERFACE(priv))
		return 0;

#if defined(CONFIG_WLAN_HAL_8814BE) || defined(CONFIG_WLAN_HAL_8812FE)
	if ((GET_CHIP_VER(priv) == VERSION_8814B) || (GET_CHIP_VER(priv) == VERSION_8812F)) {
		if (priv->pmib->dot11RFEntry.tssi_enable) {

			printk("max_power_assigned = %d, priv->pshare->tgpwr_HT1S_NEW = %d, priv->pshare->tgpwr_HT1S_NEW_offset=%d", max_power_assigned, priv->pshare->tgpwr_HT1S_NEW, priv->pshare->tgpwr_HT1S_NEW_offset);
			if (max_power_assigned == 0) {
				priv->pshare->tgpwr_HT1S_NEW += priv->pshare->tgpwr_HT1S_NEW_offset;
				priv->pshare->tgpwr_HT1S_NEW_offset = 0;
				phy_iq_calibrate(priv);
				reg_pg_info(priv);
				return 1;
			}

			for (rate = 0; rate < 4; rate++) {
				if (priv->pshare->phw->CCKTxAgc[rate] > max_power_offset || max_power_offset == 0)
					max_power_offset = priv->pshare->phw->CCKTxAgc[rate];
			}
			for (rate = 0; rate < 8; rate++) {
				if (priv->pshare->phw->OFDMTxAgcOffset[rate] > max_power_offset || max_power_offset == 0)
					max_power_offset = priv->pshare->phw->OFDMTxAgcOffset[rate];
			}
			for (rate = 0; rate < 16; rate++) {
				if (priv->pshare->phw->MCSTxAgcOffset[rate] > max_power_offset || max_power_offset == 0)
					max_power_offset = priv->pshare->phw->MCSTxAgcOffset[rate];
			}

			if (GET_CHIP_VER(priv) == VERSION_8814B) {
				for (rate = 16; rate < 32; rate++) {
					if (priv->pshare->phw->MCSTxAgcOffset[rate] > max_power_offset || max_power_offset == 0)
						max_power_offset = priv->pshare->phw->MCSTxAgcOffset[rate];
				}
			}

			for (rate = 0; rate < 18; rate++) {
				if (priv->pshare->phw->VHTTxAgcOffset[rate] > max_power_offset || max_power_offset == 0)
					max_power_offset = priv->pshare->phw->VHTTxAgcOffset[rate];
			}

			if (GET_CHIP_VER(priv) == VERSION_8814B) {
				for (rate = 18; rate < 36; rate++) {
					if (priv->pshare->phw->VHTTxAgcOffset[rate] > max_power_offset || max_power_offset == 0)
						max_power_offset = priv->pshare->phw->VHTTxAgcOffset[rate];
				}
			}
			max_power_real = (max_power_offset + priv->pshare->tgpwr_HT1S_NEW + priv->pshare->tgpwr_HT1S_NEW_offset) / 4;
			if ((max_power_real > max_power_assigned) && (max_power_assigned * 4 < (max_power_offset + priv->pshare->tgpwr_HT1S_NEW + priv->pshare->tgpwr_HT1S_NEW_offset))) {
				tgpwr_HT1S_NEW_offset_tmp = (max_power_offset + priv->pshare->tgpwr_HT1S_NEW + priv->pshare->tgpwr_HT1S_NEW_offset) - max_power_assigned * 4;
				if (tgpwr_HT1S_NEW_offset_tmp > 64)
					tgpwr_HT1S_NEW_offset_tmp = 64;
				if (tgpwr_HT1S_NEW_offset_tmp < -12)
					tgpwr_HT1S_NEW_offset_tmp = -12;
				priv->pshare->tgpwr_HT1S_NEW        = priv->pshare->tgpwr_HT1S_NEW + priv->pshare->tgpwr_HT1S_NEW_offset - tgpwr_HT1S_NEW_offset_tmp;
				priv->pshare->tgpwr_HT1S_NEW_offset = tgpwr_HT1S_NEW_offset_tmp;
			} else {
				priv->pshare->tgpwr_HT1S_NEW += priv->pshare->tgpwr_HT1S_NEW_offset;
				priv->pshare->tgpwr_HT1S_NEW_offset = 0;
			}
			phy_iq_calibrate(priv);
			reg_pg_info(priv);
			return 1;
		}
	}
#endif
	return 1;
}
