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
 * $Revision: 35891 $
 * $Date: 2013-01-07 14:58:10 +0800 (Mon, 07 Jan 2013) $
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
#include <common/rt_autoconf.h>
#include <dev_config.h>
#include <soc/soc.h>
#include <soc/type.h>
#include <common/debug/rt_log.h>
#include <ioal/mem32.h>
#include <osal/isr.h>
#include <osal/cache.h>
#include <osal/lib.h>
#include <osal/print.h>
#include <osal/thread.h>
#include <osal/time.h>
#include <osal/sem.h>
#include <osal/spl.h>
#include <osal/atomic.h>
#include <osal/wait.h>
#include <drv/nic/r8389.h>
#include <drv/swcore/rtl8389.h>

/*
 * Symbol Definition
 */
#define NIC_CODE_REDUCE

/* CPUIISR - CPU Interface Interrupt Status Register */
#define INT_PHDS7_INT_PHDS0_OFFSET             (13)                                         /* Interrupt of EN_PHD running out status, write '1' to clear.     */
#define INT_PHDS7_INT_PHDS0_MASK               (0xFFU << INT_PHDS7_INT_PHDS0_OFFSET)        /* Interrupt of EN_PHD running out status, write '1' to clear.     */
#define INT_MBDS_OFFSET                        (12)                                         /* Interrupt of EN_MBD running out status, write '1' to clear.     */
#define INT_MBDS_MASK                          (0x1U << INT_MBDS_OFFSET)                    /* Interrupt of EN_MBD running out status, write '1' to clear.     */
#define INT_TX_DONE1_INT_TX_DONE0_OFFSET       (10)                                         /* Interrupt of EN_TX_DONE pending status, write '1' to clear.     */
#define INT_TX_DONE1_INT_TX_DONE0_MASK         (0x3U << INT_TX_DONE1_INT_TX_DONE0_OFFSET)   /* Interrupt of EN_TX_DONE pending status, write '1' to clear.     */
#define INT_TX_DONE1_OFFSET                    (11)
#define INT_TX_DONE1_MASK                      (0x1U << INT_TX_DONE1_OFFSET)
#define INT_TX_DONE0_OFFSET                    (10)
#define INT_TX_DONE0_MASK                      (0x1U << INT_TX_DONE0_OFFSET)
#define INT_RX_DONE7_INT_RX_DONE0_OFFSET       (2)                                          /* Interrupt of EN_RX_DONE pending status, write '1' to clear.     */
#define INT_RX_DONE7_INT_RX_DONE0_MASK         (0xFFU << INT_RX_DONE7_INT_RX_DONE0_OFFSET)  /* Interrupt of EN_RX_DONE pending status, write '1' to clear.     */
#define INT_RX_DONE7_OFFSET                    (9)
#define INT_RX_DONE7_MASK                      (0x1U << INT_RX_DONE7_OFFSET)
#define INT_RX_DONE6_OFFSET                    (8)
#define INT_RX_DONE6_MASK                      (0x1U << INT_RX_DONE6_OFFSET)
#define INT_RX_DONE5_OFFSET                    (7)
#define INT_RX_DONE5_MASK                      (0x1U << INT_RX_DONE5_OFFSET)
#define INT_RX_DONE4_OFFSET                    (6)
#define INT_RX_DONE4_MASK                      (0x1U << INT_RX_DONE4_OFFSET)
#define INT_RX_DONE3_OFFSET                    (5)
#define INT_RX_DONE3_MASK                      (0x1U << INT_RX_DONE3_OFFSET)
#define INT_RX_DONE2_OFFSET                    (4)
#define INT_RX_DONE2_MASK                      (0x1U << INT_RX_DONE2_OFFSET)
#define INT_RX_DONE1_OFFSET                    (3)
#define INT_RX_DONE1_MASK                      (0x1U << INT_RX_DONE1_OFFSET)
#define INT_RX_DONE0_OFFSET                    (2)
#define INT_RX_DONE0_MASK                      (0x1U << INT_RX_DONE0_OFFSET)
#define INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET (0)                                          /* Interrupt of EN_TX_ALLDONE pending status, write '1' to clear.  */
#define INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK   (0x3U << INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET) /* Interrupt of EN_TX_ALLDONE pending status, write '1' to clear.  */

#define CPUIIMR_ENABLE_MASK                 (INT_PHDS7_INT_PHDS0_MASK | INT_MBDS_MASK | INT_TX_DONE1_INT_TX_DONE0_MASK | INT_RX_DONE7_INT_RX_DONE0_MASK | INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK)
#define CPUIIMR_DISABLE_MASK                (0x00000000U)
#define CPUIISR_CLEARALL_MASK               (0xFFFFFFFFU)
#define CPUICR_DISABLE_MASK                 (0x00000000U)

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

#if defined(CONFIG_SDK_RX_THREAD)
#define NIC_RX_INTR_THREAD_NAME     "NIC_Rx_Intr_Thread"
#endif

/*
 * Data Type Definition
 */
typedef struct nic_rx_cb_entry_s
{
    drv_nic_rx_cb_f rx_callback;
    void *pCookie;
} nic_rx_cb_entry_t;

typedef struct nic_info_s
{
    uint32          rxFDPBase[NIC_RXRING_NUM][NIC_RXRING_SIZE];
    uint32          txFDPBase[NIC_TXRING_NUM][NIC_TXRING_SIZE];
    uint32          mBFDPBase[NIC_MBRING_SIZE];
    nic_pkthdr_t    rxPktHdr[NIC_RX_PKTHDR_NUM];
    nic_pkthdr_t    txPktHdr[NIC_TX_PKTHDR_NUM];
    nic_mbuf_t      rxMbuf[NIC_RX_MBUF_NUM];
    nic_mbuf_t      txMbuf[NIC_TX_MBUF_NUM];
} nic_info_t;

/*
 * Data Declaration
 */
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
#if 0
static osal_mutex_t nic_sem[RTK_MAX_NUM_OF_UNIT];
#endif
#if defined(CONFIG_SDK_RX_THREAD)
static uint32   nic_rx_isr_process = 0;
static osal_atomic_t nic_rx_wait_for_intr;
static osal_wait_queue_head_t nic_rx_intr_wait_queue;
#endif

/* Pointer for Software */
static nic_info_t   *pNicInfo;
static uint32       *pNic_rxFDPBase[NIC_RXRING_NUM];
static uint32       *pNic_txFDPBase[NIC_TXRING_NUM];
static uint32       *pNic_mBFDPBase;
static uint32       *pNic_rxCDPIdx[NIC_RXRING_NUM];
static uint32       *pNic_txCDPIdx[NIC_TXRING_NUM];
static uint32       *pNic_mBCDPIdx;
static uint32       *pNic_rxRDPIdx[NIC_RXRING_NUM];
static uint32       *pNic_txRDPIdx[NIC_TXRING_NUM];
static uint32       *pNic_mBRDPIdx;

/* NIC Tx/Rx debug information
 * The machanism is always enabled
 */
static uint32       nic_debug_flag;
static uint32       nic_tx_success_cntr;
static uint32       nic_tx_failed_cntr;
static uint32       nic_rx_success_cntr;
static uint32       nic_rx_failed_cntr;

/*
 * Macro Definition
 */
/* semaphore handling */
#if 0
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
#endif
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
static int32 _insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, nic_cpuTag_t *pCpuTag);
static int32 _removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, nic_cpuTag_t *pCpuTag);
#if defined(CONFIG_SDK_RX_THREAD)
static int32 _nic_rx_intr_attach(void);
static void _nic_rx_isr(uint32 nic_rx_iisr);
static void *_nic_rx_intr_thread(void *pArg);
static void _nic_rx_isr_handler(uint32 cpu_iisr);
#endif


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
    uint32  temp;
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

    ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_TAG_ID_CONTROL_ADDR, \
        RTL8389_CPU_TAG_ID_CONTROL_CPU_TAG_ID_OFFSET, RTL8389_CPU_TAG_ID_CONTROL_CPU_TAG_ID_MASK, &temp);
    
    *(pU8 + 12) = ((temp >> 8) & 0xFF);
    *(pU8 + 13) = (temp & 0xFF);

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
    uint32  temp;
    uint16  ethtype;

    if ((NULL == pPkt) || (NULL == ppNewPkt) || (NULL == pOffset) || (NULL == pCpuTag))
        return RT_ERR_FAILED;

#if defined(__MODEL_USER__) || defined(__MODEL_KERNEL__)
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_TAG_ID_CONTROL_ADDR, &temp);
    if (0 == temp)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_TAG_ID_CONTROL_ADDR, REALTEK_CPUTAG_ID << CPU_TAG_ID_OFFSET);
    }
#endif

    *(((uint8 *)&ethtype) + 0) = *(pPkt + 12);
    *(((uint8 *)&ethtype) + 1) = *(pPkt + 13);

    ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_TAG_ID_CONTROL_ADDR, \
        RTL8389_CPU_TAG_ID_CONTROL_CPU_TAG_ID_OFFSET, RTL8389_CPU_TAG_ID_CONTROL_CPU_TAG_ID_MASK, &temp);
    
    if (ethtype != (uint16)temp)
    {
        return RT_ERR_FAILED;
    }

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

    _RT_LOG(LOG_DEBUG, MOD_NIC, "ringId = %d", ringId);

    if (ringId >= NIC_RXRING_NUM)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "invalid ringId(%d)!", ringId);
        nic_rx_failed_cntr++;
        return RT_ERR_FAILED;
    }

    /* Update software current pointer */
    //if ((ret = reg_read(NIC_DEFAULT_UNIT_ID, RX_PKTHDR_DESCRIPTOR_0_CONTROL + ringId, &temp)) != RT_ERR_OK)
    //{
    //    nic_rx_failed_cntr++;
    //    return ret;
    //}
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8389_RX_PKTHDR_DESCRIPTOR_0_CONTROL_ADDR + (0x4 * ringId), &temp);
    
    pNic_rxCDPIdx[ringId] = (uint32 *)temp;    /* The limitation */

    do
    {
        uint32        offset, scan_offset;
        uint8         pkt_data;
        nic_pkthdr_t  *pktHdr;
        nic_cpuTag_t  cputag;
        drv_nic_pkt_t *pPacket;

        if ((*pNic_rxRDPIdx[ringId] & NIC_RING_SWOWNBIT) != 0)
            break;

        /* Prepare to be reclaim */
        pktHdr = (nic_pkthdr_t *)((*pNic_rxRDPIdx[ringId]) & NIC_ADDR_MASK);
        if (NULL == pktHdr || NULL == pktHdr->ph_mbuf || NULL == pktHdr->ph_mbuf->packet)
            break;

        /* NIC Rx debug message */
        if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
        {
            int i;
            int dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */

            osal_printf("=== [NIC RX Debug] ================================= Len: %d \n", pktHdr->ph_mbuf->m_len);
            for (i = 0; i < dump_len; i++)
            {
                if (i == (pktHdr->ph_mbuf->m_len))
                    break;
                if (0 == (i % 16))
                    osal_printf("[%04X] ", i);
                osal_printf("%02X ", *(pktHdr->ph_mbuf->packet->data + i));
                if (15 == (i % 16))
                    osal_printf("\n");
            }
            osal_printf("\n");
        }

        pPacket = pktHdr->ph_mbuf->packet;
        pPacket->length = pktHdr->ph_mbuf->m_len;
        pPacket->tail = pPacket->data + pPacket->length;

        /* Remove the CPU Rx tag from the packet data buffer */
        if (RT_ERR_OK != _removeCPUTag(pPacket->data, &pPacket->data, &offset, &cputag))
        {
            nic_rx_failed_cntr++;
            return RT_ERR_FAILED;
        }

        pPacket->rx_tag.source_port     = cputag.un.rx.SPHY;
        pPacket->rx_tag.priority        = cputag.un.rx.PRI;
        pPacket->rx_tag.reason          = cputag.un.rx.REASON;
        pPacket->rx_tag.l2_error        = cputag.un.rx.L2ERR;
        pPacket->rx_tag.l3_error        = cputag.un.rx.L3ERR;
        pPacket->rx_tag.l4_error        = cputag.un.rx.L4ERR;
        pPacket->rx_tag.pppoe           = cputag.un.rx.PPPoE;
        pPacket->rx_tag.svid_tagged     = cputag.un.rx.SEXIST;
        pPacket->rx_tag.cvid_tagged     = cputag.un.rx.CEXIST;
        pPacket->rx_tag.drop_precedence = cputag.un.rx.DP;
        pPacket->rx_tag.l2_format       = cputag.un.rx.L2FMT;
        pPacket->rx_tag.l3_l4_format    = cputag.un.rx.L34FMT;
        
        scan_offset = 12;
        if (pPacket->rx_tag.svid_tagged)
        {
            pkt_data = *(pktHdr->ph_mbuf->packet->data + scan_offset + 2);
            pPacket->rx_tag.outer_pri = (pkt_data >> 5) & 0x7;
            pPacket->rx_tag.outer_vid = ((pkt_data & 0xF) << 8);
            pkt_data = *(pktHdr->ph_mbuf->packet->data + scan_offset + 3);
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
            pkt_data = *(pktHdr->ph_mbuf->packet->data + scan_offset + 2);
            pPacket->rx_tag.inner_pri = (pkt_data >> 5) & 0x7;
            pPacket->rx_tag.inner_vid = ((pkt_data & 0xF) << 8);
            pkt_data = *(pktHdr->ph_mbuf->packet->data + scan_offset + 3);
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
            osal_printf(" SPHY : %d \n", cputag.un.rx.SPHY);
            osal_printf(" PRI : %d \n", cputag.un.rx.PRI);
            osal_printf(" REASON : %d \n", cputag.un.rx.REASON);
            osal_printf(" L2ERR : %d \n", cputag.un.rx.L2ERR);
            osal_printf(" L3ERR : %d \n", cputag.un.rx.L3ERR);
            osal_printf(" L4ERR : %d \n", cputag.un.rx.L4ERR);
            osal_printf(" PPPoE : %d \n", cputag.un.rx.PPPoE);
            osal_printf(" SEXIST : %d \n", cputag.un.rx.SEXIST);
            osal_printf(" CEXIST : %d \n", cputag.un.rx.CEXIST);
            osal_printf(" DP : %d \n", cputag.un.rx.DP);
            osal_printf(" L2FMT : %d \n", cputag.un.rx.L2FMT);
            osal_printf(" L34FMT : %d \n", cputag.un.rx.L34FMT);
        }

#ifdef NIC_RX_THREAD
        /* enqueue this packet into Rx Queue */
        if (pNic_Rx_Queue[_nic_rx_enQueueCnt] == NULL)
        {
            /* enqueue */
            pNic_Rx_Queue[_nic_rx_enQueueCnt] = pktHdr->ph_mbuf->packet;
            _nic_rx_enQueueCnt = (_nic_rx_enQueueCnt + 1) % NIC_RX_THREAD_QUEUE_LENGTH;
        }
        else
        {
            /* We have to free this packet here (because the rx queue is full) */
            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf->packet);
        }
#else
        nic_rx_handle = NIC_RX_NOT_HANDLED;
        for (i = 0; i < NIC_RX_CB_PRIORITY_NUMBER; i++)
        {
            if (_nic_rx_cb_tbl[i].rx_callback != NULL)
            {
                nic_rx_handle = _nic_rx_cb_tbl[i].rx_callback(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf->packet, _nic_rx_cb_tbl[i].pCookie);
                if (NIC_RX_HANDLED_OWNED == nic_rx_handle)
                    break;
            }
        }
        if (nic_rx_handle != NIC_RX_HANDLED_OWNED)
        {   /* We have to free this packet here */
            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf->packet);
        }
#endif

        /* Alloc a new packet data buffer */
        if (RT_ERR_OK == _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
        {
            pktHdr->ph_mbuf->packet = pPacket;
            pktHdr->ph_mbuf->m_extbuf = (uint8 *)(UNCACHE(pPacket->head));
            pktHdr->ph_mbuf->m_extsize = (pPacket->end - pPacket->head);
            pktHdr->ph_mbuf->m_data = (uint8 *)(UNCACHE(pPacket->data));
            pktHdr->ph_mbuf->m_len = 0;
            if ((ret = osal_cache_memory_flush((uint32)pPacket->head, (pPacket->end - pPacket->head))) != RT_ERR_OK)
            {
                return ret;
            }

            MEMORY_BARRIER();
            *(pktHdr->ph_mbuf->ring_entry) |= NIC_RING_SWOWNBIT;    /* Only set the SwOwn bit */

#ifndef NIC_CODE_REDUCE
            /* To guarantee it's write done */
            do
            {
                uint32 chk;
                chk = *(pktHdr->ph_mbuf->ring_entry);
            } while (0);
#endif
        }

        pktHdr->ph_mbuf->m_pkthdr = NULL;
        pktHdr->ph_mbuf = NULL;
        pktHdr->ph_len = 0;
        MEMORY_BARRIER();
        *(pktHdr->ring_entry) |= NIC_RING_SWOWNBIT;  /* Only set the SwOwn bit */

#ifndef NIC_CODE_REDUCE
        /* To guarantee it's write done */
        do
        {
            uint32 chk;
            chk = *(pktHdr->ring_entry);
        } while (0);
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
    _RT_LOG(LOG_DEBUG, MOD_NIC, "ringId = %d", ringId);

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
        pktHdr = (nic_pkthdr_t *)((*pNic_txRDPIdx[ringId]) & NIC_ADDR_MASK);
        if (NULL == pktHdr || NULL == pktHdr->ph_mbuf || NULL == pktHdr->ph_mbuf->packet)
        {
            break;
        }
        /* Callback Tx CB Function (auto-free the abandoned packet) */
        if (pktHdr->tx_callback == NULL)
        {
            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf->packet);
        }
        else
        {
            pktHdr->tx_callback(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf->packet, pktHdr->cookie);
            pktHdr->tx_callback = NULL;
        }
        pktHdr->ph_mbuf->packet = NULL;

        /* Jump to next */
        pNic_txRDPIdx[ringId] += 1;
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
    drv_nic_pkt_t *pPacket;

    /* Update software current pointer */
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8389_RX_MBUF_DESCRIPTOR_CONTROL_ADDR, &temp);
    
    pNic_mBCDPIdx = (uint32 *)temp;

    do
    {
        nic_mbuf_t *pMBuf;

        if ((*pNic_mBRDPIdx & NIC_RING_SWOWNBIT) != 0)
            break;

        /* Prepare to be reclaim */
        pMBuf = (nic_mbuf_t *)((*pNic_mBRDPIdx) & NIC_ADDR_MASK);
        if (NULL == pMBuf || pMBuf->m_pkthdr != NULL)
            break;

        /* Alloc a new packet data buffer */
        if (RT_ERR_OK != _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Out of memory ! (alloc a new packet data buffer failed)");
            break;
        }

        pMBuf->packet = pPacket;
        pMBuf->m_extbuf = (uint8 *)(UNCACHE(pPacket->head));
        pMBuf->m_extsize = (pPacket->end - pPacket->head);
        pMBuf->m_data = (uint8 *)(UNCACHE(pPacket->data));
        pMBuf->m_len = 0;
        if ((ret = osal_cache_memory_flush((uint32)pPacket->head, (pPacket->end - pPacket->head))) != RT_ERR_OK)
        {
            return ret;
        }
        MEMORY_BARRIER();
        *(pMBuf->ring_entry) |= NIC_RING_SWOWNBIT;    /* Only set the SwOwn bit */

#ifndef NIC_CODE_REDUCE
        /* To guarantee it's write done */
        do
        {
            uint32 chk;
            chk = *(pMBuf->ring_entry);
        } while (0);
#endif

        /* Jump to next */
        pNic_mBRDPIdx += 1;
        if (pNic_mBRDPIdx == (pNic_mBFDPBase + NIC_MBRING_SIZE))
            pNic_mBRDPIdx = pNic_mBFDPBase;
    } while (pNic_mBRDPIdx != pNic_mBCDPIdx);

    return RT_ERR_OK;
} /* end of _nic_isr_mbRoutine */


#if defined(CONFIG_SDK_RX_THREAD)
static int32 _nic_rx_intr_attach(void)
{
    int32 ret;    

    /* Check arguments */
    ret = osal_thread_create(NIC_RX_INTR_THREAD_NAME, 4096, -20, (void *)_nic_rx_intr_thread, NULL);
    if (0 == ret)
    {
      RT_ERR(ret, MOD_RTCORE, "NIC Rx interrupt thread create failed");
      return RT_ERR_FAILED;     
    }           
    return RT_ERR_OK;
} /* end of _nic_rx_intr_attach */

static void _nic_rx_isr(uint32 nic_rx_iisr)
{    
    nic_rx_isr_process = nic_rx_iisr;
    ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, nic_rx_iisr);

    if (osal_atomic_dec_return(&nic_rx_wait_for_intr) >= 0) 
    {
        osal_wake_up_interruptible(&nic_rx_intr_wait_queue);
    }
    return;
} /* end of _nic_rx_isr */

static void *_nic_rx_intr_thread(void *pArg)
{   
    osal_init_waitqueue_head(&nic_rx_intr_wait_queue);

    while(1)
    {            
        if (nic_rx_wait_for_intr.counter <= 0)
        {
            osal_atomic_inc(&nic_rx_wait_for_intr);
        }      
        osal_wait_event_interruptible(nic_rx_intr_wait_queue, osal_atomic_read(&nic_rx_wait_for_intr) <= 0);
        /*                      
        * only run the nic_rx interrupt handler once.
        */
        osal_atomic_set(&nic_rx_wait_for_intr, 0);
        
        _nic_rx_isr_handler(nic_rx_isr_process);             
    }    
    return NULL;
} /* end of _nic_rx_intr_thread */

static void _nic_rx_isr_handler(uint32 nic_rx_isr_mask)
{
    /* Rx have 8 rings (0..7); need to process when any rx ring isr done */
    if (nic_rx_isr_mask & INT_RX_DONE7_INT_RX_DONE0_MASK)
    {
        if (nic_rx_isr_mask & INT_RX_DONE7_MASK)   /* Process Rx ring 7 ISR */
        {
            _nic_isr_rxRoutine(7);
        }

        if (nic_rx_isr_mask & INT_RX_DONE6_MASK)   /* Process Rx ring 6 ISR */
        {
            _nic_isr_rxRoutine(6);
        }

        if (nic_rx_isr_mask & INT_RX_DONE5_MASK)   /* Process Rx ring 5 ISR */
        {
            _nic_isr_rxRoutine(5);
        }

        if (nic_rx_isr_mask & INT_RX_DONE4_MASK)   /* Process Rx ring 4 ISR */
        {
            _nic_isr_rxRoutine(4);
        }

        if (nic_rx_isr_mask & INT_RX_DONE3_MASK)   /* Process Rx ring 3 ISR */
        {
            _nic_isr_rxRoutine(3);
        }

        if (nic_rx_isr_mask & INT_RX_DONE2_MASK)   /* Process Rx ring 2 ISR */
        {
            _nic_isr_rxRoutine(2);
        }

        if (nic_rx_isr_mask & INT_RX_DONE1_MASK)   /* Process Rx ring 1 ISR */
        {
            _nic_isr_rxRoutine(1);
        }

        if (nic_rx_isr_mask & INT_RX_DONE0_MASK)   /* Process Rx ring 0 ISR */
        {
            _nic_isr_rxRoutine(0);
        }
    }

    return;
} /* end of _nic_rx_isr_handler */
#endif

/* Function Name:
 *      _nic_isr_handler
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
_nic_isr_handler(void *isr_param)
{
    uint32  cpu_iisr;

    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, &cpu_iisr);

    /* Rx have 8 rings (0..7); need to process when any rx ring isr done */
    if (cpu_iisr & INT_RX_DONE7_INT_RX_DONE0_MASK)
    {
#if defined(CONFIG_SDK_RX_THREAD)
        _nic_rx_isr(cpu_iisr & INT_RX_DONE7_INT_RX_DONE0_MASK);
#else
        if (cpu_iisr & INT_RX_DONE7_MASK)   /* Process Rx ring 7 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE7_MASK);
            _nic_isr_rxRoutine(7);
        }

        if (cpu_iisr & INT_RX_DONE6_MASK)   /* Process Rx ring 6 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE6_MASK);
            _nic_isr_rxRoutine(6);
        }

        if (cpu_iisr & INT_RX_DONE5_MASK)   /* Process Rx ring 5 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE5_MASK);
            _nic_isr_rxRoutine(5);
        }

        if (cpu_iisr & INT_RX_DONE4_MASK)   /* Process Rx ring 4 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE4_MASK);
            _nic_isr_rxRoutine(4);
        }

        if (cpu_iisr & INT_RX_DONE3_MASK)   /* Process Rx ring 3 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE3_MASK);
            _nic_isr_rxRoutine(3);
        }

        if (cpu_iisr & INT_RX_DONE2_MASK)   /* Process Rx ring 2 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE2_MASK);
            _nic_isr_rxRoutine(2);
        }

        if (cpu_iisr & INT_RX_DONE1_MASK)   /* Process Rx ring 1 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE1_MASK);
            _nic_isr_rxRoutine(1);
        }

        if (cpu_iisr & INT_RX_DONE0_MASK)   /* Process Rx ring 0 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_RX_DONE0_MASK);
            _nic_isr_rxRoutine(0);
        }
#endif
    }

    /* Tx have 2 rings (0..1); need to process when any tx ring isr done */
    if (cpu_iisr & INT_TX_DONE1_INT_TX_DONE0_MASK)
    {
        if (cpu_iisr & INT_TX_DONE1_MASK)   /* Process Tx ring 1 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_TX_DONE1_MASK);
            _nic_isr_txRoutine(1);
        }

        if (cpu_iisr & INT_TX_DONE0_MASK)   /* Process Tx ring 0 ISR */
        {
            ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_TX_DONE0_MASK);
            _nic_isr_txRoutine(0);
        }
    }

    if (cpu_iisr & INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK);
    }

    /* pktHdr Runout */
    if (cpu_iisr & INT_PHDS7_INT_PHDS0_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_PHDS7_INT_PHDS0_MASK);
    }

    /* mBuffer Runout */
    if (cpu_iisr & INT_MBDS_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, INT_MBDS_MASK);
        _nic_isr_mbRoutine();
    }

    return OSAL_INT_HANDLED;
} /* end of _nic_isr_handler */

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
    uint32  temp, chip_type, int_ver;
    int32   i, j, k, l;
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
    ioal_mem32_field_write(unit, RTL8389_PORT_28_MAC_CONTROL_ADDR, \
        RTL8389_PORT_28_MAC_CONTROL_EN_TXRX_P28_OFFSET, RTL8389_PORT_28_MAC_CONTROL_EN_TXRX_P28_MASK, 0);
    
    osal_time_usleep(50 * 1000); /* delay 50mS */
    
    RT_LOG(LOG_DEBUG, MOD_NIC, "Reset NIC (R8389)... ");
    
    ioal_mem32_field_write(unit, RTL8389_RESET_GLOBAL_CONTROL_ADDR, \
        RTL8389_RESET_GLOBAL_CONTROL_SW_NIC_RST_OFFSET, RTL8389_RESET_GLOBAL_CONTROL_SW_NIC_RST_MASK, 0);
    
    do
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
        ioal_mem32_field_read(unit, RTL8389_RESET_GLOBAL_CONTROL_ADDR, \
            RTL8389_RESET_GLOBAL_CONTROL_SW_NIC_RST_OFFSET, RTL8389_RESET_GLOBAL_CONTROL_SW_NIC_RST_MASK, &temp);
    } while (temp != 1);

    RT_LOG(LOG_DEBUG, MOD_NIC, "OK");

    /* CPU port: Enable MAC Tx/Rx */
    ioal_mem32_field_write(unit, RTL8389_PORT_28_MAC_CONTROL_ADDR, \
        RTL8389_PORT_28_MAC_CONTROL_EN_TXRX_P28_OFFSET, RTL8389_PORT_28_MAC_CONTROL_EN_TXRX_P28_MASK, 3);

    /* Enable internal register display */
    ioal_mem32_field_write(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, \
        RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_OFFSET, RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_MASK, 0xA);

    /* Read chip type and int reversion and do patch for RTL8329M A-CUT */
    ioal_mem32_field_read(unit, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_ADDR, \
        RTL8389_GLOBAL_MAC_INTERNAL_STATUS_CHIP_TYP_OFFSET, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_CHIP_TYP_MASK, &chip_type);

    ioal_mem32_field_read(unit, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_ADDR, \
        RTL8389_GLOBAL_MAC_INTERNAL_STATUS_INT_VER_OFFSET, RTL8389_GLOBAL_MAC_INTERNAL_STATUS_INT_VER_MASK, &int_ver);
    
    if ((2 == chip_type) && (0 == int_ver))
    {
        do
        {   
            /* apply patch for RTL8329M A-CUT */
            ioal_mem32_write(unit, RTL8389_SWITCH_PLL_CONTROL0_ADDR, 0x00008EA4);
            ioal_mem32_read(unit, RTL8389_SWITCH_PLL_CONTROL0_ADDR, &temp);  
            
        } while (temp != 0x00008EA4);
    }

    ioal_mem32_field_write(unit, RTL8389_GLOBAL_MAC_CONTROL0_ADDR, \
        RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_OFFSET, RTL8389_GLOBAL_MAC_CONTROL0_EN_INT_REG_MASK, 0x0);

    /* Set 25~28 Combo-Port + CPU port to join the Lookup Miss Flooding Portmask */
    ioal_mem32_read(unit, RTL8389_LOOKUP_MISS_FLOODING_PORTMASK_ADDR, &temp);  
    temp |= 0x1F000000;
    ioal_mem32_write(unit, RTL8389_LOOKUP_MISS_FLOODING_PORTMASK_ADDR, temp);  

    /* CPU port: Enable MAC Tx/Rx */
    ioal_mem32_field_write(unit, RTL8389_PORT_28_MAC_CONTROL_ADDR, \
        RTL8389_PORT_28_MAC_CONTROL_EN_TXRX_P28_OFFSET, RTL8389_PORT_28_MAC_CONTROL_EN_TXRX_P28_MASK, 3);

    /* CPU port: Force link-up */
    ioal_mem32_field_write(unit, RTL8389_PORT_28_PHY_CONTROL_ADDR, \
        RTL8389_PORT_28_PHY_CONTROL_EN_FORCE_LINK_P28_OFFSET, RTL8389_PORT_28_PHY_CONTROL_EN_FORCE_LINK_P28_MASK, 1);

    /* Reset to default value */
    ioal_mem32_write(unit, RTL8389_CPU_INTERFACE_INTERRUPT_MASK_ADDR, CPUIIMR_DISABLE_MASK);  
    ioal_mem32_write(unit, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, CPUIISR_CLEARALL_MASK);  
    ioal_mem32_write(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, CPUICR_DISABLE_MASK);  

    /* SRAM MAP Setting - Pointer to NIC info memory */
    /* physical address: 0x1000_0000 --> virtual memory 0xB000_0000 */
    /* USE SRAM: 0xB000_0000 ~ 0xB000_7FFF (32KB)    */
    REG32(SRAMSBR0) = SRAM_SEG_BASE;        /* SRAM segment base = 0x0000_0000 */
    REG32(SRAMSSR0) = SRAM_SEG_SIZE_32KB;   /* SRAM segment size = 32K bytes   */
    REG32(SRAMSAR0) = (SRAM_SEG_ADDR | SRAM_ENABLE);    /* SRAM segment addr = 0x1000_0000 (0xB000_0000) */
    pNicInfo = (nic_info_t *)0xB0000000;    /* MAP to virtual memory (UNCACHE) */
    REG32(UMSSR0) = SRAM_SEG_SIZE_32KB;
    REG32(UMSAR0) = (SRAM_SEG_ADDR | SRAM_ENABLE);

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
    pNic_mBFDPBase = pNicInfo->mBFDPBase;
    RT_LOG(LOG_DEBUG, MOD_NIC, "pNic_mBFDPBase = %p", pNic_mBFDPBase);
    pNic_mBCDPIdx = pNic_mBFDPBase;
    pNic_mBRDPIdx = pNic_mBFDPBase;

    /* Rx pktHdrs */
    i = 0;
    for (j = 0; j < NIC_RXRING_NUM; j++)
    {
        for (k = 0; k < NIC_RXRING_SIZE; k++)
        {
            nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->rxPktHdr[i];
            pPktHdr->ph_mbuf     = NULL;
            pPktHdr->ph_len      = 0;
            pPktHdr->ring_entry  = (pNic_rxFDPBase[j] + k);

            *(pNic_rxFDPBase[j] + k) = ((k + 1) == NIC_RXRING_SIZE)? \
                (uint32)pPktHdr | NIC_RING_WRAPBIT | NIC_RING_SWOWNBIT : \
                (uint32)pPktHdr | NIC_RING_SWOWNBIT;

            RT_LOG(LOG_DEBUG, MOD_NIC, "RxRing[%01d][%01d] &0x%08X = 0x%08X", j, k, (uint32)(pNic_rxFDPBase[j] + k), *(pNic_rxFDPBase[j] + k));
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
            pPktHdr->ph_mbuf     = NULL;
            pPktHdr->ph_len      = 0;
            pPktHdr->ring_entry  = (pNic_txFDPBase[j] + k);

            *(pNic_txFDPBase[j] + k) = ((k + 1) == NIC_TXRING_SIZE)? \
                (uint32)pPktHdr | NIC_RING_WRAPBIT : \
                (uint32)pPktHdr;

            RT_LOG(LOG_DEBUG, MOD_NIC, "TxRing[%01d][%01d] &0x%08X = 0x%08X", j, k, (uint32)(pNic_txFDPBase[j] + k), *(pNic_txFDPBase[j] + k));
            i++;
        }
    }

    /* Rx mBuf */
    for (i = 0; i < NIC_MBRING_SIZE; i++)
    {
        nic_mbuf_t *pMBuf;

        if (i >= NIC_RX_MBUF_NUM)
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "The Rx mbuf number is not enough to hang on mBuf ring!");
            break;
        }

        pMBuf = &pNicInfo->rxMbuf[i];
        pMBuf->ring_entry = (pNic_mBFDPBase + i);

        *(pNic_mBFDPBase + i) = ((i + 1) == NIC_MBRING_SIZE)? \
            (uint32)pMBuf | NIC_RING_WRAPBIT : \
            (uint32)pMBuf;

        RT_LOG(LOG_DEBUG, MOD_NIC, "MbRing[%02d] &0x%08X = 0x%08X", i, (uint32)(pNic_mBFDPBase + i), *(pNic_mBFDPBase + i));
    }

    /* Tx mBuf */
    i = 0;
    for (j = 0; j < NIC_TXRING_NUM; j++)
    {
        for (k = 0; k < NIC_TXRING_SIZE; k++)
        {
            for (l = 0; l < NIC_TX_MBUF_MAX; l++)
            {
                nic_mbuf_t *pMBuf;

                if (i >= NIC_TX_MBUF_NUM)
                {
                    RT_LOG(LOG_DEBUG, MOD_NIC, "The Tx mbuf number is not enough to hang on Tx ring!");
                    break;
                }

                pMBuf = &pNicInfo->txMbuf[i];
                pMBuf->ring_entry = (pNic_txFDPBase[j] + k);

                if (0 == l)
                {
                    nic_pkthdr_t *pPktHdr;

                    pPktHdr = (nic_pkthdr_t *)(pNicInfo->txFDPBase[j][k] & NIC_ADDR_MASK);
                    pPktHdr->ph_mbuf = pMBuf;
                }

                i++;
            }
        }
    }

    /* Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_register(RTK_DEV_NIC, _nic_isr_handler, NULL))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }

    /* Setting Registers */
    for (i = 0; i < NIC_RXRING_NUM; i++)
        ioal_mem32_write(unit, RTL8389_RX_PKTHDR_DESCRIPTOR_0_CONTROL_ADDR + (0x4 * i), (uint32)pNic_rxFDPBase[i]);  

    for (i = 0; i < NIC_TXRING_NUM; i++)
        ioal_mem32_write(unit, RTL8389_TX_PKTHDR_DESCRIPTOR_0_CONTROL_ADDR + (0x4 * i), (uint32)pNic_txFDPBase[i]);  
    
    ioal_mem32_write(unit, RTL8389_RX_MBUF_DESCRIPTOR_CONTROL_ADDR, (uint32)pNic_mBFDPBase);  
    ioal_mem32_write(unit, RTL8389_CPU_INTERFACE_INTERRUPT_MASK_ADDR, CPUIIMR_ENABLE_MASK);  

    ioal_mem32_field_write(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, \
        RTL8389_CPU_INTERFACE_CONTROL_LENCRC_OFFSET, RTL8389_CPU_INTERFACE_CONTROL_LENCRC_MASK, 1);

    ioal_mem32_field_write(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, \
        RTL8389_CPU_INTERFACE_CONTROL_TX_CMD_OFFSET, RTL8389_CPU_INTERFACE_CONTROL_TX_CMD_MASK, 1);
    
    /* Prepare the mBufs once */
    _nic_isr_mbRoutine();

#if defined(CONFIG_SDK_RX_THREAD)
    /* attach the nic_rx thread */
    _nic_rx_intr_attach();
#endif

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
    uint32  packetSize;
    int32   ret = RT_ERR_FAILED;
    nic_pkthdr_t    *pPktHdr;
    nic_mbuf_t      *pMBuf;
    nic_cpuTag_t    cputag;

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
    if (NULL == pPktHdr || NULL == pPktHdr->ph_mbuf || NULL != pPktHdr->ph_mbuf->packet)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr is NOT available!");
        return RT_ERR_FAILED;
    }

    /* Insert the CPU Tx tag into the packet data buffer */
    if (pPacket->as_txtag)
    {
        uint32 offset;

        /* Check the reserved headroom size have at least 8 bytes */
        if ((pPacket->data - pPacket->head) < (CPU_TAG_ID_LEN + sizeof(nic_cpuTag_t)))
        {
            /* The reserved space is NOT enough */
            RT_LOG(LOG_DEBUG, MOD_NIC, "pPacket->as_txtag = %d", pPacket->as_txtag);
            RT_LOG(LOG_DEBUG, MOD_NIC, "(pPacket->data - pPacket->head) = %d", (pPacket->data - pPacket->head));
            return RT_ERR_FAILED;
        }

        cputag.un.tx.PROTO      = REALTEK_CPUTAG_PROTOCOL_ID; /* CPU tag protocol id */
        cputag.un.tx.DPM        = pPacket->tx_tag.dst_port_mask;
        cputag.un.tx.ASDPM      = pPacket->tx_tag.as_dst_port_mask;
        cputag.un.tx.ASP        = pPacket->tx_tag.as_priority;
        cputag.un.tx.ASPRMK     = pPacket->tx_tag.as_port_remark;
        cputag.un.tx.L2CALC     = pPacket->tx_tag.l2_recalculate;
        cputag.un.tx.L3CALC     = pPacket->tx_tag.l3_recalculate;
        cputag.un.tx.L4CALC     = pPacket->tx_tag.l4_recalculate;
        cputag.un.tx.DP         = pPacket->tx_tag.drop_precedence;
        cputag.un.tx.PRI        = pPacket->tx_tag.priority;
        cputag.un.tx.L2LEARNING = !(pPacket->tx_tag.l2_learning);

        if (RT_ERR_OK != _insertCPUTag(pPacket->data, (pPacket->data - pPacket->head), &pPacket->data, &offset, &cputag))
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Error - CPU Tx tag insert failed!");
            return RT_ERR_FAILED;
        }
        
        pPacket->length += offset;
    }

    /* Step: Calc the pktbuf number */
    packetNum = 1;    /* Support single pktBuf now - one descriptor vs. one mbuf */
    packetSize = pPacket->length;

    pPktHdr->tx_callback = fTxCb;    /* Tx Callback function */
    pPktHdr->cookie = pCookie;
    pMBuf = pPktHdr->ph_mbuf;

    pMBuf->packet = pPacket;
    pMBuf->m_pkthdr = pPktHdr;
    pMBuf->m_extbuf = (uint8 *)(UNCACHE(pPacket->head));
    pMBuf->m_extsize = (pPacket->end - pPacket->head);
    pMBuf->m_data = (uint8 *)(UNCACHE(pPacket->data));
    pMBuf->m_len = (pPacket->length);
    
    if ((ret = osal_cache_memory_flush((uint32)pPacket->head, (pPacket->end - pPacket->head))) != RT_ERR_OK)
    {
        return ret;
    }
    
    pMBuf->m_pkthdr = pPktHdr;
    pPktHdr->ph_mbuf = pMBuf;

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

        if (pPacket->as_txtag)
            osal_printf("=== [NIC TX Debug] ================================= Len: %d + %d\n", pMBuf->m_len - 8, 8);
        else
            osal_printf("=== [NIC TX Debug] ================================= Len: %d \n", pMBuf->m_len);

        for (i = 0; i < dump_len; i++)
        {
            if (i == (pMBuf->m_len))
                break;
            if (0 == (i % 16))
                osal_printf("[%04X] ", i);
            osal_printf("%02X ", *(pMBuf->m_data + i));
            if (15 == (i % 16))
                osal_printf("\n");
        }
        osal_printf("\n");
    }

    if ((nic_debug_flag & DEBUG_TX_CPU_TAG_BIT) && pPacket->as_txtag)
    {
        osal_printf("=== [NIC TX Debug - CPU Tx Tag Information] ============ \n");
        osal_printf(" DPM : 0x%0x \n", cputag.un.tx.DPM);
        osal_printf(" ASDPM : %d \n", cputag.un.tx.ASDPM);
        osal_printf(" ASP : %d \n", cputag.un.tx.ASP);
        osal_printf(" ASPRMK : %d \n", cputag.un.tx.ASPRMK);
        osal_printf(" L2CALC : %d \n", cputag.un.tx.L2CALC);
        osal_printf(" L3CALC : %d \n", cputag.un.tx.L3CALC);
        osal_printf(" L4CALC : %d \n", cputag.un.tx.L4CALC);
        osal_printf(" DP : %d \n", cputag.un.tx.DP);
        osal_printf(" PRI : %d \n", cputag.un.tx.PRI);
        osal_printf(" L2LEARNING : %d \n", cputag.un.tx.L2LEARNING);
    }

    /* Set the TX Fetch Notify bit */
    ioal_mem32_field_write(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, \
        RTL8389_CPU_INTERFACE_CONTROL_TXFN_OFFSET, RTL8389_CPU_INTERFACE_CONTROL_TXFN_MASK, 1);
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
    ioal_mem32_field_write(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, \
        RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_OFFSET, RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_MASK, 1);

    RT_LOG(LOG_DEBUG, MOD_NIC, "NIC (R8389) Rx Start... ");

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
    ioal_mem32_field_write(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, \
        RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_OFFSET, RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_MASK, 0);
    
    RT_LOG(LOG_DEBUG, MOD_NIC, "NIC (R8389) Rx Stop... ");
    
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
 *      r8389_init
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
r8389_init(uint32 unit, drv_nic_initCfg_t *pInitCfg)
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
} /* end of r8389_init */


/* Function Name:
 *      r8389_pkt_tx
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
r8389_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
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
} /* end of r8389_pkt_tx */


/* Function Name:
 *      r8389_rx_start
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
r8389_rx_start(uint32 unit)
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
} /* end of r8389_rx_start */


/* Function Name:
 *      r8389_rx_stop
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
r8389_rx_stop(uint32 unit)
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
} /* end of r8389_rx_stop */


/* Function Name:
 *      r8389_rx_register
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
r8389_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags)
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
} /* end of r8389_rx_register */


/* Function Name:
 *      r8389_rx_unregister
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
r8389_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb)
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
} /* end of r8389_rx_unregister */


/* Function Name:
 *      r8389_pkt_alloc
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
r8389_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket)
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
} /* end of r8389_pkt_alloc */

/* Function Name:
 *      r8389_pkt_free
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
r8389_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket)
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
} /* end of r8389_pkt_free */

/* NIC Tx/Rx debug */
/* Function Name:
 *      r8389_debug_set
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
r8389_debug_set(uint32 unit, uint32 flags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    nic_debug_flag = flags;
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8389_debug_set */

/* Function Name:
 *      r8389_debug_get
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
r8389_debug_get(uint32 unit, uint32 *pFlags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    *pFlags = nic_debug_flag;
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8389_debug_get */

/* Function Name:
 *      r8389_counter_dump
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
r8389_counter_dump(uint32 unit)
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
} /* end of r8389_counter_dump */

/* Function Name:
 *      r8389_counter_clear
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
r8389_counter_clear(uint32 unit)
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
} /* end of r8389_counter_clear */


/* Function Name:
 *      r8389_bufStatus_dump
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
 *      - Rx Ring mBuffer (FDPBase, CDPIdx, RDPIdx)
 *      - Tx Ring mBuffer (FDPBase, CDPIdx, RDPIdx)
 *      2) From HW View
 *      - Rx Ring Packet Header(CDPIdx)
 *      - Tx Ring Packet Header(CDPIdx)
 *      - Rx Ring mBuffer (CDPIdx)
 *      - Tx Ring mBuffer (CDPIdx)
 *      3) Register Information
 *      - CPUIIMR (CPU Interface Interrupt Mask Register)
 *      - CPUIISR (CPU Interface Interrupt Status Register)
 *      - CPUICR  (CPU Interface Control Register)
 */
int32
r8389_bufStatus_dump(uint32 unit)
{
    uint32  i, value;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    osal_printf("RXRING  SW_rxFDPBase  SW_RxCDPIdx  HW_RxCDPIdx  SW_RxRDPIdx \n");
    osal_printf("=========================================================== \n");
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_read(unit, RTL8389_RX_PKTHDR_DESCRIPTOR_0_CONTROL_ADDR + (0x4 * i), &value);

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
        ioal_mem32_read(unit, RTL8389_TX_PKTHDR_DESCRIPTOR_0_CONTROL_ADDR + (0x4 * i), &value);

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

    osal_printf("        SW_mBFDPBase  SW_mBCDPIdx  HW_mBCDPIdx  SW_mBRDPIdx \n");
    osal_printf("=========================================================== \n");
    ioal_mem32_read(unit, RTL8389_RX_MBUF_DESCRIPTOR_CONTROL_ADDR, &value);

    osal_printf("  (p)   0x%08x    0x%08x   0x%08x   0x%08x \n",
                ((uint32)pNic_mBFDPBase), ((uint32)pNic_mBCDPIdx), value, ((uint32)pNic_mBRDPIdx));
    osal_printf("----------------------------------------------------------- \n");
    osal_printf("  (v)   0x%08x    0x%08x   0x%08x   0x%08x \n",
                (*(uint32 *)pNic_mBFDPBase), (*(uint32 *)pNic_mBCDPIdx), (*(uint32 *)value), (*(uint32 *)pNic_mBRDPIdx));
    osal_printf("\n");

    ioal_mem32_read(unit, RTL8389_CPU_INTERFACE_INTERRUPT_MASK_ADDR, &value);
    osal_printf("CPUIIMR[0xBB00312C] = 0x%08x ", value);
    osal_printf("[EN_PHD7_0=0x%x, ", (value & RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_PHD7_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_PHD7_0_OFFSET);
    osal_printf("EN_MBD=0x%x,\n", (value & RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_MBD_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_MBD_OFFSET);
    osal_printf("   EN_TX_DONE1_0=0x%x, ", (value & RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_TX_DONE1_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_TX_DONE1_0_OFFSET);
    osal_printf("EN_RX_DONE7_0=0x%x, ", (value & RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_RX_DONE7_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_RX_DONE7_0_OFFSET);
    osal_printf("EN_TX_ALLDONE1_0=0x%x]\n", (value & RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_TX_ALLDONE1_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_MASK_EN_TX_ALLDONE1_0_OFFSET);

    ioal_mem32_read(unit, RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_ADDR, &value);
    osal_printf("CPUIISR[0xBB003130] = 0x%08x ", value);
    osal_printf("[INT_PHDS7_0=0x%x, ", (value & RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_PHDS7_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_PHDS7_0_OFFSET);
    osal_printf("INT_MBDS=0x%x,\n", (value & RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_MBDS_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_MBDS_OFFSET);
    osal_printf("   INT_TX_DONE1_0=0x%x, ", (value & RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_TX_DONE1_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_TX_DONE1_0_OFFSET);
    osal_printf("INT_RX_DONE7_0=0x%x, ", (value & RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_RX_DONE7_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_RX_DONE7_0_OFFSET);
    osal_printf("INT_TX_ALL_DONE1_0=0x%x]\n", (value & RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_TX_ALL_DONE1_0_MASK) >> RTL8389_CPU_INTERFACE_INTERRUPT_STATUS_INT_TX_ALL_DONE1_0_OFFSET);

    ioal_mem32_read(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, &value);  
    osal_printf("CPUICR[0xBB003134] = 0x%08x ", value);
    osal_printf("[TX_CMD=0x%x, ", (value & RTL8389_CPU_INTERFACE_CONTROL_TX_CMD_MASK) >> RTL8389_CPU_INTERFACE_CONTROL_TX_CMD_OFFSET);
    osal_printf("RX_CMD=0x%x,\n", (value & RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_MASK) >> RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_OFFSET);
    osal_printf("   BURST_SIZE=0x%x, ", (value & RTL8389_CPU_INTERFACE_CONTROL_BURST_SIZE_MASK) >> RTL8389_CPU_INTERFACE_CONTROL_BURST_SIZE_OFFSET);
    osal_printf("TXFN=0x%x, ", (value & RTL8389_CPU_INTERFACE_CONTROL_TXFN_MASK) >> RTL8389_CPU_INTERFACE_CONTROL_TXFN_OFFSET);
    osal_printf("LENCRC=0x%x]\n", (value & RTL8389_CPU_INTERFACE_CONTROL_LENCRC_MASK) >> RTL8389_CPU_INTERFACE_CONTROL_LENCRC_OFFSET);
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8389_bufStatus_dump */

/* Function Name:
 *      r8389_pkthdrMbuf_dump
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
r8389_pkthdrMbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags)
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
            osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ph_mbuf);
            osal_printf("ring[%d]_pkthdr[%d]->ph_len = 0x%04x\n", i, j, (uint16)pRing_pkthdr->ph_len);
            osal_printf("ring[%d]_pkthdr[%d]->ph_reserve = 0x%04x\n", i, j, (uint16)pRing_pkthdr->ph_reserve);
            osal_printf("ring[%d]_pkthdr[%d]->ring_entry = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ring_entry);
            osal_printf("ring[%d]_pkthdr[%d]->tx_callback = 0x%08x\n", i, j, (uint32)pRing_pkthdr->tx_callback);
            osal_printf("ring[%d]_pkthdr[%d]->cookie = 0x%08x\n", i, j, (uint32)pRing_pkthdr->cookie);
            if (NULL != pRing_pkthdr->ph_mbuf)
            {
                osal_printf("------------------- its mbuf ----------------------\n");
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_pkthdr = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ph_mbuf->m_pkthdr);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_next = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ph_mbuf->m_next);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_len = 0x%04x\n", i, j, (uint16)pRing_pkthdr->ph_mbuf->m_len);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_extsize = 0x%04x\n", i, j, (uint16)pRing_pkthdr->ph_mbuf->m_extsize);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_data = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ph_mbuf->m_data);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_extbuf = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ph_mbuf->m_extbuf);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->packet = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ph_mbuf->packet);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->ring_entry = 0x%08x\n", i, j, (uint32)pRing_pkthdr->ph_mbuf->ring_entry);
                if ((pRing_pkthdr->ph_mbuf->packet != NULL) && (TRUE == flags))
                {
                    uint32  k;
                    uint32  dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */
                    uint32  pkt_len = pRing_pkthdr->ph_mbuf->m_len;
                    uint8   *pPkt_data = pRing_pkthdr->ph_mbuf->m_data;
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
        }
        osal_printf("###################################################\n");
    }
    NIC_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of r8389_pkthdrMbuf_dump */

/* Function Name:
 *      r8389_rxStatus_get
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
r8389_rxStatus_get(uint32 unit, uint32 *pStatus)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    NIC_SEM_LOCK(unit);
    ioal_mem32_field_read(unit, RTL8389_CPU_INTERFACE_CONTROL_ADDR, \
        RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_OFFSET, RTL8389_CPU_INTERFACE_CONTROL_RX_CMD_MASK, pStatus);
    NIC_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of r8389_rxStatus_get */

