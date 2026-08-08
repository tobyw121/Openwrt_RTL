/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 57334 $
 * $Date: 2015-03-30 14:13:45 +0800 (Mon, 30 Mar 2015) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) port
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/port.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_port_link_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_LINK_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pStatus = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_linkMedia_get(uint32 unit, rtk_port_t port, rtk_port_linkStatus_t *pStatus, rtk_port_media_t *pMedia)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_LINKMEDIA_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pStatus = port_cfg.data;
    *pMedia = port_cfg.media;

    return RT_ERR_OK;
}

int32 rtk_port_speedDuplex_get(uint32 unit, rtk_port_t port, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex)
{
    rtdrv_port_speedDuplex_t sd_status;

    sd_status.unit = unit;
    sd_status.port = port;
    GETSOCKOPT(RTDRV_PORT_SPEED_DUPLEX_GET, &sd_status, rtdrv_port_speedDuplex_t, 1);
    *pSpeed = sd_status.speed;
    *pDuplex = sd_status.duplex;

    return RT_ERR_OK;
}

int32 rtk_port_flowctrl_get(uint32 unit, rtk_port_t port, uint32 *pTxStatus, uint32 *pRxStatus)
{
    rtdrv_port_flowctrl_t fc_status;

    fc_status.unit = unit;
    fc_status.port = port;
    GETSOCKOPT(RTDRV_PORT_FLOW_CTRL_GET, &fc_status, rtdrv_port_flowctrl_t, 1);
    *pTxStatus = fc_status.tx_status;
    *pRxStatus = fc_status.rx_status;

    return RT_ERR_OK;
}

int32 rtk_port_cpuPortId_get(uint32 unit, rtk_port_t *pPort)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    GETSOCKOPT(RTDRV_PORT_CPU_PORT_ID_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    *pPort = unit_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_phyAutoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_EN_AUTONEGO_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnabled = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_phyAutoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enabled;
    SETSOCKOPT(RTDRV_PORT_EN_AUTONEGO_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_phyAutoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtdrv_port_autoNegoAbility_t an_ability;

    an_ability.unit = unit;
    an_ability.port = port;
    GETSOCKOPT(RTDRV_PORT_AUTONEGO_ABIL_GET, &an_ability, rtdrv_port_autoNegoAbility_t, 1);
    memcpy(pAbility, &an_ability.ability, sizeof(rtk_port_phy_ability_t));

    return RT_ERR_OK;
}

int32 rtk_port_phyAutoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
   rtdrv_port_autoNegoAbility_t an_ability;

    an_ability.unit = unit;
    an_ability.port = port;
    memcpy(&an_ability.ability, pAbility, sizeof(rtk_port_phy_ability_t));
    SETSOCKOPT(RTDRV_PORT_AUTONEGO_ABIL_SET, &an_ability, rtdrv_port_autoNegoAbility_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_phyForceModeAbility_get(uint32 unit, rtk_port_t port, rtk_port_speed_t *pSpeed, rtk_port_duplex_t *pDuplex, rtk_enable_t *pFlowControl)
{
    rtdrv_port_forceModeAbility_t fm_ability;

    fm_ability.unit = unit;
    fm_ability.port = port;
    GETSOCKOPT(RTDRV_PORT_FORCE_MODE_ABIL_GET, &fm_ability, rtdrv_port_forceModeAbility_t, 1);
    *pSpeed = fm_ability.speed;
    *pDuplex = fm_ability.duplex;
    *pFlowControl = fm_ability.flowctrl;

    return RT_ERR_OK;
}

int32 rtk_port_phyForceModeAbility_set(uint32 unit, rtk_port_t port, rtk_port_speed_t speed, rtk_port_duplex_t duplex, rtk_enable_t flowControl)
{
    rtdrv_port_forceModeAbility_t fm_ability;

    fm_ability.unit = unit;
    fm_ability.port = port;
    fm_ability.speed = speed;
    fm_ability.duplex = duplex;
    fm_ability.flowctrl = flowControl;
    SETSOCKOPT(RTDRV_PORT_FORCE_MODE_ABIL_SET, &fm_ability, rtdrv_port_forceModeAbility_t, 1);

    return RT_ERR_OK;
}

int32
rtk_port_phyMasterSlave_get(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   *pMasterSlaveCfg,
    rtk_port_masterSlave_t   *pMasterSlaveActual)
{
    rtdrv_port_masterSlave_t masterSlave_cfg;

    masterSlave_cfg.unit = unit;
    masterSlave_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_MASTER_SLAVE_GET, &masterSlave_cfg, rtdrv_port_masterSlave_t, 1);
    *pMasterSlaveCfg = masterSlave_cfg.masterSlaveCfg;
    *pMasterSlaveActual = masterSlave_cfg.masterSlaveActual;

    return RT_ERR_OK;
}

int32
rtk_port_phyMasterSlave_set(
    uint32              unit,
    rtk_port_t          port,
    rtk_port_masterSlave_t   masterSlaveCfg)
{
    rtdrv_port_masterSlave_t masterSlave_cfg;

    masterSlave_cfg.unit = unit;
    masterSlave_cfg.port = port;
    masterSlave_cfg.masterSlaveCfg = masterSlaveCfg;
    SETSOCKOPT(RTDRV_PORT_MASTER_SLAVE_SET, &masterSlave_cfg, rtdrv_port_masterSlave_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_phyReg_get(uint32 unit, rtk_port_t port, uint32 page, rtk_port_phy_reg_t reg, uint32 *pData)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.port = port;
    phy_data.page = page;
    phy_data.reg = reg;
    GETSOCKOPT(RTDRV_PORT_PHY_REG_GET, &phy_data, rtdrv_port_phyReg_t, 1);
    *pData = phy_data.data;

    return RT_ERR_OK;
}

int32 rtk_port_phyReg_set(uint32 unit, rtk_port_t port, uint32 page, rtk_port_phy_reg_t reg, uint32 data)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.port = port;
    phy_data.page = page;
    phy_data.reg = reg;
    phy_data.data = data;
    SETSOCKOPT(RTDRV_PORT_PHY_REG_SET, &phy_data, rtdrv_port_phyReg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_port_phyReg_broadcast_set(
    uint32              unit,
    uint32              page,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.page = page;
    phy_data.reg = reg;
    phy_data.data = data;
    SETSOCKOPT(RTDRV_PORT_PHY_REG_BROADCAST_SET, &phy_data, rtdrv_port_phyReg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_port_phyReg_broadcast_set */

int32
rtk_port_phyReg_broadcastID_set(
    uint32              unit,
    uint32              broadcastID)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.broadcastID = broadcastID;
    SETSOCKOPT(RTDRV_PORT_PHY_REG_BROADCAST_ID_SET, &phy_data, rtdrv_port_phyReg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_port_phyReg_broadcastID_set */



int32
rtk_port_phyExtParkPageReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              *pData)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.port = port;
    phy_data.page = mainPage;
    phy_data.extPage = extPage;
    phy_data.parkPage = parkPage;
    phy_data.reg = reg;
    GETSOCKOPT(RTDRV_PORT_PHY_EXT_PARK_PAGE_REG_GET, &phy_data, rtdrv_port_phyReg_t, 1);
    *pData = phy_data.data;

    return RT_ERR_OK;
}    /* end of rtk_port_phyExtParkPageReg_get */

int32
rtk_port_phyExtParkPageReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.port = port;
    phy_data.page = mainPage;
    phy_data.extPage = extPage;
    phy_data.parkPage = parkPage;
    phy_data.reg = reg;
    phy_data.data = data;
    SETSOCKOPT(RTDRV_PORT_PHY_EXT_PARK_PAGE_REG_SET, &phy_data, rtdrv_port_phyReg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_port_phyExtParkPageReg_set */

int32
rtk_port_phymaskExtParkPageReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mainPage,
    uint32              extPage,
    uint32              parkPage,
    rtk_port_phy_reg_t  reg,
    uint32              data)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.portmask = *pPortmask;
    phy_data.page = mainPage;
    phy_data.extPage = extPage;
    phy_data.parkPage = parkPage;
    phy_data.reg = reg;
    phy_data.data = data;
    SETSOCKOPT(RTDRV_PORT_PHYMASK_EXT_PARK_PAGE_REG_SET, &phy_data, rtdrv_port_phyReg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_port_phymaskExtParkPageReg_set */

int32
rtk_port_phyMmdReg_get(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              *pData)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.port = port;
    phy_data.mmdAddr= mmdAddr;
    phy_data.reg = mmdReg;
    GETSOCKOPT(RTDRV_PORT_PHY_MMD_REG_GET, &phy_data, rtdrv_port_phyReg_t, 1);
    *pData = phy_data.data;

    return RT_ERR_OK;
}    /* end of rtk_port_phyMmdReg_get */

int32
rtk_port_phyMmdReg_set(
    uint32              unit,
    rtk_port_t          port,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.port = port;
    phy_data.mmdAddr= mmdAddr;
    phy_data.reg = mmdReg;
    phy_data.data = data;
    SETSOCKOPT(RTDRV_PORT_PHY_MMD_REG_SET, &phy_data, rtdrv_port_phyReg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_port_phyMmdReg_set */

int32
rtk_port_phymaskMmdReg_set(
    uint32              unit,
    rtk_portmask_t      *pPortmask,
    uint32              mmdAddr,
    uint32              mmdReg,
    uint32              data)
{
    rtdrv_port_phyReg_t phy_data;

    phy_data.unit = unit;
    phy_data.portmask = *pPortmask;
    phy_data.mmdAddr= mmdAddr;
    phy_data.reg = mmdReg;
    phy_data.data = data;
    SETSOCKOPT(RTDRV_PORT_PHYMASK_MMD_REG_SET, &phy_data, rtdrv_port_phyReg_t, 1);

    return RT_ERR_OK;
}    /* end of rtk_port_phymaskMmdReg_set */

int32 rtk_port_isolation_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pPortmask)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_ISOLATION_GET, &port_cfg, rtdrv_portCfg_t, 1);
    memcpy(pPortmask, &port_cfg.portmask, sizeof(rtk_portmask_t));

    return RT_ERR_OK;
}

int32 rtk_port_isolation_set(uint32 unit, rtk_port_t port, rtk_portmask_t portmask)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    memcpy(&port_cfg.portmask, &portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_PORT_ISOLATION_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_isolation_add(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.targetPort = iso_port;
    SETSOCKOPT(RTDRV_PORT_ISOLATION_ADD, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_isolation_add */


int32 rtk_port_isolation_del(uint32 unit, rtk_port_t port, rtk_port_t iso_port)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.targetPort = iso_port;
    SETSOCKOPT(RTDRV_PORT_ISOLATION_DEL, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_isolation_del */

int32 rtk_port_adminEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_EN_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnabled = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_adminEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enabled;
    SETSOCKOPT(RTDRV_PORT_EN_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_txEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_TX_EN_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnabled = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_txEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enabled;
    SETSOCKOPT(RTDRV_PORT_TX_EN_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_rxEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_RX_EN_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnabled = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_rxEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enabled;
    SETSOCKOPT(RTDRV_PORT_RX_EN_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_backpressureEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_BACK_PRESSURE_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnabled = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_backpressureEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enabled)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enabled;
    SETSOCKOPT(RTDRV_PORT_BACK_PRESSURE_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_phyComboPortMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_PHY_MEDIA_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pMedia = port_cfg.media;

    return RT_ERR_OK;
}

int32 rtk_port_phyComboPortMedia_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.media = media;
    SETSOCKOPT(RTDRV_PORT_PHY_MEDIA_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}


int32 rtk_port_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_GREEN_ENABLE_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnable = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enable;
    SETSOCKOPT(RTDRV_PORT_GREEN_ENABLE_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_GIGA_LITE_ENABLE_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnable = port_cfg.data;

    return RT_ERR_OK;
}

int32 rtk_port_gigaLiteEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enable;
    SETSOCKOPT(RTDRV_PORT_GIGA_LITE_ENABLE_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_udldEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_UDLDENABLE_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_port_udldEnable_get */


int32 rtk_port_udldEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.port = port;
    config.enable = enable;
    SETSOCKOPT(RTDRV_PORT_UDLDENABLE_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_udldEnable_set */


int32 rtk_port_udldLinkUpAutoTriggerEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_PORT_UDLDLINKUPAUTOTRIGGERENABLE_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pEnable = config.autoTriggerEnable;

    return RT_ERR_OK;
} /* end of rtk_port_udldLinkUpAutoTriggerEnable_get */


int32 rtk_port_udldLinkUpAutoTriggerEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.autoTriggerEnable = enable;
    SETSOCKOPT(RTDRV_PORT_UDLDLINKUPAUTOTRIGGERENABLE_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
}  /* end of rtk_port_udldLinkUpAutoTriggerEnable_set */


int32 rtk_port_udldTrigger_start(uint32 unit, rtk_port_t port)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.port = port;
    SETSOCKOPT(RTDRV_PORT_UDLDTRIGGER_START, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_udldTrigger_start */


int32 rtk_port_udldStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldStatus_t *pStatus)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_UDLDSTATUS_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pStatus = config.status;

    return RT_ERR_OK;
} /* end of rtk_port_udldStatus_get */


int32 rtk_port_udldEchoAction_get(uint32 unit, rtk_port_udldEchoAction_t *pAction)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_PORT_UDLDECHOACTION_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pAction = config.action;

    return RT_ERR_OK;
}


int32 rtk_port_udldEchoAction_set(uint32 unit, rtk_port_udldEchoAction_t action)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.action = action;
    SETSOCKOPT(RTDRV_PORT_UDLDECHOACTION_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
}


int32 rtk_port_udldLinkStatus_get(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t *pStatus)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_UDLDLINKSTATUS_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pStatus = config.linkStatus;

    return RT_ERR_OK;
}


int32 rtk_port_udldLinkStatus_set(uint32 unit, rtk_port_t port, rtk_port_udldLinkStatus_t status)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.port = port;
    config.linkStatus = status;
    SETSOCKOPT(RTDRV_PORT_UDLDLINKSTATUS_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_udldAutoDisableFailedPortEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_PORT_UDLDAUTODISABLEFAILEDPORTENABLE_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_port_udldAutoDisableFailedPortEnable_get */


int32 rtk_port_udldAutoDisableFailedPortEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.enable = enable;
    SETSOCKOPT(RTDRV_PORT_UDLDAUTODISABLEFAILEDPORTENABLE_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
}  /* end of rtk_port_udldAutoDisableFailedPortEnable_set */


int32 rtk_port_udldInterval_get(uint32 unit, rtk_port_udldInterval_t *pInterval)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_PORT_UDLDINTERVAL_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pInterval = config.interval;

    return RT_ERR_OK;
} /* end of rtk_port_udldInterval_get */


int32 rtk_port_udldInterval_set(uint32 unit, rtk_port_udldInterval_t interval)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.interval = interval;
    SETSOCKOPT(RTDRV_PORT_UDLDINTERVAL_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_udldInterval_set */


int32 rtk_port_udldRetryCount_get(uint32 unit, uint32 *pRetryCount)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_PORT_UDLDRETRYCOUNT_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pRetryCount = config.retryCount;

    return RT_ERR_OK;
} /* end of rtk_port_udldRetryCount_get */


int32 rtk_port_udldRetryCount_set(uint32 unit, uint32 retryCount)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.retryCount = retryCount;
    SETSOCKOPT(RTDRV_PORT_UDLDRETRYCOUNT_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
}/* end of rtk_port_udldRetryCount_set */


int32 rtk_port_udldLedIndicateEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    GETSOCKOPT(RTDRV_PORT_UDLDLEDINDICATEENABLE_GET, &config, rtdrv_portUdldCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_port_udldLedIndicateEnable_get */


int32 rtk_port_udldLedIndicateEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_portUdldCfg_t config;

    config.unit = unit;
    config.enable = enable;
    SETSOCKOPT(RTDRV_PORT_UDLDLEDINDICATEENABLE_SET, &config, rtdrv_portUdldCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_udldLedIndicateEnable_set */


int32 rtk_port_rldpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_RLDPENABLE_GET, &config, rtdrv_portRldpCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_port_rldpEnable_get */


int32 rtk_port_rldpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    config.enable = enable;
    SETSOCKOPT(RTDRV_PORT_RLDPENABLE_SET, &config, rtdrv_portRldpCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_rldpEnable_set */


int32 rtk_port_rldpStatus_get(
    uint32                          unit,
    rtk_port_t                      port,
    rtk_port_rldpNormalStatus_t     *pNormalStatus,
    rtk_port_rldpSelfStatus_t       *pSelfStatus)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_RLDPSTATUS_GET, &config, rtdrv_portRldpCfg_t, 1);
    *pNormalStatus = config.normalStatus;
    *pSelfStatus = config.selfStatus;

    return RT_ERR_OK;
} /* end of rtk_port_rldpStatus_get */


int32 rtk_port_rldpAutoBlockEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_RLDPAUTOBLOCKENABLE_GET, &config, rtdrv_portRldpCfg_t, 1);
    *pEnable = config.enable;

    return RT_ERR_OK;
} /* end of rtk_port_rldpAutoBlockEnable_get */


int32 rtk_port_rldpAutoBlockEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    config.enable = enable;
    SETSOCKOPT(RTDRV_PORT_RLDPAUTOBLOCKENABLE_SET, &config, rtdrv_portRldpCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_rldpAutoBlockEnable_set */


int32 rtk_port_rldpInterval_get(uint32 unit, rtk_port_t port, uint32 *pInterval)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_RLDPINTERVAL_GET, &config, rtdrv_portRldpCfg_t, 1);
    *pInterval = config.interval;

    return RT_ERR_OK;
} /* end of rtk_port_rldpInterval_get */


int32 rtk_port_rldpInterval_set(uint32 unit, rtk_port_t port, uint32 interval)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    config.interval = interval;
    SETSOCKOPT(RTDRV_PORT_RLDPINTERVAL_SET, &config, rtdrv_portRldpCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_rldpInterval_set */


int32 rtk_port_rldpSelfLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_RLDPSELFLOOPAGINGTIME_GET, &config, rtdrv_portRldpCfg_t, 1);
    *pAgingTime = config.agingTime;

    return RT_ERR_OK;
} /* end of rtk_port_rldpSelfLoopAgingTime_get */


int32 rtk_port_rldpSelfLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    config.agingTime = agingTime;
    SETSOCKOPT(RTDRV_PORT_RLDPSELFLOOPAGINGTIME_SET, &config, rtdrv_portRldpCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_rldpSelfLoopAgingTime_set */


int32 rtk_port_rldpNormalLoopAgingTime_get(uint32 unit, rtk_port_t port, uint32 *pAgingTime)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_RLDPNORMALLOOPAGINGTIME_GET, &config, rtdrv_portRldpCfg_t, 1);
    *pAgingTime = config.agingTime;

    return RT_ERR_OK;
} /* end of rtk_port_rldpNormalLoopAgingTime_get */


int32 rtk_port_rldpNormalLoopAgingTime_set(uint32 unit, rtk_port_t port, uint32 agingTime)
{
    rtdrv_portRldpCfg_t config;

    config.unit = unit;
    config.port = port;
    config.agingTime = agingTime;
    SETSOCKOPT(RTDRV_PORT_RLDPNORMALLOOPAGINGTIME_SET, &config, rtdrv_portRldpCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_rldpNormalLoopAgingTime_set */

int32
rtk_port_phyCrossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
    rtdrv_portCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_PHY_CROSSOVERMODE_GET, &config, rtdrv_portCfg_t, 1);
    *pMode = config.data;

    return RT_ERR_OK;
} /* end of rtk_port_phyCrossOverMode_get */

int32
rtk_port_phyCrossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
    rtdrv_portCfg_t config;

    config.unit = unit;
    config.port = port;
    config.data = mode;
    SETSOCKOPT(RTDRV_PORT_PHY_CROSSOVERMODE_SET, &config, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_phyCrossOverMode_set */

int32
rtk_port_phyCrossOverStatus_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_status_t *pStatus)
{
    rtdrv_portCfg_t config;

    config.unit = unit;
    config.port = port;
    GETSOCKOPT(RTDRV_PORT_PHY_CROSSOVERSTATUS_GET, &config, rtdrv_portCfg_t, 1);
    *pStatus = config.data;

    return RT_ERR_OK;
} /* end of rtk_port_phyCrossOverStatus_get */


int32 rtk_port_phyComboPortFiberMedia_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_PHY_FIBER_MEDIA_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pMedia = port_cfg.fiber_media;

    return RT_ERR_OK;
} /* end of rtk_port_phyComboPortFiberMedia_get */

int32 rtk_port_phyComboPortFiberMedia_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.fiber_media = media;
    SETSOCKOPT(RTDRV_PORT_PHY_FIBER_MEDIA_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_phyComboPortFiberMedia_set */

int32 rtk_port_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    GETSOCKOPT(RTDRV_PORT_LINKDOWN_POWERSAVING_ENABLE_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pEnable = port_cfg.data;

    return RT_ERR_OK;
} /* end of rtk_port_linkDownPowerSavingEnable_get */

int32 rtk_port_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.port = port;
    port_cfg.data = enable;
    SETSOCKOPT(RTDRV_PORT_LINKDOWN_POWERSAVING_ENABLE_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
} /* end of rtk_port_linkDownPowerSavingEnable_set */

int32 rtk_port_vlanBasedIsolationEntry_get(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.index = index;
    GETSOCKOPT(RTDRV_PORT_VLAN_ISOLATION_GET, &port_cfg, rtdrv_portCfg_t, 1);
    memcpy(pEntry, &(port_cfg.vlanIsoEntry), sizeof(rtk_port_vlanIsolationEntry_t));

    return RT_ERR_OK;
}

int32 rtk_port_vlanBasedIsolationEntry_set(uint32 unit, uint32 index, rtk_port_vlanIsolationEntry_t* pEntry)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.index = index;
    memcpy(&(port_cfg.vlanIsoEntry), pEntry, sizeof(rtk_port_vlanIsolationEntry_t));
    SETSOCKOPT(RTDRV_PORT_VLAN_ISOLATION_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_port_vlanBasedIsolation_vlanSource_get(uint32 unit, rtk_port_vlanIsolationSrc_t *pVlanSrc)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    GETSOCKOPT(RTDRV_PORT_VLAN_ISOLATION_VLANSOURCE_GET, &port_cfg, rtdrv_portCfg_t, 1);
    *pVlanSrc = port_cfg.vlanIsoSrc;

    return RT_ERR_OK;
}

int32 rtk_port_vlanBasedIsolation_vlanSource_set(uint32 unit, rtk_port_vlanIsolationSrc_t vlanSrc)
{
    rtdrv_portCfg_t port_cfg;

    port_cfg.unit = unit;
    port_cfg.vlanIsoSrc = vlanSrc;
    SETSOCKOPT(RTDRV_PORT_VLAN_ISOLATION_VLANSOURCE_SET, &port_cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}

int32
rtk_port_fiberDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    rtdrv_portCfg_t cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    GETSOCKOPT(RTDRV_PORT_FIBERDOWNSPEEDENABLE_GET, &cfg, rtdrv_portCfg_t, 1);
    memcpy(pEnable, &cfg.data, sizeof(rtk_enable_t));

    return RT_ERR_OK;
}   /* end of rtk_port_fiberDownSpeedEnable_get */

int32
rtk_port_fiberDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    memcpy(&cfg.data, &enable, sizeof(rtk_enable_t));
    SETSOCKOPT(RTDRV_PORT_FIBERDOWNSPEEDENABLE_SET, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_fiberDownSpeedEnable_set */

int32
rtk_port_downSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    rtdrv_portCfg_t cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    GETSOCKOPT(RTDRV_PORT_DOWNSPEEDENABLE_GET, &cfg, rtdrv_portCfg_t, 1);
    memcpy(pEnable, &cfg.data, sizeof(rtk_enable_t));

    return RT_ERR_OK;
}   /* end of rtk_port_fiberDownSpeedEnable_get */

int32
rtk_port_downSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    memcpy(&cfg.data, &enable, sizeof(rtk_enable_t));
    SETSOCKOPT(RTDRV_PORT_DOWNSPEEDENABLE_SET, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_fiberDownSpeedEnable_set */


int32
rtk_port_fiberNwayForceLinkEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    rtdrv_portCfg_t cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    GETSOCKOPT(RTDRV_PORT_FIBERNWAYFORCELINKENABLE_GET, &cfg, rtdrv_portCfg_t, 1);
    memcpy(pEnable, &cfg.data, sizeof(rtk_enable_t));

    return RT_ERR_OK;
}   /* end of rtk_port_fiberNwayForceLinkEnable_get */

int32
rtk_port_fiberNwayForceLinkEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    memcpy(&cfg.data, &enable, sizeof(rtk_enable_t));
    SETSOCKOPT(RTDRV_PORT_FIBERNWAYFORCELINKENABLE_SET, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_fiberNwayForceLinkEnable_set */



int32
rtk_port_fiberInternalLoopBackEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    memcpy(&cfg.data, &enable, sizeof(rtk_enable_t));
    SETSOCKOPT(RTDRV_PORT_FIBERINTERNALLOOPBACKENABLE_SET, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_fiberOAMLoopBackEnable_set */

int32
rtk_port_fiberOAMLoopBackEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    memcpy(&cfg.data, &enable, sizeof(rtk_enable_t));
    SETSOCKOPT(RTDRV_PORT_FIBEROAMLOOPBACKENABLE_SET, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_fiberOAMLoopBackEnable_set */

/* Function Name:
 *      rtk_port_10gMedia_set
 * Description:
 *      Set 10G port media of the specific port
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - port media
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) The media value is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
int32
rtk_port_10gMedia_set(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t media)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    memcpy(&cfg.media_10g, &media, sizeof(rtk_port_10gMedia_t));
    SETSOCKOPT(RTDRV_PORT_10GMEDIA_SET, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_10gMedia_set */

/* Function Name:
 *      rtk_port_10gMedia_get
 * Description:
 *      Get 10G port media of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      media   - pointer to the media type
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      (1) The media type of the port is as following:
 *          - PORT_10GMEDIA_FIBER_10G,
 *          - PORT_10GMEDIA_FIBER_1G,
 *          - PORT_10GMEDIA_DAC_50CM,
 *          - PORT_10GMEDIA_DAC_100CM,
 *          - PORT_10GMEDIA_DAC_300CM,
 */
int32
rtk_port_10gMedia_get(uint32 unit, rtk_port_t port, rtk_port_10gMedia_t *media)
{
    rtdrv_portCfg_t cfg;

    /* parameter check */
    RT_PARAM_CHK((NULL == media), RT_ERR_NULL_POINTER);

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    GETSOCKOPT(RTDRV_PORT_10GMEDIA_GET, &cfg, rtdrv_portCfg_t, 1);
    memcpy(media, &cfg.media_10g, sizeof(rtk_port_10gMedia_t));

    return RT_ERR_OK;
}   /* end of rtk_port_10gMedia_get */

int32
rtk_port_10gSds_restart(uint32 unit, rtk_port_t port)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    SETSOCKOPT(RTDRV_PORT_10GSDS_RESTART, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_10gSds_restart */

int32
rtk_port_10g_init(uint32 unit, rtk_port_t port)
{
    rtdrv_portCfg_t cfg;

    /* function body */
    memcpy(&cfg.unit, &unit, sizeof(uint32));
    memcpy(&cfg.port, &port, sizeof(rtk_port_t));
    SETSOCKOPT(RTDRV_PORT_10G_INIT, &cfg, rtdrv_portCfg_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_port_10g_init */
