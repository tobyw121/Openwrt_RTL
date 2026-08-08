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
 * Purpose : Definition those public statistic APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) statistic counter reset
 *           2) statistic counter get
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
#include <ioal/mem32.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_table_struct.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <dal/ssw/dal_ssw_stat.h>
#include <rtk/default.h>
#include <rtk/stat.h>

/* 
 * Symbol Definition 
 */

/* 
 * Data Declaration 
 */
static uint32               stat_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         stat_sem[RTK_MAX_NUM_OF_UNIT];
static uint32               port_reg_gap;


/*
 * Macro Definition
 */
/* trap semaphore handling */
#define STAT_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(stat_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_STAT),"semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define STAT_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(stat_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_STAT), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)

/*
 * Function Declaration
 */

/* Module Name : STAT */

/* Function Name:
 *      dal_ssw_stat_init
 * Description:
 *      Initialize stat module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 * Note:
 *      Must initialize stat module before calling any stat APIs.
 */
int32
dal_ssw_stat_init(uint32 unit)
{
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    stat_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    stat_sem[unit] = osal_sem_mutex_create();
    if (0 == stat_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, MOD_STP, "semaphore create failed");
        return RT_ERR_FAILED;
    }
        
    port_reg_gap = SSW_PORT1_ETHERNET_LIKE_MIB_COUNTER0r - SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER0r;
    
    /* set init flag to complete init */
    stat_init[unit] = INIT_COMPLETED;

    /* Enable IOL maximum retry to active dot3StatsExcessiveCollisions counter */
    val = 1;
    if ((ret = reg_field_write(unit, SSW_GLOBAL_MAC_CONTROL1r, SSW_EN_IOL_MAX_RETRYf, &val) != RT_ERR_OK))
    {
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }

    return RT_ERR_OK;
} /* end of dal_ssw_stat_init */


/* Function Name:
 *      dal_ssw_stat_global_reset
 * Description:
 *      Reset the global counters in the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID               - invalid unit id
 * Note:
 *      None
 */
int32
dal_ssw_stat_global_reset(uint32 unit)
{
    int32   ret;
    uint32  loop;
    uint32  value;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);
    
    value = 1;
    
    STAT_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_MIB_SYSTEM_CONTROLr, SSW_MIB_GLOBAL_RSTf, &value)) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }
    
    for(loop = 0; loop < 0xFFFFFFFF; loop++)
    {
        if (((ret = reg_field_read(unit, SSW_MIB_SYSTEM_CONTROLr, SSW_MIB_GLOBAL_RSTf, &value)) == RT_ERR_OK)
            && (0 == value))
        {
            break;
        }
    }
    STAT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_stat_global_reset */


/* Function Name:
 *      dal_ssw_stat_port_reset
 * Description:
 *      Reset the specified port counters in the specified device.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 * Note:
 *      None
 */
int32
dal_ssw_stat_port_reset(uint32 unit, rtk_port_t port)
{
    int32   ret;
    uint32  loop;
    uint32  value;
    rtk_portmask_t  resetPortmask;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    osal_memset(&resetPortmask, 0, sizeof(resetPortmask));
    RTK_PORTMASK_PORT_SET(resetPortmask, port);
    value = 1;
    
    STAT_SEM_LOCK(unit);
    /* get entry from CHIP*/
    if ((ret = reg_field_write(unit, SSW_MIB_SYSTEM_CONTROLr, SSW_MIB_PER_PORT_RSTf, &resetPortmask.bits[0])) != RT_ERR_OK)
    {
        STAT_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
        return ret;
    }
    
    for(loop = 0; loop < 0xFFFFFFFF; loop++)
    {
        if (((ret = reg_field_read(unit, SSW_MIB_SYSTEM_CONTROLr, SSW_MIB_RST_FLAGf, &value)) == RT_ERR_OK)
            && (0 == value))
        {
            break;
        }
    }
    STAT_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_ssw_stat_port_reset*/


/* Function Name:
 *      dal_ssw_stat_global_get
 * Description:
 *      Get one specified global counter in the specified device.
 * Input:
 *      unit     - unit id
 *      cntr_idx - specified global counter index
 * Output:
 *      pCntr   - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 *      RT_ERR_STAT_INVALID_GLOBAL_CNTR - Invalid Global Counter
 * Note:
 *      None
 */
int32
dal_ssw_stat_global_get(uint32 unit, rtk_stat_global_type_t cntr_idx, uint64 *pCntr)
{
    int32   ret;
    uint32  wordCount;
    uint32  value;
    uint32  reg_idx;
    uint32  field_idx;
    uint32  reg_addr;
    uint32  word_count = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d, cntr_idx=%d", unit, cntr_idx);
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(cntr_idx >= MIB_GLOBAL_CNTR_END, RT_ERR_STAT_INVALID_GLOBAL_CNTR);
    
    switch (cntr_idx)
    {
        case DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX:
            reg_idx = SSW_BRIDGE_AND_BRIDGE_EXTENSION_MIB_COUNTER0r;
            field_idx = SSW_DOT1DTPLEARNEDENTRYDISCARDSf;
            word_count = 1;
            break;
        case DOT1D_TP_PORT_IN_DISCARDS_INDEX:
            reg_idx = SSW_BRIDGE_AND_BRIDGE_EXTENSION_MIB_COUNTER1r;
            field_idx = SSW_DOT1DTPPORTINDISCARDSf;
            word_count = 1;
            break;
        case OUT_UCAST_PKTS_INDEX:
            reg_idx = SSW_TX_UNICAST_PACKETSr;
            field_idx = SSW_OUTUNICASTPKTSCNTf;
            word_count = 1;
            break;
        case OUT_MCAST_PKTS_INDEX:
            reg_idx = SSW_TX_MULTICAST_PACKETSr;
            field_idx = SSW_OUTMULTICASTPKTSCNTf;
            word_count = 1;
            break;
        case OUT_BCAST_PKTS_INDEX:
            reg_idx = SSW_TX_BROADCAST_PACKETSr;
            field_idx = SSW_OUTBROADCASTPKTSCNTf;
            word_count = 1;
            break;
        case EGR_LACK_RESOURCE_DROP_INDEX:
            reg_idx = SSW_DROP_COUNTER0r;
            field_idx = SSW_EGRESSLACKRESOURCEDROPf;
            word_count = 1;
            break;
        default:
            /* Return 0 if chip not supoprted the counter */
            *pCntr = 0;
            return RT_ERR_OK;
    }

    *pCntr = 0;
    
    STAT_SEM_LOCK(unit);
    
    /* E0005347 */
    /* For 64-bit counter fetch, need to access address 0xXXXXXXX0 or 0xXXXXXXX8 for triggering ASIC to latch 
       latest correct value before getting 32-bit counter */
       
    if (word_count == 1)
    {
        if ((ret = reg_idx2Addr_get(unit, reg_idx, &reg_addr)) != RT_ERR_OK)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
        
        if (reg_addr & 0x00000007)
        { /* reg_addr is not in 8-bytes boundary */
            /* read data from nearest 8-bytes boundary addres */
            if ((ret = ioal_mem32_read(unit, (reg_addr&0xFFFFFFF8), &value)) != RT_ERR_OK)
            {
                STAT_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
                return ret;
            }
        }       
    }
    /* End of E0005347 */
    
    for (wordCount = 0; wordCount < word_count; wordCount++, reg_idx++, field_idx++)
    {
        if ((ret = reg_field_read(unit, reg_idx, field_idx, &value) != RT_ERR_OK))
        {
            STAT_SEM_UNLOCK(unit);            
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
        
        *pCntr = (*pCntr << 32) + ((uint64)value);
    }
    
    STAT_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "pCntr=%llu", *pCntr);
    
    return RT_ERR_OK;
} /*dal_ssw_stat_global_get  */


/* Function Name:
 *      dal_ssw_stat_global_getAll
 * Description:
 *      Get all global counters in the specified device.
 * Input:
 *      unit           - unit id
 * Output:
 *      pGlobal_cntrs - pointer buffer of global counter structure
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID                  - invalid unit id
 *      RT_ERR_NULL_POINTER             - input parameter may be null pointer
 *      RT_ERR_STAT_GLOBAL_CNTR_FAIL    - Could not retrieve/reset Global Counter
 * Note:
 *      None
 */
int32
dal_ssw_stat_global_getAll(uint32 unit, rtk_stat_global_cntr_t *pGlobal_cntrs)
{
    int32   ret;
    uint64  counter;
 
    rtk_stat_global_type_t counter_idx;
      
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pGlobal_cntrs), RT_ERR_NULL_POINTER);
    
    for (counter_idx = DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX; counter_idx < MIB_GLOBAL_CNTR_END; counter_idx++)
    {
        
        if ((ret = dal_ssw_stat_global_get(unit, counter_idx, &counter)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
        
        switch (counter_idx)
        {
            case DOT1D_TP_LEARNED_ENTRY_DISCARDS_INDEX:
                pGlobal_cntrs->dot1dTpLearnedEntryDiscards = counter;
                break;
            case DOT1D_TP_PORT_IN_DISCARDS_INDEX:
                pGlobal_cntrs->dot1dTpPortInDiscards = counter;
                break;
            case OUT_UCAST_PKTS_INDEX:
                pGlobal_cntrs->OutUnicastPktsCnt = counter;
                break;
            case OUT_MCAST_PKTS_INDEX:
                pGlobal_cntrs->OutMulticastPktsCnt = counter;
                break;
            case OUT_BCAST_PKTS_INDEX:
                pGlobal_cntrs->OutBrocastPktsCnt = counter;
                break;
            case EGR_LACK_RESOURCE_DROP_INDEX:
                pGlobal_cntrs->egrLackResourceDrop = counter;
                break;
            default:
                break;
        }
    }

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "dot1dTpLearnedEntryDiscards=%ul\
           dot1dTpPortInDiscards=%ul, OutUnicastPktsCnt=%ul, OutMulticastPktsCnt=%ul, OutBrocastPktsCnt=%ul\
           egrLackResourceDrop=%ul", pGlobal_cntrs->dot1dTpLearnedEntryDiscards, pGlobal_cntrs->dot1dTpPortInDiscards,
           pGlobal_cntrs->OutUnicastPktsCnt, pGlobal_cntrs->OutMulticastPktsCnt, pGlobal_cntrs->OutBrocastPktsCnt, 
           pGlobal_cntrs->egrLackResourceDrop);
    
    return RT_ERR_OK;
} /* end of dal_ssw_stat_global_getAll */


/* Function Name:
 *      dal_ssw_stat_port_get
 * Description:
 *      Get one specified port counter in the specified device.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      cntr_idx - specified port counter index
 * Output:
 *      pCntr   - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_OUT_OF_RANGE        - input parameter out of range
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32
dal_ssw_stat_port_get(uint32 unit, rtk_port_t port, rtk_stat_port_type_t cntr_idx, uint64 *pCntr)
{
    int32   ret;
    uint32  wordCount;
    uint32  value;
    uint32  reg_addr;
    uint32  word_count = 0;
    rtk_ssw_reg_list_t  reg_idx;
    rtk_ssw_regField_list_t  field_idx;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d, port=%d, cntr_idx=%d", unit, port, cntr_idx);
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pCntr), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK(cntr_idx >= MIB_PORT_CNTR_END, RT_ERR_OUT_OF_RANGE);
    
    switch(cntr_idx)
    {
        case IF_OUT_OCTETS_INDEX:
            reg_idx = SSW_PORT0_MIB_II_AND_INTERFACE_GROUP_MIB_COUNTER3r + port * port_reg_gap;
            field_idx = SSW_IFOUTOCTETS_63_32f;
            word_count = 2;
            break;
        case IF_IN_OCTETS_INDEX:
            reg_idx = SSW_PORT0_MIB_II_AND_INTERFACE_GROUP_MIB_COUNTER5r + port * port_reg_gap;
            field_idx = SSW_IFINOCTETS_63_32f;
            word_count = 2;
            break;
        case IF_IN_UCAST_PKTS_INDEX:
            reg_idx = SSW_PORT0_MIB_II_AND_INTERFACE_GROUP_MIB_COUNTER7r + port * port_reg_gap;
            field_idx = SSW_IFINPKTSf;
            word_count = 1;
            break;
        case DOT3_OUT_PAUSE_FRAMES_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER0r + port * port_reg_gap;
            field_idx = SSW_DOT3OUTPAUSEFRAMESf;
            word_count = 1;
            break;
        case DOT3_OUT_PAUSE_ON_FRAMES_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER1r + port * port_reg_gap;
            field_idx = SSW_DOT3OUTPAUSEONFRAMESf;
            word_count = 1;
            break;
        case DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER2r + port * port_reg_gap;
            field_idx = SSW_DOT3STATSEXCESSIVECOLLISIONSf;
            word_count = 1;
            break;
        case DOT3_STATS_LATE_COLLISIONS_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER3r + port * port_reg_gap;
            field_idx = SSW_DOT3STATSLATECOLLISIONSf;
            word_count = 1;
            break;
        case DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER4r + port * port_reg_gap;
            field_idx = SSW_DOT3STATSDEFERREDTRANSMISSIONSf;
            word_count = 1;
            break;
        case DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER5r + port * port_reg_gap;
            field_idx = SSW_DOT3STATSMULTIPLECOLLISIONFRAMESf;
            word_count = 1;
            break;
        case DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER6r + port * port_reg_gap;
            field_idx = SSW_DOT3STATSSINGLECOLLISIONFRAMESf;
            word_count = 1;
            break;
        case DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER7r + port * port_reg_gap;
            field_idx = SSW_DOT3CONTROLINUNKNOWNOPCODESf;
            word_count = 1;
            break;
        case DOT3_IN_PAUSE_FRAMES_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER8r + port * port_reg_gap;
            field_idx = SSW_DOT3INPAUSEFRAMESf;
            word_count = 1;
            break;
        case DOT3_STATS_SYMBOL_ERRORS_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER9r + port * port_reg_gap;
            field_idx = SSW_DOT3STATSSYMBOLERRORSf;
            word_count = 1;
            break;
        case DOT3_STATS_FCS_ERRORS_INDEX:
            reg_idx = SSW_PORT0_ETHERNET_LIKE_MIB_COUNTER10r + port * port_reg_gap;
            field_idx = SSW_DOT3STATSFCSERRORSf;
            word_count = 1;
            break;
        case ETHER_STATS_JABBERS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER0r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSJABBERSf;
            word_count = 1;
            break;
        case ETHER_STATS_COLLISIONS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER1r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSCOLLISIONSf;
            word_count = 1;
            break;
        case ETHER_STATS_MULTICAST_PKTS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER2r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSMULTICASTPKTSf;
            word_count = 1;
            break;
        case ETHER_STATS_BROADCAST_PKTS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER3r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSBROADCASTPKTSf;
            word_count = 1;
            break;
        case ETHER_STATS_FRAGMENTS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER4r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSFRAGMENTSf;
            word_count = 1;
            break;
        case ETHER_STATS_PKTS_64OCTETS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER5r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSPKTS64OCTETSf;
            word_count = 1;
            break;
        case ETHER_STATS_PKTS_65TO127OCTETS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER6r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSPKTS65TO127OCTETSf;
            word_count = 1;
            break;
        case ETHER_STATS_PKTS_128TO255OCTETS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER7r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSPKTS128TO255OCTETSf;
            word_count = 1;
            break;
        case ETHER_STATS_PKTS_256TO511OCTETS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER8r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSPKTS256TO511OCTETSf;
            word_count = 1;
            break;
        case ETHER_STATS_PKTS_512TO1023OCTETS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER9r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSPKTS512TO1023OCTETSf;
            word_count = 1;
            break;
        case ETHER_STATS_PKTS_1024TO1518OCTETS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER10r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSPKTS1024TOMAXOCTETSf;
            word_count = 1;
            break;
        case ETHER_STATS_OVERSIZE_PKTS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER11r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSOVERSIZEPKTSf;
            word_count = 1;
            break;
        case ETHER_STATS_OCTETS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER12r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSOCTETS_63_32f;
            word_count = 2;
            break;
        case ETHER_STATS_UNDER_SIZE_PKTS_INDEX:
            reg_idx = SSW_PORT0_RMON_MIB_COUNTER14r + port * port_reg_gap;
            field_idx = SSW_ETHERSTATSUNDERSIZEPKTSf;
            word_count = 1;
            break;
        case IGR_LACK_PKT_BUF_DROP_INDEX:
            reg_idx = SSW_PORT0_DROP_COUNTER14r + port * port_reg_gap;
            field_idx = SSW_INGRESSLACKPKTBUFDROPf;
            word_count = 1;
            break;
        case FLOWCTRL_ON_DROP_PKT_CNT_INDEX:
            reg_idx = SSW_PORT0_DROP_COUNTER15r + port * port_reg_gap;
            field_idx = SSW_FLOWCTRLONDROPPKTCNTf;
            word_count = 1;
            break;
        case TX_CRC_CHECK_FAIL_CNT_INDEX:
            reg_idx = SSW_PORT0_TX_CRC_CHECK_FAILr + port * port_reg_gap;
            field_idx = SSW_TXCRCCHECKFAILCNTf;
            word_count = 1;
            break;
        case SMART_TRIGGER_HIT0_INDEX:
            reg_idx = SSW_PORT0_SMART_TRIGGERING_HIT_0r + port * port_reg_gap;
            field_idx = SSW_SMARTTRIGGERHIT0f;
            word_count = 1;
            break;
        case SMART_TRIGGER_HIT1_INDEX:
            reg_idx = SSW_PORT0_SMART_TRIGGERING_HIT_1r + port * port_reg_gap;
            field_idx = SSW_SMARTTRIGGERHIT1f;
            word_count = 1;
            break;
        case IF_OUT_UCAST_PKTS_CNT_INDEX:
            reg_idx = SSW_PORT0_MIB_II_AND_INTERFACE_GROUP_MIB_COUNTER0r + port * port_reg_gap;
            field_idx = SSW_IFOUTUCASTPKTSf;
            word_count = 1;
            break;
        case IF_OUT_MULTICAST_PKTS_CNT_INDEX:
            reg_idx = SSW_PORT0_INTERFACE_GROUP_MIB_COUNTER1r + port * port_reg_gap;
            field_idx = SSW_IFOUTMULTICASTPKTSf;
            word_count = 1;
            break;
        case IF_OUT_BROADCAST_PKTS_CNT_INDEX:
            reg_idx = SSW_PORT0_INTERFACE_GROUP_MIB_COUNTER2r + port * port_reg_gap;
            field_idx = SSW_IFOUTBROADCASTPKTSf;
            word_count = 1;
            break;
        default:
            /* Return 0 if chip not supoprted the counter */
            *pCntr = 0;
            return RT_ERR_OK;
    }

    *pCntr = 0;
    
    STAT_SEM_LOCK(unit);    
    
    /* E0005347 */
    /* For 64-bit counter fetch, need to access address 0xXXXXXXX0 or 0xXXXXXXX8 for triggering ASIC to latch 
       latest correct value before getting 32-bit counter */
       
    if (word_count == 1)
    {
        if ((ret = reg_idx2Addr_get(unit, reg_idx, &reg_addr)) != RT_ERR_OK)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }            
        if ((ret = ioal_mem32_read(unit, (reg_addr - 4), &value)) != RT_ERR_OK)
        {
            STAT_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }       
    }
    /* End of E0005347 */
      
    for (wordCount = 0; wordCount < word_count; wordCount++, reg_idx++, field_idx++)
    {
        if ((ret = reg_field_read(unit, reg_idx, field_idx, &value) != RT_ERR_OK))
        {
            STAT_SEM_UNLOCK(unit);            
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return RT_ERR_STAT_PORT_CNTR_FAIL;
        }
        
        *pCntr = (*pCntr << 32) + ((uint64)value);
    }
    
    STAT_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "pCntr=%llu", *pCntr);
    
    return RT_ERR_OK;
} /* end of dal_ssw_stat_port_get */


/* Function Name:
 *      dal_ssw_stat_port_getAll
 * Description:
 *      Get all counters of one specified port in the specified device.
 * Input:
 *      unit         - unit id
 *      port         - port id
 * Output:
 *      pPort_cntrs - pointer buffer of counter value
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 *      RT_ERR_STAT_PORT_CNTR_FAIL - Could not retrieve/reset Port Counter
 * Note:
 *      None
 */
int32
dal_ssw_stat_port_getAll(uint32 unit, rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs)
{
    int32   ret;
    uint64  counter;
    rtk_stat_port_type_t    counter_idx;

    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_STAT), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(stat_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPort_cntrs), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_PORT_EXIST(unit, port), RT_ERR_PORT_ID);
    
    for (counter_idx = IF_IN_OCTETS_INDEX; counter_idx < MIB_PORT_CNTR_END; counter_idx++)
    {        
        if ((ret = dal_ssw_stat_port_get(unit, port, counter_idx, &counter)) != RT_ERR_OK)
        {
            RT_ERR(ret, (MOD_DAL|MOD_STAT), "");
            return ret;
        }
        
        switch (counter_idx)
        {
            case IF_OUT_OCTETS_INDEX:
                pPort_cntrs->ifOutOctets = counter;
                break;
            case IF_IN_OCTETS_INDEX:
                pPort_cntrs->ifInOctets = counter;
                break;
            case IF_IN_UCAST_PKTS_INDEX:
                pPort_cntrs->ifInUcastPkts = counter;
                break;
            case DOT3_OUT_PAUSE_FRAMES_INDEX:
                pPort_cntrs->dot3OutPauseFrames = counter;
                break;
            case DOT3_OUT_PAUSE_ON_FRAMES_INDEX:
                pPort_cntrs->dot3OutPauseOnFrames = counter;
                break;
            case DOT3_STATS_EXCESSIVE_COLLISIONS_INDEX:
                pPort_cntrs->dot3StatsExcessiveCollisions = counter;
                break;
            case DOT3_STATS_LATE_COLLISIONS_INDEX:
                pPort_cntrs->dot3StatsLateCollisions = counter;
                break;
            case DOT3_STATS_DEFERRED_TRANSMISSIONS_INDEX:
                pPort_cntrs->dot3StatsDeferredTransmissions = counter;
                break;
            case DOT3_STATS_MULTIPLE_COLLISION_FRAMES_INDEX:
                pPort_cntrs->dot3StatsMultipleCollisionFrames = counter;
                break;
            case DOT3_STATS_SINGLE_COLLISION_FRAMES_INDEX:
                pPort_cntrs->dot3StatsSingleCollisionFrames = counter;
                break;
            case DOT3_CONTROL_IN_UNKNOWN_OPCODES_INDEX:
                pPort_cntrs->dot3ControlInUnknownOpcodes = counter;
                break;
            case DOT3_IN_PAUSE_FRAMES_INDEX:
                pPort_cntrs->dot3InPauseFrames = counter;
                break;
            case DOT3_STATS_SYMBOL_ERRORS_INDEX:
                pPort_cntrs->dot3StatsSymbolErrors = counter;
                break;
            case DOT3_STATS_FCS_ERRORS_INDEX:
                pPort_cntrs->dot3StatsFCSErrors = counter;
                break;
            case ETHER_STATS_JABBERS_INDEX:
                pPort_cntrs->etherStatsJabbers = counter;
                break;
            case ETHER_STATS_COLLISIONS_INDEX:
                pPort_cntrs->etherStatsCollisions = counter;
                break;
            case ETHER_STATS_MULTICAST_PKTS_INDEX:
                pPort_cntrs->etherStatsMulticastPkts = counter;
                break;
            case ETHER_STATS_BROADCAST_PKTS_INDEX:
                pPort_cntrs->etherStatsBroadcastPkts = counter;
                break;
            case ETHER_STATS_FRAGMENTS_INDEX:
                pPort_cntrs->etherStatsFragments = counter;
                break;
            case ETHER_STATS_PKTS_64OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts64Octets = counter;
                break;
            case ETHER_STATS_PKTS_65TO127OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts65to127Octets = counter;
                break;
            case ETHER_STATS_PKTS_128TO255OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts128to255Octets = counter;
                break;
            case ETHER_STATS_PKTS_256TO511OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts256to511Octets = counter;
                break;
            case ETHER_STATS_PKTS_512TO1023OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts512to1023Octets = counter;
                break;
            case ETHER_STATS_PKTS_1024TO1518OCTETS_INDEX:
                pPort_cntrs->etherStatsPkts1024toMaxOctets = counter;
                break;
            case ETHER_STATS_OVERSIZE_PKTS_INDEX:
                pPort_cntrs->etherStatsOversizePkts = counter;
                break;
            case ETHER_STATS_OCTETS_INDEX:
                pPort_cntrs->etherStatsOctets = counter;
                break;
            case ETHER_STATS_UNDER_SIZE_PKTS_INDEX:
                pPort_cntrs->etherStatsUndersizePkts = counter;
                break;
            case IGR_LACK_PKT_BUF_DROP_INDEX:
                pPort_cntrs->igrLackPktBufDrop = counter;
                break;
            case FLOWCTRL_ON_DROP_PKT_CNT_INDEX:
                pPort_cntrs->flowCtrlOnDropPktCnt = counter;
                break;
            case TX_CRC_CHECK_FAIL_CNT_INDEX:
                pPort_cntrs->txCrcCheckFailCnt = counter;
                break;
            case SMART_TRIGGER_HIT0_INDEX:
                pPort_cntrs->smartTriggerHit0 = counter;
                break;
            case SMART_TRIGGER_HIT1_INDEX:
                pPort_cntrs->smartTriggerHit1 = counter;
                break;
            case IF_OUT_UCAST_PKTS_CNT_INDEX:
                pPort_cntrs->ifOutUcastPkts = counter;
                break;
            case IF_OUT_MULTICAST_PKTS_CNT_INDEX:
                pPort_cntrs->ifOutMulticastPkts = counter;
                break;
            case IF_OUT_BROADCAST_PKTS_CNT_INDEX:
                pPort_cntrs->ifOutBrocastPkts = counter;
                break;
            
            default:
                break;
        }
    }
    
    return RT_ERR_OK;
} /* end of dal_ssw_stat_port_getAll */

