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
 * $Revision: 38552 $
 * $Date: 2013-04-12 16:43:28 +0800 (Fri, 12 Apr 2013) $
 *
 * Purpose : chip symbol and data type definition in the SDK.
 *
 * Feature : chip symbol and data type definition
 *
 */

/*
 * Include Files
 */
#include <ioal/mem32.h>
#include <drv/swcore/chip.h>
#include <drv/swcore/sys.h>

#if defined(CONFIG_SDK_RTL8390)
#include <drv/swcore/rtl8390.h>
#endif
#if defined(CONFIG_SDK_RTL8389)
#include <drv/swcore/rtl8389.h>
#endif
#if defined(CONFIG_SDK_RTL8328)
#include <drv/swcore/rtl8328.h>
#endif
#if defined(CONFIG_SDK_RTL8380)
#include <drv/swcore/rtl8380.h>
#endif

/*
 * Symbol Definition
 */
#define SYS_MAC_DB_SIZE     (sizeof(sys_mac_db)/sizeof(cid_group_t))

/*
 * Data Declaration
 */
const static cid_group_t sys_mac_db[] =
{
#if defined(CONFIG_SDK_RTL8389)
    {RTL8389M_CHIP_ID, SYS_MAC_RTL8389},
    {RTL8329M_CHIP_ID, SYS_MAC_RTL8389},
    {RTL8377M_CHIP_ID, SYS_MAC_RTL8389},
    {RTL8389L_CHIP_ID, SYS_MAC_RTL8389},
#endif
#if defined(CONFIG_SDK_RTL8328)
    {RTL8328M_CHIP_ID, SYS_MAC_RTL8328},
    {RTL8328S_CHIP_ID, SYS_MAC_RTL8328},
    {RTL8328L_CHIP_ID, SYS_MAC_RTL8328},
#endif
#if defined(CONFIG_SDK_RTL8390)
    {RTL8352M_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8353M_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8391M_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8392M_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8393M_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8396M_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8352MES_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8353MES_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8392MES_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8393MES_CHIP_ID, SYS_MAC_RTL8390},
    {RTL8396MES_CHIP_ID, SYS_MAC_RTL8390},
#endif
#if defined(CONFIG_SDK_RTL8380)
    {RTL8330M_CHIP_ID, SYS_MAC_RTL8380},
    {RTL8332M_CHIP_ID, SYS_MAC_RTL8380},
    {RTL8380M_CHIP_ID, SYS_MAC_RTL8380},
    {RTL8382M_CHIP_ID, SYS_MAC_RTL8380},
    {RTL8330MES_CHIP_ID, SYS_MAC_RTL8380},
    {RTL8332MES_CHIP_ID, SYS_MAC_RTL8380},
    {RTL8380MES_CHIP_ID, SYS_MAC_RTL8380},
    {RTL8382MES_CHIP_ID, SYS_MAC_RTL8380},    
#endif
};

const static sys_mac_reg_t sys_mac_addr[SYS_MAC_END] =
{
#if defined(CONFIG_SDK_RTL8389)
    {   /* SYS_MAC_RTL8389 */
        RTL8389_SWITCH_MAC_ADDRESS0_ADDR,
        RTL8389_SWITCH_MAC_ADDRESS1_ADDR,
        RTL8389_SWITCH_MAC_ADDRESS1_SW_MAC_ADDR_OFFSET,
        RTL8389_SWITCH_MAC_ADDRESS1_SW_MAC_ADDR_MASK
    },
#endif
#if defined(CONFIG_SDK_RTL8328)
    {   /* SYS_MAC_RTL8328 */
        RTL8328_SWITCH_MANAGEMENT_MAC_ADDRESS0_ADDR,
        RTL8328_SWITCH_MANAGEMENT_MAC_ADDRESS1_ADDR,
        RTL8328_SWITCH_MANAGEMENT_MAC_ADDRESS1_MAN_MAC_15_0_OFFSET,
        RTL8328_SWITCH_MANAGEMENT_MAC_ADDRESS1_MAN_MAC_15_0_MASK
    },
#endif
#if defined(CONFIG_SDK_RTL8390)
    {   /* SYS_MAC_RTL8390 */
        RTL8390_MAC_ADDR_CTRL_ADDR,
        RTL8390_MAC_ADDR_CTRL_ADDR + 4,
        RTL8390_MAC_ADDR_CTRL_SW_MAC_ADDR_31_0_OFFSET,
        RTL8390_MAC_ADDR_CTRL_SW_MAC_ADDR_31_0_MASK
    },
#endif
#if defined(CONFIG_SDK_RTL8380)
    {   /* SYS_MAC_RTL8380 */
        RTL8380_MAC_ADDR_CTRL_ADDR, //RTL8380_MAC_ADDR_CTRL_ALE_ADDR,
        RTL8380_MAC_ADDR_CTRL_ADDR + 4,//RTL8380_MAC_ADDR_CTRL_ALE_ADDR + 4,
        RTL8380_MAC_ADDR_CTRL_SW_MAC_ADDR_31_0_OFFSET, //RTL8380_MAC_ADDR_CTRL_ALE_SW_MAC_ADDR_31_0_OFFSET,
        RTL8380_MAC_ADDR_CTRL_SW_MAC_ADDR_31_0_MASK, //RTL8380_MAC_ADDR_CTRL_ALE_SW_MAC_ADDR_31_0_MASK
    },
#endif

};


/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      drv_swcore_sysMac_get
 * Description:
 *      Get mac address of the system
 * Input:
 *      None
 * Output:
 *      pMacAddr - mac address
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32
drv_swcore_sysMac_get(uint8 *pMacAddr)
{
    uint32 i, idx, mac_high, mac_low;
	uint32 chip_id, chip_rev_id;
	int32  ret; 

    /* parameter check */
    RT_PARAM_CHK((NULL == pMacAddr), RT_ERR_NULL_POINTER);

    ret = drv_swcore_cid_get((unsigned int)0, (unsigned int *)&chip_id, (unsigned int *)&chip_rev_id);
    if(RT_ERR_OK == ret)
    {
#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
	if(((chip_id & FAMILY_ID_MASK) == RTL8390_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8350_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8380_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8330_FAMILY_ID))
	{		
    		for (i=0, idx=0; i<SYS_MAC_DB_SIZE; i++)
	        	if(!drv_swcore_cid_cmp(SYS_UNIT_ID, sys_mac_db[i].cid))
	        	    idx = sys_mac_db[i].gid;

#if defined(CONFIG_SDK_RTL8380) && defined(CONFIG_SDK_FPGA_PLATFORM)
		    /*8380 FPGA Issue: should write register once before read register*/
		    ioal_mem32_write(SYS_UNIT_ID, 0xb7e4, 0x0);
#endif

		    ioal_mem32_read(SYS_UNIT_ID, sys_mac_addr[idx].mac_high, &mac_high);
		    ioal_mem32_read(SYS_UNIT_ID, sys_mac_addr[idx].mac_low, &mac_low);

		    *(pMacAddr + 0) = (mac_high & 0x0000FF00) >> 8;
		    *(pMacAddr + 1) = (mac_high & 0x000000FF) >> 0;
		    *(pMacAddr + 2) = (mac_low & 0xFF000000) >> 24;
		    *(pMacAddr + 3) = (mac_low & 0x00FF0000) >> 16;
		    *(pMacAddr + 4) = (mac_low & 0x0000FF00) >> 8;
		    *(pMacAddr + 5) = (mac_low & 0x000000FF) >> 0;
	}		
#endif

#if defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8389)
	if(((chip_id & RTL8328_FAMILY_ID) == RTL8328_FAMILY_ID) || ((chip_id & RTL8389_FAMILY_ID) == RTL8389_FAMILY_ID))
	{
		for (i=0, idx=0; i<SYS_MAC_DB_SIZE; i++)
        		if(!drv_swcore_cid_cmp(SYS_UNIT_ID, sys_mac_db[i].cid))
            			idx = sys_mac_db[i].gid;

		ioal_mem32_read(SYS_UNIT_ID, sys_mac_addr[idx].mac_high, &mac_high);
    		ioal_mem32_field_read(SYS_UNIT_ID, sys_mac_addr[idx].mac_low, \
        		sys_mac_addr[idx].low_offset, sys_mac_addr[idx].low_mask, &mac_low);

    		*(pMacAddr + 0) = (mac_high & 0xFF000000) >> 24;
    		*(pMacAddr + 1) = (mac_high & 0x00FF0000) >> 16;
    		*(pMacAddr + 2) = (mac_high & 0x0000FF00) >> 8;
    		*(pMacAddr + 3) = (mac_high & 0x000000FF) >> 0;
    		*(pMacAddr + 4) = (mac_low & 0xFF00) >> 8;
    		*(pMacAddr + 5) = (mac_low & 0x00FF) >> 0;
	}	
#endif

   	}
        return RT_ERR_OK;
} /* end of drv_swcore_sysMac_get */

/* Function Name:
 *      drv_swcore_sysMac_set
 * Description:
 *      Set mac address of the system
 * Input:
 *      pMacAddr - mac address
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_NULL_POINTER
 * Note:
 *      None
 */
int32
drv_swcore_sysMac_set(uint8 *pMacAddr)
{
    uint32 i, idx, mac_high, mac_low;
	uint32 chip_id, chip_rev_id;
	int32  ret; 

    /* parameter check */
    RT_PARAM_CHK((NULL == pMacAddr), RT_ERR_NULL_POINTER);

    for (i=0, idx=0; i<SYS_MAC_DB_SIZE; i++)
        if(!drv_swcore_cid_cmp(SYS_UNIT_ID, sys_mac_db[i].cid))
            idx = sys_mac_db[i].gid;

	ret = drv_swcore_cid_get((unsigned int)0, (unsigned int *)&chip_id, (unsigned int *)&chip_rev_id);
	if(RT_ERR_OK == ret)
	{
#if defined(CONFIG_SDK_RTL8380) || defined(CONFIG_SDK_RTL8390)
	    if(((chip_id & FAMILY_ID_MASK) == RTL8390_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8350_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8380_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8330_FAMILY_ID))
	    {		
    		mac_high = mac_low = 0;
    		mac_high |= *(pMacAddr + 0) << 8;
    		mac_high |= *(pMacAddr + 1) << 0;
    		mac_low  |= *(pMacAddr + 2) << 24;
    		mac_low  |= *(pMacAddr + 3) << 16;
    		mac_low  |= *(pMacAddr + 4) << 8;
    		mac_low  |= *(pMacAddr + 5) << 0;

    		ioal_mem32_write(SYS_UNIT_ID, sys_mac_addr[idx].mac_high, mac_high);
    		ioal_mem32_write(SYS_UNIT_ID, sys_mac_addr[idx].mac_low, mac_low);
#if defined(CONFIG_SDK_RTL8380)
			if(((chip_id & FAMILY_ID_MASK) == RTL8380_FAMILY_ID) || ((chip_id & FAMILY_ID_MASK) == RTL8330_FAMILY_ID))
			{
    		/* Write to another two duplicate registers, ALE/MAC block */
    		ioal_mem32_write(SYS_UNIT_ID, RTL8380_MAC_ADDR_CTRL_ALE_ADDR, mac_high);
    		ioal_mem32_write(SYS_UNIT_ID, RTL8380_MAC_ADDR_CTRL_ALE_ADDR + 4, mac_low);
    		ioal_mem32_write(SYS_UNIT_ID, RTL8380_MAC_ADDR_CTRL_MAC_ADDR, mac_high);
    		ioal_mem32_write(SYS_UNIT_ID, RTL8380_MAC_ADDR_CTRL_MAC_ADDR + 4, mac_low);
			}
#endif
	    }
#endif

#if defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8389)
	   if(((chip_id & RTL8328_FAMILY_ID) == RTL8328_FAMILY_ID) || ((chip_id & RTL8389_FAMILY_ID) == RTL8389_FAMILY_ID))
  	   {
    		mac_high = mac_low = 0;
    		mac_high |= *(pMacAddr + 0) << 24;
    		mac_high |= *(pMacAddr + 1) << 16;
    		mac_high |= *(pMacAddr + 2) << 8;
    		mac_high |= *(pMacAddr + 3) << 0;
    		mac_low  |= *(pMacAddr + 4) << 8;
    		mac_low  |= *(pMacAddr + 5) << 0;

    		ioal_mem32_write(SYS_UNIT_ID, sys_mac_addr[idx].mac_high, mac_high);
    		ioal_mem32_field_write(SYS_UNIT_ID, sys_mac_addr[idx].mac_low, \
        		sys_mac_addr[idx].low_offset, sys_mac_addr[idx].low_mask, mac_low);
	   }	
#endif
	}
    return RT_ERR_OK;
} /* end of drv_swcore_sysMac_set */

