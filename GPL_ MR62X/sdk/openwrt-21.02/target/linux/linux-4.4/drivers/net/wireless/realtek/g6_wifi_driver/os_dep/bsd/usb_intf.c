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

#ifndef CONFIG_USB_HCI

#error "CONFIG_USB_HCI shall be on!\n"

#endif

#if defined (PLATFORM_LINUX) && defined (PLATFORM_FREEBSD)

#error "Shall be Linux or FREEBSD, but not both!\n"

#endif

#ifdef PLATFORM_FREEBSD
#include <dev/usb/usb_core.h>
#include <dev/usb/usb_compat_linux.h>
#include <dev/usb/usb_process.h>
#include <dev/usb/usb_device.h>
#include <dev/usb/usb_util.h>
#include <dev/usb/usb_busdma.h>
#include <dev/usb/usb_transfer.h>
#include <dev/usb/usb_hub.h>
#include <dev/usb/usb_request.h>
#include <dev/usb/usb_debug.h>

struct ifnet *rtw_init_netdev(void);
int rtw_init_netdev_name(struct ifnet *pifp);
#endif /* PLATFORM_FREEBSD */


#ifdef CONFIG_80211N_HT
extern int rtw_ht_enable;
extern int rtw_bw_mode;
extern int rtw_ampdu_enable;//for enable tx_ampdu
#endif

static struct usb_interface *pintf;

#ifdef CONFIG_SIGNAL_PID_WHILE_DRV_INIT
int ui_pid = 0;
#endif


//#ifndef PLATFORM_FREEBSD

#ifndef PLATFORM_FREEBSD
extern int pm_netdev_open(struct ifnet *pnetdev,u8 bnormal);
static int rtw_suspend(struct usb_interface *intf, pm_message_t message);
static int rtw_resume(struct usb_interface *intf);
#endif /* PLATFORM_FREEBSD */

static int rtw_drv_init(struct usb_interface *pusb_intf,const struct usb_device_id *pdid);
static void rtw_dev_remove(struct usb_interface *pusb_intf);

#define USB_VENDER_ID_REALTEK		0x0BDA

//DID_USB_V71_20110628
static struct usb_device_id rtw_usb_id_tbl[] ={
#ifdef CONFIG_RTL8192C
	/*=== Realtek demoboard ===*/		
	{USB_DEVICE(0x0BDA, 0x8191)},//Default ID

	/****** 8188CUS ********/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8176)},//8188cu 1*1 dongole
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8170)},//8188CE-VAU USB minCard
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817E)},//8188CE-VAU USB minCard
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817A)},//8188cu Slim Solo
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817B)},//8188cu Slim Combo
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817D)},//8188RU High-power USB Dongle
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8754)},//8188 Combo for BC4
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817F)},//8188RU

	/****** 8192CUS ********/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8177)},//8191cu 1*2
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8178)},//8192cu 2*2
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x817C)},//8192CE-VAU USB minCard
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8186)},//8192CE-VAU USB minCard

	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8191)},//8192CU 2*2
	{USB_DEVICE(0x1058, 0x0631)},//Alpha, 8192CU

	/*=== Customer ID ===*/	
	/****** 8188CUS Dongle ********/
	{USB_DEVICE(0x2019, 0xED17)},//PCI - Edimax
	{USB_DEVICE(0x0DF6, 0x0052)},//Sitecom - Edimax
	{USB_DEVICE(0x7392, 0x7811)},//Edimax - Edimax
	{USB_DEVICE(0x07B8, 0x8189)},//Abocom - Abocom
	{USB_DEVICE(0x0EB0, 0x9071)},//NO Brand - Etop
	{USB_DEVICE(0x06F8, 0xE033)},//Hercules - Edimax
	{USB_DEVICE(0x103C, 0x1629)},//HP - Lite-On ,8188CUS Slim Combo
	{USB_DEVICE(0x2001, 0x3308)},//D-Link - Alpha
	{USB_DEVICE(0x050D, 0x1102)},//Belkin - Edimax
	{USB_DEVICE(0x2019, 0xAB2A)},//Planex - Abocom
	{USB_DEVICE(0x20F4, 0x648B)},//TRENDnet - Cameo
	{USB_DEVICE(0x4855, 0x0090)},// - Feixun
	{USB_DEVICE(0x13D3, 0x3357)},// - AzureWave
	{USB_DEVICE(0x0DF6, 0x005C)},//Sitecom - Edimax
	{USB_DEVICE(0x0BDA, 0x5088)},//Thinkware - CC&C
	{USB_DEVICE(0x4856, 0x0091)},//NetweeN - Feixun
	{USB_DEVICE(0x9846, 0x9041)},//Netgear - Cameo
	{USB_DEVICE(0x0846, 0x9041)},//Netgear - Cameo
	{USB_DEVICE(0x2019, 0x4902)},//Planex - Etop
	{USB_DEVICE(0x2019, 0xAB2E)},//SW-WF02-AD15 -Abocom
	
	/****** 8188CE-VAU ********/
	{USB_DEVICE(0x13D3, 0x3359)},// - Azwave
	{USB_DEVICE(0x13D3, 0x3358)},// - Azwave

	/****** 8188CUS Slim Solo********/
	{USB_DEVICE(0x04F2, 0xAFF7)},//XAVI - XAVI
	{USB_DEVICE(0x04F2, 0xAFF9)},//XAVI - XAVI
	{USB_DEVICE(0x04F2, 0xAFFA)},//XAVI - XAVI

	/****** 8188CUS Slim Combo ********/
	{USB_DEVICE(0x04F2, 0xAFF8)},//XAVI - XAVI
	{USB_DEVICE(0x04F2, 0xAFFB)},//XAVI - XAVI
	{USB_DEVICE(0x04F2, 0xAFFC)},//XAVI - XAVI
	{USB_DEVICE(0x2019, 0x1201)},//Planex - Vencer
	
	/****** 8192CUS Dongle ********/
	{USB_DEVICE(0x2001, 0x3307)},//D-Link - Cameo
	{USB_DEVICE(0x2001, 0x330A)},//D-Link - Alpha
	{USB_DEVICE(0x2001, 0x3309)},//D-Link - Alpha
	{USB_DEVICE(0x0586, 0x341F)},//Zyxel - Abocom
	{USB_DEVICE(0x7392, 0x7822)},//Edimax - Edimax
	{USB_DEVICE(0x2019, 0xAB2B)},//Planex - Abocom
	{USB_DEVICE(0x07B8, 0x8178)},//Abocom - Abocom
	{USB_DEVICE(0x07AA, 0x0056)},//ATKK - Gemtek
	{USB_DEVICE(0x4855, 0x0091)},// - Feixun
	{USB_DEVICE(0x050D, 0x2102)},//Belkin - Sercomm
	{USB_DEVICE(0x050D, 0x2102)},//Belkin - Sercomm
	{USB_DEVICE(0x050D, 0x2103)},//Belkin - Edimax
	{USB_DEVICE(0x20F4, 0x624D)},//TRENDnet
	{USB_DEVICE(0x0DF6, 0x0061)},//Sitecom - Edimax
	{USB_DEVICE(0x0B05, 0x17AB)},//ASUS - Edimax
#endif
#ifdef CONFIG_RTL8192D
	/*=== Realtek demoboard ===*/
	/****** 8192DU ********/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8193)},//8192DU-VC
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8194)},//8192DU-VS
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8111)},//Realtek 5G dongle for WiFi Display

	/*=== Customer ID ===*/
	/****** 8192DU-VC ********/
	{USB_DEVICE(0x2019, 0xAB2C)},//PCI - Abocm
	{USB_DEVICE(0x2019, 0x4903)},//PCI - ETOP
	{USB_DEVICE(0x2019, 0x4904)},//PCI - ETOP
	{USB_DEVICE(0x07B8, 0x8193)},//Abocom - Abocom

	/****** 8192DU-VS ********/
	{USB_DEVICE(0x20F4, 0x664B)},//TRENDnet

	/****** 8192DU-WiFi Display Dongle ********/
	{USB_DEVICE(0x2019, 0xAB2D)},//Planex - Abocom ,5G dongle for WiFi Display
#endif
	{}	/* Terminating entry */
};

int const rtw_usb_id_len = sizeof(rtw_usb_id_tbl) / sizeof(struct usb_device_id);

static struct specific_device_id specific_device_id_tbl[] = {
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x8177, .flags=SPEC_DEV_ID_DISABLE_HT},//8188cu 1*1 dongole, (b/g mode only)
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x817E, .flags=SPEC_DEV_ID_DISABLE_HT},//8188CE-VAU USB minCard (b/g mode only)
	{.idVendor=0x0b05, .idProduct=0x1791, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3311, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3359, .flags=SPEC_DEV_ID_DISABLE_HT},//Russian customer -Azwave (8188CE-VAU  g mode)	
#ifdef RTK_DMP_PLATFORM
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x8111, .flags=SPEC_DEV_ID_ASSIGN_IFNAME}, // Realtek 5G dongle for WiFi Display
	{.idVendor=0x2019, .idProduct=0xAB2D, .flags=SPEC_DEV_ID_ASSIGN_IFNAME}, // PCI-Abocom 5G dongle for WiFi Display
#endif /* RTK_DMP_PLATFORM */
	{}
};

typedef struct _driver_priv{

	struct usb_driver rtw_usb_drv;
	int drv_registered;

}drv_priv, *pdrv_priv;


static drv_priv drvpriv = {
	.rtw_usb_drv.name = (char*)DRV_NAME,
	.rtw_usb_drv.probe = rtw_drv_init,
	.rtw_usb_drv.disconnect = rtw_dev_remove,
	.rtw_usb_drv.id_table = rtw_usb_id_tbl,
#ifndef PLATFORM_FREEBSD
	.rtw_usb_drv.suspend =  rtw_suspend,
	.rtw_usb_drv.resume = rtw_resume,
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22))
  	.rtw_usb_drv.reset_resume   = rtw_resume,
#endif
#endif /* PLATFORM_FREEBSD */
};

//MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);


static inline int RT_usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

static inline int RT_usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

static inline int RT_usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
}

static inline int RT_usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
 	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
}

static inline int RT_usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_out(epd));
}

static inline int RT_usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_int(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

#ifdef CONFIG_USB_VENDOR_REQ_PREALLOC
u8 rtw_init_intf_priv(_adapter * padapter)
{
	u8 rst = _SUCCESS; 
	
	_rtw_mutex_init(&padapter->dvobjpriv.usb_vendor_req_mutex);

	padapter->dvobjpriv.usb_alloc_vendor_req_buf = rtw_zmalloc(MAX_USB_IO_CTL_SIZE);

	if (padapter->dvobjpriv.usb_alloc_vendor_req_buf   == NULL){
		padapter->dvobjpriv.usb_alloc_vendor_req_buf  =NULL;
		printk("alloc usb_vendor_req_buf failed... /n");
		rst = _FAIL;
		goto exit;
	}
	padapter->dvobjpriv.usb_vendor_req_buf  = 
		(u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(padapter->dvobjpriv.usb_alloc_vendor_req_buf ), ALIGNMENT_UNIT);
exit:
	return rst;
	
}

u8 rtw_deinit_intf_priv(_adapter * padapter)
{
	u8 rst = _SUCCESS; 

	if(padapter->dvobjpriv.usb_vendor_req_buf)
	{
		rtw_mfree(padapter->dvobjpriv.usb_alloc_vendor_req_buf,MAX_USB_IO_CTL_SIZE);
	}
	
exit:
	return rst;
	
}
#endif
static u32 usb_dvobj_init(_adapter *padapter)
{
	int	i;
	//u8	val8;
	int	status = _SUCCESS;
	struct usb_device_descriptor 	*pdev_desc;
#ifndef PLATFORM_FREEBSD
	struct usb_host_config			*phost_conf;
	struct usb_config_descriptor		*pconf_desc;
#endif
	struct usb_host_interface		*phost_iface;
	struct usb_interface_descriptor	*piface_desc;
	struct usb_host_endpoint		*phost_endp;
	struct usb_endpoint_descriptor	*pendp_desc;
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;


	pdvobjpriv->padapter = padapter;

	pdvobjpriv->RtNumInPipes = 0;
	pdvobjpriv->RtNumOutPipes = 0;

	//padapter->EepromAddressSize = 6;
	//pdvobjpriv->nr_endpoint = 6;

	pdev_desc = &pusbd->descriptor;

#if 0
	RTW_INFO("\n8712_usb_device_descriptor:\n");
	RTW_INFO("bLength=%x\n", pdev_desc->bLength);
	RTW_INFO("bDescriptorType=%x\n", pdev_desc->bDescriptorType);
	RTW_INFO("bcdUSB=%x\n", pdev_desc->bcdUSB);
	RTW_INFO("bDeviceClass=%x\n", pdev_desc->bDeviceClass);
	RTW_INFO("bDeviceSubClass=%x\n", pdev_desc->bDeviceSubClass);
	RTW_INFO("bDeviceProtocol=%x\n", pdev_desc->bDeviceProtocol);
	RTW_INFO("bMaxPacketSize0=%x\n", pdev_desc->bMaxPacketSize0);
	RTW_INFO("idVendor=%x\n", pdev_desc->idVendor);
	RTW_INFO("idProduct=%x\n", pdev_desc->idProduct);
	RTW_INFO("bcdDevice=%x\n", pdev_desc->bcdDevice);
	RTW_INFO("iManufacturer=%x\n", pdev_desc->iManufacturer);
	RTW_INFO("iProduct=%x\n", pdev_desc->iProduct);
	RTW_INFO("iSerialNumber=%x\n", pdev_desc->iSerialNumber);
	RTW_INFO("bNumConfigurations=%x\n", pdev_desc->bNumConfigurations);
#endif

#ifndef PLATFORM_FREEBSD
	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;
#endif

#if 0
	RTW_INFO("\n8712_usb_configuration_descriptor:\n");
	RTW_INFO("bLength=%x\n", pconf_desc->bLength);
	RTW_INFO("bDescriptorType=%x\n", pconf_desc->bDescriptorType);
	RTW_INFO("wTotalLength=%x\n", pconf_desc->wTotalLength);
	RTW_INFO("bNumInterfaces=%x\n", pconf_desc->bNumInterfaces);
	RTW_INFO("bConfigurationValue=%x\n", pconf_desc->bConfigurationValue);
	RTW_INFO("iConfiguration=%x\n", pconf_desc->iConfiguration);
	RTW_INFO("bmAttributes=%x\n", pconf_desc->bmAttributes);
	RTW_INFO("bMaxPower=%x\n", pconf_desc->bMaxPower);
#endif

	//RTW_INFO("\n/****** num of altsetting = (%d) ******/\n", pintf->num_altsetting);

	phost_iface = &pintf->altsetting[0];
	piface_desc = &phost_iface->desc;

#if 0
	RTW_INFO("\n8712_usb_interface_descriptor:\n");
	RTW_INFO("bLength=%x\n", piface_desc->bLength);
	RTW_INFO("bDescriptorType=%x\n", piface_desc->bDescriptorType);
	RTW_INFO("bInterfaceNumber=%x\n", piface_desc->bInterfaceNumber);
	RTW_INFO("bAlternateSetting=%x\n", piface_desc->bAlternateSetting);
	RTW_INFO("bNumEndpoints=%x\n", piface_desc->bNumEndpoints);
	RTW_INFO("bInterfaceClass=%x\n", piface_desc->bInterfaceClass);
	RTW_INFO("bInterfaceSubClass=%x\n", piface_desc->bInterfaceSubClass);
	RTW_INFO("bInterfaceProtocol=%x\n", piface_desc->bInterfaceProtocol);
	RTW_INFO("iInterface=%x\n", piface_desc->iInterface);
#endif

#ifdef PLATFORM_FREEBSD
	pdvobjpriv->NumInterfaces = 1;
	pdvobjpriv->InterfaceNumber = 0;
	pdvobjpriv->nr_endpoint = 2;
#else // PLATFORM_FREEBSD
	pdvobjpriv->NumInterfaces = pconf_desc->bNumInterfaces;
	pdvobjpriv->InterfaceNumber = piface_desc->bInterfaceNumber;
	pdvobjpriv->nr_endpoint = piface_desc->bNumEndpoints;
#endif // PLATFORM_FREEBSD

	//RTW_INFO("\ndump usb_endpoint_descriptor:\n");

	for (i = 0; i < pdvobjpriv->nr_endpoint; i++)
	{
		phost_endp = phost_iface->endpoint + i;
		if (phost_endp)
		{
			pendp_desc = &phost_endp->desc;

			RTW_INFO("\nusb_endpoint_descriptor(%d):\n", i);
			RTW_INFO("bLength=%x\n",pendp_desc->bLength);
			RTW_INFO("bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			RTW_INFO("bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			//RTW_INFO("bmAttributes=%x\n",pendp_desc->bmAttributes);
			//RTW_INFO("wMaxPacketSize=%x\n",pendp_desc->wMaxPacketSize);
			//RTW_INFO("wMaxPacketSize=%x\n",le16_to_cpu(pendp_desc->wMaxPacketSize));
			RTW_INFO("bInterval=%x\n",pendp_desc->bInterval);
			//RTW_INFO("bRefresh=%x\n",pendp_desc->bRefresh);
			//RTW_INFO("bSynchAddress=%x\n",pendp_desc->bSynchAddress);

			if (RT_usb_endpoint_is_bulk_in(pendp_desc))
			{
				RTW_INFO("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtNumInPipes++;
			}
			else if (RT_usb_endpoint_is_int_in(pendp_desc))
			{
				RTW_INFO("RT_usb_endpoint_is_int_in = %x, Interval = %x\n", RT_usb_endpoint_num(pendp_desc),pendp_desc->bInterval);
				pdvobjpriv->RtNumInPipes++;
			}
			else if (RT_usb_endpoint_is_bulk_out(pendp_desc))
			{
				RTW_INFO("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtNumOutPipes++;
			}
			pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}
	
	RTW_INFO("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

	if (pusbd->speed == USB_SPEED_HIGH)
	{
		pdvobjpriv->usb_speed = RTW_USB_SPEED_2;
		RTW_INFO("USB_SPEED_HIGH\n");
	}
	else
	{
		pdvobjpriv->usb_speed = RTW_USB_SPEED_1_1;
		RTW_INFO("NON USB_SPEED_HIGH\n");
	}

	//.2
	if ((init_io_priv(padapter)) == _FAIL)
	{
		status = _FAIL;
	}
#ifdef CONFIG_USB_VENDOR_REQ_PREALLOC
	if((rtw_init_intf_priv(padapter) )== _FAIL)
	{
		status = _FAIL;
	}
#endif
	//.3 misc
	_rtw_init_sema(&(padapter->dvobjpriv.usb_suspend_sema), 0);	

	intf_read_chip_version(padapter);

	//.4 usb endpoint mapping
	intf_chip_configure(padapter);

	ATOMIC_SET(&pdvobjpriv->continual_io_error, 0);
	

	return status;
}

static void usb_dvobj_deinit(_adapter * padapter){

	//struct dvobj_priv *pdvobjpriv=&padapter->dvobjpriv;

#ifdef CONFIG_USB_VENDOR_REQ_PREALLOC
	rtw_deinit_intf_priv(padapter);
#endif
}

static void decide_chip_type_by_usb_device_id(_adapter *padapter, const struct usb_device_id *pdid)
{
	//u32	i;
	//u16	vid, pid;

	padapter->chip_type = NULL_CHIP_TYPE;

	//vid = pdid->idVendor;
	//pid = pdid->idProduct;

	//TODO: dynamic judge 92c or 92d according to usb vid and pid.
#ifdef CONFIG_RTL8192C
	padapter->chip_type = RTL8188C_8192C;
	padapter->HardwareType = HARDWARE_TYPE_RTL8192CU;
	RTW_INFO("CHIP TYPE: RTL8188C_8192C\n");
#endif

#ifdef CONFIG_RTL8192D
	padapter->chip_type = RTL8192D;
	padapter->HardwareType = HARDWARE_TYPE_RTL8192DU;
	RTW_INFO("CHIP TYPE: RTL8192D\n");
#endif

}

static void usb_intf_start(_adapter *padapter)
{


	if (padapter->hal_func.inirp_init != NULL)
		padapter->hal_func.inirp_init(padapter);


}

static void usb_intf_stop(_adapter *padapter)
{


	//disabel_hw_interrupt
	if(padapter->bSurpriseRemoved == _FALSE)
	{
		//device still exists, so driver can do i/o operation
		//TODO:
	}

	//cancel in irp
	if (padapter->hal_func.inirp_deinit != NULL)
		padapter->hal_func.inirp_deinit(padapter);

	//cancel out irp
	write_port_cancel(padapter);

	//todo:cancel other irps


}

static void rtw_dev_unload(_adapter *padapter)
{
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(padapter);
	//struct ifnet *pnetdev= (struct ifnet*)padapter->pifp;
	//u8 val8;

	if(padapter->bup == _TRUE)
	{
		RTW_INFO("===> rtw_dev_unload\n");

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
		if(!pwrctl->bInternalAutoSuspend )
		rtw_stop_drv_threads(padapter);


		printf("%s-%d: padapter->bSurpriseRemoved = %d\n", __FUNCTION__, __LINE__, padapter->bSurpriseRemoved);
		//s5.
		if(padapter->bSurpriseRemoved == _FALSE)
		{
			//RTW_INFO("r871x_dev_unload()->rtl871x_hal_deinit()\n");
			#ifdef CONFIG_WOWLAN
			if(pwrctl->bSupportRemoteWakeup==_TRUE){
				RTW_INFO("%s bSupportRemoteWakeup==_TRUE  do not run rtw_hal_deinit()\n",__FUNCTION__);
			}
			else
			#endif
			{
				rtw_hal_deinit(padapter);
			}
			padapter->bSurpriseRemoved = _TRUE;
		}

		padapter->bup = _FALSE;

	}


	RTW_INFO("<=== rtw_dev_unload\n");


}

static void process_spec_devid(const struct usb_device_id *pdid)
{
	u16 vid, pid;
	u32 flags;
	int i;
	int num = sizeof(specific_device_id_tbl)/sizeof(struct specific_device_id);

	for(i=0; i<num; i++)
	{
		vid = specific_device_id_tbl[i].idVendor;
		pid = specific_device_id_tbl[i].idProduct;
		flags = specific_device_id_tbl[i].flags;

#ifdef CONFIG_80211N_HT
		if((pdid->idVendor==vid) && (pdid->idProduct==pid) && (flags&SPEC_DEV_ID_DISABLE_HT))
		{
			 rtw_ht_enable = 0;
			 rtw_bw_mode = 0;
			 rtw_ampdu_enable = 0;
		}
#endif

#ifdef RTK_DMP_PLATFORM
		// Change the ifname to wlan10 when PC side WFD dongle plugin on DMP platform.
		// It is used to distinguish between normal and PC-side wifi dongle/module.
		if((pdid->idVendor==vid) && (pdid->idProduct==pid) && (flags&SPEC_DEV_ID_ASSIGN_IFNAME))
		{
			extern char* ifname;
			strncpy(ifname, "wlan10", 6); 
			//RTW_INFO("%s()-%d: ifname=%s, vid=%04X, pid=%04X\n", __FUNCTION__, __LINE__, ifname, vid, pid);
		}
#endif /* RTK_DMP_PLATFORM */

	}
}

#ifdef SUPPORT_HW_RFOFF_DETECTED
int rtw_hw_suspend(_adapter *padapter )
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct usb_interface *pusb_intf = padapter->dvobjpriv.pusbintf;	
	struct ifnet *pnetdev = padapter->pifp;
	

	if((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		RTW_INFO("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);		
		goto error_exit;
	}
	
	if(padapter)//system suspend
	{		
		LeaveAllPowerSaveMode(padapter);
		
		RTW_INFO("==> rtw_hw_suspend\n");	
		_enter_pwrlock(&pwrpriv->lock);
		pwrpriv->bips_processing = _TRUE;
		//padapter->net_closed = _TRUE;
		//s1.
		if(pnetdev)
		{
			netif_carrier_off(pnetdev);
			rtw_netif_stop_queue(pnetdev);
		}

		//s2.
		rtw_disassoc_cmd(padapter, 500, RTW_CMDF_DIRECTLY);
		
		//s2-2.  indicate disconnect to os
		//rtw_indicate_disconnect(padapter);
		{
			struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;		

			if(check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
			{
				_clr_fwstate_(pmlmepriv, WIFI_ASOC_STATE);
				#ifdef CONFIG_RTW_SW_LED
				rtw_led_control(padapter, LED_CTL_NO_LINK);
				#endif
				rtw_os_indicate_disconnect(padapter, 0, _FALSE);
				
#ifdef CONFIG_LPS
				//donnot enqueue cmd
				rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_DISCONNECT, RTW_CMDF_DIRECTLY);
#endif
			}

		}
		//s2-3.
		rtw_free_assoc_resources(padapter);

		//s2-4.
		rtw_free_network_queue(padapter,_TRUE);

		rtw_ips_dev_unload(padapter);			

		pwrpriv->current_rfpwrstate = rf_off;
		pwrpriv->bips_processing = _FALSE;		

		_exit_pwrlock(&pwrpriv->lock);
	}
	else
		goto error_exit;
	
	return 0;
	
error_exit:
	RTW_INFO("%s, failed \n",__FUNCTION__);
	return (-1);

}

int rtw_hw_resume(_adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct usb_interface *pusb_intf = padapter->dvobjpriv.pusbintf;
	struct ifnet *pnetdev = padapter->pifp;


	if(padapter)//system resume
	{	
		RTW_INFO("==> rtw_hw_resume\n");
		_enter_pwrlock(&pwrpriv->lock);
		pwrpriv->bips_processing = _TRUE;
		rtw_reset_drv_sw(padapter);
	
		if(pm_netdev_open(pnetdev,_FALSE) != 0)
		{
			_exit_pwrlock(&pwrpriv->lock);
			goto error_exit;
		}

		netif_device_attach(pnetdev);	
		netif_carrier_on(pnetdev);

		rtw_netif_wake_queue(pnetdev);

		pwrpriv->bkeepfwalive = _FALSE;
		pwrpriv->brfoffbyhw = _FALSE;
		
		pwrpriv->current_rfpwrstate = rf_on;
		pwrpriv->bips_processing = _FALSE;	
	
		_exit_pwrlock(&pwrpriv->lock);
	}
	else
	{
		goto error_exit;	
	}

	
	return 0;
error_exit:
	RTW_INFO("%s, Open net dev failed \n",__FUNCTION__);
	return (-1);
}
#endif

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/
static int rtw_drv_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
//	int i;

	uint status;
	_adapter *padapter = NULL;
	struct dvobj_priv *pdvobjpriv;
	//struct net_device *pnetdev;
	struct ifnet *pnetdev;

	RTW_INFO("+rtw_drv_init\n");

	//2009.8.13, by Thomas
	// In this probe function, O.S. will provide the usb interface pointer to driver.
	// We have to increase the reference count of the usb device structure by using the usb_get_dev function.
	usb_get_dev(interface_to_usbdev(pusb_intf));

	pintf = pusb_intf;	

	//step 0.
	process_spec_devid(pdid);

	//step 1. set USB interface data
	// init data
	pnetdev = rtw_init_netdev();
	if (!pnetdev) 
		goto error;
	
#ifndef PLATFORM_FREEBSD	
	SET_NETDEV_DEV(pnetdev, &pusb_intf->dev);
#endif

	padapter = rtw_netdev_priv(pnetdev);
	padapter->bDriverStopped=_TRUE;
	pdvobjpriv = &padapter->dvobjpriv;
	pdvobjpriv->padapter = padapter;
	pdvobjpriv->pusbintf = pusb_intf ;
	pdvobjpriv->pusbdev = interface_to_usbdev(pusb_intf);

	if (loadparam(padapter) == _FAIL) {
		goto error;
	}

	// set data
        rtw_usb_set_intfdata(pusb_intf, pnetdev);

	//set interface_type to usb
	padapter->interface_type = RTW_USB;

	//step 1-1., decide the chip_type via vid/pid
	decide_chip_type_by_usb_device_id(padapter, pdid);

	//step 2.	
	if(padapter->chip_type == RTL8188C_8192C)
	{
#ifdef CONFIG_RTL8192C
		rtl8192cu_set_hal_ops(padapter);
#endif
	}
	else if(padapter->chip_type == RTL8192D)
	{
#ifdef CONFIG_RTL8192D
		rtl8192du_set_hal_ops(padapter);
#endif
	}
	else
	{
		RTW_INFO("Detect NULL_CHIP_TYPE\n");
		status = _FAIL;
		goto error;
	}

	//step 3.	initialize the dvobj_priv 
	padapter->dvobj_init=&usb_dvobj_init;
	padapter->dvobj_deinit=&usb_dvobj_deinit;
	padapter->intf_start=&usb_intf_start;
	padapter->intf_stop=&usb_intf_stop;

	//step 3.
	//initialize the dvobj_priv ,include Chip version		
	if (padapter->dvobj_init == NULL){
		goto error;
	}

	status = padapter->dvobj_init(padapter);	
	if (status != _SUCCESS) {
		goto error;
	}

	//step 4. read efuse/eeprom data and get mac_addr
	intf_read_chip_info(padapter);	

	//step 5.
	status = rtw_init_drv_sw(padapter);
	if(status ==_FAIL){
		goto error;
	}

#ifdef CONFIG_PM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18))
	if(adapter_to_pwrctl(padapter)->bSupportRemoteWakeup)
	{
		pdvobjpriv->pusbdev->do_remote_wakeup=1;
		pusb_intf->needs_remote_wakeup = 1;		
		device_init_wakeup(&pusb_intf->dev, 1);
		RTW_INFO("pwrctrlpriv.bSupportRemoteWakeup~~~~~~\n");
		RTW_INFO("pwrctrlpriv.bSupportRemoteWakeup~~~[%d]~~~\n",device_may_wakeup(&pusb_intf->dev));
	}
#endif
#endif

	// alloc dev name after read efuse.
	rtw_init_netdev_name(pnetdev);
       
	rtw_macaddr_cfg(adapter_mac_addr(padapter), padapter->eeprompriv.mac_addr);
	rtw_init_wifidirect_addrs(padapter, adapter_mac_addr(padapter), adapter_mac_addr(padapter));
       
#ifndef PLATFORM_FREEBSD
	_rtw_memcpy(pnetdev->dev_addr, adapter_mac_addr(padapter), ETH_ALEN);
	RTW_INFO("MAC Address from pnetdev->dev_addr= " MAC_FMT "\n", MAC_ARG(pnetdev->dev_addr));	


	//step 6.
	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0) {
		goto error;
	}
#else
       /* Tell FreeBSD network stack we exist */
	ether_ifattach(pnetdev, adapter_mac_addr(padapter));
#endif /* PLATFORM_FREEBSD */

	//RTW_INFO("-871x_drv - drv_init, success!\n");

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(padapter);
#endif

#ifdef CONFIG_PLATFORM_RTD2880B
	RTW_INFO("wlan link up\n");
	rtd2885_wlan_netlink_sendMsg("linkup", "8712");
#endif


#ifdef CONFIG_SIGNAL_PID_WHILE_DRV_INIT
	if(ui_pid!=0) {
#ifdef PLATFORM_LINUX
		RTW_INFO("ui_pid:%d\n",ui_pid);
		#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27))
		kill_pid(find_vpid(ui_pid), SIGUSR2, 1);
		#endif
		#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,26))
		kill_proc(ui_pid, SIGUSR2, 1);
		#endif
	}	
#endif
#endif

	RTW_INFO("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		,(int)padapter->bDriverStopped
		,(int)padapter->bSurpriseRemoved
		,padapter->bup
		,padapter->hw_init_completed
	);

	RTW_INFO("-rtw_drv_init\n");
	return 0;

error:

	usb_put_dev(interface_to_usbdev(pusb_intf));//decrease the reference count of the usb device structure if driver fail on initialzation

        rtw_usb_set_intfdata(pusb_intf, NULL);

	usb_dvobj_deinit(padapter);
	
	if (pnetdev)
	{
		//unregister_netdev(pnetdev);
		rtw_free_netdev(pnetdev);
	}

	//RTW_INFO("-871x_usb - drv_init, fail!\n");

	return -ENODEV;
}

/*
 * dev_remove() - our device is being removed
*/
//rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both
static void rtw_dev_remove(struct usb_interface *pusb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	_adapter *padapter = dvobj->padapter;
	struct ifnet *pnetdev = padapter->pifp
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	u8	bResetDevice = _FALSE;


	printf("%s()-%d: interface_to_usbdev(pusb_intf)=%p, pusb_intf->bsd_iface_index=%d\n", __FUNCTION__, __LINE__, interface_to_usbdev(pusb_intf), pusb_intf->bsd_iface_index);
        rtw_usb_set_intfdata(pusb_intf, NULL);

	if(padapter)
	{
		RTW_INFO("+rtw_dev_remove\n");

#ifdef CONFIG_HOSTAPD_MLME
		hostapd_mode_unload(padapter);
#endif
		if (padapter->bFWReady == _TRUE)
			LeaveAllPowerSaveMode(padapter);

		if(check_fwstate(pmlmepriv, WIFI_ASOC_STATE))
			rtw_disassoc_cmd(padapter, 0, RTW_CMDF_DIRECTLY);

		if(drvpriv.drv_registered == _TRUE)
		{
			RTW_INFO("r871xu_dev_remove():padapter->bSurpriseRemoved == _TRUE\n");
			padapter->bSurpriseRemoved = _TRUE;
		}
		/*else
		{
			//RTW_INFO("r871xu_dev_remove():module removed\n");
			padapter->hw_init_completed = _FALSE;
		}*/

		if(padapter->DriverState != DRIVER_DISAPPEAR)
		{
			if(pnetdev) {
				printf("%s()-%d: padapter->DriverState=%d\n", __FUNCTION__, __LINE__, padapter->DriverState); 
				taskqueue_drain(taskqueue_fast, &padapter->xmitpriv.xmit_tasklet);
				taskqueue_drain(taskqueue_fast, &padapter->recvpriv.recv_tasklet);
#ifdef CONFIG_RX_INDICATE_QUEUE
				taskqueue_drain(taskqueue_thread, &padapter->recvpriv.rx_indicate_tasklet);
#endif	// CONFIG_RX_INDICATE_QUEUE
				ether_ifdetach(pnetdev);
			}
		}

		rtw_cancel_all_timer(padapter);

		rtw_dev_unload(padapter);

		RTW_INFO("+r871xu_dev_remove, hw_init_completed=%d\n", padapter->hw_init_completed);

		//Modify condition for 92DU DMDP 2010.11.18, by Thomas
		//move code to here, avoid access null pointer. 2011.05.25.
		if((dvobj->NumInterfaces != 2) || (dvobj->InterfaceNumber == 1))
			bResetDevice = _TRUE;

		//s6.
		if(padapter->dvobj_deinit)
		{
			padapter->dvobj_deinit(padapter);
		}


		//after rtw_free_drv_sw(), padapter has beed freed, don't refer to it.
		rtw_free_drv_sw(padapter);	
		
	}

	usb_put_dev(interface_to_usbdev(pusb_intf));//decrease the reference count of the usb device structure when disconnect

	//If we didn't unplug usb dongle and remove/insert modlue, driver fails on sitesurvey for the first time when device is up . 
	//Reset usb port for sitesurvey fail issue. 2009.8.13, by Thomas
	if(_TRUE == bResetDevice)
	{
#ifndef PLATFORM_FREEBSD
		if(interface_to_usbdev(pusb_intf)->state != USB_STATE_NOTATTACHED)
		{
			RTW_INFO("usb attached..., try to reset usb device\n");
			usb_reset_device(interface_to_usbdev(pusb_intf));
		}
#endif /* PLATFORM_FREEBSD */
	}
	
	RTW_INFO("-r871xu_dev_remove, done\n");

#ifdef CONFIG_PLATFORM_RTD2880B
	RTW_INFO("wlan link down\n");
	rtd2885_wlan_netlink_sendMsg("linkdown", "8712");
#endif


	return;

}

#ifndef PLATFORM_FREEBSD
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)) 
extern int console_suspend_enabled;
#endif
#endif /* PLATFORM_FREEBSD */

static int __init rtw_drv_entry(void)
{
#ifdef CONFIG_PLATFORM_RTK_DMP
	u32 tmp;
	tmp=readl((volatile unsigned int*)0xb801a608);
	tmp &= 0xffffff00;
	tmp |= 0x55;
	writel(tmp,(volatile unsigned int*)0xb801a608);//write dummy register for 1055
#endif



	dump_drv_version(RTW_DBGDUMP);
#ifndef PLATFORM_FREEBSD
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24)) 
	console_suspend_enabled=0;
#endif	

	drvpriv.drv_registered = _TRUE;
#endif /* PLATFORM_FREEBSD */

	return usb_register(&drvpriv.rtw_usb_drv);
}

static void __exit rtw_drv_halt(void)
{
	RTW_INFO("+rtw_drv_halt\n");
	drvpriv.drv_registered = _FALSE;
	usb_deregister(&drvpriv.rtw_usb_drv);
	RTW_INFO("-rtw_drv_halt\n");

	rtw_mstat_dump(RTW_DBGDUMP);
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);


/*
init (driver module)-> r8712u_drv_entry
probe (sd device)-> r871xu_drv_init(dev_init)
open (net_device) ->netdev_open
close (net_device) ->netdev_close
remove (sd device) ->r871xu_dev_remove
exit (driver module)-> r8712u_drv_halt
*/


/*
r8711s_drv_entry()
r8711u_drv_entry()
r8712s_drv_entry()
r8712u_drv_entry()
*/

//#endif


