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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : SoC register definition
 *
 * Feature : Define the GPIO registers
 *
 */

#ifndef __GPIO_REG_H__
#define __GPIO_REG_H__

#include <soc/var/var_gpio_reg.h>

#ifndef GPIO_REG_DEFINIED
#define GPIO_REG_DEFINIED

/*
 * GPIO
 */
#define GPIO_BASE        0xB8003500
#define PABCDCNR        (GPIO_BASE + 0x00)
#define PABCDDIR        (GPIO_BASE + 0x08)
#define PABCDDAT        (GPIO_BASE + 0x0C)
#define PABCDISR        (GPIO_BASE + 0x10)
#define PABIMR          (GPIO_BASE + 0x14)
#define PCDIMR          (GPIO_BASE + 0x18)

#define PEFGCNR         (GPIO_BASE + 0x1C)
#define PEFGDIR         (GPIO_BASE + 0x24)
#define PEFGDAT         (GPIO_BASE + 0x28)
#define PEFGISR         (GPIO_BASE + 0x2C)
#define PEFIMR          (GPIO_BASE + 0x30)
#define PGIMR           (GPIO_BASE + 0x34)

#endif /* end of GPIO_REG_DEFINIED */

#endif /* end of __GPIO_REG_H__ */

