
#ifndef _8192CD_MBO_H_
#define _8192CD_MBO_H_

#ifdef CONFIG_MBO

#define MBO_RSSI_THRESHOLD			(priv->pmib->multiBand.mbo_rssi_threshold)		// 0
#define MBO_STA_NUM					(priv->pmib->multiBand.mbo_max_sta_num)			// 64	//20180413
#define MBO_CH_UTIL					(priv->pmib->multiBand.mbo_ch_util_threshold)	// 250	//20180413

enum _REASON_CODE_IN_ASSOCIATION_DISALLOWED_ATTRIBUTE {
	UNSPECIFIED_REASON							= 1,
	MAXIMUM_NUMBER_OF_ASSOCIATED_STAS_REACHED	= 2,
	AIR_INTERFACE_IS_OVERLOADED					= 3,
	AUTHENTICATION_SERVER_OVERLOADED			= 4,
	INSUFFICIENT_RSSI							= 5
};

unsigned int check_association_disallowed(struct rtl8192cd_priv *priv, int frame_type, unsigned char rssi);
unsigned char *set_mbo_ap_capability_indication_attr(unsigned char *pbuf, unsigned int *frlen);
unsigned char *set_association_disallowed_attr(unsigned char *pbuf, unsigned int *frlen, unsigned int assoc_disallowed_status);
unsigned char *set_transition_reason_code_attr(unsigned char *pbuf, unsigned int *frlen);
unsigned char *set_association_retry_delay_attr(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen);
unsigned char *construct_mbo_ie(unsigned char *pbuf, unsigned int *frlen, unsigned int assoc_disallow);
unsigned char *construct_btm_req_mbo_ie(struct rtl8192cd_priv *priv, unsigned char *pbuf, unsigned int *frlen);
#endif


#ifdef CONFIG_MBO_CLI
unsigned char *set_cellular_data_capabilities_attr(unsigned char *pbuf, unsigned int *frlen);
unsigned char *set_transition_rejection_reason_code_attr(unsigned char *pbuf, unsigned int *frlen);
unsigned char *set_non_preferred_channel_report_attr(unsigned char *pbuf, unsigned int *frlen, unsigned char channel, unsigned char op_class, unsigned char preference, unsigned char reason);
unsigned char *construct_probe_req_mbo_ie(unsigned char *pbuf, unsigned int *frlen);
unsigned char *construct_btm_rsp_mbo_ie(unsigned char *pbuf, unsigned int *frlen);
unsigned char *construct_assoc_req_mbo_ie(unsigned char *pbuf, unsigned int *frlen, unsigned char channel, unsigned char op_class, unsigned char preference, unsigned char reason);
unsigned char *construct_mbo_anqp_element(unsigned char *pbuf, unsigned int *frlen);
void issue_WNM_Notify_channel_report(struct rtl8192cd_priv *priv);
void issue_MBO_GAS_req(struct rtl8192cd_priv *priv, unsigned char type);
#endif


#endif
