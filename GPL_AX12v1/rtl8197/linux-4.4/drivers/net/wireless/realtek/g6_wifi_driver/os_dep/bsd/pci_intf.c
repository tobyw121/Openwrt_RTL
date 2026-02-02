/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
 *
 *
 ******************************************************************************/
#define _HCI_INTF_C_

#include <drv_types.h>

#ifndef CONFIG_PCI_HCI

#error "CONFIG_PCI_HCI shall be on!\n"

#endif


#if defined (PLATFORM_LINUX) && defined (PLATFORM_WINDOWS)

#error "Shall be Linux or Windows, but not both!\n"

#endif

#ifdef CONFIG_80211N_HT
extern int rtw_ht_enable;
extern int rtw_bw_mode;
extern int rtw_ampdu_enable;//for enable tx_ampdu
#endif

#ifdef CONFIG_PM
extern int pm_netdev_open(struct net_device *pnetdev);
static int rtw_suspend(struct pci_dev *pdev, pm_message_t state);
static int rtw_resume(struct pci_dev *pdev);
#endif


static int rtw_drv_init(struct pci_dev *pdev, const struct pci_device_id *pdid);
static void rtw_dev_remove(struct pci_dev *pdev);

static struct specific_device_id specific_device_id_tbl[] = {
	{.idVendor=0x0b05, .idProduct=0x1791, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3311, .flags=SPEC_DEV_ID_DISABLE_HT},
	{}
};

struct pci_device_id rtw_pci_id_tbl[] = {
#ifdef CONFIG_RTL8192C
	{PCI_DEVICE(PCI_VENDER_ID_REALTEK, 0x8191)},
	{PCI_DEVICE(PCI_VENDER_ID_REALTEK, 0x8178)},
	{PCI_DEVICE(PCI_VENDER_ID_REALTEK, 0x8177)},
	{PCI_DEVICE(PCI_VENDER_ID_REALTEK, 0x8176)},
#endif
#ifdef CONFIG_RTL8192D
	{PCI_DEVICE(PCI_VENDER_ID_REALTEK, 0x8193)},
	{PCI_DEVICE(PCI_VENDER_ID_REALTEK, 0x002B)},
#endif
	{},
};

struct pci_drv_priv {
	struct pci_driver rtw_pci_drv;
	int drv_registered;
};


static struct pci_drv_priv pci_drvpriv = {
	.rtw_pci_drv.name = (char*)DRV_NAME,
	.rtw_pci_drv.probe = rtw_drv_init,
	.rtw_pci_drv.remove = rtw_dev_remove,
	.rtw_pci_drv.id_table = rtw_pci_id_tbl,
#ifdef CONFIG_PM	
	.rtw_pci_drv.suspend = rtw_suspend,
	.rtw_pci_drv.resume = rtw_resume,
#else	
	.rtw_pci_drv.suspend = NULL,
	.rtw_pci_drv.resume = NULL,
#endif
};


MODULE_DEVICE_TABLE(pci, rtw_pci_id_tbl);

static u8 rtw_pci_get_amd_l1_patch(_adapter *padapter)
{
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct pci_dev	*pdev = pdvobjpriv->ppcidev;
	struct pci_dev	*bridge_pdev = pdev->bus->self;
	u8	status = _FALSE;
	u8	offset_e0;
	u32	offset_e4;

	//NdisRawWritePortUlong(PCI_CONF_ADDRESS,pcicfg_addrport + 0xE0);
	//NdisRawWritePortUchar(PCI_CONF_DATA, 0xA0);
	pci_write_config_byte(bridge_pdev, 0xE0, 0xA0);

	//NdisRawWritePortUlong(PCI_CONF_ADDRESS,pcicfg_addrport + 0xE0);
	//NdisRawReadPortUchar(PCI_CONF_DATA, &offset_e0);
	pci_read_config_byte(bridge_pdev, 0xE0, &offset_e0);

	if (offset_e0 == 0xA0) {
		//NdisRawWritePortUlong(PCI_CONF_ADDRESS, pcicfg_addrport + 0xE4);
		//NdisRawReadPortUlong(PCI_CONF_DATA, &offset_e4);
		pci_read_config_dword(bridge_pdev, 0xE4, &offset_e4);
		if (offset_e4 & BIT(23))
			status = _TRUE;
	}

	return status;
}

static void rtw_pci_parse_configuration(struct pci_dev *pdev, _adapter *padapter)
{
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct pci_priv	*pcipriv = &(pdvobjpriv->pcipriv);
	u8 tmp;
	int pos;
	u8 linkctrl_reg;

	//Link Control Register
	pos = pci_find_capability(pdev, PCI_CAP_ID_EXP);
	pci_read_config_byte(pdev, pos + PCI_EXP_LNKCTL, &linkctrl_reg);
	pcipriv->linkctrl_reg = linkctrl_reg;

	//RTW_INFO("Link Control Register = %x\n", pcipriv->linkctrl_reg);

	pci_read_config_byte(pdev, 0x98, &tmp);
	tmp |= BIT(4);
	pci_write_config_byte(pdev, 0x98, tmp);

	//tmp = 0x17;
	//pci_write_config_byte(pdev, 0x70f, tmp);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)) || (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18))
#define rtw_pci_interrupt(x,y,z) rtw_pci_interrupt(x,y)
#endif

static irqreturn_t rtw_pci_interrupt(int irq, void *priv, struct pt_regs *regs)
{
	_adapter			*padapter = (_adapter *)priv;
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;


	if (pdvobjpriv->irq_enabled == 0) {
		return IRQ_HANDLED;
	}

	if (padapter->hal_func.interrupt_handler(padapter) == _FAIL)
		return IRQ_HANDLED;
		//return IRQ_NONE;

	return IRQ_HANDLED;
}

static u32 pci_dvobj_init(_adapter *padapter)
{
	u32	status = _SUCCESS;
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct pci_priv	*pcipriv = &(pdvobjpriv->pcipriv);
	struct pci_dev	*pdev = pdvobjpriv->ppcidev;
	struct pci_dev	*bridge_pdev = pdev->bus->self;
	u8	tmp;


#if 1
	if(bridge_pdev){
		pci_read_config_byte(bridge_pdev,
				     bridge_pdev->pcie_cap + PCI_EXP_LNKCTL,
				     &pcipriv->pcibridge_linkctrlreg);

		if (bridge_pdev->vendor == AMD_VENDOR_ID)
			pcipriv->amd_l1_patch = rtw_pci_get_amd_l1_patch(padapter);
	}
#else
	//
	// Find bridge related info. 
	//
	rtw_get_pci_bus_info(padapter,
				  pdev->vendor,
				  pdev->device,
				  (u8) pdvobjpriv->irqline,
				  0x02, 0x80, U1DONTCARE,
				  &pcipriv->busnumber,
				  &pcipriv->devnumber,
				  &pcipriv->funcnumber);

	rtw_find_bridge_info(padapter);

	if (bridge_pdev) {
		rtw_get_link_control_field(padapter,
						pcipriv->pcibridge_busnum,
						pcipriv->pcibridge_devnum,
						pcipriv->pcibridge_funcnum);

		if (bridge_pdev->vendor == AMD_VENDOR_ID) {
			pcipriv->amd_l1_patch =
				rtw_get_amd_l1_patch(padapter,
							pcipriv->pcibridge_busnum,
							pcipriv->pcibridge_devnum,
							pcipriv->pcibridge_funcnum);
		}
	}
#endif

	//
	// Allow the hardware to look at PCI config information.
	//
	rtw_pci_parse_configuration(pdev, padapter);

	RTW_INFO("pcidev busnumber:devnumber:funcnumber:"
		"vendor:link_ctl %d:%d:%d:%x:%x\n",
		pcipriv->busnumber,
		pcipriv->devnumber,
		pcipriv->funcnumber,
		pdev->vendor,
		pcipriv->linkctrl_reg);

	RTW_INFO("pci_bridge busnumber:devnumber:funcnumber:vendor:"
		"pcie_cap:link_ctl_reg: %d:%d:%d:%x:%x:%x:%x\n", 
		pcipriv->pcibridge_busnum,
		pcipriv->pcibridge_devnum,
		pcipriv->pcibridge_funcnum,
		(bridge_pdev ? bridge_pdev->vendor : 0x0),
		(bridge_pdev ? bridge_pdev->pcie_cap : 0x0),
		pcipriv->pcibridge_linkctrlreg,
		pcipriv->amd_l1_patch);

	//.2
	if ((init_io_priv(padapter)) == _FAIL)
	{
		status = _FAIL;
	}
	rtw_reset_continual_io_error(pdvobjpriv);

	//.3
	intf_read_chip_version(padapter);
	//.4
	intf_chip_configure(padapter);


	return status;
}

static void pci_dvobj_deinit(_adapter * padapter)
{
	//struct dvobj_priv *pdvobjpriv=&padapter->dvobjpriv;


}


static void decide_chip_type_by_pci_device_id(_adapter *padapter, struct pci_dev *pdev)
{
	u16	venderid, deviceid, irqline;
	u8	revisionid;
	struct dvobj_priv	*pdvobjpriv=&padapter->dvobjpriv;


	venderid = pdev->vendor;
	deviceid = pdev->device;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23))
	pci_read_config_byte(pdev, PCI_REVISION_ID, &revisionid); // PCI_REVISION_ID 0x08
#else
	revisionid = pdev->revision;
#endif
	pci_read_config_word(pdev, PCI_INTERRUPT_LINE, &irqline); // PCI_INTERRUPT_LINE 0x3c
	pdvobjpriv->irqline = irqline;


	//
	// Decide hardware type here. 
	//
	if( deviceid == HAL_HW_PCI_8185_DEVICE_ID ||
	    deviceid == HAL_HW_PCI_8188_DEVICE_ID ||
	    deviceid == HAL_HW_PCI_8198_DEVICE_ID)
	{
		RTW_INFO("Adapter (8185/8185B) is found- VendorID/DeviceID=%x/%x\n", venderid, deviceid);
		padapter->HardwareType=HARDWARE_TYPE_RTL8185;
	}
	else if (deviceid == HAL_HW_PCI_8190_DEVICE_ID ||
		deviceid == HAL_HW_PCI_0045_DEVICE_ID ||
		deviceid == HAL_HW_PCI_0046_DEVICE_ID ||
		deviceid == HAL_HW_PCI_DLINK_DEVICE_ID)
	{
		RTW_INFO("Adapter(8190 PCI) is found - vendorid/deviceid=%x/%x\n", venderid, deviceid);
		padapter->HardwareType = HARDWARE_TYPE_RTL8190P;
	}
	else if (deviceid == HAL_HW_PCI_8192_DEVICE_ID ||
		deviceid == HAL_HW_PCI_0044_DEVICE_ID ||
		deviceid == HAL_HW_PCI_0047_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8192SE_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8174_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8173_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8172_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8171_DEVICE_ID)
	{
		// 8192e and and 8192se may have the same device ID 8192. However, their Revision
		// ID is different
		// Added for 92DE. We deferentiate it from SVID,SDID.
		if( pdev->subsystem_vendor == 0x10EC && pdev->subsystem_device == 0xE020){
			padapter->HardwareType = HARDWARE_TYPE_RTL8192DE;
			RTW_INFO("Adapter(8192DE) is found - VendorID/DeviceID/RID=%X/%X/%X\n", venderid, deviceid, revisionid);
		}else{
			switch (revisionid) {
				case HAL_HW_PCI_REVISION_ID_8192PCIE:
					RTW_INFO("Adapter(8192 PCI-E) is found - vendorid/deviceid=%x/%x\n", venderid, deviceid);
					padapter->HardwareType = HARDWARE_TYPE_RTL8192E;
					break;
				case HAL_HW_PCI_REVISION_ID_8192SE:
					RTW_INFO("Adapter(8192SE) is found - vendorid/deviceid=%x/%x\n", venderid, deviceid);
					padapter->HardwareType = HARDWARE_TYPE_RTL8192SE;
					break;
				default:
					RTW_INFO("Err: Unknown device - vendorid/deviceid=%x/%x\n", venderid, deviceid);
					padapter->HardwareType = HARDWARE_TYPE_RTL8192SE;
					break;
			}
		}
	}
	else if(deviceid==HAL_HW_PCI_8723E_DEVICE_ID )
	{//RTL8723E may have the same device ID with RTL8192CET
		padapter->HardwareType = HARDWARE_TYPE_RTL8723E;
		RTW_INFO("Adapter(8723 PCI-E) is found - VendorID/DeviceID=%x/%x\n", venderid, deviceid);
	}
	else if (deviceid == HAL_HW_PCI_8192CET_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8192CE_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8191CE_DEVICE_ID ||
		deviceid == HAL_HW_PCI_8188CE_DEVICE_ID) 
	{
		RTW_INFO("Adapter(8192C PCI-E) is found - vendorid/deviceid=%x/%x\n", venderid, deviceid);
		padapter->HardwareType = HARDWARE_TYPE_RTL8192CE;
	}
	else if (deviceid == HAL_HW_PCI_8192DE_DEVICE_ID ||
		deviceid == HAL_HW_PCI_002B_DEVICE_ID ){
		padapter->HardwareType = HARDWARE_TYPE_RTL8192DE;
		RTW_INFO("Adapter(8192DE) is found - VendorID/DeviceID/RID=%X/%X/%X\n", venderid, deviceid, revisionid);
	}
	else
	{
		RTW_INFO("Err: Unknown device - vendorid/deviceid=%x/%x\n", venderid, deviceid);
		//padapter->HardwareType = HAL_DEFAULT_HARDWARE_TYPE;
	}


	padapter->chip_type = NULL_CHIP_TYPE;

	//TODO:
#ifdef CONFIG_RTL8192C
	padapter->chip_type = RTL8188C_8192C;
	padapter->HardwareType = HARDWARE_TYPE_RTL8192CE;
#endif
#ifdef CONFIG_RTL8192D
	pdvobjpriv->InterfaceNumber = revisionid;

	padapter->chip_type = RTL8192D;
	padapter->HardwareType = HARDWARE_TYPE_RTL8192DE;
#endif

}

static void pci_intf_start(_adapter *padapter)
{

	RTW_INFO("+pci_intf_start\n");

	//Enable hw interrupt
	padapter->hal_func.enable_interrupt(padapter);

	RTW_INFO("-pci_intf_start\n");
}

static void pci_intf_stop(_adapter *padapter)
{


	//Disable hw interrupt
	if(padapter->bSurpriseRemoved == _FALSE)
	{
		//device still exists, so driver can do i/o operation
		padapter->hal_func.disable_interrupt(padapter);

		rtw_hal_set_hwreg(padapter, HW_VAR_PCIE_STOP_TX_DMA, 0);

		rtw_hal_irp_reset(padapter);
	}
	else
	{
		// Clear irq_enabled to prevent handle interrupt function.
		padapter->dvobjpriv.irq_enabled = 0;
	}


}


static void rtw_dev_unload(_adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;


	if(padapter->bup == _TRUE)
	{
		RTW_INFO("+rtw_dev_unload\n");

		padapter->bDriverStopped = _TRUE;
		#ifdef CONFIG_XMIT_ACK
		if (padapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
		#endif

		//s3.
		if(padapter->intf_stop)
		{
			padapter->intf_stop(padapter);
		}

		//s4.
		rtw_stop_drv_threads(padapter);


		//s5.
		if(padapter->bSurpriseRemoved == _FALSE)
		{
			RTW_INFO("r871x_dev_unload()->rtl871x_hal_deinit()\n");
			rtw_hal_deinit(padapter);

			padapter->bSurpriseRemoved = _TRUE;
		}

		padapter->bup = _FALSE;

	}


	RTW_INFO("-rtw_dev_unload\n");


}

static void disable_ht_for_spec_devid(const struct pci_device_id *pdid)
{
#ifdef CONFIG_80211N_HT
	u16 vid, pid;
	u32 flags;
	int i;
	int num = sizeof(specific_device_id_tbl)/sizeof(struct specific_device_id);

	for(i=0; i<num; i++)
	{
		vid = specific_device_id_tbl[i].idVendor;
		pid = specific_device_id_tbl[i].idProduct;
		flags = specific_device_id_tbl[i].flags;

		if((pdid->vendor==vid) && (pdid->device==pid) && (flags&SPEC_DEV_ID_DISABLE_HT))
		{
			 rtw_ht_enable = 0;
			 rtw_bw_mode = 0;
			 rtw_ampdu_enable = 0;
		}

	}
#endif
}

#ifdef CONFIG_PM
static int rtw_suspend(struct pci_dev *pdev, pm_message_t state)
{	


	return 0;
}

static int rtw_resume(struct pci_dev *pdev)
{


	
	return 0;
}
#endif

#ifdef RTK_DMP_PLATFORM
#define pci_iounmap(x,y) iounmap(y)
#endif

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/
static int rtw_drv_init(struct pci_dev *pdev, const struct pci_device_id *pdid)
{
	int i, err = -ENODEV;

	uint status;
	_adapter *padapter = NULL;
	struct dvobj_priv *pdvobjpriv;
	struct net_device *pnetdev;
	unsigned long pmem_start, pmem_len, pmem_flags;
	u8	bdma64 = _FALSE;

	//RTW_INFO("+rtw_drv_init\n");

	err = pci_enable_device(pdev);
	if (err) {
		RTW_INFO(KERN_ERR "%s : Cannot enable new PCI device\n", pci_name(pdev));
		return err;
	}

#ifdef CONFIG_64BIT_DMA
	if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) {
		RTW_INFO("RTL819xCE: Using 64bit DMA\n");
		if (pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(64))) {
			RTW_INFO(KERN_ERR "Unable to obtain 64bit DMA for consistent allocations\n");
			err = -ENOMEM;
			pci_disable_device(pdev);
			return err;
		}
		bdma64 = _TRUE;
	} else 
#endif
	{
		if (!pci_set_dma_mask(pdev, DMA_BIT_MASK(32))) {
			if (pci_set_consistent_dma_mask(pdev, DMA_BIT_MASK(32))) {
				RTW_INFO(KERN_ERR "Unable to obtain 32bit DMA for consistent allocations\n");
				err = -ENOMEM;
				pci_disable_device(pdev);
				return err;
			}
		}
	}

	pci_set_master(pdev);

	//step 0.
	disable_ht_for_spec_devid(pdid);


	//step 1. set USB interface data
	// init data
	pnetdev = rtw_init_netdev(NULL);
	if (!pnetdev){
		err = -ENOMEM;
		goto fail1;
	}

	if(bdma64){
		pnetdev->features |= NETIF_F_HIGHDMA;
	}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,5,0)
	SET_NETDEV_DEV(pnetdev, &pdev->dev);
#endif

	padapter = rtw_netdev_priv(pnetdev);
	pdvobjpriv = &padapter->dvobjpriv;
	pdvobjpriv->padapter = padapter;
	pdvobjpriv->ppcidev = pdev;

	if (loadparam(padapter, pnetdev) == _FAIL) {
		goto error;
	}

	// set data
	pci_set_drvdata(pdev, pdvobjpriv);

	err = pci_request_regions(pdev, DRV_NAME);
	if (err) {
		RTW_INFO(KERN_ERR "Can't obtain PCI resources\n");
		goto fail1;
	}
	//MEM map
	pmem_start = pci_resource_start(pdev, 2);
	pmem_len = pci_resource_len(pdev, 2);
	pmem_flags = pci_resource_flags(pdev, 2);

#ifdef RTK_DMP_PLATFORM
	pdvobjpriv->pci_mem_start = (unsigned long)ioremap_nocache( pmem_start, pmem_len);
#else
	pdvobjpriv->pci_mem_start = (unsigned long)pci_iomap(pdev, 2, pmem_len);	// shared mem start
#endif
	if (pdvobjpriv->pci_mem_start == 0) {
		RTW_INFO(KERN_ERR "Can't map PCI mem\n");
		goto fail2;
	}

	RTW_INFO("Memory mapped space start: 0x%08lx len:%08lx flags:%08lx, after map:0x%08lx\n",
		pmem_start, pmem_len, pmem_flags, pdvobjpriv->pci_mem_start);

	// Disable Clk Request */
	pci_write_config_byte(pdev, 0x81, 0);
	// leave D3 mode */
	pci_write_config_byte(pdev, 0x44, 0);
	pci_write_config_byte(pdev, 0x04, 0x06);
	pci_write_config_byte(pdev, 0x04, 0x07);


	//set interface_type to usb
	padapter->interface_type = RTW_PCIE;

	//step 1-1., decide the chip_type via vid/pid
	decide_chip_type_by_pci_device_id(padapter, pdev);

	//step 2.	
	if(padapter->chip_type== RTL8188C_8192C)
	{
#ifdef CONFIG_RTL8192C
		rtl8192ce_set_hal_ops(padapter);
#endif
	}
	else if(padapter->chip_type == RTL8192D)
	{
#ifdef CONFIG_RTL8192D
		rtl8192de_set_hal_ops(padapter);
#endif
	}
	else
	{
		status = _FAIL;
		goto error;
	}

	//step 3.	initialize the dvobj_priv 
	padapter->dvobj_init=&pci_dvobj_init;
	padapter->dvobj_deinit=&pci_dvobj_deinit;
	padapter->intf_start=&pci_intf_start;
	padapter->intf_stop=&pci_intf_stop;

	if (padapter->dvobj_init == NULL){
		goto error;
	}

	status = padapter->dvobj_init(padapter);	
	if (status != _SUCCESS) {
		goto error;
	}

	pnetdev->irq = pdev->irq;

	//step 4. read efuse/eeprom data and get mac_addr
	intf_read_chip_info(padapter);	

	//step 5. 
	status = rtw_init_drv_sw(padapter);
	if(status ==_FAIL){
		goto error;
	}

	status = padapter->hal_func.inirp_init(padapter);
	if(status ==_FAIL){
		goto error;
	}
	rtw_init_netdev_name(pnetdev, padapter->registrypriv.ifname);
	rtw_macaddr_cfg(adapter_mac_addr(padapter), padapter->eeprompriv.mac_addr);
	rtw_init_wifidirect_addrs(padapter, adapter_mac_addr(padapter), adapter_mac_addr(padapter));

	_rtw_memcpy(pnetdev->dev_addr, adapter_mac_addr(padapter), ETH_ALEN);
	RTW_INFO("MAC Address from pnetdev->dev_addr= "MAC_FMT"\n", MAC_ARG(pnetdev->dev_addr));	


	padapter->hal_func.disable_interrupt(padapter);

#if defined(IRQF_SHARED)
	err = request_irq(pdev->irq, &rtw_pci_interrupt, IRQF_SHARED, DRV_NAME, padapter);
#else
	err = request_irq(pdev->irq, &rtw_pci_interrupt, SA_SHIRQ, DRV_NAME, padapter);
#endif
	if (err) {
		RTW_INFO("Error allocating IRQ %d",pdev->irq);
		goto error;
	} else {
		pdvobjpriv->irq_alloc = 1;
		RTW_INFO("Request_irq OK, IRQ %d\n",pdev->irq);
	}

	//step 7.
	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0) {
		goto error;
	}

	//RTW_INFO("-871x_drv - drv_init, success!\n");

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(padapter);
#endif

#ifdef CONFIG_PLATFORM_RTD2880B
	RTW_INFO("wlan link up\n");
	rtd2885_wlan_netlink_sendMsg("linkup", "8712");
#endif

	return 0;

error:

	pci_set_drvdata(pdev, NULL);

	if (pdvobjpriv->irq_alloc) {
		free_irq(pdev->irq, padapter);
		pdvobjpriv->irq_alloc = 0;
	}

	if (pdvobjpriv->pci_mem_start != 0) {
		pci_iounmap(pdev, (void *)pdvobjpriv->pci_mem_start);
	}

	pci_dvobj_deinit(padapter);

	if (pnetdev)
	{
		//unregister_netdev(pnetdev);
		rtw_free_netdev(pnetdev);
	}

fail2:
	pci_release_regions(pdev);

fail1:
	pci_disable_device(pdev);

	RTW_INFO("-871x_pci - drv_init, fail!\n");

	return err;
}

/*
 * dev_remove() - our device is being removed
*/
//rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both
static void rtw_dev_remove(struct pci_dev *pdev)
{
	struct dvobj_priv *pdvobjpriv = pci_get_drvdata(pdev);
	_adapter *padapter = pdvobjpriv->padapter;
	struct net_device *pnetdev = padapter->pnetdev;


	if (unlikely(!padapter)) {
		return;
	}

	RTW_INFO("+rtw_dev_remove\n");

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_unload(padapter);
#endif
	if (padapter->bFWReady == _TRUE)
		LeaveAllPowerSaveMode(padapter);

#ifdef RTK_DMP_PLATFORM    
	padapter->bSurpriseRemoved = _FALSE;	// always trate as device exists
                                                // this will let the driver to disable it's interrupt
#else	
	if(pci_drvpriv.drv_registered == _TRUE)
	{
		//RTW_INFO("r871xu_dev_remove():padapter->bSurpriseRemoved == _TRUE\n");
		padapter->bSurpriseRemoved = _TRUE;
	}
	/*else
	{
		//RTW_INFO("r871xu_dev_remove():module removed\n");
		padapter->hw_init_completed = _FALSE;
	}*/
#endif

	if(pnetdev){
		unregister_netdev(pnetdev); //will call netdev_close()
	}

	rtw_cancel_all_timer(padapter);

	rtw_dev_unload(padapter);

	RTW_INFO("+r871xu_dev_remove, hw_init_completed=%d\n", padapter->hw_init_completed);

	if (pdvobjpriv->irq_alloc) {
		free_irq(pdev->irq, padapter);
		pdvobjpriv->irq_alloc = 0;
	}

	if (pdvobjpriv->pci_mem_start != 0) {
		pci_iounmap(pdev, (void *)pdvobjpriv->pci_mem_start);
		pci_release_regions(pdev);
	}

	pci_disable_device(pdev);
	pci_set_drvdata(pdev, NULL);

	padapter->hal_func.inirp_deinit(padapter);
	//s6.
	if(padapter->dvobj_deinit)
	{
		padapter->dvobj_deinit(padapter);
	}

	
	rtw_free_drv_sw(padapter);

	//after rtw_free_drv_sw(), padapter has beed freed, don't refer to it.

	RTW_INFO("-r871xu_dev_remove, done\n");

#ifdef CONFIG_PLATFORM_RTD2880B
	RTW_INFO("wlan link down\n");
	rtd2885_wlan_netlink_sendMsg("linkdown", "8712");
#endif


	return;

}


static int __init rtw_drv_entry(void)
{
	int ret = 0;

	dump_drv_version(RTW_DBGDUMP);
	pci_drvpriv.drv_registered = _TRUE;
	ret = pci_register_driver(&pci_drvpriv.rtw_pci_drv);


	return ret;
}

static void __exit rtw_drv_halt(void)
{
	RTW_INFO("+rtw_drv_halt\n");
	pci_drvpriv.drv_registered = _FALSE;
	pci_unregister_driver(&pci_drvpriv.rtw_pci_drv);
	RTW_INFO("-rtw_drv_halt\n");

	rtw_mstat_dump(RTW_DBGDUMP);
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);

