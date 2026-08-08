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
 * $Revision: 35134 $
 * $Date: 2012-12-07 19:52:08 +0800 (Fri, 07 Dec 2012) $
 *
 * Purpose : Define diag shell database.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Diag shell database.
 */
 
#include <stdio.h>
#include <string.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <rtk/switch.h>
#include <diag_om.h>

static int  current_chip_id = DIAG_OM_CHIP_ID_DEFAULT;
static rtk_switch_devInfo_t chip_devInfo;

int32
diag_om_get_chip_id(int *chipid)
{
    if (NULL == chipid)
        return RT_ERR_FAILED;
        
    *chipid = current_chip_id;
    return RT_ERR_OK; 
} /* end of diag_get_chip_id */

int32
diag_om_set_chip_id(int chipid)
{
    if (chipid > DIAG_OM_CHIP_ID_MAX)
        return RT_ERR_FAILED;
        
    current_chip_id = chipid;
    return RT_ERR_OK; 
} /* end of diag_set_chip_id */

int32
diag_om_get_deviceInfo(uint32 unit, rtk_switch_devInfo_t *pDevInfo)
{
    if (unit > DIAG_OM_CHIP_ID_MAX)
        return RT_ERR_FAILED;
        
    if (NULL == pDevInfo)
        return RT_ERR_FAILED;
        
    memcpy(pDevInfo, &chip_devInfo, sizeof(rtk_switch_devInfo_t));
    return RT_ERR_OK; 
} /* end of diag_om_get_deviceInfo */

int32
diag_om_set_deviceInfo(uint32 unit)
{
    rtk_switch_devInfo_t devInfo;
    
    if (unit > DIAG_OM_CHIP_ID_MAX)
        return RT_ERR_FAILED;

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    if (rtk_switch_deviceInfo_get(unit, &devInfo) != RT_ERR_OK)
    {
        return RT_ERR_FAILED;
    }
    
    memcpy(&chip_devInfo, &devInfo, sizeof(rtk_switch_devInfo_t));
    return RT_ERR_OK; 
} /* end of diag_om_set_deviceInfo */

int32
diag_om_get_realChipId(uint32 unit, uint32 chipId)
{    
    if (unit > DIAG_OM_CHIP_ID_MAX)
        return FALSE;

    if (chipId == chip_devInfo.chipId)
        return TRUE;
    else
        return FALSE;
} /* end of diag_om_get_realChipId */

int32
diag_om_get_familyId(uint32 unit, uint32 familyId)
{    
    if (unit > DIAG_OM_CHIP_ID_MAX)
        return FALSE;

    if (familyId == chip_devInfo.familyId)
        return TRUE;
    else
        return FALSE;
} /* end of diag_om_get_familyId */

int32
diag_om_get_testChipId(uint32 unit, uint32 chipId)
{    
    if (unit > DIAG_OM_CHIP_ID_MAX)
        return FALSE;

    if ((chipId & 0xffff) == (chip_devInfo.chipId & 0xffff))
        return TRUE;
    else
        return FALSE;
} /* end of diag_om_get_testChipId */

