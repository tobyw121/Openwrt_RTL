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
 * $Revision: 53061 $
 * $Date: 2014-11-14 16:25:10 +0800 (Fri, 14 Nov 2014) $
 *
 * Purpose : PHY 8328 intra serdes Driver APIs.
 *
 * Feature : PHY 8328 intra serdes Driver APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_common.h>
#include <hal/phy/phy_8328.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
rt_phydrv_t phy_8328_serdes_ge =
{
    RT_PHYDRV_RTL8328_SERDES_GE,
    phy_8328_init,
    phy_8328_media_get,
    (int32 (*)(uint32, rtk_port_t, rtk_port_media_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_phy_ability_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_phy_ability_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32 *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32 *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_rtctResult_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_mode_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_mode_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,       
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8328_gigaLiteEnable_get,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,      
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32 , rtk_port_t , rtk_port_masterSlave_t *, rtk_port_masterSlave_t *))phy_common_unavail,
    (int32 (*)(uint32 , rtk_port_t , rtk_port_masterSlave_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8328_serdes_ge */

/* Function Name:
 *      phy_8328_init
 * Description:
 *      Initialize 8328 MAC internal serdes PHY.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8328_init(uint32 unit, rtk_port_t port)
{
    return RT_ERR_OK;
} /* end of phy_8328_init */

/* Function Name:
 *      phy_8328_media_get
 * Description:
 *      Get 8328 serdes PHY media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. media type is PORT_MEDIA_FIBER
 */
int32
phy_8328_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    *pMedia = PORT_MEDIA_FIBER;
    return RT_ERR_OK;
} /* end of phy_8328_media_get */

/* Function Name:
 *      phy_8328_gigaLiteEnable_get
 * Description:
 *      Get the status of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8328 is not supported the per-port Giga Lite feature.
 */
int32
phy_8328_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    *pEnable = DISABLED;
    return RT_ERR_OK;
} /* end of phy_8328_gigaLiteEnable_get */
