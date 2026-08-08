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
 * $Revision: 42525 $
 * $Date: 2013-09-03 14:59:18 +0800 (Tue, 03 Sep 2013) $
 *
 * Purpose : Definition those public EEE routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) EEE enable/disable
 */

#ifndef __RTK_EEE_H__
#define __RTK_EEE_H__

/*
 * Include Files
 */
#include <common/rt_type.h>

/*
 * Symbol Definition
 */

typedef enum rtk_eee_txSleepMode_e
{
    SLEEP_MODE_LOW_RATE = 0,    /* enter sleep mode if data rate is low */
    SLEEP_MODE_QUEUE_EMPTY,     /* enter sleep mode if all queues are empty */
    SLEEP_MODE_LOW_RATE_AND_QUEUE_EMPTY,    /* enter sleep mode if data rate is low and all queues are empty */
    SLEEP_MODE_END
} rtk_eee_txSleepMode_t;

typedef enum rtk_eee_txWakeMode_e
{
    WAKE_MODE_ANY_PKT_TX = 0,   /* wake up if any packet is going to TX */
    WAKE_MODE_QOS_BASED,    /* high queue packets would immediately cause waking up; low queue packets would hold a while to cause wake up */
    WAKE_MODE_END
} rtk_eee_txWakeMode_t;

typedef enum rtk_eee_eeep_state_e
{
    EEE_EEEP_WAKE_STATE = 0,    /* the port is wake up */
    EEE_EEEP_SLEEP_STATE,   /* the port is in sleep state */
    EEE_EEEP_STATE_END
} rtk_eee_eeep_state_t;

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_eee_init
 * Description:
 *      Initialize EEE module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize EEE module before calling any EEE APIs.
 */
extern int32
rtk_eee_init(uint32 unit);


/* Module Name    : EEE                */
/* Sub-module Name: EEE enable/disable */

/* Function Name:
 *      rtk_eee_portEnable_get
 * Description:
 *      Get enable status of EEE function in the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of EEE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_eee_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_eee_portEnable_set
 * Description:
 *      Set enable status of EEE function in the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of EEE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Needs to restart auto-negotiation for the configuration to take effect.
 */
extern int32
rtk_eee_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      rtk_eee_portState_get
 * Description:
 *      Get the EEE nego result state of a port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pState - pointer to the EEE port nego result state
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_eee_portState_get(
    uint32 unit,
    rtk_port_t port,
    rtk_enable_t *pState);

/* Function Name:
 *      rtk_eeep_portEnable_get
 * Description:
 *      Get enable status of EEEP function in the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of EEEP
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_eeep_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      rtk_eeep_portEnable_set
 * Description:
 *      Set enable status of EEEP function in the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of EEEP
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      None
 */
extern int32
rtk_eeep_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

#endif /* __RTK_EEE_H__ */
