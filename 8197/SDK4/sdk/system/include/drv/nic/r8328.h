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
#ifndef __NIC_R8328_H__ 
#define __NIC_R8328_H__

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

#define NIC_PKTHDR_MBUF_MODE_RX (0)
#define NIC_PKTHDR_MBUF_MODE_TX (1)

typedef struct esw_nic_cpuTag_s
{
    union {
        struct {
#ifdef CONFIG_SDK_ENDIAN_LITTLE
            uint8 PROTO;

            uint8 PRI:3;
            uint8 SPHY:5;

            uint8 L34FMT:4;
            uint8 L2FMT:4;

            uint8 L4ERR:1;
            uint8 L3ERR:1;
            uint8 L2ERR:1;
            uint8 PPPoE:1;
            uint8 RESERVED2:2;
            uint8 EXTRATAG:1;
            uint8 RESERVED1:1;  

            uint16 REASON;           

            uint16 OVID:12;
            uint16 RESERVED3:1;
            uint16 OPRI:3;            

            uint32 IVID:12;
            uint32 reserved4:1;
            uint32 IPRI:3;
#else
            uint8   PROTO;      /* Define the 8328 CPU tag. Value is 0x4. */
            uint8   SPHY:5;     /* Source port number */
            uint8   PRI:3;      /* Assigned priority */
            uint8   L2FMT:4;    /* Layer 2 format */
            uint8   L34FMT:4;   /* Layer 3/4 format */
            uint8   RESERVED1:1;/* Reserved bit */
            uint8   EXTRATAG:1; /* Extra tag exist */
            uint8   RESERVED2:2;/* Reserved bit */
            uint8   PPPoE:1;    /* Whether PPPoE header exists*/
            uint8   L2ERR:1;    /* Layer 2 CRC error */
            uint8   L3ERR:1;    /* IPv4 checksum error */
            uint8   L4ERR:1;    /* TCP, UDP, ICMP and IGMP checksum error */
            
            uint16  REASON;     /* Reason to CPU */
            
            uint16  OPRI:3;     /* Outer vlan tag priority, from pmi.dopri */
            uint16  RESERVED3:1;/* Reserved bit */
            uint16  OVID:12;    /* OVID, from pmi.dovid */
            uint16  IPRI:3;     /* inner vlan tag priority, from pmi.dipri */
            uint16  RESERVED4:1;/* Reserved bit */
            uint16  IVID:12;    /* IVID, from pmi.divid */
#endif            
        } __attribute__ ((aligned(1), packed)) rx;
        struct {
#ifdef CONFIG_SDK_ENDIAN_LITTLE
            uint8   PROTO;
            uint8   DPM_HIGHEST;     /* When ADSPM is 1, DPM is th packet destination port mask. */
            uint8   DPM_HIGH;
            uint8   DPM_LOW;
            
            uint8   L2CALC:1;
            uint8   ASPRMK:1;
            uint8   ASP:1;
            uint8   ASDPM:1;
            uint8   DPM_LOWEST:4;

            uint8   RESERVED1:1;
            uint8   PRI:3;
            uint8   DP:2;
            uint8   L4CALC:1; 
            uint8   L3CALC:1; 
            
            uint16 FLAG;     
            uint16 RESERVED2;
#else
            uint8   PROTO;      /* Define the 8389 CPU tag. Value is 0x4. */
            uint8   DPM_HIGHEST;     /* When ADSPM is 1, DPM is th packet destination port mask. */
            uint8   DPM_HIGH;
            uint8   DPM_LOW;
            
            uint8   DPM_LOWEST:4;     /* When ADSPM is 1, DPM is th packet destination port mask. */
            uint8   ASDPM:1;    /* Assign the destination port directly. */
            uint8   ASP:1;      /* Assign priority directly. */
            uint8   ASPRMK:1;   /* When ASPRMK is 1, the packet will be remarked based on port remarking setting. */
            uint8   L2CALC:1;   /* Recalculate the layer2 CRC */
            
            uint8   L3CALC:1;   /* Recalculate the IPv4 checksum */
            uint8   L4CALC:1;   /* Recalculate the TCP,UDP,ICMP,IGMP checksum */
            uint8   DP:2;       /* Reserved for being compatible with RTL8316s. Always 0 here */
            uint8   PRI:3;      /* Assigned priority */
            uint8   RESERVED1:1; /* Reserved bit */
            
            uint16  FLAG;       /* Module enable flag */
            uint16  RESERVED2;
#endif            
        } __attribute__ ((aligned(1), packed)) tx;
    } un;
} esw_nic_cpuTag_t;


typedef struct esw_nic_pkthdr_s
{
    struct esw_nic_mbuf_s *ph_mbuf;
    uint16  ph_len;
#ifdef    CONFIG_SDK_ENDIAN_LITTLE
    uint16  ph_reserve:7,
    ph_tag:1,
    ph_reserve1:8;
#else
    uint16  ph_tag:1,
    ph_reserve:15;  
#endif
    uint32  ph_tag0;
    uint32  ph_tag1;
    uint32  ph_tag2;

    /* Used by Software */
    uint32  *ring_entry;
    drv_nic_tx_cb_f tx_callback;
    void    *cookie;
} esw_nic_pkthdr_t;

typedef struct esw_nic_mbuf_s
{
    struct esw_nic_pkthdr_s *m_pkthdr;
    struct esw_nic_mbuf_s *m_next;
#if 0
#ifdef __LITTLE_ENDIAN
    uint16  m_extsize;
    uint16  m_len;
#else
    uint16  m_len;
    uint16  m_extsize;
#endif
#endif
    uint16  m_len;
    uint16  m_extsize;
    
    uint8   *m_data;
    uint8   *m_extbuf;

    /* Used by Software */
    struct drv_nic_pkt_s *packet;
    uint32  *ring_entry;
} esw_nic_mbuf_t;

typedef enum nic_module_enable_flag_e
{
    NIC_MODULE_ENABLE_FLAG_FC_EN = 0,             /* 0 */
    NIC_MODULE_ENABLE_FLAG_INGRESS_ACL_EN,        /* 1 */
    NIC_MODULE_ENABLE_FLAG_EGRESS_ACL_EN,         /* 2 */
    NIC_MODULE_ENABLE_FLAG_EGRESS_VLAN_TRAN_EN,   /* 3 */
    NIC_MODULE_ENABLE_FLAG_INGRESS_MAC1X_EN,      /* 4 */
    NIC_MODULE_ENABLE_FLAG_EGRESS_MAC1x_EN,       /* 5 */
    NIC_MODULE_ENABLE_FLAG_ATTACK_PREV_EN,        /* 6 */
    NIC_MODULE_ENABLE_FLAG_TAG_STADE_EN,          /* 7 */
    NIC_MODULE_ENABLE_FLAG_SML_EN,                /* 8 */
    NIC_MODULE_ENABLE_FLAG_INGRESS_VLAN_EN,       /* 9 */
    NIC_MODULE_ENABLE_FLAG_EGRESS_VLAN_EN,        /* 10 */
    NIC_MODULE_ENABLE_FLAG_LINKAGGR_LOADSHARE_EN, /* 11 */
    NIC_MODULE_ENABLE_FLAG_EGRESS_SPT_EN,         /* 12 */
    NIC_MODULE_ENABLE_FLAG_EGRESS_LDP_EN,         /* 13 */
    NIC_MODULE_ENABLE_FLAG_END,
} nic_module_enable_flag_t;

#define NIC_MODULE_ENABLE_FLAG_FC_EN_BIT                  (1 << NIC_MODULE_ENABLE_FLAG_FC_EN)                   /* 0x00000001 */
#define NIC_MODULE_ENABLE_FLAG_INGRESS_ACL_EN_BIT         (1 << NIC_MODULE_ENABLE_FLAG_INGRESS_ACL_EN)          /* 0x00000002 */
#define NIC_MODULE_ENABLE_FLAG_EGRESS_ACL_EN_BIT          (1 << NIC_MODULE_ENABLE_FLAG_EGRESS_ACL_EN)           /* 0x00000004 */
#define NIC_MODULE_ENABLE_FLAG_EGRESS_VLAN_TRAN_EN_BIT    (1 << NIC_MODULE_ENABLE_FLAG_EGRESS_VLAN_TRAN_EN)     /* 0x00000008 */
#define NIC_MODULE_ENABLE_FLAG_INGRESS_MAC1X_EN_BIT       (1 << NIC_MODULE_ENABLE_FLAG_INGRESS_MAC1X_EN)        /* 0x00000010 */
#define NIC_MODULE_ENABLE_FLAG_EGRESS_MAC1x_EN_BIT        (1 << NIC_MODULE_ENABLE_FLAG_EGRESS_MAC1x_EN)         /* 0x00000020 */
#define NIC_MODULE_ENABLE_FLAG_ATTACK_PREV_EN_BIT         (1 << NIC_MODULE_ENABLE_FLAG_ATTACK_PREV_EN)          /* 0x00000040 */
#define NIC_MODULE_ENABLE_FLAG_TAG_STADE_EN_BIT           (1 << NIC_MODULE_ENABLE_FLAG_TAG_STADE_EN)            /* 0x00000080 */
#define NIC_MODULE_ENABLE_FLAG_SML_EN_BIT                 (1 << NIC_MODULE_ENABLE_FLAG_SML_EN)                  /* 0x00000100 */
#define NIC_MODULE_ENABLE_FLAG_INGRESS_VLAN_EN_BIT        (1 << NIC_MODULE_ENABLE_FLAG_INGRESS_VLAN_EN)         /* 0x00000200 */
#define NIC_MODULE_ENABLE_FLAG_EGRESS_VLAN_EN_BIT         (1 << NIC_MODULE_ENABLE_FLAG_EGRESS_VLAN_EN)          /* 0x00000400 */
#define NIC_MODULE_ENABLE_FLAG_LINKAGGR_LOADSHARE_EN_BIT  (1 << NIC_MODULE_ENABLE_FLAG_LINKAGGR_LOADSHARE_EN)   /* 0x00000800 */
#define NIC_MODULE_ENABLE_FLAG_EGRESS_SPT_EN_BIT          (1 << NIC_MODULE_ENABLE_FLAG_EGRESS_SPT_EN)           /* 0x00001000 */
#define NIC_MODULE_ENABLE_FLAG_EGRESS_LDP_EN_BIT          (1 << NIC_MODULE_ENABLE_FLAG_EGRESS_LDP_EN)           /* 0x00002000 */
#define NIC_MODULE_ENABLE_FLAG_ALL_BIT                    ((1 << NIC_MODULE_ENABLE_FLAG_END)-1)                 /* 0x00003FFF */


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
extern int32
r8328_init(uint32 unit, drv_nic_initCfg_t *pInitCfg);

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
 *      When fTxCb is NULL, driver will free packet and not callback any more.
 */
extern int32
r8328_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie);

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
extern int32
r8328_rx_start(uint32 unit);

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
extern int32
r8328_rx_stop(uint32 unit);

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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
r8328_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags);

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
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
r8328_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb);

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
extern int32
r8328_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket);

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
extern int32
r8328_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket);

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
extern int32
r8328_debug_set(uint32 unit, uint32 flags);

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
extern int32
r8328_debug_get(uint32 unit, uint32 *pFlags);

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
extern int32
r8328_counter_dump(uint32 unit);

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
extern int32
r8328_counter_clear(uint32 unit);


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
extern int32
r8328_bufStatus_dump(uint32 unit);

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
extern int32
r8328_pkthdrMbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags);

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
extern int32
r8328_rxStatus_get(uint32 unit, uint32 *pStatus);

/* Function Name:
 *      r8328_isr_handler
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
r8328_isr_handler(uint32 intr_status);

#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
/* Function Name:
 *      r8328_pieCpuEntry_add
 * Description:
 *      Add the pie CPU entry to software shadow in the specified device.
 * Input:
 *      unit      - unit id
 *      entry_idx - ACL entry index
 *      pCpuEntry - CPU entry data for software parsing
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
r8328_pieCpuEntry_add(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry);

/* Function Name:
 *      r8328_pieCpuEntry_del
 * Description:
 *      Del the pie CPU entry from software shadow in the specified device.
 * Input:
 *      unit      - unit id
 *      entry_idx - ACL entry index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note:
 *      None
 */
extern int32
r8328_pieCpuEntry_del(uint32 unit, uint32 entry_idx);

/* Function Name:
 *      r8328_pieCpuEntry_get
 * Description:
 *      Get the pie CPU entry from software shadow in the specified device.
 * Input:
 *      unit      - unit id
 *      entry_idx - ACL entry index
 * Output:
 *      pCpuEntry - CPU entry data for software parsing
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
r8328_pieCpuEntry_get(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry);

/* Function Name:
 *      r8328_pieCpuEntry_set
 * Description:
 *      Set the pie CPU entry to software shadow in the specified device.
 * Input:
 *      unit      - unit id
 *      entry_idx - ACL entry index
 *      pCpuEntry - CPU entry data for software parsing
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
r8328_pieCpuEntry_set(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry);

#endif /* end of defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG) */
#endif /* __NIC_R8328_H__ */
