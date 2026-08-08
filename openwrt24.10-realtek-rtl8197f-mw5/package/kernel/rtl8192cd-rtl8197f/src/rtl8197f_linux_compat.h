#ifndef __MW5_LINUX_COMPAT_H__
#define __MW5_LINUX_COMPAT_H__

#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/timer.h>
#include <linux/workqueue.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/dma-mapping.h>
#include <linux/io.h>
#include <linux/timekeeping.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/printk.h>
#include <asm/io.h>
#include <net/cfg80211.h>

#ifndef printk
#define printk _printk
#endif

#ifndef IEEE80211_BAND_2GHZ
#define IEEE80211_BAND_2GHZ NL80211_BAND_2GHZ
#endif
#ifndef IEEE80211_BAND_5GHZ
#define IEEE80211_BAND_5GHZ NL80211_BAND_5GHZ
#endif

#ifndef PCI_DMA_TODEVICE
#define PCI_DMA_TODEVICE DMA_TO_DEVICE
#endif
#ifndef PCI_DMA_FROMDEVICE
#define PCI_DMA_FROMDEVICE DMA_FROM_DEVICE
#endif

/* PCI DMA convenience wrappers removed from newer kernels. */
#ifndef pci_alloc_consistent
#define pci_alloc_consistent(pdev, size, dma_handle) \
	dma_alloc_coherent(&(pdev)->dev, (size), (dma_handle), GFP_KERNEL)
#endif
#ifndef pci_free_consistent
#define pci_free_consistent(pdev, size, cpu_addr, dma_handle) \
	dma_free_coherent(&(pdev)->dev, (size), (cpu_addr), (dma_handle))
#endif
#ifndef pci_map_single
#define pci_map_single(pdev, ptr, size, direction) \
	dma_map_single(&(pdev)->dev, (ptr), (size), (direction))
#endif
#ifndef pci_unmap_single
#define pci_unmap_single(pdev, dma_addr, size, direction) \
	dma_unmap_single(&(pdev)->dev, (dma_addr), (size), (direction))
#endif
#ifndef pci_dma_sync_single_for_cpu
#define pci_dma_sync_single_for_cpu(pdev, dma_addr, size, direction) \
	dma_sync_single_for_cpu(&(pdev)->dev, (dma_addr), (size), (direction))
#endif
#ifndef pci_dma_sync_single_for_device
#define pci_dma_sync_single_for_device(pdev, dma_addr, size, direction) \
	dma_sync_single_for_device(&(pdev)->dev, (dma_addr), (size), (direction))
#endif

#ifndef ioremap_nocache
#define ioremap_nocache(addr, size) ioremap((addr), (size))
#endif
#ifndef PDE_DATA
#define PDE_DATA(inode) pde_data(inode)
#endif
#ifndef CPHYSADDR
#define CPHYSADDR(addr) ((unsigned long)virt_to_phys((void *)(unsigned long)(addr)))
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
struct timeval {
	time64_t tv_sec;
	suseconds_t tv_usec;
};
static inline void mw5_do_gettimeofday(struct timeval *tv)
{
	struct timespec64 ts;

	ktime_get_real_ts64(&ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / NSEC_PER_USEC;
}
#ifndef do_gettimeofday
#define do_gettimeofday(tv) mw5_do_gettimeofday(tv)
#endif
#ifndef HAVE_PROC_OPS
#define HAVE_PROC_OPS 1
#endif
#endif

/*
 * Realtek rtl8192cd was written for the pre-4.15 timer API where timer_list
 * exposed ->function, ->data and ->expires. Keep that ABI local to this driver
 * while using a native Linux timer internally.
 */
struct mw5_legacy_timer {
	struct timer_list timer;
	unsigned long expires;
	void (*function)(unsigned long);
	unsigned long data;
};

static inline void mw5_legacy_timer_trampoline(struct timer_list *ktimer)
{
	struct mw5_legacy_timer *timer =
		container_of(ktimer, struct mw5_legacy_timer, timer);

	if (timer->function)
		timer->function(timer->data);
}

static inline void mw5_legacy_timer_init(struct mw5_legacy_timer *timer)
{
	timer->function = NULL;
	timer->data = 0;
	timer->expires = 0;
	timer_setup(&timer->timer, mw5_legacy_timer_trampoline, 0);
}

static inline int mw5_legacy_mod_timer(struct mw5_legacy_timer *timer,
				       unsigned long expires)
{
	timer->expires = expires;
	return mod_timer(&timer->timer, expires);
}

static inline void mw5_legacy_add_timer(struct mw5_legacy_timer *timer)
{
	mod_timer(&timer->timer, timer->expires);
}

static inline int mw5_legacy_del_timer(struct mw5_legacy_timer *timer)
{
	return timer_delete(&timer->timer);
}

static inline int mw5_legacy_del_timer_sync(struct mw5_legacy_timer *timer)
{
	return timer_delete_sync(&timer->timer);
}

static inline int mw5_legacy_timer_pending(const struct mw5_legacy_timer *timer)
{
	return timer_pending(&timer->timer);
}

#ifndef RTL_PRIV_DATA_SIZE
#define RTL_PRIV_DATA_SIZE 128
#endif

/* IPX itself disappeared from Linux, but rtl8192cd only parses the on-wire
 * header for legacy bridge/NAT25 compatibility. */
#ifndef _MW5_IPX_WIRE_HEADER
#define _MW5_IPX_WIRE_HEADER
struct ipx_address {
	__be32 net;
	u8 node[6];
	__be16 sock;
} __packed;
struct ipxhdr {
	__be16 ipx_checksum;
	__be16 ipx_pktsize;
	u8 ipx_tctrl;
	u8 ipx_type;
	struct ipx_address ipx_dest;
	struct ipx_address ipx_source;
} __packed;
#endif

#ifndef BSP_BOND_97FB
#define BSP_BOND_97FB 1
#define BSP_BOND_97FN 2
#define BSP_BOND_97FS 3
#endif


/* v42 board glue and phase telemetry, linked into the same rtl8192cd module. */
unsigned int rtl819x_bond_option(void);
bool rtl8197f_wlan_external_enabled(void);
void rtl8197f_wlan_phase_set(int phase, const char *name, int rc);

#endif /* __MW5_LINUX_COMPAT_H__ */
