#include "backup_config.h"
#include "usbmodem_log.h"
#include "netifd.h"
#include "interface.h"
#include "proto.h"
#include "ubus.h"
#include "system.h"
#include "connect.h"


static struct uci_context *bk_uci_ctx = NULL;
static struct uci_package *bk_uci_pkg = NULL;
#ifdef FEATURE_3G4G_MODULES_INDEPENDENT
static enum conn_mode curr_conn_mode = CONN_MODE_CABLE_PREFERRED;
#else
static enum conn_mode curr_conn_mode = CONN_MODE_CABLE_ONLY;
#endif
static enum network_type current_network = NETWORK_TYPE_WIRED;
static int curr_nw_status = 0;
static bool backup_enable = false; /* false = off, true = on*/

char *network_type_str[] = {
	"",
	"WIRED",
	"MOBILE",
	""
};

#define CONNECT_IFACE_WAN       	"wan"
#define CONNECT_IFACE_INTERNET  	"internet"
#define CONNECT_IFACE_MOBILE		"mobile"
#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
#define CONNECT_IFACE_VPN			"vpn"
#endif

enum network_type get_network_type(const char* name)
{
	if (strcmp(name, CONNECT_IFACE_WAN) == 0 ||
		strcmp(name, CONNECT_IFACE_INTERNET) == 0)
	{
		return NETWORK_TYPE_WIRED;
	}
	else if (strcmp(name, CONNECT_IFACE_MOBILE) == 0)
	{
		return NETWORK_TYPE_MOBILE;
	}
	return NETWORK_TYPE_MAX;
}

int set_curr_nw_status(int ns)
{
	curr_nw_status = ns;
	return 0;
}

int get_curr_nw_status()
{
	return curr_nw_status;
}

bool get_backup_enable()
{
	return backup_enable;	/* false = off, true = on*/
}

bool backup_auto_dial(const char* ifname)
{
	enum network_type nt = get_network_type(ifname);
	bool ret = true;

#ifdef FEATURE_3G4G_MODULES_INDEPENDENT
	if (1)
#else
	if (backup_enable)
#endif
	{
		/*
		 * In x_ONLY mode: auto dial ifname that belongs to x
		 * In x_PREFERRED mode: if primary connection is offline, auto dial any ifname
		 *                      else do what x_ONLY mode do
		 */
		switch (curr_conn_mode)
		{
		case CONN_MODE_CABLE_PREFERRED:
			if (!(curr_nw_status & 0x1))	/* primary connection is NOT online */
			{
				break;
			}
		case CONN_MODE_CABLE_ONLY:
			if (nt == NETWORK_TYPE_MOBILE)
			{
				ret = false;
			}
			break;
		case CONN_MODE_MOBILE_PREFERRED:
			if (!(curr_nw_status & 0x1))
			{
				break;
			}
		case CONN_MODE_MOBILE_ONLY:
			if (nt == NETWORK_TYPE_WIRED)
			{
				ret = false;
			}
			break;
		default:
			break;
		}
	}
	else	/* If backup NOT enabled, NEVER auto dial mobile iface*/
	{
		if (nt == NETWORK_TYPE_MOBILE)
		{
			ret = false;
		}
	}
	return ret;
}

int set_ubus_curr_nw(enum network_type type)
{
	if (type != NETWORK_TYPE_WIRED && type != NETWORK_TYPE_MOBILE)
	{
		return -1;
	}
	current_network = type;

	if (backup_enable)
	{
		USBMODEM_DBG(LOG_BK_SWITCH_NW, LP_STRING, network_type_str[type]);
	}
	return 0;
}

char* get_ubus_curr_nw()
{
	if (current_network < NETWORK_TYPE_NONE || current_network > NETWORK_TYPE_MAX)
	{
		return NULL;
	}
	return network_type_str[current_network];
}

static char* bk_config_option_str(const char *section_name, const char *name)
{
	struct uci_package *pkg = bk_uci_pkg;
	struct uci_section *section = NULL;

	section = uci_lookup_section(bk_uci_ctx, pkg, section_name);
	if (!section)
	{
		return NULL;
	}
	return (char *)uci_lookup_option_string(bk_uci_ctx, section, name);
}

static int config_option_bool(const char *section_name, const char *name, bool *enable)
{
	char *val = bk_config_option_str(section_name, name);
	if (!val)
	{
		return -1;
	}

	if (strcmp(val, "on") == 0)
	{
		*enable = true;
	}
	else
	{
		*enable = false;
	}

	return 0;
}

static int config_option_int(const char *section_name, const char *name, int *val)
{
	char *str = bk_config_option_str(section_name, name);
	unsigned long tmp;

	if (!str)
	{
		return -1;
	}

	tmp = strtoul(str, NULL, 10);
	if (tmp == ULONG_MAX)
	{
		return -1;
	}

	*val = (int)tmp;

	return 0;
}

static int bk_uci_init()
{
	int ret = 0;
	bk_uci_ctx = uci_alloc_context();
	if (bk_uci_ctx == NULL)
	{
		return -1;
	}

	ret = uci_load(bk_uci_ctx, BACKUP_UCI_CFG, &bk_uci_pkg);
	if (ret != 0)
	{
		uci_free_context(bk_uci_ctx);
		bk_uci_ctx = NULL;
	}

	return ret;
}

static void bk_uci_exit()
{
	if (bk_uci_ctx && bk_uci_pkg)
	{
		uci_unload(bk_uci_ctx, bk_uci_pkg);
		bk_uci_pkg = NULL;
		uci_free_context(bk_uci_ctx);
		bk_uci_ctx = NULL;
	}
	return;
}

int uci_bk_cfg_get(struct nw_backup_cfg *bk_cfg)
{
	int ret = 0;
	int tmp = 0;

	if (bk_cfg == NULL)
	{
		return -1;
	}

	memset(bk_cfg, 0, sizeof(struct nw_backup_cfg));

	ret = bk_uci_init();
	if (ret != 0)
	{
		return ret;
	}

	ret = config_option_bool("backup", "enable", &bk_cfg->use_backup);
	if (ret != 0)
	{
		goto exit;
	}
	backup_enable = bk_cfg->use_backup;

	if (backup_enable)
	{
		ret = config_option_int("backup", "preferred", &tmp);
		if (ret != 0)
		{
			goto exit;
		}
		curr_conn_mode = bk_cfg->mode = (enum conn_mode)tmp;
	}

exit:
	bk_uci_exit();

	return ret;
}

#ifdef SUPPORT_VPN_CLIENT_IN_3G4G_MODULES
int set_vpn_client_parent(const char* ifname)
{
	struct uci_context *uci_ctx;
	struct uci_package *uci_network;
	struct uci_section *section = NULL; 
	const char *old_parent = NULL;
	struct interface *iface = NULL;
	int ret = 0;

	uci_ctx = uci_alloc_context();
	uci_set_confdir(uci_ctx, "/etc/config");
	if (UCI_OK != uci_load(uci_ctx, "network", &uci_network))
	{
		uci_perror(uci_ctx, "network");
		goto not_exist;
	}
	
	if (NULL != ( section = uci_lookup_section(uci_ctx, uci_network, "vpn") ))	
	{	
		old_parent = uci_lookup_option_string(uci_ctx, section, "parent");

		if ( !strcmp(old_parent, ifname) )
		{
			//system("echo old_parent == new_parent, pass > /dev/console");
			goto not_exist;
		}
		
		struct uci_ptr parent_op_ptr = {
			.package = "network",
			.section = "vpn",
			.option = "parent",
			.value = ifname,
		};

		if (UCI_OK != uci_set(uci_ctx, &parent_op_ptr))
		{
			netifd_log_message(L_ERROR, "Failed to set vpn parent\n");
			goto not_exist;
		}
		if (UCI_OK != uci_commit(uci_ctx, &uci_network, false))
		{
			netifd_log_message(L_ERROR, "Failed to commit vpn parent set\n");
			goto not_exist;
		}
		//system("echo success reset_vpn_client_parent > /dev/console");

		ret = 1;
	
	}  
	
	
not_exist:
	uci_unload(uci_ctx, uci_network);
	uci_free_context(uci_ctx);
	uci_ctx = NULL;
	

	if (ret)
	{
		vlist_for_each_element(&interfaces, iface, node)
		{
			if (strcmp(iface->name, CONNECT_IFACE_VPN) != 0)
			{
				continue;
			}

			interface_set_down(iface);
			interface_set_available(iface, true);
			interface_set_up(iface);
		}
		//system("echo have restart vpn iface > /dev/console");
	}
	
	//system("echo end set_vpn_client_parent5 > /dev/console");
	
	return ret;  

}

#endif


