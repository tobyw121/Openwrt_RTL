/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 * 
 * $Revision: 21987 $
 * $Date: 2011-09-05 16:53:49 +0800 (Mon, 05 Sep 2011) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) trunk
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtk/trunk.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_trunk_port_get(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;   
    GETSOCKOPT(RTDRV_TRUNK_PORT_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
    memcpy(pTrunk_member_portmask, &trunk_cfg.trk_member, sizeof(rtk_portmask_t));

    return RT_ERR_OK;    
}

int32 rtk_trunk_port_set(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;
    memcpy(&trunk_cfg.trk_member, pTrunk_member_portmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_TRUNK_PORT_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
      
    return RT_ERR_OK;    
}

int32 rtk_trunk_distributionAlgorithm_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_bitmask)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;   
    GETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
    *pAlgo_bitmask = trunk_cfg.algo_bitmask;

    return RT_ERR_OK;    
}

int32 rtk_trunk_distributionAlgorithm_set(uint32 unit, uint32 trk_gid, uint32 algo_bitmask)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;
    trunk_cfg.algo_bitmask = algo_bitmask;
    SETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
      
    return RT_ERR_OK;    
}

int32 rtk_trunk_hashMappingTable_get(uint32 unit, uint32 trk_gid, rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;   
    GETSOCKOPT(RTDRV_TRUNK_HASH_MAPPING_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
    memcpy(pHash2Port_array, &trunk_cfg.hash2Port_array, sizeof(rtk_trunk_hashVal2Port_t));

    return RT_ERR_OK;    
}

int32 rtk_trunk_hashMappingTable_set(uint32 unit, uint32 trk_gid, rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;
    memcpy(&trunk_cfg.hash2Port_array, pHash2Port_array, sizeof(rtk_trunk_hashVal2Port_t));
    SETSOCKOPT(RTDRV_TRUNK_HASH_MAPPING_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
      
    return RT_ERR_OK;    
}

int32 rtk_trunk_mode_get(uint32 unit, rtk_trunk_mode_t *pMode)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    GETSOCKOPT(RTDRV_TRUNK_TRUNK_MODE_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
    *pMode = trunk_cfg.mode;   
    
    return RT_ERR_OK;    
}

int32 rtk_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.mode = mode;
    SETSOCKOPT(RTDRV_TRUNK_TRUNK_MODE_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
      
    return RT_ERR_OK;    
}

int32 rtk_trunk_port_link_notification(uint32 unit, rtk_port_t trunkMemberPort, rtk_port_linkStatus_t linkStatus)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_memberport = trunkMemberPort;
    trunk_cfg.linkStat = linkStatus;
    SETSOCKOPT(RTDRV_TRUNK_PORT_LINK_NOTIFICATION, &trunk_cfg, rtdrv_trunkCfg_t, 1);    

    return RT_ERR_OK;    
}

int32 rtk_trunk_representPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pRepresPort)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trunk_id;
    GETSOCKOPT(RTDRV_TRUNK_REPRESENTPORT_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
    *pRepresPort = trunk_cfg.represPort;
      
    return RT_ERR_OK;    
} /* end of rtk_trunk_representPort_get */


int32 rtk_trunk_representPort_set(uint32 unit, uint32 trunk_id, rtk_port_t represPort)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trunk_id;
    trunk_cfg.represPort = represPort;
    SETSOCKOPT(RTDRV_TRUNK_REPRESENTPORT_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
      
    return RT_ERR_OK;    
} /* end of rtk_trunk_representPort_set */


int32 rtk_trunk_floodMode_get(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t *pFloodMode)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trunk_id;
    GETSOCKOPT(RTDRV_TRUNK_FLOODMODE_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
    *pFloodMode = trunk_cfg.floodMode;
      
    return RT_ERR_OK;    
} /* end of rtk_trunk_floodMode_get */


int32 rtk_trunk_floodMode_set(uint32 unit, uint32 trunk_id, rtk_trunk_floodMode_t floodMode)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trunk_id;
    trunk_cfg.floodMode = floodMode;
    SETSOCKOPT(RTDRV_TRUNK_FLOODMODE_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
      
    return RT_ERR_OK;    
} /* end of rtk_trunk_floodMode_set */


int32 rtk_trunk_floodPort_get(uint32 unit, uint32 trunk_id, rtk_port_t *pFloodPort)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trunk_id;
    GETSOCKOPT(RTDRV_TRUNK_FLOODPORT_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
    *pFloodPort = trunk_cfg.floodPort;
      
    return RT_ERR_OK;    
} /* end of rtk_trunk_floodPort_get */


int32 rtk_trunk_floodPort_set(uint32 unit, uint32 trunk_id, rtk_port_t floodPort)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trunk_id;
    trunk_cfg.floodPort = floodPort;
    SETSOCKOPT(RTDRV_TRUNK_FLOODPORT_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
      
    return RT_ERR_OK;    
} /* end of rtk_trunk_floodPort_set */

int32 rtk_trunk_distributionAlgorithmBind_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_idx)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;
    GETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_BIND_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
    *pAlgo_idx = trunk_cfg.algo_id;
      
    return RT_ERR_OK;    
}

int32 rtk_trunk_distributionAlgorithmBind_set(uint32 unit, uint32 trk_gid, uint32 algo_idx)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;
    trunk_cfg.algo_id = algo_idx;
    
    SETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_BIND_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);   
      
    return RT_ERR_OK;   
}

int32 rtk_trunk_distributionAlgorithmParam_get(uint32 unit, uint32 algo_idx, uint32 *pAlgo_bitmask)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.algo_id = algo_idx;   
    GETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_PARAM_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
    *pAlgo_bitmask = trunk_cfg.algo_bitmask;

    return RT_ERR_OK;    
}

int32 rtk_trunk_distributionAlgorithmParam_set(uint32 unit, uint32 algo_idx, uint32 algo_bitmask)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.algo_id = algo_idx;
    trunk_cfg.algo_bitmask = algo_bitmask;
    SETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_PARAM_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
      
    return RT_ERR_OK;  
}

int32 rtk_trunk_distributionAlgorithmShift_get(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.algo_id = algo_idx;   
    GETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_SHIFT_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
    *pShift = trunk_cfg.shift;

    return RT_ERR_OK; 
}

int32 rtk_trunk_distributionAlgorithmShift_set(uint32 unit, uint32 algo_idx, rtk_trunk_distAlgoShift_t *pShift)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.algo_id = algo_idx;
    trunk_cfg.shift = *pShift;
    SETSOCKOPT(RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_SHIFT_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
      
    return RT_ERR_OK; 
}

int32 rtk_trunk_trafficSeparate_get(uint32 unit, uint32 trk_gid, rtk_trunk_separateType_t *pSeparateType)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;   
    GETSOCKOPT(RTDRV_TRUNK_TRAFFIC_SEPARATE_GET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
    *pSeparateType = trunk_cfg.separate;

    return RT_ERR_OK; 
}

int32 rtk_trunk_trafficSeparate_set(uint32 unit, uint32 trk_gid, rtk_trunk_separateType_t separateType)
{
    rtdrv_trunkCfg_t trunk_cfg;

    trunk_cfg.unit = unit;
    trunk_cfg.trk_gid = trk_gid;
    trunk_cfg.separate = separateType;
    SETSOCKOPT(RTDRV_TRUNK_TRAFFIC_SEPARATE_SET, &trunk_cfg, rtdrv_trunkCfg_t, 1);    
      
    return RT_ERR_OK; 
}

