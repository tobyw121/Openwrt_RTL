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
 * $Revision: 40458 $
 * $Date: 2013-06-25 09:47:32 +0800 (Tue, 25 Jun 2013) $
 *
 * Purpose : Definition of Statistic API
 *
 * Feature : The file includes the following modules and sub-modules
 *           (1) Statistic Counter Reset
 *           (2) Statistic Counter Get
 *
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/stat.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Module Name : STAT */

/* Function Name:
 *      rtk_stat_init
 * Description:
 *      Initialize stat module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_PORT_CNTR_FAIL   - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize stat module before calling any stat APIs.
 */
int32
rtk_stat_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_init(unit);
} /* end of rtk_stat_init */

/* Function Name:
 *      rtk_stat_enable_get
 * Description:
 *      Get the status to check whether MIB counters are enabled or not.
 * Input:
 *      unit    - unit id
 * Output:
 *      pEnable - pointer buffer of enable or disable MIB counters
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
rtk_stat_enable_get(uint32 unit, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_enable_get(unit, pEnable);
} /* end of rtk_stat_enable_get */

/* Function Name:
 *      rtk_stat_enable_set
 * Description:
 *      Set the status to enable or disable MIB counters
 * Input:
 *      unit   - unit id
 *      enable - enable/disable MIB counters
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8380
 * Note:
 *      None
 */
int32
rtk_stat_enable_set(uint32 unit, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_enable_set(unit, enable);
} /* end of rtk_stat_enable_set */

/* Function Name:
 *      rtk_stat_global_reset
 * Description:
 *      Reset the global counters in the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_NOT_INIT              - The module is not initial
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_stat_global_reset(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_global_reset(unit);
} /* end of rtk_stat_global_reset */

/* Function Name:
 *      rtk_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_stat_port_reset(uint32 unit, rtk_port_t port)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->stat_port_reset(unit, phy_port);
    }
#else
    return RT_MAPPER(unit)->stat_port_reset(unit, port);
#endif
} /* end of rtk_stat_port_reset */

/* Function Name:
 *      rtk_stat_global_get
 * Description:
 *      Get one specified global counter in the specified device.
 * Input:
 *      unit     - unit id
 *      cntr_idx - specified global counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NOT_INIT                 - The module is not initial
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      rtk_stat_global_type_t \ Chip :             8389    8328    8390    8380
 *      - DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX     O       O       O       O
 *      - DOT1D_TP_IN_DISCARDS_INDEX                O       X       X       X 
 *      - OUT_UCAST_PKTS_INDEX                      O       X       X       X 
 *      - OUT_MCAST_PKTS_INDEX                      O       X       X       X 
 *      - OUT_BCAST_PKTS_INDEX                      O       X       X       X 
 *      - EGR_LACK_RESOURCE_DROP_INDEX              O       X       X       X 
 */
int32
rtk_stat_global_get(uint32 unit, rtk_stat_global_type_t cntr_idx, uint64 *pCntr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_global_get(unit, cntr_idx, pCntr);
} /* end of rtk_stat_global_get */

/* Function Name:
 *      rtk_stat_global_getAll
 * Description:
 *      Get all global counters in the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      pGlobal_cntrs - pointer buffer of global counter structure
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 *      RT_ERR_NOT_INIT              - The module is not initial
 *      RT_ERR_NULL_POINTER          - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL - Could not retrieve/reset Global Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_stat_global_getAll(uint32 unit, rtk_stat_global_cntr_t *pGlobal_cntrs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_global_getAll(unit, pGlobal_cntrs);
} /* end of rtk_stat_global_getAll */

/* Function Name:
 *      rtk_stat_port_get
 * Description:
 *      Get one specified port counter in the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      cntr_idx - specified port counter index
 * Output:
 *      pCntr    - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE        - input parameter out of range
 *      RT_ERR_INPUT               - Invalid input parameter
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Supported management frame is as following:
 *      - rtk_stat_port_type_t \ Chip:                  8389    8328    8390    8380
 *      - IF_IN_OCTETS_INDEX                            O       O       O       O
 *      - IF_IN_UCAST_PKTS_INDEX                        O       O       O       O
 *      - IF_IN_MULTICAST_PKTS_INDEX                    X       X       O       O
 *      - IF_IN_BROADCAST_PKTS_INDEX                    X       X       O       O
 *      - IF_IN_DISCARDS_INDEX                          X       O       X       X
 *      - IF_OUT_OCTETS_INDEX                           O       O       O       O
 *      - IF_OUT_DISCARDS_INDEX                         X       O       O       O
 *      - IF_OUT_UCAST_PKTS_CNT_INDEX                   O       O       O       O
 *      - IF_OUT_MULTICAST_PKTS_CNT_INDEX               O       O       O       O
 *      - IF_OUT_BROADCAST_PKTS_CNT_INDEX               O       O       O       O
 *      - IP_IN_RECEIVES_INDEX                          X       O       X       X
 *      - IP_IN_DISCARDS_INDEX                          X       O       X       X
 *      - IF_HC_IN_OCTETS_INDEX                         X       O       O       O
 *      - IF_HC_IN_UCAST_PKTS_INDEX                     X       O       X       X
 *      - IF_HC_IN_MULTICAST_PKTS_INDEX                 X       O       X       X
 *      - IF_HC_IN_BROADCAST_PKTS_INDEX                 X       O       X       X
 *      - IF_HC_OUT_OCTETS_INDEX                        X       O       O       O
 *      - IF_HC_OUT_UCAST_PKTS_INDEX                    X       O       X       X
 *      - IF_HC_OUT_MULTICAST_PKTS_INDEX                X       O       X       X
 *      - IF_HC_OUT_BROADCAST_PKTS_INDEX                X       O       X       X
 *      - DOT1D_BASE_PORT_DELAY_EXCEEDED_DISCARDS_INDEX X       O       X       X
 *      - DOT1D_TP_PORT_IN_DISCARDS_INDEX               X       O       O       O
 *      - DOT1D_TP_HC_PORT_IN_DISCARDS_INDEX            X       O       X       X
 *      - DOT3_IN_PAUSE_FRAMES_INDEX                    O       O       O       O
 *      - DOT3_OUT_PAUSE_FRAMES_INDEX                   O       O       O       O
 *      - DOT3_OUT_PAUSE_ON_FRAMES_INDEX                O       O       X       X
 *      - DOT3_STATS_ALIGNMENT_ERRORS_INDEX             X       O       X       X
 *      - DOT3_STATS_FCS_ERRORS_INDEX                   O       O       X       X
 *      - DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX      O       O       O       O
 *      - DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX    O       O       O       O
 *      - DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX       O       O       O       O
 *      - DOT3_STATS_LATE_COLLISIONS_INDEX              O       O       O       O
 *      - DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX         O       O       O       O
 *      - DOT3_STATS_FRAME_TOO_LONGS_INDEX              X       O       X       X
 *      - DOT3_STATS_SYMBOL_ERRORS_INDEX                O       O       O       O
 *      - DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX         O       O       O       O
 *      - ETHER_STATS_DROP_EVENTS_INDEX                 X       O       O       O
 *      - ETHER_STATS_OCTETS_INDEX                      O       O       X       X
 *      - ETHER_STATS_BROADCAST_PKTS_INDEX              O       O       O       O
 *      - ETHER_STATS_MULTICAST_PKTS_INDEX              O       O       O       O
 *      - ETHER_STATS_UNDER_SIZE_PKTS_INDEX             O       O       O       O
 *      - ETHER_STATS_OVERSIZE_PKTS_INDEX               O       O       O       O
 *      - ETHER_STATS_FRAGMENTS_INDEX                   O       O       O       O
 *      - ETHER_STATS_JABBERS_INDEX                     O       O       O       O
 *      - ETHER_STATS_COLLISIONS_INDEX                  O       O       O       O
 *      - ETHER_STATS_CRC_ALIGN_ERRORS_INDEX            X       X       O       O
 *      - ETHER_STATS_PKTS_64OCTETS_INDEX               O       O       O       O
 *      - ETHER_STATS_PKTS_65TO127OCTETS_INDEX          O       O       O       O
 *      - ETHER_STATS_PKTS_128TO255OCTETS_INDEX         O       O       O       O
 *      - ETHER_STATS_PKTS_256TO511OCTETS_INDEX         O       O       O       O
 *      - ETHER_STATS_PKTS_512TO1023OCTETS_INDEX        O       O       O       O
 *      - ETHER_STATS_PKTS_1024TO1518OCTETS_INDEX       O       O       O       O
 *      - ETHER_STATS_TX_OCTETS_INDEX                   X       O       X       X
 *      - ETHER_STATS_TX_UNDER_SIZE_PKTS_INDEX          X       O       O       O
 *      - ETHER_STATS_TX_OVERSIZE_PKTS_INDEX            X       O       O       O
 *      - ETHER_STATS_TX_PKTS_64OCTETS_INDEX            X       O       O       O
 *      - ETHER_STATS_TX_PKTS_65TO127OCTETS_INDEX       X       O       O       O
 *      - ETHER_STATS_TX_PKTS_128TO255OCTETS_INDEX      X       O       O       O
 *      - ETHER_STATS_TX_PKTS_256TO511OCTETS_INDEX      X       O       O       O
 *      - ETHER_STATS_TX_PKTS_512TO1023OCTETS_INDEX     X       O       O       O
 *      - ETHER_STATS_TX_PKTS_1024TO1518OCTETS_INDEX    X       O       O       O
 *      - ETHER_STATS_TX_PKTS_1519TOMAXOCTETS_INDEX     X       X       O       O
 *      - ETHER_STATS_TX_BROADCAST_PKTS_INDEX           X       X       O       O
 *      - ETHER_STATS_TX_MULTICAST_PKTS_INDEX           X       X       O       O
 *      - ETHER_STATS_TX_FRAGMENTS_INDEX                X       X       X       X
 *      - ETHER_STATS_TX_JABBERS_INDEX                  X       X       X       X
 *      - ETHER_STATS_TX_CRC_ALIGN_ERROR_INDEX          X       X       X       X
 *      - ETHER_STATS_RX_UNDER_SIZE_PKTS_INDEX          X       X       O       O
 *      - ETHER_STATS_RX_UNDER_SIZE_DROP_PKTS_INDEX     X       X       O       O
 *      - ETHER_STATS_RX_OVERSIZE_PKTS_INDEX            X       X       O       O
 *      - ETHER_STATS_RX_PKTS_64OCTETS_INDEX            X       X       O       O
 *      - ETHER_STATS_RX_PKTS_65TO127OCTETS_INDEX       X       X       O       O
 *      - ETHER_STATS_RX_PKTS_128TO255OCTETS_INDEX      X       X       O       O
 *      - ETHER_STATS_RX_PKTS_256TO511OCTETS_INDEX      X       X       O       O
 *      - ETHER_STATS_RX_PKTS_512TO1023OCTETS_INDEX     X       X       O       O
 *      - ETHER_STATS_RX_PKTS_1024TO1518OCTETS_INDEX    X       X       O       O
 *      - ETHER_STATS_RX_PKTS_1519TOMAXOCTETS_INDEX     X       X       O       O
 *      - RX_LENGTH_FIELD_ERROR_INDEX                   X       X       O       X
 *      - RX_FALSE_CARRIER_TIMES_INDEX                  X       X       O       X
 *      - RX_UNDER_SIZE_OCTETS_INDEX                    X       X       O       X
 *      - TX_ETHER_STATS_FRAGMENTS_INDEX                X       X       O       X
 *      - TX_ETHER_STATS_JABBERS_INDEX                  X       X       O       X
 *      - TX_ETHER_STATS_CRC_ALIGN_ERRORS_INDEX         X       X       O       X
 *      - RX_FRAMING_ERRORS_INDEX                       X       X       O       X
 *      - IGR_LACK_PKT_BUF_DROP_INDEX                   O       X       X       X
 *      - FLOWCTRL_ON_DROP_PKT_CNT_INDEX                O       X       X       X
 *      - TX_CRC_CHECK_FAIL_CNT_INDEX                   O       X       X       X
 *      - SMART_TRIGGER_HIT0_INDEX                      O       X       X       X
 *      - SMART_TRIGGER_HIT1_INDEX                      O       X       X       X
 *      - RX_MAC_DISCARDS_INDEX                         X       X       O       O 
 */
int32
rtk_stat_port_get(uint32 unit, rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->stat_port_get(unit, phy_port, cntr_idx, pCntr);
    }
#else
    return RT_MAPPER(unit)->stat_port_get(unit, port, cntr_idx, pCntr);
#endif
} /* end of rtk_stat_port_get */

/* Function Name:
 *      rtk_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pPort_cntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      None
 */
int32
rtk_stat_port_getAll(uint32 unit, rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->stat_port_getAll(unit, phy_port, pPort_cntrs);
    }
#else
    return RT_MAPPER(unit)->stat_port_getAll(unit, port, pPort_cntrs);
#endif
} /* end of rtk_stat_port_getAll */

/* Function Name:
 *      rtk_stat_smon_get
 * Description:
 *      Get one specified SMON counter in specified device.
 * Input:
 *      unit     - unit id
 *      pri      - priority
 *      cntr_idx - index of specified SMON counter
 * Output:
 *      pCntr    - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                - invalid unit id
 *      RT_ERR_NOT_INIT               - The module is not initial
 *      RT_ERR_NULL_POINTER           - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL    - Could not retrieve/reset SMON Counter
 *      RT_ERR_STAT_INVALID_SMON_CNTR - Could not retrieve/reset SMON Counter
 * Applicable:
 *      8328
 * Note:
 *      (1) The cntr_idx value is as following:
 *          - SMON_PRIO_STATS_PKTS
 *          - SMON_PRIO_STATS_OCTETS
 */
int32
rtk_stat_smon_get(uint32 unit, rtk_pri_t pri, rtk_stat_smon_type_t cntr_idx, uint64 *pCntr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_smon_get(unit, pri, cntr_idx, pCntr);
} /* end of rtk_stat_smon_get */

/* Function Name:
 *      rtk_stat_smon_getAll
 * Description:
 *      Get all specified SMON counter in specified device.
 * Input:
 *      unit  - unit id
 *      pri   - priority
 * Output:
 *      pCntr - pointer buffer of counte value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_NULL_POINTER        - inputt parameter may be null pointer
 *      RT_ERR_STAT_SMON_CNTR_FAIL - Could not retrieve/reset SMON Counter
 * Applicable:
 *      8328
 * Note:
 *      None
 */
int32
rtk_stat_smon_getAll(uint32 unit, rtk_pri_t pri, rtk_stat_smon_cntr_t *pCntr)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_smon_getAll(unit, pri, pCntr);
} /* end of rtk_stat_smon_getAll */

/* Function Name:
 *      rtk_stat_tagLenCntIncEnable_get
 * Description:
 *      Get RX/TX counter to include or exclude tag length in the specified device.
 * Input:
 *      unit        - unit id
 *      tagCnt_type - specified RX counter or TX counter
 * Output:
 *      pEnable     - pointer buffer of including/excluding tag length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      Inner(4B) and outer(4B) tag length can be included or excluded to the counter through the API.
 */
int32
rtk_stat_tagLenCntIncEnable_get(uint32 unit, rtk_stat_tagCnt_type_t tagCnt_type, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_tagLenCntIncEnable_get(unit, tagCnt_type, pEnable);
} /* end of rtk_stat_tagLenCntIncEnable_get */

/* Function Name:
 *      rtk_stat_tagLenCntIncEnable_set
 * Description:
 *      Set RX/TX counter to include or exclude tag length in the specified device.
 * Input:
 *      unit        - unit id
 *      tagCnt_type - specified RX counter or TX counter
 *      enable      - include/exclude Tag length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      Inner(4B) and outer(4B) tag length can be included or excluded to the counter through the API.
 */
int32
rtk_stat_tagLenCntIncEnable_set(uint32 unit, rtk_stat_tagCnt_type_t tagCnt_type, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->stat_tagLenCntIncEnable_set(unit, tagCnt_type, enable);
} /* end of rtk_stat_tagLenCntIncEnable_set */
