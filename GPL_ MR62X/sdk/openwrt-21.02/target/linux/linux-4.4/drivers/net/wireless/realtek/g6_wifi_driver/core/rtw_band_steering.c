#define _RTW_BAND_STEERING_C_

#include <drv_types.h>

#ifdef CONFIG_BAND_STEERING

static struct b_steer_block_entry *block_entry_lookup(
	_adapter *adapter, u8 *mac)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;

	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(adapter->bsteerpriv.block_list[i]);
		if (ent->used && _rtw_memcmp(ent->mac, mac, 6))
			return ent;
	}

	return NULL;
}

/* ----------------------------------------- */
/* ---------- APIs for core layer ---------- */
/* ----------------------------------------- */
void _band_steering_expire(_adapter *adapter)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;

	if (adapter->bsteerpriv.inited == _FALSE)
		return;

	_rtw_spinlock_bh(&(adapter->bsteerpriv.lock));

	/* block entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(adapter->bsteerpriv.block_list[i]);
		if (!ent->used)
			continue;

		if (ent->entry_expire) {
			ent->entry_expire--;
			if (ent->entry_expire == 0)
				ent->used = 0;
		}
	}

	_rtw_spinunlock_bh(&(adapter->bsteerpriv.lock));

	return;
}

s32 _band_steering_block_chk(_adapter *adapter, u8 *mac)
{
	s32 ret = 0;
	struct b_steer_block_entry *ent = NULL;

	if (adapter->bsteerpriv.inited == _FALSE)
		return 0;

	_rtw_spinlock_bh(&(adapter->bsteerpriv.lock));

	ent = block_entry_lookup(adapter, mac);
	if (ent)
		ret = 1;

	_rtw_spinunlock_bh(&(adapter->bsteerpriv.lock));

	return ret;
}

void _band_steering_block_entry_add(_adapter *adapter, u8 *mac)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;

	if (adapter->bsteerpriv.inited == _FALSE)
		return;

	_rtw_spinlock_bh(&(adapter->bsteerpriv.lock));

	ent = block_entry_lookup(adapter, mac);

	/* already exist */
	if (ent) {
		ent->entry_expire = B_STEER_BLOCK_ENTRY_EXPIRE;
		goto func_return;
	}

	/* find an empty entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		if (!adapter->bsteerpriv.block_list[i].used) {
			ent = &(adapter->bsteerpriv.block_list[i]);
			break;
		}
	}

	/* add the entry */
	if (ent) {
		ent->used = 1;
		_rtw_memcpy(ent->mac, mac, 6);
		ent->entry_expire = B_STEER_BLOCK_ENTRY_EXPIRE;
	}

func_return:
	_rtw_spinunlock_bh(&(adapter->bsteerpriv.lock));

	return;
}

void _band_steering_block_entry_del(_adapter *adapter, u8 *mac)
{
	struct b_steer_block_entry *ent = NULL;

	if (adapter->bsteerpriv.inited == _FALSE)
		return;

	_rtw_spinlock_bh(&(adapter->bsteerpriv.lock));

	ent = block_entry_lookup(adapter, mac);
	if (ent)
		ent->used = 0;

	_rtw_spinunlock_bh(&(adapter->bsteerpriv.lock));

	return;
}

void _band_steering_roam_block_entry_add(_adapter *adapter, u8 *mac)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;

	if (adapter->bsteerpriv.inited == _FALSE)
		return;

	_rtw_spinlock_bh(&(adapter->bsteerpriv.lock));

	ent = block_entry_lookup(adapter, mac);

	/* already exist */
	if (ent)
		goto func_return;

	/* find an empty entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		if (!adapter->bsteerpriv.block_list[i].used) {
			ent = &(adapter->bsteerpriv.block_list[i]);
			break;
		}
	}

	/* add the entry */
	if (ent) {
		ent->used = 1;
		_rtw_memcpy(ent->mac, mac, 6);
		ent->entry_expire = B_STEER_ROAM_BLOCK_ENTRY_EXPIRE;
	}

func_return:
	_rtw_spinunlock_bh(&(adapter->bsteerpriv.lock));

	return;
}

void _band_steering_init(_adapter *adapter)
{
	u8 i;
	struct b_steer_block_entry *ent = NULL;

	_rtw_spinlock_init(&(adapter->bsteerpriv.lock));

	adapter->bsteerpriv.inited = _TRUE;

	_rtw_spinlock_bh(&(adapter->bsteerpriv.lock));

	/* block entry */
	for (i = 0; i < B_STEER_ENTRY_NUM; i++) {
		ent = &(adapter->bsteerpriv.block_list[i]);
		ent->used = 0;
	}

	_rtw_spinunlock_bh(&(adapter->bsteerpriv.lock));

	return;
}

#endif

