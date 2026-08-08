/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 31097 $
 * $Date: 2012-07-18 14:23:49 +0800 (Wed, 18 Jul 2012) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) QoS
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 rtk_qos_queueNum_get(uint32 unit, uint32 *pQueue_num)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_QUEUE_NUM_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pQueue_num = qos_cfg.queue_num;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_queueNum_set(uint32 unit, uint32 queue_num)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.queue_num = queue_num;
    SETSOCKOPT(RTDRV_QOS_QUEUE_NUM_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_priMap_get(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.queue_num = queue_num;
    GETSOCKOPT(RTDRV_QOS_PRI_MAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pPri2qid, &qos_cfg.pri2qid, sizeof(rtk_qos_pri2queue_t));
    
    return RT_ERR_OK;    
}

int32 rtk_qos_priMap_set(uint32 unit, uint32 queue_num, rtk_qos_pri2queue_t *pPri2qid)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.queue_num = queue_num;
    memcpy(&qos_cfg.pri2qid, pPri2qid, sizeof(rtk_qos_pri2queue_t));
    SETSOCKOPT(RTDRV_QOS_PRI_MAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_1pPriRemap_get(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t *pInt_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dot1p_pri = dot1p_pri;
    GETSOCKOPT(RTDRV_QOS_1P_PRI_REMAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInt_pri = qos_cfg.int_pri;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_1pPriRemap_set(uint32 unit, rtk_pri_t dot1p_pri, rtk_pri_t int_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.dot1p_pri = dot1p_pri;
    qos_cfg.int_pri = int_pri;
    SETSOCKOPT(RTDRV_QOS_1P_PRI_REMAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dpSrcSel_get(uint32 unit, rtk_qos_dpSrc_t *pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_DP_SRC_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.dpSrcType;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_dpSrcSel_set(uint32 unit, rtk_qos_dpSrc_t type)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dpSrcType = type;
    SETSOCKOPT(RTDRV_QOS_DP_SRC_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_qos_priSelGroup_get(uint32 unit, uint32 grp_idx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    GETSOCKOPT(RTDRV_QOS_PRI_SEL_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pWeightOfPriSel, &(qos_cfg.priSelWeight), sizeof(rtk_qos_priSelWeight_t));
    
    return RT_ERR_OK;    
}

int32 rtk_qos_priSelGroup_set(uint32 unit, uint32 grp_idx, rtk_qos_priSelWeight_t *pWeightOfPriSel)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    memcpy(&(qos_cfg.priSelWeight), pWeightOfPriSel, sizeof(rtk_qos_priSelWeight_t));
    SETSOCKOPT(RTDRV_QOS_PRI_SEL_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portPriSelGroup_get(uint32 unit, rtk_port_t port, uint32 *pPriSelGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_PRI_SEL_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pPriSelGrp_idx = qos_cfg.index;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portPriSelGroup_set(uint32 unit, rtk_port_t port, uint32 priSelGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = priSelGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_PRI_SEL_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portDp_get(uint32 unit, rtk_port_t port, uint32 *pDp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_DP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDp = qos_cfg.dp;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portDp_set(uint32 unit, rtk_port_t port, uint32 dp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.dp = dp;
    SETSOCKOPT(RTDRV_QOS_PORT_DP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portInnerPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_INNER_PRI_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pPri = qos_cfg.int_pri;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portInnerPri_set(uint32 unit, rtk_port_t port, rtk_pri_t pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.int_pri = pri;
    SETSOCKOPT(RTDRV_QOS_PORT_INNER_PRI_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portOuterPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pPri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_OUTER_PRI_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pPri = qos_cfg.int_pri;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portOuterPri_set(uint32 unit, rtk_port_t port, rtk_pri_t pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.int_pri = pri;
    SETSOCKOPT(RTDRV_QOS_PORT_OUTER_PRI_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portOuterDEI_get(uint32 unit, rtk_port_t port, uint32 *pDEI)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_OUTER_DEI_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDEI = qos_cfg.dei;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portOuterDEI_set(uint32 unit, rtk_port_t port, uint32 DEI)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.dei = DEI;
    SETSOCKOPT(RTDRV_QOS_PORT_OUTER_DEI_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_deiDpRemap_get(uint32 unit, uint32 dei, uint32 * pDp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dei = dei;
    GETSOCKOPT(RTDRV_QOS_DEI_DP_REMAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDp = qos_cfg.dp;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_deiDpRemap_set(uint32 unit, uint32 dei, uint32 dp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.dei = dei;
    qos_cfg.dp = dp;
    SETSOCKOPT(RTDRV_QOS_DEI_DP_REMAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portDEISrcSel_get(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t * pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_DEI_SRC_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.deiSrc;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portDEISrcSel_set(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t type)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.deiSrc = type;
    SETSOCKOPT(RTDRV_QOS_PORT_DEI_SRC_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscpDpRemap_get(uint32 unit, uint32 dscp, uint32 *pDp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dscp = dscp;
    GETSOCKOPT(RTDRV_QOS_DSCP_DP_REMAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDp = qos_cfg.dp;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_dscpDpRemap_set(uint32 unit, uint32 dscp, uint32 dp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.dscp = dscp;
    qos_cfg.dp = dp;
    SETSOCKOPT(RTDRV_QOS_DSCP_DP_REMAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscpPriRemap_get(uint32 unit, uint32 dscp, rtk_pri_t *pInt_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dscp = dscp;
    GETSOCKOPT(RTDRV_QOS_DSCP_PRI_REMAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInt_pri = qos_cfg.int_pri;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_dscpPriRemap_set(uint32 unit, uint32 dscp, rtk_pri_t int_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.dscp = dscp;
    qos_cfg.int_pri = int_pri;
    SETSOCKOPT(RTDRV_QOS_DSCP_PRI_REMAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portPri_get(uint32 unit, rtk_port_t port, rtk_pri_t *pInt_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_PRI_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInt_pri = qos_cfg.int_pri;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portPri_set(uint32 unit, rtk_port_t port, rtk_pri_t int_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.int_pri = int_pri;
    SETSOCKOPT(RTDRV_QOS_PORT_PRI_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_priSel_get(uint32 unit, uint32 *pPort_pri, uint32 *pClass_pri, uint32 *pAcl_pri, uint32 *pDscp_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_PRI_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pPort_pri = qos_cfg.port_pri;
    *pClass_pri = qos_cfg.class_pri;
    *pAcl_pri = qos_cfg.acl_pri;
    *pDscp_pri = qos_cfg.dscp_pri;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_priSel_set(uint32 unit, uint32 port_pri, uint32 class_pri, uint32 acl_pri, uint32 dscp_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port_pri = port_pri;
    qos_cfg.class_pri = class_pri;
    qos_cfg.acl_pri = acl_pri;
    qos_cfg.dscp_pri = dscp_pri;
    SETSOCKOPT(RTDRV_QOS_PRI_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscpPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    uint32      dscp,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.dscp = dscp;
    GETSOCKOPT(RTDRV_QOS_DSCP_PRI_REMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInt_pri = qos_cfg.int_pri;
    *pDp = qos_cfg.dp;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_dscpPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    uint32      dscp,
    rtk_pri_t   int_pri,
    uint32      dp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.dscp = dscp;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp = dp;
    SETSOCKOPT(RTDRV_QOS_DSCP_PRI_REMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portDscpPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pDscpGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_DSCP_PRI_REMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDscpGrp_idx = qos_cfg.index;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portDscpPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 dscpGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = dscpGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_DSCP_PRI_REMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_1pPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.dot1p_pri = dot1p_pri;
    GETSOCKOPT(RTDRV_QOS_1P_PRI_REMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInt_pri = qos_cfg.int_pri;
    *pDp = qos_cfg.dp;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_1pPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    rtk_pri_t   int_pri,
    uint32      dp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.dot1p_pri = dot1p_pri;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp = dp;
    SETSOCKOPT(RTDRV_QOS_1P_PRI_REMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_port1pPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pInnerPriGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_1P_PRI_REMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInnerPriGrp_idx = qos_cfg.index;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_port1pPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 innerPriGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = innerPriGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_1P_PRI_REMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pPriRemap_get(uint32 unit, rtk_pri_t dot1p_pri, uint32 dei, rtk_pri_t * pInt_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dei  = dei;
    qos_cfg.dot1p_pri = dot1p_pri;
    GETSOCKOPT(RTDRV_QOS_OUTER_1P_PRI_REMAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInt_pri = qos_cfg.int_pri;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pPriRemap_set(uint32 unit, rtk_pri_t dot1p_pri, uint32 dei, rtk_pri_t int_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.dei = dei;
    qos_cfg.dot1p_pri = dot1p_pri;
    qos_cfg.int_pri = int_pri;
    SETSOCKOPT(RTDRV_QOS_OUTER_1P_PRI_REMAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pPriRemapGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    uint32      dei,
    rtk_pri_t   *pInt_pri,
    uint32      *pDp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.dot1p_pri = dot1p_pri;
    qos_cfg.dei  = dei;
    GETSOCKOPT(RTDRV_QOS_OUTER_1P_PRI_REMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInt_pri = qos_cfg.int_pri;
    *pDp      = qos_cfg.dp;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pPriRemapGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   dot1p_pri,
    uint32      dei,
    rtk_pri_t   int_pri,
    uint32      dp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.dot1p_pri = dot1p_pri;
    qos_cfg.dei = dei;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp = dp;
    SETSOCKOPT(RTDRV_QOS_OUTER_1P_PRI_REMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pPriRemapGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuterPriGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_OUTER_1P_PRI_REMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pOuterPriGrp_idx = qos_cfg.index;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pPriRemapGroup_set(uint32 unit, rtk_port_t port, uint32 outerPriGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = outerPriGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_OUTER_1P_PRI_REMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portPriMap_get(uint32 unit, rtk_port_t port, rtk_pri_t pri, rtk_qid_t *pQueue)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.int_pri = pri;
    GETSOCKOPT(RTDRV_QOS_PORT_PRI_MAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pQueue = qos_cfg.queue;
    
    return RT_ERR_OK;    
}

int32 rtk_qos_portPriMap_set(uint32 unit, rtk_port_t port, rtk_pri_t pri, rtk_qid_t queue)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.int_pri = pri;
    qos_cfg.queue = queue;
    SETSOCKOPT(RTDRV_QOS_PORT_PRI_MAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_1P_REMARK_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.remark_enable;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.remark_enable = enable;
    SETSOCKOPT(RTDRV_QOS_1P_REMARK_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_DSCP_REMARK_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.remark_enable;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.remark_enable = enable;
    SETSOCKOPT(RTDRV_QOS_DSCP_REMARK_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_1pDfltPri_get(uint32 unit, rtk_pri_t * pDot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_1P_DFLT_PRI_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.dot1p_dflt_pri;

    return RT_ERR_OK;    
}

int32 rtk_qos_1pDfltPri_set(uint32 unit, rtk_pri_t dot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dot1p_dflt_pri = dot1p_pri;
    SETSOCKOPT(RTDRV_QOS_1P_DFLT_PRI_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemarkSrcSel_get(uint32 unit, rtk_qos_1pRmkSrc_t * pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_1P_REMARK_SRC_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.rmksrc_1p;

    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemarkSrcSel_set(uint32 unit, rtk_qos_1pRmkSrc_t type)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.rmksrc_1p = type;
    SETSOCKOPT(RTDRV_QOS_1P_REMARK_SRC_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemark_get(uint32 unit, rtk_pri_t int_pri, rtk_pri_t *pDot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.int_pri = int_pri;
    GETSOCKOPT(RTDRV_QOS_1P_REMARK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.dot1p_pri;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemark_set(uint32 unit, rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dot1p_pri = dot1p_pri;
    SETSOCKOPT(RTDRV_QOS_1P_REMARK_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   *pDot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp    = dp;
    GETSOCKOPT(RTDRV_QOS_1P_REMARK_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.dot1p_pri;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_1pRemarkGroup_set(
    uint32 unit,
    uint32 grp_idx,
    rtk_pri_t int_pri,
    uint32 dp,
    rtk_pri_t dot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp    = dp;
    qos_cfg.dot1p_pri = dot1p_pri;
    SETSOCKOPT(RTDRV_QOS_1P_REMARK_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_port1pRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pInner1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_1P_REMARK_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInner1pRemarkGrp_idx = qos_cfg.index;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_port1pRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 inner1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = inner1pRemarkGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_1P_REMARK_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_port1pPriMapGroup_get(uint32 unit, rtk_port_t port, uint32 *pInner1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_1P_PRIMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pInner1pRemarkGrp_idx = qos_cfg.index;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_port1pPriMapGroup_set(uint32 unit, rtk_port_t port, uint32 inner1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = inner1pRemarkGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_1P_PRIMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_out1pRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_OUT_1P_REMARK_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.remark_enable;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_out1pRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.remark_enable = enable;
    SETSOCKOPT(RTDRV_QOS_OUT_1P_REMARK_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pRemarkSrcSel_get(uint32 unit, rtk_qos_outer1pRmkSrc_t * pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_OUT_1P_REMARK_SRC_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.rmksrc_outer1p;

    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pRemarkSrcSel_set(uint32 unit, rtk_qos_outer1pRmkSrc_t type)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.rmksrc_outer1p = type;
    SETSOCKOPT(RTDRV_QOS_OUT_1P_REMARK_SRC_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   *pDot1p_pri,
    uint32      *pDei)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp      = dp;
    GETSOCKOPT(RTDRV_QOS_OUTER_1P_REMARK_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.dot1p_pri;
    *pDei       = qos_cfg.dei;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_outer1pRemarkGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    rtk_pri_t   dot1p_pri,
    uint32      dei)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp      = dp;
    qos_cfg.dot1p_pri   = dot1p_pri;
    qos_cfg.dei     = dei;
    SETSOCKOPT(RTDRV_QOS_OUT_1P_REMARK_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pDfltPriSrcSel_get(uint32 unit, rtk_port_t port, rtk_qos_outer1pDfltSrc_t *pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_OUTER_1P_DFLT_SRC_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.out1p_dflt_src;

    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pDfltPriSrcSel_set(uint32 unit, rtk_port_t port, rtk_qos_outer1pDfltSrc_t type)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.out1p_dflt_src = type;
    SETSOCKOPT(RTDRV_QOS_PORT_OUT_1P_DFLT_SRC_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuter1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_OUTER_1P_REMARK_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pOuter1pRemarkGrp_idx = qos_cfg.index;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 outer1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = outer1pRemarkGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_OUT_1P_REMARK_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pPriMapGroup_get(uint32 unit, rtk_port_t port, uint32 *pOuter1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_OUTER_1P_PRIMAP_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pOuter1pRemarkGrp_idx = qos_cfg.index;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_portOuter1pPriMapGroup_set(uint32 unit, rtk_port_t port, uint32 outer1pRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = outer1pRemarkGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_OUT_1P_PRIMAP_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemarkSrcSel_get(uint32 unit, rtk_qos_dscpRmkSrc_t * pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_DSCP_REMARK_SRC_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.rmksrc_dscp;

    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemarkSrcSel_set(uint32 unit, rtk_qos_dscpRmkSrc_t type)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.rmksrc_dscp = type;
    SETSOCKOPT(RTDRV_QOS_DSCP_REMARK_SRC_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemark_get(uint32 unit, rtk_pri_t int_pri, uint32 *pDscp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.int_pri = int_pri;
    GETSOCKOPT(RTDRV_QOS_DSCP_REMARK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDscp = qos_cfg.dscp;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemark_set(uint32 unit, rtk_pri_t int_pri, uint32 dscp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dscp = dscp;
    SETSOCKOPT(RTDRV_QOS_DSCP_REMARK_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscp2Dot1pRemark_get(uint32 unit, uint32 dscp, rtk_pri_t *pDot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.org_dscp = dscp;
    GETSOCKOPT(RTDRV_QOS_DSCP2DOT1P_REMARK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.dot1p_pri;
        
    return RT_ERR_OK;    
}
    
int32 rtk_qos_dscp2Dot1pRemark_set(uint32 unit, uint32 dscp, rtk_pri_t dot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.org_dscp = dscp;
    qos_cfg.dot1p_pri = dot1p_pri;
    SETSOCKOPT(RTDRV_QOS_DSCP2DOT1P_REMARK_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscp2Outer1pRemark_get(uint32 unit, uint32 dscp, rtk_pri_t *pDot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.org_dscp = dscp;
    GETSOCKOPT(RTDRV_QOS_DSCP2OUT1P_REMARK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.dot1p_pri;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_dscp2Outer1pRemark_set(uint32 unit, uint32 dscp, rtk_pri_t dot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.org_dscp = dscp;
    qos_cfg.dot1p_pri = dot1p_pri;
    SETSOCKOPT(RTDRV_QOS_DSCP2OUT1P_REMARK_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscp2DscpRemark_get(uint32 unit, uint32 dscp, uint32 *pDscp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.org_dscp = dscp;
    GETSOCKOPT(RTDRV_QOS_DSCP2DSCP_REMARK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDscp = qos_cfg.dscp;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_dscp2DscpRemark_set(uint32 unit, uint32 dscp, uint32 rmkDscp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.org_dscp = dscp;
    qos_cfg.dscp = rmkDscp;
    SETSOCKOPT(RTDRV_QOS_DSCP2DSCP_REMARK_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemarkGroup_get(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    uint32      *pDscp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp   = dp;
    GETSOCKOPT(RTDRV_QOS_DSCP_REMARK_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDscp = qos_cfg.dscp;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_dscpRemarkGroup_set(
    uint32      unit,
    uint32      grp_idx,
    rtk_pri_t   int_pri,
    uint32      dp,
    uint32      dscp)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.index = grp_idx;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dp   = dp;
    qos_cfg.dscp = dscp;
    SETSOCKOPT(RTDRV_QOS_DSCP_REMARK_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portdscpRemarkGroup_get(uint32 unit, rtk_port_t port, uint32 *pDscpRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_DSCP_REMARK_GROUP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDscpRemarkGrp_idx = qos_cfg.index;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_portdscpRemarkGroup_set(uint32 unit, rtk_port_t port, uint32 dscpRemarkGrp_idx)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.index = dscpRemarkGrp_idx;
    SETSOCKOPT(RTDRV_QOS_PORT_DSCP_REMARK_GROUP_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

#if 1
int32 rtk_qos_outer1pRemark_get(uint32 unit, rtk_pri_t int_pri, rtk_pri_t * pDot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.int_pri = int_pri;
    GETSOCKOPT(RTDRV_QOS_OUT_1P_REMARK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.dot1p_pri;
        
    return RT_ERR_OK;    
} 

int32 rtk_qos_outer1pRemark_set(uint32 unit, rtk_pri_t int_pri, rtk_pri_t dot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.int_pri = int_pri;
    qos_cfg.dot1p_pri = dot1p_pri;
    SETSOCKOPT(RTDRV_QOS_OUT_1P_REMARK_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_deiRemark_get(uint32 unit, uint32 dp, uint32 *pDei)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dp = dp;
    GETSOCKOPT(RTDRV_QOS_DEI_REMARK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDei = qos_cfg.dei;
        
    return RT_ERR_OK;    
} 

int32 rtk_qos_deiRemark_set(uint32 unit, uint32 dp, uint32 dei)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.dp = dp;
    qos_cfg.dei = dei;
    SETSOCKOPT(RTDRV_QOS_DEI_REMARK_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_deiRemarkEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_DEI_REMARK_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.remark_enable;
        
    return RT_ERR_OK;    
} 

int32 rtk_qos_deiRemarkEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.remark_enable = enable;
    SETSOCKOPT(RTDRV_QOS_DEI_REMARK_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portDEIRemarkTagSel_get(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t *pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_DEI_REMARK_TAG_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.deiSrc;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_portDEIRemarkTagSel_set(uint32 unit, rtk_port_t port, rtk_qos_deiSel_t type)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.deiSrc = type;
    SETSOCKOPT(RTDRV_QOS_PORT_DEI_REMARK_TAG_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}
#endif

int32 rtk_qos_schedulingAlgorithm_get(uint32 unit, rtk_port_t port, rtk_qos_scheduling_type_t *pScheduling_type)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_SCHEDULING_ALGORITHM_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pScheduling_type = qos_cfg.scheduling_type;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_schedulingAlgorithm_set(uint32 unit, rtk_port_t port, rtk_qos_scheduling_type_t scheduling_type)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.scheduling_type = scheduling_type;
    SETSOCKOPT(RTDRV_QOS_SCHEDULING_ALGORITHM_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_schedulingQueue_get(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_SCHEDULING_QUEUE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pQweights, &qos_cfg.qweights, sizeof(rtk_qos_queue_weights_t));
        
    return RT_ERR_OK;    
}

int32 rtk_qos_schedulingQueue_set(uint32 unit, rtk_port_t port, rtk_qos_queue_weights_t *pQweights)     
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    memcpy(&qos_cfg.qweights, pQweights, sizeof(rtk_qos_queue_weights_t));
    SETSOCKOPT(RTDRV_QOS_SCHEDULING_QUEUE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_wfqFixedBandwidthEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.queue   = queue;
    GETSOCKOPT(RTDRV_QOS_WFQ_FIXED_BANDWIDTH_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable    = qos_cfg.enable;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_wfqFixedBandwidthEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_qid_t       queue,
    rtk_enable_t    enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.queue   = queue;
    qos_cfg.enable  = enable;
    SETSOCKOPT(RTDRV_QOS_WFQ_FIXED_BANDWIDTH_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidAlgo_get(uint32 unit, rtk_qos_congAvoidAlgo_t *pAlgo)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_ALGO_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pAlgo    = qos_cfg.congAvoid_algo;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidAlgo_set(uint32 unit, rtk_qos_congAvoidAlgo_t algo)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.congAvoid_algo  = algo;
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_ALGO_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidQueueThreshEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable    = qos_cfg.enable;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidQueueThreshEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.enable  = enable;
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidPortThreshEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_PORT_THRESH_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable    = qos_cfg.enable;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidPortThreshEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.enable  = enable;
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_PORT_THRESH_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidSysThreshEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_SYS_THRESH_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable    = qos_cfg.enable;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidSysThreshEnable_set(uint32 unit,  rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.enable  = enable;
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_SYS_THRESH_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidSysThresh_get(
    uint32                      unit,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dp   = dp;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_SYS_THRESH_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pCongAvoidThresh, &(qos_cfg.congAvoid_thresh), sizeof(rtk_qos_congAvoidThresh_t));
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidSysThresh_set(
    uint32                      unit,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.dp      = dp;
    memcpy(&(qos_cfg.congAvoid_thresh), pCongAvoidThresh, sizeof(rtk_qos_congAvoidThresh_t));
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_SYS_THRESH_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidSysDropProbability_get(
    uint32  unit,
    uint32  dp,
    uint32  *pProbability)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dp   = dp;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_SYS_DROP_PROBABILITY_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pProbability = qos_cfg.data;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidSysDropProbability_set(
    uint32  unit,
    uint32  dp,
    uint32  probability)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.dp      = dp;
    qos_cfg.data    = probability;
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_SYS_DROP_PROBABILITY_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidPortThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.dp   = dp;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_PORT_THRESH_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pCongAvoidThresh, &(qos_cfg.congAvoid_thresh), sizeof(rtk_qos_congAvoidThresh_t));
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidPortThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.port    = port;
    qos_cfg.dp      = dp;
    memcpy(&(qos_cfg.congAvoid_thresh), pCongAvoidThresh, sizeof(rtk_qos_congAvoidThresh_t));
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_PORT_THRESH_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidQueueThresh_get(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.queue   = queue;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pCongAvoidThresh, &(qos_cfg.congAvoid_thresh), sizeof(rtk_qos_congAvoidThresh_t));
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidQueueThresh_set(
    uint32                      unit,
    rtk_port_t                  port,
    rtk_qid_t                   queue,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.port    = port;
    qos_cfg.queue      = queue;
    memcpy(&(qos_cfg.congAvoid_thresh), pCongAvoidThresh, sizeof(rtk_qos_congAvoidThresh_t));
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidGlobalQueueThresh_get(
    uint32                      unit,
    rtk_qid_t                   queue,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.queue = queue;
    qos_cfg.dp = dp;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_THRESH_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pCongAvoidThresh, &(qos_cfg.congAvoid_thresh), sizeof(rtk_qos_congAvoidThresh_t));
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidGlobalQueueThresh_set(
    uint32                      unit,
    rtk_qid_t                   queue,
    uint32                      dp,
    rtk_qos_congAvoidThresh_t   *pCongAvoidThresh)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.queue = queue;
    qos_cfg.dp = dp;
    memcpy(&(qos_cfg.congAvoid_thresh), pCongAvoidThresh, sizeof(rtk_qos_congAvoidThresh_t));
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_THRESH_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidGlobalQueueDropProbability_get(
    uint32      unit,
    rtk_qid_t   queue,
    uint32      dp,
    uint32      *pProbability)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.queue = queue;
    qos_cfg.dp = dp;
    GETSOCKOPT(RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_DROP_PROBABILITY_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pProbability = qos_cfg.data;
        
    return RT_ERR_OK;    
}

int32 rtk_qos_congAvoidGlobalQueueDropProbability_set(
    uint32      unit,
    rtk_qid_t   queue,
    uint32      dp,
    uint32      probability)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.queue = queue;
    qos_cfg.dp = dp;
    qos_cfg.data = probability;
    SETSOCKOPT(RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_DROP_PROBABILITY_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_wredSysThresh_get(uint32 unit, uint32 dp, rtk_qos_wredThresh_t *pThresh)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dp = dp;
    GETSOCKOPT(RTDRV_QOS_WRED_SYS_THRESH_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    memcpy(pThresh, &(qos_cfg.wred_thresh), sizeof(rtk_qos_wredThresh_t));
        
    return RT_ERR_OK;    
}

int32 rtk_qos_wredSysThresh_set(uint32 unit, uint32 dp, rtk_qos_wredThresh_t *pThresh)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.dp    = dp;
    memcpy(&(qos_cfg.wred_thresh), pThresh, sizeof(rtk_qos_wredThresh_t));
    SETSOCKOPT(RTDRV_QOS_WRED_SYS_THRESH_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_wredWeight_get(uint32 unit, uint32 *pWeight)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_WRED_WEIGHT_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pWeight = qos_cfg.data;
            
    return RT_ERR_OK;    
}

int32 rtk_qos_wredWeight_set(uint32 unit, uint32 weight)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.data    = weight;
    SETSOCKOPT(RTDRV_QOS_WEIGHT_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_wredMpd_get(uint32 unit, uint32 *pMpd)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_WRED_MPD_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pMpd = qos_cfg.data;
            
    return RT_ERR_OK;    
}

int32 rtk_qos_wredMpd_set(uint32 unit, uint32 mpd)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.data    = mpd;
    SETSOCKOPT(RTDRV_QOS_MPD_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_wredEcnEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_WRED_ECN_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;
            
    return RT_ERR_OK;    
}

int32 rtk_qos_wredEcnEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.enable    = enable;
    SETSOCKOPT(RTDRV_QOS_ECN_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_wredCntReverseEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_WRED_CNT_REVERSE_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;
            
    return RT_ERR_OK;    
}

int32 rtk_qos_wredCntReverseEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit    = unit;
    qos_cfg.enable    = enable;
    SETSOCKOPT(RTDRV_QOS_CNT_REVERSE_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 rtk_qos_portAvbStreamReservationClassEnable_get(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qos_avbSrClass_t    srClass,
    rtk_enable_t            *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port    = port;
    qos_cfg.srClass = srClass;
    GETSOCKOPT(RTDRV_QOS_AVB_SR_CLASS_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;
    
    return RT_ERR_OK;
}

int32 rtk_qos_portAvbStreamReservationClassEnable_set(
    uint32                  unit,
    rtk_port_t              port,
    rtk_qos_avbSrClass_t    srClass,
    rtk_enable_t            enable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port    = port;
    qos_cfg.srClass = srClass;
    qos_cfg.enable  = enable;
    SETSOCKOPT(RTDRV_QOS_AVB_SR_CLASS_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 rtk_qos_avbStreamReservationConfig_get(uint32 unit, rtk_qos_avbSrConf_t *pSrConf)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    GETSOCKOPT(RTDRV_QOS_AVB_SR_CONFIG_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pSrConf = qos_cfg.srConf;
    
    return RT_ERR_OK;
}

int32 rtk_qos_avbStreamReservationConfig_set(uint32 unit, rtk_qos_avbSrConf_t *pSrConf)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.srConf  = *pSrConf;
    SETSOCKOPT(RTDRV_QOS_AVB_SR_CONFIG_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;
}

int32 rtk_qos_pkt2CpuPriRemap_get(uint32 unit, rtk_pri_t intPri, rtk_pri_t *pNewPri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.int_pri = intPri;
    GETSOCKOPT(RTDRV_QOS_PKT2CPU_PRI_REMAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pNewPri = qos_cfg.new_pri;
    
    return RT_ERR_OK;
}

int32 rtk_qos_pkt2CpuPriRemap_set(uint32 unit, rtk_pri_t intPri, rtk_pri_t newPri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.int_pri = intPri;
    qos_cfg.new_pri = newPri;
    SETSOCKOPT(RTDRV_QOS_PKT2CPU_PRI_REMAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;
}

int32
rtk_qos_portPri2IgrQMap_get(uint32 unit, rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port = port; 
    GETSOCKOPT(RTDRV_QOS_PRI2IGR_QUEUE_MAP_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pPri2qid = qos_cfg.pri2qid;
    
    return RT_ERR_OK;
}

int32
rtk_qos_portPri2IgrQMap_set(uint32 unit, rtk_port_t port, rtk_qos_pri2queue_t *pPri2qid)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port = port;
    qos_cfg.pri2qid = *pPri2qid; 
    SETSOCKOPT(RTDRV_QOS_PRI2IGR_QUEUE_MAP_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
                         
    return RT_ERR_OK;
}


int32
rtk_qos_portPri2IgrQMapEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port = port; 
    GETSOCKOPT(RTDRV_QOS_PRI2IGR_QUEUE_MAP_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;
    
    return RT_ERR_OK;
}

int32
rtk_qos_portPri2IgrQMapEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port = port;
    qos_cfg.enable= enable;
    SETSOCKOPT(RTDRV_QOS_PRI2IGR_QUEUE_MAP_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
                         
    return RT_ERR_OK;
}


int32 
rtk_qos_igrQueueWeight_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, uint32 *pQweight)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port = port;
    qos_cfg.queue = queue;
    GETSOCKOPT(RTDRV_QOS_IGR_QUEUE_WEIGHT_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pQweight = qos_cfg.data;
    
    return RT_ERR_OK;
}


int32 
rtk_qos_igrQueueWeight_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, uint32 qWeight)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit    = unit;
    qos_cfg.port = port;
    qos_cfg.queue = queue;
    qos_cfg.data = qWeight;
    SETSOCKOPT(RTDRV_QOS_IGR_QUEUE_WEIGHT_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
                         
    return RT_ERR_OK;
}

int32
rtk_qos_1pDfltPriSrcSel_get(uint32 unit, rtk_qos_1pDfltPriSrc_t * pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_1P_DFLT_PRI_SRC_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.dflt_src_1p;

    return RT_ERR_OK;    
}

int32
rtk_qos_1pDfltPriSrcSel_set(uint32 unit, rtk_qos_1pDfltPriSrc_t type)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.dflt_src_1p = type;
    SETSOCKOPT(RTDRV_QOS_1P_DFLT_PRI_SRC_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_qos_portOuter1pRemarkSrcSel_get(uint32 unit, rtk_port_t port, rtk_qos_outer1pRmkSrc_t * pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_OUTER_1P_REMARK_SRC_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.rmksrc_outer1p;

    return RT_ERR_OK;    
}

int32
rtk_qos_portOuter1pRemarkSrcSel_set(uint32 unit, rtk_port_t port, rtk_qos_outer1pRmkSrc_t type)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.rmksrc_outer1p = type;
    SETSOCKOPT(RTDRV_QOS_PORT_OUTER_1P_REMARK_SRC_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_qos_outer1pDfltPri_get(uint32 unit, rtk_pri_t * pDot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_OUTER_1P_DFLT_PRI_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDot1p_pri = qos_cfg.out1p_dflt_pri;

    return RT_ERR_OK;    
}

int32
rtk_qos_outer1pDfltPri_set(uint32 unit, rtk_pri_t dot1p_pri)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.out1p_dflt_pri = dot1p_pri;
    SETSOCKOPT(RTDRV_QOS_OUTER_1P_DFLT_PRI_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_qos_outer1pDfltPriCfgSrcSel_get(uint32 unit, rtk_qos_outer1pDfltCfgSrc_t *pDflt_sel)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_OUTER_1P_DFLT_PRI_CFG_SRC_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDflt_sel = qos_cfg.out1p_dflt_cfg_dir;

    return RT_ERR_OK;    
}

int32
rtk_qos_outer1pDfltPriCfgSrcSel_set(uint32 unit, rtk_qos_outer1pDfltCfgSrc_t dflt_sel)
{
    rtdrv_qosCfg_t qos_cfg;

    qos_cfg.unit = unit;
    qos_cfg.out1p_dflt_cfg_dir = dflt_sel;
    SETSOCKOPT(RTDRV_QOS_OUTER_1P_DFLT_PRI_CFG_SRC_SET, &qos_cfg, rtdrv_qosCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
rtk_qos_invldDscpVal_get(uint32 unit, uint32 *pDscp) 
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_INVLD_DSCP_VAL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDscp = qos_cfg.dscp;

    return RT_ERR_OK;    
}

int32
rtk_qos_invldDscpVal_set(uint32 unit, uint32 dscp)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dscp = dscp;
    SETSOCKOPT(RTDRV_QOS_INVLD_DSCP_VAL_SET, &qos_cfg, rtdrv_qosCfg_t, 1);

    return RT_ERR_OK;    
}

int32
rtk_qos_invldDscpMask_get(uint32 unit, uint32 *pDscpMask) 
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_INVLD_DSCP_MASK_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pDscpMask = qos_cfg.dscp;

    return RT_ERR_OK;    
}

int32
rtk_qos_invldDscpMask_set(uint32 unit, uint32 dscpMask) 
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.dscp = dscpMask;
    SETSOCKOPT(RTDRV_QOS_INVLD_DSCP_MASK_SET, &qos_cfg, rtdrv_qosCfg_t, 1);

    return RT_ERR_OK;    
}

int32
rtk_qos_portInvldDscpEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    GETSOCKOPT(RTDRV_QOS_PORT_INVLD_DSCP_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;

    return RT_ERR_OK;    
}

int32
rtk_qos_portInvldDscpEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable) 
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.enable = enable;
    SETSOCKOPT(RTDRV_QOS_PORT_INVLD_DSCP_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32
rtk_qos_invldDscpEnable_get(uint32 unit, rtk_enable_t *pEnable) 
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_INVLD_DSCP_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;

    return RT_ERR_OK;    
}

int32
rtk_qos_invldDscpEnable_set(uint32 unit, rtk_enable_t enable) 
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.enable = enable;
    SETSOCKOPT(RTDRV_QOS_INVLD_DSCP_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 
rtk_qos_portPriRemapEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_PORT_PRI_REMAP_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;

    return RT_ERR_OK;    
}

int32 
rtk_qos_portPriRemapEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.enable = enable;
    SETSOCKOPT(RTDRV_QOS_PORT_PRI_REMAP_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 
rtk_qos_portPriRemapSel_get(uint32 unit, rtk_qos_portPriRemapSel_t *pType)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_PORT_PRI_REMAP_SEL_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pType = qos_cfg.portPriRemap_type;

    return RT_ERR_OK;    
}

int32 
rtk_qos_portPriRemapSel_set(uint32 unit, rtk_qos_portPriRemapSel_t type)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.portPriRemap_type = type;
    SETSOCKOPT(RTDRV_QOS_PORT_PRI_REMAP_SEL_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 
rtk_qos_portInnerPriRemapEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_PORT_IPRI_REMAP_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;

    return RT_ERR_OK;    
}

int32 
rtk_qos_portInnerPriRemapEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.enable = enable;
    SETSOCKOPT(RTDRV_QOS_PORT_IPRI_REMAP_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 
rtk_qos_portOuterPriRemapEnable_get(uint32 unit, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    GETSOCKOPT(RTDRV_QOS_PORT_OPRI_REMAP_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;

    return RT_ERR_OK;    
}

int32 
rtk_qos_portOuterPriRemapEnable_set(uint32 unit, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.enable = enable;
    SETSOCKOPT(RTDRV_QOS_PORT_OPRI_REMAP_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}

int32 
rtk_qos_queueStrictEnable_get(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t *pEnable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.queue = queue;
    GETSOCKOPT(RTDRV_QOS_QUEUE_STRICT_ENABLE_GET, &qos_cfg, rtdrv_qosCfg_t, 1);
    *pEnable = qos_cfg.enable;

    return RT_ERR_OK;    
}


int32 
rtk_qos_queueStrictEnable_set(uint32 unit, rtk_port_t port, rtk_qid_t queue, rtk_enable_t enable)
{
    rtdrv_qosCfg_t qos_cfg;
    
    qos_cfg.unit = unit;
    qos_cfg.port = port;
    qos_cfg.queue = queue;
    qos_cfg.enable = enable;
    SETSOCKOPT(RTDRV_QOS_QUEUE_STRICT_ENABLE_SET, &qos_cfg, rtdrv_qosCfg_t, 1);
    
    return RT_ERR_OK;    
}


