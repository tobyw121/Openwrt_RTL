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
 * $Revision: 21577 $
 * $Date: 2011-08-27 12:02:46 +0800 (Sat, 27 Aug 2011) $
 *
 * Purpose : Definition those public TRUNK APIs and its data type in the SDK.
 * 
 * Feature : The file have include the following module and sub-modules
 *           1) Trunk
 * 
 */ 

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_trunk.h>
#include <dal/ssw/dal_ssw_port.h>
#include <rtk/default.h>
#include <rtk/trunk.h>

/* 
 * Symbol Definition 
 */
#define RTK_DEFAULT_TRUNK_MODE_IN_SSW_FAMILY        TRUNK_MODE_NORMAL

/* 
 * Data Declaration 
 */
static uint32               trunk_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         trunk_sem[RTK_MAX_NUM_OF_UNIT];
static rtk_portmask_t       *pTrunkMemberSet[RTK_MAX_NUM_OF_UNIT];

#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
static rtk_portmask_t       *pTrunkLinkUpMemberSet[RTK_MAX_NUM_OF_UNIT];
#endif

const static uint16 linkAggregationGroupParameter0_regidx[] = {SSW_LINK_AGGREGATION_GROUP0_PARAMETER0r,
                                                               SSW_LINK_AGGREGATION_GROUP1_PARAMETER0r,
                                                               SSW_LINK_AGGREGATION_GROUP2_PARAMETER0r,
                                                               SSW_LINK_AGGREGATION_GROUP3_PARAMETER0r,
                                                               SSW_LINK_AGGREGATION_GROUP4_PARAMETER0r,
                                                               SSW_LINK_AGGREGATION_GROUP5_PARAMETER0r,
                                                               SSW_LINK_AGGREGATION_GROUP6_PARAMETER0r,
                                                               SSW_LINK_AGGREGATION_GROUP7_PARAMETER0r};
const static uint16 linkAggregationGroupParameter1_regidx[] = {SSW_LINK_AGGREGATION_GROUP0_PARAMETER1r, 
                                                               SSW_LINK_AGGREGATION_GROUP1_PARAMETER1r, 
                                                               SSW_LINK_AGGREGATION_GROUP2_PARAMETER1r, 
                                                               SSW_LINK_AGGREGATION_GROUP3_PARAMETER1r, 
                                                               SSW_LINK_AGGREGATION_GROUP4_PARAMETER1r, 
                                                               SSW_LINK_AGGREGATION_GROUP5_PARAMETER1r, 
                                                               SSW_LINK_AGGREGATION_GROUP6_PARAMETER1r, 
                                                               SSW_LINK_AGGREGATION_GROUP7_PARAMETER1r};

const static uint16 linkAggregationGroupParameter1_to_6_regidx[][RTK_MAX_NUM_OF_TRUNK_HASH_VAL] = {{SSW_LINK_AGGREGATION_GROUP0_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP0_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP0_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP0_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP0_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP0_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP0_PARAMETER6r},
                                                                                                   {SSW_LINK_AGGREGATION_GROUP1_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP1_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP1_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP1_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP1_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP1_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP1_PARAMETER6r},
                                                                                                   {SSW_LINK_AGGREGATION_GROUP2_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP2_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP2_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP2_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP2_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP2_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP2_PARAMETER6r},
                                                                                                   {SSW_LINK_AGGREGATION_GROUP3_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP3_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP3_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP3_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP3_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP3_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP3_PARAMETER6r}, 
                                                                                                   {SSW_LINK_AGGREGATION_GROUP4_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP4_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP4_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP4_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP4_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP4_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP4_PARAMETER6r}, 
                                                                                                   {SSW_LINK_AGGREGATION_GROUP5_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP5_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP5_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP5_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP5_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP5_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP5_PARAMETER6r}, 
                                                                                                   {SSW_LINK_AGGREGATION_GROUP6_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP6_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP6_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP6_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP6_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP6_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP6_PARAMETER6r}, 
                                                                                                   {SSW_LINK_AGGREGATION_GROUP7_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER1r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER1r, 
                                                                                                    SSW_LINK_AGGREGATION_GROUP7_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER2r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER2r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP7_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER3r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER3r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP7_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER4r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER4r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP7_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER5r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER5r,
                                                                                                    SSW_LINK_AGGREGATION_GROUP7_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER6r, SSW_LINK_AGGREGATION_GROUP7_PARAMETER6r}};
                                                                                                                          
const static uint16 grphashmask_fieldidx[] = {SSW_GRP0_HASH_MASKf, SSW_GRP1_HASH_MASKf, SSW_GRP2_HASH_MASKf, SSW_GRP3_HASH_MASKf, SSW_GRP4_HASH_MASKf, SSW_GRP5_HASH_MASKf, SSW_GRP6_HASH_MASKf, SSW_GRP7_HASH_MASKf};
const static uint16 grpportmask_fieldidx[] = {SSW_GRP0_TRK_MASKf, SSW_GRP1_TRK_MASKf, SSW_GRP2_TRK_MASKf, SSW_GRP3_TRK_MASKf, SSW_GRP4_TRK_MASKf, SSW_GRP5_TRK_MASKf, SSW_GRP6_TRK_MASKf, SSW_GRP7_TRK_MASKf};
const static uint16 grphashval_fieldidx[][RTK_MAX_NUM_OF_TRUNK_HASH_VAL] = {{SSW_GRP0_HASHV0f, SSW_GRP0_HASHV1f, SSW_GRP0_HASHV2f, SSW_GRP0_HASHV3f, SSW_GRP0_HASHV4f, SSW_GRP0_HASHV5f, SSW_GRP0_HASHV6f, SSW_GRP0_HASHV7f, SSW_GRP0_HASHV8f, SSW_GRP0_HASHV9f,
                                                                             SSW_GRP0_HASHV10f, SSW_GRP0_HASHV11f, SSW_GRP0_HASHV12f, SSW_GRP0_HASHV13f, SSW_GRP0_HASHV14f, SSW_GRP0_HASHV15f, SSW_GRP0_HASHV16f, SSW_GRP0_HASHV17f, SSW_GRP0_HASHV18f, SSW_GRP0_HASHV19f,
                                                                             SSW_GRP0_HASHV20f, SSW_GRP0_HASHV21f, SSW_GRP0_HASHV22f, SSW_GRP0_HASHV23f, SSW_GRP0_HASHV24f, SSW_GRP0_HASHV25f, SSW_GRP0_HASHV26f, SSW_GRP0_HASHV27f, SSW_GRP0_HASHV28f, SSW_GRP0_HASHV29f,
                                                                             SSW_GRP0_HASHV30f, SSW_GRP0_HASHV31f},
                                                                            {SSW_GRP1_HASHV0f, SSW_GRP1_HASHV1f, SSW_GRP1_HASHV2f, SSW_GRP1_HASHV3f, SSW_GRP1_HASHV4f, SSW_GRP1_HASHV5f, SSW_GRP1_HASHV6f, SSW_GRP1_HASHV7f, SSW_GRP1_HASHV8f, SSW_GRP1_HASHV9f,
                                                                             SSW_GRP1_HASHV10f, SSW_GRP1_HASHV11f, SSW_GRP1_HASHV12f, SSW_GRP1_HASHV13f, SSW_GRP1_HASHV14f, SSW_GRP1_HASHV15f, SSW_GRP1_HASHV16f, SSW_GRP1_HASHV17f, SSW_GRP1_HASHV18f, SSW_GRP1_HASHV19f,
                                                                             SSW_GRP1_HASHV20f, SSW_GRP1_HASHV21f, SSW_GRP1_HASHV22f, SSW_GRP1_HASHV23f, SSW_GRP1_HASHV24f, SSW_GRP1_HASHV25f, SSW_GRP1_HASHV26f, SSW_GRP1_HASHV27f, SSW_GRP1_HASHV28f, SSW_GRP1_HASHV29f,
                                                                             SSW_GRP1_HASHV30f, SSW_GRP1_HASHV31f},                                                                                                                            
                                                                            {SSW_GRP2_HASHV0f, SSW_GRP2_HASHV1f, SSW_GRP2_HASHV2f, SSW_GRP2_HASHV3f, SSW_GRP2_HASHV4f, SSW_GRP2_HASHV5f, SSW_GRP2_HASHV6f, SSW_GRP2_HASHV7f, SSW_GRP2_HASHV8f, SSW_GRP2_HASHV9f,
                                                                             SSW_GRP2_HASHV10f, SSW_GRP2_HASHV11f, SSW_GRP2_HASHV12f, SSW_GRP2_HASHV13f, SSW_GRP2_HASHV14f, SSW_GRP2_HASHV15f, SSW_GRP2_HASHV16f, SSW_GRP2_HASHV17f, SSW_GRP2_HASHV18f, SSW_GRP2_HASHV19f,
                                                                             SSW_GRP2_HASHV20f, SSW_GRP2_HASHV21f, SSW_GRP2_HASHV22f, SSW_GRP2_HASHV23f, SSW_GRP2_HASHV24f, SSW_GRP2_HASHV25f, SSW_GRP2_HASHV26f, SSW_GRP2_HASHV27f, SSW_GRP2_HASHV28f, SSW_GRP2_HASHV29f,
                                                                             SSW_GRP2_HASHV30f, SSW_GRP2_HASHV31f},
                                                                            {SSW_GRP3_HASHV0f, SSW_GRP3_HASHV1f, SSW_GRP3_HASHV2f, SSW_GRP3_HASHV3f, SSW_GRP3_HASHV4f, SSW_GRP3_HASHV5f, SSW_GRP3_HASHV6f, SSW_GRP3_HASHV7f, SSW_GRP3_HASHV8f, SSW_GRP3_HASHV9f,
                                                                             SSW_GRP3_HASHV10f, SSW_GRP3_HASHV11f, SSW_GRP3_HASHV12f, SSW_GRP3_HASHV13f, SSW_GRP3_HASHV14f, SSW_GRP3_HASHV15f, SSW_GRP3_HASHV16f, SSW_GRP3_HASHV17f, SSW_GRP3_HASHV18f, SSW_GRP3_HASHV19f,
                                                                             SSW_GRP3_HASHV20f, SSW_GRP3_HASHV21f, SSW_GRP3_HASHV22f, SSW_GRP3_HASHV23f, SSW_GRP3_HASHV24f, SSW_GRP3_HASHV25f, SSW_GRP3_HASHV26f, SSW_GRP3_HASHV27f, SSW_GRP3_HASHV28f, SSW_GRP3_HASHV29f,
                                                                             SSW_GRP3_HASHV30f, SSW_GRP3_HASHV31f},
                                                                            {SSW_GRP4_HASHV0f, SSW_GRP4_HASHV1f, SSW_GRP4_HASHV2f, SSW_GRP4_HASHV3f, SSW_GRP4_HASHV4f, SSW_GRP4_HASHV5f, SSW_GRP4_HASHV6f, SSW_GRP4_HASHV7f, SSW_GRP4_HASHV8f, SSW_GRP4_HASHV9f,
                                                                             SSW_GRP4_HASHV10f, SSW_GRP4_HASHV11f, SSW_GRP4_HASHV12f, SSW_GRP4_HASHV13f, SSW_GRP4_HASHV14f, SSW_GRP4_HASHV15f, SSW_GRP4_HASHV16f, SSW_GRP4_HASHV17f, SSW_GRP4_HASHV18f, SSW_GRP4_HASHV19f,
                                                                             SSW_GRP4_HASHV20f, SSW_GRP4_HASHV21f, SSW_GRP4_HASHV22f, SSW_GRP4_HASHV23f, SSW_GRP4_HASHV24f, SSW_GRP4_HASHV25f, SSW_GRP4_HASHV26f, SSW_GRP4_HASHV27f, SSW_GRP4_HASHV28f, SSW_GRP4_HASHV29f,
                                                                             SSW_GRP4_HASHV30f, SSW_GRP4_HASHV31f},
                                                                            {SSW_GRP5_HASHV0f, SSW_GRP5_HASHV1f, SSW_GRP5_HASHV2f, SSW_GRP5_HASHV3f, SSW_GRP5_HASHV4f, SSW_GRP5_HASHV5f, SSW_GRP5_HASHV6f, SSW_GRP5_HASHV7f, SSW_GRP5_HASHV8f, SSW_GRP5_HASHV9f,
                                                                             SSW_GRP5_HASHV10f, SSW_GRP5_HASHV11f, SSW_GRP5_HASHV12f, SSW_GRP5_HASHV13f, SSW_GRP5_HASHV14f, SSW_GRP5_HASHV15f, SSW_GRP5_HASHV16f, SSW_GRP5_HASHV17f, SSW_GRP5_HASHV18f, SSW_GRP5_HASHV19f,
                                                                             SSW_GRP5_HASHV20f, SSW_GRP5_HASHV21f, SSW_GRP5_HASHV22f, SSW_GRP5_HASHV23f, SSW_GRP5_HASHV24f, SSW_GRP5_HASHV25f, SSW_GRP5_HASHV26f, SSW_GRP5_HASHV27f, SSW_GRP5_HASHV28f, SSW_GRP5_HASHV29f,
                                                                             SSW_GRP5_HASHV30f, SSW_GRP5_HASHV31f},                                                 
                                                                            {SSW_GRP6_HASHV0f, SSW_GRP6_HASHV1f, SSW_GRP6_HASHV2f, SSW_GRP6_HASHV3f, SSW_GRP6_HASHV4f, SSW_GRP6_HASHV5f, SSW_GRP6_HASHV6f, SSW_GRP6_HASHV7f, SSW_GRP6_HASHV8f, SSW_GRP6_HASHV9f,
                                                                             SSW_GRP6_HASHV10f, SSW_GRP6_HASHV11f, SSW_GRP6_HASHV12f, SSW_GRP6_HASHV13f, SSW_GRP6_HASHV14f, SSW_GRP6_HASHV15f, SSW_GRP6_HASHV16f, SSW_GRP6_HASHV17f, SSW_GRP6_HASHV18f, SSW_GRP6_HASHV19f,
                                                                             SSW_GRP6_HASHV20f, SSW_GRP6_HASHV21f, SSW_GRP6_HASHV22f, SSW_GRP6_HASHV23f, SSW_GRP6_HASHV24f, SSW_GRP6_HASHV25f, SSW_GRP6_HASHV26f, SSW_GRP6_HASHV27f, SSW_GRP6_HASHV28f, SSW_GRP6_HASHV29f,
                                                                             SSW_GRP6_HASHV30f, SSW_GRP6_HASHV31f},
                                                                            {SSW_GRP7_HASHV0f, SSW_GRP7_HASHV1f, SSW_GRP7_HASHV2f, SSW_GRP7_HASHV3f, SSW_GRP7_HASHV4f, SSW_GRP7_HASHV5f, SSW_GRP7_HASHV6f, SSW_GRP7_HASHV7f, SSW_GRP7_HASHV8f, SSW_GRP7_HASHV9f,
                                                                             SSW_GRP7_HASHV10f, SSW_GRP7_HASHV11f, SSW_GRP7_HASHV12f, SSW_GRP7_HASHV13f, SSW_GRP7_HASHV14f, SSW_GRP7_HASHV15f, SSW_GRP7_HASHV16f, SSW_GRP7_HASHV17f, SSW_GRP7_HASHV18f, SSW_GRP7_HASHV19f,
                                                                             SSW_GRP7_HASHV20f, SSW_GRP7_HASHV21f, SSW_GRP7_HASHV22f, SSW_GRP7_HASHV23f, SSW_GRP7_HASHV24f, SSW_GRP7_HASHV25f, SSW_GRP7_HASHV26f, SSW_GRP7_HASHV27f, SSW_GRP7_HASHV28f, SSW_GRP7_HASHV29f,
                                                                             SSW_GRP7_HASHV30f, SSW_GRP7_HASHV31f}};

/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define TRUNK_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(trunk_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_TRUNK), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define TRUNK_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(trunk_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_TRUNK), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/* 
 * Function Declaration 
 */
static int32 _dal_ssw_trunk_init_config(uint32 unit);
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
static int32 _dal_ssw_trunk_setHashbyPortMask(uint32 unit, uint32 trk_gid);
#endif

/* Module Name    : port     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_ssw_trunk_init
 * Description:
 *      Initialize trunk module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize trunk module before calling any trunk APIs.
 */
int32
dal_ssw_trunk_init(uint32 unit)
{
    int32   ret;
    
    trunk_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    trunk_sem[unit] = osal_sem_mutex_create();
    if (0 == trunk_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_TRUNK), "semaphore create failed");
        return RT_ERR_FAILED;
    }
    
    /* allocate memory for trunk database for this unit */
    pTrunkMemberSet[unit] = (rtk_portmask_t *)osal_alloc(HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
    if (NULL == pTrunkMemberSet[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_TRUNK|MOD_DAL), "memory allocate failed");
        return RT_ERR_FAILED;
    }
    /* reset trunk member */
    osal_memset(pTrunkMemberSet[unit], 0, HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
    
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    pTrunkLinkUpMemberSet[unit] = (rtk_portmask_t *)osal_alloc(HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
    if (NULL == pTrunkLinkUpMemberSet[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_TRUNK|MOD_DAL), "memory allocate failed");
        osal_free(pTrunkMemberSet[unit]);
        pTrunkMemberSet[unit] = NULL;
        return RT_ERR_FAILED;
    }
    /* reset trunk member */
    osal_memset(pTrunkLinkUpMemberSet[unit], 0, HAL_MAX_NUM_OF_TRUNK(unit)*sizeof(rtk_portmask_t));
#endif

    /* set init flag to complete init */
    trunk_init[unit] = INIT_COMPLETED;    
    
    /* initialize default configuration */
    if ((ret = _dal_ssw_trunk_init_config(unit)) != RT_ERR_OK)
    {
        trunk_init[unit] = INIT_NOT_COMPLETED;
        osal_free(pTrunkMemberSet[unit]);
        pTrunkMemberSet[unit] = NULL;
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
        osal_free(pTrunkLinkUpMemberSet[unit]);
        pTrunkLinkUpMemberSet[unit] = NULL;
#endif
        RT_ERR(ret, (MOD_TRUNK|MOD_DAL), "init default configuration failed");
        return ret;
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_init */


/* Function Name:
 *      dal_ssw_trunk_distributionAlgorithm_get
 * Description:
 *      Get the distribution algorithm of the trunk group id from the specified device.
 * Input:
 *      unit           - unit id
 *      trk_gid        - trunk group id
 * Output:
 *      pAlgo_bitmask - pointer buffer of bitmask of the distribution algorithm
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. You can use OR opertions in following bits to decide your algorithm.
 *      - #define TRUNK_DISTRIBUTION_ALGO_SPA_BIT   (source port)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT  (source mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT  (destination mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SIP_BIT   (source ip)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DIP_BIT   (destination ip)
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_distributionAlgorithm_get(uint32 unit, uint32 trk_gid, uint32 *pAlgo_bitmask)
{
    int32 ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d",
           unit, trk_gid);
       
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit)), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pAlgo_bitmask), RT_ERR_NULL_POINTER);
      
    TRUNK_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, (uint32)linkAggregationGroupParameter1_regidx[trk_gid], (uint32)grphashmask_fieldidx[trk_gid], pAlgo_bitmask)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pAlgo_bitmask=%x",
           *pAlgo_bitmask);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_distributionAlgorithm_get */


/* Function Name:
 *      dal_ssw_trunk_distributionAlgorithm_set
 * Description:
 *      Set the distribution algorithm of the trunk group id from the specified device.
 * Input:
 *      unit         - unit id
 *      trk_gid      - trunk group id
 *      algo_bitmask - bitmask of the distribution algorithm
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 *      RT_ERR_LA_HASHMASK - invalid hash mask
 * Note:
 *      1. You can use OR opertions in following bits to decide your algorithm.
 *      - #define TRUNK_DISTRIBUTION_ALGO_SPA_BIT   (source port)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SMAC_BIT  (source mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DMAC_BIT  (destination mac)
 *      - #define TRUNK_DISTRIBUTION_ALGO_SIP_BIT   (source ip)
 *      - #define TRUNK_DISTRIBUTION_ALGO_DIP_BIT   (destination ip)
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_distributionAlgorithm_set(uint32 unit, uint32 trk_gid, uint32 algo_bitmask)
{
    int32 ret;
   
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d, pAlgo_bitmask=%x",
           unit, trk_gid, algo_bitmask);
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit)), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((algo_bitmask > TRUNK_DISTRIBUTION_ALGO_MASKALL), RT_ERR_LA_HASHMASK);
    RT_PARAM_CHK((algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT), RT_ERR_LA_HASHMASK);
    RT_PARAM_CHK((algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT), RT_ERR_LA_HASHMASK);
       
    TRUNK_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, (uint32)linkAggregationGroupParameter1_regidx[trk_gid], (uint32)grphashmask_fieldidx[trk_gid], &algo_bitmask)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_distributionAlgorithm_set */


/* Function Name:
 *      dal_ssw_trunk_hashMappingTable_get
 * Description:
 *      Get hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit              - unit id
 *      trk_gid           - trunk group id
 * Output:
 *      pHash2Port_array - pointer buffer of ports associate with the hash value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_hashMappingTable_get(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    int32   ret;
    uint32  hashVal_idx;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d",
           unit, trk_gid);
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pHash2Port_array), RT_ERR_NULL_POINTER);
       
    TRUNK_SEM_LOCK(unit);
    
    for (hashVal_idx = 0; hashVal_idx < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); hashVal_idx++)
    {
        /* get entry from CHIP*/
        if ((ret = reg_field_read(unit, 
                                  (uint32)linkAggregationGroupParameter1_to_6_regidx[trk_gid][hashVal_idx], 
                                  (uint32)grphashval_fieldidx[trk_gid][hashVal_idx], 
                                  &value)) != RT_ERR_OK)        
        {
            TRUNK_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
            return ret;
        }
        
        pHash2Port_array->value[hashVal_idx] = (uint8) value;
    }
    
    TRUNK_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pHash2Port_array=%x",
           *pHash2Port_array);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_hashMappingTable_get */


/* Function Name:
 *      dal_ssw_trunk_hashMappingTable_set
 * Description:
 *      Set hash value to port array in the trunk group id from the specified device.
 * Input:
 *      unit              - unit id
 *      trk_gid           - trunk group id
 *      pHash2Port_array - ports associate with the hash value
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID            - invalid unit id
 *      RT_ERR_LA_TRUNK_ID        - invalid trunk ID
 *      RT_ERR_NULL_POINTER       - input parameter may be null pointer
 *      RT_ERR_LA_TRUNK_NOT_EXIST - the trunk doesn't exist                     
 *      RT_ERR_LA_NOT_MEMBER_PORT - the port is not a member port of the trunk  
 *      RT_ERR_LA_CPUPORT         - CPU port can not be aggregated port         
 * Note:
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_hashMappingTable_set(
    uint32                   unit,
    uint32                   trk_gid,
    rtk_trunk_hashVal2Port_t *pHash2Port_array)
{
    int32   ret;
    uint32  hashVal_idx;
    uint32  value;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", unit, trk_gid);  
        
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pHash2Port_array), RT_ERR_NULL_POINTER);
    for (hashVal_idx = 0; hashVal_idx < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); hashVal_idx++)
    {
        RT_PARAM_CHK((pHash2Port_array->value[hashVal_idx] > (HAL_GET_MAX_ETHER_PORT(unit))), RT_ERR_PORT_ID);
    }
    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pHash2Port_array=%x", pHash2Port_array);  
       
    TRUNK_SEM_LOCK(unit);
    /* should we add checking for input value of hask2port_array */
    for (hashVal_idx = 0; hashVal_idx < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); hashVal_idx++)
    {                      
        value = (uint32)(pHash2Port_array->value[hashVal_idx]);
        /* write value to CHIP*/
        if ((ret = reg_field_write(unit, 
                                   (uint32)linkAggregationGroupParameter1_to_6_regidx[trk_gid][hashVal_idx], 
                                   (uint32)grphashval_fieldidx[trk_gid][hashVal_idx], 
                                   &value)) != RT_ERR_OK)
        {
            TRUNK_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
            return ret;
        }
    }
    
    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_hashMappingTable_set */


/* Function Name:
 *      dal_ssw_trunk_mode_get
 * Description:
 *      Get the trunk mode from the specified device.
 * Input:
 *      unit   - unit id
 * Output:
 *      pMode - pointer buffer of trunk mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_mode_get(uint32 unit, rtk_trunk_mode_t *pMode)
{
    int32   ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pMode), RT_ERR_NULL_POINTER);
    
    value = 0;
    TRUNK_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, SSW_LINK_AGGREGATION_CONTROL0r, SSW__3AD_DUMBf, &value)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    switch (value)
    {
        case 0:
            *pMode = TRUNK_MODE_NORMAL;
            break;
        case 1:
            *pMode = TRUNK_MODE_DUMB;
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pMode=%d", *pMode);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_mode_get */


/* Function Name:
 *      dal_ssw_trunk_mode_set
 * Description:
 *      Set the trunk mode to the specified device.
 * Input:
 *      unit - unit id
 *      mode - trunk mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_INPUT   - invalid input parameter 
 * Note:
 *      1. The enum of the trunk mode as following
 *      - TRUNK_MODE_NORMAL
 *      - TRUNK_MODE_DUMB
 *      2. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_mode_set(uint32 unit, rtk_trunk_mode_t mode)
{
    int32 ret;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, mode=%d", unit, mode);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((mode >= TRUNK_MODE_END), RT_ERR_INPUT); 
    
    switch (mode)
    {
        case TRUNK_MODE_NORMAL:
            value = 0; 
            break;
        case TRUNK_MODE_DUMB:
            value = 1; 
            break;
        default:
            return RT_ERR_FAILED;
    }
    
    TRUNK_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_LINK_AGGREGATION_CONTROL0r, SSW__3AD_DUMBf, &value)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_mode_set */


/* Function Name:
 *      dal_ssw_trunk_port_get
 * Description:
 *      Get the members of the trunk id from the specified device.
 * Input:
 *      unit                    - unit id
 *      trk_gid                 - trunk group id
 * Output:
 *      pTrunk_member_portmask - pointer buffer of trunk member ports
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_LA_TRUNK_ID  - invalid trunk ID
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_port_get(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    int32   ret;
    uint32  value;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", 
           unit, trk_gid);  
    
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID); /* trunk_id is max_num - 1, so it will use "trk_gid > max_num_of_trunk" */
    RT_PARAM_CHK((NULL == pTrunk_member_portmask), RT_ERR_NULL_POINTER);
        
    TRUNK_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_read(unit, 
                              (uint32)linkAggregationGroupParameter0_regidx[trk_gid], 
                              (uint32)grpportmask_fieldidx[trk_gid], 
                              &value)) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    RTK_PORTMASK_WORD_SET(*pTrunk_member_portmask, 0, value);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pTrunk_member_portmask=%x", 
           pTrunk_member_portmask->bits[0]); 
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_port_get */


/* Function Name:
 *      dal_ssw_trunk_port_set
 * Description:
 *      Set the members of the trunk id to the specified device.
 * Input:
 *      unit                    - unit id
 *      trk_gid                 - trunk group id
 *      pTrunk_member_portmask - trunk member ports
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID           - invalid unit id
 *      RT_ERR_LA_TRUNK_ID       - invalid trunk ID
 *      RT_ERR_NULL_POINTER      - null pointer
 *      RT_ERR_LA_MEMBER_OVERLAP - the specified port mask is overlapped with other group
 *      RT_ERR_LA_PORTNUM_DUMB   - it can only aggregate at most four ports when 802.1ad dumb mode
 *      RT_ERR_LA_PORTNUM_NORMAL - it can only aggregate at most eight ports when 802.1ad normal mode
 *      RT_ERR_LA_PORTMASK       - error port mask
 * Note:
 *      1. Normal mode support 8 trunk members in each group, but dumb mode only 4.
 */
int32
dal_ssw_trunk_port_set(uint32 unit, uint32 trk_gid, rtk_portmask_t *pTrunk_member_portmask)
{
    int32   ret;
    uint32  grp_num;
    uint32  num_of_port;
    rtk_trunk_mode_t  trunkMode;
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    rtk_port_t trunkMemberPort;
#endif

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, trk_gid=%d", unit, trk_gid); 
        
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(trk_gid >= HAL_MAX_NUM_OF_TRUNK(unit), RT_ERR_LA_TRUNK_ID);
    RT_PARAM_CHK((NULL == pTrunk_member_portmask), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(pTrunk_member_portmask->bits[0] >= (1 << (HAL_GET_MAX_PORT(unit))), RT_ERR_LA_PORTMASK);

    /* Display debug message */
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "pTrunk_member_portmask=%x", pTrunk_member_portmask->bits[0]); 
    
    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_ssw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    
    /* check number of trunk member */
    if (TRUNK_MODE_NORMAL == trunkMode )
    {
        RT_PARAM_CHK(((num_of_port = RTK_PORTMASK_GET_PORT_COUNT(*pTrunk_member_portmask)) > HAL_MAX_NUM_OF_TRUNKMEMBER(unit)), RT_ERR_LA_PORTNUM_NORMAL);
    } 
    else 
    {
        RT_PARAM_CHK(((num_of_port = RTK_PORTMASK_GET_PORT_COUNT(*pTrunk_member_portmask)) > HAL_MAX_NUM_OF_DUMB_TRUNKMEMBER(unit)), RT_ERR_LA_PORTNUM_DUMB);
    }
    
    /* check whether new member set is overlap with other trunk */
    for (grp_num = 0; grp_num < HAL_MAX_NUM_OF_TRUNK(unit); grp_num++)
    {
        if (grp_num != trk_gid)
        {
            RT_PARAM_CHK((pTrunk_member_portmask->bits[0] & pTrunkMemberSet[unit][grp_num].bits[0]) 
                        , RT_ERR_LA_MEMBER_OVERLAP);
        }
    }        
    
    TRUNK_SEM_LOCK(unit);
    /* get entry from CHIP */
    if ((ret = reg_field_write(unit, 
                               linkAggregationGroupParameter0_regidx[trk_gid], 
                               (uint32)grpportmask_fieldidx[trk_gid], 
                               &(pTrunk_member_portmask->bits[0]))) != RT_ERR_OK)
    {
        TRUNK_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    TRUNK_SEM_UNLOCK(unit);
    
    RTK_PORTMASK_ASSIGN(pTrunkMemberSet[unit][trk_gid], *pTrunk_member_portmask);
    

#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    /* Maintain link up trunk member port mask */
    /* Reset port mask to empty */
    RTK_PORTMASK_RESET(pTrunkLinkUpMemberSet[unit][trk_gid]);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Clear pTrunkLinkUpMemberSet[%d][%d]=0x%x", unit, trk_gid, pTrunkLinkUpMemberSet[unit][trk_gid].bits[0]); 
    /* Check member port link status to create link up port bitmask */
    /* Scan all ports */
    for (trunkMemberPort = 0; trunkMemberPort < RTK_MAX_NUM_OF_PORTS; trunkMemberPort++)
    {
        /* If port is in the member port list */
        if (RTK_PORTMASK_IS_PORT_SET(pTrunkMemberSet[unit][trk_gid], trunkMemberPort))
        {
            rtk_port_linkStatus_t linkStatus;

            /*Check link status of the member port*/
            if ((ret = dal_ssw_port_link_get(unit, trunkMemberPort, &linkStatus)))
            {
                RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "Get port link status fail");
                return ret;
            }
            dal_ssw_trunk_port_link_notification(unit, trunkMemberPort, linkStatus);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, Trunk=%d, Trunk_member_port=0x%d", unit,trk_gid, trunkMemberPort); 
        }
    }
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Updated pTrunkLinkUpMemberSet[%d][%d]=0x%x", unit, trk_gid, pTrunkLinkUpMemberSet[unit][trk_gid].bits[0]); 
#endif
    
    return RT_ERR_OK;
} /* end of dal_ssw_trunk_port_set */

/* Function Name:
 *      dal_ssw_trunk_port_link_notification
 * Description:
 *      Notify link state change of ports to trunk failover handling.
 * Input:
 *      unit            - unit id
 *      trunkMemberPort - link state change port 
 *      linkStatus      - the new link state
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID     - invalid unit id
 *      RT_ERR_LA_TRUNK_ID - invalid trunk ID
 * Note:
 *      None
 */
int32
dal_ssw_trunk_port_link_notification(uint32 unit, rtk_port_t trunkMemberPort, rtk_port_linkStatus_t linkStatus)
{
#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
    uint32  ret;
    uint32  grp_num;
    rtk_trunk_mode_t trunkMode;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "unit=%d, Trunk_member_port=%d, LinkStatus=%d", unit, trunkMemberPort, linkStatus); 
        
    /* check Init status */
    RT_INIT_CHK(trunk_init[unit]);

    /* get trunk mode for checking number of trunk member */
    if ((ret = dal_ssw_trunk_mode_get(unit, &trunkMode)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "Get Trunk mode fail");
        return ret;
    }
    
    /* check number of trunk member */
    if (TRUNK_MODE_NORMAL != trunkMode )
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "Only work on Normal mode");
        return ret;
    }

    /* Check all trunk to see if incoming port is belong to a trunk */
    for (grp_num = 0; grp_num < HAL_MAX_NUM_OF_TRUNK(unit); grp_num++)
    {
        /* If the port is in the trunk member list */
        if (RTK_PORTMASK_IS_PORT_SET(pTrunkMemberSet[unit][grp_num], trunkMemberPort))
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Ready surve Tgid=%d, TrunkMemberPortMask=0x%x, LinkupPortMask=0x%x", 
                   grp_num, pTrunkMemberSet[unit][grp_num].bits[0], pTrunkLinkUpMemberSet[unit][grp_num].bits[0]);
            
            /* Update trunkLinkUpMemberSet */
            if (PORT_LINKUP == linkStatus)
            {
                /* Link up to add the port to the bitmask */
                RTK_PORTMASK_PORT_SET(pTrunkLinkUpMemberSet[unit][grp_num], trunkMemberPort);
            }
            else if (PORT_LINKDOWN == linkStatus)
            {
                /* Link down to add the port to the bitmask */
                RTK_PORTMASK_PORT_CLEAR(pTrunkLinkUpMemberSet[unit][grp_num], trunkMemberPort);
            }
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Tgid=%d, TrunkMemberPortMask=0x%x, LinkupPortMask=0x%x", 
                   grp_num, pTrunkMemberSet[unit][grp_num].bits[0], pTrunkLinkUpMemberSet[unit][grp_num].bits[0]);

            /* Update the hash mapping table */
            ret = _dal_ssw_trunk_setHashbyPortMask(unit, grp_num);
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Tgid=%d break", grp_num);
            break;
        }
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "finished");
    return RT_ERR_OK;
#else
    return RT_ERR_DRIVER_NOT_FOUND;
#endif

}/* end of dal_ssw_trunk_port_link_notification */

/* Function Name:
 *      _dal_ssw_trunk_init_config
 * Description:
 *      Initialize default configuration for trunk module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      Must initialize trunk module before calling this API
 */
static int32
_dal_ssw_trunk_init_config(uint32 unit)
{
    int32   ret;
    uint32  tgid;
    rtk_portmask_t portmask;
    
    portmask.bits[0] = RTK_DEFAULT_TRUNK_MEMBER_PORTMASK;
    
    if ((ret = dal_ssw_trunk_mode_set(unit, RTK_DEFAULT_TRUNK_MODE_IN_SSW_FAMILY)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_TRUNK|MOD_DAL), "");
        return ret;
    }
            
    for (tgid = 0; tgid < HAL_MAX_NUM_OF_TRUNK(unit); tgid++)
    {
        if ((ret = dal_ssw_trunk_port_set(unit, tgid, &portmask)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_TRUNK|MOD_DAL), "");
            return ret;
        }         
        if ((ret = dal_ssw_trunk_distributionAlgorithm_set(unit, tgid, RTK_DEFAULT_TRUNK_DISTRIBUTION_ALGORITHM)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_TRUNK|MOD_DAL), "");
            return ret;
        }                
    }
    
    return RT_ERR_OK;
} /* end of _dal_ssw_trunk_init_config */

#if defined(CONFIG_TRUNK_FAILOVER_HANDLING)
/* Function Name:
 *      _dal_ssw_trunk_setHashbyPortMask
 * Description:
 *      Update Hash mapping table by trunk group ID with link up member port list
 * Input:
 *      unit    - unit id
 *      trk_gid - trunk group id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32
_dal_ssw_trunk_setHashbyPortMask(uint32 unit, uint32 trk_gid)
{
    uint32  hashVal_idx;
    uint32  numOfLinkupPort = 0;
    int32   ret;
    rtk_port_t member_port, trunk_member[HAL_MAX_NUM_OF_TRUNKMEMBER(unit)];
    rtk_trunk_hashVal2Port_t hashVal;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Enter HashHandling Tgid=%d, TrunkLinkUpMemberSet=0x%x", trk_gid, pTrunkLinkUpMemberSet[unit][trk_gid]);
    
    osal_memset(&hashVal, 0, sizeof(rtk_trunk_hashVal2Port_t));

    for (member_port = 0; member_port < RTK_MAX_NUM_OF_PORTS; member_port++)
    {
        if (RTK_PORTMASK_IS_PORT_SET(pTrunkLinkUpMemberSet[unit][trk_gid], member_port))
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "HashHandling number=%d, trunk_member=%d", numOfLinkupPort, member_port);
            trunk_member[numOfLinkupPort] = member_port;
            numOfLinkupPort++;
        }
    }
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "HashHandling total numOfLinkupPort=%d", numOfLinkupPort);
    if (numOfLinkupPort > 0)
    {
        for (hashVal_idx = 0; hashVal_idx < HAL_MAX_NUM_OF_TRUNKHASHVAL(unit); hashVal_idx++)
        {
            hashVal.value[hashVal_idx] = trunk_member[hashVal_idx - (hashVal_idx / numOfLinkupPort) * numOfLinkupPort];
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_TRUNK), "Tgid=%d, hashVal=%d", trk_gid, hashVal.value[hashVal_idx]);
        }
    }
    if ((ret = dal_ssw_trunk_hashMappingTable_set(unit, trk_gid, &hashVal)))
    {
        RT_ERR(ret, (MOD_DAL|MOD_TRUNK), "");
        return ret;
    }
    return RT_ERR_OK;
} /* end of _dal_ssw_trunk_setHashbyPortMask */
#endif
