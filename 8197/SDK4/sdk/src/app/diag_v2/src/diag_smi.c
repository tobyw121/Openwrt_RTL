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
 * $Revision: 40450 $
 * $Date: 2013-06-24 17:45:49 +0800 (Mon, 24 Jun 2013) $
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
#include <drv/smi/smi.h>
#include <drv/gpio/gpio.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>
#include <drv/gpio/ext_gpio.h>

#ifdef CMD_SMI_GET_SMI_GROUP
/*
 * smi get <UINT:smi_group>
 */
cparser_result_t cparser_cmd_smi_get_smi_group(cparser_context_t *context,
    uint32_t *smi_group_ptr)
{
    uint32  unit = 0;
    uint32  sck_gpio_port = 0;
    uint32  sck_gpio_pin = 0;
    uint32  sda_gpio_port = 0;
    uint32  sda_gpio_pin = 0;
    uint32  smi_type = 0;
    uint32  smi_chipid = 0;
    uint32  smi_delay = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((smi_group_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*smi_group_ptr >= SMI_DEVICE_MAX), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(drv_smi_group_get(&sck_gpio_port, &sck_gpio_pin, &sda_gpio_port, &sda_gpio_pin,*smi_group_ptr), ret);
    DIAG_UTIL_ERR_CHK(drv_smi_type_get(&smi_type, &smi_chipid, &smi_delay, *smi_group_ptr), ret);
    diag_util_printf("    SMI:%d, SCK:Port:0x%02X,PIN:0x%02X, SDA:Port:0x%02X,PIN:0x%02X, Type:0x%02X, CHIPID:0x%02X, Delay:0x%d", *smi_group_ptr, sck_gpio_port, sck_gpio_pin, sda_gpio_port, sda_gpio_pin, smi_type, smi_chipid, smi_delay);
    diag_util_mprintf("\n");
    return CPARSER_OK;
}
#endif

#ifdef CMD_SMI_SET_SMI_GROUP_SCK_PORT_SCK_PORT_SCK_PIN_SCK_PIN_SDA_PORT_SDA_PORT_SDA_PIN_SDA_PIN_TYPE_SMI_TYPE_CHIPID_SMI_CHIPID_DELAY_SMI_DELAY
/*
 * smi set <UINT:smi_group> sck-port <UINT:sck_port> sck-pin <UINT:sck_pin> sda-port <UINT:sda_port> sda-pin <UINT:sda_pin> type <UINT:smi_type> chipid <UINT:smi_chipid> delay <UINT:smi_delay>
 */
cparser_result_t cparser_cmd_smi_set_smi_group_sck_gpiodev_sck_gpiodev_sck_pin_sck_pin_sda_gpiodev_sda_gpiodev_sda_pin_sda_pin_type_smi_type_chipid_smi_chipid_delay_smi_delay(cparser_context_t *context,
    uint32_t *smi_group_ptr,
    uint32_t *sck_gpiodev_ptr,
    uint32_t *sck_pin_ptr,
    uint32_t *sda_gpiodev_ptr,
    uint32_t *sda_pin_ptr,
    uint32_t *smi_type_ptr,
    uint32_t *smi_chipid_ptr,
    uint32_t *smi_delay_ptr)    
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((smi_group_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*smi_group_ptr >= SMI_DEVICE_MAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((sck_gpiodev_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*sck_gpiodev_ptr >= GPIO_PORT_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((sck_pin_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*sck_pin_ptr > EXT_GPIO_ID_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((sda_gpiodev_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*sda_gpiodev_ptr >= GPIO_PORT_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((sda_pin_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*sda_pin_ptr > EXT_GPIO_ID_END), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((smi_type_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*smi_type_ptr > SMI_TYPE_MAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((smi_chipid_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(drv_smi_init(*sck_gpiodev_ptr, *sck_pin_ptr, *sda_gpiodev_ptr, *sda_pin_ptr, *smi_group_ptr), ret);
    DIAG_UTIL_ERR_CHK(drv_smi_type_set(*smi_type_ptr, *smi_chipid_ptr, *smi_delay_ptr, *smi_group_ptr), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_SMI_GET_REG_SMI_GROUP_REG_ADD
/*
 * smi get reg <UINT:smi_group> <UINT:reg_add>
 */
cparser_result_t cparser_cmd_smi_get_reg_smi_group_reg_add(cparser_context_t *context,
    uint32_t *smi_group_ptr,
    uint32_t *reg_add_ptr)
{
    uint32  unit = 0;
    uint32  data;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((smi_group_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*smi_group_ptr >= SMI_DEVICE_MAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((reg_add_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(drv_smi_read(*reg_add_ptr, &data, *smi_group_ptr), ret);
    diag_util_printf("    READ:SMI:%d, Register:0x%02X, VALUE:0x%02X", *smi_group_ptr, *reg_add_ptr, data);
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_SMI_SET_REG_SMI_GROUP_REG_ADD_REG_DATA
/*
 * smi set reg <UINT:smi_group> <UINT:reg_add> <UINT:reg_data>
 */
cparser_result_t cparser_cmd_smi_set_reg_smi_group_reg_add_reg_data(cparser_context_t *context,
    uint32_t *smi_group_ptr,
    uint32_t *reg_add_ptr,
    uint32_t *reg_data_ptr)
{
    uint32  unit = 0;
    uint32  data;
    int32   ret = RT_ERR_FAILED;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_PARAM_CHK();
    DIAG_UTIL_PARAM_RANGE_CHK((smi_group_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((*smi_group_ptr >= SMI_DEVICE_MAX), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((reg_add_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);
    DIAG_UTIL_PARAM_RANGE_CHK((reg_data_ptr == NULL), CPARSER_ERR_INVALID_PARAMS);

    DIAG_UTIL_ERR_CHK(drv_smi_write(*reg_add_ptr, *reg_data_ptr, *smi_group_ptr), ret);
    DIAG_UTIL_ERR_CHK(drv_smi_read(*reg_add_ptr, &data, *smi_group_ptr), ret);
    diag_util_printf("    WRITE:SMI:%d, Register:0x%02X, VALUE:0x%02X", *smi_group_ptr, *reg_add_ptr, data);
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif


#if defined(CONFIG_SDK_RTL8231)
#endif

