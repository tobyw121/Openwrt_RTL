/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision$
 * $Date$
 *
 * Purpose : Definition function prototype of RTK API.
 *
 * Feature : Function prototype definition
 *
 */

#ifndef __RTK_API_H__
#define __RTK_API_H__

/*
 * Include Files
 */
#include <rtk_types.h>
#include <port.h>
#include <qos.h>
#include <acl.h>
#include <storm.h>
#include <cpu.h>
#include <l2.h>
#include <led.h>
#include <rate.h>
#include <stat.h>
#include <vlan.h>
#include <rtl8367c_asicdrv.h>
#include "dal/rtl8367d/rtl8367d_asicdrv.h"

/*
 * Data Type Declaration
 */
#ifndef CONFIG_RTL_CUSTOM_PASSTHRU
#define CONFIG_RTL_CUSTOM_PASSTHRU 1
#endif

#define PASSTHRU_VLAN_ID 2

typedef enum rtk_switch_maxPktLen_e
{
    MAXPKTLEN_1522B = 0,
    MAXPKTLEN_1536B,
    MAXPKTLEN_1552B,
    MAXPKTLEN_16000B,
    MAXPKTLEN_END
} rtk_switch_maxPktLen_t;

#define CHIPTYPE (rtk_switch_chipType_get())
#define rtl83xx_setAsicPHYReg(phyNo, phyAddr, phyData)	rtk_port_phyReg_set(phyNo, phyAddr, phyData)
#define rtl83xx_setAsicReg(reg, value)	((CHIPTYPE==4) ? rtl8367d_setAsicReg(reg, value) : rtl8367c_setAsicReg(reg, value))
#define rtl83xx_getAsicReg(reg, pvalue)	((CHIPTYPE==4) ? rtl8367d_getAsicReg(reg, pvalue) : rtl8367c_getAsicReg(reg, pvalue))
#define rtl83xx_getAsicPHYReg(phyNo, phyAddr, pRegData )	rtk_port_phyReg_get(phyNo, phyAddr, pRegData )

#define RTL83XX_REG_CHIP_RESET	((CHIPTYPE==4) ? RTL8367D_REG_CHIP_RESET:RTL8367C_REG_CHIP_RESET)
#define RTL83XX_CHIP_RST_OFFSET	((CHIPTYPE==4) ? RTL8367D_CHIP_RST_OFFSET:RTL8367C_CHIP_RST_OFFSET)
#define RTL83XX_MAX_LIMITED_L2ENTRY_NUM ((CHIPTYPE==4) ? 2056:2112)
#define RTL83XX_VIDMAX	RTK_VID_MAX
#define RTL83XX_PORT4_ENABLE_OFFSET	((CHIPTYPE==4) ? RTL8367D_PORT4_ENABLE_OFFSET :RTL8367C_PORT4_ENABLE_OFFSET)
#define RTL83XX_QOS_RATE_INPUT_MAX	RTK_QOS_RATE_INPUT_MAX
#define RTL83XX_METERNO	RTK_MAX_METER_ID + 1
#define MAX_METERNUM 40
#define RTL83XX_REG_FLOWCTRL_DEBUG_CTRL0 ((CHIPTYPE==4) ? RTL8367D_REG_FLOWCTRL_DEBUG_CTRL0 :RTL8367C_REG_FLOWCTRL_DEBUG_CTRL0)
#define RTL83XX_REG_FLOWCTRL_PORT_PAGE_COUNT ((CHIPTYPE==4) ? RTL8367D_REG_FLOWCTRL_PORT_PAGE_COUNT : RTL8367C_REG_FLOWCTRL_PORT_PAGE_COUNT)
#define RTL83XX_REG_FLOWCTRL_QUEUE0_PAGE_COUNT ((CHIPTYPE==4) ? RTL8367D_REG_FLOWCTRL_QUEUE0_PAGE_COUNT : RTL8367C_REG_FLOWCTRL_QUEUE0_PAGE_COUNT)
#define RTL83XX_REG_FLOWCTRL_PORT_MAX_PAGE_COUNT ((CHIPTYPE==4) ? RTL8367D_REG_FLOWCTRL_PORT_MAX_PAGE_COUNT : RTL8367C_REG_FLOWCTRL_PORT_MAX_PAGE_COUNT)
#define RTL83XX_REG_FLOWCTRL_QUEUE0_MAX_PAGE_COUNT ((CHIPTYPE==4) ? RTL8367D_REG_FLOWCTRL_QUEUE0_MAX_PAGE_COUNT : RTL8367C_REG_FLOWCTRL_QUEUE0_MAX_PAGE_COUNT)

#define STORM_UNUC_INDEX                            28
#define STORM_UNMC_INDEX                            29
#define STORM_MC_INDEX                              30
#define STORM_BC_INDEX                              31

#endif /* __RTK_API_H__ */

