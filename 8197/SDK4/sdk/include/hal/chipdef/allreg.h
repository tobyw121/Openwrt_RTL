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
 * $Revision: 50086 $
 * $Date: 2014-08-12 15:16:36 +0800 (Tue, 12 Aug 2014) $
 *
 * Purpose : chip register and its field definition in the SDK.
 *
 * Feature : chip register and its field definition
 *
 */

#ifndef __HAL_CHIPDEF_ALLREG_H__
#define __HAL_CHIPDEF_ALLREG_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>

/*
 * Data Type Declaration
 */

#if defined(CONFIG_SDK_REG_FIELD_TYPE)
/* register field structure definition */
typedef enum rtk_field_type_e
{
    FIELD_TYPE_RO = 0,        /* read only */
    FIELD_TYPE_WO,            /* write only */
    FIELD_TYPE_RW,            /* read, write */
    FIELD_TYPE_RC,            /* read to clear */
    FIELD_TYPE_RW1C,          /* read, write 1 to clear */
    FIELD_TYPE_RWAC,          /*read, writ, write 1 and auto clear to 0 after command done*/
    FIELD_TYPE_NONE,          /* no use */
    FIELD_TYPE_END
} rtk_field_type_t;
#endif

typedef struct rtk_regField_s
{
    uint32 name;              /* field name */
    uint16 lsp;               /* LSP (Least Significant Position) of the field */
    uint16 len;               /* field length */
#if defined(CONFIG_SDK_REG_FIELD_TYPE)
    rtk_field_type_t type;    /* r/w type */
#endif
} rtk_regField_t;

/* register structure definition */
typedef struct rtk_reg_s
{
    uint32 offset;            /* offset address */
    uint32 field_num;         /* total field numbers */
    uint32 lport:8;           /* port start index */
    uint32 hport:8;           /* port end index */
    uint32 larray:8;          /* array start index */
    uint32 harray:8;          /* array end index */
    uint32 portlist_index:16;  /* portlist index */
    uint32 bit_offset:16;      /* array offset */
    rtk_regField_t *pFields;   /* register fields */
#if defined(CONFIG_SDK_REG_DFLT_VAL)
    #if defined(CONFIG_SDK_RTL8380)
        uint64 default_value;     /* default value */
    #else
    uint32 default_value;     /* default value */
#endif
#endif
} rtk_reg_t;

#if defined(CONFIG_SDK_DUMP_REG_WITH_NAME)
/* register name structure definition */
typedef struct rtk_regName_s
{
    char name[64];         /* total field numbers */
} rtk_regName_t;
#endif  /* CONFIG_SDK_DUMP_REG_WITH_NAME */

#if defined(CONFIG_SDK_REG_DFLT_VAL)
#define RTK_REG_DFLT_VAL(value)     value
#else
#define RTK_REG_DFLT_VAL(value)
#endif

#if defined(CONFIG_SDK_REG_FIELD_TYPE)
#define RTK_REG_FIELD_TYPE(value)     value
#else
#define RTK_REG_FIELD_TYPE(value)
#endif

#endif  /* __HAL_CHIPDEF_ALLREG_H__ */
