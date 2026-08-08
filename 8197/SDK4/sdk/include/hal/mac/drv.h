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
 * $Revision: 54450 $
 * $Date: 2014-12-30 14:31:41 +0800 (Tue, 30 Dec 2014) $
 *
 * Purpose : mac driver service APIs in the SDK.
 *
 * Feature : mac driver service APIs
 *
 */

#ifndef __HAL_MAC_DRV_H__
#define __HAL_MAC_DRV_H__


/*
 * Macro Declaration
 */
/* table semaphore handling */
#define MEM_SEM_LOCK(unit, group)\
do {\
    if (osal_sem_mutex_take(hal_ctrl[unit].tbl_sem[group], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "table semaphore lock group %d failed", group);\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define MEM_SEM_UNLOCK(unit, group)\
do {\
    if (osal_sem_mutex_give(hal_ctrl[unit].tbl_sem[group]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "table semaphore unlock group %d failed", group);\
        return RT_ERR_FAILED;\
    }\
} while(0)

/* PHY semaphore handling */
#define PHY_SEM_LOCK(unit)\
do {\
    if (osal_sem_mutex_take(hal_ctrl[unit].phy_sem, OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "PHY semaphore lock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

#define PHY_SEM_UNLOCK(unit)\
do {\
    if (osal_sem_mutex_give(hal_ctrl[unit].phy_sem) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_FAILED, (MOD_HAL), "PHY semaphore unlock failed");\
        return RT_ERR_FAILED;\
    }\
} while(0)

/* serdes patch config */
#if defined(CONFIG_SDK_RTL8390)

#define SERDES_SET(_reg, _sb, _eb, _val) drv_serdes_set(unit, _reg, _sb, _eb, _val)

void drv_serdes_set(uint32 unit, uint32 reg, uint32 endBit,
    uint32 startBit, uint32 val);
#endif  /* defined(CONFIG_SDK_RTL8390) */

/*
 * Function Declaration
 */

/* Function Name:
 *      table_find
 * Description:
 *      Find this kind of table structure in this specified chip.
 * Input:
 *      unit  - unit id
 *      table - table index
 * Output:
 *      None
 * Return:
 *      NULL      - Not found
 *      Otherwise - Pointer of table structure that found
 * Note:
 *      None
 */
extern rtk_table_t *
table_find (uint32 unit, uint32 table);

/* Function Name:
 *      rtl8389_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8389_port_probe(uint32 unit);

/* Function Name:
 *      rtl8389_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8389_init(uint32 unit);

/* Function Name:
 *      rtl8389_miim_read
 * Description:
 *      Get PHY registers from rtl8389 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8389_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData);

/* Function Name:
 *      rtl8389_miim_write
 * Description:
 *      Set PHY registers in rtl8389 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8389_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      rtl8389_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8389 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8389_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data);

/* Function Name:
 *      rtl8389_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      1. The addr argument of RTL8389 PIE table is not continuous bits from
 *         LSB bits, we do one compiler option patch for this.
 *      2. If you don't use the RTL8389 chip, please turn off the "RTL8389"
 *         definition symbol, then performance will be improved.
 */
extern int32
rtl8389_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);

/* Function Name:
 *      rtl8389_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      1. The addr argument of RTL8389 PIE table is not continuous bits from
 *         LSB bits, we do one compiler option patch for this.
 *      2. If you don't use the RTL8389 chip, please turn off the "RTL8389"
 *         definition symbol, then performance will be improved.
 */
extern int32
rtl8389_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);

//#############################################################################

/* Function Name:
 *      rtl8328_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8328_port_probe(uint32 unit);

/* Function Name:
 *      rtl8328_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8328_init(uint32 unit);

/* Function Name:
 *      rtl8328_miim_read
 * Description:
 *      Get PHY registers from rtl8328 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8328_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData);

/* Function Name:
 *      rtl8328_miim_write
 * Description:
 *      Set PHY registers in rtl8328 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8328_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      rtl8328_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8328 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8328_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data);

/* Function Name:
 *      rtl8328_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
extern int32
rtl8328_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);

/* Function Name:
 *      rtl8328_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
extern int32
rtl8328_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);

//#############################################################################

/* Function Name:
 *      rtl8390_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8390_port_probe(uint32 unit);


/* Function Name:
 *      rtl8390_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8390_init(uint32 unit);


/* Function Name:
 *      rtl8390_miim_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData);



/* Function Name:
 *      rtl8390_miim_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      rtl8390_miim_park_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_park_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData);

/* Function Name:
 *      rtl8390_miim_park_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      parkPage    - PHY park page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_park_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      rtl8390_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8390 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data);

/* Function Name:
 *      rtl8390_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8390 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data);

/* Function Name:
 *      rtl8390_miim_broadcast_write
 * Description:
 *      Set PHY registers in rtl8390 family chips with broadcast mechanism.
 * Input:
 *      unit    - unit id
 *      page    - page id
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. page valid range is 0 ~ 31
 *      2. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_broadcast_write(
    uint32      unit,
    uint32      page,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      rtl8390_miim_extParkPage_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_extParkPage_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mainPage,
    uint32      extPage,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData);


/* Function Name:
 *      rtl8390_miim_extParkPage_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mainPage    - main page id
 *      extPage     - extension page id
 *      parkPage    - parking page id
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_extParkPage_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mainPage,
    uint32      extPage,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data);

/* Function Name:
 *      rtl8390_miim_extParkPage_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8390 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_extParkPage_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mainPage,
    uint32          extPage,
    uint32          parkPage,
    uint32          phy_reg,
    uint32          data);

/* Function Name:
 *      rtl8390_miim_mmd_read
 * Description:
 *      Get PHY registers from rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData);


/* Function Name:
 *      rtl8390_miim_mmd_write
 * Description:
 *      Set PHY registers in rtl8390 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data);

/* Function Name:
 *      rtl8390_miim_mmd_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8390 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      mmdAddr  - mmd device address
 *      mmdReg   - mmd reg id
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8390_miim_mmd_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mmdAddr,
    uint32          mmdReg,
    uint32          data);

/* Function Name:
 *      rtl8390_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
extern int32
rtl8390_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);


/* Function Name:
 *      rtl8390_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
extern int32
rtl8390_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);

/* Function Name:
 *      rtl8390_miim_pollingEnable_get
 * Description:
 *      Get the mac polling PHY status of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
rtl8390_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled);

/* Function Name:
 *      rtl8390_miim_pollingEnable_set
 * Description:
 *      Set the mac polling PHY status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enabled - mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
rtl8390_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled);

/* Function Name:
 *      rtl8390_miim_globalPollingEnable_get
 * Description:
 *      Get the global mac polling PHY status.
 * Input:
 *      unit     - unit id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
rtl8390_miim_globalPollingEnable_get(
    uint32          unit,
    rtk_enable_t    *pEnabled);

/* Function Name:
 *      rtl8390_miim_globalPollingEnable_set
 * Description:
 *      Set the global mac polling PHY status.
 * Input:
 *      unit    - unit id
 *      enabled - global mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
rtl8390_miim_globalPollingEnable_set(
    uint32          unit,
    rtk_enable_t    enabled);

/* Function Name:
 *      rtl8390_serdes_rst
 * Description:
 *      Reset Serdes and original patch are kept.
 * Input:
 *      unit    - unit id
 *      sds_num    - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 * Note:
 *      None
 */
extern int32
rtl8390_serdes_rst(
	uint32  unit,
	uint32 sds_num);


//#############################################################################

/* Function Name:
 *      rtl8380_port_probe
 * Description:
 *      Probe the select port interface settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8380_port_probe(uint32 unit);


/* Function Name:
 *      rtl8380_init
 * Description:
 *      Initialize the specified settings of the chip.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
rtl8380_init(uint32 unit);


/* Function Name:
 *      rtl8380_miim_read
 * Description:
 *      Get PHY registers from rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      *pData);


/* Function Name:
 *      rtl8380_miim_write
 * Description:
 *      Set PHY registers in rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      phy_reg,
    uint32      data);


/* Function Name:
 *      rtl8380_miim_park_read
 * Description:
 *      Get PHY registers from rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_park_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      *pData);


/* Function Name:
 *      rtl8380_miim_park_write
 * Description:
 *      Set PHY registers in rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      page    - PHY page
 *      phy_reg - PHY register
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_park_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      page,
    uint32      parkPage,
    uint32      phy_reg,
    uint32      data);



/* Function Name:
 *      rtl8380_miim_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8380 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      page     - PHY page
 *      phy_reg  - PHY register
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          page,
    uint32          phy_reg,
    uint32          data);


/* Function Name:
 *      rtl8380_miim_mmd_read
 * Description:
 *      Get PHY registers from rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 * Output:
 *      pData   - pointer buffer of read data
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_mmd_read(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      *pData);


/* Function Name:
 *      rtl8380_miim_mmd_write
 * Description:
 *      Set PHY registers in rtl8380 family chips.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      mmdAddr - mmd device address
 *      mmdReg  - mmd reg id
 *      data    - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. port valid range is 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_mmd_write(
    uint32      unit,
    rtk_port_t  port,
    uint32      mmdAddr,
    uint32      mmdReg,
    uint32      data);

/* Function Name:
 *      rtl8380_miim_mmd_portmask_write
 * Description:
 *      Set PHY registers in those portmask of rtl8380 family chips.
 * Input:
 *      unit     - unit id
 *      portmask - portmask
 *      mmdAddr  - mmd device address
 *      mmdReg   - mmd reg id
 *      data     - Read data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_PAGE_ID  - invalid PHY page id
 *      RT_ERR_PHY_REG_ID   - invalid PHY reg id
 * Note:
 *      1. portmask valid range is bit 0 ~ 27
 *      2. page valid range is 0 ~ 31
 *      3. phy_reg valid range is 0 ~ 31
 */
extern int32
rtl8380_miim_mmd_portmask_write(
    uint32          unit,
    rtk_portmask_t  portmask,
    uint32          mmdAddr,
    uint32          mmdReg,
    uint32          data);



/* Function Name:
 *      rtl8380_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * Output:
 *      pData - pointer buffer of table entry data
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
extern int32
rtl8380_table_read(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);


/* Function Name:
 *      rtl8380_table_write
 * Description:
 *      Write one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 *      pData - pointer buffer of table entry data
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 *      RT_ERR_OUT_OF_RANGE       - input parameter out of range
 *      RT_ERR_CHIP_NOT_SUPPORTED - functions not supported by this chip model
 *      RT_ERR_INPUT              - invalid input parameter
 * Note:
 *      None
 */
extern int32
rtl8380_table_write(
    uint32  unit,
    uint32  table,
    uint32  addr,
    uint32  *pData);

/* Function Name:
 *      rtl8380_miim_pollingEnable_get
 * Description:
 *      Get the mac polling PHY status of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pEnabled - pointer buffer of mac polling PHY status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
rtl8380_miim_pollingEnable_get(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    *pEnabled);

/* Function Name:
 *      rtl8380_miim_pollingEnable_set
 * Description:
 *      Set the mac polling PHY status of the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enabled - mac polling PHY status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - Failed
 * Note:
 *      None
 */
extern int32
rtl8380_miim_pollingEnable_set(
    uint32          unit,
    rtk_port_t      port,
    rtk_enable_t    enabled);


/* Function Name:
 *      rtl8380_serdes_rst
 * Description:
 *      Reset Serdes and original patch are kept.
 * Input:
 *      unit    - unit id
 *      sds_num    - serdes id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 * Note:
 *      None
 */
extern int32
rtl8380_serdes_rst(
	uint32  unit,
	uint32 sds_num);

#if defined(CONFIG_SDK_DUMP_TABLE_WITH_NAME)
/* Function Name:
 *      table_name_get
 * Description:
 *      Get table name of the specified register index.
 * Input:
 *      unit  - unit id
 *      table   - register index
 * Output:
 *      pData - pointer buffer of table name
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 *      RT_ERR_OUT_OF_RANGE - input parameter out of range
 * Note:
 *      None
 */
extern int32
table_name_get(uint32 unit, uint32 table, char *pData);
#endif

#endif  /* __HAL_MAC_DRV_H__ */
