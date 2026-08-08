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
 * Purpose : chip table and its field definition in the SDK.
 *
 * Feature : chip table and its field definition
 *
 */

#ifndef __HAL_CHIPDEF_ALLMEM_H__
#define __HAL_CHIPDEF_ALLMEM_H__

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>

/*
 * Data Type Declaration
 */
/* indirect control group enum definition */
typedef enum rtk_indirectCtrlGroup_e
{
#if defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    INDIRECT_CTRL_GROUP_TABLE,
#endif
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    INDIRECT_CTRL_GROUP_L2,
    INDIRECT_CTRL_GROUP_PKT_ENC,
    INDIRECT_CTRL_GROUP_EGR_CTRL,
#endif
#if defined(CONFIG_SDK_RTL8328)
    INDIRECT_CTRL_GROUP_STORM,
#endif
    RTK_INDIRECT_CTRL_GROUP_END
} rtk_indirectCtrlGroup_t;


/* table field structure definition */
typedef struct rtk_tableField_s
{
    uint16 lsp;               /* LSP (Least Significant Position) of the field */
    uint16 len;               /* field length */
} rtk_tableField_t;

/* table structure definition */
typedef struct rtk_table_s
{
    uint32 type;              /* access table type */
    uint32 size;              /* table size */
    uint32 datareg_num;       /* total data registers */
    uint32 field_num;         /* total field numbers */
    rtk_tableField_t *pFields; /* table fields */
} rtk_table_t;

#if defined(CONFIG_SDK_DUMP_TABLE_WITH_NAME)
/* table name structure definition */
typedef struct rtk_tableName_s
{
    char name[64];         /* total field numbers */
} rtk_tableName_t;
#endif  /* CONFIG_SDK_DUMP_TABLE_WITH_NAME */


/*
 * Macro Definition
 */

/* Declaration the size of table entry
 * Each structure is a word array that size is the maximum in all supported chips.
 */
#define MEM_ENTRY_DECLARE(name, words)\
    typedef struct {\
        uint32 entry_data[words];\
    } name

/* structure is word array that have the maximum value of all supported chips. */
#if (defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380))
MEM_ENTRY_DECLARE(l2cam_entry_t, 3);
MEM_ENTRY_DECLARE(l2_entry_t, 3);
MEM_ENTRY_DECLARE(log_entry_t, 3);
MEM_ENTRY_DECLARE(multicast_index_entry_t, 2);
#endif
#if (defined(CONFIG_SDK_RTL8389))
MEM_ENTRY_DECLARE(pie89_entry_t, 19);
MEM_ENTRY_DECLARE(svlan_entry_t, 3);
#endif
MEM_ENTRY_DECLARE(vlan_entry_t, 3);
#if (defined(CONFIG_SDK_RTL8328))
MEM_ENTRY_DECLARE(pie_act_entry_t, 6);
MEM_ENTRY_DECLARE(entry_hit_indi_sta_entry_t, 4);
MEM_ENTRY_DECLARE(policer_entry_t, 4);
MEM_ENTRY_DECLARE(pie_rule_entry_t, 10);
MEM_ENTRY_DECLARE(cbccr_entry_t, 1);
MEM_ENTRY_DECLARE(move_pie_entry_t, 2);
MEM_ENTRY_DECLARE(tsscr0_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr1_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr2_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr3_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr4_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr5_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr6_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr7_entry_t, 1);
MEM_ENTRY_DECLARE(tsscr8_entry_t, 1);
#endif
#if (defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380))
MEM_ENTRY_DECLARE(spt_entry_t, 4);
#endif
#if (defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380))
MEM_ENTRY_DECLARE(acl_entry_t, 18);
MEM_ENTRY_DECLARE(meter_entry_t, 2);
MEM_ENTRY_DECLARE(vlan_untag_entry_t, 2);
MEM_ENTRY_DECLARE(vlan_igrcnvt_entry_t, 5);
MEM_ENTRY_DECLARE(vlan_egrcnvt_entry_t, 6);
MEM_ENTRY_DECLARE(routing_entry_t, 2);
MEM_ENTRY_DECLARE(mpls_lib_entry_t, 2);
MEM_ENTRY_DECLARE(sched_entry_t, 9);
MEM_ENTRY_DECLARE(spg_port_entry_t, 8);
MEM_ENTRY_DECLARE(out_q_entry_t, 8);
MEM_ENTRY_DECLARE(acl_igrRule_entry_t, 18);
MEM_ENTRY_DECLARE(acl_egrRule_entry_t, 17);
#endif

#endif  /* __HAL_CHIPDEF_ALLMEM_H__ */
