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
 * $Revision: 56898 $
 * $Date: 2015-03-16 19:06:53 +0800 (Mon, 16 Mar 2015) $
 *
 * Purpose : PHY 8218B/8218FB/8214FC Driver APIs.
 *
 * Feature : PHY 8218B/8218FB/8214FC Driver APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <osal/time.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/common/halctrl.h>
#include <hal/common/miim.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_common.h>
#include <hal/phy/phy_8218b.h>
#include <hal/phy/phy_8218b_patch.h>

/*
 * Symbol Definition
 */

#define RTCT_ENABLE                 (0)
#define RTCT_CH_A                   (4)
#define RTCT_CH_B                   (5)
#define RTCT_CH_C                   (6)
#define RTCT_CH_D                   (7)
#define RTCT_DONE                   (15)

#define RTCT_BASE_ADDR              (0x802a)
#define RTCT_LEN_ADDR_A             (RTCT_BASE_ADDR + 0x2)
#define RTCT_LEN_ADDR_B             (RTCT_BASE_ADDR + 0x6)
#define RTCT_LEN_ADDR_C             (RTCT_BASE_ADDR + 0xa)
#define RTCT_LEN_ADDR_D             (RTCT_BASE_ADDR + 0xe)
#define RTCT_STATUS_ADDR_A          (RTCT_BASE_ADDR + 0x0)
#define RTCT_STATUS_ADDR_B          (RTCT_BASE_ADDR + 0x4)
#define RTCT_STATUS_ADDR_C          (RTCT_BASE_ADDR + 0x8)
#define RTCT_STATUS_ADDR_D          (RTCT_BASE_ADDR + 0xc)

#define RTCT_STATUS_NORMAL          (5)
#define RTCT_STATUS_SHORT           (4)
#define RTCT_STATUS_OPEN            (3)
#define RTCT_STATUS_MISSMATCH_SHORT (2)
#define RTCT_STATUS_MISSMATCH_OPEN  (1)
#define RTCT_STATUS_LINE_DRIVER     (0)

#define PHY_MEDIA_LINKDOWN          (0)
#define PHY_MEDIA_LINKUP            (1)

typedef struct hal_phy_info_s
{
    uint8   auto_1000f[RTK_MAX_NUM_OF_PORTS]; /* copper, reg[9].bit[9] shadow for patch down-speed mechanism */
} hal_phy_info_t;

typedef enum phy_8218b_regPtp_e
{
    REG_8218B_PTP_BASE              = 0x1600,
    REG_8218B_PTP_TIME_NSEC_L       = 0x1600,
    REG_8218B_PTP_TIME_NSEC_H       = 0x1601,
    REG_8218B_PTP_TIME_SEC_L        = 0x1602,
    REG_8218B_PTP_TIME_SEC_H        = 0x1603,
    REG_8218B_PTP_TIME_CFG_0        = 0x1604,
    REG_8218B_PTP_OTAG_TPID         = 0x1605,
    REG_8218B_PTP_ITAG_TPID         = 0x1606,
    REG_8218B_PTP_MAC_ADDR_L        = 0x1607,
    REG_8218B_PTP_MAC_ADDR_M        = 0x1608,
    REG_8218B_PTP_MAC_ADDR_H        = 0x1609,
    REG_8218B_PTP_TIME_NSEC_L_RO    = 0x160A,
    REG_8218B_PTP_TIME_NSEC_H_RO    = 0x160B,
    REG_8218B_PTP_TIME_SEC_L_RO     = 0x160C,
    REG_8218B_PTP_TIME_SEC_H_RO     = 0x160D,
    REG_8218B_PTP_TIME_CFG_1        = 0x160E,
    REG_8218B_PTP_TIME_INT_STS_P    = 0x160F,

    /* per-port register addr-offset: 0x10 */
    REG_8218B_PTP_TIME_TX_SID_PN    = 0x1610,
    REG_8218B_PTP_TIME_RX_SID_PN    = 0x1614,
    REG_8218B_PTP_TIME_NSEC_L_PN    = 0x1618,
    REG_8218B_PTP_TIME_NSEC_H_PN    = 0x1619,
    REG_8218B_PTP_TIME_SEC_L_PN     = 0x161A,
    REG_8218B_PTP_TIME_SEC_H_PN     = 0x161B,
    REG_8218B_PTP_TIME_CTRL_PN      = 0x161C,
} phy_8218b_regPtp_t;

typedef enum phy_8218b_regPtpField_e
{
    REG_FIELD_8218B_PTP_PHY_EN              = (0x1 << 8),

    REG_FIELD_8218B_PTP_CMD_EXEC            = (0x1 << 15),
    REG_FIELD_8218B_PTP_CMD_OP_MSK          = (0x3 << 12),
    REG_FIELD_8218B_PTP_CMD_OP_READ         = (0x0 << 12),
    REG_FIELD_8218B_PTP_CMD_OP_WRITE        = (0x1 << 12),
    REG_FIELD_8218B_PTP_CMD_OP_ADJ_INC      = (0x2 << 12),
    REG_FIELD_8218B_PTP_CMD_OP_ADJ_DEC      = (0x3 << 12),
    REG_FIELD_8218B_PTP_TIME_NSEC_H         = (0x7FF << 0),

    REG_FIELD_8218B_PTP_CFG_TIMER_EN_FRC    = (0x1 << 2),
    REG_FIELD_8218B_PTP_CFG_TIMER_1588_EN   = (0x1 << 1),
} phy_8218b_regPtpField_t;

#define REG_8218B_PTP_OFFSET_PORT(port) ((port % 8) * 0x10)
#define REG_8218B_PTP_OFFSET_MSGTYPE(type) (type % 4)


static hal_phy_info_t   *pPhy_info[RTK_MAX_NUM_OF_UNIT];
static uint32           phyInfo_alloc[RTK_MAX_NUM_OF_UNIT];

/*
 * Data Declaration
 */
rt_phydrv_t phy_8218Bdrv_ge =
{
    RT_PHYDRV_RTL8218B,
    phy_8218b_init,
    phy_8218b_media_get,
    (int32 (*)(uint32, rtk_port_t, rtk_port_media_t))phy_common_unavail,
    phy_common_autoNegoEnable_get,
    phy_common_autoNegoEnable_set,
    phy_8218b_autoNegoAbility_get,
    phy_8218b_autoNegoAbility_set,
    phy_common_duplex_get,
    phy_common_duplex_set,
    phy_8218b_speed_get,
    phy_8218b_speed_set,
    phy_8218b_enable_set,
    phy_8218b_rtctResult_get,
    phy_8218b_rtct_start,
    phy_8218b_greenEnable_get,
    phy_8218b_greenEnable_set,
    phy_8218b_eeeEnable_get,
    phy_8218b_eeeEnable_set,
    phy_8218b_crossOverMode_get,
    phy_8218b_crossOverMode_set,
    phy_8218b_crossOverStatus_get,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t))phy_common_unavail,
    phy_8218b_linkDownPowerSavingEnable_get,
    phy_8218b_linkDownPowerSavingEnable_set,
    phy_8218b_broadcastEnable_set,
    phy_8218b_broadcastID_set,
    phy_8218b_gigaLiteEnable_get,
    phy_8218b_gigaLiteEnable_set,
    phy_8218b_eeepEnable_get,
    phy_8218b_eeepEnable_set,
    phy_8218b_patch_set,
    phy_8218b_downSpeedEnable_get,
    phy_8218b_downSpeedEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8218b_ptpSwitchMacAddr_get,
    phy_8218b_ptpSwitchMacAddr_set,
    phy_8218b_ptpRefTime_get,
    phy_8218b_ptpRefTime_set,
    phy_8218b_ptpRefTimeAdjust_set,
    phy_8218b_ptpRefTimeEnable_get,
    phy_8218b_ptpRefTimeEnable_set,
    phy_8218b_ptpEnable_get,
    phy_8218b_ptpEnable_set,
    phy_8218b_ptpRxTimestamp_get,
    phy_8218b_ptpTxTimestamp_get,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8218Bdrv_ge */

/* Add the RTL8218FB driver and hook same function pointer as RTL8218B right now.
 * If the RTL8218FB have the different configure, then will separate the APIs.
 */
rt_phydrv_t phy_8218FBdrv_ge =
{
    RT_PHYDRV_RTL8218FB,
    phy_8218b_init,
    phy_8218fb_media_get,
    phy_8218fb_media_set,
    phy_common_autoNegoEnable_get,
    phy_common_autoNegoEnable_set,
    phy_8218fb_autoNegoAbility_get,
    phy_8218fb_autoNegoAbility_set,
    phy_common_duplex_get,
    phy_common_duplex_set,
    phy_8218b_speed_get,
    phy_8218fb_speed_set,
    phy_8218b_enable_set,
    phy_8218b_rtctResult_get,
    phy_8218b_rtct_start,
    phy_8218b_greenEnable_get,
    phy_8218b_greenEnable_set,
    phy_8218b_eeeEnable_get,
    phy_8218b_eeeEnable_set,
    phy_8218b_crossOverMode_get,
    phy_8218b_crossOverMode_set,
    phy_8218b_crossOverStatus_get,
    phy_8218fb_fiber_media_get,
    phy_8218fb_fiber_media_set,
    phy_8218b_linkDownPowerSavingEnable_get,
    phy_8218b_linkDownPowerSavingEnable_set,
    phy_8218b_broadcastEnable_set,
    phy_8218b_broadcastID_set,
    phy_8218b_gigaLiteEnable_get,
    phy_8218b_gigaLiteEnable_set,
    phy_8218b_eeepEnable_get,
    phy_8218b_eeepEnable_set,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8218b_downSpeedEnable_get,
    phy_8218b_downSpeedEnable_set,
    phy_8218fb_fiberDownSpeedEnable_get,
    phy_8218fb_fiberDownSpeedEnable_set,
    phy_8218fb_fiberNwayForceLink_get,
    phy_8218fb_fiberNwayForceLink_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8218b_ptpSwitchMacAddr_get,
    phy_8218b_ptpSwitchMacAddr_set,
    phy_8218b_ptpRefTime_get,
    phy_8218b_ptpRefTime_set,
    phy_8218b_ptpRefTimeAdjust_set,
    phy_8218b_ptpRefTimeEnable_get,
    phy_8218b_ptpRefTimeEnable_set,
    phy_8218b_ptpEnable_get,
    phy_8218b_ptpEnable_set,
    phy_8218b_ptpRxTimestamp_get,
    phy_8218b_ptpTxTimestamp_get,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8218FBdrv_ge */

rt_phydrv_t phy_8218FBdrv_MP_ge =
{
    RT_PHYDRV_RTL8218FB_MP,
    phy_8218fb_init,
    phy_8218fb_media_get,
    phy_8218fb_media_set,
    phy_8218fb_autoNegoEnable_get,
    phy_8218fb_autoNegoEnable_set,
    phy_8218fb_MP_autoNegoAbility_get,
    phy_8218fb_MP_autoNegoAbility_set,
    phy_8218fb_duplex_get,
    phy_8218fb_duplex_set,
    phy_8218fb_speed_get,
    phy_8218fb_MP_speed_set,
    phy_8218fb_enable_set,
    phy_8218b_rtctResult_get,
    phy_8218b_rtct_start,
    phy_8218b_greenEnable_get,
    phy_8218b_greenEnable_set,
    phy_8218b_eeeEnable_get,
    phy_8218b_eeeEnable_set,
    phy_8218b_crossOverMode_get,
    phy_8218b_crossOverMode_set,
    phy_8218b_crossOverStatus_get,
    phy_8218fb_fiber_media_get,
    phy_8218fb_fiber_media_set,
    phy_8218b_linkDownPowerSavingEnable_get,
    phy_8218b_linkDownPowerSavingEnable_set,
    phy_8218b_broadcastEnable_set,
    phy_8218b_broadcastID_set,
    phy_8218b_gigaLiteEnable_get,
    phy_8218b_gigaLiteEnable_set,
    phy_8218b_eeepEnable_get,
    phy_8218b_eeepEnable_set,
    phy_8218fb_mp_patch_set,
    phy_8218b_downSpeedEnable_get,
    phy_8218b_downSpeedEnable_set,
    phy_8218fb_fiberDownSpeedEnable_get,
    phy_8218fb_fiberDownSpeedEnable_set,
    phy_8218fb_fiberNwayForceLink_get,
    phy_8218fb_fiberNwayForceLink_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8218b_ptpSwitchMacAddr_get,
    phy_8218b_ptpSwitchMacAddr_set,
    phy_8218b_ptpRefTime_get,
    phy_8218b_ptpRefTime_set,
    phy_8218b_ptpRefTimeAdjust_set,
    phy_8218b_ptpRefTimeEnable_get,
    phy_8218b_ptpRefTimeEnable_set,
    phy_8218b_ptpEnable_get,
    phy_8218b_ptpEnable_set,
    phy_8218b_ptpRxTimestamp_get,
    phy_8218b_ptpTxTimestamp_get,
    phy_8218fb_MP_masterSlave_get,
    phy_8218fb_MP_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8218FBdrv_MP_ge */

/* Add the RTL8214FC driver and hook same function pointer as RTL8218B right now.
 * If the RTL8214FC have the different configure, then will separate the APIs.
 */
rt_phydrv_t phy_8214FCdrv_ge =
{
    RT_PHYDRV_RTL8214FC,
    phy_8218b_init,
    phy_8214fc_media_get,
    phy_8214fc_media_set,
    phy_common_autoNegoEnable_get,
    phy_common_autoNegoEnable_set,
    phy_8218fb_autoNegoAbility_get,
    phy_8218fb_autoNegoAbility_set,
    phy_common_duplex_get,
    phy_common_duplex_set,
    phy_8218b_speed_get,
    phy_8214fc_speed_set,
    phy_8218b_enable_set,
    phy_8218b_rtctResult_get,
    phy_8218b_rtct_start,
    phy_8218b_greenEnable_get,
    phy_8218b_greenEnable_set,
    phy_8218b_eeeEnable_get,
    phy_8218b_eeeEnable_set,
    phy_8218b_crossOverMode_get,
    phy_8218b_crossOverMode_set,
    phy_8218b_crossOverStatus_get,
    phy_8214fc_fiber_media_get,
    phy_8214fc_fiber_media_set,
    phy_8218b_linkDownPowerSavingEnable_get,
    phy_8218b_linkDownPowerSavingEnable_set,
    phy_8214fc_broadcastEnable_set,
    phy_8214fc_broadcastID_set,
    phy_8218b_gigaLiteEnable_get,
    phy_8218b_gigaLiteEnable_set,
    phy_8218b_eeepEnable_get,
    phy_8218b_eeepEnable_set,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8218b_downSpeedEnable_get,
    phy_8218b_downSpeedEnable_set,
    phy_8214fc_fiberDownSpeedEnable_get,
    phy_8214fc_fiberDownSpeedEnable_set,
    phy_8214fc_fiberNwayForceLink_get,
    phy_8214fc_fiberNwayForceLink_set,
    phy_8214fc_fiberOAMLoopBack_set,
    phy_8218b_ptpSwitchMacAddr_get,
    phy_8218b_ptpSwitchMacAddr_set,
    phy_8218b_ptpRefTime_get,
    phy_8218b_ptpRefTime_set,
    phy_8218b_ptpRefTimeAdjust_set,
    phy_8218b_ptpRefTimeEnable_get,
    phy_8218b_ptpRefTimeEnable_set,
    phy_8218b_ptpEnable_get,
    phy_8218b_ptpEnable_set,
    phy_8218b_ptpRxTimestamp_get,
    phy_8218b_ptpTxTimestamp_get,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8214FCdrv_ge */

rt_phydrv_t phy_8214FCdrv_MP_ge =
{
    RT_PHYDRV_RTL8214FC_MP,
    phy_8214fc_init,
    phy_8214fc_media_get,
    phy_8214fc_media_set,
    phy_8214fc_autoNegoEnable_get,
    phy_8214fc_autoNegoEnable_set,
    phy_8214fc_autoNegoAbility_get,
    phy_8214fc_autoNegoAbility_set,
    phy_8214fc_duplex_get,
    phy_8214fc_duplex_set,
    phy_8214fc_speed_get,
    phy_8214fc_MP_speed_set,
    phy_8214fc_enable_set,
    phy_8218b_rtctResult_get,
    phy_8218b_rtct_start,
    phy_8218b_greenEnable_get,
    phy_8218b_greenEnable_set,
    phy_8218b_eeeEnable_get,
    phy_8218b_eeeEnable_set,
    phy_8218b_crossOverMode_get,
    phy_8218b_crossOverMode_set,
    phy_8218b_crossOverStatus_get,
    phy_8214fc_fiber_media_get,
    phy_8214fc_fiber_media_set,
    phy_8218b_linkDownPowerSavingEnable_get,
    phy_8218b_linkDownPowerSavingEnable_set,
    phy_8214fc_broadcastEnable_set,
    phy_8214fc_broadcastID_set,
    phy_8218b_gigaLiteEnable_get,
    phy_8218b_gigaLiteEnable_set,
    phy_8218b_eeepEnable_get,
    phy_8218b_eeepEnable_set,
    phy_8214fc_mp_patch_set,
    phy_8218b_downSpeedEnable_get,
    phy_8218b_downSpeedEnable_set,
    phy_8214fc_fiberDownSpeedEnable_get,
    phy_8214fc_fiberDownSpeedEnable_set,
    phy_8214fc_fiberNwayForceLink_get,
    phy_8214fc_fiberNwayForceLink_set,
    phy_8214fc_fiberOAMLoopBack_set,
    phy_8218b_ptpSwitchMacAddr_get,
    phy_8218b_ptpSwitchMacAddr_set,
    phy_8218b_ptpRefTime_get,
    phy_8218b_ptpRefTime_set,
    phy_8218b_ptpRefTimeAdjust_set,
    phy_8218b_ptpRefTimeEnable_get,
    phy_8218b_ptpRefTimeEnable_set,
    phy_8218b_ptpEnable_get,
    phy_8218b_ptpEnable_set,
    phy_8218b_ptpRxTimestamp_get,
    phy_8218b_ptpTxTimestamp_get,
    phy_8214fc_MP_masterSlave_get,
    phy_8214fc_MP_masterSlave_set,
    phy_8214fc_fiberInternalLoopBack_set,
}; /* end of phy_8214FCdrv_MP_ge */

rt_phydrv_t phy_8380drv_int_ge =
{
    RT_PHYDRV_RTL8380_INT_GE,
    phy_8218b_init,
    phy_8218b_media_get,
    (int32 (*)(uint32, rtk_port_t, rtk_port_media_t))phy_common_unavail,
    phy_common_autoNegoEnable_get,
    phy_common_autoNegoEnable_set,
    phy_8218b_autoNegoAbility_get,
    phy_8218b_autoNegoAbility_set,
    phy_common_duplex_get,
    phy_common_duplex_set,
    phy_8218b_speed_get,
    phy_8218b_speed_set,
    phy_8380_int_enable_set,
    phy_int8380_rtctResult_get,   /* phy_8218b_rtctResult_get, */
    phy_8218b_rtct_start,
    phy_8218b_greenEnable_get,
    phy_8218b_greenEnable_set,
    phy_8218b_eeeEnable_get,
    phy_8218b_eeeEnable_set,
    phy_8218b_crossOverMode_get,
    phy_8218b_crossOverMode_set,
    phy_8218b_crossOverStatus_get,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t))phy_common_unavail,
    phy_8218b_linkDownPowerSavingEnable_get,
    phy_8218b_linkDownPowerSavingEnable_set,
    phy_8218b_broadcastEnable_set,
    phy_8218b_broadcastID_set,
    phy_8218b_gigaLiteEnable_get,
    phy_8218b_gigaLiteEnable_set,
    phy_8218b_eeepEnable_get,
    phy_8218b_eeepEnable_set,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8218b_downSpeedEnable_get,
    phy_8218b_downSpeedEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8218b_ptpSwitchMacAddr_get,
    phy_8218b_ptpSwitchMacAddr_set,
    phy_8218b_ptpRefTime_get,
    phy_8218b_ptpRefTime_set,
    phy_8218b_ptpRefTimeAdjust_set,
    phy_8218b_ptpRefTimeEnable_get,
    phy_8218b_ptpRefTimeEnable_set,
    phy_8218b_ptpEnable_get,
    phy_8218b_ptpEnable_set,
    phy_8218b_ptpRxTimestamp_get,
    phy_8218b_ptpTxTimestamp_get,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8218Bdrv_ge */

rt_phyInfo_t phy_8218B_info =
{
    PORT_NUM_IN_8218B,
    {0, 0, 0, 0, 0, 0, 0, 0},
};

rt_phyInfo_t phy_8218FB_info =
{
    PORT_NUM_IN_8218FB,
    {0, 0, 0, 0, 1, 1, 1, 1},
};

rt_phyInfo_t phy_8214FC_info =
{
    PORT_NUM_IN_8214FC,
    {1, 1, 1, 1, 0, 0, 0, 0},
};

int32 _phy_8214fc_intMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *media, uint32 *link_status);

void phy_patchBit_set(int unit, int port, int page, int reg, unsigned char endBit,
    unsigned char startBit, unsigned int inVal)
{
    unsigned char   len;
    unsigned char   i;
    unsigned int    val, mask;

    //osal_time_usleep(10);
    len = endBit - startBit + 1;

    if (16 == len)
        val = inVal;
    else
    {
        mask = 0;
        for (i = startBit; i <= endBit; ++i)
            mask |= (1 << i);

        hal_miim_park_read(unit, port, page, 0x1f, reg, &val);
        val &= ~(mask);
        val |= (inVal << startBit);
    }

    hal_miim_park_write(unit, port, page, 0x1f, reg, val);
}

int32 _phy_8218b_autoCrossOver_set(uint32 unit, rtk_port_t port)
{
    rtk_enable_t    pollSts[PORT_NUM_IN_8218B];
    rtk_port_t  pid;
    uint32      val, modelId;
    uint32      romId, maxPage = HAL_MIIM_PAGE_ID_MAX(unit);
    int         ret, i;

    if (0 != (port % PORT_NUM_IN_8218B))
        return RT_ERR_OK;

    val = 0x4;
    ret = hal_miim_write(unit, port, 0, 27, val);
    if(ret != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 0, 28, &romId)) != RT_ERR_OK)
         return ret;

    if ((ret = hal_miim_read(unit, port, 0, 3, &val)) != RT_ERR_OK)
         return ret;

    modelId = (val & ModelNumber_MASK) >> ModelNumber_OFFSET;
    if (PHY_MODEL_ID_RTL8218B_INT == modelId)
    {
        if (romId != 1)
            return RT_ERR_OK;
    }
    else
    {
        if (romId > 2)
            return RT_ERR_OK;
    }

    for (i = 0; i < PORT_NUM_IN_8218B; ++i)
    {
        int j;

        pid = port + i;

        /* patch request */
        if ((ret = hal_miim_write(unit, pid, 0xb82, 16, 0x10)) != RT_ERR_OK)
            return ret;

        osal_time_usleep(1000);

        /* patch ready */
        for (j = 0; j < 100; ++j)
        {
            if ((ret = hal_miim_read(unit, pid, 0xb80, 16, &val)) != RT_ERR_OK)
                return ret;

            if ((val & 0x40) != 0)
            {
                break;
            }
        }

        if ((val & 0x40) == 0)
        {
            osal_printf("port %d patch ready fail %x\n", pid, val);
        }

        hal_miim_pollingEnable_get(unit, pid, &pollSts[i]);
        hal_miim_pollingEnable_set(unit, pid, DISABLED);

        /*  #patch key sram 0x8146 */
        phy_patchBit_set(unit, pid, maxPage, 0x001f, 15, 0, 0x0a43);
        /*  #patch key sram 0x8146 */
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0x8146);
        if (PHY_MODEL_ID_RTL8218B_INT == modelId)
        {
            phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, (0x7500 + romId));
        }
        else
        {
            phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, (0x7600 + romId));
        }

        /* #patch lock page 0xb82, reg.23.b0 */
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xb82e);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0001);

        phy_patchBit_set(unit, pid, maxPage, 0x001f, 15, 0, 0x0a43);

        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xB820);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0090);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xA012);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0000);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xA014);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c04);

        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3f);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3f);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3f);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd70c);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6111);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6090);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd076);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd188);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c13);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd076);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd188);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c13);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6090);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd077);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd188);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c13);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd078);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd188);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd03b);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd198);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd700);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x3220);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x3c3e);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd024);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd18b);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd012);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd19b);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x3231);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x4c3e);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd70c);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6212);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6111);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6090);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd07a);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd07b);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6090);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd07c);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd07d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6111);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6090);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd07e);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd07f);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x6090);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd080);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2c3d);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd081);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0xd189);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x2511);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x251b);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xA01A);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0000);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xA006);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0fff);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xA004);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0fff);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xA002);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0fff);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xA000);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x1504);
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0xB820);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0010);

        /* ####clear patch_key & patch_lock##### */
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0x0000);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0000);
        /*  #patch lock page 0xb82, reg.23.b0 */
        phy_patchBit_set(unit, pid, maxPage, 0x001f, 15, 0, 0x0b82);
        phy_patchBit_set(unit, pid, maxPage, 0x0017, 15, 0, 0x0000);
        phy_patchBit_set(unit, pid, maxPage, 0x001f, 15, 0, 0x0a43);
        /* #patch key sram 0x8146 */
        phy_patchBit_set(unit, pid, maxPage, 0x001B, 15, 0, 0x8146);
        phy_patchBit_set(unit, pid, maxPage, 0x001C, 15, 0, 0x0000);

        /* patch release */
        phy_patchBit_set(unit, pid, 0xb82, 16, 4, 4, 0);
        phy_patchBit_set(unit, pid, maxPage, 31, 15, 0, 0);

        hal_miim_pollingEnable_set(unit, pid, pollSts[i]);
    }

    for (i = 0; i < PORT_NUM_IN_8218B; ++i)
    {
        pid = port + i;

        hal_miim_pollingEnable_get(unit, pid, &pollSts[i]);
        hal_miim_pollingEnable_set(unit, pid, DISABLED);
    }

    osal_time_usleep(20000);

    for (i = 0; i < PORT_NUM_IN_8218B; ++i)
    {
        pid = port + i;

        phy_patchBit_set(unit, pid, maxPage, 0x001f, 15, 0, 0x0a45);
        phy_patchBit_set(unit, pid, maxPage, 0x0016, 2, 0, i);
        phy_patchBit_set(unit, pid, maxPage, 31, 15, 0, 0);
    }

    phy_patchBit_set(unit, port + 3, maxPage, 0x001f, 15, 0, 0x0a45);
    phy_patchBit_set(unit, port + 3, maxPage, 0x0016, 2, 0, 2);
    phy_patchBit_set(unit, port + 3, maxPage, 31, 15, 0, 0);

    phy_patchBit_set(unit, port + 7, maxPage, 0x001f, 15, 0, 0x0a45);
    phy_patchBit_set(unit, port + 7, maxPage, 0x0016, 2, 0, 6);
    phy_patchBit_set(unit, port + 7, maxPage, 31, 15, 0, 0);

    for (i = 0; i < PORT_NUM_IN_8218B; ++i)
    {
        pid = port + i;
        hal_miim_pollingEnable_set(unit, pid, pollSts[i]);
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_init
 * Description:
 *      Initialize PHY 8218.
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
phy_8218b_init(uint32 unit, rtk_port_t port)
{
    uint32  base_port = 0;
    uint32  val;
    int32   ret = RT_ERR_FAILED;
    rtk_port_media_t restore_media;

    base_port = port - (port % PORT_NUM_IN_8218B);

    /* Initialize the PHY software shadow */
    if (phyInfo_alloc[unit] == 0)
    {
        pPhy_info[unit] = (hal_phy_info_t *)osal_alloc(sizeof(hal_phy_info_t));
        if (NULL == pPhy_info[unit])
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "memory allocate failed");
            return RT_ERR_FAILED;
        }
        osal_memset(pPhy_info[unit], 0, sizeof(hal_phy_info_t));
        phyInfo_alloc[unit] = 1;
    }

    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    /*
     * 0b00: mean receive IPG 64 bit time
     * 0b01: mean receive IPG 32 bit time
     * For Marvell EEE IOP issue, Need to take care to RTL8218B
     */

    /*Update RTCT threshold*/

    if ((ret = phy_media_get(unit, port, &restore_media)) != RT_ERR_OK)
        return ret;

    if (restore_media != PORT_MEDIA_COPPER)
    {
        if ((ret = phy_media_set(unit, port, PORT_MEDIA_COPPER)) != RT_ERR_OK)
            return ret;
    }
     /* Set default value for reset checking in waMon */
     if ((ret = hal_miim_write(unit, base_port, 0, 27, 0x801e)) != RT_ERR_OK)
         return ret;
     if ((ret = hal_miim_write(unit, base_port, 0, 28, 0x1)) != RT_ERR_OK)
         return ret;

#if defined(CONFIG_SDK_RTL8380)
    /* For 838x & 833x, set related regs to enable serdes always linkup */
    if ((HAL_IS_RTL8380_FAMILY_ID(unit)) || (HAL_IS_RTL8330_FAMILY_ID(unit)))
    {
        uint32 phy_data;

        /*18B or 14FC*/
        if((port %8) == 0)
        {
            phy_data = 8;
            ret = hal_miim_write(unit,  port,  0,  30, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0x9703;
            ret = hal_miim_write(unit,  port,  0x404,  0x10, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0x9403;
            ret = hal_miim_write(unit,  port,  0x424,  0x10, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0;
            ret = hal_miim_write(unit,  port,  0,  30, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
    }

    /* For 838x & 833x, Set PKTGEN RXPATH is from MAC TX & Enable PKTGEN RX for Phy error counter */
    if ((HAL_IS_RTL8380_FAMILY_ID(unit)) || (HAL_IS_RTL8330_FAMILY_ID(unit)))
    {
        uint32 phy_data;

        ret = hal_miim_read(unit,  port,  0xc80,  16, &phy_data);
        if(ret != RT_ERR_OK)
            return ret;

        phy_data &= ~0xF;
        phy_data |= 0x1<<2;
        phy_data |= 0x2<<0;
        ret = hal_miim_write(unit,  port,  0xc80,  16, phy_data);
        if(ret != RT_ERR_OK)
            return ret;
    }
#endif

    /* Read the chip copper Auto-1000F ability value and initial the shadow value */
    if ((ret = hal_miim_read(unit, port, 0xa42, 9, &val)) != RT_ERR_OK)
        return ret;
    /* store to shadow */
    (*pPhy_info[unit]).auto_1000f[port] = (val >> _1000Base_TFullDuplex_OFFSET) & 0x1;

    if (restore_media != PORT_MEDIA_COPPER)
    {
        if ((ret = phy_media_set(unit, port, restore_media)) != RT_ERR_OK)
            return ret;
    }

    phy_8218b_eeeEnable_set(unit, port, DISABLED);

    _phy_8218b_autoCrossOver_set(unit, port);

    return RT_ERR_OK;
} /* end of phy_8218b_init */

/*
 * Function Declaration
 */
/* Function Name:
 *      phy_8214fc_init
 * Description:
 *      Initialize PHY 8214fc.
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
phy_8214fc_init(uint32 unit, rtk_port_t port)
{
    uint32  base_port = 0;
    uint32  val;
    uint32 phy_data;
    int32   ret = RT_ERR_FAILED;
    rtk_port_media_t restore_media;

    base_port = port - (port % PORT_NUM_IN_8214FC);

    /* Initialize the PHY software shadow */
    if (phyInfo_alloc[unit] == 0)
    {
        pPhy_info[unit] = (hal_phy_info_t *)osal_alloc(sizeof(hal_phy_info_t));
        if (NULL == pPhy_info[unit])
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "memory allocate failed");
            return RT_ERR_FAILED;
        }
        osal_memset(pPhy_info[unit], 0, sizeof(hal_phy_info_t));
        phyInfo_alloc[unit] = 1;
    }

    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    /*
     * 0b00: mean receive IPG 64 bit time
     * 0b01: mean receive IPG 32 bit time
     * For Marvell EEE IOP issue, Need to take care to RTL8218B
     */

    /*Update RTCT threshold*/

    if ((ret = phy_media_get(unit, port, &restore_media)) != RT_ERR_OK)
        return ret;

    if (restore_media != PORT_MEDIA_COPPER)
    {
        if ((ret = phy_media_set(unit, port, PORT_MEDIA_COPPER)) != RT_ERR_OK)
            return ret;
    }

    /* Set default value for reset checking in waMon */
    if ((ret = hal_miim_write(unit, base_port, 0, 29, 0x8)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, base_port, 0x268, 16, &phy_data)) != RT_ERR_OK)
         return ret;

    phy_data &= ~(0xF<<12);
    phy_data |= (0xA<<12);

    if ((ret = hal_miim_write(unit, base_port, 0x268, 16, phy_data)) != RT_ERR_OK)
         return ret;

    if ((ret = hal_miim_write(unit, base_port, 0, 29, 0x0)) != RT_ERR_OK)
        return ret;

#if defined(CONFIG_SDK_RTL8380)
    /* For 838x & 833x, set related regs to enable serdes always linkup */
    if ((HAL_IS_RTL8380_FAMILY_ID(unit)) || (HAL_IS_RTL8330_FAMILY_ID(unit)))
    {
        uint32 phy_data;

        /*18B or 14FC*/
        if((port %8) == 0)
        {
            phy_data = 8;
            ret = hal_miim_write(unit,  port,  0,  30, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0x9703;
            ret = hal_miim_write(unit,  port,  0x404,  0x10, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0x9403;
            ret = hal_miim_write(unit,  port,  0x424,  0x10, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0;
            ret = hal_miim_write(unit,  port,  0,  30, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }
    }

    /* For 838x & 833x, Set PKTGEN RXPATH is from MAC TX & Enable PKTGEN RX for Phy error counter */
    if ((HAL_IS_RTL8380_FAMILY_ID(unit)) || (HAL_IS_RTL8330_FAMILY_ID(unit)))
    {
        uint32 phy_data;

        ret = hal_miim_read(unit,  port,  0xc80,  16, &phy_data);
        if(ret != RT_ERR_OK)
            return ret;

        phy_data &= ~0xF;
        phy_data |= 0x1<<2;
        phy_data |= 0x2<<0;
        ret = hal_miim_write(unit,  port,  0xc80,  16, phy_data);
        if(ret != RT_ERR_OK)
            return ret;
    }
#endif

    /* Read the chip copper Auto-1000F ability value and initial the shadow value */
    if ((ret = hal_miim_read(unit, port, 0xa42, 9, &val)) != RT_ERR_OK)
        return ret;
    /* store to shadow */
    (*pPhy_info[unit]).auto_1000f[port] = (val >> _1000Base_TFullDuplex_OFFSET) & 0x1;

    if (restore_media != PORT_MEDIA_COPPER)
    {
        if ((ret = phy_media_set(unit, port, restore_media)) != RT_ERR_OK)
            return ret;
    }

    phy_8214fc_fiber_media_set(unit, port, PORT_FIBER_MEDIA_AUTO);
    phy_8214fc_fiberDownSpeedEnable_set(unit, port, DISABLED);

    phy_8218b_eeeEnable_set(unit, port, DISABLED);

#if defined(CONFIG_SDK_RTL8390)
    if ((HAL_IS_RTL8390_FAMILY_ID(unit)) || (HAL_IS_RTL8350_FAMILY_ID(unit)))
    {
        if ((ret = hal_miim_write(unit, port, 0, 29, 0x1)) != RT_ERR_OK)
            return ret;

        if ((ret = hal_miim_write(unit, port, 0xa5d, 0x10, 0x0)) != RT_ERR_OK)
             return ret;

        if ((ret = hal_miim_write(unit, port, 0, 29, 0x3)) != RT_ERR_OK)
            return ret;

        if ((ret = hal_miim_write(unit, port, 0xd, 0x10, 0x119A)) != RT_ERR_OK)
             return ret;

        if ((ret = hal_miim_write(unit, port, 0, 29, 0x0)) != RT_ERR_OK)
            return ret;
    }
#endif  /* CONFIG_SDK_RTL8390 */
    return RT_ERR_OK;
} /* end of phy_8214fc_init */

/*
 * Function Declaration
 */
/* Function Name:
 *      phy_8218fb_init
 * Description:
 *      Initialize PHY 8218fb.
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
phy_8218fb_init(uint32 unit, rtk_port_t port)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
    {
        phy_8214fc_init(unit, port);
    }
    else
    {
        phy_8218b_init(unit, port);
    }

    return RT_ERR_OK;
} /* end of phy_8218fb_init */

/* Function Name:
 *      phy_8218b_autoNegoAbility_get
 * Description:
 *      Get ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218b_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData4;
    uint32  phyData9;
    rtk_enable_t     enable;

    phy_common_autoNegoEnable_get(unit, port, &enable);

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
    pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
        return ret;

    pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
    pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
    pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
    pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
    pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
    pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;

    return ret;
} /* end of phy_8218b_autoNegoAbility_get */

/* Function Name:
 *      phy_8218b_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218b_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData0;
    uint32  phyData4;
    uint32  phyData9;
    rtk_enable_t     enable;

    phy_common_autoNegoEnable_get(unit, port, &enable);

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
    phyData4 = phyData4
            | (pAbility->FC << Pause_R4_OFFSET)
            | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
        return ret;

    phyData4 = phyData4 &
            ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
    phyData4 = phyData4
            | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
            | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
            | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
            | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

    phyData9 = phyData9 & ~(_1000Base_TFullDuplex_MASK | _1000Base_THalfDuplex_MASK);
    phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
               | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
        return ret;


    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
        return ret;

    /* Force re-autonegotiation if AN is on*/
    if (ENABLED == enable)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
    }

    return ret;
} /* end of phy_8218b_autoNegoAbility_set */

/* Function Name:
 *      phy_8218b_speed_get
 * Description:
 *      Get link speed status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218b_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    int32   ret;
    uint32  val;
    uint32  phyData0;
    rtk_port_media_t restore_media, media;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
                  | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = phy_media_get(unit, port, &restore_media)) != RT_ERR_OK)
        return ret;

    if (restore_media == PORT_MEDIA_FIBER_AUTO)
    {
        media = PORT_MEDIA_FIBER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else if (restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        media = PORT_MEDIA_COPPER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else
        media = restore_media;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
              | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);

    if (restore_media == PORT_MEDIA_FIBER_AUTO || restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        if ((ret = phy_media_set(unit, port, restore_media)) != RT_ERR_OK)
            return ret;
    }

    return ret;
} /* end of phy_8218b_speed_get */

/* Function Name:
 *      phy_8218b_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
int32
phy_8218b_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32   ret;
    uint32  phyData0;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8218b_speed_set */

/* Function Name:
 *      phy_8218fb_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
int32
phy_8218fb_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32   ret;
    uint32  val;
    uint32  phyData0;
    rtk_port_media_t restore_media, media;

    RT_PARAM_CHK(((port % PORT_NUM_IN_8218FB) < 4) && (speed == PORT_SPEED_1000M), RT_ERR_CHIP_NOT_SUPPORTED);

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
        phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;

        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = phy_media_get(unit, port, &restore_media)) != RT_ERR_OK)
        return ret;

    if (restore_media == PORT_MEDIA_FIBER_AUTO)
    {
        media = PORT_MEDIA_FIBER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else if (restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        media = PORT_MEDIA_COPPER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else
        media = restore_media;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if (restore_media == PORT_MEDIA_FIBER_AUTO || restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        if ((ret = phy_media_set(unit, port, restore_media)) != RT_ERR_OK)
            return ret;
    }

    return ret;
} /* end of phy_8218fb_speed_set */

/* Function Name:
 *      phy_8214fc_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
int32
phy_8214fc_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32   ret;
    uint32  val;
    uint32  phyData0;
    rtk_port_media_t restore_media, media;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
        phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;

        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = phy_media_get(unit, port, &restore_media)) != RT_ERR_OK)
        return ret;

    if (restore_media == PORT_MEDIA_FIBER_AUTO)
    {
        media = PORT_MEDIA_FIBER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else if (restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        media = PORT_MEDIA_COPPER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else
        media = restore_media;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if (restore_media == PORT_MEDIA_FIBER_AUTO || restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        if ((ret = phy_media_set(unit, port, restore_media)) != RT_ERR_OK)
            return ret;
    }

    return ret;
} /* end of phy_8214fc_speed_set */


/* Function Name:
 *      phy_8380_int_enable_set
 * Description:
 *      Set PHY interface status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8380_int_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  phyData, phyData_restore;
    rtk_port_media_t media;

    if ((ret = phy_media_get(unit, port, &media)) != RT_ERR_OK)
    {
        return ret;
    }

    if (media == PORT_MEDIA_FIBER_AUTO || media == PORT_MEDIA_COPPER_AUTO)
    {
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;

        if (DISABLED == enable)
        {
            /* isolate first */
            phyData &= ~(Isolate_MASK);
            phyData |= (1 << Isolate_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;

            /* power down */
            phyData &= ~(PowerDown_MASK);
            phyData |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;

            /* isolate release */
            phyData &= ~(Isolate_MASK);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            /* set not isolate and power up at the same time */
            phyData &= ~(PowerDown_MASK);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }

        /* Configure the GPHY page to fiber */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0003)) != RT_ERR_OK)
            return ret;

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;

        if (DISABLED == enable)
        {
            /* isolate first */
            phyData &= ~(Isolate_MASK);
            phyData |= (1 << Isolate_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;

            /* power down */
            phyData &= ~(PowerDown_MASK);
            phyData |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;

            /* isolate release */
            phyData &= ~(Isolate_MASK);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            /* set not isolate and power up at the same time */
            phyData &= ~(PowerDown_MASK);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    else
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;


        if (DISABLED == enable)
        {
            /* isolate first */
            phyData &= ~(Isolate_MASK);
            phyData |= (1 << Isolate_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;

            /* power down */
            phyData &= ~(PowerDown_MASK);
            phyData |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;

            /* isolate release */
            phyData &= ~(Isolate_MASK);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            if((phyData & PowerDown_MASK) != 0) /*Original Status is disable*/
            {
                phyData &= ~(PowerDown_MASK);

                /*E0015926*/
                if(media == PORT_MEDIA_COPPER)
                {
                    phyData_restore = phyData;
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                        return ret;

                    phyData |= (1 << Reset_OFFSET);
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                        return ret;
                    while(1)
                    {
                        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
                            return ret;
                        if((phyData & Reset_MASK) == 0)
                        {
                            phyData = phyData_restore;
                            break;
                        }
                    }
                }
            }
            else
            {
                /*Original Status is enable*/
                phyData &= ~(PowerDown_MASK);
            }

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }
    }
    return ret;
} /* end of phy_8380_int_enable_set */


/* Function Name:
 *      phy_8218b_enable_set
 * Description:
 *      Set PHY interface status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218b_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  phyData, phyData_restore;
    rtk_port_media_t media;

    if ((ret = phy_media_get(unit, port, &media)) != RT_ERR_OK)
    {
        return ret;
    }

    if (media == PORT_MEDIA_FIBER_AUTO || media == PORT_MEDIA_COPPER_AUTO)
    {
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;
        phyData &= ~(PowerDown_MASK);
        if (DISABLED == enable)
            phyData |= (1 << PowerDown_OFFSET);
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;

        /* Configure the GPHY page to fiber */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0003)) != RT_ERR_OK)
            return ret;

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;
        phyData &= ~(PowerDown_MASK);
        if (DISABLED == enable)
            phyData |= (1 << PowerDown_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    else
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;


        if (DISABLED == enable)
        {
            phyData |= (1 << PowerDown_OFFSET);
        }else{
            if((phyData & PowerDown_MASK) != 0) /*Original Status is disable*/
            {
                phyData &= ~(PowerDown_MASK);
                /*E0015926*/
                if(media == PORT_MEDIA_COPPER)
                {
                    phyData_restore = phyData;
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                        return ret;

                    phyData |= (1 << Reset_OFFSET);
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                        return ret;
                    while(1)
                    {
                        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
                            return ret;
                        if((phyData & Reset_MASK) == 0)
                        {
                            phyData = phyData_restore;
                            break;
                        }
                    }
                }
            }else{  /*Original Status is enable*/
                phyData &= ~(PowerDown_MASK);
            }
        }

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;
    }
    return ret;
} /* end of phy_8218b_enable_set */

/* Function Name:
 *      phy_8218b_rtctResult_get
 * Description:
 *      Get test result of RTCT.
 * Input:
 *      unit        - unit id
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The result unit is cm
 */
int32
phy_8218b_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData, fixed_page;
    uint32  speed;

        /* Check the port is link up or not? */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData)) != RT_ERR_OK)
            return ret;

        phy_common_speed_get(unit, port, &speed);

        if ((phyData & LinkStatus_MASK) && speed != 0)
        {
            /* If the port is link up,
                     * return cable length from green function
                     */

            fixed_page = 0xa88;

            /* The Length is store in [7:0], and the unit is meter*/
            if ((ret = hal_miim_read(unit, port, fixed_page, 16, &phyData)) != RT_ERR_OK)
                return ret;

            pRtctResult->linkType = PORT_SPEED_1000M;
            pRtctResult->ge_result.channelALen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelBLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelCLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelDLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelAShort = 0;
            pRtctResult->ge_result.channelBShort = 0;
            pRtctResult->ge_result.channelCShort = 0;
            pRtctResult->ge_result.channelDShort = 0;
            pRtctResult->ge_result.channelAOpen = 0;
            pRtctResult->ge_result.channelBOpen = 0;
            pRtctResult->ge_result.channelCOpen = 0;
            pRtctResult->ge_result.channelDOpen = 0;
            pRtctResult->ge_result.channelAMismatch = 0;
            pRtctResult->ge_result.channelBMismatch = 0;
            pRtctResult->ge_result.channelCMismatch = 0;
            pRtctResult->ge_result.channelDMismatch = 0;
            pRtctResult->ge_result.channelALinedriver = 0;
            pRtctResult->ge_result.channelBLinedriver = 0;
            pRtctResult->ge_result.channelCLinedriver = 0;
            pRtctResult->ge_result.channelDLinedriver = 0;
        }
        else
        {
            /* If the port is link down,
                      * return cable length from RTCT function
                      */
            /* Page 0xa42, Register 17
                      * bit[15]: cable test finished or not?
                      *             1: Finished
                      *             0: Not finished
                      */
            fixed_page = 0xa42;

            if ((ret = hal_miim_read(unit, port, fixed_page, 17, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_DONE) & 0x1) != 0x1)
                return RT_ERR_PHY_RTCT_NOT_FINISH;

            pRtctResult->linkType = PORT_SPEED_1000M;
            /* Length = (Index/64)*8ns*(0.2m/ns) = Index/80 (m) = (1.25) * Index (cm) */

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_A)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelALen = (phyData)*5/4;

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_B)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelBLen = (phyData)*5/4;

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_C)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelCLen = (phyData)*5/4;

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_D)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelDLen = (phyData)*5/4;

            /* === Channel A Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_A)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelALinedriver = 1;

            /* === Channel B Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_B)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBLinedriver = 1;

            /* === Channel C Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_C)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCLinedriver = 1;

            /* === Channel D Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_D)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDLinedriver = 1;
        }

    return ret;
} /* end of phy_8218b_rtctResult_get */

/* Function Name:
 *      phy_8218b_rtct_start
 * Description:
 *      Start PHY interface RTCT test of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - chip not supported
 * Note:
 *      None
 */
int32
phy_8218b_rtct_start(uint32 unit, rtk_port_t port)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData, fixed_page;
    uint32  speed;

        /* Check the port is link up or not? */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData)) != RT_ERR_OK)
            return ret;

        phy_common_speed_get(unit, port, &speed);

        if ((phyData & LinkStatus_MASK) && speed != 0)
        {
            /* Configure the GPHY page to copper */
            if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
                return ret;

            fixed_page = 0;

            /* get value from CHIP*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, 0x8011)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            /* enable green feature */
            phyData |= (1 << 9);
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, 0x8011)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, fixed_page, 28, phyData)) != RT_ERR_OK)
                return ret;
    
            /* Configure the GPHY page to auto */
            if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            if ((ret = hal_miim_read(unit, port, 0xa42, 17, &phyData)) != RT_ERR_OK)
                return ret;

            phyData |= (1 << RTCT_ENABLE);
            phyData |= (1 << RTCT_CH_A);
            phyData |= (1 << RTCT_CH_B);
            phyData |= (1 << RTCT_CH_C);
            phyData |= (1 << RTCT_CH_D);

            if ((ret = hal_miim_write(unit, port, 0xa42, 17, phyData)) != RT_ERR_OK)
                return ret;
        }

    return ret;
} /* end of phy_8218b_rtct_start */

/* Function Name:
 *      phy_8218b_greenEnable_get
 * Description:
 *      Get the status of link-up green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to status of link-up green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8218b is supported the per-port link-up green feature.
 */
int32
phy_8218b_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  phyData, fixed_page;
    rtk_port_media_t    media;
    uint32              link_sts;

    fixed_page = 0;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* get value from CHIP*/
        if ((ret = hal_miim_write(unit, port, fixed_page, 27, 0x8011)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
            return ret;

        if ((phyData >> 9) & 0x1)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218b_greenEnable_get */

/* Function Name:
 *      phy_8218b_greenEnable_set
 * Description:
 *      Set the status of link-up green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-up  green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8218b is supported the per-port link-up green feature.
 */
int32
phy_8218b_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value, fixed_page;
    int32   ret = RT_ERR_FAILED;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        fixed_page = 0;

        /* get value from CHIP*/
        if ((ret = hal_miim_write(unit, port, fixed_page, 27, 0x8011)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_read(unit, port, fixed_page, 28, &value)) != RT_ERR_OK)
            return ret;

        if (ENABLED == enable)
        {
            value |= (1 << 9);
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, 0x8011)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, fixed_page, 28, value)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            value &= ~(1 << 9);
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, 0x8011)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, fixed_page, 28, value)) != RT_ERR_OK)
                return ret;
        }

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218b_greenEnable_set */

/* Function Name:
 *      phy_8218b_eeeEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_8218b_eeeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  phyData;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{

        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, 0xa43, 25, &phyData)) != RT_ERR_OK)
            return ret;

        if (((phyData >> 5) & 0x1) == 0x1)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218b_eeeEnable_get */

/* Function Name:
 *      phy_8218_eeeEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_eeeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t     an_enable;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{

        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        phy_common_autoNegoEnable_get(unit, port, &an_enable);

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, 0xa43, 25, &phyData)) != RT_ERR_OK)
                return ret;

        #if 0 /* always configure to MAC mode EEE, the mode configuration could be aparted to another API in the future */
        if (ENABLED == enable)
        {
            phyData |= (1 << 5);
            if ((ret = hal_miim_write(unit, port, 0xa43, 25, phyData)) != RT_ERR_OK)
                return ret;
        }
        else
        #else
        {
            phyData &= ~(1 << 5);
            if ((ret = hal_miim_write(unit, port, 0xa43, 25, phyData)) != RT_ERR_OK)
                return ret;
        }
        #endif

        if(ENABLED == enable)
            phyData = 0x6; /*enable 100M/1000M EEE ability*/
        else
            phyData = 0x0; /*disable 100M/1000M EEE ability*/

        ret = hal_miim_mmd_write(unit, port, 7, 60, phyData);
        if ((ret != RT_ERR_OK)&&(ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            return ret;
        }

        if ((ret = hal_miim_read(unit, port, 0xa42, 20, &phyData)) != RT_ERR_OK)
        {
            return ret;
        }

        if(ENABLED == enable)
            phyData |= 0x1 << 7; /*enable 500M EEE ability*/
        else
            phyData &= ~(0x1 << 7); /*disable 500M EEE ability*/

        if ((ret = hal_miim_write(unit, port, 0xa42, 20, phyData)) != RT_ERR_OK)
        {
            return ret;
        }

        /* Force re-autonegotiation if AN is on*/
        if (ENABLED == an_enable)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
                return ret;

            phyData = phyData & ~(RestartAutoNegotiation_MASK);
            phyData = phyData | (an_enable << RestartAutoNegotiation_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218b_eeeEnable_set */

/* Function Name:
 *      phy_8218b_crossOverMode_get
 * Description:
 *      Get cross over mode in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
phy_8218b_crossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData, force_mode, mdi;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, 0xa43, 24, &phyData)) != RT_ERR_OK)
            return ret;

        if((phyData & (1 << 9)) != 0)
        {
            force_mode = 1;
            if((phyData & (1 << 8)) != 0)
                mdi = 1;
            else
                mdi = 0;
        }else{
            force_mode = 0;
        }

        if (force_mode)
        {
            if (mdi)
                *pMode = PORT_CROSSOVER_MODE_MDI;
            else
                *pMode = PORT_CROSSOVER_MODE_MDIX;
        }
        else
            *pMode = PORT_CROSSOVER_MODE_AUTO;

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of phy_8218b_crossOverMode_get */

/* Function Name:
 *      phy_8218b_crossOverMode_set
 * Description:
 *      Set cross over mode in the specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - cross over mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
phy_8218b_crossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, 0xa43, 24, &phyData)) != RT_ERR_OK)
            return ret;

        switch (mode)
        {
            case PORT_CROSSOVER_MODE_AUTO:
                phyData &= ~(1 << 9);
                break;
            case PORT_CROSSOVER_MODE_MDI:
                phyData |= (1 << 9);
                phyData |= (1 << 8);
                break;
            case PORT_CROSSOVER_MODE_MDIX:
                phyData |= (1 << 9);
                phyData &= ~(1 << 8);
                break;
            default:
                return RT_ERR_INPUT;
        }

        if ((ret = hal_miim_write(unit, port, 0xa43, 24, phyData)) != RT_ERR_OK)
            return ret;

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218_crossOverMode_set */

/* Function Name:
 *      phy_8218b_crossOverStatus_get
 * Description:
 *      Get cross over status in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 *      RT_ERR_PHY_FIBER_LINKUP - This feature is not supported in this mode
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_STATUS_MDI
 *      - PORT_CROSSOVER_STATUS_MDIX
 */
int32
phy_8218b_crossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, 0xa43, 26, &phyData)) != RT_ERR_OK)
            return ret;

        if((phyData & (1 << 1)) != 0)
        {
            *pStatus = PORT_CROSSOVER_STATUS_MDI;
        }else{
            *pStatus = PORT_CROSSOVER_STATUS_MDIX;
        }

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }

    return RT_ERR_OK;
} /* end of phy_8218b_crossOverStatus_get */


/* Function Name:
 *      phy_8218b_linkDownPowerSavingEnable_get
 * Description:
 *      Get the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of link-down power saving
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8218b is supported the per-port link-down power saving
 */
int32
phy_8218b_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  value;
    int32   ret = RT_ERR_FAILED;

    /* For Link-Down Power Saving (per-port) */
    if ((ret = hal_miim_read(unit, port, 0xa43, 24, &value)) != RT_ERR_OK)
        return ret;

    if (((value >> 2) & 0x1) == 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return RT_ERR_OK;
} /* end of phy_8218b_linkDownPowerSavingEnable_get */

/* Function Name:
 *      phy_8218b_linkDownPowerSavingEnable_set
 * Description:
 *      Set the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-down power saving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8218b is supported the per-port link-down power saving
 */
int32
phy_8218b_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value;
    int32   ret = RT_ERR_FAILED;

    /* For Link-Down Power Saving (per-port) */
    if ((ret = hal_miim_read(unit, port, 0xa43, 24, &value)) != RT_ERR_OK)
        return ret;

    value &= ~(0x1 << 2);
    if (ENABLED == enable)
    {
        value |= (0x1 << 2);
    }
    if ((ret = hal_miim_write(unit, port, 0xa43, 24, value)) != RT_ERR_OK)
        return ret;

    return RT_ERR_OK;
} /* end of phy_8218b_linkDownPowerSavingEnable_set */


/* Function Name:
 *      phy_8218b_gigaLiteEnable_get
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
 *      1. The RTL8218b is supported the per-port Giga Lite feature.
 */
int32
phy_8218b_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  value;
    int32   ret = RT_ERR_FAILED;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
       if ((ret = hal_miim_write(unit, port, GIGA_LITE_CTRL_REG, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* For Giga Lite (per-port) */
       if ((ret = hal_miim_read(unit, port, GIGA_LITE_CTRL_REG, 20, &value)) != RT_ERR_OK)
           return ret;

       if (((value >> 9) & 0x1) == 0x1)
           *pEnable = ENABLED;
       else
           *pEnable = DISABLED;

        /* Configure the GPHY page to auto */
       if ((ret = hal_miim_write(unit, port, GIGA_LITE_CTRL_REG, 29, 0x0000)) != RT_ERR_OK)
           return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218b_gigaLiteEnable_get */

/* Function Name:
 *      phy_8218b_gigaLiteEnable_set
 * Description:
 *      Set the status of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of Giga Lite
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8218b is supported the per-port Giga Lite feature.
 */
int32
phy_8218b_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  value;
    int32   ret = RT_ERR_FAILED;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, GIGA_LITE_CTRL_REG, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* My 500M-Full Ability Enable Option*/
        if ((ret = hal_miim_read(unit, port, 0xa4a, 17, &value)) != RT_ERR_OK)
            return ret;
            value |= (0x1 << 8);
        if ((ret = hal_miim_write(unit, port, 0xa4a, 17, value)) != RT_ERR_OK)
            return ret;

        /* bit[2]: configure 2-pair auto-downspeed */
        /* bit[9]: configure retry speed down to 500M */
        if ((ret = hal_miim_read(unit, port, 0xa44, 17, &value)) != RT_ERR_OK)
            return ret;

        value &= ~(0x1 << 9);
        value &= ~(0x1 << 2);
        if (ENABLED == enable)
        {
            value |= (0x1 << 9);
            value |= (0x1 << 2);
        }
        if ((ret = hal_miim_write(unit, port, 0xa44, 17, value)) != RT_ERR_OK)
            return ret;


        /* For Giga Lite (per-port) */
        if ((ret = hal_miim_read(unit, port, GIGA_LITE_CTRL_REG, 20, &value)) != RT_ERR_OK)
            return ret;

        value &= ~(0x1 << 9);
        if (ENABLED == enable)
        {
            value |= (0x1 << 9);
        }

        if ((ret = hal_miim_write(unit, port, GIGA_LITE_CTRL_REG, 20, value)) != RT_ERR_OK)
            return ret;

        /* Do the restart N-WAY */
        if ((ret = hal_miim_read(unit, port, 0, 0, &value)) != RT_ERR_OK)
            return ret;
        value |= (0x1UL<<9);
        if ((ret = hal_miim_write(unit, port, 0, 0, value)) != RT_ERR_OK)
            return ret;


        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, GIGA_LITE_CTRL_REG, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }


    return RT_ERR_OK;
} /* end of phy_8218b_gigaLiteEnable_set */

/* Function Name:
 *      phy_8218b_media_get
 * Description:
 *      Get PHY 8218B media type.
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
 *      The media type is return PORT_MEDIA_COPPER.
 */
int32
phy_8218b_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    *pMedia = PORT_MEDIA_COPPER;
    return RT_ERR_OK;
} /* end of phy_8218b_media_get */

/* Function Name:
 *      _phy_8214fc_combo_media_get
 * Description:
 *      Get 8214FC & 8218FB combo port media type.
 * Input:
 *      unit        - unit id
 *      basePort    - base port id
 *      reg         - port media register
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      (1) media type is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 */
int32
_phy_8214fc_combo_media_get(uint32 unit, rtk_port_t basePort, uint32 reg,
    rtk_port_media_t *pMedia)
{
    uint32  val;
    int32   ret = RT_ERR_FAILED, rv;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    if ((ret = hal_miim_write(unit, basePort, 0xa42, 29, 0x0008)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, basePort, 0x266, reg, &val)) != RT_ERR_OK)
        goto ERR;

    /* bit[10]: 0b0 (auto mode), 0b1 (force mode)
     * bit[11]: 0b0 - prefer fiber media if bit[11]=0b0, force fiber media if bit[11]=0b1
     *          0b1 - prefer copper media if bit[11]=0b0, force copper media if bit[11]=0b1
     */
    if (0x400 == (val & 0x400))
    {
        if (0x800 == (val & 0x800))
            *pMedia = PORT_MEDIA_COPPER;
        else
            *pMedia = PORT_MEDIA_FIBER;
    }
    else
    {
        if (0x800 == (val & 0x800))
            *pMedia = PORT_MEDIA_COPPER_AUTO;
        else
            *pMedia = PORT_MEDIA_FIBER_AUTO;
    }
ERR:
    if ((rv = hal_miim_write(unit, basePort, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return rv;

    return ret;
}   /* end of _phy_8214fc_combo_media_get */

/* Function Name:
 *      _phy_8214fc_combo_media_set
 * Description:
 *      Set 8214FC & 8218FB combo port media type.
 * Input:
 *      unit        - unit id
 *      basePort    - base port id
 *      port        - port id
 *      reg         - port media register
 *      media       - phy media type
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 * Note:
 *      (1) media type is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 */
int32
_phy_8214fc_combo_media_set(uint32 unit, rtk_port_t basePort, rtk_port_t port,
    uint32 reg, rtk_port_media_t media)
{
    uint32  val;
    uint32  phyOriDataCopper, phyOriDataFiber;
    uint32  phyNewDataCopper, phyNewDataFiber;
    int32   ret = RT_ERR_FAILED, rv;

    /* only store the powerDown bit in combo port */
    /* In auto mode, link from fiber to copper cause to copper power up */
    /* copper */
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 0xa40, 16, &phyOriDataCopper)) != RT_ERR_OK)
        goto ERR;

    /* fiber */
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 16, &phyOriDataFiber)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_write(unit, basePort, 0xa42, 29, 0x0008)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_read(unit, basePort, 0x266, reg, &val)) != RT_ERR_OK)
        goto ERR;

    switch (media)
    {
        case PORT_MEDIA_COPPER:
            val |= (1<<10);
            val |= (1<<11);
            break;
        case PORT_MEDIA_FIBER:
            val |= (1<<10);
            val &= ~(1<<11);
            break;
        case PORT_MEDIA_COPPER_AUTO:
            val &= ~(1<<10);
            val |= (1<<11);
            break;
        case PORT_MEDIA_FIBER_AUTO:
            val &= ~(1<<10);
            val &= ~(1<<11);
            break;
        default:
            break;
    }

    if ((ret = hal_miim_write(unit, basePort, 0x266, reg, val)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_write(unit, basePort, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        goto ERR;

    /* only store the powerDown bit in combo port */
    /* In auto mode, link from fiber to copper cause to copper power up */
    /* Configure copper */
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_read(unit, port, 0xa40, 16, &phyNewDataCopper)) != RT_ERR_OK)
        goto ERR;

    if ((phyOriDataCopper & PowerDown_MASK) != (phyNewDataCopper & PowerDown_MASK))
    {
        if (phyOriDataCopper & PowerDown_MASK)
            phyNewDataCopper |= (1 << PowerDown_OFFSET);
        else
            phyNewDataCopper &= ~(PowerDown_MASK);
    }

    if ((ret = hal_miim_write(unit, port, 0xa40, 16, phyNewDataCopper)) != RT_ERR_OK)
        goto ERR;

    /* Configure fiber */
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 16, &phyNewDataFiber)) != RT_ERR_OK)
        goto ERR;

    if ((phyOriDataFiber & PowerDown_MASK) != (phyNewDataFiber & PowerDown_MASK))
    {
        if (phyOriDataFiber & PowerDown_MASK)
            phyNewDataFiber |= (1 << PowerDown_OFFSET);
        else
            phyNewDataFiber &= ~(PowerDown_MASK);
    }

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 16, phyNewDataFiber)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 3)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_read(unit, port, 8, 20, &val)) != RT_ERR_OK)
        goto ERR;

    /* default setting */
    val &= ~(0xF << 12);
    /* force fiber port to 2.5G */
    if (PORT_MEDIA_COPPER == media)
        val |= (7 << 12);

    if ((ret = hal_miim_write(unit, port, 8, 20, val)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_read(unit, port, 0xC, 16, &val)) != RT_ERR_OK)
        goto ERR;

    /* default setting */
    val &= ~(0xFF);
    /* force fiber port tx */
    if (PORT_MEDIA_COPPER == media)
        val |= 0xEF;
    if ((ret = hal_miim_write(unit, port, 0xC, 16, val)) != RT_ERR_OK)
        goto ERR;

ERR:
    if ((rv = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000)) != RT_ERR_OK)
        return rv;

    return ret;
}   /* end of _phy_8214fc_combo_media_set */

/* Function Name:
 *      phy_8218fb_media_get
 * Description:
 *      Get PHY 8218FB media type.
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
 *      (1) media type is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 */
int32
phy_8218fb_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    uint32  base_port = 0, reg = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    switch (port % PORT_NUM_IN_8218FB)
    {
        case 4:
            reg = 16;
            break;
        case 5:
            reg = 19;
            break;
        case 6:
            reg = 20;
            break;
        case 7:
            reg = 21;
            break;
        default:
            *pMedia = PORT_MEDIA_COPPER;
            return RT_ERR_OK;
    }

    base_port = port - (port % PORT_NUM_IN_8218FB);
    if ((ret = _phy_8214fc_combo_media_get(unit, base_port, reg, pMedia)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8218fb_media_get */

/* Function Name:
 *      phy_8218fb_media_set
 * Description:
 *      Get PHY 8218FB media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      (1) media type is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 */
int32
phy_8218fb_media_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    uint32  base_port = 0, reg = 0;
    int32   ret = RT_ERR_FAILED;

    switch (port % PORT_NUM_IN_8218FB)
    {
        case 4:
            reg = 16;
            break;
        case 5:
            reg = 19;
            break;
        case 6:
            reg = 20;
            break;
        case 7:
            reg = 21;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    base_port = port - (port % PORT_NUM_IN_8218FB);
    if ((ret = _phy_8214fc_combo_media_set(unit, base_port, port, reg, media)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8218fb_media_set */

/* Function Name:
 *      phy_8214fc_media_get
 * Description:
 *      Get PHY 8214FC media type.
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
 *      (1) media type is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 */
int32
phy_8214fc_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    uint32  base_port = 0, reg = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    switch (port % PORT_NUM_IN_8214FC)
    {
        case 0:
            reg = 16;
            break;
        case 1:
            reg = 19;
            break;
        case 2:
            reg = 20;
            break;
        case 3:
            reg = 21;
            break;
        default:
            *pMedia = PORT_MEDIA_COPPER;
            return RT_ERR_OK;
    }

    base_port = port - (port % PORT_NUM_IN_8214FC);
    if ((ret = _phy_8214fc_combo_media_get(unit, base_port, reg, pMedia)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fc_media_get */

/* Function Name:
 *      phy_8214fc_media_set
 * Description:
 *      Get PHY 8214FC media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      (1) media type is as following:
 *          - PORT_MEDIA_COPPER
 *          - PORT_MEDIA_FIBER
 *          - PORT_MEDIA_COPPER_AUTO
 *          - PORT_MEDIA_FIBER_AUTO
 */
int32
phy_8214fc_media_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    uint32  base_port = 0, reg = 0;
    int32   ret = RT_ERR_FAILED;

    switch (port % PORT_NUM_IN_8214FC)
    {
        case 0:
            reg = 16;
            break;
        case 1:
            reg = 19;
            break;
        case 2:
            reg = 20;
            break;
        case 3:
            reg = 21;
            break;
        default:
            return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    base_port = port - (port % PORT_NUM_IN_8214FC);
    if ((ret = _phy_8214fc_combo_media_set(unit, base_port, port, reg, media)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fc_media_set */

/* Function Name:
 *      phy_8218b_broadcastEnable_set
 * Description:
 *      Set enable status of broadcast mode
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - broadcast enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218b_broadcastEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  base_port = 0;
    uint32  phyData;

    base_port = port - (port % PORT_NUM_IN_8218B);

    /* get value from CHIP*/
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0008)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 31, 0x0266)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, base_port, 0xa42, 22, &phyData)) != RT_ERR_OK)
        return ret;
    if (enable)
        phyData |= (0x1 << (port-base_port+8));
    else
        phyData &= ~(0x1 << (port-base_port+8));
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 22, phyData)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8218b_broadcastEnable_set */

/* Function Name:
 *      phy_8218b_broadcastID_set
 * Description:
 *      Set broadcast ID
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      broadcastID   - broadcast ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218b_broadcastID_set(uint32 unit, rtk_port_t port, uint32 broadcastID)
{
    int32   ret;
    uint32  base_port = 0;
    uint32  phyData;

    base_port = port - (port % PORT_NUM_IN_8218B);

    /* get value from CHIP*/
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0008)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 31, 0x0266)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, base_port, 0xa42, 22, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(0x1f);
    phyData |= (broadcastID & 0x1f);
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 22, phyData)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8218b_broadcastID_set */

/* Function Name:
 *      phy_8214fc_broadcastEnable_set
 * Description:
 *      Set enable status of broadcast mode
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - broadcast enable status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_broadcastEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  base_port = 0;
    uint32  phyData;

    base_port = port - (port % PORT_NUM_IN_8214FC);

    /* get value from CHIP*/
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0008)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 31, 0x0266)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, base_port, 0xa42, 22, &phyData)) != RT_ERR_OK)
        return ret;
    if (enable)
        phyData |= (0x1 << (port-base_port+8));
    else
        phyData &= ~(0x1 << (port-base_port+8));
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 22, phyData)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fc_broadcastEnable_set */

/* Function Name:
 *      phy_8214fc_broadcastID_set
 * Description:
 *      Set broadcast ID
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      broadcastID   - broadcast ID
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_broadcastID_set(uint32 unit, rtk_port_t port, uint32 broadcastID)
{
    int32   ret;
    uint32  base_port = 0;
    uint32  phyData;

    base_port = port - (port % PORT_NUM_IN_8214FC);

    /* get value from CHIP*/
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0008)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 31, 0x0266)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, base_port, 0xa42, 22, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(0x1f);
    phyData |= (broadcastID & 0x1f);
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 22, phyData)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, base_port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fc_broadcastID_set */

/* Function Name:
 *      phy_8218fb_autoNegoAbility_get
 * Description:
 *      Get ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  val, phyData4, phyData9;
    rtk_port_media_t restore_media, media;
    hal_control_t   *pHalCtrl;
    uint32  phyExtStatus15, is_fiber_linkup = 0, is_copper_linkup = 0;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;

    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
            return ret;

        if (is_fiber_linkup)
        {
            /* 1000Base-X Linkup */
            pAbility->FC = (phyData4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
            pAbility->AsyFC = (phyData4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
            pAbility->Half_10 = 0;
            pAbility->Full_10 = 0;
            pAbility->Half_100 = 0;
            pAbility->Full_100 = 0;
            pAbility->Half_1000 = (phyData4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
            pAbility->Full_1000 = (phyData4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;
        }
        else if (is_copper_linkup)
        {
            pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
            pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;

            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
                return ret;

            pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
            pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
            pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
            pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
            pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
            pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;
        }
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = phy_media_get(unit, port, &restore_media)) != RT_ERR_OK)
        return ret;

    if (restore_media == PORT_MEDIA_FIBER_AUTO)
    {
        media = PORT_MEDIA_FIBER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else if (restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        media = PORT_MEDIA_COPPER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else
        media = restore_media;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        pAbility->FC = (phyData4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
        pAbility->AsyFC = (phyData4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
        pAbility->Half_10 = 0;
        pAbility->Full_10 = 0;
        pAbility->Half_100 = 0;
        pAbility->Full_100 = 0;
        pAbility->Half_1000 = (phyData4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
        pAbility->Full_1000 = (phyData4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;
    }
    else
    {
        pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
        pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
            return ret;

        pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
        pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
        pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
        pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
        pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
        pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;
    }

    if (restore_media == PORT_MEDIA_FIBER_AUTO || restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        if ((ret = phy_media_set(unit, port, restore_media)) != RT_ERR_OK)
            return ret;
    }

    return ret;
} /* end of phy_8218fb_autoNegoAbility_get */

/* Function Name:
 *      phy_8218fb_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData0;
    uint32  phyData4;
    uint32  phyData9;
    uint32  val;
    rtk_enable_t     enable;
    rtk_port_media_t restore_media, media;
    hal_control_t   *pHalCtrl;
    uint32  phyExtStatus15, is_fiber_linkup = 0, is_copper_linkup = 0;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;

    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        phy_autoNegoEnable_get(unit, port, &enable);

        /* get value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
            return ret;

        if (is_fiber_linkup)
        {
            /* 1000Base-X Linkup */
            phyData4 = phyData4 & ~(_1000BaseX_Pause_R4_MASK | _1000BaseX_AsymmetricPause_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET)
                    | (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET);
            phyData4 = phyData4 & ~(_1000BaseX_HalfDuplex_R4_MASK | _1000BaseX_FullDuplex_R4_MASK);
            phyData4 = phyData4 | (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET)
                    | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
                return ret;
        }
        else if (is_copper_linkup)
        {
            phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->FC << Pause_R4_OFFSET)
                    | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
                return ret;

            phyData4 = phyData4 &
                    ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
                    | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
                    | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
                    | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

            phyData9 = phyData9 & ~(_1000Base_TFullDuplex_MASK | _1000Base_THalfDuplex_MASK);
            phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                       | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
                return ret;

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
                return ret;
        }

        /* Force re-autonegotiation if AN is on */
        if (ENABLED == enable)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
                return ret;

            phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
            phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
                return ret;
        }
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = phy_media_get(unit, port, &restore_media)) != RT_ERR_OK)
        return ret;

    if (restore_media == PORT_MEDIA_FIBER_AUTO)
    {
        media = PORT_MEDIA_FIBER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else if (restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        media = PORT_MEDIA_COPPER;
        if ((ret = phy_media_set(unit, port, media)) != RT_ERR_OK)
            return ret;
    }
    else
        media = restore_media;

    phy_autoNegoEnable_get(unit, port, &enable);

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        phyData4 = phyData4 & ~(_1000BaseX_Pause_R4_MASK | _1000BaseX_AsymmetricPause_R4_MASK);
        phyData4 = phyData4
                | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET)
                | (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET);
        phyData4 = phyData4 & ~(_1000BaseX_HalfDuplex_R4_MASK | _1000BaseX_FullDuplex_R4_MASK);
        phyData4 = phyData4 | (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET)
                | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
            return ret;

    }
    else if (PORT_MEDIA_COPPER == media)
    {
        phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
        phyData4 = phyData4
                | (pAbility->FC << Pause_R4_OFFSET)
                | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
            return ret;

        phyData4 = phyData4 &
                ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
        phyData4 = phyData4
                | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
                | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
                | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
                | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

        phyData9 = phyData9 & ~(_1000Base_TFullDuplex_MASK | _1000Base_THalfDuplex_MASK);
        phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                   | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
            return ret;


        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
            return ret;
    }

    /* Force re-autonegotiation if AN is on*/
    if (ENABLED == enable)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
    }

    if (restore_media == PORT_MEDIA_FIBER_AUTO || restore_media == PORT_MEDIA_COPPER_AUTO)
    {
        if ((ret = phy_media_set(unit, port, restore_media)) != RT_ERR_OK)
            return ret;
    }

    return ret;
} /* end of phy_8218fb_autoNegoAbility_set */

/* Function Name:
 *      phy_8218b_auto_1000f_get
 * Description:
 *      Get PHY 8218B/8218FB/8214FC copper 1000f ability from shadow.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pAbility - pointer to copper 1000f ability
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. copper 1000f ability value is as following:
 *      - 0: not the ability
 *      - 1: have the ability
 */
int32
phy_8218b_auto_1000f_get(uint32 unit, rtk_port_t port, uint32 *pAbility)
{
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pPhy_info[unit]), RT_ERR_NULL_POINTER);

    *pAbility = (*pPhy_info[unit]).auto_1000f[port];
    return RT_ERR_OK;
} /* end of phy_8218b_auto_1000f_get */

/* Function Name:
 *      phy_8218b_auto_1000f_set
 * Description:
 *      Set PHY 8218B/8218FB/8214FC copper 1000f ability to shadow.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      ability  - copper 1000f ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. copper 1000f ability value is as following:
 *      - 0: not the ability
 *      - 1: have the ability
 */
int32
phy_8218b_auto_1000f_set(uint32 unit, rtk_port_t port, uint32 ability)
{
    RT_PARAM_CHK((ability != 0) && (ability != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pPhy_info[unit]), RT_ERR_NULL_POINTER);

    (*pPhy_info[unit]).auto_1000f[port] = ability;
    return RT_ERR_OK;
} /* end of phy_8218b_auto_1000f_set */

/* Function Name:
 *      phy_8218b_eeepEnable_get
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
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_8218b_eeepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  phyData;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{
        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, 0xa43, 17, &phyData)) != RT_ERR_OK)
            return ret;

        if (((phyData >> 9) & 0x1) == 0x1)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218b_eeepEnable_get */

/* Function Name:
 *      phy_8218_eeepEnable_set
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
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_eeepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t     an_enable;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if((PHY_MEDIA_LINKUP == link_sts) && (PORT_MEDIA_FIBER == media))
    {
        /* register is in copper page and can't access when fiber is linkup. */
        return RT_ERR_PHY_FIBER_LINKUP;
    }else{

        /* Configure the GPHY page to copper */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0001)) != RT_ERR_OK)
            return ret;

        phy_common_autoNegoEnable_get(unit, port, &an_enable);

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, 0xa43, 17, &phyData)) != RT_ERR_OK)
                return ret;

        if (ENABLED == enable)
        {
            phyData |= (1 << 9);  /* enable eeep rx */
            phyData |= (1 << 10); /* enable eeep slave rx */
            if ((ret = hal_miim_write(unit, port, 0xa43, 17, phyData)) != RT_ERR_OK)
                return ret;
        }
        else
        {
            phyData &= ~(1 << 9);  /* disable eeep rx */
            phyData &= ~(1 << 10); /* disable eeep slave rx */
            if ((ret = hal_miim_write(unit, port, 0xa43, 17, phyData)) != RT_ERR_OK)
                return ret;
        }

        /* Force re-autonegotiation if AN is on*/
        if (ENABLED == an_enable)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
                return ret;

            phyData = phyData & ~(RestartAutoNegotiation_MASK);
            phyData = phyData | (an_enable << RestartAutoNegotiation_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
                return ret;
        }

        /* Configure the GPHY page to auto */
        if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
            return ret;
    }
    return RT_ERR_OK;
} /* end of phy_8218b_eeepEnable_set */

/* Function Name:
 *      phy_8218fb_fiber_media_get
 * Description:
 *      Get PHY 8218FB fiber media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy fiber media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
phy_8218fb_fiber_media_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
{
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((port % PORT_NUM_IN_8218FB) < 4, RT_ERR_CHIP_NOT_SUPPORTED);

    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0003)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 0x14, &val)) != RT_ERR_OK)
        goto ERR;

    switch (((val >> 12) & 0xF))
    {
        case 0:
            *pMedia = PORT_FIBER_MEDIA_AUTO;
            break;
        case 9:
            *pMedia = PORT_FIBER_MEDIA_1000;
            break;
        case 11:
            *pMedia = PORT_FIBER_MEDIA_100;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
            goto ERR;
    }

ERR:
    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8218fb_fiber_media_get */

/* Function Name:
 *      phy_8218fb_fiber_media_set
 * Description:
 *      Set PHY 8218FB fiber media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media fiber type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_AUTO
 *      - PORT_FIBER_MEDIA_100
 */
int32
phy_8218fb_fiber_media_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    uint32  val, config;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((port % PORT_NUM_IN_8218FB) < 4, RT_ERR_CHIP_NOT_SUPPORTED);

    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0003)) != RT_ERR_OK)
        return ret;

    switch (media)
    {
        case PORT_FIBER_MEDIA_AUTO:
            config = 0;
            break;

        case PORT_FIBER_MEDIA_1000:
            config = 9;
            break;

        case PORT_FIBER_MEDIA_100:
            config = 11;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
            goto ERR;
    }

    if ((ret = hal_miim_read(unit, port, 8, 0x14, &val)) != RT_ERR_OK)
        goto ERR;

    val &= ~(0xF << 12);
    val |= (config << 12);
    if ((ret = hal_miim_write(unit, port, 8, 0x14, val)) != RT_ERR_OK)
        goto ERR;

ERR:
    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8218fb_fiber_media_set */

/* Function Name:
 *      phy_8214fc_fiber_media_get
 * Description:
 *      Get PHY 8214FC fiber media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy fiber media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_AUTO
 *      - PORT_FIBER_MEDIA_100
 */
int32
phy_8214fc_fiber_media_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
{
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0003)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 0x14, &val)) != RT_ERR_OK)
        goto ERR;

    switch (((val >> 12) & 0xF))
    {
        case 0:
            *pMedia = PORT_FIBER_MEDIA_AUTO;
            break;
        case 9:
            *pMedia = PORT_FIBER_MEDIA_1000;
            break;
        case 11:
            *pMedia = PORT_FIBER_MEDIA_100;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
            goto ERR;
    }

ERR:
    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fc_fiber_media_get */

/* Function Name:
 *      phy_8214fc_fiber_media_set
 * Description:
 *      Get PHY 8214FC fiber media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media fiber type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
phy_8214fc_fiber_media_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    uint32  val, config;
    int32   ret = RT_ERR_FAILED;

    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0003)) != RT_ERR_OK)
        return ret;

    switch (media)
    {
        case PORT_FIBER_MEDIA_AUTO:
            config = 0;
            break;

        case PORT_FIBER_MEDIA_1000:
            config = 9;
            break;

        case PORT_FIBER_MEDIA_100:
            config = 11;
            break;
        default:
            ret = RT_ERR_CHIP_NOT_SUPPORTED;
            goto ERR;
    }

        if ((ret = hal_miim_read(unit, port, 8, 0x14, &val)) != RT_ERR_OK)
            goto ERR;

        val &= ~(0xF << 12);
        val |= (config << 12);
        if ((ret = hal_miim_write(unit, port, 8, 0x14, val)) != RT_ERR_OK)
        goto ERR;

ERR:
    if ((ret = hal_miim_write(unit, port, 0xa42, 29, 0x0000)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fc_fiber_media_set */


/* Function Name:
 *      _phy_8214fc_intMedia_get
 * Description:
 *      Get 8214fc internal media
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      media         - internal media
 *      link_status  - media link status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
_phy_8214fc_intMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *media, uint32 *link_status)
{
    uint32              phyData1;
    int32               ret;
    uint8               is_fiber_linkup = 0, is_copper_linkup = 0;

    /* get media */
    if ((ret = phy_media_get(unit, port, media)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData1)) != RT_ERR_OK)
        goto ERR;
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData1)) != RT_ERR_OK)
        goto ERR;

    if (phyData1 & LinkStatus_MASK)
    {
        *link_status = PHY_MEDIA_LINKUP;
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_EXTENDED_STATUS_REG, &phyData1)) != RT_ERR_OK)
            goto ERR;
        if((phyData1 & _1000BaseX_FullDuplex_R15_MASK) || (phyData1 & _1000BaseX_HalfDuplex_R15_MASK))
        {
            is_fiber_linkup = PHY_MEDIA_LINKUP; /*Link up media is Fiber*/
        }
        if((phyData1 & _1000Base_TFullDuplex_R15_MASK) || (phyData1 & _1000Base_THalfDuplex_R15_MASK))
        {
            is_copper_linkup = PHY_MEDIA_LINKUP; /*Link up media is Copper*/
        }

    }else{
        is_fiber_linkup = PHY_MEDIA_LINKDOWN;
        is_copper_linkup = PHY_MEDIA_LINKDOWN;
        *link_status = PHY_MEDIA_LINKDOWN;
    }

    if (PORT_MEDIA_FIBER == *media || PORT_MEDIA_COPPER == *media)
        return RT_ERR_OK;

    /* nego media */
    if (is_copper_linkup == is_fiber_linkup)
    {
        if (PORT_MEDIA_FIBER_AUTO == *media)
            *media = PORT_MEDIA_FIBER;
        else if (PORT_MEDIA_COPPER_AUTO == *media)
            *media = PORT_MEDIA_COPPER;
    }
    else if (is_fiber_linkup)
        *media = PORT_MEDIA_FIBER;
    else
        *media = PORT_MEDIA_COPPER;

    return RT_ERR_OK;
ERR:
    return ret;
}   /* end of _phy_8214fc_intMedia_get */

/* Function Name:
 *      phy_8214fc_autoNegoEnable_get
 * Description:
 *      Get 8214FC autonegotiation enable status of the specific port
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
phy_8214fc_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtk_port_media_t    media;
    uint32              link_sts;
    uint32              phyData0, page, reg;
    int32               ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
    }
    else
    {
        page = 0xa40;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
    }

    if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
        goto ERR;

    if (phyData0 & AutoNegotiationEnable_MASK)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_autoNegoEnable_get */

/* Function Name:
 *      phy_8214fc_autoNegoEnable_set
 * Description:
 *      Set 8214FC autonegotiation enable status of the specific port
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
phy_8214fc_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtk_port_media_t    media;
    uint32              link_sts;
    uint32              phyData0, page, reg;
    int32               ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
    }
    else
    {
        page = 0xa40;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
    }

    if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
        goto ERR;

    phyData0 = phyData0 & ~(AutoNegotiationEnable_MASK | RestartAutoNegotiation_MASK);
    phyData0 = phyData0 | ((enable << AutoNegotiationEnable_OFFSET) | (1 << RestartAutoNegotiation_OFFSET));

    if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
        goto ERR;

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_autoNegoEnable_set */

/* Function Name:
 *      phy_8214fc_autoNegoAbility_get
 * Description:
 *      Get 8214FC ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_autoNegoAbility_get(uint32 unit, rtk_port_t port,
    rtk_port_phy_ability_t *pAbility)
{
    rtk_port_media_t    media;
    uint32              link_sts;
    uint32              phyData4, phyData9;
    uint32              page, reg;
    int32               ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        reg = PHY_INT_REG_BASE + PHY_AN_ADVERTISEMENT_REG;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData4)) != RT_ERR_OK)
            goto ERR;

        /* 1000Base-X Linkup */
        pAbility->FC = (phyData4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
        pAbility->AsyFC = (phyData4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
        pAbility->Half_10 = 0;
        pAbility->Full_10 = 0;
        pAbility->Half_100 = 0;
        pAbility->Full_100 = 0;
        pAbility->Half_1000 = (phyData4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
        pAbility->Full_1000 = (phyData4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;
    }
    else
    {
        page = 0xa40;
        reg = PHY_INT_REG_BASE + PHY_AN_ADVERTISEMENT_REG;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData4)) != RT_ERR_OK)
            goto ERR;

        page = 0xa41;
        reg = PHY_INT_REG_BASE + (PHY_1000_BASET_CONTROL_REG % PHY_INT_REG_NEXT);
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData9)) != RT_ERR_OK)
            goto ERR;

        pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
        pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;
        pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
        pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
        pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
        pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
        pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
        pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;
    }

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_autoNegoAbility_get */

/* Function Name:
 *      phy_8214fc_autoNegoAbility_set
 * Description:
 *      Set 8214FC ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_autoNegoAbility_set(uint32 unit, rtk_port_t port,
    rtk_port_phy_ability_t *pAbility)
{
    rtk_port_media_t    media;
    rtk_enable_t        enable;
    uint32              link_sts;
    uint32              phyData0, phyData4, phyData9;
    int32               ret, page, reg;

    if ((ret = phy_autoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
        return ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        reg = PHY_INT_REG_BASE + PHY_AN_ADVERTISEMENT_REG;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData4)) != RT_ERR_OK)
            goto ERR;

        phyData4 = phyData4 & ~(_1000BaseX_Pause_R4_MASK | _1000BaseX_AsymmetricPause_R4_MASK);
        phyData4 = phyData4 | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET)
                | (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET);
        phyData4 = phyData4 & ~(_1000BaseX_HalfDuplex_R4_MASK | _1000BaseX_FullDuplex_R4_MASK);
        phyData4 = phyData4 | (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET)
                | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);

        if ((ret = hal_miim_write(unit, port, page, reg, phyData4)) != RT_ERR_OK)
            goto ERR;

        /* Force re-autonegotiation if AN is on */
        if (ENABLED == enable)
        {
            reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

            if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
                goto ERR;

            phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
            phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

            if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
                goto ERR;
        }
    }
    else
    {
        page = 0xa40;
        /* register 4 */
        reg = PHY_INT_REG_BASE + PHY_AN_ADVERTISEMENT_REG;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData4)) != RT_ERR_OK)
            goto ERR;

        phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
        phyData4 = phyData4 | (pAbility->FC << Pause_R4_OFFSET)
                | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

        phyData4 = phyData4 &
                ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
        phyData4 = phyData4
                | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
                | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
                | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
                | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

        if ((ret = hal_miim_write(unit, port, page, reg, phyData4)) != RT_ERR_OK)
            goto ERR;

        /* register 9 */
        page = 0xa41;
        reg = PHY_INT_REG_BASE + (PHY_1000_BASET_CONTROL_REG % PHY_INT_REG_NEXT);
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData9)) != RT_ERR_OK)
            goto ERR;

        phyData9 = phyData9 & ~(_1000Base_TFullDuplex_MASK | _1000Base_THalfDuplex_MASK);
        phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                   | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

        if ((ret = hal_miim_write(unit, port, page, reg, phyData9)) != RT_ERR_OK)
            goto ERR;

        /* Force re-autonegotiation if AN is on */
        if (ENABLED == enable)
        {
            page = 0xa40;
            reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

            if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
                goto ERR;

            phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
            phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

            if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
                goto ERR;
        }
    }

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_autoNegoAbility_set */

/* Function Name:
 *      phy_8214fc_duplex_get
 * Description:
 *      Get 8214FC duplex mode status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDuplex - pointer to PHY duplex mode status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex)
{
    rtk_port_media_t    media;
    uint32              link_sts;
    uint32              phyData0;
    uint32              page, reg;
    int32               ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
    }
    else
    {
        page = 0xa40;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
    }

    if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
        goto ERR;

    *pDuplex = (phyData0 & DuplexMode_MASK) >> DuplexMode_OFFSET;

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_duplex_get */

/* Function Name:
 *      phy_8214fc_duplex_set
 * Description:
 *      Set 8214FC duplex mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      duplex        - duplex mode of the port, full or half
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex)
{
    rtk_port_media_t    media;
    uint32              link_sts;
    uint32              phyData0;
    uint32              page, reg;
    int32               ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
    }
    else
    {
        page = 0xa40;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
    }

    if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
        goto ERR;

    phyData0 = phyData0 & ~(DuplexMode_MASK);
    phyData0 = phyData0 | (duplex << DuplexMode_OFFSET);

    if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
        goto ERR;

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_duplex_set */

/* Function Name:
 *      phy_8214fc_speed_get
 * Description:
 *      Get 8214FC MP link speed status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    rtk_port_media_t    media;
    uint32              link_sts;
    uint32              phyData0;
    uint32              page, reg;
    int32               ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
    }
    else
    {
        page = 0xa40;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
    }

    if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
        goto ERR;

    *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
            | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_speed_get */

/* Function Name:
 *      phy_8214fc_MP_speed_set
 * Description:
 *      Set 8214FC MP speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
int32
phy_8214fc_MP_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    rtk_port_media_t    media;
    uint32              link_sts;
    uint32              phyData0;
    uint32              page, reg;
    int32               ret;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

    if (PORT_MEDIA_FIBER == media)
    {
        page = PHY_PAGE_0;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
    }
    else
    {
        page = 0xa40;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
    }

    if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
        goto ERR;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1))
            | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
        goto ERR;

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_MP_speed_set */

/* Function Name:
 *      phy_8214fc_enable_set
 * Description:
 *      Set PHY interface status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtk_port_media_t    media;
    uint32              phyData0;
    uint32              page, reg;
    int32               ret;

    if ((ret = phy_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + PHY_CONTROL_REG;

    /* for fiber force and auto */
    if (PORT_MEDIA_COPPER != media)
    {
        page = PHY_PAGE_0;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 &= ~(PowerDown_MASK);
        if (DISABLED == enable)
            phyData0 |= (1 << PowerDown_OFFSET);

        if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
            goto ERR;
    }

    /* for copper force and auto */
    if (PORT_MEDIA_FIBER != media)
    {
        page = 0xa40;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 &= ~(PowerDown_MASK);
        if (DISABLED == enable)
            phyData0 |= (1 << PowerDown_OFFSET);

        if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
            goto ERR;
    }

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_enable_set */

/* Function Name:
 *      phy_8218fb_autoNegoEnable_get
 * Description:
 *      Get 8218FB autonegotiation enable status of the specific port
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
phy_8218fb_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_autoNegoEnable_get(unit, port, pEnable);
    else
        return phy_common_autoNegoEnable_get(unit, port, pEnable);

    return RT_ERR_OK;
} /* end of phy_8218fb_autoNegoEnable_get */

/* Function Name:
 *      phy_8218fb_autoNegoEnable_set
 * Description:
 *      Set 8218FB autonegotiation enable status of the specific port
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
phy_8218fb_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_autoNegoEnable_set(unit, port, enable);
    else
        return phy_common_autoNegoEnable_set(unit, port, enable);

    return RT_ERR_OK;
} /* end of phy_8218fb_autoNegoEnable_set */

/* Function Name:
 *      phy_8218fb_MP_autoNegoAbility_get
 * Description:
 *      Get 8218FB ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_MP_autoNegoAbility_get(uint32 unit, rtk_port_t port,
    rtk_port_phy_ability_t *pAbility)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_autoNegoAbility_get(unit, port, pAbility);
    else
        return phy_8218b_autoNegoAbility_get(unit, port, pAbility);

    return RT_ERR_OK;
} /* end of phy_8218fb_MP_autoNegoAbility_get */

/* Function Name:
 *      phy_8218fb_MP_autoNegoAbility_set
 * Description:
 *      Set 8218FB ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_MP_autoNegoAbility_set(uint32 unit, rtk_port_t port,
    rtk_port_phy_ability_t *pAbility)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_autoNegoAbility_set(unit, port, pAbility);
    else
        return phy_8218b_autoNegoAbility_set(unit, port, pAbility);

    return RT_ERR_OK;
} /* end of phy_8218fb_MP_autoNegoAbility_set */

/* Function Name:
 *      phy_8218fb_duplex_get
 * Description:
 *      Get 8218FB duplex mode status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDuplex - pointer to PHY duplex mode status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_duplex_get(unit, port, pDuplex);
    else
        return phy_common_duplex_get(unit, port, pDuplex);

    return RT_ERR_OK;
} /* end of phy_8218fb_duplex_get */

/* Function Name:
 *      phy_8218fb_duplex_set
 * Description:
 *      Set 8218FB duplex mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      duplex        - duplex mode of the port, full or half
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_duplex_set(unit, port, duplex);
    else
        return phy_common_duplex_set(unit, port, duplex);

    return RT_ERR_OK;
} /* end of phy_8218fb_duplex_set */

/* Function Name:
 *      phy_8218fb_speed_get
 * Description:
 *      Get 8218FB MP link speed status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_speed_get(unit, port, pSpeed);
    else
        return phy_8218b_speed_get(unit, port, pSpeed);

    return RT_ERR_OK;
} /* end of phy_8218fb_speed_get */

/* Function Name:
 *      phy_8218fb_MP_speed_set
 * Description:
 *      Set 8218FB MP speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_MP_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_MP_speed_set(unit, port, speed);
    else
        return phy_8218b_speed_set(unit, port, speed);

    return RT_ERR_OK;
} /* end of phy_8218fb_MP_speed_set */

/* Function Name:
 *      phy_8218fb_enable_set
 * Description:
 *      Set PHY interface status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_enable_set(unit, port, enable);
    else
        return phy_8218b_enable_set(unit, port, enable);

    return RT_ERR_OK;
} /* end of phy_8218fb_enable_set */

/* Function Name:
 *      phy_int8380_rtctResult_get
 * Description:
 *      Get test result of RTCT.
 * Input:
 *      unit        - unit id
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      The result unit is cm
 */
int32
phy_int8380_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData, fixed_page;
    hal_control_t   *pHalCtrl;
    uint32 speed;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    /* Check the port is link up or not? */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData)) != RT_ERR_OK)
        return ret;

    phy_common_speed_get(unit, port, &speed);

    if ((phyData & LinkStatus_MASK) && speed != 0)
    {
        /* If the port is link up,
         * return cable length from green function
         */

        fixed_page = 0xa88;

        /* The Length is store in [7:0], and the unit is meter*/
        if ((ret = hal_miim_read(unit, port, fixed_page, 16, &phyData)) != RT_ERR_OK)
            return ret;

        if (HAL_IS_GE_PORT(unit, port))
        {
            pRtctResult->linkType = PORT_SPEED_1000M;
            pRtctResult->ge_result.channelALen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelBLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelCLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelDLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelAShort = 0;
            pRtctResult->ge_result.channelBShort = 0;
            pRtctResult->ge_result.channelCShort = 0;
            pRtctResult->ge_result.channelDShort = 0;
            pRtctResult->ge_result.channelAOpen = 0;
            pRtctResult->ge_result.channelBOpen = 0;
            pRtctResult->ge_result.channelCOpen = 0;
            pRtctResult->ge_result.channelDOpen = 0;
            pRtctResult->ge_result.channelAMismatch = 0;
            pRtctResult->ge_result.channelBMismatch = 0;
            pRtctResult->ge_result.channelCMismatch = 0;
            pRtctResult->ge_result.channelDMismatch = 0;
            pRtctResult->ge_result.channelALinedriver = 0;
            pRtctResult->ge_result.channelBLinedriver = 0;
            pRtctResult->ge_result.channelCLinedriver = 0;
            pRtctResult->ge_result.channelDLinedriver = 0;
        }
        else /* if (HAL_IS_FE_PORT(unit, port)) */
        {
            pRtctResult->linkType = PORT_SPEED_100M;
            pRtctResult->fe_result.isRxShort = 0;
            pRtctResult->fe_result.isTxShort = 0;
            pRtctResult->fe_result.isRxOpen = 0;
            pRtctResult->fe_result.isTxOpen = 0;
            pRtctResult->fe_result.isRxMismatch = 0;
            pRtctResult->fe_result.isTxMismatch = 0;
            pRtctResult->fe_result.isRxLinedriver = 0;
            pRtctResult->fe_result.isTxLinedriver = 0;
            pRtctResult->fe_result.rxLen = (phyData & 0x00FF)*100;
            pRtctResult->fe_result.txLen = (phyData & 0x00FF)*100;
        }
    }
    else
    {
        /* If the port is link down,
         * return cable length from RTCT function
         */
        /* Page 0xa42, Register 17
         * bit[15]: cable test finished or not?
         *             1: Finished
         *             0: Not finished
         */
         fixed_page = 0xa42;

         if ((ret = hal_miim_read(unit, port, fixed_page, 17, &phyData)) != RT_ERR_OK)
            return ret;

         if(((phyData >> RTCT_DONE) & 0x1) != 0x1)
            return RT_ERR_PHY_RTCT_NOT_FINISH;

        if (HAL_IS_GE_PORT(unit, port))
        {
            pRtctResult->linkType = PORT_SPEED_1000M;
            /* Length = (Index/64)*8ns*(0.2m/ns) = Index/80 (m) = (1.25) * Index (cm) */

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_A)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelALen = (phyData)*5/4;

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_B)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelBLen = (phyData)*5/4;

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_C)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelCLen = (phyData)*5/4;

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_D)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->ge_result.channelDLen = (phyData)*5/4;

            /* === Channel A Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_A)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelAMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelALinedriver = 1;

            /* === Channel B Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_B)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelBLinedriver = 1;

            /* === Channel C Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_C)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelCLinedriver = 1;

            /* === Channel D Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_D)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->ge_result.channelDLinedriver = 1;
        }
        else /* if (HAL_IS_FE_PORT(unit, port)) */
        {
            pRtctResult->linkType = PORT_SPEED_100M;
            /* Length = (Index/64)*8ns*(0.2m/ns) = Index/80 (m) = (1.25) * Index (cm) */

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_A)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->fe_result.rxLen = (phyData)*5/4;

            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_LEN_ADDR_B)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;
            phyData &= 0x3FFF; /*[13:0] are valid*/
            pRtctResult->fe_result.txLen = (phyData)*5/4;

            /* === Channel A Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_A)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->fe_result.isRxShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->fe_result.isRxOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->fe_result.isRxMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->fe_result.isRxMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->fe_result.isRxLinedriver = 1;

            /* === Channel B Status ===*/
            if ((ret = hal_miim_write(unit, port, fixed_page, 27, RTCT_STATUS_ADDR_B)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, fixed_page, 28, &phyData)) != RT_ERR_OK)
                return ret;

            if(((phyData >> RTCT_STATUS_SHORT) & 0x1) != 0x0)
                pRtctResult->fe_result.isTxShort = 1;
            if(((phyData >> RTCT_STATUS_OPEN) & 0x1) != 0x0)
                pRtctResult->fe_result.isTxOpen = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_SHORT) & 0x1) != 0x0)
                pRtctResult->fe_result.isTxMismatch = 1;
            if(((phyData >> RTCT_STATUS_MISSMATCH_OPEN) & 0x1) != 0x0)
                pRtctResult->fe_result.isTxMismatch |= 0x2;
            if(((phyData >> RTCT_STATUS_LINE_DRIVER) & 0x1) != 0x0)
                pRtctResult->fe_result.isTxLinedriver = 1;
        }
    }

    return ret;
} /* end of phy_int8380_rtctResult_get */


/* Function Name:
 *      phy_8218_patch_set
 * Description:
 *      Set patch to PHY.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_patch_set(uint32 unit, rtk_port_t port)
{
    int32 ret;
    ret = sub_phy_8218b_patch_set(unit, port);
    return ret;
} /* end of phy_8218b_patch_set */

/* Function Name:
 *      phy_8214fc_mp_patch_set
 * Description:
 *      Set patch to PHY.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8214fc_mp_patch_set(uint32 unit, rtk_port_t port)
{
    int32 ret;
    ret = sub_phy_8214fc_mp_patch_set(unit, port);
    return ret;
} /* end of phy_8214fc_mp_patch_set */

/* Function Name:
 *      phy_8218fb_mp_patch_set
 * Description:
 *      Set patch to PHY.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218fb_mp_patch_set(uint32 unit, rtk_port_t port)
{
    int32 ret;
    ret = sub_phy_8218fb_mp_patch_set(unit, port);
    return ret;
} /* end of phy_8218fb_mp_patch_set */

/* Function Name:
 *      phy_8214fc_fiberDownSpeedEnable_get
 * Description:
 *      Get fiber down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of fiber down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8214fc_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    uint32  phyData;
    int32   ret;

    if ((ret = hal_miim_write(unit, port, 0, 29, 3)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 17, &phyData)) != RT_ERR_OK)
        goto ERR;

    if (phyData & (1 << 5))
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    if ((ret = hal_miim_write(unit, port, 0, 29, 0)) != RT_ERR_OK)
        return ret;
    return ret;
ERR:
    hal_miim_write(unit, port, 0, 29, 0);
    return ret;
}

/* Function Name:
 *      phy_8214fc_fiberDownSpeedEnable_set
 * Description:
 *      Set fiber down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of fiber down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8214fc_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret;

    if ((ret = hal_miim_write(unit, port, 0, 29, 3)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 17, &phyData)) != RT_ERR_OK)
        goto ERR;

    if (ENABLED == enable)
        phyData |= (1 << 5);
    else
        phyData &= ~(1 << 5);

    if ((ret = hal_miim_write(unit, port, 8, 17, phyData)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_write(unit, port, 0, 29, 0)) != RT_ERR_OK)
        return ret;

    return ret;
ERR:
    hal_miim_write(unit, port, 0, 29, 0);
    return ret;
}

/* Function Name:
 *      phy_8218fb_fiberDownSpeedEnable_get
 * Description:
 *      Get fiber down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of fiber down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8218fb_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((port % PORT_NUM_IN_8218FB) < 4, RT_ERR_CHIP_NOT_SUPPORTED);

    return phy_8214fc_fiberDownSpeedEnable_get(unit, port, pEnable);
}

/* Function Name:
 *      phy_8218fb_fiberDownSpeedEnable_set
 * Description:
 *      Set fiber down speed status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of fiber down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8218fb_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    RT_PARAM_CHK((port % PORT_NUM_IN_8218FB) < 4, RT_ERR_CHIP_NOT_SUPPORTED);

    return phy_8214fc_fiberDownSpeedEnable_set(unit, port, enable);
}

/* Function Name:
 *      phy_8218b_utpDownSpeedEnable_get
 * Description:
 *      Get UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of UTP down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8218b_utpDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    uint32  phyData;
    int32   ret;

    if ((ret = hal_miim_write(unit, port, 0, 30, 1)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 0xa44, 17, &phyData)) != RT_ERR_OK)
        goto ERR;

    if (phyData & (1 << 3))
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    if ((ret = hal_miim_write(unit, port, 0, 30, 0)) != RT_ERR_OK)
        return ret;
    return ret;
ERR:
    hal_miim_write(unit, port, 0, 30, 0);
    return ret;
}

/* Function Name:
 *      phy_8218b_utpDownSpeedEnable_set
 * Description:
 *      Set UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of UTP down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8218b_utpDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret;

    if ((ret = hal_miim_write(unit, port, 0, 30, 1)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 0xa44, 17, &phyData)) != RT_ERR_OK)
        goto ERR;

    /* DownSpeed to 100M*/
    phyData &= ~(1 << 5);

    if (ENABLED == enable)
        phyData |= (1 << 3);
    else
        phyData &= ~(1 << 3);

    if ((ret = hal_miim_write(unit, port, 0xa44, 17, phyData)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_write(unit, port, 0, 30, 0)) != RT_ERR_OK)
        return ret;

    return ret;
ERR:
    hal_miim_write(unit, port, 0, 30, 0);
    return ret;
}

/* Function Name:
 *      phy_8218b_downSpeedEnable_get
 * Description:
 *      Get UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8218b_downSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    int32   ret;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if(PORT_MEDIA_FIBER == media)
    {
        ret = phy_8214fc_fiberDownSpeedEnable_get(unit, port, pEnable);
    }else{
        ret = phy_8218b_utpDownSpeedEnable_get(unit, port, pEnable);
    }
    return ret;
}

/* Function Name:
 *      phy_8218b_downSpeedEnable_set
 * Description:
 *      Set UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8218b_downSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    int32   ret;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fc_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if(PORT_MEDIA_FIBER == media)
    {
        ret = phy_8214fc_fiberDownSpeedEnable_set(unit, port, enable);
    }else{
        ret = phy_8218b_utpDownSpeedEnable_set(unit, port, enable);
    }
    return ret;
}

/* Function Name:
 *      phy_8214fc_fiberNwayForceLink_get
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pEnable - fiber Nway force links status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_fiberNwayForceLink_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    uint32  phyData;
    int32   ret;

    if ((ret = hal_miim_write(unit, port, 0, 29, 3)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 20, &phyData)) != RT_ERR_OK)
        goto ERR;

    if ((phyData >> 2) & 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    if ((ret = hal_miim_write(unit, port, 0, 29, 0)) != RT_ERR_OK)
        return ret;

    return ret;
ERR:
    hal_miim_write(unit, port, 0, 29, 0);
    return ret;
}   /* end of phy_8214fc_fiberNwayForceLink_get */

/* Function Name:
 *      phy_8214fc_fiberNwayForceLink_set
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      enable - fiber Nway force links status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_fiberNwayForceLink_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret;

    if ((ret = hal_miim_write(unit, port, 0, 29, 3)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 20, &phyData)) != RT_ERR_OK)
        goto ERR;

    if (ENABLED == enable)
        phyData |= (1 << 2);
    else
        phyData &= ~(1 << 2);

    if ((ret = hal_miim_write(unit, port, 8, 20, phyData)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_write(unit, port, 0, 29, 0)) != RT_ERR_OK)
        return ret;

    return ret;
ERR:
    hal_miim_write(unit, port, 0, 29, 0);
    return ret;
}   /* end of phy_8214fc_fiberNwayForceLink_set */

/* Function Name:
 *      phy_8218fb_fiberNwayForceLink_get
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pEnable - fiber Nway force links status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_fiberNwayForceLink_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((port % PORT_NUM_IN_8218FB) < 4, RT_ERR_CHIP_NOT_SUPPORTED);

    return phy_8214fc_fiberNwayForceLink_get(unit, port, pEnable);
}   /* end of phy_8218fb_fiberNwayForceLink_get */

/* Function Name:
 *      phy_8218fb_fiberNwayForceLink_set
 * Description:
 *      When fiber port is configured N-way,
 *      which can link with link partner is configured force mode.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      enable - fiber Nway force links status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8218fb_fiberNwayForceLink_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    RT_PARAM_CHK((port % PORT_NUM_IN_8218FB) < 4, RT_ERR_CHIP_NOT_SUPPORTED);

    return phy_8214fc_fiberNwayForceLink_set(unit, port, enable);
}   /* end of phy_8218fb_fiberNwayForceLink_set */


/* Function Name:
 *      phy_8214fc_fiberOAMLoopBack_set
 * Description:
 *      Set Fiber-Port OAM Loopback feature,
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      enable - Fiber-Port OAM Loopback feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_fiberOAMLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  phyData;
    uint32  reg_val;
    int32    ret;

    /*Enable Loopback*/
    /*Backup*/
    if ((ret = hal_miim_read(unit, port, 0, 30, &phyData)) != RT_ERR_OK)
        return ret;

    reg_val = 0x03;
    if ((ret = hal_miim_write(unit, port, 0, 30, reg_val)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 16, &reg_val)) != RT_ERR_OK)
        return ret;

    if(enable == ENABLED)
        reg_val |= (1UL<<4);
    else
        reg_val &= ~(1UL<<4);

    if ((ret = hal_miim_write(unit, port, 8, 16, reg_val)) != RT_ERR_OK)
        return ret;

    /*Restore*/
    if ((ret = hal_miim_write(unit, port, 0, 30, phyData)) != RT_ERR_OK)
        return ret;

    if(enable == ENABLED)
    {
        /*Delay until PHY Linkup*/
        osal_time_usleep(100 * 1000); /* delay 10mS */
    }

    return ret;
}   /* end of phy_8214fc_fiberOAMLoopBack_set */


/* Function Name:
 *      _phy_8218b_ptpReg_get
 * Description:
 *      Get PTP register data of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      reg_addr            - reg address
 * Output:
 *      pData               - pointer to the PHY reg data
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      None
 */
static int32
_phy_8218b_ptpReg_get(uint32 unit, rtk_port_t port, uint32 reg_addr, uint32 *pData)
{
    int32 ret;
    rtk_port_t port_base;
    uint32 page, reg;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d, reg_addr=0x%x, reg=0x%x",
           unit, port, reg_addr);

    /* parameter check */
    RT_PARAM_CHK((NULL == pData), RT_ERR_NULL_POINTER);

    port_base = port - (port % 8);
    page = (reg_addr - (reg_addr % 8)) / 8;
    reg = ((reg_addr - REG_8218B_PTP_BASE) % 8) + 16;

    if ((ret = hal_miim_write(unit, port_base, 0, 29, 8)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_read(unit, port_base, page, reg, pData)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_write(unit, port_base, 0, 29, 0)) != RT_ERR_OK)
    {
        return ret;
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "pData=0x%x", *pData);

    return RT_ERR_OK;
}

/* Function Name:
 *      _phy_8218b_ptpReg_set
 * Description:
 *      Set PTP register data of the specific port
 * Input:
 *      unit               - unit id
 *      port               - port id
 *      reg_addr           - reg address
 *      reg_val            - reg value
 *      reg_msk            - reg value mask
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
int32
_phy_8218b_ptpReg_set(uint32 unit, uint32 port, uint32 reg_addr, uint32 reg_val, uint32 reg_msk)
{
    int32 ret;
    rtk_port_t port_base;
    uint32 page, reg, ori_val;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TIME), "unit=%d, port=%d, reg_addr=0x%x, reg_val=0x%x, reg_msk=0x%x",
            unit, port, reg_addr, reg_val, reg_msk);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);

    port_base = port - (port % 8);
    page = (reg_addr - (reg_addr % 8)) / 8;
    reg = ((reg_addr - REG_8218B_PTP_BASE) % 8) + 16;

    if ((ret = hal_miim_write(unit, port_base, 0, 29, 8)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((reg_msk & 0xFFFF) != 0xFFFF)
    {
        if ((ret = hal_miim_read(unit, port_base, page, reg, &ori_val)) != RT_ERR_OK)
        {
            return ret;
        }

        reg_val = (ori_val & ~reg_msk) | (reg_val & reg_msk);
    }
    if ((ret = hal_miim_write(unit, port_base, page, reg, reg_val)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = hal_miim_write(unit, port_base, 0, 29, 0)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}


/* Function Name:
 *      phy_8218b_ptpSwitchMacAddr_get
 * Description:
 *      Get the Switch MAC address setting of PHY of the specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      pSwitchMacAddr - point to the Switch MAC Address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_ptpSwitchMacAddr_get(uint32 unit, rtk_port_t port, rtk_mac_t *pSwitchMacAddr)
{
    int32 ret;
    uint32 mac_h, mac_m, mac_l;

    if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_MAC_ADDR_H, &mac_h)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_MAC_ADDR_M, &mac_m)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_MAC_ADDR_L, &mac_l)) != RT_ERR_OK)
    {
        return ret;
    }

    pSwitchMacAddr->octet[0] = (uint8)(mac_h >> 8);
    pSwitchMacAddr->octet[1] = (uint8)(mac_h & 0xFF);
    pSwitchMacAddr->octet[2] = (uint8)(mac_m >> 8);
    pSwitchMacAddr->octet[3] = (uint8)(mac_m & 0xFF);
    pSwitchMacAddr->octet[4] = (uint8)(mac_l >> 8);
    pSwitchMacAddr->octet[5] = (uint8)(mac_l & 0xFF);

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpSwitchMacAddr_set
 * Description:
 *      Set the Switch MAC address setting of PHY of the specified port.
 * Input:
 *      unit           - unit id
 *      port           - port id
 *      pSwitchMacAddr - point to the Switch MAC Address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_ptpSwitchMacAddr_set(uint32 unit, rtk_port_t port, rtk_mac_t *pSwitchMacAddr)
{
    int32 ret;
    uint32 mac_h, mac_m, mac_l;

    mac_h = (pSwitchMacAddr->octet[0] << 8) | (pSwitchMacAddr->octet[1]);
    mac_m = (pSwitchMacAddr->octet[2] << 8) | (pSwitchMacAddr->octet[3]);
    mac_l = (pSwitchMacAddr->octet[4] << 8) | (pSwitchMacAddr->octet[5]);

    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_MAC_ADDR_H, mac_h, 0xFFFF)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_MAC_ADDR_M, mac_m, 0xFFFF)) != RT_ERR_OK)
    {
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_MAC_ADDR_L, mac_l, 0xFFFF)) != RT_ERR_OK)
    {
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpRefTime_get
 * Description:
 *      Get the reference time of PHY of the specified port.
 * Input:
 *      unit       - unit id
 * Output:
 *      pTimeStamp - pointer buffer of the reference time
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
phy_8218b_ptpRefTime_get(uint32 unit, rtk_port_t port,
    rtk_time_timeStamp_t *pTimeStamp)
{
    int32 ret;
    uint32 sec_l, sec_h, nsec_l, nsec_h;
    uint32 reg_val, reg_msk;

    /* execute reading command, [15] = 0x1 executing, [13:12] = 0x0 read */
    reg_val = REG_FIELD_8218B_PTP_CMD_EXEC | REG_FIELD_8218B_PTP_CMD_OP_READ;
    reg_msk = REG_FIELD_8218B_PTP_CMD_EXEC | REG_FIELD_8218B_PTP_CMD_OP_MSK;

    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_NSEC_H, reg_val, reg_msk)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    do {
        if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_TIME_NSEC_H, &reg_val)) != RT_ERR_OK)
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
            return ret;
        }
    } while (reg_val & REG_FIELD_8218B_PTP_CMD_EXEC); /* busy watting */

    if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_TIME_NSEC_L_RO, &nsec_l)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_TIME_NSEC_H_RO, &nsec_h)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_TIME_SEC_L_RO, &sec_l)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_TIME_SEC_H_RO, &sec_h)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpRefTime_set
 * Description:
 *      Set the reference time of PHY of the specified port.
 * Input:
 *      unit      - unit id
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_ptpRefTime_set(uint32 unit, rtk_port_t port, rtk_time_timeStamp_t timeStamp)
{
    int32 ret;
    uint32 sec_l, sec_h, nsec_l, nsec_h;
    uint32 reg_val;

    /* adjust Timer of PHY */
    sec_l = (timeStamp.sec) & 0xFFFF;
    sec_h = (timeStamp.sec) >> 16;
    /* convert nsec to 8nsec */
    nsec_l = (timeStamp.nsec >> 3) & 0xFFFF;
    nsec_h = ((timeStamp.nsec >> 3) >> 16) & 0x07FF;

    /* execute writing command, [15] = 0x1 executing, [13:12] = 0x1 write */
    nsec_h = nsec_h | REG_FIELD_8218B_PTP_CMD_EXEC | REG_FIELD_8218B_PTP_CMD_OP_WRITE;

    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_SEC_L, sec_l, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_SEC_H, sec_h, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_NSEC_L, nsec_l, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_NSEC_H, nsec_h, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    /* busy watting */
    do {
        if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_TIME_NSEC_H, &reg_val)) != RT_ERR_OK)
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
            return ret;
        }
    } while (reg_val & REG_FIELD_8218B_PTP_CMD_EXEC);

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpRefTimeAdjust_set
 * Description:
 *      Adjust the reference time of PHY of the specified port.
 * Input:
 *      unit      - unit id
 *      port    - port id
 *      sign      - significant
 *      timeStamp - reference timestamp value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      sign=0 for positive adjustment, sign=1 for negative adjustment.
 */
int32
phy_8218b_ptpRefTimeAdjust_set(uint32 unit, rtk_port_t port, uint32 sign, rtk_time_timeStamp_t timeStamp)
{
    int32 ret;
    uint32 sec_l, sec_h, nsec_l, nsec_h;
    uint32 reg_val;

    /* adjust Timer of PHY */
    sec_l = (timeStamp.sec) & 0xFFFF;
    sec_h = (timeStamp.sec) >> 16;
    /* convert nsec to 8nsec */
    nsec_l = (timeStamp.nsec >> 3) & 0xFFFF;
    nsec_h = ((timeStamp.nsec >> 3) >> 16) & 0x07FF;

    /* execute adjusting command, [15] = 0x1 executing, [13:12] = 0x2 inc / 0x3 dec */
    if (sign == 0)
    {
        nsec_h |= REG_FIELD_8218B_PTP_CMD_OP_ADJ_INC;
    } else {
        nsec_h |= REG_FIELD_8218B_PTP_CMD_OP_ADJ_DEC;
    }
    nsec_h = nsec_h | REG_FIELD_8218B_PTP_CMD_EXEC;

    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_SEC_L, sec_l, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_SEC_H, sec_h, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_NSEC_L, nsec_l, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_set(unit, port, REG_8218B_PTP_TIME_NSEC_H, nsec_h, 0xFFFF)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    /* busy watting */
    do {
        if ((ret = _phy_8218b_ptpReg_get(unit, port, REG_8218B_PTP_TIME_NSEC_H, &reg_val)) != RT_ERR_OK)
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
            return ret;
        }
    } while (reg_val & REG_FIELD_8218B_PTP_CMD_EXEC);

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpRefTimeEnable_get
 * Description:
 *      Get the enable state of reference time of PHY of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status
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
phy_8218b_ptpRefTimeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;
    uint32 reg_addr, reg_val, reg_msk;

    reg_addr = REG_8218B_PTP_TIME_CFG_0;
    reg_msk = (REG_FIELD_8218B_PTP_CFG_TIMER_EN_FRC | REG_FIELD_8218B_PTP_CFG_TIMER_1588_EN);

    if ((ret = _phy_8218b_ptpReg_get(unit, port, reg_addr, &reg_val)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    if (reg_val & reg_msk)
    {
        *pEnable = ENABLED;
    }
    else
    {
        *pEnable = DISABLED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpRefTimeEnable_set
 * Description:
 *      Set the enable state of reference time of PHY of the specified port.
 * Input:
 *      unit   - unit id
 *      port    - port id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_ptpRefTimeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 reg_addr, reg_val, reg_msk;

    reg_addr = REG_8218B_PTP_TIME_CFG_0;
    reg_val = (enable == ENABLED)? \
        (REG_FIELD_8218B_PTP_CFG_TIMER_EN_FRC | REG_FIELD_8218B_PTP_CFG_TIMER_1588_EN) : 0;
    reg_msk = (REG_FIELD_8218B_PTP_CFG_TIMER_EN_FRC | REG_FIELD_8218B_PTP_CFG_TIMER_1588_EN);

    if ((ret = _phy_8218b_ptpReg_set(unit, port, reg_addr, reg_val, reg_msk)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpEnable_get
 * Description:
 *      Get PTP status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT         - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_8218b_ptpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;
    uint32 reg_addr, reg_val;

    reg_addr = REG_8218B_PTP_TIME_CTRL_PN + REG_8218B_PTP_OFFSET_PORT(port);

    if ((ret = _phy_8218b_ptpReg_get(unit, port, reg_addr, &reg_val)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    if (reg_val & REG_FIELD_8218B_PTP_PHY_EN)
    {
        *pEnable = ENABLED;
    } else {
        *pEnable = DISABLED;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpEnable_set
 * Description:
 *      Set PTP status of the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT     - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218b_ptpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 reg_addr, reg_val;

    reg_addr = REG_8218B_PTP_TIME_CTRL_PN + REG_8218B_PTP_OFFSET_PORT(port);
    reg_val = (enable == ENABLED)? REG_FIELD_8218B_PTP_PHY_EN : 0;

    if ((ret = _phy_8218b_ptpReg_set(unit, port, reg_addr, reg_val, REG_FIELD_8218B_PTP_PHY_EN)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpRxTimestamp_get
 * Description:
 *      Get PTP Rx timstamp according to the PTP identifier on the dedicated port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      identifier - indentifier of PTP packet
 * Output:
 *      pTimeStamp - pointer buffer of TIME timestamp
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_8218b_ptpRxTimestamp_get(uint32 unit, rtk_port_t port,
    rtk_time_ptpIdentifier_t identifier, rtk_time_timeStamp_t *pTimeStamp)
{
    int32 ret;
    uint32 reg_addr, reg_val;
    uint32 sec_l, sec_h, nsec_l, nsec_h;

    reg_addr = REG_8218B_PTP_TIME_RX_SID_PN + \
               REG_8218B_PTP_OFFSET_PORT(port) + \
               REG_8218B_PTP_OFFSET_MSGTYPE(identifier.msgType);

    if ((ret = _phy_8218b_ptpReg_get(unit, port, reg_addr, &reg_val)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Input sequence ID NEED match currently sequence ID of chip */
    if (reg_val != identifier.sequenceId)
    {
        reg_addr = REG_8218B_PTP_TIME_CTRL_PN + REG_8218B_PTP_OFFSET_PORT(port);
        reg_val = 0x1 << (4 + REG_8218B_PTP_OFFSET_MSGTYPE(identifier.msgType));
        if ((ret = _phy_8218b_ptpReg_set(unit, port, reg_addr, reg_val, reg_val)) != RT_ERR_OK)
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
            return ret;
        }

        return RT_ERR_INPUT;
    }

    /* Get Rx Timestamp */
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_SEC_L_PN + REG_8218B_PTP_OFFSET_PORT(port), &sec_l)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_SEC_H_PN + REG_8218B_PTP_OFFSET_PORT(port), &sec_h)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_NSEC_L_PN + REG_8218B_PTP_OFFSET_PORT(port), &nsec_l)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_NSEC_H_PN + REG_8218B_PTP_OFFSET_PORT(port), &nsec_h)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    /* Clear the pending status */
    reg_addr = REG_8218B_PTP_TIME_CTRL_PN + REG_8218B_PTP_OFFSET_PORT(port);
    reg_val = 0x1 << (4 + REG_8218B_PTP_OFFSET_MSGTYPE(identifier.msgType));
    if ((ret = _phy_8218b_ptpReg_set(unit, port, reg_addr, reg_val, reg_val)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    pTimeStamp->sec = (sec_h << 16) | (sec_l & 0xFFFF);
    pTimeStamp->nsec = (((nsec_h & 0x7FF) << 16) | (nsec_l & 0xFFFF)) << 3; /* convert 8nsec to nsec */

    return RT_ERR_OK;
}

/* Function Name:
 *      phy_8218b_ptpTxTimestamp_get
 * Description:
 *      Get PTP Tx timstamp according to the PTP identifier on the dedicated port from the specified device.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      identifier - indentifier of PTP packet
 * Output:
 *      pTimeStamp - pointer buffer of TIME timestamp
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
extern int32
phy_8218b_ptpTxTimestamp_get(uint32 unit, rtk_port_t port,
    rtk_time_ptpIdentifier_t identifier, rtk_time_timeStamp_t *pTimeStamp)
{
    int32 ret;
    uint32 reg_addr, reg_val;
    uint32 sec_l, sec_h, nsec_l, nsec_h;

    reg_addr = REG_8218B_PTP_TIME_TX_SID_PN + \
               REG_8218B_PTP_OFFSET_PORT(port) + \
               REG_8218B_PTP_OFFSET_MSGTYPE(identifier.msgType);

    if ((ret = _phy_8218b_ptpReg_get(unit, port, reg_addr, &reg_val)) != RT_ERR_OK)
    {
        return ret;
    }

    /* Input sequence ID NEED match currently sequence ID of chip */
    if (reg_val != identifier.sequenceId)
    {
        /* Clear the pending status */
        reg_addr = REG_8218B_PTP_TIME_CTRL_PN + REG_8218B_PTP_OFFSET_PORT(port);
        reg_val = 0x1 << (REG_8218B_PTP_OFFSET_MSGTYPE(identifier.msgType));
        if ((ret = _phy_8218b_ptpReg_set(unit, port, reg_addr, reg_val, reg_val)) != RT_ERR_OK)
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
            return ret;
        }

        return RT_ERR_NOT_FINISH;
    }

    /* Get Tx Timestamp */
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_SEC_L_PN + REG_8218B_PTP_OFFSET_PORT(port), &sec_l)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_SEC_H_PN + REG_8218B_PTP_OFFSET_PORT(port), &sec_h)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_NSEC_L_PN + REG_8218B_PTP_OFFSET_PORT(port), &nsec_l)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }
    if ((ret = _phy_8218b_ptpReg_get(unit, port, \
        REG_8218B_PTP_TIME_NSEC_H_PN + REG_8218B_PTP_OFFSET_PORT(port), &nsec_h)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    /* Clear the pending status */
    reg_addr = REG_8218B_PTP_TIME_CTRL_PN + REG_8218B_PTP_OFFSET_PORT(port);
    reg_val = 0x1 << (REG_8218B_PTP_OFFSET_MSGTYPE(identifier.msgType));
    if ((ret = _phy_8218b_ptpReg_set(unit, port, reg_addr, reg_val, reg_val)) != RT_ERR_OK)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "");
        return ret;
    }

    pTimeStamp->sec = (sec_h << 16) | (sec_l & 0xFFFF);
    pTimeStamp->nsec = (((nsec_h & 0x7FF) << 16) | (nsec_l & 0xFFFF)) << 3; /* convert 8nsec to nsec */

    return RT_ERR_OK;
}


/* Function Name:
 *      phy_8214fc_MP_masterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      masterSlave         - PHY master slave configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_INPUT         - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8214fc_MP_masterSlave_set(uint32 unit, rtk_port_t port, rtk_port_masterSlave_t masterSlave)
{
    rtk_port_media_t    media;
    uint32              phyData0;
    uint32              page, reg;
    int32               ret;

    if ((ret = phy_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + (PHY_1000_BASET_CONTROL_REG % PHY_INT_REG_NEXT);

    /* for fiber force and auto */
    if (PORT_MEDIA_COPPER != media)
    {
        page = PHY_PAGE_1;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 &= ~ (0x3 << 11);
        switch(masterSlave)
        {
            case PORT_AUTO_MODE:
                phyData0 |= 0x0 << 11;
                break;
            case PORT_SLAVE_MODE:
                phyData0 |= 0x2 << 11;
                break;
            case PORT_MASTER_MODE:
                phyData0 |= 0x3 << 11;
                break;
            default:
                return RT_ERR_INPUT;
        }

        if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
            return ret;

            goto ERR;
    }

    /* for copper force and auto */
    if (PORT_MEDIA_FIBER != media)
    {
        page = 0xa41;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 &= ~ (0x3 << 11);
        switch(masterSlave)
        {
            case PORT_AUTO_MODE:
                phyData0 |= 0x0 << 11;
                break;
            case PORT_SLAVE_MODE:
                phyData0 |= 0x2 << 11;
                break;
            case PORT_MASTER_MODE:
                phyData0 |= 0x3 << 11;
                break;
            default:
                return RT_ERR_INPUT;
        }

        if ((ret = hal_miim_write(unit, port, page, reg, phyData0)) != RT_ERR_OK)
            return ret;

    }

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_MP_masterSlave_set */

/* Function Name:
 *      phy_8214fc_MP_masterSlave_get
 * Description:
 *      Get PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pMasterSlaveCfg     - pointer to the PHY master slave configuration
 *      pMasterSlaveActual  - pointer to the PHY master slave actual link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      This function only works on giga/ 10g port to get its master/slave mode configuration.
 */
int32
phy_8214fc_MP_masterSlave_get(uint32 unit, rtk_port_t port, rtk_port_masterSlave_t *pMasterSlaveCfg, rtk_port_masterSlave_t *pMasterSlaveActual)
{
    rtk_port_media_t    media;
    uint32              phyData0;
    uint32              page, reg;
    int32               ret;

    if ((ret = phy_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    reg = PHY_INT_REG_BASE + (PHY_1000_BASET_CONTROL_REG % PHY_INT_REG_NEXT);

    /* for fiber force and auto */
    if (PORT_MEDIA_COPPER != media)
    {
        page = PHY_PAGE_1;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0003)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 = (phyData0 >> 11) & 0x3;
        switch(phyData0)
        {
            case 0x2:
                *pMasterSlaveCfg = PORT_SLAVE_MODE;
                break;
            case 0x3:
                *pMasterSlaveCfg = PORT_MASTER_MODE;
                break;
            default:
                *pMasterSlaveCfg = PORT_AUTO_MODE;
                break;
        }

        reg = PHY_INT_REG_BASE + (PHY_1000_BASET_STATUS_REG % PHY_INT_REG_NEXT);
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 = (phyData0 >> 14) & 0x1;
        switch(phyData0)
        {
            case 0x0:
                *pMasterSlaveActual = PORT_SLAVE_MODE;
                break;
            case 0x1:
                *pMasterSlaveActual = PORT_MASTER_MODE;
                break;
            default:
                *pMasterSlaveActual = PORT_SLAVE_MODE;
                break;
         }

            goto ERR;
    }

    /* for copper force and auto */
    if (PORT_MEDIA_FIBER != media)
    {
        page = 0xa41;
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0001)) != RT_ERR_OK)
            goto ERR;
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 = (phyData0 >> 11) & 0x3;
        switch(phyData0)
        {
            case 0x2:
                *pMasterSlaveCfg = PORT_SLAVE_MODE;
                break;
            case 0x3:
                *pMasterSlaveCfg = PORT_MASTER_MODE;
                break;
            default:
                *pMasterSlaveCfg = PORT_AUTO_MODE;
                break;
        }

        reg = PHY_INT_REG_BASE + (PHY_1000_BASET_STATUS_REG % PHY_INT_REG_NEXT);
        if ((ret = hal_miim_read(unit, port, page, reg, &phyData0)) != RT_ERR_OK)
            goto ERR;

        phyData0 = (phyData0 >> 14) & 0x1;
        switch(phyData0)
        {
            case 0x0:
                *pMasterSlaveActual = PORT_SLAVE_MODE;
                break;
            case 0x1:
                *pMasterSlaveActual = PORT_MASTER_MODE;
                break;
            default:
                *pMasterSlaveActual = PORT_SLAVE_MODE;
                break;
         }

    }

ERR:
    hal_miim_write(unit, port, PHY_PAGE_0, 29, 0x0000);
    hal_miim_write(unit, port, PHY_PAGE_0, PHY_PAGE_SELECTION_REG, 0x0000);
    return ret;
} /* end of phy_8214fc_MP_masterSlave_get */

/* Function Name:
 *      phy_8218fb_MP_masterSlave_set
 * Description:
 *      Set PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 *      masterSlave         - PHY master slave configuration
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_INPUT         - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8218fb_MP_masterSlave_set(uint32 unit, rtk_port_t port, rtk_port_masterSlave_t masterSlave)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_MP_masterSlave_set(unit, port, masterSlave);
    else
        return phy_common_masterSlave_set(unit, port, masterSlave);

    return RT_ERR_OK;
} /* end of phy_8218fb_MP_masterSlave_set */

/* Function Name:
 *      phy_8218fb_MP_masterSlave_get
 * Description:
 *      Get PHY configuration of master/slave mode of the specific port
 * Input:
 *      unit                - unit id
 *      port                - port id
 * Output:
 *      pMasterSlaveCfg     - pointer to the PHY master slave configuration
 *      pMasterSlaveActual  - pointer to the PHY master slave actual link status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID       - invalid port id
 *      RT_ERR_NULL_POINTER  - input parameter may be null pointer
 * Note:
 *      This function only works on giga/ 10g port to get its master/slave mode configuration.
 */
int32
phy_8218fb_MP_masterSlave_get(uint32 unit, rtk_port_t port, rtk_port_masterSlave_t *pMasterSlaveCfg, rtk_port_masterSlave_t *pMasterSlaveActual)
{
    if (phy_8218FB_info.isComboPhy[port % PORT_NUM_IN_8218FB])
        return phy_8214fc_MP_masterSlave_get(unit, port, pMasterSlaveCfg, pMasterSlaveActual);
    else
        return phy_common_masterSlave_get(unit, port, pMasterSlaveCfg, pMasterSlaveActual);

    return RT_ERR_OK;

} /* end of phy_8218fb_MP_masterSlave_get */

/* Function Name:
 *      phy_8214fc_fiberInternalLoopBack_set
 * Description:
 *      Set Fiber-Port Internal Loopback feature,
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      enable - Fiber-Port Internal Loopback feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fc_fiberInternalLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  phyData;
    uint32  reg_val, reg_val1;
    int32    ret;

    /*Enable Loopback*/
    /*Backup*/
    if ((ret = hal_miim_read(unit, port, 0, 30, &phyData)) != RT_ERR_OK)
        return ret;

    reg_val = 0x03;
    if ((ret = hal_miim_write(unit, port, 0, 30, reg_val)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 8, 16, &reg_val)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 0, 16, &reg_val1)) != RT_ERR_OK)
        return ret;


    if(enable == ENABLED)
    {
        reg_val &= ~(1UL<<8);
        reg_val1 |= (1UL<<14);

    }else{
        reg_val |= (1UL<<8);
        reg_val1 &= ~(1UL<<14);
    }	

    if ((ret = hal_miim_write(unit, port, 8, 16, reg_val)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_write(unit, port, 0, 16, reg_val1)) != RT_ERR_OK)
        return ret;


    /*Restore*/
    if ((ret = hal_miim_write(unit, port, 0, 30, phyData)) != RT_ERR_OK)
        return ret;

    if(enable == ENABLED)
    {
        /*Delay until PHY Linkup*/
        osal_time_usleep(100 * 1000); /* delay 10mS */
    }

    reg_val = 0x0;
    if ((ret = hal_miim_write(unit, port, 0, 30, reg_val)) != RT_ERR_OK)
        return ret;


    return ret;
}   /* end of phy_8214fc_fiberInternalLoopBack_set */


