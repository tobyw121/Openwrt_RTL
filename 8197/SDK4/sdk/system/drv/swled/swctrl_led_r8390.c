
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
 * $Revision: $
 * $Date: $
 *
 * Purpose : Software Control LED for RTL8231 Serial Mode
 *
 * Feature : For RTL8389 access
 * 
 */

#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/thread.h>
#include <ioal/mem32.h>
#include <osal/isr.h>
#include <osal/time.h>
#include <drv/swled/swctrl_led_main.h>
#include <drv/swcore/rtl8390.h>

/* Function Name: 
 *		rtl8390_getAsicPortSpeedDuplex
 * Description: 
 *		Get the specified port's speed and duplex status.
 * Input:  
 *		port		- the specified port
 * Output: 
 *		
 *		  pSpeed	   - the link status of specified poty. 
 *                              10M = 0; 100M = 1; 1000M =2 
 *		  pDuplex	   - the link status of specified poty.
 *					FULL = 1; HALF = 0
 * Return: 
 *		RT_ERR_OK		  - function is successed
 *		RT_ERR_FAILED	  - function is failed
 * Note: 
 *		  
 */
int32 rtl8390_getAsicPortSpeedDuplex(int32 port, uint32 *pSpeed, uint32 *pDuplex)
{
    int32 ret;
    uint32 val1, val2;
    uint32 phy_port;

    if ((port<MIN_PORT_ID) || (port>=MAX_PORT_ID))
        return RT_ERR_PORT_ID;
    if (pSpeed == NULL)
        return RT_ERR_NULL_POINTER;
    if (pDuplex == NULL)
        return RT_ERR_NULL_POINTER;

    phy_port = port;
    if (port >= 52 && port <= 55)
    {
        phy_port = port - 4;
    }

    ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_SPD_STS_ADDR(phy_port), &val1);
    if (RT_ERR_OK != ret)
        return ret;

    ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_DUP_STS_ADDR(phy_port), &val2);
    if (RT_ERR_OK != ret)
        return ret;

    *pSpeed = (val1 &  RTL8390_MAC_LINK_SPD_STS_SPD_STS_MASK(phy_port) ) >>  RTL8390_MAC_LINK_SPD_STS_SPD_STS_OFFSET(phy_port);
    *pDuplex = (val2 &  RTL8390_MAC_LINK_DUP_STS_DUP_STS_MASK(phy_port) ) >>  RTL8390_MAC_LINK_DUP_STS_DUP_STS_OFFSET(phy_port);

    return RT_ERR_OK;
}

/* Function Name: 
 *		rtl8390_getAsicPortLinkStat
 * Description: 
 *		Get the specified port's link status.
 * Input:  
 *		port		- the specified port
 * Output: 
 *		
 *	      pLinkStat 	   - the link status of specified poty.
 * Return: 
 *		RT_ERR_OK		  - function is successed
 *		RT_ERR_FAILED	  - function is failed
 * Note: 
 *		  
 */
int32 rtl8390_getAsicPortLinkStat(int32 port, uint32 *pLinkStat)
{
    int32 ret;
    uint32 val;
    uint32 phy_port, link_media;

    if ((port<MIN_PORT_ID) || (port>=MAX_PORT_ID))
        return RT_ERR_PORT_ID;

    if (pLinkStat == NULL)
        return RT_ERR_NULL_POINTER;

    phy_port = port;
    if (port >= 52 && port <= 55)
    {
        phy_port = port - 4;
    }

    ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_STS_ADDR(phy_port), &val);
    ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_STS_ADDR(phy_port), &val);
    if (RT_ERR_OK != ret)
        return ret;

    *pLinkStat = (val &  RTL8390_MAC_LINK_STS_LINK_STS_MASK(phy_port) ) >>  RTL8390_MAC_LINK_STS_LINK_STS_OFFSET(phy_port);
    if (*pLinkStat)
    {
#if 1
        if (port >= 48 && port <= 49)
        {
            ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_MEDIA_STS_ADDR(phy_port), &val);
            link_media = (val &  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_MASK(phy_port) ) >>  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_OFFSET(phy_port);
            if (link_media == 1)
                (*pLinkStat) = 0;
        }
        if (port >= 50 && port <= 51)
        {
            ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_MEDIA_STS_ADDR(phy_port), &val);
            link_media = (val &  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_MASK(phy_port) ) >>  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_OFFSET(phy_port);
            if (link_media == 0)
                (*pLinkStat) = 0;
        }
#else
        if (port >= 48 && port <= 51)
        {
            ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_MEDIA_STS_ADDR(phy_port), &val);
            link_media = (val &  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_MASK(phy_port) ) >>  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_OFFSET(phy_port);
            if (link_media == 1)
                (*pLinkStat) = 0;
        }
        if (port >= 52 && port <= 55)
        {
            ret = swCtrl_led_getAsicReg(0xBB000000+RTL8390_MAC_LINK_MEDIA_STS_ADDR(phy_port), &val);
            link_media = (val &  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_MASK(phy_port) ) >>  RTL8390_MAC_LINK_MEDIA_STS_MEDIA_STS_OFFSET(phy_port);
            if (link_media == 0)
                (*pLinkStat) = 0;
        }
#endif
    }

    return RT_ERR_OK;
}


/* Function Name: 
 *		rtl8390_getAsicMIBPortCounter
 * Description: 
 *		Set bits value of a specified register.
 * Input:  
 *	  port		- the specified port.
 *	  mibIdx		- the specified MIB counter index.
 * Output: 
 *	  counter 	       - MIB counter value. 
 * Return: 
 *		RT_ERR_OK		  - function is successed
 * Note: 
 *		  
 */
int32 rtl8390_getAsicMIBPortCounter(int32 port, uint32 mibIdx, uint64* num)
{
    int32 ret;
    uint32 val;
    uint32 phy_port;
#if !defined(__FLASH_SRAM_ONLY__)
    uint64 msk;
#endif

    if ((port < MIN_PORT_ID) || (port >= MAX_PORT_ID))
        return  RT_ERR_PORT_ID;

    if (num == NULL)
        return RT_ERR_NULL_POINTER;

    phy_port = port;
    if (port >= 52 && port <= 55)
    {
        phy_port = port - 4;
    }

    {
    rtk_MIBCounterInfo_t mibInfo[] = {
          {0xBB000000+RTL8390_STAT_PORT_STANDARD_MIB_ADDR(phy_port)+0, 0xFFFFFFFF, 0, 64},
          {0xBB000000+RTL8390_STAT_PORT_STANDARD_MIB_ADDR(phy_port)+8, 0xFFFFFFFF, 0, 64},
          {0xBB000000+RTL8390_STAT_PORT_STANDARD_MIB_ADDR(phy_port)+96, 0xFFFFFFFF, 0, 32},
          {0xBB000000+RTL8390_STAT_PORT_STANDARD_MIB_ADDR(phy_port)+128, 0xFFFFFFFF, 0, 32},
          {0xBB000000+RTL8390_STAT_PORT_STANDARD_MIB_ADDR(phy_port)+104, 0xFFFFFFFF, 0, 32},
    };

    if (mibInfo[mibIdx].bitNum > REG_WIDTH)
    {
        /*64bit counter*/
        ret = swCtrl_led_getAsicReg(mibInfo[mibIdx].regAddr, &val);
        if(RT_ERR_OK != ret)
            return ret;

#ifdef __FLASH_SRAM_ONLY__
        *num = ((uint64)val & (uint64)0xFFFFFFFF) * 1024 * 1024 * 1024 * 4;
#else
        msk = ((uint64)1 << (mibInfo[mibIdx].bitNum - REG_WIDTH)) - 1;
        *num = ((uint64)val & msk) << REG_WIDTH;
#endif

        ret = swCtrl_led_getAsicReg(mibInfo[mibIdx].regAddr + 4, &val);
        if(RT_ERR_OK != ret)
            return ret;

        *num |= (val & mibInfo[mibIdx].regMsk) >> mibInfo[mibIdx].regOffset;
    }
    else
    {
        if ((mibInfo[mibIdx].regAddr & 0x4) == 0x4)
        {
            ret = swCtrl_led_getAsicReg(mibInfo[mibIdx].regAddr - 4, &val);
            if(RT_ERR_OK != ret)
                return ret;
        }

        ret = swCtrl_led_getAsicReg(mibInfo[mibIdx].regAddr, &val);
        if(RT_ERR_OK != ret)
            return ret;

        *num = (val & mibInfo[mibIdx].regMsk) >> mibInfo[mibIdx].regOffset;
    }
    }

    return RT_ERR_OK;
}
