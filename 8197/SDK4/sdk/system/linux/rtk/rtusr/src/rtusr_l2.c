/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 * 
 * $Revision: 32986 $
 * $Date: 2012-09-26 17:15:52 +0800 (Wed, 26 Sep 2012) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) l2 address
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_l2_addr_get(uint32 unit, rtk_l2_ucastAddr_t *pL2_data)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    memcpy(&l2_data.data, pL2_data, sizeof(rtk_l2_ucastAddr_t));
    GETSOCKOPT(RTDRV_L2_ADDR_GET, &l2_data, rtdrv_l2_addrData_t, 1);
    memcpy(pL2_data, &l2_data.data, sizeof(rtk_l2_ucastAddr_t));
    
    return RT_ERR_OK;    
}

int32 rtk_l2_nextValidAddr_get(uint32 unit, int32 *pScan_idx, uint32 include_static, rtk_l2_ucastAddr_t *pL2_data)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    l2_data.index = *pScan_idx;
    l2_data.static_flag = include_static;    
    memcpy(&l2_data.data, pL2_data, sizeof(rtk_l2_ucastAddr_t));
    GETSOCKOPT(RTDRV_L2_ADDR_GETNEXT, &l2_data, rtdrv_l2_addrData_t, 1);
    *pScan_idx = l2_data.index;
    memcpy(pL2_data, &l2_data.data, sizeof(rtk_l2_ucastAddr_t));
    
    return RT_ERR_OK;    
}

int32 rtk_l2_addr_delAll(uint32 unit, uint32 include_static)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    l2_data.static_flag = include_static;
    SETSOCKOPT(RTDRV_L2_ADDR_DELALL, &l2_data, rtdrv_l2_addrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_addr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    l2_data.vid = vid;
    memcpy(&l2_data.mac, pMac, sizeof(rtk_mac_t));
    SETSOCKOPT(RTDRV_L2_ADDR_DEL, &l2_data, rtdrv_l2_addrData_t, 1);
     
    return RT_ERR_OK;    
}

int32 rtk_l2_addr_add(uint32 unit, rtk_l2_ucastAddr_t *pL2_addr)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    memcpy(&l2_data.data, pL2_addr, sizeof(rtk_l2_ucastAddr_t));
    SETSOCKOPT(RTDRV_L2_ADDR_ADD, &l2_data, rtdrv_l2_addrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_aging_get(uint32 unit, uint32 *pAging_time)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;    
    GETSOCKOPT(RTDRV_L2_AGING_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    *pAging_time = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_aging_set(uint32 unit, uint32 aging_time)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    unit_cfg.data = aging_time;
    SETSOCKOPT(RTDRV_L2_AGING_SET, &unit_cfg, rtdrv_unitCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portAgingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_l2_ageCfg_t age_cfg;

    age_cfg.unit = unit;
    age_cfg.port = port;
    GETSOCKOPT(RTDRV_L2_PORT_AGING_ENABLE_GET, &age_cfg, rtdrv_l2_ageCfg_t, 1);
    *pEnable = age_cfg.enable;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portAgingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_l2_ageCfg_t age_cfg;

    age_cfg.unit = unit;
    age_cfg.port = port;
    age_cfg.enable = enable;
    SETSOCKOPT(RTDRV_L2_PORT_AGING_ENABLE_SET, &age_cfg, rtdrv_l2_ageCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_ucastAddr_flush(uint32 unit, rtk_l2_flushCfg_t *pConfig)
{
    rtdrv_flushType_t flush_type;

    flush_type.unit = unit;
    memcpy(&flush_type.config, pConfig, sizeof(rtk_l2_flushCfg_t));
    SETSOCKOPT(RTDRV_L2_UCASTADDR_FLUSH, &flush_type, rtdrv_flushType_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ucastAddr_flush */

int32 rtk_l2_flushLinkDownPortAddrEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    GETSOCKOPT(RTDRV_L2_EN_FLUSH_LINK_DOWN_PORT_GET, &unit_cfg, rtdrv_unitCfg_t, 1);
    *pEnable = unit_cfg.data;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_flushLinkDownPortAddrEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_unitCfg_t unit_cfg;

    unit_cfg.unit = unit;
    unit_cfg.data = enable;
    SETSOCKOPT(RTDRV_L2_EN_FLUSH_LINK_DOWN_PORT_SET, &unit_cfg, rtdrv_unitCfg_t, 1);

    return RT_ERR_OK;    
}

int32 rtk_l2_ipMcastAddr_get(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;
    memcpy(&mcast_data.ip_m_data, pIpmcast_addr, sizeof(rtk_l2_ipMcastAddr_t));
    GETSOCKOPT(RTDRV_L2_IP_MCAST_ADDR_GET, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    memcpy(pIpmcast_addr, &mcast_data.ip_m_data, sizeof(rtk_l2_ipMcastAddr_t)); 

    return RT_ERR_OK;
}

int32 rtk_l2_nextValidIpMcastAddr_get(uint32 unit, int32 *pScan_idx, rtk_l2_ipMcastAddr_t *pL2_data)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;   
    mcast_data.index = *pScan_idx;
    memcpy(&mcast_data.ip_m_data, pL2_data, sizeof(rtk_l2_ipMcastAddr_t));
    GETSOCKOPT(RTDRV_L2_IP_MCAST_ADDR_GETNEXT, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    *pScan_idx = mcast_data.index;
    memcpy(pL2_data, &mcast_data.ip_m_data, sizeof(rtk_l2_ipMcastAddr_t));

    return RT_ERR_OK;
}

int32 rtk_l2_ipMcastAddr_add(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;
    memcpy(&mcast_data.ip_m_data, pIpmcast_addr, sizeof(rtk_l2_ipMcastAddr_t));
    SETSOCKOPT(RTDRV_L2_IP_MCAST_ADDR_ADD, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_ipMcastAddr_del(uint32 unit, ipaddr_t sip, ipaddr_t dip, rtk_vlan_t vid)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;
    mcast_data.ip_m_data.sip = sip;
    mcast_data.ip_m_data.dip = dip;
    mcast_data.ip_m_data.rvid = vid;       
    SETSOCKOPT(RTDRV_L2_IP_MCAST_ADDR_DEL, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_ipMcastAddr_set(uint32 unit, rtk_l2_ipMcastAddr_t *pIpmcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;
    memcpy(&mcast_data.ip_m_data, pIpmcast_addr, sizeof(rtk_l2_ipMcastAddr_t));
    SETSOCKOPT(RTDRV_L2_IP_MCAST_ADDR_SET, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_ipMcastAddr_add_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;
    memcpy(&mcast_data.ip_m_data, pIpMcast_addr, sizeof(rtk_l2_ipMcastAddr_t));
    SETSOCKOPT(RTDRV_L2_IPMCASTADDR_ADD_WITH_INDEX, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;
}

int32 rtk_l2_ipMcastAddr_get_with_index(uint32 unit, rtk_l2_ipMcastAddr_t *pIpMcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;   
    memcpy(&mcast_data.ip_m_data, pIpMcast_addr, sizeof(rtk_l2_ipMcastAddr_t));
    GETSOCKOPT(RTDRV_L2_IPMCASTADDR_GET_WITH_INDEX, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    memcpy(pIpMcast_addr, &mcast_data.ip_m_data, sizeof(rtk_l2_ipMcastAddr_t));

    return RT_ERR_OK;
}

int32 rtk_l2_ip6mcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode)
{
    rtdrv_unitCfg_t cfg;

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_L2_IP6MCASTMODE_GET, &cfg, rtdrv_unitCfg_t, 1);
    *pMode = cfg.data;

    return RT_ERR_OK;
}

int32 rtk_l2_ip6mcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode)
{
    rtdrv_unitCfg_t cfg;

    cfg.unit = unit;
    cfg.data = mode;
    SETSOCKOPT(RTDRV_L2_IP6MCASTMODE_SET, &cfg, rtdrv_unitCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_ip6McastAddr_del(uint32 unit, rtk_ipv6_addr_t sip, rtk_ipv6_addr_t dip, rtk_vlan_t vid)
{
    rtdrv_l2_ip6McstAddrData_t ip6Mcast_data;

    ip6Mcast_data.unit = unit;
    ip6Mcast_data.ip6_m_data.sip = sip;
    ip6Mcast_data.ip6_m_data.dip = dip;
    ip6Mcast_data.ip6_m_data.rvid = vid;
    SETSOCKOPT(RTDRV_L2_IP6_MCAST_ADDR_DEL, &ip6Mcast_data, rtdrv_l2_ip6McstAddrData_t, 1);
    
    return RT_ERR_OK;
}

int32 rtk_l2_mcastAddr_get(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;   
    memcpy(&mcast_data.m_data, pMcast_addr, sizeof(rtk_l2_mcastAddr_t));
    GETSOCKOPT(RTDRV_L2_MCAST_ADDR_GET, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    memcpy(pMcast_addr, &mcast_data.m_data, sizeof(rtk_l2_mcastAddr_t));
    
    return RT_ERR_OK;    
}

int32 rtk_l2_nextValidMcastAddr_get(uint32 unit, int32 *pScan_idx, rtk_l2_mcastAddr_t *pL2_data)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;   
    mcast_data.index = *pScan_idx;
    memcpy(&mcast_data.m_data, pL2_data, sizeof(rtk_l2_mcastAddr_t));
    GETSOCKOPT(RTDRV_L2_MCAST_ADDR_GETNEXT, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    *pScan_idx = mcast_data.index;
    memcpy(pL2_data, &mcast_data.m_data, sizeof(rtk_l2_mcastAddr_t));

    return RT_ERR_OK;    
}

int32 rtk_l2_mcastAddr_add(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;   
    memcpy(&mcast_data.m_data, pMcast_addr, sizeof(rtk_l2_mcastAddr_t));
    SETSOCKOPT(RTDRV_L2_MCAST_ADDR_ADD, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_mcastAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;   
    mcast_data.m_data.rvid = vid;    
    memcpy(&mcast_data.m_data.mac, pMac, sizeof(rtk_mac_t));   
    SETSOCKOPT(RTDRV_L2_MCAST_ADDR_DEL, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;      
}

int32 rtk_l2_mcastAddr_add_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;
    
    mcast_data.unit = unit;
    memcpy(&(mcast_data.m_data), pMcast_addr, sizeof(rtk_l2_mcastAddr_t));
    SETSOCKOPT(RTDRV_L2_MCASTADDR_ADD_WITH_INDEX, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;
}

int32 rtk_l2_mcastAddr_get_with_index(uint32 unit, rtk_l2_mcastAddr_t *pMcast_addr)
{
    rtdrv_l2_mcastAddrData_t mcast_data;

    mcast_data.unit = unit;   
    memcpy(&(mcast_data.m_data), pMcast_addr, sizeof(rtk_l2_mcastAddr_t));
    GETSOCKOPT(RTDRV_L2_MCASTADDR_GET_WITH_INDEX, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    memcpy(pMcast_addr, &(mcast_data.m_data), sizeof(rtk_l2_mcastAddr_t));

    return RT_ERR_OK;
}

int32
rtk_l2_mcastFwdIndex_alloc(
    uint32          unit,
    int32           *pFwdIndex)
{
    rtdrv_l2_mcastAddrData_t mcast_data;
    
    mcast_data.unit = unit;
    mcast_data.fwdIndex = *pFwdIndex;
    GETSOCKOPT(RTDRV_L2_MCASTFWDINDEX_ALLOC, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    memcpy(pFwdIndex, &(mcast_data.fwdIndex), sizeof(int32));
    
    return RT_ERR_OK;
}

int32
rtk_l2_mcastFwdIndex_free(
    uint32          unit,
    int32           index)
{
    rtdrv_l2_mcastAddrData_t mcast_data;
    
    mcast_data.unit = unit;
    mcast_data.fwdIndex = index;
    SETSOCKOPT(RTDRV_L2_MCASTFWDINDEX_FREE, &mcast_data, rtdrv_l2_mcastAddrData_t, 1);
    
    return RT_ERR_OK;
}

int32
rtk_l2_mcastFwdIndexFreeCount_get(uint32 unit, uint32 *pFreeCount)
{
    rtdrv_l2_fwdTblEntry_t  entryContent;

    memset(&entryContent, 0, sizeof(rtdrv_l2_fwdTblEntry_t));
    
    entryContent.unit = unit;
    GETSOCKOPT(RTDRV_L2_MCASTFWDINDEXFREECOUNT_GET, &entryContent, rtdrv_l2_fwdTblEntry_t, 1);
    *pFreeCount = entryContent.freeCount;

    return RT_ERR_OK;
}

int32
rtk_l2_mcastFwdPortmask_get(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          *pCrossVlan)
{
    rtdrv_l2_fwdTblEntry_t  entryContent;

    memset(&entryContent, 0, sizeof(rtdrv_l2_fwdTblEntry_t));
    
    entryContent.unit = unit;
    entryContent.entryIdx = index;
    GETSOCKOPT(RTDRV_L2_MCASTFWDPORTMASK_GET, &entryContent, rtdrv_l2_fwdTblEntry_t, 1);
    memcpy(pPortmask, &entryContent.portMask, sizeof(rtk_portmask_t));
    *pCrossVlan = entryContent.crossVlan;

    return RT_ERR_OK;
} /* end of rtk_l2_mcastFwdPortmask_get */

int32
rtk_l2_mcastFwdPortmask_set(
    uint32          unit,
    int32           index,
    rtk_portmask_t  *pPortmask,
    uint32          crossVlan
)
{
    rtdrv_l2_fwdTblEntry_t  entryContent;
    
    memset(&entryContent, 0, sizeof(rtdrv_l2_fwdTblEntry_t));
    
    entryContent.unit = unit;
    entryContent.entryIdx = index;
    entryContent.crossVlan = crossVlan;
    memcpy(&entryContent.portMask, pPortmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_L2_MCASTFWDPORTMASK_SET, &entryContent, rtdrv_l2_fwdTblEntry_t, 1);    
    
    return RT_ERR_OK;
} /* end of rtk_l2_mcastFwdPortmask_set */

int32 rtk_l2_cpuMacAddr_add(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;   
    l2_data.vid = vid;    
    memcpy(&l2_data.mac, pMac, sizeof(rtk_mac_t));   
    SETSOCKOPT(RTDRV_L2_CPU_MAC_ADDR_ADD, &l2_data, rtdrv_l2_addrData_t, 1);
    
    return RT_ERR_OK;      
}

int32 rtk_l2_cpuMacAddr_del(uint32 unit, rtk_vlan_t vid, rtk_mac_t *pMac)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;   
    l2_data.vid = vid;    
    memcpy(&l2_data.mac, pMac, sizeof(rtk_mac_t));   
    SETSOCKOPT(RTDRV_L2_CPU_MAC_ADDR_DEL, &l2_data, rtdrv_l2_addrData_t, 1);
    
    return RT_ERR_OK;      
}

int32 rtk_l2_learningCnt_get(uint32 unit, uint32 *pMac_cnt)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LEARNING_CNT_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pMac_cnt = l2_learn.mac_cnt;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_limitLearningCnt_get(uint32 unit, uint32 *pMac_cnt)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LIMIT_LEARNING_CNT_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pMac_cnt = l2_learn.mac_cnt;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_limitLearningCnt_set(uint32 unit, uint32 mac_cnt)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.mac_cnt = mac_cnt;
    SETSOCKOPT(RTDRV_L2_LIMIT_LEARNING_CNT_SET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_limitLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LIMIT_LEARNING_CNT_ACTION_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pAction = l2_learn.action;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_limitLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.action = action;
    SETSOCKOPT(RTDRV_L2_LIMIT_LEARNING_CNT_ACTION_SET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_PORT_LEARNING_CNT_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pMac_cnt = l2_learn.mac_cnt;
    
    return RT_ERR_OK;    
}

int32
rtk_l2_portLimitLearningCntEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ENABLE_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pEnable = l2_learn.enable;
    
    return RT_ERR_OK;    
} /* end of rtk_l2_portLimitLearningCntEnable_get */

int32 rtk_l2_portLimitLearningCntEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    l2_learn.enable = enable;
    SETSOCKOPT(RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ENABLE_SET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    
    return RT_ERR_OK;    
} /* end of rtk_l2_portLimitLearningCntEnable_set */

int32 rtk_l2_portLimitLearningCnt_get(uint32 unit, rtk_port_t port, uint32 *pMac_cnt)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_PORT_LIMIT_LEARNING_CNT_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pMac_cnt = l2_learn.mac_cnt;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portLimitLearningCnt_set(uint32 unit, rtk_port_t port, uint32 mac_cnt)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    l2_learn.mac_cnt = mac_cnt;
    SETSOCKOPT(RTDRV_L2_PORT_LIMIT_LEARNING_CNT_SET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portLimitLearningCntAction_get(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t *pAction)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ACTION_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pAction = l2_learn.action;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portLimitLearningCntAction_set(uint32 unit, rtk_port_t port, rtk_l2_limitLearnCntAction_t action)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    l2_learn.action = action;
    SETSOCKOPT(RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ACTION_SET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portLastLearnedMac_get(uint32 unit, rtk_port_t port, rtk_fid_t *pFid, rtk_mac_t *pMac)
{
    rtdrv_l2_learnCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_PORTLASTLEARNEDMAC_GET, &l2_learn, rtdrv_l2_learnCnt_t, 1);
    *pFid = l2_learn.fid;
    memcpy(pMac, &l2_learn.mac, sizeof(rtk_mac_t));    
    
    return RT_ERR_OK;    
} /* end of rtk_l2_portLastLearnedMac_get */


int32 rtk_l2_fidLimitLearningEntry_get(
    uint32                    unit,
    uint32                    fid_macLimit_idx, 
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{    
    rtdrv_l2_learnFidCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.entryIdx = fid_macLimit_idx;
    GETSOCKOPT(RTDRV_L2_FIDLIMITLEARNINGENTRY_GET, &l2_learn, rtdrv_l2_learnFidCnt_t, 1);
    memcpy(pFidMacLimitEntry, &l2_learn.fidMacLimitEntry, sizeof(rtk_l2_fidMacLimitEntry_t));    
    
    return RT_ERR_OK;   
} /* end of rtk_l2_fidLimitLearningEntry_get */


int32
rtk_l2_fidLimitLearningEntry_set(
    uint32                    unit,
    uint32                    fid_macLimit_idx,
    rtk_l2_fidMacLimitEntry_t *pFidMacLimitEntry)
{    
    rtdrv_l2_learnFidCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.entryIdx = fid_macLimit_idx;
    memcpy(&l2_learn.fidMacLimitEntry, pFidMacLimitEntry, sizeof(rtk_l2_fidMacLimitEntry_t));  
    SETSOCKOPT(RTDRV_L2_FIDLIMITLEARNINGENTRY_SET, &l2_learn, rtdrv_l2_learnFidCnt_t, 1);     
    
    return RT_ERR_OK;   
} /* end of rtk_l2_fidLimitLearningEntry_set */


int32 rtk_l2_fidLearningCnt_get(uint32 unit, uint32 fid_macLimit_idx, uint32 *pNum)
{    
    rtdrv_l2_learnFidCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.entryIdx = fid_macLimit_idx;
    GETSOCKOPT(RTDRV_L2_FIDLEARNINGCNT_GET, &l2_learn, rtdrv_l2_learnFidCnt_t, 1);
    *pNum = l2_learn.mac_cnt;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_fidLearningCnt_get */


int32
rtk_l2_fidLearningCnt_reset(uint32 unit, uint32 fid_macLimit_idx)
{    
    rtdrv_l2_learnFidCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.entryIdx = fid_macLimit_idx;
    SETSOCKOPT(RTDRV_L2_FIDLEARNINGCNT_RESET, &l2_learn, rtdrv_l2_learnFidCnt_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_fidLearningCnt_reset */


int32 rtk_l2_fidLastLearnedMac_get(
    uint32      unit, 
    uint32      fid_macLimit_idx, 
    rtk_fid_t   *pFid, 
    rtk_mac_t   *pMac)
{    
    rtdrv_l2_learnFidCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.entryIdx = fid_macLimit_idx;
    GETSOCKOPT(RTDRV_L2_FIDLASTLEARNEDMAC_GET, &l2_learn, rtdrv_l2_learnFidCnt_t, 1);
    *pFid = l2_learn.fid;
    memcpy(pMac, &l2_learn.mac, sizeof(rtk_mac_t));
    
    return RT_ERR_OK;   
} /* end of rtk_l2_fidLastLearnedMac_get */

int32 rtk_l2_fidLearningCntAction_get(uint32 unit, rtk_l2_limitLearnCntAction_t *pAction)
{
    rtdrv_l2_learnFidCnt_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_FID_LIMIT_LEARNING_CNT_ACTION_GET, &l2_learn, rtdrv_l2_learnFidCnt_t, 1);
    *pAction = l2_learn.action;
    
    return RT_ERR_OK;   
}

int32 rtk_l2_fidLearningCntAction_set(uint32 unit, rtk_l2_limitLearnCntAction_t action)
{
    rtdrv_l2_learnFidCnt_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.action = action;
    SETSOCKOPT(RTDRV_L2_FID_LIMIT_LEARNING_CNT_ACTION_SET, &l2_learn, rtdrv_l2_learnFidCnt_t, 1);
    
    return RT_ERR_OK;   
}

int32 rtk_l2_limitLearningTrapPri_get(uint32 unit, rtk_pri_t *pPriority)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPPRI_GET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    *pPriority = l2_learn.priority;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapPri_get */

int32
rtk_l2_limitLearningTrapPri_set(uint32 unit, rtk_pri_t priority)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.priority = priority;
    SETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPPRI_SET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapPri_set */

int32 rtk_l2_limitLearningTrapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPPRIENABLE_GET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    *pEnable = l2_learn.enable;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapPriEnable_get */


int32 rtk_l2_limitLearningTrapPriEnable_set(uint32 unit, rtk_enable_t enable)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.enable = enable;
    SETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPPRIENABLE_SET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapPriEnable_set */


int32
rtk_l2_limitLearningTrapDP_get(uint32 unit, uint32 *pDp)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPDP_GET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    *pDp = l2_learn.dpValue;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapDP_get */


int32 rtk_l2_limitLearningTrapDP_set(uint32 unit, uint32 dp)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.dpValue = dp;
    SETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPDP_SET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapDP_set */

int32 rtk_l2_limitLearningTrapDPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPDPENABLE_GET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    *pEnable = l2_learn.enable;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapDP_get */


int32 rtk_l2_limitLearningTrapDPEnable_set(uint32 unit, rtk_enable_t enable)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.enable = enable;
    SETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPDPENABLE_SET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapDP_set */


int32 rtk_l2_limitLearningTrapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPADDCPUTAGENABLE_GET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    *pEnable = l2_learn.insertCpuTag;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapAddCPUTagEnable_get */

int32 rtk_l2_limitLearningTrapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{    
    rtdrv_l2_learnPriDp_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.insertCpuTag = enable;
    SETSOCKOPT(RTDRV_L2_LIMITLEARNINGTRAPADDCPUTAGENABLE_SET, &l2_learn, rtdrv_l2_learnPriDp_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_limitLearningTrapAddCPUTagEnable_set */


int32 rtk_l2_camEnable_get(uint32 unit, rtk_enable_t *pEnable)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_CAMENABLE_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pEnable = l2_learn.enable;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_camEnable_get */


int32 rtk_l2_camEnable_set(uint32 unit, rtk_enable_t enable)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.enable = enable;
    SETSOCKOPT(RTDRV_L2_CAMENABLE_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_camEnable_set */


int32 rtk_l2_hashAlgo_get(uint32 unit, uint32 *pHash_algo)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_HASHALGO_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pHash_algo = l2_learn.hash_algo;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_hashAlgo_get */


int32 rtk_l2_hashAlgo_set(uint32 unit, uint32 hash_algo)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.hash_algo = hash_algo;
    SETSOCKOPT(RTDRV_L2_HASHALGO_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_hashAlgo_set */

int32 rtk_l2_vlanMode_get(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t *pVlanMode)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_VLANMODE_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pVlanMode = l2_learn.vlanMode;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_vlanMode_get */


int32 rtk_l2_vlanMode_set(uint32 unit, rtk_port_t port, rtk_l2_vlanMode_t vlanMode)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    l2_learn.vlanMode = vlanMode;
    SETSOCKOPT(RTDRV_L2_VLANMODE_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_vlanMode_set */


int32 rtk_l2_learningEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_LEARNINGENABLE_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pEnable = l2_learn.enable;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_learningEnable_get */


int32 rtk_l2_learningEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    l2_learn.enable = enable;
    SETSOCKOPT(RTDRV_L2_LEARNINGENABLE_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_learningEnable_set */


int32 rtk_l2_newMacOp_get(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_l2_newMacLrnMode_t  *pLrnMode, 
    rtk_action_t            *pFwdAction)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_NEWMACOP_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pLrnMode = l2_learn.lrnMode;
    *pFwdAction = l2_learn.fwdAction;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_newMacOp_get */ 


int32 rtk_l2_newMacOp_set(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_l2_newMacLrnMode_t  lrnMode, 
    rtk_action_t            fwdAction)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    l2_learn.lrnMode = lrnMode;
    l2_learn.fwdAction = fwdAction;
    SETSOCKOPT(RTDRV_L2_NEWMACOP_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_newMacOp_set */


int32 rtk_l2_LRUEnable_get(uint32 unit, rtk_enable_t *pEnable)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_LRUENABLE_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pEnable = l2_learn.enable;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_LRUEnable_get */


int32 rtk_l2_LRUEnable_set(uint32 unit, rtk_enable_t enable)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.enable = enable;
    SETSOCKOPT(RTDRV_L2_LRUENABLE_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_LRUEnable_set */


int32 rtk_l2_ucastLookupMode_get(uint32 unit, rtk_l2_ucastLookupMode_t *pUcast_lookupMode)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_UCASTLOOKUPMODE_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pUcast_lookupMode = l2_learn.ucast_lookupMode;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_ucastLookupMode_get */


int32 rtk_l2_ucastLookupMode_set(uint32 unit, rtk_l2_ucastLookupMode_t ucast_lookupMode)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.ucast_lookupMode = ucast_lookupMode;
    SETSOCKOPT(RTDRV_L2_UCASTLOOKUPMODE_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_ucastLookupMode_set */

int32 rtk_l2_lookupMissFloodPortMask_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;   
    l2_data.type = type;
    GETSOCKOPT(RTDRV_L2_LOOKUP_MISS_FLOOD_GET, &l2_data, rtdrv_l2_addrData_t, 1);
    memcpy(pFlood_portmask, &l2_data.portmask, sizeof(rtk_portmask_t));

    return RT_ERR_OK;
}

int32 rtk_l2_lookupMissFloodPortMask_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_portmask_t *pFlood_portmask)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    memcpy(&l2_data.portmask, pFlood_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_L2_LOOKUP_MISS_FLOOD_SET, &l2_data, rtdrv_l2_addrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_srcPortEgrFilterMask_get(uint32 unit, rtk_portmask_t *pFlood_portmask)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;   
    GETSOCKOPT(RTDRV_L2_SRC_PORT_EGR_FILTER_GET, &l2_data, rtdrv_l2_addrData_t, 1);
    memcpy(pFlood_portmask, &l2_data.portmask, sizeof(rtk_portmask_t));

    return RT_ERR_OK;
}

int32 rtk_l2_srcPortEgrFilterMask_set(uint32 unit, rtk_portmask_t *pFlood_portmask)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    memcpy(&l2_data.portmask, pFlood_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_L2_SRC_PORT_EGR_FILTER_SET, &l2_data, rtdrv_l2_addrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_legalMoveToPorts_get(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalMoveTo_portmask)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;   
    l2_data.port = port;
    GETSOCKOPT(RTDRV_L2_LEGAL_MOVETO_PORTMASK_GET, &l2_data, rtdrv_l2_addrData_t, 1);
    memcpy(pLegalMoveTo_portmask, &l2_data.portmask, sizeof(rtk_portmask_t));

    return RT_ERR_OK;
}

int32 rtk_l2_legalMoveToPorts_set(uint32 unit, rtk_port_t port, rtk_portmask_t *pLegalMoveTo_portmask)
{
    rtdrv_l2_addrData_t l2_data;

    l2_data.unit = unit;
    l2_data.port = port;
    memcpy(&l2_data.portmask, pLegalMoveTo_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_L2_LEGAL_MOVETO_PORTMASK_SET, &l2_data, rtdrv_l2_addrData_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_legalPortMoveAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    rtdrv_l2_portAct_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_LEGAL_MOVETO_ACTION_GET, &l2_learn, rtdrv_l2_portAct_t, 1);
    *pAction = l2_learn.action;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_legalPortMoveAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    rtdrv_l2_portAct_t l2_action;

    l2_action.unit = unit;
    l2_action.port = port;
    l2_action.action = action;
    SETSOCKOPT(RTDRV_L2_LEGAL_MOVETO_ACTION_SET, &l2_action, rtdrv_l2_portAct_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_legalPortMoveFlushAddrEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_l2_common_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    GETSOCKOPT(RTDRV_L2_LEGAL_MOVETO_FLUSH_ENABLE_GET, &cfg, rtdrv_l2_common_t, 1);
    *pEnable = cfg.value;
    
    return RT_ERR_OK;
}

int32 rtk_l2_legalPortMoveFlushAddrEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_l2_common_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.value = enable;
    SETSOCKOPT(RTDRV_L2_LEGAL_MOVETO_FLUSH_ENABLE_SET, &cfg, rtdrv_l2_common_t, 1);
    
    return RT_ERR_OK;
}

int32 rtk_l2_illegalPortMoveAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pAction)
{
    rtdrv_l2_portAct_t l2_action;

    l2_action.unit = unit;
    l2_action.port = port;
    GETSOCKOPT(RTDRV_L2_ILLEGAL_MOVETO_ACTION_GET, &l2_action, rtdrv_l2_portAct_t, 1);
    *pAction = l2_action.action;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_illegalPortMoveAction_set(uint32 unit, rtk_port_t port, rtk_action_t action)
{
    rtdrv_l2_portAct_t l2_action;

    l2_action.unit = unit;
    l2_action.port = port;
    l2_action.action = action;
    SETSOCKOPT(RTDRV_L2_ILLEGAL_MOVETO_ACTION_SET, &l2_action, rtdrv_l2_portAct_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_staticPortMoveAction_get(uint32 unit, rtk_port_t port, rtk_action_t *pFwdAction)
{
    rtdrv_l2_portAct_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    GETSOCKOPT(RTDRV_L2_STTC_PORT_MOVE_ACTION_GET, &l2_learn, rtdrv_l2_portAct_t, 1);
    *pFwdAction = l2_learn.action;
    
    return RT_ERR_OK;
}

int32 rtk_l2_staticPortMoveAction_set(uint32 unit, rtk_port_t port, rtk_action_t fwdAction)
{
    rtdrv_l2_portAct_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.port = port;
    l2_learn.action = fwdAction;
    SETSOCKOPT(RTDRV_L2_STTC_PORT_MOVE_ACTION_SET, &l2_learn, rtdrv_l2_portAct_t, 1);
    
    return RT_ERR_OK;
}

int32 rtk_l2_lookupMissFloodPortMask_set_with_idx(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx, rtk_portmask_t *pFlood_portmask)
{
    rtdrv_l2_lkMiss_t l2_lkmiss;

    l2_lkmiss.unit = unit;
    l2_lkmiss.type = type;
    l2_lkmiss.index = idx;
    l2_lkmiss.portMask = *pFlood_portmask;
    SETSOCKOPT(RTDRV_L2_LOOKUP_MISS_FLOOD_PMSK_SET_WITH_IDX, &l2_lkmiss, rtdrv_l2_lkMiss_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_lookupMissFloodPortMaskIdx_get(uint32 unit, rtk_l2_lookupMissType_t type, uint32 *pIdx)
{
    rtdrv_l2_lkMiss_t l2_lkmiss;

    l2_lkmiss.unit = unit;
    l2_lkmiss.type = type;
    GETSOCKOPT(RTDRV_L2_LOOKUP_MISS_FLOODPORTMASK_IDX_GET, &l2_lkmiss, rtdrv_l2_lkMiss_t, 1);
    *pIdx = l2_lkmiss.index;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_lookupMissFloodPortMaskIdx_set(uint32 unit, rtk_l2_lookupMissType_t type, uint32 idx)
{
    rtdrv_l2_lkMiss_t l2_lkmiss;

    l2_lkmiss.unit = unit;
    l2_lkmiss.type = type;
    l2_lkmiss.index = idx;
    SETSOCKOPT(RTDRV_L2_LOOKUP_MISS_FLOODPORTMASK_IDX_SET, &l2_lkmiss, rtdrv_l2_lkMiss_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_lookupMissAction_get(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t *pAction)
{
    rtdrv_l2_lkMiss_t l2_action;

    l2_action.unit = unit;
    l2_action.type = type;
    GETSOCKOPT(RTDRV_L2_LOOKUP_MISS_ACTION_GET, &l2_action, rtdrv_l2_lkMiss_t, 1);
    *pAction = l2_action.action;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_lookupMissAction_set(uint32 unit, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
    rtdrv_l2_lkMiss_t l2_action;

    l2_action.unit = unit;
    l2_action.type = type;
    l2_action.action = action;
    SETSOCKOPT(RTDRV_L2_LOOKUP_MISS_ACTION_SET, &l2_action, rtdrv_l2_lkMiss_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portLookupMissAction_get(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t *pAction)
{
    rtdrv_l2_lkMiss_t l2_action;

    l2_action.unit = unit;
    l2_action.port = port;
    l2_action.type = type;
    GETSOCKOPT(RTDRV_L2_PORT_LOOKUP_MISS_ACTION_GET, &l2_action, rtdrv_l2_lkMiss_t, 1);
    *pAction = l2_action.action;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portLookupMissAction_set(uint32 unit, rtk_port_t port, rtk_l2_lookupMissType_t type, rtk_action_t action)
{
    rtdrv_l2_lkMiss_t l2_action;

    l2_action.unit = unit;
    l2_action.port = port;
    l2_action.type = type;
    l2_action.action = action;
    SETSOCKOPT(RTDRV_L2_PORT_LOOKUP_MISS_ACTION_SET, &l2_action, rtdrv_l2_lkMiss_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_exceptionAddrAction_get(
    uint32                          unit, 
    rtk_l2_exceptionAddrType_t      exceptType, 
    rtk_action_t                    *pAction)
{
    rtdrv_l2_exceptSa_t     l2_exceptSa;
    l2_exceptSa.unit = unit;
    l2_exceptSa.type = exceptType;

    GETSOCKOPT(RTDRV_L2_EXCEPTION_SA_ACTION_GET, &l2_exceptSa, rtdrv_l2_exceptSa_t, 1);
    *pAction = l2_exceptSa.action;

    return RT_ERR_OK;    
}

int32 rtk_l2_exceptionAddrAction_set(
    uint32                          unit, 
    rtk_l2_exceptionAddrType_t      exceptType, 
    rtk_action_t                    action)
{
    rtdrv_l2_exceptSa_t     l2_exceptSa;
    l2_exceptSa.unit = unit;
    l2_exceptSa.type = exceptType;
    l2_exceptSa.action = action;
    SETSOCKOPT(RTDRV_L2_EXCEPTION_SA_ACTION_SET, &l2_exceptSa, rtdrv_l2_exceptSa_t, 1);

    return RT_ERR_OK;    
} /*end of rtk_l2_exceptionAddrAction_set*/

int32 rtk_l2_notificationEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    GETSOCKOPT(RTDRV_L2_NOTIFICATIONENABLE_GET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    *pEnable = l2_learn.enable;
    
    return RT_ERR_OK;
} /* end of rtk_l2_notificationEnable_get */

int32 rtk_l2_notificationEnable_set(uint32 unit, rtk_enable_t enable)
{    
    rtdrv_l2_learnCfg_t l2_learn;

    l2_learn.unit = unit;
    l2_learn.enable = enable;
    SETSOCKOPT(RTDRV_L2_NOTIFICATIONENABLE_SET, &l2_learn, rtdrv_l2_learnCfg_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_notificationEnable_set */

int32 rtk_l2_notificationEventEnable_get(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t *pEnable)
{
    rtdrv_l2_notifyEventCfg_t l2_notifyEventType;

    l2_notifyEventType.unit = unit;
    l2_notifyEventType.event = event;
    GETSOCKOPT(RTDRV_L2_NOTIFICATIONENABLE_EVENT_TYPE_GET, &l2_notifyEventType, rtdrv_l2_notifyEventCfg_t, 1);
    *pEnable = l2_notifyEventType.enable;
    
    return RT_ERR_OK;
}

int32 rtk_l2_notificationEventEnable_set(uint32 unit, rtk_l2_notifyEvent_t event, rtk_enable_t enable)
{
    rtdrv_l2_notifyEventCfg_t l2_notifyEventType;

    l2_notifyEventType.unit = unit;
    l2_notifyEventType.event = event;
    l2_notifyEventType.enable = enable;
    SETSOCKOPT(RTDRV_L2_NOTIFICATIONENABLE_EVENT_TYPE_SET, &l2_notifyEventType, rtdrv_l2_notifyEventCfg_t, 1);
    
    return RT_ERR_OK;   
}
int32 rtk_l2_notificationBackPressureThresh_get(uint32 unit, uint32 *pThresh)
{
    rtdrv_l2_common_t l2_common;

    l2_common.unit = unit;
    GETSOCKOPT(RTDRV_L2_NOTIFICATION_BACKPRESSURE_THRESH_GET, &l2_common, rtdrv_l2_common_t, 1);
    *pThresh = l2_common.value;
    
    return RT_ERR_OK;
} /* end of rtk_l2_notificationBackPressureThresh_get */

int32 rtk_l2_notificationBackPressureThresh_set(uint32 unit, uint32 thresh)
{
    rtdrv_l2_common_t l2_common;

    l2_common.unit = unit;
    l2_common.value = thresh;
    SETSOCKOPT(RTDRV_L2_NOTIFICATION_BACKPRESSURE_THRESH_SET, &l2_common, rtdrv_l2_common_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_notificationBackPressureThresh_set */

int32 rtk_l2_secureMacMode_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_l2_common_t l2_common;

    l2_common.unit = unit;
    GETSOCKOPT(RTDRV_L2_SECURE_MAC_MODE_GET, &l2_common, rtdrv_l2_common_t, 1);
    *pEnable = l2_common.value;
    
    return RT_ERR_OK;
} /* end of rtk_l2_notificationBackPressureThresh_get */

int32 rtk_l2_secureMacMode_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_l2_common_t l2_common;

    l2_common.unit = unit;
    l2_common.value = enable;
    SETSOCKOPT(RTDRV_L2_SECURE_MAC_MODE_SET, &l2_common, rtdrv_l2_common_t, 1);
    
    return RT_ERR_OK;   
} /* end of rtk_l2_secureMacMode_set */

int32 rtk_l2_lookupMissPri_set(uint32 unit, rtk_pri_t pri)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    l2_pri.pri = pri;
    SETSOCKOPT(RTDRV_L2_LOOKUPMISSPRI_SET, &l2_pri, rtdrv_l2_pri_t, 1);

    return RT_ERR_OK;    
} /*end of rtk_l2_lookupMissPri_set*/

int32 rtk_l2_lookupMissPri_get(uint32 unit, rtk_pri_t *pPri)
{   
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;    
    GETSOCKOPT(RTDRV_L2_LOOKUPMISSPRI_GET, &l2_pri, rtdrv_l2_pri_t, 1);
    *pPri = l2_pri.pri;

    return RT_ERR_OK;    
} /*end of rtk_l2_lookupMissPri_get*/

int32 rtk_l2_lookupMissPriEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    l2_pri.enable = enable;
    SETSOCKOPT(RTDRV_L2_LOOKUPMISSPRIENABLE_SET, &l2_pri, rtdrv_l2_pri_t, 1);

    return RT_ERR_OK;    
} /*end of rtk_l2_lookupMissPri_set*/

int32 rtk_l2_lookupMissPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{   
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;    
    GETSOCKOPT(RTDRV_L2_LOOKUPMISSPRIENABLE_GET, &l2_pri, rtdrv_l2_pri_t, 1);
    *pEnable = l2_pri.enable;

    return RT_ERR_OK;    
} /*end of rtk_l2_lookupMissPriEnable_get*/

int32 rtk_l2_lookupMissDP_get(uint32 unit, uint32 *pDp)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    GETSOCKOPT(RTDRV_L2_LOOKUPMISSDP_GET, &l2_pri, rtdrv_l2_pri_t, 1);
    *pDp = l2_pri.dpValue;

    return RT_ERR_OK;    
} /* end of rtk_l2_lookupMissDP_get */


int32 rtk_l2_lookupMissDP_set(uint32 unit, uint32 dp)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    l2_pri.dpValue= dp;
    SETSOCKOPT(RTDRV_L2_LOOKUPMISSDP_SET, &l2_pri, rtdrv_l2_pri_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_lookupMissDP_set */


int32 rtk_l2_lookupMissDPEnable_get(uint32 unit, rtk_enable_t *pEnable)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    GETSOCKOPT(RTDRV_L2_LOOKUPMISSDPENABLE_GET, &l2_pri, rtdrv_l2_pri_t, 1);
    *pEnable = l2_pri.enable;

    return RT_ERR_OK;    
} /* end of rtk_l2_lookupMissDPEnable_get */


int32 rtk_l2_lookupMissDPEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    l2_pri.enable = enable;
    SETSOCKOPT(RTDRV_L2_LOOKUPMISSDPENABLE_SET, &l2_pri, rtdrv_l2_pri_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_lookupMissDPEnable_set */


int32 rtk_l2_lookupMissAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    GETSOCKOPT(RTDRV_L2_LOOKUPMISSADDCPUTAGENABLE_GET, &l2_pri, rtdrv_l2_pri_t, 1);
    *pEnable = l2_pri.insertCpuTag;

    return RT_ERR_OK;    
} /* end of rtk_l2_lookupMissAddCPUTagEnable_get */


int32 rtk_l2_lookupMissAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_pri_t               l2_pri;
    l2_pri.unit = unit;
    l2_pri.insertCpuTag = enable;
    SETSOCKOPT(RTDRV_L2_LOOKUPMISSADDCPUTAGENABLE_SET, &l2_pri, rtdrv_l2_pri_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_lookupMissAddCPUTagEnable_set */

int32 rtk_l2_mcastLookupMode_get(
    uint32                      unit, 
    rtk_l2_mcastLookupMode_t    *pMcast_lookupMode, 
    rtk_fid_t                   *pFixed_fid)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    GETSOCKOPT(RTDRV_L2_MCASTLOOKUPMODE_GET, &l2_config, rtdrv_l2_learnCfg_t, 1);
    *pMcast_lookupMode = l2_config.mcast_lookupMode;
    *pFixed_fid = l2_config.fixed_fid;

    return RT_ERR_OK;    
} /* end of rtk_l2_mcastLookupMode_get */


int32 rtk_l2_mcastLookupMode_set(
    uint32                      unit, 
    rtk_l2_mcastLookupMode_t    mcast_lookupMode, 
    rtk_fid_t                   fixed_fid)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    l2_config.mcast_lookupMode = mcast_lookupMode;
    l2_config.fixed_fid = fixed_fid;
    SETSOCKOPT(RTDRV_L2_MCASTLOOKUPMODE_SET, &l2_config, rtdrv_l2_learnCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_mcastLookupMode_set */

int32 rtk_l2_ipmcEnable_get(uint32 unit, rtk_enable_t *pEnable)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    GETSOCKOPT(RTDRV_L2_IPMCENABLE_GET, &l2_config, rtdrv_l2_learnCfg_t, 1);
    *pEnable = l2_config.enable;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcEnable_get */


int32 rtk_l2_ipmcEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    l2_config.enable = enable;
    SETSOCKOPT(RTDRV_L2_IPMCENABLE_SET, &l2_config, rtdrv_l2_learnCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcEnable_set */


int32 rtk_l2_ipmcMode_get(uint32 unit, rtk_l2_ipmcMode_t *pMode)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    GETSOCKOPT(RTDRV_L2_IPMCMODE_GET, &l2_config, rtdrv_l2_learnCfg_t, 1);
    *pMode = l2_config.ipmcMode;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcMode_get */


int32 rtk_l2_ipmcMode_set(uint32 unit, rtk_l2_ipmcMode_t mode)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    l2_config.ipmcMode = mode;
    SETSOCKOPT(RTDRV_L2_IPMCMODE_SET, &l2_config, rtdrv_l2_learnCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcMode_set */

int32 rtk_l2_ipMcastAddrChkEnable_get(uint32 unit, rtk_enable_t *pEnable)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    GETSOCKOPT(RTDRV_L2_IPMC_DIP_CHK_GET, &l2_config, rtdrv_l2_learnCfg_t, 1);
    *pEnable = l2_config.dip_check;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipMcastAddrChkEnable_get */


int32 rtk_l2_ipMcastAddrChkEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_learnCfg_t               l2_config;
    l2_config.unit = unit;
    l2_config.dip_check = enable;
    SETSOCKOPT(RTDRV_L2_IPMC_DIP_CHK_SET, &l2_config, rtdrv_l2_learnCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipMcastAddrChkEnable_set */

int32 rtk_l2_ipMcstFidVidCompareEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_l2_common_t cfg;
    
    cfg.unit = unit;
    GETSOCKOPT(RTDRV_L2_IPMC_VLAN_COMPARE_GET, &cfg, rtdrv_l2_common_t, 1);
    *pEnable = cfg.value;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipMcstFidVidCompareEnable_get */

int32 rtk_l2_ipMcstFidVidCompareEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_l2_common_t cfg;
    
    cfg.unit = unit;
    cfg.value = enable;
    SETSOCKOPT(RTDRV_L2_IPMC_VLAN_COMPARE_SET, &cfg, rtdrv_l2_common_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipMcstFidVidCompareEnable_set */

 
int32 rtk_l2_ipmcDstAddrMismatchAction_get(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    *pMismatch_action)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    GETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHACTION_GET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);
    *pMismatch_action = l2_ipmcCfg.mismatch_action;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchAction_get */


int32 rtk_l2_ipmcDstAddrMismatchAction_set(
    uint32                          unit,
    rtk_l2_ipmc_mismatchType_t      type,
    rtk_l2_ipmcMismatch_action_t    mismatch_action)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    l2_ipmcCfg.mismatch_action = mismatch_action;
    SETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHACTION_SET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchAction_set */


int32 rtk_l2_ipmcDstAddrMismatchPri_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t *pPriority)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    GETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHPRI_GET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);
    *pPriority = l2_ipmcCfg.priority;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchPri_get */


int32 rtk_l2_ipmcDstAddrMismatchPri_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_pri_t priority)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    l2_ipmcCfg.priority = priority;
    SETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHPRI_SET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchPri_set */


int32 rtk_l2_ipmcDstAddrMismatchPriEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    GETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHPRIENABLE_GET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);
    *pEnable = l2_ipmcCfg.enable;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchPriEnable_get */


int32 rtk_l2_ipmcDstAddrMismatchPriEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    l2_ipmcCfg.enable = enable;
    SETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHPRIENABLE_SET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchPriEnable_set */


int32 rtk_l2_ipmcDstAddrMismatchDP_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 *pDp)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    GETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHDP_GET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);
    *pDp = l2_ipmcCfg.dpValue;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchDP_get */


int32 rtk_l2_ipmcDstAddrMismatchDP_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, uint32 dp)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    l2_ipmcCfg.dpValue = dp;
    SETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHDP_SET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchDP_set */


int32 rtk_l2_ipmcDstAddrMismatchDPEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t  *pEnable)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    GETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHDPENABLE_GET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);
    *pEnable = l2_ipmcCfg.enable;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchDPEnable_get */


int32 rtk_l2_ipmcDstAddrMismatchDPEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    l2_ipmcCfg.enable = enable;
    SETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHDPENABLE_SET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchDPEnable_set */


int32 rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t *pEnable)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    GETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHADDCPUTAGENABLE_GET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);
    *pEnable = l2_ipmcCfg.insertCpuTag;

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get */


int32 rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(uint32 unit, rtk_l2_ipmc_mismatchType_t type, rtk_enable_t enable)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;
    
    l2_ipmcCfg.unit = unit;
    l2_ipmcCfg.type = type;    
    l2_ipmcCfg.insertCpuTag = enable;
    SETSOCKOPT(RTDRV_L2_IPMCDSTADDRMISMATCHADDCPUTAGENABLE_SET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set */

int32 rtk_l2_ipmc_routerPorts_get(uint32 unit, rtk_portmask_t *pPortmask)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;

    memset(&l2_ipmcCfg, 0, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
    
    l2_ipmcCfg.unit = unit;
    GETSOCKOPT(RTDRV_L2_IPMC_ROUTERPORTS_GET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);
    memcpy(pPortmask, &l2_ipmcCfg.router_portMask, sizeof(rtk_portmask_t));

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmc_routerPorts_get */


int32 rtk_l2_ipmc_routerPorts_set(uint32 unit, rtk_portmask_t *pPortmask)
{  
    rtdrv_l2_ipmcMismatchCfg_t l2_ipmcCfg;

    memset(&l2_ipmcCfg, 0, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
    
    l2_ipmcCfg.unit = unit;
    memcpy(&l2_ipmcCfg.router_portMask, pPortmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_L2_IPMC_ROUTERPORTS_SET, &l2_ipmcCfg, rtdrv_l2_ipmcMismatchCfg_t, 1);    

    return RT_ERR_OK;    
} /* end of rtk_l2_ipmc_routerPorts_set */



int32 rtk_l2_ip6CareByte_get(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 *pCareByte)
{
    rtdrv_l2_hashCareByte_t l2_hashCareByte;

    l2_hashCareByte.unit = unit;
    l2_hashCareByte.type = type;
    GETSOCKOPT(RTDRV_L2_HASHCAREBYTE_GET, &l2_hashCareByte, rtdrv_l2_hashCareByte_t, 1);
    *pCareByte = l2_hashCareByte.value;
    
    return RT_ERR_OK;   
} /* end of rtk_l2_ip6CareByte_get */


int32 rtk_l2_ip6CareByte_set(uint32 unit, rtk_l2_ip6_careByte_type_t type, uint32 careByte)
{
    rtdrv_l2_hashCareByte_t l2_hashCareByte;

    l2_hashCareByte.unit = unit;
    l2_hashCareByte.type = type;
    l2_hashCareByte.value = careByte;
    SETSOCKOPT(RTDRV_L2_HASHCAREBYTE_SET, &l2_hashCareByte, rtdrv_l2_hashCareByte_t, 1);
    
    return RT_ERR_OK;
} /* end of rtk_l2_ip6CareByte_set */


int32 rtk_l2_ip6McastAddr_add(uint32 unit, rtk_l2_ip6McastAddr_t *pIp6mcast_addr)
{
    rtdrv_l2_ip6McstAddrData_t ip6Mcast_data;

    ip6Mcast_data.unit = unit;
    ip6Mcast_data.ip6_m_data = *pIp6mcast_addr;
    SETSOCKOPT(RTDRV_L2_IP6_MCAST_ADDR_ADD, &ip6Mcast_data, rtdrv_l2_ip6McstAddrData_t, 1);
    
    return RT_ERR_OK;
} /* end of rtk_l2_ip6CareByte_get */

int32 rtk_l2_ip6McastAddr_get(uint32 unit, rtk_l2_ip6McastAddr_t *pIp6mcast_addr)
{
    rtdrv_l2_ip6McstAddrData_t ip6Mcast_data;

    ip6Mcast_data.unit = unit;
    ip6Mcast_data.ip6_m_data = *pIp6mcast_addr;
    GETSOCKOPT(RTDRV_L2_IP6_MCAST_ADDR_GET, &ip6Mcast_data, rtdrv_l2_ip6McstAddrData_t, 1);
    memcpy(pIp6mcast_addr, &ip6Mcast_data.ip6_m_data, sizeof(rtk_l2_ip6McastAddr_t)); 

    return RT_ERR_OK;
}

int32 rtk_l2_nextValidIp6McastAddr_get(uint32 unit, int32 *pScan_idx, rtk_l2_ip6McastAddr_t *pL2_data)
{
    rtdrv_l2_ip6McstAddrData_t ip6_data;

    ip6_data.unit = unit;   
    ip6_data.index = *pScan_idx;
    memcpy(&ip6_data.ip6_m_data, pL2_data, sizeof(rtk_l2_ip6McastAddr_t));
    GETSOCKOPT(RTDRV_L2_IP6_MCAST_ADDR_GETNEXT, &ip6_data, rtdrv_l2_ip6McstAddrData_t, 1);
    *pScan_idx = ip6_data.index;
    memcpy(pL2_data, &ip6_data.ip6_m_data, sizeof(rtk_l2_ip6McastAddr_t));

    return RT_ERR_OK;
}

int32 rtk_l2_trapPri_get(uint32 unit, rtk_pri_t *pPriority)
{  
    rtdrv_l2_learnPriDp_t l2_learnPri;

    memset(&l2_learnPri, 0, sizeof(rtdrv_l2_learnPriDp_t));
    
    l2_learnPri.unit = unit;
    GETSOCKOPT(RTDRV_L2_TRAPPRI_GET, &l2_learnPri, rtdrv_l2_learnPriDp_t, 1);
    *pPriority = l2_learnPri.priority;

    return RT_ERR_OK;    
} /* end of rtk_l2_trapPri_get */


int32 rtk_l2_trapPri_set(uint32 unit, rtk_pri_t priority)
{  
    rtdrv_l2_learnPriDp_t l2_learnPri;

    memset(&l2_learnPri, 0, sizeof(rtdrv_l2_learnPriDp_t));
    
    l2_learnPri.unit = unit;
    l2_learnPri.priority = priority;
    SETSOCKOPT(RTDRV_L2_TRAPPRI_SET, &l2_learnPri, rtdrv_l2_learnPriDp_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_trapPri_set */


int32 rtk_l2_trapPriEnable_get(uint32 unit, rtk_enable_t *pEnable)
{  
    rtdrv_l2_learnPriDp_t l2_learnPri;

    memset(&l2_learnPri, 0, sizeof(rtdrv_l2_learnPriDp_t));
    
    l2_learnPri.unit = unit;
    GETSOCKOPT(RTDRV_L2_TRAPPRIENABLE_GET, &l2_learnPri, rtdrv_l2_learnPriDp_t, 1);
    *pEnable = l2_learnPri.enable;

    return RT_ERR_OK;    
} /* end of rtk_l2_trapPriEnable_get */


int32 rtk_l2_trapPriEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_learnPriDp_t l2_learnPri;

    memset(&l2_learnPri, 0, sizeof(rtdrv_l2_learnPriDp_t));
    
    l2_learnPri.unit = unit;
    l2_learnPri.enable = enable;
    SETSOCKOPT(RTDRV_L2_TRAPPRIENABLE_SET, &l2_learnPri, rtdrv_l2_learnPriDp_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_trapPriEnable_set */



int32 rtk_l2_trapAddCPUTagEnable_get(uint32 unit, rtk_enable_t *pEnable)
{  
    rtdrv_l2_learnPriDp_t l2_learnPri;

    memset(&l2_learnPri, 0, sizeof(rtdrv_l2_learnPriDp_t));
    
    l2_learnPri.unit = unit;
    GETSOCKOPT(RTDRV_L2_TRAPADDCPUTAGENABLE_GET, &l2_learnPri, rtdrv_l2_learnPriDp_t, 1);
    *pEnable = l2_learnPri.insertCpuTag;

    return RT_ERR_OK;    
} /* end of rtk_l2_trapAddCPUTagEnable_get */


int32 rtk_l2_trapAddCPUTagEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_learnPriDp_t l2_learnPri;

    memset(&l2_learnPri, 0, sizeof(rtdrv_l2_learnPriDp_t));
    
    l2_learnPri.unit = unit;
    l2_learnPri.insertCpuTag = enable;
    SETSOCKOPT(RTDRV_L2_TRAPADDCPUTAGENABLE_SET, &l2_learnPri, rtdrv_l2_learnPriDp_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_trapAddCPUTagEnable_set */

int32 rtk_l2_mcastBlockPortmask_get(uint32 unit, rtk_portmask_t *pPortmask)
{  
    rtdrv_l2_portmaskCfg_t  l2_cfg;

    memset(&l2_cfg, 0, sizeof(rtdrv_l2_portmaskCfg_t));

    l2_cfg.unit = unit;
    GETSOCKOPT(RTDRV_L2_MCASTBLOCKPORTMASK_GET, &l2_cfg, rtdrv_l2_portmaskCfg_t, 1);
    memcpy(pPortmask, &l2_cfg.portMask, sizeof(rtk_portmask_t));

    return RT_ERR_OK;    
} /* end of rtk_l2_mcastBlockPortmask_get */

int32 rtk_l2_mcastBlockPortmask_set(uint32 unit, rtk_portmask_t *pPortmask)
{  
    rtdrv_l2_portmaskCfg_t  l2_cfg;

    memset(&l2_cfg, 0, sizeof(rtdrv_l2_portmaskCfg_t));
    
    l2_cfg.unit = unit;
    memcpy(&l2_cfg.portMask, pPortmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_L2_MCASTBLOCKPORTMASK_SET, &l2_cfg, rtdrv_l2_portmaskCfg_t, 1);    

    return RT_ERR_OK;    
} /* end of rtk_l2_mcastBlockPortmask_set */

int32 rtk_l2_addrEntry_get(uint32 unit, uint32 index, rtk_l2_entry_t *pL2_entry)
{
    rtdrv_l2_entry_t  l2_cfg;

    memset(&l2_cfg, 0, sizeof(rtdrv_l2_entry_t));

    l2_cfg.unit = unit;
    l2_cfg.index = index;
    GETSOCKOPT(RTDRV_L2_ADDRENTRY_GET, &l2_cfg, rtdrv_l2_entry_t, 1);
    memcpy(pL2_entry, &l2_cfg.entry, sizeof(rtk_l2_entry_t));

    return RT_ERR_OK;
} /* end of rtk_l2_addrEntry_get */

int32
rtk_l2_conflictAddr_get(
    uint32          unit,
    rtk_l2_entry_t  *pL2Addr,
    rtk_l2_entry_t  *pCfAddrList,
    uint32          cfAddrList_size,
    uint32          *pCf_retCnt)
{
    rtdrv_l2_conflict_t  l2_cfg;

    memset(&l2_cfg, 0, sizeof(rtdrv_l2_conflict_t));

    l2_cfg.unit = unit;
    memcpy(&l2_cfg.input, pL2Addr, sizeof(rtk_l2_entry_t));
    l2_cfg.size_of_output_buf = cfAddrList_size;
    GETSOCKOPT(RTDRV_L2_CONFLICT_ADDR_GET, &l2_cfg, rtdrv_l2_conflict_t, 1);
    memcpy(pCfAddrList, &l2_cfg.output, sizeof(rtk_l2_entry_t) * cfAddrList_size);
    (*pCf_retCnt) = l2_cfg.ret_valid_cnt;

    return RT_ERR_OK;
} /* end of rtk_l2_conflictAddr_get */

int32 rtk_l2_zeroSALearningEnable_get(uint32 unit, rtk_enable_t *pEnable)
{  
    rtdrv_l2_learnCfg_t l2_learnCfg;

    memset(&l2_learnCfg, 0, sizeof(rtdrv_l2_learnCfg_t));
    
    l2_learnCfg.unit = unit;
    GETSOCKOPT(RTDRV_L2_ZEROSALEARNINGENABLE_GET, &l2_learnCfg, rtdrv_l2_learnCfg_t, 1);
    *pEnable = l2_learnCfg.enable;

    return RT_ERR_OK;    
} /* end of rtk_l2_zeroSALearningEnable_get */


int32 rtk_l2_zeroSALearningEnable_set(uint32 unit, rtk_enable_t enable)
{  
    rtdrv_l2_learnCfg_t l2_learnCfg;

    memset(&l2_learnCfg, 0, sizeof(rtdrv_l2_learnCfg_t));
    
    l2_learnCfg.unit = unit;
    l2_learnCfg.enable = enable;
    SETSOCKOPT(RTDRV_L2_ZEROSALEARNINGENABLE_SET, &l2_learnCfg, rtdrv_l2_learnCfg_t, 1);

    return RT_ERR_OK;    
} /* end of rtk_l2_zeroSALearningEnable_set */

int32 rtk_l2_portDynamicPortMoveForbidEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_l2_common_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    GETSOCKOPT(RTDRV_L2_PORT_DYNM_PORTMOVE_FORBID_ENABLE_GET, &cfg, rtdrv_l2_common_t, 1);
    *pEnable = cfg.value;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_portDynamicPortMoveForbidEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_l2_common_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.value = enable;
    SETSOCKOPT(RTDRV_L2_PORT_DYNM_PORTMOVE_FORBID_ENABLE_SET, &cfg, rtdrv_l2_common_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_l2_dynamicPortMoveForbidAction_get(uint32 unit, rtk_action_t *pAction)
{
    rtdrv_l2_common_t cfg;

    cfg.unit = unit;
    GETSOCKOPT(RTDRV_L2_DYNM_PORTMOVE_FORBID_ACTION_GET, &cfg, rtdrv_l2_common_t, 1);
    *pAction = cfg.value;
    
    return RT_ERR_OK;    
}

int32 rtk_l2_dynamicPortMoveForbidAction_set(uint32 unit, rtk_action_t action)
{
    rtdrv_l2_common_t cfg;

    cfg.unit = unit;
    cfg.value = action;
    SETSOCKOPT(RTDRV_L2_DYNM_PORTMOVE_FORBID_ACTION_SET, &cfg, rtdrv_l2_common_t, 1);
    
    return RT_ERR_OK;    
}

int32
rtk_l2_portMacFilterEnable_get(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t *pEnable)
{
    rtdrv_l2_mac_filter_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.filterMode = filterMode;
    GETSOCKOPT(RTDRV_L2_PORT_MAC_FILTER_ENABLE_GET, &cfg, rtdrv_l2_mac_filter_t, 1);
    *pEnable = cfg.enable;
    
    return RT_ERR_OK;  
}

int32
rtk_l2_portMacFilterEnable_set(uint32 unit, rtk_port_t port, rtk_l2_macFilterMode_t filterMode, rtk_enable_t enable)
{
    rtdrv_l2_mac_filter_t cfg;

    cfg.unit = unit;
    cfg.port = port;
    cfg.filterMode = filterMode;
    cfg.enable = enable;
    SETSOCKOPT(RTDRV_L2_PORT_MAC_FILTER_ENABLE_SET, &cfg, rtdrv_l2_mac_filter_t, 1);
    
    return RT_ERR_OK;  
}

int32
rtk_l2_hwNextValidAddr_get(
    uint32              unit,
    int32               *pScan_idx,
    rtk_l2_nextValidType_t type,
    rtk_l2_entry_t  *pEntry)
{
    rtdrv_l2_entry_get_t cfg;

    cfg.unit = unit;
    cfg.scan_idx = *pScan_idx;
    cfg.type = type;
    GETSOCKOPT(RTDRV_L2_HW_NEXT_VALID_ADDR_GET, &cfg, rtdrv_l2_entry_get_t, 1);
    *pEntry = cfg.entry;
    
    return RT_ERR_OK;  
}

