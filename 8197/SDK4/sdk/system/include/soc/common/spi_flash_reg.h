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
 * Feature : Define the SPI FLASH registers
 *
 */

#ifndef __SPI_FLASH_REG_H__
#define __SPI_FLASH_REG_H__

#include <soc/var/var_spi_flash_reg.h>

#ifndef SPI_FLASH_REG_DEFINIED
#define SPI_FLASH_REG_DEFINIED

#define SPI_REG_BASE      (0xB8001200)
#define SFCR              (SPI_REG_BASE+0x000)
#define SFCSR             (SPI_REG_BASE+0x004)
#define SFDR              (SPI_REG_BASE+0x008)

#endif /* end of SPI_FLASH_REG_DEFINIED */

#endif /* end of __SPI_FLASH_REG_H__ */

