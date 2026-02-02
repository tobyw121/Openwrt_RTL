#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include "os.h"
#include "driver_def.h"

/*
if INITIATIVE_DUAL_BAND_DETECT is defined, wlan manager will trigger
sta to send probe request, instead of listening passively
*/
#ifdef CONFIG_BAND_STEERING
#define INITIATIVE_DUAL_BAND_DETECT
#endif

/* GENERIC cross modules */

typedef unsigned char	u8;
typedef uint16_t		u16;
typedef uint32_t		u32;
typedef uint64_t		u64;
typedef char			s8;
typedef int16_t			s16;
typedef int32_t			s32;

/* WKARD: TBD */
#define PROC_NB_REPORT_FMT		"/proc/net/rtl8852ae/%s/nb_report"
#define BSS_TM_REQ_PARAMS_FMT	\
		"valid_int=%u pref=%u disassoc_imminent=%u disassoc_timer=%u abridged=%u "	\
		"neighbor=%02x:%02x:%02x:%02x:%02x:%02x,%u,%u,%u,%u"

#define MAC_FMT			"%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(x)		((u8 *)(x))[0], ((u8 *)(x))[1], \
						((u8 *)(x))[2], ((u8 *)(x))[3], \
						((u8 *)(x))[4], ((u8 *)(x))[5]

#define INTF_NUM			2
#define BAND_NUM			INTF_NUM
#define SSID_NUM			5
#define MAX_STA_NUM			64
#define HASH_TBL_SIZE		64
#define BAND_NAME(x)		((x == BAND_ON_5G) ? "5G" : "24G")

extern u8 global_dbg_level;

struct com_nb_rpt_hdr {
	u8  id;
	u8  len;
	u8  bssid[6];
	u32 bss_info;
	u8  reg_class;
	u8  ch_num;
	u8  phy_type;
	u8  preference;
	u8  enable;
};

struct com_wifi_list {
    struct com_wifi_list *next;
    struct com_wifi_list **pprev;
};

struct com_sta {
	u8  used;
	u8  mac[6];
	u8  rssi;
	u32 link_time;
	u32 tx_tp;
	u32 rx_tp;
	u8  status;
	u8  is_dual_band;

#ifdef CONFIG_BAND_STEERING
	u8  b_steer_roam_cnt;
	u8  b_steer_roam_detect;
#ifdef INITIATIVE_DUAL_BAND_DETECT
	u8  b_steer_bss_tm_req_cnt;
#endif
#endif

	struct com_wifi_list list;
};

struct com_frame {
	u8  used;
	u16 frame_type;
	u8  sa[6];
	u32 aging;

	struct com_wifi_list list;
};

struct com_priv {
	u8  active;
	u8  mac[6];
	u8  root;
	u8  band;
	u8  ssid;
	s8  name[16];
	u16 ch;
	u8  ch_clm;
	u8  ch_noise;
	u32 tx_tp;
	u32 rx_tp;
	u8  sta_num;
	struct com_sta sta_list[MAX_STA_NUM];
	struct com_wifi_list *sta_hash_tbl[HASH_TBL_SIZE];

	struct com_frame frame_db[MAX_STA_NUM];
	struct com_wifi_list *frame_hash_tbl[HASH_TBL_SIZE];

	struct com_nb_rpt_hdr self_nb_rpt;

#ifdef CONFIG_BAND_STEERING
	u8 band_steering_enable;
	struct com_priv *grp_priv;
#endif
};

struct com_device {
	struct com_priv priv[BAND_NUM][SSID_NUM];
	s8 config_fname[64];
};

__inline static u32 wifi_mac_hash(const u8 *mac)
{
	u32 x;

	x = mac[0];
	x = (x << 2) ^ mac[1];
	x = (x << 2) ^ mac[2];
	x = (x << 2) ^ mac[3];
	x = (x << 2) ^ mac[4];
	x = (x << 2) ^ mac[5];

	x ^= x >> 8;
	x  = x & (HASH_TBL_SIZE - 1);

	return x;
}

__inline__ static void wifi_list_link(
	struct com_wifi_list *link, struct com_wifi_list **head)
{
	link->next = *head;

	if (link->next != NULL)
		link->next->pprev = &link->next;

	*head = link;

	link->pprev = head;
}

__inline__ static void wifi_list_unlink(struct com_wifi_list *link)
{
	*(link->pprev) = link->next;

	if (link->next != NULL)
		link->next->pprev = link->pprev;

	link->next = NULL;
	link->pprev = NULL;
}

enum {
	_MSG_PRINT_ = 0,
	_MSG_ERR_ = 1,
	_MSG_WARN_ = 2,
	_MSG_INFO_ = 3,
	_MSG_MAX_ = 4
};

__inline__ static void MSG_PRINT(s8 *prefix, s8 *fmt, ...)
{
	va_list va;

	if (global_dbg_level >= _MSG_PRINT_) {
		va_start(va, fmt);
		printf("%s ", prefix);
		vprintf(fmt, va);
		printf("\n");
		va_end(va);
	}
}

__inline__ static void MSG_ERR(s8 *prefix, s8 *fmt, ...)
{
	va_list va;

	if (global_dbg_level >= _MSG_ERR_) {
		va_start(va, fmt);
		printf("%s ERROR: ", prefix);
		vprintf(fmt, va);
		printf("\n");
		va_end(va);
	}
}

__inline__ static void MSG_WARN(s8 *prefix, s8 *fmt, ...)
{
	va_list va;

	if (global_dbg_level >= _MSG_WARN_) {
		va_start(va, fmt);
		printf("%s WARN: ", prefix);
		vprintf(fmt, va);
		printf("\n");
		va_end(va);
	}
}

__inline__ static void MSG_INFO(s8 *prefix, s8 *fmt, ...)
{
	va_list va;

	if (global_dbg_level >= _MSG_INFO_) {
		va_start(va, fmt);
		printf("%s ", prefix);
		vprintf(fmt, va);
		printf("\n");
		va_end(va);
	}
}

#endif

