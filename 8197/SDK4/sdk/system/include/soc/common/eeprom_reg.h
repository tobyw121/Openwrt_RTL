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
 * Feature : Define the EEPROM registers
 *
 */

#ifndef __EEPROM_REG_H__
#define __EEPROM_REG_H__

#include <soc/var/var_eeprom_reg.h>

#ifndef EEPROM_REG_DEFINIED
#define EEPROM_REG_DEFINIED

#define EEPROM_BASE        (0xB8001400)
#define EECR               (EEPROM_BASE+0x00)
#define EECSR              (EEPROM_BASE+0x04)
#define EEAR               (EEPROM_BASE+0x08)
#define EEDR               (EEPROM_BASE+0x0C)

#endif /* end of EEPROM_REG_DEFINIED */

#endif /* end of __EEPROM_REG_H__ */

