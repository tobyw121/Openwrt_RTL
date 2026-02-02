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
#ifndef __NETIFD_WIRELESS_H
#define __NETIFD_WIRELESS_H

#include <libubox/utils.h>
#include <libubox/list.h>
#include <libubox/blobmsg.h>
#include <netinet/in.h>
#include "device.h"
#include "interface.h"

#define BRIDGE_NAME 32

struct wireless_device {
	struct vlist_node node;
	struct blob_attr *config;
	const struct device_type *devtype;
	char vap[IFNAMSIZ + 1];
	char ifname[IFNAMSIZ + 1];
	char bridge[BRIDGE_NAME];
};


void wireless_device_hotplug_event(const char *name, bool add);
void wireless_init(void);
void wireless_device_create(const char *vap, const char *name, struct blob_attr *config, const struct device_type *devtype, char *type);
void wireless_reload(void);

#endif
