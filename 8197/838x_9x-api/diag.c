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
 * Purpose : Definition those public diagnostic routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           (1) Remote/Local Loopback
 *           (2) RTCT
 *           (3) Dump whole registers and tables
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <common/type.h>
#include <dal/dal_mgmt.h>
#include <rtk/default.h>
#include <rtk/diag.h>
#include <hal/mac/mem.h>
#include <hal/common/halctrl.h>
#include <hal/mac/drv.h>
#include <hal/mac/reg.h>
#include <osal/print.h>
#include <osal/memory.h>
#include <ioal/ioal_init.h>
#include <ioal/mem32.h>
#include <drv/swcore/chip.h>


/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Module Name    : diagnostic */
/* Sub-module Name: Global     */

/* Function Name:
 *      rtk_diag_init
 * Description:
 *      Initialize diagnostic module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8328, 8389, 8390, 8380
 * Note:
 *      (1) Module must be initialized before using all of APIs in this module
 */
int32
rtk_diag_init(uint32 unit)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->diag_init(unit);
} /* end of rtk_diag_init */

/* Function Name:
 *      rtk_diag_portMacRemoteLoopbackEnable_get
 * Description:
 *      Get the mac remote loopback enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac remote loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8389
 * Note:
 *      (1) The mac remote loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
rtk_diag_portMacRemoteLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->diag_portMacRemoteLoopbackEnable_get(unit, port, pEnable);
} /* end of rtk_diag_portMacRemoteLoopbackEnable_get */

/* Function Name:
 *      rtk_diag_portMacRemoteLoopbackEnable_set
 * Description:
 *      Set the mac remote loopback enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of mac remote loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8328, 8389
 * Note:
 *      (1) The mac remote loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Remote loopback is used to loopback packet RX to switch core back to the outer interface.
 */
int32
rtk_diag_portMacRemoteLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->diag_portMacRemoteLoopbackEnable_set(unit, port, enable);
} /* end of rtk_diag_portMacRemoteLoopbackEnable_set */

/* Function Name:
 *      rtk_diag_portMacLocalLoopbackEnable_get
 * Description:
 *      Get the mac local loopback enable status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to the enable status of mac local loopback
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *       8389
 * Note:
 *      (1) The mac local loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
rtk_diag_portMacLocalLoopbackEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->diag_portMacLocalLoopbackEnable_get(unit, port, pEnable);
} /* end of rtk_diag_portMacLocalLoopbackEnable_get */

/* Function Name:
 *      rtk_diag_portMacLocalLoopbackEnable_set
 * Description:
 *      Set the mac local loopback enable status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of mac local loopback
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Applicable:
 *      8389
 * Note:
 *      (1) The mac local loopback enable status of the port is as following:
 *          - DISABLE
 *          - ENABLE
 *      (2) Local loopback is used to loopback packet TX from switch core back to switch core.
 */
int32
rtk_diag_portMacLocalLoopbackEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    return RT_MAPPER(unit)->diag_portMacLocalLoopbackEnable_set(unit, port, enable);
} /* end of rtk_diag_portMacLocalLoopbackEnable_set */


/* Module Name    : Diag */
/* Sub-module Name: RTCT */

/* Function Name:
 *      rtk_diag_portRtctResult_get
 * Description:
 *      Get test result of Realtek Cable Tester.
 * Input:
 *      unit        - unit id
 *      port        - the port for retriving RTCT test result
 * Output:
 *      pRtctResult - RTCT result
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID             - invalid unit id
 *      RT_ERR_NOT_INIT            - The module is not initial
 *      RT_ERR_PORT_ID             - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT             - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER        - input parameter may be null pointer
 * Applicable:
 *      8328, 8389, 8390, 8380
 * Note:
 *      (1) If linkType is PORT_SPEED_1000M, test result will be stored in ge_result.
 *          If linkType is PORT_SPEED_10M or PORT_SPEED_100M, test result will be stored in fe_result.
 *      (2) The unit of cable lenght is meter.
 */
int32
rtk_diag_portRtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    int32  phy_port = RTK_PORT_TO_PHYSICAL_PORT(unit, port);

    return RT_MAPPER(unit)->diag_portRtctResult_get(unit, phy_port, pRtctResult);
    }
#else
    return RT_MAPPER(unit)->diag_portRtctResult_get(unit, port, pRtctResult);
#endif
} /* end of rtk_diag_portRtctResult_get */

/* Function Name:
 *      rtk_diag_rtctEnable_set
 * Description:
 *      Start Realtek Cable Tester for ports.
 * Input:
 *      unit      - unit id
 *      pPortmask - the ports for RTCT test
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Applicable:
 *      8328, 8389, 8390, 8380
 * Note:
 *      When RTCT starts, the port won't transmit and receive normal traffic.
 */
int32
rtk_diag_rtctEnable_set(uint32 unit, rtk_portmask_t *pPortmask)
{
    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

#if defined(CONFIG_SDK_PORT_VIEW_ZERO_BASE_PORT)
    {
    rtk_portmask_t  temp_portmask = RTK_PORTMASK_TO_PHYSICAL_PORTMASK(unit, *pPortmask);

    return RT_MAPPER(unit)->diag_rtct_start(unit, &temp_portmask);
    }
#else
    return RT_MAPPER(unit)->diag_rtct_start(unit, pPortmask);
#endif
} /* end of rtk_diag_rtctEnable_set */

/* Function Name:
 *      rtk_diag_table_whole_read
 * Description:
 *      Dump whole table content in console for debugging
 * Input:
 *      unit        - unit id
 *      table_index - dumped table index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_NOT_INIT     - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - dumped table index is out of range
 * Applicable:
 *      8389, 8328, 8380
 * Note:
 *      None
 */
int32
rtk_diag_table_whole_read(uint32 unit, uint32 table_index)
{
    rtk_table_t *pDump_table = NULL;
    uint32 *pDump_data_buf = NULL;
    uint32 dump_table_loop_index, dump_entry_loop_index;
    uint32 dump_entry_size,total_table_num;
    uint32 reverse_valid;
    uint32 reverse_data;
    uint32 ret;
#if defined(CONFIG_SDK_DUMP_TABLE_WITH_NAME)
	char   table_name[64];
#endif
	total_table_num = HAL_GET_MAX_TABLE_IDX(unit);
    osal_printf("\nTotal Table Number = %d",total_table_num);

	RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);
	if(table_index != 0xff)	/*Just dump specific table*/
		RT_PARAM_CHK((table_index >= HAL_GET_MAX_TABLE_IDX(unit)), RT_ERR_OUT_OF_RANGE);

	for (dump_table_loop_index = 0; dump_table_loop_index < total_table_num; dump_table_loop_index++)
	{
		if(table_index != 0xff) /*Just dump specific table*/
		{
			dump_table_loop_index = table_index;
			total_table_num = dump_table_loop_index;
		}
	    pDump_table = table_find(unit, dump_table_loop_index);
#if defined(CONFIG_SDK_DUMP_TABLE_WITH_NAME)
		osal_memset(&table_name, 0, 64);
		ret = table_name_get(unit, dump_table_loop_index, table_name);
		if(ret == RT_ERR_OK)
		{
	    	osal_printf("\nTable Type(Phyical ID)  = %d, [%s]",pDump_table->type,table_name);
		}else{
	    	osal_printf("\nTable Type(Phyical ID)  = %d",pDump_table->type);
		}
#else
        osal_printf("\nTable Type(Phyical ID)  = %d",pDump_table->type);
#endif
	    osal_printf("\nTable size              = %d",pDump_table->size);
	    osal_printf("\nTable datareg_num       = %d",pDump_table->datareg_num);
	    osal_printf("\nTable field_num         = %d\n",pDump_table->field_num);
		osal_printf("\nTable Index			   = %d\n",dump_table_loop_index);

		if(table_index == 0xff) /*Dump whole tables*/
		{
			if((HAL_IS_RTL8390_FAMILY_ID(unit))||(HAL_IS_RTL8350_FAMILY_ID(unit)))
				if((dump_table_loop_index > 4)&&(dump_table_loop_index < 10))
					continue;
			if((HAL_IS_RTL8380_FAMILY_ID(unit))||(HAL_IS_RTL8330_FAMILY_ID(unit)))
				if((dump_table_loop_index > 5)&&(dump_table_loop_index < 11))
					continue;
		}

		pDump_data_buf = (uint32 *)osal_alloc((uint32)(sizeof(uint32)*pDump_table->datareg_num));

	    for (dump_entry_loop_index = 0; dump_entry_loop_index < (pDump_table->size); dump_entry_loop_index++)
	    {
    	    osal_memset(pDump_data_buf, 0, (sizeof(uint32)*pDump_table->datareg_num));
	    #if 0 /*If we don't care the L2 hash table entry doesn't contain XXX*/
    	    table_read(unit, table_index, dump_entry_loop_index, pDump_data_buf);
	    #endif
    	    reverse_valid = REVERSE_BIT_INVALID;
        	reverse_data = REVERSE_BIT_INVALID;
	        if ((ret = RT_MAPPER(unit)->diag_table_read(unit, dump_table_loop_index, dump_entry_loop_index, pDump_data_buf, &reverse_valid, &reverse_data)) != RT_ERR_OK)
    	    {
        	    osal_printf("\nRead table failed., ERROR Code = %x\n", ret);
            	return RT_ERR_FAILED;
	        }
    	    osal_printf("\nEntry(%05d) ",dump_entry_loop_index);

	        /*Display by Data Reg*/
    	    for (dump_entry_size = 0; dump_entry_size < pDump_table->datareg_num; dump_entry_size++)
	        {
        	    if ((dump_entry_size != 0)&&((dump_entry_size%8) == 0))
            	    osal_printf("\n            ");

    	        osal_printf("-%08x",*(pDump_data_buf+(dump_entry_size)));
        	}

    	}
	    osal_printf("\n");
    	osal_free((void *)pDump_data_buf);
	}
    return RT_ERR_OK;
} /* end of rtk_diag_table_whole_read */


/* Function Name:
 *      rtk_diag_phy_reg_whole_read
 * Description:
 *      Dump all standard registers of PHY of all ports
 * Input:
 *      unit - unit id
 * Output:
 *      all registers value in console.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8390, 8380
 * Note:
 */
int32
rtk_diag_phy_reg_whole_read(uint32 unit)
{
    rtk_switch_devInfo_t    devInfo;
    uint32                  port,page=0,pData,i;
    rtk_port_phy_reg_t      reg[]={ PHY_REG_CONTROL,
                                    PHY_REG_STATUS,
                                    PHY_REG_IDENTIFIER_1,
                                    PHY_REG_IDENTIFIER_2,
                                    PHY_REG_AN_ADVERTISEMENT,
                                    PHY_REG_AN_LINKPARTNER,
                                    PHY_REG_1000_BASET_CONTROL,
                                    PHY_REG_1000_BASET_STATUS,
                                    PHY_REG_END};

    memset(&devInfo, 0, sizeof(rtk_switch_devInfo_t));
    if (rtk_switch_deviceInfo_get(unit, &devInfo) != RT_ERR_OK)
    {
        return RT_ERR_FAILED;
    }

    osal_printf("PHY registers: (format: reg:value,...)\n");

    for (port = (uint32)devInfo.ether.min; port <= (uint32)devInfo.ether.max; port++)
    {
        if (!RTK_PORTMASK_IS_PORT_SET(devInfo.ether.portmask, port))
            continue;

        osal_printf("Port%2d",port);
        osal_printf("    ");
        i=0;
        while(reg[i]!=PHY_REG_END){
            if( RT_MAPPER(unit)->port_phyReg_get(unit, port, page, reg[i], &pData) != RT_ERR_OK)
            {
                return RT_ERR_FAILED;
            }
            osal_printf("%d:0x%04x,",reg[i],pData);
            i++;
        }
        osal_printf("\n");
    }

    return RT_ERR_OK;
}

/* Function Name:
 *      rtk_diag_reg_whole_read
 * Description:
 *      Dump all registers' value in console for debugging
 * Input:
 *      unit - unit id
 * Output:
 *      all registers value in console.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Applicable:
 *      8389, 8328, 8380
 * Note:
 */
int32
rtk_diag_reg_whole_read(uint32 unit)
{
    uint32  dump_reg_loop_index;
    uint32  addr = 0;
    uint32  value = 0;
    uint32  dim1, dim2, field_no_of_word, dim2_max, array_dim;
    int32   ret = RT_ERR_FAILED;
    rtk_reg_info_t  reg_data;
    rtk_reg_info_t  dump_reg_info;
	uint32	bit_offset_size;
	uint32	bit_offset_32unit, dump_loop;
#ifdef CONFIG_SDK_DUMP_REG_WITH_NAME
    char	reg_name[64];
#endif

    osal_memset(&reg_data, 0, sizeof(rtk_reg_info_t));

    RT_PARAM_CHK((unit > RTK_MAX_UNIT_ID) || NULL == RT_MGMT(unit), RT_ERR_UNIT_ID);

    osal_printf("\nMAX_REG_IDX = %d\n",HAL_GET_MAX_REG_IDX(unit));

    /* dump registers of MAC */
    for (dump_reg_loop_index = 0; dump_reg_loop_index < HAL_GET_MAX_REG_IDX(unit); dump_reg_loop_index++)
    {
        if (RT_ERR_OK != reg_info_get(unit, dump_reg_loop_index, &dump_reg_info))
        {
            osal_printf("\nCANNOT get reg info\n");
            return RT_ERR_FAILED;
        }

        if (dump_reg_info.offset >= SWCORE_MEM_SIZE)
            return RT_ERR_FAILED;

#ifdef CONFIG_SDK_DUMP_REG_WITH_NAME
        osal_memset(&reg_name, 0, 64);
		ret = reg_name_get(unit, dump_reg_loop_index, reg_name);
        if (RT_ERR_OK == ret)
        {
			osal_printf("\nREG-Idx(%04d)[%s] bit_offset = %d: ",dump_reg_loop_index, reg_name, dump_reg_info.bit_offset);
    	}else{
			osal_printf("\nREG-Idx(%04d) bit_offset = %d: ",dump_reg_loop_index,dump_reg_info.bit_offset);
    	}
#else
		osal_printf("\nREG-Idx(%04d) bit_offset = %d: ",dump_reg_loop_index,dump_reg_info.bit_offset);
#endif
        if (dump_reg_info.bit_offset == 0)
        {   /* non register array case */
            RT_PARAM_CHK(ioal_mem32_read(unit, dump_reg_info.offset, &value), ret);
            osal_printf("\nRegister 0x%x : 0x%08x", dump_reg_info.offset, value);
        }
        else if ((dump_reg_info.bit_offset % 32) == 0)
        {   /* register array case (unit: word) */
            if (dump_reg_info.bit_offset == 32)
            {
                for (dim1 = dump_reg_info.lport; dim1 <= dump_reg_info.hport; dim1++)
                {
                    for (dim2 = dump_reg_info.larray; dim2 <= dump_reg_info.harray; dim2++)
                    {
                        if (dump_reg_info.is_PpBlock)
                            addr = dump_reg_info.offset + (dim1 - dump_reg_info.lport) * 0x100 + (dim2 - dump_reg_info.larray) * 0x4;
                        else
                            addr = dump_reg_info.offset + (dim1 - dump_reg_info.lport) * (dump_reg_info.harray - dump_reg_info.larray + 1) * 0x4 + (dim2 - dump_reg_info.larray) * 0x4;
                        RT_PARAM_CHK(ioal_mem32_read(unit, addr, &value), ret);
                        osal_printf("\nRegister 0x%x : 0x%08x", addr, value);
                    }
                }
            }
            else
            {
            	/*vv============*/
            	bit_offset_size = dump_reg_info.bit_offset;

                for (dim1 = dump_reg_info.lport; dim1 <= dump_reg_info.hport; dim1++)
                {
                    for (dim2 = dump_reg_info.larray; dim2 <= dump_reg_info.harray; dim2++)
                    {
                    	bit_offset_32unit = (bit_offset_size / 32);
						for(dump_loop = 0; dump_loop < bit_offset_32unit; dump_loop++)
						{
	                        if (dump_reg_info.is_PpBlock)
    	                        addr = dump_reg_info.offset + (dim1 - dump_reg_info.lport) * 0x100 + (dim2 - dump_reg_info.larray) * 0x4;
        	                else
            	                addr = dump_reg_info.offset + (dim1 - dump_reg_info.lport) * (dump_reg_info.harray - dump_reg_info.larray + 1) * ( bit_offset_32unit * 0x4) + (dim2 - dump_reg_info.larray) * ( bit_offset_32unit * 0x4) + (dump_loop * 4);
                	        RT_PARAM_CHK(ioal_mem32_read(unit, addr, &value), ret);
                    	    osal_printf("\nRegister 0x%x : 0x%08x", addr, value);
						}
                    }
                }
				/*^^============*/

            }
        }
        else
        {
            /* register array case (unit: bit) */
            field_no_of_word = 32/dump_reg_info.bit_offset;
            if (dump_reg_info.lport != dump_reg_info.hport)
            {
                if (dump_reg_info.larray != dump_reg_info.harray)
                    array_dim = 2;
                else
                    array_dim = 1;
            }
            else
            {
                array_dim = 0;
            }

            osal_printf("port = (%d~%d), array = (%d~%d)",dump_reg_info.lport, dump_reg_info.hport,dump_reg_info.larray,dump_reg_info.harray);
            if (array_dim == 2)
            {
                if ((dump_reg_info.harray-dump_reg_info.larray+1) % field_no_of_word)
                    dim2_max = (dump_reg_info.harray-dump_reg_info.larray+1)/field_no_of_word;
                else
                    dim2_max = (dump_reg_info.harray-dump_reg_info.larray+1)/field_no_of_word -1;
                for (dim1 = dump_reg_info.lport; dim1 <= dump_reg_info.hport; dim1++)
                {
                    osal_printf("\nPort/Index(%d)",dim1);
                    for (dim2 = 0; dim2 <= dim2_max; dim2++)
                    {
                        if (dump_reg_info.is_PpBlock)
                            addr = dump_reg_info.offset + (dim1- dump_reg_info.lport) * 0x100 + (dim2) * 0x4;
                        else
                            addr = dump_reg_info.offset + (dim1 - dump_reg_info.lport) * (dim2_max + 1) * 0x4 + (dim2) * 0x4;
                        RT_PARAM_CHK(ioal_mem32_read(unit, addr, &value), ret);
                        osal_printf("\nRegister 0x%x : 0x%08x", addr, value);
                    }
                }
            }
            else if (array_dim == 1) /*One Dim in Port Index*/
            {
                if ((dump_reg_info.hport-dump_reg_info.lport+1) % field_no_of_word)
                    dim2_max = (dump_reg_info.hport-dump_reg_info.lport+1)/field_no_of_word;
                else
                    dim2_max = (dump_reg_info.hport-dump_reg_info.lport+1)/field_no_of_word -1;

                for (dim2 = 0; dim2 <= dim2_max; dim2++)
                {
                    addr = dump_reg_info.offset + (dim2) * 0x4;
                    RT_PARAM_CHK(ioal_mem32_read(unit, addr, &value), ret);
                    osal_printf("\nRegister 0x%x : 0x%08x", addr, value);
                }
            }
            else
            {
				/*VV============*/
                if (dump_reg_info.larray != dump_reg_info.harray) /*One Dim in Array Index*/
                {
                	if ((dump_reg_info.harray-dump_reg_info.larray+1) % field_no_of_word)
                    	dim2_max = (dump_reg_info.harray-dump_reg_info.larray+1)/field_no_of_word;
	                else
    	                dim2_max = (dump_reg_info.harray-dump_reg_info.larray+1)/field_no_of_word -1;

        	        for (dim2 = 0; dim2 <= dim2_max; dim2++)
            	    {
                	    addr = dump_reg_info.offset + (dim2) * 0x4;
                    	RT_PARAM_CHK(ioal_mem32_read(unit, addr, &value), ret);
	                    osal_printf("\nRegister 0x%x : 0x%08x", addr, value);
    	            }
                }else
                {
				/*^^============*/
                	osal_printf("\nRegister 0x%x : Unexpected Case\n", dump_reg_info.offset);
                }
            }
        }
        osal_printf("\n");
    }
    return RT_ERR_OK;
} /* end of rtk_diag_reg_whole_read */



/* Function Name:
 *      rtk_diag_peripheral_register_dump
 * Description:
 *      Dump Chip peripheral registers
 * Input:
 *      unit           - unit id
 * Output:
 *      N/A
 * Return:
 *      RT_ERR_OK        - OK
 *      RT_ERR_FAILED   - Failed
 * Note:
 *      None
 */
int32
rtk_diag_peripheral_register_dump(uint32 unit)
{
	int ret;

	ret = drv_swcore_register_dump(unit);

    return RT_ERR_OK;

} /* end of rtk_diag_peripheral_register_dump */

