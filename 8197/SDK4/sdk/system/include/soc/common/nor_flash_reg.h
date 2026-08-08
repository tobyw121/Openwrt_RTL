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
 * Feature : Define the NOR FLASH registers
 *
 */

#ifndef __NOR_FLASH_REG_H__
#define __NOR_FLASH_REG_H__

#include <soc/var/var_nor_flash_reg.h>

#ifndef NOR_FLASH_REG_DEFINIED
#define NOR_FLASH_REG_DEFINIED

#define NOR_REG_BASE      (0xB8001100)
#define NFCR              (NOR_REG_BASE+0x000)


#endif /* end of NOR_FLASH_REG_DEFINIED */

#endif /* end of __NOR_FLASH_REG_H__ */

