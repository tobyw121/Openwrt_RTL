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
 * $Revision: 54450 $
 * $Date: 2014-12-30 14:31:41 +0800 (Tue, 30 Dec 2014) $
 *
 * Purpose : mac driver service APIs in the SDK.
 *
 * Feature : mac driver service APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
#include <drv/rtl8231/rtl8231.h>
#endif
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#include <hal/chipdef/cypress/rtk_cypress_table_struct.h>
#include <hal/chipdef/maple/rtk_maple_reg_struct.h>
#include <hal/chipdef/maple/rtk_maple_table_struct.h>
#include <hal/common/halctrl.h>
#include <hal/phy/phydef.h>
#include <hal/mac/drv.h>
#include <hal/mac/reg.h>
#include <hal/mac/mac_probe.h>
#include <soc/type.h>
#include <osal/time.h>
#include <ioal/mem32.h>
/*
 * Symbol Definition
 */
#define CHECKBUSY_TIMES (3000)


/*
 * Macro Declaration
 */
#define PHY_BUSY_WAIT_LOOP(unit, REG, MASK)\
{\
    uint32 i;\
    uint32 regVal;\
    for (i = 0; i < CHECKBUSY_TIMES; i++)\
    {\
        if (reg_read(unit, REG, &regVal) != RT_ERR_OK)\
        {\
            PHY_SEM_UNLOCK(unit);\
            return RT_ERR_FAILED;\
        }\
        if (0 == (regVal & MASK))\
        {\
            break;\
        }\
    }\
    if (CHECKBUSY_TIMES == i)\
    {\
        PHY_SEM_UNLOCK(unit);\
        return RT_ERR_FAILED;\
    }\
}

#if defined(CONFIG_SDK_RTL8390)

void drv_serdes_set(uint32 unit, uint32 reg, uint32 endBit,
    uint32 startBit, uint32 val)
{
    uint32  configVal, len, mask;
    uint32  i;

    len = endBit - startBit + 1;

    if (32 == len)
        configVal = val;
    else
    {
        mask = 0;
        for (i = startBit; i <= endBit; ++i)
            mask |= (1 << i);

        ioal_mem32_read(unit, reg, &configVal);
        configVal &= ~(mask);
        configVal |= (val << startBit);
    }
    //RT_DBG(LOG_EVENT, (MOD_DAL|MOD_PORT), "reg 0x%x val 0x%x",reg, configVal);
    ioal_mem32_write(unit, reg, configVal);

    return;
}

#endif

/*
 * Function Declaration
 */
#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
static int32 rtl83xx_mdcSem_callback(uint32 unit, uint32 type);
#endif

/* Function Name:
 *      table_find
 * Description:
 *      Find this kind of table structure in this specified chip.
 * Input:
 *      unit  - unit id
 *      table - table index
 * Output:
 *      None
 * Return:
 *      NULL      - Not found
 *      Otherwise - Pointer of table structure that found
 * Note:
 *      None
 */
rtk_table_t *
table_find (uint32 unit, uint32 table)
{
    hal_control_t *pHalCtrl = NULL;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return NULL;

    return (pHalCtrl->pChip_driver->pTable_list[table]);
} /* end of table_find */

#if defined(CONFIG_SDK_DUMP_TABLE_WITH_NAME)
/* Function Name:
 *      table_name_get
 * Description:
 *      Get table name of the specified register index.
 * Input:
 *      unit  - unit id
 *      table   - register index
 * Output:
 *      pData - pointer buffer of table name
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
int32
table_name_get(uint32 unit, uint32 table, char *pData)
{

    hal_control_t   *pHalCtrl = NULL;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, table=%d", unit, table);
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

	if(pHalCtrl->pChip_driver->pTable_name_list == NULL)
	{
		return RT_ERR_NULL_POINTER;
	}else{
		osal_memcpy(pData, pHalCtrl->pChip_driver->pTable_name_list[table]->name, 64);
	}

    return RT_ERR_OK;
} /* end of table_name_get */
#endif


#if defined(CONFIG_SDK_RTL8389)
/* Function Name:
 *      rtl8389_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8389_port_probe(uint32 unit)
{
    uint32  fiber_interface = 0;
    int32   ret = RT_ERR_FAILED;

    /* Probe intra-link serdes 0 & 1 interface mode (RSGMII or Fiber) */
    if ((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_FX0f, &fiber_interface)) != RT_ERR_OK)
    {
        return ret;
    }
    if (1 == fiber_interface)
        hal_portInfo_update(unit, 24, RT_GE_SERDES_PORT);

    if ((ret = reg_field_read(unit, SSW_GLOBAL_MAC_INTERFACE_CONTROL1r, SSW_SEL_INTRA_FX1f, &fiber_interface)) != RT_ERR_OK)
    {
        return ret;
    }
    if (1 == fiber_interface)
        hal_portInfo_update(unit, 25, RT_GE_SERDES_PORT);

    return RT_ERR_OK;
} /* end of rtl8389_port_probe */


/* Function Name:
 *      rtl8389_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8389_init(uint32 unit)
{
/* FLOWCTRL module */
#define FCON_SYSTEM_HIGHON_THRESH       0x38E
#define FCON_SYSTEM_HIGHOFF_THRESH      0x35C
#define FCON_SYSTEM_LOWON_THRESH        0x30C
#define FCON_SYSTEM_LOWOFF_THRESH       0x2DA
#define FCON_PORT_HIGHON_THRESH         0x12C
#define FCON_PORT_HIGHOFF_THRESH        0x8C
#define FCON_PORT_LOWON_THRESH          0x17

#define FCOFF_SYSTEM_HIGHON_THRESH      0x38E
#define FCOFF_SYSTEM_HIGHOFF_THRESH     0x35C
#define FCOFF_SYSTEM_LOWON_THRESH       0x30C
#define FCOFF_SYSTEM_LOWOFF_THRESH      0x2DA
#define FCOFF_PORT_HIGHON_THRESH        0x12C
#define FCOFF_PORT_HIGHOFF_THRESH       0x8C
#define FCOFF_PORT_LOWON_THRESH         0x17

#define EGRDROP_PORT_HIGH_THRESH        0xF
#define EGRDROP_QUEUE_HIGH_THRESH       0x700

    /* Initialize the threshold value of chip buffer if need */
    uint32      value = 0, reg_idx = 0, temp;
    int32       ret = RT_ERR_FAILED;
    rtk_port_t  port, max_port;
    rtk_qid_t   queue;

#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
    /* register rtl8231 mdc semphore callback function */
    if ((ret = drv_rtl8231_mdcSem_register(unit, rtl83xx_mdcSem_callback)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }
#endif

    /* Set per system flow control on threshold */
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCON_SYSTEM_HIGHON_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, SSW_TH_GLOBAL_HIGH_ONf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCON_SYSTEM_HIGHOFF_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, SSW_TH_GLOBAL_HIGH_OFFf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL1r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }

    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCON_SYSTEM_LOWON_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, SSW_TH_GLOBAL_LOW_ONf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCON_SYSTEM_LOWOFF_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, SSW_TH_GLOBAL_LOW_OFFf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL2r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }

    /* Set per system flow control off threshold */
    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCOFF_SYSTEM_HIGHON_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, SSW_TH_GLOBAL_FCOFF_HIGH_ONf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCOFF_SYSTEM_HIGHOFF_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, SSW_TH_GLOBAL_FCOFF_HIGH_OFFf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL3r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }

    if ((ret = reg_read(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCOFF_SYSTEM_LOWON_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, SSW_TH_GLOBAL_FCOFF_LOW_ONf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    temp = FCOFF_SYSTEM_LOWOFF_THRESH;
    if ((ret = reg_field_set(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, SSW_TH_GLOBAL_FCOFF_LOW_OFFf, &temp, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    if ((ret = reg_write(unit, SSW_FLOW_CONTROL_GLOBAL_THRESHOLD_CONTROL4r, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }

    /* Set per port flow control threshold */
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        uint16 flow_control_threshold_for_control2_regidx[] = {SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_0_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_1_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_2_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_3_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_4_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_5_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_6_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_7_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_8_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_9_CONTROL2r\
            ,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_10_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_11_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_12_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_13_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_14_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_15_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_16_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_17_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_18_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_19_CONTROL2r\
            ,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_20_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_21_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_22_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_23_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_24_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_25_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_26_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_27_CONTROL2r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_28_CONTROL2r};
        uint16 th_port_high_fcoff_on_fieldidx[] = {SSW_TH_PORT0_FCOFF_HIGH_ONf, SSW_TH_PORT1_FCOFF_HIGH_ONf, SSW_TH_PORT2_FCOFF_HIGH_ONf, SSW_TH_PORT3_FCOFF_HIGH_ONf, SSW_TH_PORT4_FCOFF_HIGH_ONf, SSW_TH_PORT5_FCOFF_HIGH_ONf, SSW_TH_PORT6_FCOFF_HIGH_ONf, SSW_TH_PORT7_FCOFF_HIGH_ONf, SSW_TH_PORT8_FCOFF_HIGH_ONf, SSW_TH_PORT9_FCOFF_HIGH_ONf\
            ,SSW_TH_PORT10_FCOFF_HIGH_ONf, SSW_TH_PORT11_FCOFF_HIGH_ONf, SSW_TH_PORT12_FCOFF_HIGH_ONf, SSW_TH_PORT13_FCOFF_HIGH_ONf, SSW_TH_PORT14_FCOFF_HIGH_ONf, SSW_TH_PORT15_FCOFF_HIGH_ONf, SSW_TH_PORT16_FCOFF_HIGH_ONf, SSW_TH_PORT17_FCOFF_HIGH_ONf, SSW_TH_PORT18_FCOFF_HIGH_ONf, SSW_TH_PORT19_FCOFF_HIGH_ONf\
            ,SSW_TH_PORT20_FCOFF_HIGH_ONf, SSW_TH_PORT21_FCOFF_HIGH_ONf, SSW_TH_PORT22_FCOFF_HIGH_ONf, SSW_TH_PORT23_FCOFF_HIGH_ONf, SSW_TH_PORT24_FCOFF_HIGH_ONf, SSW_TH_PORT25_FCOFF_HIGH_ONf, SSW_TH_PORT26_FCOFF_HIGH_ONf, SSW_TH_PORT27_FCOFF_HIGH_ONf, SSW_TH_PORT28_FCOFF_HIGH_ONf};
        uint16 th_port_high_fcoff_off_fieldidx[] = {SSW_TH_PORT0_FCOFF_HIGH_OFFf, SSW_TH_PORT1_FCOFF_HIGH_OFFf, SSW_TH_PORT2_FCOFF_HIGH_OFFf, SSW_TH_PORT3_FCOFF_HIGH_OFFf, SSW_TH_PORT4_FCOFF_HIGH_OFFf, SSW_TH_PORT5_FCOFF_HIGH_OFFf, SSW_TH_PORT6_FCOFF_HIGH_OFFf, SSW_TH_PORT7_FCOFF_HIGH_OFFf, SSW_TH_PORT8_FCOFF_HIGH_OFFf, SSW_TH_PORT9_FCOFF_HIGH_OFFf\
            ,SSW_TH_PORT10_FCOFF_HIGH_OFFf, SSW_TH_PORT11_FCOFF_HIGH_OFFf, SSW_TH_PORT12_FCOFF_HIGH_OFFf, SSW_TH_PORT13_FCOFF_HIGH_OFFf, SSW_TH_PORT14_FCOFF_HIGH_OFFf, SSW_TH_PORT15_FCOFF_HIGH_OFFf, SSW_TH_PORT16_FCOFF_HIGH_OFFf, SSW_TH_PORT17_FCOFF_HIGH_OFFf, SSW_TH_PORT18_FCOFF_HIGH_OFFf, SSW_TH_PORT19_FCOFF_HIGH_OFFf\
            ,SSW_TH_PORT20_FCOFF_HIGH_OFFf, SSW_TH_PORT21_FCOFF_HIGH_OFFf, SSW_TH_PORT22_FCOFF_HIGH_OFFf, SSW_TH_PORT23_FCOFF_HIGH_OFFf, SSW_TH_PORT24_FCOFF_HIGH_OFFf, SSW_TH_PORT25_FCOFF_HIGH_OFFf, SSW_TH_PORT26_FCOFF_HIGH_OFFf, SSW_TH_PORT27_FCOFF_HIGH_OFFf, SSW_TH_PORT28_FCOFF_HIGH_OFFf};
        uint16 flow_control_threshold_for_control1_regidx[] = {SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_0_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_1_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_2_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_3_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_4_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_5_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_6_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_7_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_8_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_9_CONTROL1r\
            ,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_10_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_11_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_12_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_13_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_14_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_15_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_16_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_17_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_18_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_19_CONTROL1r\
            ,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_20_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_21_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_22_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_23_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_24_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_25_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_26_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_27_CONTROL1r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_28_CONTROL1r};
        uint16 th_port_fcoff_low_fieldidx[] = {SSW_PORT0_FCOFF_LOWf, SSW_PORT1_FCOFF_LOWf, SSW_PORT2_FCOFF_LOWf, SSW_PORT3_FCOFF_LOWf, SSW_PORT4_FCOFF_LOWf, SSW_PORT5_FCOFF_LOWf, SSW_PORT6_FCOFF_LOWf, SSW_PORT7_FCOFF_LOWf, SSW_PORT8_FCOFF_LOWf, SSW_PORT9_FCOFF_LOWf\
            ,SSW_PORT10_FCOFF_LOWf, SSW_PORT11_FCOFF_LOWf, SSW_PORT12_FCOFF_LOWf, SSW_PORT13_FCOFF_LOWf, SSW_PORT14_FCOFF_LOWf, SSW_PORT15_FCOFF_LOWf, SSW_PORT16_FCOFF_LOWf, SSW_PORT17_FCOFF_LOWf, SSW_PORT18_FCOFF_LOWf, SSW_PORT19_FCOFF_LOWf\
            ,SSW_PORT20_FCOFF_LOWf, SSW_PORT21_FCOFF_LOWf, SSW_PORT22_FCOFF_LOWf, SSW_PORT23_FCOFF_LOWf, SSW_PORT24_FCOFF_LOWf, SSW_PORT25_FCOFF_LOWf, SSW_PORT26_FCOFF_LOWf, SSW_PORT27_FCOFF_LOWf, SSW_PORT28_FCOFF_LOWf};
        uint16 flow_control_threshold_for_control0_regidx[] = {SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_0_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_1_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_2_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_3_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_4_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_5_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_6_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_7_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_8_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_9_CONTROL0r\
            ,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_10_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_11_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_12_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_13_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_14_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_15_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_16_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_17_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_18_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_19_CONTROL0r\
            ,SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_20_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_21_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_22_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_23_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_24_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_25_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_26_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_27_CONTROL0r, SSW_FLOW_CONTROL_THRESHOLD_FOR_PORT_28_CONTROL0r};
        uint16 th_port_high_on_fieldidx[] = {SSW_TH_PORT0_HIGH_ONf, SSW_TH_PORT1_HIGH_ONf, SSW_TH_PORT2_HIGH_ONf, SSW_TH_PORT3_HIGH_ONf, SSW_TH_PORT4_HIGH_ONf, SSW_TH_PORT5_HIGH_ONf, SSW_TH_PORT6_HIGH_ONf, SSW_TH_PORT7_HIGH_ONf, SSW_TH_PORT8_HIGH_ONf, SSW_TH_PORT9_HIGH_ONf\
            ,SSW_TH_PORT10_HIGH_ONf, SSW_TH_PORT11_HIGH_ONf, SSW_TH_PORT12_HIGH_ONf, SSW_TH_PORT13_HIGH_ONf, SSW_TH_PORT14_HIGH_ONf, SSW_TH_PORT15_HIGH_ONf, SSW_TH_PORT16_HIGH_ONf, SSW_TH_PORT17_HIGH_ONf, SSW_TH_PORT18_HIGH_ONf, SSW_TH_PORT19_HIGH_ONf\
            ,SSW_TH_PORT20_HIGH_ONf, SSW_TH_PORT21_HIGH_ONf, SSW_TH_PORT22_HIGH_ONf, SSW_TH_PORT23_HIGH_ONf, SSW_TH_PORT24_HIGH_ONf, SSW_TH_PORT25_HIGH_ONf, SSW_TH_PORT26_HIGH_ONf, SSW_TH_PORT27_HIGH_ONf, SSW_TH_PORT28_HIGH_ONf};
        uint16 th_port_high_off_fieldidx[] = {SSW_TH_PORT0_HIGH_OFFf, SSW_TH_PORT1_HIGH_OFFf, SSW_TH_PORT2_HIGH_OFFf, SSW_TH_PORT3_HIGH_OFFf, SSW_TH_PORT4_HIGH_OFFf, SSW_TH_PORT5_HIGH_OFFf, SSW_TH_PORT6_HIGH_OFFf, SSW_TH_PORT7_HIGH_OFFf, SSW_TH_PORT8_HIGH_OFFf, SSW_TH_PORT9_HIGH_OFFf\
            ,SSW_TH_PORT10_HIGH_OFFf, SSW_TH_PORT11_HIGH_OFFf, SSW_TH_PORT12_HIGH_OFFf, SSW_TH_PORT13_HIGH_OFFf, SSW_TH_PORT14_HIGH_OFFf, SSW_TH_PORT15_HIGH_OFFf, SSW_TH_PORT16_HIGH_OFFf, SSW_TH_PORT17_HIGH_OFFf, SSW_TH_PORT18_HIGH_OFFf, SSW_TH_PORT19_HIGH_OFFf\
            ,SSW_TH_PORT20_HIGH_OFFf, SSW_TH_PORT21_HIGH_OFFf, SSW_TH_PORT22_HIGH_OFFf, SSW_TH_PORT23_HIGH_OFFf, SSW_TH_PORT24_HIGH_OFFf, SSW_TH_PORT25_HIGH_OFFf, SSW_TH_PORT26_HIGH_OFFf, SSW_TH_PORT27_HIGH_OFFf, SSW_TH_PORT28_HIGH_OFFf};
        uint16 th_port_low_fieldidx[] = {SSW_PORT0_LOWf, SSW_PORT1_LOWf, SSW_PORT2_LOWf, SSW_PORT3_LOWf, SSW_PORT4_LOWf, SSW_PORT5_LOWf, SSW_PORT6_LOWf, SSW_PORT7_LOWf, SSW_PORT8_LOWf, SSW_PORT9_LOWf\
            ,SSW_PORT10_LOWf, SSW_PORT11_LOWf, SSW_PORT12_LOWf, SSW_PORT13_LOWf, SSW_PORT14_LOWf, SSW_PORT15_LOWf, SSW_PORT16_LOWf, SSW_PORT17_LOWf, SSW_PORT18_LOWf, SSW_PORT19_LOWf\
            ,SSW_PORT20_LOWf, SSW_PORT21_LOWf, SSW_PORT22_LOWf, SSW_PORT23_LOWf, SSW_PORT24_LOWf, SSW_PORT25_LOWf, SSW_PORT26_LOWf, SSW_PORT27_LOWf, SSW_PORT28_LOWf};
        uint16 per_port_egress_drop_threshold_control_regidx[] = {SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL0r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL0r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL1r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL1r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL2r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL2r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL3r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL3r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL4r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL4r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL5r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL5r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL6r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL6r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL7r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL7r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL8r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL8r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL9r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL9r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL10r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL10r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL11r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL11r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL12r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL12r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL13r, SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL13r\
            ,SSW_PER_PORT_EGRESS_DROP_THRESHOLD_CONTROL14r};
        uint16 th_ppe_drop_on_fieldidx[] = {SSW_TH_PP0E_DROPf, SSW_TH_PP1E_DROPf, SSW_TH_PP2E_DROPf, SSW_TH_PP3E_DROPf, SSW_TH_PP4E_DROPf, SSW_TH_PP5E_DROPf, SSW_TH_PP6E_DROPf, SSW_TH_PP7E_DROPf, SSW_TH_PP8E_DROPf, SSW_TH_PP9E_DROPf\
            ,SSW_TH_PP10E_DROPf, SSW_TH_PP11E_DROPf, SSW_TH_PP12E_DROPf, SSW_TH_PP13E_DROPf, SSW_TH_PP14E_DROPf, SSW_TH_PP15E_DROPf, SSW_TH_PP16E_DROPf, SSW_TH_PP17E_DROPf, SSW_TH_PP18E_DROPf, SSW_TH_PP19E_DROPf\
            ,SSW_TH_PP20E_DROPf, SSW_TH_PP21E_DROPf, SSW_TH_PP22E_DROPf, SSW_TH_PP23E_DROPf, SSW_TH_PP24E_DROPf, SSW_TH_PP25E_DROPf, SSW_TH_PP26E_DROPf, SSW_TH_PP27E_DROPf, SSW_TH_PP28E_DROPf};

        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        reg_idx = flow_control_threshold_for_control2_regidx[port];
        temp = FCOFF_PORT_HIGHON_THRESH;
        if ((ret = reg_field_write(unit, reg_idx, th_port_high_fcoff_on_fieldidx[port], &temp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        temp = FCOFF_PORT_HIGHOFF_THRESH;
        if ((ret = reg_field_write(unit, reg_idx, th_port_high_fcoff_off_fieldidx[port], &temp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }

        reg_idx = flow_control_threshold_for_control1_regidx[port];
        temp = FCOFF_PORT_LOWON_THRESH;
        if ((ret = reg_field_write(unit, reg_idx, th_port_fcoff_low_fieldidx[port], &temp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }

        reg_idx = flow_control_threshold_for_control0_regidx[port];
        if ((ret = reg_read(unit, reg_idx, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        temp = FCON_PORT_HIGHON_THRESH;
        if ((ret = reg_field_set(unit, reg_idx, th_port_high_on_fieldidx[port], &temp, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        temp = FCON_PORT_HIGHOFF_THRESH;
        if ((ret = reg_field_set(unit, reg_idx, th_port_high_off_fieldidx[port], &temp, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        if ((ret = reg_write(unit, reg_idx, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }

        reg_idx = flow_control_threshold_for_control1_regidx[port];
        temp = FCON_PORT_LOWON_THRESH;
        if ((ret = reg_field_write(unit, reg_idx, th_port_low_fieldidx[port], &temp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        temp = EGRDROP_PORT_HIGH_THRESH;
        if ((ret = reg_field_write(unit, per_port_egress_drop_threshold_control_regidx[port], th_ppe_drop_on_fieldidx[port], &temp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
    }

    for (queue = 0; queue < HAL_MAX_NUM_OF_QUEUE(unit); queue++)
    {
        uint16 per_queue_egress_drop_threshold_control_regidx[] =
            {  SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL0r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL0r\
             , SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL1r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL1r\
             , SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL2r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL2r\
             , SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL3r, SSW_PER_QUEUE_EGRESS_DROP_THRESHOLD_CONTROL3r};
        uint16 th_pqe_drop_on_fieldidx[] = {SSW_TH_PQ0E_DROPf, SSW_TH_PQ1E_DROPf, SSW_TH_PQ2E_DROPf, SSW_TH_PQ3E_DROPf, SSW_TH_PQ4E_DROPf, SSW_TH_PQ5E_DROPf, SSW_TH_PQ6E_DROPf, SSW_TH_PQ7E_DROPf};

        temp = EGRDROP_QUEUE_HIGH_THRESH;
        if ((ret = reg_field_write(unit, per_queue_egress_drop_threshold_control_regidx[queue], th_pqe_drop_on_fieldidx[queue], &temp)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
    }

    return RT_ERR_OK;
} /* end of rtl8389_init */


/* Function Name:
 *      rtl8389_miim_read
 * Description:
 *      Get PHY registers from rtl8389 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8389_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData)
{
#if !defined(CONFIG_SDK_MODEL_MODE)
    uint32 temp, val;
    int32  ret = RT_ERR_FAILED;
#endif /* end of !defined(CONFIG_SDK_MODEL_MODE) */

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x", unit, port, page, phy_reg);
    //RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

#if defined(CONFIG_SDK_MODEL_MODE)
    /* The part code is hard core for x86 simulation in RTL8329M/RTL8389M
     * port 0 ~ 23 is FE and PHY is RTL8208GB in RTL8329M
     * port 0 ~ 23 is GE and PHY is RTL8214 in RTL8389M
     * port 24 ~ 27 is GE and PHY is RTL8214F
     */
  #define SINGLE_MEDIA_PORTS    (24)

    if (PHY_PAGE_0 == page)
    {
        switch (phy_reg)
        {
            case PHY_IDENTIFIER_1_REG:
                *pData = PHY_IDENT_OUI_03_18;
                break;
            case PHY_IDENTIFIER_2_REG:
                if (port < SINGLE_MEDIA_PORTS)  /* 0 ~ 23 is RTL8208GB */
                    *pData = (OUI_19_24_MASK << OUI_19_24_OFFSET) | PHY_IDENT_RTL8208GB;
                else    /* 24 ~ 27 is RTL8214F */
                    *pData = (OUI_19_24_MASK << OUI_19_24_OFFSET) | PHY_IDENT_RTL8214F;
                break;
        }
    }
    else if (PHY_PAGE_8 == page)
    {
        switch (phy_reg)
        {
            /* RTL8214 and RTL8214F can't identify from page 0, register 2 and 3
             * Read the page 8, register 17 to identify RTL8214 and RTL8214F
             * bit[13:10] = 0b1111 mean RTL8214
             * bit[13:10] = 0b1100 mean RTL8214F
             */
            case 17:
                if (port < SINGLE_MEDIA_PORTS)
                    *pData = 0x3c00;    /* RTL8214 PHY */
                else
                    *pData = 0x3000;    /* RTL8214F PHY */
                break;
        }
    }
    else
    {
        return RT_ERR_FAILED;
    }
#else
    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_INDATAf, &port, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, SSW_PHY_REG_ACCESS_CONTROL0r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, SSW_PHY_REG_ACCESS_CONTROL0r, 0x1);

    /* get the read operation result to temp */
    if ((ret = reg_read(unit, SSW_PHY_REG_ACCESS_CONTROL2r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* fill the DATA[15:0] from temp to pData */
    if ((ret = reg_field_get(unit, SSW_PHY_REG_ACCESS_CONTROL2r, SSW_DATAf, pData, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    PHY_SEM_UNLOCK(unit);

    RT_LOG(LOG_TRACE, MOD_HAL, "pData=0x%x", *pData);
#endif /* end of defined(CONFIG_SDK_MODEL_MODE) */

    return RT_ERR_OK;
} /* end of rtl8389_miim_read */


/* Function Name:
 *      rtl8389_miim_write
 * Description:
 *      Set PHY registers in rtl8389 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8389_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    uint32  temp, val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, phy_reg, data);
    //RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    val = 1 << (port);
    if ((ret = reg_field_write(unit, SSW_PHY_REG_ACCESS_CONTROL1r, SSW_PHYMSKf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_INDATAf, &data, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the write operation */
    if ((ret = reg_write(unit, SSW_PHY_REG_ACCESS_CONTROL0r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, SSW_PHY_REG_ACCESS_CONTROL0r, 0x1);

    PHY_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of rtl8389_miim_write */


/* Function Name:
 *      rtl8389_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8389 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8389_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data)
{
    uint32  temp, val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%x, page=0x%x, phy_reg=0x%x, data=0x%x", unit, portmask.bits[0], page, phy_reg, data);
    //RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    //if ((ret = reg_field_write(unit, SSW_PHY_REG_ACCESS_CONTROL1r, SSW_PHYMSKf, 1 << (port))) != RT_ERR_OK)
    if ((ret = reg_field_write(unit, SSW_PHY_REG_ACCESS_CONTROL1r, SSW_PHYMSKf, &portmask.bits[0])) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[4:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_INDATAf, &data, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_PHY_REG_ACCESS_CONTROL0r, SSW_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the write operation */
    if ((ret = reg_write(unit, SSW_PHY_REG_ACCESS_CONTROL0r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, SSW_PHY_REG_ACCESS_CONTROL0r, 0x1);

    PHY_SEM_UNLOCK(unit);
    return RT_ERR_OK;
} /* end of rtl8389_miim_portmask_write */


/* Function Name:
 *      rtl8389_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      1. The addr argument of RTL8389 PIE table is not continuous bits from
 *         LSB bits, we do one compiler option patch for this.
 *      2. If you don't use the RTL8389 chip, please turn off the "RTL8389"
 *         definition symbol, then performance will be improved.
 */
int32
rtl8389_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, val;
    uint32      busy;
    uint32      i;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);

#if (defined(CONFIG_SDK_RTL8389))
    /* Patch for RTL8389 PIE Table
     * PIE: ACCADDR [14:0] = {00000 & BankSel [2:0] & 0 & Addr [5:0]}
     * bit[6] must be 0 and use 0x40 check it!
     * pTable->size is 512 and need right offset 1 to check the input addr
     */
    if (SSW_PIE89_TABLEt == table)
    {
        RT_PARAM_CHK(((addr >= (pTable->size << 1) || 1 == (addr & 0x40))), RT_ERR_INPUT);
    }
    else
#endif /* end of defined(CONFIG_SDK_RTL8389) */
    {
        RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);
    }

    MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_TABLE);

    /* initialize variable */
    reg_data = 0;
    busy = 0;

    /* Command hardware to execute indirect table access
     * 0b0: not execute
     * 0b1: execute
     * Note: This bit is common used by software and hardware.
     *       When hardware completes the table access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_EXECUTEf, &val, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Table access operation
     * 0b0: write
     * 0b1: read
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_ACCMDf, &val, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* access table type */
    val = pTable->type;
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_ACCTBTPf, &val, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Select access address of the table */
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_ACCADDRf, &addr, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Write indirect control register to start the read operation */
    if ((ret = reg_write(unit, SSW_INDIRECT_CONTROLr, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Wait operation completed */
    do
    {
        if ((ret = reg_field_read(unit, SSW_INDIRECT_CONTROLr, SSW_EXECUTEf, &busy)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }
    } while (busy);

    /* Read table data from indirect data register */
    for (i = 0; i < pTable->datareg_num; i++)
    {
        if ((ret = reg_read(unit, SSW_INDIRECT_DATA0_FOR_CPUr + i, pData + i)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }
    }

    MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);

    return RT_ERR_OK;
} /* end of rtl8389_table_read */


/* Function Name:
 *      rtl8389_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      1. The addr argument of RTL8389 PIE table is not continuous bits from
 *         LSB bits, we do one compiler option patch for this.
 *      2. If you don't use the RTL8389 chip, please turn off the "RTL8389"
 *         definition symbol, then performance will be improved.
 */
int32
rtl8389_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, val;
    uint32      busy;
    uint32      i;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);

#if (defined(CONFIG_SDK_RTL8389))
    /* Patch for RTL8389 PIE Table
     * PIE: ACCADDR [14:0] = {00000 & BankSel [2:0] & 0 & Addr [5:0]}
     * bit[6] must be 0 and use 0x40 check it!
     * pTable->size is 512 and need right offset 1 to check the input addr
     */
    if (SSW_PIE89_TABLEt == table)
    {
        RT_PARAM_CHK(((addr >= (pTable->size << 1) || 1 == (addr & 0x40))), RT_ERR_INPUT);
    }
    else
#endif /* end of defined(CONFIG_SDK_RTL8389) */
    {
        RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);
    }

    MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_TABLE);

    /* initialize variable */
    reg_data = 0;
    busy = 0;

    /* Write pre-configure table data to indirect data register */
    for (i = 0; i < pTable->datareg_num; i++)
    {
        val = *(pData + i);
        if ((ret = reg_write(unit, SSW_INDIRECT_DATA0_FOR_CPUr + i, &val)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }
    }

    /* Command hardware to execute indirect table access
     * 0b0: not execute
     * 0b1: execute
     * Note: This bit is common used by software and hardware.
     *       When hardware completes the table access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_EXECUTEf, &val, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Table access operation
     * 0b0: write
     * 0b1: read
     */
    val = 0;
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_ACCMDf, &val, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* access table type */
    val = pTable->type;
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_ACCTBTPf, &val, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Select access address of the table */
    if ((ret = reg_field_set(unit, SSW_INDIRECT_CONTROLr, SSW_ACCADDRf, &addr, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Write indirect control register to start the write operation */
    if ((ret = reg_write(unit, SSW_INDIRECT_CONTROLr, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
        return ret;
    }

    /* Wait operation completed */
    do
    {
        if ((ret = reg_field_read(unit, SSW_INDIRECT_CONTROLr, SSW_EXECUTEf, &busy)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }
    } while (busy);

    MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);

    return RT_ERR_OK;
} /* end of rtl8389_table_write */
#endif


#if defined(CONFIG_SDK_RTL8328)
/* Function Name:
 *      rtl8328_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8328_port_probe(uint32 unit)
{
    return RT_ERR_OK;
} /* end of rtl8328_port_probe */


/* Function Name:
 *      rtl8328_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8328_init(uint32 unit)
{
/* Global */
#define SYSDROPTH           0x4DE
#define SRXHTH_ON           0x128   /* 0x170, Old value in SDK v2.0.1 */
#define SRXHTH_OFF          0xF8    /* 0x140, Old value in SDK v2.0.1 */
#define SRXLTH_ON           0xE0    /* 0x120, Old value in SDK v2.0.1 */
#define SRXLTH_OFF          0xA0    /* 0xE0 , Old value in SDK v2.0.1 */
#define FCOFF_SRXHTH_ON     0x1E0
#define FCOFF_SRXHTH_OFF    0x1B0
#define FCOFF_SRXLTH_ON     0x190
#define FCOFF_SRXLTH_OFF    0x160
#define STXHTH_ON           0x400   /* System Tx High On for DP0 */
#define STXHTH_OFF          0x3D0   /* System Tx High Off for DP0 */
#define STXLTH_ON           0x292   /* System Tx Low On for DP0 */
#define STXLTH_OFF          0x262   /* System Tx Low Off for DP0 */
#define STXHTH_ON_L1        0x39C   /* System Tx High On for DP1 */
#define STXHTH_OFF_L1       0x36C   /* System Tx High Off for DP1 */
#define STXLTH_ON_L1        0x24C   /* System Tx Low On for DP1 */
#define STXLTH_OFF_L1       0x21C   /* System Tx Low Off for DP1 */
#define STXHTH_ON_L2        0x338   /* System Tx High On for DP2 */
#define STXHTH_OFF_L2       0x308   /* System Tx High Off for DP2 */
#define STXLTH_ON_L2        0x206   /* System Tx Low On for DP2 */
#define STXLTH_OFF_L2       0x1D6   /* System Tx Low Off for DP2 */
/* Per-port */
#define CPAGETH_ON_FE       0x70
#define CPAGETH_OFF_FE      0x40
#define GPAGETH_ON_FE       0x12
#define GPAGETH_OFF_FE      0x0C
#define CPAGETH_ON_GE       0xC0
#define CPAGETH_OFF_GE      0x80
#define GPAGETH_ON_GE       0x36    /* 0x24, Old value in SDK v2.0.1 */
#define GPAGETH_OFF_GE      0x2A    /* 0x1E, Old value in SDK v2.0.1 */
#define PTXHTH_ON           0xA8    /* Port Tx High On for DP0 */
#define PTXLTH_ON           0x2E    /* Port Tx Low On for DP0 */
#define PTXHTH_ON_L1        0x94    /* Port Tx High On for DP1 */
#define PTXLTH_ON_L1        0x22    /* Port Tx Low On for DP1 */
#define PTXHTH_ON_L2        0x80    /* Port Tx High On for DP2 */
#define PTXLTH_ON_L2        0x18    /* Port Tx Low On for DP2 */
#define QTXHTH_ON           0xB2    /* Queue Tx High On for Ethernet port */
#define QTXLTH_ON           0x18    /* Queue Tx Low On for Ethernet port */
#define CPU_Q0TXHTH_ON      0x48    /* Queue0 Tx High On for CPU port */
#define CPU_Q0TXLTH_ON      0x18    /* Queue0 Tx Low On for CPU port */
#define CPU_Q1TXHTH_ON      0x4E    /* Queue1 Tx High On for CPU port */
#define CPU_Q1TXLTH_ON      0x20    /* Queue1 Tx Low On for CPU port */
#define CPU_Q2TXHTH_ON      0x54    /* Queue2 Tx High On for CPU port */
#define CPU_Q2TXLTH_ON      0x26    /* Queue2 Tx Low On for CPU port */
#define CPU_Q3TXHTH_ON      0x5A    /* Queue3 Tx High On for CPU port */
#define CPU_Q3TXLTH_ON      0x2C    /* Queue3 Tx Low On for CPU port */
#define CPU_Q4TXHTH_ON      0x60    /* Queue4 Tx High On for CPU port */
#define CPU_Q4TXLTH_ON      0x32    /* Queue4 Tx Low On for CPU port */
#define CPU_Q5TXHTH_ON      0x66    /* Queue5 Tx High On for CPU port */
#define CPU_Q5TXLTH_ON      0x38    /* Queue5 Tx Low On for CPU port */
#define CPU_Q6TXHTH_ON      0x6C    /* Queue6 Tx High On for CPU port */
#define CPU_Q6TXLTH_ON      0x3E    /* Queue6 Tx Low On for CPU port */
#define CPU_Q7TXHTH_ON      0x72    /* Queue7 Tx High On for CPU port */
#define CPU_Q7TXLTH_ON      0x44    /* Queue7 Tx Low On for CPU port */

    int32       ret = RT_ERR_FAILED;
    uint32      queue, max_queue, value;
    rtk_port_t  port, max_port, cpu_port;

    /*Sel6b Default value change to 0b0*/
    value = 0;
    if ((ret = reg_field_write(unit, ESW_GLOBAL_MAC_L2_MISC0r, ESW_SEL6Bf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    /* Configure l2 multicast lookup by VID */
    value = 0;
    if((ret = reg_field_write(unit, ESW_ADDRESS_LOOKUP_BASED_ON_VID_OR_FID_DECISION_CONTROLr,
                    ESW_MLFIDf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
    /* register rtl8231 mdc semphore callback function */
    if ((ret = drv_rtl8231_mdcSem_register(unit, rtl83xx_mdcSem_callback)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }
#endif

    /* Initialize the global threshold value of chip buffer */
    value = SYSDROPTH;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_DROP_THRESHOLD_CONTROLr, ESW_SYSDROPTHf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = SRXHTH_ON;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, ESW_SRXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = SRXHTH_OFF;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL0r, ESW_SRXHTH_OFFf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = SRXLTH_ON;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, ESW_SRXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = SRXLTH_OFF;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_RX_USED_THRESHOLD_CONTROL1r, ESW_SRXLTH_OFFf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = FCOFF_SRXHTH_ON;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, ESW_FCOFF_SRXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = FCOFF_SRXHTH_OFF;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL0r, ESW_FCOFF_SRXHTH_OFFf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = FCOFF_SRXLTH_ON;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, ESW_FCOFF_SRXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = FCOFF_SRXLTH_OFF;
    if ((ret = reg_field_write(unit, ESW_PUBLIC_PAGE_FLOW_CONTROL_OFF_RX_USED_THRESHOLD_CONTROL1r, ESW_FCOFF_SRXLTH_OFFf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }

    value = STXHTH_ON;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL0r, ESW_STXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXHTH_OFF;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL0r, ESW_STXHTH_OFFf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXLTH_ON;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL1r, ESW_STXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXLTH_OFF;
    if ((ret = reg_field_write(unit, ESW_SYSTEM_PAGE_TX_USED_THRESHOLD_CONTROL1r, ESW_STXLTH_OFFf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXHTH_ON_L1;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL0r, ESW_STXHTHON_L1_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXHTH_OFF_L1;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL0r, ESW_STXHTHOFF_L1_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXLTH_ON_L1;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL1r, ESW_STXLTHON_L1_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXLTH_OFF_L1;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL1r, ESW_STXLTHOFF_L1_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXHTH_ON_L2;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL2r, ESW_STXHTHON_L2_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXHTH_OFF_L2;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL2r, ESW_STXHTHOFF_L2_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXLTH_ON_L2;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL3r, ESW_STXLTHON_L2_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = STXLTH_OFF_L2;
    if ((ret = reg_field_write(unit, ESW_ENHANCED_PAUSE_SYSTEM_PAGE_THRESHOLD_CONTROL3r, ESW_STXLTHOFF_L2_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }

    /* Initialize the per-port threshold value of chip buffer */
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port <= max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if (HAL_IS_FE_PORT(unit, port))
        {
            value = CPAGETH_ON_FE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r,
                port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_ONf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
            value = CPAGETH_OFF_FE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r,
                port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_OFFf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
            value = GPAGETH_ON_FE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r,
                port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_ONf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
            value = GPAGETH_OFF_FE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r,
                port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_OFFf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
        }
        if (HAL_IS_GE_PORT(unit, port) || HAL_IS_CPU_PORT(unit, port))
        {
            value = CPAGETH_ON_GE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r,
                port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_ONf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
            value = CPAGETH_OFF_GE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL0r,
                port, REG_ARRAY_INDEX_NONE, ESW_CPAGETH_OFFf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
            value = GPAGETH_ON_GE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r,
                port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_ONf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
            value = GPAGETH_OFF_GE;
            if((ret = reg_array_field_write(unit, ESW_PORT_RX_PAGE_THRESHOLD_CONTROL1r,
                port, REG_ARRAY_INDEX_NONE, ESW_GPAGETH_OFFf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
        }
    }

#if defined(CONFIG_SDK_WA_EEE_COMPATIBLE)
    /* configure RX IPG will reduce 4 tx clock cycle and modify tx rate check for EEE compatible issue */
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port <= max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        value = 1;
        if ((ret = reg_array_field_write(unit, ESW_PORT_L2_MISC0r, port, REG_ARRAY_INDEX_NONE,
                                        ESW_SHORTRXITFSPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL), "");
            return ret;
        }
        value = 1;
        if ((ret = reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL0r, port, REG_ARRAY_INDEX_NONE,
                                        ESW_EEE_REQ_SET_2_0f, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL), "");
            return ret;
        }
        value = 0x20;
        if ((ret = reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL2r, port, REG_ARRAY_INDEX_NONE,
                                        ESW_EEE_TX_THRf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL), "");
            return ret;
        }
    }

    /* Adjust Giga Tw timer parameter for eee */
    for (port = 0; port <= max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        if (HAL_IS_GE_PORT(unit, port) || HAL_IS_CPU_PORT(unit, port))
        {
            value = 0x14;
            if ((ret = reg_array_field_write(unit, ESW_PORT_EEE_MAC_CONTROL3r, port, REG_ARRAY_INDEX_NONE,
                                            ESW_EEE_TIMER_TW_GIGAf, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL), "");
                return ret;
            }
        }
    }
#endif

    /* Configure all ethernet ports
     * - Turn off the L3/L4 checksum error drop
     * - Turn on the L2 CRC error drop
     * - congest-avoidance port-threshold and queue-threshold
     */
    max_port = HAL_GET_MAX_PORT(unit);
    max_queue = HAL_MAX_NUM_OF_QUEUE(unit);
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }

        value = 0;
        if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr,
            port, REG_ARRAY_INDEX_NONE, ESW_PL4CSKERRDROPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL), "");
            return ret;
        }

        value = 0;
        if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr,
            port, REG_ARRAY_INDEX_NONE, ESW_PL3CSKERRDROPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL), "");
            return ret;
        }

        value = 1;
        if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr,
            port, REG_ARRAY_INDEX_NONE, ESW_PL2CRCERRDROPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL), "");
            return ret;
        }

        value = PTXHTH_ON;
        if ((ret = reg_array_field_write(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL0r, port
                            , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ONf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        value = PTXLTH_ON;
        if ((ret = reg_array_field_write(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL1r, port
                            , REG_ARRAY_INDEX_NONE , ESW_TXLTH_ONf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        value = PTXHTH_ON_L1;
        if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL0r, port
                            , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ON_L1_EPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        value = PTXLTH_ON_L1;
        if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL1r, port
                            , REG_ARRAY_INDEX_NONE , ESW_TXLTH_ON_L1_EPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        value = PTXHTH_ON_L2;
        if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL2r, port
                            , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ON_L2_EPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        value = PTXLTH_ON_L2;
        if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL3r, port
                            , REG_ARRAY_INDEX_NONE , ESW_TXLTH_ON_L2_EPf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
            return ret;
        }
        value = 0xA00;
        if ((ret = reg_array_field_write(unit, ESW_PORT_LEAKY_BUKET_CONTROLr, port, REG_ARRAY_INDEX_NONE
                            , ESW_MINBKTBWHIGHTHf, &value)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
            return ret;
        }
        for (queue = 0; queue < max_queue; queue++)
        {
            value = QTXHTH_ON;
            if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE0_TX_PAGE_THRESHOLD_CONTROL0r+2*queue, port, REG_ARRAY_INDEX_NONE
                                , ESW_Q0TXHTH_ONf+4*queue, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
                return ret;
            }
            value = QTXLTH_ON;
            if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE0_TX_PAGE_THRESHOLD_CONTROL1r+2*queue, port, REG_ARRAY_INDEX_NONE
                                , ESW_Q0TXLTH_ONf+4*queue, &value)) != RT_ERR_OK)
            {
                RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
                return ret;
            }
        }
    }

    /* Configure CPU port
     * - Turn off the L3/L4 checksum error drop
     * - Turn off the L2 CRC error drop
     * - congest-avoidance port-threshold and queue-threshold
     */
    /* E0007119 */
    cpu_port = HAL_GET_CPU_PORT(unit);
    value = 0;
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr,
        cpu_port, REG_ARRAY_INDEX_NONE, ESW_PL4CSKERRDROPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    value = 0;
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr,
        cpu_port, REG_ARRAY_INDEX_NONE, ESW_PL3CSKERRDROPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    value = 0;
    if((ret = reg_array_field_write(unit, ESW_PORT_PACKET_PARSER_CHECKSUM_CONTROLr,
        cpu_port, REG_ARRAY_INDEX_NONE, ESW_PL2CRCERRDROPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }
    /* E0007119 */

    /* congest-avoidance port-threshold */
    value = PTXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port
                        , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = PTXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port
                        , REG_ARRAY_INDEX_NONE , ESW_TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = PTXHTH_ON_L1;
    if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL0r, cpu_port
                        , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ON_L1_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = PTXLTH_ON_L1;
    if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL1r, cpu_port
                        , REG_ARRAY_INDEX_NONE , ESW_TXLTH_ON_L1_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = PTXHTH_ON_L2;
    if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL2r, cpu_port
                        , REG_ARRAY_INDEX_NONE , ESW_TXHTH_ON_L2_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = PTXLTH_ON_L2;
    if ((ret = reg_array_field_write(unit, ESW_PORT_ENHANCED_PAUSE_THRESHOLD_CONTROL3r, cpu_port
                        , REG_ARRAY_INDEX_NONE , ESW_TXLTH_ON_L2_EPf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    /* congest-avoidance queue-threshold */
    value = CPU_Q0TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE0_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q0TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q0TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE0_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q0TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q1TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE1_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q1TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q1TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE1_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q1TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q2TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE2_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q2TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q2TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE2_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q2TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q3TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE3_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q3TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q3TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE3_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q3TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q4TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE4_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q4TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q4TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE4_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q4TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q5TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE5_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q5TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q5TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE5_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q5TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q6TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE6_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q6TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q6TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE6_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q6TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q7TXHTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE7_TX_PAGE_THRESHOLD_CONTROL0r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q7TXHTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = CPU_Q7TXLTH_ON;
    if ((ret = reg_array_field_write(unit, ESW_PORT_QUEUE7_TX_PAGE_THRESHOLD_CONTROL1r, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_Q7TXLTH_ONf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL|MOD_FLOWCTRL), "");
        return ret;
    }
    value = 0xA00;
    if ((ret = reg_array_field_write(unit, ESW_PORT_LEAKY_BUKET_CONTROLr, cpu_port, REG_ARRAY_INDEX_NONE
                        , ESW_MINBKTBWHIGHTHf, &value)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_FLOWCTRL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of rtl8328_init */

/* Function Name:
 *      rtl8328_miim_read
 * Description:
 *      Get PHY registers from rtl8328 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 *      4. Support access by PHY_ADDRESS, DATA_SOURCE = 0b0 (from register 2) and
 *         REG_TYPE (0b0: Clause 22)
 */
int32
rtl8328_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData)
{
    uint32 temp, val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, page=0x%x, phy_reg=0x%x", unit, port, page, phy_reg);

    //RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * PHYADDR[4:0] is the PHY address; Valid if WRITEPHYMETHOD = 0b0
     */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PHYADDR_4_0f, &port, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_REG_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PAGE_6_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select data source from
     * 0b0: when trig write PHY register, the data source from PHY REG Access Control Register 2
     * 0b1: when trig write PHY register, the data source from PHY property configure register
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_DATA_SOURCEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Clause22 PHY Register
     * 0b1: Clause45 PHY Register
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PHY_REG_TYPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select write PHY method
     * 0b0: Write PHY register used PHY address, this Method can only write one register of one PHY at one time
     * 0b1: Write PHY register used Port Mask, this Method can write the same one register of several PHY at one time
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_WRITEPHYMETHODf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b11: read
     * 0b01: write
     */
    val = 3;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, ESW_PHY_REG_ACCESS_CONTROL0r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, ESW_PHY_REG_ACCESS_CONTROL0r, 0x1);

    /* get the read operation result to temp */
    if ((ret = reg_read(unit, ESW_PHY_REG_ACCESS_CONTROL2r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* fill the DATA[15:0] from temp to pData */
    if ((ret = reg_field_get(unit, ESW_PHY_REG_ACCESS_CONTROL2r, ESW_DATAf, pData, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8328_miim_read */

/* Function Name:
 *      rtl8328_miim_write
 * Description:
 *      Set PHY registers in rtl8328 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 *      4. Support access by PHY_ADDRESS, DATA_SOURCE = 0b0 (from register 2) and
 *         REG_TYPE (0b0: Clause 22)
 */
int32
rtl8328_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    uint32  temp, val;
    int32   ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, phy_reg, data);
    //RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Input parameters:
     * Address for MMD or Data content read/write from/to PHY Reg.
     */
    if ((ret = reg_field_write(unit, ESW_PHY_REG_ACCESS_CONTROL2r, ESW_DATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Select PHY to access */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PHYADDR_4_0f, &port, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_REG_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PAGE_6_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select data source from
     * 0b0: when trig write PHY register, the data source from PHY REG Access Control Register 2
     * 0b1: when trig write PHY register, the data source from PHY property configure register
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_DATA_SOURCEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Clause22 PHY Register
     * 0b1: Clause45 PHY Register
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PHY_REG_TYPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select write PHY method
     * 0b0: Write PHY register used PHY address, this Method can only write one register of one PHY at one time
     * 0b1: Write PHY register used Port Mask, this Method can write the same one register of several PHY at one time
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_WRITEPHYMETHODf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b11: read
     * 0b01: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the write operation */
    if ((ret = reg_write(unit, ESW_PHY_REG_ACCESS_CONTROL0r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, ESW_PHY_REG_ACCESS_CONTROL0r, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8328_miim_write */

/* Function Name:
 *      rtl8328_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8328 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8328_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data)
{
    uint32  temp, val;
    int32   ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, portmask=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, portmask.bits[0], page, phy_reg, data);
    //RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Input parameters:
     * Address for MMD or Data content read/write from/to PHY Reg.
     */
    if ((ret = reg_field_write(unit, ESW_PHY_REG_ACCESS_CONTROL2r, ESW_DATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = reg_field_write(unit, ESW_PHY_REG_ACCESS_CONTROL1r, ESW_PHYMSK_28_0f, &portmask.bits[0])) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Select register number to access */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_REG_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PAGE_6_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select data source from
     * 0b0: when trig write PHY register, the data source from PHY REG Access Control Register 2
     * 0b1: when trig write PHY register, the data source from PHY property configure register
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_DATA_SOURCEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Clause22 PHY Register
     * 0b1: Clause45 PHY Register
     */
    val = 0;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_PHY_REG_TYPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select write PHY method
     * 0b0: Write PHY register used PHY address, this Method can only write one register of one PHY at one time
     * 0b1: Write PHY register used Port Mask, this Method can write the same one register of several PHY at one time
     */
    val = 1;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_WRITEPHYMETHODf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b11: read
     * 0b01: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, ESW_PHY_REG_ACCESS_CONTROL0r, ESW_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the write operation */
    if ((ret = reg_write(unit, ESW_PHY_REG_ACCESS_CONTROL0r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, ESW_PHY_REG_ACCESS_CONTROL0r, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8328_table_read */

/* Function Name:
 *      rtl8328_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
int32
rtl8328_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, val;
    uint32      busy;
    uint32      i;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);

    if (table < ESW_TSSCR0t)
    {
        /* Table Indirect Access via INCR & INDR */
        MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_TABLE);

        /* initialize variable */
        reg_data = 0;
        busy = 0;

        /* Command to configure who occupied table access
         * 0b0: Occupied by others
         * 0b1: Only occupied by CPU
         * Note: RRCP can't set this bit.
         */
        val = 1;
        if ((ret = reg_field_write(unit, ESW_INDIRECT_CONTROLr, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }
        val = 1;
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_CPUOCCUPf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Command hardware to execute indirect table access
         * 0b0: not execute
         * 0b1: execute
         * Note: This bit is common used by software and hardware.
         *       When hardware completes the table access, it will clear this bit.
         */
        val = 1;
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_EXECUTEf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Table access operation
         * 0b0: write
         * 0b1: read
         */
        val = 1;
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_ACCMDf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* access table type */
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_ACCTBTPf, &(pTable->type), &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Select access address of the table */
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_ACCADDRf, &addr, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Write indirect control register to start the read operation */
        if ((ret = reg_write(unit, ESW_INDIRECT_CONTROLr, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Wait operation completed */
        do
        {
            if ((ret = reg_field_read(unit, ESW_INDIRECT_CONTROLr, ESW_EXECUTEf, &busy)) != RT_ERR_OK)
            {
                MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                return ret;
            }
        } while (busy);

        if (ESW_ACL_COUNTERt == table)
        {
            uint32  indirect_ctrl_reg_val, indirect_data_reg_val, retry;

            /* save the indirect_ctrl_reg value */
            indirect_ctrl_reg_val = reg_data;
            indirect_data_reg_val = 0;
            retry = 0;

            /* Read table data from indirect data register */
            for (i = 0; i < pTable->datareg_num; i++)
            {
                if ((ret = reg_read(unit, ESW_INDIRECT_DATA0_FOR_CPUr + i, pData + i)) != RT_ERR_OK)
                {
                    MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                    return ret;
                }
                indirect_data_reg_val |= *(pData + i);
            }

            while ((indirect_data_reg_val == 0) && (++retry <= 15))
            {
                /* Write indirect control register to start the read operation */
                if ((ret = reg_write(unit, ESW_INDIRECT_CONTROLr, &indirect_ctrl_reg_val)) != RT_ERR_OK)
                {
                    MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                    return ret;
                }

                /* Wait operation completed */
                do
                {
                    if ((ret = reg_field_read(unit, ESW_INDIRECT_CONTROLr, ESW_EXECUTEf, &busy)) != RT_ERR_OK)
                    {
                        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                        return ret;
                    }
                } while (busy);

                /* Read table data from indirect data register */
                indirect_data_reg_val = 0;
                for (i = 0; i < pTable->datareg_num; i++)
                {
                    if ((ret = reg_read(unit, ESW_INDIRECT_DATA0_FOR_CPUr + i, pData + i)) != RT_ERR_OK)
                    {
                        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                        return ret;
                    }
                    indirect_data_reg_val |= *(pData + i);
                }
            }
        }
        else
        {
            /* Read table data from indirect data register */
            for (i = 0; i < pTable->datareg_num; i++)
            {
                if ((ret = reg_read(unit, ESW_INDIRECT_DATA0_FOR_CPUr + i, pData + i)) != RT_ERR_OK)
                {
                    MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                    return ret;
                }
            }
        }

        val = 0;
        if ((ret = reg_field_write(unit, ESW_INDIRECT_CONTROLr, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
    }
    else
    {
        /* Table Indirect Access via TRAFFSTORMFILTER_INDREACCESS_REG0 & TRAFFSTORMFILTER_INDREACCESS_REG1 */
        MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_STORM);

        /* initialize variable */
        reg_data = 0;
        busy = 0;

        /* Command hardware to access storm filter's register
         * 1: request to access storm filter's register
         *    when finished access, the hw will self-clear this bit to 0
         *    the sw must make sure indirect_access=0 when setting this register
         */
        val = 1;
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_INDIRECT_ACCESSf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Table access operation
         * 0b0: read
         * 0b1: write
         */
        val = 0;
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_ACCESS_RWf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* access table type */
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_ACCESS_TYPEf, &(pTable->type), &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Select access address of the table */
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_ACCESS_PORTf, &addr, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Write indirect control register to start the read operation */
        if ((ret = reg_write(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Wait operation completed */
        do
        {
            if ((ret = reg_field_read(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_INDIRECT_ACCESSf, &busy)) != RT_ERR_OK)
            {
                MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
                return ret;
            }
        } while (busy);

        /* Read table data from indirect data register */
        for (i = 0; i < pTable->datareg_num; i++)
        {
            if ((ret = reg_read(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG1r + i, pData + i)) != RT_ERR_OK)
            {
                MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
                return ret;
            }
        }

        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
    }
    return RT_ERR_OK;
} /* end of rtl8328_table_read */

/* Function Name:
 *      rtl8328_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
int32
rtl8328_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, val;
    uint32      busy;
    uint32      i;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);
    /* Check for ACL counter clear all action */
    if (ESW_ACL_COUNTERt != table)
        RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);

    if (table < ESW_TSSCR0t)
    {
        /* Table Indirect Access via INCR & INDR */
        MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_TABLE);

        /* initialize variable */
        reg_data = 0;
        busy = 0;

        val = 1;
        if ((ret = reg_field_write(unit, ESW_INDIRECT_CONTROLr, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Write pre-configure table data to indirect data register */
        for (i = 0; i < pTable->datareg_num; i++)
        {
            if ((ret = reg_write(unit, ESW_INDIRECT_DATA0_FOR_CPUr + i, (pData + i))) != RT_ERR_OK)
            {
                MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                return ret;
            }
        }

        val = 1;
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_CPUOCCUPf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }
        /* Command hardware to execute indirect table access
         * 0b0: not execute
         * 0b1: execute
         * Note: This bit is common used by software and hardware.
         *       When hardware completes the table access, it will clear this bit.
         */
        val = 1;
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_EXECUTEf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Table access operation
         * 0b0: write
         * 0b1: read
         */
        val = 0;
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_ACCMDf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* access table type */
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_ACCTBTPf, &(pTable->type), &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Select access address of the table */
        if ((ret = reg_field_set(unit, ESW_INDIRECT_CONTROLr, ESW_ACCADDRf, &addr, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Write indirect control register to start the write operation */
        if ((ret = reg_write(unit, ESW_INDIRECT_CONTROLr, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        /* Wait operation completed */
        do
        {
            if ((ret = reg_field_read(unit, ESW_INDIRECT_CONTROLr, ESW_EXECUTEf, &busy)) != RT_ERR_OK)
            {
                MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
                return ret;
            }
        } while (busy);

        val = 0;
        if ((ret = reg_field_write(unit, ESW_INDIRECT_CONTROLr, ESW_CPUOCCUPf, &val)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
            return ret;
        }

        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_TABLE);
    }
    else
    {
        /* Table Indirect Access via TRAFFSTORMFILTER_INDREACCESS_REG0 & TRAFFSTORMFILTER_INDREACCESS_REG1 */
        MEM_SEM_LOCK(unit, INDIRECT_CTRL_GROUP_STORM);

        /* initialize variable */
        reg_data = 0;
        busy = 0;

        /* Write pre-configure table data to indirect data register */
        for (i = 0; i < pTable->datareg_num; i++)
        {
            if ((ret = reg_write(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG1r + i, (pData + i))) != RT_ERR_OK)
            {
                MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
                return ret;
            }
        }

        /* Command hardware to access storm filter's register
         * 1: request to access storm filter's register
         *    when finished access, the hw will self-clear this bit to 0
         *    the sw must make sure indirect_access=0 when setting this register
         */
        val = 1;
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_INDIRECT_ACCESSf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Table access operation
         * 0b0: read
         * 0b1: write
         */
        val = 1;
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_ACCESS_RWf, &val, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* access table type */
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_ACCESS_TYPEf, &(pTable->type), &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Select access address of the table */
        if ((ret = reg_field_set(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_ACCESS_PORTf, &addr, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Write indirect control register to start the write operation */
        if ((ret = reg_write(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, &reg_data)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
            return ret;
        }

        /* Wait operation completed */
        do
        {
            if ((ret = reg_field_read(unit, ESW_TRAFFSTORMFILTER_INDREACCESS_REG0r, ESW_INDIRECT_ACCESSf, &busy)) != RT_ERR_OK)
            {
                MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
                return ret;
            }
        } while (busy);

        MEM_SEM_UNLOCK(unit, INDIRECT_CTRL_GROUP_STORM);
    }
    return RT_ERR_OK;
} /* end of rtl8328_table_write */
#endif

#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
/* Function Name:
 *      rtl83xx_mdcSem_callback
 * Description:
 *      Take/Give MDC/MDIO semaphore resource by lower layer in specified device.
 * Input:
 *      unit - unit id
 *      type - semaphore type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 * Note:
 *      The type value 0 mean lock the semaphore; 1 mean unlock the semaphore.
 */
static int32
rtl83xx_mdcSem_callback(uint32 unit, uint32 type)
{
    if (type == 0) /* LOCK */
    {
        PHY_SEM_LOCK(unit);
    }
    else
    {
        PHY_SEM_UNLOCK(unit);
    }

    return RT_ERR_OK;
} /* end of rtl83xx_mdcSem_callback */
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      rtl8390_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8390_port_probe(uint32 unit)
{
    return RT_ERR_OK;
} /* end of rtl8390_port_probe */

/* Function Name:
 *      rtl8390_serdes_init
 * Description:
 *      Initialize the specified serdes settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8390_serdes_init(uint32 unit)
{
    uint32  val;

    if ((RTL8392M_CHIP_ID >> 16) == (HAL_GET_CHIP_ID(unit) >> 16))
    {
        ioal_mem32_read(unit, 0xAF40, &val);
        val &= ~(0xF << 24);
        val |= 0xF << 24;
        ioal_mem32_write(unit, 0xAF40, val);
    }

    return RT_ERR_OK;
}   /* end of rtl8390_serdes_init */

/* Function Name:
 *      rtl8390_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8390_init(uint32 unit)
{
    int32       ret = RT_ERR_FAILED;
#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)

    /* register rtl8231 mdc semphore callback function */
    if ((ret = drv_rtl8231_mdcSem_register(unit, rtl83xx_mdcSem_callback)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }
#endif

    if ((ret = rtl8390_serdes_init(unit)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "Init serdes fail");
        return ret;
    }

    /* Initialize the threshold value of chip buffer if need */
    return RT_ERR_OK;
} /* end of rtl8390_init */


/* Function Name:
 *      rtl8390_miim_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData)
{
    uint32 temp;
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, page=0x%x, phy_reg=0x%x", unit, port, page, phy_reg);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * INDATA[5:0] is the PHY address WHEN RWOP = 0b0
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &port)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    val = 0x1ff;
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* get the read operation result to pData */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_DATAf, pData)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8390_miim_read */


/* Function Name:
 *      rtl8390_miim_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    rtk_port_t  max_port, portId;
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, phy_reg, data);
    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    max_port = HAL_GET_MAX_PORT(unit);
    for (portId = 0; portId < max_port; portId++)
    {
        if (!HAL_IS_PORT_EXIST(unit, portId))
        {
            continue;
        }
        if(port == portId)
            val = 1;
        else
            val = 0;
        if ((ret = reg_array_field_write(unit,
                              CYPRESS_PHYREG_PORT_CTRLr,
                              portId,
                              REG_ARRAY_INDEX_NONE,
                              CYPRESS_PHYMSKf,
                              &val)) != RT_ERR_OK)
        {
            PHY_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0x1ff;
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_write */

/* Function Name:
 *      rtl8390_miim_park_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_park_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData)
{
    uint32 temp;
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, page=0x%x, phy_reg=0x%x", unit, port, page, phy_reg);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((parkPage > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * INDATA[5:0] is the PHY address WHEN RWOP = 0b0
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &port)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    val = 0x1ff;
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* get the read operation result to pData */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_DATAf, pData)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8390_miim_park_read */


/* Function Name:
 *      rtl8390_miim_park_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_park_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data)
{
    rtk_port_t  max_port, portId;
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, phy_reg, data);
    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((parkPage > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    max_port = HAL_GET_MAX_PORT(unit);
    for (portId = 0; portId < max_port; portId++)
    {
        if (!HAL_IS_PORT_EXIST(unit, portId))
        {
            continue;
        }
        if(port == portId)
            val = 1;
        else
            val = 0;
        if ((ret = reg_array_field_write(unit,
                              CYPRESS_PHYREG_PORT_CTRLr,
                              portId,
                              REG_ARRAY_INDEX_NONE,
                              CYPRESS_PHYMSKf,
                              &val)) != RT_ERR_OK)
        {
            PHY_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0x1ff;
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_park_write */

/* Function Name:
 *      rtl8390_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8390 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data)
{
    rtk_port_t  port, max_port;
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%x, page=0x%x, phy_reg=0x%x, data=0x%x", unit, portmask.bits[0], page, phy_reg, data);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        if(RTK_PORTMASK_IS_PORT_SET(portmask, port))
            val = 1;
        else
            val = 0;

        if ((ret = reg_array_field_write(unit,
                              CYPRESS_PHYREG_PORT_CTRLr,
                              port,
                              REG_ARRAY_INDEX_NONE,
                              CYPRESS_PHYMSKf,
                              &val)) != RT_ERR_OK)
        {
            PHY_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0x1ff;
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_portmask_write */

/* Function Name:
 *      rtl8390_miim_broadcast_write
 * Description:
 *      Set PHY registers in rtl8390 family chips with broadcast mechanism.
 * Input:
 *      unit    - unit id
 *      page    - page id
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. page valid range is 0 ~ 31
 *      2. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_broadcast_write(
    uint32      unit,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, reg=0x%x \
           data=0x%x", unit, page, phy_reg, data);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    val = 0x1ff;
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* check the FAIL bit */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_FAILf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if(val == 1)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PHY_ACCESS_FAIL;
    }

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_write */


/* Function Name:
 *      rtl8390_miim_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_extParkPage_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mainPage,
    uint32      extPage,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData)
{
    uint32 temp;
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x",
           unit, port, mainPage, extPage, parkPage, phy_reg);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    //RT_PARAM_CHK((mainPage > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * INDATA[5:0] is the PHY address WHEN RWOP = 0b0
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &port)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &mainPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &extPage)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* get the read operation result to pData */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_DATAf, pData)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* check the FAIL bit */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_FAILf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if(val == 1)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PHY_ACCESS_FAIL;
    }

    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8390_miim_read */


/* Function Name:
 *      rtl8390_miim_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_extParkPage_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mainPage,
    uint32      extPage,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data)
{
    rtk_port_t  max_port, portId;
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x \
           data=0x%x", unit, port, mainPage, extPage, parkPage, phy_reg, data);
    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    //RT_PARAM_CHK((page >= PHY_PAGE_MAX), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    max_port = HAL_GET_MAX_PORT(unit);
    for (portId = 0; portId < max_port; portId++)
    {
        if (!HAL_IS_PORT_EXIST(unit, portId))
        {
            continue;
        }
        if(port == portId)
            val = 1;
        else
            val = 0;
        if ((ret = reg_array_field_write(unit,
                              CYPRESS_PHYREG_PORT_CTRLr,
                              portId,
                              REG_ARRAY_INDEX_NONE,
                              CYPRESS_PHYMSKf,
                              &val)) != RT_ERR_OK)
        {
            PHY_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &mainPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &extPage)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* check the FAIL bit */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_FAILf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if(val == 1)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PHY_ACCESS_FAIL;
    }

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_write */

/* Function Name:
 *      rtl8390_miim_extParkPage_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8390 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_extParkPage_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mainPage,
    uint32          extPage,
    uint32          parkPage,
    uint32          phy_reg,
    uint32          data)
{
    rtk_port_t  port, max_port;
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%8x 0x%8x, mainPage=0x%x, extPage=0x%x, parkPage=0x%x, reg=0x%x \
           data=0x%x", unit, portmask.bits[1], portmask.bits[0], mainPage, extPage, parkPage, phy_reg, data);
    //RT_PARAM_CHK((page >= PHY_PAGE_MAX), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        if(RTK_PORTMASK_IS_PORT_SET(portmask, port))
            val = 1;
        else
            val = 0;

        if ((ret = reg_array_field_write(unit,
                              CYPRESS_PHYREG_PORT_CTRLr,
                              port,
                              REG_ARRAY_INDEX_NONE,
                              CYPRESS_PHYMSKf,
                              &val)) != RT_ERR_OK)
        {
            PHY_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_REGf, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_MAIN_PAGEf, &mainPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_PARK_PAGEf, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_CTRLr, CYPRESS_EXT_PAGEf, &extPage)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* check the FAIL bit */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_FAILf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if(val == 1)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PHY_ACCESS_FAIL;
    }

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_portmask_write */

/* Function Name:
 *      rtl8390_miim_mmd_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData)
{
    uint32 temp;
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x, reg=0x%x",
           unit, port, mmdAddr, mmdReg);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    //RT_PARAM_CHK((mainPage > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    //RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * INDATA[5:0] is the PHY address WHEN RWOP = 0b0
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &port)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd device address to access */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_MMD_CTRLr, CYPRESS_MMD_DEVADf, &mmdAddr)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd register to access */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_MMD_CTRLr, CYPRESS_MMD_REGf, &mmdReg)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* get the read operation result to pData */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_DATAf, pData)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* check the FAIL bit */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_FAILf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if(val == 1)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PHY_ACCESS_FAIL;
    }

    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8390_miim_mmd_read */


/* Function Name:
 *      rtl8390_miim_mmd_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data)
{
    rtk_port_t  max_port, portId;
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, port, mmdAddr, mmdReg, data);
    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    //RT_PARAM_CHK((page >= PHY_PAGE_MAX), RT_ERR_PHY_PAGE_ID);
    //RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    max_port = HAL_GET_MAX_PORT(unit);
    for (portId = 0; portId < max_port; portId++)
    {
        if (!HAL_IS_PORT_EXIST(unit, portId))
        {
            continue;
        }
        if(port == portId)
            val = 1;
        else
            val = 0;
        if ((ret = reg_array_field_write(unit,
                              CYPRESS_PHYREG_PORT_CTRLr,
                              portId,
                              REG_ARRAY_INDEX_NONE,
                              CYPRESS_PHYMSKf,
                              &val)) != RT_ERR_OK)
        {
            PHY_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd device address to access */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_MMD_CTRLr, CYPRESS_MMD_DEVADf, &mmdAddr)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd register to access */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_MMD_CTRLr, CYPRESS_MMD_REGf, &mmdReg)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* check the FAIL bit */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_FAILf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if(val == 1)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PHY_ACCESS_FAIL;
    }

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_mmd_write */

/* Function Name:
 *      rtl8390_miim_mmd_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8390 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      mmdAddr  - mmd device address
 *      mmdReg   - mmd reg id
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8390_miim_mmd_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mmdAddr,
    uint32          mmdReg,
    uint32          data)
{
    rtk_port_t  port, max_port;
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%8x 0x%8x, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, portmask.bits[1], portmask.bits[0], mmdAddr, mmdReg, data);
    //RT_PARAM_CHK((page >= PHY_PAGE_MAX), RT_ERR_PHY_PAGE_ID);
    //RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (!HAL_IS_PORT_EXIST(unit, port))
        {
            continue;
        }
        if(RTK_PORTMASK_IS_PORT_SET(portmask, port))
            val = 1;
        else
            val = 0;

        if ((ret = reg_array_field_write(unit,
                              CYPRESS_PHYREG_PORT_CTRLr,
                              port,
                              REG_ARRAY_INDEX_NONE,
                              CYPRESS_PHYMSKf,
                              &val)) != RT_ERR_OK)
        {
            PHY_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
            return ret;
        }
    }
    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_DATA_CTRLr, CYPRESS_INDATAf, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd device address to access */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_MMD_CTRLr, CYPRESS_MMD_DEVADf, &mmdAddr)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd register to access */
    if ((ret = reg_field_write(unit, CYPRESS_PHYREG_MMD_CTRLr, CYPRESS_MMD_REGf, &mmdReg)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* Broadcast operation
     * 0b0: normal
     * 0b1: broadcast
     */
    val = 0;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_BROADCASTf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, CYPRESS_PHYREG_ACCESS_CTRLr, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, CYPRESS_PHYREG_ACCESS_CTRLr, 0x1);

    /* check the FAIL bit */
    if ((ret = reg_field_read(unit, CYPRESS_PHYREG_ACCESS_CTRLr, CYPRESS_FAILf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    if(val == 1)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PHY_ACCESS_FAIL;
    }

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_mmd_portmask_write */

/* Function Name:
 *      rtl8390_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
int32
rtl8390_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, reg_value;
    uint32      busy;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;
    uint32      groupId;
    rtk_indirectCtrlGroup_t ctrlGroup[] = {
        INDIRECT_CTRL_GROUP_L2,
        INDIRECT_CTRL_GROUP_TABLE,
        INDIRECT_CTRL_GROUP_PKT_ENC,
        INDIRECT_CTRL_GROUP_EGR_CTRL };
    rtk_cypress_reg_list_t ctrlReg[] = {
        CYPRESS_TBL_ACCESS_L2_CTRLr,
        CYPRESS_TBL_ACCESS_CTRL_0r,
        CYPRESS_TBL_ACCESS_CTRL_1r,
        CYPRESS_TBL_ACCESS_CTRL_2r };
    rtk_cypress_reg_list_t dataReg[] = {
        CYPRESS_TBL_ACCESS_L2_DATAr,
        CYPRESS_TBL_ACCESS_DATA_0r,
        CYPRESS_TBL_ACCESS_DATA_1r,
        CYPRESS_TBL_ACCESS_DATA_2r };
    uint32      index;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);

    switch (table)
    {
    case CYPRESS_L2_UCt:
    case CYPRESS_L2_MCt:
    case CYPRESS_L2_IP_MC_SIPt:
    case CYPRESS_L2_IP_MCt:
    case CYPRESS_L2_CAM_UCt:
    case CYPRESS_L2_CAM_MCt:
    case CYPRESS_L2_CAM_IP_MC_SIPt:
    case CYPRESS_L2_CAM_IP_MCt:
    case CYPRESS_MC_PMSKt:
    case CYPRESS_L2_NEXT_HOPt:
    case CYPRESS_L2_NH_LEGACYt:
        groupId = 0;
        break;

    case CYPRESS_VLANt:
    case CYPRESS_VLAN_IGR_CNVTt:
    case CYPRESS_VLAN_IP_SUBNET_BASEDt:
    case CYPRESS_VLAN_MAC_BASEDt:
    case CYPRESS_IACLt:
    case CYPRESS_EACLt:
    case CYPRESS_METERt:
    case CYPRESS_LOGt:
    case CYPRESS_MSTIt:
        groupId = 1;
        break;

    case CYPRESS_UNTAGt:
    case CYPRESS_VLAN_EGR_CNVTt:
    case CYPRESS_ROUTINGt:
    case CYPRESS_MPLS_LIBt:
        groupId = 2;
        break;

    case CYPRESS_SCHEDt:
    case CYPRESS_SPG_PORTt:
    case CYPRESS_OUT_Qt:
        groupId = 3;
        break;

    default:
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    MEM_SEM_LOCK(unit, ctrlGroup[groupId]);

    /* initialize variable */
    reg_data = 0;
    busy = 0;

    /* Command hardware to execute indirect table access
     * 0b0: not execute
     * 0b1: execute
     * Note: This bit is common used by software and hardware.
     *       When hardware completes the table access, it will clear this bit.
     */
    reg_value = 1;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_EXECf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Table access operation
     * 0b0: read
     * 0b1: write
     */
    reg_value = 0;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_CMDf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* access table type */
    reg_value = pTable->type;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_TBLf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Select access address of the table */
    reg_value = addr;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_ADDRf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Write indirect control register to start the read operation */
    if ((ret = reg_write(unit, ctrlReg[groupId], &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

#if !defined(CONFIG_VIRTUAL_ARRAY_ONLY)
    /* Wait operation completed */
    do
    {
        if ((ret = reg_field_read(unit, ctrlReg[groupId], CYPRESS_EXECf, &busy)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    } while (busy);
#endif

    /* Read table data from indirect data register */
    for (index=0; index<(pTable->datareg_num); index++)
    {
        if ((ret = reg_array_read(unit, dataReg[groupId], REG_ARRAY_INDEX_NONE, index, pData + index)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    }

    MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);

    return RT_ERR_OK;
} /* end of rtl8390_table_read */


/* Function Name:
 *      rtl8390_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
int32
rtl8390_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, reg_value;
    uint32      busy;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;
    uint32      groupId;
    rtk_indirectCtrlGroup_t ctrlGroup[] = {
        INDIRECT_CTRL_GROUP_L2,
        INDIRECT_CTRL_GROUP_TABLE,
        INDIRECT_CTRL_GROUP_PKT_ENC,
        INDIRECT_CTRL_GROUP_EGR_CTRL };
    rtk_cypress_reg_list_t ctrlReg[] = {
        CYPRESS_TBL_ACCESS_L2_CTRLr,
        CYPRESS_TBL_ACCESS_CTRL_0r,
        CYPRESS_TBL_ACCESS_CTRL_1r,
        CYPRESS_TBL_ACCESS_CTRL_2r };
    rtk_cypress_reg_list_t dataReg[] = {
        CYPRESS_TBL_ACCESS_L2_DATAr,
        CYPRESS_TBL_ACCESS_DATA_0r,
        CYPRESS_TBL_ACCESS_DATA_1r,
        CYPRESS_TBL_ACCESS_DATA_2r };
    uint32      index;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);

    if (CYPRESS_LOGt == table)
    {
        RT_PARAM_CHK(((addr & 0xFFFFF3FF) >= pTable->size), RT_ERR_OUT_OF_RANGE);
    }
    else
    {
    RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);
    }

    switch (table)
    {
    case CYPRESS_L2_UCt:
    case CYPRESS_L2_MCt:
    case CYPRESS_L2_IP_MC_SIPt:
    case CYPRESS_L2_IP_MCt:
    case CYPRESS_L2_CAM_UCt:
    case CYPRESS_L2_CAM_MCt:
    case CYPRESS_L2_CAM_IP_MC_SIPt:
    case CYPRESS_L2_CAM_IP_MCt:
    case CYPRESS_MC_PMSKt:
    case CYPRESS_L2_NEXT_HOPt:
    case CYPRESS_L2_NH_LEGACYt:
        groupId = 0;
        break;

    case CYPRESS_VLANt:
    case CYPRESS_VLAN_IGR_CNVTt:
    case CYPRESS_VLAN_IP_SUBNET_BASEDt:
    case CYPRESS_VLAN_MAC_BASEDt:
    case CYPRESS_IACLt:
    case CYPRESS_EACLt:
    case CYPRESS_METERt:
    case CYPRESS_LOGt:
    case CYPRESS_MSTIt:
        groupId = 1;
        break;

    case CYPRESS_UNTAGt:
    case CYPRESS_VLAN_EGR_CNVTt:
    case CYPRESS_ROUTINGt:
    case CYPRESS_MPLS_LIBt:
        groupId = 2;
        break;

    case CYPRESS_SCHEDt:
    case CYPRESS_SPG_PORTt:
    case CYPRESS_OUT_Qt:
        groupId = 3;
        break;

    default:
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    MEM_SEM_LOCK(unit, ctrlGroup[groupId]);

    /* initialize variable */
    reg_data = 0;
    busy = 0;

    /* Write pre-configure table data to indirect data register */
    for (index=0; index<(pTable->datareg_num); index++)
    {
        if ((ret = reg_array_write(unit, dataReg[groupId], REG_ARRAY_INDEX_NONE, index, pData + index)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    }

    /* Command hardware to execute indirect table access
     * 0b0: not execute
     * 0b1: execute
     * Note: This bit is common used by software and hardware.
     *       When hardware completes the table access, it will clear this bit.
     */
    reg_value = 1;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_EXECf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Table access operation
     * 0b0: read
     * 0b1: write
     */
    reg_value = 1;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_CMDf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* access table type */
    reg_value = pTable->type;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_TBLf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Select access address of the table */
    reg_value = addr;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], CYPRESS_ADDRf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Write indirect control register to start the write operation */
    if ((ret = reg_write(unit, ctrlReg[groupId], &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

#if !defined(CONFIG_VIRTUAL_ARRAY_ONLY)
    /* Wait operation completed */
    do
    {
        if ((ret = reg_field_read(unit, ctrlReg[groupId], CYPRESS_EXECf, &busy)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    } while (busy);
#endif

    MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);

    return RT_ERR_OK;
} /* end of rtl8390_table_write */

/* Function Name:
 *      rtl8390_miim_pollingEnable_get
 * Description:
 *      Get the mac polling PHY status of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
rtl8390_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled)
{
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d", unit, port);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnabled), RT_ERR_NULL_POINTER);

    if ((ret = reg_array_field_read(unit, CYPRESS_SMI_PORT_POLLING_CTRLr,
        port, REG_ARRAY_INDEX_NONE, CYPRESS_SMI_POLLING_PMSKf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    if (val == 1)
        (*pEnabled) = ENABLED;
    else
        (*pEnabled) = DISABLED;

    return RT_ERR_OK;
} /* end of rtl8390_miim_pollingEnable_get */

/* Function Name:
 *      rtl8390_miim_pollingEnable_set
 * Description:
 *      Set the mac polling PHY status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enabled - mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
rtl8390_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled)
{
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, enabled=%d", unit, port, enabled);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enabled != DISABLED && enabled != ENABLED), RT_ERR_INPUT);

    if (enabled)
        val = 1;
    else
        val = 0;

    if ((ret = reg_array_field_write(unit, CYPRESS_SMI_PORT_POLLING_CTRLr,
        port, REG_ARRAY_INDEX_NONE, CYPRESS_SMI_POLLING_PMSKf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of rtl8390_miim_pollingEnable_set */

/* Function Name:
 *      rtl8390_miim_globalPollingEnable_get
 * Description:
 *      Get the global mac polling PHY status.
 * Input:
 *      unit     - unit id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
rtl8390_miim_globalPollingEnable_get(
    uint32          unit,
    rtk_enable_t    *pEnabled)
{
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d", unit);

    RT_PARAM_CHK((NULL == pEnabled), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(unit, CYPRESS_SMI_GLB_CTRLr, CYPRESS_MDX_POLLING_ENf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    if (val == 1)
        (*pEnabled) = ENABLED;
    else
        (*pEnabled) = DISABLED;

    return RT_ERR_OK;
} /* end of rtl8390_miim_globalPollingEnable_get */

/* Function Name:
 *      rtl8390_miim_globalPollingEnable_set
 * Description:
 *      Set the global mac polling PHY status.
 * Input:
 *      unit    - unit id
 *      enabled - global mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
rtl8390_miim_globalPollingEnable_set(
    uint32          unit,
    rtk_enable_t    enabled)
{
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, enabled=%d", unit, enabled);

    RT_PARAM_CHK((enabled != DISABLED && enabled != ENABLED), RT_ERR_INPUT);

    if (enabled)
        val = 1;
    else
        val = 0;

    if ((ret = reg_field_write(unit, CYPRESS_SMI_GLB_CTRLr, CYPRESS_MDX_POLLING_ENf, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of rtl8390_miim_globalPollingEnable_set */


/* Function Name:
 *      rtl8390_serdes_rst
 * Description:
 *      Reset Serdes and original patch are kept.
 * Input:
 *      unit    - unit id
 *      sds_num    - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_OUT_OF_RANGE - Serdes index is not support.
 * Note:
 *      None
 */
int32
rtl8390_serdes_rst(
    uint32  unit,
    uint32 sds_num)
{
    uint32  sdsReg[] = {0xA328, 0xA728, 0xAB28, 0xAF28, 0xB320, 0xB728, 0xBB20};
    uint32  addr_ofst = 0x400;
    uint32  ofst, sdsAddr;

    if (sds_num >= RTK_MAX_NUM_OF_SERDES)
    {
        RT_DBG(LOG_DEBUG, (MOD_HAL|MOD_SWITCH), "Serdes[%d] doesn't exist\n", sds_num);
        return RT_ERR_OUT_OF_RANGE;
    }

    ofst = addr_ofst * (sds_num / 2);
    sdsAddr = sdsReg[sds_num/2] + (0x80 * (sds_num % 2));

    if (sds_num < 8 || sds_num == 10 || sds_num == 11) {
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x0050);
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x00f0);
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x0);

        SERDES_SET(sdsAddr,  0 , 0 , 0x0);
        SERDES_SET(sdsAddr,  9 , 9 , 0x1);
        osal_time_usleep(100 * 1000);
        SERDES_SET(sdsAddr,  9 , 9 , 0x0);
    } else if (sds_num == 8 || sds_num == 9) {
        SERDES_SET(0xb3f8,  31 , 16 , 0x0005);
        SERDES_SET(0xb3f8,  31 , 16 , 0x000f);
        SERDES_SET(0xb3f8,  31 , 16 , 0x0);

        SERDES_SET(0xb320,  3 , 3 , 0x0);
        SERDES_SET(0xb340,  15 , 15 , 0x1);
        osal_time_usleep(100 * 1000);
        SERDES_SET(0xb340,  15 , 15 , 0x0);
    } else if (sds_num == 12 || sds_num == 13) {
        SERDES_SET(0xbbf8,  31 , 16 , 0x0005);
        SERDES_SET(0xbbf8,  31 , 16 , 0x000f);
        SERDES_SET(0xbbf8,  31 , 16 , 0x0);

        SERDES_SET(0xbb20,  3 , 3 , 0x0);
        SERDES_SET(0xbb40,  15 , 15 , 0x1);
        osal_time_usleep(100 * 1000);
        SERDES_SET(0xbb40,  15 , 15 , 0x0);
    } else {
        return RT_ERR_FAILED;
    }

    SERDES_SET(0xa004 + ofst,  31 , 16 , 0x7146);
    osal_time_usleep(100 * 1000);
    SERDES_SET(0xa004 + ofst,  31 , 16 , 0x7106);
    SERDES_SET(0xa004 + ofst + 0x100,  31 , 16 , 0x7146);
    osal_time_usleep(100 * 1000);
    SERDES_SET(0xa004 + ofst + 0x100,  31 , 16 , 0x7106);

    return RT_ERR_OK;
}   /* end of rtl8390_serdes_rst */


#endif

#if defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      rtl8380_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8380_port_probe(uint32 unit)
{
    return RT_ERR_OK;
} /* end of rtl8390_port_probe */


/* Function Name:
 *      rtl8380_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
rtl8380_init(uint32 unit)
{
#if defined(CONFIG_SDK_RTL8231) && defined(CONFIG_SDK_KERNEL_LINUX_KERNEL_MODE)
    int32       ret = RT_ERR_FAILED;

    /* register rtl8231 mdc semphore callback function */
    if ((ret = drv_rtl8231_mdcSem_register(unit, rtl83xx_mdcSem_callback)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }
#endif

    /* Initialize the threshold value of chip buffer if need */
    return RT_ERR_OK;
} /* end of rtl8390_init */


/* Function Name:
 *      rtl8380_miim_read
 * Description:
 *      Get PHY registers from rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData)
{
    uint32 temp;
    uint32 val;
    uint32 port_rmk_regaddr;
    uint32 port_rmk_phyid;
    uint32 parkPage;

    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, page=0x%x, phy_reg=0x%x", unit, port, page, phy_reg);

    //RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * INDATA[5:0] is the PHY address WHEN RWOP = 0b0
     */
     /*For 80 8ports mode & 16 ports mode, loader may remark phyID,
        so here get real phyID according to remark table, although 82m no need this, but here also using this mechanism*/
    if((port >= 0) && (port <= 5))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT0_5_ADDR_CTRLr;
    }
    else if((port >= 6) && (port <= 11))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT6_11_ADDR_CTRLr;
    }
    else if((port >= 12) && (port <= 17))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT12_17_ADDR_CTRLr;
    }
    else if((port >= 18) && (port <= 23))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT18_23_ADDR_CTRLr;
    }
    else if((port >= 24) && (port <= 27))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT24_27_ADDR_CTRLr;
    }
    else
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PORT_ID;
    }

    if ((ret = reg_read(unit, port_rmk_regaddr, &port_rmk_phyid)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    port = (port_rmk_phyid>>(5*(port%6))) & 0x1F;
    if(port >= 28)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PORT_ID;
    }

    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &port)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_REG_ADDR_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_MAIN_PAGE_11_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select parked page*/
    /*Always not Park, because 80 mac polling always send right page*/
    parkPage = 0x1f;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_PARK_PAGE_4_0f, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    /* get the read operation result to pData */
    if ((ret = reg_field_read(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_DATA_15_0f, pData)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8390_miim_read */


/* Function Name:
 *      rtl8380_miim_write
 * Description:
 *      Set PHY registers in rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data)
{
    uint32  temp;
    uint32  val;
    uint32 parkPage;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, phy_reg, data);
   // RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    val = 1<<port;
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_0r, MAPLE_PHY_MASKf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_REG_ADDR_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_MAIN_PAGE_11_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select parked page*/
    /*Always not Park, because 80 mac polling always send right page*/
    parkPage = 0x1f;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_PARK_PAGE_4_0f, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8380_miim_write */


/* Function Name:
 *      rtl8380_miim_park_read
 * Description:
 *      Get PHY registers from rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_park_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData)
{
    uint32 temp;
    uint32 val;
    uint32 port_rmk_regaddr;
    uint32 port_rmk_phyid;

    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, page=0x%x, phy_reg=0x%x", unit, port, page, phy_reg);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((parkPage > 0x1F), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * INDATA[5:0] is the PHY address WHEN RWOP = 0b0
     */
     /*For 80 8ports mode & 16 ports mode, loader may remark phyID,
        so here get real phyID according to remark table, although 82m no need this, but here also using this mechanism*/
    if((port >= 0) && (port <= 5))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT0_5_ADDR_CTRLr;
    }
    else if((port >= 6) && (port <= 11))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT6_11_ADDR_CTRLr;
    }
    else if((port >= 12) && (port <= 17))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT12_17_ADDR_CTRLr;
    }
    else if((port >= 18) && (port <= 23))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT18_23_ADDR_CTRLr;
    }
    else if((port >= 24) && (port <= 27))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT24_27_ADDR_CTRLr;
    }
    else
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PORT_ID;
    }

    if ((ret = reg_read(unit, port_rmk_regaddr, &port_rmk_phyid)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    port = (port_rmk_phyid>>(5*(port%6))) & 0x1F;
    if(port >= 28)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PORT_ID;
    }

    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &port)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_REG_ADDR_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_MAIN_PAGE_11_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select parked page*/
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_PARK_PAGE_4_0f, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    /* get the read operation result to pData */
    if ((ret = reg_field_read(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_DATA_15_0f, pData)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8390_miim_read */


/* Function Name:
 *      rtl8380_miim_park_write
 * Description:
 *      Set PHY registers in rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_park_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data)
{
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, page=0x%x, phy_reg=0x%x, data=0x%x", unit, port, page, phy_reg, data);
    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((page > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((parkPage > 0x1F), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    val = 1<<port;
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_0r, MAPLE_PHY_MASKf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_REG_ADDR_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_MAIN_PAGE_11_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select parked page*/
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_PARK_PAGE_4_0f, &parkPage, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8380_miim_write */





/* Function Name:
 *      rtl8380_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8380 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data)
{
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%x, page=0x%x, phy_reg=0x%x, data=0x%x", unit, portmask.bits[0], page, phy_reg, data);
    RT_PARAM_CHK((page >= PHY_PAGE_MAX), RT_ERR_PHY_PAGE_ID);
    RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    val = portmask.bits[0];
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_0r, MAPLE_PHY_MASKf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }
    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select register number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_REG_ADDR_4_0f, &phy_reg, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select page number to access */
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_MAIN_PAGE_11_0f, &page, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }


    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8390_miim_portmask_write */


/* Function Name:
 *      rtl8380_miim_mmd_read
 * Description:
 *      Get PHY registers from rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData)
{
    uint32 temp;
    uint32 val;
    uint32 port_rmk_regaddr;
    uint32 port_rmk_phyid;

    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x, reg=0x%x",
           unit, port, mmdAddr, mmdReg);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    //RT_PARAM_CHK((mainPage > HAL_MIIM_PAGE_ID_MAX(unit)), RT_ERR_PHY_PAGE_ID);
    //RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    PHY_SEM_LOCK(unit);

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * INDATA[5:0] is the PHY address WHEN RWOP = 0b0
     */
     /*For 80 8ports mode & 16 ports mode, loader may remark phyID,
        so here get real phyID according to remark table, although 82m no need this, but here also using this mechanism*/
    if((port >= 0) && (port <= 5))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT0_5_ADDR_CTRLr;
    }
    else if((port >= 6) && (port <= 11))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT6_11_ADDR_CTRLr;
    }
    else if((port >= 12) && (port <= 17))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT12_17_ADDR_CTRLr;
    }
    else if((port >= 18) && (port <= 23))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT18_23_ADDR_CTRLr;
    }
    else if((port >= 24) && (port <= 27))
    {
        port_rmk_regaddr = MAPLE_SMI_PORT24_27_ADDR_CTRLr;
    }
    else
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PORT_ID;
    }

    if ((ret = reg_read(unit, port_rmk_regaddr, &port_rmk_phyid)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    port = (port_rmk_phyid>>(5*(port%6))) & 0x1F;
    if(port >= 28)
    {
        PHY_SEM_UNLOCK(unit);
        return RT_ERR_PORT_ID;
    }

    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &port)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd device address to access */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_3r, MAPLE_MMD_DEVAD_4_0f, &mmdAddr)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd register to access */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_3r, MAPLE_MMD_REG_15_0f, &mmdReg)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 0;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    /* get the read operation result to pData */
    if ((ret = reg_field_read(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_DATA_15_0f, pData)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }
    PHY_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_HAL), "pData=0x%x", *pData);

    return RT_ERR_OK;
} /* end of rtl8380_miim_mmd_read */


/* Function Name:
 *      rtl8380_miim_mmd_write
 * Description:
 *      Set PHY registers in rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data)
{
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, port=%d, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, port, mmdAddr, mmdReg, data);
    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    //RT_PARAM_CHK((page >= PHY_PAGE_MAX), RT_ERR_PHY_PAGE_ID);
    //RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    val = 1<<port;
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_0r, MAPLE_PHY_MASKf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd device address to access */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_3r, MAPLE_MMD_DEVAD_4_0f, &mmdAddr)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd register to access */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_3r, MAPLE_MMD_REG_15_0f, &mmdReg)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8380_miim_mmd_write */

/* Function Name:
 *      rtl8380_miim_mmd_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8380 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      mmdAddr  - mmd device address
 *      mmdReg   - mmd reg id
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
int32
rtl8380_miim_mmd_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mmdAddr,
    uint32          mmdReg,
    uint32          data)
{
    uint32  temp;
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_LOG(LOG_TRACE, MOD_HAL, "unit=%d, portmask=0x%8x 0x%8x, mmdAddr=0x%x, mmdReg=0x%x \
           data=0x%x", unit, portmask.bits[1], portmask.bits[0], mmdAddr, mmdReg, data);
    //RT_PARAM_CHK((page >= PHY_PAGE_MAX), RT_ERR_PHY_PAGE_ID);
    //RT_PARAM_CHK((phy_reg >= PHY_REG_MAX), RT_ERR_PHY_REG_ID);

    PHY_SEM_LOCK(unit);

    /* Select PHY to access */
    val = portmask.bits[0];
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_0r, MAPLE_PHY_MASKf, &val)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_PORT), "");
        return ret;
    }

    /* initialize variable */
    temp = 0;

    /* Input parameters:
     * If RWOP = 0(read), then INDATA[15:0] = {Reserved & PORT_ID[5:0]}
     * If RWOP = 1(write), then INDATA[15:0] = DATA[15:0]
     */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_2r, MAPLE_INDATA_15_0f, &data)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd device address to access */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_3r, MAPLE_MMD_DEVAD_4_0f, &mmdAddr)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select mmd register to access */
    if ((ret = reg_field_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_3r, MAPLE_MMD_REG_15_0f, &mmdReg)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Select PHY register type
     * 0b0: Normal register
     * 0b1: MMD register
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_TYPEf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Read/Write operation
     * 0b0: read
     * 0b1: write
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_RWOPf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* Request MAC to access PHY MII register
     * 0b0: complete access
     * 0b1: execute access
     * Note: When MAC completes access, it will clear this bit.
     */
    val = 1;
    if ((ret = reg_field_set(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, MAPLE_CMDf, &val, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* write register to active the read operation */
    if ((ret = reg_write(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, &temp)) != RT_ERR_OK)
    {
        PHY_SEM_UNLOCK(unit);
        return ret;
    }

    /* busy waiting until reg.bit[0] = 0b0 (MAC completes access) */
    PHY_BUSY_WAIT_LOOP(unit, MAPLE_SMI_ACCESS_PHY_CTRL_1r, 0x1);

    PHY_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of rtl8380_miim_mmd_portmask_write */



/* Function Name:
 *      rtl8380_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
int32
rtl8380_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, reg_value;
    uint32      busy;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;
    uint32      groupId;
    rtk_indirectCtrlGroup_t ctrlGroup[] = {
        INDIRECT_CTRL_GROUP_L2,
        INDIRECT_CTRL_GROUP_TABLE,
        INDIRECT_CTRL_GROUP_PKT_ENC,
        INDIRECT_CTRL_GROUP_EGR_CTRL };
    rtk_maple_reg_list_t ctrlReg[] = {
        MAPLE_TBL_ACCESS_L2_CTRLr,
        MAPLE_TBL_ACCESS_CTRL_0r,
        MAPLE_TBL_ACCESS_CTRL_1r
         };
    rtk_maple_reg_list_t dataReg[] = {
        MAPLE_TBL_ACCESS_L2_DATAr,
        MAPLE_TBL_ACCESS_DATA_0r,
        MAPLE_TBL_ACCESS_DATA_1r
         };
    uint32      index;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);
    RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);

    switch (table)
    {
        case MAPLE_L2_UCt:
        case MAPLE_L2_MCt:
        case MAPLE_L2_IP_MC_SIPt:
        case MAPLE_L2_IP_MCt:
        case MAPLE_L2_NEXT_HOPt:
        case MAPLE_L2_NEXT_HOP_LEGACYt:
        case MAPLE_L2_CAM_UCt:
        case MAPLE_L2_CAM_MCt:
        case MAPLE_L2_CAM_IP_MC_SIPt:
        case MAPLE_L2_CAM_IP_MCt:
        case MAPLE_MC_PMSKt:
            groupId = 0;
            break;

        case MAPLE_VLANt:
        case MAPLE_IACLt:
        case MAPLE_LOGt:
        case MAPLE_MSTIt:
            groupId = 1;
            break;

        case MAPLE_UNTAGt:
        case MAPLE_VLAN_EGR_CNVTt:
        case MAPLE_ROUTINGt:
            groupId = 2;
            break;

        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    MEM_SEM_LOCK(unit, ctrlGroup[groupId]);

    /* initialize variable */
    reg_data = 0;
    busy = 0;

    /* Command hardware to execute indirect table access
     * 0b0: not execute
     * 0b1: execute
     * Note: This bit is common used by software and hardware.
     *       When hardware completes the table access, it will clear this bit.
     */
    reg_value = 1;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_EXECf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Table access operation
     * 0b0: write
     * 0b1: read
     */
    reg_value = 1;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_CMDf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* access table type */
    reg_value = pTable->type;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_TBLf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Select access address of the table */
    reg_value = addr;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_ADDRf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Write indirect control register to start the read operation */
    if ((ret = reg_write(unit, ctrlReg[groupId], &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

#if !defined(CONFIG_VIRTUAL_ARRAY_ONLY)
    /* Wait operation completed */
    do
    {
        if ((ret = reg_field_read(unit, ctrlReg[groupId], MAPLE_EXECf, &busy)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    } while (busy);
#endif

    /* Read table data from indirect data register */
    for (index=0; index<(pTable->datareg_num); index++)
    {
        if ((ret = reg_array_read(unit, dataReg[groupId], REG_ARRAY_INDEX_NONE, index, pData + index)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    }

    MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);

    return RT_ERR_OK;
} /* end of rtl8390_table_read */


/* Function Name:
 *      rtl8380_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
int32
rtl8380_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    uint32      reg_data, reg_value;
    uint32      busy;
    int32       ret = RT_ERR_FAILED;
    rtk_table_t *pTable = NULL;
    uint32      groupId;
    rtk_indirectCtrlGroup_t ctrlGroup[] = {
        INDIRECT_CTRL_GROUP_L2,
        INDIRECT_CTRL_GROUP_TABLE,
        INDIRECT_CTRL_GROUP_PKT_ENC,
        INDIRECT_CTRL_GROUP_EGR_CTRL };
    rtk_maple_reg_list_t ctrlReg[] = {
        MAPLE_TBL_ACCESS_L2_CTRLr,
        MAPLE_TBL_ACCESS_CTRL_0r,
        MAPLE_TBL_ACCESS_CTRL_1r,
        };
    rtk_maple_reg_list_t dataReg[] = {
        MAPLE_TBL_ACCESS_L2_DATAr,
        MAPLE_TBL_ACCESS_DATA_0r,
        MAPLE_TBL_ACCESS_DATA_1r,
        };
    uint32      index;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, table=%d, addr=0x%x", unit, table, addr);

    /* parameter check */
    RT_PARAM_CHK((table >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);
    pTable = table_find(unit, table);
    /* NULL means the table is not supported in this chip unit */
    RT_PARAM_CHK((NULL == pTable), RT_ERR_CHIP_NOT_SUPPORTED);
    if(MAPLE_LOGt != table)
        RT_PARAM_CHK((addr >= pTable->size), RT_ERR_OUT_OF_RANGE);
    else
        RT_PARAM_CHK(((addr&0x7f) >= pTable->size), RT_ERR_OUT_OF_RANGE);

    switch (table)
    {
        case MAPLE_L2_UCt:
        case MAPLE_L2_MCt:
        case MAPLE_L2_IP_MC_SIPt:
        case MAPLE_L2_IP_MCt:
        case MAPLE_L2_NEXT_HOPt:
        case MAPLE_L2_NEXT_HOP_LEGACYt:
        case MAPLE_L2_CAM_UCt:
        case MAPLE_L2_CAM_MCt:
        case MAPLE_L2_CAM_IP_MC_SIPt:
        case MAPLE_L2_CAM_IP_MCt:
        case MAPLE_MC_PMSKt:
            groupId = 0;
            break;

        case MAPLE_VLANt:
        case MAPLE_IACLt:
        case MAPLE_LOGt:
        case MAPLE_MSTIt:
            groupId = 1;
            break;

        case MAPLE_UNTAGt:
        case MAPLE_VLAN_EGR_CNVTt:
        case MAPLE_ROUTINGt:
             groupId = 2;
            break;

        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    MEM_SEM_LOCK(unit, ctrlGroup[groupId]);

    /* initialize variable */
    reg_data = 0;
    busy = 0;

    /* Write pre-configure table data to indirect data register */
    for (index=0; index<(pTable->datareg_num); index++)
    {
        if ((ret = reg_array_write(unit, dataReg[groupId], REG_ARRAY_INDEX_NONE, index, pData + index)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    }

    /* Command hardware to execute indirect table access
     * 0b0: not execute
     * 0b1: execute
     * Note: This bit is common used by software and hardware.
     *       When hardware completes the table access, it will clear this bit.
     */
    reg_value = 1;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_EXECf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Table access operation
     * 0b0: write
     * 0b1: read
     */
    reg_value = 0;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_CMDf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* access table type */
    reg_value = pTable->type;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_TBLf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Select access address of the table */
    reg_value = addr;
    if ((ret = reg_field_set(unit, ctrlReg[groupId], MAPLE_ADDRf, &reg_value, &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

    /* Write indirect control register to start the write operation */
    if ((ret = reg_write(unit, ctrlReg[groupId], &reg_data)) != RT_ERR_OK)
    {
        MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
        return ret;
    }

#if !defined(CONFIG_VIRTUAL_ARRAY_ONLY)
    /* Wait operation completed */
    do
    {
        if ((ret = reg_field_read(unit, ctrlReg[groupId], MAPLE_EXECf, &busy)) != RT_ERR_OK)
        {
            MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);
            return ret;
        }
    } while (busy);
#endif

    MEM_SEM_UNLOCK(unit, ctrlGroup[groupId]);

    return RT_ERR_OK;
} /* end of rtl8380_table_write */

/* Function Name:
 *      rtl8380_miim_pollingEnable_get
 * Description:
 *      Get the mac polling PHY status of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
rtl8380_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled)
{
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d", unit, port);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pEnabled), RT_ERR_NULL_POINTER);

    if ((ret = reg_field_read(unit, MAPLE_SMI_POLL_CTRLr,
        MAPLE_SMI_POLL_MASK_27_0f, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    if (((val >> port) & 0x1) == 1)
        (*pEnabled) = ENABLED;
    else
        (*pEnabled) = DISABLED;

    return RT_ERR_OK;
} /* end of rtl8380_miim_pollingEnable_get */

/* Function Name:
 *      rtl8380_miim_pollingEnable_set
 * Description:
 *      Set the mac polling PHY status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enabled - mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
int32
rtl8380_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enable)
{
    uint32 val;
    int32  ret = RT_ERR_FAILED;

    RT_DBG(LOG_DEBUG, (MOD_HAL), "unit=%d, port=%d, enable=%d", unit, port, enable);

    RT_PARAM_CHK((!HAL_IS_ETHER_PORT(unit, port)), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable != DISABLED && enable != ENABLED), RT_ERR_INPUT);

    if ((ret = reg_field_read(unit, MAPLE_SMI_POLL_CTRLr,
        MAPLE_SMI_POLL_MASK_27_0f, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    if (enable)
        val |= (1 << port);
    else
        val &= ~(1 << port);

    if ((ret = reg_field_write(unit, MAPLE_SMI_POLL_CTRLr,
        MAPLE_SMI_POLL_MASK_27_0f, &val)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_HAL), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of rtl8380_miim_pollingEnable_set */


/* Function Name:
 *      rtl8380_serdes_rst
 * Description:
 *      Reset Serdes and original patch are kept.
 * Input:
 *      unit    - unit id
 *      sds_num    - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_OUT_OF_RANGE - Serdes index is not support.
 * Note:
 *      None
 */
int32
rtl8380_serdes_rst(
    uint32  unit,
    uint32 sds_num)
{
    int32 ret;
    uint32 val;

    /*rx reset*/
    ioal_mem32_read(unit, 0xF3A4+sds_num*0x100, &val);
    val |= 1UL<<9;
    ioal_mem32_write(unit, 0xF3A4+sds_num*0x100, val);
    val &= ~(1UL<<9);
    ioal_mem32_write(unit, 0xF3A4+sds_num*0x100, val);

    /*cmu reset*/
    val = 0x4040;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);
    val = 0x4740;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);
    val = 0x47c0;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);
    val = 0x4000;
    ioal_mem32_write(unit, 0xF380+sds_num*0x100, val);

    /*software reset*/
    val = 0x7146;
    ioal_mem32_write(unit, 0xE78C+sds_num*0x200, val);
    val = 0x7106;
    ioal_mem32_write(unit, 0xE78C+sds_num*0x200, val);

    /*tx & rx reset*/
    val = 0x0400;
    ioal_mem32_write(unit, 0xE780+sds_num*0x200, val);
    val = 0x0403;
    ioal_mem32_write(unit, 0xE780+sds_num*0x200, val);

    ret = RT_ERR_OK;
    return ret;
}   /* end of rtl8380_serdes_rst */



#endif

