enum{
	SIOCDE_GETPRIOINFO = 0x9020,
	SIOCDE_GETRADIOTIMEUSEINFO,
	SIOCDE_GETBSSINFO,
	SIOCDE_GETSTAWIFI6CAPABILITY,
	SIOCDE_GETRADIOCACINFO,
	SIOCDE_GETRADIOWIFI6APROLE,
	SIOCDE_GETRADIOWIFI6STAROLE,
	SIOCDE_SETDEVCACREQ,
	SIOCDE_GETDEVCACSTATUS,
	SIOCDE_GETSTALASTRATEINFO,
	SIOCDE_GETSTAUSEINFO,
	SIOCDE_GETSTATIDINFO,
	SIOCDE_GETASSOCDATA,
	SIOCDE_GETDISASSOCDATA,
	SIOCDE_GETFAILCONNDATA,
};

int rtw_prio_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_radio_time_use_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_bss_de_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_sta_wifi6_capablity_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_sta_tid_queue_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_radio_cac_capablity_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_radio_wifi6_ap_role_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_radio_wifi6_sta_role_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_dev_cac_status_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_sta_last_rate_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_sta_use_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_assoc_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_disassoc_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_fail_conn_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_dev_cac_status_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra);
int rtw_ioctl_de_private_get(struct net_device *dev, struct iwreq *wrq, u16 subcmd);
int rtw_ioctl_de_private_set(struct net_device *dev, struct iwreq *wrq, u16 subcmd);
