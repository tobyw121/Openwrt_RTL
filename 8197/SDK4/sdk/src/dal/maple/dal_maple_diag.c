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
 * $Revision: 49127 $
 * $Date: 2014-07-08 16:43:43 +0800 (Tue, 08 Jul 2014) $
 *
 * Purpose : Definition those public Port APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) Port
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
#include <osal/thread.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/time.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/maple/rtk_maple_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/maple/rtk_maple_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/dal_common.h>
#include <dal/maple/dal_maple_diag.h>
#include <dal/maple/dal_maple_vlan.h>
#include <rtk/port.h>
#include <rtk/default.h>
#include <drv/intr/intr.h>
#include <dal/maple/dal_maple_l2.h>

/*
 * Symbol Definition
 */


/*
 * Data Declaration
 */
static uint32               diag_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         diag_sem[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define DIAG_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(diag_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_DIAG), "semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define DIAG_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(diag_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_DIAG), "semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/*
 * Function Declaration
 */

/* Module Name    : diagnostic     */
/* Sub-module Name: Global */

/* Function Name:
 *      dal_maple_diag_init
 * Description:
 *      Initialize diagnostic module of the specified device.
 * Input:
 *      unit          - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. Module must be initialized before using all of APIs in this module
 */
int32
dal_maple_diag_init(uint32 unit)
{
    diag_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    diag_sem[unit] = osal_sem_mutex_create();
    if (0 == diag_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "semaphore create failed");
        return RT_ERR_FAILED;
    }


    /* set init flag to complete init */
    diag_init[unit] = INIT_COMPLETED;

    return RT_ERR_OK;
}/* end of dal_cypress_port_init */


/* Module Name    : Diag */
/* Sub-module Name: RTCT */

/* Function Name:
 *      dal_maple_diag_portRtctResult_get
 * Description:
 *      Get test result of RTCT.
 * Input:
 *      unit        - unit id
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      If linkType is PORT_SPEED_1000M, test result will be stored in ge_result.
 *      If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 */
int32
dal_maple_diag_portRtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32 ret;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, port=%d",
           unit, port);

    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);

    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pRtctResult), RT_ERR_NULL_POINTER);

    osal_memset(pRtctResult, 0, sizeof(rtk_rtctResult_t));

    DIAG_SEM_LOCK(unit);

    /* Get RTCT Result */
    if ((ret = phy_rtctResult_get(unit, port, pRtctResult)) != RT_ERR_OK)
    {
        DIAG_SEM_UNLOCK(unit);
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
        return ret;
    }
    DIAG_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /*end of dal_maple_diag_portRtctResult_get*/

/* Function Name:
 *      dal_maple_diag_rtct_start
 * Description:
 *      Start RTCT for ports.
 *      When enable RTCT, the port won't transmit and receive normal traffic.
 * Input:
 *      unit      - unit id
 *      pPortmask - the ports for RTCT test
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_maple_diag_rtct_start(uint32 unit, rtk_portmask_t *pPortmask)
{
    rtk_port_t  port, max_port;
    int32       ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "unit=%d, *pPortmask=0x%x",
           unit, *pPortmask);

    /* check Init status */
    RT_INIT_CHK(diag_init[unit]);

    DIAG_SEM_LOCK(unit);
    max_port = HAL_GET_MAX_PORT(unit);
    for (port = 0; port < max_port; port++)
    {
        if (RTK_PORTMASK_IS_PORT_SET(*pPortmask, port))
        {
            if ((ret = phy_rtct_start(unit, port)) != RT_ERR_OK)
            {
                DIAG_SEM_UNLOCK(unit);
                RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "ret=0x%x", ret);
                return ret;
            }
        }
    }
    DIAG_SEM_UNLOCK(unit);

    return RT_ERR_OK;
} /* end of dal_maple_diag_rtct_start */

/* Function Name:
 *      dal_maple_diag_table_read
 * Description:
 *      Read one specified table entry by table index.
 * Input:
 *      unit  - unit id
 *      table - table index
 *      addr  - entry address of the table
 * 	  pData - pointer buffer for read back table entry
 *      pRev_vaild - used to sure the revbits is vaild.
 * 	  pRevbits - pointer buffer for read reverse bits which are not contain in entry.
 * Output:
 * 	  pData - pointer buffer for read back table entry
 *      pRev_vaild - used to sure the revbits is vaild.
 * 	  pRevbits - pointer buffer for read reverse bits which are not contain in entry.
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - Failed
 * Note:
 *      1. Basically, this is a transparent API for table read.
 *      2. For L2 hash table, this API will converse the hiding 12-bits,
 *          and provide to upper layer by pRevbits parameter.
 *      3. addr format :
 *          From RTK and realchip view : hash_key[13:2]location[1:0]
 */
int32
dal_maple_diag_table_read(uint32  unit, uint32  table, uint32  addr, uint32  *pData, uint32 *pRev_vaild, uint32 *pRevbits)
{
#if 1

	uint32 l2_entry_index;
	uint32 entry_vailed, ipmc_fmt, entry_vid;
	int32  ret;
	dal_maple_l2_entry_t l2_enrty;

	if((INT_MAPLE_L2_IP_MC_RTL8380 <= table)&&(INT_MAPLE_L2_UC_RTL8380 >= table))
	{
		ret = dal_maple_l2_getL2EntryfromHash_dump(unit, addr, &l2_enrty, &entry_vailed);
		if(TRUE == entry_vailed)
		{
			osal_printf("\n***Entry(%d) is Valid - ",addr);
			switch(l2_enrty.entry_type)
			{
			case L2_UNICAST:
				osal_printf("TYPE is L2_UNICAST,\n   VID = %d",l2_enrty.unicast.fid);
				break;
			case L2_MULTICAST:
				osal_printf("TYPE is L2_MULTICAST,\n   VID = %d",l2_enrty.l2mcast.rvid);
				break;
			case IP4_MULTICAST:
				ret = dal_maple_l2_ipmcMode_get(unit, &ipmc_fmt);
				if(ipmc_fmt == 0)
					entry_vid = l2_enrty.ipmcast_mc_ip.rvid;
				else if(ipmc_fmt == 1)
					entry_vid = l2_enrty.ipmcast_ip_mc_sip.rvid;
				else
					entry_vid = l2_enrty.ipmcast_ip_mc.rvid;
				osal_printf("TYPE is IP4_MULTICAST,\n   VID = %d",entry_vid);
				break;
			case IP6_MULTICAST:
				ret = dal_maple_l2_ipmcMode_get(unit, &ipmc_fmt);
				if(ipmc_fmt == 0)
					entry_vid = l2_enrty.ipmcast_mc_ip.rvid;
				else if(ipmc_fmt == 1)
					entry_vid = l2_enrty.ipmcast_ip_mc_sip.rvid;
				else
					entry_vid = l2_enrty.ipmcast_ip_mc.rvid;
				osal_printf("TYPE is IP6_MULTICAST,\n   VID = %d",entry_vid);
				break;
			default:
				osal_printf("TYPE Error\n");
			}
			l2_entry_index = addr;
			table_read(unit, table, l2_entry_index, pData);
		}else{
			l2_entry_index = addr;
			table_read(unit, table, l2_entry_index, pData);
		}
		return RT_ERR_OK;

#if	0   /* Mask fro debugging */
		RT_PARAM_CHK(table_field_get(unit, ESW_L2_ISFTIDXt, ESW_L2_ISFTIDX_AETf, &l2_table_entry_type, pData), RT_ERR_FAILED);
		if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AED_47_16f, &aed0, pData)) != RT_ERR_OK)
		{
			RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
			return ret;
		}

		if ((ret = table_field_get(unit, ESW_L2_ISNOTFTIDXt, ESW_L2_ISNOTFTIDX_AED_15_0f, &aed1, pData)) != RT_ERR_OK)
		{
			RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "");
			return ret;
		}
		RT_PARAM_CHK(_dal_esw_l2_hash_entry_GetReverseBit(h_key, hash_alog_type, l2_table_entry_type, aed0, aed1, &l2_reserved_bits), RT_ERR_FAILED);
#endif

	}else{
		l2_entry_index = addr;
		table_read(unit, table, l2_entry_index, pData);
		*pRev_vaild = L2ENTRY_REVERSE_INVALID;
	}
#endif
    return RT_ERR_OK;
} /* end of dal_maple_diag_table_read */


