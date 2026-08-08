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
 * $Revision: 48962 $
 * $Date: 2014-07-01 10:17:05 +0800 (Tue, 01 Jul 2014) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file constitute all functions which is not call RTK layer APIs
 *           1) register APIs
 *           2) nic debug APIs
 *           3) external GPIO APIs
 *           4) rtl8231 APIs
 *           5) internal GPIO APIs
 *
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <rtusr_util.h>
#include <rtdrv/rtdrv_netfilter.h>
#include <drv/gpio/generalCtrl_gpio.h>


/* Switch Register APIs */
int32 ioal_mem32_read(uint32 unit, uint32 reg, uint32 *pValue)
{
    rtdrv_regCfg_t reg_cfg;
    
    reg_cfg.unit = unit;
    reg_cfg.reg = reg;
    
    GETSOCKOPT(RTDRV_REG_REGISTER_GET, &reg_cfg, rtdrv_regCfg_t, 1);
    *pValue = reg_cfg.value;
    
    return RT_ERR_OK;
} /* end of ioal_mem32_read */

int32 ioal_mem32_write(uint32 unit, uint32 reg, uint32 value)
{
    rtdrv_regCfg_t reg_cfg;

    reg_cfg.unit = unit;
    reg_cfg.reg = reg;
    reg_cfg.value = value;
    SETSOCKOPT(RTDRV_REG_REGISTER_SET, &reg_cfg, rtdrv_regCfg_t, 1); 

    return RT_ERR_OK;    
} /* end of ioal_mem32_write */

int32 reg_idx2Addr_get(uint32 unit, uint32 regIdx, uint32 *pAddr)
{
    rtdrv_regCfg_t reg_cfg;
    
    reg_cfg.unit = unit;
    reg_cfg.reg = regIdx;
    
    GETSOCKOPT(RTDRV_REG_IDX2ADDR_GET, &reg_cfg, rtdrv_regCfg_t, 1);
    *pAddr = reg_cfg.value;
    
    return RT_ERR_OK;    
} /* end of reg_idx2Addr_get */

int32 reg_idxMax_get(uint32 unit, uint32 *pMax)
{
    rtdrv_regCfg_t reg_cfg;
    
    reg_cfg.unit = unit;
    
    GETSOCKOPT(RTDRV_REG_IDXMAX_GET, &reg_cfg, rtdrv_regCfg_t, 1);
    *pMax = reg_cfg.value;
    
    return RT_ERR_OK;    
} /* end of reg_idx2Addr_get */

/* Any Register APIs */
int32 debug_mem_read(uint32 unit, uint32 addr, uint32 *pValue)
{
    rtdrv_regCfg_t reg_cfg;
    
    reg_cfg.unit = unit;
    reg_cfg.reg = addr;

    GETSOCKOPT(RTDRV_DEBUG_MEM_READ, &reg_cfg, rtdrv_regCfg_t, 1);
    *pValue = reg_cfg.value;
    
    return RT_ERR_OK;
} /* end of debug_mem_read */

int32 debug_mem_write(uint32 unit, uint32 addr, uint32 value)
{
    rtdrv_regCfg_t reg_cfg;

    reg_cfg.unit = unit;
    reg_cfg.reg = addr;
    reg_cfg.value = value;
    SETSOCKOPT(RTDRV_DEBUG_MEM_WRITE, &reg_cfg, rtdrv_regCfg_t, 1); 

    return RT_ERR_OK;    
} /* end of debug_mem_write */

/* NIC Debug APIs */
int32 drv_nic_dbg_get(uint32 unit, uint32 *pFlags)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    GETSOCKOPT(RTDRV_NIC_DEBUG_GET, &nic_cfg, rtdrv_nicCfg_t, 1);
    *pFlags = nic_cfg.flags;
    
    return RT_ERR_OK;    
} /* end of drv_nic_dbg_get */

int32 drv_nic_dbg_set(uint32 unit, uint32 flags)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    nic_cfg.flags = flags;
    SETSOCKOPT(RTDRV_NIC_DEBUG_SET, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;    
} /* end of drv_nic_dbg_set */

int32 drv_nic_cntr_dump(uint32 unit)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    SETSOCKOPT(RTDRV_NIC_COUNTER_DUMP, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;    
} /* end of drv_nic_cntr_dump */

int32 drv_nic_cntr_clear(uint32 unit)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    SETSOCKOPT(RTDRV_NIC_COUNTER_CLEAR, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;    
} /* end of drv_nic_cntr_clear */

int32 drv_nic_ringbuf_dump(uint32 unit)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    SETSOCKOPT(RTDRV_NIC_BUFFER_DUMP, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;    
} /* end of drv_nic_ringbuf_dump */

int32 drv_nic_pkthdr_mbuf_dump(uint32 unit, uint32 mode, uint32 start, uint32 end, uint32 flags)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    nic_cfg.mode = mode;
    nic_cfg.start = start;
    nic_cfg.end = end;
    nic_cfg.flags = flags;
    SETSOCKOPT(RTDRV_NIC_PKTHDR_MBUF_DUMP, &nic_cfg, rtdrv_nicCfg_t, 1); 

    return RT_ERR_OK;    
} /* end of drv_nic_pkthdr_mbuf_dump */

int32 drv_nic_rx_status_get(uint32 unit, uint32 *pStatus)
{
    rtdrv_nicCfg_t nic_cfg;

    nic_cfg.unit = unit;
    GETSOCKOPT(RTDRV_NIC_RX_STATUS_GET, &nic_cfg, rtdrv_nicCfg_t, 1);
    *pStatus = nic_cfg.rx_status;

    return RT_ERR_OK;    
} /* end of drv_nic_rx_status_get */

int32
table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    rtdrv_tblCfg_t tbl_cfg;

    tbl_cfg.unit = unit;
    tbl_cfg.table = table;
    tbl_cfg.addr = addr;
    memcpy(tbl_cfg.value, pData, 20*sizeof(uint32));
    SETSOCKOPT(RTDRV_TABLE_WRITE, &tbl_cfg, rtdrv_tblCfg_t, 1); 

    return RT_ERR_OK;    
}

int32
table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData)
{
    rtdrv_tblCfg_t tbl_cfg;
    
    tbl_cfg.unit = unit;
    tbl_cfg.table = table;
    tbl_cfg.addr = addr;

    GETSOCKOPT(RTDRV_TABLE_READ, &tbl_cfg, rtdrv_tblCfg_t, 1);
    
    memcpy(pData, tbl_cfg.value, 20*sizeof(uint32));
    
    return RT_ERR_OK;
} /* end of table_read */

int32 reg_info_get(uint32 unit, uint32 regIdx, rtk_reg_info_t *pData)
{
    rtdrv_regCfg_t reg_cfg;

    reg_cfg.unit = unit;
    reg_cfg.reg = regIdx;

    GETSOCKOPT(RTDRV_REG_INFO_GET, &reg_cfg, rtdrv_regCfg_t, 1);
    memcpy(pData, &reg_cfg.data, sizeof(rtk_reg_info_t));

    return RT_ERR_OK;
} /* end of reg_info_get */

#if defined(CONFIG_SDK_RTL8231)
int32 
drv_rtl8231_i2c_read(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 *pData)
{
    rtdrv_rtl8231Cfg_t rtl8231_cfg;

    rtl8231_cfg.unit = unit;
    rtl8231_cfg.phyId_or_slaveAddr = slave_addr;
    rtl8231_cfg.reg_addr = reg_addr;
    GETSOCKOPT(RTDRV_RTL8231_I2C_READ, &rtl8231_cfg, rtdrv_rtl8231Cfg_t, 1);
    *pData = rtl8231_cfg.data;

    return RT_ERR_OK;
}

int32 
drv_rtl8231_i2c_write(uint32 unit, uint32 slave_addr, uint32 reg_addr, uint32 data)
{
    rtdrv_rtl8231Cfg_t rtl8231_cfg;

    rtl8231_cfg.unit = unit;
    rtl8231_cfg.phyId_or_slaveAddr = slave_addr;
    rtl8231_cfg.reg_addr = reg_addr;
    rtl8231_cfg.data = data;
    SETSOCKOPT(RTDRV_RTL8231_I2C_WRITE, &rtl8231_cfg, rtdrv_rtl8231Cfg_t, 1); 

    return RT_ERR_OK;
}

int32 
drv_rtl8231_mdc_read(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 *pData)
{
    rtdrv_rtl8231Cfg_t rtl8231_cfg;

    rtl8231_cfg.unit = unit;
    rtl8231_cfg.phyId_or_slaveAddr = phy_id;
    rtl8231_cfg.page = page;
    rtl8231_cfg.reg_addr = reg_addr;
    GETSOCKOPT(RTDRV_RTL8231_MDC_READ, &rtl8231_cfg, rtdrv_rtl8231Cfg_t, 1);
    *pData = rtl8231_cfg.data;

    return RT_ERR_OK;
}

int32 
drv_rtl8231_mdc_write(uint32 unit, uint32 phy_id, uint32 page, uint32 reg_addr, uint32 data)
{
    rtdrv_rtl8231Cfg_t rtl8231_cfg;

    rtl8231_cfg.unit = unit;
    rtl8231_cfg.phyId_or_slaveAddr = phy_id;
    rtl8231_cfg.page = page;
    rtl8231_cfg.reg_addr = reg_addr;
    rtl8231_cfg.data = data;
    SETSOCKOPT(RTDRV_RTL8231_MDC_WRITE, &rtl8231_cfg, rtdrv_rtl8231Cfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_devReady_get(uint32 unit, uint32 dev, uint32 *pIsReady)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    GETSOCKOPT(RTDRV_EXTGPIO_DEV_READY_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pIsReady = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_dev_get(uint32 unit, uint32 dev, drv_extGpio_devConf_t *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    GETSOCKOPT(RTDRV_EXTGPIO_DEV_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pData = extGpio_cfg.extGpio_devConfData;

    return RT_ERR_OK;
}

int32
drv_extGpio_dev_init(uint32 unit, uint32 dev, drv_extGpio_devConf_t *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.extGpio_devConfData = *pData;
    SETSOCKOPT(RTDRV_EXTGPIO_DEV_INIT, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_devEnable_get(uint32 unit, uint32 dev, rtk_enable_t *pEnable)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    GETSOCKOPT(RTDRV_EXTGPIO_DEV_ENABLE_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pEnable = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_devEnable_set(uint32 unit, uint32 dev, rtk_enable_t enable)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.data = enable;
    SETSOCKOPT(RTDRV_EXTGPIO_DEV_ENABLE_SET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_syncEnable_get(uint32 unit, uint32 dev, rtk_enable_t *pEnable)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    GETSOCKOPT(RTDRV_EXTGPIO_SYNC_ENABLE_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pEnable = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_syncEnable_set(uint32 unit, uint32 dev, rtk_enable_t enable)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.data = enable;
    SETSOCKOPT(RTDRV_EXTGPIO_SYNC_ENABLE_SET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_syncStatus_get(uint32 unit, uint32 dev, uint32 *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    GETSOCKOPT(RTDRV_EXTGPIO_SYNC_STATUS_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pData = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_i2c_read(uint32 unit, uint32 dev, uint32 reg, uint32 *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.reg = reg;
    GETSOCKOPT(RTDRV_EXTGPIO_I2C_READ, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pData = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_i2c_write(uint32 unit, uint32 dev, uint32 reg, uint32 data)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.reg = reg;
    extGpio_cfg.data = data;
    SETSOCKOPT(RTDRV_EXTGPIO_I2C_WRITE, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);

    return RT_ERR_OK;
}

int32
drv_extGpio_i2c_init(uint32 unit, uint32 dev, uint32 i2c_clock, uint32 i2c_data)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.gpioId = i2c_clock;
    extGpio_cfg.data = i2c_data;
    SETSOCKOPT(RTDRV_EXTGPIO_I2C_INIT, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_sync_start(uint32 unit, uint32 dev)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    SETSOCKOPT(RTDRV_EXTGPIO_SYNC_START, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_pin_get(uint32 unit, uint32 dev, uint32 gpioId, drv_extGpio_conf_t *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.gpioId = gpioId;
    GETSOCKOPT(RTDRV_EXTGPIO_PIN_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pData = extGpio_cfg.extGpio_confData;

    return RT_ERR_OK;
}

int32
drv_extGpio_pin_init(uint32 unit, uint32 dev, uint32 gpioId, drv_extGpio_conf_t *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.gpioId = gpioId;
    extGpio_cfg.extGpio_confData = *pData;
    SETSOCKOPT(RTDRV_EXTGPIO_PIN_INIT, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_dataBit_get(uint32 unit, uint32 dev, uint32 gpioId, uint32 *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.gpioId = gpioId;
    GETSOCKOPT(RTDRV_EXTGPIO_DATABIT_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pData = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_dataBit_set(uint32 unit, uint32 dev, uint32 gpioId, uint32 data)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.gpioId = gpioId;
    extGpio_cfg.data = data;
    SETSOCKOPT(RTDRV_EXTGPIO_DATABIT_SET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_reg_read(uint32 unit, uint32 dev, uint32 reg, uint32 *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.reg = reg;
    GETSOCKOPT(RTDRV_EXTGPIO_REG_READ, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pData = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_reg_write(uint32 unit, uint32 dev, uint32 reg, uint32 data)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.reg = reg;
    extGpio_cfg.data = data;
    SETSOCKOPT(RTDRV_EXTGPIO_REG_WRITE, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extGpio_direction_get(uint32 unit, uint32 dev, uint32 gpioId, drv_gpio_direction_t *pData)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.gpioId = gpioId;
    GETSOCKOPT(RTDRV_EXTGPIO_DIRECTION_GET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1);
    *pData = extGpio_cfg.data;

    return RT_ERR_OK;
}

int32
drv_extGpio_direction_set(uint32 unit, uint32 dev, uint32 gpioId, drv_gpio_direction_t data)
{
    rtdrv_extGpioCfg_t extGpio_cfg;

    extGpio_cfg.unit = unit;
    extGpio_cfg.dev = dev;
    extGpio_cfg.gpioId = gpioId;
    extGpio_cfg.data = data;
    SETSOCKOPT(RTDRV_EXTGPIO_DIRECTION_SET, &extGpio_cfg, rtdrv_extGpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_extSmi_dev_init(uint32 unit, uint32 periferal, uint32 phyAddrSCK, uint32 phyAddrSDA, uint32 gpioIdSCK, uint32 gpioIdSDA)
{
    rtdrv_extSmiCfg_t extSmi_cfg;

    extSmi_cfg.unit = unit;
    extSmi_cfg.periferal = periferal;
    extSmi_cfg.phyAddrSCK = phyAddrSCK;
    extSmi_cfg.phyAddrSDA = phyAddrSDA;
    extSmi_cfg.gpioIdSCK = gpioIdSCK;
    extSmi_cfg.gpioIdSDA = gpioIdSDA;
    SETSOCKOPT(RTDRV_EXT_SMI_INIT, &extSmi_cfg, rtdrv_extSmiCfg_t, 1);

    return RT_ERR_OK;
}

int32
drv_extSmi_read(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 *pRdata)
{
    rtdrv_extSmiCfg_t extSmi_cfg;

    extSmi_cfg.unit = unit;
    extSmi_cfg.periferal = periferal;
    extSmi_cfg.addrs = mAddrs;
    GETSOCKOPT(RTDRV_EXT_SMI_READ, &extSmi_cfg, rtdrv_extSmiCfg_t, 1);
    *pRdata = extSmi_cfg.rdata;

    return RT_ERR_OK;
}

int32
drv_extSmi_write(uint32 unit, uint32 periferal, uint32 mAddrs, uint32 wData)
{
    rtdrv_extSmiCfg_t extSmi_cfg;

    extSmi_cfg.unit = unit;
    extSmi_cfg.periferal = periferal;
    extSmi_cfg.addrs = mAddrs;
    extSmi_cfg.rdata = wData;
    SETSOCKOPT(RTDRV_EXT_SMI_WRITE, &extSmi_cfg, rtdrv_extSmiCfg_t, 1);

    return RT_ERR_OK;
}

#endif

int32 
drv_gpio_init(
    gpioID gpioId, 
    drv_gpio_control_t function,
    drv_gpio_direction_t direction,
    drv_gpio_interruptType_t interruptEnable)
{
    rtdrv_gpioCfg_t gpio_cfg;

    gpio_cfg.gpioId = gpioId;
    gpio_cfg.function = function;
    gpio_cfg.direction = direction;
    gpio_cfg.interruptEnable = interruptEnable;
    SETSOCKOPT(RTDRV_GPIO_PIN_INIT, &gpio_cfg, rtdrv_gpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32 drv_gpio_dataBit_init(gpioID gpioId, uint32 data)
{
    rtdrv_gpioCfg_t gpio_cfg;

    gpio_cfg.gpioId = gpioId;
    gpio_cfg.data = data;
    SETSOCKOPT(RTDRV_GPIO_DATABIT_INIT, &gpio_cfg, rtdrv_gpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32 
drv_gpio_dataBit_get(gpioID gpioId, uint32 *pData)
{
    rtdrv_gpioCfg_t gpio_cfg;

    gpio_cfg.gpioId = gpioId;
    GETSOCKOPT(RTDRV_GPIO_DATABIT_GET, &gpio_cfg, rtdrv_gpioCfg_t, 1);
    *pData = gpio_cfg.data;

    return RT_ERR_OK;
}

int32 
drv_gpio_dataBit_set(gpioID gpioId, uint32 data)
{
    rtdrv_gpioCfg_t gpio_cfg;

    gpio_cfg.gpioId = gpioId;
    gpio_cfg.data = data;
    SETSOCKOPT(RTDRV_GPIO_DATABIT_SET, &gpio_cfg, rtdrv_gpioCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_generalCtrlGPIO_dev_init(uint32 unit,
	drv_generalCtrlGpio_devId_t dev,
	uint32 pinId, 
	drv_generalCtrlGpio_devConf_t *pData)
{
	rtdrv_generalCtrlGpioCfg_t generalCtrl_gpio_cfg;

	generalCtrl_gpio_cfg.unit = unit;
	generalCtrl_gpio_cfg.dev = dev;
	generalCtrl_gpio_cfg.gpioId = pinId;	

	generalCtrl_gpio_cfg.genCtrl_gpioDev.ext_gpio.access_mode = pData->ext_gpio.access_mode;
	generalCtrl_gpio_cfg.genCtrl_gpioDev.ext_gpio.address= pData->ext_gpio.address;

    SETSOCKOPT(RTDRV_GENCTRL_GPIO_DEV_INIT, &generalCtrl_gpio_cfg, rtdrv_generalCtrlGpioCfg_t, 1); 
		
    return RT_ERR_OK;

}


int32 
drv_generalCtrlGPIO_dataBit_set(uint32 unit,	drv_generalCtrlGpio_devId_t dev, uint32 pinId, uint32 data)
{
	rtdrv_generalCtrlGpioCfg_t generalCtrl_gpio_cfg;

	generalCtrl_gpio_cfg.unit = unit;
	generalCtrl_gpio_cfg.dev = dev;
	generalCtrl_gpio_cfg.gpioId = pinId;
	generalCtrl_gpio_cfg.data = data;

    SETSOCKOPT(RTDRV_GENCTRL_GPIO_DATABIT_SET, &generalCtrl_gpio_cfg, rtdrv_generalCtrlGpioCfg_t, 1); 
		
    return RT_ERR_OK;

}

int32 
drv_generalCtrlGPIO_dataBit_get(uint32 unit,	drv_generalCtrlGpio_devId_t dev, uint32 pinId, uint32 *data)
{
	rtdrv_generalCtrlGpioCfg_t generalCtrl_gpio_cfg;

	generalCtrl_gpio_cfg.unit = unit;
	generalCtrl_gpio_cfg.dev = dev;
	generalCtrl_gpio_cfg.gpioId = pinId;

    GETSOCKOPT(RTDRV_GENCTRL_GPIO_DATABIT_GET, &generalCtrl_gpio_cfg, rtdrv_generalCtrlGpioCfg_t, 1); 

	*data = generalCtrl_gpio_cfg.data;
		
    return RT_ERR_OK;

}


int32 
drv_generalCtrlGPIO_pin_init( 
	uint32 unit,
	drv_generalCtrlGpio_devId_t dev,
    uint32 pinId, 
    drv_generalCtrlGpio_pinConf_t *pData)
{
	rtdrv_generalCtrlGpioCfg_t generalCtrl_gpio_cfg;

	generalCtrl_gpio_cfg.unit = unit;
	generalCtrl_gpio_cfg.dev = dev;
	generalCtrl_gpio_cfg.gpioId = pinId;	

	generalCtrl_gpio_cfg.genCtrl_gpioDev.direction = pData->direction;
	generalCtrl_gpio_cfg.genCtrl_gpioDev.default_value = pData->default_value;

	generalCtrl_gpio_cfg.genCtrl_gpioPin.direction = pData->direction;
	
	generalCtrl_gpio_cfg.genCtrl_gpioPin.int_gpio.function = pData->int_gpio.function;
	generalCtrl_gpio_cfg.genCtrl_gpioPin.int_gpio.interruptEnable= pData->int_gpio.interruptEnable;

	generalCtrl_gpio_cfg.genCtrl_gpioPin.ext_gpio.direction = pData->direction;
	generalCtrl_gpio_cfg.genCtrl_gpioPin.ext_gpio.debounce = 0;
	generalCtrl_gpio_cfg.genCtrl_gpioPin.ext_gpio.inverter = 0;


    SETSOCKOPT(RTDRV_GENCTRL_GPIO_PIN_INIT, &generalCtrl_gpio_cfg, rtdrv_generalCtrlGpioCfg_t, 1); 
		
    return RT_ERR_OK;


}


int32 
drv_generalCtrlGPIO_devEnable_set(uint32 unit, drv_generalCtrlGpio_devId_t dev, rtk_enable_t enable)
{
	rtdrv_generalCtrlGpioCfg_t generalCtrl_gpio_cfg;

	generalCtrl_gpio_cfg.unit = unit;
	generalCtrl_gpio_cfg.dev = dev;
	generalCtrl_gpio_cfg.data = enable;

    SETSOCKOPT(RTDRV_GENCTRL_GPIO_DEV_ENABLE, &generalCtrl_gpio_cfg, rtdrv_generalCtrlGpioCfg_t, 1); 
	
    return RT_ERR_OK;


}


int32
drv_smi_init(uint32 portSCK, uint32 pinSCK, uint32 portSDA, uint32 pinSDA, uint32 dev)
{
    rtdrv_smiCfg_t smi_cfg;

    smi_cfg.portSCK = portSCK;
    smi_cfg.pinSCK = pinSCK;
    smi_cfg.portSDA = portSDA;
    smi_cfg.pinSDA = pinSDA;
    smi_cfg.dev = dev;
    SETSOCKOPT(RTDRV_SMI_INIT, &smi_cfg, rtdrv_smiCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_smi_group_get(uint32 * pPortSCK, uint32 * pPinSCK, uint32 * pPortSDA, uint32 * pPinSDA, uint32 dev)
{
    rtdrv_smiCfg_t smi_cfg;
    smi_cfg.dev = dev;
    GETSOCKOPT(RTDRV_SMI_GROUP_GET, &smi_cfg, rtdrv_smiCfg_t, 1); 
    *pPortSCK = smi_cfg.portSCK;
    *pPinSCK = smi_cfg.pinSCK;
    *pPortSDA = smi_cfg.portSDA;
    *pPinSDA = smi_cfg.pinSDA;

    return RT_ERR_OK;
}

int32
drv_smi_type_set(uint32 type, uint32 chipid, uint32 delay, uint32 dev)
{
    rtdrv_smiCfg_t smi_cfg;

    smi_cfg.type = type;
    smi_cfg.chipid = chipid;
    smi_cfg.delay = delay;
    smi_cfg.dev = dev;
    SETSOCKOPT(RTDRV_SMI_TYPE_SET, &smi_cfg, rtdrv_smiCfg_t, 1); 

    return RT_ERR_OK;
}

int32
drv_smi_type_get(uint32 * ptype, uint32 * pchipid, uint32 * pdelay, uint32 dev)
{
    rtdrv_smiCfg_t smi_cfg;

    smi_cfg.dev = dev;
    GETSOCKOPT(RTDRV_SMI_TYPE_GET, &smi_cfg, rtdrv_smiCfg_t, 1); 
    *ptype = smi_cfg.type;
    *pchipid = smi_cfg.chipid;
    *pdelay = smi_cfg.delay;

    return RT_ERR_OK;
}

int32
drv_smi_read(uint32 mAddrs, uint32 *pRdata, uint32 dev)
{
    rtdrv_smiCfg_t smi_cfg;

    smi_cfg.dev = dev;
    smi_cfg.addrs = mAddrs;
    GETSOCKOPT(RTDRV_SMI_READ, &smi_cfg, rtdrv_smiCfg_t, 1); 
    *pRdata = smi_cfg.rdata;

    return RT_ERR_OK;
}

int32
drv_smi_write(uint32 mAddrs, uint32 wData, uint32 dev)
{
    rtdrv_smiCfg_t smi_cfg;

    smi_cfg.dev = dev;
    smi_cfg.addrs = mAddrs;
    smi_cfg.rdata = wData;
    SETSOCKOPT(RTDRV_SMI_WRITE, &smi_cfg, rtdrv_smiCfg_t, 1); 

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_diag_portMacRemoteLoopbackEnable_get
 * Description:
 *      Get the mac remote loopback enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac remote loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) The mac remote loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
rtk_diag_portMacRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_diagCfg_t diag_cfg;

    diag_cfg.unit = unit;
    diag_cfg.port = port;   
    GETSOCKOPT(RTDRV_DIAG_MAC_REMOTE_LOOPBACK_GET, &diag_cfg, rtdrv_diagCfg_t, 1);    
    *pEnable = diag_cfg.enable;

    return RT_ERR_OK;  
} /* end of rtk_diag_portMacRemoteLoopbackEnable_get */


/* Function Name:
 *      rtk_diag_portMacRemoteLoopbackEnable_set
 * Description:
 *      Set the mac remote loopback enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of mac remote loopback
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
 *      (1) The mac remote loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
rtk_diag_portMacRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_diagCfg_t diag_cfg;

    diag_cfg.unit = unit;
    diag_cfg.port = port;   
    diag_cfg.enable = enable;
    SETSOCKOPT(RTDRV_DIAG_MAC_REMOTE_LOOPBACK_SET, &diag_cfg, rtdrv_diagCfg_t, 1); 

    return RT_ERR_OK;
} /* end of rtk_diag_portMacRemoteLoopbackEnable_set */

/* Function Name:
 *      rtk_diag_portMacLocalLoopbackEnable_get
 * Description:
 *      Get the mac local loopback enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac local loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      (1) The mac local loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
rtk_diag_portMacLocalLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    rtdrv_diagCfg_t diag_cfg;

    diag_cfg.unit = unit;
    diag_cfg.port = port;   
    GETSOCKOPT(RTDRV_DIAG_MAC_LOCAL_LOOPBACK_GET, &diag_cfg, rtdrv_diagCfg_t, 1);    
    *pEnable = diag_cfg.enable;

    return RT_ERR_OK;  
} /* end of rtk_diag_portMacLocalLoopbackEnable_get */

/* Function Name:
 *      rtk_diag_portMacLocalLoopbackEnable_set
 * Description:
 *      Set the mac local loopback enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of mac local loopback
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
 *      (1) The mac local loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
rtk_diag_portMacLocalLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    rtdrv_diagCfg_t diag_cfg;

    diag_cfg.unit = unit;
    diag_cfg.port = port;   
    diag_cfg.enable = enable;
    SETSOCKOPT(RTDRV_DIAG_MAC_LOCAL_LOOPBACK_SET, &diag_cfg, rtdrv_diagCfg_t, 1); 

    return RT_ERR_OK;
} /* end of rtk_diag_portMacLocalLoopbackEnable_set */

/* Module Name    : Diag */
/* Sub-module Name: RTCT */

/* Function Name:
 *      rtk_diag_portRtctResult_get
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
 *      If linkType is PORT_SPEED_1000M, test result will be stored in ge_result. 
 *      If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 */
int32
rtk_diag_portRtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    rtdrv_diagCfg_t diag_cfg;
    
    diag_cfg.unit = unit;
    diag_cfg.port = port;   
    GETSOCKOPT(RTDRV_DIAG_RTCTRESULT_GET, &diag_cfg, rtdrv_diagCfg_t, 1); 
    memcpy(pRtctResult, &diag_cfg.rtctResult, sizeof(rtk_rtctResult_t));

    return RT_ERR_OK;    
} /*end of rtk_diag_portRtctResult_get*/

/* Function Name:
 *      rtk_diag_rtctEnable_set
 * Description:
 *      Start RTCT for ports. 
 *      When enable RTCT, the port won't transmit and receive normal traffic.
 * Input:
 *      unit      - unit id
 *      pPortmask - the ports for RTCT test
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32 
rtk_diag_rtctEnable_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    rtdrv_diagCfg_t diag_cfg;
    
    diag_cfg.unit = unit;
    memcpy(&diag_cfg.portmask, pPortmask, sizeof(rtk_portmask_t));
    SETSOCKOPT(RTDRV_DIAG_RTCTENABLE_SET, &diag_cfg, rtdrv_diagCfg_t, 1); 

    return RT_ERR_OK; 
} /* end of rtk_diag_rtctEnable_set */

int32 rtk_diag_table_whole_read(uint32 unit, uint32 table_index)
{
    rtdrv_diagCfg_t diag_cfg;

    memset(&diag_cfg, 0, sizeof(rtdrv_diagCfg_t));	
    diag_cfg.unit = unit;
    diag_cfg.target_index= table_index;
	diag_cfg.type= DUMP_TYPE_TABLE;
    GETSOCKOPT(RTDRV_DIAG_TABLE_WHOLE_READ, &diag_cfg, rtdrv_diagCfg_t, 1);

    return RT_ERR_OK;    
}

int32 rtk_diag_peripheral_register_dump(uint32 unit)
{
    rtdrv_diagCfg_t diag_cfg;

    memset(&diag_cfg, 0, sizeof(rtdrv_diagCfg_t));
    diag_cfg.unit = unit;
    GETSOCKOPT(RTDRV_DIAG_PERIPHERAL_REG_READ, &diag_cfg, rtdrv_diagCfg_t, 1);

    return RT_ERR_OK;
}

int32 rtk_diag_reg_whole_read(uint32 unit)
{
    rtdrv_diagCfg_t diag_cfg;

    memset(&diag_cfg, 0, sizeof(rtdrv_diagCfg_t));	
    diag_cfg.unit = unit;   
	diag_cfg.type= DUMP_TYPE_REG;
    GETSOCKOPT(RTDRV_DIAG_REG_WHOLE_READ, &diag_cfg, rtdrv_diagCfg_t, 1);

    return RT_ERR_OK;    
}

int32 rtk_diag_phy_reg_whole_read(uint32 unit)
{
    rtdrv_diagCfg_t diag_cfg;

    memset(&diag_cfg, 0, sizeof(rtdrv_diagCfg_t));
    diag_cfg.unit = unit;
	diag_cfg.type= DUMP_TYPE_REG;
    GETSOCKOPT(RTDRV_DIAG_PHY_REG_READ, &diag_cfg, rtdrv_diagCfg_t, 1);

    return RT_ERR_OK;
}

