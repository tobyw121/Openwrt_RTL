/******************************************************************************
 *
 * Copyright(c)  2019 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/
#define _OSDEP_SERVICE_FREEBSD_C_

#include <drv_types.h>

inline int RTW_STATUS_CODE(int error_code)
{
	return error_code;
}


/* review again */
struct sk_buff *dev_alloc_skb(unsigned int size)
{
	struct sk_buff *skb = NULL;
	u8 *data = NULL;

	/* skb = _rtw_zmalloc(sizeof(struct sk_buff)); */ /* for skb->len, etc. */
	skb = _rtw_malloc(sizeof(struct sk_buff));
	if (!skb)
		goto out;
	data = _rtw_malloc(size);
	if (!data)
		goto nodata;

	skb->head = (unsigned char *)data;
	skb->data = (unsigned char *)data;
	skb->tail = (unsigned char *)data;
	skb->end = (unsigned char *)data + size;
	skb->len = 0;
	/* printf("%s()-%d: skb=%p, skb->head = %p\n", __FUNCTION__, __LINE__, skb, skb->head); */

out:
	return skb;
nodata:
	_rtw_mfree(skb, sizeof(struct sk_buff));
	skb = NULL;
	goto out;

}

void dev_kfree_skb_any(struct sk_buff *skb)
{
	/* printf("%s()-%d: skb->head = %p\n", __FUNCTION__, __LINE__, skb->head); */
	if (skb->head)
		_rtw_mfree(skb->head, 0);
	/* printf("%s()-%d: skb = %p\n", __FUNCTION__, __LINE__, skb); */
	if (skb)
		_rtw_mfree(skb, 0);
}
struct sk_buff *skb_clone(const struct sk_buff *skb)
{
	return NULL;
}

inline struct sk_buff *_rtw_skb_alloc(u32 sz)
{
	return dev_alloc_skb(sz);

}

inline void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}


inline struct sk_buff *_rtw_pskb_copy(struct sk_buff *skb)
{
	return NULL;
}
void _rtw_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		_rtw_skb_free(skb);
}

void _rtw_memcpy(void *dst, const void *src, u32 sz)
{
	memcpy(dst, src, sz);
}

inline void _rtw_memmove(void *dst, const void *src, u32 sz)
{
	#error "TBD\n"
}

int _rtw_memcmp(const void *dst, const void *src, u32 sz)
{
	/* under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0 */

	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
}

void _rtw_memset(void *pbuf, int c, u32 sz)
{
	memset(pbuf, c, sz);
}

static inline void __list_add(_list *pnew, _list *pprev, _list *pnext)
{
	pnext->prev = pnew;
	pnew->next = pnext;
	pnew->prev = pprev;
	pprev->next = pnew;
}



void _rtw_init_listhead(_list *list)
{
	list->next = list;
	list->prev = list;
}


/*
For the following list_xxx operations,
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32 rtw_is_list_empty(_list *phead)
{

	if (phead->next == phead)
		return _TRUE;
	else
		return _FALSE;
}

void rtw_list_insert_head(_list *plist, _list *phead)
{
	__list_add(plist, phead, phead->next);
}

void rtw_list_insert_tail(_list *plist, _list *phead)
{
	__list_add(plist, phead->prev, phead);
}

inline void rtw_list_splice(_list *list, _list *head)
{
	#error "TBD\n"
}

inline void rtw_list_splice_init(_list *list, _list *head)
{
	#error "TBD\n"
}

inline void rtw_list_splice_tail(_list *list, _list *head)
{
	#error "TBD\n"
}

inline void rtw_hlist_head_init(rtw_hlist_head *h)
{
	#error "TBD\n"
}

inline void rtw_hlist_add_head(rtw_hlist_node *n, rtw_hlist_head *h)
{
	#error "TBD\n"
}

inline void rtw_hlist_del(rtw_hlist_node *n)
{
	#error "TBD\n"
}

inline void rtw_hlist_add_head_rcu(rtw_hlist_node *n, rtw_hlist_head *h)
{
	#error "TBD\n"
}

inline void rtw_hlist_del_rcu(rtw_hlist_node *n)
{
	#error "TBD\n"
}

void rtw_init_timer(_timer *ptimer, void *pfunc, void *ctx)
{
	_init_timer(ptimer, pfunc, ctx);
}

/*

Caller must check if the list is empty before calling rtw_list_delete

*/


extern _adapter * prtw_lock;

void rtw_mtx_lock(_lock *plock)
{
	if (prtw_lock)
		mtx_lock(&prtw_lock->glock);
	else
		printf("%s prtw_lock==NULL", __FUNCTION__);
}
void rtw_mtx_unlock(_lock *plock)
{
	if (prtw_lock)
		mtx_unlock(&prtw_lock->glock);
	else
		printf("%s prtw_lock==NULL", __FUNCTION__);

}


systime _rtw_get_current_time(void)
{
	struct timeval tvp;
	getmicrotime(&tvp);
	return tvp.tv_sec;
}

inline u32 _rtw_systime_to_ms(systime stime)
{
	return stime * 1000;
}

inline u32 _rtw_systime_to_us(systime stime)
{
	/*return jiffies_to_usecs(stime);*/
	#error "TBD\n"
}

inline systime _rtw_ms_to_systime(u32 ms)
{
	return ms / 1000;
}

inline systime _rtw_us_to_systime(u32 us)
{
	#error "TBD\n"
}

inline bool _rtw_time_after(systime a, systime b)
{
	#error "TBD\n"
}

void rtw_sleep_schedulable(int ms)
{
	DELAY(ms * 1000);
	return ;
}

void rtw_msleep_os(int ms)
{
	/* Delay for delay microseconds */
	DELAY(ms * 1000);
	return ;
}
void rtw_usleep_os(int us)
{
	/* Delay for delay microseconds */
	DELAY(us);

	return ;
}
#ifdef DBG_DELAY_OS
void _rtw_mdelay_os(int ms, const char *func, const int line)
{
	RTW_INFO("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);
}
void _rtw_udelay_os(int us, const char *func, const int line)
{
	RTW_INFO("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);
}
#else
void rtw_mdelay_os(int ms)
{
	DELAY(ms * 1000);
	return ;
}
void rtw_udelay_os(int us)
{
	/* Delay for delay microseconds */
	DELAY(us);
	return ;
}
#endif
void rtw_yield_os(void)
{
	yield();
}

inline void rtw_set_bit(int nr, unsigned long *addr)
{
	#error "TBD\n";
}

inline void rtw_clear_bit(int nr, unsigned long *addr)
{
	#error "TBD\n";
}

inline int rtw_test_and_clear_bit(int nr, unsigned long *addr)
{
	#error "TBD\n";
}


/*
* Test if the specifi @param path is a direct and readable
* @param path the path of the direct to test
* @return _TRUE or _FALSE
*/
int rtw_is_dir_readable(const char *path)
{
	/* Todo... */
	return _FALSE;
}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable(const char *path)
{
	/* Todo... */
	return _FALSE;
}

/*
* Test if the specifi @param path is a file and readable.
* If readable, @param sz is got
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable_with_size(const char *path, u32 *sz)
{
	/* Todo... */
	return _FALSE;
}
/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrieve_from_file(const char *path, u8 *buf, u32 sz)
{
	/* Todo... */
	return 0;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(const char *path, u8 *buf, u32 sz)
{
	/* Todo... */
	return 0;
}


/*
 * Copy a buffer from userspace and write into kernel address
 * space.
 *
 * This emulation just calls the FreeBSD copyin function (to
 * copy data from user space buffer into a kernel space buffer)
 * and is designed to be used with the above io_write_wrapper.
 *
 * This function should return the number of bytes not copied.
 * I.e. success results in a zero value.
 * Negative error values are not returned.
 */
unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
	if (copyin(from, to, n) != 0) {
		/* Any errors will be treated as a failure
		   to copy any of the requested bytes */
		return n;
	}

	return 0;
}

unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
	if (copyout(from, to, n) != 0) {
		/* Any errors will be treated as a failure
		   to copy any of the requested bytes */
		return n;
	}

	return 0;
}


/*
 * The usb_register and usb_deregister functions are used to register
 * usb drivers with the usb subsystem. In this compatibility layer
 * emulation a list of drivers (struct usb_driver) is maintained
 * and is used for probing/attaching etc.
 *
 * usb_register and usb_deregister simply call these functions.
 */
int
usb_register(struct usb_driver *driver)
{
	rtw_usb_linux_register(driver);
	return 0;
}


int
usb_deregister(struct usb_driver *driver)
{
	rtw_usb_linux_deregister(driver);
	return 0;
}

void module_init_exit_wrapper(void *arg)
{
	int (*func)(void) = arg;
	func();
	return;
}


#ifdef CONFIG_PLATFORM_SPRD
	#ifdef do_div
		#undef do_div
	#endif
	#include <asm-generic/div64.h>
#endif

u64 rtw_modular64(u64 x, u64 y)
{
	return x % y;
}

u64 rtw_division64(u64 x, u64 y)
{
	return x / y;
}

inline u32 rtw_random32(void)
{
#error "to be implemented\n"
}

inline u32 rtw_mirror_dump(u8 *hdr, u32 hdr_len, u8 *buf, u32 sz)
{
	return 0;
}
