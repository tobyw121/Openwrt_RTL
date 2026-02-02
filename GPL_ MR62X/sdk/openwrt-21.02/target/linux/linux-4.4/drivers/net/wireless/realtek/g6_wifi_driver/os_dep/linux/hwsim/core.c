/*
 * Copyright(c) 2018 Realtek Corporation. All rights reserved.
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
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */
#include "rtw_hwsim.h"

/* based on pci_dvobj_init() */
static void rtw_hwsim_init_devobj(struct dvobj_priv *devobj, struct device *dev)
{
	devobj->interface_type = RTW_HCI_PCIE;

#if defined(CONFIG_RTL8852A)
	devobj->chip_type = RTL8852A;
#endif
}

struct dvobj_priv *rtw_hwsim_alloc_devobj(struct device *dev)
{
	struct dvobj_priv *devobj;

	devobj = devobj_init();
	if (devobj == NULL) {
		RTW_ERR("%s(): devobj_init failed\n", __func__);
		goto fail;
	}

	dev_set_drv_stopped(devobj);

	rtw_hwsim_init_devobj(devobj, dev);

	return devobj;

fail:
	return NULL;
}

void rtw_hwsim_free_devobj(struct dvobj_priv *devobj)
{
	devobj_deinit(devobj);
}

static void rtw_hwsim_set_intf_ops(struct _ADAPTER *padapter,
				   struct _io_ops *ops)
{
#if defined(CONFIG_RTL8852A)
	rtl8814be_set_intf_ops(ops);
#endif
}

static int rtw_hwsim_set_hal_ops(struct _ADAPTER *padapter)
{
	/* alloc memory for HAL DATA */
	if (rtw_hal_data_init(padapter) == _FAIL)
		return -1;

#if defined(CONFIG_RTL8814B)
//	if (rtw_get_chip_type(padapter) == RTL8814B)
		rtl8814be_set_hal_ops(padapter);
#endif

	if (rtw_hal_ops_check(padapter) == _FAIL)
		return -1;

	if (hal_spec_init(padapter) == _FAIL)
		return -1;

	return 0;
}

/**
 * rtw_hwsim_init_adapter - Initialize the adapter in vif
 * @vif: the virtual interface whose adapter is to be initialized
 * @devobj: the device object that governs the adapters
 * Returns: 0 on success or -1 if failed
 *
 * This function is modified from rtw_pci_primary_adapter_init() to initialize
 * an adapter structure without hw.
 */
int rtw_hwsim_init_adapter(struct rtw_hwsim_vif *vif)
{
	struct _adapter *padapter;
	struct dvobj_priv *devobj;

	padapter = vif_to_adapter(vif);
	devobj = vif_to_devobj(vif);
	padapter->dvobj = devobj;

	RTW_INFO("%s(): init padapter=0x%p, devobj=0x%p\n", __func__,
		 padapter, devobj);

	if (rtw_load_registry(padapter) != _SUCCESS) {
		RTW_ERR("%s(): load param failed\n", __func__);
		goto fail;
	}

	dev_set_drv_stopped(devobj);

	/* add adapter to devobj */
	devobj->padapters[devobj->iface_nums] = padapter;
	devobj->iface_nums++;

	padapter->iface_id = IFACE_ID0;
	padapter->isprimary = _TRUE;
	padapter->adapter_type = PRIMARY_ADAPTER;
	padapter->hw_port = HW_PORT0;

	if (rtw_init_io_priv(padapter, rtw_hwsim_set_intf_ops) == _FAIL) {
		RTW_ERR("%s(): init io failed\n", __func__);
		goto fail;
	}

	if (rtw_hwsim_set_hal_ops(padapter) < 0) {
		RTW_ERR("%s(): set hal ops failed\n", __func__);
		goto fail;
	}

	padapter->intf_start = NULL;
	padapter->intf_stop = NULL;

	if (rtw_init_drv_sw(padapter) == _FAIL) {
		RTW_ERR("%s(): init drv sw failed\n", __func__);
		goto fail;
	}

	if (rtw_hal_inirp_init(padapter) == _FAIL) {
		RTW_ERR("%s(): rtw_hal_inirp_init() failed\n", __func__);
		goto fail;
	}

	/* copy address to adapter */
	_rtw_memcpy(padapter->mac_addr, vif->address.addr, ETH_ALEN);

	return 0;

fail:
	return -1;
}

/*
 * Copied from rtw_pci_primary_adapter_deinit()
 */
void rtw_hwsim_deinit_adapter(struct _ADAPTER *padapter)
{
	struct mlme_priv *mlme = &padapter->mlmepriv;

/*	if (check_fwstate(mlme, WIFI_ASOC_STATE)) */
/*		rtw_disassoc_cmd(padapter, 0, RTW_CMDF_DIRECTLY); */

/* #ifdef CONFIG_AP_MODE */
/*	if (MLME_IS_AP(padapter) || MLME_IS_MESH(padapter)) { */
/*		free_mlme_ap_info(padapter); */
/* #ifdef CONFIG_HOSTAPD_MLME */
/*		hostapd_mode_unload(padapter); */
/* #endif */
/*	} */
/* #endif */

	rtw_cancel_all_timer(padapter);
/* #ifdef CONFIG_WOWLAN */
/*	adapter_to_pwrctl(padapter)->wowlan_mode = _FALSE; */
/* #endif /\* CONFIG_WOWLAN *\/ */
	rtw_dev_unload(padapter);

	rtw_hal_inirp_deinit(padapter);
	rtw_free_drv_sw(padapter);

/*	rtw_os_ndev_free(padapter); */

/* #ifdef RTW_HALMAC */
/*	rtw_halmac_deinit_adapter(adapter_to_dvobj(padapter)); */
/* #endif /\* RTW_HALMAC *\/ */

/* #ifdef CONFIG_PLATFORM_RTD2880B */
/*	RTW_INFO("wlan link down\n"); */
/*	rtd2885_wlan_netlink_sendMsg("linkdown", "8712"); */
/* #endif	 */
}
