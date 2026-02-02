/*
 * Copyright (C) 2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : RTK switch high-level API for RTL83XX
 * Feature : Here is a list of all functions and variables in this module.
 *
 * API source : Realtek_Unmanaged_Switch_API_V1.4.0_20200611
 */

#include <linux/version.h>
#include <linux/string.h>
#include <linux/seq_file.h>

#include <rtk_error.h>
#include <rtk_switch.h>
#include <rtk_api.h>

#ifdef __KERNEL__
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#define CONFIG_RTL_PROC_NEW	1
#endif
#endif

#ifdef CONFIG_OPENWRT_SDK
#include <linux/kernel.h>
#include "../common/smi.h"
#endif

#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
#include <net/rtl/rtl_acl.h>
#endif
#include <asm/uaccess.h>

/* ---------------------------------------------------------------------- */
/* definitions */

#define CONFIG_LAN_WAN_ISOLATION 1
#define CONFIG_RTK_REFINE_PORT_DUPLEX_MODE 1

#if defined(CONFIG_RTL_8197F)
#if defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT) || defined(CONFIG_RTL_8367SB_SUPPORT) || defined(CONFIG_RTL_8367S_SUPPORT)
#define CONFIG_RTL_ENABLE_EXT_SSC		1 	// for EMI
#elif defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#else // 8367RB
#define CONFIG_RTL_8367R_SUPPORT		1
#define CONFIG_RTL_ENABLE_EXT_SSC		1 	// for EMI
#endif
#endif

//#define RTL83XX_USE_ONE_LED_PER_PORT		1

#if defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT) || defined(CONFIG_RTL_8367SB_SUPPORT) || defined(CONFIG_RTL_8367S_SUPPORT)
#define EXT_PORT_FOR_HOST			EXT_PORT1
#define LED_GROUP_X					LED_GROUP_1

#elif defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT)
#define EXT_PORT_FOR_HOST           EXT_PORT0
#define LED_GROUP_X                 LED_GROUP_1

#else // 8367RB, 8367R_VB
#define EXT_PORT_FOR_HOST			EXT_PORT0
#define EXT_PORT_FOR_RGMII2			EXT_PORT1
#define LED_GROUP_X					LED_GROUP_2
#endif

#ifndef CONFIG_RTL_8197F
#define ENABLE_8367RB_RGMII2	1
#endif

#if defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#define TX_DELAY		0
#define RX_DELAY		6
#else
#define TX_DELAY		0
#define RX_DELAY		2
#ifdef ENABLE_8367RB_RGMII2
#define _2ND_TX_DELAY		1
#define _2ND_RX_DELAY		3
#endif
#endif

#if defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT) || defined(CONFIG_RTL_8367SB_SUPPORT) || defined(CONFIG_RTL_8367S_SUPPORT)
#define TX_DELAY_ENABLE_SSC		0
#define RX_DELAY_ENABLE_SSC		5
#elif defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#endif

#define WAN_VID			1
#define LAN_VID			2

#if defined(CONFIG_RTL_EXCHANGE_PORTMASK)
#if defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#define RTL83XX_WAN         1
#else
/* TODO: Use CONFIG_TP_ IMAGE wraps it*/
#define	RTL83XX_WAN			2		// WAN port is set to 83XX port 2	
//#define	RTL83XX_WAN			0 // WAN port is set to 83XX port 0
#endif
#else
#if defined(CONFIG_RTL_8366SC_SUPPORT) || defined(CONFIG_RTL_8363NB_SUPPORT) || defined(CONFIG_RTL_8365MB_SUPPORT) || defined(CONFIG_RTL_8364NB_SUPPORT)
#define	RTL83XX_WAN			3
#else
#define	RTL83XX_WAN			4
#endif
#endif

#define	RTL_WANPORT_MASK		(0x1 << RTL83XX_WAN)
#define RTL_LANPORT_MASK        (0xFF & (~RTL_WANPORT_MASK)) // may UTP_P0~UTP_P7

#define GATEWAY_MODE			0
#define BRIDGE_MODE				1

#define RTL83XX_WAN_PORT_BITMAP 		(1<<RTL83XX_WAN)
#define RTL83XX_LAN_PORT_BITMAP        (0xFF & (~RTL_WANPORT_MASK)) // may UTP_p0 ~ UTP_P7
#define RTL83XX_LAN_EFID				2
#define RTL83XX_LAN_FID				2


#define SW_API(x) do { \
    if ((ret = x) != RT_ERR_OK) { \
        printk("### Switch API %s failed, ret = %d\n", #x, ret); \
        goto fail; \
    } \
} while(0)

#define SW_API2(x) do { \
    if ((ret = x) != RT_ERR_OK) { \
        goto fail; \
    } \
} while(0)

/* ---------------------------------------------------------------------- */
/* function declaration */

/* ---------------------------------------------------------------------- */
/* variables */
unsigned long irq_flags;
extern int rtl865x_curOpMode;
rtk_uint32 vportmask, vutpportmask;
rtk_uint32 r8367_cpu_port = EXT_PORT_FOR_HOST; // logical cpu port
static int rtl8197d_op_mode = GATEWAY_MODE;
rtk_uint32 chip_type;
const char chip_name[6][10] =
{
    "8367C",
    "8370B",
    "8364B",
    "8363SC_VB",
    "8367D",
    "None"
};


/* ---------------------------------------------------------------------- */

#ifdef CONFIG_RTL_83XX_API_V1_4
int rtl8367c_smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData)
{
    return smi_read(mAddrs, rData);
}
int rtl8367c_smi_write(rtk_uint32 mAddrs, rtk_uint32 rData)
{
    return smi_write(mAddrs, rData);
}

rtk_api_ret_t rtl_stat_port_getserial(rtk_port_t port, rtk_stat_port_cntr_t *pPort_cntrs)
{
    return rtk_stat_port_getSerial(port, pPort_cntrs);
}

rtk_api_ret_t rtk_port_phyTestMode_set(rtk_port_t port, rtk_port_phy_test_mode_t mode)
{
    rtk_uint32          data, regData, i;
    rtk_api_ret_t       retVal;

    RTK_CHK_PORT_IS_UTP(port);

    if(mode >= PHY_TEST_MODE_END)
        return RT_ERR_INPUT;

    if (PHY_TEST_MODE_NORMAL != mode)
    {
        /* Other port should be Normal mode */
        RTK_SCAN_ALL_LOG_PORT(i)
        {
            if(rtk_switch_isUtpPort(i) == RT_ERR_OK)
            {
                if(i != port)
                {
                    if ((retVal = rtk_port_phyReg_get(rtk_switch_port_L2P_get(i), 9, &data)) != RT_ERR_OK)
                        return retVal;

                    if((data & 0xE000) != 0)
                        return RT_ERR_NOT_ALLOWED;
                }
            }
        }
    }

    if ((retVal = rtk_port_phyReg_get(rtk_switch_port_L2P_get(port), 9, &data)) != RT_ERR_OK)
        return retVal;

    data &= ~0xE000;
    data |= (mode << 13);
    if ((retVal = rtk_port_phyReg_set(rtk_switch_port_L2P_get(port), 9, data)) != RT_ERR_OK)
        return retVal;

    if (PHY_TEST_MODE_4 == mode)
    {
        if((retVal = rtl83xx_setAsicReg(0x13C2, 0x0249)) != RT_ERR_OK)
            return retVal;

        if((retVal = rtl83xx_getAsicReg(0x1300, &regData)) != RT_ERR_OK)
            return retVal;

        if( (regData == 0x0276) || (regData == 0x0597) )
        {
            if ((retVal = rtk_port_phyOCPReg_set(rtk_switch_port_L2P_get(port), 0xbcc2, 0xF4F4)) != RT_ERR_OK)
                return retVal;
        }

        if( (regData == 0x6367) )
        {
            if ((retVal = rtk_port_phyOCPReg_set(rtk_switch_port_L2P_get(port), 0xa436, 0x80c1)) != RT_ERR_OK)
                return retVal;

            if ((retVal = rtk_port_phyOCPReg_set(rtk_switch_port_L2P_get(port), 0xa438, 0xfe00)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

rtk_api_ret_t rtl_83xx_restartPHYNway(rtk_uint32 port)
{
	rtk_uint32 reg0 = 0;
#define RESTART_AUTONEGO   (1 << 9)

	/* port number validation */
	if (port > RTK_PHY_ID_MAX)
		return RT_ERR_PORT_ID;

	/* read current PHY reg 0 */
	if(rtk_port_phyReg_get(port, 0, &reg0 ) != RT_ERR_OK)
	{
		printk("read port %d reg0 failed\n", port);
		return RT_ERR_FAILED;
	}

	/* avoid link down again */
	if (reg0 & 0x0800)
	{
		//printk("read port %d reg0=0x%x, return\n", port, reg0);
		return RT_ERR_FAILED;
	}

	/* enable 'restart Nway' bit */
	reg0 |= RESTART_AUTONEGO;
	
	/* write PHY reg 0 */
	rtk_port_phyReg_set(port, 0, reg0);

	return RT_ERR_OK;
}
#endif

/* New API do not have these*/

/* Function Name:
 *      rtk_l2_flushType_set
 * Description:
 *      Flush L2 mac address by type in the specified device.
 * Input:
 *      type - flush type
 *      vid - VLAN id
 *      portOrTid - port id or trunk id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_PORT_ID      - Invalid port number.
 *      RT_ERR_VLAN_VID     - Invalid VID parameter.
 *      RT_ERR_INPUT        - Invalid input parameters.
 * Note:
 *      This function trigger flushing of per-port L2 learning.
 *      When flushing operaton completes, the corresponding bit will be clear.
 *      The flush type as following:
 *      - FLUSH_TYPE_BY_PORT        (physical port)
 *      - FLUSH_TYPE_BY_PORT_VID    (physical port + VID)
 */
rtk_api_ret_t rtk_l2_flushType_set(rtk_l2_flushType_t type, rtk_vlan_t vid, rtk_l2_flushItem_t portOrTid)
{
    rtk_api_ret_t retVal;
	rtk_l2_flushCfg_t flush_Config;
    memset(&flush_Config,0x00,sizeof(rtk_l2_flushCfg_t));
	
    if (type>=FLUSH_TYPE_END)
        return RT_ERR_INPUT;

    if (portOrTid > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    switch (type)
    {
	    case FLUSH_TYPE_BY_PORT:
	    	flush_Config.flushByPort=ENABLED;
			flush_Config.port=portOrTid;
	       	break;
	    case FLUSH_TYPE_BY_PORT_VID:
			flush_Config.flushByVid=ENABLED;
			flush_Config.vid=vid;
			flush_Config.flushByPort=ENABLED;
			flush_Config.port=portOrTid;
	        break;
	    default:
	        break;
    }

	retVal=rtk_l2_ucastAddr_flush(&flush_Config);

    return retVal;
}

/* Function Name:
 *      rtk_switch_maxPktLen_set
 * Description:
 *      Set the max packet length of the specific unit
 * Input:
 *      len - max packet length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - Failed
 *      RT_ERR_SMI          - SMI access error
 *      RT_ERR_INPUT 		- Invalid input parameters.
 * Note:
 *      The API can set max packet length of the specific unit to
 *      - MAXPKTLEN_1522B,
 *      - MAXPKTLEN_1536B,
 *      - MAXPKTLEN_1552B,
 *      - MAXPKTLEN_16000B.
 */
rtk_api_ret_t rtk_switch_maxPktLen_set(rtk_switch_maxPktLen_t len)
{
    rtk_api_ret_t retVal;
	rtk_uint32 port_id;

    if (len>=RTK_SWITCH_MAX_PKTLEN)
        return RT_ERR_INPUT;

    if ((retVal = rtk_switch_maxPktLenCfg_set(0,len)) != RT_ERR_OK) {
        return retVal;
    }
	for(port_id=0; port_id<RTK_MAX_NUM_OF_PORT; port_id++){
		if ((retVal = rtk_switch_portMaxPktLen_set(port_id,MAXPKTLEN_LINK_SPEED_GE,0)) != RT_ERR_OK)
        	return retVal;
	}

    return RT_ERR_OK;
}

rtk_api_ret_t rtk_phyPatch_set(rtk_uint32 opt)
{
    rtk_api_ret_t retVal;
    rtk_uint32 port;
    rtk_uint32 data;

    if( (opt != 0) && (opt != 1) )
        return RT_ERR_FAILED;

    for(port = 0; port <= 4; port++)
    {
        if (vutpportmask & (1 << port))
        {
            if( (retVal = rtk_port_phyOCPReg_get(port, 0xA42C, &data)) != RT_ERR_OK)
                return retVal;

            data |= 0x0010;
            if( (retVal = rtk_port_phyOCPReg_set(port, 0xA42C, data)) != RT_ERR_OK)
                return retVal;

            if(opt == 0)
            {
                if( (retVal = rtk_port_phyOCPReg_set(port, 0xBC02, 0x00D0)) != RT_ERR_OK)
                    return retVal;
            }
            else
            {
                if( (retVal = rtk_port_phyOCPReg_set(port, 0xBC02, 0x00F0)) != RT_ERR_OK)
                    return retVal;
            }

            if ((retVal = rtk_port_phyReg_get(port, PHY_CONTROL_REG, &data)) != RT_ERR_OK)
                return retVal;

            data |= (0x0001 << 9);
            if ((retVal = rtk_port_phyReg_set(port, PHY_CONTROL_REG, data)) != RT_ERR_OK)
                return retVal;
        }
    }

    return RT_ERR_OK;
}

/* END of New API do not have these*/

#ifdef ENABLE_8367RB_RGMII2
// for tr181
int rtk_rgmii_set(int enable)
{
    rtk_port_mac_ability_t mac_cfg;

    mac_cfg.forcemode = PORT_MAC_FORCE;
    mac_cfg.speed = PORT_SPEED_1000M;
    mac_cfg.duplex = PORT_FULL_DUPLEX;
    if (enable == TRUE)
        mac_cfg.link = PORT_LINKUP;
    else
        mac_cfg.link = PORT_LINKDOWN;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    rtk_port_macForceLinkExt_set(EXT_PORT_FOR_RGMII2, MODE_EXT_RGMII, &mac_cfg);

    return 0;
}
#endif

int RTL83XX_init(void)
{
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
    rtk_portmask_t portmask;
    unsigned int ret, i;

    // since we have set 8367RB GPIO reset pin to 0 and then to 1, so RTL83XX_REG_CHIP_RESET is no need
    //// do the whole chip reset in case the 8367 may be set in boot loader
    //rtl83xx_setAsicReg(RTL83XX_REG_CHIP_RESET, (1<<RTL83XX_CHIP_RST_OFFSET));
    //mdelay(1200);

    /* Initial Chip */
    SW_API2(rtk_switch_init());

    chip_type = rtk_switch_chipType_get();
    printk("Switch v1.4 API: switchChip= %s\n", chip_name[chip_type]);

    /* if no need: one fw.bin can run in both 8367RB-VC and 8367RB-VB, can add:
     *   #ifdef CONFIG_RTL_8367RB_VC_SUPPORT
     *     #define EXT_PORT_FOR_HOST			EXT_PORT1
     *   #endif
     * and remove the following code.
     */
    if (chip_type == CHIP_RTL8367D)
    {
        r8367_cpu_port = EXT_PORT1; // 8367RB-VC only has EXT_PORT1 (RGMII)
    }

    SW_API(rtk_switch_portMask_get(&portmask));
    vportmask = portmask.bits[0];
    SW_API(rtk_switch_utpPortMask_get(&portmask));
    vutpportmask = portmask.bits[0];

#if defined(CONFIG_LAN_WAN_ISOLATION) && defined(CONFIG_TP_IMAGE)
		portmask.bits[0] = (vutpportmask | BIT(r8367_cpu_port));
	#ifdef ENABLE_8367RB_RGMII2
		portmask.bits[0] |= BIT(EXT_PORT_FOR_RGMII2);
	#endif

		portmask.bits[0] &= ~(BIT(RTL83XX_WAN));
		for (i=0; i<=4; i++)
		{
			if(i != RTL83XX_WAN)
			{
				SW_API(rtk_port_isolation_set(i, &portmask));
			}
		}
		
		portmask.bits[0] = BIT(r8367_cpu_port) | (BIT(RTL83XX_WAN));
		SW_API(rtk_port_isolation_set(RTL83XX_WAN, &portmask));
#endif

    /* Enable LED Group 0&1 from P0 to P4 */
    portmask.bits[0] = vutpportmask;
#if !defined(CONFIG_RTL_8363NB_SUPPORT)
//    SW_API(rtk_led_enable_set(LED_GROUP_0, &portmask));
//    SW_API(rtk_led_enable_set(LED_GROUP_X, &portmask));
#endif

    /* note: rtk_port_rgmiiDelayExt_set should be called before rtk_port_macForceLinkExt_set
             to avoid the rxc glitch */
    SW_API(rtk_port_rgmiiDelayExt_set(r8367_cpu_port, TX_DELAY, RX_DELAY));

#if defined(CONFIG_RTL_ENABLE_EXT_SSC)
    if (rtk_port_macForceLinkExtSSC_set(r8367_cpu_port, 1) == RT_ERR_OK)
    {
        // the RGMII Tx/Rx delay will be effected when enable SSC, need to re-find them.
        SW_API(rtk_port_rgmiiDelayExt_set(r8367_cpu_port, TX_DELAY_ENABLE_SSC, RX_DELAY_ENABLE_SSC));
    }
#endif

    /* Set external interface 0 to RGMII with Force mode, 1000M, Full-duple, enable TX&RX pause*/
    mode = MODE_EXT_RGMII ;
    mac_cfg.forcemode = PORT_MAC_FORCE;
    mac_cfg.speed = PORT_SPEED_1000M;
    mac_cfg.duplex = PORT_FULL_DUPLEX;
    mac_cfg.link = PORT_LINKUP;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    SW_API(rtk_port_macForceLinkExt_set(r8367_cpu_port,mode,&mac_cfg));

#ifdef ENABLE_8367RB_RGMII2
    //rtk_port_rgmiiDelayExt_set(EXT_PORT_FOR_RGMII2, _2ND_TX_DELAY, _2ND_RX_DELAY);

    mode = MODE_EXT_RGMII ;
    mac_cfg.forcemode = PORT_MAC_FORCE;
    mac_cfg.speed = PORT_SPEED_1000M;
    mac_cfg.duplex = PORT_FULL_DUPLEX;
    mac_cfg.link = PORT_LINKUP;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    SW_API(rtk_port_macForceLinkExt_set(EXT_PORT_FOR_RGMII2,mode,&mac_cfg));
#endif

    /* set CPU port */
    SW_API(rtk_cpu_enable_set(ENABLED));
    SW_API(rtk_cpu_tagPort_set(r8367_cpu_port, CPU_INSERT_TO_NONE));

    // for LED setting
#ifdef RTL83XX_USE_ONE_LED_PER_PORT
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LINK_ACT));
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT));
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LINK_ACT));
#else
    /* demo board use 2 LEDs for each port */
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD1000ACT));
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_SPD10010ACT));
#endif
#ifdef CONFIG_TP_IMAGE	
    //set switch led off, led 0 1 2 -> port 2 3 4
    SW_API(rtk_led_modeForce_set(0, LED_GROUP_0, LED_FORCE_OFF));
    SW_API(rtk_led_modeForce_set(1, LED_GROUP_0, LED_FORCE_OFF));
    SW_API(rtk_led_modeForce_set(2, LED_GROUP_0, LED_FORCE_OFF));
#endif
#if defined(CONFIG_LAN_WAN_ISOLATION) && !defined(CONFIG_TP_IMAGE)
    portmask.bits[0] = (vutpportmask | BIT(r8367_cpu_port));
	#ifdef ENABLE_8367RB_RGMII2
    portmask.bits[0] |= BIT(EXT_PORT_FOR_RGMII2);
	#endif

    for (i=0; i<=4; i++)
        SW_API(rtk_port_isolation_set(i, &portmask));
#endif

    #if defined(CONFIG_RTK_REFINE_PORT_DUPLEX_MODE)
    SW_API(rtk_port_duplexAutoChange_set(ENABLED));
    #endif

    /* Enable auto down speed to 10M */
    SW_API(rtk_port_autoDownSpeed10M_set(ENABLED));

    if (chip_type != CHIP_RTL8367D)
    {
        /* 8367D has doubled the buffer number, the threshold default vale of 8367D
         * is larger than the adjusted value (below setting for 0x121f~0x1226) in 8367C
         */
        #if defined(CONFIG_RTL_8363NB_SUPPORT)
        rtl83xx_setAsicReg(0x121f, 0x01D0);
        rtl83xx_setAsicReg(0x1220, 0x01AC);
        rtl83xx_setAsicReg(0x1221, 0x019E);
        rtl83xx_setAsicReg(0x1222, 0x017A);
        rtl83xx_setAsicReg(0x1223, 0x01D0);
        rtl83xx_setAsicReg(0x1224, 0x01AC);
        rtl83xx_setAsicReg(0x1225, 0x019E);
        rtl83xx_setAsicReg(0x1226, 0x017A);
        #else
        // for 802.11ac logo 4.2.40 test (udp test item)
        rtl83xx_setAsicReg(0x121f, 0x01D6);
        rtl83xx_setAsicReg(0x1220, 0x01B8);
        rtl83xx_setAsicReg(0x1221, 0x01CC);
        rtl83xx_setAsicReg(0x1222, 0x01AE);
        rtl83xx_setAsicReg(0x1223, 0x0302);
        rtl83xx_setAsicReg(0x1224, 0x02E4);
        rtl83xx_setAsicReg(0x1225, 0x02D0);
        rtl83xx_setAsicReg(0x1226, 0x02A8);
        #endif

        #if !defined(CONFIG_RTL_8363NB_SUPPORT)
        /* Enhance the anti-jamming capability of the port,
           and enhance the compatibility of the mismatch resistance. */
        SW_API(rtk_phyPatch_set(0));
        #endif
    }

fail:
    return ret;
}

#include <rtl8367d_reg.h>
int rtk_83xx_short_ipg_set(unsigned int mode)
{
    if (mode == ENABLED)
    {
        // 8367RB-VB, EXT-PORT0
        rtl8367c_setAsicRegBit(RTL8367D_REG_PORT6_MISC_CFG,RTL8367D_PORT6_MISC_CFG_SMALL_TAG_IPG_OFFSET,1);
        // 8367RB-VC, EXT-PORT1
        rtl8367c_setAsicRegBit(RTL8367D_REG_PORT7_MISC_CFG,RTL8367D_PORT7_MISC_CFG_SMALL_TAG_IPG_OFFSET,1);
    }
    else
    {
        rtl8367c_setAsicRegBit(RTL8367D_REG_PORT6_MISC_CFG,RTL8367D_PORT6_MISC_CFG_SMALL_TAG_IPG_OFFSET,0);
        rtl8367c_setAsicRegBit(RTL8367D_REG_PORT7_MISC_CFG,RTL8367D_PORT7_MISC_CFG_SMALL_TAG_IPG_OFFSET,0);
    }
    return RT_ERR_OK;
}

int RTL83XX_init_switch_mode(void)
{
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
    rtk_portmask_t portmask;
    unsigned int ret;

    /* Initial Chip */
    SW_API2(rtk_switch_init());

    chip_type = rtk_switch_chipType_get();
    printk("Switch v1.4 API: switchChip= %s\n", chip_name[chip_type]);

    if (chip_type == CHIP_RTL8367D)
    {
        r8367_cpu_port = EXT_PORT1; // 8367RB-VC only has EXT_PORT1 (RGMII)
    }

    SW_API(rtk_switch_portMask_get(&portmask));
    vportmask = portmask.bits[0];
    SW_API(rtk_switch_utpPortMask_get(&portmask));
    vutpportmask = portmask.bits[0];

    /* Enable LED Group 0&1 from P0 to P4 */
    portmask.bits[0]=vutpportmask;
//    SW_API(rtk_led_enable_set(LED_GROUP_0, &portmask));
//    SW_API(rtk_led_enable_set(LED_GROUP_2, &portmask));

    /* Set TX delay to 0 and RX delay to 2 */
    SW_API(rtk_port_rgmiiDelayExt_set(r8367_cpu_port, TX_DELAY, RX_DELAY));

    /* Set external interface 0 to RGMII with Force mode, 1000M, Full-duple, enable TX&RX pause*/
    mode = MODE_EXT_RGMII ;
    mac_cfg.forcemode = PORT_MAC_FORCE;
    mac_cfg.speed = PORT_SPEED_1000M;
    mac_cfg.duplex = PORT_FULL_DUPLEX;
    mac_cfg.link = PORT_LINKUP;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    SW_API(rtk_port_macForceLinkExt_set(r8367_cpu_port,mode,&mac_cfg));

    /* set port 5 as CPU port */
    SW_API(rtk_cpu_enable_set(ENABLED));
    SW_API(rtk_cpu_tagPort_set(r8367_cpu_port, CPU_INSERT_TO_NONE));

    portmask.bits[0] = BIT(r8367_cpu_port) | 0x1F;
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNDA, &portmask));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNMC, &portmask));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_BC, &portmask));

    if (chip_type != CHIP_RTL8367D)
        SW_API(rtk_switch_maxPktLen_set(MAXPKTLEN_16000B));
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_LINK_ACT));
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_1, LED_CONFIG_LINK_ACT));
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_LINK_ACT));

fail:
    return ret;
}


struct _vlan_conf
{
    rtk_vlan_t 		vid;
    rtk_uint32		mbrmsk;
    rtk_uint32		untagmsk;
    rtk_fid_t			fid;
    rtk_pri_t 			priority;
};

#define _VID_END	(RTL83XX_VIDMAX+1)
#define _8367RB_RGMII2		EXT_PORT1

// please assign different fid for them
struct _vlan_conf vc_gateway[] =
{
#ifdef ENABLE_8367RB_RGMII2
    { 	LAN_VID,	 	(RTL_LANPORT_MASK | BIT(_8367RB_RGMII2)),   	(RTL_LANPORT_MASK | BIT(_8367RB_RGMII2)),	0, 0 },
#else
    { 	LAN_VID,	 	RTL_LANPORT_MASK,   	RTL_LANPORT_MASK,	0, 0 },
#endif
    {	WAN_VID,	RTL_WANPORT_MASK,   RTL_WANPORT_MASK,	1, 0},
    {	PASSTHRU_VLAN_ID,	(RTL_LANPORT_MASK|RTL_WANPORT_MASK),   (RTL_LANPORT_MASK|RTL_WANPORT_MASK), 0, 0},//for IPv6
    {	_VID_END,	0, 0, 0, 0}
};

struct _vlan_conf vc_bridge_svl[] =
{
    { 	LAN_VID,	 	(RTL_LANPORT_MASK | RTL_WANPORT_MASK),   	(RTL_LANPORT_MASK | RTL_WANPORT_MASK),	2, 0 },
    {	_VID_END,	0, 0, 0, 0}
};


int _vlan_setting(struct _vlan_conf vc[])
{
    int i, j, ret;
    rtk_vlan_cfg_t vlanCfg;

    for(i=0; vc[i].vid <= RTL83XX_VIDMAX; i++)
    {
        memset(&vlanCfg,0x00,sizeof(rtk_vlan_cfg_t));
        vlanCfg.mbr.bits[0] = (vc[i].mbrmsk & vutpportmask) | BIT(r8367_cpu_port);
        vlanCfg.untag.bits[0] = vc[i].untagmsk & vutpportmask;
        vlanCfg.fid_msti = vc[i].fid;
        SW_API(rtk_vlan_set(vc[i].vid, &vlanCfg));

        if(vc[i].vid == PASSTHRU_VLAN_ID)
            continue;

        /* set pvid*/
        for(j=0; j<5; j++)
        {
            if  ((1<<j)& (vc[i].mbrmsk))
                rtk_vlan_portPvid_set(j, vc[i].vid, vc[i].priority);
        }
    }

fail:
    return 0;
}

int RTL83XX_vlan_init(void)
{
    _vlan_setting(vc_gateway);

    rtl8197d_op_mode = GATEWAY_MODE;
    return 0;
}

int RTL83XX_vlan_reinit(int mode)
{
#if defined (CONFIG_RTL_IVL_SUPPORT)
    // when CONFIG_RTL_IVL_SUPPORT is defined, keep vc_gateway setting for gateway and bridge mode both

    rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, r8367_cpu_port);
#else

    if (mode==rtl8197d_op_mode) // no need tio do the re-initialization
        return 0;

    rtk_vlan_init();

    if (mode==GATEWAY_MODE)
        _vlan_setting(vc_gateway);

    else
        _vlan_setting(vc_bridge_svl);

#endif

    rtl8197d_op_mode = mode;
    return 0;
}

/* Port Isolation */
int rtl_port_isolation_RTL83XX_set(unsigned short port,unsigned int mask)
{
    int ret;
    rtk_portmask_t mbrmsk;

	mbrmsk.bits[0] = mask;

    SW_API(rtk_port_isolation_set(port, &mbrmsk));

fail:
    return 0;
}

#if defined(CONFIG_RTL_VLAN_8021Q) && (defined(CONFIG_RTL_8367R_SUPPORT) || defined(CONFIG_RTL_83XX_SUPPORT))
#if 0
int rtl_vlan_RTL8367R_set(unsigned short vid, unsigned int tagmask, unsigned int mask)
{
    rtk_portmask_t mbrmsk, untag;
    rtk_api_ret_t retVal;
    int i;
    unsigned untagmask;

    if(vid==WAN_VID || vid == LAN_VID)
        return 0;

    untagmask = mask&(~tagmask);

    mbrmsk.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);

    //panic_printk("untagmsk is 0x%x\n", (untagmask&RTL83XX_LAN_PORT_BITMAP));

    if(tagmask&RTL83XX_WAN_PORT_BITMAP)
        untag.bits[0] = 0;
    else
        untag.bits[0] = (BIT(r8367_cpu_port)|RTL83XX_WAN_PORT_BITMAP|(untagmask&(RTL83XX_LAN_PORT_BITMAP & vutpportmask)));

    retVal=rtk_vlan_set(vid, mbrmsk, untag, 0);

    return 0;
}
#else
int rtl_vlan_RTL83XX_set(unsigned short vid, unsigned int tagmask, unsigned int mask, unsigned int fid)
{
    int ret;
    rtk_vlan_cfg_t vlanCfg;

    unsigned untagmask;
    untagmask = mask&(~tagmask);

    memset(&vlanCfg,0x00,sizeof(rtk_vlan_cfg_t));

    if (mask == 0 && tagmask == 0)//clear
    {
        vlanCfg.mbr.bits[0] = mask;
        vlanCfg.untag.bits[0] = untagmask;
        vlanCfg.fid_msti = fid;
    }
    else
    {
        //mbrmsk.bits[0] = (mask);
        //panic_printk("%s %d vid = %u mbrmsk.bits[0]=%u untagmsk.bits[0]=%u\n", __FUNCTION__, __LINE__, vid, mbrmsk.bits[0], untagmsk.bits[0]);
        vlanCfg.mbr.bits[0] = (mask) |BIT(r8367_cpu_port);

        vlanCfg.mbr.bits[0] &= vportmask;

        vlanCfg.untag.bits[0] = untagmask;

        if(vlanCfg.untag.bits[0] & ((1<<6) | (1<<8)))
            vlanCfg.untag.bits[0] |= BIT(r8367_cpu_port);
        vlanCfg.untag.bits[0] &= vportmask;

        vlanCfg.fid_msti = fid;
    }

    SW_API(rtk_vlan_set(vid, &vlanCfg));

fail:
    return 0;
}

#endif

int rtl_83XX_vlan_get(unsigned int i, unsigned int *mbrmsk, unsigned int *untagmsk, unsigned int *fid)
{
    rtk_api_ret_t ret = 0;
    rtk_vlan_cfg_t vlan1;

    if (!mbrmsk || !untagmsk || !fid)
        return -1;

    memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
    ret = rtk_vlan_get(i, &vlan1);
    if (ret == RT_ERR_OK)
    {
        *mbrmsk = vlan1.mbr.bits[0];
        *untagmsk = vlan1.untag.bits[0];
        *fid = vlan1.fid_msti;
        return 0;
    }

    return -1;
}
#endif

#if defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_MULTI_PHY_VIR_WAN)
#include <net/rtl/phy_multi_wan_portmask.h>
#define	RTL83XX_WAN_PORTMASK  	wanport_mask
#define RTL83XX_LAN_PORTMASK   0x1f & (~RTL83XX_WAN_PORTMASK)

int RTL83XX_vlan_set(void)
{
    rtk_portmask_t mbrmsk;
    rtk_vlan_cfg_t vlanCfg;
    rtk_api_ret_t ret;
    int i;

    memset(&vlanCfg, 0x00, sizeof(rtk_vlan_cfg_t));
    for(i=0; i<4096; i++)
    {
        //if (i==WAN_VID ||i==LAN_VID)
#if defined(CONFIG_RTL_VLAN_8021Q) && !defined(CONFIG_RTL_MULTI_LAN_DEV)
        if (i==WAN_VID || i==LAN_VID ||(i==1)
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
                ||i==PASSTHRU_VLAN_ID
#endif
           )
#else
        if (i==WAN_VID ||i==LAN_VID || (i==3) || (i==4) || (i==5) || (i==1)
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
                ||i==PASSTHRU_VLAN_ID
#endif
           ) /* RTK VLAN */
#endif
        {
            vlanCfg.mbr.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
            vlanCfg.untag.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
        }
        else
        {
            vlanCfg.mbr.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
            vlanCfg.untag.bits[0] = 0;
        }
        SW_API(rtk_vlan_set(i, &vlanCfg)); //all vlan's fid is 0
    }

    /* set pvid :  wan:8   lan:9  */
    for(i=0; i<5; i++)
    {
#if defined (CONFIG_RTL_IVL_SUPPORT)
        if (BIT(i)& RTL83XX_WAN_PORTMASK)
            SW_API(rtk_vlan_portPvid_set(i, WAN_VID,0));
        else
            SW_API(rtk_vlan_portPvid_set(i, LAN_VID,0));
#else
        if(rtl865x_curOpMode==GATEWAY_MODE)
        {
            if (BIT(i)& RTL83XX_WAN_PORTMASK)
                SW_API(rtk_vlan_portPvid_set(i, WAN_VID,0));
            else
                SW_API(rtk_vlan_portPvid_set(i, LAN_VID,0));
        }
        else
        {
            SW_API(rtk_vlan_portPvid_set(i, LAN_VID,0));
        }
#endif
    }

    if (chip_type != CHIP_RTL8367D)
    {
        /* set wan port efid=1, other ports efid=2 */
        for(i=0; i<5; i++)
        {
#if defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_MULTI_PHY_VIR_WAN)
            SW_API(rtk_port_efid_set(i,RTL83XX_LAN_EFID));
#else
#if defined (CONFIG_RTL_IVL_SUPPORT)
            if (BIT(i)& RTL83XX_WAN_PORTMASK)
                SW_API(rtk_port_efid_set(i,1));
            else
                SW_API(rtk_port_efid_set(i,RTL83XX_LAN_EFID));
#else
            if(rtl865x_curOpMode==GATEWAY_MODE)
            {
                if (BIT(i)& RTL83XX_WAN_PORTMASK)
                    SW_API(rtk_port_efid_set(i,1));
                else
                    SW_API(rtk_port_efid_set(i,RTL83XX_LAN_EFID));
            }
            else
            {
                SW_API(rtk_port_efid_set(i,RTL83XX_LAN_EFID));
            }
#endif
#endif
        }
    }
	else
	{
		 for(i=0; i<5; i++)
         {
#if defined(CONFIG_RTL_8021Q_VLAN_SUPPORT_MULTI_PHY_VIR_WAN)
            SW_API(rtk_vlan_portFid_set(i,1,RTL83XX_LAN_FID));
#else
#if defined (CONFIG_RTL_IVL_SUPPORT)
            if (BIT(i)& RTL83XX_WAN_PORTMASK)
                SW_API(rtk_vlan_portFid_set(i,1,1));
            else
                SW_API(rtk_vlan_portFid_set(i,1,RTL83XX_LAN_FID));
#else
            if(rtl865x_curOpMode==GATEWAY_MODE)
            {
                if (BIT(i)& RTL83XX_WAN_PORTMASK)
                    SW_API(rtk_vlan_portFid_set(i,1,1));
                else
                    SW_API(rtk_vlan_portFid_set(i,1,RTL83XX_LAN_FID));
            }
            else
            {
                SW_API(rtk_vlan_portFid_set(i,1,RTL83XX_LAN_FID));
            }
#endif
#endif
        }
		
	}

    // suggested by HM-Chung
#if !defined (CONFIG_RTL_8021Q_VLAN_SUPPORT_MULTI_PHY_VIR_WAN)
#if defined (CONFIG_RTL_IVL_SUPPORT)
    for (i=0; i<5; i++)
    {
        if (BIT(i)& RTL83XX_WAN_PORTMASK)
            mbrmsk.bits[0] = BIT(r8367_cpu_port);
        else
            mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port)) & ~RTL83XX_WAN_PORTMASK;

        SW_API(rtk_port_isolation_set(i, &mbrmsk));
    }
#else
    for (i=0; i<5; i++)
    {
        if (rtl865x_curOpMode == GATEWAY_MODE)
        {
            if (BIT(i)& RTL83XX_WAN_PORTMASK)
                mbrmsk.bits[0] = BIT(r8367_cpu_port);
            else
                mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port)) & ~RTL83XX_WAN_PORTMASK;
        }
        else
            mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port));

        SW_API(rtk_port_isolation_set(i, &mbrmsk));
    }
#endif
#endif

#if defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
    for (i=0; i<5; i++)
    {
        rtk_l2_limitLearningCnt_set(i, RTL83XX_MAX_LIMITED_L2ENTRY_NUM);
        rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, i);
        rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, LAN_VID, i);
    }
#else
#if defined (CONFIG_RTL_IVL_SUPPORT)
    for (i=0; i<5; i++)
    {
        if(BIT(i)& RTL83XX_WAN_PORTMASK)
        {
            // no matter Gateway or Bridge mode, always disable wan port L2 learning
            rtk_l2_limitLearningCnt_set(i, 0);
            rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, i);
        }
    }
#else
    if (rtl865x_curOpMode == GATEWAY_MODE)
    {
        for (i=0; i<5; i++)
        {
            if(BIT(i)& RTL83XX_WAN_PORTMASK)
            {
                rtk_l2_limitLearningCnt_set(i, 0);
                rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, i);
                rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, LAN_VID, i);
            }
        }
    }
    else
    {
        for (i=0; i<5; i++)
        {
            if(BIT(i)& RTL83XX_WAN_PORTMASK)
            {
                rtk_l2_limitLearningCnt_set(i, RTL83XX_MAX_LIMITED_L2ENTRY_NUM);
                rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, i);
                rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, LAN_VID, i);
            }
        }
    }
#endif
#endif

    /* disable cpu port's mac addr learning ability */
    rtk_l2_limitLearningCnt_set(r8367_cpu_port,0);

    /* disable unknown unicast/mcast/bcast flooding between LAN ports */
    mbrmsk.bits[0] = BIT(r8367_cpu_port);
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNDA, &mbrmsk));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNMC, &mbrmsk));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_BC, &mbrmsk));

fail:
    return ret;
}
#else
int RTL83XX_vlan_set(void)
{
    rtk_portmask_t mbrmsk;
    rtk_vlan_cfg_t vlanCfg;
    rtk_api_ret_t ret;
    int i;

    memset(&vlanCfg, 0x00, sizeof(rtk_vlan_cfg_t));
    for(i=0; i<4096; i++)
    {

#if defined(CONFIG_RTL_VLAN_8021Q) && !defined(CONFIG_RTL_MULTI_LAN_DEV)
        if (i==WAN_VID || i==LAN_VID ||(i==1)
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
                ||i==PASSTHRU_VLAN_ID
#endif
           )
#else
        if (i==WAN_VID ||i==LAN_VID || (i==3) || (i==4) || (i==5) || (i==1)
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
                ||i==PASSTHRU_VLAN_ID
#endif
           ) /* RTK VLAN */
#endif
        {
            vlanCfg.mbr.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
            vlanCfg.untag.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
        }
        else
        {
            vlanCfg.mbr.bits[0] = (BIT(r8367_cpu_port)| vutpportmask);
            vlanCfg.untag.bits[0] = 0;
        }
        SW_API(rtk_vlan_set(i, &vlanCfg)); //all vlan's fid is 0
    }

    /* set pvid :  wan:8   lan:9  */
    for(i=0; i<5; i++)
    {
#if defined (CONFIG_RTL_IVL_SUPPORT)
        if (i == RTL83XX_WAN)
            SW_API(rtk_vlan_portPvid_set(i, WAN_VID,0));
        else
            SW_API(rtk_vlan_portPvid_set(i, LAN_VID,0));
#else
        if(rtl865x_curOpMode==GATEWAY_MODE)
        {
            if (i == RTL83XX_WAN)
                SW_API(rtk_vlan_portPvid_set(i, WAN_VID,0));
            else
                SW_API(rtk_vlan_portPvid_set(i, LAN_VID,0));
        }
        else
        {
            SW_API(rtk_vlan_portPvid_set(i, LAN_VID,0));
        }
#endif
    }

    if (chip_type != CHIP_RTL8367D)
    {
        /* set wan port efid=1, other ports efid=2 */
        for(i=0; i<5; i++)
        {
#if defined (CONFIG_RTL_IVL_SUPPORT)
            if (i == RTL83XX_WAN)
                SW_API(rtk_port_efid_set(i,1));
            else
                SW_API(rtk_port_efid_set(i,RTL83XX_LAN_EFID));
#else
            if(rtl865x_curOpMode==GATEWAY_MODE)
            {
                if (i == RTL83XX_WAN)
                    SW_API(rtk_port_efid_set(i,1));
                else
                    SW_API(rtk_port_efid_set(i,RTL83XX_LAN_EFID));
            }
            else
            {
                SW_API(rtk_port_efid_set(i,RTL83XX_LAN_EFID));
            }
#endif
        }
    }
	else
	{
		 for(i=0; i<5; i++)
         {

#if defined (CONFIG_RTL_IVL_SUPPORT)
            if (i == RTL83XX_WAN)
                SW_API(rtk_vlan_portFid_set(i,1,1));
            else
                SW_API(rtk_vlan_portFid_set(i,1,RTL83XX_LAN_FID));
#else
            if(rtl865x_curOpMode==GATEWAY_MODE)
            {
                if (i == RTL83XX_WAN)
                    SW_API(rtk_vlan_portFid_set(i,1,1));
                else
                    SW_API(rtk_vlan_portFid_set(i,1,RTL83XX_LAN_FID));
            }
            else
            {
                SW_API(rtk_vlan_portFid_set(i,1,RTL83XX_LAN_FID));
            }
#endif
        }
		
	}

    // suggested by HM-Chung
    for (i=0; i<5; i++)
    {
#if defined (CONFIG_RTL_IVL_SUPPORT)
        if (i == RTL83XX_WAN)
            mbrmsk.bits[0] = BIT(r8367_cpu_port);
        else
            mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port)) & ~BIT(RTL83XX_WAN);

        SW_API(rtk_port_isolation_set(i, &mbrmsk));
#else
        if (rtl865x_curOpMode == GATEWAY_MODE)
        {
            if (i == RTL83XX_WAN)
                mbrmsk.bits[0] = BIT(r8367_cpu_port);
            else
                mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port)) & ~BIT(RTL83XX_WAN);
        }
        else
            mbrmsk.bits[0] = (vutpportmask | BIT(r8367_cpu_port));

        SW_API(rtk_port_isolation_set(i, &mbrmsk));
#endif
    }

#if defined (CONFIG_RTL_IVL_SUPPORT)
    //if (rtl865x_curOpMode != GATEWAY_MODE)
    {
        // no matter Gateway or Bridge mode, always disable wan port L2 learning
        rtk_l2_limitLearningCnt_set(RTL83XX_WAN, 0);
        rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, RTL83XX_WAN);
    }
#else
    if (rtl865x_curOpMode == GATEWAY_MODE)
    {
        rtk_l2_limitLearningCnt_set(RTL83XX_WAN, 0);
        rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, RTL83XX_WAN);
        rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, LAN_VID, RTL83XX_WAN);
    }
    else
    {
        rtk_l2_limitLearningCnt_set(RTL83XX_WAN, RTL83XX_MAX_LIMITED_L2ENTRY_NUM);
        rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, WAN_VID, RTL83XX_WAN);
        rtk_l2_flushType_set(FLUSH_TYPE_BY_PORT, LAN_VID, RTL83XX_WAN);
    }
#endif

    /* disable cpu port's mac addr learning ability */
    rtk_l2_limitLearningCnt_set(r8367_cpu_port,0);

    /* disable unknown unicast/mcast/bcast flooding between LAN ports */
    mbrmsk.bits[0] = BIT(r8367_cpu_port);
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNDA, &mbrmsk));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNMC, &mbrmsk));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_BC, &mbrmsk));

fail:
    return ret;
}
#endif

void RTL83XX_cpu_tag(int enable)
{
    rtk_cpu_tagPort_set(r8367_cpu_port,CPU_INSERT_TO_ALL);
	rtk_cpu_tagLength_set(CPU_LEN_4BYTES);
	rtk_cpu_acceptLength_set(CPU_RX_64BYTES);
    rtk_cpu_enable_set(enable);
}

void set_83XX_L2(unsigned int *mac, int intf_wan, int is_static)
{
    rtk_mac_t Mac;
    rtk_l2_ucastAddr_t L2_data;

    memset(&L2_data, 0, sizeof(rtk_l2_ucastAddr_t));

    L2_data.efid= (intf_wan)? 1 : 2;
    L2_data.port=r8367_cpu_port;
    L2_data.is_static=is_static;
    L2_data.fid = (intf_wan)? 1 : RTL83XX_LAN_FID;

    memcpy(&Mac.octet[0], mac, 6);
    rtk_l2_addr_add(&Mac,  &L2_data);
}

void del_83XX_L2(rtk_mac_t *pMac)
{
    rtk_l2_ucastAddr_t L2_data;
	int i;
	
    memset(&L2_data, 0, sizeof(rtk_l2_ucastAddr_t));
    if (chip_type != CHIP_RTL8367D)
	{
		L2_data.fid = 0;
		L2_data.efid = RTL83XX_LAN_EFID;
		if (rtk_l2_addr_get(pMac, &L2_data) == RT_ERR_OK)
			rtk_l2_addr_del(pMac, &L2_data);

    }
	else
	{
		L2_data.efid = 0;	
		for(i = 0; i <= 3 ; i++)
		{
			L2_data.fid = i;
			if (rtk_l2_addr_get(pMac, &L2_data) == RT_ERR_OK)
				rtk_l2_addr_del(pMac, &L2_data);
		}
	}

    return;
}

#if 0
void get_all_L2(void)
{
    int i, ret;
    rtk_l2_addr_table_t p;

    for (i=1; i<=RTK_MAX_NUM_OF_LEARN_LIMIT; i++)
    {
        p.index = i;
        ret = rtk_l2_entry_get(&p);
        if (ret == RT_ERR_OK)
        {
            printk(" [%d] mac: %02x:%02x:%02x:%02x:%02x:%02x, portmask: 0x%x, age: %d, fid: %d\n", i,
                   p.mac.octet[0],p.mac.octet[1],p.mac.octet[2],p.mac.octet[3],p.mac.octet[4],p.mac.octet[5],
                   p.portmask, p.age, p.fid);
        }
    }
    return;
}
#endif

enum
{
    PORT_DOWN=0,
    HALF_DUPLEX_10M,
    HALF_DUPLEX_100M,
    HALF_DUPLEX_1000M,
    DUPLEX_10M,
    DUPLEX_100M,
    DUPLEX_1000M,
    PORT_AUTO,
    PORT_UP,
    AN_10M,
    AN_100M,
    AN_AUTO
};

rtk_api_ret_t set_83XX_speed_mode(int port, int mode)
{
    rtk_port_phy_ability_t phyAbility;

    memset(&phyAbility, 0, sizeof(rtk_port_phy_ability_t));

    /* Flow Control Off */
    phyAbility.FC = 0;
    phyAbility.AsyFC = 0;
    phyAbility.AutoNegotiation = 1;

    if (mode == HALF_DUPLEX_10M) //10M half
    {
        phyAbility.Half_10 = 1;
    }
    else if (mode == DUPLEX_10M)	//10M full
    {
        phyAbility.Full_10 = 1;
    }
    else if (mode == HALF_DUPLEX_100M) // 100M half
    {
        phyAbility.Half_100 = 1;
    }
    else if (mode == DUPLEX_100M) // 100M full
    {
        phyAbility.Full_100 = 1;
    }
    else if (mode == DUPLEX_1000M) // 1000M
    {
        phyAbility.Full_1000 = 1;
    }
    else if (mode == AN_10M)
    {
        phyAbility.Half_10 = 1;
        phyAbility.Full_10 = 1;
    }
    else if (mode == AN_100M)
    {
        phyAbility.Half_100 = 1;
        phyAbility.Full_100 = 1;
    }
    else
    {
        phyAbility.Half_10 = 1;
        phyAbility.Full_10 = 1;
        phyAbility.Half_100 = 1;
        phyAbility.Full_100 = 1;
        phyAbility.Full_1000 = 1;
    }

    return (rtk_port_phyAutoNegoAbility_set(port, &phyAbility));
}

void rtl83XX_reset(void)
{
    rtl83xx_setAsicReg(RTL83XX_REG_CHIP_RESET, (1<<RTL83XX_CHIP_RST_OFFSET));
    return;
}

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
int rtl8367_setProtocolBasedVLAN_ipv6PassThru(rtk_vlan_proto_type_t proto_type,rtk_vlan_t cvid, int cmdFlag, int portmask)
{
    rtk_port_t port;
    int ret=RT_ERR_OK;
    int memberport_flag;
    rtk_vlan_protoAndPortInfo_t info;
    info.proto_type=proto_type;
    info.frame_type=FRAME_TYPE_ETHERNET;
    info.cvid=cvid;
    info.cpri=0;

    if(cmdFlag==TRUE)
    {
        //printk("ADD[%s]:[%d].\n",__FUNCTION__,__LINE__);
        /*add */
        for(port=0; port<5; port++)
        {

            memberport_flag = (1<<port) & portmask;

            if(memberport_flag && proto_type == 0x86DD)
            {
                //close bridge lan port ipv6 passthru
                printk("%s:%d rtl83xx port:%d close hw ipv6 passthru\n",__FUNCTION__, __LINE__, port);
            }
            else
            {
                ret=rtk_vlan_protoAndPortBasedVlan_add(port, &info);
            }

        }
    }
    else
    {
        //printk("DEL[%s]:[%d].\n",__FUNCTION__,__LINE__);
        /*delete */
        for(port=0; port<5; port++)
        {

            ret=rtk_vlan_protoAndPortBasedVlan_del(port,  proto_type, FRAME_TYPE_ETHERNET);
        }
    }
    return ret;
}

int rtl8367_setProtocolBasedVLAN(rtk_vlan_proto_type_t proto_type,rtk_vlan_t cvid, int cmdFlag)
{
    rtk_port_t port;
    int ret;
    rtk_vlan_protoAndPortInfo_t info;
    info.proto_type=proto_type;
    info.frame_type=FRAME_TYPE_ETHERNET;
    info.cvid=cvid;
    info.cpri=0;

    if(cmdFlag==TRUE)
    {
        //printk("ADD[%s]:[%d].\n",__FUNCTION__,__LINE__);
        /*add */
        for(port=0; port<5; port++)
        {

            ret=rtk_vlan_protoAndPortBasedVlan_add(port, &info);

        }
    }
    else
    {
        //printk("DEL[%s]:[%d].\n",__FUNCTION__,__LINE__);
        /*delete */
        for(port=0; port<5; port++)
        {

            ret=rtk_vlan_protoAndPortBasedVlan_del(port,  proto_type, FRAME_TYPE_ETHERNET);
        }
    }
    return ret;
}
#endif

#if defined(CONFIG_RTK_VLAN_SUPPORT) || defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_OPENWRT_SDK) || defined(CONFIG_RTL_HW_VLAN_SUPPORT)
int rtl865x_enableRtl83xxToCpuAcl(void)
{
    int retVal;
    rtk_filter_field_t	filter_field[2];
    rtk_filter_cfg_t	cfg;
    rtk_filter_action_t	act;
    rtk_filter_number_t	ruleNum = 0;

    memset(filter_field, 0, 2*sizeof(rtk_filter_field_t));
    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));
    filter_field[0].fieldType  = FILTER_FIELD_DMAC;
    if ((retVal = rtk_filter_igrAcl_field_add(&cfg,	&filter_field[0])) != RT_ERR_OK)
        return retVal;

    /*add all ports to active ports*/
    cfg.activeport.value.bits[0] = vutpportmask;
    cfg.activeport.mask.bits[0] = vportmask;
    cfg.invert = FALSE;
    act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;
    if ((retVal = rtk_filter_igrAcl_cfg_add(0, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return retVal;
    return RT_ERR_OK;
}

int rtl865x_disableRtl83xxToCpuAcl(void)
{
    return rtk_filter_igrAcl_cfg_del(0);
}
#endif

#if defined(CONFIG_RTK_REFINE_PORT_DUPLEX_MODE)
#define MIB_STATE_FRAG_VAL_UPDATE_BASE		0x13A0
#define MIB_STATE_FRAG_CTL_UPDATE_BASE	0x13A4
int rtk_refinePortDuplexMode(void)
{
    rtk_stat_counter_t stateFragCounters = 0;
    int port;

    for(port=0; port<4; port++)
    {
        rtk_stat_port_get(port, STAT_EtherStatsFragments, &stateFragCounters);
        //if(port == 1)
        //printk("stateFragCounters is %d\n", (unsigned short)stateFragCounters);
        rtl83xx_setAsicReg(MIB_STATE_FRAG_VAL_UPDATE_BASE+port, (unsigned short)stateFragCounters);
        rtl83xx_setAsicReg(MIB_STATE_FRAG_CTL_UPDATE_BASE+port, 0x1);
    }
    return RT_ERR_OK;
}
#endif

#if 1//defined(CONFIG_RTL_HW_VLAN_SUPPORT)
#if defined(CONFIG_RTL_PROC_NEW)
int rtl_8367_chip_type_read(struct seq_file *s, void *v)
{
    rtk_uint32 chip_type;;

    chip_type = rtk_switch_chipType_get();
 	
    seq_printf(s, "switchChip = %s\n", chip_name[chip_type]);

    return 0;
}

int rtl_8367r_vlan_read(struct seq_file *s, void *v)
{
    int  i = 0, ret = 0;
    unsigned int pvid = 0, priority = 0, l2_count = 0;
    rtk_vlan_cfg_t vlan1;
    rtk_l2_addr_table_t l2_entry;
    int logic_port = 0;
    rtk_data_t efid = 0;
    rtk_portmask_t portmsk;
    rtk_stp_state_t stp_state = 0;
	rtk_fid_t fid = 0;
	rtk_enable_t enable = 0;

    seq_printf(s, "%s\n", "vlan:");
    for (i=1; i <= 1000; i++)
    {
        if(i!=1 && i!=2 && i!=8 && i!=9 && i%100!=0)
            continue;

        memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));

        if ((rtk_vlan_get(i, &vlan1)== 0) && (vlan1.mbr.bits[0] != 0))
            seq_printf(s, "vid %d Mbrmsk 0x%x Untagmsk 0x%x fid %u\n", i, vlan1.mbr.bits[0], vlan1.untag.bits[0], vlan1.fid_msti);
    }

    seq_printf(s, "\n%s\n", "pvid:");
    for(i=0; i<8; i++)
    {

        logic_port = rtk_switch_port_P2L_get(i);
        if(logic_port == UNDEFINE_PORT)
        {
            continue;
        }

        pvid = priority = 0;
        if (rtk_vlan_portPvid_get(logic_port, &pvid, &priority) == 0)
            seq_printf(s, "port %d pvid %u pri %u\n", i, pvid, priority);
    }

    if(chip_type != CHIP_RTL8367D)
	{
		seq_printf(s,"\n%s\n", "efid:");
		for(i=0; i<8; i++)
		{
		    efid = 0;
		    logic_port = rtk_switch_port_P2L_get(i);
		    if(logic_port == UNDEFINE_PORT)
		    {
		        continue;
		    }
		    if (rtk_port_efid_get(logic_port, &efid)==0)
		        seq_printf(s,"port %d efid %u\n", i, efid);
		}
	}
	else
	{
		seq_printf(s,"\n%s\n", "fid:");
		for(i=0; i<8; i++)
		{
			fid = 0;
			logic_port = rtk_switch_port_P2L_get(i);
			if(logic_port == UNDEFINE_PORT)
	        {
	            continue;
	        }
			if (rtk_vlan_portFid_get(logic_port, &enable, &fid)==0)
				seq_printf(s,"port %d enable %d fid %u\n", i, enable, fid);
		}
	}	

    seq_printf(s,"\n%s\n", "port isolation:");
    for(i=0; i<8; i++)
    {
        logic_port = rtk_switch_port_P2L_get(i);
        if(logic_port == UNDEFINE_PORT)
        {
            continue;
        }
        memset(&portmsk, 0x00, sizeof(portmsk));
        if (rtk_port_isolation_get(logic_port, &portmsk)==0)
            seq_printf(s,"port %d portmask 0x%x \n", i, portmsk.bits[0]);
    }

    seq_printf(s, "\n%s\n", "port stp_state:");
    for(i=0; i<8; i++)
    {
        rtk_stp_mstpState_get(0, i, &stp_state);
        seq_printf(s, "port%d  stp_state %d   \n", i, stp_state);
    }

    seq_printf(s, "\n%s\n", "l2:");
    /*Get All Lookup Table and Print the valid entry*/

    for (i=1; i<=RTL83XX_MAX_LIMITED_L2ENTRY_NUM; i++)
    {
        memset(&l2_entry,0,sizeof(rtk_l2_addr_table_t));
        l2_entry.index = i;
        ret = rtk_l2_entry_get(&l2_entry);
        if (ret==RT_ERR_OK)
        {
            if(l2_entry.is_ipmul)
            {

                seq_printf(s, "\r\nIndex SourceIP DestinationIP MemberPort State\n");
                seq_printf(s, "%4d ", l2_entry.index);
                seq_printf(s,"%0x ",(l2_entry.sip));
                seq_printf(s,"%0x ",(l2_entry.dip));
                seq_printf(s,"%-8x ",l2_entry.portmask.bits[0]);
                seq_printf(s,"%s \n",(l2_entry.is_static? "Static" : "Auto"));
            }
            else if(l2_entry.mac.octet[0]&0x01)
            {
                seq_printf(s,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                           l2_entry.index,
                           l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                           l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                           l2_entry.portmask.bits[0], l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                           (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                           l2_entry.age);
            }
            else if((l2_entry.age!=0)||(l2_entry.is_static==1))
            {
                seq_printf(s,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                           l2_entry.index,l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                           l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                           l2_entry.portmask.bits[0], l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                           (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                           l2_entry.age);
            }

			l2_count++;
        }
    }
    seq_printf(s, "l2 entry count: %d  \n", l2_count);

    return 0;
}

#else
int rtl_8367r_vlan_read( char *page, char **start, off_t off, int count, int *eof, void *data )
{
    int len = 0, i = 0, ret = 0;
    unsigned int pvid = 0, priority = 0, fid = 0;
    rtk_portmask_t Mbrmsk = {0}, Untagmsk= {0};

    len = sprintf(page, "%s\n", "vlan:");
    for (i=0; i < 4096; i++)
    {
        memset(&Mbrmsk, 0x00, sizeof(Mbrmsk));
        memset(&Untagmsk, 0x00, sizeof(Untagmsk));
        fid =0;
        if ((rtk_vlan_get(i, &Mbrmsk, &Untagmsk, &fid)== 0) && (Mbrmsk.bits[0] != 0))
            len += sprintf(page+len, "vid %d Mbrmsk 0x%x Untagmsk 0x%x fid %u\n", i, Mbrmsk.bits[0], Untagmsk.bits[0], fid);
    }

    len += sprintf(page+len, "\n%s\n", "pvid:");
    for(i=0; i<8; i++)
    {
        pvid = priority = 0;
        if (rtk_vlan_portPvid_get(i, &pvid, &priority)==0)
            len += sprintf(page+len, "port %d pvid %u pri %u\n", i, pvid, priority);
    }


    len += sprintf(page+len, "\n%s\n", "l2:");
    /*Get All Lookup Table and Print the valid entry*/
    rtk_l2_addr_table_t l2_entry;
    for (i=1; i<=RTL83XX_MAX_LIMITED_L2ENTRY_NUM; i++)
    {
        memset(&l2_entry,0,sizeof(rtk_l2_addr_table_t));
        l2_entry.index = i;
        ret = rtk_l2_entry_get(&l2_entry);
        if (ret==RT_ERR_OK)
        {
            if(l2_entry.is_ipmul)
            {

                len += sprintf(page+len, "\r\nIndex SourceIP DestinationIP MemberPort State\n");
                len += sprintf(page+len, "%4d ", l2_entry.index);
                len += sprintf(page+len,"%0x ",(l2_entry.sip));
                len += sprintf(page+len,"%0x ",(l2_entry.dip));
                len += sprintf(page+len,"%-8x ",l2_entry.portmask);
                len += sprintf(page+len,"%s \n",(l2_entry.is_static? "Static" : "Auto"));
            }
            else if(l2_entry.mac.octet[0]&0x01)
            {
                len += sprintf(page+len,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                               l2_entry.index,
                               l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                               l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                               l2_entry.portmask, l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                               (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                               l2_entry.age);
            }
            else if((l2_entry.age!=0)||(l2_entry.is_static==1))
            {
                len += sprintf(page+len,"%4d %2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X %-8x %-4d %-4s %-5s %s %d\n",
                               l2_entry.index,l2_entry.mac.octet[0],l2_entry.mac.octet[1],l2_entry.mac.octet[2],
                               l2_entry.mac.octet[3],l2_entry.mac.octet[4],l2_entry.mac.octet[5],
                               l2_entry.portmask, l2_entry.fid, (l2_entry.auth ? "Auth" : "x"),
                               (l2_entry.sa_block? "Block" : "x"), (l2_entry.is_static? "Static" : "Auto"),
                               l2_entry.age);
            }
        }
    }

    if (len <= off+count) *eof = 1;
    *start = page + off;
    len -= off;
    if (len>count)
        len = count;
    if (len<0)
        len = 0;

    return len;
}
#endif
#endif

#if defined(CONFIG_RTL_DNS_TRAP)
int rtl_83xx_add_acl_for_dns(unsigned int acl_idx)
{
#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
    rtl83xx_acl_rule_t aclRule;
    int retVal;

    memset(&aclRule, 0, sizeof(rtl83xx_acl_rule_t));
    aclRule.member = vutpportmask;
    aclRule.prio = RTL83XX_ACL_PRIO_DNS_TO_CPU;
    aclRule.filter.protocal = 0x11;
    aclRule.filter.protocal_mask = 0xFF;
    aclRule.filter.dport = 53;
    aclRule.filter.dport_mask = 0xFFFF;
    aclRule.filter.dport_type = FILTER_MASK;

    aclRule.action.act_type = RTL83XX_ACL_TRAP_CPU;
    rtl83xx_addAclRule(&aclRule);
	
#if defined(CONFIG_IPV6)
	memset(&aclRule, 0, sizeof(rtl83xx_acl_rule_t));
	aclRule.member = vutpportmask;
	aclRule.prio = RTL83XX_ACL_PRIO_DNS_TO_CPU;
	aclRule.filter.protocal6 = 0x11;
	aclRule.filter.protocal6_mask = 0xFF;
	aclRule.filter.dport = 53;
	aclRule.filter.dport_mask = 0xffff;
	aclRule.filter.dport_type = FILTER_MASK;
		
	aclRule.action.act_type = RTL83XX_ACL_TRAP_CPU;
	rtl83xx_addAclRule(&aclRule);
#endif
	retVal = rtl83xx_syncAclTblToAsic();
    return retVal;
	
#else
    int retVal; //, acl_idx=0;
    rtk_filter_field_t	filter_field[2];
    rtk_filter_cfg_t	cfg;
    rtk_filter_action_t	act;
    rtk_filter_number_t	ruleNum = 0;

    /* disable cpu port's mac addr learning ability */
    //rtl8367b_setAsicLutLearnLimitNo(r8367_cpu_port,0);

    /* disable unknown unicast/mcast/bcast flooding between LAN ports */
    //rtl83xx_setAsicReg(RTL8367B_REG_UNDA_FLOODING_PMSK, BIT(r8367_cpu_port));

    memset(&filter_field, 0, 2*sizeof(rtk_filter_field_t));
    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));
    /*Search all MAC (data & mask are all "0")*/
    filter_field[0].fieldType = FILTER_FIELD_UDP_DPORT;
    filter_field[0].filter_pattern_union.udpDstPort.dataType = FILTER_FIELD_DATA_MASK;
    filter_field[0].filter_pattern_union.udpDstPort.value = 53;
    filter_field[0].filter_pattern_union.udpDstPort.mask = 0xFFFF;
    filter_field[0].next = NULL;
    if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field[0])) != RT_ERR_OK)
        return RT_ERR_FAILED;

    cfg.careTag.tagType[CARE_TAG_UDP].value = TRUE;
    cfg.careTag.tagType[CARE_TAG_UDP].mask = TRUE;
    /*Add port0~port4 to active ports*/
    cfg.activeport.value.bits[0] = vutpportmask;
    cfg.activeport.mask.bits[0] = vportmask;
    cfg.invert =FALSE;
    /*Set Action to Trap to CPU*/
    act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;
    if ((retVal = rtk_filter_igrAcl_cfg_add(acl_idx, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return RT_ERR_FAILED;

    return RT_ERR_OK;
#endif
}

int rtl_83xx_remove_acl_for_dns(unsigned int acl_idx)
{
    int ret;
    ret = rtk_filter_igrAcl_cfg_del(acl_idx);
    return ret;
}
#endif

#if defined (IMPROVE_MCAST_PERFORMANCE_WITH_RTL8367)
int rtl_initMcastImprove(void)
{
    return rtk_qos_init(1);
}
int rtl865x_enableRtl8367McastPriorityAcl(int priority)
{
    int retVal;
    rtk_filter_field_t	filter_field[2];
    rtk_filter_cfg_t	cfg;
    rtk_filter_action_t	act;
    rtk_filter_number_t	ruleNum = 0;


    memset(filter_field, 0, 2*sizeof(rtk_filter_field_t));
    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));

    filter_field[0].fieldType = FILTER_FIELD_DMAC;
    filter_field[0].filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
    filter_field[0].filter_pattern_union.dmac.value.octet[0] = 0x01;
    filter_field[0].filter_pattern_union.dmac.value.octet[1] = 0x00;
    filter_field[0].filter_pattern_union.dmac.value.octet[2] = 0x5e;
    filter_field[0].filter_pattern_union.dmac.mask.octet[0] = 0xFF;
    filter_field[0].filter_pattern_union.dmac.mask.octet[1] = 0xFF;
    filter_field[0].filter_pattern_union.dmac.mask.octet[2] = 0xFF;
    filter_field[0].next = NULL;
    if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field[0])) != RT_ERR_OK)
        return retVal;

    cfg.activeport.value.bits[0] = vutpportmask;
    cfg.activeport.mask.bits[0] = vportmask;
    cfg.invert = FALSE;
    act.actEnable[FILTER_ENACT_PRIORITY] = TRUE;
    act.filterPriority = priority;

    if ((retVal = rtk_filter_igrAcl_cfg_add(1, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return retVal;

    return RT_ERR_OK;

}
int rtl865x_disableRtl8367McastPriorityAcl(void)
{
    return rtk_filter_igrAcl_cfg_del(1);
}

int rtl_enable_mCast_improve(int enable)
{
    if(enable)
    {
        rtk_qos_queueNum_set(r8367_cpu_port, 2);
        rtl865x_enableRtl8367McastPriorityAcl(7);
    }
    else
    {
        rtk_qos_queueNum_set(r8367_cpu_port, 1);
        rtl865x_disableRtl8367McastPriorityAcl();
    }
    return 0;
}
#endif

#if defined(CONFIG_RTL_83XX_QOS_SUPPORT) || defined(CONFIG_RTL_83XX_QOS_TEST)
#define MAX_PHY_PORT_NUM 5
#if defined(CONFIG_RTL_8198C) || defined(CONFIG_RTL_8197F)
#define MAX_RTL83XX_QOS_QUEUE_NUM 8
#else
#define MAX_RTL83XX_QOS_QUEUE_NUM 6
#endif
#define	QOS_VALID_MASK	0x2
#define	QOS_TYPE_MASK		0x1
#define	QOS_TYPE_STR		0x0	/*0x0|QOS_VALID_MASK*/
#define	QOS_TYPE_WFQ		0x1	/*0x1|QOS_VALID_MASK*/
#if defined CONFIG_RTL_83XX_QOS_SUPPORT
#define CPU_METER_ID1        16
#define CPU_METER_ID2        17
#define CPU_METER_ID3		 24 //for cpu port 7
#define UNKNOWN_OWNER_METER  0
#define WAN_METER            1
#define LAN_METER            2
#define CPU_METER            3
int qos_meter_owner[MAX_METERNUM];
#endif

enum PriDecIdx
{
    PORT_BASE	= 0,
    D1P_BASE,
    DSCP_BASE,
    ACL_BASE,
    NAT_BASE,
#if defined(CONFIG_RTL_8197F)
    VID_BASE,
#endif
    PRI_TYPE_NUM,
};

/*port, acl, dscp, 1q, 1ad, cvlan, da, sa for 8367c*/
CONST_T rtk_uint32 g_priorityDecision_67C[8] = {0,7,2,1,5,6,4,3};
/*port, acl, dscp, 1q, 1ad for 8367d*/
CONST_T rtk_uint32 g_priorityDecision_67D[8] = {0,4,2,1,3};

#if defined CONFIG_RTL_PROC_NEW
int rtl_83xxQosReadProc(struct seq_file *s, void *v)
{
    int port, logicPort, queueNum, i, queue, retVal=0;
    rtk_qos_queue_weights_t qWeights;
    rtk_meter_id_t meterId;
    rtk_meter_type_t type;
    rtk_rate_t qRate, pRate;
    rtk_enable_t qIfg_include, pIfg_include, qEnable;
    rtk_uint32 Bucket_size;
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
    rtk_rate_t inRate;
    rtk_enable_t inIfg_include, inFc_enable;

    rtk_port_macForceLinkExt_get(r8367_cpu_port,&mode,&mac_cfg);

    seq_printf(s, "%s\n", "rtl83xx cpu flow control");
    seq_printf(s, "\ttx pause:%d\n", mac_cfg.txpause);
    seq_printf(s, "\trx pause:%d\n", mac_cfg.rxpause);

    seq_printf(s, "%s\n", "rtl83xx qos related parameters");
    for(port=0; port <= RTK_PORT_ID_MAX; port++)
    {
        logicPort=rtk_switch_port_P2L_get(port);
        //printk("[%s:%d] port: %d, logicPort: %d\n", __FUNCTION__, __LINE__, port, logicPort);
        if(logicPort == UNDEFINE_PORT)
        {
            continue;
        }

        if((retVal=rtk_qos_queueNum_get(logicPort,&queueNum)) != RT_ERR_OK)
        {
            //printk("[%s:%d] rtk_qos_queueNum_get fail, retVal: %d, port: %d\n",
            //__FUNCTION__, __LINE__, retVal, port);
            continue;
        }

        if((retVal=rtk_rate_egrBandwidthCtrlRate_get(logicPort, &pRate, &pIfg_include)) != RT_ERR_OK)
        {
            //printk("[%s:%d] rtk_rate_egrBandwidthCtrlRate_get fail, retVal: %d, port: %d\n",
            //__FUNCTION__, __LINE__, retVal, port);
            continue;
        }

        if((retVal=rtk_qos_schedulingQueue_get(logicPort, &qWeights)) != RT_ERR_OK)
        {
            //printk("[%s:%d] rtk_qos_schedulingQueue_get fail, retVal: %d, port: %d\n",
            //__FUNCTION__, __LINE__, retVal, port);
            continue;
        }

        if((retVal=rtk_rate_egrQueueBwCtrlEnable_get(logicPort, RTK_WHOLE_SYSTEM, &qEnable)) != RT_ERR_OK)
        {
            //printk("[%s:%d] rtk_rate_egrQueueBwCtrlEnable_get fail, retVal: %d, port: %d\n",
            //__FUNCTION__, __LINE__, retVal, port);
            continue;
        }
        if (logicPort < RTK_MAX_NUM_OF_PORT)
        {
            seq_printf(s,"<%d> queueNum:%d portRate:%d Ifg include:%d queueBwCtrl:%s\n",
                       logicPort, queueNum, pRate, pIfg_include, (qEnable==ENABLED)?"enabled":"disable");
        }
        else
        {
            seq_printf(s,"<EXT_PORT%d> queueNum:%d portRate:%d Ifg include:%d queueBwCtrl:%s\n",
                       (logicPort-16), queueNum, pRate, pIfg_include, (qEnable==ENABLED)?"enabled":"disable");
        }

        if((retVal=rtk_rate_igrBandwidthCtrlRate_get(logicPort, &inRate, &inIfg_include, &inFc_enable)) != RT_ERR_OK)
        {
            //printk("[%s:%d] rtk_rate_igrBandwidthCtrlRate_get fail, retVal: %d, port: %d\n",
            //__FUNCTION__, __LINE__, retVal, port);
            continue;
        }
        seq_printf(s,"\tingressBw:%d Ifg include:%d flowControl:%d\n",
                   inRate, inIfg_include, inFc_enable);

        seq_printf(s,"\tQueue Parameters:\n ");
        for(queue=0; queue<RTK_MAX_NUM_OF_QUEUE; queue++)
        {
            if((retVal=rtk_rate_egrQueueBwCtrlRate_get(logicPort, queue, &meterId)) != RT_ERR_OK)
            {
                //printk("[%s:%d] rtk_rate_egrQueueBwCtrlRate_get fail, retVal: %d, port: %d\n",
                //__FUNCTION__, __LINE__, retVal, port);
                continue;
            }

            seq_printf(s,"\t[%d] type:%s, weight:%d, MeterId:%d\n",
                       queue, (qWeights.weights[queue]==0)?"SP":"WFQ", qWeights.weights[queue], meterId);
        }

        seq_printf(s,"\tMeter Parameters:\n ");
        if(port<4)
        {
            for(i=port*8; i<=port*8+7; i++)
            {
                if((retVal=rtk_rate_shareMeter_get(i, &type, &qRate, &qIfg_include)) != RT_ERR_OK)
                {
                    //printk("[%s:%d] rtk_rate_shareMeter_get fail, retVal: %d, port: %d\n",
                    //__FUNCTION__, __LINE__, retVal, port);
                    continue;
                }
                if((retVal=rtk_rate_shareMeterBucket_get(i,&Bucket_size)) != RT_ERR_OK)
                {
                    //printk("[%s:%d] rtk_rate_shareMeterBucket_get fail, retVal: %d, port: %d\n",
                    //__FUNCTION__, __LINE__, retVal, port);
                    continue;
                }
                if(i==port*8)
                    seq_printf(s,"\t");
                if(i==port*8+4)
                    seq_printf(s,"\n\t");
                seq_printf(s,"[%d]%d,%d,%d ", i, qRate, qIfg_include,Bucket_size);
            }
            seq_printf(s,"\n");
        }
        else
        {
            for(i=(port-4)*8; i<=(port-4)*8+7; i++)
            {
                if(i==(port-4)*8)
                    seq_printf(s,"\t");
                if(i==(port-4)*8+4)
                    seq_printf(s,"\n\t");
                if((retVal=rtk_rate_shareMeter_get(i, &type, &qRate, &qIfg_include)) != RT_ERR_OK)
                {
                    //printk("[%s:%d] rtk_rate_shareMeter_get fail, retVal: %d, port: %d\n",
                    //__FUNCTION__, __LINE__, retVal, port);
                    continue;
                }
                if((retVal=rtk_rate_shareMeterBucket_get(i,&Bucket_size)) != RT_ERR_OK)
                {
                    //printk("[%s:%d] rtk_rate_shareMeterBucket_get fail, retVal: %d, port: %d\n",
                    //__FUNCTION__, __LINE__, retVal, port);
                    continue;
                }
                seq_printf(s,"[%d]%d,%d,%d ", i, qRate, qIfg_include, Bucket_size);
            }
            seq_printf(s,"\n");
        }

    }
    return 0;
}
#else
int rtl_83xxQosReadProc(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int len;

    int port, queueNum, i, queue;
    rtk_qos_queue_weights_t qWeights;
    rtk_meter_id_t meterId;
    rtk_rate_t qRate, pRate;
    rtk_enable_t qIfg_include, pIfg_include, qEnable;
    rtk_uint32 Bucket_size;
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;
    rtk_rate_t inRate;
    rtk_enable_t inIfg_include, inFc_enable;
    rtk_meter_type_t type;

    rtk_port_macForceLinkExt_get(EXT_PORT_1,&mode,&mac_cfg);

    len = sprintf(page, "%s\n", "rtl83xx cpu flow control");
    len += sprintf(page+len, "\ttx pause:%d\n", mac_cfg.txpause);
    len += sprintf(page+len, "\trx pause:%d\n", mac_cfg.rxpause);
    len += sprintf(page+len, "%s\n", "rtl83xx qos related parameters");

    for(port=0; port<RTK_PORT_ID_MAX; port++)
    {
        rtk_qos_queueNum_get(port,&queueNum);
        rtk_rate_egrBandwidthCtrlRate_get(port, &pRate, &pIfg_include);

        rtk_qos_schedulingQueue_get(port, &qWeights);
        rtk_rate_egrQueueBwCtrlEnable_get(port, RTK_WHOLE_SYSTEM, &qEnable);

        len += sprintf(page+len,"<%d> queueNum:%d portRate:%d Ifg include:%d queueBwCtrl:%s\n",
                       port, queueNum, pRate, pIfg_include, (qEnable==ENABLED)?"enabled":"disable");
        rtk_rate_igrBandwidthCtrlRate_get(port, &inRate, &inIfg_include, &inFc_enable);
        len += sprintf(page+len,"\tingressBw:%d Ifg include:%d flowControl:%d\n",
                       inRate, inIfg_include, inFc_enable);

        len += sprintf(page+len,"\tQueue Parameters:\n ");
        for(queue=0; queue<RTK_MAX_NUM_OF_QUEUE; queue++)
        {
            rtk_rate_egrQueueBwCtrlRate_get(port, queue, &meterId);

            len += sprintf(page+len,"\t[%d] type:%s, weight:%d, MeterId:%d\n",
                           queue, (qWeights.weights[queue]==0)?"SP":"WFQ", qWeights.weights[queue], meterId);
        }

        len += sprintf(page+len,"\tMeter Parameters:\n ");
        if(port<4)
        {
            for(i=port*8; i<=port*8+7; i++)
            {
                rtk_rate_shareMeter_get(i, &type, &qRate, &qIfg_include);
                rtk_rate_shareMeterBucket_get(i,&Bucket_size);
                if(i==port*8)
                    len += sprintf(page+len,"\t");
                if(i==port*8+4)
                    len += sprintf(page+len,"\n\t");
                len += sprintf(page+len,"[%d]%d,%d,%d ", i, qRate, qIfg_include,Bucket_size);
            }
            len += sprintf(page+len,"\n");
        }
        else
        {
            for(i=(port-4)*8; i<=(port-4)*8+7; i++)
            {
                if(i==(port-4)*8)
                    len += sprintf(page+len,"\t");
                if(i==(port-4)*8+4)
                    len += sprintf(page+len,"\n\t");
                rtk_rate_shareMeter_get(i, &type, &qRate, &qIfg_include);
                rtk_rate_shareMeterBucket_get(i,&Bucket_size);
                len += sprintf(page+len,"[%d]%d,%d,%d ", i, qRate, qIfg_include, Bucket_size);
            }
            len += sprintf(page+len,"\n");
        }

    }

    if (len <= off+count)
        *eof = 1;

    *start = page + off;
    len -= off;

    if (len>count)
        len = count;

    if (len<0) len = 0;

    return len;
}
#endif

int rtl_83xxQosWriteProc(struct file *file, const char *buffer,
                         unsigned long count, void *data)
{
    char tmp[256];
    char		*strptr;
    char		*tokptr;
    int port, logic_port, qid, page, retVal;
    unsigned int queue_page[RTK_MAX_NUM_OF_QUEUE];
    rtk_priority_select_t pPriDec;
    rtk_pri_t prio, dot1q;
    rtk_dscp_t dscp;
    rtk_queue_num_t queue_num;
    rtk_qos_pri2queue_t pPri2qid;
    rtk_port_t portId;

    if (count < 2)
        return -EFAULT;

    if (buffer && !copy_from_user(tmp, buffer, count))
    {

        tmp[count] = '\0';
        strptr=tmp;
        tokptr = strsep(&strptr," ");
        if (tokptr==NULL)
        {
            goto errout;
        }

        if (!memcmp(tokptr, "current", 7))
        {
            printk( "Current Page for Egress Port and Queues\n");
            printk( "PortNo.\tPort\tQ0\tQ1\tQ2\tQ3\tQ4\tQ5\tQ6\tQ7\n");
            for(port=0; port<=RTK_PORT_ID_MAX; port++)
            {
                /*Egress Port page number*/
                rtl83xx_setAsicReg(RTL83XX_REG_FLOWCTRL_DEBUG_CTRL0,port);
                rtl83xx_getAsicReg(RTL83XX_REG_FLOWCTRL_PORT_PAGE_COUNT,&page);

                for(qid=0; qid<=7; qid++)
                {
                    rtl83xx_getAsicReg(RTL83XX_REG_FLOWCTRL_QUEUE0_PAGE_COUNT+qid,&queue_page[qid]);
                }

                printk("%2d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", port,page,
                       queue_page[0],queue_page[1],queue_page[2],queue_page[3],queue_page[4],queue_page[5],queue_page[6],queue_page[7]);
            }

        }
        else if(!memcmp(tokptr, "max",3))
        {
            printk( "Maximum Page for Egress Port and Queues\n");
            printk( "PortNo.\tPort\tQ0\tQ1\tQ2\tQ3\tQ4\tQ5\tQ6\tQ7\n");
            for(port=0; port<RTK_PORT_ID_MAX; port++)
            {
                /*Egress Port page number*/
                rtl83xx_setAsicReg(RTL83XX_REG_FLOWCTRL_DEBUG_CTRL0,port);
                rtl83xx_getAsicReg(RTL83XX_REG_FLOWCTRL_PORT_MAX_PAGE_COUNT,&page);

                for(qid=0; qid<=7; qid++)
                {
                    rtl83xx_getAsicReg(RTL83XX_REG_FLOWCTRL_QUEUE0_MAX_PAGE_COUNT+qid,&queue_page[qid]);
                }

                printk("%2d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", port,page,
                       queue_page[0],queue_page[1],queue_page[2],queue_page[3],queue_page[4],queue_page[5],queue_page[6],queue_page[7]);

            }
        }
        else if(!memcmp(tokptr, "decision",8))
        {
            tokptr = strsep(&strptr," ");
            if (tokptr==NULL)
            {
                goto errout;
            }

            if(!memcmp(tokptr, "show",4))
            {
                if((retVal=rtk_qos_priSel_get(PRIDECTBL_IDX0, &pPriDec)) != RT_ERR_OK)
                {
                    goto errout;
                }

                printk("tbl0 priority decision:\n");
                printk("port:%x, 1p:%x, acl:%x, dscp:%x, cvlan:%x, svlan:%x, smac:%x, dmac:%x\n",
                       (0x1<<pPriDec.port_pri), (0x1<<pPriDec.dot1q_pri), (0x1<<pPriDec.acl_pri), (0x1<<pPriDec.dscp_pri),
                       (0x1<<pPriDec.cvlan_pri), (0x1<<pPriDec.svlan_pri), (0x1<<pPriDec.smac_pri), (0x1<<pPriDec.dmac_pri));

                if((retVal=rtk_qos_priSel_get(PRIDECTBL_IDX1, &pPriDec)) != RT_ERR_OK)
                {
                    goto errout;
                }

                printk("\ntbl1 priority decision:\n");
                printk("port:%x, 1p:%x, acl:%x, dscp:%x, cvlan:%x, svlan:%x, smac:%x, dmac:%x\n",
                       (0x1<<pPriDec.port_pri), (0x1<<pPriDec.dot1q_pri), (0x1<<pPriDec.acl_pri), (0x1<<pPriDec.dscp_pri),
                       (0x1<<pPriDec.cvlan_pri), (0x1<<pPriDec.svlan_pri), (0x1<<pPriDec.smac_pri), (0x1<<pPriDec.dmac_pri));

            }
            else if(!memcmp(tokptr, "set",3))
            {
                int cnt=0;
                tokptr = strsep(&strptr," ");
                if (tokptr==NULL)
                {
                    goto errout;
                }

                cnt = sscanf(tokptr, "port %d 1p %d acl %d dscp %d cvlan %d svlan %d dmac %d smac %d",
                             &pPriDec.port_pri, &pPriDec.dot1q_pri, &pPriDec.acl_pri, &pPriDec.dscp_pri, &pPriDec.cvlan_pri, &pPriDec.svlan_pri, &pPriDec.dmac_pri, &pPriDec.smac_pri);

                if((retVal=rtk_qos_priSel_set(PRIDECTBL_IDX0, &pPriDec)) != RT_ERR_OK)
                {
                    goto errout;
                }

                if((retVal=rtk_qos_priSel_set(PRIDECTBL_IDX1, &pPriDec)) != RT_ERR_OK)
                {
                    goto errout;
                }
            }
        }
        else if(!memcmp(tokptr, "portpri",7))
        {
            printk("port pri:\n");
            for(port=0; port<=RTK_PORT_ID_MAX; port++)
            {
                logic_port = rtk_switch_port_P2L_get(port);
                if(logic_port == UNDEFINE_PORT)
                {
                    continue;
                }

                rtk_qos_portPri_get(logic_port,&prio);
                printk("\t%d:%d ", logic_port, prio);
            }
            printk("\n");
        }
        else if(!memcmp(tokptr, "dot1qpri",8))
        {
            printk("1p remap:\n");
            for(dot1q=0; dot1q<=RTK_PRIMAX; dot1q++)
            {
                rtk_qos_1pPriRemap_get(dot1q,&prio);
                printk("\t%d:%d ", dot1q, prio);
            }
            printk("\n");
        }
        else if(!memcmp(tokptr, "dscppri",7))
        {
            printk("dscp remap:\n");
            for(dscp=0; dscp<=RTK_DSCPMAX; dscp++)
            {
                rtk_qos_dscpPriRemap_get(dscp,&prio);
                printk("\t%d:%d ", dscp, prio);
            }
            printk("\n");
        }
        else if(!memcmp(tokptr, "cvlanpri",8))
        {
        }
        else if(!memcmp(tokptr, "svlanpri",8))
        {
        }
        else if(!memcmp(tokptr, "dmacpri",7))
        {
        }
        else if(!memcmp(tokptr, "smacpri",7))
        {
        }
        else if(!memcmp(tokptr, "prio2qid", 8))
        {
            printk("num\tprio0\tprio1\tprio2\tprio3\tprio4\tprio5\tprio6\tprio7\n");
            for(queue_num=1; queue_num<=RTK_QUEUENO; queue_num++)
            {
                printk("[%d]\t", queue_num);

                rtk_qos_priMap_get(queue_num, &pPri2qid);
                for(prio=0; prio<RTK_MAX_NUM_OF_PRIORITY; prio++)
                {
                    printk("%d\t", pPri2qid.pri2queue[prio]);
                }
                printk("\n");
            }
        }
        else if(!memcmp(tokptr, "queuenum", 7))
        {
            printk("port\tqueueNum\n");
            for(portId=0; portId<=RTK_PORT_ID_MAX; portId++)
            {
                queue_num=0;
                rtk_qos_queueNum_get(portId, &queue_num);
                printk("%d\t%d\n", portId, queue_num);
            }
        }
        else if(!memcmp(tokptr, "dscp_remark",11))
        {
        }
        else if(!memcmp(tokptr, "1p_remark", 9))
        {
        }
        else
        {
            goto errout;
        }
    }
    else
    {
errout:
        printk("error input!\n");
    }

    return count;
}

int rtl83xx_qos_init(void)
{
    //rtk_uint32 priority;
    //rtk_api_ret_t retVal;
    //rtk_queue_num_t queueNum;
    //rtk_uint32 priDec;
    int i;

    //the priority to qid matrix should be equal to 97F/9xd...
    //change this matrix can't work!!!!??
#if 0
    CONST_T rtk_uint16 g_prioritytToQid[8][8]=
    {
        {0,0,0,0,0,0,0,0},
        {0,0,0,0,5,5,5,5},
        {0,0,0,0,1,1,5,5},
        {0,0,0,1,2,2,5,5},
        {0,0,0,1,2,3,5,5},
        {0,0,1,2,3,4,5,5},
        {0,0,1,2,3,4,5,6},
        {0,1,2,3,4,5,6,7}
    };
#endif

    rtk_qos_init(1);
#if 0
    /*Set Priority to Qid*/
    for(queueNum = 0; priority <RTK_MAX_NUM_OF_QUEUE; priority++)
    {
        for (priority = 0; priority <= RTL8367B_PRIMAX; priority++)
        {
            if ((retVal = rtl8367b_setAsicPriorityToQIDMappingTable(queueNum, priority, g_prioritytToQid[queueNum][priority])) != RT_ERR_OK)
                return retVal;
        }

    }
#endif

    /*Change Priority Decision Order*/
#if 0 //depends on qos rule
    for (priDec = 0; priDec < PRIDEC_END; priDec++)
    {
        if ((retVal = rtl83xx_setAsicPriorityDecision(PRIDECTBL_IDX0, priDec, g_priorityDecision[priDec])) != RT_ERR_OK)
            return retVal;
        if ((retVal = rtl83xx_setAsicPriorityDecision(PRIDECTBL_IDX1, priDec, g_priorityDecision[priDec])) != RT_ERR_OK)
            return retVal;
    }
#endif

    for(i=0; i<RTL83XX_METERNO; i++)
    {
        if(i==CPU_METER_ID1 || i==CPU_METER_ID2 || i==CPU_METER_ID3)
            qos_meter_owner[i] = CPU_METER;
        else
            qos_meter_owner[i] = UNKNOWN_OWNER_METER;
    }
    return RT_ERR_OK;
}

int rtl83xx_dscp_remark_status_get(unsigned int *status)
{
    int retVal = RT_ERR_FAILED;
    if(status == NULL)
        return retVal;

    retVal = rtk_qos_dscpRemarkEnable_get(RTK_WHOLE_SYSTEM, status);
    return retVal;
}

int rtl83xx_1p_remark_status_get(unsigned int port, unsigned int *status)
{
    int retVal = RT_ERR_FAILED;
    if(status == NULL)
        return retVal;

    retVal = rtk_qos_1pRemarkEnable_get(port, status);
    return retVal;
}

void rtl83xx_dscp_remark_config_get(void)
{
    rtk_enable_t status;

    rtk_qos_dscpRemarkEnable_get(RTK_WHOLE_SYSTEM, &status);
    printk("[%s:%d] RTK_WHOLE_SYSTEM dscp remark status: %d\n\n", __FUNCTION__, __LINE__, status);
}

void rtl83xx_1p_remark_config_get(rtk_port_t port)
{
    rtk_enable_t status;

    rtk_qos_1pRemarkEnable_get(port, &status);
    printk("[%s:%d] port%d 802.1p remark status: %d\n\n", __FUNCTION__, __LINE__, port, status);
}

void rtl83xx_dscp_remark_get(rtk_pri_t int_pri)
{
    rtk_dscp_t dscp;

    rtk_qos_dscpRemark_get(int_pri, &dscp);
    printk("[%s:%d] 83xx priority %d remark dscp to %d\n", __FUNCTION__, __LINE__,int_pri, dscp);
}

void rtl83xx_1p_remark_get(rtk_pri_t int_pri)
{
    rtk_pri_t dot1p_pri;

    rtk_qos_1pRemark_get(int_pri, &dot1p_pri);
    printk("[%s:%d] 83xx priority %d remark 802.1p to %d\n", __FUNCTION__, __LINE__,int_pri, dot1p_pri);
}

int rtl83xx_remark_confg_get(int port, int *dscpRemkStat, int *d1pRemkStas)
{
    rtk_enable_t d1pStatus, dscpStatus;
    rtk_api_ret_t retVal;

    retVal = rtk_qos_1pRemarkEnable_get(port, &d1pStatus);
    if(retVal != RT_ERR_OK)
        return retVal;
    *d1pRemkStas = d1pStatus;

    retVal = rtk_qos_dscpRemarkEnable_get(RTK_WHOLE_SYSTEM, &dscpStatus);
    if(retVal != RT_ERR_OK)
        return retVal;
    *dscpRemkStat = dscpStatus;

    return RT_ERR_OK;
}

int rtl83xx_remark_get(int sysPri, int *dscpRemk, int *d1pRemk)
{
    rtk_api_ret_t retVal;
    rtk_pri_t dot1p_pri;
    rtk_dscp_t dscp;
    rtk_pri_t int_pri = sysPri;

    retVal = rtk_qos_1pRemark_get(int_pri, &dot1p_pri);
    if(retVal != RT_ERR_OK)
        return retVal;
    *d1pRemk = dot1p_pri;

    retVal = rtk_qos_dscpRemark_get(int_pri, &dscp);
    if(retVal != RT_ERR_OK)
        return retVal;
    *dscpRemk = dscp;

    return RT_ERR_OK;
}

void rtl83xx_dscp_remark_set(unsigned int remarkPort, unsigned int sysPri, unsigned int dscpValue)
{
    rtk_pri_t int_pri = sysPri;
    rtk_dscp_t dscp = dscpValue;
    unsigned int logic_port = rtk_switch_port_P2L_get(remarkPort);

	if(chip_type==CHIP_RTL8367D)
    	rtk_qos_dscpRemarkEnable_set(logic_port, ENABLED);
	else
		rtk_qos_dscpRemarkEnable_set(RTK_WHOLE_SYSTEM, ENABLED);
	
    rtk_qos_dscpRemark_set(int_pri, dscp);
}

void rtl83xx_1p_remark_set(unsigned int remarkPort, unsigned int sysPri, unsigned int dot1pValue)
{
   	rtk_port_t port = remarkPort;
   	rtk_pri_t int_pri = sysPri;
   	rtk_pri_t dot1p_pri = dot1pValue;
	unsigned int logic_port = rtk_switch_port_P2L_get(port);

    rtk_qos_1pRemarkEnable_set(logic_port, ENABLED);
    rtk_qos_1pRemark_set(int_pri, dot1p_pri);
}

// flush remark setting
void rtl83xx_flush_dscp_remark_setting(void)
{
    rtk_pri_t int_pri;

    for(int_pri = 0; int_pri <= RTK_PRIMAX; int_pri++)
    {
        rtk_qos_dscpRemark_set(int_pri, 0);
    }
}

// disable remark control
void rtl83xx_disable_dscp_remark_setting(unsigned int remarkPort)
{
	unsigned int logic_port = rtk_switch_port_P2L_get(remarkPort);
	if(chip_type==CHIP_RTL8367D)
    	rtk_qos_dscpRemarkEnable_set(logic_port, DISABLED);
	else
		rtk_qos_dscpRemarkEnable_set(RTK_WHOLE_SYSTEM,DISABLED);
}

void rtl83xx_flush_1p_remark_setting(void)
{
    rtk_pri_t int_pri;

    for(int_pri = 0; int_pri <= RTK_PRIMAX; int_pri++)
    {
        rtk_qos_1pRemark_set(int_pri, 0);
    }
}

void rtl83xx_disable_1p_remark_setting(void)
{
    rtk_port_t port;

    for(port = 0; port <= RTK_PORT_ID_MAX; port++)
    {
        rtk_qos_1pRemarkEnable_set(port, DISABLED);
    }
}

void rtl83xx_setFlowControl(int qosEnable);
extern int rtl8651_disableVlanRemark(void);
/* 	this function is only called by rtk_cmd 83xx qos,
	and it will disable RTL819X 802.1p remark incase of RTL83XX 802.1p-based qos */
void rtl83xx_rechange_qos(unsigned int flag)
{
    rtl83xx_setFlowControl(flag);
    rtl8651_disableVlanRemark();

    if(flag == 0)
    {
        int port, logic_port, queue;
        rtk_qos_queue_weights_t qweights;
        memset(&qweights, 0, sizeof(rtk_qos_queue_weights_t));

        for(port = 0; port <= RTK_PORT_ID_MAX; port++)
        {
            logic_port = rtk_switch_port_P2L_get(port);
            if(logic_port == UNDEFINE_PORT)
            {
                continue;
            }

            rtk_rate_egrBandwidthCtrlRate_set(logic_port, RTL83XX_QOS_RATE_INPUT_MAX, ENABLED);
            rtk_qos_queueNum_set(logic_port, 1);
            rtk_qos_schedulingQueue_set(logic_port,&qweights);

            for (queue = 0; queue < RTK_MAX_NUM_OF_QUEUE; queue++)
            {
                qweights.weights[queue] = 0;
                if(port < 4)
                    rtk_rate_egrQueueBwCtrlRate_set(logic_port, queue, port*8);
                else
                    rtk_rate_egrQueueBwCtrlRate_set(logic_port, queue, (port-4)*8);
            }
            rtk_rate_egrQueueBwCtrlEnable_set(logic_port, RTK_WHOLE_SYSTEM,DISABLED);
        }
    }

    return;
}

#if defined CONFIG_RTL_83XX_QOS_TEST
extern void rtl865x_qos_set(void);
int rtl83xx_qos_test(void)
{
    int qid; // port;
    //rtk_priority_select_t PriDec;
    rtk_qos_queue_weights_t qweights;

    //97D
    rtl865x_qos_set();

    //init qos
    rtl83xx_qos_init();

    //set wan port queue num=2
    rtk_qos_queueNum_set(RTL83XX_PORT4_ENABLE_OFFSET, 2);
    //rtk_qos_queueNum_set(r8367_cpu_port, 2);

    //set port priority
#if 0
    rtk_qos_portPri_set(0,0);
    rtk_qos_portPri_set(1,0);
    rtk_qos_portPri_set(2,7);
    rtk_qos_portPri_set(3,7);
#endif

    //set queue parameter on CPU port
    //strict priority
    for (qid = 0; qid < RTK_MAX_NUM_OF_QUEUE; qid ++)
    {
        if(qid==0 || qid==7)
            qweights.weights[qid] = 0;
        else
            qweights.weights[qid] = 0;
    }
#if 1
    rtk_qos_schedulingQueue_set(RTL83XX_PORT4_ENABLE_OFFSET,&qweights);
    //rtk_rate_egrBandwidthCtrlRate_set(RTL8367B_PORT4_ENABLE_OFFSET, 204800, 1);

    //set queue 0 rate to 10M and queue 7 rate to 5M
    rtk_rate_shareMeter_set(0, METER_TYPE_KBPS, 15360, ENABLED);
    rtk_rate_shareMeter_set(1, METER_TYPE_KBPS, 10240, ENABLED);
    rtk_rate_shareMeterBucket_set(0,10000);
    rtk_rate_shareMeterBucket_set(1,30000);
    rtk_rate_egrQueueBwCtrlEnable_set(RTL83XX_PORT4_ENABLE_OFFSET,0xFF,ENABLED);
    rtk_rate_egrQueueBwCtrlRate_set(RTL83XX_PORT4_ENABLE_OFFSET,0,0);
    rtk_rate_egrQueueBwCtrlRate_set(RTL83XX_PORT4_ENABLE_OFFSET,7,1);
#else
    rtk_qos_schedulingQueue_set(r8367_cpu_port,&qweights);
    rtk_rate_egrBandwidthCtrlRate_set(r8367_cpu_port, 204800, 1);

    //set queue 0 rate to 10M and queue 7 rate to 5M
    rtk_rate_shareMeter_set(16, 204800, ENABLED);
    rtk_rate_shareMeter_set(17, 153600, ENABLED);
    rtk_rate_shareMeterBucket_set(16,10000);
    rtk_rate_shareMeterBucket_set(17,30000);
    rtk_rate_egrQueueBwCtrlEnable_set(r8367_cpu_port,0xFF,ENABLED);
    rtk_rate_egrQueueBwCtrlRate_set(r8367_cpu_port,0,16);
    rtk_rate_egrQueueBwCtrlRate_set(r8367_cpu_port,7,17);
#endif

    return (RT_ERR_OK);
}
#endif

#if defined CONFIG_RTL_83XX_QOS_SUPPORT
int rtl83xx_qosSetIngressBandwidth(unsigned int memberPort, unsigned int Kbps)
{
    unsigned int port, logic_port;

    //todo: wait output queue empty?
    for(port=0; port<MAX_PHY_PORT_NUM; port++)
    {
        if(((1<<port)&memberPort)==0)
            continue;

		logic_port = rtk_switch_port_P2L_get(port);
		
        if(Kbps==0)
            rtk_rate_igrBandwidthCtrlRate_set(logic_port, RTL83XX_QOS_RATE_INPUT_MAX, DISABLED, ENABLED);
        else
            rtk_rate_igrBandwidthCtrlRate_set(logic_port, Kbps, ENABLED, ENABLED);
    }
    return 0;

}

int rtl83xx_qosSetBandwidth(unsigned int memberPort, unsigned int Kbps)
{
    unsigned int port, logic_port;
    int ret;

    //todo: wait output queue empty?
    for(port=0; port<MAX_PHY_PORT_NUM; port++)
    {
        if(((1<<port)&memberPort)==0)
            continue;

		logic_port = rtk_switch_port_P2L_get(port);
        ret=rtk_rate_egrBandwidthCtrlRate_set(logic_port, Kbps, ENABLED);
    }
    return 0;
}

int rtl83xx_qosFlushBandwidth(unsigned int memberPort)
{
    unsigned int port, logic_port;

    //todo: wait output queue empty?
    for(port=0; port<MAX_PHY_PORT_NUM; port++)
    {
        if(((1<<port)&memberPort)==0)
            continue;

		logic_port = rtk_switch_port_P2L_get(port);
        rtk_rate_egrBandwidthCtrlRate_set(logic_port, RTL83XX_QOS_RATE_INPUT_MAX, ENABLED);
    }
    return 0;
}

int rtl83xx_qosGetMeterId(unsigned int port, unsigned int bw, unsigned int wanPortMask, unsigned int *meterId)
{
    rtk_meter_id_t idbegin, idend;
    rtk_meter_id_t i;
    int retVal = RT_ERR_FAILED;
    //rtk_rate_t Rate;
    //rtk_data_t Ifg_include;

    if(port > RTK_PORT_ID_MAX || meterId == NULL)
        return retVal;

    if(port<4)
    {
        idbegin = port*8;
        idend = port*8+7;
    }
    else
    {
        idbegin = (port-4)*8;
        idend = (port-4)*8+7;
    }

    for(i=idbegin; i<=idend; i++)
    {
        if(qos_meter_owner[i] == UNKNOWN_OWNER_METER)
            break;
    }
    if(i > idend)
    {
        return retVal;
    }

    //printk("get meter of port %d, id:%d, bw:%d, [%s:%d]\n", port, i, bw, __FUNCTION__, __LINE__);
    if(wanPortMask & (1<<port))
    {
        qos_meter_owner[i]=WAN_METER;
    }
    else
    {
        qos_meter_owner[i]=LAN_METER;
    }

    if((retVal = rtk_rate_shareMeter_set(i, METER_TYPE_KBPS, bw, ENABLED)) != RT_ERR_OK)
        return retVal;
    //rtk_rate_shareMeterBucket_set(j,QOS_BUCKET_SIZE);

    *meterId = i;
    return retVal;
}
//ok
int rtl83xx_qosFlushMeter(int port, unsigned int wanPortMask)
{
    int meterId,idbegin,idend;

    if(port<0 || port>=MAX_PHY_PORT_NUM)
        return -1;

    if(port<4)
    {
        idbegin = port*8;
        idend = port*8+7;
    }
    else
    {
        idbegin = (port-4)*8;
        idend = (port-4)*8+7;
    }

    //printk("flush meter of port %d, [%s:%d]\n", port, __FUNCTION__, __LINE__);

    for(meterId=idbegin; meterId<=idend; meterId++)
    {
        if(((wanPortMask & (1<<port)) && (qos_meter_owner[meterId]==WAN_METER))
                ||(!(wanPortMask & (1<<port)) && (qos_meter_owner[meterId]==LAN_METER)))
        {
            rtk_rate_shareMeter_set(meterId, METER_TYPE_KBPS, RTL83XX_QOS_RATE_INPUT_MAX, DISABLED);
            qos_meter_owner[meterId]=UNKNOWN_OWNER_METER;
            //always use default bucket size
            //rtk_rate_shareMeterBucket_set(meterId,DEFAULT_QOS_BUCKET_SIZE);
        }
    }
    return 0;
}

//ok
int rtl83xx_closeQos(unsigned int memberPort, unsigned int wanPortMask)
{
    unsigned int port, qid, logic_port;
    rtk_qos_queue_weights_t qweights;

    //todo: wait output queue empty?
    //for(port=0;port<MAX_PHY_PORT_NUM;port++)
    for(port=0; port<7; port++)	// include CPU port6
    {
        if(((1<<port)&memberPort)==0)
            continue;

		logic_port = rtk_switch_port_P2L_get(port);

        for (qid = 0; qid < RTK_MAX_NUM_OF_QUEUE; qid ++)
        {
            qweights.weights[qid] = 0;
            if(port<4)
                rtk_rate_egrQueueBwCtrlRate_set(logic_port, qid, port*8);
            else
                rtk_rate_egrQueueBwCtrlRate_set(port, qid, (port-4)*8);
        }

        rtk_qos_queueNum_set(logic_port, 1);
        rtk_qos_schedulingQueue_set(logic_port,&qweights);
        rtk_rate_egrQueueBwCtrlEnable_set(logic_port,RTK_WHOLE_SYSTEM,DISABLED);
        rtl83xx_qosFlushMeter(port, wanPortMask);
    }

    //close 8367 cpu port flow control
    return 0;

}

int rtl83xx_qosProcessQueue(unsigned int memberPort, unsigned int queueNum, unsigned int *queueFlag, unsigned int *queueId, unsigned int *queueBw, unsigned int *queueWeight, unsigned int wanPortMask)
{
    unsigned int port, queue, qid, meterId, logic_port;
    rtk_qos_queue_weights_t qweights;
    unsigned int set_error=0;
    unsigned int all_phyPortMask = vutpportmask;

    memset(&qweights, 0, sizeof(qweights));

    for(port = 0; port < MAX_PHY_PORT_NUM; port++)
    {
        if(((1<<port)&memberPort)==0)
            continue;
		
		logic_port = rtk_switch_port_P2L_get(port);
		//#if defined(CONFIG_TPLINK_QOS)
		//rtk_qos_queueNum_set(logic_port, 8);
		//#else
        rtk_qos_queueNum_set(logic_port, queueNum);
		//#endif
        rtk_rate_egrQueueBwCtrlEnable_set(logic_port,RTK_WHOLE_SYSTEM, ENABLED);

		

        for (queue=0; queue<MAX_RTL83XX_QOS_QUEUE_NUM; queue++)
        {
            if((queueFlag[queue]&QOS_VALID_MASK)==0 || queueBw[queue] == 0)
                continue;

            qid = queueId[queue];
            if((queueFlag[queue]&QOS_TYPE_MASK)==QOS_TYPE_STR)
            {
                qweights.weights[qid] = 0;
            }
            else
            {
                qweights.weights[qid] = queueWeight[queue];
                if(qweights.weights[qid] > QOS_WEIGHT_MAX)
                    qweights.weights[qid] = QOS_WEIGHT_MAX;
            }

            if(rtl83xx_qosGetMeterId(port, queueBw[queue], wanPortMask, &meterId) != 0)
            {
                set_error = 1;
            }
            else
            {
                rtk_rate_egrQueueBwCtrlRate_set(logic_port,qid,meterId);
            }
        }
        rtk_qos_schedulingQueue_set(logic_port,&qweights);
    }
    if(set_error)
    {
        printk("rtl83xx_qosGetMeterId failed!\n");
        rtl83xx_qosFlushBandwidth(all_phyPortMask);
        rtl83xx_closeQos(all_phyPortMask, wanPortMask);
    }
    return 0;
}

void rtl83xx_setFlowControl(int qosEnable)
{
    /* Set external interface 0 to RGMII with Force mode, 1000M, Full-duple, enable TX&RX pause*/
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode ;

    rtk_port_macForceLinkExt_get(r8367_cpu_port,&mode,&mac_cfg);
    if(((mac_cfg.txpause==ENABLED) || (mac_cfg.rxpause==ENABLED)) && qosEnable==1)
    {
        mac_cfg.txpause = DISABLED;
        mac_cfg.rxpause = DISABLED;
        rtk_port_macForceLinkExt_set(r8367_cpu_port,mode,&mac_cfg);
    }
    else if(((mac_cfg.txpause==DISABLED) || (mac_cfg.rxpause==DISABLED)) && qosEnable==0)
    {
        mac_cfg.txpause = ENABLED;
        mac_cfg.rxpause = ENABLED;
        rtk_port_macForceLinkExt_set(r8367_cpu_port,mode,&mac_cfg);
    }
}

int rtl83xx_qosFlushPortBasePri(void)
{
    rtk_port_t port, logic_port;
    int ret=1;
    for(port=0; port<=RTK_PORT_ID_MAX; port++)
    {
    	logic_port = rtk_switch_port_P2L_get(port);
        if(rtk_qos_portPri_set(logic_port, 0) != RT_ERR_OK)
            ret = 0;
    }
    return ret;
}
int rtl83xx_qosFlushDscpBasePri(void)
{
    rtk_dscp_t dscp;
    int ret=1;
    for(dscp=0; dscp<=RTK_PORT_ID_MAX; dscp++)
    {
        if(rtk_qos_dscpPriRemap_set(dscp, 0) != RT_ERR_OK)
            ret = 0;
    }
    return ret;
}
int rtl83xx_qosFlush1pBasePri(void)
{
    rtk_pri_t dot1p_pri;
    int ret=1;
    for(dot1p_pri=0; dot1p_pri<=RTK_PRIMAX; dot1p_pri++)
    {
        if(rtk_qos_1pPriRemap_set(dot1p_pri, 0) != RT_ERR_OK)
            ret = 0;
    }
    return ret;
}

#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
int rtl83xx_qosFlushAclBasePri(unsigned int memberPort)
{
    rtl83xx_acl_rule_t compare_rule;
    if(memberPort==0xFFFFFFFF)
        rtl83xx_flushAclRulebyPrio(RTL83XX_ACL_PRIO_HW_BRIDGE_QOS);
    else
    {
        memset(&compare_rule, 0, sizeof(rtl83xx_acl_rule_t));
        compare_rule.prio = RTL83XX_ACL_PRIO_HW_BRIDGE_QOS;
        compare_rule.member = memberPort;
        rtl83xx_flushAclRulebyFlag((COMPARE_FLAG_PRIO|COMPARE_FLAG_MEMBER), &compare_rule);
    }
    rtl83xx_syncAclTblToAsic();

    return 0;
}
#endif

int rtl83xx_qosRestorePriDecision(unsigned int flag)
{
   	rtk_priority_select_t pri_sel;
	
    if(flag & (1<<PORT_BASE))
    {
    	if(chip_type==CHIP_RTL8367C)
			pri_sel.port_pri= g_priorityDecision_67C[QOS_PRIDEC_PORT];
		else
			pri_sel.port_pri= g_priorityDecision_67D[QOS_PRIDEC_PORT];
    }

    if(flag & (1<<ACL_BASE))
    {
    	if(chip_type==CHIP_RTL8367C)
			pri_sel.acl_pri= g_priorityDecision_67C[QOS_PRIDEC_ACL];
		else
			pri_sel.acl_pri= g_priorityDecision_67D[QOS_PRIDEC_ACL];
    }

    if(flag & (1<<DSCP_BASE))
    {
    	if(chip_type==CHIP_RTL8367C)
			pri_sel.dscp_pri= g_priorityDecision_67C[QOS_PRIDEC_DSCP];
		else
			pri_sel.dscp_pri= g_priorityDecision_67D[QOS_PRIDEC_DSCP];
    }

    if(flag & (1<<D1P_BASE))
    {
    	if(chip_type==CHIP_RTL8367C)
			pri_sel.dot1q_pri= g_priorityDecision_67C[QOS_PRIDEC_1Q];
		else
			pri_sel.dot1q_pri= g_priorityDecision_67D[QOS_PRIDEC_1Q];
    }

	rtk_qos_priSel_set(PRIDECTBL_IDX0,&pri_sel);	
	rtk_qos_priSel_set(PRIDECTBL_IDX1,&pri_sel);

    return 0;
}

int rtl83xx_qosSetPriDecision(unsigned int priSrc, unsigned int decisionPri)
{
    int retVal;
	rtk_priority_select_t pri_sel;

    if((chip_type==CHIP_RTL8367D) && (priSrc < QOS_PRIDEC_CVLAN) && (decisionPri == 0))
		return RT_ERR_QOS_SEL_PRI_SOURCE;
    else if((chip_type==CHIP_RTL8367C) && (decisionPri == 0))
        return RT_ERR_QOS_SEL_PRI_SOURCE;

	memset(&pri_sel,0,sizeof(pri_sel));

	switch(priSrc){
		case QOS_PRIDEC_PORT:
			pri_sel.port_pri=decisionPri;
			break;
		case QOS_PRIDEC_ACL:
			pri_sel.acl_pri=decisionPri;
			break;
		case QOS_PRIDEC_DSCP:
			pri_sel.dscp_pri=decisionPri;
			break;
		case QOS_PRIDEC_1Q:
			pri_sel.dot1q_pri=decisionPri;
			break;
		case QOS_PRIDEC_1AD:
			pri_sel.svlan_pri=decisionPri;
			break;
		case QOS_PRIDEC_CVLAN:
			pri_sel.cvlan_pri=decisionPri;
			break;
		case QOS_PRIDEC_DA:
			pri_sel.dmac_pri=decisionPri;
			break;
		case QOS_PRIDEC_SA:
			pri_sel.smac_pri=decisionPri;
			break;
		default:
			break;
	}

    if((retVal = rtk_qos_priSel_set(PRIDECTBL_IDX0, &pri_sel)) != RT_ERR_OK)
    {
        return retVal;
    }

    return rtk_qos_priSel_set(PRIDECTBL_IDX1, &pri_sel);
}

int rtl83xx_qosGetPriDecisionAll(unsigned int priDec[], unsigned int len)
{
    int retVal = RT_ERR_FAILED;
	rtk_priority_select_t pri_sel;

    if(priDec == NULL || len != QOS_PRIDEC_END)
        return retVal;

	if((retVal=rtk_qos_priSel_get(PRIDECTBL_IDX0,&pri_sel)) != RT_ERR_OK)
		return retVal;

	priDec[QOS_PRIDEC_PORT]	=pri_sel.port_pri;
	priDec[QOS_PRIDEC_ACL]	=pri_sel.acl_pri;
	priDec[QOS_PRIDEC_DSCP]	=pri_sel.dscp_pri;
	priDec[QOS_PRIDEC_1Q]	=pri_sel.dot1q_pri;
	priDec[QOS_PRIDEC_1AD]	=pri_sel.svlan_pri;
	priDec[QOS_PRIDEC_CVLAN]=pri_sel.cvlan_pri;
	priDec[QOS_PRIDEC_DA]	=pri_sel.dmac_pri;
	priDec[QOS_PRIDEC_SA]	=pri_sel.smac_pri;

    return RT_ERR_OK;
}

#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
int rtl83xx_qosSetAclBasePri(rtl83xx_acl_rule_t *acl_rule)
{
    int retVal;

    acl_rule->prio = RTL83XX_ACL_PRIO_HW_BRIDGE_QOS;
    if((retVal = rtl83xx_addAclRule(acl_rule)) != 0)
    {
        printk("[%s:%d] rtl83xx_addAclRule error, %d", __FUNCTION__, __LINE__, retVal);
        return retVal;
    }
    if((retVal = rtl83xx_syncAclTblToAsic()) != 0)
    {
        printk("[%s:%d] rtl83xx_syncAclTblToAsic error, %d", __FUNCTION__, __LINE__, retVal);
        return retVal;
    }
    return retVal;
}
#endif

int rtl83xx_qosSetPortBasePri(int port, int prio)
{
    int retVal;
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return RT_ERR_FAILED;
    }

    if((retVal = rtk_qos_portPri_set(logic_port, prio)) != RT_ERR_OK)
        return retVal;

    return retVal;
}

int rtl83xx_qosGetPortBasePri(unsigned int port, unsigned int *prio)
{
    int retVal = RT_ERR_FAILED;
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return retVal;
    }

    if(prio == NULL)
        return retVal;

    retVal = rtk_qos_portPri_get(logic_port, prio);
    return retVal;
}

int rtl83xx_qosSetDscpBasePri(int dscp, int prio)
{
    int retVal;

    if((retVal = rtk_qos_dscpPriRemap_set(dscp, prio)) != RT_ERR_OK)
        return retVal;

    return retVal;
}

int rtl83xx_qosGetDscpBasePri(unsigned int dscp, unsigned int *prio)
{
    int retVal = RT_ERR_FAILED;
    if(prio == NULL)
        return retVal;

    retVal = rtk_qos_dscpPriRemap_get(dscp, prio);
    return retVal;
}


int rtl83xx_qosSet1pBasePri(int dot1p_pri, int prio)
{
    int retVal;

    if((retVal = rtk_qos_1pPriRemap_set(dot1p_pri, prio)) != RT_ERR_OK)
        return retVal;

    return retVal;
}

int rtl83xx_qosGet1pBasePri(unsigned int dot1p_pri, unsigned int *prio)
{
    int retVal = RT_ERR_FAILED;
    if(prio == NULL)
        return retVal;

    retVal= rtk_qos_1pPriRemap_get(dot1p_pri, prio);
    return retVal;
}

int rtl83xx_qosSetQueueNum(unsigned int port, unsigned int queue_num)
{
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return RT_ERR_FAILED;
    }

    return rtk_qos_queueNum_set(logic_port, queue_num);
}

int rtl83xx_qosGetQueueNum(unsigned int port, unsigned int *queue_num)
{
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return RT_ERR_FAILED;
    }

    return rtk_qos_queueNum_get(logic_port, queue_num);
}

int rtl83xx_qosSetQueueType(unsigned int port, unsigned int queue_id, unsigned int weight)
{
    int retVal;
	rtk_qos_queue_weights_t Qweights;

	Qweights.weights[queue_id]=weight;

	retVal=rtk_qos_schedulingQueue_set(port,&Qweights);

    return retVal;
}

int rtl83xx_qosGetQueueType(unsigned int port, unsigned int queue_id, unsigned int *weight)
{
    int retVal = RT_ERR_FAILED;
	rtk_qos_queue_weights_t Qweights;

    if(weight == NULL)
        return retVal;

    if((retVal=rtk_qos_schedulingQueue_get(port,&Qweights)) != RT_ERR_OK)
        return retVal;

   *weight=Qweights.weights[queue_id];

    return RT_ERR_OK;
}

int rtl83xx_qosSetPriToQid(unsigned int queue_num, unsigned int pri, unsigned int qid)
{
	rtk_qos_pri2queue_t Pri2qid;

	memset(&Pri2qid,0,sizeof(Pri2qid));
	Pri2qid.pri2queue[pri]=qid;
	
    return rtk_qos_priMap_set(queue_num, &Pri2qid);
}

int rtl83xx_qosGetPriToQid(unsigned int queue_num, unsigned int pri, unsigned int *qid)
{
	rtk_qos_pri2queue_t Pri2qid;
	int retVal;

	if((retVal=rtk_qos_priMap_get(queue_num,&Pri2qid)) != RT_ERR_OK)
		return retVal;

	*qid=Pri2qid.pri2queue[pri];
	
    return RT_ERR_OK;
}

int rtl83xx_qosSetQueueRate(int port, int portmask, int queue_id, int bandwidth)
{
    int retVal, meterId;
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return RT_ERR_FAILED;
    }

    if((retVal = rtl83xx_qosGetMeterId(port, bandwidth, portmask, &meterId)) != RT_ERR_OK)
    {
        return retVal;
    }

    if((retVal = rtk_rate_egrQueueBwCtrlEnable_set(logic_port, 0xFF, ENABLED)) != RT_ERR_OK)
        return retVal;

    retVal = rtk_rate_egrQueueBwCtrlRate_set(logic_port, queue_id, meterId);
    return retVal;
}

int rtl83xx_qosGetQueueBwCtrl(unsigned int port, unsigned int *enable)
{
    int retVal = RT_ERR_FAILED;
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return retVal;
    }

    if(enable == NULL)
        return retVal;

    retVal = rtk_rate_egrQueueBwCtrlEnable_get(logic_port, RTK_WHOLE_SYSTEM, enable);
    return retVal;
}

int rtl83xx_qosGetQueueRate(unsigned int port, unsigned int queue_id, unsigned int *enable)
{
    int retVal = RT_ERR_FAILED;
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return retVal;
    }

    if(enable == NULL)
        return retVal;

    retVal = rtk_rate_egrQueueBwCtrlEnable_get(logic_port, RTK_WHOLE_SYSTEM, enable);
    return retVal;
}

int rtl83xx_qosSetPortBandwidth(unsigned int port, unsigned int bandwidth)
{
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return RT_ERR_FAILED;
    }

    return rtk_rate_egrBandwidthCtrlRate_set(logic_port, bandwidth, ENABLED);
}

int rtl83xx_qosGetPortBandwidth(unsigned int port, unsigned int *bandwidth)
{
    unsigned int ifg_include;
    int retVal = RT_ERR_FAILED;
    unsigned int logic_port = rtk_switch_port_P2L_get(port);
    if(logic_port == UNDEFINE_PORT)
    {
        return retVal;
    }

    if(bandwidth == NULL)
        return retVal;

    retVal = rtk_rate_egrBandwidthCtrlRate_get(logic_port, bandwidth, &ifg_include);
    return retVal;
}
#endif
#endif

#if defined(CONFIG_RTL_VLAN_8021Q) || defined(CONFIG_RTL_HW_VLAN_SUPPORT) || defined(CONFIG_RTL_ISP_MULTI_WAN_SUPPORT)
int rtl865x_enableRtl83xxBCMCToCpu(unsigned int acl_idx)
{
#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
    rtl83xx_acl_rule_t aclRule;
    int retVal;

    memset(&aclRule, 0, sizeof(rtl83xx_acl_rule_t));
    aclRule.member = vutpportmask;
    aclRule.prio = RTL83XX_ACL_PRIO_MCBC_TO_CPU;
    aclRule.filter.dmac.octet[0] = 0x01;
    aclRule.filter.dmac_mask.octet[0] = 0x01;
    aclRule.action.act_type = RTL83XX_ACL_TRAP_CPU;

    rtl83xx_addAclRule(&aclRule);
    retVal = rtl83xx_syncAclTblToAsic();

    //printk("%s %d BC MC to cpu !\n", __func__, __LINE__);
    return retVal;
#else
    int retVal;
    rtk_filter_field_t  filter_field[2];
    rtk_filter_cfg_t    cfg;
    rtk_filter_action_t act;
    rtk_filter_number_t ruleNum = 0;


    memset(filter_field, 0, 2*sizeof(rtk_filter_field_t));
    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));

    filter_field[0].fieldType = FILTER_FIELD_DMAC;
    filter_field[0].filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
    filter_field[0].filter_pattern_union.dmac.value.octet[0] = 0x01;

    filter_field[0].filter_pattern_union.dmac.mask.octet[0] = 0x1;
    filter_field[0].next = NULL;
    if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field[0])) != RT_ERR_OK)
        return retVal;


    cfg.activeport.value.bits[0] = vutpportmask;
    cfg.activeport.mask.bits[0] = vportmask;
    cfg.invert = FALSE;

    act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;

    if ((retVal = rtk_filter_igrAcl_cfg_add(1, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return retVal;

    //printk("%s %d BC MC to cpu !\n", __func__, __LINE__);
    return RT_ERR_OK;
#endif
}

int rtl865x_disableRtl83xxBCMCToCpu(unsigned int acl_idx)
{
    int ret = -1;

#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
    rtl83xx_flushAclRulebyPrio(RTL83XX_ACL_PRIO_MCBC_TO_CPU);
    ret = rtl83xx_syncAclTblToAsic();
#else
    ret = rtk_filter_igrAcl_cfg_del(acl_idx);
#endif

    return ret;
}

int rtl865x_enableRtl83xxUCToCpu(unsigned int acl_idx)
{
#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
    rtl83xx_acl_rule_t aclRule;
    int retVal;

    memset(&aclRule, 0, sizeof(rtl83xx_acl_rule_t));
    aclRule.member = vutpportmask;
    aclRule.prio = RTL83XX_ACL_PRIO_UC_TO_CPU;
    aclRule.filter.dmac.octet[0] = 0x00;
    aclRule.filter.dmac_mask.octet[0] = 0x01;
    aclRule.action.act_type = RTL83XX_ACL_TRAP_CPU;

    rtl83xx_addAclRule(&aclRule);
    retVal = rtl83xx_syncAclTblToAsic();
    return retVal;
#else
    int retVal;
    rtk_filter_field_t	filter_field[2];
    rtk_filter_cfg_t	cfg;
    rtk_filter_action_t act;
    rtk_filter_number_t ruleNum = 0;


    memset(filter_field, 0, 2*sizeof(rtk_filter_field_t));
    memset(&cfg, 0, sizeof(rtk_filter_cfg_t));
    memset(&act, 0, sizeof(rtk_filter_action_t));

    filter_field[0].fieldType = FILTER_FIELD_DMAC;
    filter_field[0].filter_pattern_union.dmac.dataType = FILTER_FIELD_DATA_MASK;
    filter_field[0].filter_pattern_union.dmac.value.octet[0] = 0x00;

    filter_field[0].filter_pattern_union.dmac.mask.octet[0] = 0x1;
    filter_field[0].next = NULL;
    if ((retVal = rtk_filter_igrAcl_field_add(&cfg, &filter_field[0])) != RT_ERR_OK)
        return retVal;


    //cfg.activeport.dataType = FILTER_FIELD_DATA_MASK;
    cfg.activeport.value.bits[0] = vutpportmask;
    cfg.activeport.mask.bits[0] = vportmask;
    cfg.invert = FALSE;

    act.actEnable[FILTER_ENACT_TRAP_CPU] = TRUE;

    if ((retVal = rtk_filter_igrAcl_cfg_add(acl_idx, &cfg, &act, &ruleNum)) != RT_ERR_OK)
        return retVal;
    //printk("%s %d UC to cpu !\n", __func__, __LINE__);
    return RT_ERR_OK;
#endif
}

int rtl865x_disableRtl83xxUCToCpu(unsigned int acl_idx)
{
    int ret = -1;
#if defined(CONFIG_RTL_83XX_ACL_SUPPORT)
    rtl83xx_flushAclRulebyPrio(RTL83XX_ACL_PRIO_UC_TO_CPU);
    ret = rtl83xx_syncAclTblToAsic();
#else
    ret = rtk_filter_igrAcl_cfg_del(acl_idx);
#endif
    return ret;
}
#endif

int rtl865x_enableRtl83xxDropVlan(unsigned int portmask)
{
    rtl83xx_acl_rule_t aclRule;
    int retVal;

    memset(&aclRule, 0, sizeof(rtl83xx_acl_rule_t));
    aclRule.member = portmask;
    aclRule.prio = -30000;
    aclRule.action.act_type = RTL83XX_ACL_DROP;

  aclRule.careTag = (1<<CARE_TAG_CTAG);

    rtl83xx_addAclRule(&aclRule);

    retVal = rtl83xx_syncAclTblToAsic();

  //printk("[%s:%d]retVal %d careTag %x\n",__FUNCTION__,__LINE__,retVal,aclRule.careTag);
  
    return retVal;
}

int rtl865x_disableRtl83xxDropVlan(void)
{
    int retVal = -1;

    rtl83xx_flushAclRulebyPrio(-30000);
    retVal = rtl83xx_syncAclTblToAsic();

    return retVal;
}

void rtl_get_83xx_snr(void)
{
    int i, phy_id;
    unsigned long flags;
    rtk_port_mac_ability_t sts;
    unsigned int sum_snr_a = 0, sum_snr_b = 0, sum_snr_c = 0, sum_snr_d = 0, snr_tmp=0;

    local_irq_save(flags);
    for (phy_id=0; phy_id <=RTK_PHY_ID_MAX; phy_id++)
    {
        memset(&sts, 0, sizeof(rtk_port_mac_ability_t));
        if ((rtk_port_macStatus_get(phy_id, &sts)==0) && (sts.link==1))
        {
            for (i=0; i<10; i++)
            {
                if (rtk_port_phyOCPReg_get(phy_id, 0xA8C0, &snr_tmp) == RT_ERR_OK)
                {
                    sum_snr_a += snr_tmp;
                }
                else
                {
                    printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
                    local_irq_restore(flags);
                    return;
                }

                if (rtk_port_phyOCPReg_get(phy_id, 0xA9C0, &snr_tmp) == RT_ERR_OK)
                {
                    sum_snr_b += snr_tmp;
                }
                else
                {
                    printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
                    local_irq_restore(flags);
                    return;
                }

                if (rtk_port_phyOCPReg_get(phy_id, 0xAAC0, &snr_tmp) == RT_ERR_OK)
                {
                    sum_snr_c += snr_tmp;
                }
                else
                {
                    printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
                    local_irq_restore(flags);
                    return;
                }

                if (rtk_port_phyOCPReg_get(phy_id, 0xABC0, &snr_tmp) == RT_ERR_OK)
                {
                    sum_snr_d += snr_tmp;
                }
                else
                {
                    printk("%s[%d], rtl8367b_getAsicPHYOCPReg() Failed\n", __FUNCTION__, __LINE__);
                    local_irq_restore(flags);
                    return;
                }
            }

            sum_snr_a = sum_snr_a/10;
            sum_snr_b = sum_snr_b/10;
            sum_snr_c = sum_snr_c/10;
            sum_snr_d = sum_snr_d/10;

            printk("Port[%d] link speed is %s, CH_A_SNR is %d, CH_B_SNR is %d, CH_C_SNR is %d, CH_D_SNR is %d\n",
                   phy_id, (sts.speed)==0x1?"100M":((sts.speed)==0x2?"1G": ((sts.speed) ==0x0?"10M":"Unkown")),
                   sum_snr_a, sum_snr_b, sum_snr_c, sum_snr_d);
        }
    }
    printk("\n## NOTE: snr value translating to dB value is by: -(10 * log10(snr/pow(2,18)))\n");

    local_irq_restore(flags);
    return;
}

int rtl83xx_storm_controlRate_set(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t rate, rtk_enable_t ifg_include)
{
    rtk_api_ret_t retVal;
    rtk_uint32 enable, index;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (storm_type >= STORM_GROUP_END)
        return RT_ERR_SFC_UNKNOWN_GROUP;

    if (rate > RTK_QOS_RATE_INPUT_MAX || rate < RTK_QOS_RATE_INPUT_MIN || (rate % RTK_QOS_GRANULARTY_UNIT_KBPS))
        return RT_ERR_RATE ;

    if (RTK_QOS_RATE_INPUT_MAX == rate)
        enable = FALSE;
    else {
        enable = TRUE;
    }

	if ((retVal = rtk_rate_stormControlPortEnable_set(port,storm_type,enable)) != RT_ERR_OK)
           return retVal;

	if(enable){
		switch (storm_type)
	    {
	    case STORM_GROUP_UNKNOWN_UNICAST:
			index = STORM_UNUC_INDEX;
	        break;
	    case STORM_GROUP_UNKNOWN_MULTICAST:
			index = STORM_UNMC_INDEX;
	        break;
	    case STORM_GROUP_MULTICAST:
			index = STORM_MC_INDEX;
	        break;
	    case STORM_GROUP_BROADCAST:
			index = STORM_BC_INDEX;
			break;
	    default:
	        break;
	    }

		if ((retVal = rtk_rate_stormControlMeterIdx_set(port,storm_type,index)) != RT_ERR_OK)
           return retVal;

		retVal = rtk_rate_shareMeter_set(index,METER_TYPE_KBPS,rate,ifg_include);

		return retVal;
	}

    return RT_ERR_OK;
}

int rtl83xx_storm_controlRate_get(rtk_port_t port, rtk_rate_storm_group_t storm_type, rtk_rate_t *pRate, rtk_enable_t *pIfg_include)
{
    rtk_api_ret_t retVal;
    rtk_uint32 enable;
    rtk_uint32 index, type;

    if (port > RTK_PORT_ID_MAX)
        return RT_ERR_PORT_ID;

    if (storm_type >= STORM_GROUP_END) {
        return RT_ERR_SFC_UNKNOWN_GROUP;
    }
	if((retVal=rtk_rate_stormControlPortEnable_get(port,storm_type,&enable)) != RT_ERR_OK)
		return retVal;

	if(enable){
		if((retVal=rtk_rate_stormControlMeterIdx_get(port,storm_type,&index)) != RT_ERR_OK)
			return retVal;

		retVal=rtk_rate_shareMeter_get(index,&type,pRate,pIfg_include);

		return retVal;
	}

    return RT_ERR_OK;
}

#if defined CONFIG_RTL_PROC_NEW
int rtl_83xx_share_meter_read_proc(struct seq_file *s, void *v)
{
    unsigned int i, rate, ifg, b_size, type;
    int retVal;

    seq_printf(s,"rtl83xx share meter parameters:\n\n");
    for(i = 0; i <= RTK_MAX_METER_ID; i++)
    {
        if((retVal = rtk_rate_shareMeter_get(i, &type, &rate, &ifg)) != RT_ERR_OK)
        {
            printk("[%s:%d] share meter %d get failure!\n", __FUNCTION__, __LINE__, i);
        }

        if((retVal = rtk_rate_shareMeterBucket_get(i, &b_size)) != RT_ERR_OK)
        {
            printk("[%s:%d] share meter %d bucket get failure!\n", __FUNCTION__, __LINE__, i);
        }

        seq_printf(s,"[%d] type: %s, rate: %d, %s ifg, bucket size: %d\n", i,
                   ((type == METER_TYPE_PPS) ? "pps" : "bps"), rate, (ifg>0 ? "include" : "exclude"), b_size);
    }

    return 0;
}
#endif

#if defined CONFIG_RTL_PROC_NEW
int rtl_83xx_storm_ctrl_read_proc(struct seq_file *s, void *v)
{
    unsigned int port, logic_port, rate, pIfg_include;

    seq_printf(s, "rtl83xx storm control:\n");
    for(port = 0; port < RTK_MAX_NUM_OF_PORT; port++)
    {
        logic_port = rtk_switch_port_P2L_get(port);
        if(logic_port == UNDEFINE_PORT)
        {
            continue;
        }

        if(rtl83xx_storm_controlRate_get(logic_port, STORM_GROUP_UNKNOWN_UNICAST, &rate, &pIfg_include) == RT_ERR_OK)
        {
            seq_printf(s, "\n	port%d, %s unknown unicast, meter: %d, rate=%d kbps\n",
                       port, ((rate==(0x1FFFF<<3)) ? "disable" : "enable"), STORM_UNUC_INDEX, rate);
        }

        if(rtl83xx_storm_controlRate_get(logic_port, STORM_GROUP_UNKNOWN_MULTICAST, &rate, &pIfg_include) == RT_ERR_OK)
        {
            seq_printf(s, "	port%d, %s unknown multicast, meter: %d, rate=%d kbps\n",
                       port, ((rate==(0x1FFFF<<3)) ? "disable" : "enable"), STORM_UNMC_INDEX, rate);
        }

        if(rtl83xx_storm_controlRate_get(logic_port, STORM_GROUP_MULTICAST, &rate, &pIfg_include) == RT_ERR_OK)
        {
            seq_printf(s, "	port%d, %s multicast, meter: %d, rate=%d kbps\n",
                       port, ((rate==(0x1FFFF<<3)) ? "disable" : "enable"), STORM_MC_INDEX, rate);
        }

        if(rtl83xx_storm_controlRate_get(logic_port, STORM_GROUP_BROADCAST, &rate, &pIfg_include) == RT_ERR_OK)
        {
            seq_printf(s, "	port%d, %s broadcast, meter: %d, rate=%d kbps\n",
                       port, ((rate==(0x1FFFF<<3)) ? "disable" : "enable"), STORM_BC_INDEX, rate);
        }
    }

    return 0;
}
#endif

int rtl83xx_set_storm_control(unsigned int type, unsigned int rate)
{
    unsigned int port, logic_port;

    for(port = 0; port < RTK_MAX_NUM_OF_PORT; port++)
    {
        logic_port = rtk_switch_port_P2L_get(port);
        if(logic_port == UNDEFINE_PORT)
        {
            continue;
        }

        if(rtl83xx_storm_controlRate_set(port, type, rate, 1) != RT_ERR_OK)
            return -1;
    }

    return 0;
}

int rtl_83xx_storm_ctrl_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char tmpBuf[64], strType[16];
    unsigned int type, rate, num;

    if(count > 63)
    {
        goto ESC_ERROR;
    }

    if (buffer && !copy_from_user(tmpBuf, buffer, count))
    {
        tmpBuf[count-1]='\0';
        num = sscanf(tmpBuf, "%s %d", strType, &rate);
        if (num != 2)
        {
            goto ESC_ERROR;
        }

        if(strcmp(strType, "uuc") == 0)
        {
            type = 0;
        }
        else if(strcmp(strType, "umc") == 0)
        {
            type = 1;
        }
        else if(strcmp(strType, "mc") == 0)
        {
            type = 2;
        }
        else if(strcmp(strType, "bc") == 0)
        {
            type = 3;
        }
        else
        {
            goto ESC_ERROR;
        }

        if(rtl83xx_set_storm_control(type, rate) != 0)
            goto ESC_ERROR;

        return count;
    }

ESC_ERROR:
    printk("Invalid params. \n");
    return -EFAULT;
}

rtk_int32 rtl_mirror_portBased_set(rtk_uint32 mirroring_port, rtk_uint32 Mirrored_rx_portmask, rtk_uint32 Mirrored_tx_portmask)
{
    rtk_portmask_t pMirrored_rx_portmask;
    rtk_portmask_t pMirrored_tx_portmask;
    rtk_uint32 i;
    rtk_uint32 mirroring_port_tmp;

    memset(&pMirrored_rx_portmask,0,sizeof(rtk_portmask_t));
    memset(&pMirrored_tx_portmask,0,sizeof(rtk_portmask_t));
    pMirrored_rx_portmask.bits[0] = Mirrored_rx_portmask;
    pMirrored_tx_portmask.bits[0] = Mirrored_tx_portmask;
    for (i = 0; i< RTK_MAX_NUM_OF_PORT; i++)
    {
        if (mirroring_port&(1<<i))
        {
            mirroring_port_tmp = i;
            break;
        }
        else
        {
            mirroring_port_tmp = RTK_PORT_ID_MAX+1;
        }
    }
    return rtk_mirror_portBased_set(mirroring_port_tmp,&pMirrored_rx_portmask,&pMirrored_tx_portmask);
}

rtk_int32 rtl_mirror_portBased_get(rtk_uint32 *mirroring_port, rtk_uint32 *Mirrored_rx_portmask, rtk_uint32 *Mirrored_tx_portmask)
{
    rtk_portmask_t pMirrored_rx_portmask;
    rtk_portmask_t pMirrored_tx_portmask;
    rtk_int32 ret;

    memset(&pMirrored_rx_portmask,0,sizeof(rtk_portmask_t));
    memset(&pMirrored_tx_portmask,0,sizeof(rtk_portmask_t));
    ret=rtk_mirror_portBased_get(mirroring_port,&pMirrored_rx_portmask,&pMirrored_tx_portmask);
    *Mirrored_rx_portmask = pMirrored_rx_portmask.bits[0];
    *Mirrored_tx_portmask = pMirrored_tx_portmask.bits[0];
    *mirroring_port=1<<(*mirroring_port);
    return ret;
}

rtk_int32 rtl_mirror_portIso_set(rtk_uint32 isolation)
{
    if(isolation == 0)
        return rtk_mirror_portIso_set(DISABLED);
    else
        return rtk_mirror_portIso_set(ENABLED);
}

rtk_int32 rtl_mirror_portIso_get(rtk_uint32 *isolation)
{
    rtk_enable_t pEnable;
    rtk_int32 ret;

    ret = rtk_mirror_portIso_get(&pEnable);
    *isolation = pEnable;
    return ret;
}

int rtl_port_isolation_leak_set(bool enable)
{
    return rtk_mirror_isolationLeaky_set(enable,enable);
}

int rtl83xx_set_port_flow_control(unsigned int port,unsigned int enable)
{
	int ret=RT_ERR_FAILED;
	rtk_port_phy_ability_t phyAbility;
	unsigned int logic_port;

    memset(&phyAbility, 0, sizeof(rtk_port_phy_ability_t));
	
	logic_port = rtk_switch_port_P2L_get(port);
	
	ret=rtk_port_phyAutoNegoAbility_get(logic_port, &phyAbility);
	if(ret != RT_ERR_OK)
		return RT_ERR_FAILED;

	phyAbility.FC=enable;
	phyAbility.AsyFC=enable;

	ret=rtk_port_phyAutoNegoAbility_set(logic_port,&phyAbility);

	if(ret != RT_ERR_OK)
		return RT_ERR_FAILED;

	return RT_ERR_OK;
}

int rtl83xx_get_port_flow_control(unsigned int port,unsigned int *enable)
{
	int ret=RT_ERR_FAILED;
	rtk_port_phy_ability_t phyAbility;
	unsigned int logic_port;

	if(!enable)
		return ret;

    memset(&phyAbility, 0, sizeof(rtk_port_phy_ability_t));
	
	logic_port = rtk_switch_port_P2L_get(port);
	
	ret=rtk_port_phyAutoNegoAbility_get(logic_port, &phyAbility);
	if(ret != RT_ERR_OK){
		*enable=0xFF;
		return RT_ERR_FAILED;
	}

	*enable=phyAbility.FC;

	return RT_ERR_OK;	
}

#ifdef CONFIG_RTL_PROC_NEW
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <linux/fs.h>
#include <linux/proc_fs.h>
extern struct proc_dir_entry proc_root;
#endif
static int rtl_flow_control_read(struct seq_file *s, void *v)
{
	int port,enable;

	for(port=0; port<=4;port++){
		rtl83xx_get_port_flow_control(port,&enable);
		seq_printf(s,"port %d FC %d\n",port,enable);
	}
	
	return 0;	
}

int rtl_flow_control_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, rtl_flow_control_read,NULL));
}

//echo port 1 enable 0 >/proc/rtl_83xx_flow_control
static int rtl_flow_control_write( struct file *filp, const char *buff,unsigned long len, void *data )
{
	
	char tmpbuf[512];
	char *strptr;
	char *cmdptr;
	unsigned int port,enable;
	
	if (len>512) 
		goto errout;

	if (buff && !copy_from_user(tmpbuf, buff, len))
	{
		tmpbuf[len] = '\0';
		strptr=tmpbuf;
		if (strlen(strptr)==0) 
			goto errout;

		cmdptr = strsep(&strptr," ");
		if (cmdptr==NULL){ 
			printk("[%s:%d] strptr %s\n",__FUNCTION__,__LINE__,strptr);
			goto errout;
		}

		if(strncmp(cmdptr, "port",4) == 0){
			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL){ 
				printk("[%s:%d] strptr %s\n",__FUNCTION__,__LINE__,strptr);
				goto errout;
			}

			port=simple_strtol(cmdptr,NULL,0);

			cmdptr = strsep(&strptr," ");
			if (cmdptr==NULL){ 
				printk("[%s:%d] strptr %s\n",__FUNCTION__,__LINE__,strptr);
				goto errout;
			}
			
			if(strncmp(cmdptr, "enable",5) == 0){
				cmdptr = strsep(&strptr," ");
				if (cmdptr==NULL){ 
					printk("[%s:%d] strptr %s\n",__FUNCTION__,__LINE__,strptr);
					goto errout;
				}

				enable=simple_strtol(cmdptr,NULL,0);

				rtl83xx_set_port_flow_control(port,enable);
			}
			
		}
		else{
			goto errout;
		}	

		return len;
	}
errout:
	printk("error input\n");
	return -1;
}

static ssize_t rtl_flow_control_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	    return rtl_flow_control_write(file, userbuf,count, off);
}

struct file_operations rtl_flow_control_proc_fops = {
		.open		 = rtl_flow_control_single_open,
		.write 		 = rtl_flow_control_single_write,
		.read		 = seq_read,
		.llseek		 = seq_lseek,
		.release	 = single_release,
};

int rtl_initFlowControlProc(void)
{
	proc_create_data("rtl_83xx_flow_control",0,&proc_root,&rtl_flow_control_proc_fops,NULL);
							 
	return 0;
}
#endif

//API for port FC setting end