#ifndef __BACKUP_CONFIG_H
#define __BACKUP_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uci.h>
#include <limits.h>

#define BACKUP_UCI_CFG	"/etc/config/protocol"

enum network_type
{
	NETWORK_TYPE_NONE = 0,
	NETWORK_TYPE_WIRED,
	NETWORK_TYPE_MOBILE,
	NETWORK_TYPE_MAX
};

enum conn_mode
{
	CONN_MODE_MOBILE_PREFERRED = 0,
	CONN_MODE_MOBILE_ONLY,
	CONN_MODE_CABLE_PREFERRED,
	CONN_MODE_CABLE_ONLY,
	CONN_MODE_MAX 
};

struct nw_backup_cfg
{
	bool use_backup;
	enum conn_mode mode;
	enum network_type primary_nw;
	enum network_type secondary_nw;
	enum network_type detecting_nw;
	enum network_type current_nw;
	int detect_ret;
	int stable;
};

enum network_type get_network_type(const char* name);
int uci_bk_cfg_get(struct nw_backup_cfg *bk_cfg);
void unset_nw(enum network_type type);
int set_curr_nw_status(int ns);
int get_curr_nw_status();
bool get_backup_enable();
int set_ubus_curr_nw(enum network_type type);
char* get_ubus_curr_nw();
bool backup_auto_dial(const char* ifname);

#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
int set_vpn_client_parent(const char* ifname);
#endif


#endif
