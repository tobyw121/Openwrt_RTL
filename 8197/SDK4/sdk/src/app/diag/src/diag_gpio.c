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
 * $Revision: 22088 $
 * $Date: 2011-09-06 19:04:33 +0800 (Tue, 06 Sep 2011) $
 *
 * Purpose : Definition those internal GPIO command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) internal GPIO commands.
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <drv/gpio/gpio.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>

#ifdef CMD_GPIO_GET_GPIO_PORT_GPIO_PORT_GPIO_PIN_GPIO_PIN
/* 
 * gpio get gpio-port <UINT:gpio_port> gpio-pin <UINT:gpio_pin>
 */
cparser_result_t cparser_cmd_gpio_get_gpio_port_gpio_port_gpio_pin_gpio_pin(cparser_context_t *context,
    uint32_t *gpio_port_ptr,
    uint32_t *gpio_pin_ptr)
{
    uint32      unit = 0;
    uint32      gpio_data = 0;
    gpioID      gpioId;
    int32       ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_port_ptr >= GPIO_PORT_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_pin_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_pin_ptr > GPIO_PIN_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    gpioId = GPIO_ID(*gpio_port_ptr, *gpio_pin_ptr);
    DIAG_UTIL_ERR_CHK(drv_gpio_dataBit_get(gpioId, &gpio_data), ret);
    diag_util_printf("    0x%04X  ", gpio_data);
    diag_util_mprintf("\n");
    return CPARSER_OK;
}
#endif

#ifdef CMD_GPIO_SET_GPIO_PORT_GPIO_PORT_GPIO_PIN_GPIO_PIN_INIT_NORMAL_PERIPHERAL_IN_OUT_DISABLE_FALLING_EDGE_RISING_EDGE_BOTH_EDGE
/* 
 * gpio set gpio-port <UINT:gpio_port> gpio-pin <UINT:gpio_pin> init ( normal | peripheral ) ( in | out ) ( disable | falling-edge | rising-edge | both-edge )
 */
cparser_result_t cparser_cmd_gpio_set_gpio_port_gpio_port_gpio_pin_gpio_pin_init_normal_peripheral_in_out_disable_falling_edge_rising_edge_both_edge(cparser_context_t *context,
    uint32_t *gpio_port_ptr,
    uint32_t *gpio_pin_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    gpioID      gpioId;
    drv_gpio_control_t function;
    drv_gpio_direction_t direction;
    drv_gpio_interruptType_t interruptEnable;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_port_ptr >= GPIO_PORT_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_pin_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_pin_ptr > GPIO_PIN_MAX), CPARSER_ERR_INVALID_PARAMS);

    gpioId = GPIO_ID(*gpio_port_ptr, *gpio_pin_ptr);
    if ('n' == TOKEN_CHAR(7, 0))
        function = GPIO_CTRLFUNC_NORMAL;
    else
        function = GPIO_CTRLFUNC_DEDICATE_PERIPHERAL;

    if ('i' == TOKEN_CHAR(8, 0))
        direction = GPIO_DIR_IN;
    else
        direction = GPIO_DIR_OUT;

    if ('d' == TOKEN_CHAR(9, 0))
        interruptEnable = GPIO_INT_DISABLE;
    else if ('f' == TOKEN_CHAR(9, 0))
        interruptEnable = GPIO_INT_FALLING_EDGE;
    else if ('r' == TOKEN_CHAR(9, 0))
        interruptEnable = GPIO_INT_RISING_EDGE;
    else
        interruptEnable = GPIO_INT_BOTH_EDGE;

    DIAG_UTIL_ERR_CHK(drv_gpio_init(gpioId, function, direction, interruptEnable), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_GPIO_SET_GPIO_PORT_GPIO_PORT_GPIO_PIN_GPIO_PIN_INIT_DATA
/* 
 * gpio set gpio-port <UINT:gpio_port> gpio-pin <UINT:gpio_pin> init <UINT:data>
 */
cparser_result_t cparser_cmd_gpio_set_gpio_port_gpio_port_gpio_pin_gpio_pin_init_data(cparser_context_t *context,
    uint32_t *gpio_port_ptr,
    uint32_t *gpio_pin_ptr,
    uint32_t *data_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    gpioID      gpioId;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_port_ptr >= GPIO_PORT_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_pin_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_pin_ptr > GPIO_PIN_MAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((data_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    gpioId = GPIO_ID(*gpio_port_ptr, *gpio_pin_ptr);
    DIAG_UTIL_ERR_CHK(drv_gpio_dataBit_init(gpioId, *data_ptr), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_GPIO_SET_GPIO_PORT_GPIO_PORT_GPIO_PIN_GPIO_PIN_DATA
/* 
 * gpio set gpio-port <UINT:gpio_port> gpio-pin <UINT:gpio_pin> <UINT:data>
 */
cparser_result_t cparser_cmd_gpio_set_gpio_port_gpio_port_gpio_pin_gpio_pin_data(cparser_context_t *context,
    uint32_t *gpio_port_ptr,
    uint32_t *gpio_pin_ptr,
    uint32_t *data_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    gpioID      gpioId;

    DIAG_OM_GET_CHIP_ID(unit);  
            
    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_port_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_port_ptr >= GPIO_PORT_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((gpio_pin_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*gpio_pin_ptr > GPIO_PIN_MAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((data_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    gpioId = GPIO_ID(*gpio_port_ptr, *gpio_pin_ptr);
    DIAG_UTIL_ERR_CHK(drv_gpio_dataBit_set(gpioId, *data_ptr), ret);
    return CPARSER_OK;
}
#endif

