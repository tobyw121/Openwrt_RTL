#ifndef _RTW_BAND_STEERING_H_
#define _RTW_BAND_STEERING_H_

#include "8192cd_cfg.h"

#ifdef CONFIG_RTK_BAND_STEERING

#define B_STEER_ENTRY_NUM					64

#define B_STEER_BLOCK_ENTRY_EXPIRE			60		// 120 s
#define B_STEER_ROAM_BLOCK_ENTRY_EXPIRE		5		// 10 s

struct b_steer_block_entry {
	u8 used;
	u8 mac[6];
	u32 entry_expire;
};

struct b_steer_priv {
	struct b_steer_block_entry block_list[B_STEER_ENTRY_NUM];
	spinlock_t lock;
};

#endif

#endif

