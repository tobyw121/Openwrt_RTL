/*
 *  Open enhanced, Opportunistic Wireless Encryption
 *
 *  Copyright (c) 2019 Realtek Semiconductor Corp.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */
 
#ifndef _8192CD_OWE_H_
#define _8192CD_OWE_H_


/* function extern */
void owe_init(struct rtl8192cd_priv *priv);
void owe_free(struct rtl8192cd_priv *priv);
void owe_pmk_cache_init(void);
bool owe_is_root_open(struct rtl8192cd_priv *priv);
bool owe_is_owebss(struct rtl8192cd_priv *priv);
bool owe_construct_vd_ie(struct rtl8192cd_priv *priv);
void owe_set_itf_owebss(struct rtl8192cd_priv *priv);
int owe_assoc_req_process(struct rtl8192cd_priv *priv, unsigned char * cli_pub, unsigned char *pear_mac, unsigned char *rsnie);
void owe_calc_mic(OCTET_STRING EAPOLMsgSend, int algo, unsigned char *key, int keylen);
int owe_check_mic(struct rtl8192cd_priv *priv, OCTET_STRING EAPOLMsgRecvd, unsigned char *key, int keylen);
void owe_dbg_pmk_cache(struct rtl8192cd_priv *priv);
#endif	/* _8192CD_OWE_H_ */
