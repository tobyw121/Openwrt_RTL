/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 48574 $
 * $Date: 2014-06-13 13:33:33 +0800 (Fri, 13 Jun 2014) $
 *
 * Purpose : A Linux Ethernet driver for the Realtek Switch SOC.
 *
 * Feature : NIC module
 *
 */

/*
 * Include Files
 */
#include <linux/version.h>

#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/ethtool.h>
#include <linux/slab.h>
#include <linux/signal.h>
#include <linux/time.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/time.h>
#include <dev_config.h>
#include <common/rt_error.h>
#include <osal/print.h>
#include <drv/nic/nic.h>
#include <drv/swcore/chip.h>
#include <drv/swcore/sys.h>
#include <rtnic/rtnic_drv.h>

/*
 * Symbol Definition
 */
#define PKTBUF_ALLOC(size)      kmalloc(size, GFP_ATOMIC)
#define PKTBUF_FREE(pktbuf)     kfree(pktbuf)

/* System support multiple rx handler priority mechanism
 * The number of rx handler priority is defined in NIC_RX_CB_PRIORITY_NUMBER symbol
 * - 0 is highest priority
 * - NIC_RX_CB_PRIORITY_NUMBER-1 is lowest priority
 *
 * When higher priority rx handler return NIC_RX_HANDLED_OWNED, the lower priority
 * rx handler will not charge to be callback.
 *
 * You need to care rx handler sequence in your system, for example:
 * If you have RRCP & RTNIC rx handler both; you maybe define RRCP rx handler is 0 and 
 * RTNIC rx handler is 1 depend on your application.
 */
#define RTNIC_RX_HANDLER_PRIORITY   1

/*
 * Data Declaration
 */
struct rtnic_priv
{
    uint16  ready;
    uint32  msg_enable;
    uint8   traffic_enable;
    struct net_device   *pNdev;
    struct net_device_stats ndev_stats;
    uint32  rx_queue_num;
    uint32  tx_queue_num;
    spinlock_t  lock;
};

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
unsigned char gNIC_ADDRESS[6];
#endif

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtnic_pkt_alloc
 * Description:
 *      packet allocation 
 * Input:
 *      unit     - unit id
 *      size     - alloc size
 *      flags    - alloc flags
 * Output:
 *      ppPacket - pointer buffer to the allocated packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */ 
static int32 rtnic_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket)
{
    struct sk_buff *pSkb;
    drv_nic_pkt_t *pPacket;

    pPacket = PKTBUF_ALLOC(sizeof(drv_nic_pkt_t));    
    if (NULL == pPacket)
    {
        return RT_ERR_FAILED;
    }

    /* Allocate more space than spcified, so that we can make the whole DMA block has its own cache lines, and guarantee the coherence futher*/
    pSkb = dev_alloc_skb(size + RTNIC_PKTLEN_RSVD + (32 - ((size + RTNIC_PKTLEN_RSVD)%32)) + 28);
    if (NULL == pSkb)
    {
        PKTBUF_FREE(pPacket);
        return RT_ERR_FAILED;
    } 

    /* Make the begining address of DMA block cache line (32-byte) aligned */
    pSkb->data = (void*)((uint32)(pSkb->data + 28) & 0xffffffe0);
    skb_reserve(pSkb, RTNIC_PKTLEN_RSVD);  


    memset(pPacket, 0, sizeof(drv_nic_pkt_t));
    pPacket->head = pSkb->head;
    pPacket->data = pSkb->data;
    pPacket->tail = pSkb->data;
    pPacket->end = pSkb->end;
    pPacket->length = 0;
    pPacket->buf_id = (void *)pSkb;
    pPacket->next = NULL;


    *ppPacket = pPacket;

    return RT_ERR_OK;
} /* end of rtnic_pkt_alloc */

/* Function Name:
 *      rtnic_pkt_free
 * Description:
 *      free allocated packet 
 * Input:
 *      unit    - unit id
 *      pPacket - pointer buffer to the packet 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */ 
static int32 rtnic_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket)
{
    struct sk_buff *pSkb;

    if (NULL == pPacket)
    {
        osal_printf("Error: pPacket is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        return RT_ERR_FAILED;
    }

    pSkb = (struct sk_buff *)(pPacket->buf_id);
    if (NULL == pSkb)
    {
        osal_printf("Error: skb is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        return RT_ERR_FAILED;
    }

    /*
     * Always exactly free this packet
     * to prevent 'page allocation failure' problem when ISR hold all CPU time continuously
     */
#if 0
    dev_kfree_skb_any(pSkb);
#else
    dev_kfree_skb(pSkb);
#endif

    PKTBUF_FREE(pPacket);

    return RT_ERR_OK;
} /* end of rtnic_pkt_free */

/* Function Name:
 *      rtnic_rx_callback
 * Description:
 *      packet RX callback function 
 * Input:
 *      unit    - unit id
 *      pPacket - pointer buffer to the packet 
 *      pCookie - cookie data buffer
 * Output:
 *      None
 * Return:
 *      NIC_RX_HANDLED_OWNED
 *      NIC_RX_NOT_HANDLED
 * Note:
 *      None
 */ 
static drv_nic_rx_t rtnic_rx_callback(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie)
{
    struct net_device *pNdev = (struct net_device *)pCookie;
    struct rtnic_priv *pPriv = netdev_priv(pNdev);
    struct sk_buff *pSkb;
    int32  retval;
#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
    uint32 chip_id, chip_rev_id, chip_family_id;
#endif


    if (NULL == pPacket)
    {
        osal_printf("Error: pPacket is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        goto _exit;
    }

    pSkb = (struct sk_buff *)(pPacket->buf_id);
    if (NULL == pSkb)
    {
        osal_printf("Error: skb is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        goto _exit;
    }

    /* save the packet info into some fields */
    pSkb->data = pPacket->data;
    pSkb->tail = pPacket->tail;
    pSkb->len = pPacket->length;
    if (pPriv)
    {
        pPriv->ndev_stats.rx_packets++;
        pPriv->ndev_stats.rx_bytes += pSkb->len;
    }

    pSkb->dev = pNdev;
    pSkb->protocol = eth_type_trans(pSkb, pNdev);
    pSkb->ip_summed = CHECKSUM_UNNECESSARY;
    pNdev->last_rx = jiffies;

    PKTBUF_FREE(pPacket);


    /* To kernel procotol-stack */
#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
    if (RT_ERR_OK != drv_swcore_cid_get(unit, &chip_id, &chip_rev_id))
        return RT_ERR_FAILED;
    chip_family_id = chip_id & FAMILY_ID_MASK;
    if (chip_family_id == RTL8390_FAMILY_ID || chip_family_id == RTL8350_FAMILY_ID)
    retval = netif_rx_ni(pSkb);
    else
        retval = netif_rx(pSkb);
#else
    retval = netif_rx(pSkb);
#endif

    if (NET_RX_DROP == retval)
    {
        if (pPriv)
        {
            pPriv->ndev_stats.rx_dropped++;
        }
    }

    return NIC_RX_HANDLED_OWNED;

_exit:
    return NIC_RX_NOT_HANDLED;
} /* end of rtnic_rx_callback */

/* Function Name:
 *      rtnic_tx_callback
 * Description:
 *      packet TX callback function 
 * Input:
 *      unit    - unit id
 *      pPacket - pointer buffer to the packet 
 *      pCookie - cookie data buffer
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */ 
static void rtnic_tx_callback(uint32 unit, drv_nic_pkt_t *pPacket, void *pCookie)
{
    struct net_device *pNdev = (struct net_device *)pCookie;
    struct rtnic_priv *pPriv = netdev_priv(pNdev);
    struct sk_buff *pSkb;

    if (NULL == pPacket)
    {
        osal_printf("Error: pPacket is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        goto _exit;
    }

    pSkb = (struct sk_buff *)(pPacket->buf_id);
    if (NULL == pSkb)
    {
        osal_printf("Error: skb is NULL at %s():%d\n", __FUNCTION__, __LINE__);
        PKTBUF_FREE(pPacket);
        goto _exit;
    }

    if (pPriv)
    {
        pPriv->ndev_stats.tx_packets++;
        pPriv->ndev_stats.tx_bytes += pSkb->len;
    }

    dev_kfree_skb_irq(pSkb);

    PKTBUF_FREE(pPacket);

_exit:
    netif_wake_queue(pNdev);    
    return;
} /* end of rtnic_tx_callback */

/* Function Name:
 *      rtnic_open
 * Description:
 *      open function of the kernel module 
 * Input:
 *      pNdev - net_device struct handled by the kernel  
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */ 
static int32 rtnic_open(struct net_device *pNdev)
{
    struct rtnic_priv *pPriv = netdev_priv(pNdev);

    if (netif_msg_ifup(pPriv))
    {
        osal_printf("%s: enabling interface.\n", pNdev->name);
    }

    netif_start_queue(pNdev);
    drv_nic_rx_start(RTNIC_UNIT_ID);

    return RT_ERR_OK;
} /* end of rtnic_open */

/* Function Name:
 *      rtnic_close
 * Description:
 *      close function of the kernel module
 * Input:
 *      pNdev - net_device struct handled by the kernel  
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */ 
static int32 rtnic_close(struct net_device *pNdev)
{
    struct rtnic_priv *pPriv = netdev_priv(pNdev);

    if (netif_msg_ifdown(pPriv))
    {
        osal_printf("%s: disabling interface.\n", pNdev->name);
    }

    drv_nic_rx_stop(RTNIC_UNIT_ID);
    netif_stop_queue(pNdev);

    return RT_ERR_OK;
} /* end of rtnic_close */

/* Function Name:
 *      rtnic_get_stats
 * Description:
 *      get the stats of the net device
 * Input:
 *      pNdev - net_device struct handled by the kernel  
 * Output:
 *      None
 * Return:
 *      net_device_stats structure
 * Note:
 *      None
 */ 
static struct net_device_stats *rtnic_get_stats(struct net_device *pNdev)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,19))
    struct rtnic_priv *pPriv = pNdev->priv;
#endif
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
	struct rtnic_priv *pPriv = (struct rtnic_priv *)netdev_priv(pNdev);	
#endif
    return (&pPriv->ndev_stats);
} /* end of rtnic_get_stats */

/* Function Name:
 *      rtnic_start_xmit
 * Description:
 *      Tx start function of the kernel module
 * Input:
 *      pSkb  - raw packet data
 *      pNdev - net_device struct handled by the kernel  
 * Output:
 *      None
 * Return:
 *      NET_XMIT_SUCCESS
 *      NET_XMIT_DROP
 * Note:
 *      None
 */ 
static int32 rtnic_start_xmit(struct sk_buff *pSkb, struct net_device *pNdev)
{
    struct rtnic_priv *pPriv = netdev_priv(pNdev);
    drv_nic_pkt_t *pPacket;

    /* stopping send other packet */
    netif_stop_queue(pNdev);

    pPacket = PKTBUF_ALLOC(sizeof(drv_nic_pkt_t));
    if (NULL == pPacket)
    {
        pPriv->ndev_stats.tx_errors++;
        dev_kfree_skb(pSkb);
        osal_printf("Error: Out of memory at %s():%d\n", __FUNCTION__, __LINE__);
        goto xmit_exit;
    }
	memset(pPacket, 0, sizeof(drv_nic_pkt_t));

    /* setting the CPU tx tag */
    pPacket->as_txtag = 0;

    /* raw packet */
    pPacket->buf_id = (void *)pSkb;
    pPacket->head = pSkb->head;
    pPacket->data = pSkb->data;
    pPacket->tail = pSkb->data + pSkb->len;
    pPacket->end = pSkb->end;
    pPacket->length = pSkb->len;
    pPacket->next = NULL;

    if (RT_ERR_OK == drv_nic_pkt_tx(0, pPacket, rtnic_tx_callback, (void *)pNdev))
    {
        pNdev->trans_start = jiffies;
    }
    else
    {
        pPriv->ndev_stats.tx_dropped++;
        PKTBUF_FREE(pPacket);
        dev_kfree_skb(pSkb);
    }

xmit_exit:
    netif_start_queue(pNdev);

    return NETDEV_TX_OK;
} /* end of rtnic_start_xmit */

/* Function Name:
 *      rtnic_tx_timeout
 * Description:
 *      Tx timeout function of the kernel module
 * Input:
 *      pNdev - net_device struct handled by the kernel  
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */ 
static void rtnic_tx_timeout(struct net_device *pNdev)
{
    osal_printf("Tx Timeout!!! Can't send packet\n");
    return;
} /* end of rtnic_tx_timeout */

/* Function Name:
 *      rtnic_ioctl
 * Description:
 *      ioctl function of the kernel module
 * Input:
 *      pNdev - net_device struct handled by the kernel  
 *      pRq   - interface request structure used for socket ioctl
 *      cmd   - ioctl command type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */ 
static int32 rtnic_ioctl (struct net_device *pNdev, struct ifreq *pRq, int32 cmd)
{    
    return RT_ERR_OK;
} /* end of rtnic_ioctl */

/* Function Name:
 *      rtnic_set_multicast_list
 * Description:
 *      set multicast list
 * Input:
 *      pNdev - net_device struct handled by the kernel   
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */ 
static void rtnic_set_multicast_list(struct net_device *pNdev)
{
    return;
} /* end of rtnic_set_multicast_list */

/* Function Name:
 *      rtnic_set_mac_address
 * Description:
 *      set mac address
 * Input:
 *      pNdev - net_device struct handled by the kernel   
 *      ptr   - data buffer
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 * Note:
 *      None
 */ 
static int32 rtnic_set_mac_address(struct net_device *pNdev, void *ptr)
{
    struct rtnic_priv *pPriv = netdev_priv(pNdev);
    unsigned long flags;
    struct sockaddr *sa = ptr;
    uint32 i;

    spin_lock_irqsave(&pPriv->lock, flags);

    for (i = 0; i < 6; ++i)
        pNdev->dev_addr[i] = sa->sa_data[i];

    spin_unlock_irqrestore(&pPriv->lock, flags);

    drv_swcore_sysMac_set(sa->sa_data);

    return RT_ERR_OK;
} /* end of rtnic_set_mac_address */

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
const struct net_device_ops rtnic_netdev_ops = {
	.ndo_open = rtnic_open,
	.ndo_stop = rtnic_close,	
	.ndo_do_ioctl = rtnic_ioctl,
	.ndo_tx_timeout = rtnic_tx_timeout,
	.ndo_start_xmit = rtnic_start_xmit,
	.ndo_set_multicast_list = rtnic_set_multicast_list,
	.ndo_set_mac_address = rtnic_set_mac_address,
	.ndo_get_stats = rtnic_get_stats,
};	

#if 0
static void rtnic_dev_setup(struct net_device *dev)
{
	printk("\nrtnic_dev_setup()\n");

}
#endif
#endif


/* Function Name:
 *      rtnic_init
 * Description:
 *      Init rtnic driver 
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      -ENOMEM
 *      -EIO
 * Note:
 *      None
 */
static int32 __init rtnic_init(void)
{
    struct net_device *pNdev;
    struct rtnic_priv *pPriv;
    drv_nic_initCfg_t initCfg;

#if defined(__RTNIC_MODULE__)
    osal_printf(KERN_INFO"Init RTNIC Driver Module....");
#endif

    pNdev = alloc_etherdev(sizeof(struct rtnic_priv));
    if (NULL == pNdev)
    {
#if defined(__RTNIC_MODULE__)
        osal_printf("FAIL ");
#endif
        osal_printf("#cause: Out of Memory!!\n");
        return -ENOMEM;
    }

    pPriv = netdev_priv(pNdev);
    memset(pPriv, 0, sizeof(struct rtnic_priv));
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,19))	
    SET_MODULE_OWNER(pNdev);
    pPriv->pNdev = pNdev;
    pNdev->open = rtnic_open;
    pNdev->stop = rtnic_close;
    pNdev->do_ioctl = rtnic_ioctl;
    pNdev->watchdog_timeo = RTNIC_TX_TIMEOUT;
    pNdev->tx_timeout = rtnic_tx_timeout;
    pNdev->hard_start_xmit = rtnic_start_xmit;
    pNdev->set_multicast_list = rtnic_set_multicast_list;
    pNdev->set_mac_address = rtnic_set_mac_address;
    pNdev->get_stats = rtnic_get_stats;
#endif
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))	
	pPriv->pNdev = pNdev;
	pNdev->netdev_ops = &rtnic_netdev_ops;
	pNdev->watchdog_timeo = RTNIC_TX_TIMEOUT;
#endif
    pNdev->irq = rtk_dev[RTK_DEV_NIC].irq;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,19))		
    drv_swcore_sysMac_get((uint8 *)&pNdev->dev_addr);
    //memcpy(pNdev->dev_addr, mac, 6);
#endif    
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))	    
    drv_swcore_sysMac_get((uint8 *)&gNIC_ADDRESS);
    pNdev->dev_addr = (unsigned char *)&gNIC_ADDRESS;
	pNdev->rtnl_link_ops = NULL;	
#endif	
    spin_lock_init(&pPriv->lock);

    if (register_netdev(pNdev) != 0)
    {
#if defined(__RTNIC_MODULE__)
        osal_printf("FAIL ");
#endif
        osal_printf("#cause: Couldn't Register The Device!!\n");
        goto out;
    }

    /* NIC Initialization */
    initCfg.pkt_alloc = rtnic_pkt_alloc;
    initCfg.pkt_free = rtnic_pkt_free;
    initCfg.pkt_size = RTNIC_MAX_PKTLEN;

    if (RT_ERR_OK != drv_nic_init(RTNIC_UNIT_ID, &initCfg))
    {
#if defined(__RTNIC_MODULE__)
        osal_printf("FAIL ");
#endif
        osal_printf("#cause: drv_nic_init() failed!!\n");
        goto out;
    }

    /* Register NIC Rx Handler */
    if (RT_ERR_OK != drv_nic_rx_register(RTNIC_UNIT_ID, RTNIC_RX_HANDLER_PRIORITY, rtnic_rx_callback, (void *)pNdev, 0))
    {
#if defined(__RTNIC_MODULE__)
        osal_printf("FAIL ");
#endif
        osal_printf("#cause: NIC Rx Handler Registration Failed!!\n");
        goto out;
    }

#if defined(__RTNIC_MODULE__)
    osal_printf("OK\n");
#endif
    return RT_ERR_OK;

out:
    free_netdev(pNdev);
#ifndef __RTNIC_MODULE__
    osal_printf(KERN_INFO"Init Switch Ethernet Driver....FAIL\n");
#endif

    return -EIO;
} /* end of rtnic_init */

/* Function Name:
 *      rtnic_exit
 * Description:
 *      Exit rtnic driver
 * Input:
 *      None 
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void __exit rtnic_exit(void)
{

#if defined(__RTNIC_MODULE__)
    osal_printf(KERN_INFO"Exit RTNIC Driver Module....OK\n");
#endif
    return;
} /* end of rtnic_exit */

module_init(rtnic_init);
module_exit(rtnic_exit);

MODULE_DESCRIPTION ("Switch SDK Ethernet Driver Module");
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,19))
MODULE_LICENSE("GPL");
#endif

