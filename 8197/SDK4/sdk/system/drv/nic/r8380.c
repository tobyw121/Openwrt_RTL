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
 * $Revision: 52364 $
 * $Date: 2014-10-21 14:48:46 +0800 (Tue, 21 Oct 2014) $
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
#include <drv/nic/nic.h>
#include <drv/nic/r8380.h>
#include <drv/swcore/rtl8380.h>
#include <linux/netdevice.h>
#include <rtnic/rtnic_drv.h>
#include <soc/soc.h>


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
#define SRAM_SEG_BASE       (0xBF000000)
#define SRAM_SEG_SIZE_128KB  (0xa)
#define SRAM_SEG_ADDR       (0x10000000)
#define SRAM_DISABLE        (0)
#define SRAM_ENABLE         (1)

#undef NIC_RX_THREAD
#define NIC_RX_THREAD_STACK_SIZE        8192
#define NIC_RX_THREAD_PRI               0
#define NIC_RX_THREAD_QUEUE_LENGTH      2048
#define NIC_RX_THREAD_BURST_MAX         256
#define NIC_RX_THREAD_SLEEP_TIME        10000   /* 10 mS */


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
#ifdef __LITTLE_ENDIAN
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

//#error fix the CPU tag format to support big litten
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
static drv_nic_initCfg_t _nic_init_conf;
#ifdef NIC_RX_THREAD
static osal_thread_t    _nic_rx_thread;
static osal_sem_t       _nic_rx_sem;
static drv_nic_pkt_t    *pNic_Rx_Queue[NIC_RX_THREAD_QUEUE_LENGTH];
static uint32           _nic_rx_enQueueCnt = 0;
static uint32           _nic_rx_deQueueCnt = 0;
static int32            _nic_rx_thread_exit = 0;
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


static uint32       cpuTagId;
static uint32		rxCRCInclude;

static uint32 		reasonTbl[][2] = 
{
	{0,  											0, },
	{NIC_RX_REASON_RLDP_RLPP,						0, },
	{NIC_RX_REASON_RMA,								0, },
	{NIC_RX_REASON_IGR_VLAN_FILTER,					0, },
	{NIC_RX_REASON_INNER_OUTTER_CFI,				0, },
	{NIC_RX_REASON_MY_MAC,							0, },
	{NIC_RX_REASON_SPECIAL_TRAP,					0, },
	{NIC_RX_REASON_SPECIAL_COPY,					0, },
	{NIC_RX_REASON_ROUTING_EXCEPTION,				0, },
	{NIC_RX_REASON_UNKWN_UCST_MCST,					0, },
	{NIC_RX_REASON_MAC_CONSTRAINT_SYS,				NIC_RX_REASON_MAC_CONSTRAINT, },
	{NIC_RX_REASON_MAC_CONSTRAINT_VLAN,				NIC_RX_REASON_MAC_CONSTRAINT, },
	{NIC_RX_REASON_MAC_CONSTRAINT_PORT,				NIC_RX_REASON_MAC_CONSTRAINT, },
	{NIC_RX_REASON_CRC_ERROR,						0, },
	{NIC_RX_REASON_IP6_UNKWN_EXT_HDR,				0, },
	{NIC_RX_REASON_NORMAL_FWD,						0, },
};


/*
 * Macro Definition
 */
/* semaphore handling */
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
static int32 _insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, maple_nic_cpuTag_t *pCpuTag);
#endif
static int32 _removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, maple_nic_cpuTag_t *pCpuTag);



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

static int32 _nic_rx_reason_translate(drv_nic_pkt_t *pPacket)
{
#define RTL8380_MAX_REASON_NM  15
	uint16 reason = pPacket->rx_tag.reason;
	
	if (pPacket->rx_tag.acl_hit)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_ACL_HIT);
	if (pPacket->rx_tag.new_sa)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_NEW_SA);
	if (pPacket->rx_tag.l2_pmv)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_L2_PMV);
	if (pPacket->rx_tag.atk_hit)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_ATTACK);
	if (pPacket->rx_tag.mirror_hit)
		NIC_REASON_MASK_SET(*pPacket, NIC_RX_REASON_MIRROR);
    
	if (reason <= RTL8380_MAX_REASON_NM)
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
#if 1
    uint32 index;
    uint32 cnt_val;
#endif
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
    temp = temp | 0xa0000000;
    pNic_rxCDPIdx[ringId] = (uint32 *)temp;    /* The limitation */

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
        uint8           *pData;
        uint32          ethType;
        nic_pkthdr_t    *pktHdr;
        maple_nic_cpuTag_t    *pCputag;
        drv_nic_pkt_t   *pPacket;


        if ((*pNic_rxRDPIdx[ringId] & NIC_RING_SWOWNBIT) != 0)
            break;

        /* Prepare to be reclaim */
        pktHdr = (nic_pkthdr_t *)(((*pNic_rxRDPIdx[ringId]) | 0xa0000000) & NIC_ADDR_MASK);
        if (NULL == pktHdr || NULL == pktHdr->buf_addr || NULL == pktHdr->packet)
            break;
        
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
            pPacket->rx_tag.qid             = pCputag->un.rx.QID;
            pPacket->rx_tag.source_port     = pCputag->un.rx.SPN;
            pPacket->rx_tag.mirror_hit      = pCputag->un.rx.MIR_HIT;
            pPacket->rx_tag.acl_hit         = pCputag->un.rx.ACL_HIT;
            pPacket->rx_tag.acl_index       = pCputag->un.rx.ACL_IDX;
            pPacket->rx_tag.svid_tagged     = pCputag->un.rx.OTAGIF;
            pPacket->rx_tag.cvid_tagged     = pCputag->un.rx.ITAGIF;
            pPacket->rx_tag.fvid            = pCputag->un.rx.RVID;
            pPacket->rx_tag.mac_cst         = pCputag->un.rx.MAC_CST;
            pPacket->rx_tag.atk_hit         = pCputag->un.rx.ATK_HIT; 
            pPacket->rx_tag.atk_type        = pCputag->un.rx.ATK_TYPE; 
            pPacket->rx_tag.new_sa          = pCputag->un.rx.NEW_SA;
            pPacket->rx_tag.l2_pmv          = pCputag->un.rx.L2_PMV; /*89B will be both static or dynamic*/
            pPacket->rx_tag.reason          = pCputag->un.rx.REASON;
            _nic_rx_reason_translate(pPacket);
        }
        else /*these branch will never be reached because 89B always store CPU tag in pktHdr other than payload*/
        {
#if 1
            maple_nic_cpuTag_t cputag;
            
            scan_offset = 12;
            pData = pktHdr->packet->data + scan_offset;
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
                pPacket->rx_tag.acl_hit         = pCputag->un.rx.ACL_HIT;
                pPacket->rx_tag.acl_index       = pCputag->un.rx.ACL_IDX;
                pPacket->rx_tag.svid_tagged     = cputag.un.rx.OTAGIF;
                pPacket->rx_tag.cvid_tagged     = cputag.un.rx.ITAGIF;
                pPacket->rx_tag.fvid            = cputag.un.rx.RVID;
                pPacket->rx_tag.qid             = cputag.un.rx.QID;
                pPacket->rx_tag.atk_hit         = pCputag->un.rx.ATK_HIT; 
                pPacket->rx_tag.atk_type        = cputag.un.rx.ATK_TYPE;
                pPacket->rx_tag.mac_cst         = cputag.un.rx.MAC_CST;
                pPacket->rx_tag.new_sa          = cputag.un.rx.NEW_SA;
                pPacket->rx_tag.l2_pmv          = cputag.un.rx.L2_PMV;
                pPacket->rx_tag.reason          = cputag.un.rx.REASON;
		        _nic_rx_reason_translate(pPacket);
            }
#endif
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
             /*If no outer tag, assign rx port-based outer vid*/
            ioal_mem32_read(NIC_DEFAULT_UNIT_ID,\
                RTL8380_VLAN_PORT_PB_VLAN_ADDR((uint32)pPacket->rx_tag.source_port), &temp);
             pPacket->rx_tag.outer_vid = (temp>>16) & 0xFFF;
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
            ioal_mem32_read(NIC_DEFAULT_UNIT_ID,\
                RTL8380_VLAN_PORT_PB_VLAN_ADDR((uint32)pPacket->rx_tag.source_port), &temp);
             pPacket->rx_tag.inner_vid = (temp>>2) & 0xFFF;
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
            osal_printf(" ATK_HIT : %d \n", pPacket->rx_tag.atk_hit);
            osal_printf(" ATK_TYPE : %d \n", pPacket->rx_tag.atk_type);
            osal_printf(" NEW_SA : %d \n", pPacket->rx_tag.new_sa);
            osal_printf(" L2_PMV : %d \n", pPacket->rx_tag.l2_pmv);
            osal_printf(" REASON : %d \n", pPacket->rx_tag.reason);
        }

#ifdef NIC_RX_THREAD
        /* enqueue this packet into Rx Queue */
        if (pNic_Rx_Queue[_nic_rx_enQueueCnt] == NULL)
        {
            /* enqueue */
            pNic_Rx_Queue[_nic_rx_enQueueCnt] = pktHdr->packet;
            _nic_rx_enQueueCnt = (_nic_rx_enQueueCnt + 1) % NIC_RX_THREAD_QUEUE_LENGTH;
        }
        else
        {
            /* We have to free this packet here (because the rx queue is full) */
            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pktHdr->packet);
        }
#else
        nic_rx_handle = NIC_RX_NOT_HANDLED;
		if (0 == rxCRCInclude)
		{
			/* packet passed to higher layer doesn't need CRC field */
			pPacket->length -= 4;
			pPacket->tail -= 4;
		}
        
        for (i = 0; i < NIC_RX_CB_PRIORITY_NUMBER; i++)
        {
            if (_nic_rx_cb_tbl[i].rx_callback != NULL)
            {
                nic_rx_handle = _nic_rx_cb_tbl[i].rx_callback(NIC_DEFAULT_UNIT_ID, pktHdr->packet, _nic_rx_cb_tbl[i].pCookie);
                if (NIC_RX_HANDLED_OWNED == nic_rx_handle)
                {
                    break;
                }
            }
        }
        if (nic_rx_handle != NIC_RX_HANDLED_OWNED)
        {   /* We have to free this packet here */
            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pktHdr->packet);
        }
#endif

        pktHdr->packet = NULL;
        /* Alloc a new packet data buffer */
        if (RT_ERR_OK == _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
        {
            pktHdr->packet = pPacket;
            pktHdr->buf_addr = (uint8 *)((uint32)UNCACHE(pPacket->data) & 0x0fffffff);
            pktHdr->buf_size = (pPacket->end - pPacket->data);
            pktHdr->buf_len = 0;
            if ((ret = osal_cache_memory_flush((uint32)pPacket->head, (pPacket->end - pPacket->head))) != RT_ERR_OK)
            {
                return ret;
            }

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

#if 1
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
#endif

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
        pktHdr = (nic_pkthdr_t *)((*pNic_txRDPIdx[ringId] | 0xa0000000) & NIC_ADDR_MASK);
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
        if (pNic_txRDPIdx[ringId] == (pNic_txFDPBase[ringId] + nic_8380_txing_size_of_max_limit))
            pNic_txRDPIdx[ringId] = pNic_txFDPBase[ringId];
    } while (pNic_txRDPIdx[ringId] != pNic_txCDPIdx[ringId]); /*cw:where to get this pointer?*/

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
        temp = temp | 0xa0000000;
        pNic_rxCDPIdx[i] = (uint32 *)temp;
        do
        {
            nic_pkthdr_t *pPktHdr;

            if ((*pNic_rxRDPIdx[i] & NIC_RING_SWOWNBIT) != 0)
                break;

            /* Prepare to be reclaim */
            pPktHdr = (nic_pkthdr_t *)(((*pNic_rxRDPIdx[i]) | 0xa0000000) & NIC_ADDR_MASK);
            if (NULL == pPktHdr || pPktHdr->packet != NULL)
                break;

            /* Alloc a new packet data buffer */
            if (RT_ERR_OK != _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
            {
                RT_LOG(LOG_DEBUG, MOD_NIC, "Out of memory ! (alloc a new packet data buffer failed)");
                break;
            }

            pPktHdr->packet = pPacket;
            pPktHdr->buf_addr = (uint8 *)((uint32)UNCACHE(pPacket->data) & 0x0fffffff);
            pPktHdr->buf_size = (pPacket->end - pPacket->data);
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
 *      _maple_nic_isr_handler
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
_maple_nic_isr_handler(void *isr_param)
{
    uint32  cpu_iisr;

    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, &cpu_iisr);

    /* Rx have 8 rings (0..7); need to process when any rx ring isr done */
    if (cpu_iisr & INT_RX_DONE7_INT_RX_DONE0_MASK)
    {
        if (cpu_iisr & INT_RX_DONE7_MASK)   /* Process Rx ring 7 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE7_MASK);
            _nic_isr_rxRoutine(7);
        }

        if (cpu_iisr & INT_RX_DONE6_MASK)   /* Process Rx ring 6 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE6_MASK);
            _nic_isr_rxRoutine(6);
        }

        if (cpu_iisr & INT_RX_DONE5_MASK)   /* Process Rx ring 5 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE5_MASK);
            _nic_isr_rxRoutine(5);
        }

        if (cpu_iisr & INT_RX_DONE4_MASK)   /* Process Rx ring 4 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE4_MASK);
            _nic_isr_rxRoutine(4);
        }

        if (cpu_iisr & INT_RX_DONE3_MASK)   /* Process Rx ring 3 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE3_MASK);
            _nic_isr_rxRoutine(3);
        }

        if (cpu_iisr & INT_RX_DONE2_MASK)   /* Process Rx ring 2 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE2_MASK);
            _nic_isr_rxRoutine(2);
        }

        if (cpu_iisr & INT_RX_DONE1_MASK)   /* Process Rx ring 1 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE1_MASK);
            _nic_isr_rxRoutine(1);
        }

        if (cpu_iisr & INT_RX_DONE0_MASK)   /* Process Rx ring 0 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_RX_DONE0_MASK);
            _nic_isr_rxRoutine(0);
        }
    }

    /* Tx have 2 rings (0..1); need to process when any tx ring isr done */
    if (cpu_iisr & INT_TX_DONE1_INT_TX_DONE0_MASK)
    {
        if (cpu_iisr & INT_TX_DONE1_MASK)   /* Process Tx ring 1 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_TX_DONE1_MASK);
            _nic_isr_txRoutine(1);
        }

        if (cpu_iisr & INT_TX_DONE0_MASK)   /* Process Tx ring 0 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_TX_DONE0_MASK);
            _nic_isr_txRoutine(0);
        }
    }

    if (cpu_iisr & INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK);
    }

    /* mBuffer Runout */
    if (cpu_iisr & RTL8380_DMA_IF_INTR_STS_RX_RUN_OUT_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_INTR_STS_ADDR, RTL8380_DMA_IF_INTR_STS_RX_RUN_OUT_MASK);
        _nic_isr_mbRoutine();
    }

    return OSAL_INT_HANDLED;
} /* end of _maple_nic_isr_handler */

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
    uint32  temp;
    uint32  index;
    int32   i, j, k;
    int32   ret = RT_ERR_FAILED;

    uint32 cnt_val;

    /* Check arguments */
    RT_PARAM_CHK(NULL == pInitCfg->pkt_alloc, RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(NULL == pInitCfg->pkt_free, RT_ERR_NULL_POINTER);

    /* Reset the NIC Tx/Rx debug information */
    nic_debug_flag = 0;

//nic_debug_flag = DEBUG_RX_CPU_TAG_BIT |DEBUG_RX_RAW_LEN_BIT |DEBUG_TX_CPU_TAG_BIT |DEBUG_TX_RAW_LEN_BIT;
    nic_tx_success_cntr = 0;
    nic_tx_failed_cntr = 0;
    nic_rx_success_cntr = 0;
    nic_rx_failed_cntr = 0;

    flag_es = 0;
    if ((nic_chipId[unit] & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        nic_8380_txing_size_of_max_limit = 1;
    else
        nic_8380_txing_size_of_max_limit = NIC_8380_TXRING_SIZE;
    //osal_printf("_nic_init API set the nic_8380_txing_size_of_max_limit = %d\n", nic_8380_txing_size_of_max_limit);

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
    
        /* Reset NIC only */
        ioal_mem32_field_write(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_MASK, 1);
    
        osal_time_usleep(50 * 1000); /* delay 50mS */
    
        do
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
            ioal_mem32_field_read(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_MASK, &temp);
        } while (temp != 0);
    
        RT_LOG(LOG_DEBUG, MOD_NIC, "OK");
    }

    /* CPU port: Enable MAC Tx/Rx */
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), RTL8380_MAC_PORT_CTRL_TXRX_EN_OFFSET, RTL8380_MAC_PORT_CTRL_TXRX_EN_MASK, 3);

#if 0
    /* Enable internal register display */
    ioal_mem32_field_write(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, \
        RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_OFFSET, RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_MASK, 0xA);

    /* Read chip type and int reversion and do patch for RTL8329M A-CUT */
    ioal_mem32_field_read(unit, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_ADDR, \
        RTL8389_GLOBAL_MAC_INTERNAL_STATUS_CHIP_TYP_OFFSET, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_CHIP_TYP_MASK, &chip_type);

    ioal_mem32_field_read(unit, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_ADDR, \
        RTL8389_GLOBAL_MAC_INTERNAL_STATUS_INT_VER_OFFSET, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_INT_VER_MASK, &int_ver);
    
    ioal_mem32_field_write(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, \
        RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_OFFSET, RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_MASK, 0x0);
#endif


#if 0
    /* Set 25~28 Combo-Port + CPU port to join the Lookup Miss Flooding Portmask */
    ioal_mem32_write(unit, RTL8380_TBL_ACCESS_L2_CTRL_ADDR, 0x28000);
    ioal_mem32_read(unit, RTL8380_TBL_ACCESS_L2_DATA_ADDR(0), &temp);
    temp |= 0x80000000;
    ioal_mem32_write(unit, RTL8380_TBL_ACCESS_L2_DATA_ADDR(0), temp);
    ioal_mem32_write(unit, RTL8380_TBL_ACCESS_L2_CTRL_ADDR, 0x38000);
#endif


    /*Speed, duplex, flow control*/
    /*REG32(MAC_FORCE_MODE_CTRLr(CPUPORT)) = 0x6192F;*/
    ioal_mem32_write(unit, RTL8380_MAC_FORCE_MODE_CTRL_ADDR(28), 0x6192F);

    /*Set port28 CRC error forward, disable CRC check*/
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), \
        RTL8380_MAC_PORT_CTRL_RX_CHK_CRC_EN_OFFSET, RTL8380_MAC_PORT_CTRL_RX_CHK_CRC_EN_MASK, 1);

#if 0
    /*Only for test chip FPGA*/
    /*set port24,port26 linkup*/
    ioal_mem32_field_write(unit, RTL8380_MAC_FORCE_MODE_CTRL_ADDR(24), \
        RTL8380_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_OFFSET, RTL8380_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_MASK, 1);
    ioal_mem32_field_write(unit, RTL8380_MAC_FORCE_MODE_CTRL_ADDR(24), \
        RTL8380_MAC_FORCE_MODE_CTRL_MAC_FORCE_EN_OFFSET, RTL8380_MAC_FORCE_MODE_CTRL_MAC_FORCE_EN_MASK, 1);

    ioal_mem32_field_write(unit, RTL8380_MAC_FORCE_MODE_CTRL_ADDR(26), \
        RTL8380_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_OFFSET, RTL8380_MAC_FORCE_MODE_CTRL_FORCE_LINK_EN_MASK, 1);
    ioal_mem32_field_write(unit, RTL8380_MAC_FORCE_MODE_CTRL_ADDR(26), \
        RTL8380_MAC_FORCE_MODE_CTRL_MAC_FORCE_EN_OFFSET, RTL8380_MAC_FORCE_MODE_CTRL_MAC_FORCE_EN_MASK, 1);
#endif

    /*HOL register*/
    /*REG32(DMA_IF_RX_RING_SIZEr) = 0xFFFFFFFF;*/
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
    
#if 0
    /* SRAM MAP Setting - Pointer to NIC info memory */
    /* physical address: 0x1000_0000 --> virtual memory 0xB000_0000 */
    /* USE SRAM: 0xB000_0000 ~ 0xB000_7FFF (32KB)    */
    REG32(SRAMSBR0) = SRAM_SEG_BASE;        /* SRAM segment base = 0x0000_0000 */
    REG32(SRAMSSR0) = SRAM_SEG_SIZE_128KB;   /* SRAM segment size = 128K bytes   */
    REG32(SRAMSAR0) = (SRAM_SEG_ADDR | SRAM_ENABLE);    /* SRAM segment addr = 0x1000_0000 (0xB000_0000) */
    pNicInfo = (nic_info_t *)0xB0000000;    /* MAP to virtual memory (UNCACHE) */
    REG32(UMSSR0) = SRAM_SEG_SIZE_32KB;
    REG32(UMSAR0) = (SRAM_SEG_ADDR | SRAM_ENABLE);
#endif

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
                ((uint32)pPktHdr & 0x0fffffff) | NIC_RING_WRAPBIT : \
                ((uint32)pPktHdr & 0x0fffffff);

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
                ((uint32)pPktHdr & 0x0fffffff) | NIC_RING_WRAPBIT : \
                ((uint32)pPktHdr & 0x0fffffff);

            i++;
        }
    }


#if 0
    /*Show pktHdrs information*/
    printk("Before allocating space\n");
    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->rxPktHdr[i];
            printk("RX Ring[%d]--Entry[%d] pktHdr:  buf_addr[%p]--buf_size[%8x]\n", j, k, pPktHdr->buf_addr, pPktHdr->buf_size);

            i++;
        }
    }
#endif

    /* Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_register(RTK_DEV_NIC, _maple_nic_isr_handler, NULL))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }

    /* Setting Registers */
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_RX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_rxFDPBase[i] & 0x0fffffff);  
    }

    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_TX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_txFDPBase[i] & 0x0fffffff);  
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


#if 0
    /*Show pktHdrs information*/
    printk("After allocating space\n");

    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->rxPktHdr[i];
            printk("RX Ring[%d]--Entry[%d] pktHdr:  buf_addr[%p]--buf_size[%8x]\n", j, k, pPktHdr->buf_addr, pPktHdr->buf_size);

            i++;
        }
    }
#endif

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

#if 0
    uint32 cnt_val;
#endif

    if (NULL == pPacket)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - pPacket is NULL!");
        return RT_ERR_FAILED;
    }

    flag_es = 0;
    if ((nic_chipId[unit] & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
    {
        struct sk_buff *pOrigSkb = NULL;
        struct sk_buff *pNewSkb = NULL;

        if(((uint32)(pPacket->data) & 0x3) != 0x0) /*not 4-byte align*/
        {
            pOrigSkb = (struct sk_buff *)(pPacket->buf_id);
    
            pNewSkb = dev_alloc_skb(pPacket->length + RTNIC_PKTLEN_RSVD + 4);
            if (NULL == pNewSkb)
            {
                osal_printf("Error: Out of memory at %s():%d\n", __FUNCTION__, __LINE__);
                goto no_patch;
            }
    
            memcpy(pNewSkb->data, pPacket->data, pPacket->length);
    
            /* raw packet */
            pPacket->buf_id = (void *)pNewSkb;
            pPacket->head = pNewSkb->head;
            pPacket->data = pNewSkb->data;
            pPacket->tail = pNewSkb->data + pPacket->length + 4;
            pPacket->end = pNewSkb->end;
            pPacket->length = pPacket->length + 4;
            pPacket->next = NULL;
    
            dev_kfree_skb(pOrigSkb);
        }
    }
no_patch:

    if (flag_es)
        txRingId = 0;   /*FPGA platform always TX Ring0*/
    else
    {
        /* Step: Decide the target queue */
        txRingId = (pPacket->tx_tag.priority > 3) ? 1 : 0;    /* mapping 8 priority to 2 queues */
    }

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

    pPktHdr = (nic_pkthdr_t *)((*pNic_txCDPIdx[txRingId] | 0xa0000000) & NIC_ADDR_MASK);
    /* Step: Double Confirm (Check the pktHdr status) */
    if (NULL == pPktHdr || NULL != pPktHdr->packet)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr is NOT available!");
        return RT_ERR_FAILED;
    }

    
    RT_LOG(LOG_DEBUG, MOD_NIC, "%s():%d  pPktHdr->packet:%p   as_txtag:%d\n", __FUNCTION__, __LINE__, pPktHdr->packet, pPacket->as_txtag);

    if (flag_es)
    {
        /*Set CPU port can communicate with all ports, because port-isolation will be modified if CPU Tag ASDPM*/
        uint32 value;
        value = 0x1FFFFFFFUL;
        ioal_mem32_write(unit, RTL8380_PORT_ISO_CTRL_ADDR(28), value);
    }

    /* Set  the CPU Tx tag into the packet header */
    if (pPacket->as_txtag)
    {
#if 0
        /* Check the reserved headroom size have at least 12 bytes */
        if ((pPacket->data - pPacket->head) < (CPU_TAG_ID_LEN + sizeof(maple_nic_cpuTag_t)))
        {
            /* The reserved space is NOT enough */
            RT_LOG(LOG_DEBUG, MOD_NIC, "pPacket->as_txtag = %d", pPacket->as_txtag);
            RT_LOG(LOG_DEBUG, MOD_NIC, "(pPacket->data - pPacket->head) = %d", (pPacket->data - pPacket->head));

            return RT_ERR_FAILED;
        }
#endif

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

        if (flag_es)
        {
            /*Solution for 80 Test chip CPU Tag AssignDPM bug*/
            /*Use ACL redirect to multiple portss to cover CPU Tag ASDPM bug, 
                this method supposes that if Not AssignDPM then user donnot need  ACL Action to be active*/
            /*1: ACL reserves the first entry to set pkt from CPU port with CPU Tag redirect to multiple ports on ACL INIT*/
            /*2: PortMask table reserves one entry for ACL redirect action above ON L2 INIT*/
    
            /*Test chip bug, must always not set Assign DPM*/
            pPktHdr->cpuTag.un.tx.AS_DPM  = 0;
    
            if((pPacket->tx_tag.as_dst_port_mask) && (pPacket->tx_tag.dst_port_mask))
            {
                uint32 pmsk_idx,value;
                pmsk_idx = 0x1;
                
                /*rewrite field: ACL ACT*/
                pPktHdr->cpuTag.un.tx.ACL_ACT         = 0x1;
                
                /*set acl portmask table enty: pmsk_idx*/
                ioal_mem32_write(unit, RTL8380_TBL_ACCESS_L2_DATA_ADDR(0), pPacket->tx_tag.dst_port_mask);
                value = 0x14000;
                value |= pmsk_idx;
                ioal_mem32_write(unit, RTL8380_TBL_ACCESS_L2_CTRL_ADDR, value);
                
                /*check write operation complete or not*/
                ioal_mem32_read(unit, RTL8380_TBL_ACCESS_L2_CTRL_ADDR, &value);
                while(value & 0x10000)
                {
                    ioal_mem32_read(unit, RTL8380_TBL_ACCESS_L2_CTRL_ADDR, &value);
                }
    
            }
            else
            {
                /*rewrite field: ACL ACT*/
                pPktHdr->cpuTag.un.tx.ACL_ACT         = 0x0;
            }
        }
        else
        {
            /*Final chip*/
            pPktHdr->cpuTag.un.tx.DPM_TYPE         = pPacket->tx_tag.dpm_type;
            pPktHdr->cpuTag.un.tx.AS_DPM            = pPacket->tx_tag.as_dst_port_mask;
            pPktHdr->cpuTag.un.tx.DPM                  = pPacket->tx_tag.dst_port_mask;
        }
    }
    else
    {
        pPktHdr->cpuTag.un.tx.CPUTAGIF = FALSE;
    }

    /* Step: Calc the pktbuf number */
    packetNum = 1;    /* Support single pktBuf now - one descriptor vs. one mbuf */

    pPktHdr->tx_callback = fTxCb;    /* Tx Callback function */
    pPktHdr->cookie = pCookie;
    pPktHdr->packet = pPacket;
    pPktHdr->buf_addr = (uint8 *)((UNCACHE(pPacket->data) & 0x0fffffff));

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
            pPktHdr->buf_len = pPacket->length;
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
    if (pNic_txCDPIdx[txRingId] == (pNic_txFDPBase[txRingId] + nic_8380_txing_size_of_max_limit))
        pNic_txCDPIdx[txRingId] = pNic_txFDPBase[txRingId];

    /* NIC Tx debug message */
    if (nic_debug_flag & DEBUG_TX_RAW_LEN_BIT)
    {
        int i;
        int dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */

        osal_printf("=== [NIC TX Debug] ================================= Len: %d \n", pPktHdr->buf_len);

        osal_printf("pPktHdr->buf_addr is %p\n",pPktHdr->buf_addr);

        for (i = 0; i < dump_len; i++)
        {
            if (i == (pPktHdr->buf_len))
                break;
            if (0 == (i % 16))
                osal_printf("[%04X] ", i);
            osal_printf("%02X ", *(uint8*)((uint32)(pPktHdr->buf_addr + i) | 0xa0000000));
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

#if 0
    {
        /*Nic reg*/
        uint32 cnt_val;
        ioal_mem32_read(unit, RTL8380_DMA_IF_CTRL_ADDR, &cnt_val);
        osal_printf(" DMA_IF_CTRL is %x \n", cnt_val);
        ioal_mem32_read(unit, RTL8380_DMA_IF_CTRL_ADDR, &cnt_val);
        osal_printf(" DMA_IF_CTRL is %x \n", cnt_val);
        ioal_mem32_read(unit, RTL8380_DMA_IF_CTRL_ADDR, &cnt_val);
        osal_printf(" DMA_IF_CTRL is %x \n", cnt_val);
        /*ALE reg*/
        ioal_mem32_read(unit, 0x4100, &cnt_val);
        osal_printf(" ALE_PORT_ISO is %x \n", cnt_val);
        ioal_mem32_read(unit, 0x4100, &cnt_val);
        osal_printf(" ALE_PORT_ISO is %x \n", cnt_val);
        ioal_mem32_read(unit, 0x4100, &cnt_val);
        osal_printf(" ALE_PORT_ISO is %x \n", cnt_val);
        /*MAC reg*/
        ioal_mem32_read(unit, 0xa520, &cnt_val);
        osal_printf(" Port MAC TPID CTRL is %x \n", cnt_val);
        ioal_mem32_read(unit, 0xa520, &cnt_val);
        osal_printf(" Port MAC TPID CTRL is %x \n", cnt_val);
        ioal_mem32_read(unit, 0xa520, &cnt_val);
        osal_printf(" Port MAC TPID CTRL is %x \n", cnt_val);
    }
#else
    {   /*Chig bug, sometimes lextra bus will access wrong data*/
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
#endif

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

#ifdef NIC_RX_THREAD
/* Function Name:
 *      _nic_rx_thread
 * Description:
 *      (Thread) to dequeue the rx packet to each callback function
 * Input:
 *      pInput - point to the handler struct of the creator
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void _nic_rx_thread_func(void *pInput)
{
    drv_nic_rx_t nic_rx_handle = NIC_RX_NOT_HANDLED;
    uint32 burst = 0;
    int32 i;

    /* forever loop */
    for (;;)
    {
        /* deQueue */
        if (pNic_Rx_Queue[_nic_rx_deQueueCnt] != NULL)
        {
            /* Callback */
            nic_rx_handle = NIC_RX_NOT_HANDLED;
            for (i = 0; i < NIC_RX_CB_PRIORITY_NUMBER; i++)
            {
                if (_nic_rx_cb_tbl[i].rx_callback != NULL)
                {
                    nic_rx_handle = _nic_rx_cb_tbl[i].rx_callback(NIC_DEFAULT_UNIT_ID, pNic_Rx_Queue[_nic_rx_deQueueCnt], _nic_rx_cb_tbl[i].pCookie);
                    if (NIC_RX_HANDLED_OWNED == nic_rx_handle)
                        break;
                }
            }
            if (nic_rx_handle != NIC_RX_HANDLED_OWNED)
            {   /* We have to free this packet here */
                _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pNic_Rx_Queue[_nic_rx_deQueueCnt]);
            }

            pNic_Rx_Queue[_nic_rx_deQueueCnt] = NULL;
            _nic_rx_deQueueCnt = ((_nic_rx_deQueueCnt + 1) % NIC_RX_THREAD_QUEUE_LENGTH);

            burst += 1;
            if (burst > NIC_RX_THREAD_BURST_MAX)   /* once a round */
            {
                /* force to sleep for a tick */
                /* Sleep 10ms for release cpu resource. */
                osal_time_usleep(NIC_RX_THREAD_SLEEP_TIME);
                burst = 0;
            }
        }
        else
        {
            /* Sleep 10ms for release cpu resource. */
            osal_time_usleep(NIC_RX_THREAD_SLEEP_TIME);
            burst = 0;
        }

        if (_nic_rx_thread_exit)
        {
            osal_printf("Break the NIC Rx thread\n");
            break;
        }
    }

    _nic_rx_thread_exit = 0;

    osal_thread_exit(0);
} /* end of _nic_rx_thread_func */

#endif

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
#ifdef NIC_RX_THREAD
    int32 i;
#endif

    /* Check arguments */
    RT_PARAM_CHK(NULL == pInitCfg, RT_ERR_NULL_POINTER);

    /* Check whether it is inited, if inited, return fail */
    if (INIT_COMPLETED == nic_init[unit])
        return ret;

    /* create semaphore */
    nic_lock = 0;
#if 0
    nic_sem[unit] = osal_sem_mutex_create();
    if (0 == nic_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_NIC), "semaphore create failed");
        return RT_ERR_FAILED;
    }
#endif

#ifdef NIC_RX_THREAD
    /* Init Rx Queue */
    for (i=0; i<NIC_RX_THREAD_QUEUE_LENGTH; i++)
    {
        pNic_Rx_Queue[i] = NULL;
    }
    _nic_rx_enQueueCnt = 0;
    _nic_rx_deQueueCnt = 0;
    _nic_rx_thread_exit = 0;

    /* Create a RX Thread to handle the Rx packets */
    if ((osal_thread_t)NULL == (_nic_rx_thread = osal_thread_create("RTK NIC Rx Thread", NIC_RX_THREAD_STACK_SIZE, NIC_RX_THREAD_PRI, (void *)_nic_rx_thread_func, NULL)))
    {
        osal_printf("RTK NIC Rx Thread create failed\n");

        return RT_ERR_FAILED;
    }
#endif

    /* Initialize the NIC module */
    if ((ret = _nic_init(unit, pInitCfg)) != RT_ERR_OK)
    {
#ifdef NIC_RX_THREAD
        _nic_rx_thread_exit = 1;
#endif
        return ret;
    }

    /* set init flag to complete init */
    nic_init[unit] = INIT_COMPLETED;
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
 *      When fTxCb is NULL, driver will free packet and not callback any more.
 */
int32
r8380_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
{
    int32 ret = RT_ERR_FAILED;
    uint32  temp;

    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8380_DMA_IF_TX_CUR_DESC_ADDR_CTRL_ADDR(0), &temp);

    //osal_printf("=== [r8380_pkt_tx] ================================= s\n");

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

    //osal_printf("r8380_pkt_tx ret is %x\n",ret);

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

    /* Check arguments */

    /* Dispatch */
    NIC_SEM_LOCK(unit);
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

    /* Check arguments */

    /* Dispatch */
    NIC_SEM_LOCK(unit);
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

    /* Dispatch */
    NIC_SEM_LOCK(unit);
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

    /* Dispatch */
    NIC_SEM_LOCK(unit);
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
    //RT_PARAM_CHK(size > _nic_init_conf.pkt_size, RT_ERR_OUT_OF_RANGE);
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

    NIC_SEM_LOCK(unit);
    nic_debug_flag = flags;
    NIC_SEM_UNLOCK(unit);

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

    NIC_SEM_LOCK(unit);
    *pFlags = nic_debug_flag;
    NIC_SEM_UNLOCK(unit);

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

    NIC_SEM_LOCK(unit);
    osal_printf("Tx success counter : %0d \n", nic_tx_success_cntr);
    osal_printf("Tx failed counter  : %0d \n", nic_tx_failed_cntr);
    osal_printf("Rx success counter : %0d \n", nic_rx_success_cntr);
    osal_printf("Rx failed counter  : %0d \n", nic_rx_failed_cntr);
    NIC_SEM_UNLOCK(unit);
    
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

    NIC_SEM_LOCK(unit);
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

    osal_printf("TXRING  SW_txFDPBase  SW_TxCDPIdx  HW_TxCDPIdx  SW_TxRDPIdx \n");
    osal_printf("=========================================================== \n");
    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_read(unit, RTL8380_DMA_IF_TX_CUR_DESC_ADDR_CTRL_ADDR(i), &value);

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
    NIC_SEM_UNLOCK(unit);

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
        ring_size = nic_8380_txing_size_of_max_limit;
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

    NIC_SEM_LOCK(unit);
    ioal_mem32_field_read(unit, RTL8380_DMA_IF_CTRL_ADDR, \
        RTL8380_DMA_IF_CTRL_RX_EN_OFFSET, RTL8380_DMA_IF_CTRL_RX_EN_MASK, pStatus);
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8380_rxStatus_get */


/* Function Name:
 *      _nic_reset
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
_nic_reset(uint32 unit)
{
#if 1
    uint32  flag_es;
    uint32  temp;
    uint32  index;
    int32   i, j, k;
    int32   ret = RT_ERR_FAILED;

    uint32 cnt_val;
    uint32 tx_disable_pmsk;    

    uint8*    rxPktHdr_bufferAddr[NIC_RX_PKTHDR_NUM];
    uint32    rxPktHdr_bufferSize[NIC_RX_PKTHDR_NUM];
    struct drv_nic_pkt_s *    rxPktHdr_pPacket[NIC_RX_PKTHDR_NUM];
    void    *    rxPktHdr_pCookie[NIC_RX_PKTHDR_NUM];

    flag_es = 0;
    if ((nic_chipId[unit] & 0xFFFF)  == 0x6966)
        flag_es = 1;

    /*Disable Port28 from TX-Portmask*/
    ioal_mem32_read(unit, RTL8380_MAC_TX_DISABLE_ADDR, &tx_disable_pmsk);
    tx_disable_pmsk |= (1UL<<28);
    ioal_mem32_write(unit, RTL8380_MAC_TX_DISABLE_ADDR,  tx_disable_pmsk);


    /* Disable NIC rx/tx*/
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), RTL8380_MAC_PORT_CTRL_TXRX_EN_OFFSET, RTL8380_MAC_PORT_CTRL_TXRX_EN_MASK, 0);

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

    /* Un-Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_unregister(RTK_DEV_NIC))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Un-Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }
    
    osal_time_usleep(50 * 1000); /* delay 50mS */

    if (flag_es)
    {
        /*Donot reset NIC, because of NIC RESET bug*/
    }
    else
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Reset NIC & Queue (R8380)... ");
    
        /* Reset NIC & Reset Queue at the same time*/
        ioal_mem32_write(unit, RTL8380_RST_GLB_CTRL_0_ADDR, 0xc);
    
        osal_time_usleep(50 * 1000); /* delay 50mS */

        /*Check NIC Reset OK?*/
        do
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
            ioal_mem32_field_read(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_MASK, &temp);
        } while (temp != 0);

        /*Check Queue Reset OK?*/
        do
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
            ioal_mem32_field_read(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_Q_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_Q_RST_MASK, &temp);
        } while (temp != 0);
    
        RT_LOG(LOG_DEBUG, MOD_NIC, "OK");
    }
    

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

#if 0
    printk("&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n");
    printk("Before clear space\n");
#endif

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

    /*Back up cluster space address & size*/
    /*RX*/
    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            rxPktHdr_bufferAddr[i] = pNicInfo->rxPktHdr[i].buf_addr;
            rxPktHdr_bufferSize[i] = pNicInfo->rxPktHdr[i].buf_size;
            rxPktHdr_pPacket[i] = pNicInfo->rxPktHdr[i].packet;
            rxPktHdr_pCookie[i] = pNicInfo->rxPktHdr[i].cookie;
            i++;
        }
    }

#if 0
    /*Show pktHdrs information*/
    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->rxPktHdr[i];
            printk("RX Ring[%d]--Entry[%d] pktHdr:  buf_addr[%p]--buf_size[%8x]\n", j, k, pPktHdr->buf_addr, pPktHdr->buf_size);
            i++;
        }
    }
#endif

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
                ((uint32)pPktHdr & 0x0fffffff) | NIC_RING_WRAPBIT : \
                ((uint32)pPktHdr & 0x0fffffff);

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
                ((uint32)pPktHdr & 0x0fffffff) | NIC_RING_WRAPBIT : \
                ((uint32)pPktHdr & 0x0fffffff);

            i++;
        }
    }

    /*Restore RX pktHdr Cluster space address & size*/
    /*RX*/
    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            pNicInfo->rxPktHdr[i].buf_addr = rxPktHdr_bufferAddr[i];
            pNicInfo->rxPktHdr[i].buf_size = rxPktHdr_bufferSize[i];
            pNicInfo->rxPktHdr[i].packet = rxPktHdr_pPacket[i];
            pNicInfo->rxPktHdr[i].cookie = rxPktHdr_pCookie[i];
            *(pNicInfo->rxPktHdr[i].ring_entry) |= NIC_RING_SWOWNBIT;    /* Only set the SwOwn bit */
            i++;
        }
    }

#if 0
    /*Show pktHdrs information*/
    printk("After restore space\n");
    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->rxPktHdr[i];
            printk("RX Ring[%d]--Entry[%d] pktHdr:  buf_addr[%p]--buf_size[%8x]\n", j, k, pPktHdr->buf_addr, pPktHdr->buf_size);

            i++;
        }
    }
#endif


    /* Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_register(RTK_DEV_NIC, _maple_nic_isr_handler, NULL))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }
    

    /* Setting Registers */
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_RX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_rxFDPBase[i] & 0x0fffffff);  
    }

    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_TX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_txFDPBase[i] & 0x0fffffff);  
    }
    

    /* CPU port: Enable MAC Tx/Rx */
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), RTL8380_MAC_PORT_CTRL_TXRX_EN_OFFSET, RTL8380_MAC_PORT_CTRL_TXRX_EN_MASK, 3);

    /*Every thing is ok now, NIC can RX/TX, and enable interrupt trigger*/
    ioal_mem32_write(unit, RTL8380_DMA_IF_INTR_MSK_ADDR, CPUIIMR_ENABLE_MASK);  
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_RX_EN_OFFSET, RTL8380_DMA_IF_CTRL_RX_EN_MASK, 1);
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_TX_EN_OFFSET, RTL8380_DMA_IF_CTRL_TX_EN_MASK, 1);

    /*Enable Port28 from TX-Portmask*/
    ioal_mem32_read(unit, RTL8380_MAC_TX_DISABLE_ADDR, &tx_disable_pmsk);
    tx_disable_pmsk &= ~(1UL<<28);
    ioal_mem32_write(unit, RTL8380_MAC_TX_DISABLE_ADDR,  tx_disable_pmsk);
    
    return RT_ERR_OK;
#else
    uint32  flag_es;
    uint32  temp;
    uint32  index;
    int32   i, j, k;
    int32   ret = RT_ERR_FAILED;

    uint32 cnt_val;

    /* Reset the NIC Tx/Rx debug information */
    nic_debug_flag = 0;

    nic_tx_success_cntr = 0;
    nic_tx_failed_cntr = 0;
    nic_rx_success_cntr = 0;
    nic_rx_failed_cntr = 0;

    flag_es = 0;
    if ((nic_chipId[unit] & 0xFFFF)  == 0x6966)
        flag_es = 1;

    if (flag_es)
        nic_8380_txing_size_of_max_limit = 1;
    else
        nic_8380_txing_size_of_max_limit = NIC_8380_TXRING_SIZE;

    /* Disable NIC rx/tx*/
    ioal_mem32_field_write(unit, RTL8380_MAC_PORT_CTRL_ADDR(28), RTL8380_MAC_PORT_CTRL_TXRX_EN_OFFSET, RTL8380_MAC_PORT_CTRL_TXRX_EN_MASK, 0);

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

    /* Un-Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_unregister(RTK_DEV_NIC))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Un-Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }
    
    printk("%s,%d\n",__FUNCTION__,__LINE__);

    if (flag_es)
    {
        /*Donot reset NIC, because of NIC RESET bug*/
    }
    else
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Reset NIC (R8380)... ");
    
        /* Reset NIC only */
        ioal_mem32_field_write(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_MASK, 1);
    
        osal_time_usleep(50 * 1000); /* delay 50mS */
    
        do
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
            ioal_mem32_field_read(unit, RTL8380_RST_GLB_CTRL_0_ADDR, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_OFFSET, RTL8380_RST_GLB_CTRL_0_SW_NIC_RST_MASK, &temp);
        } while (temp != 0);
    
        RT_LOG(LOG_DEBUG, MOD_NIC, "OK");
    }

    printk("%s,%d\n",__FUNCTION__,__LINE__);

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

    printk("%s,%d\n",__FUNCTION__,__LINE__);
    

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

    printk("%s,%d\n",__FUNCTION__,__LINE__);


    /*Free RX cluster space*/
    i = 0;
    printk("%s,%d\n",__FUNCTION__,__LINE__);
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            if(NULL != pNicInfo->rxPktHdr[i].packet)
            {
                _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pNicInfo->rxPktHdr[i].packet);;
            }
            i++;
        }
    }
    printk("%s,%d\n",__FUNCTION__,__LINE__);

    /*Free Ring & pktHdrs space*/
    if(NULL != pNicInfo)
    {
        osal_free(pNicInfo);
    }

    printk("%s,%d\n",__FUNCTION__,__LINE__);  
    
    pNicInfo = (nic_info_t *) osal_alloc(sizeof(nic_info_t));

    printk("%s,%d\n",__FUNCTION__,__LINE__);
    
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
                ((uint32)pPktHdr & 0x0fffffff) | NIC_RING_WRAPBIT : \
                ((uint32)pPktHdr & 0x0fffffff);

            i++;
        }
    }

    printk("%s,%d\n",__FUNCTION__,__LINE__);

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
                ((uint32)pPktHdr & 0x0fffffff) | NIC_RING_WRAPBIT : \
                ((uint32)pPktHdr & 0x0fffffff);

            i++;
        }
    }

    printk("%s,%d\n",__FUNCTION__,__LINE__);

    /* Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_register(RTK_DEV_NIC, _maple_nic_isr_handler, NULL))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }

    /* Setting Registers */
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_RX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_rxFDPBase[i] & 0x0fffffff);  
    }

    for (i = 0; i < NIC_TXRING_NUM; i++)
    {
        ioal_mem32_write(unit, RTL8380_DMA_IF_TX_BASE_DESC_ADDR_CTRL_ADDR(i), (uint32)pNic_txFDPBase[i] & 0x0fffffff);  
    }

    printk("%s,%d\n",__FUNCTION__,__LINE__);
    

    ioal_mem32_field_read(unit, RTL8380_MAC_CPU_TAG_ID_CTRL_ADDR, RTL8380_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_OFFSET, \
        RTL8380_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_MASK, &cpuTagId);    
#if defined(__MODEL_USER__) || defined(__MODEL_KERNEL__)
    if (0 == cpuTagId)
    {
        ioal_mem32_write(unit, RTL8380_MAC_CPU_TAG_ID_CTRL_ADDR, REALTEK_CPUTAG_ID << RTL8380_MAC_CPU_TAG_ID_CTRL_CPU_TAG_ID_OFFSET);
        cpuTagId = REALTEK_CPUTAG_ID;
    }
#endif

    printk("%s,%d\n",__FUNCTION__,__LINE__);

    /* Prepare the cluster space */
    _nic_isr_mbRoutine();

    printk("%s,%d\n",__FUNCTION__,__LINE__);

    printk("%s,%d\n",__FUNCTION__,__LINE__);

    /*Every thing is ok now, NIC can RX/TX, and enable interrupt trigger*/
    ioal_mem32_write(unit, RTL8380_DMA_IF_INTR_MSK_ADDR, CPUIIMR_ENABLE_MASK);  
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_RX_EN_OFFSET, RTL8380_DMA_IF_CTRL_RX_EN_MASK, 1);
    ioal_mem32_field_write(unit, RTL8380_DMA_IF_CTRL_ADDR, RTL8380_DMA_IF_CTRL_TX_EN_OFFSET, RTL8380_DMA_IF_CTRL_TX_EN_MASK, 1);
    
    return RT_ERR_OK;
#endif
} /* end of _nic_init */




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
        NIC_SEM_UNLOCK(unit);
        return ret;
    }

    NIC_SEM_UNLOCK(unit);

    return ret;
} /* end of r8380_nic_reset */



