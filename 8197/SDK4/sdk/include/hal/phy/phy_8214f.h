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
 * $Revision: 34988 $
 * $Date: 2012-12-05 13:58:28 +0800 (Wed, 05 Dec 2012) $
 *
 * Purpose : PHY 8214F Driver APIs.
 *
 * Feature : PHY 8214F Driver APIs
 *
 */

#ifndef __HAL_PHY_PHY_8214F_H__
#define __HAL_PHY_PHY_8214F_H__

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <rtk/port.h>


/*
 * Symbol Definition
 */

#define PORT_NUM_IN_8214F   (4)


/*
 * Data Declaration
 */
extern rt_phydrv_t phy_8214Fdrv_ge;
extern rt_phydrv_t phy_8214drv_ge;


/* Function Name:
 *      phy_8214f_media_get
 * Description:
 *      Get PHY 8214F media type.
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
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 */
extern int32
phy_8214f_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      phy_8214f_media_set
 * Description:
 *      Get PHY 8214F media type.
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
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 *      2. there are 3 major steps for change the media from original to new
 *         - turn off original media
 *         - turn on new media
 *         - set new media to first priority
 *      3. PORT_MEDIA_COPPER_AUTO & PORT_MEDIA_FIBER_AUTO return RT_ERR_CHIP_NOT_SUPPORTED.
 */
extern int32
phy_8214f_media_set(uint32 unit, rtk_port_t port, rtk_port_media_t media);

/* Function Name:
 *      phy_8214f_init
 * Description:
 *      Initialize PHY 8214F.
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
extern int32
phy_8214f_init(uint32 unit, rtk_port_t port);

/* Function Name:
 *      phy_8214f_autoNegoAbility_get
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
extern int32
phy_8214f_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      phy_8214f_autoNegoAbility_set
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
extern int32
phy_8214f_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility);

/* Function Name:
 *      phy_8214f_speed_get
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
extern int32
phy_8214f_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed);

/* Function Name:
 *      phy_8214_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
extern int32
phy_8214_speed_set(uint32 unit, rtk_port_t port, uint32 speed);

/* Function Name:
 *      phy_8214f_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
extern int32
phy_8214f_speed_set(uint32 unit, rtk_port_t port, uint32 speed);

/* Function Name:
 *      phy_8214f_enable_set
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
extern int32
phy_8214f_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      phy_8214f_rtctResult_get
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
 *      None
 */
extern int32
phy_8214f_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult);

/* Function Name:
 *      phy_8214f_rtct_start
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
extern int32
phy_8214f_rtct_start(uint32 unit, rtk_port_t port);

/* Function Name:
 *      phy_8214f_greenEnable_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      1. The RTL8214F is only supported the per-chip link-up green feature.
 *      2. The setting will be apply to 4 ports in the chip.
 */
extern int32
phy_8214f_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      phy_8214f_greenEnable_set
 * Description:
 *      Set the status of link-up green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-up green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      None
 */
extern int32
phy_8214f_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      phy_8214f_crossOverMode_get
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
extern int32
phy_8214f_crossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode);

/* Function Name:
 *      phy_8214f_crossOverMode_set
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
extern int32
phy_8214f_crossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode);

/* Function Name:
 *      phy_8214f_linkDownPowerSavingEnable_get
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
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. The RTL8214F is only supported the per-chip link-down power saving
 *      2. The setting will be apply to 4 ports in the chip.
 */
extern int32
phy_8214f_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

/* Function Name:
 *      phy_8214f_linkDownPowerSavingEnable_set
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
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. The RTL8214F is only supported the per-chip link-down power saving
 *      2. The setting will be apply to 4 ports in the chip.
 */
extern int32
phy_8214f_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable);

/* Function Name:
 *      phy_8214_media_get
 * Description:
 *      Get 8214 serdes PHY media type.
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
 *      1. media type is PORT_MEDIA_COPPER
 */
extern int32
phy_8214_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia);

/* Function Name:
 *      phy_8214f_gigaLiteEnable_get
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
 *      1. The RTL8214F is not supported the per-port Giga Lite feature.
 */
extern int32
phy_8214f_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable);

#endif /* __HAL_PHY_PHY_8214F_H__ */
