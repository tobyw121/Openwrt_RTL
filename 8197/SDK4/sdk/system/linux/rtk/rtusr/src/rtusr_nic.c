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
 * $Revision: 13309 $
 * $Date: 2010-10-13 13:27:11 +0800 (Wed, 13 Oct 2010) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) NIC
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>

int32 drv_nic_rx_start(uint32 unit)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    SETSOCKOPT(RTDRV_NIC_RX_START, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;    
}

int32 drv_nic_rx_stop(uint32 unit)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    SETSOCKOPT(RTDRV_NIC_RX_STOP, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;    
}

#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
int32
drv_nic_pieCpuEntry_add(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry)
{
    rtdrv_nicCfg_t nic_cfg;

    memset(&nic_cfg, 0, sizeof(rtdrv_nicCfg_t));
    nic_cfg.unit = unit;
    nic_cfg.index = entry_idx;
    memcpy(&nic_cfg.cpu_entry, pCpuEntry, sizeof(drv_nic_CpuEntry_t));
    SETSOCKOPT(RTDRV_NIC_CPU_ENTRY_ADD, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;
} /* end of drv_nic_pieCpuEntry_add */

int32
drv_nic_pieCpuEntry_del(uint32 unit, uint32 entry_idx)
{
    rtdrv_nicCfg_t nic_cfg;

    memset(&nic_cfg, 0, sizeof(rtdrv_nicCfg_t));
    nic_cfg.unit = unit;
    nic_cfg.index = entry_idx;
    SETSOCKOPT(RTDRV_NIC_CPU_ENTRY_DEL, &nic_cfg, rtdrv_nicCfg_t, 1);

    return RT_ERR_OK;
} /* end of drv_nic_pieCpuEntry_del */

int32
drv_nic_pieCpuEntry_get(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry)
{
    rtdrv_nicCfg_t nic_cfg;
    
    memset(&nic_cfg, 0, sizeof(rtdrv_nicCfg_t));
    nic_cfg.unit = unit;
    nic_cfg.index = entry_idx;
    GETSOCKOPT(RTDRV_NIC_CPU_ENTRY_GET, &nic_cfg, rtdrv_nicCfg_t, 1);
    memcpy(pCpuEntry, &nic_cfg.cpu_entry, sizeof(drv_nic_CpuEntry_t));
    
    return RT_ERR_OK;  
} /* end of drv_nic_pieCpuEntry_get */

int32
drv_nic_pieCpuEntry_set(uint32 unit, uint32 entry_idx, drv_nic_CpuEntry_t *pCpuEntry)
{
    rtdrv_nicCfg_t nic_cfg;

    memset(&nic_cfg, 0, sizeof(rtdrv_nicCfg_t));
    nic_cfg.unit = unit;
    nic_cfg.index = entry_idx;
    memcpy(&nic_cfg.cpu_entry, pCpuEntry, sizeof(drv_nic_CpuEntry_t));
    SETSOCKOPT(RTDRV_NIC_CPU_ENTRY_SET, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;
} /* end of drv_nic_pieCpuEntry_set */
#endif
