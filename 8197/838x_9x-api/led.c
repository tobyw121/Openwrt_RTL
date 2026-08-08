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
 * $Revision: 28275 $
 * $Date: 2012-04-24 15:52:41 +0800 (Tue, 24 Apr 2012) $
 *
 * Purpose : Definition those public LED APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) LED parameter settings
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <dal/dal_mgmt.h>
#include <rtk/port.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_led_init
 * Description:
 *      Initialize led module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      Must initialize led module before calling any led APIs.
 */
int32
rtk_led_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_init(unit);
} /* end of rtk_led_init */

/* Function Name:
 *      rtk_led_portEnable_get
 * Description:
 *      Get led status on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the led status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_led_portEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_portEnable_get(unit, port, pEnable);
} /* end of rtk_led_portEnable_get */

/* Function Name:
 *      rtk_led_portEnable_set
 * Description:
 *      Set led status on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - led status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      None
 */
int32
rtk_led_portEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_portEnable_set(unit, port, enable);
} /* end of rtk_led_portEnable_set */

/* Function Name:
 *      rtk_led_sysEnable_get
 * Description:
 *      Get led status of specified LED type.
 * Input:
 *      unit    - unit id
 *      type    - system led type
 * Output:
 *      pEnable - pointer to the led status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The type of system LED as following:
 *      - LED_TYPE_SYS
 *      - LED_TYPE_ALARM
 */
int32
rtk_led_sysEnable_get(uint32 unit, rtk_led_type_t type, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_sysEnable_get(unit, type, pEnable);
} /* end of rtk_led_sysEnable_get */

/* Function Name:
 *      rtk_led_sysEnable_set
 * Description:
 *      Set led status of specified LED type.
 * Input:
 *      unit   - unit id
 *      type   - system led type
 *      enable - led status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389, 8328, 8390, 8380
 * Note:
 *      The type of system LED as following:
 *      - LED_TYPE_SYS
 *      - LED_TYPE_ALARM
 */
int32
rtk_led_sysEnable_set(uint32 unit, rtk_led_type_t type, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || RT_MGMT(unit) == NULL, RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_sysEnable_set(unit, type, enable);
} /* end of rtk_led_sysEnable_set */

/* Function Name:
 *      rtk_led_portLedEntitySwCtrlEnable_get
 * Description:
 *      Get LED software control status on specified LED entity and port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      entity - LED entity id
 * Output:
 *      pEnable - software control mode status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Three LED entities (LED0/LED1/LED2) are supported per port. Each LED entity can be switched
 *          to software control mode individually.
 *      (2) rtk_led_portLedEntitySwCtrlEnable_set/rtk_led_portLedEntitySwCtrlMode_set doesn't take effect
 *          unless 'rtk_led_swCtrl_start' is invoked.
 */
int32
rtk_led_portLedEntitySwCtrlEnable_get(
    uint32          unit,
    rtk_port_t      port,
    uint32          entity,
    rtk_enable_t    *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->led_portLedEntitySwCtrlEnable_get(unit, phy_port, entity, pEnable);
    }
#else
    return RT_MAPPER(unit)->led_portLedEntitySwCtrlEnable_get(unit, port, entity, pEnable);
#endif
} /* end of rtk_led_portLedEntitySwCtrlEnable_get */

/* Function Name:
 *      rtk_led_portLedEntitySwCtrlEnable_set
 * Description:
 *      Set LED software control status on specified LED entity and port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      entity - LED entity id
 *      enable - software control mode status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Three LED entities (LED0/LED1/LED2) are supported per port. Each LED entity can be switched
 *          to software control mode individually.
 *      (2) rtk_led_portLedEntitySwCtrlEnable_set/rtk_led_portLedEntitySwCtrlMode_set doesn't take effect
 *          unless 'rtk_led_swCtrl_start' is invoked.
 */
int32
rtk_led_portLedEntitySwCtrlEnable_set(
    uint32          unit,
    rtk_port_t      port,
    uint32          entity,
    rtk_enable_t    enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->led_portLedEntitySwCtrlEnable_set(unit, phy_port, entity, enable);
    }
#else
    return RT_MAPPER(unit)->led_portLedEntitySwCtrlEnable_set(unit, port, entity, enable);
#endif
} /* end of rtk_led_portLedEntitySwCtrlEnable_set */

/* Function Name:
 *      rtk_led_portLedEntitySwCtrlMode_get
 * Description:
 *      Get display mode of software control LED on specified port, LED entity and media.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      entity - LED entity id
 *      media  - media type
 * Output:
 *      pMode  - LED display mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Media type only supports PORT_MEDIA_COPPER and PORT_MEDIA_FIBER.
 *      (2) System software control mode only support:
 *          - RTK_LED_SWCTRL_MODE_OFF,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_32MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_64MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_128MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_256MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_512MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_1024MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_CONTINUE
 *      (3) rtk_led_portLedEntitySwCtrlEnable_set/rtk_led_portLedEntitySwCtrlMode_set doesn't take effect
 *          unless 'rtk_led_swCtrl_start' is invoked.
 */
int32
rtk_led_portLedEntitySwCtrlMode_get(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  entity,
    rtk_port_media_t        media,
    rtk_led_swCtrl_mode_t   *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->led_portLedEntitySwCtrlMode_get(unit, phy_port, entity, media, pMode);
    }
#else
    return RT_MAPPER(unit)->led_portLedEntitySwCtrlMode_get(unit, port, entity, media, pMode);
#endif
} /* end of rtk_led_portLedEntitySwCtrlMode_get */

/* Function Name:
 *      rtk_led_portLedEntitySwCtrlMode_set
 * Description:
 *      Set display mode of software control LED on specified port, LED entity and media.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      entity - LED entity id
 *      media  - media type
 *      mode   - LED display mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      (1) Media type only supports PORT_MEDIA_COPPER and PORT_MEDIA_FIBER.
 *      (2) System software control mode only support:
 *          - RTK_LED_SWCTRL_MODE_OFF,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_32MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_64MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_128MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_256MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_512MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_1024MS,
 *          - RTK_LED_SWCTRL_MODE_BLINKING_CONTINUE
 *      (3) rtk_led_portLedEntitySwCtrlEnable_set/rtk_led_portLedEntitySwCtrlMode_set doesn't take effect
 *          unless 'rtk_led_swCtrl_start' is invoked.
 */
int32
rtk_led_portLedEntitySwCtrlMode_set(
    uint32                  unit,
    rtk_port_t              port,
    uint32                  entity,
    rtk_port_media_t        media,
    rtk_led_swCtrl_mode_t   mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->led_portLedEntitySwCtrlMode_set(unit, phy_port, entity, media, mode);
    }
#else
    return RT_MAPPER(unit)->led_portLedEntitySwCtrlMode_set(unit, port, entity, media, mode);
#endif
} /* end of rtk_led_portLedEntitySwCtrlMode_set */

/* Function Name:
 *      rtk_led_swCtrl_start
 * Description:
 *      Apply LED software control configuration.
 * Input:
 *      unit   - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 * Applicable:
 *      8390
 * Note:
 *      The API must be called for software control LED "Enable/Mode" configurations to take effect.
 */
int32
rtk_led_swCtrl_start(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_swCtrl_start(unit);
} /* end of rtk_led_swCtrl_start */

/* Function Name:
 *      rtk_led_sysMode_get
 * Description:
 *      Get system LED display mode.
 * Input:
 *      unit  - unit id
 * Output:
 *      pMode - LED display mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8390, 8380
 * Note:
 *      System software control mode only support:
 *      - RTK_LED_SWCTRL_MODE_OFF
 *      - RTK_LED_SWCTRL_MODE_BLINKING_512MS
 *      - RTK_LED_SWCTRL_MODE_BLINKING_1024MS
 *      - RTK_LED_SWCTRL_MODE_BLINKING_CONTINUE
 */
int32
rtk_led_sysMode_get(uint32 unit, rtk_led_swCtrl_mode_t *pMode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_sysMode_get(unit, pMode);
} /* end of rtk_led_sysMode_get */

/* Function Name:
 *      rtk_led_sysMode_set
 * Description:
 *      Set system LED display mode.
 * Input:
 *      unit - unit id
 *      mode - LED display mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8390, 8380
 * Note:
 *      System software control mode only support:
 *      - RTK_LED_SWCTRL_MODE_OFF
 *      - RTK_LED_SWCTRL_MODE_BLINKING_64MS
 *      - RTK_LED_SWCTRL_MODE_BLINKING_1024MS
 *      - RTK_LED_SWCTRL_MODE_BLINKING_CONTINUE
 */
int32
rtk_led_sysMode_set(uint32 unit, rtk_led_swCtrl_mode_t mode)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->led_sysMode_set(unit, mode);
} /* end of rtk_led_sysMode_set */
