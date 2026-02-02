/*! Copyright(c) 2008-2014 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     connect.c
 *\brief        
 *\details  
 *
 *\author   Zhu Xianfeng<zhuxianfeng@tp-link.net>
 *\version  1.0.0
 *\date     29May14
 *
 *\warning  
 *
 *\history \arg 29May14, Zhu Xianfeng, create the file.
 */
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "netifd.h"
#include "interface.h"
#include "proto.h"
#include "system.h"
#include "connect.h"
#ifdef FEATURE_3G4G_MODULES
#if FEATURE_3G4G_MODULES == 2
#include "backup_config_v2.h"
#else
#include "backup_config.h"
#endif
#include "usbmodem_log.h"
#endif

#ifdef EXTEND_SOCKETOPT_BASE_NUM
#define IPT_STAT_BASE_CTL       (1280)
#else
#define IPT_STAT_BASE_CTL       (128)
#endif

#define IPT_STAT_SET_NET        (IPT_STAT_BASE_CTL)
#define IPT_STAT_DEL_ALL        (IPT_STAT_BASE_CTL + 5)
#define IPT_STAT_GET_WAN_STAT   (IPT_STAT_BASE_CTL + 1)
#ifdef FEATURE_3G4G_MODULES
#define IPT_STAT_GET_WANS_TX_STAT	(IPT_STAT_BASE_CTL + 2)

#define MAX_CONNECT_IFACE_NUM	10
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)           (sizeof(x)/sizeof(x[0]))
#endif

#define CONNECT_MACADDR_LEN     	(6)
#define CONNECT_TIMER_PERIOD		(1000)
#define dprintf(fmt, args...)   	D(INTERFACE, fmt, ##args)

struct ipt_net
{
    uint32_t ip;
    uint32_t mask;
};

#ifndef FEATURE_3G4G_MODULES
struct ipt_stat
{
    uint32_t rcv_pkt;
    uint32_t rcv_byte;
    uint32_t snd_pkt;
    uint32_t snd_byte;
};
#else
struct ipt_tx_stat{
	char tx_devname[IFNAMSIZ + 1];
	uint32_t snd_pkt;
	uint32_t snd_byte;
};

struct {
	struct ipt_tx_stat stats[MAX_CONNECT_IFACE_NUM];
	int num;
}wans_tx_data;

struct iface_stat
{
	char ifname[IFNAMSIZ + 1];
	time_t last_in_sec;			/* lan -> wan had outgoing pkts last second    */
	time_t last_ex_sec;			/* lan -> wan hadn't outgoing pkts last second */
	struct ipt_tx_stat last_stats[2];
};
#endif
struct conn_handler
{
    enum interface_conn_mode mode;
    void (*process)(struct interface *iface);
};
#ifndef FEATURE_3G4G_MODULES
static int connect_ipt_outgone();
#else
static int connect_ipt_outgone(struct interface *iface);
#endif
static void connect_auto(struct interface *iface);
static void connect_manual(struct interface *iface);
static void connect_demand(struct interface *iface);
static void connect_timebased(struct interface *iface);
static void connect_dft_route(int add);

/*add by wanghao*/
char connect_iface_wan_ifname[16] = {0};
/*add end*/

static int initialized;
static int tmp_ip_set;
static char tmp_ip_str[16];
#ifndef FEATURE_3G4G_MODULES
static time_t last_in_sec;         /* lan -> wan had outgoing pkts last second    */
static time_t last_ex_sec = 0;         /* lan -> wan hadn't outgoing pkts last second */
static struct ipt_stat last_stat;
#else
static struct iface_stat ifaces_stat[MAX_CONNECT_IFACE_NUM];
static int ifaces_stat_len = 0;
static bool get_tx_info = false;
#endif
static struct uloop_timeout timeout;
static struct conn_handler conn_handlers[] =
{
    {
        .mode = IFCM_AUTO,
        .process = connect_auto,
    },
    {
        .mode = IFCM_MANUAL,
        .process = connect_manual,
    },
    {
        .mode = IFCM_DEMAND,
        .process = connect_demand,
    },
    {
        .mode = IFCM_TIMEBASED,
        .process = connect_timebased,
    },
};
#ifdef FEATURE_3G4G_MODULES
static int find_wan_tx_data_index(char *tx_devname)
{
	int i = 0;
	for (i = 0; i < wans_tx_data.num; i++)
	{
		if (strcmp(wans_tx_data.stats[i].tx_devname, tx_devname) == 0)
		{
			return i;
		}
	}

	return -1;
}

static int find_ifaces_stats_index(char *ifname)
{
	int i = 0;
	for (i = 0; i < ifaces_stat_len; i++)
	{
		if (strcmp(ifaces_stat[i].ifname, ifname) == 0)
		{
			return i;
		}
	}

	return -1;
}

static struct device * get_dev_by_iface(struct interface *iface, bool real_dev)
{
	struct interface *phy_if = NULL;
	struct device * ret_dev = NULL;

	if (real_dev)
	{
		if ((!strcmp(iface->name, CONNECT_IFACE_INTERNET) ||
		!strcmp(iface->name, CONNECT_IFACE_INTERNETV6)) &&
		!iface->main_dev.dev)
		{
			phy_if = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);
		}
	}

	if (!phy_if)
	{
		phy_if = iface;
	}

	if ((strcmp(iface->name, CONNECT_IFACE_MOBILE) == 0 || !real_dev) && iface->l3_dev.dev)
	{
		ret_dev = iface->l3_dev.dev;
	}
	else if (phy_if->main_dev.dev)
	{
		ret_dev = phy_if->main_dev.dev;
	}

	return ret_dev;
}

static bool has_two_connections(struct interface *iface)
{
	bool ret = false;
	if (strcmp(iface->name, CONNECT_IFACE_INTERNET) == 0)
	{
		if (iface->proto_handler && (strcmp(iface->proto_handler->name, "l2tp") == 0 ||
			strcmp(iface->proto_handler->name, "pptp") == 0 || 
			strcmp(iface->proto_handler->name, "pppoe") == 0 ||
			strcmp(iface->proto_handler->name, "pppoeshare") == 0))
		{
			ret = true;
		}
	}

	return ret;
}

static int wan_tx_stat_increase(struct interface *iface)
{
	int ret = 0;
	int tx_data_index = 0;
	int iface_id = 0;
	int count = 0;
	bool real_dev = true;
	static struct device * dev = NULL;
	iface_id = find_ifaces_stats_index(iface->name);
	if (iface_id < 0)
	{
		return ret;
	}

cmp:
	dev = get_dev_by_iface(iface, real_dev);
	if (!dev)
	{
		ret |= 0;
		goto nxt_conn;
	}
	tx_data_index = find_wan_tx_data_index(dev->ifname);
	if (tx_data_index < 0)
	{
		ret |= 0;
		goto nxt_conn;
	}

	if (wans_tx_data.stats[tx_data_index].snd_pkt != 0 &&
		wans_tx_data.stats[tx_data_index].snd_pkt > ifaces_stat[iface_id].last_stats[count].snd_pkt)
	{
		ret |= 1;
	}

	memcpy(&(ifaces_stat[iface_id].last_stats[count]), &(wans_tx_data.stats[tx_data_index]), sizeof(struct ipt_tx_stat));

nxt_conn:
	if (has_two_connections(iface) && count < 1)
	{
		count++;
		real_dev = false;
		goto cmp;
	}

	return ret;
}

static bool all_wans_down()
{
	struct interface *iface = NULL;
	struct interface *tmp_iface = NULL;
	vlist_for_each_element(&interfaces, iface, node)
	{
		if (strcmp(iface->name, CONNECT_IFACE_WAN) != 0 &&
			strcmp(iface->name, CONNECT_IFACE_INTERNET) != 0 &&
			strcmp(iface->name, CONNECT_IFACE_MOBILE) != 0)
		{
			continue;
		}

		if (iface->state != IFS_DOWN)
		{
			/* Fixed me? when pppoe disconnect, the wan interface do not disconnect... */
			if (strcmp(iface->name, CONNECT_IFACE_WAN) == 0 &&
				vlist_find(&interfaces, CONNECT_IFACE_INTERNET, tmp_iface, node))
			{
				continue;
			}

			return false;
		}
	}
	return true;
}

static bool all_wans_not_ondemand()
{
	struct interface *iface = NULL;
	vlist_for_each_element(&interfaces, iface, node)
	{
		if (strcmp(iface->name, CONNECT_IFACE_WAN) != 0 &&
			strcmp(iface->name, CONNECT_IFACE_INTERNET) != 0 &&
			strcmp(iface->name, CONNECT_IFACE_MOBILE) != 0)
		{
			continue;
		}

		if (iface->conn_mode == IFCM_DEMAND)
		{
			return false;
		}
	}
	return true;
}
#endif

static void
connect_ifup(struct interface *iface)
{
    struct interface *parent = NULL;

    if (!strcmp(iface->name, CONNECT_IFACE_INTERNET) ||
        !strcmp(iface->name, CONNECT_IFACE_INTERNETV6))
    {
        parent = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);

        if (parent && parent->state == IFS_DOWN)
        {
            /* dprintf("interface(%p): %s, available: %d, connectable: %d, state: %d, conn_mode: %d\n",
                    iface, iface->name, iface->available, 
                    iface->connectable, iface->state, iface->conn_mode); */
            (void)interface_set_up(parent);
        }

        if (iface->state == IFS_DOWN && iface->connectable)
        {
            dprintf("interface(%s) set up\n", iface->name);
            (void)interface_set_up(iface);
        }
    }
#ifdef FEATURE_3G4G_MODULES
    else if (!strcmp(iface->name, CONNECT_IFACE_MOBILE))
    {
#ifdef FEATURE_3G4G_MODULES_INDEPENDENT
        if (iface->state == IFS_DOWN )
#else
        if (iface->state == IFS_DOWN && get_backup_enable())
#endif
        {
            (void)interface_set_up(iface);
            (void)interface_set_up_child(iface);
        }
    }
#endif
    else 
    {
        /* wan or wanv6 */
        if (iface->state == IFS_DOWN)
        {
            (void)interface_set_up(iface);
	    (void)interface_set_up_child(iface);
        }
    }
}

void
connect_ifdown(struct interface *iface)
{
#if 1
    struct interface *parent = NULL;
    bool pppoe = false;

    if (!strcmp(iface->name, CONNECT_IFACE_INTERNET) ||
        !strcmp(iface->name, CONNECT_IFACE_INTERNETV6))
    {
        parent = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);

        /* Sequential action: 
         *   1. Down L2TP/PPTP/PPPoE;
         *   2. Down DHCP;
         * Make sure both PPP Termination and DHCP Release frame was transmitted.
         */

        (void)interface_set_down(iface);

        /*
         * ppp类型的链接不检查其父interface，即第二链接的状态
         */
        if (iface->proto_handler && ((strcmp(iface->proto_handler->name, "pppoe") == 0) || 
                    (strcmp(iface->proto_handler->name, "pppoeshare") == 0)  || 
                    (strcmp(iface->proto_handler->name, "pptp") == 0) ||
                    (strcmp(iface->proto_handler->name, "l2tp") == 0)))
        {
            pppoe = true;
        }

        if (iface->state == IFS_DOWN && parent && !pppoe)
        {
            (void)interface_set_down(parent);
        }
    }
    else 
    {
        /* wan or wanv6 */
        (void)interface_set_down(iface);
	(void)interface_set_down_child(iface);
    }
#else
    (void)interface_set_down(iface);
#endif
}


static bool 
connect_state_is_down(struct interface *iface)
{
	struct interface *parent = NULL;
	
	if (iface->state == IFS_UP) 
	{
		return false;
	}

    /*
     * ppp类型的链接断开时，不对其父interface操作
     */
	if ((!strcmp(iface->name, CONNECT_IFACE_INTERNET) ||
		!strcmp(iface->name, CONNECT_IFACE_INTERNETV6)) &&
        !(iface->proto_handler && (strcmp(iface->proto_handler->name, "pppoe") == 0 || 
                    strcmp(iface->proto_handler->name, "pppoeshare") == 0 ||
                    strcmp(iface->proto_handler->name, "pptp") == 0 ||
                    strcmp(iface->proto_handler->name, "l2tp") == 0 )))
	{
		parent = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);

		if (parent && parent->state == IFS_UP)
		{
			return false;
		}
	}

	return true;
}

static void 
connect_auto(struct interface *iface)
{
#if	0
     bool dhcp = false;
    struct interface *internet = NULL;

    internet = vlist_find(&interfaces, CONNECT_IFACE_INTERNET, internet, node);
    if (!internet && strcmp(iface->name, CONNECT_IFACE_WAN) == 0 &&
        iface->proto_handler && strcmp(iface->proto_handler->name, "dhcp") == 0)
    {
        dhcp = true;
    }


    /* device unplugged */
    if (dhcp && !iface->link_state)
    {
        if (iface->state == IFS_UP)
        {
            (void)interface_set_down(iface);
        }
        
        return;
    }
#endif
    bool static_proto = false;
    struct interface *internet = NULL;

    internet = vlist_find(&interfaces, CONNECT_IFACE_INTERNET, internet, node);
    if (!internet && strcmp(iface->name, CONNECT_IFACE_WAN) == 0 &&
        iface->proto_handler && strcmp(iface->proto_handler->name, "static") == 0)
    {
        static_proto = true;
    }

    /* device unplugged */
    if (static_proto && !iface->link_state)
    {
        if (iface->state == IFS_UP)
        {
            (void)interface_set_down(iface);
        }
#ifdef MTK_MT762X
        const char* wan_type = config_option_str(CONNECT_IFACE_WAN, "type");
        if(!strcmp(iface->name, CONNECT_IFACE_WAN) && wan_type && !strcmp(wan_type, "bridge"))
	{
	    //do nothing,goto connect_ifup
        }
        else
	{
            return;
	}
#else
        return;
#endif
    }

#ifndef FEATURE_3G4G_MODULES
    if (iface->state != IFS_UP) 
#else
    if (iface->state != IFS_UP && backup_auto_dial(iface->name)) 
#endif
    {
        connect_ifup(iface);
    }
}

static void 
connect_manual(struct interface *iface)
{
	time_t now = 0;
	int ret = 0;
#ifdef FEATURE_3G4G_MODULES
	int iface_id = 0;
#endif

	dprintf("interface: %s, state: %d, idle: %ld\n", 
		iface->name, iface->state, iface->conn_time.idle);
#ifdef FEATURE_3G4G_MODULES
	iface_id = find_ifaces_stats_index(iface->name);
	if (iface_id < 0)
	{
		return;
	}
#endif

	if (iface->conn_time.idle == 0)
	{
		return;
	}

	if (connect_state_is_down(iface)) 
	{
#ifndef FEATURE_3G4G_MODULES
        last_ex_sec = 0;
#else
		ifaces_stat[iface_id].last_ex_sec = 0;
#endif
		return;
	}
#ifndef FEATURE_3G4G_MODULES
    ret = connect_ipt_outgone();
#else
	ret = connect_ipt_outgone(iface);
#endif
	dprintf("lan -> wan had outgoing pkts: %s\n", ret ? "yes" : "no");
	now = time(NULL);
	if (ret)
	{
#ifndef FEATURE_3G4G_MODULES
        /* lan -> wan had outgoing pkts */
		last_ex_sec = now;
#else
		/* lan -> wan had outgoing pkts */
		ifaces_stat[iface_id].last_ex_sec = now;
#endif
		return;
	}

#ifndef FEATURE_3G4G_MODULES
	if (!last_ex_sec)
	{
		last_ex_sec = now;
	}

	if ((now - last_ex_sec) >= iface->conn_time.idle)
	{
#else
	if (!ifaces_stat[iface_id].last_ex_sec)
	{
		ifaces_stat[iface_id].last_ex_sec = now;
	}

	if ((now - ifaces_stat[iface_id].last_ex_sec) >= iface->conn_time.idle)
	{
		USBMODEM_DBG(LOG_CONN_MODE_MANUALLY_TIMEOUT, LP_STRING, iface->name);
#endif
		connect_ifdown(iface);
		dprintf("interface(%s) set down\n", iface->name);
	}

	return; 
}

static void 
connect_demand(struct interface *iface)
{
    time_t now;
    int ret = 0;
#ifndef FEATURE_3G4G_MODULES	
    struct interface *parent = NULL;
#endif
    
#ifdef FEATURE_3G4G_MODULES
    int iface_id = 0;
    struct interface *tmp_iface = NULL;
    static bool need_syn = true;

    iface_id = find_ifaces_stats_index(iface->name);
    if (iface_id < 0)
    {
        return;
    }

    tmp_iface = vlist_find(&interfaces, CONNECT_IFACE_WAN, tmp_iface, node);
    if (tmp_iface == NULL)
    {
        return;
    }
#endif

#ifndef FEATURE_3G4G_MODULES
    if (!connect_state_is_down(iface))
    {
        connect_dft_route(0);

        if (iface->conn_time.idle == 0)
        {
            return;
        }

        ret = connect_ipt_outgone();
        now = time(NULL);
        if (ret)
        {
            last_in_sec = now;
            return;
        }

        if (last_in_sec == 0)
        {
            last_in_sec = now;
        }

        if ((now - last_in_sec) >= iface->conn_time.idle)
        {
            connect_ifdown(iface);
        }
    }
    else 
    {
        /* 第二链接按目前需求，需要为auto的模式 */
        parent = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);
        if (parent && parent->state == IFS_DOWN && parent->connectable)
        {
            connect_ifup(parent);
        }
        
        /* pppoe可以不设置第二链接，这时需要配置一个临时路由信息，以便能获取到用户的数据包 */
        if (iface->state == IFS_DOWN && (iface->proto_handler &&
                (strcmp(iface->proto_handler->name, "pppoe") == 0 || 
                strcmp(iface->proto_handler->name, "pppoeshare") == 0)))
        {
            connect_dft_route(1);
        }

        ret = connect_ipt_outgone();
        if (!ret)
        {
            return;
        }

        last_in_sec = time(NULL);
        
        /* pppoe可以不设置第二链接，如果检查到数据包了，准备开始链接，就去掉之前设置的临时路由 */
       if (iface->state == IFS_DOWN && (iface->proto_handler &&
                (strcmp(iface->proto_handler->name, "pppoe") == 0 || 
                strcmp(iface->proto_handler->name, "pppoeshare") == 0)))
        {
        connect_dft_route(0);
        }
        connect_ifup(iface);
    }
#else
    if (!connect_state_is_down(iface))
    {
        need_syn = true;
        connect_dft_route(0);

        if (iface->conn_time.idle == 0)
        {
            return;
        }

        ret = connect_ipt_outgone(iface);
        now = time(NULL);
        if (ret)
        {
            ifaces_stat[iface_id].last_in_sec = now;
            return;
        }

        if (ifaces_stat[iface_id].last_in_sec == 0)
        {
            ifaces_stat[iface_id].last_in_sec = now;
        }

        if ((now - ifaces_stat[iface_id].last_in_sec) >= iface->conn_time.idle)
        {
            USBMODEM_DBG(LOG_CONN_MODE_DEMAND_TIMEOUT, LP_STRING, iface->name);
            connect_ifdown(iface);
        }
    }
    else if (all_wans_down())
    {
        if (need_syn)
        {
            connect_ipt_outgone(tmp_iface);	/* just syn wan stats to avoid auto dial beaucause error data */
            need_syn = false;
        }
        connect_dft_route(1);

        ret = connect_ipt_outgone(tmp_iface);
        if (!ret)
        {
            return;
        }

        connect_dft_route(0);

        /* all demand iface up */
        USBMODEM_DBG(LOG_CONN_MODE_DEMAND);
        vlist_for_each_element(&interfaces, tmp_iface, node)
        {
            if (strcmp(tmp_iface->name, CONNECT_IFACE_WAN) != 0 &&
                strcmp(tmp_iface->name, CONNECT_IFACE_INTERNET) != 0 &&
                strcmp(tmp_iface->name, CONNECT_IFACE_MOBILE) != 0)
            {
                continue;
            }

            if (tmp_iface->conn_mode != IFCM_DEMAND)
            {
                continue;
            }

            connect_ifup(tmp_iface);

            iface_id = find_ifaces_stats_index(tmp_iface->name);
            if (iface_id >= 0)
            {
                ifaces_stat[iface_id].last_in_sec = time(NULL);
            }
        }
    }
    else
    {
        need_syn = true;
    }
#endif
    return;
}

static void 
connect_timebased(struct interface *iface)
{
    struct tm *cur;
    time_t cur_sec;
    int valid = 0;
    int ret = 0;
	struct timespec ts;

	ret = clock_gettime(CLOCK_REALTIME, &ts);
    if (ret != 0) {
        return;
    }

	cur = localtime(&ts.tv_sec);
	cur->tm_year += 1900;
	cur->tm_mon  += 1;

    cur_sec = cur->tm_hour * 60 * 60 + cur->tm_min * 60 + cur->tm_sec;
    if (cur_sec > iface->conn_time.start && cur_sec < iface->conn_time.end)
    {
        valid = 1;
    }

    if (!connect_state_is_down(iface) && !valid)
    {
        connect_ifdown(iface);
    }
    else if (connect_state_is_down(iface) && valid)
    {
        connect_ifup(iface);
    }
}

#ifndef FEATURE_3G4G_MODULES
static int 
connect_process()
{
    struct interface *iface, *phy_if = NULL;
    struct interface *internet = NULL;
    int i = 0;
    bool link = true;

    internet = vlist_find(&interfaces, CONNECT_IFACE_INTERNET, internet, node);
    if (internet != NULL && internet->conn_mode != IFCM_DEMAND)
    {
        connect_dft_route(0);
    }

    vlist_for_each_element(&interfaces, iface, node) 
    {
        if (strcmp(iface->name, CONNECT_IFACE_WAN) != 0 &&
            strcmp(iface->name, CONNECT_IFACE_WANV6) != 0 &&
            strcmp(iface->name, CONNECT_IFACE_INTERNET) != 0)
        {
            continue;
        }

     /*dprintf("interface(%p): %s, available: %d, connectable: %d, state: %d, conn_mode: %d\n",
                iface, iface->name, iface->available, 
                iface->connectable, iface->state, iface->conn_mode); */
	 

	/*
	** provide successful connection support for
 	** l2tp/pppoe/pptp + dhcp/staticip case.
	*/
#if	0
        if (!iface->available)
        {
            continue;
        }
#endif
	 if ((!strcmp(iface->name, CONNECT_IFACE_INTERNET) ||
		!strcmp(iface->name, CONNECT_IFACE_INTERNETV6)) &&
		!iface->main_dev.dev)
	 {
		phy_if = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);
	 }
	 
	 if (!phy_if)
	 {
		phy_if = iface;
	 }

	if (phy_if->main_dev.dev)
	{
		link = system_get_link_state(phy_if->main_dev.dev);
			device_set_link(phy_if->main_dev.dev, link);
#if !defined(QCA_IPQ50XX)
			if(!strncmp(phy_if->main_dev.dev->ifname, CONNECT_IFACE_BR_WAN, 6)) 
			{
				char cmd[32] = {0};
				sprintf(cmd, "ifconfig %s up", connect_iface_wan_ifname);
				system(cmd);
				link =  system_get_link_state_from_ifname(connect_iface_wan_ifname);
			}
#endif
	}

		/* add by wanghao */
		const char* wan_type = config_option_str(CONNECT_IFACE_WAN, "type");
        if ((!iface->connectable || !link) &&
		(iface->proto_handler && (strcmp(iface->proto_handler->name, "static") 
								|| (!strcmp(iface->name, CONNECT_IFACE_WAN) 
								&& wan_type && !strcmp(wan_type, "bridge")) ||(!strcmp(iface->name, 
								CONNECT_IFACE_WANV6) && !strcmp(iface->proto_handler->name, "static")))))//add by wanghao, if br-wan, down it.
        {
		if (iface->state == IFS_SETUP || iface->state == IFS_UP)
            		connect_ifdown(iface);

            /*
             * 只针对internet interface， 防止wan interface的状态影响internet interface
             * 进而导致到了max idle time却不断开链接
             */
            if (!strcmp(iface->name, CONNECT_IFACE_INTERNET))
            {
		last_in_sec = last_ex_sec = 0;
            }
		continue;
        }

        for (i = 0; i < ARRAY_SIZE(conn_handlers); i++)
        {
            if (iface->conn_mode == conn_handlers[i].mode)
            {
                conn_handlers[i].process(iface);
                break;
            }

            /* Invalid connect mode, ignore */
        }
    }

    return 0;
}
#else
static int 
connect_process()
{
    struct interface *iface, *phy_if = NULL;
    int i = 0;
    bool link = true;

    if (all_wans_not_ondemand())
    {
        connect_dft_route(0);
    }

    vlist_for_each_element(&interfaces, iface, node) 
    {
        phy_if = NULL;
        if (strcmp(iface->name, CONNECT_IFACE_WAN) != 0 &&
            strcmp(iface->name, CONNECT_IFACE_WANV6) != 0 &&
            strcmp(iface->name, CONNECT_IFACE_INTERNET) != 0 &&
            strcmp(iface->name, CONNECT_IFACE_MOBILE) != 0)
        {
            continue;
        }

	if (strcmp(iface->name, CONNECT_IFACE_MOBILE) == 0)
	{
	     //system("echo [NETIFD]mobile connect_process >/dev/console");
	}

        /* dprintf("interface(%p): %s, available: %d, connectable: %d, state: %d, conn_mode: %d\n",
                iface, iface->name, iface->available, 
                iface->connectable, iface->state, iface->conn_mode); */

    /*
    ** provide successful connection support for
    ** l2tp/pppoe/pptp + dhcp/staticip case.
    */
#if	0
        if (!iface->available)
        {
            continue;
        }
#endif
        if ((i = find_ifaces_stats_index(iface->name)) < 0)
        {
            i = ifaces_stat_len;
            strcpy(ifaces_stat[i].ifname, iface->name);
            ifaces_stat_len++;
        }

        if ((!strcmp(iface->name, CONNECT_IFACE_INTERNET) ||
            !strcmp(iface->name, CONNECT_IFACE_INTERNETV6)) &&
            !iface->main_dev.dev)
        {
            phy_if = vlist_find(&interfaces, CONNECT_IFACE_WAN, iface, node);
        }
        if (!phy_if)
        {
            phy_if = iface;
        }

        if (strcmp(iface->name, CONNECT_IFACE_MOBILE) == 0 && iface->l3_dev.dev)
        {
            link = system_get_34g_link_state(iface->l3_dev.dev);
            device_set_link(iface->l3_dev.dev, link);
        }
        else if (phy_if->main_dev.dev)
        {
            if(phy_if->main_dev.dev->external)
            {
                link = phy_if->main_dev.dev->link_active;
            }
            else
            {
                link = system_get_link_state(phy_if->main_dev.dev);
                        device_set_link(phy_if->main_dev.dev, link);
#if !defined(QCA_IPQ50XX)
                if(!strncmp(phy_if->main_dev.dev->ifname, CONNECT_IFACE_BR_WAN, 6)) 
                {
                    link =  system_get_link_state_from_ifname(connect_iface_wan_ifname);
                }
#endif
            }
        }

        if ((!iface->connectable || !link) &&
            (iface->proto_handler && strcmp(iface->proto_handler->name, "static")))
        {
            if ((iface->state == IFS_SETUP && strcmp(iface->name, CONNECT_IFACE_MOBILE) != 0) || iface->state == IFS_UP)
                connect_ifdown(iface);

            if (strcmp(iface->name, CONNECT_IFACE_MOBILE) != 0 ||
                !iface->connectable ||
                (iface->conn_mode != IFCM_DEMAND && iface->conn_mode != IFCM_AUTO))
            {
                ifaces_stat[i].last_in_sec = ifaces_stat[i].last_ex_sec = 0;
                continue;
            }
        }

        for (i = 0; i < ARRAY_SIZE(conn_handlers); i++)
        {
            if (iface->conn_mode == conn_handlers[i].mode)
            {
                conn_handlers[i].process(iface);
                break;
            }

            /* Invalid connect mode, ignore */
        }
    }

    return 0;
}
#endif

static void 
connect_timeout(struct uloop_timeout *timeout)
{
#ifdef FEATURE_3G4G_MODULES
    get_tx_info = false;
#endif
    (void)connect_process();
    (void)uloop_timeout_set(timeout, CONNECT_TIMER_PERIOD);
}

#if 0
static int
connect_dev_hasip(const char *ifname)
{
    uint32_t ip = 0;
    int ret = 0;

    ret = system_if_get_addr(ifname, &ip);
    if (ret != 0 || ip == 0)
    {
        return 0;
    }

    return 1;
}
#endif

static void
connect_dft_route(int add)
{
    const char *dftgw = "1.0.0.1";
    const char *wan_ifname;
    const char *lan_ifname;
    const char *lan_ipaddr;
    const char* wan_type = NULL;
    char *p = NULL;
    char *buff = NULL;
    char *str = NULL;
    char lan_ifname_first[16] = {0};
    uint8_t macaddr[CONNECT_MACADDR_LEN] = {0};
    int ret = 0;
    size_t len = 0;

    wan_type = config_option_str(CONNECT_IFACE_WAN, "type");

    if((wan_type != NULL) && (0 == strcmp(wan_type, "bridge")))
    {
        wan_ifname = CONNECT_IFACE_BR_WAN;
    }
    else
    {
        wan_ifname = config_option_str(CONNECT_IFACE_WAN, "ifname");
    }

    if (!wan_ifname) 
    {
        dprintf("interface %s: invalid option ifname\n", CONNECT_IFACE_WAN);
        return;
    }

    if (add)
    {
#if 0
        if (tmp_ip_set || connect_dev_hasip(wan_ifname))
        {
            /* tmp ip addr for wan's device is set already, or it has ip */
            return;
        }
#endif

        lan_ifname = config_option_str(CONNECT_IFACE_LAN, "ifname");
        if (!lan_ifname)
        {
            dprintf("interface %s: invalid option ifname\n", CONNECT_IFACE_LAN);
            return;
        }

        len = strlen(lan_ifname);
        buff = malloc(len + 1);
        if (buff == NULL)
        {
            dprintf("out of memory\n");
            return;
        }
        memset(buff, 0, len + 1);
        strncpy(buff, lan_ifname, len);
        str = buff;

        while(1)
        {
            p = strsep(&str, " ");
            if (p == NULL)
                break;

            len = strlen(p);
            if (len > 0 && len < 16)
            {
                strncpy(lan_ifname_first, p, len);
                break;
            }
            else if (len >= 16)
            {
                free(buff);
                dprintf("interface %s: invalid option ifname, too long\n", CONNECT_IFACE_LAN);
                return;
            }
        }

        if (buff)
            free(buff);

        lan_ipaddr = config_option_str(CONNECT_IFACE_LAN, "ipaddr");
        if (!lan_ipaddr)
        {
            dprintf("interface %s: invalid option ipaddr\n", CONNECT_IFNAME_LAN);
            return;
        }

        ret = system_if_get_macaddr(lan_ifname_first, macaddr);
        if (ret != 0)
        {
            dprintf("ifname %s: can't get macaddr\n", lan_ifname_first);
            return;
        }

        /* system_exec_fmt("ifconfig %s 0.0.0.0", wan_ifname); */
        memset(tmp_ip_str, 0, sizeof(tmp_ip_str));
        if ((macaddr[CONNECT_MACADDR_LEN - 1] != 0x01) &&
            (macaddr[CONNECT_MACADDR_LEN - 1] != 0xFF))
        {
            snprintf(tmp_ip_str, sizeof(tmp_ip_str), "1.0.%d.%d", 
                     macaddr[CONNECT_MACADDR_LEN - 2], 
                     macaddr[CONNECT_MACADDR_LEN - 1]);
        }
        else
        {
            snprintf(tmp_ip_str, sizeof(tmp_ip_str), "1.0.%d.10", 
                     macaddr[CONNECT_MACADDR_LEN - 2]);
        }

        /* system_exec_fmt("ifconfig %s %s", wan_ifname, tmp_ip_str); */
        system_exec_fmt("ip addr add %s/8 dev %s", tmp_ip_str, wan_ifname);
        system_exec_fmt("ip route replace default via %s dev %s", dftgw, wan_ifname);
        system_exec_fmt("ifconfig %s up", wan_ifname);
        system_exec_fmt("nat del dns && nat add dns { tcp %s %s } && nat add dns { udp %s %s }", lan_ipaddr, dftgw, lan_ipaddr, dftgw);
        tmp_ip_set = 1;
    }
    else 
    {
        if (tmp_ip_set)
        {
            /* delete tmp ip and default route */
            system_exec_fmt("ip addr del %s/8 dev %s", tmp_ip_str, wan_ifname);
            /* dnsproxy will execute nat del dns */
            /* system_exec_cmd("nat del dns"); */
            tmp_ip_set = 0;
        }
    }
}
#ifndef FEATURE_3G4G_MODULES
static int 
connect_ipt_stat(struct ipt_stat *stat)
{
    socklen_t len = sizeof(struct ipt_stat);
    int sock = -1;
    int ret = 0;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0)
    {
        dprintf("create socket failed\n");
        ret = -1;
        goto out;
    }

    ret = getsockopt(sock, IPPROTO_IP, IPT_STAT_GET_WAN_STAT, stat, &len);
    if (ret != 0)
    {
        dprintf("setsockopt failed to get stats\n");
        goto out;
    }
    else if (len != sizeof(struct ipt_stat))
    {
        dprintf("len %d != %lu\n", len, (unsigned long)sizeof(struct ipt_stat));
        ret = -1;
        goto out;
    }

    ret = 0;

out:
    if (sock > -1)
    {
        close(sock);
    }
    return ret;
}
#else
static int 
connect_ipt_stat()
{
    socklen_t len = sizeof(wans_tx_data);
    int sock = -1;
    int ret = 0;

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0)
    {
        dprintf("create socket failed\n");
        ret = -1;
        goto out;
    }

    memset(&wans_tx_data, 0, len);
    ret = getsockopt(sock, IPPROTO_IP, IPT_STAT_GET_WANS_TX_STAT, &wans_tx_data, &len);
    if (ret != 0)
    {
        dprintf("setsockopt failed to get stats\n");
        goto out;
    }
    else if (len != sizeof(wans_tx_data))
    {
        dprintf("len %d != %lu\n", len, (unsigned long)sizeof(wans_tx_data));
        ret = -1;
        goto out;
    }

    ret = 0;

out:
    if (sock > -1)
    {
        close(sock);
    }
    return ret;
}
#endif

#ifndef FEATURE_3G4G_MODULES
static int
connect_ipt_outgone()
{
    struct ipt_stat ipts;
    int ret = 0;

    ret = connect_ipt_stat(&ipts);
    if (ret != 0)
    {
        return 0;
    }

    dprintf("old/new send pkts: %d, %d\n", last_stat.snd_pkt, ipts.snd_pkt);
    if (ipts.snd_pkt != 0 && ipts.snd_pkt > last_stat.snd_pkt)
    {
        ret = 1;
    }
    memcpy(&last_stat, &ipts, sizeof(struct ipt_stat));

    return ret;
}
#else
static int
connect_ipt_outgone(struct interface *iface)
{
    int ret = 0;

    if (!get_tx_info)
    {
        ret = connect_ipt_stat();
        if (ret != 0)
        {
            return 0;
        }
        get_tx_info = true;
    }

    ret = wan_tx_stat_increase(iface);

    return ret;
}
#endif

static int
connect_cfg_ip4addr(const char *secname, const char *name, uint32_t *ip4addr)
{
    const char *val = NULL;
    struct in_addr addr;
    int ret = 0;

    val = config_option_str(secname, name);
    if (!val) 
    {
        dprintf("interface lan doesn't exist ipaddr\n");
        ret = -1;
        goto out;
    }

    ret = inet_aton(val, &addr);
    if (ret == 0)
    {
        ret = -1;
        dprintf("Invalid lan ipaddr\n");
        goto out;
    }

    *ip4addr = ntohl(addr.s_addr);
    ret = 0;

out:
    return ret;
}

static int 
connect_ipt_init()
{
    struct ipt_net ipnet;
    int sock = -1;
    int ret = 0;

#if 0
    ret = system_if_get_addr(CONNECT_IFNAME_LAN, &ipnet.ip);
    if (ret != 0)
    {
        dprintf("system_if_get_addr(lan) failed\n");
        goto out;
    }

    ret = system_if_get_mask(CONNECT_IFNAME_LAN, &ipnet.mask);
    if (ret != 0)
    {
        dprintf("system_if_get_mask(lan) failed\n");
        goto out;
    }

#else
    ret = connect_cfg_ip4addr(CONNECT_IFACE_LAN, "ipaddr", &ipnet.ip);
    if (ret != 0)
    {
        goto out;
    }

    ret = connect_cfg_ip4addr(CONNECT_IFACE_LAN, "netmask", &ipnet.mask);
    if (ret != 0)
    {
        goto out;
    }
#endif

    sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) 
    {
        dprintf("create socket failed\n");
        ret = -1;
        goto out;
    }

    ret = setsockopt(sock, IPPROTO_IP, IPT_STAT_DEL_ALL, NULL, 0);
    if (ret != 0)
    {
        dprintf("setsockopt stats del all failed");
        goto out;
    }

    ret = setsockopt(sock, IPPROTO_IP, IPT_STAT_SET_NET, &ipnet, sizeof(ipnet));
    if (ret != 0)
    {
        dprintf("setsockopt stats set net failed");
        goto out;
    }

    ret = 0;

out:
	system("ubus call tfstats reset");
    if (sock > -1)
    {
        close(sock);
    }
    return ret;
}

void connect_dump_info(struct blob_buf *b)
{
    blobmsg_add_u32(b, "timeout remaining", uloop_timeout_remaining(&timeout));
}

int connect_init()
{
    int ret = 0;

    if (initialized)
    {
        return ret;
    }
#ifdef FEATURE_3G4G_MODULES
    memset(ifaces_stat, 0, sizeof(struct iface_stat) * MAX_CONNECT_IFACE_NUM);
    ifaces_stat_len = 0;
#endif
    ret = connect_ipt_init();
    if (ret != 0)
    {
        dprintf("connect_ipt_init failed\n");
        return ret;
    }

	/* add by wanghao */
	FILE	*fp;
	char fpLine[128] = {0};
	char *head = NULL;
	char *tail = NULL;

	if ((fp = fopen("/etc/profile.d/profile", "r")) != NULL)
	{
		while (1)
		{
			fgets(fpLine, sizeof(fpLine) - 1, fp);
			if (feof(fp))
			{
				break;
			}
			
			if ((head = strstr(fpLine, "wan_ifname")) != NULL)
			{
				head += strlen("wan_ifname '");
				tail = strstr(head, "'");
				strncpy(connect_iface_wan_ifname, head, tail - head);
				connect_iface_wan_ifname[tail - head] = '\0';
				break;
			}
		}
		fclose(fp);
	}
	/* add end */

    timeout.cb = connect_timeout;
    ret = uloop_timeout_set(&timeout, CONNECT_TIMER_PERIOD);

#if FEATURE_3G4G_MODULES
    detect_init_exit(true);
#endif

    initialized = 1;
    return ret;
}

void connect_exit()
{
    if (initialized)
    {
#if FEATURE_3G4G_MODULES
	detect_init_exit(false);
#endif
        uloop_timeout_cancel(&timeout);
        initialized = 0;
    }
}
