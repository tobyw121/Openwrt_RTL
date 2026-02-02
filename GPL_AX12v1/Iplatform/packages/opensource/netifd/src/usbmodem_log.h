/*! Copyright(c) 2014 Shenzhen TP-LINK Technologies Co.Ltd.
 * File    : usbmodem_log.h
 * Author  : Liu Hao <liuhao@tp-link.net>
 * Detail  : Utility for modem handle
 * Version : 1.0
 * Date    : 18 Nov, 2014
 */

#ifndef _USBMODEM_LOG_H
#define _USBMODEM_LOG_H

#define MODEM_DEBUG  1

/*************************************************************************/
/*                             include                                   */
/*************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*************************************************************************/
/*                             defines                                   */
/*************************************************************************/

#define USBMODEM_ID 292

#define START_ADD_CARD 10
#define START_SEARCH_TABLE 11
#define FIND_IN_MODEMTABLE 12
#define INSMOD_MODULES 13
#define FIND_TTY_CPORT 14
#define FIND_TTY_DPORT 15
#define GET_IFNAME 16
#define PARSE_INF_Y 17
#define PARSE_INF_N 18
#define LOG_BK_SWITCH_NW 36
#define LOG_BK_NETWORK_ON 37
#define LOG_CONN_MODE_DEMAND 38
#define LOG_CONN_MODE_DEMAND_TIMEOUT 39
#define LOG_CONN_MODE_MANUALLY_TIMEOUT 40
#define LOG_BK_UNSET_NW 41
#define LOG_BK_NETWORK_OFF 42

#define USBMODEM_INFO(msg_id, args...) while(0) {}
#define USBMODEM_DBG(msg_id, args...) while(0) {}

#ifdef MODEM_DEBUG
	#define MODEM_LOG(fmt, arg...)  \
        printf("%s->%d:"fmt, __FUNCTION__, __LINE__, ##arg)
#else
	#define MODEM_LOG(fmt, arg...) \
		printf("%s->%d:"fmt, __FUNCTION__, __LINE__, ##arg)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
