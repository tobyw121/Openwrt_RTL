/******************************************************************************
 *
 * Copyright(c) 2007 - 2020 Realtek Corporation.
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
#if defined(CONFIG_MP_INCLUDED)
#include <rtw_efuse.h>

void rtw_efuse_cmd(_adapter *padapter,
							struct rtw_efuse_phl_arg *pefuse_arg ,
							enum rtw_efuse_phl_cmdid cmdid)
{
	u32 i = 0;
	pefuse_arg->mp_class = MP_CLASS_EFUSE;
	pefuse_arg->cmd = cmdid;
	pefuse_arg->cmd_ok = 0;

	rtw_mp_set_phl_cmd(padapter, (void*)pefuse_arg, sizeof(struct rtw_efuse_phl_arg));

	while (i != 10) {
			rtw_msleep_os(10);
			rtw_mp_get_phl_cmd(padapter, (void*)pefuse_arg,      sizeof(struct rtw_efuse_phl_arg));
			if (pefuse_arg->cmd_ok) {
				RTW_INFO("%s,eFuse GET CMD OK !!!\n", __func__);
				break;
			} else {
				rtw_msleep_os(10);
				if (i > 10) {
					RTW_INFO("%s, eFuse GET CMD FAIL !!!\n", __func__);
					break;
				}
				i++;
			}
	}
}

u32 rtw_efuse_get_map_size(_adapter *padapter , u16 *size , enum rtw_efuse_phl_cmdid cmdid)
{
	struct rtw_efuse_phl_arg *efuse_arg = NULL;
	u8 res = _FAIL;

	efuse_arg = _rtw_malloc(sizeof(struct rtw_efuse_phl_arg));
	if (efuse_arg) {
		_rtw_memset((void *)efuse_arg, 0, sizeof(struct rtw_efuse_phl_arg));
		rtw_efuse_cmd(padapter, efuse_arg, cmdid);
		if (efuse_arg->cmd_ok) {
				*size = efuse_arg->io_value;
				res = _SUCCESS;
		} else {
				*size = 0;
				res = _FAIL;
		}
	}
	if (efuse_arg)
		_rtw_mfree(efuse_arg, sizeof(struct rtw_efuse_phl_arg));

	return res;
}

u32 rtw_efuse_get_available_size(_adapter *padapter , u16 *size)
{
	struct rtw_efuse_phl_arg *efuse_arg = NULL;
	u8 res = _FAIL;

	efuse_arg = _rtw_malloc(sizeof(struct rtw_efuse_phl_arg));
	if (efuse_arg) {
		_rtw_memset((void *)efuse_arg, 0, sizeof(struct rtw_efuse_phl_arg));
		rtw_efuse_cmd(padapter, efuse_arg, RTW_EFUSE_CMD_WIFI_GET_AVL_SIZE);
		if (efuse_arg->cmd_ok) {
				*size = efuse_arg->io_value;
				res = _SUCCESS;
		} else {
				*size = 0;
				res = _FAIL;
		}
	}
	if (efuse_arg)
		_rtw_mfree(efuse_arg, sizeof(struct rtw_efuse_phl_arg));

	return res;
}

static u8 rtw_efuse_read_map2shadow(_adapter *padapter)
{
	struct rtw_efuse_phl_arg *efuse_arg = NULL;
	u8 res = _SUCCESS;

	efuse_arg = _rtw_malloc(sizeof(struct rtw_efuse_phl_arg));
	if (efuse_arg) {
		_rtw_memset((void *)efuse_arg, 0, sizeof(struct rtw_efuse_phl_arg));
		rtw_efuse_cmd(padapter, efuse_arg, RTW_EFUSE_CMD_WIFI_UPDATE_MAP);
		if (efuse_arg->cmd_ok)
				res = _SUCCESS;
		else
				res = _FAIL;
	}
	if (efuse_arg)
		_rtw_mfree(efuse_arg, sizeof(struct rtw_efuse_phl_arg));

	return res;
}

static u8 rtw_efuse_get_shadow_map(_adapter *padapter, u8 *map, u16 size)
{
	struct rtw_efuse_phl_arg *efuse_arg = NULL;
	u8 res = _FAIL;

	efuse_arg = _rtw_malloc(sizeof(struct rtw_efuse_phl_arg));
	if (efuse_arg) {

		efuse_arg->buf_len = size;
		rtw_efuse_cmd(padapter, efuse_arg, RTW_EFUSE_CMD_SHADOW_MAP2BUF);
		if (efuse_arg->cmd_ok) {
				_rtw_memcpy((void *)map, efuse_arg->poutbuf, size);
				res = _SUCCESS;
		} else
				res = _FAIL;
	}
	if (efuse_arg)
		_rtw_mfree(efuse_arg, sizeof(struct rtw_efuse_phl_arg));

	return res;
}


u8 rtw_efuse_map_read(_adapter * adapter, u16 addr, u16 cnts, u8 *data)
{
	struct dvobj_priv *d;
	u8 *efuse = NULL;
	u16 size, i;
	int err;
	u8 status = _SUCCESS;

	err = rtw_efuse_get_map_size(adapter, &size, RTW_EFUSE_CMD_WIFI_GET_LOG_SIZE);
	if (err == _FAIL) {
		status = _FAIL;
		RTW_INFO("halmac_get_logical_efuse_size fail\n");
		goto exit;
	}
	/* size error handle */
	if ((addr + cnts) > size) {
		if (addr < size)
			cnts = size - addr;
		else {
			status = _FAIL;
			RTW_INFO(" %s() ,addr + cnts) > size fail\n", __func__);
			goto exit;
		}
	}

	efuse = rtw_zmalloc(size);
	if (efuse) {
		if (rtw_efuse_read_map2shadow(adapter) == _SUCCESS) {
			err = rtw_efuse_get_shadow_map(adapter, efuse, size -1);
			if (err == _FAIL) {
				rtw_mfree(efuse, size);
				status = _FAIL;
				RTW_INFO(" %s() ,halmac_read_logical_efus map fail\n", __func__);
				goto exit;
			}
		} else {
			RTW_INFO(" %s() ,rtw_efuse_read_map2shadow FAIL !!!\n", __func__);
			rtw_mfree(efuse, size);
			status = _FAIL;
			goto exit;
		}
		if (efuse) {
			RTW_INFO(" %s() ,cp efuse to data\n", __func__);
			_rtw_memcpy(data, efuse + addr, cnts);
			rtw_mfree(efuse, size);
		}
	} else {
			RTW_INFO(" %s() ,alloc efuse fail\n", __func__);
			goto exit;
	}
	status = _SUCCESS;
exit:

	return status;
}

static u8 rtw_efuse_map_file_load(_adapter *padapter, u8 *filepath)
{
	struct rtw_efuse_phl_arg *efuse_arg = NULL;
	u8 res = _FAIL;

	if (filepath) {
		RTW_INFO("efuse file path %s len %zu", filepath, strlen(filepath));

		efuse_arg = _rtw_malloc(sizeof(struct rtw_efuse_phl_arg));
		if (efuse_arg) {
			_rtw_memset((void *)efuse_arg, 0, sizeof(struct rtw_efuse_phl_arg));
			_rtw_memcpy(efuse_arg->pfile_path, filepath, strlen(filepath));
			rtw_efuse_cmd(padapter, efuse_arg, RTW_EFUSE_CMD_FILE_MAP_LOAD);
		}
	}

	if (efuse_arg) {
		if (efuse_arg->cmd_ok)
			res = _SUCCESS;
		else
			res = _FAIL;
	_rtw_mfree(efuse_arg, sizeof(struct rtw_efuse_phl_arg));
	}

	return res;
}

static u8 rtw_efuse_mask_file_load(_adapter *padapter, u8 *filepath)
{
	struct rtw_efuse_phl_arg *efuse_arg = NULL;
	u8 res = _FAIL;

	if (filepath) {
		RTW_INFO("efuse file path %s len %zu", filepath, strlen(filepath));
		efuse_arg = _rtw_malloc(sizeof(struct rtw_efuse_phl_arg));
		if (efuse_arg) {
			_rtw_memset((void *)efuse_arg, 0, sizeof(struct rtw_efuse_phl_arg));
			_rtw_memcpy(efuse_arg->pfile_path, filepath, strlen(filepath));
			rtw_efuse_cmd(padapter, efuse_arg, RTW_EFUSE_CMD_FILE_MASK_LOAD);
		}
	}

	if (efuse_arg) {
		if (efuse_arg->cmd_ok)
			res = _SUCCESS;
		else
			res = _FAIL;
		_rtw_mfree(efuse_arg, sizeof(struct rtw_efuse_phl_arg));
	}

	return res;
}

int rtw_ioctl_efuse_get(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	_adapter *padapter = rtw_netdev_priv(dev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	char *pch, *ptmp, *token, *tmp[3] = {0x00, 0x00, 0x00};
	u16 i = 0, j = 0, mapLen = 0, addr = 0, cnts = 0;
	int err = 0;
	char *pextra = NULL;
	u8 *pre_efuse_map = NULL;
	u16 max_available_len = 0;

	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length)) {
		err = -EFAULT;
		RTW_INFO("%s: copy_from_user fail!!\n", __FUNCTION__);
		//goto exit;
		return err;
	}

	*(extra +  wrqu->data.length) = '\0';
	pch = extra;
	RTW_INFO("%s: in=%s\n", __FUNCTION__, extra);

	i = 0;
	/* mac 16 "00e04c871200" rmap,00,2 */
	while ((token = strsep(&pch, ",")) != NULL) {
		if (i > 2)
			break;
		tmp[i] = token;
		i++;
	}
	pre_efuse_map = rtw_zmalloc(RTW_MAX_EFUSE_MAP_LEN);
	if (pre_efuse_map == NULL)
			goto exit;

	if (strcmp(tmp[0], "status") == 0) {
		//sprintf(extra, "Load File efuse=%s,Load File MAC=%s"
		//	, efuse->file_status == EFUSE_FILE_FAILED ? "FAIL" : "OK"
		//	, pHalData->macaddr_file_status == MACADDR_FILE_FAILED ? "FAIL" : "OK"
		//       );
		goto exit;
	} else if (strcmp(tmp[0], "drvmap") == 0) {
		static u8 drvmaporder = 0;
		u8 *efuse_data = NULL;
		u32 shift, cnt;
		u32 blksz = 0x200; /* The size of one time show, default 512 */
		//EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, _FALSE);

		//efuse_data = efuse->data;

		shift = blksz * drvmaporder;
		efuse_data += shift;
		cnt = mapLen - shift;

		if (cnt > blksz) {
			cnt = blksz;
			drvmaporder++;
		} else
			drvmaporder = 0;

		sprintf(extra, "\n");
		for (i = 0; i < cnt; i += 16) {
			pextra = extra + strlen(extra);
			pextra += sprintf(pextra, "0x%02x\t", shift + i);
			for (j = 0; j < 8; j++)
				pextra += sprintf(pextra, "%02X ", efuse_data[i + j]);
			pextra += sprintf(pextra, "\t");
			for (; j < 16; j++)
				pextra += sprintf(pextra, "%02X ", efuse_data[i + j]);
			pextra += sprintf(pextra, "\n");
		}
		if ((shift + cnt) < mapLen)
			pextra += sprintf(pextra, "\t...more (left:%d/%d)\n", mapLen-(shift + cnt), mapLen);

	} else if (strcmp(tmp[0], "realmap") == 0) {
		static u8 order = 0;
		u32 shift, cnt;
		u32 blksz = 0x200; /* The size of one time show, default 512 */
		u8 *efuse_data = NULL;

		rtw_efuse_get_map_size(padapter, &mapLen, RTW_EFUSE_CMD_WIFI_GET_LOG_SIZE);
		mapLen -= 1;

 		if (pre_efuse_map) {
			if (rtw_efuse_map_read(padapter, 0, mapLen, pre_efuse_map) == _FAIL) {
				RTW_INFO("%s: read realmap Fail!!\n", __FUNCTION__);
				err = -EFAULT;
			} else {
				efuse_data = pre_efuse_map;

				_rtw_memset(extra, '\0', strlen(extra));
				shift = blksz * order;
				efuse_data += shift;
				cnt = mapLen - shift;
				if (cnt > blksz) {
					cnt = blksz;
					order++;
				} else
					order = 0;

				sprintf(extra, "\n");
				for (i = 0; i < cnt; i += 16) {
					pextra = extra + strlen(extra);
					pextra += sprintf(pextra, "0x%02x\t", shift + i);
					for (j = 0; j < 8; j++)
						pextra += sprintf(pextra, "%02X ", efuse_data[i + j]);
					pextra += sprintf(pextra, "\t");
					for (; j < 16; j++)
						pextra += sprintf(pextra, "%02X ", efuse_data[i + j]);
					pextra += sprintf(pextra, "\n");

					if (strlen(extra) > 0x7FF)
						break;
				}
				if ((shift + cnt) < mapLen)
					pextra += sprintf(pextra, "\t...more (left:%d/%d)\n", mapLen - (shift + cnt), mapLen);
				}
		}
	} else if (strcmp(tmp[0], "rmap") == 0) {
		u8 *data = NULL;
		if ((tmp[1] == NULL) || (tmp[2] == NULL)) {
			RTW_INFO("%s: rmap Fail!! Parameters error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		/* rmap addr cnts */
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		RTW_INFO("%s: addr=%x\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0) {
			RTW_INFO("%s: rmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		RTW_INFO("%s: cnts=%d\n", __FUNCTION__, cnts);

		rtw_efuse_get_map_size(padapter, &mapLen, RTW_EFUSE_CMD_WIFI_GET_LOG_SIZE);

		if ((addr + cnts) > mapLen) {
			RTW_INFO("%s: addr(0x%X)+cnts(%d) over mapLen %d parameter error!\n", __FUNCTION__, addr, cnts, mapLen);
			err = -EINVAL;
			goto exit;
		}

		if (pre_efuse_map) {
			if (rtw_efuse_map_read(padapter, addr, cnts, pre_efuse_map) == _FAIL) {
				RTW_INFO("%s: rtw_efuse_map_read Fail!!\n", __FUNCTION__);
				err = -EFAULT;
			} else {
				data = pre_efuse_map;
				*extra = 0;
				pextra = extra;
				for (i = 0; i < cnts; i++) {
					pextra += sprintf(pextra, "0x%02X ", data[i]);
				}
			}
		}
	} else if (strcmp(tmp[0], "ableraw") == 0) {
            rtw_efuse_get_available_size(padapter, &max_available_len);
            sprintf(extra, "[available raw size]= %d bytes\n", max_available_len);
        } else
		sprintf(extra, "Command not found!");

exit:
	if (pre_efuse_map)
		rtw_mfree(pre_efuse_map, RTW_MAX_EFUSE_MAP_LEN);
	if (!err)
		wrqu->data.length = strlen(extra);
	RTW_INFO("%s: strlen(extra) =%zu\n", __FUNCTION__, strlen(extra));
	if(!copy_to_user(wrqu->data.pointer, extra, wrqu->data.length))
		err = -EFAULT;

	return err;
}


int rtw_ioctl_efuse_set(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wdata, char *extra)
{
	struct iw_point *wrqu;
	_adapter *padapter;
	struct pwrctrl_priv *pwrctrlpriv ;

	u8 ips_mode = IPS_NUM; /* init invalid value */
	u8 lps_mode = PM_PS_MODE_NUM; /* init invalid value */
	u32 i = 0, j = 0, jj, kk;
	//u8 *setdata = NULL;
	u8 *ShadowMapBT = NULL;
	u8 *ShadowMapWiFi = NULL;
	u8 *setrawdata = NULL;
	char *pch, *ptmp, *token, *tmp[3] = {0x00, 0x00, 0x00};
	u16 addr = 0xFF, cnts = 0, BTStatus = 0 , max_available_len = 0;
	u16 wifimaplen;
	int err;
	boolean bcmpchk = _TRUE;

	wrqu = (struct iw_point *)wdata;
	padapter = rtw_netdev_priv(dev);
	pwrctrlpriv = adapter_to_pwrctl(padapter);
	//pHalData = GET_HAL_DATA(adapter_to_dvobj(padapter));
	//pEfuseHal = &pHalData->EfuseHal;
	//pHalFunc = &hal->hal_func;

	err = 0;

	if (copy_from_user(extra, wrqu->pointer, wrqu->length))
		return -EFAULT;

	*(extra + wrqu->length) = '\0';

#ifdef CONFIG_LPS
	lps_mode = pwrctrlpriv->power_mgnt;/* keep org value */
	rtw_pm_set_lps(padapter, PM_PS_MODE_ACTIVE);
#endif

#ifdef CONFIG_IPS
	ips_mode = pwrctrlpriv->ips_mode;/* keep org value */
	rtw_pm_set_ips(padapter, IPS_NONE);
#endif

	pch = extra;
	RTW_INFO("%s: in=%s\n", __FUNCTION__, extra);

	i = 0;
	while ((token = strsep(&pch, ",")) != NULL) {
		if (i > 2)
			break;
		tmp[i] = token;
		i++;
	}

	/* tmp[0],[1],[2] */
	/* wmap,addr,00e04c871200 */
	if (strcmp(tmp[0], "wmap") == 0) {
		if ((tmp[1] == NULL) || (tmp[2] == NULL)) {
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts % 2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		RTW_INFO("%s: addr=0x%X\n", __FUNCTION__, addr);
		RTW_INFO("%s: cnts=%d\n", __FUNCTION__, cnts);
		RTW_INFO("%s: map data=%s\n", __FUNCTION__, tmp[2]);

		//for (jj = 0, kk = 0; jj < cnts; jj++, kk += 2)
		//	setdata[jj] = key_2char2num(tmp[2][kk], tmp[2][kk + 1]);

		//EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&max_available_len, _FALSE);

		if ((addr + cnts) > max_available_len) {
			RTW_INFO("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}
/*
		if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL) {
			RTW_INFO("%s: rtw_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		*extra = 0;
		RTW_INFO("%s: after rtw_efuse_map_write to _rtw_memcmp\n", __func__);

		if (rtw_efuse_mask_map_read(padapter, addr, cnts, ShadowMapWiFi) == _SUCCESS) {
			if (_rtw_memcmp((void *)ShadowMapWiFi , (void *)setdata, cnts)) {
				RTW_INFO("%s: WiFi write map afterf compare success\n", __FUNCTION__);
				sprintf(extra, "WiFi write map compare OK\n");
				err = 0;
				goto exit;
			} else {
				sprintf(extra, "WiFi write map compare FAIL\n");
				RTW_INFO("%s: WiFi write map compare Fail\n", __FUNCTION__);
				err = 0;
				goto exit;
			}
		}
*/
		}
exit:

	wrqu->length = strlen(extra);

	if (padapter->registrypriv.mp_mode == 0) {
#ifdef CONFIG_IPS
		rtw_pm_set_ips(padapter, ips_mode);
#endif /* CONFIG_IPS */

#ifdef CONFIG_LPS
		rtw_pm_set_lps(padapter, lps_mode);
#endif /* CONFIG_LPS */
	}

	return err;
}

int rtw_ioctl_efuse_file_map_load(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	char *rtw_efuse_file_map_path;
	u8 Status = 0;
	_adapter *padapter= rtw_netdev_priv(dev);
	struct mp_priv *pmp_priv = &padapter->mppriv;

	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	rtw_efuse_file_map_path = extra;
	rtw_efuse_file_map_path[wrqu->data.length] = '\0';

	if (rtw_is_file_readable(rtw_efuse_file_map_path) == _TRUE) {
		RTW_INFO("%s do rtw_efuse_mask_file_read = %s!\n", __func__, rtw_efuse_file_map_path);
		Status = rtw_efuse_map_file_load(padapter, rtw_efuse_file_map_path);
		if (Status == _TRUE) {
			pmp_priv->bloadefusemap = _TRUE;
			sprintf(extra, "efuse Map file file_read OK\n");
		} else {
			pmp_priv->bloadefusemap = _FALSE;
			sprintf(extra, "efuse Map file file_read FAIL\n");
		}
	} else {
		sprintf(extra, "efuse file readable FAIL\n");
		RTW_INFO("%s rtw_is_file_readable fail!\n", __func__);
	}

	wrqu->data.length = strlen(extra);
	return 0;
}

int rtw_ioctl_efuse_file_mask_load(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	char *rtw_efuse_file_map_path;
	u8 Status = 0;
	_adapter *padapter= rtw_netdev_priv(dev);
	struct mp_priv *pmp_priv = &padapter->mppriv;

	if (copy_from_user(extra, wrqu->data.pointer, wrqu->data.length))
		return -EFAULT;

	rtw_efuse_file_map_path = extra;

	rtw_efuse_file_map_path[wrqu->data.length] = '\0';

	if (rtw_is_file_readable(rtw_efuse_file_map_path) == _TRUE) {
		RTW_INFO("%s do rtw_efuse_mask_file_read = %s!\n", __func__, rtw_efuse_file_map_path);
		Status = rtw_efuse_mask_file_load(padapter, rtw_efuse_file_map_path);
		if (Status == _TRUE) {
			pmp_priv->bloadefusemap = _TRUE;
			sprintf(extra, "efuse Mask file file_read OK\n");
		} else {
			pmp_priv->bloadefusemap = _FALSE;
			sprintf(extra, "efuse Mask file file_read FAIL\n");
		}
	} else {
		sprintf(extra, "efuse file readable FAIL\n");
		RTW_INFO("%s rtw_is_file_readable fail!\n", __func__);
	}

	wrqu->data.length = strlen(extra);
	return 0;
}

#endif /*#if defined(CONFIG_MP_INCLUDED)*/

