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
 * $Revision: 38707 $
 * $Date: 2013-04-18 19:53:07 +0800 (Thu, 18 Apr 2013) $
 *
 * Purpose : Definition of Network Interface Controller API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) CPU Tag
 *           (2) NIC Tx
 *           (3) NIC Rx
 *
 */

/*
 * Include Files
 */
#include <drv/nic/nic.h>
#include <drv/nic/probe.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      drv_nic_init
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize nic module before calling any nic APIs.
 */
int32
drv_nic_init(uint32 unit, drv_nic_initCfg_t *pInitCfg)
{
    return NIC_CTRL(unit).init(unit, pInitCfg);
} /* end of drv_nic_init */


/* Function Name:
 *      drv_nic_pkt_tx
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      When fTxCb is NULL, driver will free packet and not callback any more.
 */
int32
drv_nic_pkt_tx(uint32 unit, drv_nic_pkt_t *pPacket, drv_nic_tx_cb_f fTxCb, void *pCookie)
{
    return NIC_CTRL(unit).pkt_tx(unit, pPacket, fTxCb, pCookie);
} /* end of drv_nic_pkt_tx */


/* Function Name:
 *      drv_nic_rx_start
 * Description:
 *      Start the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
drv_nic_rx_start(uint32 unit)
{
    return NIC_CTRL(unit).rx_start(unit);
} /* end of drv_nic_rx_start */


/* Function Name:
 *      drv_nic_rx_stop
 * Description:
 *      Stop the rx action of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
drv_nic_rx_stop(uint32 unit)
{
    return NIC_CTRL(unit).rx_stop(unit);
} /* end of drv_nic_rx_stop */


/* Function Name:
 *      drv_nic_rx_register
 * Description:
 *      Register to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback (255 is lowest)
 *      fRxCb    - pointer to a handler of received packets
 *      pCookie  - application data returned with callback (can be null)
 *      flags    - optional flags for reserved
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
drv_nic_rx_register(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb, void *pCookie, uint32 flags)
{
    return NIC_CTRL(unit).rx_register(unit, priority, fRxCb, pCookie, flags);
} /* end of drv_nic_rx_register */


/* Function Name:
 *      drv_nic_rx_unregister
 * Description:
 *      Unregister to receive callbacks for received packets of the specified device.
 * Input:
 *      unit     - unit id
 *      priority - Relative priority of the callback (255 is lowest)
 *      fRxCb    - pointer to a handler of received packets (can be null)
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
drv_nic_rx_unregister(uint32 unit, uint8 priority, drv_nic_rx_cb_f fRxCb)
{
    return NIC_CTRL(unit).rx_unregister(unit, priority, fRxCb);
} /* end of drv_nic_rx_unregister */

/* Function Name:
 *      drv_nic_pkt_alloc
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
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
drv_nic_pkt_alloc(uint32 unit, int32 size, uint32 flags, drv_nic_pkt_t **ppPacket)
{
    return NIC_CTRL(unit).pkt_alloc(unit, size, flags, ppPacket);
} /* end of drv_nic_pkt_alloc */

/* Function Name:
 *      drv_nic_pkt_free
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
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
drv_nic_pkt_free(uint32 unit, drv_nic_pkt_t *pPacket)
{
    return NIC_CTRL(unit).pkt_free(unit, pPacket);
} /* end of drv_nic_pkt_free */



/* Function Name:
 *      drv_nic_reset
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
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
drv_nic_reset(uint32 unit)
{
    return NIC_CTRL(unit).nic_reset(unit);
} /* end of drv_nic_reset */



#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
/* Function Name:
 *      drv_nic_pieCpuEntry_add
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
drv_nic_pieCpuEntry_add(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry)
{
    return NIC_CTRL(unit).pieCpuEntry_add(unit, entry_idx, pCpuEntry);
} /* end of drv_nic_pieCpuEntry_add */

/* Function Name:
 *      drv_nic_pieCpuEntry_del
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
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
drv_nic_pieCpuEntry_del(uint32 unit, uint32 entry_idx)
{
    return NIC_CTRL(unit).pieCpuEntry_del(unit, entry_idx);
} /* end of drv_nic_pieCpuEntry_del */

/* Function Name:
 *      drv_nic_pieCpuEntry_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
drv_nic_pieCpuEntry_get(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry)
{
    return NIC_CTRL(unit).pieCpuEntry_get(unit, entry_idx, pCpuEntry);
} /* end of drv_nic_pieCpuEntry_get */

/* Function Name:
 *      drv_nic_pieCpuEntry_set
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
drv_nic_pieCpuEntry_set(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry)
{
    return NIC_CTRL(unit).pieCpuEntry_set(unit, entry_idx, pCpuEntry);
} /* end of drv_nic_pieCpuEntry_set */
#endif /* end of defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG) */
