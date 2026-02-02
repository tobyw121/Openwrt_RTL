/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#define _HCI_INTF_C_

#include <drv_types.h>

#ifndef CONFIG_GSPI_HCI
#error "CONFIG_GSPI_HCI should be on!\n"
#endif

extern char *ifname;

typedef struct _driver_priv {
	int drv_registered;
} drv_priv, *pdrv_priv;

extern unsigned int oob_irq;
static drv_priv drvpriv = {

};

static void rtw_decide_chip_type_by_gspi_device_id(struct dvobj_priv *dvobj)
{
	/*dvobj->chip_type = pdid->driver_data;*/
}
static irqreturn_t spi_interrupt_thread(int irq, void *data)
{
	struct dvobj_priv *dvobj;
	PGSPI_DATA pgspi_data;


	dvobj = (struct dvobj_priv *)data;
	pgspi_data = dvobj_to_gspi(dvobj);

	/* spi_int_hdl(padapter); */
	if (pgspi_data->priv_wq)
		queue_delayed_work(pgspi_data->priv_wq, &pgspi_data->irq_work, 0);

	return IRQ_HANDLED;
}

static u8 gspi_alloc_irq(struct dvobj_priv *dvobj)
{
	PGSPI_DATA pgspi_data;
	struct spi_device *spi;
	int err;


	pgspi_data = dvobj_to_gspi(dvobj);
	spi = pgspi_data->func;

	err = request_irq(oob_irq, spi_interrupt_thread,
		IRQF_TRIGGER_FALLING,/* IRQF_TRIGGER_HIGH; */ /* |IRQF_ONESHOT, */
			  DRV_NAME, dvobj);
	/* err = request_threaded_irq(oob_irq, NULL, spi_interrupt_thread, */
	/*		IRQF_TRIGGER_FALLING, */
	/*		DRV_NAME, dvobj); */
	if (err < 0) {
		RTW_INFO("Oops: can't allocate irq %d err:%d\n", oob_irq, err);
		goto exit;
	}
	enable_irq_wake(oob_irq);
	/* disable_irq(oob_irq); */

exit:
	return err ? _FAIL : _SUCCESS;
}

static u8 gspi_init(struct dvobj_priv *dvobj)
{
	PGSPI_DATA pgspi_data;
	int err = 0;



	if (NULL == dvobj) {
		RTW_ERR("%s: driver object is NULL!\n", __func__);
		err = -1;
		goto exit;
	}

	pgspi_data = dvobj_to_gspi(dvobj);

	pgspi_data->block_transfer_len = 4;
	pgspi_data->tx_block_mode = 0;
	pgspi_data->rx_block_mode = 0;

exit:

	if (err)
		return _FAIL;
	return _SUCCESS;
}

static void gspi_deinit(struct dvobj_priv *dvobj)
{
	PGSPI_DATA pgspi_data;
	struct spi_device *spi;
	int err;



	if (NULL == dvobj) {
		RTW_ERR("%s: driver object is NULL!\n", __FUNCTION__);
		return;
	}

	pgspi_data = dvobj_to_gspi(dvobj);
	spi = pgspi_data->func;

	if (spi) {
		disable_irq(oob_irq);
		free_irq(oob_irq, dvobj);
	}
}

static struct dvobj_priv *gspi_dvobj_init(struct spi_device *spi)
{
	int status = _FAIL;
	struct dvobj_priv *dvobj = NULL;
	PGSPI_DATA pgspi;


	dvobj = devobj_init();
	if (dvobj == NULL)
		goto exit;

	/* spi init */
	/* This is the only SPI value that we need to set here, the rest
	 * comes from the board-peripherals file */
	spi->bits_per_word = 32;
	spi->max_speed_hz = 48 * 1000 * 1000;
	/* here mode 0 and 3 all ok, */
	/* 3 can run under 48M clock when SPI_CTL4 bit14 IS_FST set to 1 */
	/* 0 can run under 24M clock, but can run under 48M when SPI_CTL4 bit14 IS_FST set to 1 and Ctl0_reg[1:0] set to 3. */
	spi->mode = SPI_MODE_3;
	spi_setup(spi);

#if 1
	/* RTW_INFO("set spi ==========================%d\n", spi_setup(spi)); */

	RTW_INFO("%s, mode = %d\n", __func__, spi->mode);
	RTW_INFO("%s, bit_per_word = %d\n", __func__, spi->bits_per_word);
	RTW_INFO("%s, speed = %d\n", __func__, spi->max_speed_hz);
	RTW_INFO("%s, chip_select = %d\n", __func__, spi->chip_select);
	RTW_INFO("%s, controller_data = %d\n", __func__, *(int *)spi->controller_data);
	RTW_INFO("%s, irq= %d\n", __func__, oob_irq);
#endif

	spi_set_drvdata(spi, dvobj);
	pgspi = dvobj_to_gspi(dvobj);
	pgspi->func = spi;

	if (gspi_init(dvobj) != _SUCCESS) {
		RTW_INFO("%s: initialize GSPI Failed!\n", __FUNCTION__);
		goto free_dvobj;
	}

	dvobj->interface_type = RTW_GSPI;
	rtw_decide_chip_type_by_gspi_device_id(dvobj);

	rtw_reset_continual_io_error(dvobj);
	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && dvobj) {
		spi_set_drvdata(spi, NULL);

		devobj_deinit(dvobj);

		dvobj = NULL;
	}

exit:

	return dvobj;
}

static void gspi_dvobj_deinit(struct spi_device *spi)
{
	struct dvobj_priv *dvobj = spi_get_drvdata(spi);


	spi_set_drvdata(spi, NULL);
	if (dvobj) {
		gspi_deinit(dvobj);
		devobj_deinit(dvobj);
	}

}

static void spi_irq_work(void *data)
{
	struct delayed_work *dwork;
	PGSPI_DATA pgspi;
	struct dvobj_priv *dvobj;


	dwork = container_of(data, struct delayed_work, work);
	pgspi = container_of(dwork, GSPI_DATA, irq_work);

	dvobj = spi_get_drvdata(pgspi->func);
	if (!dvobj_get_primary_adapter(dvobj)) {
		RTW_INFO("%s primary adapter == NULL !!\n", __func__);
		return;
	}
	spi_int_hdl(dvobj_get_primary_adapter(dvobj));
}

static _adapter * rtw_gspi_primary_adapter_init(struct dvobj_priv *dvobj)
{
	int status = _FAIL;
	_adapter *padapter = NULL;
	u8 hw_mac_addr[ETH_ALEN] = {0};

	padapter = (_adapter *)rtw_zvmalloc(sizeof(*padapter));
	if (NULL == padapter)
		goto exit;

	if (rtw_load_registry(padapter) != _SUCCESS)
		goto free_adapter;

	padapter->dvobj = dvobj;

	dev_set_drv_stopped(adapter_to_dvobj(padapter)); /*init*/

	dvobj->padapters[dvobj->iface_nums++] = padapter;
	padapter->iface_id = IFACE_ID0;

	/* set adapter_type/iface type for primary padapter */
	padapter->isprimary = _TRUE;
	padapter->adapter_type = PRIMARY_ADAPTER;
	padapter->hw_port = HW_PORT0;


	/* 3 3. init driver special setting, interface, OS and hardware relative */
	/* 4 3.1 set hardware operation functions */
	/* hal_set_hal_ops(padapter); */
	if (rtw_set_hal_ops(padapter) == _FAIL)
		goto free_hal_data;

	if (rtw_init_io_priv(dvobj, spi_set_intf_ops) == _FAIL) {
		goto free_hal_data;
	}

#if 0 /*GEORGIA_TODO_REDEFINE_IO*/

	{
		u32 ret = 0;
		RTW_INFO("read start:\n");
		spi_write8_endian(padapter, SPI_LOCAL_OFFSET | 0xF0, 0x00, 1);
		/* rtw_write8(padapter, SPI_LOCAL_OFFSET | 0xF0, 0x03); */
		ret = rtw_read32(padapter, SPI_LOCAL_OFFSET | 0xF0);
		RTW_INFO("read end 0xF0 read32:%x:\n", ret);
		RTW_INFO("read end 0xF0 read8:%x:\n", rtw_read8(padapter, SPI_LOCAL_OFFSET | 0xF0));

	}
#endif
	rtw_hal_read_chip_version(padapter);

	rtw_hal_chip_configure(padapter);

	/* 3 6. read efuse/eeprom data */
	if (rtw_hal_read_chip_info(padapter) == _FAIL)
		goto free_hal_data;

	/* 3 7. init driver common data */
	if (rtw_init_drv_sw(padapter) == _FAIL) {
		goto free_hal_data;
	}

	/* get mac addr */
	rtw_hw_get_mac_addr(dvobj, hw_mac_addr);

	/* set mac addr */
	rtw_macaddr_cfg(adapter_mac_addr(padapter), hw_mac_addr);

	rtw_init_wifidirect_addrs(padapter, adapter_mac_addr(padapter), adapter_mac_addr(padapter));
#if 0 /*GEORGIA_TODO_REMOVE_IT_FOR_PHL_ARCH*/
	rtw_hal_disable_interrupt(GET_HAL_DATA(dvobj));
#endif
	RTW_INFO("bDriverStopped:%s, bSurpriseRemoved:%s, netif_up:%d, hw_init_completed:%s\n"
		 , dev_is_drv_stopped(dvobj) ? "True" : "False"
		 , dev_is_surprise_removed(dvobj) ? "True" : "False"
		 , padapter->netif_up
		 , rtw_hw_is_init_completed(dvobj) ? "True" : "False"
		);

	status = _SUCCESS;

free_hal_data:

free_adapter:
	if (status != _SUCCESS && padapter) {
		rtw_vmfree((u8 *)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_gspi_primary_adapter_deinit(_adapter *adapter)
{
	rtw_free_drv_sw(adapter);

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndev_free(adapter);

	rtw_vmfree(adapter, sizeof(_adapter));
}

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
 */
static int rtw_dev_probe(struct spi_device *spi)
{
	int status = _FAIL;
	struct dvobj_priv *dvobj;
	struct net_device *pnetdev;
	_adapter *padapter = NULL;


	RTW_INFO("RTW: %s line:%d", __FUNCTION__, __LINE__);

	dvobj = gspi_dvobj_init(spi);
	if (dvobj == NULL) {
		RTW_INFO("%s: Initialize device object priv Failed!\n", __FUNCTION__);
		goto exit;
	}

	padapter = rtw_gspi_primary_adapter_init(dvobj);
	if (padapter == NULL) {
		RTW_INFO("rtw_init_primary_adapter Failed!\n");
		goto free_dvobj;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_drv_add_vir_ifaces(dvobj) == _FAIL)
		goto free_if_vir;
#endif

	/* dev_alloc_name && register_netdev */
	if (rtw_os_ndevs_init(dvobj) != _SUCCESS)
		goto free_if_vir;

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(padapter);
#endif

	if (gspi_alloc_irq(dvobj) != _SUCCESS)
		goto os_ndevs_deinit;

#ifdef CONFIG_GLOBAL_UI_PID
	if (ui_pid[1] != 0) {
		RTW_INFO("ui_pid[1]:%d\n", ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}
#endif

	status = _SUCCESS;

os_ndevs_deinit:
	if (status != _SUCCESS)
		rtw_os_ndevs_deinit(dvobj);

#ifdef CONFIG_CONCURRENT_MODE
free_if_vir:
	if (status != _SUCCESS) {

		rtw_drv_stop_vir_ifaces(dvobj);
		rtw_drv_free_vir_ifaces(dvobj);
	}
#endif

	if (status != _SUCCESS && padapter)
		rtw_gspi_primary_adapter_deinit(padapter);

free_dvobj:
	if (status != _SUCCESS)
		gspi_dvobj_deinit(spi);

exit:
	return status == _SUCCESS ? 0 : -ENODEV;
}

static int /*__devexit*/  rtw_dev_remove(struct spi_device *spi)
{
	struct dvobj_priv *dvobj = spi_get_drvdata(spi);
	struct pwrctrl_priv *pwrctl = dvobj_to_pwrctl(dvobj);
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);

	dvobj->processing_dev_remove = _TRUE;

	/* TODO: use rtw_os_ndevs_deinit instead at the first stage of driver's dev deinit function */
	rtw_os_ndevs_unregister(dvobj);

#if defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_ANDROID_POWER)
	rtw_unregister_early_suspend(pwrctl);
#endif

#if 0 /*GEORGIA_TODO_FIXIT*/
	if (GET_HAL_DATA(dvobj)->fw_ready == _TRUE) {
		rtw_pm_set_ips(padapter, IPS_NONE);
		rtw_pm_set_lps(padapter, PM_PS_MODE_ACTIVE);
		LeaveAllPowerSaveMode(padapter);
	}
#endif
	dev_set_drv_stopped(adapter_to_dvobj(padapter));	/*for stop thread*/

#if 0 /*#ifdef CONFIG_CORE_CMD_THREAD*/
	rtw_stop_cmd_thread(padapter);
#endif
#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_stop_vir_ifaces(dvobj);
#endif
	rtw_drv_stop_prim_iface(padapter);

	if (rtw_hw_is_init_completed(dvobj)) 
		rtw_hw_stop(dvobj);
	dev_set_surprise_removed(dvobj);

	rtw_gspi_primary_adapter_deinit(padapter);

#ifdef CONFIG_CONCURRENT_MODE
	rtw_drv_free_vir_ifaces(dvobj);
#endif
	devobj_data_deinit(dvobj);
	gspi_dvobj_deinit(spi);



	return 0;
}


static int rtw_gspi_suspend(struct spi_device *spi, pm_message_t mesg)
{
	struct dvobj_priv *psdpriv  = spi_get_drvdata(spi);
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(psdpriv);
	_adapter *padapter = dvobj_get_primary_adapter(psdpriv);
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;
	int ret = 0;
	u8 ch, bw, offset;

	if (pwrpriv->bInSuspend == _TRUE) {
		RTW_INFO("%s bInSuspend = %d\n", __FUNCTION__, pwrpriv->bInSuspend);
		pdbgpriv->dbg_suspend_error_cnt++;
		goto exit;
	}

	ret = rtw_suspend_common(padapter);

exit:
	return ret;
}
int rtw_resume_process(_adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;

	if (pwrpriv->bInSuspend == _FALSE) {
		pdbgpriv->dbg_resume_error_cnt++;
		RTW_INFO("%s bInSuspend = %d\n", __FUNCTION__, pwrpriv->bInSuspend);
		return -1;
	}

	return rtw_resume_common(padapter);
}



static int rtw_gspi_resume(struct spi_device *spi)
{
	struct dvobj_priv *dvobj = spi_get_drvdata(spi);
	_adapter *padapter = dvobj_get_primary_adapter(dvobj);
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
	int ret = 0;


	RTW_INFO("==> %s (%s:%d)\n", __FUNCTION__, current->comm, current->pid);

#ifdef CONFIG_RESUME_IN_WORKQUEUE
	rtw_resume_in_workqueue(pwrpriv);
#else
	if (rtw_is_earlysuspend_registered(pwrpriv)
#ifdef CONFIG_WOWLAN
	    && !pwrpriv->wowlan_mode
#endif /* CONFIG_WOWLAN */
#ifdef CONFIG_AP_WOWLAN
	    && !pwrpriv->wowlan_ap_mode
#endif /* CONFIG_AP_WOWLAN*/
	   ) {
		/* jeff: bypass resume here, do in late_resume */
		rtw_set_do_late_resume(pwrpriv, _TRUE);
	} else {
		/* rtw_lock_suspend_timeout(4000); */
		rtw_resume_lock_suspend();

		ret = rtw_resume_process(padapter);
		rtw_resume_unlock_suspend();
	}
#endif /* CONFIG_RESUME_IN_WORKQUEUE */

	RTW_INFO("<========  %s return %d\n", __FUNCTION__, ret);
	return ret;

}


static struct spi_driver rtw_spi_drv = {
	.probe = rtw_dev_probe,
	.remove = rtw_dev_remove,
	.suspend = rtw_gspi_suspend,
	.resume = rtw_gspi_resume,
	.driver = {
		.name = "wlan_spi",
		.bus = &spi_bus_type,
		.owner = THIS_MODULE,
	}

};

static int __init rtw_drv_entry(void)
{
	int ret = 0;

	RTW_PRINT("module init start\n");
	dump_drv_version(RTW_DBGDUMP);
#ifdef BTCOEXVERSION
	RTW_PRINT(DRV_NAME" BT-Coex version = %s\n", BTCOEXVERSION);
#endif /* BTCOEXVERSION */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
	/* console_suspend_enabled=0; */
#endif

	drvpriv.drv_registered = _TRUE;
	rtw_suspend_lock_init();
	rtw_drv_proc_init();
	rtw_ndev_notifier_register();
	rtw_inetaddr_notifier_register();

	rtw_wifi_gpio_init();
	rtw_wifi_gpio_wlan_ctrl(WLAN_PWDN_ON);

	ret = spi_register_driver(&rtw_spi_drv);

	if (ret != 0) {
		drvpriv.drv_registered = _FALSE;
		rtw_suspend_lock_uninit();
		rtw_drv_proc_deinit();
		rtw_ndev_notifier_unregister();
		rtw_inetaddr_notifier_unregister();

		rtw_wifi_gpio_wlan_ctrl(WLAN_PWDN_OFF);
		rtw_wifi_gpio_deinit();

		goto exit;
	}

exit:
	RTW_PRINT("module init ret=%d\n", ret);
	return ret;
}

static void __exit rtw_drv_halt(void)
{
	RTW_PRINT("module exit start\n");

	drvpriv.drv_registered = _FALSE;

	spi_unregister_driver(&rtw_spi_drv);

	rtw_wifi_gpio_wlan_ctrl(WLAN_PWDN_OFF);
	rtw_wifi_gpio_deinit();

	rtw_suspend_lock_uninit();
	rtw_drv_proc_deinit();
	rtw_ndev_notifier_unregister();
	rtw_inetaddr_notifier_unregister();

	RTW_PRINT("module exit success\n");

	rtw_mstat_dump(RTW_DBGDUMP);
}
module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
