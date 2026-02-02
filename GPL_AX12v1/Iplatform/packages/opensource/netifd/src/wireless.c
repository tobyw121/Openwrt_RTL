/*
 * netifd - network interface daemon
 * Copyright (C) 2013 Felix Fietkau <nbd@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* The wireless configuration is projected on the following objects
 *
 * 1. wireless device
 * 2. wireless interface
 * 3. wireless vlan
 * 4. wireless station
 *
 * A wireless device is a phy or simplified a wireless card.
 * A wireless interface is a SSID on a phy.
 * A wireless vlan can be assigned to a wireless interface. A wireless interface can
 *   have multiple vlans.
 * A wireless station is a client connected to an wireless interface.
 */


#include <signal.h>
#include <unistd.h>
#include <uci.h>
#include <sys/ioctl.h>
#include "netifd.h"
#include "wireless.h"
#include "config.h"
#include "ubus.h"
#include "connect.h"
#include "system.h"

struct vlist_tree wireless_devices;

/* creates a wireless device object. Called by config */
void
wireless_device_create(const char *vap, const char *name, struct blob_attr *config, const struct device_type *devtype , char* bridge)
{
	struct wireless_device *wdev = NULL;
	struct blob_attr *config_new = NULL;

	if(vlist_find(&wireless_devices, name, wdev, node))
		return;
	
	wdev = (struct wireless_device *)calloc(1, sizeof(struct wireless_device));
	if (!wdev)
	{
		D(INTERFACE, "create wireless device %s fail\n", name);
		return;
	}

	if(config)
	{
		config_new = config_memdup(config);
		if (!config_new)
		{
			free(wdev);
			return;
		}
	}

	strncpy(wdev->vap, vap, sizeof(wdev->vap));
	strncpy(wdev->ifname, name, sizeof(wdev->ifname));
	wdev->config = config_new;
	wdev->devtype = devtype;
	strncpy(wdev->bridge, bridge, sizeof(wdev->bridge));

	vlist_add(&wireless_devices, &wdev->node, wdev->ifname);

	D(INTERFACE, "create wireless device %s \n", name);
}

static void
wireless_device_release(struct wireless_device *wdev)
{
	if(!wdev)
		return;
	if(wdev->config)
		free(wdev->config);

	free(wdev);
}

/* vlist update call for wireless device list */
static void
wdev_update(struct vlist_tree *tree, struct vlist_node *node_new,
	    struct vlist_node *node_old)
{
	struct wireless_device *wd_old = container_of(node_old, struct wireless_device, node);
	//struct wireless_device *wd_new = container_of(node_new, struct wireless_device, node);

	if(wd_old)
		wireless_device_release(wd_old);

}

static int interface_handle_link(struct interface *iface, struct wireless_device *wdev, bool add, bool link_ext)
{
	struct device *dev = NULL;

	dev = device_get(wdev->ifname, add ? (link_ext ? 2 : 1) : 0);
	if (!dev)
		return 1;

	if (!add){
		device_set_present(dev, add);
		return interface_remove_link(iface, dev);
	}

	if(strcmp(wdev->ifname, iface->ifname) != 0)
		return 1;

	if(!wdev->config)
		return 1;
		
	device_set_config(dev, wdev->devtype ? wdev->devtype : &simple_device_type, wdev->config);
	
	/*
	device_create中根据network配置创建device后会current_config置true
	无线接口在device_create无法创建，因此在这里创建后设置
	dev->config是用于device_free，无线直接存储在wdev->config,因此不设置
	*/
	dev->current_config = true;

	if (iface->device_config)
		device_set_config(dev, &simple_device_type, iface->config);
	
	if (!link_ext)
		device_set_present(dev, true);

	return interface_add_link(iface, dev);
}


static void wireless_gatway_interface_handle_link(struct wireless_device *wdev, bool add)
{
	struct interface *ifaceWan = NULL;
	struct interface *ifaceWanv6 = NULL;

	ifaceWan = vlist_find(&interfaces, CONNECT_IFACE_WAN, ifaceWan, node);
	if(ifaceWan && ifaceWan->connectable)
		interface_handle_link(ifaceWan, wdev, add, true);

	ifaceWanv6 = vlist_find(&interfaces, CONNECT_IFACE_WANV6, ifaceWanv6, node);
	if(ifaceWanv6 && ifaceWanv6->connectable)
		interface_handle_link(ifaceWanv6, wdev, add, true);

}

static void wireless_bridge_interface_handle_link(struct wireless_device *wdev, bool add)
{
	struct interface *iface = NULL;
	struct device *dev = NULL;

	iface = vlist_find(&interfaces, wdev->bridge, iface, node);
	if(!iface || iface->state != IFS_UP)
	{
		D(INTERFACE, "error : iface %s not add\n", wdev->bridge);
		return;
	}

	dev = device_get(wdev->ifname, add ?  2 : 0);
	if (!dev)
	{
		D(INTERFACE, "iface %s get fail\n", wdev->ifname);
		return;
	}

	D(INTERFACE, "iface %s action %s\n", wdev->ifname, add ? "ADD" : "DEL");

	if (add)
	{
		if (!dev->link_active)
			system_bridge_delif(iface->main_dev.dev, dev);

		bridge_add_member_ext(iface->main_dev.dev, dev);
	}
	else
		bridge_del_member_ext(iface->main_dev.dev, dev);

	return;
}

/**/
void wireless_reload(void)
{
	struct wireless_device *wdev;
	struct interface *iface = NULL;
	int flags;

	vlist_for_each_element(&wireless_devices, wdev, node) 
	{
		if(strlen(wdev->ifname) == 0)
			continue;

		flags = system_if_get_flags(wdev->ifname);
		if(!(flags & IFF_UP))
			continue;

		D(INTERFACE, "reload wireless device %s \n", wdev->ifname);

		if(strcmp(wdev ->bridge, CONNECT_IFACE_WAN) == 0)
		{
			iface = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);
			if(!iface)
				continue;

			if ( strcmp(wdev->ifname, iface->ifname) != 0)
				continue;

			wireless_gatway_interface_handle_link(wdev, true);
		}
		
		if(strcmp(wdev ->bridge, CONNECT_IFACE_LAN) == 0)
			wireless_bridge_interface_handle_link(wdev, true);
	}

	return;
}

void wireless_init(void)
{
	vlist_init(&wireless_devices, avl_strcmp, wdev_update);
	wireless_devices.keep_old = true;
	wireless_devices.no_delete = true;
}

void wireless_device_hotplug_event(const char *name, bool add)
{
	struct wireless_device *wdev;
	size_t len;
	
	if(!name || (len = strlen(name)) == 0)
		return;

	vlist_for_each_element(&wireless_devices, wdev, node) 
	{
		if (!wdev->ifname)
			continue;

		if (strlen(wdev->ifname) != len ||
		    strncmp(wdev->ifname, name, len) != 0)
			continue;

		if(strcmp(wdev->bridge, CONNECT_IFACE_WAN) == 0)
			wireless_gatway_interface_handle_link(wdev, add);

		if(strcmp(wdev->bridge, CONNECT_IFACE_LAN) == 0)
			wireless_bridge_interface_handle_link(wdev, add);
	}
}

