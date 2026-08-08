/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 17826 $
 * $Date: 2011-05-12 15:24:35 +0800 (Thu, 12 May 2011) $
 *
 * Purpose : Internal/External GPIO configuration for LOS Monitor Mechanism
 *
 * Feature : The file have include the following module and sub-modules
 *           1) extGpioDevConf configuration (sample)
 *           2) gpioDevConf configuration (sample)
 *
 * Note: The sample configuartion is for demo board and if customer's board have different layout, 
 *       please modify the file for their board if use the SDK mechanism.
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <drv/gpio/gpio.h>
#include <drv/gpio/ext_gpio.h>
#include <dal/dal_losMon.h>

/* 
 * Symbol Definition
 */

/*
 * Data Declaration
 */
const dal_losMon_extGpio_devConf_t extGpioDevConf[DAL_LOSMON_EXT_GPIO_DEV_NUM] =
{   /* Use only when "#define LOSMON_GPIO_TYPE EXT_GPIO" */
  { /* DEV-0 */ 
    .devId = 28,
    {        
      /* Start of EXT-GPIO Configuration for LOS configuration
       *
       * [Default Value]
       *   .direction = GPIO_DIR_IN
       *   .debounce  = DISABLED
       *   .inverter  = DISABLED,
       */
      {   .gpio = EXT_GPIO_ID0,
          .direction = GPIO_DIR_IN,
          .debounce = DISABLED,
          .inverter = DISABLED,
          .port     = 24,
      },
      {   .gpio = EXT_GPIO_ID1,
          .direction = GPIO_DIR_IN,
          .debounce = DISABLED,
          .inverter = DISABLED,
          .port     = 25,
      },
      {   .gpio = EXT_GPIO_ID3,
          .direction = GPIO_DIR_IN,
          .debounce = DISABLED,
          .inverter = DISABLED,
          .port     = 26,
      },
      {   .gpio = EXT_GPIO_ID4,
          .direction = GPIO_DIR_IN,
          .debounce = DISABLED,
          .inverter = DISABLED,
          .port     = 27,
      },
      /* End of EXT-GPIO Configuration */  
      {   .gpio = EXT_GPIO_ID_END, }
    }
  },
};

const dal_losMon_gpio_devConf_t  gpioDevConf = 
{   /* Use only when "#define LOSMON_GPIO_TYPE INT_GPIO" */
    {
        {
            .gpio = GPIO_ID(GPIO_PORT_A, 3),
            .function = GPIO_CTRLFUNC_NORMAL,
            .direction = GPIO_DIR_IN,
            .interruptEnable = GPIO_INT_DISABLE,
            .port     = 24,
        },
        {
            .gpio = GPIO_ID(GPIO_PORT_A, 2),
            .function = GPIO_CTRLFUNC_NORMAL,
            .direction = GPIO_DIR_IN,
            .interruptEnable = GPIO_INT_DISABLE,
            .port     = 25,
        },
        {
            .gpio = GPIO_ID(GPIO_PORT_A, 1),
            .function = GPIO_CTRLFUNC_NORMAL,
            .direction = GPIO_DIR_IN,
            .interruptEnable = GPIO_INT_DISABLE,
            .port     = 26,
        },
        {
            .gpio = GPIO_ID(GPIO_PORT_A, 0),
            .function = GPIO_CTRLFUNC_NORMAL,
            .direction = GPIO_DIR_IN,
            .interruptEnable = GPIO_INT_DISABLE,
            .port     = 27,
        },
    },
};
