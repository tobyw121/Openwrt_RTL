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
 * $Revision: 56661 $
 * $Date: 2015-03-09 11:27:48 +0800 (Mon, 09 Mar 2015) $
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
#include <soc/soc.h>
#include <common/debug/rt_log.h>
#include <ioal/ioal_init.h>
#include <ioal/mem32.h>
#include <osal/lib.h>
#include <osal/time.h>
#include <osal/inet.h>
#include <osal/sem.h>
#include <osal/cache.h>
#include <drv/nic/nic_rx.h>
#include <drv/nic/r8380.h>
#include <drv/swcore/rtl8380.h>
#include <rtk/default.h>
#include <endian.h>

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


/* DMA memory convert function */
#define DMA_ADDR_KRN2USR(krn_addr)    (((uint32)krn_addr - (uint32)KRNVIRT(RTL8380_DMA_PHYS_BASE)) + dma_base)
#define DMA_ADDR_USR2KRN(usr_addr)    (((uint32)usr_addr - dma_base) + (uint32)KRNVIRT(RTL8380_DMA_PHYS_BASE))

#define KRNVIRT2(addr)           ((unsigned int)(addr) | (0xa0000000))

/* Descriptor(ring and pktHdr) memory convert function */
#define DESC_ADDR_KRN2USR(unit, krn_addr)    (((uint32)krn_addr - (uint32)KRNVIRT2(RTL8380_DESC_PHYS_BASE)) + desc_base[unit])
#define DESC_ADDR_USR2KRN(unit, usr_addr)    (((uint32)usr_addr - desc_base[unit]) + (uint32)KRNVIRT2(RTL8380_DESC_PHYS_BASE))


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
    /* word [0] */ 
    uint8 *buf_addr;
#if __BYTE_ORDER == __LITTLE_ENDIAN
    /* word [1] */
    uint16  buf_size:14;
    uint16          :2;
    uint16  reserved;

    /* word [2] */    
    uint16  buf_len:14;
    uint16          :2;
    uint16  pkt_offset:14;
    uint16      :1;
    uint16  more:1;

    /* word [3] */    
    uint16  Reserved_1; /*cw it may cause problem here, fix this*/

#else /* if big endian*/

    /* word [1] */
    uint16  reserved;
    uint16          :2;
    uint16  buf_size:14;

    /* word [2] */    
    uint16  more:1;
    uint16      :1;
    uint16  pkt_offset:14;
    uint16          :2;
    uint16  buf_len:14;

    /* word [3] */    
    uint16  Reserved_1;
#endif
    /* word [3] ~ word [5]*/       
    maple_nic_cpuTag_t    cpuTag;

    /* Used by Software */
    struct drv_nic_pkt_s *packet;
    uint32  *ring_entry;
    drv_nic_tx_cb_f tx_callback;
    void    *cookie;
} nic_pkthdr_t;

typedef struct nic_info_s
{
    uint32          rxFDPBase[NIC_RXRING_NUM][NIC_RXRING_SIZE];
    uint32          txFDPBase[NIC_TXRING_NUM][NIC_8380_TXRING_SIZE];
    nic_pkthdr_t    rxPktHdr[NIC_RX_PKTHDR_NUM];
    nic_pkthdr_t    txPktHdr[NIC_8380_TX_PKTHDR_NUM];
} nic_info_t;


/*
 * Data Declaration
 */
extern uint32   nic_chipId[RTK_MAX_NUM_OF_UNIT];
static uint32   nic_8380_txing_size_of_max_limit = 1;

static uint32   nic_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static nic_rx_cb_entry_t _nic_rx_cb_tbl[NIC_RX_CB_PRIORITY_NUMBER];
drv_nic_initCfg_t _nic_init_conf;

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
static uint32       nic_tx_success_cntr_tx_debug;
static uint32       nic_tx_success_cntr_interrupt;
static uint32       nic_tx_failed_cntr;
static uint32       nic_rx_success_cntr;
static uint32       nic_rx_failed_cntr;
static uint32       nic_rx_lack_buf_cntr;
static uint32       _nic_rx_intr_cb_cnt = 0;

extern uint32       dma_base;
extern uint32       desc_base[RTK_MAX_NUM_OF_UNIT];


static uint32       cpuTagId;

static uint32 		reasonTbl[][2] = 
{
	{0,  											0,},
	{NIC_RX_REASON_RLDP_RLPP,						0,},
	{NIC_RX_REASON_RMA,								0,},
	{NIC_RX_REASON_IGR_VLAN_FILTER,					0,},
	{NIC_RX_REASON_INNER_OUTTER_CFI,				0,},
	{NIC_RX_REASON_MY_MAC,							0,},
	{NIC_RX_REASON_SPECIAL_TRAP,					0,},
	{NIC_RX_REASON_SPECIAL_COPY,					0,},
	{NIC_RX_REASON_ROUTING_EXCEPTION,				0,},
	{NIC_RX_REASON_UNKWN_UCST_MCST,					0,},
	{NIC_RX_REASON_MAC_CONSTRAINT_SYS,				NIC_RX_REASON_MAC_CONSTRAINT,},
	{NIC_RX_REASON_MAC_CONSTRAINT_VLAN,				NIC_RX_REASON_MAC_CONSTRAINT,},
	{NIC_RX_REASON_MAC_CONSTRAINT_PORT,				NIC_RX_REASON_MAC_CONSTRAINT,},
	{NIC_RX_REASON_CRC_ERROR,						0,},
	{NIC_RX_REASON_IP6_UNKWN_EXT_HDR,				0,},
	{NIC_RX_REASON_NORMAL_FWD,						0,},
};

static osal_mutex_t     nic_sem[RTK_MAX_NUM_OF_UNIT];


/*
 * Function Declaration
 */
static int32 _nic_init(uint32 unit, drv_nic_initCfg_t *pInitCfg);
static int32 _nic_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie);
static int32 _nic_rx_start(uint32 unit);
static int32 _nic_rx_stop(uint32 unit);
static int32 _nic_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags);
static int32 _nic_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb);
//static int32 _insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, maple_nic_cpuTag_t *pCpuTag);
//static int32 _removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, maple_nic_cpuTag_t *pCpuTag);

/*
 * Macro Definition
 */
/* semaphore handling */
#define NIC_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(nic_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_NIC), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define NIC_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(nic_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_NIC), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */
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
_insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, maple_nic_cpuTag_t *pCpuTag)
{
    uint8   *pU8;

    if ((NULL == pPkt) || (NULL == ppNewPkt)|| (NULL == pOffset) || (NULL == pCpuTag))
    {
        return RT_ERR_FAILED;
    }

    if (headroom < (CPU_TAG_ID_LEN + sizeof(maple_nic_cpuTag_t)))
    {
        return RT_ERR_FAILED;
    }

    /* copy the packet */
    pU8 = (uint8 *)(pPkt - (CPU_TAG_ID_LEN + sizeof(maple_nic_cpuTag_t)));
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
    osal_memcpy((uint8 *)(pU8 + 14), (uint8 *)pCpuTag, sizeof(maple_nic_cpuTag_t));

    if (ppNewPkt)
    {
        *ppNewPkt = (uint8 *)pU8;
    }

    if (pOffset)
    {
        *pOffset = (CPU_TAG_ID_LEN + sizeof(maple_nic_cpuTag_t));
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
#if 0
static int32
_removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, maple_nic_cpuTag_t *pCpuTag)
{
    if ((NULL == pPkt) || (NULL == ppNewPkt) || (NULL == pOffset) || (NULL == pCpuTag))
        return RT_ERR_FAILED;

    /* cpuTag copy: offset 14 bytes = DA(6) + SA(6)+ CPU_TAG_ID(2) */
    osal_memcpy((uint8 *)pCpuTag, (uint8 *)(pPkt + 14), sizeof(maple_nic_cpuTag_t));

    /* fix the packet */
    *(pPkt + 23) = *(pPkt + 11);
    *(pPkt + 22) = *(pPkt + 10);
    *(pPkt + 21) = *(pPkt + 9);
    *(pPkt + 20) = *(pPkt + 8);
    *(pPkt + 19) = *(pPkt + 7);
    *(pPkt + 18) = *(pPkt + 6);
    *(pPkt + 17) = *(pPkt + 5);
    *(pPkt + 16) = *(pPkt + 4);
    *(pPkt + 15) = *(pPkt + 3);
    *(pPkt + 14) = *(pPkt + 2);
    *(pPkt + 13)  = *(pPkt + 1);
    *(pPkt + 12)  = *(pPkt + 0);

    if (ppNewPkt)
    {
        *ppNewPkt = (pPkt + (CPU_TAG_ID_LEN + sizeof(maple_nic_cpuTag_t)));
    }

    if (pOffset)
    {
        *pOffset = (CPU_TAG_ID_LEN + sizeof(maple_nic_cpuTag_t));
    }

    return RT_ERR_OK;
} /* end of _removeCPUTag */
#endif

static int32 _nic_rx_reason_translate(drv_nic_pkt_t *pPacket)
{
#define RTL8380_MAX_REASON_NM  15
	uint16 reason = pPacket->rx_tag.reason;
	
	if (0 == reason)
	{
		if (pPacket->rx_tag.acl_hit)
			NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_ACL_HIT);
		if (pPacket->rx_tag.new_sa)
			NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_NEW_SA);
		if (pPacket->rx_tag.l2_pmv)
			NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_L2_PMV);
		if (pPacket->rx_tag.atk_type)
			NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_ATTACK);
		if (pPacket->rx_tag.mirror_hit)
			NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_MIRROR);
	}
	else if (reason <= RTL8380_MAX_REASON_NM)
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
    
#ifndef NIC_RX_THREAD
    uint32  i;
    drv_nic_rx_t nic_rx_handle = NIC_RX_NOT_HANDLED;
#endif

    RT_LOG(LOG_DEBUG, MOD_NIC, "ringId = %d", ringId);

    if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
    {
        osal_printf("[%02X]\n ", ringId);
    }

    if (ringId >= NIC_RXRING_NUM)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "invalid ringId(%d)!", ringId);
        nic_rx_failed_cntr++;
        return RT_ERR_FAILED;
    }
   
    
    /* Update software current pointer */
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_RX_CUR_DESC_ADDR_CTRL_ADDR(ringId), &temp);
    pNic_rxCDPIdx[ringId] = (uint32 *)(DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (temp | 0xa0000000)));    /* The limitation */


    if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
    {
        osal_printf("RX Current descriptor: [%02X]\n ", temp);
        ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_RX_BASE_DESC_ADDR_CTRL_ADDR(ringId), &temp);
        osal_printf("RX Base descriptor: [%02X]\n ", temp);
    }


    do
    {
        uint32          flag_es;
        uint32          offset = 0, scan_offset = 0;
        uint8           pkt_data;

        uint8           handled = FALSE;
        drv_nic_pkt_t   *pNewPacket;

        uint8           reclaim_mbuf = TRUE;

        nic_pkthdr_t    *pktHdr;
        maple_nic_cpuTag_t    *pCputag;
        drv_nic_pkt_t   *pPacket;

        nic_rx_queue_t  **ppRx_queue;
        nic_rx_queue_t  *pRx_queue = NULL;

        ppRx_queue = &pRx_queue;


        if ((*pNic_rxRDPIdx[ringId] & NIC_RING_SWOWNBIT) != 0)
            break;

        /* patch code: recover the value back */
        (*pNic_rxRDPIdx[ringId]) |= 0xa0000000; 

        /* Prepare to be reclaim */
        pktHdr = (nic_pkthdr_t *)((DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, ((*pNic_rxRDPIdx[ringId]) | 0xa0000000))) & NIC_ADDR_MASK);
        if (NULL == pktHdr || NULL == pktHdr->buf_addr || NULL == pktHdr->packet)
        {
            break;
        }
        
        flag_es = 0;
        if ((nic_chipId[NIC_DEFAULT_UNIT_ID] & 0xFFFF)  == 0x6966)
            flag_es = 1;

        /*Only for test chip*/
        if (flag_es)
        {
            int32 counter;
            for(counter = pktHdr->buf_len-1; counter>=0; counter--)
                *(pktHdr->packet->data+counter) = *(pktHdr->packet->data+counter-2);
        }

        /* NIC Rx debug message */
        if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
        {
            int i;
            int dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */
            
            osal_printf("=== [NIC RX Debug] ========================pktHdr->buf_addr %p\n", pktHdr->buf_addr);
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

        pPacket = pktHdr->packet;
        if (flag_es)
        {
            /*Test chip bug, 4 bytes may be lost*/
            pPacket->length = pktHdr->buf_len + 4;
        }
        else
            pPacket->length = pktHdr->buf_len;

        pPacket->tail = pPacket->data + pPacket->length;

        /* 89B port28 always has CPU-Tag, port26,24 can be set with CPU-Tag or not*/
        pCputag = &pktHdr->cpuTag;
        if (pCputag->un.rx.CPUTAGIF)
        {
            pPacket->rx_tag.qid                    = pCputag->un.rx.QID;
            pPacket->rx_tag.source_port         = pCputag->un.rx.SPN;
            pPacket->rx_tag.mirror_hit          = pCputag->un.rx.MIR_HIT;
            pPacket->rx_tag.acl_hit             = pCputag->un.rx.ACL_HIT;
            pPacket->rx_tag.acl_index           = pCputag->un.rx.ACL_IDX;
            pPacket->rx_tag.svid_tagged         = pCputag->un.rx.OTAGIF;
            pPacket->rx_tag.cvid_tagged         = pCputag->un.rx.ITAGIF;
            pPacket->rx_tag.fvid                = pCputag->un.rx.RVID;
            pPacket->rx_tag.mac_cst             = pCputag->un.rx.MAC_CST;
            pPacket->rx_tag.atk_type            = pCputag->un.rx.ATK_TYPE; /*attack type will be invalid?*/
            pPacket->rx_tag.new_sa              = pCputag->un.rx.NEW_SA;
            pPacket->rx_tag.l2_pmv              = pCputag->un.rx.L2_PMV; /*89B will be both static or dynamic*/
            pPacket->rx_tag.reason              = pCputag->un.rx.REASON;
            _nic_rx_reason_translate(pPacket);
        }
        else /*these branch will never be reached because 89B always store CPU tag in pktHdr other than payload*/
        {
            /* Do nothing here */
        }

        /*DMAC & SMAC offset*/
        scan_offset = 12;
        if (pPacket->rx_tag.svid_tagged)
        {
            pkt_data = *(pktHdr->packet->data + scan_offset + 2);
            pPacket->rx_tag.outer_pri = (pkt_data >> 5) & 0x7;
            pPacket->rx_tag.outer_vid = ((pkt_data & 0xF) << 8);
            pkt_data = *(pktHdr->packet->data + scan_offset + 3);
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
            pkt_data = *(pktHdr->packet->data + scan_offset + 2);
            pPacket->rx_tag.inner_pri = (pkt_data >> 5) & 0x7;
            pPacket->rx_tag.inner_vid = ((pkt_data & 0xF) << 8);
            pkt_data = *(pktHdr->packet->data + scan_offset + 3);
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
            osal_printf(" QID : %d \n", pPacket->rx_tag.qid);
            osal_printf(" SPN : %d \n", pPacket->rx_tag.source_port);
            osal_printf(" MIR_HIT : %d \n", pPacket->rx_tag.mirror_hit);
            osal_printf(" ACL_HIT : %d \n", pPacket->rx_tag.acl_hit);
            osal_printf(" ACL_IDX : %d \n", pPacket->rx_tag.acl_index);
            osal_printf(" OTAGIF : %d \n", pPacket->rx_tag.svid_tagged);
            osal_printf(" ITAGIF : %d \n", pPacket->rx_tag.cvid_tagged);
            osal_printf(" RVID : %d \n", pPacket->rx_tag.fvid);
            osal_printf(" MAC_CST : %d \n", pPacket->rx_tag.mac_cst);
            osal_printf(" ATK_TYPE : %d \n", pPacket->rx_tag.atk_type);
            osal_printf(" NEW_SA : %d \n", pPacket->rx_tag.new_sa);
            osal_printf(" L2_PMV : %d \n", pPacket->rx_tag.l2_pmv);
            osal_printf(" REASON : %d \n", pPacket->rx_tag.reason);
        }


        /* Begin to process the packet */
        nic_rx_handle = NIC_RX_NOT_HANDLED;
        
        /* Process interrupt callback function */
        if (0 != _nic_rx_intr_cb_cnt)
        {    
            for (i = 0; i < NIC_RX_CB_PRIORITY_NUMBER; i++)
            {
                if (_nic_rx_cb_tbl[i].rx_callback != NULL)
                {
                    nic_rx_handle = _nic_rx_cb_tbl[i].rx_callback(NIC_DEFAULT_UNIT_ID, pktHdr->packet, _nic_rx_cb_tbl[i].pCookie);
                }
                switch (nic_rx_handle)
                {
                    case NIC_RX_NOT_HANDLED:
                        break;
                    case NIC_RX_HANDLED:
                        break;
                    case NIC_RX_HANDLED_OWNED:
                        handled = TRUE;
                        pktHdr->packet = NULL;
                        break;
                    default:
                        break;                        
                }
                if (handled)
                {    
                    break;  
                }
            }
       
        }

        /* Process non-interrupt callback function */
        if (FALSE == handled) 
        {         
            /* Alloc a new packet data buffer */
            if (RT_ERR_OK == _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pNewPacket))
            {
                nic_rx_queueInfo_get(NIC_DEFAULT_UNIT_ID, ringId, ppRx_queue);        
                if ((pRx_queue->drop_thresh > 0) && (pRx_queue->count < pRx_queue->drop_thresh))
                {
                    if ((ret = nic_rx_pkt_enqueue(ringId, pPacket)) == RT_ERR_OK)
                    {                
                        pktHdr->packet = NULL;
                       
                        /* Notify RX thread */
                        nic_rx_thread_notify(NIC_DEFAULT_UNIT_ID);
                        nic_rx_handle = NIC_RX_HANDLED_OWNED;
                    }
                    else
                    {
                         _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pNewPacket);
                        nic_rx_lack_buf_cntr++;
                    }
                }
                else
                {
                    _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pNewPacket);
                    nic_rx_lack_buf_cntr++;
                }
            }
            else
            {
                nic_rx_lack_buf_cntr++;     
            }
        }

        if (NULL == pktHdr->packet)
        {
                pktHdr->packet = pNewPacket;
                pktHdr->buf_addr = (uint8 *)(DMA_ADDR_USR2KRN(pNewPacket->data));
                pktHdr->buf_size = (pNewPacket->end - pNewPacket->data);
                pktHdr->buf_len = 0;
                if ((ret = osal_cache_memory_flush((uint32)(DMA_ADDR_USR2KRN(pNewPacket->head)), (uint32)(pNewPacket->end - pNewPacket->head))) != RT_ERR_OK)
                {
                    osal_printf(" %s,%d, [osal_cache_memory_flush] failed!\n",__FUNCTION__, __LINE__);
                    return ret;
                }
        }
        else
        {
            pPacket->data -= offset;
            if ((ret = osal_cache_memory_flush((uint32)(DMA_ADDR_USR2KRN(pPacket->head)), (pPacket->end - pPacket->head))) != RT_ERR_OK)
            {
                osal_printf(" %s,%d, [osal_cache_memory_flush] failed!\n",__FUNCTION__, __LINE__);
                return ret;
            }    
        }                       

        /* Reclaim pkthdr/mbuf descriptor */
        if(TRUE == reclaim_mbuf)
        { 
            MEMORY_BARRIER();
            *(pktHdr->ring_entry) |= NIC_RING_SWOWNBIT;    /* Only set the SwOwn bit */

#ifndef NIC_CODE_REDUCE
            /* To guarantee it's write done */
            do
            {
                uint32 chk;
                chk = *(pktHdr->ring_entry);
            } while (0);
#endif
        }

        /* Jump to next */
        pNic_rxRDPIdx[ringId] += 1;
        if (pNic_rxRDPIdx[ringId] == (pNic_rxFDPBase[ringId] + NIC_RXRING_SIZE))
            pNic_rxRDPIdx[ringId] = pNic_rxFDPBase[ringId];
        nic_rx_success_cntr++;
    } while (pNic_rxRDPIdx[ringId] != pNic_rxCDPIdx[ringId]);


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

    do
    {
        nic_pkthdr_t *pktHdr;

        if ((*pNic_txRDPIdx[ringId] & NIC_RING_SWOWNBIT) != 0)
            break;

        /* Prepare to be reclaim */
        pktHdr = (nic_pkthdr_t *)((DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (*pNic_txRDPIdx[ringId] | 0xa0000000)) & NIC_ADDR_MASK));
        if (NULL == pktHdr || NULL == pktHdr->buf_addr
            || (NULL == pktHdr->tx_callback && NULL == pktHdr->packet))
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

        nic_tx_success_cntr_interrupt++;

        /* Jump to next */
        pNic_txRDPIdx[ringId] += 1;
        if (pNic_txRDPIdx[ringId] == (pNic_txFDPBase[ringId] + nic_8380_txing_size_of_max_limit))
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
        ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_RX_CUR_DESC_ADDR_CTRL_ADDR(i), &temp);
        temp = DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (temp | 0xa0000000));
        pNic_rxCDPIdx[i] = (uint32 *)temp;

        do
        {
            nic_pkthdr_t *pPktHdr;

            if ((*pNic_rxRDPIdx[i] & NIC_RING_SWOWNBIT) != 0)
                break;
     
            /* Prepare to be reclaim */
            pPktHdr = (nic_pkthdr_t *)((DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (*pNic_rxRDPIdx[i] | 0xa0000000)) & NIC_ADDR_MASK));
            
            if (NULL == pPktHdr || pPktHdr->packet != NULL)
                 break;

            /* Alloc a new packet data buffer */
            if (RT_ERR_OK != _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
            {
                RT_LOG(LOG_DEBUG, MOD_NIC, "Out of memory ! (alloc a new packet data buffer failed)");
                break;
            }

            pPktHdr->packet = pPacket;
            pPktHdr->buf_addr = ((uint8 *)(DMA_ADDR_USR2KRN(pPacket->data)));
            pPktHdr->buf_size = (pPacket->end - pPacket->data);
            pPktHdr->buf_len = 0;

            if ((ret = osal_cache_memory_flush((uint32)DMA_ADDR_USR2KRN(pPacket->head), (pPacket->end - pPacket->head))) != RT_ERR_OK)
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
 *      _nic_isr_mbRoutine2
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
 int32
_nic_isr_mbRoutine2(uint32 ringId)
{
    int32  ret = RT_ERR_FAILED;
    int32 rng_idx;
    drv_nic_pkt_t *pPacket;
    uint32 temp;
    uint32 *rng_ptr;
    uint32 *rng_ptr_tmp;

    /* In theory, code no needs to run this function because of rxRoutine implementation method. */
    /* Read Current Ring entry Pointer */
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_RX_CUR_DESC_ADDR_CTRL_ADDR(ringId), &temp);
    temp = DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (temp | 0xa0000000));
    rng_ptr_tmp = (uint32 *)temp;	
    
    /* Check from current Ring entry Pointer*/
    for (rng_idx = 0; rng_idx < NIC_RXRING_SIZE; rng_idx++)
    {
        nic_pkthdr_t *pPktHdr;

        rng_ptr = rng_ptr_tmp + rng_idx;

        /*If ends, wrap to start address*/
        if(rng_ptr == (pNic_rxFDPBase[ringId]+NIC_RXRING_SIZE))
            rng_ptr = pNic_rxFDPBase[ringId];

        /*If Now Switch Owned, Bypass*/
        if (((*rng_ptr) & NIC_RING_SWOWNBIT) != 0)
            continue;
		
        /*Check PktHeader has attached Cluster Space or not?*/
        pPktHdr = (nic_pkthdr_t *)((DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, ((*rng_ptr) | 0xa0000000)) & NIC_ADDR_MASK));


        if (NULL == pPktHdr || pPktHdr->packet != NULL)
            	continue;

        /* Alloc a new packet data buffer */
        if (RT_ERR_OK != _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
        {
	        if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
	        {
	            osal_printf("==Out of memory ! (alloc a new packet data buffer failed)\n");
	        }
            break;
        }

        pPktHdr->packet = pPacket;
        pPktHdr->buf_addr = ((uint8 *)(DMA_ADDR_USR2KRN(pPacket->data)));
        pPktHdr->buf_size = (pPacket->end - pPacket->data);
        pPktHdr->buf_len = 0;
        
        if ((ret = osal_cache_memory_flush((uint32)DMA_ADDR_USR2KRN(pPacket->head), (pPacket->end - pPacket->head))) != RT_ERR_OK)
        {
            osal_printf(" %s,%d, [osal_cache_memory_flush] failed!\n",__FUNCTION__, __LINE__);
            return ret;
        }
        MEMORY_BARRIER();
        
        *(pPktHdr->ring_entry) |= NIC_RING_SWOWNBIT;    /* Set the Swith Own bit */

#ifndef NIC_CODE_REDUCE
        /* To guarantee it's write done */
        do
        {
            uint32 chk;
            chk = *(pPktHdr->ring_entry);
        } while (0);
#endif
    }

    return RT_ERR_OK;
} /* end of _nic_isr_mbRoutine2 */



/* Function Name:
 *      r8380_isr_handler
 * Description:
 *      Nic interrupt handle routine.
 * Input:
 *      intr_status - isr status.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      OSAL_INT_HANDLED
 * Note:
 *      None
 */
int32 
r8380_isr_handler(uint32 intr_status)
{
    uint32  cpu_iisr;
    uint32 unit;

    unit = 0;

    cpu_iisr = intr_status;


    /* Rx have 8 rings (0..7); need to process when any rx ring isr done */
    if (cpu_iisr & INT_RX_DONE7_INT_RX_DONE0_MASK)
    {
        if (cpu_iisr & INT_RX_DONE7_MASK)   /* Process Rx ring 7 ISR */
        {
            _nic_isr_rxRoutine(7);
        }

        if (cpu_iisr & INT_RX_DONE6_MASK)   /* Process Rx ring 6 ISR */
        {
            _nic_isr_rxRoutine(6);
        }

        if (cpu_iisr & INT_RX_DONE5_MASK)   /* Process Rx ring 5 ISR */
        {
            _nic_isr_rxRoutine(5);
        }

        if (cpu_iisr & INT_RX_DONE4_MASK)   /* Process Rx ring 4 ISR */
        {
            _nic_isr_rxRoutine(4);
        }

        if (cpu_iisr & INT_RX_DONE3_MASK)   /* Process Rx ring 3 ISR */
        {
            _nic_isr_rxRoutine(3);
        }

        if (cpu_iisr & INT_RX_DONE2_MASK)   /* Process Rx ring 2 ISR */
        {
            _nic_isr_rxRoutine(2);
        }

        if (cpu_iisr & INT_RX_DONE1_MASK)   /* Process Rx ring 1 ISR */
        {
            _nic_isr_rxRoutine(1);
        }

        if (cpu_iisr & INT_RX_DONE0_MASK)   /* Process Rx ring 0 ISR */
        {
            _nic_isr_rxRoutine(0);
        }
    }


    /* Tx have 2 rings (0..1); need to process when any tx ring isr done */
    if (cpu_iisr & INT_TX_DONE1_INT_TX_DONE0_MASK)
    {
        if (cpu_iisr & INT_TX_DONE1_MASK)   /* Process Tx ring 1 ISR */
        {
            NIC_SEM_LOCK(unit);
            _nic_isr_txRoutine(1);
            NIC_SEM_UNLOCK(unit);
        }

        if (cpu_iisr & INT_TX_DONE0_MASK)   /* Process Tx ring 0 ISR */
        {
            NIC_SEM_LOCK(unit);
            _nic_isr_txRoutine(0);
            NIC_SEM_UNLOCK(unit);
        }
    }


    /* RX Cluster Space Runout */
    if (cpu_iisr & RTL8380_DMA_IF_INTR_STS_RX_RUN_OUT_MASK)
    {
        if (cpu_iisr & (1UL<<7))   /* Process Rx ring 7 Runout ISR */
        {
           _nic_isr_rxRoutine(7);
           _nic_isr_mbRoutine2(7);
        }

        if (cpu_iisr & (1UL<<6))   /* Process Rx ring 6 Runout ISR */
        {
           _nic_isr_rxRoutine(6);
           _nic_isr_mbRoutine2(6);
        }

        if (cpu_iisr & (1UL<<5))   /* Process Rx ring 5 Runout ISR */
        {
           _nic_isr_rxRoutine(5);
           _nic_isr_mbRoutine2(5);
        }

        if (cpu_iisr & (1UL<<4))   /* Process Rx ring 4 Runout ISR */
        {
           _nic_isr_rxRoutine(4);
           _nic_isr_mbRoutine2(4);
        }

        if (cpu_iisr & (1UL<<3))   /* Process Rx ring 3 Runout ISR */
        {
           _nic_isr_rxRoutine(3);
           _nic_isr_mbRoutine2(3);
        }

        if (cpu_iisr & (1UL<<2))   /* Process Rx ring 2 Runout ISR */
        {
           _nic_isr_rxRoutine(2);
           _nic_isr_mbRoutine2(2);
        }

        if (cpu_iisr & (1UL<<1))   /* Process Rx ring 1 Runout ISR */
        {
           _nic_isr_rxRoutine(1);
           _nic_isr_mbRoutine2(1);
        }

        if (cpu_iisr & (1UL<<0))   /* Process Rx ring 0 Runout ISR */
        {
           _nic_isr_rxRoutine(0);
           _nic_isr_mbRoutine2(0);
        }
    }

    return RT_ERR_OK;
} /* end of r8380_isr_handler */

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
    uint32  flag_es;
    uint32  temp, base;
    uint32  index;
    int32   i, j, k;
    int32   ret = RT_ERR_FAILED;

    uint32 cnt_val;

    /* Check arguments */
    RT_PARAM_CHK(NULL == pInitCfg->pkt_alloc, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pInitCfg->pkt_free, RT_ERR_NULL_POINTER);

    /* Reset the NIC Tx/Rx debug information */
    nic_debug_flag = 0;
    nic_tx_success_cntr = 0;
    nic_tx_failed_cntr = 0;
    nic_rx_success_cntr = 0;
    nic_rx_failed_cntr = 0;
    nic_rx_lack_buf_cntr = 0;

    flag_es = 0;
    if ((nic_chipId[unit] & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        nic_8380_txing_size_of_max_limit = 1;
    else
        nic_8380_txing_size_of_max_limit = NIC_8380_TXRING_SIZE;

    _nic_init_conf.pkt_size  = pInitCfg->pkt_size;
    _nic_init_conf.pkt_alloc = pInitCfg->pkt_alloc;
    _nic_init_conf.pkt_free  = pInitCfg->pkt_free;

    /* Disable NIC rx/tx*/
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), RTL8380_MAC_PORT_CTRL_TXRX_EN_OFFSET, RTL8380_MAC_PORT_CTRL_TXRX_EN_MASK, 0);
    
    osal_time_usleep(50 * 1000); /* delay 50mS */
    
    if (flag_es)
    {
        /*Donot reset NIC, because of NIC RESET bug*/
    }
    else
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Reset NIC (R8380)... ");
    
        ioal_mem32_field_write(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_MASK, 1);
    
        do
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
            ioal_mem32_field_read(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_MASK, &temp);
        } while (temp != 0);

        RT_LOG(LOG_DEBUG, MOD_NIC, "OK");
    }

    /* CPU port: Enable MAC Tx/Rx */
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), RTL8380_MAC_PORT_CTRL_TXRX_EN_OFFSET, RTL8380_MAC_PORT_CTRL_TXRX_EN_MASK, 3);

    /*Speed, duplex, flow control*/
    ioal_mem32_write(unit, RTL8380_MAC_FORCE_MODE_CTRL_ADDR(28), 0x6192F);

    /*Set port28 CRC error forward, disable CRC check*/
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), \
        RTL8380_MAC_PORT_CTRL_RX_CHK_CRC_EN_OFFSET, RTL8380_MAC_PORT_CTRL_RX_CHK_CRC_EN_MASK, 1);
    
    /*HOL register*/
    if (flag_es)
    {
        /*Only for test chip*/
        for(index = 0; index<NIC_RXRING_NUM; index++)
        ioal_mem32_field_write(unit, RTL8380_DMA_IF_RX_RING_SIZE_ADDR(index), \
        RTL8380_DMA_IF_RX_RING_SIZE_SIZE_OFFSET(index), RTL8380_DMA_IF_RX_RING_SIZE_SIZE_MASK(index), 0xF); 

        /*clear the counter*/
        for(index = 0; index<NIC_RXRING_NUM; index++)
        {
            /*Read ring counter*/
            ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_RX_RING_CNTR_ADDR(index), \
                RTL8380_DMA_IF_RX_RING_CNTR_CNTR_OFFSET(index), RTL8380_DMA_IF_RX_RING_CNTR_CNTR_MASK(index), &cnt_val); 
            /*Write the same ring counter*/
            ioal_mem32_field_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_RX_RING_CNTR_ADDR(index), \
                RTL8380_DMA_IF_RX_RING_CNTR_CNTR_OFFSET(index), RTL8380_DMA_IF_RX_RING_CNTR_CNTR_MASK(index), cnt_val); 
        }
    }
    else
    {
        /*Disable Head of Line featrue*/
        for(index = 0; index<NIC_RXRING_NUM; index++)
        ioal_mem32_field_write(unit, RTL8380_DMA_IF_RX_RING_SIZE_ADDR(index), \
            RTL8380_DMA_IF_RX_RING_SIZE_SIZE_OFFSET(index), RTL8380_DMA_IF_RX_RING_SIZE_SIZE_MASK(index), 0x0); 
    }

    if (flag_es)
    {
        /*Set port28 forwarding extracted from inner VLAN Tag*/
        index = 28;
        ioal_mem32_field_write(unit, RTL8380_VLAN_PORT_PB_VLAN_ADDR(index), \
            RTL8380_VLAN_PORT_PB_VLAN_IPVID_FMT_OFFSET, RTL8380_VLAN_PORT_PB_VLAN_IPVID_FMT_MASK, 0x0); 
    }

    /* Reset to default value */
    ioal_mem32_write(unit, RTL8380_DMA_IF_INTR_MSK_ADDR, CPUIIMR_DISABLE_MASK);
    ioal_mem32_write(unit, RTL8380_DMA_IF_INTR_STS_ADDR, CPUIISR_CLEARALL_MASK);
    if (flag_es)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_CTRL_ADDR, DMA_IF_CTRL_DISABLE_MASK);
    }
    else
    {
        /*RX Trunk Enable, Trunk Length is 0x640; TX Padding Enable*/
        ioal_mem32_write(unit, RTL8380_DMA_IF_CTRL_ADDR, 0x6400020);
    }

    /* Prepare for the NIC Info structure space */
    /* Now put the Ring entry and PktHeader into SDRAM */
    RT_ERR_CHK(ioal_init_memRegion_get(unit, IOAL_MEM_DESC, &base), ret);  
    pNicInfo = (nic_info_t *)base;    
   
    if (sizeof(nic_info_t) > RTL8380_DESC_MEM_SIZE)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error: The required size of nic_info_t is bigger than allocated memory!");
        return RT_ERR_FAILED;
    }

    if ((NIC_RXRING_NUM * NIC_RXRING_SIZE * _nic_init_conf.pkt_size) > RTL8380_DMA_MEM_SIZE)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error: The required size of cluster is bigger than allocated memory!");
        return RT_ERR_FAILED;
    }

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
    RT_ERR_CHK(osal_cache_memory_flush((uint32)KRNVIRT2(RTL8380_DESC_PHYS_BASE), sizeof(nic_info_t)), ret);   
    
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
                ((uint32)DESC_ADDR_USR2KRN(unit, pPktHdr)) | NIC_RING_WRAPBIT : \
                ((uint32)DESC_ADDR_USR2KRN(unit, pPktHdr));

            i++;
        }
    }

    /* Tx pktHdrs */
    i = 0;
    for (j = 0; j < NIC_TXRING_NUM; j++)
    {
        for (k = 0; k < nic_8380_txing_size_of_max_limit; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->txPktHdr[i];
            pPktHdr->buf_size   = 0;
            pPktHdr->pkt_offset = 0;
            pPktHdr->buf_len    = 0;
            pPktHdr->ring_entry  = (pNic_txFDPBase[j] + k);

            /*Set all the entries to CPU owned*/
            *(pNic_txFDPBase[j] + k) = ((k + 1) == nic_8380_txing_size_of_max_limit)? \
                ((uint32)DESC_ADDR_USR2KRN(unit, pPktHdr)) | NIC_RING_WRAPBIT : \
                ((uint32)DESC_ADDR_USR2KRN(unit, pPktHdr));

            i++;
        }
    }

    /* Setting Registers */
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_RX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)DESC_ADDR_USR2KRN(unit, pNic_rxFDPBase[i]));
    }

    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_TX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)DESC_ADDR_USR2KRN(unit, pNic_txFDPBase[i]));  
    }
    

    ioal_mem32_field_read(unit, RTL8380_MAC_CPU_TAG_ID_CTRL_ADDR, RTL8380_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_OFFSET, \
    RTL8380_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_MASK, &cpuTagId);    
    
#if defined(__MODEL_USER__) || defined(__MODEL_KERNEL__)
    if (0 == cpuTagId)
    {
        ioal_mem32_write(unit, RTL8380_MAC_CPU_TAG_ID_CTRL_ADDR, REALTEK_CPUTAG_ID << RTL8380_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_OFFSET);
        cpuTagId = REALTEK_CPUTAG_ID;
    }
#endif

    /* Prepare the cluster space */
    _nic_isr_mbRoutine();

    /*Every thing is ok now, NIC can RX/TX, and enable interrupt trigger*/
    ioal_mem32_write(unit, RTL8380_DMA_IF_INTR_MSK_ADDR, CPUIIMR_ENABLE_MASK);  
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_RX_EN_OFFSET, RTL8380_DMA_IF_CTRL_RX_EN_MASK, 1);
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_TX_EN_OFFSET, RTL8380_DMA_IF_CTRL_TX_EN_MASK, 1);

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
    uint32  flag_es;
    uint32  txRingId;
    uint32  packetNum;
    int32   ret = RT_ERR_FAILED;
    nic_pkthdr_t    *pPktHdr;

    if (NULL == pPacket)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - pPacket is NULL!");
        return RT_ERR_FAILED;
    }

    flag_es = 0;
    if ((nic_chipId[unit] & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        txRingId = 0;   /*FPGA platform always TX Ring0*/
    else
    {
        /* Step: Decide the target queue */
        txRingId = (pPacket->tx_tag.priority > 3) ? 1 : 0;    /* mapping 8 priority to 2 queues */
    }

    /* Step: Find a pktHdr */
    if (((*pNic_txCDPIdx[txRingId]) & NIC_RING_SWOWNBIT) != 0)
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

    pPktHdr = (nic_pkthdr_t *)((DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (*pNic_txCDPIdx[txRingId] | 0xa0000000))) & NIC_ADDR_MASK);
    
    /* Step: Double Confirm (Check the pktHdr status) */
    if (NULL == pPktHdr || NULL != pPktHdr->packet)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr is NOT available!");
		
        return RT_ERR_MBUF_PKT_NOT_AVAILABLE;
    }


    /* Set  the CPU Tx tag into the packet header */
    if (pPacket->as_txtag)
    {
        /*CPU Tag content*/
        pPktHdr->cpuTag.un.tx.CPUTAGIF       = 0x4; /*PROTO ID*/
        pPktHdr->cpuTag.un.tx.BP_FLTR1        = pPacket->tx_tag.bp_fltr1;
        pPktHdr->cpuTag.un.tx.BP_FLTR2        = pPacket->tx_tag.bp_fltr2;
        pPktHdr->cpuTag.un.tx.AS_TAGSTS     = pPacket->tx_tag.as_tagSts;
        pPktHdr->cpuTag.un.tx.ACL_ACT         = pPacket->tx_tag.acl_act;
        pPktHdr->cpuTag.un.tx.RVID_SEL        = pPacket->tx_tag.rvid_sel;
        pPktHdr->cpuTag.un.tx.L2LEARNING     = pPacket->tx_tag.l2_learning;
        pPktHdr->cpuTag.un.tx.AS_PRI             = pPacket->tx_tag.as_priority;
        pPktHdr->cpuTag.un.tx.PRI                   = pPacket->tx_tag.priority;

        /*Final chip*/
        pPktHdr->cpuTag.un.tx.DPM_TYPE         = pPacket->tx_tag.dpm_type;
        pPktHdr->cpuTag.un.tx.AS_DPM            = pPacket->tx_tag.as_dst_port_mask;
        pPktHdr->cpuTag.un.tx.DPM                  = pPacket->tx_tag.dst_port_mask;
    }
    else
        pPktHdr->cpuTag.un.tx.CPUTAGIF = FALSE;

    /* Step: Calc the pktbuf number */
    packetNum = 1;    /* Support single pktBuf now - one descriptor vs. one mbuf */

    pPktHdr->tx_callback = fTxCb;    /* Tx Callback function */
    pPktHdr->cookie = pCookie;
    pPktHdr->packet = pPacket;
    pPktHdr->buf_addr = (uint8 *)(DMA_ADDR_USR2KRN(pPacket->data));

    /*MAC28 will drop pkts less than 64bytes, so must extend pkt to 64bytes if less than 64bytes*/
    if (flag_es)
    {
        if(pPacket->length < 64)
        {
            pPktHdr->buf_size = 64;
            pPktHdr->buf_len = 64;
        }
        else
        {
            pPktHdr->buf_size = (pPacket->end - pPacket->data);
            if (flag_es)
                pPktHdr->buf_len = pPacket->length;
            else
                pPktHdr->buf_len = pPacket->length+4;
        }
    }
    else
    {
        /*Final Chip should enable padding featrue, otherwise the code below not works right*/
        pPktHdr->buf_size = (pPacket->end - pPacket->data);
        
        /*To avoid packets(less than 60bytes) occur uncontrolled 4 bytes rubbish, here pPacket->length
            will not add 4bytes for these kind of packets(less than 60bytes)*/
        if(pPacket->length < 60)
        {
            pPktHdr->buf_len = pPacket->length;
        }
        else
        {
            pPktHdr->buf_len = pPacket->length+4;
        }
    }

    if ((ret = osal_cache_memory_flush((uint32)DMA_ADDR_USR2KRN(pPacket->head), (pPacket->end - pPacket->head))) != RT_ERR_OK)
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

    nic_tx_success_cntr_tx_debug++;

    /* Jump to next */
    pNic_txCDPIdx[txRingId] += 1;
    if (pNic_txCDPIdx[txRingId] == (pNic_txFDPBase[txRingId] + nic_8380_txing_size_of_max_limit))
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
            osal_printf("%02X ", *(uint8*)((uint32)DMA_ADDR_KRN2USR((pPktHdr->buf_addr + i))));
            if (15 == (i % 16))
                osal_printf("\n");
        }
        osal_printf("\n");
    }
    if ((nic_debug_flag & DEBUG_TX_CPU_TAG_BIT) && pPacket->as_txtag)
    {
        osal_printf("=== [NIC TX Debug - CPU Tx Tag Information] ============ \n");
        osal_printf(" BP_FLTR1 : 0x%0x \n", pPktHdr->cpuTag.un.tx.BP_FLTR1);
        osal_printf(" BP_FLTR2 : 0x%0x \n", pPktHdr->cpuTag.un.tx.BP_FLTR2);
        osal_printf(" AS_TAGSTS : 0x%0x \n", pPktHdr->cpuTag.un.tx.AS_TAGSTS);
        osal_printf(" ACL_ACT : 0x%0x \n", pPktHdr->cpuTag.un.tx.ACL_ACT);
        osal_printf(" RVID_SEL : 0x%0x \n", pPktHdr->cpuTag.un.tx.RVID_SEL);
        osal_printf(" L2LEARNING : 0x%0x \n", pPktHdr->cpuTag.un.tx.L2LEARNING);
        osal_printf(" AS_PRI : 0x%0x \n", pPktHdr->cpuTag.un.tx.AS_PRI);
        osal_printf(" PRI : 0x%0x \n", pPktHdr->cpuTag.un.tx.PRI);
        osal_printf(" DPM_TYPE : 0x%0x \n", pPktHdr->cpuTag.un.tx.DPM_TYPE);
        osal_printf(" AS_DPM : 0x%0x \n", pPktHdr->cpuTag.un.tx.AS_DPM);
        osal_printf(" DPM : 0x%0x \n", pPktHdr->cpuTag.un.tx.DPM);
    }

    {   /*Chip bug, sometimes lextra bus will access wrong data*/
        uint32 times;
        uint32 cnt_val;
        times = 0;
        while(times < 10)
        {
            ioal_mem32_read(unit, RTL8380_DMA_IF_CTRL_ADDR, &cnt_val);
            if((cnt_val & 0xc) == 0xc)break;
            times++;
        }
    }

    /* Set the TX Fetch Notify bit */
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_TX_FETCH_OFFSET, RTL8380_DMA_IF_CTRL_TX_FETCH_MASK, 1);

    if (flag_es)
        osal_time_usleep(10); /* delay 10us */

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
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_RX_EN_OFFSET, RTL8380_DMA_IF_CTRL_RX_EN_MASK, 1);

    RT_LOG(LOG_DEBUG, MOD_NIC, "NIC (R8380) Rx Start... ");

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
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_RX_EN_OFFSET, RTL8380_DMA_IF_CTRL_RX_EN_MASK, 0);
    
    RT_LOG(LOG_DEBUG, MOD_NIC, "NIC (R8380) Rx Stop... ");
    
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

    if (NULL == _nic_rx_cb_tbl[priority].rx_callback)
    {
        _nic_rx_cb_tbl[priority].rx_callback = fRxCb;
        _nic_rx_cb_tbl[priority].pCookie     = pCookie;
        _nic_rx_intr_cb_cnt++;
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
        _nic_rx_intr_cb_cnt--;
    }
    else
    {
        /* Handler is nonexistent */
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of _nic_rx_unregister */


static int32
_nic_reset(uint32 unit)
{
    /*To be continued*/
    return RT_ERR_OK;
}

/* Function Name:
 *      r8380_init
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
r8380_init(uint32 unit, drv_nic_initCfg_t *pInitCfg)
{
    int32 ret = RT_ERR_FAILED;

    /* Check arguments */
    RT_PARAM_CHK(NULL == pInitCfg, RT_ERR_NULL_POINTER);

    /* Check whether it is inited, if inited, return fail */
    if (INIT_COMPLETED == nic_init[unit])
        return ret;

    /* create semaphore */
    nic_sem[unit] = osal_sem_mutex_create();
    if (0 == nic_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_NIC), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    /* Initialize the NIC module */
    RT_ERR_CHK(_nic_init(unit, pInitCfg), ret);
    RT_ERR_CHK(nic_rx_thread_init(unit), ret);

    /* set init flag to complete init */
    nic_init[unit] = INIT_COMPLETED;

    RT_LOG(LOG_DEBUG, MOD_NIC, "Init NIC R8380...OK");    

    return ret;
} /* end of r8380_init */

/* Function Name:
 *      r8380_pkt_tx
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
 *      None
 */
int32
r8380_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
{
    uint32  txRingId;
    uint32  tryCount = 5;
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(NULL == pPacket, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == fTxCb, RT_ERR_NULL_POINTER);

    NIC_SEM_LOCK(unit);

    /* Step: Decide the target queue, mapping 8 priority to 2 queues */
    txRingId = (pPacket->tx_tag.priority > 3) ? 1 : 0;

    do
    {
        /* Dispatch */
        ret = _nic_pkt_tx(unit, pPacket, fTxCb, pCookie);
        if (RT_ERR_MBUF_PKT_NOT_AVAILABLE == ret)
        {            
            _nic_isr_txRoutine(txRingId);
        }
        else
        {
            break;
        }

        tryCount--;
    } while(tryCount > 0);

    NIC_SEM_UNLOCK(unit);

    if (RT_ERR_OK == ret)
        nic_tx_success_cntr++;
    else
        nic_tx_failed_cntr++;


    return ret;
} /* end of r8380_pkt_tx */

/* Function Name:
 *      r8380_rx_start
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
r8380_rx_start(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
	
    /* Check arguments */
    ret = nic_rx_thread_create(unit);

    /* Dispatch */
    ret = _nic_rx_start(unit);

    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8380_rx_start */


/* Function Name:
 *      r8380_rx_stop
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
r8380_rx_stop(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);

    /* Check arguments */
    ret = nic_rx_thread_destroy(unit);

    /* Dispatch */
    ret = _nic_rx_stop(unit);

    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8380_rx_stop */


/* Function Name:
 *      r8380_rx_register
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
r8380_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

    NIC_SEM_LOCK(unit);

    /* Dispatch */
    ret = _nic_rx_register(unit, priority, fRxCb, pCookie, flags);

    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8380_rx_register */

/* Function Name:
 *      r8380_rx_unregister
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
r8380_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

    NIC_SEM_LOCK(unit);

    /* Dispatch */
    ret = _nic_rx_unregister(unit, priority, fRxCb);

    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8380_rx_unregister */

/* Function Name:
 *      r8380_pkt_alloc
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
r8380_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(size > _nic_init_conf.pkt_size, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == _nic_init_conf.pkt_alloc, RT_ERR_NULL_POINTER);

    /* Dispatch */
    ret = _nic_init_conf.pkt_alloc(unit, size, flags, ppPacket);

    return ret;
} /* end of r8380_pkt_alloc */

/* Function Name:
 *      r8380_pkt_free
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
r8380_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket)
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
} /* end of r8380_pkt_free */

/* NIC Tx/Rx debug */
/* Function Name:
 *      r8380_debug_set
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
r8380_debug_set(uint32 unit, uint32 flags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    nic_debug_flag = flags;

    return RT_ERR_OK;
} /* end of r8380_debug_set */

/* Function Name:
 *      r8380_debug_get
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
r8380_debug_get(uint32 unit, uint32 *pFlags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    *pFlags = nic_debug_flag;

    return RT_ERR_OK;
} /* end of r8380_debug_get */

/* Function Name:
 *      r8380_counter_dump
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
r8380_counter_dump(uint32 unit)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    osal_printf("Tx success counter : %0d \n", nic_tx_success_cntr);
    osal_printf("Tx failed counter  : %0d \n", nic_tx_failed_cntr);
    osal_printf("Rx success counter : %0d \n", nic_rx_success_cntr);
    osal_printf("Rx failed counter  : %0d \n", nic_rx_failed_cntr);
    osal_printf("Tx success counter (interrupt) : %0d \n", nic_tx_success_cntr_interrupt);
    osal_printf("Tx  success counter (debug) : %0d \n", nic_tx_success_cntr_tx_debug);
    osal_printf(" sram_base[unit] : %8x \n",  desc_base[unit]);
    return RT_ERR_OK;
} /* end of r8380_counter_dump */

/* Function Name:
 *      r8380_counter_clear
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
r8380_counter_clear(uint32 unit)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    nic_tx_success_cntr = 0;
    nic_tx_failed_cntr = 0;
    nic_rx_success_cntr = 0;
    nic_rx_failed_cntr = 0;

    nic_tx_success_cntr_interrupt = 0;
    nic_tx_success_cntr_tx_debug = 0;
    NIC_SEM_UNLOCK(unit);
	
    return RT_ERR_OK;
} /* end of r8380_counter_clear */

/* Function Name:
 *      r8380_bufStatus_dump
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
r8380_bufStatus_dump(uint32 unit)
{
    uint32  i, value;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

#if 0
    osal_printf("RXRING  SW_rxFDPBase  SW_RxCDPIdx  HW_RxCDPIdx  SW_RxRDPIdx \n");
    osal_printf("=========================================================== \n");
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_read(unit, RTL8380_DMA_IF_RX_CUR_DESC_ADDR_CTRL_ADDR(i), &value);

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
#endif

    osal_printf("TXRING  SW_txFDPBase  SW_TxCDPIdx  HW_TxCDPIdx  SW_TxRDPIdx \n");
    osal_printf("=======================TXXXXXXXXXXXXXXXXXXX=========================== \n");
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_read(unit, RTL8380_DMA_IF_TX_CUR_DESC_ADDR_CTRL_ADDR(i), &value);

        osal_printf(" %d(p)   0x%08x    0x%08x   0x%08x   0x%08x \n",
                    i, ((uint32)pNic_txFDPBase[i]), ((uint32)pNic_txCDPIdx[i]), value, ((uint32)pNic_txRDPIdx[i]));
    }
    osal_printf("\n");

    osal_printf("TXRING txFDPBase  Base+1   Base+2   Base+3.....\n");
#if 1
    osal_printf("========================================\n");
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        osal_printf(" %d(p)  0x%08x %08x %08x %08x %08x %08x %08x %08x\n",
                    i, ((uint32)(pNic_txFDPBase[i]+0)), ((uint32)(pNic_txFDPBase[i] + 1)),
                    ((uint32)(pNic_txFDPBase[i] + 2)), ((uint32)(pNic_txFDPBase[i] + 3)),
                    ((uint32)(pNic_txFDPBase[i] + 4)), ((uint32)(pNic_txFDPBase[i] + 5)),
                    ((uint32)(pNic_txFDPBase[i] + 6)), ((uint32)(pNic_txFDPBase[i] + 7)));

        osal_printf(" %d(p)  0x%08x %08x %08x %08x %08x %08x %08x %08x\n",
                    i,((uint32)(pNic_txFDPBase[i] + 8)), ((uint32)(pNic_txFDPBase[i] + 9)),
                    ((uint32)(pNic_txFDPBase[i] + 10)), ((uint32)(pNic_txFDPBase[i] + 11)),
                    ((uint32)(pNic_txFDPBase[i] + 12)), ((uint32)(pNic_txFDPBase[i] + 13)),
                    ((uint32)(pNic_txFDPBase[i] + 14)), ((uint32)(pNic_txFDPBase[i] + 15)));

        osal_printf(" %d(p)  0x%08x %08x %08x %08x %08x %08x %08x %08x\n",
                    i,((uint32)(pNic_txFDPBase[i] + 16)), ((uint32)(pNic_txFDPBase[i] + 17)),
                     ((uint32)(pNic_txFDPBase[i] + 18)), ((uint32)(pNic_txFDPBase[i] + 19)),
                    ((uint32)(pNic_txFDPBase[i] + 20)), ((uint32)(pNic_txFDPBase[i] + 21)),
                    ((uint32)(pNic_txFDPBase[i] + 22)), ((uint32)(pNic_txFDPBase[i] + 23)));


        osal_printf(" %d(p)  0x%08x %08x %08x %08x %08x %08x %08x %08x\n",
                    i,((uint32)(pNic_txFDPBase[i] + 24)), ((uint32)(pNic_txFDPBase[i] + 25)),
                    ((uint32)(pNic_txFDPBase[i] + 26)), ((uint32)(pNic_txFDPBase[i] + 27)),
                    ((uint32)(pNic_txFDPBase[i] + 28)), ((uint32)(pNic_txFDPBase[i] + 29)),
                    ((uint32)(pNic_txFDPBase[i] + 30)), ((uint32)(pNic_txFDPBase[i] + 31)));
    }
#endif

#if 1
    osal_printf("--------------------------------------------\n");
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        osal_printf(" %d(v)  0x%08x %08x %08x %08x %08x %08x %08x %08x \n",
                    i, (*(uint32 *)(pNic_txFDPBase[i]+0)), (*(uint32 *)(pNic_txFDPBase[i] + 1)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 2)), (*(uint32 *)(pNic_txFDPBase[i] + 3)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 4)), (*(uint32 *)(pNic_txFDPBase[i] + 5)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 6)), (*(uint32 *)(pNic_txFDPBase[i] + 7))); 
        osal_printf(" %d(v)  0x%08x %08x %08x %08x %08x %08x %08x %08x \n",
                    i,   (*(uint32 *)(pNic_txFDPBase[i] + 8)), (*(uint32 *)(pNic_txFDPBase[i] + 9)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 10)), (*(uint32 *)(pNic_txFDPBase[i] + 11)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 12)), (*(uint32 *)(pNic_txFDPBase[i] + 13)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 14)), (*(uint32 *)(pNic_txFDPBase[i] + 15))); 
        osal_printf(" %d(v)  0x%08x %08x %08x %08x %08x %08x %08x %08x \n",
                    i,  (*(uint32 *)(pNic_txFDPBase[i] + 16)), (*(uint32 *)(pNic_txFDPBase[i] + 17)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 18)), (*(uint32 *)(pNic_txFDPBase[i] + 19)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 20)), (*(uint32 *)(pNic_txFDPBase[i] + 21)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 22)), (*(uint32 *)(pNic_txFDPBase[i] + 23)) );
        osal_printf(" %d(v)  0x%08x %08x %08x %08x %08x %08x %08x %08x \n",
                    i,  (*(uint32 *)(pNic_txFDPBase[i] + 24)), (*(uint32 *)(pNic_txFDPBase[i] + 25)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 26)), (*(uint32 *)(pNic_txFDPBase[i] + 27)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 28)), (*(uint32 *)(pNic_txFDPBase[i] + 29)),
                    (*(uint32 *)(pNic_txFDPBase[i] + 30)), (*(uint32 *)(pNic_txFDPBase[i] + 31)) );
    }
#endif

    osal_printf("\n");

    ioal_mem32_read(unit, RTL8380_DMA_IF_INTR_MSK_ADDR, &value);
    osal_printf("CPUIIMR[0xBB007830] = 0x%08x ", value);
    osal_printf("[EN_PHD7_0=0x%x, ", (value & RTL8380_DMA_IF_INTR_MSK_RX_RUN_OUT_MASK) >> RTL8380_DMA_IF_INTR_MSK_RX_RUN_OUT_OFFSET);
    osal_printf("   EN_TX_DONE1_0=0x%x, ", (value & RTL8380_DMA_IF_INTR_MSK_TX_DONE_MASK) >> RTL8380_DMA_IF_INTR_MSK_TX_DONE_OFFSET);
    osal_printf("EN_RX_DONE7_0=0x%x, ", (value & RTL8380_DMA_IF_INTR_MSK_RX_DONE_MASK) >> RTL8380_DMA_IF_INTR_MSK_RX_DONE_OFFSET);
    osal_printf("EN_TX_ALLDONE1_0=0x%x]\n", (value & RTL8380_DMA_IF_INTR_MSK_TX_ALL_DONE_MASK) >> RTL8380_DMA_IF_INTR_MSK_TX_ALL_DONE_OFFSET);

    ioal_mem32_read(unit, RTL8380_DMA_IF_INTR_STS_ADDR, &value);
    osal_printf("CPUIISR[0xBB003130] = 0x%08x ", value);
    osal_printf("[INT_PHDS7_0=0x%x, ", (value & RTL8380_DMA_IF_INTR_STS_RX_RUN_OUT_MASK) >> RTL8380_DMA_IF_INTR_STS_RX_RUN_OUT_OFFSET);
    osal_printf("   INT_TX_DONE1_0=0x%x, ", (value & RTL8380_DMA_IF_INTR_STS_TX_DONE_MASK) >> RTL8380_DMA_IF_INTR_STS_TX_DONE_OFFSET);
    osal_printf("INT_RX_DONE7_0=0x%x, ", (value & RTL8380_DMA_IF_INTR_STS_RX_DONE_MASK) >> RTL8380_DMA_IF_INTR_STS_RX_DONE_OFFSET);
    osal_printf("INT_TX_ALL_DONE1_0=0x%x]\n", (value & RTL8380_DMA_IF_INTR_STS_TX_ALL_DONE_MASK) >> RTL8380_DMA_IF_INTR_STS_TX_ALL_DONE_OFFSET);

    ioal_mem32_read(unit, RTL8380_DMA_IF_CTRL_ADDR, &value);  
    osal_printf("CPUICR[0xBB003134] = 0x%08x ", value);
    osal_printf("[TX_CMD=0x%x, ", (value & RTL8380_DMA_IF_CTRL_TX_EN_MASK) >> RTL8380_DMA_IF_CTRL_TX_EN_OFFSET);
    osal_printf("RX_CMD=0x%x,\n", (value & RTL8380_DMA_IF_CTRL_RX_EN_MASK) >> RTL8380_DMA_IF_CTRL_RX_EN_OFFSET);
    osal_printf("TXFN=0x%x, ", (value & RTL8380_DMA_IF_CTRL_TX_FETCH_MASK) >> RTL8380_DMA_IF_CTRL_TX_FETCH_OFFSET);
    osal_printf("TX_BUSY=0x%x]\n", (value & RTL8380_DMA_IF_CTRL_TX_BUSY_MASK) >> RTL8380_DMA_IF_CTRL_TX_BUSY_OFFSET);

    return RT_ERR_OK;
} /* end of r8380_bufStatus_dump */

/* Function Name:
 *      r8380_pkthdrMbuf_dump
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
r8380_pkthdrMbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags)
{
    uint32  i, j, ring_size;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    osal_printf("------- Formal Information -------------------------\n");
    if (NIC_PKTHDR_MBUF_MODE_RX == mode)
    {
        osal_printf("==== Dump Rx packet header and mbuf ====\n");
        ring_size = NIC_RXRING_SIZE;
    }
    else
    {
        osal_printf("==== Dump Tx packet header and mbuf ====\n");
        ring_size = nic_8380_txing_size_of_max_limit;
    }

    for (i = start; i <= end; i++)
    {
        for (j = 0; j < ring_size; j++)
        {
            nic_pkthdr_t    *pRing_pkthdr;
            if (NIC_PKTHDR_MBUF_MODE_RX == mode)
                pRing_pkthdr = (nic_pkthdr_t *)(DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (*((uint32 *)(pNic_rxFDPBase[i]+j))  | 0xa0000000)) & NIC_ADDR_MASK);
            else
                pRing_pkthdr = (nic_pkthdr_t *)(DESC_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, (*((uint32 *)(pNic_txFDPBase[i]+j))  | 0xa0000000)) & NIC_ADDR_MASK);
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
                uint8   *pPkt_data = (uint8 *) DMA_ADDR_KRN2USR(pRing_pkthdr->packet->data);
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

    return RT_ERR_OK;
} /* end of r8380_pkthdrMbuf_dump */

/* Function Name:
 *      r8380_rxStatus_get
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
r8380_rxStatus_get(uint32 unit, uint32 *pStatus)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    ioal_mem32_field_read(unit, RTL8380_DMA_IF_CTRL_ADDR, \
        RTL8380_DMA_IF_CTRL_RX_EN_OFFSET, RTL8380_DMA_IF_CTRL_RX_EN_MASK, pStatus);

    return RT_ERR_OK;
} /* end of r8380_rxStatus_get */



/* Function Name:
 *      r8380_nic_reset
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
r8380_nic_reset(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;
    
    /* Check whether it is inited, if not intialized, return fail */
    if (INIT_COMPLETED != nic_init[unit])
        return ret;

    NIC_SEM_LOCK(unit);

    /* Initialize the NIC module */
    if ((ret = _nic_reset(unit)) != RT_ERR_OK)
    {
        return ret;
    }

    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8380_nic_reset */



