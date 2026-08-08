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
 * $Revision: 41809 $
 * $Date: 2013-08-05 16:12:02 +0800 (Mon, 05 Aug 2013) $
 *
 * Purpose : Definition those TRUNK command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) trunk configuration
 *
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <rtk/switch.h>
#include <rtk/port.h>
#include <rtk/trunk.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


#ifdef CMD_TRUNK_DUMP_TRUNK_GROUP_ALL
/*
 * trunk dump trunk-group all
 */
cparser_result_t cparser_cmd_trunk_dump_trunk_group_all(cparser_context_t *context)
{
    uint32                      unit = 0;
    uint32                      min_trk_gid = 0;
    uint32                      max_trk_gid = 0;
    uint32                      algo_bitmask = 0, algo_bitmask_count = 0;
    uint32                      index = 0;
    uint32                      trunk_index = 0;
    uint32                      max_hash_value = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_trunk_mode_t            mode = TRUNK_MODE_NORMAL;
    rtk_portmask_t              trunk_member_portmask;
    rtk_trunk_hashVal2Port_t    hash2Port_array;
    rtk_port_t                  trunkResperentPort, floodPort;
    rtk_trunk_floodMode_t       floodMode;
    uint8                       memPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_switch_devInfo_t        devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        DIAG_UTIL_ERR_CHK(rtk_trunk_mode_get(unit, &mode), ret);
        if (TRUNK_MODE_DUMB == mode)
        {
            diag_util_mprintf("Trunk mode : dumb\n");
        }
        else
        {
            diag_util_mprintf("Trunk mode : normal\n");
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    /* show all trunk groups inforamtion */
    min_trk_gid = 0; /* PORT_MIN_TRUNK */
    max_trk_gid = devInfo.capacityInfo.max_num_of_trunk - 1; /* PORT_MAX_TRUNK */

    for (trunk_index = min_trk_gid; trunk_index <= max_trk_gid; trunk_index++)
    {
        diag_util_mprintf("\nTrunk %d:\n", trunk_index);

        if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithm_get(unit, trunk_index, &algo_bitmask), ret);
            diag_util_mprintf("Distribution algorithm : ");
            algo_bitmask_count = 0;
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SPA_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-port":",src-port");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-mac":",src-mac");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-mac":",dst-mac");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SIP_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-ip":",src-ip");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DIP_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-ip":",dst-ip");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-l4-port":",src-l4-port");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-l4-port":",dst-l4-port");
            }
            diag_util_mprintf("\n");
        }

        memset(&trunk_member_portmask, 0, sizeof(rtk_portmask_t));
        DIAG_UTIL_ERR_CHK(rtk_trunk_port_get(unit, trunk_index, &trunk_member_portmask), ret);
        memset(memPortList, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(memPortList, &trunk_member_portmask);
        diag_util_mprintf("Member : %s\n", memPortList);

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            if (TRUNK_MODE_NORMAL == mode)
            {
                ret = rtk_trunk_representPort_get(unit, trunk_index, &trunkResperentPort);
                if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
                else
                    diag_util_mprintf("Represent port : %u\n", trunkResperentPort);
    
                ret = rtk_trunk_floodMode_get(unit, trunk_index, &floodMode);
                if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
                else
                {
                    if(FLOOD_MODE_BY_CONFIG == floodMode)
                    {
                        diag_util_mprintf("Flood mode : by config\n");
    
                        ret = rtk_trunk_floodPort_get(unit, trunk_index, &floodPort);
                        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                        {
                            DIAG_ERR_PRINT(ret);
                            return CPARSER_NOT_OK;
                        }
                        else
                        {
                            diag_util_mprintf("   Flood configed port number : %u\n", floodPort);
                        }
                    }
                    else
                        diag_util_mprintf("Flood mode : by hash value mapping\n");
                }
    
                memset(&hash2Port_array, 0, sizeof(rtk_trunk_hashVal2Port_t));
                DIAG_UTIL_ERR_CHK(rtk_trunk_hashMappingTable_get(unit, trunk_index, &hash2Port_array), ret);
                diag_util_mprintf("Hash mapping table:\n");
                diag_util_mprintf("Hash value | Port \n");
                diag_util_mprintf("-----------+------\n");
                max_hash_value = devInfo.capacityInfo.max_num_of_trunkHashVal;
                for (index = 0; index < max_hash_value; index++)
                {
                    diag_util_mprintf("%10d   %3d\n", index, hash2Port_array.value[index]);
                }
            }
        }
    } /* end of for (trunk_index = min_trk_gid; trunk_index <= max_trk_gid; trunk_index++) */

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_dump_trunk_id */
#endif

#ifdef CMD_TRUNK_DUMP_TRUNK_GROUP_TRUNK_ID
/*
 * trunk dump trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_dump_trunk_group_trunk_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                      unit = 0;
    uint32                      min_trk_gid = 0;
    uint32                      max_trk_gid = 0;
    uint32                      algo_bitmask = 0, algo_bitmask_count = 0;
    uint32                      index = 0;
    uint32                      trunk_index = 0;
    uint32                      max_hash_value = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_trunk_mode_t            mode = TRUNK_MODE_NORMAL;
    rtk_portmask_t              trunk_member_portmask;
    rtk_trunk_hashVal2Port_t    hash2Port_array;
    rtk_port_t                  trunkResperentPort, floodPort;
    rtk_trunk_floodMode_t       floodMode;
    uint8                       memPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_switch_devInfo_t        devInfo;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
    {
        DIAG_UTIL_ERR_CHK(rtk_trunk_mode_get(unit, &mode), ret);
        if (TRUNK_MODE_DUMB == mode)
        {
            diag_util_mprintf("Trunk mode : dumb\n");
        }
        else
        {
            diag_util_mprintf("Trunk mode : normal\n");
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    /* show specific trunk group inforamtion */
    min_trk_gid = *trunk_id_ptr;
    max_trk_gid = *trunk_id_ptr;

    for (trunk_index = min_trk_gid; trunk_index <= max_trk_gid; trunk_index++)
    {
        diag_util_mprintf("\nTrunk %d:\n", trunk_index);

        if (DIAG_OM_GET_FAMILYID(RTL8389_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithm_get(unit, trunk_index, &algo_bitmask), ret);
            diag_util_mprintf("Distribution algorithm : ");
            algo_bitmask_count = 0;
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SPA_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-port":",src-port");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-mac":",src-mac");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-mac":",dst-mac");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SIP_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-ip":",src-ip");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DIP_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-ip":",dst-ip");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-l4-port":",src-l4-port");
            }
            if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT)
            {
                algo_bitmask_count++;
                diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-l4-port":",dst-l4-port");
            }
            diag_util_mprintf("\n");
        }

        memset(&trunk_member_portmask, 0, sizeof(rtk_portmask_t));
        DIAG_UTIL_ERR_CHK(rtk_trunk_port_get(unit, trunk_index, &trunk_member_portmask), ret);
        memset(memPortList, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
        diag_util_lPortMask2str(memPortList, &trunk_member_portmask);
        diag_util_mprintf("Member : %s\n", memPortList);

        if (DIAG_OM_GET_FAMILYID(RTL8328_FAMILY_ID))
        {
            if (TRUNK_MODE_NORMAL == mode)
            {
                ret = rtk_trunk_representPort_get(unit, trunk_index, &trunkResperentPort);
                if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
                else
                    diag_util_mprintf("Represent port : %u\n", trunkResperentPort);
    
                ret = rtk_trunk_floodMode_get(unit, trunk_index, &floodMode);
                if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                {
                    DIAG_ERR_PRINT(ret);
                    return CPARSER_NOT_OK;
                }
                else
                {
                    if(FLOOD_MODE_BY_CONFIG == floodMode)
                    {
                        diag_util_mprintf("Flood mode : by config\n");
    
                        ret = rtk_trunk_floodPort_get(unit, trunk_index, &floodPort);
                        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
                        {
                            DIAG_ERR_PRINT(ret);
                            return CPARSER_NOT_OK;
                        }
                        else
                        {
                            diag_util_mprintf("   Flood configed port number : %u\n", floodPort);
                        }
                    }
                    else
                        diag_util_mprintf("Flood mode : by hash value mapping\n");
                }
    
                memset(&hash2Port_array, 0, sizeof(rtk_trunk_hashVal2Port_t));
                DIAG_UTIL_ERR_CHK(rtk_trunk_hashMappingTable_get(unit, trunk_index, &hash2Port_array), ret);
                diag_util_mprintf("Hash mapping table:\n");
                diag_util_mprintf("Hash value | Port \n");
                diag_util_mprintf("-----------+------\n");
                max_hash_value = devInfo.capacityInfo.max_num_of_trunkHashVal;
                for (index = 0; index < max_hash_value; index++)
                {
                    diag_util_mprintf("%10d   %3d\n", index, hash2Port_array.value[index]);
                }
            }
        }
    } /* end of for (trunk_index = min_trk_gid; trunk_index <= max_trk_gid; trunk_index++) */

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_dump_trunk_id */
#endif

#ifdef CMD_TRUNK_GET_MODE
/*
 * trunk get mode
 */
cparser_result_t cparser_cmd_trunk_get_mode(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_trunk_mode_t    mode = TRUNK_MODE_NORMAL;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trunk_mode_get(unit, &mode), ret);
    if (TRUNK_MODE_DUMB == mode)
    {
        diag_util_mprintf("Trunk mode: dumb\n");
    }
    else
    {
        diag_util_mprintf("Trunk mode: normal\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_get_mode */
#endif


#ifdef CMD_TRUNK_GET_MEMBER_PORT_TRUNK_GROUP_TRUNK_ID
/*
 * trunk get member-port trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_get_member_port_trunk_group_trunk_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    uint8                       memPortList[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_portmask_t              trunk_member_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&trunk_member_portmask, 0, sizeof(rtk_portmask_t));
    DIAG_UTIL_ERR_CHK(rtk_trunk_port_get(unit, *trunk_id_ptr, &trunk_member_portmask), ret);
    memset(memPortList, 0, DIAG_UTIL_PORT_MASK_STRING_LEN);
    diag_util_lPortMask2str(memPortList, &trunk_member_portmask);
    diag_util_mprintf("Member : %s\n", memPortList);

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_get_member_port_trunk_id */
#endif

#ifdef CMD_TRUNK_GET_FLOOD_MODE_TRUNK_GROUP_TRUNK_ID
/*
 * trunk get flood-mode trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_get_flood_mode_trunk_group_trunk_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                      unit = 0;
    uint32                      trunk_index = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_trunk_floodMode_t       floodMode;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_trunk_floodMode_get(unit, trunk_index, &floodMode);
    if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        if(FLOOD_MODE_BY_CONFIG == floodMode)
        {
            diag_util_mprintf("Flood mode : by config\n");
        }
        else
            diag_util_mprintf("Flood mode : by hash value mapping\n");
    }


    return CPARSER_OK;
} /* end of cparser_cmd_trunk_get_flood_mode_trunk_id */
#endif


#ifdef CMD_TRUNK_GET_REPRESENT_PORT_TRUNK_GROUP_TRUNK_ID
/*
 * trunk get represent-port trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_get_represent_port_trunk_group_trunk_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_trunk_mode_t            mode = TRUNK_MODE_NORMAL;
    rtk_port_t                  trunkResperentPort;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trunk_mode_get(unit, &mode), ret);

    if (TRUNK_MODE_NORMAL == mode)
    {
        ret = rtk_trunk_representPort_get(unit, *trunk_id_ptr, &trunkResperentPort);
        if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
        else
            diag_util_mprintf("Represent port : %u\n", trunkResperentPort);
    }
    else
        diag_util_mprintf("Represent port is invalid when trunk is dumb mode\n");

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_get_represent_port_trunk_id */
#endif

#ifdef CMD_TRUNK_GET_FLOOD_CONFIG_PORT_TRUNK_GROUP_TRUNK_ID
/*
 * trunk get flood-config-port trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_get_flood_config_port_trunk_group_trunk_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                      unit = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                  floodPort;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_trunk_floodPort_get(unit, *trunk_id_ptr, &floodPort);
    if((ret != RT_ERR_OK) && (ret != RT_ERR_CHIP_NOT_SUPPORTED))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    else
    {
        diag_util_mprintf("Flood configed port number : %u\n", floodPort);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_get_flood_config_port_trunk_id */
#endif

#ifdef CMD_TRUNK_GET_DISTRIBUTE_ALGORITHM_TRUNK_GROUP_TRUNK_ID
/*
 * trunk get distribute-algorithm trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_get_distribute_algorithm_trunk_group_trunk_id(cparser_context_t *context, uint32_t *trunk_id_ptr)
{
    uint32  unit = 0;
    uint32  algo_bitmask = 0, algo_bitmask_count = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("\nTrunk %d:\n", *trunk_id_ptr);
    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithm_get(unit, *trunk_id_ptr, &algo_bitmask), ret);
    diag_util_mprintf("Distribution algorithm : ");
    algo_bitmask_count = 0;
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SPA_BIT)
    {
        algo_bitmask_count++;
        diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-port":",src-port");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
    {
        algo_bitmask_count++;
        diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-mac":",src-mac");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
    {
        algo_bitmask_count++;
        diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-mac":",dst-mac");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SIP_BIT)
    {
        algo_bitmask_count++;
        diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-ip":",src-ip");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DIP_BIT)
    {
        algo_bitmask_count++;
        diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-ip":",dst-ip");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT)
    {
        algo_bitmask_count++;
        diag_util_mprintf("%s",(algo_bitmask_count == 1)?"src-l4-port":",src-l4-port");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT)
    {
        algo_bitmask_count++;
        diag_util_mprintf("%s",(algo_bitmask_count == 1)?"dst-l4-port":",dst-l4-port");
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_get_distribute_algorithm_trunk_id */
#endif



#ifdef CMD_TRUNK_GET_HASH_MAPPING_TRUNK_GROUP_TRUNK_ID
/*
 * trunk get hash-mapping trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_get_hash_mapping_trunk_group_trunk_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                      unit = 0;
    uint32                      max_hash_value = 0;
    uint32                      index = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_trunk_hashVal2Port_t    hash2Port_array;
    rtk_switch_devInfo_t        devInfo;
    rtk_trunk_mode_t            mode = TRUNK_MODE_NORMAL;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_switch_deviceInfo_get(unit, &devInfo), ret);

    if (TRUNK_MODE_NORMAL == mode)
    {
        memset(&hash2Port_array, 0, sizeof(rtk_trunk_hashVal2Port_t));
        DIAG_UTIL_ERR_CHK(rtk_trunk_hashMappingTable_get(unit, *trunk_id_ptr, &hash2Port_array), ret);
        diag_util_mprintf("Hash mapping table:\n");
        diag_util_mprintf("Hash value | Port \n");
        diag_util_mprintf("-----------+------\n");
        max_hash_value = devInfo.capacityInfo.max_num_of_trunkHashVal;
        for (index = 0; index < max_hash_value; index++)
        {
            diag_util_mprintf("%10d   %3d\n", index, hash2Port_array.value[index]);
        }
    }
    else
        diag_util_mprintf("hash-mapping is invalid when trunk mode is Normal\n");

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_get_hash_mapping_trunk_id_hash_value_port */
#endif


#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_TRUNK_GROUP_TRUNK_ID_SRC_PORT_SRC_MAC_DST_MAC_SRC_IP_DST_IP_SRC_L4_PORT_DST_L4_PORT
/*
 * trunk set distribute-algorithm trunk-group <UINT:trunk_id> { src-port } { src-mac } { dst-mac } { src-ip } { dst-ip } { src-l4-port } { dst-l4-port }
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_trunk_group_trunk_id_src_port_src_mac_dst_mac_src_ip_dst_ip_src_l4_port_dst_l4_port(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32  unit = 0;
    uint32  trk_gid = 0;
    uint32  algo_bitmask = 0;
    uint32  option_num = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    trk_gid = *trunk_id_ptr;
    if (TOKEN_NUM < 6)
    {
        diag_util_mprintf("User config: ERROR! need to input algorithm\n");
    }
    else
    {
        for (option_num = 5; option_num < TOKEN_NUM; option_num++)
        {
            if ('s' == TOKEN_CHAR(option_num, 0))
            {
                if ('p' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SPA_BIT;
                }
                else if ('m' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SMAC_BIT;
                }
                else if ('i' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SIP_BIT;
                }
                else if ('l' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT;
                }
                else
                {
                    diag_util_mprintf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }
            }
            else if ('d' == TOKEN_CHAR(option_num, 0))
            {
                if ('m' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DMAC_BIT;
                }
                else if ('i' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DIP_BIT;
                }
                else if ('l' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT;
                }
                else
                {
                    diag_util_mprintf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }
            }
            else
            {
                diag_util_mprintf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }
        } /* end of for (option_num = 4; option_num < TOKEN_NUM; option_num++) */
    }

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithm_set(unit, trk_gid, algo_bitmask), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trunk_set_distribute_algorithm_trunk_id_src_port_src_mac_dst_mac_src_ip_dst_ip_src_l4_port_dst_l4_port */
#endif

#ifdef CMD_TRUNK_SET_FLOOD_MODE_TRUNK_GROUP_TRUNK_ID_CONFIG_HASH
/*
 * trunk set flood-mode trunk-group <UINT:trunk_id> ( config | hash )
 */
cparser_result_t cparser_cmd_trunk_set_flood_mode_trunk_group_trunk_id_config_hash(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    uint32                  trk_gid = 0;
    rtk_trunk_floodMode_t   mode = FLOOD_MODE_BY_HASH;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    trk_gid = *trunk_id_ptr;
    if ('c' == TOKEN_CHAR(5,0))
    {
        mode = FLOOD_MODE_BY_CONFIG;
    }
    else
    {
        mode = FLOOD_MODE_BY_HASH;
    }

    DIAG_UTIL_ERR_CHK(rtk_trunk_floodMode_set(unit, trk_gid, mode), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trunk_set_flood_mode_trunk_id_by_config_by_hash */
#endif

#ifdef CMD_TRUNK_SET_HASH_MAPPING_TRUNK_GROUP_TRUNK_ID_HASH_VALUE_VALUE_PORT_PORT
/*
 * trunk set hash-mapping trunk-group <UINT:trunk_id> hash-value <MASK_LIST:value> port <UINT:port>
 */
cparser_result_t cparser_cmd_trunk_set_hash_mapping_trunk_group_trunk_id_hash_value_value_port_port(cparser_context_t *context,
    uint32_t *trunk_id_ptr,
    char **value_ptr,
    uint32_t *port_ptr)
{
    uint32                      unit = 0;
    uint32                      trk_gid = 0;
    uint32                      hash_value = 0;
    int32                       ret = RT_ERR_FAILED;
    rtk_port_t                  port = 0;
    rtk_trunk_hashVal2Port_t    hash2Port_array;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    trk_gid = *trunk_id_ptr;
    hash_value = **value_ptr;
    port = *port_ptr;

    memset(&hash2Port_array, 0, sizeof(rtk_trunk_hashVal2Port_t));
    DIAG_UTIL_ERR_CHK(rtk_trunk_hashMappingTable_get(unit, trk_gid, &hash2Port_array), ret);

    /* set trunk hash mapping */
    hash2Port_array.value[hash_value] = port;
    DIAG_UTIL_ERR_CHK(rtk_trunk_hashMappingTable_set(unit, trk_gid, &hash2Port_array), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trunk_set_hash_mapping_trunk_id_hash_value_port */
#endif




#ifdef CMD_TRUNK_SET_MEMBER_PORT_TRUNK_GROUP_TRUNK_ID_PORTS_NONE
/*
 * trunk set member-port trunk-group <UINT:trunk_id> ( <PORT_LIST:ports> | none )
 */
cparser_result_t cparser_cmd_trunk_set_member_port_trunk_group_trunk_id_ports_none(cparser_context_t *context,
    uint32_t *trunk_id_ptr,
    char **ports_ptr)
{
    uint32          unit = 0;
    uint32          trk_gid = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_portmask_t  trunk_member_portmask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    trk_gid = *trunk_id_ptr;
    memset(&trunk_member_portmask, 0, sizeof(rtk_portmask_t));

    if ('n' != TOKEN_CHAR(5,0))
        diag_util_str2LPortMask((uint8 *)*ports_ptr, &trunk_member_portmask);

    DIAG_UTIL_ERR_CHK(rtk_trunk_port_set(unit, trk_gid, &trunk_member_portmask), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trunk_set_member_port_trunk_id_port_all */
#endif

#ifdef CMD_TRUNK_SET_MODE_DUMB_NORMAL
/*
 * trunk set mode ( dumb | normal )
 */
cparser_result_t cparser_cmd_trunk_set_mode_dumb_normal(cparser_context_t *context)
{
    uint32              unit = 0;
    int32               ret = RT_ERR_FAILED;
    rtk_trunk_mode_t    mode = TRUNK_MODE_NORMAL;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('n' == TOKEN_CHAR(3,0))
    {
        /* set normal mode */
        mode = TRUNK_MODE_NORMAL;
    }
    else
    {
        /* set dumb mode */
        mode = TRUNK_MODE_DUMB;
    }

    DIAG_UTIL_ERR_CHK(rtk_trunk_mode_set(unit, mode), ret);
    return CPARSER_OK;
} /* end of cparser_cmd_trunk_set_mode_dumb_normal */
#endif

#ifdef CMD_TRUNK_SET_FLOOD_CONFIG_PORT_REPRESENT_PORT_TRUNK_GROUP_TRUNK_ID_PORT_PORT
/*
 * trunk set ( flood-config-port | represent-port ) trunk-group <UINT:trunk_id> port <UINT:port>
 */
cparser_result_t cparser_cmd_trunk_set_flood_config_port_represent_port_trunk_group_trunk_id_port_port(cparser_context_t *context,
    uint32_t *trunk_id_ptr, uint32_t *port_ptr)
{
    uint32      unit = 0;
    int32       ret = RT_ERR_FAILED;
    uint32      trk_gid = 0;
    rtk_port_t  port;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    trk_gid = *trunk_id_ptr;
    port = *port_ptr;

    if ('r' == TOKEN_CHAR(2, 0))
    {
        DIAG_UTIL_ERR_CHK(rtk_trunk_representPort_set(unit, trk_gid, port), ret);
    }
    else
    {
        DIAG_UTIL_ERR_CHK(rtk_trunk_floodPort_set(unit, trk_gid, port), ret);
    }

    return CPARSER_OK;
} /* end of cparser_cmd_trunk_set_flood_config_port_represent_port_trunk_id_port */
#endif

#ifdef CMD_TRUNK_GET_DISTRIBUTE_ALGORITHM_BIND_TRUNK_GROUP_TRUNK_ID
/*
 * trunk get distribute-algorithm bind trunk-group <UINT:trunk_id>
 */
cparser_result_t cparser_cmd_trunk_get_distribute_algorithm_bind_trunk_group_trunk_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  algo_idx ;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmBind_get(unit, *trunk_id_ptr, &algo_idx), ret);
    diag_util_mprintf("Trunk %d binds to distribution algorithm %d\n", *trunk_id_ptr, algo_idx);


    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_GET_DISTRIBUTE_ALGORITHM_PARAMETER_ALGORITHM_ID_ALGO_ID
/*
 * trunk get distribute-algorithm parameter algorithm-id <UINT:algo_id>
 */
cparser_result_t cparser_cmd_trunk_get_distribute_algorithm_parameter_algorithm_id_algo_id(cparser_context_t *context,
    uint32_t *algo_id_ptr)
{
    uint32  unit = 0;
    uint32  algo_bitmask = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("\nIndex %d ", *algo_id_ptr);
    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmParam_get(unit, *algo_id_ptr, &algo_bitmask), ret);
    diag_util_mprintf("distribution algorithm: ");
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SPA_BIT)
    {
        diag_util_mprintf("src-port|");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SMAC_BIT)
    {
        diag_util_mprintf("src-mac|");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DMAC_BIT)
    {
        diag_util_mprintf("dst-mac|");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SIP_BIT)
    {
        diag_util_mprintf("src-ip|");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DIP_BIT)
    {
        diag_util_mprintf("dst-ip|");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT)
    {
        diag_util_mprintf("src-l4-port|");
    }
    if (algo_bitmask & TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT)
    {
        diag_util_mprintf("dst-l4-port|");
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_GET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID
/*
 * trunk get distribute-algorithm shift algorithm-id <UINT:algo_id>
 */
cparser_result_t cparser_cmd_trunk_get_distribute_algorithm_shift_algorithm_id_algo_id(cparser_context_t *context,
    uint32_t *algo_id_ptr)
{
    uint32  unit = 0;
    rtk_trunk_distAlgoShift_t shift;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);

    diag_util_mprintf("Shift of algorithm %d parameters:\n", *algo_id_ptr);
    diag_util_mprintf("SPA: %d bits\n", shift.spa_shift);
    diag_util_mprintf("SMAC: %d bits\n", shift.smac_shift);
    diag_util_mprintf("DMAC: %d bits\n", shift.dmac_shift);
    diag_util_mprintf("SIP: %d bits\n", shift.sip_shift);
    diag_util_mprintf("DIP: %d bits\n", shift.dip_shift);
    diag_util_mprintf("SPORT: %d bits\n", shift.sport_shift);
    diag_util_mprintf("DPORT: %d bits\n", shift.dport_shift);
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_GET_TRAFFIC_SEPARATION_TRUNK_GROUP_TRUNK_ID_KNOWN_MCAST_STATE
/*
 * trunk get traffic-separation trunk-group <UINT:trunk_id> known-mcast state
 */
cparser_result_t cparser_cmd_trunk_get_traffic_separation_trunk_group_trunk_id_known_mcast_state(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32  unit = 0;
    rtk_trunk_separateType_t separate;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trunk_trafficSeparate_get(unit, *trunk_id_ptr, &separate), ret);

    diag_util_mprintf("Traffic separate of trunk %d:\n", *trunk_id_ptr);
    if(separate == SEPARATE_NONE)
    {
        diag_util_mprintf("Separate Known Multicast: Disable\n");
    }
    else if(separate == SEPARATE_KNOWN_MULTI)
    {
        diag_util_mprintf("Separate Known Multicast: Enable\n");
    }
    else if(separate == SEPARATE_FLOOD)
    {
        diag_util_mprintf("Separate Known Multicast: Disable\n");
    }
    else if(separate == SEPARATE_KNOWN_MULTI_AND_FLOOD)
    {
        diag_util_mprintf("Separate Known Multicast: Enable\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_GET_TRAFFIC_SEPARATION_TRUNK_GROUP_TRUNK_ID_FLOODING_STATE
/*
 * trunk get traffic-separation trunk-group <UINT:trunk_id> flooding state
 */
cparser_result_t cparser_cmd_trunk_get_traffic_separation_trunk_group_trunk_id_flooding_state(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32  unit = 0;
    rtk_trunk_separateType_t separate;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_trunk_trafficSeparate_get(unit, *trunk_id_ptr, &separate), ret);

    diag_util_mprintf("Traffic separate of trunk %d:\n", *trunk_id_ptr);
    if(separate == SEPARATE_NONE)
    {
        diag_util_mprintf("Separate Flooding Traffic: Disable\n");
    }
    else if(separate == SEPARATE_KNOWN_MULTI)
    {
        diag_util_mprintf("Separate Flooding Traffic: Disable\n");
    }
    else if(separate == SEPARATE_FLOOD)
    {
        diag_util_mprintf("Separate Flooding Traffic: Enable\n");
    }
    else if(separate == SEPARATE_KNOWN_MULTI_AND_FLOOD)
    {
        diag_util_mprintf("Separate Flooding Traffic: Enable\n");
    }
    
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_BIND_TRUNK_GROUP_TRUNK_ID_ALGO_ID_ALGO_ID
/*
 * trunk set distribute-algorithm bind trunk-group <UINT:trunk_id> algo-id <UINT:algo_id>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_bind_trunk_group_trunk_id_algo_id_algo_id(cparser_context_t *context,
    uint32_t *trunk_id_ptr,
    uint32_t *algo_id_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmBind_set(unit, *trunk_id_ptr, *algo_id_ptr), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_PARAMETER_ALGORITHM_ID_ALGO_ID_DST_IP_DST_L4_PORT_DST_MAC_SRC_IP_SRC_L4_PORT_SRC_MAC_SRC_PORT
/*
 * trunk set distribute-algorithm parameter algorithm-id <UINT:algo_id> { dst-ip } { dst-l4-port } { dst-mac } { src-ip } { src-l4-port } { src-mac } { src-port }
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_parameter_algorithm_id_algo_id_dst_ip_dst_l4_port_dst_mac_src_ip_src_l4_port_src_mac_src_port(cparser_context_t *context,
    uint32_t *algo_id_ptr)
{
    uint32  unit = 0;
    uint32  algo_bitmask = 0;
    int32   option_num = 0;
    int32   ret = RT_ERR_FAILED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (TOKEN_NUM < 7)
    {
        algo_bitmask = 0;
    }
    else
    {
        for (option_num = 6; option_num < TOKEN_NUM; option_num++)
        {
            if ('s' == TOKEN_CHAR(option_num, 0))
            {
                if ('p' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SPA_BIT;
                }
                else if ('m' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SMAC_BIT;
                }
                else if ('i' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SIP_BIT;
                }
                else if ('l' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_SRC_L4PORT_BIT;
                }
                else
                {
                    diag_util_mprintf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }
            }
            else if ('d' == TOKEN_CHAR(option_num, 0))
            {
                if ('m' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DMAC_BIT;
                }
                else if ('i' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DIP_BIT;
                }
                else if ('l' == TOKEN_CHAR(option_num, 4))
                {
                    algo_bitmask |= TRUNK_DISTRIBUTION_ALGO_DST_L4PORT_BIT;
                }
                else
                {
                    diag_util_mprintf("User config: Error!\n");
                    return CPARSER_NOT_OK;
                }
            }
            else
            {
                diag_util_mprintf("User config: Error!\n");
                return CPARSER_NOT_OK;
            }
        } /* end of for (option_num = 6; option_num < TOKEN_NUM; option_num++) */
    }

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmParam_set(unit, *algo_id_ptr, algo_bitmask), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID_SRC_PORT_SHIFT
/*
 * trunk set distribute-algorithm shift algorithm-id <UINT:algo_id> src-port <UINT:shift>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_shift_algorithm_id_algo_id_src_port_shift(cparser_context_t *context,
    uint32_t *algo_id_ptr,
    uint32_t *shift_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trunk_distAlgoShift_t shift;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);

    shift.spa_shift = *shift_ptr;

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_set(unit, *algo_id_ptr, &shift), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID_SRC_MAC_SHIFT
/*
 * trunk set distribute-algorithm shift algorithm-id <UINT:algo_id> src-mac <UINT:shift>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_shift_algorithm_id_algo_id_src_mac_shift(cparser_context_t *context,
    uint32_t *algo_id_ptr,
    uint32_t *shift_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trunk_distAlgoShift_t shift;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);

    shift.smac_shift = *shift_ptr;

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_set(unit, *algo_id_ptr, &shift), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID_DST_MAC_SHIFT
/*
 * trunk set distribute-algorithm shift algorithm-id <UINT:algo_id> dst-mac <UINT:shift>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_shift_algorithm_id_algo_id_dst_mac_shift(cparser_context_t *context,
    uint32_t *algo_id_ptr,
    uint32_t *shift_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trunk_distAlgoShift_t shift;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);

    shift.dmac_shift = *shift_ptr;
        
    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_set(unit, *algo_id_ptr, &shift), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID_SRC_IP_SHIFT
/*
 * trunk set distribute-algorithm shift algorithm-id <UINT:algo_id> src-ip <UINT:shift>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_shift_algorithm_id_algo_id_src_ip_shift(cparser_context_t *context,
    uint32_t *algo_id_ptr,
    uint32_t *shift_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trunk_distAlgoShift_t shift;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);

    shift.sip_shift = *shift_ptr;
        
    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_set(unit, *algo_id_ptr, &shift), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID_SRC_PORT_SHIFT
/*
 * trunk set distribute-algorithm shift algorithm-id <UINT:algo_id> dst-ip <UINT:shift>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_shift_algorithm_id_algo_id_dst_ip_shift(cparser_context_t *context,
    uint32_t *algo_id_ptr,
    uint32_t *shift_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trunk_distAlgoShift_t shift;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);

    shift.dip_shift = *shift_ptr;
        
    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_set(unit, *algo_id_ptr, &shift), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID_SRC_L4_PORT_SHIFT
/*
 * trunk set distribute-algorithm shift algorithm-id <UINT:algo_id> src-l4-port <UINT:shift>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_shift_algorithm_id_algo_id_src_l4_port_shift(cparser_context_t *context,
    uint32_t *algo_id_ptr,
    uint32_t *shift_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trunk_distAlgoShift_t shift;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);

    shift.sport_shift = *shift_ptr;    
        
    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_set(unit, *algo_id_ptr, &shift), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_DISTRIBUTE_ALGORITHM_SHIFT_ALGORITHM_ID_ALGO_ID_DST_L4_PORT_SHIFT
/*
 * trunk set distribute-algorithm shift algorithm-id <UINT:algo_id> dst-l4-port <UINT:shift>
 */
cparser_result_t cparser_cmd_trunk_set_distribute_algorithm_shift_algorithm_id_algo_id_dst_l4_port_shift(cparser_context_t *context,
    uint32_t *algo_id_ptr,
    uint32_t *shift_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_trunk_distAlgoShift_t shift;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_get(unit, *algo_id_ptr, &shift), ret);
   
    shift.dport_shift = *shift_ptr;
        
    DIAG_UTIL_ERR_CHK(rtk_trunk_distributionAlgorithmShift_set(unit, *algo_id_ptr, &shift), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_TRAFFIC_SEPARATION_TRUNK_GROUP_TRUNK_ID_KNOWN_MCAST_STATE_DISABLE_ENABLE
/*
 * trunk set traffic-separation trunk-group <UINT:trunk_id> known-mcast state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_trunk_set_traffic_separation_trunk_group_trunk_id_known_mcast_state_disable_enable(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trunk_separateType_t    separate = SEPARATE_NONE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_trafficSeparate_get(unit, *trunk_id_ptr, &separate), ret);

    if (0 == osal_strcmp(TOKEN_STR(7), "disable"))
    {
        switch(separate)
        {
            case SEPARATE_NONE:
                break;
            case SEPARATE_KNOWN_MULTI:
                separate = SEPARATE_NONE;
                break;
            case SEPARATE_FLOOD:
                break;
            case SEPARATE_KNOWN_MULTI_AND_FLOOD:
                separate = SEPARATE_FLOOD;
                break;
            default:
                break;
        }
    }
    else
    {
        switch(separate)
        {
            case SEPARATE_NONE:
                separate = SEPARATE_KNOWN_MULTI;
                break;
            case SEPARATE_KNOWN_MULTI:
                break;
            case SEPARATE_FLOOD:
                separate = SEPARATE_KNOWN_MULTI_AND_FLOOD;
                break;
            case SEPARATE_KNOWN_MULTI_AND_FLOOD:
                break;
            default:
                break;
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_trunk_trafficSeparate_set(unit, *trunk_id_ptr, separate), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_TRUNK_SET_TRAFFIC_SEPARATION_TRUNK_GROUP_TRUNK_ID_FLOODING_STATE_DISABLE_ENABLE
/*
 * trunk set traffic-separation trunk-group <UINT:trunk_id> flooding state ( disable | enable )
 */ 
cparser_result_t cparser_cmd_trunk_set_traffic_separation_trunk_group_trunk_id_flooding_state_disable_enable(cparser_context_t *context,
    uint32_t *trunk_id_ptr)
{
    uint32                  unit = 0;
    int32                   ret = RT_ERR_FAILED;
    rtk_trunk_separateType_t    separate = SEPARATE_NONE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_trunk_trafficSeparate_get(unit, *trunk_id_ptr, &separate), ret);

    if (0 == osal_strcmp(TOKEN_STR(7), "disable"))
    {
        switch(separate)
        {
            case SEPARATE_NONE:
                break;
            case SEPARATE_KNOWN_MULTI:
                break;
            case SEPARATE_FLOOD:
                separate = SEPARATE_NONE;
                break;
            case SEPARATE_KNOWN_MULTI_AND_FLOOD:
                separate = SEPARATE_KNOWN_MULTI;
                break;
            default:
                break;
        }
    }
    else
    {
        switch(separate)
        {
            case SEPARATE_NONE:
                separate = SEPARATE_FLOOD;
                break;
            case SEPARATE_KNOWN_MULTI:
                separate = SEPARATE_KNOWN_MULTI_AND_FLOOD;
                break;
            case SEPARATE_FLOOD:
                break;
            case SEPARATE_KNOWN_MULTI_AND_FLOOD:
                break;
            default:
                break;
        }
    }

    DIAG_UTIL_ERR_CHK(rtk_trunk_trafficSeparate_set(unit, *trunk_id_ptr, separate), ret);
    return CPARSER_OK;
}
#endif

