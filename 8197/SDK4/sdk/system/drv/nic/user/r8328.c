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
 * $Revision: 37185 $
 * $Date: 2013-02-25 18:09:13 +0800 (Mon, 25 Feb 2013) $
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
#include <drv/nic/r8328.h>
#include <drv/swcore/rtl8328.h>
#include <rtk/default.h>





/*
 * Symbol Definition
 */
#define NIC_CODE_REDUCE

/* CPUIISR - CPU Interface Interrupt Status Register */
#define INT_PHCNTS7_INT_PHCNTS0_OFFSET         (21)                                         /* Interrupt of EN_PHCNTD status, write ????to clear.     */
#define INT_PHCNTS7_INT_PHCNTS0_MASK           (0xFFU << INT_PHCNTS7_INT_PHCNTS0_OFFSET)    /* Interrupt of EN_PHCNTD status, write ????to clear.     */
#define INT_PHDS7_INT_PHDS0_OFFSET             (13)                                         /* Interrupt of EN_PHD running out status, write '1' to clear. */
#define INT_PHDS7_INT_PHDS0_MASK               (0xFFU << INT_PHDS7_INT_PHDS0_OFFSET)        /* Interrupt of EN_PHD running out status, write '1' to clear. */
#define INT_MBDS_OFFSET                        (12)                                         /* Interrupt of EN_MBD running out status, write '1' to clear. */
#define INT_MBDS_MASK                          (0x1U << INT_MBDS_OFFSET)                    /* Interrupt of EN_MBD running out status, write '1' to clear. */
#define INT_TX_DONE1_INT_TX_DONE0_OFFSET       (10)                                         /* Interrupt of EN_TX_DONE pending status, write '1' to clear. */
#define INT_TX_DONE1_INT_TX_DONE0_MASK         (0x3U << INT_TX_DONE1_INT_TX_DONE0_OFFSET)   /* Interrupt of EN_TX_DONE pending status, write '1' to clear. */
#define INT_TX_DONE1_OFFSET                    (11)
#define INT_TX_DONE1_MASK                      (0x1U << INT_TX_DONE1_OFFSET)
#define INT_TX_DONE0_OFFSET                    (10)
#define INT_TX_DONE0_MASK                      (0x1U << INT_TX_DONE0_OFFSET)
#define INT_RX_DONE7_INT_RX_DONE0_OFFSET       (2)                                          /* Interrupt of EN_RX_DONE pending status, write '1' to clear. */
#define INT_RX_DONE7_INT_RX_DONE0_MASK         (0xFFU << INT_RX_DONE7_INT_RX_DONE0_OFFSET)  /* Interrupt of EN_RX_DONE pending status, write '1' to clear. */
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

//#define CPUIIMR_ENABLE_MASK                 (INT_PHCNTS7_INT_PHCNTS0_MASK | INT_PHDS7_INT_PHDS0_MASK | INT_MBDS_MASK | INT_TX_DONE1_INT_TX_DONE0_MASK | INT_RX_DONE7_INT_RX_DONE0_MASK | INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK)
#define CPUIIMR_ENABLE_MASK                 (INT_PHDS7_INT_PHDS0_MASK | INT_MBDS_MASK | INT_TX_DONE1_INT_TX_DONE0_MASK | INT_RX_DONE7_INT_RX_DONE0_MASK | INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK)
#define CPUIIMR_DISABLE_MASK                (0x00000000U)
#define CPUIISR_CLEARALL_MASK               (0xFFFFFFFFU)
#define CPUICR_DISABLE_MASK                 (0x00000000U)

/* Platform dependence */
#define	SWNIC_USE_SRAM
#undef	SWNIC_USE_STATIC
#undef	SWNIC_USE_DYNAMIC

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
#define SRAM_SEG_ADDR       (0x12000000)
#define SRAM_DISABLE        (0)
#define SRAM_ENABLE         (1)

/* SRAM memory convert function */
#define SRAM_ADDR_KRN2USR(unit, krn_addr)    (((uint32)krn_addr - (uint32)RTL8328_SRAM_VIRT_BASE) + sram_base[unit])
#define SRAM_ADDR_USR2KRN(unit, usr_addr)    (((uint32)usr_addr - sram_base[unit]) + (uint32)RTL8328_SRAM_VIRT_BASE)

/* DMA memory convert function */
#define DMA_ADDR_KRN2USR(krn_addr)    (((uint32)krn_addr - (uint32)KRNVIRT(RTL8328_DMA_PHYS_BASE)) + dma_base)
#define DMA_ADDR_USR2KRN(usr_addr)    (((uint32)usr_addr - dma_base) + (uint32)KRNVIRT(RTL8328_DMA_PHYS_BASE))

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
    esw_nic_pkthdr_t    rxPktHdr[NIC_RX_PKTHDR_NUM];
    esw_nic_pkthdr_t    txPktHdr[NIC_TX_PKTHDR_NUM];
    esw_nic_mbuf_t      rxMbuf[NIC_RX_MBUF_NUM];
    esw_nic_mbuf_t      txMbuf[NIC_TX_MBUF_NUM];
} nic_info_t;


/*
 * Data Declaration
 */
#if defined(SWNIC_USE_STATIC)
static nic_info_t swNic_Ram;    /* Physical memory for swNic */
#endif
static uint32   nic_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static nic_rx_cb_entry_t _nic_rx_cb_tbl[NIC_RX_CB_PRIORITY_NUMBER];
drv_nic_initCfg_t _nic_init_conf;
static uint8 svid_tagif[16] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0 };
static uint8 cvid_tagif[16] = { 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0 };
static osal_mutex_t     nic_sem[RTK_MAX_NUM_OF_UNIT];

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
static uint32       nic_debug_flag = DEBUG_TX_RAW_LEN_BIT | DEBUG_RX_RAW_LEN_BIT | DEBUG_RX_CPU_TAG_BIT | DEBUG_TX_CPU_TAG_BIT;
static uint32       nic_tx_success_cntr;
static uint32       nic_tx_failed_cntr;
static uint32       nic_rx_success_cntr;
static uint32       nic_rx_failed_cntr;
static uint32       nic_rx_lack_buf_cntr;
static uint32       _nic_rx_intr_cb_cnt = 0;

extern uint32       sram_base[RTK_MAX_NUM_OF_UNIT];
extern uint32       dma_base;

/*
 * Function Declaration
 */
static int32 _nic_init(uint32 unit, drv_nic_initCfg_t *pInitCfg);
static int32 _nic_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie);
static int32 _nic_rx_start(uint32 unit);
static int32 _nic_rx_stop(uint32 unit);
static int32 _nic_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags);
static int32 _nic_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb);
static int32 _insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, esw_nic_cpuTag_t *pCpuTag);
static int32 _removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, esw_nic_cpuTag_t *pCpuTag);


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
_insertCPUTag(uint8 *pPkt, uint32 headroom, uint8 **ppNewPkt, uint32 *pOffset, esw_nic_cpuTag_t *pCpuTag)
{
    uint32  temp;
    uint8   *pU8;

    if ((NULL == pPkt) || (NULL == ppNewPkt)|| (NULL == pOffset) || (NULL == pCpuTag))
    {
        return RT_ERR_FAILED;
    }

    if (headroom < (CPU_TAG_ID_LEN + sizeof(esw_nic_cpuTag_t)))
    {
        return RT_ERR_FAILED;
    }

    /* copy the packet */
    pU8 = (uint8 *)(pPkt - (CPU_TAG_ID_LEN + sizeof(esw_nic_cpuTag_t)));
    osal_memcpy(pU8, pPkt, 12);
    /*
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
    */

    ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8328_CPU_TAG_CONTROL_ADDR, \
        RTL8328_CPU_TAG_CONTROL_CPUPID_OFFSET, RTL8328_CPU_TAG_CONTROL_CPUPID_MASK, &temp);

    *(pU8 + 12) = ((temp >> 8) & 0xFF);
    *(pU8 + 13) = (temp & 0xFF);

    /* cpuTag insert: offset 14 bytes = DA(6) + SA(6)+ CPU_TAG_ID(2) */
    osal_memcpy((uint8 *)(pU8 + 14), (uint8 *)pCpuTag, sizeof(esw_nic_cpuTag_t));

    if (ppNewPkt)
    {
        *ppNewPkt = (uint8 *)pU8;
    }

    if (pOffset)
    {
        *pOffset = (CPU_TAG_ID_LEN + sizeof(esw_nic_cpuTag_t));
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
_removeCPUTag(uint8 *pPkt, uint8 **ppNewPkt, uint32 *pOffset, esw_nic_cpuTag_t *pCpuTag)
{
#if defined(__MODEL_USER__) || defined(__MODEL_KERNEL__)
    uint32  temp;
#endif

    if ((NULL == pPkt) || (NULL == ppNewPkt) || (NULL == pOffset) || (NULL == pCpuTag))
        return RT_ERR_FAILED;

#if defined(__MODEL_USER__) || defined(__MODEL_KERNEL__)
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, CPU_TAG_CONTROL_ADDR, &temp);
    if (0 == temp)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8328_CPU_TAG_CONTROL_ADDR, REALTEK_CPUTAG_ID << RTL8328_CPU_TAG_CONTROL_CPUPID_OFFSET);
    }
#endif

    /* cpuTag copy: offset 14 bytes = DA(6) + SA(6)+ CPU_TAG_ID(2) */
    osal_memcpy((uint8 *)pCpuTag, (uint8 *)(pPkt + 14), sizeof(esw_nic_cpuTag_t));

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
    *(pPkt + 13) = *(pPkt + 1);
    *(pPkt + 12) = *(pPkt + 0);

    if (ppNewPkt)
    {
        *ppNewPkt = (pPkt + (CPU_TAG_ID_LEN + sizeof(esw_nic_cpuTag_t)));
    }

    if (pOffset)
    {
        *pOffset = (CPU_TAG_ID_LEN + sizeof(esw_nic_cpuTag_t));
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
    uint32  i, temp;
    int32   ret = RT_ERR_FAILED;
    drv_nic_rx_t nic_rx_handle = NIC_RX_NOT_HANDLED;

    _RT_LOG(LOG_DEBUG, MOD_NIC, "ringId = %d", ringId);

    if (ringId >= NIC_RXRING_NUM)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "invalid ringId(%d)!", ringId);
        nic_rx_failed_cntr++;
        return RT_ERR_FAILED;
    }

    /* Update software current pointer */
//    if ((ret = reg_read(NIC_DEFAULT_UNIT_ID, ESW_CPURPDCR0r + ringId, &temp)) != RT_ERR_OK)
//    {
//        nic_rx_failed_cntr++;
//        return ret;
//    }
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8328_CPURPDCR0_ADDR + (0x4 * ringId), &temp);
    
    pNic_rxCDPIdx[ringId] = (uint32 *)(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, temp));    /* The limitation */

    do
    {
        uint8               handled = FALSE;
        uint8               reclaim_mbuf = TRUE;
        uint32              offset = 0;
        esw_nic_mbuf_t      *pMbuf;
        esw_nic_pkthdr_t    *pktHdr;
        esw_nic_cpuTag_t    cputag;
        drv_nic_pkt_t       *pPacket;
        nic_rx_queue_t      **ppRx_queue;
        nic_rx_queue_t      *pRx_queue = NULL;
        uint32              tagData;

        ppRx_queue = &pRx_queue;

        if ((*pNic_rxRDPIdx[ringId] & osal_ntohl(NIC_RING_SWOWNBIT)) != 0)
            break;

        /* Prepare to be reclaim */
        pktHdr = (esw_nic_pkthdr_t *)((osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, *pNic_rxRDPIdx[ringId]))) & NIC_ADDR_MASK);
        pMbuf = (esw_nic_mbuf_t*)(osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf)));
        if (NULL == pktHdr || NULL == pMbuf || NULL == pMbuf->packet)
            break;

        /* NIC Rx debug message */
        if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
        {
            int i;
            int dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */

            osal_printf("=== [NIC RX Debug] ================================= Len: %d \n", osal_ntohs(pMbuf->m_len));
            for (i = 0; i < dump_len; i++)
            {
                if (i == osal_ntohs(pMbuf->m_len))
                    break;
                if (0 == (i % 16))
                    osal_printf("[%04X] ", i);
                osal_printf("%02X ", *(pMbuf->packet->data + i));
                if (15 == (i % 16))
                    osal_printf("\n");
            }
            osal_printf("\n");
        }

        pPacket = pMbuf->packet;
        pPacket->length = osal_ntohs(pMbuf->m_len);
        pPacket->tail = pPacket->data + pPacket->length;

        ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8328_CPUICR_ADDR, \
            RTL8328_CPUICR_PH_AUTOTAG_OFFSET, RTL8328_CPUICR_PH_AUTOTAG_MASK, &temp);

        if (1 == pktHdr->ph_tag)
        {
            tagData = osal_ntohl(pktHdr->ph_tag0);
            pPacket->rx_tag.source_port     = (tagData >> 3) & 0x1F;
            pPacket->rx_tag.priority        = tagData & 0x7;

            tagData = osal_ntohl(pktHdr->ph_tag1);
            pPacket->rx_tag.reason          = tagData & 0xFFFF;
            pPacket->rx_tag.l2_error        = (tagData >> 18) & 0x1;
            pPacket->rx_tag.l3_error        = (tagData >> 17) & 0x1;
            pPacket->rx_tag.l4_error        = (tagData >> 16) & 0x1;
            pPacket->rx_tag.pppoe           = (tagData >> 19) & 0x1;
            pPacket->rx_tag.drop_precedence = 0;    /* unsupported */
            pPacket->rx_tag.l2_format       = (tagData >> 28) & 0xF;
            pPacket->rx_tag.l3_l4_format    = (tagData >> 24) & 0xF;
            pPacket->rx_tag.extra_tag       = (tagData >> 22) & 0x1;
            pPacket->rx_tag.svid_tagged     = svid_tagif[pPacket->rx_tag.l2_format];
            pPacket->rx_tag.cvid_tagged     = cvid_tagif[pPacket->rx_tag.l2_format];

            tagData = osal_ntohl(pktHdr->ph_tag2);
            pPacket->rx_tag.inner_pri       = (tagData >> 13) & 0x7;
            pPacket->rx_tag.inner_vid       = tagData & 0xFFF;
            pPacket->rx_tag.outer_pri       = (tagData >> 29) & 0x7;
            pPacket->rx_tag.outer_vid       = (tagData >> 16) & 0xFFF;

            /* NIC Rx debug message */
            if (nic_debug_flag & DEBUG_RX_CPU_TAG_BIT)
            {
                osal_printf("=== [NIC RX Debug - CPU Rx Tag Information] ============ \n");
                osal_printf("pktHdr->ph_tag0 : 0x%x \n", pktHdr->ph_tag0);
                osal_printf("pktHdr->ph_tag1 : 0x%x \n", pktHdr->ph_tag1);
                osal_printf("pktHdr->ph_tag2 : 0x%x \n", pktHdr->ph_tag2);
                osal_printf(" SPHY : %d \n", pPacket->rx_tag.source_port);
                osal_printf(" PRI : %d \n", pPacket->rx_tag.priority);
                osal_printf(" L2FMT : %d \n", pPacket->rx_tag.l2_format);
                osal_printf(" L34FMT : %d \n", pPacket->rx_tag.l3_l4_format);
                osal_printf(" EXTRATAG : %d \n", pPacket->rx_tag.extra_tag);
                osal_printf(" PPPoE : %d \n", pPacket->rx_tag.pppoe);
                osal_printf(" L2ERR : %d \n", pPacket->rx_tag.l2_error);
                osal_printf(" L3ERR : %d \n", pPacket->rx_tag.l3_error);
                osal_printf(" L4ERR : %d \n", pPacket->rx_tag.l4_error);
                osal_printf(" REASON : %d \n", pPacket->rx_tag.reason);
                osal_printf(" OPRI : %d \n", pPacket->rx_tag.outer_pri);
                osal_printf(" OVID : %d \n", pPacket->rx_tag.outer_vid);
                osal_printf(" IPRI : %d \n", pPacket->rx_tag.inner_pri);
                osal_printf(" IVID : %d \n", pPacket->rx_tag.inner_vid);
            }
        }
        else if ((0 == pktHdr->ph_tag) && (0 == temp))
        {
            uint16  ethtype = 0;

            ethtype = osal_ntohs(*(uint16 *) (pPacket->data + 12));
            RT_LOG(LOG_DEBUG, MOD_NIC, "ethtype ; 0x%x\n", ethtype);
        
            ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8328_CPU_TAG_CONTROL_ADDR, \
                RTL8328_CPU_TAG_CONTROL_CPUPID_OFFSET, RTL8328_CPU_TAG_CONTROL_CPUPID_MASK, &temp);
        
            if ((ethtype == (uint16)temp) && (*(pPacket->data + 14) == REALTEK_CPUTAG_PROTOCOL_ID))
            {
                /* Check and remove the CPU Rx tag from the packet data buffer if exist */
                if (RT_ERR_OK != _removeCPUTag(pPacket->data, &pPacket->data, &offset, &cputag))
                {
                    nic_rx_failed_cntr++;
                    return RT_ERR_FAILED;
                }

                /*after remove cpu tag*/
                if (nic_debug_flag & DEBUG_RX_RAW_LEN_BIT)
                {
                    int i;
                    int dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */

                    osal_printf("=== [NIC RX Debug] ================================= Len: %d \n", osal_ntohs(pMbuf->m_len));
                    for (i = 0; i < dump_len; i++)
                    {
                        if (i == (osal_ntohs(pMbuf->m_len) - 12))
                            break;
                        if (0 == (i % 16))
                            osal_printf("[%04X] ", i);
                        osal_printf("%02X ", *(pMbuf->packet->data + i));
                        if (15 == (i % 16))
                            osal_printf("\n");
                    }
                    osal_printf("\n");
                }
                
                pPacket->rx_tag.source_port     = cputag.un.rx.SPHY;
                pPacket->rx_tag.priority        = cputag.un.rx.PRI;
                pPacket->rx_tag.reason          = osal_ntohs(cputag.un.rx.REASON);
                pPacket->rx_tag.l2_error        = cputag.un.rx.L2ERR;
                pPacket->rx_tag.l3_error        = cputag.un.rx.L3ERR;
                pPacket->rx_tag.l4_error        = cputag.un.rx.L4ERR;
                pPacket->rx_tag.pppoe           = cputag.un.rx.PPPoE;
                pPacket->rx_tag.drop_precedence = 0;    /* unsupported */
                pPacket->rx_tag.l2_format       = cputag.un.rx.L2FMT;
                pPacket->rx_tag.l3_l4_format    = cputag.un.rx.L34FMT;
                pPacket->rx_tag.extra_tag       = cputag.un.rx.EXTRATAG;
                pPacket->rx_tag.svid_tagged     = svid_tagif[pPacket->rx_tag.l2_format];
                pPacket->rx_tag.cvid_tagged     = cvid_tagif[pPacket->rx_tag.l2_format];

                pPacket->rx_tag.inner_pri       = cputag.un.rx.IPRI;
                pPacket->rx_tag.inner_vid       = osal_ntohs(cputag.un.rx.IVID);
                pPacket->rx_tag.outer_pri       = cputag.un.rx.OPRI;
                pPacket->rx_tag.outer_vid       = osal_ntohs(cputag.un.rx.OVID);
                pPacket->length -= offset;

                /* NIC Rx debug message */
                if (nic_debug_flag & DEBUG_RX_CPU_TAG_BIT)
                {
                    osal_printf("=== [NIC RX Debug - CPU Rx Tag Information] ============ \n");
                    osal_printf(" SPHY : %d \n", pPacket->rx_tag.source_port);
                    osal_printf(" PRI : %d \n", pPacket->rx_tag.priority);
                    osal_printf(" L2FMT : %d \n", pPacket->rx_tag.l2_format);
                    osal_printf(" L34FMT : %d \n", pPacket->rx_tag.l3_l4_format);
                    osal_printf(" EXTRATAG : %d \n", pPacket->rx_tag.extra_tag);
                    osal_printf(" PPPoE : %d \n", pPacket->rx_tag.pppoe);
                    osal_printf(" L2ERR : %d \n", pPacket->rx_tag.l2_error);
                    osal_printf(" L3ERR : %d \n", pPacket->rx_tag.l3_error);
                    osal_printf(" L4ERR : %d \n", pPacket->rx_tag.l4_error);
                    osal_printf(" REASON : %d \n", pPacket->rx_tag.reason);
                    osal_printf(" OPRI : %d \n", pPacket->rx_tag.outer_pri);
                    osal_printf(" OVID : %d \n", pPacket->rx_tag.outer_vid);
                    osal_printf(" IPRI : %d \n", pPacket->rx_tag.inner_pri);
                    osal_printf(" IVID : %d \n", pPacket->rx_tag.inner_vid);
                }
            } /* end of 'if (ethtype == (uint16)temp)' */
            else
            {
                /* NIC Rx debug message */
                if (nic_debug_flag & DEBUG_RX_CPU_TAG_BIT)
                {
                    osal_printf("=== [NIC RX Debug - CPU Rx Tag Information] ============ \n");
                    osal_printf(" CPU tag is non-exist\n");
                }
            }
        } /* end of 'if ((0 == pktHdr->ph_tag) && (0 == temp))' */
        else
        {
            /* NIC Rx debug message */
            if (nic_debug_flag & DEBUG_RX_CPU_TAG_BIT)
            {
                osal_printf("=== [NIC RX Debug - CPU Rx Tag Information] ============ \n");
                osal_printf(" CPU tag is non-exist\n");
            }
        }
 
        nic_rx_handle = NIC_RX_NOT_HANDLED;
        /* Process interrupt callback function */
        if (0 != _nic_rx_intr_cb_cnt)
        {    
            for (i = 0; i < NIC_RX_CB_PRIORITY_NUMBER; i++)
            {
                if (_nic_rx_cb_tbl[i].rx_callback != NULL)
                {
                    nic_rx_handle = _nic_rx_cb_tbl[i].rx_callback(NIC_DEFAULT_UNIT_ID, pMbuf->packet, _nic_rx_cb_tbl[i].pCookie);
                }
                switch (nic_rx_handle)
                {
                    case NIC_RX_NOT_HANDLED:
                        break;
                    case NIC_RX_HANDLED:
                        break;
                    case NIC_RX_HANDLED_OWNED:
                        handled = TRUE;
                        pMbuf->packet = NULL;
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

        if (FALSE == handled) /* Process non-interrupt callback function */
        {          
            nic_rx_queueInfo_get(NIC_DEFAULT_UNIT_ID, ringId, ppRx_queue);            
            if ((pRx_queue->drop_thresh > 0) && (pRx_queue->count < pRx_queue->drop_thresh))
            {
                if ((ret = nic_rx_pkt_enqueue(ringId, pPacket)) == RT_ERR_OK)
                {                
                    pMbuf->packet = NULL;
                    /* Notify RX thread */
                    nic_rx_thread_notify(NIC_DEFAULT_UNIT_ID);
                    nic_rx_handle = NIC_RX_HANDLED_OWNED;
                }
                else
                {
                    RT_LOG(LOG_WARNING, (MOD_NIC), "RX queue %d is full!",ringId);
                    nic_rx_lack_buf_cntr++;
                }
            }
            else
            {
                RT_LOG(LOG_WARNING, (MOD_NIC), "RX too fast, directly drop\n");   
                nic_rx_lack_buf_cntr++;           
            }
        }
       
        if (NULL == pMbuf->packet)
        {
            /* Alloc a new packet data buffer */
            if ((ret = _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket)) == RT_ERR_OK)
            {
                pMbuf->packet = pPacket;
                pMbuf->m_extbuf = (uint8*)osal_ntohl((uint8 *)(DMA_ADDR_USR2KRN(pPacket->head)));
                pMbuf->m_extsize = osal_ntohs(pPacket->end - pPacket->head);
                pMbuf->m_data = (uint8*)osal_ntohl((uint8 *)(DMA_ADDR_USR2KRN(pPacket->data)));
                pMbuf->m_len = 0;
                if ((ret = osal_cache_memory_flush((uint32)(DMA_ADDR_USR2KRN(pPacket->head)), (pPacket->end - pPacket->head))) != RT_ERR_OK)
                {
                    return ret;
                }
            }
            else
            {      
                RT_DBG(LOG_WARNING, (MOD_NIC|MOD_DAL), "Out of memory ! (alloc a new packet data buffer failed)");
                reclaim_mbuf = FALSE;
            }
        }
        else
        {
            pPacket->data -= offset;
            if ((ret = osal_cache_memory_flush((uint32)(DMA_ADDR_USR2KRN(pPacket->head)), (pPacket->end - pPacket->head))) != RT_ERR_OK)
            {
                return ret;
            }
        }
        /* Reclaim pkthdr/mbuf descriptor */
        if(TRUE == reclaim_mbuf)
        { 
            MEMORY_BARRIER();
            *(pMbuf->ring_entry) |= osal_ntohl(NIC_RING_SWOWNBIT);    /* Only set the SwOwn bit */
    
#ifndef NIC_CODE_REDUCE
            /* To guarantee it's write done */
            do
            {
                uint32 chk;
                chk = *(pMbuf->ring_entry);
            } while (0);
#endif
        }

        pMbuf->m_pkthdr = NULL;
        pMbuf = NULL;
        pktHdr->ph_len = 0;
        MEMORY_BARRIER();
        *(pktHdr->ring_entry) |= osal_ntohl(NIC_RING_SWOWNBIT);  /* Only set the SwOwn bit */

        /* When CPU has received a packet, it should set the CPURXPKT bit in the ring to 
         * inform ASIC that can send one more packet to descriptor.
         */
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8328_CPURXPKT_ADDR, 0x1 << ringId);

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
        esw_nic_pkthdr_t *pktHdr;

        if ((*pNic_txRDPIdx[ringId] & osal_ntohl(NIC_RING_SWOWNBIT)) != 0)
            break;

        /* Prepare to be reclaim */
        pktHdr = (esw_nic_pkthdr_t *)((osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, *pNic_txRDPIdx[ringId]))) & NIC_ADDR_MASK);
        if (NULL == pktHdr || NULL == pktHdr->ph_mbuf
            || (NULL == pktHdr->tx_callback && NULL == ((esw_nic_mbuf_t *)SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf))->packet))
        {
            break;
        }

        /* Callback Tx CB Function (auto-free the abandoned packet) */
        if (pktHdr->tx_callback == NULL)
        {
            _nic_init_conf.pkt_free(NIC_DEFAULT_UNIT_ID, ((esw_nic_mbuf_t*)osal_ntohl(pktHdr->ph_mbuf))->packet);
        }
        else
        {
            pktHdr->tx_callback(NIC_DEFAULT_UNIT_ID, ((esw_nic_mbuf_t*)osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf)))->packet, pktHdr->cookie);
            pktHdr->tx_callback = NULL;
        }
        ((esw_nic_mbuf_t*)osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, pktHdr->ph_mbuf)))->packet = NULL;

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
//    if ((ret = reg_read(NIC_DEFAULT_UNIT_ID, ESW_CPURMDCRr, &temp)) != RT_ERR_OK)
//    {
//        return ret;
//    }
    ioal_mem32_read(NIC_DEFAULT_UNIT_ID, RTL8328_CPURMDCR_ADDR, &temp);
    
    pNic_mBCDPIdx = (uint32 *)SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, temp);

    do
    {
        esw_nic_mbuf_t *pMBuf;

        if ((*pNic_mBRDPIdx & osal_ntohl(NIC_RING_SWOWNBIT)) != 0)
            break;

        /* Prepare to be reclaim */
        pMBuf = (esw_nic_mbuf_t *)((osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, *pNic_mBRDPIdx))) & NIC_ADDR_MASK);
        if (NULL == pMBuf || pMBuf->m_pkthdr != NULL)
            break;

        /* Alloc a new packet data buffer */
        if (RT_ERR_OK != _nic_init_conf.pkt_alloc(NIC_DEFAULT_UNIT_ID, _nic_init_conf.pkt_size, 0, &pPacket))
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "Out of memory ! (alloc a new packet data buffer failed)");
            break;
        }

        pMBuf->packet = pPacket;
        pMBuf->m_extbuf = (uint8*)osal_ntohl((uint8 *)(DMA_ADDR_USR2KRN(pPacket->head)));
        pMBuf->m_extsize = osal_ntohs(pPacket->end - pPacket->head);
        pMBuf->m_data = (uint8*)osal_ntohl((uint8 *)(DMA_ADDR_USR2KRN(pPacket->data)));
        pMBuf->m_len = 0;
        if ((ret = osal_cache_memory_flush((uint32)DMA_ADDR_USR2KRN(pPacket->head), (pPacket->end - pPacket->head))) != RT_ERR_OK)
        {
            return ret;
        }

        MEMORY_BARRIER();
        *(pMBuf->ring_entry) |= osal_ntohl(NIC_RING_SWOWNBIT);    /* Only set the SwOwn bit */

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


/* Function Name:
 *      r8328_nic_isr_handler
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
r8328_isr_handler(uint32 intr_status)
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
#if 0
    if (cpu_iisr & INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8328_CPUIISR_ADDR, INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK);
    }

    /* pktHdr Runout */
    if (cpu_iisr & INT_PHDS7_INT_PHDS0_MASK)
    {
        ioal_mem32_write(NIC_DEFAULT_UNIT_ID, RTL8328_CPUIISR_ADDR, INT_PHDS7_INT_PHDS0_MASK);
    }
#endif
    /* mBuffer Runout */
    if (cpu_iisr & INT_MBDS_MASK)
    {
        _nic_isr_mbRoutine();
    }

    /* need to check the process location and sequence */
    if (cpu_iisr & INT_PHCNTS7_INT_PHCNTS0_MASK)
    {
        /* no enable this right now, will enable late */
    }
    return RT_ERR_OK;
} /* end of r8328_isr_handler */

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
    uint32  temp, base;
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
    nic_rx_lack_buf_cntr = 0;

    _nic_init_conf.pkt_size  = pInitCfg->pkt_size;
    _nic_init_conf.pkt_alloc = pInitCfg->pkt_alloc;
    _nic_init_conf.pkt_free  = pInitCfg->pkt_free;

#if 0
    /* Reset NIC only */
    if ((ret = reg_field_write(unit, ESW_PORT_28_MAC_CONTROLr, ESW_EN_TXRX_P28f, 0)) != RT_ERR_OK)
    {
        return ret;
    }
    osal_time_usleep(50 * 1000); /* delay 50mS */
    RT_LOG(LOG_DEBUG, (MOD_NIC|MOD_DAL), "Reset NIC (CPU Interface)... ");
#endif
    
    RT_LOG(LOG_DEBUG, MOD_NIC, "Reset NIC (R8328)... ");
    
    ioal_mem32_field_write(unit, RTL8328_RESET_GLOBAL_CONTROL_ADDR, \
        RTL8328_RESET_GLOBAL_CONTROL_NIC_RST_OFFSET, RTL8328_RESET_GLOBAL_CONTROL_NIC_RST_MASK, 0);

    do
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Wait ... ");
        ioal_mem32_field_read(unit, RTL8328_RESET_GLOBAL_CONTROL_ADDR, \
            RTL8328_RESET_GLOBAL_CONTROL_NIC_RST_OFFSET, RTL8328_RESET_GLOBAL_CONTROL_NIC_RST_MASK, &temp);
    } while (temp != 1);

    RT_LOG(LOG_DEBUG, MOD_NIC, "OK");

#if 0
    /* CPU port: Enable MAC Tx/Rx */
    if ((ret = reg_field_write(unit, ESW_PORT_28_MAC_CONTROLr, ESW_EN_TXRX_P28f, 3)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Set 25~28 Combo-Port + CPU port to join the Lookup Miss Flooding Portmask */
    if ((ret = reg_read(unit, ESW_LOOKUP_MISS_FLOODING_PORTMASKr, &temp)) != RT_ERR_OK)
    {
        return ret;
    }
    temp |= 0x1F000000;
    if ((ret = reg_write(unit, ESW_LOOKUP_MISS_FLOODING_PORTMASKr, temp)) != RT_ERR_OK)
    {
        return ret;
    }
#endif

#if 0
    /* CPU port: Enable MAC Tx/Rx */
    if ((ret = reg_field_write(unit, ESW_PORT_28_MAC_CONTROLr, ESW_EN_TXRX_P28f, 3)) != RT_ERR_OK)
    {
        return ret;
    }
#endif

    /* CPU port: Force link-up */
    ioal_mem32_field_write(unit, RTL8328_GE_PORT_PROPERTY_CONFIGURE_ADDR(28), \
        RTL8328_GE_PORT_PROPERTY_CONFIGURE_EN_FORCE_LINK_OFFSET, RTL8328_GE_PORT_PROPERTY_CONFIGURE_EN_FORCE_LINK_MASK, 1);

    /* Reset to default value */
    ioal_mem32_write(unit, RTL8328_CPUIIMR_ADDR, CPUIIMR_DISABLE_MASK);  
    ioal_mem32_write(unit, RTL8328_CPUIISR_ADDR, CPUIISR_CLEARALL_MASK);  
    ioal_mem32_write(unit, RTL8328_CPUICR_ADDR, CPUICR_DISABLE_MASK);  

#ifdef SWNIC_USE_SRAM	/* USE SRAM */
    /* SRAM MAP Setting - Pointer to NIC info memory */
    /* physical address: 0x1000_0000 --> virtual memory 0xB000_0000 */
    /* USE SRAM: 0xB000_0000 ~ 0xB000_7FFF (32KB)    */
    RT_ERR_CHK(ioal_init_memRegion_get(unit, IOAL_MEM_SOC, &base), ret);    
    MEM32_WRITE(base+SRAMSBR0_OFFSET, SRAM_SEG_BASE); /* SRAM segment base = 0x0000_0000 */
    MEM32_WRITE(base+SRAMSSR0_OFFSET, SRAM_SEG_SIZE_32KB); /* SRAM segment size = 32K bytes   */
    MEM32_WRITE(base+SRAMSAR0_OFFSET, (SRAM_SEG_ADDR | SRAM_ENABLE)); /* SRAM segment addr = 0x1000_0000 (0xB000_0000) */
    MEM32_WRITE(base+UMSSR0_OFFSET, SRAM_SEG_SIZE_32KB);
    MEM32_WRITE(base+UMSAR0_OFFSET, (SRAM_SEG_ADDR | SRAM_ENABLE));

    RT_ERR_CHK(ioal_init_memRegion_get(unit, IOAL_MEM_SRAM, &base), ret);
    pNicInfo = (nic_info_t *)base;    /* MAP to virtual memory (UNCACHE) */
#elif defined(SWNIC_USE_STATIC)
    pNicInfo = &swNic_Ram;
#elif defined(SWNIC_USE_DYNAMIC)
    pNicInfo = (nic_info_t *) osal_alloc(sizeof(nic_info_t));
#else
    osal_printf("Allocate buffer mechanism don't select!\n");
    return RT_ERR_FAILED;
#endif	/* SWNIC_USE_SRAM */

    if (NULL == pNicInfo)
    {
        RT_LOG(LOG_MAJOR_ERR, MOD_NIC, "Error: Out of memory!");
        return RT_ERR_FAILED;
    }
    if (((uint32)pNicInfo & 0x3) != 0)
    {
        osal_printf("FATAL Error: pNicInfo(0x%08X) is NOT 4 Byte-Align!\n", (uint32)pNicInfo);
        return RT_ERR_FAILED;
    }
    RT_LOG(LOG_DEBUG, MOD_NIC, "pNicInfo: %p (size: %d)", pNicInfo, sizeof(nic_info_t));
    osal_memset(pNicInfo, 0, sizeof(nic_info_t));

    RT_ERR_CHK(osal_cache_memory_flush((uint32)RTL8328_SRAM_VIRT_BASE, sizeof(nic_info_t)), ret);   
    
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
            esw_nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->rxPktHdr[i];
            pPktHdr->ph_mbuf     = NULL;
            pPktHdr->ph_len      = 0;
            pPktHdr->ring_entry  = (pNic_rxFDPBase[j] + k);

            *(pNic_rxFDPBase[j] + k) = ((k + 1) == NIC_RXRING_SIZE)? \
                osal_ntohl((uint32)SRAM_ADDR_USR2KRN(unit, pPktHdr) | NIC_RING_WRAPBIT | NIC_RING_SWOWNBIT) : \
                osal_ntohl((uint32)SRAM_ADDR_USR2KRN(unit, pPktHdr) | NIC_RING_SWOWNBIT);

            RT_LOG(LOG_DEBUG, MOD_NIC, "RxRing[%01d][%01d] &0x%08X = 0x%08X", j, k, (uint32)(pNic_rxFDPBase[j] + k), osal_ntohl(*(pNic_rxFDPBase[j] + k)));
            i++;
        }
    }

    /* Tx pktHdrs */
    i = 0;
    for (j = 0; j < NIC_TXRING_NUM; j++)
    {
        for (k = 0; k < NIC_TXRING_SIZE; k++)
        {
            esw_nic_pkthdr_t *pPktHdr;

            pPktHdr = &pNicInfo->txPktHdr[i];
            pPktHdr->ph_mbuf     = NULL;
            pPktHdr->ph_len      = 0;
            pPktHdr->ring_entry  = (pNic_txFDPBase[j] + k);

            *(pNic_txFDPBase[j] + k) = ((k + 1) == NIC_TXRING_SIZE)? \
                osal_ntohl((uint32)SRAM_ADDR_USR2KRN(unit, pPktHdr) | NIC_RING_WRAPBIT) : \
                osal_ntohl((uint32)SRAM_ADDR_USR2KRN(unit, pPktHdr));

            RT_LOG(LOG_DEBUG, MOD_NIC, "TxRing[%01d][%01d] &0x%08X = 0x%08X", j, k, (uint32)(pNic_txFDPBase[j] + k), osal_ntohl(*(pNic_txFDPBase[j] + k)));
            i++;
        }
    }

    /* Rx mBuf */
    for (i = 0; i < NIC_MBRING_SIZE; i++)
    {
        esw_nic_mbuf_t *pMBuf;

        if (i >= NIC_RX_MBUF_NUM)
        {
            RT_LOG(LOG_DEBUG, MOD_NIC, "The Rx mbuf number is not enough to hang on mBuf ring!");
            break;
        }

        pMBuf = &pNicInfo->rxMbuf[i];
        pMBuf->ring_entry = (pNic_mBFDPBase + i);

        *(pNic_mBFDPBase + i) = ((i + 1) == NIC_MBRING_SIZE)? \
            osal_ntohl((uint32)SRAM_ADDR_USR2KRN(unit, pMBuf) | NIC_RING_WRAPBIT) : \
            osal_ntohl((uint32)SRAM_ADDR_USR2KRN(unit, pMBuf));

        RT_LOG(LOG_DEBUG, MOD_NIC, "MbRing[%02d] &0x%08X = 0x%08X", i, (uint32)(pNic_mBFDPBase + i), osal_ntohl(*(pNic_mBFDPBase + i)));
    }

    /* Tx mBuf */
    i = 0;
    for (j = 0; j < NIC_TXRING_NUM; j++)
    {
        for (k = 0; k < NIC_TXRING_SIZE; k++)
        {
            for (l = 0; l < NIC_TX_MBUF_MAX; l++)
            {
                esw_nic_mbuf_t *pMBuf;

                if (i >= NIC_TX_MBUF_NUM)
                {
                    RT_LOG(LOG_DEBUG, MOD_NIC, "The Tx mbuf number is not enough to hang on Tx ring!");
                    break;
                }

                pMBuf = &pNicInfo->txMbuf[i];
                pMBuf->ring_entry = (pNic_txFDPBase[j] + k);

                if (0 == l)
                {
                    esw_nic_pkthdr_t *pPktHdr;

                    pPktHdr = (esw_nic_pkthdr_t *)((osal_ntohl(SRAM_ADDR_KRN2USR(unit, pNicInfo->txFDPBase[j][k]))) & NIC_ADDR_MASK);
                    pPktHdr->ph_mbuf = (esw_nic_mbuf_t*)osal_ntohl((uint32)SRAM_ADDR_USR2KRN(unit, pMBuf));

                    RT_LOG(LOG_DEBUG, MOD_NIC, "tx pkt header[0x%08X] muf[0x%08X]", (uint32)pPktHdr, (uint32)pMBuf);
                }

                i++;
            }
        }
    }

#if 0
    /* Register NIC IRQ handler */
    if (RT_ERR_OK != osal_isr_register(RTK_DEV_NIC, _esw_nic_isr_handler, NULL))
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - Register NIC IRQ handler failed!");
        return RT_ERR_FAILED;
    }
#endif

    /* Setting Registers */
    ioal_mem32_write(unit, RTL8328_RXDSC0CNT_ADDR, NIC_RXRING_SIZE);
    ioal_mem32_write(unit, RTL8328_RXDSC1CNT_ADDR, NIC_RXRING_SIZE);
    ioal_mem32_write(unit, RTL8328_RXDSC2CNT_ADDR, NIC_RXRING_SIZE);
    ioal_mem32_write(unit, RTL8328_RXDSC3CNT_ADDR, NIC_RXRING_SIZE);
    ioal_mem32_write(unit, RTL8328_RXDSC4CNT_ADDR, NIC_RXRING_SIZE);
    ioal_mem32_write(unit, RTL8328_RXDSC5CNT_ADDR, NIC_RXRING_SIZE);
    ioal_mem32_write(unit, RTL8328_RXDSC6CNT_ADDR, NIC_RXRING_SIZE);
    ioal_mem32_write(unit, RTL8328_RXDSC7CNT_ADDR, NIC_RXRING_SIZE);

    for (i = 0; i < NIC_RXRING_NUM; i++)
        ioal_mem32_write(unit, RTL8328_CPURPDCR0_ADDR + (0x4 * i), (uint32)SRAM_ADDR_USR2KRN(unit, pNic_rxFDPBase[i]));  

    for (i = 0; i < NIC_TXRING_NUM; i++)
        ioal_mem32_write(unit, RTL8328_CPUTPDCR0_ADDR + (0x4 * i), (uint32)SRAM_ADDR_USR2KRN(unit, pNic_txFDPBase[i]));  
    
    ioal_mem32_write(unit, RTL8328_CPURMDCR_ADDR, (uint32)SRAM_ADDR_USR2KRN(unit, pNic_mBFDPBase));  
    ioal_mem32_write(unit, RTL8328_CPUIIMR_ADDR, CPUIIMR_ENABLE_MASK);  

    ioal_mem32_field_write(unit, RTL8328_CPUICR_ADDR, \
        RTL8328_CPUICR_LENCRC_OFFSET, RTL8328_CPUICR_LENCRC_MASK, 1);

    ioal_mem32_field_write(unit, RTL8328_CPUICR_ADDR, \
        RTL8328_CPUICR_TX_CMD_OFFSET, RTL8328_CPUICR_TX_CMD_MASK, 1);

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
    uint32  packetSize;
    int32   ret = RT_ERR_FAILED;
    esw_nic_pkthdr_t    *pPktHdr;
    esw_nic_mbuf_t      *pMBuf;
    esw_nic_cpuTag_t    cputag;

    if (NULL == pPacket)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "Error - pPacket is NULL!");
        return RT_ERR_FAILED;
    }

    /* Step: Decide the target queue */
    if (pPacket->as_txtag)
        txRingId = (pPacket->tx_tag.priority > 3) ? 1 : 0;    /* mapping 8 priority to 2 queues */
    else
        txRingId = 0;

    /* Step: Find a pktHdr */
    if ((SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, *pNic_txCDPIdx[txRingId]) & osal_ntohl(NIC_RING_SWOWNBIT)) != 0)
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

    pPktHdr = (esw_nic_pkthdr_t *)((osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, *pNic_txCDPIdx[txRingId]))) & NIC_ADDR_MASK);

    RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr : 0x%p", pPktHdr);
    /* Step: Double Confirm (Check the pktHdr status) */
    if (NULL == pPktHdr || NULL == pPktHdr->ph_mbuf)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr is NOT available!");
        return RT_ERR_FAILED;
    }
    if (NULL != ((esw_nic_mbuf_t*)(osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, pPktHdr->ph_mbuf))))->packet)
    {
        RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr->ph_mbuf->packet is NOT available!");
        return RT_ERR_MBUF_PKT_NOT_AVAILABLE;
    }

    pPktHdr->ph_tag = 0;
    RT_LOG(LOG_DEBUG, MOD_NIC, "ph_mbuf : 0x%p", osal_ntohl(pPktHdr->ph_mbuf));

    /* Insert the CPU Tx tag into the packet data buffer */
    RT_LOG(LOG_DEBUG, MOD_NIC, "pPacket->as_txtag = %u", pPacket->as_txtag);
    if (pPacket->as_txtag)
    {
        uint32  offset;
        uint32  temp = 0;

        ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8328_CPUICR_ADDR, \
            RTL8328_CPUICR_PH_AUTOTAG_OFFSET, RTL8328_CPUICR_PH_AUTOTAG_MASK, &temp);

        if (0 == temp)
        { 
            /* software insert cpu tag to raw data */
            cputag.un.tx.PROTO      = REALTEK_CPUTAG_PROTOCOL_ID; /* CPU tag protocol id */
            
            cputag.un.tx.DPM_LOWEST = pPacket->tx_tag.dst_port_mask & 0xF;
            cputag.un.tx.DPM_LOW = (pPacket->tx_tag.dst_port_mask >> 4) & 0xFF;
            cputag.un.tx.DPM_HIGH = (pPacket->tx_tag.dst_port_mask >> 12) & 0xFF;
            cputag.un.tx.DPM_HIGHEST = (pPacket->tx_tag.dst_port_mask >> 20) & 0xFF;
          
            cputag.un.tx.ASDPM      = pPacket->tx_tag.as_dst_port_mask;
            cputag.un.tx.ASP        = pPacket->tx_tag.as_priority;
            cputag.un.tx.ASPRMK     = pPacket->tx_tag.as_port_remark;
            cputag.un.tx.L2CALC     = pPacket->tx_tag.l2_recalculate;
            cputag.un.tx.L3CALC     = pPacket->tx_tag.l3_recalculate;
            cputag.un.tx.L4CALC     = pPacket->tx_tag.l4_recalculate;
            cputag.un.tx.DP         = pPacket->tx_tag.drop_precedence;
            cputag.un.tx.PRI        = pPacket->tx_tag.priority;
            cputag.un.tx.FLAG       = osal_ntohs(pPacket->tx_tag.flags);

            if(nic_debug_flag & DEBUG_TX_CPU_TAG_BIT)
            {
                RT_LOG(LOG_DEBUG, MOD_NIC, "cpu tag: ");
                for(temp = 0; temp < sizeof(esw_nic_cpuTag_t); temp++)
                    osal_printf("%02x ",*(((uint8*)&cputag) + temp));

                osal_printf("\n");
            }            

            /* Check the reserved headroom size have at least 12 bytes */
            if ((pPacket->data - pPacket->head) < (CPU_TAG_ID_LEN + sizeof(esw_nic_cpuTag_t)))
            {
                /* The reserved space is NOT enough */
                RT_LOG(LOG_DEBUG, MOD_NIC, "The reserved space is NOT enough");
                RT_LOG(LOG_DEBUG, MOD_NIC, "pPacket->as_txtag = %d", pPacket->as_txtag);
                RT_LOG(LOG_DEBUG, MOD_NIC, "(pPacket->data - pPacket->head) = %d", (pPacket->data - pPacket->head));
    
                return RT_ERR_FAILED;
            }
            
            if (RT_ERR_OK != _insertCPUTag(pPacket->data, (pPacket->data - pPacket->head), &pPacket->data, &offset, &cputag))
            {
                RT_LOG(LOG_DEBUG, MOD_NIC, "Error - CPU Tx tag insert failed!");
                
                return RT_ERR_FAILED;
            }
        
            pPacket->length += offset;
            pPktHdr->ph_tag = 0;
            pPktHdr->ph_len = osal_ntohs(pPacket->length);
            pPktHdr->ph_tag0 = 0;
            pPktHdr->ph_tag1 = 0;
            pPktHdr->ph_tag2 = 0;
        }
        else
        {
            uint32  ph_tag0, ph_tag1, ph_tag2;

            /* software update cpu tag to esw_nic_pkthdr_t and HW auto move to raw data */
            pPktHdr->ph_tag = 1;
            pPktHdr->ph_len = osal_ntohs(pPacket->length);

            /* fill packet header tag0 */
            ioal_mem32_field_read(NIC_DEFAULT_UNIT_ID, RTL8328_CPU_TAG_CONTROL_ADDR, \
                RTL8328_CPU_TAG_CONTROL_CPUPID_OFFSET, RTL8328_CPU_TAG_CONTROL_CPUPID_MASK, &temp);
            ph_tag0 = ((temp & 0xFFFF) << 16);
            ph_tag0 |= (REALTEK_CPUTAG_PROTOCOL_ID << 8);
            ph_tag0 |= ((pPacket->tx_tag.dst_port_mask >> 20) & 0xFF);
            pPktHdr->ph_tag0 = osal_ntohl(ph_tag0);

            /* fill packet header tag1 */
            ph_tag1 = ((pPacket->tx_tag.dst_port_mask & 0xFFFFF) << 12);
            ph_tag1 |= ((pPacket->tx_tag.as_dst_port_mask & 0x1) << 11);
            ph_tag1 |= ((pPacket->tx_tag.as_priority & 0x1) << 10);
            ph_tag1 |= ((pPacket->tx_tag.as_port_remark & 0x1) << 9);
            ph_tag1 |= ((pPacket->tx_tag.l2_recalculate & 0x1) << 8);
            ph_tag1 |= ((pPacket->tx_tag.l3_recalculate & 0x1) << 7);
            ph_tag1 |= ((pPacket->tx_tag.l4_recalculate & 0x1) << 6);
            ph_tag1 |= ((pPacket->tx_tag.drop_precedence & 0x3) << 4);
            ph_tag1 |= ((pPacket->tx_tag.priority & 0x7) << 1);
            pPktHdr->ph_tag1 = osal_ntohl(ph_tag1);

            /* fill packet header tag2 */
            ph_tag2 = ((pPacket->tx_tag.flags & 0xFFFF) << 16);            
            pPktHdr->ph_tag2 = osal_ntohl(ph_tag2);

            RT_LOG(LOG_DEBUG, MOD_NIC, "pPktHdr->ph_tag: [0x%08x] [0x%08x] [0x%08x]", pPktHdr->ph_tag0,
                         pPktHdr->ph_tag1, pPktHdr->ph_tag2);

            if (nic_debug_flag & DEBUG_TX_CPU_TAG_BIT)
            {
                /* Fill this for debugging */
                cputag.un.tx.PROTO      = REALTEK_CPUTAG_PROTOCOL_ID; /* CPU tag protocol id */
                cputag.un.tx.DPM_LOWEST = pPacket->tx_tag.dst_port_mask & 0xF;
                cputag.un.tx.DPM_LOW = (pPacket->tx_tag.dst_port_mask >> 4) & 0xFF;
                cputag.un.tx.DPM_HIGH = (pPacket->tx_tag.dst_port_mask >> 12) & 0xFF;
                cputag.un.tx.DPM_HIGHEST = (pPacket->tx_tag.dst_port_mask >> 20) & 0xFF;        
                cputag.un.tx.ASDPM      = pPacket->tx_tag.as_dst_port_mask;
                cputag.un.tx.ASP        = pPacket->tx_tag.as_priority;
                cputag.un.tx.ASPRMK     = pPacket->tx_tag.as_port_remark;
                cputag.un.tx.L2CALC     = pPacket->tx_tag.l2_recalculate;
                cputag.un.tx.L3CALC     = pPacket->tx_tag.l3_recalculate;
                cputag.un.tx.L4CALC     = pPacket->tx_tag.l4_recalculate;
                cputag.un.tx.DP         = pPacket->tx_tag.drop_precedence;
                cputag.un.tx.PRI        = pPacket->tx_tag.priority;
                cputag.un.tx.FLAG       = pPacket->tx_tag.flags;
            }
        }
    }

    /* Step: Calc the pktbuf number */
    packetNum = 1;    /* Support single pktBuf now - one descriptor vs. one mbuf */
    packetSize = pPacket->length;

    pPktHdr->tx_callback = fTxCb;    /* Tx Callback function */
    pPktHdr->cookie = pCookie;
    pMBuf = (esw_nic_mbuf_t*)(osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, pPktHdr->ph_mbuf)));

    pMBuf->packet = pPacket;
    pMBuf->m_pkthdr = (esw_nic_pkthdr_t *)osal_ntohl(pPktHdr);
    pMBuf->m_extbuf = (uint8 *)osal_ntohl((uint8 *)(DMA_ADDR_USR2KRN(pPacket->head)));
    pMBuf->m_extsize = osal_ntohs(pPacket->end - pPacket->head);
    pMBuf->m_data = (uint8 *)osal_ntohl((uint8 *)(DMA_ADDR_USR2KRN(pPacket->data)));
    pMBuf->m_len = osal_ntohs(pPacket->length);
    
    if ((ret = osal_cache_memory_flush((uint32)DMA_ADDR_USR2KRN(pPacket->head), (pPacket->end - pPacket->head))) != RT_ERR_OK)
    {
        return ret;
    }
    

    MEMORY_BARRIER();
    *(pPktHdr->ring_entry) |= osal_ntohl(NIC_RING_SWOWNBIT);

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
            osal_printf("=== [NIC TX Debug] ================================= Len: %d + %d\n", osal_ntohs(pMBuf->m_len) - 8, 8);
        else
            osal_printf("=== [NIC TX Debug] ================================= Len: %d \n", osal_ntohs(pMBuf->m_len));

        for (i = 0; i < dump_len; i++)
        {
            if (i == (osal_ntohs(pMBuf->m_len)))
                break;
            if (0 == (i % 16))
                osal_printf("[%04X] ", i);
            osal_printf("%02X ", *((uint8*)(osal_ntohl(DMA_ADDR_KRN2USR(pMBuf->m_data))) + i));
            if (15 == (i % 16))
                osal_printf("\n");
        }
        osal_printf("\n");
    }

    /* Append the packet size to less 64 bytes */
    if (osal_ntohs(pMBuf->m_len) < 60)
    {
        pMBuf->m_len = osal_ntohs(60);
        pPktHdr->ph_len = osal_ntohs(60);
    }

    RT_LOG(LOG_DEBUG, MOD_NIC, "pMBuf->m_len[0x%x], pPktHdr->ph_len[0x%x]\n", pMBuf->m_len, pPktHdr->ph_len);

    if ((nic_debug_flag & DEBUG_TX_CPU_TAG_BIT) && pPacket->as_txtag)
    {
        osal_printf("=== [NIC TX Debug - CPU Tx Tag Information] ============ \n");
        osal_printf(" DPM : 0x%0x \n", (cputag.un.tx.DPM_HIGHEST << 20) | (cputag.un.tx.DPM_HIGH << 12)
                        |(cputag.un.tx.DPM_LOW << 4) | (cputag.un.tx.DPM_LOWEST & 0xF));
        osal_printf(" ASDPM : %d \n", cputag.un.tx.ASDPM);
        osal_printf(" ASP : %d \n", cputag.un.tx.ASP);
        osal_printf(" ASPRMK : %d \n", cputag.un.tx.ASPRMK);
        osal_printf(" L2CALC : %d \n", cputag.un.tx.L2CALC);
        osal_printf(" L3CALC : %d \n", cputag.un.tx.L3CALC);
        osal_printf(" L4CALC : %d \n", cputag.un.tx.L4CALC);
        osal_printf(" DP : %d \n", cputag.un.tx.DP);
        osal_printf(" PRI : %d \n", cputag.un.tx.PRI);
        osal_printf(" FLAG : 0x%x \n", cputag.un.tx.FLAG);
    }

    /* Set the TX Fetch Notify bit */
    ioal_mem32_field_write(unit, RTL8328_CPUICR_ADDR, \
        RTL8328_CPUICR_TXFN_OFFSET, RTL8328_CPUICR_TXFN_MASK, 1);

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
    ioal_mem32_field_write(unit, RTL8328_CPUICR_ADDR, \
        RTL8328_CPUICR_RX_CMD_OFFSET, RTL8328_CPUICR_RX_CMD_MASK, 1);

    RT_LOG(LOG_DEBUG, MOD_NIC, "NIC (R8328) Rx Start... ");

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
    ioal_mem32_field_write(unit, RTL8328_CPUICR_ADDR, \
        RTL8328_CPUICR_RX_CMD_OFFSET, RTL8328_CPUICR_RX_CMD_MASK, 0);
    
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


/* Function Name:
 *      r8328_init
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
r8328_init(uint32 unit, drv_nic_initCfg_t *pInitCfg)
{
    uint32  i;
    int32   ret = RT_ERR_FAILED;

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

    /* Configure CPU Tag in packet descriptor or in packet raw data
     * ENABLED: in packet descriptor
     * DISABLED: in packet raw data
     */
    ioal_mem32_field_write(unit, RTL8328_CPUICR_ADDR, \
        RTL8328_CPUICR_PH_AUTOTAG_OFFSET, RTL8328_CPUICR_PH_AUTOTAG_MASK, RTK_DEFAULT_NIC_CPU_TAG_IN_DESCRIPTOR);

    /* Configure CPU Tag parser ability in cpu port
     * ENABLED: have parser cpu tag ability.
     * DISABLED: no parser cpu tag ability.
     */
    ioal_mem32_field_write(unit, RTL8328_CPU_TAG_CONTROL_ADDR, \
        RTL8328_CPU_TAG_CONTROL_RXCPUTAG_OFFSET, RTL8328_CPU_TAG_CONTROL_RXCPUTAG_MASK, RTK_DEFAULT_CPU_TAG_PARSER_ABILITY_IN_CPU_PORT);
    
    /* Configure insert all packets leaving CPU port with CPU tag
     * ENABLED: insert to raw data.
     * DISABLED: no insert.
     */
    ioal_mem32_field_write(unit, RTL8328_CPU_TAG_CONTROL_ADDR, \
        RTL8328_CPU_TAG_CONTROL_INSCPUTAG_OFFSET, RTL8328_CPU_TAG_CONTROL_INSCPUTAG_MASK, RTK_DEFAULT_CPU_TAG_INSERT_IN_LEAVING_CPU_PORT_PKT);

    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_field_write(unit, RTL8328_RXDSC0CNT_ADDR + (0x4 * i), \
            RTL8328_RXDSC0CNT_RXDSCCNT_AVAIL0_OFFSET, RTL8328_RXDSC0CNT_RXDSCCNT_AVAIL0_MASK, NIC_RXRING_SIZE);
}

    /* set init flag to complete init */
    nic_init[unit] = INIT_COMPLETED;

    RT_LOG(LOG_DEBUG, MOD_NIC, "Init NIC R8328...OK");    

    return RT_ERR_OK;
} /* end of r8328_init */

/* Function Name:
 *      r8328_pkt_tx
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
r8328_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
{
    uint32  txRingId;
    uint32  tryCount = 5;
    int32   ret = RT_ERR_FAILED;

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
} /* end of r8328_pkt_tx */

/* Function Name:
 *      r8328_rx_start
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
r8328_rx_start(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    ret = nic_rx_thread_create(unit);

    /* Dispatch */
    ret = _nic_rx_start(unit);

    return ret;
} /* end of r8328_rx_start */


/* Function Name:
 *      r8328_rx_stop
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
r8328_rx_stop(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    ret = nic_rx_thread_destroy(unit);

    /* Dispatch */
    ret = _nic_rx_stop(unit);

    return ret;
} /* end of r8328_rx_stop */


/* Function Name:
 *      r8328_rx_register
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
r8328_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

    /* Dispatch */
    ret = _nic_rx_register(unit, priority, fRxCb, pCookie, flags);

    return ret;
} /* end of r8328_rx_register */

/* Function Name:
 *      r8328_rx_unregister
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
r8328_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb)
{
    int32 ret = RT_ERR_FAILED;

    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    /* Check arguments */
    RT_PARAM_CHK(priority > NIC_RX_CB_PRIORITY_MAX, RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK(NULL == fRxCb, RT_ERR_NULL_POINTER);

    /* Dispatch */
    ret = _nic_rx_unregister(unit, priority, fRxCb);

    return ret;
} /* end of r8328_rx_unregister */

/* Function Name:
 *      r8328_pkt_alloc
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
r8328_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket)
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
} /* end of r8328_pkt_alloc */

/* Function Name:
 *      r8328_pkt_free
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
r8328_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket)
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
} /* end of r8328_pkt_free */

/* NIC Tx/Rx debug */
/* Function Name:
 *      r8328_debug_set
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
r8328_debug_set(uint32 unit, uint32 flags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    nic_debug_flag = flags;

    return RT_ERR_OK;
} /* end of r8328_debug_set */

/* Function Name:
 *      r8328_debug_get
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
r8328_debug_get(uint32 unit, uint32 *pFlags)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    *pFlags = nic_debug_flag;

    return RT_ERR_OK;
} /* end of r8328_debug_get */

/* Function Name:
 *      r8328_counter_dump
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
r8328_counter_dump(uint32 unit)
{
    osal_printf("Tx success counter : %0d \n", nic_tx_success_cntr);
    osal_printf("Tx failed counter  : %0d \n", nic_tx_failed_cntr);
    osal_printf("Rx success counter : %0d \n", nic_rx_success_cntr);
    osal_printf("Rx failed counter  : %0d \n", nic_rx_failed_cntr);
    
    return RT_ERR_OK;
} /* end of r8328_counter_dump */

/* Function Name:
 *      r8328_counter_clear
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
r8328_counter_clear(uint32 unit)
{
    nic_tx_success_cntr = 0;
    nic_tx_failed_cntr = 0;
    nic_rx_success_cntr = 0;
    nic_rx_failed_cntr = 0;
    
    return RT_ERR_OK;
} /* end of r8328_counter_clear */


/* Function Name:
 *      r8328_bufStatus_dump
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
r8328_bufStatus_dump(uint32 unit)
{
    uint32  i, value;

    osal_printf("RXRING  SW_rxFDPBase  SW_RxCDPIdx  HW_RxCDPIdx  SW_RxRDPIdx \n");
    osal_printf("=========================================================== \n");
    for (i = 0; i < NIC_RXRING_NUM; i++)
    {
        ioal_mem32_read(unit, RTL8328_CPURPDCR0_ADDR + (0x4 * i), &value);

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
        ioal_mem32_read(unit, RTL8328_CPUTPDCR0_ADDR + (0x4 * i), &value);

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
    ioal_mem32_read(unit, RTL8328_CPURMDCR_ADDR, &value);
    value = SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, value);

    osal_printf("  (p)   0x%08x    0x%08x   0x%08x   0x%08x \n",
                ((uint32)pNic_mBFDPBase), ((uint32)pNic_mBCDPIdx), value, ((uint32)pNic_mBRDPIdx));
    osal_printf("----------------------------------------------------------- \n");
    osal_printf("  (v)   0x%08x    0x%08x   0x%08x   0x%08x \n",
                (*(uint32 *)pNic_mBFDPBase), (*(uint32 *)pNic_mBCDPIdx), (*(uint32 *)value), (*(uint32 *)pNic_mBRDPIdx));
    osal_printf("\n");

    ioal_mem32_read(unit, RTL8328_CPUIIMR_ADDR, &value);
    osal_printf("CPUIIMR[0xBB68002C] = 0x%08x ", value);
    osal_printf("[EN_PHCNT7_0=0x%x, ", (value & INT_PHCNTS7_INT_PHCNTS0_MASK) >> INT_PHCNTS7_INT_PHCNTS0_OFFSET);
    osal_printf("EN_PHD7_0=0x%x, ", (value & INT_PHDS7_INT_PHDS0_MASK) >> INT_PHDS7_INT_PHDS0_OFFSET);
    osal_printf("EN_MBD=0x%x,\n", (value & INT_MBDS_MASK) >> INT_MBDS_OFFSET);
    osal_printf("   EN_TX_DONE1_0=0x%x, ", (value & INT_TX_DONE1_INT_TX_DONE0_MASK) >> INT_TX_DONE1_INT_TX_DONE0_OFFSET);
    osal_printf("EN_RX_DONE7_0=0x%x, ", (value & INT_RX_DONE7_INT_RX_DONE0_MASK) >> INT_RX_DONE7_INT_RX_DONE0_OFFSET);
    osal_printf("EN_TX_ALLDONE1_0=0x%x]\n", (value & INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK) >> INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET);

    ioal_mem32_read(unit, RTL8328_CPUIISR_ADDR, &value);
    osal_printf("CPUIISR[0xBB680030] = 0x%08x ", value);
    osal_printf("[INT_PHCNTS7_0=0x%x, ", (value & INT_PHCNTS7_INT_PHCNTS0_MASK) >> INT_PHCNTS7_INT_PHCNTS0_OFFSET);
    osal_printf("INT_PHDS7_0=0x%x, ", (value & INT_PHDS7_INT_PHDS0_MASK) >> INT_PHDS7_INT_PHDS0_OFFSET);
    osal_printf("INT_MBDS=0x%x,\n", (value & INT_MBDS_MASK) >> INT_MBDS_OFFSET);
    osal_printf("   INT_TX_DONE1_0=0x%x, ", (value & INT_TX_DONE1_INT_TX_DONE0_MASK) >> INT_TX_DONE1_INT_TX_DONE0_OFFSET);
    osal_printf("INT_RX_DONE7_0=0x%x, ", (value & INT_RX_DONE7_INT_RX_DONE0_MASK) >> INT_RX_DONE7_INT_RX_DONE0_OFFSET);
    osal_printf("INT_TX_ALL_DONE1_0=0x%x]\n", (value & INT_TX_ALLDONE1_INT_TX_ALLDONE0_MASK) >> INT_TX_ALLDONE1_INT_TX_ALLDONE0_OFFSET);

    ioal_mem32_read(unit, RTL8328_CPUICR_ADDR, &value);  
    osal_printf("CPUICR[0xBB680034] = 0x%08x ", value);
    osal_printf("[TX_CMD=0x%x, ", (value & RTL8328_CPUICR_TX_CMD_MASK) >> RTL8328_CPUICR_TX_CMD_OFFSET);
    osal_printf("RX_CMD=0x%x,\n", (value & RTL8328_CPUICR_RX_CMD_MASK) >> RTL8328_CPUICR_RX_CMD_OFFSET);
    osal_printf("   BURST_SIZE=0x%x, ", (value & RTL8328_CPUICR_BURST_SIZE_MASK) >> RTL8328_CPUICR_BURST_SIZE_OFFSET);
    osal_printf("TXFN=0x%x, ", (value & RTL8328_CPUICR_TXFN_MASK) >> RTL8328_CPUICR_TXFN_OFFSET);
    osal_printf("LENCRC=0x%x]\n", (value & RTL8328_CPUICR_LENCRC_MASK) >> RTL8328_CPUICR_LENCRC_OFFSET);

    return RT_ERR_OK;
} /* end of r8328_bufStatus_dump */

/* Function Name:
 *      r8328_pkthdrMbuf_dump
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
r8328_pkthdrMbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags)
{
    uint32  i, j, ring_size;

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
            esw_nic_pkthdr_t    *pRing_pkthdr;
            esw_nic_mbuf_t      *pMbuf;   
            if (NIC_PKTHDR_MBUF_MODE_RX == mode)
                pRing_pkthdr = (esw_nic_pkthdr_t *)osal_ntohl((SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, *((uint32 *)(pNic_rxFDPBase[i]+j))) & NIC_ADDR_MASK));
            else
                pRing_pkthdr = (esw_nic_pkthdr_t *)osal_ntohl((SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, *((uint32 *)(pNic_txFDPBase[i]+j))) & NIC_ADDR_MASK));
            pMbuf = (esw_nic_mbuf_t*)osal_ntohl(SRAM_ADDR_KRN2USR(NIC_DEFAULT_UNIT_ID, pRing_pkthdr->ph_mbuf));
            osal_printf("###################################################\n");
            osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf = 0x%08x\n", i, j, osal_ntohl((uint32)pRing_pkthdr->ph_mbuf));
            osal_printf("ring[%d]_pkthdr[%d]->ph_len = 0x%04x\n", i, j, osal_ntohs((uint16)pRing_pkthdr->ph_len));
            osal_printf("ring[%d]_pkthdr[%d]->ph_reserve = 0x%04x\n", i, j, osal_ntohs((uint16)pRing_pkthdr->ph_reserve));
            osal_printf("ring[%d]_pkthdr[%d]->ring_entry = 0x%08x\n", i, j, osal_ntohl((uint32)pRing_pkthdr->ring_entry));
            osal_printf("ring[%d]_pkthdr[%d]->tx_callback = 0x%08x\n", i, j, (uint32)pRing_pkthdr->tx_callback);
            osal_printf("ring[%d]_pkthdr[%d]->cookie = 0x%08x\n", i, j, (uint32)pRing_pkthdr->cookie);
            if (NULL != pRing_pkthdr->ph_mbuf)
            {
                osal_printf("------------------- its mbuf ----------------------\n");
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_pkthdr = 0x%08x\n", i, j, osal_ntohl((uint32)pMbuf->m_pkthdr));
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_next = 0x%08x\n", i, j, osal_ntohl((uint32)pMbuf->m_next));
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_len = 0x%04x\n", i, j, osal_ntohs((uint16)pMbuf->m_len));
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_extsize = 0x%04x\n", i, j, osal_ntohs((uint16)pMbuf->m_extsize));
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_data = 0x%08x\n", i, j, osal_ntohl((uint32)pMbuf->m_data));
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->m_extbuf = 0x%08x\n", i, j, osal_ntohl((uint32)pMbuf->m_extbuf));
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->packet = 0x%08x\n", i, j, (uint32)pMbuf->packet);
                osal_printf("ring[%d]_pkthdr[%d]->ph_mbuf->ring_entry = 0x%08x\n", i, j, osal_ntohl((uint32)pMbuf->ring_entry));
                if ((pMbuf->packet != NULL) && (TRUE == flags))
                {
                    uint32  k;
                    uint32  dump_len = DEBUG_DUMP_PKT_LEN; /* debug dump maximum length */
                    uint32  pkt_len = osal_ntohs(pMbuf->m_len);
                    uint8   *pPkt_data = (uint8*)osal_ntohl(DMA_ADDR_KRN2USR(pMbuf->m_data));
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
    return RT_ERR_OK;
} /* end of r8328_pkthdrMbuf_dump */

/* Function Name:
 *      r8328_rxStatus_get
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
r8328_rxStatus_get(uint32 unit, uint32 *pStatus)
{
    /* Check init state */
    RT_INIT_CHK(nic_init[unit]);

    ioal_mem32_field_read(unit, RTL8328_CPUICR_ADDR, \
        RTL8328_CPUICR_RX_CMD_OFFSET, RTL8328_CPUICR_RX_CMD_MASK, pStatus);

    return RT_ERR_OK;
} /* end of r8328_rxStatus_get */
