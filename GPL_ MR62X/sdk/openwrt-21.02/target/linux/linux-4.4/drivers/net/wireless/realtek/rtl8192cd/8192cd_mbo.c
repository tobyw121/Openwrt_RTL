#define _8192CD_MBO_C

#include "./8192cd.h"
#include "./8192cd_mbo.h"


const unsigned char oui[] = {0x50, 0x6f, 0x9a};

#ifdef CONFIG_MBO

unsigned int check_association_disallowed(struct rtl8192cd_priv *priv, int frame_type, unsigned char rssi)
{
	if(priv->assoc_num >= MBO_STA_NUM)
		return MAXIMUM_NUMBER_OF_ASSOCIATED_STAS_REACHED;

	if(priv->ext_stats.ch_utilization > MBO_CH_UTIL)
		return AIR_INTERFACE_IS_OVERLOADED;

	if((frame_type == WIFI_PROBEREQ || frame_type == WIFI_ASSOCREQ || frame_type == WIFI_REASSOCREQ) && rssi < MBO_RSSI_THRESHOLD)
		return INSUFFICIENT_RSSI;

	return 0;
}

unsigned char *set_mbo_ap_capability_indication_attr(unsigned char *pbuf, unsigned int *frlen)
{
	unsigned char temp = 0;
	// If the AP is cellular data aware, the attribute value should be 01000000. Otherwise, the value is zero.
	//*temp |= BIT(6);

	pbuf = set_ie(pbuf, _MBO_AP_CAPCABILITY_INDICATION_ATTR_ID_, 1, &temp, frlen);
	return pbuf;
}

unsigned char *set_association_disallowed_attr(unsigned char *pbuf, unsigned int *frlen, unsigned int assoc_disallowed_status)
{
	unsigned char temp = assoc_disallowed_status;

	pbuf = set_ie(pbuf, _ASSOCIATION_DISALLOWED_ATTR_ID_, 1, &temp, frlen);
	return pbuf;
}

unsigned char *set_transition_reason_code_attr(unsigned char *pbuf, unsigned int *frlen)
{
	unsigned char temp;

	if(0) // Excessive frame loss rate
		temp = 1;
	else if(0) // Excessive delay for current traffic stream
		temp = 2;
	else if(0) // Insufficient bandwidth for current traffic stream
		temp = 3;
	else if(0) // Load balancing
		temp = 4;
	else if(0) // Low RSSI
		temp = 5;
	else if(0) // Received excessive number of retransmissions
		temp = 6;
	else if(1) // High interference
		temp = 7;
	else if(0) // Gray zone
		temp = 8;
	else if(0) // Transitioning to a premium AP
		temp = 9;
	else // Unspecified
		temp = 0;

	pbuf = set_ie(pbuf, _TRANSITION_REASON_CODE_ATTR_ID_, 1, &temp, frlen);
	return pbuf;
}

unsigned char *set_association_retry_delay_attr(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen)
{
	unsigned short temp = priv->pmib->wnmEntry.dot11vRetryDelay;
	temp = cpu_to_le16(temp);

	pbuf = set_ie(pbuf, _ASSOCIATION_RETRY_DELAY_ATTR_ID_, 2, (unsigned char*)&temp, frlen);
	return pbuf;
}

// MBO IE for Beacon, Probe Response and (Re)Association Response
unsigned char *construct_mbo_ie(unsigned char *pbuf, unsigned int *frlen, unsigned int assoc_disallow)
{
	unsigned int length;
	unsigned char temp[4];

	if(assoc_disallow) // AP cannot accept new associations
		length = 10;
	else
		length = 7;

	memcpy(temp, oui, 3); //OUI
	temp[3] = 0x16; //OUI Type

	//pbuf = set_ie(pbuf, _VENDOR_SPEC_IE_, length, temp, frlen);
	*pbuf = _VENDOR_SPEC_IE_;
	*(pbuf + 1) = length;
	memcpy((void *)(pbuf + 2), (void *)temp, 4);
	*frlen = *frlen + 6;
	pbuf = pbuf + 6;

	pbuf = set_mbo_ap_capability_indication_attr(pbuf, frlen);
	if(assoc_disallow) // AP cannot accept new associations
		pbuf = set_association_disallowed_attr(pbuf, frlen, assoc_disallow);

	return pbuf;
}

// MBO IE for Bss Transition Management Request
unsigned char *construct_btm_req_mbo_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen)
{
	unsigned int length;
	unsigned char temp[4];

	if(priv->pmib->wnmEntry.dot11vBssTermination == 1)
		length = 7;
	else
		length = 11;

	memcpy(temp, oui, 3); //OUI
	temp[3] = 0x16; //OUI Type

	//pbuf = set_ie(pbuf, _VENDOR_SPEC_IE_, length, temp, frlen);
	*pbuf = _VENDOR_SPEC_IE_;
	*(pbuf + 1) = length;
	memcpy((void *)(pbuf + 2), (void *)temp, 4);
	*frlen = *frlen + 6;
	pbuf = pbuf + 6;

	pbuf = set_transition_reason_code_attr(pbuf, frlen);
	if(priv->pmib->wnmEntry.dot11vBssTermination == 0)
		pbuf = set_association_retry_delay_attr(priv, pbuf, frlen);

	return pbuf;
}

#endif

#ifdef CONFIG_MBO_CLI

unsigned char *set_cellular_data_capabilities_attr(unsigned char *pbuf, unsigned int *frlen)
{
	//value			Description
	// 0			 Reserved
	// 1			 Cellular data connection available
	// 2			 Cellular data connection not available
	// 3			 Not Cellular data capable
	// 4-255		 Reserved

	unsigned char temp = 3;

	pbuf = set_ie(pbuf, _CELLULAR_DATA_CAPCABILITIES_ATTR_ID_, 1, &temp, frlen);
	return pbuf;
}

unsigned char *set_transition_rejection_reason_code_attr(unsigned char *pbuf, unsigned int *frlen)
{

}

unsigned char *set_non_preferred_channel_report_attr(unsigned char *pbuf, unsigned int *frlen,
																	unsigned char channel, unsigned char op_class,
																	unsigned char preference, unsigned char reason)
{
	unsigned char temp[4];

	temp[0] = op_class;
	temp[1] = channel;
	temp[2] = preference; /* 0:	non-operable	1: non-preferred 2-254: reserved	255: preferred */
	temp[3] = reason; /* 0: Unspecified		1: Co-located interference		2: In-device interferer 3-255 reserved*/

	pbuf = set_ie(pbuf, _NON_PREFERRED_CHANNEL_REPORT_ATTR_ID_, 4, temp, frlen);

	return pbuf;
}

// MBO IE for Probe Request
unsigned char *construct_probe_req_mbo_ie(unsigned char *pbuf, unsigned int *frlen)
{
	unsigned int length = 7;
	unsigned char temp[4];

	memcpy(temp, oui, 3); //OUI
	temp[3] = 0x16; //OUI Type

	//pbuf = set_ie(pbuf, _VENDOR_SPEC_IE_, length, temp, frlen);
	*pbuf = _VENDOR_SPEC_IE_;
	*(pbuf + 1) = length;
	memcpy((void *)(pbuf + 2), (void *)temp, 4);
	*frlen = *frlen + 6;
	pbuf = pbuf + 6;

	pbuf = set_cellular_data_capabilities_attr(pbuf, frlen);

	return pbuf;
}

// MBO IE for Bss Transition Management Response
unsigned char *construct_btm_rsp_mbo_ie(unsigned char *pbuf, unsigned int *frlen)
{
	unsigned int length = 7;
	unsigned char temp[4];

	memcpy(temp, oui, 3); //OUI
	temp[3] = 0x16; //OUI Type

	//pbuf = set_ie(pbuf, _VENDOR_SPEC_IE_, length, temp, frlen);
	*pbuf = _VENDOR_SPEC_IE_;
	*(pbuf + 1) = length;
	memcpy((void *)(pbuf + 2), (void *)temp, 4);
	*frlen = *frlen + 6;
	pbuf = pbuf + 6;

	pbuf = set_transition_rejection_reason_code_attr(pbuf, frlen);

	return pbuf;
}

// MBO IE for (Re)Association Request
unsigned char *construct_assoc_req_mbo_ie(unsigned char *pbuf, unsigned int *frlen,
													unsigned char channel, unsigned char op_class,
													unsigned char preference, unsigned char reason)
{
	unsigned int length;
	unsigned char temp[4];

	memcpy(temp, oui, 3);
	temp[3] = 0x16;

	//pbuf = set_ie(pbuf, _VENDOR_SPECIFIC_IE_, length, temp, frlen);
	length = 13;
	*pbuf = _VENDOR_SPEC_IE_;
	*(pbuf + 1) = length;
	memcpy((void *)(pbuf + 2), (void *)temp, 4);
	*frlen = *frlen + 6;
	pbuf = pbuf + 6;

	pbuf = set_non_preferred_channel_report_attr(pbuf, frlen, channel, op_class, preference, reason);
	pbuf = set_cellular_data_capabilities_attr(pbuf, frlen);

	return pbuf;
}

unsigned char *construct_mbo_anqp_element(unsigned char *pbuf, unsigned int *frlen)
{
	// infoID
	pbuf[0] = 56797 & 0x00ff;
	pbuf[1] = (56797 & 0xff00) >> 8;
	// Length
	pbuf[2] = 6 & 0x00ff;
	pbuf[3] = (6 & 0xff00) >> 8;
	// OUI
	pbuf[4] = 0x50;
	pbuf[5] = 0x6f;
	pbuf[6] = 0x9a;
	// OUI Type
	pbuf[7] = 0x12;
	// Subtype
	pbuf[8] = 1; // MBO Query List
	// Payload
	pbuf[9] = 2; // Cellular Data Connection Preference

	*frlen += 10;
	pbuf += 10;
	return pbuf;
}

void issue_WNM_Notify_channel_report(struct rtl8192cd_priv *priv)
{
	unsigned char *pbuf;

	unsigned char channel;
	unsigned char op_class;
	unsigned char preference;
	unsigned char reason;

	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
#if 0
	if(txinsn.isPMF == 0) {
		printk("\n\nWNM_NOTIFY_FAIL: PMF is not Enabled, psta=%x\n\n");
		goto issue_WNM_NOTIFY_fail;
    }
#endif
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
	txinsn.lowest_tx_rate = txinsn.tx_rate;
	txinsn.fixed_rate = 1;
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);
	if (pbuf == NULL)
		goto issue_WNM_NOTIFY_fail;

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	if (txinsn.phdr == NULL)
		goto issue_WNM_NOTIFY_fail;

	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));

	pbuf[0] = _WNM_CATEGORY_ID_;
	pbuf[1] = _WNM_NOTIFICATION_ID_;
	pbuf[2] = 4;					// dialog token
	pbuf[3] = _VENDOR_SPEC_IE_;		// type

	// Optional Subelements
	pbuf[4] = _VENDOR_SPEC_IE_;		// subelement ID
	pbuf[5] = 8;					// length
	pbuf[6] = 0x50;					// OUI: 0x506F9A
	pbuf[7] = 0x6F;
	pbuf[8] = 0x9A;
	pbuf[9] = 0x02;					// OUI type

	op_class = 81;
	channel = 1;
	preference = 1;
	reason = 1;

	pbuf[10] = op_class;
	pbuf[11] = channel;
	pbuf[12] = preference;
	pbuf[13] = reason;

	pbuf += 14;
	txinsn.fr_len += 14;

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	memcpy((void *)GetAddr3Ptr((txinsn.phdr)), BSSID, MACADDRLEN);

	if((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		printk("issue_WNM_Notify success\n");

issue_WNM_NOTIFY_fail:
	PMFDEBUG("issue_WNM_NOTIFY failed\n");

	if (txinsn.phdr)
		release_wlanhdr_to_poll(priv, txinsn.phdr);
	if (txinsn.pframe)
		release_mgtbuf_to_poll(priv, txinsn.pframe);

}

// GAS Initial Request with ANQP Query List for Neighbor Report and MBO Query List for Cellular Data Connection Preference
void issue_MBO_GAS_req(struct rtl8192cd_priv *priv, unsigned char type)
{
	unsigned char *pbuf;
	struct stat_info *pstat;
	unsigned char dialog_token = 4;
	DECLARE_TXINSN(txinsn);

	txinsn.q_num = MANAGE_QUE_NUM;
	txinsn.fr_type = _PRE_ALLOCMEM_;
	txinsn.tx_rate = find_rate(priv, NULL, 0, 1);
#ifndef TX_LOWESTRATE
	txinsn.lowest_tx_rate = txinsn.tx_rate;
#endif
	txinsn.fixed_rate = 1;
	pbuf = txinsn.pframe = get_mgtbuf_from_poll(priv);

	txinsn.phdr = get_wlanhdr_from_poll(priv);
	memset((void *)(txinsn.phdr), 0, sizeof(struct wlan_hdr));
	pbuf[0] = _PUBLIC_CATEGORY_ID_;
	pbuf[1] = type;
	pbuf[2] = dialog_token;

	/*----------------------------- Adventisement Protocol element -----------------------------*/
	pbuf[3] = _ADVT_PROTO_IE_; 		// The IE ID of Advertisement Protocol is 108.
	pbuf[4] = 2;					// Length
	pbuf[5] = 0;					// Query Response Length Limit
	pbuf[6] = 0;					// The Adverticement Protocol ID of ANQP is 0.
	/*------------------------------------------------------------------------------------------*/

	// Query Request Length = total number of octets in the Query Request field = 16
	pbuf[7] = 16 & 0x00ff;
	pbuf[8] = (16 & 0xff00) >> 8;

	// ANQP Query List info ID is 256.
	pbuf[9] = 256 & 0x00ff;
	pbuf[10] = (256 & 0xff00) >> 8;
	// Length = 2
	pbuf[11] = 2 & 0x00ff;
	pbuf[12] = (2 & 0xff00) >> 8;
	// ANQP Query List info: Neighbor Report = 272.
	pbuf[13] = 272 & 0x00ff;
	pbuf[14] = (272 & 0xff00) >> 8;
	txinsn.fr_len += 15;
	pbuf += 15;
	// Set MBO ANQP-element
	pbuf = construct_mbo_anqp_element(pbuf, &txinsn.fr_len);

	SetFrameSubType((txinsn.phdr), WIFI_WMM_ACTION);
	memcpy((void *)GetAddr1Ptr((txinsn.phdr)), BSSID, MACADDRLEN);
	//memset((void *)GetAddr1Ptr((txinsn.phdr)), 0xff, MACADDRLEN);
	memcpy((void *)GetAddr2Ptr((txinsn.phdr)), GET_MY_HWADDR, MACADDRLEN);
	// According to the MBO Spec, address 3 field set to the wildcard BSSID value
	memset((void *)GetAddr3Ptr((txinsn.phdr)), 0xff, MACADDRLEN);
	if ((rtl8192cd_firetx(priv, &txinsn)) == SUCCESS)
		printk("issue_MBO_GAS request success\n");
}

#endif
