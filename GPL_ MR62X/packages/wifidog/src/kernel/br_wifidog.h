/*! Copyright(c) 2008-2020 Shenzhen TP-LINK Technologies Co.Ltd.
 *
 *\file     br_wifidog.h
 *\brief    common file.   
 *\details  
 *
 *\author   Ma Rujun
 *\version  1.0.0
 *\date     2020/10/21
 *
 *\warning  
 *
 *\history \arg 1.0.0, 2020/10/21, Ma Rujun, Create the file.
 */


/**************************************************************************************************/
/*                                      CONFIGURATIONS                                            */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      INCLUDE_FILES                                             */
/**************************************************************************************************/
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/version.h> 
#include <linux/debugfs.h>
#include <linux/etherdevice.h>
#include <linux/netfilter_bridge.h>
#include <linux/inetdevice.h>
#include <linux/netdevice.h>
#include <linux/proc_fs.h>
#include <linux/if_ether.h>
#include <net/tcp.h>
#include <asm-generic/int-ll64.h>

/**************************************************************************************************/
/*                                      DEFINES                                                   */
/**************************************************************************************************/

/*
 *	protocol
 */
#define IP_PROTO_TCP                			(6)
#define IP_PROTO_UDP                			(17)
/*
 *	protocol port
 */
#define TCP_HTTP_PORT               			(80)
#define TCP_HTTPS_PORT							(443)
#define DNS_PORT								(0x35)
#define DHCP_PORT								(0x43)

#define ETH_LEN                     			(14)
#define VLAN_ETH_LEN                			(18)

#define PORTAL_DFT_VLAN            				(4095)


/*
 *  portal config
 */
#define WIFIDOG_INTERFACE_MARK					(0x20000)
#define WIFIDOG_INTERFACE_MASK					(0x30000)
#define MAX_CLIENT								(32 * 8)
#define MAX_CONTROL_INTF						(8)
#define MAX_INTERFACE_LIST                      (8)
#define NF_PROCFS_PERM                          (0445)
#define FILTER_PROCFS_DIR                       "portal_filter"
#define PORTAL_INTERFACE_LIST                   "portal_interface_list"


/*
 * 	http redirect
 */
#define PORTAL_USE_JS                       	(1)
#define HTTP_CONTENT                            "http://%d.%d.%d.%d:2060/wifidog"
#define PORTAL_URL_LEN							(100)

/*
 *	 local bridge (for wifidog Auth http client)
 */
#define LOCAL_BRIDGE_NAME						"br-lan"

/*
 *  portal ctl
 */
#define BR_WIFIDOG_BASE_CTL						(266)

#define BR_WIFIDOG_ADD_MAC						(BR_WIFIDOG_BASE_CTL)
#define BR_WIFIDOG_DEL_MAC						(BR_WIFIDOG_ADD_MAC + 1)
#define BR_WIFIDOG_SET_MAX						(BR_WIFIDOG_DEL_MAC)

#define BR_WIFIDOG_GET_MAC						(BR_WIFIDOG_BASE_CTL)
#define BR_WIFIDOG_GET_MAX						(BR_WIFIDOG_BASE_CTL)




/*
 *  common function
 */
#ifndef NIPQUAD
#define NIPQUAD(addr) \
         ((unsigned char *)&addr)[0], \
         ((unsigned char *)&addr)[1], \
         ((unsigned char *)&addr)[2], \
         ((unsigned char *)&addr)[3]
#endif

#define GET_TCP_LEN(pTcpHdr)        ((ntohl(tcp_flag_word((pTcpHdr))) & 0xf0000000)>>26)
#define GET_TCP_FLAG(pTcpHdr)       (((ntohl(tcp_flag_word((pTcpHdr))) & 0x0fff0000)<<4)>>20)
#define IS_TCP_FLAG_SYN(tcpFlag)    ((tcpFlag) == 0x0002)
#define IS_TCP_FLAG_ACK(tcpFlag)    ((tcpFlag) == 0x010)
#define IS_TCP_FLAG_FIN(tcpFlag)    ((tcpFlag) & 0x001)

#define PORTAL_GET_VLAN_ID(tci)    ((ntohs(tci)) & 0x0fff)

/*
 *	portal debug
 */
#define PORTAL_ERROR(error,info,args...) \
	do {\
		if (error)\
		{\
			printk("<portal>[error]%s(): %d  -> "info"\r\n",__FUNCTION__,__LINE__,##args);\
		}\
	} while(0)

#define DEBUG_ENABLE 0

#if DEBUG_ENABLE

#define PORTAL_DEBUG(debug,info,args...) \
	do {\
		if (debug)\
		{\
			printk("<portal>[debug]%s(): %d  -> "info"\r\n",__FUNCTION__,__LINE__,##args);\
		}\
	} while(0)
	
#else

	#define PORTAL_DEBUG(debug,info,args...)

#endif





/**************************************************************************************************/
/*                                      TYPES                                                     */
/**************************************************************************************************/
struct allow_entry {
	struct list_head list;
	unsigned char used;
	unsigned char src_mac[ETH_ALEN];
};

struct controlled_entry {
	struct list_head list;
	struct net_device *dev;
};

typedef struct _TCP_TIME_STAMP{
	unsigned char type;
	unsigned char len;
	unsigned int timestamp1;
	unsigned int timestamp2;
}TCP_TIME_STAMP;



/**************************************************************************************************/
/*                                      VARIABLES                                                 */
/**************************************************************************************************/

/**************************************************************************************************/
/*                                      FUNCTIONS                                                 */
/**************************************************************************************************/