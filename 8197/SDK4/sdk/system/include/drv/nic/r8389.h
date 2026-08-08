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
 * $Revision: 9222 $
 * $Date: 2010-04-27 11:12:01 +0800 (Tue, 27 Apr 2010) $
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
#ifndef __NIC_R8389_H__ 
#define __NIC_R8389_H__

/*  
 * Include Files 
 */
#include <common/error.h>
#include <drv/nic/common.h>
#include <drv/nic/diag.h>

/* 
 * Symbol Definition 
 */
#define NIC_RXRING_NUM          (8)
#define NIC_TXRING_NUM          (2)
#define NIC_RXRING_SIZE         (8)
#define NIC_TXRING_SIZE         (32)
#define NIC_RX_PKTHDR_NUM       (NIC_RXRING_NUM * NIC_RXRING_SIZE)
#define NIC_RX_MBUF_MAX         (1)   /* Not support multiple buffers */
#define NIC_RX_MBUF_NUM         ((NIC_RXRING_NUM * NIC_RXRING_SIZE) * NIC_RX_MBUF_MAX)
#define NIC_TX_PKTHDR_NUM       (NIC_TXRING_NUM * NIC_TXRING_SIZE)
#define NIC_TX_MBUF_MAX         (1)   /* Not support multiple buffers */
#define NIC_TX_MBUF_NUM         ((NIC_TXRING_NUM * NIC_TXRING_SIZE) * NIC_TX_MBUF_MAX)
#define NIC_MBRING_SIZE         (NIC_RX_MBUF_NUM)

#define NIC_RING_WRAPBIT        (0x1 << 1)
#define NIC_RING_SWOWNBIT       (0x1 << 0)
#define NIC_ADDR_MASK           (0xFFFFFFFC)

typedef struct nic_cpuTag_s
{
    union {
        struct {
            uint8   PROTO;      /* Define the 8389 CPU tag. Value is 0x4. */
            uint8   SPHY:5;     /* Source port number */
            uint8   PRI:3;      /* Assigned priority */
            uint16  REASON;     /* Reason to CPU */
            uint8   L2ERR:1;    /* Reserved for being compatible with RTL8316s. Always 0 here */
            uint8   L3ERR:1;    /* IPv4 checksum error */
            uint8   L4ERR:1;    /* TCP,UDP,ICMP,IGMP checksum error */
            uint8   PPPoE:1;    /* HSA.PPPoE */
            uint8   SEXIST:1;   /* HSA.STAGIF */
            uint8   CEXIST:1;   /* HSA.CTAGIF */
            uint8   DP:2;       /* Reserved for being compatible with RTL8316s. Always 0 here */
            uint8   L2FMT:4;    /* Reserved for being compatible with RTL8316s. Always 0 here */
            uint8   L34FMT:4;   /* Reserved for being compatible with RTL8316s. Always 0 here */
        } __attribute__ ((aligned(1), packed)) rx;
        struct {
            uint8   PROTO;      /* Define the 8389 CPU tag. Value is 0x4. */
            uint32  DPM:28;     /* When ADSPM is 1, DPM is th packet destination port mask. */
            uint32  ASDPM:1;    /* Assign the destination port directly. */
            uint32  ASP:1;      /* Assign priority directly. */
            uint32  ASPRMK:1;   /* When ASPRMK is 1, the packet will be remarked based on port remarking setting. */
            uint32  L2CALC:1;   /* Recalculate the layer2 CRC */
            uint8   L3CALC:1;   /* Recalculate the IPv4 checksum */
            uint8   L4CALC:1;   /* Recalculate the TCP,UDP,ICMP,IGMP checksum */
            uint8   DP:2;       /* Reserved for being compatible with RTL8316s. Always 0 here */
            uint8   PRI:3;      /* Assigned priority */
            uint8   L2LEARNING:1;   /* Decide whether proceed L2 learning */
        } __attribute__ ((aligned(1), packed)) tx;
    } un;
} nic_cpuTag_t;


typedef struct nic_pkthdr_s
{
    struct nic_mbuf_s *ph_mbuf;
#ifdef __LITTLE_ENDIAN
    uint16  ph_reserve;
    uint16  ph_len;
#else
    uint16  ph_len;
    uint16  ph_reserve;
#endif

    /* Used by Software */
    uint32  *ring_entry;
    drv_nic_tx_cb_f tx_callback;
    void    *cookie;
} nic_pkthdr_t;

typedef struct nic_mbuf_s
{
    struct nic_pkthdr_s *m_pkthdr;
    struct nic_mbuf_s *m_next;
#ifdef __LITTLE_ENDIAN
    uint16  m_extsize;
    uint16  m_len;
#else
    uint16  m_len;
    uint16  m_extsize;
#endif
    uint8   *m_data;
    uint8   *m_extbuf;

    /* Used by Software */
    struct drv_nic_pkt_s *packet;
    uint32  *ring_entry;
} nic_mbuf_t;

/*
 * Data Declaration 
 */

/*
 * Macro Definition
 */
 
 
/* 
 * Function Declaration 
 */

/* Module Name    : NIC     */

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
extern int32
r8389_init(uint32 unit, drv_nic_initCfg_t *pInitCfg);

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
extern int32
r8389_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie);

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
extern int32
r8389_rx_start(uint32 unit);

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
extern int32
r8389_rx_stop(uint32 unit);

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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
r8389_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags);

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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
r8389_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb);

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
extern int32
r8389_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket);

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
extern int32
r8389_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket);

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
extern int32
r8389_debug_set(uint32 unit, uint32 flags);

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
extern int32
r8389_debug_get(uint32 unit, uint32 *pFlags);

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
extern int32
r8389_counter_dump(uint32 unit);

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
extern int32
r8389_counter_clear(uint32 unit);

/* Function Name:
 *      r8389_bufStatus_dump
 * Description:
 *      Dump NIC buffer status of the specified device.
 * Input:
 *      unit   - unit id
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
extern int32
r8389_bufStatus_dump(uint32 unit);

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
extern int32
r8389_pkthdrMbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags);

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
extern int32
r8389_rxStatus_get(uint32 unit, uint32 *pStatus);

/* Function Name:
 *      r8389_isr_handler
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
extern int32 
r8389_isr_handler(uint32 intr_status);


#endif /* __NIC_R8389_H__ */

