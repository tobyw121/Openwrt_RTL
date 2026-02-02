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

#include <linux/jiffies.h>

static int radios = 2;
module_param(radios, int, 0444);
MODULE_PARM_DESC(radios, "Number of simulated radios");

static int medium = 2;
module_param(medium, int, 0444);
MODULE_PARM_DESC(medium, "Medium, 1: local, 2: sock_udp, 3: loopback");

static unsigned int udp = 0xffffffff; 
module_param(udp, uint, 0444);
MODULE_PARM_DESC(udp, "4294967295=0xffffffff=255.255.255.255");


static u8 rtw_hwsim_oui[] = {0x00, 0xE0, 0x4c};

static struct class *rtw_hwsim_class;

static struct platform_driver rtw_hwsim_driver = {
	.driver = {
		.name = "rtw_hwsim",
	},
};

struct list_head g_data_list;
spinlock_t rtw_hwsim_data_lock;

u16 sim_machine_id;
static unsigned long rand;

/* dummy */
#ifdef CONFIG_GLOBAL_UI_PID
int ui_pid[3] = {0, 0, 0};
#endif

int rtw_resume_process(_adapter *padapter)
{
	return 0;					/*  dummy */
}

void rtw_pci_aspm_config(_adapter *padapter)
{
	/* dummy, for 8814b */
}

static inline unsigned char random(void)
{
	/* See "Numerical Recipes in C", second edition, p. 284 */
	rand = rand * 1664525L + 1013904223L;
	return (unsigned char) (rand >> 24);
}

static inline u16 rtw_hwsim_get_random_machine_id(void)
{
	rand = jiffies;
	return (u16)(random() << 8) + random();
}

static void rtw_hwsim_alloc_mac_addr(int index, struct mac_address *addr)
{
	u8 *mac_addr;

	mac_addr = addr->addr;

	memcpy(mac_addr, rtw_hwsim_oui, 3);
	*(u16 *)(mac_addr + 3) = sim_machine_id;
	*(u8 *)(mac_addr + 5) = index;

	RTW_INFO_DUMP("mac addr: ", mac_addr, 6);
}

/*
 *
 * Original architecture:
 *		 +--------------------------------------------+
 *		 |                   wiphy                    |
 *		 +--------------------------------------------+
 *		 +------------+  +------------+  +------------+
 *		 |   adapter  |  |   adapter  |  |   adapter  |
 *		 +------------+  +------------+  +------------+
 *		 +--------------------------------------------+
 *		 |                   devobj                   |
 *		 +--------------------------------------------+
 *		 +--------------------------------------------+
 *		 |                   device                   |
 *		 +--------------------------------------------+
 *
 * Hwsim architecture (i.e., multiple vif is not supported)
 *		 +-------------------+
 *		 |       wiphy       |
 *		 +-------------------+
 *		 +-------------------+
 *		 |      adapter      |
 *		 +-------------------+
 *		 +-------------------+
 *		 |      dev obj      |
 *		 +-------------------+
 *		 +-------------------+
 *		 |       radio       |
 *		 +-------------------+
 *		 +-------------------+
 *		 |  platform device  |
 *		 +-------------------+
 */
static struct rtw_hwsim_data *rtw_hwsim_new_radio(int index)
{
	int err;
	struct device *dev;
	struct wiphy *wiphy;
	struct rtw_hwsim_data *data;
	struct dvobj_priv *devobj;
	struct rtw_hwsim_vif *vif;

	/* create a platform device */
	dev = device_create(rtw_hwsim_class, NULL, 0, NULL, "rtw_hwsim%d",
			    index);
	if (IS_ERR(dev)) {
		RTW_ERR("rtw_hwsim: device_create failed (%ld)\n",
			PTR_ERR(dev));
		goto failed;
	}

	dev->driver = &rtw_hwsim_driver.driver;
	err = device_bind_driver(dev);
	if (err != 0) {
		RTW_ERR("rtw_hwsim: device_bind_driver failed (%d)\n", err);
		goto failed_bind;
	}

	/*
	 * allocate/register wiphy for the device
	 * Note: enable monitor mode on the last radio only if we have multiple
	 * radios.
	 */
	data = rtw_hwsim_cfg80211_alloc(dev, index,
	                                (radios > 1 && index == radios));
	if (data == NULL) {
		RTW_ERR("rtw_hwsim: rtw_hwsim_cfg80211_alloc failed\n");
		goto failed_cfg80211_alloc;
	}

	data->dev = dev;

	/* prepare mac addresses for the device */
	rtw_hwsim_alloc_mac_addr(index, &(data->addresses[0]));

	err = rtw_hwsim_cfg80211_register(data);
	if (err < 0) {
		RTW_ERR("rtw_hwsim_cfg80211_register failed (%d)\n", err);
		goto failed_cfg80211_register;
	}

	/* allocate devobj for the device */
	devobj = rtw_hwsim_alloc_devobj(dev);
	if (devobj == NULL) {
		RTW_ERR("rtw_hwsim: allocate devobj failed\n");
		goto failed_alloc_devobj;
	}

	data->devobj = devobj;

	/* record module variables before add interface */
	data->medium = medium;
	data->udp = udp;

	/* allocate net device for the device */
	vif = rtw_hwsim_interface_add(data);

	if (!vif) {
		RTW_ERR("Failed to instantiate a network device\n");
		goto failed_interface_add;
	}

	/* init the base class, ref rtw_wiphy_alloc() */
	data->super.dvobj = devobj;
#ifndef RTW_SINGLE_WIPHY
	data->super.adapter = &vif->adapter;
#endif

	/* add vif to data */
	data->vifs[0] = vif;
	data->n_vifs = 1;

	return data;

failed_interface_add:
	rtw_hwsim_free_devobj(data->devobj);
failed_alloc_devobj:
	rtw_hwsim_cfg80211_unregister(data);
failed_cfg80211_register:
	rtw_hwsim_cfg80211_free(data);
failed_cfg80211_alloc:
	device_release_driver(dev);
failed_bind:
	device_unregister(dev);
failed:
	return NULL;
}

static void rtw_hwsim_del_radio(struct rtw_hwsim_data *data)
{
	int i;
	struct device *dev = data->dev;

	for (i = 0; i < data->n_vifs; ++i)
		rtw_hwsim_interface_del(data->vifs[i]);

	rtw_hwsim_free_devobj(data_to_devobj(data));
	rtw_hwsim_cfg80211_unregister(data);
	rtw_hwsim_cfg80211_free(data);
	device_release_driver(dev);
	device_unregister(dev);
}

static void rtw_hwsim_del_all_radio(void)
{
	struct list_head *c, *n;

	list_for_each_safe(c, n, &g_data_list) {
		struct rtw_hwsim_data *data;

		data = list_entry(c, struct rtw_hwsim_data, list);
		list_del(c);

		RTW_PRINT("deleting radio 0x%p\n", data);
		rtw_hwsim_del_radio(data);
	}
}

static struct notifier_block rtw_hwsim_inetaddr_notifier = {
	.notifier_call = rtw_hwsim_inetaddr_notifier_call,
};

static int __init rtw_hwsim_driver_init(void)
{
	int err, i;

	RTW_INFO("%s(): medium=%d\n", __func__, medium);

	spin_lock_init(&rtw_hwsim_data_lock);

	register_inetaddr_notifier(&rtw_hwsim_inetaddr_notifier);

	err = platform_driver_register(&rtw_hwsim_driver);
	if (err) {
		RTW_ERR("platform_driver_register failed\n");
		goto fail;
	}

	rtw_hwsim_class = class_create(THIS_MODULE, "rtw_hwsim");
	if (IS_ERR(rtw_hwsim_class)) {
		RTW_ERR("class_create failed\n");
		err = PTR_ERR(rtw_hwsim_class);
		goto fail_class_create;
	}

	sim_machine_id = rtw_hwsim_get_random_machine_id();

	INIT_LIST_HEAD(&g_data_list);

	for (i = 1; i <= radios; ++i) {
		struct rtw_hwsim_data *data;

		data = rtw_hwsim_new_radio(i);
		if (!data) {
			RTW_ERR("rtw_hwsim_new_radio failed\n");
			err = -1;
			goto fail_new_radio;
		}

		list_add_tail(&data->list, &g_data_list);
		RTW_PRINT("radio 0x%p allocated\n", data);
	}

	return 0;

fail_new_radio:
	rtw_hwsim_del_all_radio();
	class_destroy(rtw_hwsim_class);
fail_class_create:
	platform_driver_unregister(&rtw_hwsim_driver);
fail:
	unregister_inetaddr_notifier(&rtw_hwsim_inetaddr_notifier);
	return err;
}

static void __exit rtw_hwsim_driver_exit(void)
{
	RTW_INFO("%s(): medium=%d\n", __func__, medium);
	unregister_inetaddr_notifier(&rtw_hwsim_inetaddr_notifier);
	rtw_hwsim_del_all_radio();
	class_destroy(rtw_hwsim_class);
	platform_driver_unregister(&rtw_hwsim_driver);
}

module_init(rtw_hwsim_driver_init);
module_exit(rtw_hwsim_driver_exit);
