#include <drv_types.h>
#ifdef CONFIG_WLAN_DE_SUPPORT
int rtw_prio_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_radio_time_use_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_bss_de_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_sta_wifi6_capablity_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_sta_tid_queue_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_radio_cac_capablity_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_radio_wifi6_ap_role_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_radio_wifi6_sta_role_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_dev_cac_status_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_sta_last_rate_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_sta_use_info_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_assoc_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_disassoc_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_fail_conn_data_get(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_dev_cac_status_set(struct net_device *dev, struct iw_request_info *info, union iwreq_data *wrqu, char *extra)
{
	return 0;
}

int rtw_ioctl_de_private_get(struct net_device *dev, struct iwreq *wrq, u16 subcmd)
{
	u8 *extra = NULL;
	int ret= -1;

	extra = (u8 *)rtw_malloc(4096);
	if(extra==NULL){
		ret = -ENOMEM;
		return ret;
	}

	memset(extra,0,4096);

	switch (subcmd){
		case SIOCDE_GETPRIOINFO:
			ret = rtw_prio_info_get(dev, NULL, &wrq->u, extra);
			break;	
		case SIOCDE_GETRADIOTIMEUSEINFO:
			ret = rtw_radio_time_use_info_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETBSSINFO:
			ret = rtw_bss_de_info_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETSTAWIFI6CAPABILITY:
			ret = rtw_sta_wifi6_capablity_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETRADIOCACINFO:
			ret = rtw_radio_cac_capablity_get(dev,NULL,&wrq->u,extra);
			break;
		case SIOCDE_GETRADIOWIFI6APROLE:
			ret = rtw_radio_wifi6_ap_role_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETRADIOWIFI6STAROLE:
			ret = rtw_radio_wifi6_sta_role_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETDEVCACSTATUS:
			ret = rtw_dev_cac_status_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETSTALASTRATEINFO:
			ret = rtw_sta_last_rate_info_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETSTAUSEINFO:
			ret = rtw_sta_use_info_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETSTATIDINFO:
			ret = rtw_sta_tid_queue_info_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETASSOCDATA:
			ret = rtw_assoc_data_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETDISASSOCDATA:
			ret = rtw_disassoc_data_get(dev, NULL, &wrq->u, extra);
			break;
		case SIOCDE_GETFAILCONNDATA:
			ret = rtw_fail_conn_data_get(dev, NULL, &wrq->u, extra);
			break;
		default:
			ret = -EIO;
			break;		
	}

	if (copy_to_user(wrq->u.data.pointer, extra, wrq->u.data.length))
		ret = -EFAULT;

	rtw_mfree(extra, 4096);

	return ret;
}

int rtw_ioctl_de_private_set(struct net_device *dev, struct iwreq *wrq, u16 subcmd)
{
	int ret;

	switch (subcmd){
		case SIOCDE_SETDEVCACREQ:
			ret = rtw_dev_cac_status_set(dev, NULL, NULL, wrq->u.data.pointer);
			break;
		default:
			ret = -EIO;
			break;
	}

	return ret;
	
}
#endif

