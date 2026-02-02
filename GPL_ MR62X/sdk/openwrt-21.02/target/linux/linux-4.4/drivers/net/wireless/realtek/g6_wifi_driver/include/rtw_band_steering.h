#ifndef _RTW_BAND_STEERING_H_
#define _RTW_BAND_STEERING_H_

#ifdef CONFIG_BAND_STEERING

#define B_STEER_ENTRY_NUM					64

#define B_STEER_BLOCK_ENTRY_EXPIRE			60
#define B_STEER_ROAM_BLOCK_ENTRY_EXPIRE		5

struct b_steer_block_entry {
	u8 used;
	u8 mac[6];
	u32 entry_expire;
};

struct b_steer_priv {
	struct b_steer_block_entry block_list[B_STEER_ENTRY_NUM];
	_lock lock;
	bool inited;
};


/* ----------------------------------------- */
/* ---------- APIs for core layer ---------- */
/* ----------------------------------------- */
void _band_steering_expire(_adapter *adapter);
s32  _band_steering_block_chk(_adapter *adapter, u8 *mac);
void _band_steering_block_entry_add(_adapter *adapter, u8 *mac);
void _band_steering_block_entry_del(_adapter *adapter, u8 *mac);
void _band_steering_roam_block_entry_add(_adapter *adapter, u8 *mac);
void _band_steering_init(_adapter *adapter);

#endif

#endif

