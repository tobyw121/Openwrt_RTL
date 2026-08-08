/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 36280 $
 * $Date: 2013-01-16 21:24:58 +0800 (Wed, 16 Jan 2013) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) oam
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/oam.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_oam_init(uint32 unit)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    SETSOCKOPT(RTDRV_OAM_INIT, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_init */


int32 rtk_oam_oamCounter_get(uint32 unit, rtk_port_t port, uint32 *pNumber)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_OAM_OAMCOUNTER_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pNumber = port_cfg.data;

    return RT_ERR_OK;
}/* end of rtk_oam_oamCounter_get */


int32 rtk_oam_txTestFrame_start(uint32 unit, rtk_oam_testFrameCfg_t *pTestFrameCfg)
{
    rtdrv_oamSpgCfg_t spgCfg;

    spgCfg.unit = unit;
    memcpy(&spgCfg.testFrameCfg, pTestFrameCfg, sizeof(rtk_oam_testFrameCfg_t));
    SETSOCKOPT(RTDRV_OAM_TXTESTFRAME_START, &spgCfg, rtdrv_oamSpgCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_txTestFrame_start */


int32 rtk_oam_txTestFrame_stop(uint32 unit)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    SETSOCKOPT(RTDRV_OAM_TXTESTFRAME_STOP, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_txTestFrame_stop */


int32 rtk_oam_txStatusOfTestFrame_get(uint32 unit, rtk_oam_testFrameTxStatus_t *pTxStatus, uint64 *pTxCount)
{
    rtdrv_oamSpgCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_TXSTATUSOFTESTFRAME_GET, &config, rtdrv_oamSpgCfg_t, 1);
    *pTxStatus = config.txStatus;
    *pTxCount = config.txCount;

    return RT_ERR_OK;
} /* end of rtk_oam_txStatusOfTestFrame_get */


int32 rtk_oam_loopbackMode_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackMode_t  *pLoopbackMode)
{
    rtdrv_oamLoopbackCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_LOOPBACKMODE_GET, &config, rtdrv_oamLoopbackCfg_t, 1);
    *pLoopbackMode = config.lpbackMode;

    return RT_ERR_OK;
} /* end of rtk_oam_loopbackMode_get */


int32 rtk_oam_loopbackMode_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackMode_t  loopbackMode)
{
    rtdrv_oamLoopbackCfg_t config;

    config.unit = unit;
    config.port = port;
    config.lpbackMode = loopbackMode;
    SETSOCKOPT(RTDRV_OAM_LOOPBACKMODE_SET, &config, rtdrv_oamLoopbackCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_loopbackMode_set */


int32 rtk_oam_loopbackCtrl_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl)
{
    rtdrv_oamLoopbackCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamLoopbackCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_LOOPBACKCTRL_GET, &config, rtdrv_oamLoopbackCfg_t, 1);
    memcpy(pLoopbackCtrl, &config.lpbackCtrl, sizeof(rtk_oam_loopbackCtrl_t));

    return RT_ERR_OK;
} /* end of rtk_oam_loopbackCtrl_get */


int32 rtk_oam_loopbackCtrl_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl)
{
    rtdrv_oamLoopbackCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamLoopbackCfg_t));

    config.unit = unit;
    config.port = port;
    memcpy(&config.lpbackCtrl, pLoopbackCtrl, sizeof(rtk_oam_loopbackCtrl_t));
    SETSOCKOPT(RTDRV_OAM_LOOPBACKCTRL_SET, &config, rtdrv_oamLoopbackCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_loopbackCtrl_set */


int32 rtk_oam_DyingGaspSend_start(uint32 unit, rtk_portmask_t *pPortmask)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    memcpy(&config.portMask, pPortmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_OAM_DYINGGASPSEND_START, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_DyingGaspSend_start */

int32 rtk_oam_portDyingGaspPayload_set(uint32 unit, rtk_port_t port,
    uint8 *pPayload, uint32 len)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    config.port = port;
    config.cnt = len;
    memcpy((char*)config.payload, (char*)pPayload, len);
    SETSOCKOPT(RTDRV_OAM_PORTDYINGGASPPAYLOAD_SET, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_oam_dyingGaspSend_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    config.enable = enable;
    SETSOCKOPT(RTDRV_OAM_DYINGGASPSEND_SET, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_dyingGaspSend_set */

int32 rtk_oam_autoDyingGaspEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_AUTODYINGGASPENABLE_GET, &config, rtdrv_oamDyingGaspCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
}/* end of rtk_oam_autoDyingGaspEnable_get */


int32 rtk_oam_autoDyingGaspEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    config.port = port;
    config.enable = enable;
    SETSOCKOPT(RTDRV_OAM_AUTODYINGGASPENABLE_SET, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_autoDyingGaspEnable_set */


int32 rtk_oam_dyingGaspTLV_get(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_DYINGGASPTLV_GET, &config, rtdrv_oamDyingGaspCfg_t, 1);
    memcpy(pTLV, &config.tlv, sizeof(rtk_oam_dyingGaspTLV_t));

    return RT_ERR_OK;
} /* end of rtk_oam_dyingGaspTLV_get */


int32 rtk_oam_dyingGaspTLV_set(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    config.port = port;
    memcpy(&config.tlv, pTLV, sizeof(rtk_oam_dyingGaspTLV_t));
    SETSOCKOPT(RTDRV_OAM_DYINGGASPTLV_SET, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
}/* end of rtk_oam_dyingGaspTLV_set */


int32 rtk_oam_dyingGaspWaitTime_get(uint32 unit, uint32 *pTime)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_DYINGGASPWAITTIME_GET, &config, rtdrv_oamDyingGaspCfg_t, 1);
    *pTime = config.waitTime;

    return RT_ERR_OK;
} /* end of rtk_oam_dyingGaspWaitTime_get */


int32 rtk_oam_dyingGaspWaitTime_set(uint32 unit, uint32 time)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    config.waitTime = time;
    SETSOCKOPT(RTDRV_OAM_DYINGGASPWAITTIME_SET, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_dyingGaspWaitTime_set */


int32 rtk_oam_cfmEntry_get(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.cfmIdx = cfm_idx;
    GETSOCKOPT(RTDRV_OAM_CFMENTRY_GET, &config, rtdrv_oamCfmCfg_t, 1);
    memcpy(pCfm, &config.cfmCfg, sizeof(rtk_oam_cfm_t));

    return RT_ERR_OK;
} /* end of rtk_oam_cfmEntry_get */


int32 rtk_oam_cfmEntry_set(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.cfmIdx = cfm_idx;
    memcpy(&config.cfmCfg, pCfm, sizeof(rtk_oam_cfm_t));
    SETSOCKOPT(RTDRV_OAM_CFMENTRY_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmEntry_set */


int32 rtk_oam_cfmPortEntry_get(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_CFMPORTENTRY_GET, &config, rtdrv_oamCfmCfg_t, 1);
    memcpy(pPortCfg, &config.portCfg, sizeof(rtk_oam_cfmPort_t));

    return RT_ERR_OK;
} /* end of rtk_oam_cfmPortEntry_get */


int32 rtk_oam_cfmPortEntry_set(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.port = port;
    memcpy(&config.portCfg, pPortCfg, sizeof(rtk_oam_cfmPort_t));
    SETSOCKOPT(RTDRV_OAM_CFMPORTENTRY_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmPortEntry_set */


int32 rtk_oam_cfmMepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_CFMMEPENABLE_GET, &config, rtdrv_oamCfmCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmMepEnable_get */


int32 rtk_oam_cfmMepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.port = port;
    config.enable = enable;
    SETSOCKOPT(RTDRV_OAM_CFMMEPENABLE_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmMepEnable_set */


int32 rtk_oam_txCCMFrame_start(uint32 unit, rtk_portmask_t *pPortmask)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    memcpy(&config.portMask, pPortmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_OAM_TXCCMFRAME_START, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_txCCMFrame_start */


int32 rtk_oam_txCCMFrame_stop(uint32 unit)
{
    rtdrv_oamDyingGaspCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamDyingGaspCfg_t));

    config.unit = unit;
    SETSOCKOPT(RTDRV_OAM_TXCCMFRAME_STOP, &config, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_txCCMFrame_stop */


int32 rtk_oam_cfmCCMFrame_get(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    config.cfmIdx= cfm_idx;
    GETSOCKOPT(RTDRV_OAM_CFMCCMFRAME_GET, &config, rtdrv_oamCcmCfg_t, 1);
    memcpy(pCcmFrame, &config.ccmFrame, sizeof(rtk_oam_ccmFrame_t));

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMFrame_get */


int32 rtk_oam_cfmCCMFrame_set(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    config.cfmIdx= cfm_idx;
    memcpy(&config.ccmFrame, pCcmFrame, sizeof(rtk_oam_ccmFrame_t));
    SETSOCKOPT(RTDRV_OAM_CFMCCMFRAME_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMFrame_set */


int32 rtk_oam_cfmCCMSnapOui_get(uint32 unit, rtk_snapOui_t *pSnapoui)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMSNAPOUI_GET, &config, rtdrv_oamCcmCfg_t, 1);
    memcpy(pSnapoui, &config.snapoui, sizeof(rtk_snapOui_t));

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMSnapOui_get */


int32 rtk_oam_cfmCCMSnapOui_set(uint32 unit, rtk_snapOui_t *pSnapoui)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    memcpy(&config.snapoui, pSnapoui, sizeof(rtk_snapOui_t));
    SETSOCKOPT(RTDRV_OAM_CFMCCMSNAPOUI_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMSnapOui_set */


int32 rtk_oam_cfmCCMEtype_get(uint32 unit, uint32 *pEtherType)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMETYPE_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *pEtherType = config.etherType;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMEtype_get */


int32 rtk_oam_cfmCCMEtype_set(uint32 unit, uint32 etherType)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    config.etherType = etherType;
    SETSOCKOPT(RTDRV_OAM_CFMCCMETYPE_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMEtype_set */


int32 rtk_oam_cfmCCMOpcode_get(uint32 unit, uint32 *pOpcode)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMOPCODE_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *pOpcode = config.opCode;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMOpcode_get */


int32 rtk_oam_cfmCCMOpcode_set(uint32 unit, uint32 opcode)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    config.opCode = opcode;
    SETSOCKOPT(RTDRV_OAM_CFMCCMOPCODE_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMOpcode_set */


int32 rtk_oam_cfmCCMFlag_get(uint32 unit, uint32 cfm_idx, uint32 *pCcmFlag)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    config.cfmIdx = cfm_idx;
    GETSOCKOPT(RTDRV_OAM_CFMCCMFLAG_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *pCcmFlag = config.ccmFlag;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMFlag_get */


int32 rtk_oam_cfmCCMFlag_set(uint32 unit, uint32 cfm_idx, uint32 ccmFlag)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    config.cfmIdx = cfm_idx;
    config.ccmFlag = ccmFlag;
    SETSOCKOPT(RTDRV_OAM_CFMCCMFLAG_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}  /* end of rtk_oam_cfmCCMFlag_set */


int32 rtk_oam_cfmCCMInterval_get(uint32 unit, rtk_oam_ccmInterval_t *pInterval)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMINTERVAL_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *pInterval = config.ccmInterval;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMInterval_get */


int32 rtk_oam_cfmCCMInterval_set(uint32 unit, rtk_oam_ccmInterval_t interval)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    config.ccmInterval = interval;
    SETSOCKOPT(RTDRV_OAM_CFMCCMINTERVAL_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCMInterval_set */


int32 rtk_oam_cfmIntfStatus_get(uint32 unit, rtk_port_t port, uint32 *pIntfStatus)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_CFMINTFSTATUS_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);
    *pIntfStatus = config.status;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmIntfStatus_get */


int32 rtk_oam_cfmIntfStatus_set(uint32 unit, rtk_port_t port, uint32 intfStatus)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    config.status = intfStatus;
    SETSOCKOPT(RTDRV_OAM_CFMINTFSTATUS_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmIntfStatus_set */


int32 rtk_oam_cfmPortStatus_get(uint32 unit, rtk_port_t port, uint32 *pPortStatus)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_CFMPORTSTATUS_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);
    *pPortStatus = config.status;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmPortStatus_get */


int32 rtk_oam_cfmPortStatus_set(uint32 unit, rtk_port_t port, uint32 portStatus)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    config.status = portStatus;
    SETSOCKOPT(RTDRV_OAM_CFMPORTSTATUS_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmPortStatus_set */


int32 rtk_oam_cfmRemoteMep_del(uint32 unit, uint32 mepid)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.mepid = mepid;
    SETSOCKOPT(RTDRV_OAM_CFMREMOTEMEP_DEL, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmRemoteMep_del */


int32 rtk_oam_cfmRemoteMep_add(uint32 unit, uint32 mepid)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.mepid = mepid;
    SETSOCKOPT(RTDRV_OAM_CFMREMOTEMEP_ADD, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmRemoteMep_add */


int32 rtk_oam_cfmCCStatus_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              remoteMepid,
    uint32              *pRdi,
    rtk_oam_ccStatus_t  *pCcStatus)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    config.mepid = remoteMepid;
    GETSOCKOPT(RTDRV_OAM_CFMCCSTATUS_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);
    *pRdi = config.rdi;
    *pCcStatus = config.ccStatus;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCStatus_get */


int32 rtk_oam_cfmCCStatus_reset(uint32 unit, rtk_port_t port)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    SETSOCKOPT(RTDRV_OAM_CFMCCSTATUS_RESET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmCCStatus_reset */


int32 rtk_oam_cfmLoopbackReplyEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_CFMLOOPBACKREPLYENABLE_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);
    *pEnable = config.loopbackEnable;

    return RT_ERR_OK;
} /* end of rtk_oam_cfmLoopbackReplyEnable_get */


int32 rtk_oam_cfmLoopbackReplyEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    config.loopbackEnable = enable;
    SETSOCKOPT(RTDRV_OAM_CFMLOOPBACKREPLYENABLE_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_oam_cfmLoopbackReplyEnable_set */


int32 rtk_oam_cfmLoopbackReplyCtrl_get(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMLOOPBACKREPLYCTRL_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);
    memcpy(pCtrl, &config.ctrl, sizeof(rtk_oam_cfmLoopbackCtrl_t));

    return RT_ERR_OK;
}   /*end ofrtk_oam_cfmLoopbackReplyCtrl_get*/

int32 rtk_oam_cfmLoopbackReplyCtrl_set(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    memcpy(&config.ctrl, pCtrl, sizeof(rtk_oam_cfmLoopbackCtrl_t));
    SETSOCKOPT(RTDRV_OAM_CFMLOOPBACKREPLYCTRL_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
}   /*end ofrtk_oam_cfmLoopbackReplyCtrl_set*/

int32 rtk_oam_loopbackMacSwapEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_LOOPBACKMACSWAPENABLE_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    *pEnable = config.loopbackEnable;

    return RT_ERR_OK;
}   /* end of rtk_oam_loopbackMacSwapEnable_get */

int32 rtk_oam_loopbackMacSwapEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.loopbackEnable = enable;
    SETSOCKOPT(RTDRV_OAM_LOOPBACKMACSWAPENABLE_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_loopbackMacSwapEnable_set */

int32 rtk_oam_portLoopbackMuxAction_get(uint32 unit, rtk_port_t port,
        rtk_action_t *pAction)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_PORTLOOPBACKMUXACTION_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    *pAction = config.action;

    return RT_ERR_OK;
}   /* end of rtk_oam_portLoopbackMuxAction_get */

int32 rtk_oam_portLoopbackMuxAction_set(uint32 unit, rtk_port_t port,
        rtk_action_t action)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    config.port = port;
    config.action = action;
    SETSOCKOPT(RTDRV_OAM_PORTLOOPBACKMUXACTION_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_portLoopbackMuxAction_set */

int32
rtk_oam_cfmCcmPcp_get(uint32 unit, uint32 *pcp)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMPCP_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *pcp = config.ccmFrame.outer_pri;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmPcp_get */

int32
rtk_oam_cfmCcmPcp_set(uint32 unit, uint32 pcp)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit                 = unit;
    config.ccmFrame.outer_pri   = pcp;
    SETSOCKOPT(RTDRV_OAM_CFMCCMPCP_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmPcp_set */

int32
rtk_oam_cfmCcmCfi_get(uint32 unit, uint32 *cfi)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMCFI_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *cfi = config.ccmFrame.outer_dei;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmCfi_get */

int32
rtk_oam_cfmCcmCfi_set(uint32 unit, uint32 cfi)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit                 = unit;
    config.ccmFrame.outer_dei   = cfi;
    SETSOCKOPT(RTDRV_OAM_CFMCCMCFI_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmCfi_set */

int32
rtk_oam_cfmCcmTpid_get(uint32 unit, uint32 *tpid)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMTPID_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *tpid = config.ccmFrame.outer_tpid;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmTpid_get */

int32
rtk_oam_cfmCcmTpid_set(uint32 unit, uint32 tpid)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit                 = unit;
    config.ccmFrame.outer_tpid  = tpid;
    SETSOCKOPT(RTDRV_OAM_CFMCCMTPID_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmTpid_set */

int32
rtk_oam_cfmCcmInstLifetime_get(uint32 unit, uint32 instance,
                                uint32 *lifetime)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    GETSOCKOPT(RTDRV_OAM_CFMCCMRESETLIFETIME_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *lifetime = config.ccmFlag;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstLifetime_get */

int32
rtk_oam_cfmCcmInstLifetime_set(uint32 unit, uint32 instance,
                                uint32 lifetime)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.ccmFlag  = lifetime;
    SETSOCKOPT(RTDRV_OAM_CFMCCMRESETLIFETIME_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstLifetime_set */

int32
rtk_oam_cfmCcmMepid_get(uint32 unit, uint32 *mepid)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMMEPID_GET, &config, rtdrv_oamCfmMiscCfg_t, 1);
    *mepid = config.mepid;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmMepid_get */

int32
rtk_oam_cfmCcmMepid_set(uint32 unit, uint32 mepid)
{
    rtdrv_oamCfmMiscCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmMiscCfg_t));

    config.unit     = unit;
    config.mepid    = mepid;
    SETSOCKOPT(RTDRV_OAM_CFMCCMMEPID_SET, &config, rtdrv_oamCfmMiscCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmMepid_set */

int32
rtk_oam_cfmCcmIntervalField_get(uint32 unit, uint32 *interval)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMINTERVALFIELD_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *interval = config.ccmFlag;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmIntervalField_get */

int32
rtk_oam_cfmCcmIntervalField_set(uint32 unit, uint32 interval)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.ccmFlag  = interval;
    SETSOCKOPT(RTDRV_OAM_CFMCCMINTERVALFIELD_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmIntervalField_set */

int32
rtk_oam_cfmCcmMdl_get(uint32 unit, uint32 *mdl)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    GETSOCKOPT(RTDRV_OAM_CFMCCMMDL_GET, &config, rtdrv_oamCfmCfg_t, 1);
    *mdl = config.cfmCfg.md_level;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmMdl_get */

int32
rtk_oam_cfmCcmMdl_set(uint32 unit, uint32 mdl)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit             = unit;
    config.cfmCfg.md_level  = mdl;
    SETSOCKOPT(RTDRV_OAM_CFMCCMMDL_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmMdl_set */

int32
rtk_oam_cfmCcmInstTagStatus_get(uint32 unit, uint32 instance,
                                rtk_enable_t *enable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    GETSOCKOPT(RTDRV_OAM_CFMCCMINSTTAGSTATUS_GET, &config, rtdrv_oamCfmCfg_t, 1);
    *enable = config.enable;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstTagStatus_get */

int32
rtk_oam_cfmCcmInstTagStatus_set(uint32 unit, uint32 instance,
                                rtk_enable_t enable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.enable   = enable;
    SETSOCKOPT(RTDRV_OAM_CFMCCMINSTTAGSTATUS_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstTagStatus_set */

int32
rtk_oam_cfmCcmInstVid_get(uint32 unit, uint32 instance,
                          rtk_vlan_t *vid)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    GETSOCKOPT(RTDRV_OAM_CFMCCMINSTVID_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *vid = config.ccmFrame.outer_vid;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmInstVid_get */

int32
rtk_oam_cfmCcmInstVid_set(uint32 unit, uint32 instance,
                          rtk_vlan_t vid)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit                 = unit;
    config.cfmIdx               = instance;
    config.ccmFrame.outer_vid   = vid;
    SETSOCKOPT(RTDRV_OAM_CFMCCMINSTVID_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstVid_set */

int32
rtk_oam_cfmCcmInstMaid_get(uint32 unit, uint32 instance,
                           uint32 *maid)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    GETSOCKOPT(RTDRV_OAM_CFMCCMINSTMAID_GET, &config, rtdrv_oamCfmCfg_t, 1);
    *maid = config.maid;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmInstMaid_get */

int32
rtk_oam_cfmCcmInstMaid_set(uint32 unit, uint32 instance,
                           uint32 maid)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.maid     = maid;
    SETSOCKOPT(RTDRV_OAM_CFMCCMINSTMAID_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstMaid_set */

int32
rtk_oam_cfmCcmInstTxStatus_get(uint32 unit, uint32 instance,
                               rtk_enable_t *enable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    GETSOCKOPT(RTDRV_OAM_CFMCCMINSTTXSTATUS_GET, &config, rtdrv_oamCfmCfg_t, 1);
    *enable = config.enable;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstTxStatus_get */

int32
rtk_oam_cfmCcmInstTxStatus_set(uint32 unit, uint32 instance,
                               rtk_enable_t enable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.enable   = enable;
    SETSOCKOPT(RTDRV_OAM_CFMCCMINSTTXSTATUS_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmInstTxStatus_set */

int32
rtk_oam_cfmCcmInstInterval_get(uint32 unit, uint32 instance,
                               uint32 *interval)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    GETSOCKOPT(RTDRV_OAM_CFMCCMINSTINTERVAL_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *interval = config.ccmInterval;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstInterval_get */

int32
rtk_oam_cfmCcmInstInterval_set(uint32 unit, uint32 instance,
                               uint32 interval)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.ccmInterval = interval;
    SETSOCKOPT(RTDRV_OAM_CFMCCMINSTINTERVAL_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstInterval_set */

int32
rtk_oam_cfmCcmTxInstPort_get(uint32 unit, uint32 instance,
                             uint32 index, rtk_port_t *port)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.portIdx  = index;
    GETSOCKOPT(RTDRV_OAM_CFMCCMTXINSTPORT_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *port = config.port;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmTxInstPort_get */

int32
rtk_oam_cfmCcmTxInstPort_set(uint32 unit, uint32 instance,
                             uint32 index, rtk_port_t port)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.portIdx  = index;
    config.port     = port;
    SETSOCKOPT(RTDRV_OAM_CFMCCMTXINSTPORT_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmTxInstPort_set */

int32
rtk_oam_cfmCcmRxInstVid_get(uint32 unit, uint32 instance,
                            rtk_vlan_t *vid)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    GETSOCKOPT(RTDRV_OAM_CFMCCMRXINSTVID_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *vid = config.ccmFrame.outer_vid;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmRxInstVid_get */

int32
rtk_oam_cfmCcmRxInstVid_set(uint32 unit, uint32 instance,
                            rtk_vlan_t vid)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit                 = unit;
    config.cfmIdx               = instance;
    config.ccmFrame.outer_vid   = vid;
    SETSOCKOPT(RTDRV_OAM_CFMCCMRXINSTVID_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmRxInstVid_set */

int32
rtk_oam_cfmCcmRxInstPort_get(uint32 unit, uint32 instance,
                             uint32 index, rtk_port_t *port)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.portIdx  = index;
    GETSOCKOPT(RTDRV_OAM_CFMCCMRXINSTPORT_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *port = config.port;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmRxInstPort_get */

int32
rtk_oam_cfmCcmRxInstPort_set(uint32 unit, uint32 instance,
                             uint32 index, rtk_port_t port)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.portIdx  = index;
    config.port     = port;
    SETSOCKOPT(RTDRV_OAM_CFMCCMRXINSTPORT_SET, &config, rtdrv_oamCcmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmRxInstPort_set */

int32
rtk_oam_cfmCcmInstAliveTime_get(uint32 unit, uint32 instance,
                            uint32 index, uint32 *time)
{
    rtdrv_oamCcmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCcmCfg_t));

    config.unit     = unit;
    config.cfmIdx   = instance;
    config.portIdx  = index;
    GETSOCKOPT(RTDRV_OAM_CFMCCMKEEPALIVE_GET, &config, rtdrv_oamCcmCfg_t, 1);
    *time = config.ccmInterval;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmCcmInstAliveTime_get */


int32
rtk_oam_cfmPortEthDmEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_OAM_CFMETHDMPORTENABLE_GET, &config, rtdrv_oamCfmCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmPortEthDmEnable_get */

int32
rtk_oam_cfmPortEthDmEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.unit = unit;
    config.port = port;
    config.enable = enable;
    SETSOCKOPT(RTDRV_OAM_CFMETHDMPORTENABLE_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_cfmPortEthDmEnable_set */

int32
rtk_oam_cfmEthDmRxTimestamp_get(
    uint32 unit,
    uint32 index,
    rtk_time_timeStamp_t *pTimeStamp)
{
    rtdrv_oamCfmCfg_t config;

    config.unit = unit;
    config.index = index;
    GETSOCKOPT(RTDRV_OAM_CFMETHDMRXTIMESTAMP_GET, &config, rtdrv_oamCfmCfg_t, 1);
    *pTimeStamp = config.timeStamp;

    return RT_ERR_OK;
}

int32
rtk_oam_dyingGaspPktCnt_get(uint32 unit, uint32 *pCnt)
{
    rtdrv_oamDyingGaspCfg_t cfg;

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_OAM_DYINGGASPPKTCNT_GET, &cfg, rtdrv_oamDyingGaspCfg_t, 1);

    *pCnt = cfg.cnt;

    return RT_ERR_OK;
}   /* end of rtk_oam_dyingGaspPktCnt_get */

int32
rtk_oam_dyingGaspPktCnt_set(uint32 unit, uint32 cnt)
{
    rtdrv_oamDyingGaspCfg_t cfg;

    cfg.unit = unit;
    cfg.cnt = cnt;
    SETSOCKOPT(RTDRV_OAM_DYINGGASPPKTCNT_SET, &cfg, rtdrv_oamDyingGaspCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_dyingGaspPktCnt_set */

int32 rtk_oam_linkFaultMonEnable_set(rtk_enable_t enable)
{
    rtdrv_oamCfmCfg_t config;

    memset(&config, 0, sizeof(rtdrv_oamCfmCfg_t));

    config.enable = enable;
    SETSOCKOPT(RTDRV_OAM_LINKFAULTMONENABLE_SET, &config, rtdrv_oamCfmCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_oam_linkFaultMonEnable_set */
