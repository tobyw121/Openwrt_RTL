/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 15712 $
 * $Date: 2011-02-11 16:12:24 +0800 (Fri, 11 Feb 2011) $
 *
 * Purpose : Definition those public NIC(Network Interface Controller) APIs and
 *           its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *            1) CPU tag
 *            2) NIC tx
 *            3) NIC rx
 *
 */

/*
 * Include Files
 */
#include <dev_config.h>
#include <soc/soc.h>
#include <soc/type.h>
#include <common/debug/rt_log.h>
#include <ioal/mem32.h>
#include <osal/isr.h>
#include <osal/cache.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/print.h>
#include <osal/thread.h>
#include <osal/time.h>
#include <osal/sem.h>
#include <osal/spl.h>
#include <drv/nic/r8390.h>
#include <drv/swcore/l2notification.h>
#include <drv/swcore/rtl8390.h>
#include <drv/nic/nic.h>
#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
#include <osal/wait.h>
#include <osal/workqueue.h>
#endif

/*
 * Symbol Definition
 */
#define READ32(ptr) ((*(ptr) << 24) | (*(ptr + 1) << 16) | (*(ptr + 2) << 8) | *(ptr + 3))
#define READ16(ptr) ((*(ptr) << 8) | (*(ptr + 1)))

#define NIC_CODE_REDUCE

/* CPUIISR - CPU Interface Interrupt Status Register */
#define INT_RX_RUN_OUT7_0_OFFSET             (0)                                         /* Interrupt of RX_RUN_OUT running out status, write '1' to clear.     */
#define INT_RX_RUN_OUT7_0_MASK               (0xFFU << INT_RX_RUN_OUT7_0_OFFSET)        /* Interrupt of RX_RUN_OUT running out status, write '1' to clear.     */
#define INT_TX_DONE1_INT_TX_DONE0_OFFSET       (16)                                         /* Interrupt of EN_TX_DONE pending status, write '1' to clear.     */
#define INT_TX_DONE1_INT_TX_DONE0_MASK         (0x3U << INT_TX_DONE1_INT_TX_DONE0_OFFSET)   /* Interrupt of EN_TX_DONE pending status, write '1' to clear.     */
#define INT_TX_DONE1_OFFSET                    (17)
#define INT_TX_DONE1_MASK                      (0x1U << INT_TX_DONE1_OFFSET)
#define INT_TX_DONE0_OFFSET                    (16)
#define INT_TX_DONE0_MASK                      (0x1U << INT_TX_DONE0_OFFSET)
#define INT_RX_DONE7_INT_RX_DONE0_OFFSET       (8)                                          /* Interrupt of EN_RX_DONE pending status, write '1' to clear.     */
#define INT_RX_DONE7_INT_RX_DONE0_MASK         (0xFFU << INT_RX_DONE7_INT_RX_DONE0_OFFSET)  /* Interrupt of EN_RX_DONE pending status, write '1' to clear.     */
#define INT_RX_DONE7_OFFSET                    (15)
#define INT_RX_DONE7_MASK                      (0x1U << INT_RX_DONE7_OFFSET)
#define INT_RX_DONE6_OFFSET                    (14)
#define INT_RX_DONE6_MASK                      (0x1U << INT_RX_DONE6_OFFSET)
#define INT_RX_DONE5_OFFSET                    (13)
#define INT_RX_DONE5_MASK                      (0x1U << INT_RX_DONE5_OFFSET)
#define INT_RX_DONE4_OFFSET                    (12)
#define INT_RX_DONE4_MASK                      (0x1U << INT_RX_DONE4_OFFSET)
#define INT_RX_DONE3_OFFSET                    (11)
#define INT_RX_DONE3_MASK                      (0x1U << INT_RX_DONE3_OFFSET)
#define INT_RX_DONE2_OFFSET                    (10)
#define INT_RX_DONE2_MASK                      (0x1U << INT_RX_DONE2_OFFSET)
#define INT_RX_DONE1_OFFSET                    (9)
#define INT_RX_DONE1_MASK                      (0x1U << INT_RX_DONE1_OFFSET)
#define INT_RX_DONE0_OFFSET                    (8)
#define INT_RX_DONE0_MASK                      (0x1U << INT_RX_DONE0_OFFSET)
#define INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET (18)                                          /* Interrupt of EN_TX_ALLDONE pending status, write '1' to clear.  */
#define INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK   (0x3U << INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET) /* Interrupt of EN_TX_ALLDONE pending status, write '1' to clear.  */

#define CPUIIMR_ENABLE_MASK                 (INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK | INT_TX_DONE1_INT_TX_DONE0_MASK | INT_RX_DONE7_INT_RX_DONE0_MASK | INT_RX_RUN_OUT7_0_MASK)
#define CPUIIMR_DISABLE_MASK                (0x00000000U)
#define CPUIISR_CLEARALL_MASK               (0xFFFFFFFFU)
#define DMA_IF_CTRL_DISABLE_MASK            (0x00000000U)

/* CTIDCR - CPU Tag ID Control Register */
#define CPU_TAG_ID_OFFSET                   (0) /* CPU tag ID */
#define REALTEK_CPUTAG_ID                   (0x8899)
#define CPU_TAG_ID_LEN                      (2) /* CPU tag ID length: 2 bytes */
#define REALTEK_CPUTAG_PROTOCOL_ID          (4)

/* NIC is not support unit id information in descriptor.
 * Define one symbol (fix 0) now and it will be replace
 * when NIC can bring unit information from descriptor
 */
#define NIC_DEFAULT_UNIT_ID                 (0)
#define DEBUG_DUMP_PKT_LEN                  (256)
#define MEMORY_BARRIER()                    ({ __asm__ __volatile__ ("": : :"memory"); })

/* Set SRAM segment configuration
 */
#define SRAM_SEG_BASE       (0x0000)
#define SRAM_SEG_SIZE_32KB  (0x8)
#define SRAM_SEG_ADDR       (0x10000000)
#define SRAM_DISABLE        (0)
#define SRAM_ENABLE         (1)

#undef NIC_RX_THREAD
#define NIC_RX_THREAD_STACK_SIZE        8192
#define NIC_RX_THREAD_PRI               0
#define NIC_RX_THREAD_QUEUE_LENGTH      2048
#define NIC_RX_THREAD_BURST_MAX         256
#define NIC_RX_THREAD_SLEEP_TIME        10000   /* 10 mS */

#define JUMBO_CLUSTER_NUM               5
#define MAX_NIC_PKT_CNT                 24000

/*
 * Data Type Definition
 */
typedef struct nic_rx_cb_entry_s
{
    drv_nic_rx_cb_f rx_callback;
    void *pCookie;
} nic_rx_cb_entry_t;

typedef struct nic_pkthdr_s
{
    uint8 *buf_addr;
#ifdef __LITTLE_ENDIAN
    /* word [0] */
    uint16  buf_size;
    uint16  reserve;
    /* word [1] */
    uint16  buf_len;
    uint16  pkt_offset:14;
    uint16      :1;
    uint16  more:1;
#else
    /* word [0] */
    uint16  reserve;
    uint16  buf_size;
    /* word [1] */
    uint16  more:1;
    uint16      :1;
    uint16  pkt_offset:14;
    uint16  buf_len;
    uint16  reserve2;
    nic_cpuTag_t    cpuTag;
#endif

    /* Used by Software */
    struct drv_nic_pkt_s *packet;
    uint32  *ring_entry;
    drv_nic_tx_cb_f tx_callback;
    void    *cookie;
} nic_pkthdr_t;

typedef struct nic_info_s
{
    uint32          rxFDPBase[NIC_RXRING_NUM][NIC_RXRING_SIZE];
    uint32          txFDPBase[NIC_TXRING_NUM][NIC_TXRING_SIZE];
    nic_pkthdr_t    rxPktHdr[NIC_RX_PKTHDR_NUM];
    nic_pkthdr_t    txPktHdr[NIC_TX_PKTHDR_NUM];
} nic_info_t;

#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
typedef struct nic_collectArrayList_s
{
    uint32                  unit;
    osal_list_head_t        list;
    struct drv_nic_pkt_s    *pPacket;
    uint8                   jumbo;
} nic_collectArrayList_t;
#endif


/*
 * Data Declaration
 */
static uint32   nic_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static nic_rx_cb_entry_t _nic_rx_cb_tbl[NIC_RX_CB_PRIORITY_NUMBER];
static drv_nic_initCfg_t _nic_init_conf;
#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
static nic_collectArrayList_t       nicListHead;
static osal_wait_queue_head_t       wq;
static atomic_t                     rxFlag;
static spinlock_t                   irqLock;
#endif

/* Pointer for Software */
static nic_info_t   *pNicInfo;
static uint32       *pNic_rxFDPBase[NIC_RXRING_NUM];
static uint32       *pNic_txFDPBase[NIC_TXRING_NUM];
static uint32       *pNic_rxCDPIdx[NIC_RXRING_NUM];
static uint32       *pNic_txCDPIdx[NIC_TXRING_NUM];
static uint32       *pNic_rxRDPIdx[NIC_RXRING_NUM];
static uint32       *pNic_txRDPIdx[NIC_TXRING_NUM];

/* NIC Tx/Rx debug information
 * The machanism is always enabled
 */
static uint32       nic_debug_flag;
static uint32       nic_tx_success_cntr;
static uint32       nic_tx_failed_cntr;
static uint32       nic_rx_success_cntr;
static uint32       nic_rx_failed_cntr;
static uint32       nic_tx_isr_cntr;
static uint32       nic_tx_ring_cntr;


static uint32       cpuTagId;
static uint32		rxCRCInclude;

#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
static atomic_t nic_rx_pkt_cnt = ATOMIC_INIT(0);
static atomic_t nic_mem_lock = ATOMIC_INIT(0);
static uint32   nicLockCnt, nicReleaseCnt;
#endif


static uint32 		reasonTbl[][2] = 
{
	{0, 										0, },
	{NIC_RX_REASON_OAM,							0, },
	{NIC_RX_REASON_CFM,							0, },
	{NIC_RX_REASON_CFM_ETHDM,					0, },
	{NIC_RX_REASON_IGR_VLAN_FILTER,				0, },
	{NIC_RX_REASON_VLAN_ERROR,					0, },
	{NIC_RX_REASON_INNER_OUTTER_CFI,			0, },
	{NIC_RX_REASON_RMA_USR_DEF1,				NIC_RX_REASON_RMA, },
	{NIC_RX_REASON_RMA_USR_DEF2,				NIC_RX_REASON_RMA, },
	{NIC_RX_REASON_RMA_BPDU,					NIC_RX_REASON_RMA, },
	{NIC_RX_REASON_RMA_LACP,					NIC_RX_REASON_RMA, },
	{NIC_RX_REASON_RMA_PTP,						NIC_RX_REASON_RMA, },
	{NIC_RX_REASON_RMA_LLDP,					NIC_RX_REASON_RMA, },
	{NIC_RX_REASON_RMA,							0, },
	{NIC_RX_REASON_IP6_HOPBYHOP_EXT_HDR_ERROR,	0, },
	{NIC_RX_REASON_IP6_UNKWN_EXT_HDR,			0, },
	{NIC_RX_REASON_IP4_HDR_ERROR,				0, },
	{NIC_RX_REASON_TTL_EXCEED,					0, },
	{NIC_RX_REASON_IP4_OPTIONS,					0, },
	{NIC_RX_REASON_IP6_HDR_ERROR,				0, },
	{NIC_RX_REASON_HOP_EXCEED,					0, },
	{NIC_RX_REASON_IP6_HOPBYHOP_OPTION,			0, },
	{NIC_RX_REASON_GW_MAC_ERROR,				0, },
	{NIC_RX_REASON_IGMP,						NIC_RX_REASON_SPECIAL_TRAP, },
	{NIC_RX_REASON_MLD,							NIC_RX_REASON_SPECIAL_TRAP, },
	{NIC_RX_REASON_EAPOL,						NIC_RX_REASON_SPECIAL_TRAP, },
	{NIC_RX_REASON_ARP_REQ,						0, },
	{NIC_RX_REASON_IP6_NEIGHBOR_DISCOVER,		0, },
	{NIC_RX_REASON_UNKWN_UCST_MCST,				0, },
	{NIC_RX_REASON_MY_MAC,						0, },
	{NIC_RX_REASON_INVALID_SA,					0, },
	{NIC_RX_REASON_NORMAL_FWD,					0, },
};

/*
 * Macro Definition
 */
static osal_spinlock_t nic_lock;
#define NIC_SEM_LOCK(unit)   osal_spl_spin_lock(&nic_lock)    
#define NIC_SEM_UNLOCK(unit) osal_spl_spin_unlock(&nic_lock)   

/*
 * Function Declaration
 */
static int32 _nic_init(uint32 unit, drv_nic_initCfg_t *pInitCfg);
static int32 _nic_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie);
static int32 _nic_rx_start(uint32 unit);
static int32 _nic_rx_stop(uint32 unit);
static int32 _nic_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags);
static int32 _nic_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb);
#if 0
static int32 _insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, nic_cpuTag_t *pCpuTag);
#endif
static int32 _removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, nic_cpuTag_t *pCpuTag);



#if 0
/* Function Name:
 *      _insertCPUTag
 * Description:
 *      Insert CPU tag to packet.
 * Input:
 *      pPkt     - pointer buffer of this packet
 *      headroom - reserved size ahead packet buffer
 *      pCpuTag  - pointer buffer of cpu tag
 * Output:
 *      ppNewPkt - pointer of new packet head
 *      pOffset  - pointer to offset value after insert cpu tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, nic_cpuTag_t *pCpuTag)
{
    uint8   *pU8;

    if ((NULL == pPkt) || (NULL == ppNewPkt)|| (NULL == pOffset) || (NULL == pCpuTag))
    {
        return RT_ERR_FAILED;
    }

    if (headroom < (CPU_TAG_ID_LEN + sizeof(nic_cpuTag_t)))
    {
        return RT_ERR_FAILED;
    }

    /* copy the packet */
    pU8 = (uint8 *)(pPkt - (CPU_TAG_ID_LEN + sizeof(nic_cpuTag_t)));
    *(pU8 + 0) = *(pPkt + 0);
    *(pU8 + 1) = *(pPkt + 1);
    *(pU8 + 2) = *(pPkt + 2);
    *(pU8 + 3) = *(pPkt + 3);
    *(pU8 + 4) = *(pPkt + 4);
    *(pU8 + 5) = *(pPkt + 5);
    *(pU8 + 6) = *(pPkt + 6);
    *(pU8 + 7) = *(pPkt + 7);
    *(pU8 + 8) = *(pPkt + 8);
    *(pU8 + 9) = *(pPkt + 9);
    *(pU8 + 10) = *(pPkt + 10);
    *(pU8 + 11) = *(pPkt + 11);

    *(pU8 + 12) = ((cpuTagId >> 8) & 0xFF);
    *(pU8 + 13) = (cpuTagId & 0xFF);

    /* cpuTag insert: offset 14 bytes = DA(6) + SA(6)+ CPU_TAG_ID(2) */
    osal_memcpy((uint8 *)(pU8 + 14), (uint8 *)pCpuTag, sizeof(nic_cpuTag_t));

    if (ppNewPkt)
    {
        *ppNewPkt = (uint8 *)pU8;
    }

    if (pOffset)
    {
        *pOffset = (CPU_TAG_ID_LEN + sizeof(nic_cpuTag_t));
    }

    return RT_ERR_OK;
} /* end of _insertCPUTag */
#endif

/* Function Name:
 *      _removeCPUTag
 * Description:
 *      Remove CPU tag from packet.
 * Input:
 *      pPkt - pointer buffer of this packet
 * Output:
 *      ppNewPkt - pointer of new packet head
 *      pOffset  - pointer to offset value after remove cpu tag
 *      pCpuTag  - pointer buffer of cpu tag
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, nic_cpuTag_t *pCpuTag)
{
    if ((NULL == pPkt) || (NULL == ppNewPkt) || (NULL == pOffset) || (NULL == pCpuTag))
        return RT_ERR_FAILED;

    /* cpuTag copy: offset 14 bytes = DA(6) + SA(6)+ CPU_TAG_ID(2) */
    osal_memcpy((uint8 *)pCpuTag, (uint8 *)(pPkt + 14), sizeof(nic_cpuTag_t));

    /* fix the packet */
    *(pPkt + 19) = *(pPkt + 11);
    *(pPkt + 18) = *(pPkt + 10);
    *(pPkt + 17) = *(pPkt + 9);
    *(pPkt + 16) = *(pPkt + 8);
    *(pPkt + 15) = *(pPkt + 7);
    *(pPkt + 14) = *(pPkt + 6);
    *(pPkt + 13) = *(pPkt + 5);
    *(pPkt + 12) = *(pPkt + 4);
    *(pPkt + 11) = *(pPkt + 3);
    *(pPkt + 10) = *(pPkt + 2);
    *(pPkt + 9)  = *(pPkt + 1);
    *(pPkt + 8)  = *(pPkt + 0);

    if (ppNewPkt)
    {
        *ppNewPkt = (pPkt + (CPU_TAG_ID_LEN + sizeof(nic_cpuTag_t)));
    }

    if (pOffset)
    {
        *pOffset = (CPU_TAG_ID_LEN + sizeof(nic_cpuTag_t));
    }

    return RT_ERR_OK;
} /* end of _removeCPUTag */

static int32 _nic_rx_reason_translate(drv_nic_pkt_t *pPacket)
{
#define RTL8390_MAX_REASON_NM  31
	uint16 reason = pPacket->rx_tag.reason;

	if (pPacket->rx_tag.acl_hit)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_ACL_HIT);
	if (pPacket->rx_tag.mac_cst)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_MAC_CONSTRAINT);
	if (pPacket->rx_tag.new_sa)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_NEW_SA);
	if (pPacket->rx_tag.l2_pmv)
        NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_L2_PMV);
	if (pPacket->rx_tag.atk_type)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_ATTACK);
	if (pPacket->rx_tag.mirror_hit)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_MIRROR);


	if (reason <= RTL8390_MAX_REASON_NM)
	{
		NIC_REASON_MASK_SET(*pPacket, reasonTbl[reason][0]);
		NIC_REASON_MASK_SET(*pPacket, reasonTbl[reason][1]);
	}

    return RT_ERR_OK;
}

/* Function Name:
 *      _nic_isr_rxRoutine
 * Description:
 *      Interrupt handle routine for progress nic rx done.
 * Input:
 *      ringId - rx ring id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      tx ring id valid range is 0 to 7
 */
static int32
_nic_isr_rxRoutine(uint32 ringId)
{
    uint32  temp;
    int32   ret = RT_ERR_FAILED;
    uint32  releaseCnt = 0;
#ifndef CONFIG_SDK_NIC_RX_CB_IN_THREAD
    uint32  i;
#endif
    drv_nic_rx_t nic_rx_handle = NIC_RX_NOT_HANDLED;
	static uint32 jumboFlag;
    static    drv_nic_pkt_t   *pPacket;

    RT_LOG(LOG_DEBUG, MOD_NIC, "ringId = %d", ringId);

    if (ringId >= NIC_RXRING_NUM)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "invalid ringId(%d)!", ringId);
        nic_rx_failed_cntr++;
        return RT_ERR_FAILED;
    }

    /* Update software current pointer */
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_RX_CUR_DESC_ADDR_CTRL_ADDR(ringId), &temp);
    pNic_rxCDPIdx[ringId] = (uint32 *)temp;    /* The limitation */


    do
    {
        uint32          offset = 0, scan_offset = 0;
        uint8           pkt_data;
        uint8           *pData;
		uint8			jumbo_first_cluster = 0;
        uint32          ethType;
        nic_pkthdr_t    *pktHdr;
        nic_cpuTag_t    *pCputag;
#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
        uint32          value;
        nic_collectArrayList_t  *pEntry;
#endif

        if ((*pNic_rxRDPIdx[ringId] & NIC_RING_SWOWNBIT) != 0)
            break;

        /* Prepare to be reclaim */
        pktHdr = (nic_pkthdr_t *)(*pNic_rxRDPIdx[ringId] & NIC_ADDR_MASK);
        if (NULL == pktHdr || NULL == pktHdr->buf_addr || NULL == pktHdr->packet)
            break;


#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
        if (!jumboFlag)
        {
            if (atomic_read(&nic_mem_lock))
                return RT_ERR_OK;
            if ((value = atomic_read(&nic_rx_pkt_cnt)) + 1 > MAX_NIC_PKT_CNT)
            {
                atomic_set(&nic_mem_lock, 1);
                ioal_mem32_read(0, RTL8390_DMA_IF_INTR_MSK_ADDR, &value);
                value &= (~RTL8390_DMA_IF_INTR_MSK_RX_DONE_MASK) & (~RTL8390_DMA_IF_INTR_MSK_RX_RUN_OUT_MASK);
                ioal_mem32_write(0, RTL8390_DMA_IF_INTR_MSK_ADDR, value);
                nicLockCnt++;
                return RT_ERR_OK;
            }

            if (pktHdr->more)
                atomic_add_return(JUMBO_CLUSTER_NUM, &nic_rx_pkt_cnt);
            else
            {
                atomic_add_return(1, &nic_rx_pkt_cnt);
            }
        }
#endif

        /* NIC Rx debug message */
        if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
        {
            int i;
            int dump_len = 1600; /* debug dump maximum length */

            osal_printf("=== [NIC RX Debug] ================================= Len: %d \n", pktHdr->buf_len);
            for (i = 0; i < dump_len; i++)
            {
                if (i == (pktHdr->buf_len))
                    break;
                if (0 == (i % 16))
                    osal_printf("[%04X] ", i);
                osal_printf("%02X ", *(pktHdr->packet->data + i));
                if (15 == (i % 16))
                    osal_printf("\n");
            }
            osal_printf("\n");
        }


		if (pktHdr->more && jumboFlag == FALSE)		/* Jumbo head */
		{
			jumboFlag = TRUE;
			jumbo_first_cluster = TRUE;
			if (RT_ERR_FAILED == _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, 9000, 0, &pPacket))
			{
				jumboFlag = FALSE;
				jumbo_first_cluster = FALSE;
				RT_LOG(LOG_DEBUG, MOD_NIC, "Out of memory ! (alloc a new packet data buffer failed)");
				break;
			}
			osal_memcpy(pPacket->data, pktHdr->buf_addr, pktHdr->buf_len);
			pPacket->length = pktHdr->buf_len;
		}
		else if (jumboFlag == TRUE)		/* Jumbo other */
		{
			osal_memcpy(pPacket->data + pPacket->length, pktHdr->buf_addr, pktHdr->buf_len);
			pPacket->length += pktHdr->buf_len;
	        pPacket->tail = pPacket->data + pPacket->length;
		}
		else		/* Normal packet */
		{
	        pPacket = pktHdr->packet;
	        pPacket->length = pktHdr->buf_len;
	        pPacket->tail = pPacket->data + pPacket->length;
		}


		if (jumbo_first_cluster || jumboFlag == 0)
		{
	        pCputag = &pktHdr->cpuTag;
	        if (pCputag->un.rx.CPUTAGIF)
	        {
	            pPacket->rx_tag.source_port     = pCputag->un.rx.SPN;
	            pPacket->rx_tag.mirror_hit      = pCputag->un.rx.MIR_HIT;
	            pPacket->rx_tag.acl_hit        	= pCputag->un.rx.ACL_HIT;
	            pPacket->rx_tag.acl_index       = pCputag->un.rx.ACL_IDX;
	            pPacket->rx_tag.svid_tagged     = pCputag->un.rx.OTAGIF;
	            pPacket->rx_tag.cvid_tagged     = pCputag->un.rx.ITAGIF;
	            pPacket->rx_tag.fvid            = pCputag->un.rx.RVID;
	            pPacket->rx_tag.qid             = pCputag->un.rx.QID;
	            pPacket->rx_tag.atk_type        = pCputag->un.rx.ATK_TYPE;
	            pPacket->rx_tag.mac_cst         = pCputag->un.rx.MAC_CST;
	            pPacket->rx_tag.sflow           = pCputag->un.rx.SFLOW;
	            pPacket->rx_tag.dm_rxIdx        = pCputag->un.rx.DM_RXIDX;
	            pPacket->rx_tag.new_sa          = pCputag->un.rx.NEW_SA;
	            pPacket->rx_tag.l2_pmv          = pCputag->un.rx.L2_PMV;
	            pPacket->rx_tag.crc             = pCputag->un.rx.CRC;
	            pPacket->rx_tag.reason          = pCputag->un.rx.REASON;
				_nic_rx_reason_translate(pPacket);
	        }
	        else
	        {
	            nic_cpuTag_t cputag;

	            scan_offset = 12;
	            pData = pPacket->data + scan_offset;
	            ethType = READ16(pData);
	            pkt_data = *(pData + 2);

	            if (ethType == cpuTagId && pkt_data == 0x04)    /* embedded CPU Tag */
	            {
	                if (RT_ERR_OK != _removeCPUTag(pPacket->data, &pPacket->data, &offset, &cputag))
	                {
	                    nic_rx_failed_cntr++;
	                    return RT_ERR_FAILED;
	                }

	                pPacket->rx_tag.source_port     = cputag.un.rx.SPN;
	                pPacket->rx_tag.mirror_hit      = cputag.un.rx.MIR_HIT;
	                pPacket->rx_tag.acl_hit        	= pCputag->un.rx.ACL_HIT;
	                pPacket->rx_tag.acl_index       = pCputag->un.rx.ACL_IDX;
	                pPacket->rx_tag.svid_tagged     = cputag.un.rx.OTAGIF;
	                pPacket->rx_tag.cvid_tagged     = cputag.un.rx.ITAGIF;
	                pPacket->rx_tag.fvid            = cputag.un.rx.RVID;
	                pPacket->rx_tag.qid             = cputag.un.rx.QID;
	                pPacket->rx_tag.atk_type        = cputag.un.rx.ATK_TYPE;
	                pPacket->rx_tag.mac_cst         = cputag.un.rx.MAC_CST;
	                pPacket->rx_tag.sflow           = cputag.un.rx.SFLOW;
	                pPacket->rx_tag.dm_rxIdx        = cputag.un.rx.DM_RXIDX;
	                pPacket->rx_tag.new_sa          = cputag.un.rx.NEW_SA;
	                pPacket->rx_tag.l2_pmv          = cputag.un.rx.L2_PMV;
	                pPacket->rx_tag.crc             = cputag.un.rx.CRC;
	                pPacket->rx_tag.reason          = cputag.un.rx.REASON;
					_nic_rx_reason_translate(pPacket);
	            }
	        }

	        scan_offset = 12;
	        if (pPacket->rx_tag.svid_tagged)
	        {
	            pkt_data = *(pPacket->data + scan_offset + 2);
	            pPacket->rx_tag.outer_pri = (pkt_data >> 5) & 0x7;
	            pPacket->rx_tag.outer_vid = ((pkt_data & 0xF) << 8);
	            pkt_data = *(pPacket->data + scan_offset + 3);
	            pPacket->rx_tag.outer_vid |= (pkt_data & 0xFF);
	            scan_offset += 4;
	        }
	        else
	        {
	             pPacket->rx_tag.outer_pri = 0;
	             pPacket->rx_tag.outer_vid = 0;
	        }
	        if (pPacket->rx_tag.cvid_tagged)
	        {
	            pkt_data = *(pPacket->data + scan_offset + 2);
	            pPacket->rx_tag.inner_pri = (pkt_data >> 5) & 0x7;
	            pPacket->rx_tag.inner_vid = ((pkt_data & 0xF) << 8);
	            pkt_data = *(pPacket->data + scan_offset + 3);
	            pPacket->rx_tag.inner_vid |= (pkt_data & 0xFF);
	        }
	        else
	        {
	             pPacket->rx_tag.inner_pri = 0;
	             pPacket->rx_tag.inner_vid = 0;
	        }
	        pPacket->length -= offset;

	        /* NIC Rx debug message */
	        if (nic_debug_flag & DEBUG_RX_CPU_TAG_BIT)
	        {
	            osal_printf("=== [NIC RX Debug - CPU Rx Tag Information] ============ \n");
	            osal_printf(" SPN : %d \n", pPacket->rx_tag.source_port);
	            osal_printf(" MIR_HIT : %d \n", pPacket->rx_tag.mirror_hit);
	            osal_printf(" ACL_HIT : %d \n", pPacket->rx_tag.acl_hit);
	            osal_printf(" ACL_IDX : %d \n", pPacket->rx_tag.acl_index);
	            osal_printf(" OTAGIF : %d \n", pPacket->rx_tag.svid_tagged);
	            osal_printf(" ITAGIF : %d \n", pPacket->rx_tag.cvid_tagged);
	            osal_printf(" OVID : %d \n", pPacket->rx_tag.outer_vid);
	            osal_printf(" IVID : %d \n", pPacket->rx_tag.inner_vid);
	            osal_printf(" FVID : %d \n", pPacket->rx_tag.fvid);
	            osal_printf(" QID : %d \n", pPacket->rx_tag.qid);
	            osal_printf(" ATK_TYPE : %d \n", pPacket->rx_tag.atk_type);
	            osal_printf(" MAC_CST : %d \n", pPacket->rx_tag.mac_cst);
	            osal_printf(" SFLOW : %d \n", pPacket->rx_tag.sflow);
	            osal_printf(" DM_RXIDX : %d \n", pPacket->rx_tag.dm_rxIdx);
	            osal_printf(" NEW_SA : %d \n", pPacket->rx_tag.new_sa);
	            osal_printf(" L2_PMV : %d \n", pPacket->rx_tag.l2_pmv);
	            osal_printf(" REASON : %d \n", pPacket->rx_tag.reason);
            	osal_printf(" CRC : %d \n", pPacket->rx_tag.crc);
	        }
		}


		if (jumboFlag == 0 || (jumboFlag && pktHdr->more == 0))
		{
	        nic_rx_handle = NIC_RX_NOT_HANDLED;
			if (0 == rxCRCInclude)
			{
				/* packet passed to higher layer doesn't need CRC field */
				pPacket->length -= 4;
				pPacket->tail -= 4;
			}


#if !defined(CONFIG_SDK_NIC_RX_CB_IN_THREAD)
	        for (i = 0; i < NIC_RX_CB_PRIORITY_NUMBER; i++)
	        {
	            if (_nic_rx_cb_tbl[i].rx_callback != NULL)
	            {

	                nic_rx_handle = _nic_rx_cb_tbl[i].rx_callback(NIC_DEFAULT_UNIT_ID, pPacket, _nic_rx_cb_tbl[i].pCookie);
	                if (NIC_RX_HANDLED_OWNED == nic_rx_handle)
	                {
	                    break;
	                }
	            }
	        }

			if (nic_rx_handle != NIC_RX_HANDLED_OWNED)
	        {   /* We have to free this packet here */
	            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pPacket);
	        }
#else
            pEntry = osal_alloc(sizeof(nic_collectArrayList_t));
            pEntry->pPacket = pPacket;
            if (jumboFlag && pktHdr->more == 0)
                pEntry->jumbo = 1;
            else
            {
                pEntry->jumbo = 0;
            }
            osal_list_add_tail(&pEntry->list, &nicListHead.list);
#endif
		}

		if (jumboFlag == 0)
		{
	        pktHdr->packet = NULL;
	        /* Alloc a new packet data buffer */
	        if (RT_ERR_OK == _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
	        {
	            pktHdr->packet = pPacket;
	            pktHdr->buf_addr = (uint8 *)(UNCACHE(pPacket->data));
	            pktHdr->buf_size = _nic_init_conf.pkt_size;
	            pktHdr->buf_len = 0;
	            if ((ret = osal_cache_memory_flush((uint32)pPacket->head, (pPacket->end - pPacket->head))) != RT_ERR_OK)
	            {
	                return ret;
	            }

	            MEMORY_BARRIER();
	            *(pktHdr->ring_entry) |= NIC_RING_SWOWNBIT;    /* Only set the SwOwn bit */
	            releaseCnt++;

#ifndef NIC_CODE_REDUCE
	            /* To guarantee it's write done */
	            do
	            {
	                uint32 chk;
	                chk = *(pktHdr->ring_entry);
	            } while (0);
#endif
	        }
		}
		else
		{
			//osal_memset(pktHdr->packet->data, 0, _nic_init_conf.pkt_size);
	        MEMORY_BARRIER();
			pktHdr->buf_len = 0;
			*(pktHdr->ring_entry) |= NIC_RING_SWOWNBIT;    /* Only set the SwOwn bit */
	        releaseCnt++;

#ifndef NIC_CODE_REDUCE
            /* To guarantee it's write done */
            do
            {
                uint32 chk;
                chk = *(pktHdr->ring_entry);
            } while (0);
#endif
		}

		if (jumboFlag && pktHdr->more == 0)
		{
			jumboFlag = FALSE;
		}


        /* Jump to next */
        pNic_rxRDPIdx[ringId] += 1;
        if (pNic_rxRDPIdx[ringId] == (pNic_rxFDPBase[ringId] + NIC_RXRING_SIZE))
            pNic_rxRDPIdx[ringId] = pNic_rxFDPBase[ringId];

		if (jumboFlag == 0 || (jumboFlag && pktHdr->more == 0))
	        nic_rx_success_cntr++;
    } while (pNic_rxRDPIdx[ringId] != pNic_rxCDPIdx[ringId]);

    ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_RX_RING_CNTR_ADDR(ringId), releaseCnt << RTL8390_DMA_IF_RX_RING_CNTR_CNTR_OFFSET(ringId));

#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
    atomic_set(&rxFlag, 1);
    osal_wake_up(&wq);
#endif

    return RT_ERR_OK;
} /* end of _nic_isr_rxRoutine */


/* Function Name:
 *      _nic_isr_txRoutine
 * Description:
 *      Interrupt handle routine for progress nic tx done.
 * Input:
 *      ringId - tx ring id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      tx ring id valid range is 0 to 1
 */
static int32
_nic_isr_txRoutine(uint32 ringId)
{
    RT_LOG(LOG_DEBUG, MOD_NIC, "ringId = %d", ringId);

    if (ringId >= NIC_TXRING_NUM)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "invalid ringId(%d)!", ringId);
        return RT_ERR_FAILED;
    }

	nic_tx_isr_cntr++;
    do
    {
        nic_pkthdr_t *pktHdr;

        if ((*pNic_txRDPIdx[ringId] & NIC_RING_SWOWNBIT) != 0)
            break;

        /* Prepare to be reclaim */
        pktHdr = (nic_pkthdr_t *)(*pNic_txRDPIdx[ringId] & NIC_ADDR_MASK);
        if (NULL == pktHdr || NULL == pktHdr->buf_addr || NULL == pktHdr->packet)
        {
            break;
        }
        /* Callback Tx CB Function (auto-free the abandoned packet) */
        if (pktHdr->tx_callback == NULL)
        {
            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pktHdr->packet);
        }
        else
        {
            pktHdr->tx_callback(NIC_DEFAULT_UNIT_ID, pktHdr->packet, pktHdr->cookie);
            pktHdr->tx_callback = NULL;
        }
        pktHdr->packet = NULL;

        /* Jump to next */
        pNic_txRDPIdx[ringId] += 1;
		nic_tx_ring_cntr++;
        if (pNic_txRDPIdx[ringId] == (pNic_txFDPBase[ringId] + NIC_TXRING_SIZE))
            pNic_txRDPIdx[ringId] = pNic_txFDPBase[ringId];
    } while (pNic_txRDPIdx[ringId] != pNic_txCDPIdx[ringId]);

    return RT_ERR_OK;
} /* end of _nic_isr_txRoutine */


/* Function Name:
 *      _nic_isr_mbRoutine
 * Description:
 *      Interrupt handle routine for progress nic mBuffer runout.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_nic_isr_mbRoutine(void)
{
    uint32 temp;
    int32  ret = RT_ERR_FAILED;
    int32   i;
    drv_nic_pkt_t *pPacket;



    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        /* Update software current pointer */
        ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_RX_CUR_DESC_ADDR_CTRL_ADDR(i), &temp);
        pNic_rxCDPIdx[i] = (uint32 *)temp;
        do
        {
            nic_pkthdr_t *pPktHdr;

            if ((*pNic_rxRDPIdx[i] & NIC_RING_SWOWNBIT) != 0)
                break;

            /* Prepare to be reclaim */
            pPktHdr = (nic_pkthdr_t *)(*pNic_rxRDPIdx[i] & NIC_ADDR_MASK);
            if (NULL == pPktHdr || pPktHdr->packet != NULL)
                break;

            /* Alloc a new packet data buffer */
            if (RT_ERR_OK != _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
            {
                RT_LOG(LOG_DEBUG, MOD_NIC, "Out of memory ! (alloc a new packet data buffer failed)");
                break;
            }

            pPktHdr->packet = pPacket;
            pPktHdr->buf_addr = (uint8 *)(UNCACHE(pPacket->data));
            pPktHdr->buf_size = _nic_init_conf.pkt_size;
            pPktHdr->buf_len = 0;
            if ((ret = osal_cache_memory_flush((uint32)pPacket->head, (pPacket->end - pPacket->head))) != RT_ERR_OK)
            {
                return ret;
            }
            MEMORY_BARRIER();
            *(pPktHdr->ring_entry) |= NIC_RING_SWOWNBIT;    /* Only set the SwOwn bit */

#ifndef NIC_CODE_REDUCE
            /* To guarantee it's write done */
            do
            {
                uint32 chk;
                chk = *(pPktHdr->ring_entry);
            } while (0);
#endif

            /* Jump to next */
            pNic_rxRDPIdx[i] += 1;
            if (pNic_rxRDPIdx[i] == (pNic_rxFDPBase[i] + NIC_RXRING_SIZE))
                pNic_rxRDPIdx[i] = pNic_rxFDPBase[i];
        } while (pNic_rxRDPIdx[i] != pNic_rxCDPIdx[i]);
    }

    return RT_ERR_OK;
} /* end of _nic_isr_mbRoutine */


/* Function Name:
 *      _cypress_nic_isr_handler
 * Description:
 *      Nic interrupt handle routine.
 * Input:
 *      isr_param - isr callback parameter
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      OSAL_INT_HANDLED
 * Note:
 *      None
 */
osal_isrret_t
_cypress_nic_isr_handler(void *isr_param)
{
    uint32  cpu_iisr, intrMask;

    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, &cpu_iisr);
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_MSK_ADDR, &intrMask);

    /* Rx have 8 rings (0..7); need to process when any rx ring isr done */
    if (cpu_iisr & INT_RX_DONE7_INT_RX_DONE0_MASK)
    {
        if (cpu_iisr & INT_RX_DONE7_MASK)   /* Process Rx ring 7 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE7_MASK);
            _nic_isr_rxRoutine(7);
        }

        if (cpu_iisr & INT_RX_DONE6_MASK)   /* Process Rx ring 6 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE6_MASK);
            _nic_isr_rxRoutine(6);
        }

        if (cpu_iisr & INT_RX_DONE5_MASK)   /* Process Rx ring 5 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE5_MASK);
            _nic_isr_rxRoutine(5);
        }

        if (cpu_iisr & INT_RX_DONE4_MASK)   /* Process Rx ring 4 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE4_MASK);
            _nic_isr_rxRoutine(4);
        }

        if (cpu_iisr & INT_RX_DONE3_MASK)   /* Process Rx ring 3 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE3_MASK);
            _nic_isr_rxRoutine(3);
        }

        if (cpu_iisr & INT_RX_DONE2_MASK)   /* Process Rx ring 2 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE2_MASK);
            _nic_isr_rxRoutine(2);
        }

        if (cpu_iisr & INT_RX_DONE1_MASK)   /* Process Rx ring 1 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE1_MASK);
            _nic_isr_rxRoutine(1);
        }

        if (cpu_iisr & INT_RX_DONE0_MASK)   /* Process Rx ring 0 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_DONE0_MASK);
            _nic_isr_rxRoutine(0);
        }
    }

    /* Tx have 2 rings (0..1); need to process when any tx ring isr done */
    if (cpu_iisr & INT_TX_DONE1_INT_TX_DONE0_MASK)
    {
        if (cpu_iisr & INT_TX_DONE1_MASK)   /* Process Tx ring 1 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_TX_DONE1_MASK);
            _nic_isr_txRoutine(1);
        }

        if (cpu_iisr & INT_TX_DONE0_MASK)   /* Process Tx ring 0 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_TX_DONE0_MASK);
            _nic_isr_txRoutine(0);
        }
    }

    if (cpu_iisr & INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK);
    }

    /* pktHdr Runout */
    if (cpu_iisr & INT_RX_RUN_OUT7_0_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_RX_RUN_OUT7_0_MASK);
    }

    /* mBuffer Runout */
    if (cpu_iisr & RTL8390_DMA_IF_INTR_STS_RX_RUN_OUT_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, RTL8390_DMA_IF_INTR_STS_RX_RUN_OUT_MASK);
        _nic_isr_mbRoutine();
    }

#if defined(CONFIG_SDK_DRIVER_L2NTFY)
    /* L2 Notification handler */
    if (cpu_iisr & intrMask & INT_NTFY_DONE_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_NTFY_DONE_MASK);
        drv_swcore_l2_notification_isr_handler(isr_param);
    }

    if (cpu_iisr & intrMask & INT_NTFY_BUF_RUN_OUT_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_NTFY_BUF_RUN_OUT_MASK);
        drv_swcore_l2_notification_buf_runout_handler(isr_param);
    }

    if (cpu_iisr & intrMask & INT_LOCAL_NTFY_BUF_RUN_OUT_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8390_DMA_IF_INTR_STS_ADDR, INT_LOCAL_NTFY_BUF_RUN_OUT_MASK);
        drv_swcore_l2_notification_localBuf_runout_handler(isr_param);
    }
#endif

    return OSAL_INT_HANDLED;
} /* end of _cypress_nic_isr_handler */

/* Function Name:
 *      _nic_init
 * Description:
 *      Initialize nic module of the specified device.
 * Input:
 *      unit     - unit id
 *      pInitCfg - pointer to initial config struct of NIC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Must initialize nic module before calling any nic APIs.
 */
static int32
_nic_init(uint32 unit, drv_nic_initCfg_t *pInitCfg)
{
    uint32  temp, temp2, notif_baseAddr;
    int32   i, j, k;
    int32   ret = RT_ERR_FAILED;

    /* Check arguments */
    RT_PARAM_CHK(NULL == pInitCfg->pkt_alloc, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pInitCfg->pkt_free, RT_ERR_NULL_POINTER);

    /* Reset the NIC Tx/Rx debug information */
    nic_debug_flag = 0;
    nic_tx_success_cntr = 0;
    nic_tx_failed_cntr = 0;
    nic_rx_success_cntr = 0;
    nic_rx_failed_cntr = 0;

    _nic_init_conf.pkt_size  = pInitCfg->pkt_size;
    _nic_init_conf.pkt_alloc = pInitCfg->pkt_alloc;
    _nic_init_conf.pkt_free  = pInitCfg->pkt_free;

    /* Reset NIC only */
    ioal_mem32_field_write(unit, RTL8390_MAC_PORT_CTRL_ADDR(52), RTL8390_MAC_PORT_CTRL_TX_EN_OFFSET, RTL8390_MAC_PORT_CTRL_TX_EN_MASK, 0);
    ioal_mem32_field_write(unit, RTL8390_MAC_PORT_CTRL_ADDR(52), RTL8390_MAC_PORT_CTRL_RX_EN_OFFSET, RTL8390_MAC_PORT_CTRL_RX_EN_MASK, 0);

    osal_time_usleep(50 * 1000); /* delay 50mS */

    RT_LOG(LOG_DEBUG, MOD_NIC, "Reset NIC (R8390)... ");
    /* Save the setting used by L2 notification */
    ioal_mem32_read(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, &temp2);
    temp2 = temp2 >> 20;
    ioal_mem32_read(unit, RTL8390_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL_ADDR, &notif_baseAddr);
    ioal_mem32_field_write(unit, RTL8390_RST_GLB_CTRL_ADDR, RTL8390_RST_GLB_CTRL_SW_NIC_RST_OFFSET, RTL8390_RST_GLB_CTRL_SW_NIC_RST_MASK, 1);

    do
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
        ioal_mem32_field_read(unit, RTL8390_RST_GLB_CTRL_ADDR, RTL8390_RST_GLB_CTRL_SW_NIC_RST_OFFSET, RTL8390_RST_GLB_CTRL_SW_NIC_RST_MASK, &temp);
    } while (temp != 0);

    /* Restore the setting used by L2 notification */
    ioal_mem32_field_write(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, RTL8390_DMA_IF_INTR_MSK_LOCAL_NTFY_BUF_RUN_OUT_OFFSET, 0x7 << RTL8390_DMA_IF_INTR_MSK_LOCAL_NTFY_BUF_RUN_OUT_OFFSET, temp2);
    ioal_mem32_write(unit, RTL8390_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL_ADDR, notif_baseAddr);
    RT_LOG(LOG_DEBUG, MOD_NIC, "OK");

    /* CPU port: Enable MAC Tx/Rx */
    ioal_mem32_field_write(unit, RTL8390_MAC_PORT_CTRL_ADDR(52), RTL8390_MAC_PORT_CTRL_TX_EN_OFFSET, RTL8390_MAC_PORT_CTRL_TX_EN_MASK, 1);
    ioal_mem32_field_write(unit, RTL8390_MAC_PORT_CTRL_ADDR(52), RTL8390_MAC_PORT_CTRL_RX_EN_OFFSET, RTL8390_MAC_PORT_CTRL_RX_EN_MASK, 1);

    /* Set CPU port to join the Lookup Miss Flooding Portmask */
    ioal_mem32_write(unit, RTL8390_TBL_ACCESS_L2_CTRL_ADDR, 0x28000);
    ioal_mem32_read(unit, RTL8390_TBL_ACCESS_L2_DATA_ADDR(0), &temp);
    temp |= 0x80000000;
    ioal_mem32_write(unit, RTL8390_TBL_ACCESS_L2_DATA_ADDR(0), temp);
    ioal_mem32_write(unit, RTL8390_TBL_ACCESS_L2_CTRL_ADDR, 0x38000);


    /* CPU port: Force link-up */
    ioal_mem32_field_write(unit, RTL8390_MAC_FORCE_MODE_CTRL_ADDR(52), \
        RTL8390_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_OFFSET, RTL8390_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_MASK, 1);
    ioal_mem32_field_write(unit, RTL8390_MAC_FORCE_MODE_CTRL_ADDR(52), \
        RTL8390_MAC_FORCE_MODE_CTRL_MAC_FORCE_EN_OFFSET, RTL8390_MAC_FORCE_MODE_CTRL_MAC_FORCE_EN_MASK, 1);

    /* Reset to default value */
    ioal_mem32_read(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, &temp);
    temp |= CPUIIMR_DISABLE_MASK;
    ioal_mem32_write(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, temp);
    ioal_mem32_write(unit, RTL8390_DMA_IF_INTR_STS_ADDR, CPUIISR_CLEARALL_MASK);
    ioal_mem32_write(unit, RTL8390_DMA_IF_CTRL_ADDR, DMA_IF_CTRL_DISABLE_MASK);

    pNicInfo = (nic_info_t *) osal_alloc(sizeof(nic_info_t));

    if (NULL == pNicInfo)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error: Out of memory!");
        return RT_ERR_FAILED;
    }
    if (((uint32)pNicInfo & 0x3) != 0)
    {
        osal_printf("FATAL Error: pNicInfo(0x%08X) is NOT 4 Byte-Align!\n", (uint32)pNicInfo);
        return RT_ERR_FAILED;
    }
    RT_LOG(LOG_DEBUG, MOD_NIC, "pNicInfo: %p (size: %d)", pNicInfo, sizeof(nic_info_t));
    osal_memset(pNicInfo, 0, sizeof(nic_info_t));
    if ((ret = osal_cache_memory_flush((uint32)pNicInfo, sizeof(nic_info_t))) != RT_ERR_OK)
    {
        return ret;
    }
    pNicInfo = (nic_info_t *)UNCACHE(pNicInfo);

    RT_LOG(LOG_DEBUG, MOD_NIC, "pNicInfo = %p", pNicInfo);
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        pNic_rxFDPBase[i] = pNicInfo->rxFDPBase[i];
        RT_LOG(LOG_DEBUG, MOD_NIC, "pNic_rxFDPBase[%01d] = %p", i, pNic_rxFDPBase[i]);
        pNic_rxCDPIdx[i] = pNic_rxFDPBase[i];
        pNic_rxRDPIdx[i] = pNic_rxFDPBase[i];
    }
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        pNic_txFDPBase[i] = pNicInfo->txFDPBase[i];
        RT_LOG(LOG_DEBUG, MOD_NIC, "pNic_txFDPBase[%01d] = %p", i, pNic_txFDPBase[i]);
        pNic_txCDPIdx[i] = pNic_txFDPBase[i];
        pNic_txRDPIdx[i] = pNic_txFDPBase[i];
    }

    /* Rx pktHdrs */
    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->rxPktHdr[i];
            pPktHdr->buf_size   = 0;
            pPktHdr->pkt_offset = 0;
            pPktHdr->more       = 0;
            pPktHdr->buf_len    = 0;
            pPktHdr->ring_entry = (pNic_rxFDPBase[j] + k);
            *(pNic_rxFDPBase[j] + k) = ((k + 1) == NIC_RXRING_SIZE)? \
                (uint32)pPktHdr | NIC_RING_WRAPBIT : \
                (uint32)pPktHdr;

            i++;
        }
    }

    /* Tx pktHdrs */
    i = 0;
    for (j = 0; j < NIC_TXRING_NUM; j++)
    {
        for (k = 0; k < NIC_TXRING_SIZE; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->txPktHdr[i];
            pPktHdr->buf_size   = 0;
            pPktHdr->pkt_offset = 0;
            pPktHdr->buf_len    = 0;
            pPktHdr->ring_entry  = (pNic_txFDPBase[j] + k);

            *(pNic_txFDPBase[j] + k) = ((k + 1) == NIC_TXRING_SIZE)? \
                (uint32)pPktHdr | NIC_RING_WRAPBIT : \
                (uint32)pPktHdr;

            i++;
        }
    }


    /* Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_register(RTK_DEV_NIC, _cypress_nic_isr_handler, NULL))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }

    /* Setting Registers */
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_field_write(unit, RTL8390_DMA_IF_RX_RING_SIZE_ADDR(i), RTL8390_DMA_IF_RX_RING_SIZE_SIZE_OFFSET(i),
                                RTL8390_DMA_IF_RX_RING_SIZE_SIZE_MASK(i), NIC_RXRING_SIZE);
    }
    ioal_mem32_write(unit, RTL8390_DMA_IF_RX_RING_CNTR_ADDR(0), 0xffffffff);

    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8390_DMA_IF_RX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_rxFDPBase[i]);
    }

    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8390_DMA_IF_TX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_txFDPBase[i]);
    }

    ioal_mem32_read(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, &temp);
    temp |= CPUIIMR_ENABLE_MASK;
    ioal_mem32_write(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, temp);
    ioal_mem32_field_write(unit, RTL8390_DMA_IF_CTRL_ADDR, RTL8390_DMA_IF_CTRL_TX_EN_OFFSET, RTL8390_DMA_IF_CTRL_TX_EN_MASK, 1);
    ioal_mem32_field_read(unit, RTL8390_MAC_CPU_TAG_ID_CTRL_ADDR, RTL8390_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_OFFSET, \
        RTL8390_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_MASK, &cpuTagId);
#if defined(__MODEL_USER__) || defined(__MODEL_KERNEL__)
    if (0 == cpuTagId)
    {
        ioal_mem32_write(unit, RTL8390_MAC_CPU_TAG_ID_CTRL_ADDR, REALTEK_CPUTAG_ID << RTL8390_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_OFFSET);
        cpuTagId = REALTEK_CPUTAG_ID;
    }
#endif
#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
    OSAL_INIT_LIST_HEAD(&nicListHead.list);
    spin_lock_init(&irqLock);
#endif

    /* Prepare the mBufs once */
    _nic_isr_mbRoutine();

    return RT_ERR_OK;
} /* end of _nic_init */


/* Function Name:
 *      _nic_pkt_tx
 * Description:
 *      Transmit a packet via nic of the specified device.
 * Input:
 *      unit    - unit id
 *      pPacket - pointer to a single packet struct
 *      fTxCb   - pointer to a handler of transmited packets
 *      pCookie - application data returned with callback (can be null)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_nic_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
{
    uint32  txRingId;
    uint32  packetNum;
    int32   ret = RT_ERR_FAILED;
    nic_pkthdr_t    *pPktHdr;

    if (NULL == pPacket)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - pPacket is NULL!");
        return RT_ERR_FAILED;
    }

    /* Step: Decide the target queue */
    txRingId = (pPacket->tx_tag.priority > 3) ? 1 : 0;    /* mapping 8 priority to 2 queues */

    /* Step: Find a pktHdr */
    if ((*pNic_txCDPIdx[txRingId] & NIC_RING_SWOWNBIT) != 0)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "No Tx Descriptor [%08x] = 0x%08x can be used!",
            (uint32)pNic_txCDPIdx[txRingId], *pNic_txCDPIdx[txRingId]);
        return RT_ERR_FAILED;
    }

    if ((pPacket->data + pPacket->length) > pPacket->tail)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "pPacket->data + pPacket->length > pPacket->tail!");
        return RT_ERR_FAILED;
    }

    pPktHdr = (nic_pkthdr_t *)(*pNic_txCDPIdx[txRingId] & NIC_ADDR_MASK);
    /* Step: Double Confirm (Check the pktHdr status) */
    if (NULL == pPktHdr || NULL != pPktHdr->packet)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr is NOT available!");
        return RT_ERR_FAILED;
    }
    RT_LOG(LOG_DEBUG, MOD_NIC, "%s():%d  pPktHdr->packet:%p   as_txtag:%d\n", __FUNCTION__, __LINE__, pPktHdr->packet, pPacket->as_txtag);

    /* Insert the CPU Tx tag into the packet data buffer */
    if (pPacket->as_txtag)
    {
        pPktHdr->cpuTag.un.tx.CPUTAGIF      = TRUE;
        pPktHdr->cpuTag.un.tx.DPM_TYPE      = pPacket->tx_tag.dpm_type;
        pPktHdr->cpuTag.un.tx.ACL_ACT       = pPacket->tx_tag.acl_act;
        pPktHdr->cpuTag.un.tx.DM_PKT        = pPacket->tx_tag.dm_pkt;
        pPktHdr->cpuTag.un.tx.DG_PKT        = pPacket->tx_tag.dg_pkt;
        pPktHdr->cpuTag.un.tx.BP_FLTR1      = pPacket->tx_tag.bp_fltr1;
        pPktHdr->cpuTag.un.tx.BP_FLTR2      = pPacket->tx_tag.bp_fltr2;
        pPktHdr->cpuTag.un.tx.AS_PRI        = pPacket->tx_tag.as_priority;
        pPktHdr->cpuTag.un.tx.PRI           = pPacket->tx_tag.priority;
        pPktHdr->cpuTag.un.tx.L2LEARNING    = pPacket->tx_tag.l2_learning;
        pPktHdr->cpuTag.un.tx.AS_TAGSTS     = pPacket->tx_tag.as_tagSts;
        pPktHdr->cpuTag.un.tx.RVID_SEL      = pPacket->tx_tag.rvid_sel;
        pPktHdr->cpuTag.un.tx.AS_DPM        = pPacket->tx_tag.as_dst_port_mask;
        pPktHdr->cpuTag.un.tx.DPM51_32      = pPacket->tx_tag.dst_port_mask_1;
        pPktHdr->cpuTag.un.tx.DPM31_0       = pPacket->tx_tag.dst_port_mask;
    }
    else
        pPktHdr->cpuTag.un.tx.CPUTAGIF = FALSE;

    /* Step: Calc the pktbuf number */
    packetNum = 1;    /* Support single pktBuf now - one descriptor vs. one mbuf */

    pPktHdr->tx_callback = fTxCb;    /* Tx Callback function */
    pPktHdr->cookie = pCookie;
    pPktHdr->packet = pPacket;
    pPktHdr->buf_addr = (uint8 *)UNCACHE(pPacket->data);
    pPktHdr->buf_size = pPacket->length;
    pPktHdr->buf_len = pPacket->length;

	if (0 == pPacket->txIncludeCRC)
	{
        pPktHdr->buf_size += 4;
        pPktHdr->buf_len += 4;
	}

    if ((ret = osal_cache_memory_flush((uint32)pPacket->head, (pPacket->end - pPacket->head))) != RT_ERR_OK)
    {
        return ret;
    }

    MEMORY_BARRIER();
    *(pPktHdr->ring_entry) |= NIC_RING_SWOWNBIT;

    /* To guarantee it's write done */
    do
    {
        uint32 chk;
        chk = *(pPktHdr->ring_entry);
    } while (0);

    /* Jump to next */
    pNic_txCDPIdx[txRingId] += 1;
    if (pNic_txCDPIdx[txRingId] == (pNic_txFDPBase[txRingId] + NIC_TXRING_SIZE))
        pNic_txCDPIdx[txRingId] = pNic_txFDPBase[txRingId];

    /* NIC Tx debug message */
    if (nic_debug_flag & DEBUG_TX_RAW_LEN_BIT)
    {
        int i;
        int dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */

        osal_printf("=== [NIC TX Debug] ================================= Len: %d \n", pPktHdr->buf_len);

        for (i = 0; i < dump_len; i++)
        {
            if (i == (pPktHdr->buf_len))
                break;
            if (0 == (i % 16))
                osal_printf("[%04X] ", i);
            osal_printf("%02X ", *(uint8*)(pPktHdr->buf_addr + i));
            if (15 == (i % 16))
                osal_printf("\n");
        }
        osal_printf("\n");
    }

    if ((nic_debug_flag & DEBUG_TX_CPU_TAG_BIT) && pPacket->as_txtag)
    {
        osal_printf("=== [NIC TX Debug - CPU Tx Tag Information] ============ \n");
        osal_printf(" DPM_TYPE : 0x%0x \n", pPktHdr->cpuTag.un.tx.DPM_TYPE);
        osal_printf(" ACL_ACT : 0x%0x \n", pPktHdr->cpuTag.un.tx.ACL_ACT);
        osal_printf(" DM_PKT : 0x%0x \n", pPktHdr->cpuTag.un.tx.DM_PKT);
        osal_printf(" DG_PKT : 0x%0x \n", pPktHdr->cpuTag.un.tx.DG_PKT);
        osal_printf(" BP_FLTR1 : 0x%0x \n", pPktHdr->cpuTag.un.tx.BP_FLTR1);
        osal_printf(" BP_FLTR2 : 0x%0x \n", pPktHdr->cpuTag.un.tx.BP_FLTR2);
        osal_printf(" AS_PRI : 0x%0x \n", pPktHdr->cpuTag.un.tx.AS_PRI);
        osal_printf(" PRI : 0x%0x \n", pPktHdr->cpuTag.un.tx.PRI);
        osal_printf(" L2LEARNING : 0x%0x \n", pPktHdr->cpuTag.un.tx.L2LEARNING);
        osal_printf(" AS_TAGSTS : 0x%0x \n", pPktHdr->cpuTag.un.tx.AS_TAGSTS);
        osal_printf(" RVID_SEL : 0x%0x \n", pPktHdr->cpuTag.un.tx.RVID_SEL);
        osal_printf(" AS_DPM : 0x%0x \n", pPktHdr->cpuTag.un.tx.AS_DPM);
        osal_printf(" DPM51_32 : 0x%0x \n", pPktHdr->cpuTag.un.tx.DPM51_32);
        osal_printf(" DPM31_0 : 0x%0x \n", pPktHdr->cpuTag.un.tx.DPM31_0);
    }

    /* Set the TX Fetch Notify bit */
    ioal_mem32_field_write(unit, RTL8390_DMA_IF_CTRL_ADDR, RTL8390_DMA_IF_CTRL_TX_FETCH_OFFSET, RTL8390_DMA_IF_CTRL_TX_FETCH_MASK, 1);
    return RT_ERR_OK;
} /* end of _nic_pkt_tx */


/* Function Name:
 *      _nic_rx_start
 * Description:
 *      Start the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_nic_rx_start(uint32 unit)
{
    ioal_mem32_field_write(unit, RTL8390_DMA_IF_CTRL_ADDR, RTL8390_DMA_IF_CTRL_RX_EN_OFFSET, RTL8390_DMA_IF_CTRL_RX_EN_MASK, 1);

    RT_LOG(LOG_DEBUG, MOD_NIC, "NIC (R8390) Rx Start... ");

    return RT_ERR_OK;
} /* end of _nic_rx_start */


/* Function Name:
 *      _nic_rx_stop
 * Description:
 *      Stop the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_nic_rx_stop(uint32 unit)
{
    ioal_mem32_field_write(unit, RTL8390_DMA_IF_CTRL_ADDR, RTL8390_DMA_IF_CTRL_RX_EN_OFFSET, RTL8390_DMA_IF_CTRL_RX_EN_MASK, 0);

    RT_LOG(LOG_DEBUG, MOD_NIC, "NIC (R8390) Rx Stop... ");

    return RT_ERR_OK;
} /* end of _nic_rx_stop */


/* Function Name:
 *      _nic_rx_register
 * Description:
 *      Register to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback
 *      fRxCb    - pointer to a handler of received packets
 *      pCookie  - application data returned with callback (can be null)
 *      flags    - optional flags for reserved
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
static int32
_nic_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags)
{
    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_FAILED);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

	if (NIC_RX_CRC_INCLUDE & flags)
		rxCRCInclude = 1;

    if (NULL == _nic_rx_cb_tbl[priority].rx_callback)
    {
        _nic_rx_cb_tbl[priority].rx_callback = fRxCb;
        _nic_rx_cb_tbl[priority].pCookie     = pCookie;
    }
    else
    {
        /* Handler is already existing */
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of _nic_rx_register */


/* Function Name:
 *      _nic_rx_unregister
 * Description:
 *      Unregister to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback
 *      fRxCb    - pointer to a handler of received packets (can be null)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
static int32
_nic_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb)
{
    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_FAILED);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

    if (_nic_rx_cb_tbl[priority].rx_callback == fRxCb)
    {
        _nic_rx_cb_tbl[priority].rx_callback = NULL;
        _nic_rx_cb_tbl[priority].pCookie     = NULL;
    }
    else
    {
        /* Handler is nonexistent */
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of _nic_rx_unregister */


#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
static void _nic_rx_thread(void *pInput)
{
    uint32  i, curNic_rx_pkt_cnt, ring, value, jumbo;
    nic_collectArrayList_t  *pEntry, *n;
    drv_nic_rx_t nic_rx_handle = NIC_RX_NOT_HANDLED;
    unsigned long flags;

    while(1)
    {
        osal_wait_event(wq, atomic_read(&rxFlag) > 0);
        atomic_set(&rxFlag, 0);

        osal_list_for_each_entry_safe(pEntry, n, &nicListHead.list, list)
        {
            for (i = 0; i < NIC_RX_CB_PRIORITY_NUMBER; i++)
            {
                if (_nic_rx_cb_tbl[i].rx_callback != NULL)
                {
                    nic_rx_handle = _nic_rx_cb_tbl[i].rx_callback(NIC_DEFAULT_UNIT_ID, pEntry->pPacket, _nic_rx_cb_tbl[i].pCookie);
                    if (NIC_RX_HANDLED_OWNED == nic_rx_handle)
                    {
                        break;
                    }
                }
            }
            if (nic_rx_handle != NIC_RX_HANDLED_OWNED)
            {   /* We have to free this packet here */
                _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pEntry->pPacket);
            }
            jumbo = pEntry->jumbo;

            spin_lock_irqsave(&irqLock, flags);
            osal_list_del(&pEntry->list);
            spin_unlock_irqrestore(&irqLock, flags);

            osal_free(pEntry);

            if (jumbo)
                curNic_rx_pkt_cnt = atomic_sub_return(JUMBO_CLUSTER_NUM, &nic_rx_pkt_cnt);
            else
                curNic_rx_pkt_cnt = atomic_sub_return(1, &nic_rx_pkt_cnt);
            if (curNic_rx_pkt_cnt == 0 && atomic_read(&nic_mem_lock))
            {
                ioal_mem32_read(0, RTL8390_DMA_IF_INTR_MSK_ADDR, &value);
                atomic_set(&nic_mem_lock, 0);
                for (ring = 0; ring < NIC_RXRING_NUM; ring++)
                    _nic_isr_rxRoutine(ring);
                value |= RTL8390_DMA_IF_INTR_MSK_RX_DONE_MASK | RTL8390_DMA_IF_INTR_MSK_RX_RUN_OUT_MASK;
                ioal_mem32_write(0, RTL8390_DMA_IF_INTR_MSK_ADDR, value);
                nicReleaseCnt++;
            }
        }
    }
}
#endif

/* Function Name:
 *      r8390_init
 * Description:
 *      Initialize nic module of the specified device.
 * Input:
 *      unit     - unit id
 *      pInitCfg - pointer to initial config struct of NIC
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Must initialize nic module before calling any nic APIs.
 */
int32
r8390_init(uint32 unit, drv_nic_initCfg_t *pInitCfg)
{
    int32 ret = RT_ERR_FAILED;

    /* Check arguments */
    RT_PARAM_CHK(NULL == pInitCfg, RT_ERR_NULL_POINTER);

    /* Check whether it is inited, if inited, return fail */
    if (INIT_COMPLETED == nic_init[unit])
        return ret;

    /* create semaphore */
    nic_lock = 0;


    /* Initialize the NIC module */
    if ((ret = _nic_init(unit, pInitCfg)) != RT_ERR_OK)
    {
        return ret;
    }

#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
    osal_init_waitqueue_head(&wq);
    if ((osal_thread_t)NULL == (osal_thread_create("RTK NIC Rx Thread", NIC_RX_THREAD_STACK_SIZE, NIC_RX_THREAD_PRI, (void *)_nic_rx_thread, NULL)))
    {
        osal_printf("RTK NIC Rx Thread create failed\n");

        return RT_ERR_FAILED;
    }
#endif

    /* set init flag to complete init */
    nic_init[unit] = INIT_COMPLETED;

    return ret;
} /* end of r8390_init */


/* Function Name:
 *      r8390_pkt_tx
 * Description:
 *      Transmit a packet via nic of the specified device.
 * Input:
 *      unit    - unit id
 *      pPacket - pointer to a single packet struct
 *      fTxCb   - pointer to a handler of transmited packets
 *      pCookie - application data returned with callback (can be null)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      When fTxCb is NULL, driver will free packet and not callback any more.
 */
int32
r8390_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(NULL == pPacket, RT_ERR_NULL_POINTER);

    /* Dispatch */
    NIC_SEM_LOCK(unit);
    ret = _nic_pkt_tx(unit, pPacket, fTxCb, pCookie);
    NIC_SEM_UNLOCK(unit);

    if (RT_ERR_OK == ret)
        nic_tx_success_cntr++;
    else
        nic_tx_failed_cntr++;

    return ret;
} /* end of r8390_pkt_tx */


/* Function Name:
 *      r8390_rx_start
 * Description:
 *      Start the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
int32
r8390_rx_start(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */

    /* Dispatch */
    NIC_SEM_LOCK(unit);
    ret = _nic_rx_start(unit);
    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8390_rx_start */


/* Function Name:
 *      r8390_rx_stop
 * Description:
 *      Stop the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
int32
r8390_rx_stop(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */

    /* Dispatch */
    NIC_SEM_LOCK(unit);
    ret = _nic_rx_stop(unit);
    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8390_rx_stop */


/* Function Name:
 *      r8390_rx_register
 * Description:
 *      Register to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback
 *      fRxCb    - pointer to a handler of received packets
 *      pCookie  - application data returned with callback (can be null)
 *      flags    - optional flags for reserved
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
r8390_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

    /* Dispatch */
    NIC_SEM_LOCK(unit);
    ret = _nic_rx_register(unit, priority, fRxCb, pCookie, flags);
    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8390_rx_register */


/* Function Name:
 *      r8390_rx_unregister
 * Description:
 *      Unregister to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback
 *      fRxCb    - pointer to a handler of received packets (can be null)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
r8390_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

    /* Dispatch */
    NIC_SEM_LOCK(unit);
    ret = _nic_rx_unregister(unit, priority, fRxCb);
    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8390_rx_unregister */


/* Function Name:
 *      r8390_pkt_alloc
 * Description:
 *      Packet allocate API in the specified device.
 * Input:
 *      unit     - unit id
 *      size     - packet size
 *      flags    - flags
 * Output:
 *      ppPacket - pointer buffer of packet
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
r8390_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(NULL == _nic_init_conf.pkt_alloc, RT_ERR_NULL_POINTER);

    /* Dispatch */
    ret = _nic_init_conf.pkt_alloc(unit, size, flags, ppPacket);

    return ret;
} /* end of r8390_pkt_alloc */

/* Function Name:
 *      r8390_pkt_free
 * Description:
 *      Packet free API in the specified device.
 * Input:
 *      unit     - unit id
 *      pPacket  - pointer buffer of packet
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
r8390_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(NULL == pPacket, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == _nic_init_conf.pkt_free, RT_ERR_NULL_POINTER);

    /* Dispatch */
    ret = _nic_init_conf.pkt_free(unit, pPacket);

    return ret;
} /* end of r8390_pkt_free */

/* NIC Tx/Rx debug */
/* Function Name:
 *      r8390_debug_set
 * Description:
 *      Set NIC debug flags of the specified device.
 * Input:
 *      unit  - unit id
 *      flags - NIC debug flags
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      There are 4 BIT flags can be selected as following:
 *      - DEBUG_RX_RAW_LEN_BIT
 *      - DEBUG_RX_CPU_TAG_BIT
 *      - DEBUG_TX_RAW_LEN_BIT
 *      - DEBUG_TX_CPU_TAG_BIT
 */
int32
r8390_debug_set(uint32 unit, uint32 flags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    nic_debug_flag = flags;
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8390_debug_set */

/* Function Name:
 *      r8390_debug_get
 * Description:
 *      Get NIC debug flags of the specified device.
 * Input:
 *      unit   - unit id
 * Output:
 *      pFlags - NIC debug flags
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      There are 4 BIT flags as following:
 *      - DEBUG_RX_RAW_LEN_BIT
 *      - DEBUG_RX_CPU_TAG_BIT
 *      - DEBUG_TX_RAW_LEN_BIT
 *      - DEBUG_TX_CPU_TAG_BIT
 */
int32
r8390_debug_get(uint32 unit, uint32 *pFlags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    *pFlags = nic_debug_flag;
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8390_debug_get */

/* Function Name:
 *      r8390_counter_dump
 * Description:
 *      Dump NIC debug counter information of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      There are 4 debug counters be dump as following:
 *      - nic_tx_success_cntr
 *      - nic_tx_failed_cntr
 *      - nic_rx_success_cntr
 *      - nic_rx_failed_cntr
 */
int32
r8390_counter_dump(uint32 unit)
{
#if 0
    uint32 i;
    drv_nic_pkt_t *pPacket;
#endif


    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    osal_printf("Tx success counter : %0d \n", nic_tx_success_cntr);
    osal_printf("Tx failed counter  : %0d \n", nic_tx_failed_cntr);
    osal_printf("Rx success counter : %0d \n", nic_rx_success_cntr);
    osal_printf("Rx failed counter  : %0d \n", nic_rx_failed_cntr);
#ifdef CONFIG_SDK_NIC_RX_CB_IN_THREAD
    osal_printf("nic_rx_pkt_cnt     : %0d \n", atomic_read(&nic_rx_pkt_cnt));
    osal_printf("nic_mem_lock       : %0d \n", atomic_read(&nic_mem_lock));
    osal_printf("nicLockCnt         : %0d \n", nicLockCnt);
    osal_printf("nicReleaseCnt      : %0d \n", nicReleaseCnt);
#endif
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8390_counter_dump */

/* Function Name:
 *      r8390_counter_clear
 * Description:
 *      Clear NIC debug counter information of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Clear following NIC debug counters
 *      - nic_tx_success_cntr
 *      - nic_tx_failed_cntr
 *      - nic_rx_success_cntr
 *      - nic_rx_failed_cntr
 */
int32
r8390_counter_clear(uint32 unit)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    nic_tx_success_cntr = 0;
    nic_tx_failed_cntr = 0;
    nic_rx_success_cntr = 0;
    nic_rx_failed_cntr = 0;
	nic_tx_isr_cntr = 0;
	nic_tx_ring_cntr = 0;
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8390_counter_clear */

/* Function Name:
 *      r8390_bufStatus_dump
 * Description:
 *      Dump NIC buffer status of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Following message are dump
 *      1) From SW View
 *      - Rx Ring Packet Header (FDPBase, CDPIdx, RDPIdx)
 *      - Tx Ring Packet Header (FDPBase, CDPIdx, RDPIdx)
 *      2) From HW View
 *      - Rx Ring Packet Header(CDPIdx)
 *      - Tx Ring Packet Header(CDPIdx)
 *      3) Register Information
 *      - CPUIIMR (CPU Interface Interrupt Mask Register)
 *      - CPUIISR (CPU Interface Interrupt Status Register)
 *      - CPUICR  (CPU Interface Control Register)
 */
int32
r8390_bufStatus_dump(uint32 unit)
{
    uint32  i, value;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    osal_printf("RXRING  SW_rxFDPBase  SW_RxCDPIdx  HW_RxCDPIdx  SW_RxRDPIdx \n");
    osal_printf("=========================================================== \n");
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_read(unit, RTL8390_DMA_IF_RX_CUR_DESC_ADDR_CTRL_ADDR(i), &value);

        osal_printf(" %d(p)   0x%08x    0x%08x   0x%08x   0x%08x \n",
                    i, ((uint32)pNic_rxFDPBase[i]), ((uint32)pNic_rxCDPIdx[i]), value, ((uint32)pNic_rxRDPIdx[i]));
    }
    osal_printf("\n");

    osal_printf("RXRING rxFDPBase  Base+1   Base+2   Base+3   Base+4   Base+5   Base+6   Base+7  \n");
    osal_printf("================================================================================\n");
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        osal_printf(" %d(p)  0x%08x %08x %08x %08x %08x %08x %08x %08x\n",
                    i, ((uint32)(pNic_rxFDPBase[i])), ((uint32)(pNic_rxFDPBase[i] + 1)),
                    ((uint32)(pNic_rxFDPBase[i] + 2)), ((uint32)(pNic_rxFDPBase[i] + 3)),
                    ((uint32)(pNic_rxFDPBase[i] + 4)), ((uint32)(pNic_rxFDPBase[i] + 5)),
                    ((uint32)(pNic_rxFDPBase[i] + 6)), ((uint32)(pNic_rxFDPBase[i] + 7)));
    }
    osal_printf("--------------------------------------------------------------------------------\n");
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        osal_printf(" %d(v)  0x%08x %08x %08x %08x %08x %08x %08x %08x\n",
                    i, (*(uint32 *)(pNic_rxFDPBase[i])), (*(uint32 *)(pNic_rxFDPBase[i] + 1)),
                    (*(uint32 *)(pNic_rxFDPBase[i] + 2)), (*(uint32 *)(pNic_rxFDPBase[i] + 3)),
                    (*(uint32 *)(pNic_rxFDPBase[i] + 4)), (*(uint32 *)(pNic_rxFDPBase[i] + 5)),
                    (*(uint32 *)(pNic_rxFDPBase[i] + 6)), (*(uint32 *)(pNic_rxFDPBase[i] + 7)));
    }
    osal_printf("\n");

    osal_printf("TXRING  SW_txFDPBase  SW_TxCDPIdx  HW_TxCDPIdx  SW_TxRDPIdx \n");
    osal_printf("=========================================================== \n");
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_read(unit, RTL8390_DMA_IF_TX_CUR_DESC_ADDR_CTRL_ADDR(i), &value);

        osal_printf(" %d(p)   0x%08x    0x%08x   0x%08x   0x%08x \n",
                    i, ((uint32)pNic_txFDPBase[i]), ((uint32)pNic_txCDPIdx[i]), value, ((uint32)pNic_txRDPIdx[i]));
    }
    osal_printf("\n");

    osal_printf("TXRING txFDPBase  Base+1   Base+2   Base+3\n");
    osal_printf("============================================\n");
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        osal_printf(" %d(p)  0x%08x %08x %08x %08x\n",
                    i, ((uint32)(pNic_txFDPBase[i])), ((uint32)(pNic_txFDPBase[i] + 1)),
                    ((uint32)(pNic_txFDPBase[i] + 2)), ((uint32)(pNic_txFDPBase[i] + 3)));
    }
    osal_printf("--------------------------------------------\n");
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        osal_printf(" %d(v)  0x%08x %08x %08x %08x\n",
                    i, (*(uint32 *)(pNic_txFDPBase[i])), (*(uint32 *)(pNic_txFDPBase[i] + 1)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 2)), (*(uint32 *)(pNic_txFDPBase[i] + 3)));
    }
    osal_printf("\n");

    ioal_mem32_read(unit, RTL8390_DMA_IF_INTR_MSK_ADDR, &value);
    osal_printf("CPUIIMR[0xBB007830] = 0x%08x ", value);
    osal_printf("[EN_PHD7_0=0x%x, ", (value & RTL8390_DMA_IF_INTR_MSK_RX_RUN_OUT_MASK) >> RTL8390_DMA_IF_INTR_MSK_RX_RUN_OUT_OFFSET);
    osal_printf("   EN_TX_DONE1_0=0x%x, ", (value & RTL8390_DMA_IF_INTR_MSK_TX_DONE_MASK) >> RTL8390_DMA_IF_INTR_MSK_TX_DONE_OFFSET);
    osal_printf("EN_RX_DONE7_0=0x%x, ", (value & RTL8390_DMA_IF_INTR_MSK_RX_DONE_MASK) >> RTL8390_DMA_IF_INTR_MSK_RX_DONE_OFFSET);
    osal_printf("EN_TX_ALLDONE1_0=0x%x]\n", (value & RTL8390_DMA_IF_INTR_MSK_TX_ALL_DONE_MASK) >> RTL8390_DMA_IF_INTR_MSK_TX_ALL_DONE_OFFSET);

    ioal_mem32_read(unit, RTL8390_DMA_IF_INTR_STS_ADDR, &value);
    osal_printf("CPUIISR[0xBB003130] = 0x%08x ", value);
    osal_printf("[INT_PHDS7_0=0x%x, ", (value & RTL8390_DMA_IF_INTR_STS_RX_RUN_OUT_MASK) >> RTL8390_DMA_IF_INTR_STS_RX_RUN_OUT_OFFSET);
    osal_printf("   INT_TX_DONE1_0=0x%x, ", (value & RTL8390_DMA_IF_INTR_STS_TX_DONE_MASK) >> RTL8390_DMA_IF_INTR_STS_TX_DONE_OFFSET);
    osal_printf("INT_RX_DONE7_0=0x%x, ", (value & RTL8390_DMA_IF_INTR_STS_RX_DONE_MASK) >> RTL8390_DMA_IF_INTR_STS_RX_DONE_OFFSET);
    osal_printf("INT_TX_ALL_DONE1_0=0x%x]\n", (value & RTL8390_DMA_IF_INTR_STS_TX_ALL_DONE_MASK) >> RTL8390_DMA_IF_INTR_STS_TX_ALL_DONE_OFFSET);

    ioal_mem32_read(unit, RTL8390_DMA_IF_CTRL_ADDR, &value);  
    osal_printf("CPUICR[0xBB003134] = 0x%08x ", value);
    osal_printf("[TX_CMD=0x%x, ", (value & RTL8390_DMA_IF_CTRL_TX_EN_MASK) >> RTL8390_DMA_IF_CTRL_TX_EN_OFFSET);
    osal_printf("RX_CMD=0x%x,\n", (value & RTL8390_DMA_IF_CTRL_RX_EN_MASK) >> RTL8390_DMA_IF_CTRL_RX_EN_OFFSET);
    osal_printf("TXFN=0x%x, ", (value & RTL8390_DMA_IF_CTRL_TX_FETCH_MASK) >> RTL8390_DMA_IF_CTRL_TX_FETCH_OFFSET);
    osal_printf("TX_BUSY=0x%x]\n", (value & RTL8390_DMA_IF_CTRL_TX_BUSY_MASK) >> RTL8390_DMA_IF_CTRL_TX_BUSY_OFFSET);
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8390_bufStatus_dump */


/* Function Name:
 *      r8390_pkthdrMbuf_dump
 * Description:
 *      Dump NIC packet header and mbuf detail information of the specified device.
 * Input:
 *      unit  - unit id
 *      mode  - tx/rx mode
 *      start - start ring id
 *      end   - end ring id
 *      flags - dump flags
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1) valid 'mode' value:
 *      - NIC_PKTHDR_MBUF_MODE_RX
 *      - NIC_PKTHDR_MBUF_MODE_TX
 *      2) valid ring id (start .. end)
 *      - Rx (0 .. 7)
 *      - Tx (0 .. 1)
 *      3) valid 'flags' value:
 *      - TRUE: include packet raw data
 *      - FALSE: exclude packet raw data
 */
int32
r8390_pkthdrMbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags)
{
    uint32  i, j, ring_size;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    osal_printf("------- Formal Information -------------------------\n");
    if (NIC_PKTHDR_MBUF_MODE_RX == mode)
    {
        osal_printf("==== Dump Rx packet header and mbuf ====\n");
        ring_size = NIC_RXRING_SIZE;
    }
    else
    {
        osal_printf("==== Dump Tx packet header and mbuf ====\n");
        ring_size = NIC_TXRING_SIZE;
    }

    for (i = start; i <= end; i++)
    {
        for (j = 0; j < ring_size; j++)
        {
            nic_pkthdr_t    *pRing_pkthdr;
            if (NIC_PKTHDR_MBUF_MODE_RX == mode)
                pRing_pkthdr = (nic_pkthdr_t *)(*((uint32 *)(pNic_rxFDPBase[i]+j)) & NIC_ADDR_MASK);
            else
                pRing_pkthdr = (nic_pkthdr_t *)(*((uint32 *)(pNic_txFDPBase[i]+j)) & NIC_ADDR_MASK);
            osal_printf("###################################################\n");
            osal_printf("ring[%d]_pkthdr[%d]->buf_addr = 0x%08x\n", i, j, (uint32)pRing_pkthdr->buf_addr);
            osal_printf("ring[%d]_pkthdr[%d]->buf_size = 0x%04x\n", i, j, (uint16)pRing_pkthdr->buf_size);
            osal_printf("ring[%d]_pkthdr[%d]->more = 0x%08x\n", i, j, (uint32)pRing_pkthdr->more);
            osal_printf("ring[%d]_pkthdr[%d]->pkt_offset = 0x%08x\n", i, j, (uint32)pRing_pkthdr->pkt_offset);
            osal_printf("ring[%d]_pkthdr[%d]->buf_len = 0x%04x\n", i, j, (uint16)pRing_pkthdr->buf_len);
            osal_printf("ring[%d]_pkthdr[%d]->tx_callback = 0x%08x\n", i, j, (uint32)pRing_pkthdr->tx_callback);
            osal_printf("ring[%d]_pkthdr[%d]->cookie = 0x%08x\n", i, j, (uint32)pRing_pkthdr->cookie);
            if ((pRing_pkthdr->packet != NULL) && (TRUE == flags))
            {
                uint32  k;
                uint32  dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */
                uint32  pkt_len = pRing_pkthdr->buf_len;
                uint8   *pPkt_data = pRing_pkthdr->packet->data;
                osal_printf("------------------- its raw data ----------------------\n");

                for (k = 0; k < dump_len; k++)
                {
                    if (k == pkt_len)
                        break;
                    if (0 == (k % 16))
                        osal_printf("[%04X] ", k);
                    osal_printf("%02X ", *(pPkt_data + k));
                    if (15 == (k % 16))
                        osal_printf("\n");
                }
                osal_printf("\n");
            }
        }
        osal_printf("###################################################\n");
    }
    NIC_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of r8390_pkthdrMbuf_dump */

/* Function Name:
 *      r8390_rxStatus_get
 * Description:
 *      Get NIC rx status of the specified device.
 * Input:
 *      unit    - unit id
 * Output:
 *      pStatus - rx status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
int32
r8390_rxStatus_get(uint32 unit, uint32 *pStatus)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    ioal_mem32_field_read(unit, RTL8390_DMA_IF_CTRL_ADDR, \
        RTL8390_DMA_IF_CTRL_RX_EN_OFFSET, RTL8390_DMA_IF_CTRL_RX_EN_MASK, pStatus);
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8390_rxStatus_get */
