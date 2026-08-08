/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 * 
 * $Revision: 37702 $
 * $Date: 2013-03-12 19:17:38 +0800 (Tue, 12 Mar 2013) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) switch
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <drv/watchdog/watchdog.h>
#include <rtk/switch.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_switch_cpuMaxPktLen_get(uint32 unit, rtk_switch_pktDir_t dir, uint32 *pLen)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.dir = dir;
    GETSOCKOPT(RTDRV_SWITCH_CPU_MAX_PKTLEN_GET, &switch_cfg, rtdrv_switchCfg_t, 1);
    *pLen = switch_cfg.len;

    return RT_ERR_OK;
}

int32 rtk_switch_cpuMaxPktLen_set(uint32 unit, rtk_switch_pktDir_t dir, uint32 len)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.dir = dir;
    switch_cfg.len = len;
    SETSOCKOPT(RTDRV_SWITCH_CPU_MAX_PKTLEN_SET, &switch_cfg, rtdrv_switchCfg_t, 1);    

    return RT_ERR_OK;    
}

int32 rtk_switch_maxPktLen_get(uint32 unit, rtk_switch_maxPktLen_t *pLen)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_MAX_PKTLEN_GET, &switch_cfg, rtdrv_switchCfg_t, 1);    
    *pLen = switch_cfg.len;
    return RT_ERR_OK;    
}

int32 rtk_switch_maxPktLen_set(uint32 unit, rtk_switch_maxPktLen_t len)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.len = len;   
    SETSOCKOPT(RTDRV_SWITCH_MAX_PKTLEN_SET, &switch_cfg, rtdrv_switchCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32
rtk_switch_maxPktLenLinkSpeed_get(uint32 unit, rtk_switch_maxPktLen_linkSpeed_t speed, uint32 *pLen)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.speed = speed;
    GETSOCKOPT(RTDRV_SWITCH_MAX_PKTLEN_LINK_SPEED_GET, &switch_cfg, rtdrv_switchCfg_t, 1);    
    *pLen = switch_cfg.maxLen;
    return RT_ERR_OK;    
}

int32
rtk_switch_maxPktLenLinkSpeed_set(uint32 unit, rtk_switch_maxPktLen_linkSpeed_t speed, uint32 len)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.speed = speed;
    switch_cfg.maxLen = len;   
    SETSOCKOPT(RTDRV_SWITCH_MAX_PKTLEN_LINK_SPEED_SET, &switch_cfg, rtdrv_switchCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_switch_maxPktLenTagLenCntIncEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_MAX_PKTLEN_TAGLENCNT_GET, &switch_cfg, rtdrv_switchCfg_t, 1);    
    *pEnable = switch_cfg.enable;
    return RT_ERR_OK;    
}

int32 rtk_switch_maxPktLenTagLenCntIncEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.enable = enable;   
    SETSOCKOPT(RTDRV_SWITCH_MAX_PKTLEN_TAGLENCNT_SET, &switch_cfg, rtdrv_switchCfg_t, 1);    
         
    return RT_ERR_OK;    
}

int32 rtk_switch_deviceInfo_get(uint32 unit, rtk_switch_devInfo_t *pDevInfo)
{
    rtdrv_switchCfg_t switch_cfg;

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_DEVICE_INFO_GET, &switch_cfg, rtdrv_switchCfg_t, 1);    
    memcpy(pDevInfo, &switch_cfg.devInfo, sizeof(rtk_switch_devInfo_t));
    return RT_ERR_OK;    
}

int32 rtk_switch_portMaxPktLen_get(uint32 unit, rtk_port_t port, uint32 *pLength)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    GETSOCKOPT(RTDRV_SWITCH_PORTMAXPKTLEN_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pLength = switch_cfg.maxLen;
    
    return RT_ERR_OK;    
}/* end of rtk_switch_portMaxPktLen_get */


int32 rtk_switch_portMaxPktLen_set(uint32 unit, rtk_port_t port, uint32 length)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    switch_cfg.maxLen = length;
    SETSOCKOPT(RTDRV_SWITCH_PORTMAXPKTLEN_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;    
} /* end of rtk_switch_portMaxPktLen_set */


int32 rtk_switch_portSnapMode_get(uint32 unit, rtk_port_t port, rtk_snapMode_t *pSnapMode)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    GETSOCKOPT(RTDRV_SWITCH_PORTSNAPMODE_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pSnapMode = switch_cfg.snapMode;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_portSnapMode_get */


int32 rtk_switch_portSnapMode_set(uint32 unit, rtk_port_t port, rtk_snapMode_t snapMode)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    switch_cfg.snapMode = snapMode;
    
    SETSOCKOPT(RTDRV_SWITCH_PORTSNAPMODE_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;    
} /* end of rtk_switch_portSnapMode_set */


int32 rtk_switch_chksumFailAction_get(
    uint32                              unit, 
    rtk_port_t                          port, 
    rtk_switch_chksum_fail_t            failType, 
    rtk_action_t                        *pAction)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    switch_cfg.failType = failType;
    GETSOCKOPT(RTDRV_SWITCH_CHKSUMFAILACTION_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pAction = switch_cfg.action;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_chksumFailAction_get */


int32 rtk_switch_chksumFailAction_set(
    uint32                              unit, 
    rtk_port_t                          port, 
    rtk_switch_chksum_fail_t            failType, 
    rtk_action_t                        action)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    switch_cfg.failType = failType;
    switch_cfg.action = action;
    SETSOCKOPT(RTDRV_SWITCH_CHKSUMFAILACTION_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;    
} /* end of rtk_switch_chksumFailAction_set */


int32 rtk_switch_recalcCRCEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    GETSOCKOPT(RTDRV_SWITCH_RECALCCRCENABLE_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pEnable = switch_cfg.enable;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_recalcCRCEnable_get */


int32 rtk_switch_recalcCRCEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.port = port;
    switch_cfg.enable = enable;
    SETSOCKOPT(RTDRV_SWITCH_RECALCCRCENABLE_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;    
} /* end of rtk_switch_recalcCRCEnable_set */



int32 rtk_switch_mgmtVlanId_get(uint32 unit, rtk_vlan_t *pVid)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_MGMTVLANID_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pVid = switch_cfg.mgmIvid;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_mgmtVlanId_get */


int32 rtk_switch_mgmtVlanId_set(uint32 unit, rtk_vlan_t vid)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.mgmIvid = vid;
    SETSOCKOPT(RTDRV_SWITCH_MGMTVLANID_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;    
} /* end of rtk_switch_mgmtVlanId_set */


int32 rtk_switch_outerMgmtVlanId_get(uint32 unit, rtk_vlan_t *pOuterVid)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_OUTERMGMTVLANID_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pOuterVid = switch_cfg.mgmOvid;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_outerMgmtVlanId_get */


int32 rtk_switch_outerMgmtVlanId_set(uint32 unit, rtk_vlan_t outerVid)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.mgmOvid = outerVid;
    SETSOCKOPT(RTDRV_SWITCH_OUTERMGMTVLANID_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;    
}  /* end of rtk_switch_outerMgmtVlanId_set */


int32 rtk_switch_mgmtMacAddr_get(uint32 unit, rtk_mac_t *pMac)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_MGMTMACADDR_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);    
    memcpy(pMac, &switch_info.mac, sizeof(rtk_mac_t));
    
    return RT_ERR_OK;    
} /* end of rtk_switch_mgmtMacAddr_get */


int32 rtk_switch_mgmtMacAddr_set(uint32 unit, rtk_mac_t *pMac)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    memcpy( &switch_info.mac, pMac, sizeof(rtk_mac_t));
    SETSOCKOPT(RTDRV_SWITCH_MGMTMACADDR_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);        
    
    return RT_ERR_OK;    
}  /* end of rtk_switch_mgmtMacAddr_set */


int32 rtk_switch_IPv4Addr_get(uint32 unit, uint32 *pIpAddr)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_IPV4ADDR_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);    
    *pIpAddr = switch_info.ipv4Addr;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_IPv4Addr_get */


int32 rtk_switch_IPv4Addr_set(uint32 unit, uint32 ipAddr)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.ipv4Addr = ipAddr;
    SETSOCKOPT(RTDRV_SWITCH_IPV4ADDR_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);    
    
    return RT_ERR_OK;    
} /* end of rtk_switch_IPv4Addr_set */


int32 rtk_switch_IPv6Addr_get(uint32 unit, rtk_ipv6_addr_t *pIpv6Addr)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_IPV6ADDR_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);    
    memcpy(pIpv6Addr, &switch_info.ipv6Addr, sizeof(rtk_ipv6_addr_t));
    
    return RT_ERR_OK;    
} /* end of rtk_switch_IPv6Addr_get */


int32 rtk_switch_IPv6Addr_set(uint32 unit, rtk_ipv6_addr_t ipv6Addr)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    memcpy(&switch_info.ipv6Addr, &ipv6Addr, sizeof(rtk_ipv6_addr_t));
    SETSOCKOPT(RTDRV_SWITCH_IPV6ADDR_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);       
    
    return RT_ERR_OK;    
}  /* end of rtk_switch_IPv6Addr_set */

int32
rtk_switch_hwInterfaceDelayEnable_get(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t *pEnable)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.type = type;
    GETSOCKOPT(RTDRV_SWITCH_DELAY_ENABLE_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    *pEnable = switch_info.enable;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_hwInterfaceDelayEnable_get */

int32
rtk_switch_hwInterfaceDelayEnable_set(uint32 unit, rtk_switch_delayType_t type, rtk_enable_t enable)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.type = type;
    switch_info.enable = enable;
    SETSOCKOPT(RTDRV_SWITCH_DELAY_ENABLE_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);       
    
    return RT_ERR_OK;    
} /* end of rtk_switch_hwInterfaceDelayEnable_set */

int32
rtk_switch_pkt2CpuFormat_get(uint32 unit, rtk_pktFormat_t *pFormat)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_PKT2CPU_FORMAT_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    *pFormat = switch_info.data;

    return RT_ERR_OK;
} /* end of rtk_switch_pkt2CpuFormat_get */

int32
rtk_switch_pkt2CpuFormat_set(uint32 unit, rtk_pktFormat_t format)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.data = format;
    SETSOCKOPT(RTDRV_SWITCH_PKT2CPU_FORMAT_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);       
    
    return RT_ERR_OK;    
} /* end of rtk_switch_pkt2CpuFormat_set */

int32 
rtk_switch_softwareResetCounter_get(uint32 unit, uint32 *pCounter)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_SOFTWARE_RESET_COUNTER_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    *pCounter = switch_info.data;

    return RT_ERR_OK;
} /* end of rtk_switch_softwareResetCounter_get */

int32
rtk_switch_pkt2CpuTypeFormat_get(uint32 unit, rtk_switch_pkt2CpuType_t type,
    rtk_pktFormat_t *pFormat)
{
    rtdrv_switchCfgInfo_t cfg;

    cfg.unit = unit;
    cfg.type = type;
    GETSOCKOPT(RTDRV_SWITCH_PKT2CPUTYPEFORMAT_GET, &cfg, rtdrv_switchCfgInfo_t, 1);

    *pFormat = cfg.format;

    return RT_ERR_OK;
}   /* end of rtk_switch_pkt2CpuTypeFormat_get */

int32
rtk_switch_pkt2CpuTypeFormat_set(uint32 unit, rtk_switch_pkt2CpuType_t type,
    rtk_pktFormat_t format)
{
    rtdrv_switchCfgInfo_t cfg;

    cfg.unit = unit;
    cfg.type = type;
    cfg.format = format;
    SETSOCKOPT(RTDRV_SWITCH_PKT2CPUTYPEFORMAT_SET, &cfg, rtdrv_switchCfgInfo_t, 1);

    return RT_ERR_OK;
}   /* end of rtk_switch_pkt2CpuTypeFormat_set */

int32 rtk_switch_pppoePassthrough_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_PPPOE_PASS_THROUGH_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    *pEnable = switch_info.enable;
    
    return RT_ERR_OK;    
} /* end of rtk_switch_pppoePassthrough_get */

int32 rtk_switch_pppoePassthrough_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.enable = enable;
    SETSOCKOPT(RTDRV_SWITCH_PPPOE_PASS_THROUGH_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);       
    
    return RT_ERR_OK;    
} /* end of rtk_switch_pppoePassthrough_set */

int32 rtk_switch_cpuPktTruncateEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_CPU_PKT_TRUNCATE_EN_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pEnable = switch_cfg.enable;

    return RT_ERR_OK;
}

int32 rtk_switch_cpuPktTruncateEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.enable = enable;
    SETSOCKOPT(RTDRV_SWITCH_CPU_PKT_TRUNCATE_EN_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;
}

int32 rtk_switch_cpuPktTruncateLen_get(uint32 unit, uint32 *pLen)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_CPU_PKT_TRUNCATE_LEN_GET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    *pLen = switch_cfg.maxLen;

    return RT_ERR_OK;
}

int32 rtk_switch_cpuPktTruncateLen_set(uint32 unit, uint32 len)
{
    rtdrv_switchCfgParam_t switch_cfg;

    switch_cfg.unit = unit;
    switch_cfg.maxLen = len;
    SETSOCKOPT(RTDRV_SWITCH_CPU_PKT_TRUNCATE_LEN_SET, &switch_cfg, rtdrv_switchCfgParam_t, 1);    
    
    return RT_ERR_OK;
}

int32 
drv_watchdog_enable_get(uint32 unit, uint32 *pEnable)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_WATCHDOG_ENABLE_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    *pEnable = switch_info.enable;

    return RT_ERR_OK;    
} /* end of drv_watchdog_enable_get */

int32 
drv_watchdog_enable_set(uint32 unit, uint32 enable)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.enable = enable;
    SETSOCKOPT(RTDRV_SWITCH_WATCHDOG_ENABLE_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);       
    
    return RT_ERR_OK;    
} /* end of drv_watchdog_enable_set */

int32 
drv_watchdog_mode_get(uint32 unit, drv_watchdog_mode_t *pMode)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_WATCHDOG_MODE_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    *pMode = switch_info.data;

    return RT_ERR_OK;
} /* end of drv_watchdog_mode_get */

int32 
drv_watchdog_mode_set(uint32 unit, drv_watchdog_mode_t mode)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.data = mode;
    SETSOCKOPT(RTDRV_SWITCH_WATCHDOG_MODE_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    
    return RT_ERR_OK;
} /* end of drv_watchdog_mode_set */

int32 
drv_watchdog_scale_get(uint32 unit, drv_watchdog_scale_t *pScale)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    GETSOCKOPT(RTDRV_SWITCH_WATCHDOG_SCALE_GET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    *pScale = switch_info.data;

    return RT_ERR_OK;
} /* end of drv_watchdog_scale_get */

int32 
drv_watchdog_scale_set(uint32 unit, drv_watchdog_scale_t scale)
{
    rtdrv_switchCfgInfo_t switch_info;

    switch_info.unit = unit;
    switch_info.data = scale;
    SETSOCKOPT(RTDRV_SWITCH_WATCHDOG_SCALE_SET, &switch_info, rtdrv_switchCfgInfo_t, 1);
    
    return RT_ERR_OK;
} /* end of drv_watchdog_scale_set */
