/******************************************************************************
 *
 * Copyright(c) 2007 - 2019 Realtek Corporation.
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
#ifndef __OSDEP_LINUX_SERVICE_H_
#define __OSDEP_LINUX_SERVICE_H_

#include <linux/version.h>
#include <linux/spinlock.h>
#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/namei.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 5))
#include <linux/kref.h>
#endif
/* #include <linux/smp_lock.h> */
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/skbuff.h>
#include <linux/circ_buf.h>
#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <asm/atomic.h>
#include <asm/io.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26))
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif
#include <linux/sem.h>
#include <linux/sched.h>
#include <linux/etherdevice.h>
#include <linux/wireless.h>
#include <net/iw_handler.h>
#include <net/addrconf.h>
#include <linux/if_arp.h>
#include <linux/rtnetlink.h>
#include <linux/delay.h>
#include <linux/interrupt.h>	/* for struct tasklet_struct */
#include <linux/ip.h>
#include <linux/kthread.h>
#include <linux/list.h>
#include <linux/vmalloc.h>
#include <linux/list_sort.h>

#if 1//def CONFIG_RECV_THREAD_MODE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <uapi/linux/sched/types.h>	/* struct sched_param */
#endif
#endif

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 5, 41))
#include <linux/tqueue.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 7, 0))
#include <uapi/linux/limits.h>
#else
#include <linux/limits.h>
#endif

#ifdef RTK_DMP_PLATFORM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12))
#include <linux/pageremap.h>
#endif
#include <asm/io.h>
#endif

#ifdef CONFIG_NET_RADIO
#define CONFIG_WIRELESS_EXT
#endif

/* Monitor mode */
#include <net/ieee80211_radiotap.h>

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24))
#include <linux/ieee80211.h>
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 25) && \
	 LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 29))
#define CONFIG_IEEE80211_HT_ADDT_INFO
#endif

#ifdef CONFIG_IOCTL_CFG80211
/*	#include <linux/ieee80211.h> */
#include <net/cfg80211.h>
#endif /* CONFIG_IOCTL_CFG80211 */


#ifdef CONFIG_HAS_EARLYSUSPEND
#include <linux/earlysuspend.h>
#endif /* CONFIG_HAS_EARLYSUSPEND */

#ifdef CONFIG_EFUSE_CONFIG_FILE
#include <linux/fs.h>
#endif

#ifdef CONFIG_USB_HCI
#include <linux/usb.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 21))
#include <linux/usb_ch9.h>
#else
#include <linux/usb/ch9.h>
#endif
#endif

#if defined(CONFIG_RTW_BRSC) || defined (CONFIG_RTL_EXT_PORT_SUPPORT)
#include <net/rtl/rtl_brsc.h>
#endif

#include <linux/ratelimit.h>

#if defined(CONFIG_RTW_GRO) && (!defined(CONFIG_RTW_NAPI))

	#error "Enable NAPI before enable GRO\n"

#endif


#if (KERNEL_VERSION(2, 6, 29) > LINUX_VERSION_CODE && defined(CONFIG_RTW_NAPI))

	#undef CONFIG_RTW_NAPI
	/*#warning "Linux Kernel version too old to support NAPI (should newer than 2.6.29)\n"*/

#endif

#if (KERNEL_VERSION(2, 6, 33) > LINUX_VERSION_CODE && defined(CONFIG_RTW_GRO))

	#undef CONFIG_RTW_GRO
	/*#warning "Linux Kernel version too old to support GRO(should newer than 2.6.33)\n"*/

#endif

static inline void *_rtw_vmalloc(u32 sz)
{
	void *pbuf;

	pbuf = vmalloc(sz);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

static inline void *_rtw_zvmalloc(u32 sz)
{
	void *pbuf;

	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		memset(pbuf, 0, sz);

	return pbuf;
}

static inline void _rtw_vmfree(void *pbuf, u32 sz)
{
	vfree(pbuf);

#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */
}

static inline void *_rtw_malloc(u32 sz)
{
	void *pbuf = NULL;

	#ifdef RTK_DMP_PLATFORM
	if (sz > 0x4000)
		pbuf = dvr_malloc(sz);
	else
	#endif
		pbuf = kmalloc(sz, in_atomic() ? GFP_ATOMIC : GFP_KERNEL);

#ifdef DBG_MEMORY_LEAK
	if (pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif /* DBG_MEMORY_LEAK */

	return pbuf;

}

static inline void *_rtw_zmalloc(u32 sz)
{
	/*kzalloc in KERNEL_VERSION(2, 6, 14)*/
	void *pbuf = _rtw_malloc(sz);

	if (pbuf != NULL)
		memset(pbuf, 0, sz);

	return pbuf;
}

static inline void _rtw_mfree(void *pbuf, u32 sz)
{
	#ifdef RTK_DMP_PLATFORM
	if (sz > 0x4000)
		dvr_free(pbuf);
	else
	#endif
		kfree(pbuf);

#ifdef DBG_MEMORY_LEAK
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif /* DBG_MEMORY_LEAK */

}

#ifdef CONFIG_USB_HCI
typedef struct urb *PURB;

static inline void *_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma)
{
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	return usb_alloc_coherent(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
	#else
	return usb_buffer_alloc(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
	#endif
}
static inline void _rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma)
{
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	usb_free_coherent(dev, size, addr, dma);
	#else
	usb_buffer_free(dev, size, addr, dma);
	#endif
}
#endif /* CONFIG_USB_HCI */


/*lock - spinlock*/
typedef	spinlock_t _lock;
static inline void _rtw_spinlock_init(_lock *plock)
{
	spin_lock_init(plock);
}
static inline void _rtw_spinlock_free(_lock *plock)
{
}
static inline void _rtw_spinlock(_lock *plock)
{
#ifdef WKARD_98D
		spin_lock_bh(plock);
#else
	spin_lock(plock);
#endif
}
static inline void _rtw_spinunlock(_lock *plock)
{
#ifdef WKARD_98D
		spin_unlock_bh(plock);
#else
	spin_unlock(plock);
#endif
}

#if 0
static inline void _rtw_spinlock_ex(_lock *plock)
{
	spin_lock(plock);
}

static inline void _rtw_spinunlock_ex(_lock *plock)
{

	spin_unlock(plock);
}
#endif
__inline static void _rtw_spinlock_irq(_lock *plock, unsigned long *flags)
{
	spin_lock_irqsave(plock, *flags);
}
__inline static void _rtw_spinunlock_irq(_lock *plock, unsigned long *flags)
{
	spin_unlock_irqrestore(plock, *flags);
}
__inline static void _rtw_spinlock_bh(_lock *plock)
{
	spin_lock_bh(plock);
}
__inline static void _rtw_spinunlock_bh(_lock *plock)
{
	spin_unlock_bh(plock);
}


/*lock - semaphore*/
typedef struct	semaphore _sema;
static inline void _rtw_init_sema(_sema *sema, int init_val)
{
	sema_init(sema, init_val);
}
static inline void _rtw_free_sema(_sema *sema)
{
}
static inline void _rtw_up_sema(_sema *sema)
{
	up(sema);
}
static inline u32 _rtw_down_sema(_sema *sema)
{
	if (down_interruptible(sema))
		return _FAIL;
	else
		return _SUCCESS;
}

/*lock - mutex*/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
  #if defined(CONFIG_WFO_VIRT_MODULE)
	#if defined(RTK_WFO_PE_LINUX_HDR)
	  typedef struct mutex		  _mutex;
	#else /* !RTK_WFO_PE_LINUX_HDR */
	  typedef union {
		  struct mutex m;
		  char wfo_reserved[ECOS_MUTEX_SIZE]; /* must be equal to sizeof(cyg_mutex_t) */
	  } _mutex;
	#endif /* RTK_WFO_PE_LINUX_HDR */
  #else /* !CONFIG_WFO_VIRT_MODULE */
	  typedef struct mutex		  _mutex;
  #endif /* CONFIG_WFO_VIRT_MODULE */
#else
	typedef struct semaphore	_mutex;
#endif
static inline void _rtw_mutex_init(_mutex *pmutex)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
	mutex_init((struct mutex*)pmutex);
#else
	init_MUTEX(pmutex);
#endif
}

static inline void _rtw_mutex_free(_mutex *pmutex)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
	mutex_destroy((struct mutex*)pmutex);
#else
#endif
}
__inline static int _rtw_mutex_lock_interruptible(_mutex *pmutex)
{
	int ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
	/* mutex_lock(pmutex); */
	ret = mutex_lock_interruptible((struct mutex*)pmutex);
#else
	ret = down_interruptible(pmutex);
#endif
	return ret;
}

__inline static int _rtw_mutex_lock(_mutex *pmutex)
{
	int ret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
	mutex_lock((struct mutex*)pmutex);
#else
	down(pmutex);
#endif
	return ret;
}

__inline static void _rtw_mutex_unlock(_mutex *pmutex)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37))
	mutex_unlock((struct mutex*)pmutex);
#else
	up(pmutex);
#endif
}


/*completion*/
typedef struct completion _completion;
static inline void _rtw_init_completion(_completion *comp)
{
	init_completion(comp);
}
static inline unsigned long _rtw_wait_for_comp_timeout(_completion *comp, unsigned long timeout)
{
	return wait_for_completion_timeout(comp, timeout);
}
static inline void _rtw_wait_for_comp(_completion *comp)
{
	return wait_for_completion(comp);
}

struct	__queue	{
	struct	list_head	queue;
	_lock	lock;
	int qlen;
};

typedef unsigned char	_buffer;

typedef struct	__queue	_queue;


/*list*/
#define LIST_CONTAINOR(ptr, type, member) \
	((type *)((char *)(ptr)-(SIZE_T)(&((type *)0)->member)))


typedef struct	list_head	_list;
/* Caller must check if the list is empty before calling rtw_list_delete*/
__inline static void rtw_list_delete(_list *plist)
{
	list_del_init(plist);
}

__inline static _list *get_next(_list	*list)
{
	return list->next;
}
__inline static _list	*get_list_head(_queue *queue)
{
	return &(queue->queue);
}
#define rtw_list_first_entry(ptr, type, member) list_first_entry(ptr, type, member)

/* hlist */
typedef struct	hlist_head	rtw_hlist_head;
typedef struct	hlist_node	rtw_hlist_node;
#define rtw_hlist_for_each_entry(pos, head, member) hlist_for_each_entry(pos, head, member)
#define rtw_hlist_for_each_safe(pos, n, head) hlist_for_each_safe(pos, n, head)
#define rtw_hlist_entry(ptr, type, member) hlist_entry(ptr, type, member)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 9, 0))
#define rtw_hlist_for_each_entry_safe(pos, np, n, head, member) hlist_for_each_entry_safe(pos, n, head, member)
#define rtw_hlist_for_each_entry_rcu(pos, node, head, member) hlist_for_each_entry_rcu(pos, head, member)
#else
#define rtw_hlist_for_each_entry_safe(pos, np, n, head, member) hlist_for_each_entry_safe(pos, np, n, head, member)
#define rtw_hlist_for_each_entry_rcu(pos, node, head, member) hlist_for_each_entry_rcu(pos, node, head, member)
#endif

/* RCU */
typedef struct rcu_head rtw_rcu_head;
#define rtw_rcu_dereference(p) rcu_dereference((p))
#define rtw_rcu_dereference_protected(p, c) rcu_dereference_protected(p, c)
#define rtw_rcu_assign_pointer(p, v) rcu_assign_pointer((p), (v))
#define rtw_rcu_read_lock() rcu_read_lock()
#define rtw_rcu_read_unlock() rcu_read_unlock()
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 34))
#define rtw_rcu_access_pointer(p) rcu_access_pointer(p)
#endif

/* rhashtable */
#include "../os_dep/linux/rtw_rhashtable.h"


/*thread*/
typedef void *_thread_hdl_;
typedef int thread_return;
typedef void *thread_context;
struct thread_hdl{
	_thread_hdl_ thread_handler;
	u8 thread_status;
};
#define THREAD_STATUS_STARTED BIT(0)
#define THREAD_STATUS_STOPPED BIT(1)
#define RST_THREAD_STATUS(t) (t->thread_status = 0)
#define SET_THREAD_STATUS(t, s) 	(t->thread_status |= s)
#define CLR_THREAD_STATUS(t, cl)	(t->thread_status &= ~(cl))
#define CHK_THREAD_STATUS(t, ck) (t->thread_status & ck)

typedef void timer_hdl_return;
typedef void *timer_hdl_context;

static inline void rtw_thread_enter(char *name)
{
	allow_signal(SIGTERM);
}

static inline void rtw_thread_exit(_completion *comp)
{
	complete_and_exit(comp, 0);
}

static inline _thread_hdl_ rtw_thread_start(int (*threadfn)(void *data),
			void *data, const char namefmt[])
{
	_thread_hdl_ _rtw_thread = NULL;

	_rtw_thread = kthread_run(threadfn, data, namefmt);
	if (IS_ERR(_rtw_thread)) {
		WARN_ON(!_rtw_thread);
		_rtw_thread = NULL;
	}
	return _rtw_thread;
}
static inline bool rtw_thread_stop(_thread_hdl_ th)
{

	return kthread_stop(th);
}
static inline void rtw_thread_wait_stop(void)
{
	#if 0
	while (!kthread_should_stop())
		rtw_msleep_os(10);
	#else
	set_current_state(TASK_INTERRUPTIBLE);
	while (!kthread_should_stop()) {
		schedule();
		set_current_state(TASK_INTERRUPTIBLE);
	}
	__set_current_state(TASK_RUNNING);
	#endif
}

static inline void flush_signals_thread(void)
{
	if (signal_pending(current))
		flush_signals(current);
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
	#define DMA_BIT_MASK(n) (((n) == 64) ? ~0ULL : ((1ULL<<(n))-1))
#endif

typedef unsigned long systime;

/*tasklet*/
#ifndef CONFIG_RTW_OS_HANDLER_EXT
typedef struct tasklet_struct _tasklet;
#endif /* CONFIG_RTW_OS_HANDLER_EXT */
typedef void (*tasklet_fn_t)(unsigned long);

/*work*/
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 41))
#ifndef CONFIG_RTW_OS_HANDLER_EXT
typedef struct work_struct _workitem;
#endif /* CONFIG_RTW_OS_HANDLER_EXT */
#else
typedef struct tq_struct _workitem;
#endif

#ifdef CONFIG_RTW_OS_HANDLER_EXT
enum rtw_handler_state {
	RTW_HDL_STATE_SCHED	/* scheduled for execution */
};

#define RTW_OS_HANDLER_TASKLET	(0)
#define RTW_OS_HANDLER_WORK	(1)

struct rtw_os_handler {
	/* tasklet and work should be first for easier migratiion 
	 * to OS handler model. */
	union {
		struct tasklet_struct task;
		struct work_struct work;
	};
	/* Data specific to tasklet and work */
	union {
		struct {
			tasklet_fn_t func;
			unsigned long data;
#if (LINUX_VERSION_CODE > KERNEL_VERSION(5, 10, 0))
			struct __call_single_data csd;
#else
			struct call_single_data csd;
#endif
			unsigned long state;
		} task_data;
		struct {
			void *func;
			void *cntx;
			struct workqueue_struct *wq;
		} work_data;
	};
	/* Common data for OS handler management */
	u8		init;
	u8		type; /* 0: tasklet, 1: work */
	u8		id;
	u8		cpu_id;
	const char 	*name;
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	u32		run_on_cpu;
	u32		run;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
};

typedef struct rtw_os_handler _tasklet;
typedef struct rtw_os_handler _workitem;

/* Core API for PHL handler */
struct rtw_phl_handler;
u8 rtw_plfm_init_handler_ext(void *drv_priv,
                             struct rtw_phl_handler *phl_handler);

u8 rtw_plfm_deinit_handler_ext(void *drv_priv,
                               struct rtw_phl_handler *phl_handler);

/* Core API for general OS handler */
struct dvobj_priv;
void rtw_init_os_handler(struct dvobj_priv *dvobj, struct rtw_os_handler *handler);
void rtw_deinit_os_handler(struct dvobj_priv *dvobj, struct rtw_os_handler *handler);

/* List of managed handlers */
enum rtw_core_handler_id {
	RTW_HANDLER_CORE_ONE_TX,
	RTW_HANDLER_CORE_NETIF_RX,
	RTW_HANDLER_CORE_RX_RECYCLE,
	RTW_CORE_HANDLER_NUM,
	/* PHL hanlers */
	RTW_HANDLER_PHL_TX = RTW_CORE_HANDLER_NUM,
	RTW_HANDLER_PHL_RX,
	RTW_HANDLER_PHL_EVT,
	RTW_HANDLER_PHL_SER,
	RTW_HANDLER_PHL_FW_WDT,
	#ifdef CONFIG_PHL_TEST_SUITE
        RTW_HANDLER_TEST,
	#endif
        RTW_HANDLER_NUM_MAX
};
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

/* tasklet functions */
#if 1
static inline void rtw_tasklet_init(_tasklet *t, tasklet_fn_t func,
                                    unsigned long data)
{
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	/* Init for un-managed tasklet */
	if (t->init == 0) {
		struct rtw_os_handler *os_handler = (struct rtw_os_handler *)t;
		t->task_data.csd.func = NULL;
		t->cpu_id = WORK_CPU_UNBOUND;
		t->type = RTW_OS_HANDLER_TASKLET;
		t->init = 1;
		t->id = RTW_HANDLER_NUM_MAX;
	}
	/* Always save tasklet function and custom data for PHL use model */
	t->task_data.func = func;
	t->task_data.data = data;
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	t->run_on_cpu = 0;
	t->run = 0;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */

#endif /* CONFIG_RTW_OS_HANDLER_EXT */
	tasklet_init((struct tasklet_struct *)t, func, data);
}

static inline void rtw_tasklet_kill(_tasklet *t)
{
	tasklet_kill((struct tasklet_struct *)t);
}

static inline void rtw_tasklet_schedule(_tasklet *t)
{
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	if ((t->task_data.csd.func != NULL) && (t->cpu_id != WORK_CPU_UNBOUND)) {
		if (!test_and_set_bit(RTW_HDL_STATE_SCHED, &t->task_data.state)) {
			smp_call_function_single_async(t->cpu_id, &t->task_data.csd);
			#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
			t->run_on_cpu++;
			#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
		}

		return;
	}
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	t->run++;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

	tasklet_schedule((struct tasklet_struct *)t);
}

static inline void rtw_tasklet_hi_schedule(_tasklet *t)
{
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	if ((t->task_data.csd.func != NULL) && (t->cpu_id != WORK_CPU_UNBOUND)) {
		if (!test_and_set_bit(RTW_HDL_STATE_SCHED, &t->task_data.state)) {
			#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
			t->run_on_cpu++;
			#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
			smp_call_function_single_async(t->cpu_id, &t->task_data.csd);
		}

		return;
	}
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	t->run++;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
#endif /* CONFIG_RTW_OS_HANDLER_EXT */

	tasklet_hi_schedule((struct tasklet_struct *)t);
}

#else
#define rtw_tasklet_init tasklet_init
#define rtw_tasklet_kill tasklet_kill
#define rtw_tasklet_schedule tasklet_schedule
#define rtw_tasklet_hi_schedule tasklet_hi_schedule
#endif

/* workitem functions */
static inline void _init_workitem(_workitem *pwork, void *pfunc, void *cntx)
{
	#ifdef CONFIG_RTW_OS_HANDLER_EXT
	/* Init for un-managed work */
	if (pwork->init == 0) {
		pwork->work_data.wq = NULL;
		pwork->cpu_id = WORK_CPU_UNBOUND;
		pwork->init = 1;
		pwork->id = RTW_HANDLER_NUM_MAX;
	}
	pwork->work_data.func = pfunc;
	pwork->work_data.cntx = cntx;
	#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
	pwork->run_on_cpu = 0;
	pwork->run = 0;
	#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
	#endif /* CONFIG_RTW_OS_HANDLER_EXT */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20))
	INIT_WORK((struct work_struct *)pwork, pfunc);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 41))
	INIT_WORK(pwork, pfunc, pwork);
#else
	INIT_TQUEUE(pwork, pfunc, pwork);
#endif
}

__inline static void _set_workitem(_workitem *pwork)
{
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 41))
#ifdef CONFIG_RTW_OS_HANDLER_EXT
	if (pwork->work_data.wq != NULL) {
		queue_work_on(pwork->cpu_id, pwork->work_data.wq,
		              (struct work_struct *)pwork);
		#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
		pwork->run_on_cpu++;
		#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
	} else {
		schedule_work((struct work_struct *)pwork);
		#ifdef CONFIG_RTW_OS_HANDLER_EXT_DBG
		pwork->run++;
		#endif /* CONFIG_RTW_OS_HANDLER_EXT_DBG */
	}
#else /* CONFIG_RTW_OS_HANDLER_EXT */
	schedule_work(pwork);
#endif /* CONFIG_RTW_OS_HANDLER_EXT */
#else /* LINUX_VERSION_CODE */
	schedule_task(pwork);
#endif /* LINUX_VERSION_CODE */
}

__inline static void _cancel_workitem_sync(_workitem *pwork)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 22))
	cancel_work_sync((struct work_struct *)pwork);
#elif (LINUX_VERSION_CODE > KERNEL_VERSION(2, 5, 41))
	flush_scheduled_work();
#else
	flush_scheduled_tasks();
#endif
}


#ifdef CONFIG_RTW_MEMPOOL
struct sk_buff *rtw_mem_pool_alloc_skb(int size);
#endif

/*skb_buffer*/
static inline struct sk_buff *_rtw_skb_alloc(u32 sz)
{
	struct sk_buff *skb = NULL;

#ifdef CONFIG_RTW_MEMPOOL
	skb =  rtw_mem_pool_alloc_skb(sz);
#else
#ifdef WKARD_98D
	skb =  __dev_alloc_skb(sz, GFP_ATOMIC);
#else
	skb =  __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
#endif
#endif
/*
	if (!skb)
		printk_ratelimited("[%s %d] alloc skb fail, size: %u\n", __func__, __LINE__, sz);
*/
	return skb;
}

static inline void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

static inline struct sk_buff *_rtw_skb_copy(const struct sk_buff *skb)
{
	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
}

static inline int _rtw_skb_linearize(struct sk_buff *skb)
{
	return skb_linearize(skb);
}

static inline struct sk_buff *_rtw_pskb_copy(struct sk_buff *skb)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	return pskb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
#else
	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
#endif
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22))
/* Porting from linux kernel, for compatible with old kernel. */
static inline unsigned char *skb_tail_pointer(const struct sk_buff *skb)
{
	return skb->tail;
}

static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
	skb->tail = skb->data;
}

static inline void skb_set_tail_pointer(struct sk_buff *skb, const int offset)
{
	skb->tail = skb->data + offset;
}

static inline unsigned char *skb_end_pointer(const struct sk_buff *skb)
{
	return skb->end;
}
#endif
static inline u8 *rtw_skb_data(struct sk_buff *pkt)
{
	return pkt->data;
}

static inline u32 rtw_skb_len(struct sk_buff *pkt)
{
	return pkt->len;
}

static inline void *rtw_skb_put_zero(struct sk_buff *skb, unsigned int len)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 13, 0)
	return skb_put_zero(skb, len);
#else
	void *tmp = skb_put(skb, len);

	memset(tmp, 0, len);

	return tmp;
#endif
}

/*timer*/
typedef struct rtw_timer_list _timer;
struct rtw_timer_list {
	struct timer_list timer;
	void (*function)(void *);
	void *arg;
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
static inline void timer_hdl(struct timer_list *in_timer)
{
	_timer *ptimer = from_timer(ptimer, in_timer, timer);

	ptimer->function(ptimer->arg);
}
#else
static inline void timer_hdl(unsigned long cntx)
{
	_timer *ptimer = (_timer *)cntx;

	ptimer->function(ptimer->arg);
}
#endif

__inline static void _init_timer(_timer *ptimer, void *pfunc, void *cntx)
{
	ptimer->function = pfunc;
	ptimer->arg = cntx;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	timer_setup(&ptimer->timer, timer_hdl, 0);
#else
	/* setup_timer(ptimer, pfunc,(u32)cntx);	 */
	ptimer->timer.function = timer_hdl;
	ptimer->timer.data = (unsigned long)ptimer;
	init_timer(&ptimer->timer);
#endif
}

__inline static void _set_timer(_timer *ptimer, u32 delay_time)
{
	mod_timer(&ptimer->timer , (jiffies + (delay_time * HZ / 1000)));
}

__inline static void _cancel_timer(_timer *ptimer, u8 *bcancelled)
{
	*bcancelled = del_timer_sync(&ptimer->timer) == 1 ? 1 : 0;
}

__inline static void _cancel_timer_async(_timer *ptimer)
{
	del_timer(&ptimer->timer);
}

/*
 * Global Mutex: can only be used at PASSIVE level.
 *   */
#define ACQUIRE_GLOBAL_MUTEX(_MutexCounter)                              \
	{                                                               \
		while (atomic_inc_return((atomic_t *)&(_MutexCounter)) != 1) { \
			atomic_dec((atomic_t *)&(_MutexCounter));        \
			msleep(10);                          \
		}                                                           \
	}

#define RELEASE_GLOBAL_MUTEX(_MutexCounter)                              \
	{                                                               \
		atomic_dec((atomic_t *)&(_MutexCounter));        \
	}


typedef	struct	net_device *_nic_hdl;
static inline int rtw_netif_queue_stopped(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	return (netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 0)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 1)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 2)) &&
		netif_tx_queue_stopped(netdev_get_tx_queue(pnetdev, 3)));
#else
	return netif_queue_stopped(pnetdev);
#endif
}

#if defined(CONFIG_WFO_VIRT_SAME_CPU)
struct net_device *wfo_rx_ndev(struct net_device *ndev);
#endif /* CONFIG_WFO_VIRT_SAME_CPU */

#if defined (CONFIG_RTL_EXT_PORT_SUPPORT)
extern int rtl_wlanFwdToEth(struct sk_buff *skb);
extern int extPortEnabled;
#endif
//extern int rtw_go_slow;
//extern int rtw_go_fast;

#if defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
extern int check_br_port_ready(struct net_device *dev);
#ifdef CONFIG_RTW_BRSC
extern int rtw_brsc_should_go(struct net_device *dev, struct sk_buff *pskb);
#endif
#endif

static inline u8 is_highpri_pkt(u8 *data)
{
	u8 *ptr;
	unsigned short eth_type;
	unsigned char ip_proto;

	ptr = data + 12;
	eth_type =  *((unsigned short *)(ptr));
	if(eth_type == __constant_htons(ETH_P_8021Q))
	{
		ptr += 4;
		eth_type = *((unsigned short *)(ptr));
	}

	if(eth_type == __constant_htons(0x893a)) {		// 1905 pkt
		return 1;
	}
	else if (eth_type == __constant_htons(ETH_P_IP)) {
		ptr += 2;		// iph
		ptr += 9;
		ip_proto = *ptr;

		if (ip_proto == 1) {	// icmp
			return 1;
		}
	}

	return 0;
}

#ifdef CONFIG_HWSIM
int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb);
#else
static inline int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb)
{
#if defined(CONFIG_WFO_VIRT_SAME_CPU)
	ndev = wfo_rx_ndev(ndev);
#endif /* CONFIG_WFO_VIRT_SAME_CPU */

#if defined(CONFIG_RTW_FC_FASTFWD)
#if defined(CONFIG_RTL_ETH_RECYCLED_SKB)
extern int rtk_fc_fastfwd_netif_rx(struct sk_buff *skb);
	skb->dev = ndev;
	skb->from_dev = ndev;
	rtk_fc_fastfwd_netif_rx(skb);
#else
extern int fwdEngine_wifi_rx(struct sk_buff *skb);
enum {
	RE8670_RX_STOP=0,
	RE8670_RX_CONTINUE,
	RE8670_RX_STOP_SKBNOFREE,
	RE8670_RX_END
};
	int ret;

	skb->dev = ndev;
	skb->data-=14;
	skb->len+=14;
	skb->from_dev = ndev;

	if(is_highpri_pkt(skb->data))
		ret = RE8670_RX_CONTINUE;
	else
		ret = fwdEngine_wifi_rx(skb);

	if(ret==RE8670_RX_CONTINUE)
	{
		skb->data+=14;
		skb->len-=14;
		return netif_rx(skb);
	}
	else if(ret==RE8670_RX_STOP)
	{
		kfree_skb(skb);
		return NET_RX_DROP;
	}
#endif

	return NET_RX_SUCCESS;
#elif defined(CONFIG_RTL_XDSL_WIFI_HWACCELERATION)
	{
		extern int rtk_xdsl_process_wifi_rx(struct sk_buff *skb);
		if(rtk_xdsl_process_wifi_rx(skb))
		{
			return netif_rx(skb);
		}
		return NET_RX_SUCCESS;
	}
#elif defined(CONFIG_RTW_BRSC) || defined(CONFIG_RTL_EXT_PORT_SUPPORT)
	int ret = RTW_BRSC_CONTINUE;
	skb->dev = ndev;
	skb->data-=14;
	skb->len+=14;

	if(check_br_port_ready(ndev))
	{
		/*update wifi side sc entry*/
		rtw_update_brsc_info(skb->data + ETH_ALEN, ndev);

#if defined(CONFIG_RTW_BRSC)
		if(rtw_brsc_should_go(ndev, skb))
			ret = rtw_brsc_go(skb,ndev);
#endif
	}
	
	if(ret == RTW_BRSC_CONTINUE)
	{	
#if defined(CONFIG_RTL_EXT_PORT_SUPPORT)
		//check if transimit by extension port
		if(extPortEnabled)
			if(rtl_wlanFwdToEth(skb)==NET_RX_SUCCESS)
				return NET_RX_SUCCESS;
#endif
		/*continue RX*/
		skb->data+=14;
		skb->len-=14;
		//rtw_go_slow++;
		return netif_rx(skb);
	}
	//rtw_go_fast++;
	return ret;
#else
	skb->dev = ndev;
	skb->from_dev = ndev;
	return netif_rx(skb);
#endif
}
#endif

#ifdef CONFIG_RTW_NAPI
static inline int _rtw_netif_receive_skb(_nic_hdl ndev, struct sk_buff *skb)
{
	skb->dev = ndev;
	return netif_receive_skb(skb);
}

#ifdef CONFIG_RTW_GRO
static inline gro_result_t _rtw_napi_gro_receive(struct napi_struct *napi, struct sk_buff *skb)
{
	return napi_gro_receive(napi, skb);
}
#endif /* CONFIG_RTW_GRO */
#endif /* CONFIG_RTW_NAPI */

static inline void rtw_netif_wake_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_wake_all_queues(pnetdev);
#else
	netif_wake_queue(pnetdev);
#endif
}

static inline void rtw_netif_start_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_start_all_queues(pnetdev);
#else
	netif_start_queue(pnetdev);
#endif
}

static inline void rtw_netif_stop_queue(struct net_device *pnetdev)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35))
	netif_tx_stop_all_queues(pnetdev);
#else
	netif_stop_queue(pnetdev);
#endif
}
static inline void rtw_netif_device_attach(struct net_device *pnetdev)
{
	netif_device_attach(pnetdev);
}
static inline void rtw_netif_device_detach(struct net_device *pnetdev)
{
	netif_device_detach(pnetdev);
}
static inline void rtw_netif_carrier_on(struct net_device *pnetdev)
{
	netif_carrier_on(pnetdev);
}
static inline void rtw_netif_carrier_off(struct net_device *pnetdev)
{
	netif_carrier_off(pnetdev);
}

static inline int rtw_merge_string(char *dst, int dst_len, const char *src1, const char *src2)
{
	int	len = 0;
	len += snprintf(dst + len, dst_len - len, "%s", src1);
	len += snprintf(dst + len, dst_len - len, "%s", src2);

	return len;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27))
	#define rtw_signal_process(pid, sig) kill_pid(find_vpid((pid)), (sig), 1)
#else /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */
	#define rtw_signal_process(pid, sig) kill_proc((pid), (sig), 1)
#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)) */


/* Suspend lock prevent system from going suspend */
#ifdef CONFIG_WAKELOCK
	#include <linux/wakelock.h>
#elif defined(CONFIG_ANDROID_POWER)
	#include <linux/android_power.h>
#endif

/* limitation of path length */
#define PATH_LENGTH_MAX PATH_MAX

/* Atomic integer operations */
#define ATOMIC_T atomic_t
static inline void ATOMIC_SET(ATOMIC_T *v, int i)
{
	atomic_set(v, i);
}

static inline int ATOMIC_READ(ATOMIC_T *v)
{
	return atomic_read(v);
}

static inline void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	atomic_add(i, v);
}
static inline void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	atomic_sub(i, v);
}

static inline void ATOMIC_INC(ATOMIC_T *v)
{
	atomic_inc(v);
}

static inline void ATOMIC_DEC(ATOMIC_T *v)
{
	atomic_dec(v);
}

static inline int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	return atomic_add_return(i, v);
}

static inline int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	return atomic_sub_return(i, v);
}

static inline int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	return atomic_inc_return(v);
}

static inline int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	return atomic_dec_return(v);
}

static inline bool ATOMIC_INC_UNLESS(ATOMIC_T *v, int u)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 15))
	return atomic_add_unless(v, 1, u);
#else
	/* only make sure not exceed after this function */
	if (ATOMIC_INC_RETURN(v) > u) {
		ATOMIC_DEC(v);
		return 0;
	}
	return 1;
#endif
}

#define NDEV_FMT "%s"
#define NDEV_ARG(ndev) ndev->name
#define ADPT_FMT "%s"
#define ADPT_ARG(adpt) ({ _adapter *_a = (_adapter *)(adpt); _a->pnetdev ? _a->pnetdev->name : NULL; })
#define FUNC_NDEV_FMT "%s(%s)"
#define FUNC_NDEV_ARG(ndev) __func__, ndev->name
#define FUNC_ADPT_FMT "%s(%s)"
#define FUNC_ADPT_ARG(adapter) __func__, (adapter->pnetdev ? adapter->pnetdev->name : NULL)
#define DEV_FMT "%s"
#define DEV_ARG(dvobj) \
	({ \
		struct dvobj_priv *d = (dvobj); \
		const char *dev_name; \
		if (d->wiphy) \
			dev_name = wiphy_name(d->wiphy); \
		else \
			dev_name = pci_name(d->pci_data.ppcidev); \
		dev_name; \
	})

#define FUNC_DEV_FMT "%s(%s)"
#define FUNC_DEV_ARG(dvobj)	__func__, DEV_ARG(dvobj)
#define FUNC_DEVID_FMT "%s(%u)"
#define FUNC_DEVID_ARG(dvobj)	__func__, (dvobj)->dev_id

#define rtw_netdev_priv(netdev) (((struct rtw_netdev_priv_indicator *)netdev_priv(netdev))->priv)
struct rtw_netdev_priv_indicator {
	void *priv;
	u32 sizeof_priv;
};
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv);
extern struct net_device *rtw_alloc_etherdev(int sizeof_priv);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#define rtw_get_same_net_ndev_by_name(ndev, name) dev_get_by_name(name)
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 26))
#define rtw_get_same_net_ndev_by_name(ndev, name) dev_get_by_name(ndev->nd_net, name)
#else
#define rtw_get_same_net_ndev_by_name(ndev, name) dev_get_by_name(dev_net(ndev), name)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24))
#define rtw_get_bridge_ndev_by_name(name) dev_get_by_name(name)
#else
#define rtw_get_bridge_ndev_by_name(name) dev_get_by_name(&init_net, name)
#endif

static inline void rtw_dump_stack(void)
{
	dump_stack();
}
#define rtw_warn_on(condition) WARN_ON(condition)
#define RTW_DIV_ROUND_UP(n, d)	DIV_ROUND_UP(n, d)
#define rtw_sprintf(buf, size, format, arg...) snprintf(buf, size, format, ##arg)

#define STRUCT_PACKED __attribute__ ((packed))

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
typedef struct call_single_data call_single_data_t;
#endif

static inline void rtw_time_to_tm(time64_t totalsecs, int offset, struct tm *result)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 8, 0))
	time64_to_tm(totalsecs, offset, result);
#else
	time_to_tm(totalsecs, offset, result);
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 85))
typedef struct timespec64 _timeval;
#else
typedef struct timeval _timeval;
#endif

static inline void rtw_do_gettimeofday(_timeval *tv)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 85))
	ktime_get_real_ts64(tv);
#else
	do_gettimeofday(tv);
#endif
}

#ifndef fallthrough
#if __GNUC__ >= 5 || defined(__clang__)
#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#if __has_attribute(__fallthrough__)
#define fallthrough __attribute__((__fallthrough__))
#endif
#endif
#ifndef fallthrough
#define fallthrough do {} while (0) /* fallthrough */
#endif
#endif

#endif /* __OSDEP_LINUX_SERVICE_H_ */
