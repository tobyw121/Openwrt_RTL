/*
 * Copyright (C) 2020 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * Purpose : RTK switch high-level API for RTL83xx
 * Feature : Here is a list of all functions and variables in this module.
 * API source : Realtek_Unmanaged_Switch_API_V1.4.0_20200611
 *
 */
#include <linux/config.h>
#include <rtk_types.h>
#include <rtk_error.h>

#include <rtk_switch.h>
#include <led.h>
#include <port.h>
#include <cpu.h>
#include <l2.h>
#include <chip.h>
#include <rtl8367c_asicdrv_port.h>
#include <led.h>
#define BIT(nr)			(1UL << (nr))

#ifdef CONFIG_SW_8366SC
#define EXT_PORT_FOR_HOST EXT_PORT1
#define RTL83XX_WAN         3
#elif defined(CONFIG_SW_8363NB)
#define EXT_PORT_FOR_HOST EXT_PORT0
#define RTL83XX_WAN         3
#else //8367RB
#define EXT_PORT_FOR_HOST EXT_PORT0
//#define RTL83XX_WAN         4
/* MR60X WAN port 2, LAN port 3/4  */
#define RTL83XX_WAN        2
#endif	

#define SW_API(x) do { \
    if ((ret = x) != RT_ERR_OK) { \
        dprintf("Switch API %s failed, ret = %d\n", #x, ret); \
        goto fail; \
    } \
} while(0)

#define SW_API2(x) do { \
    if ((ret = x) != RT_ERR_OK) { \
        goto fail; \
    } \
} while(0)

#define TX_DELAY			0
#define RX_DELAY			2

/* purpose: both defined CONFIG_SW_8367R or defined CONFIG_SW_83XX cases can use \boot\rtl83xx_v1dot4 folder */
#ifdef CONFIG_SW_8367R
int RTL8367R_init(void) { return RTL83XX_init(); }
int rtl8367b_setAsicReg(rtk_uint32 reg, rtk_uint32 value) {
	return rtl8367c_setAsicReg(reg, value);
}
int rtl8367b_getAsicReg(rtk_uint32 reg, rtk_uint32 *pValue) {
	return rtl8367c_getAsicReg(reg, pValue);
}
#endif

/* purpose: one boot.bin can for both 97G+8367RB-VB and 97G+8367RB-VC board */
static unsigned int logical_cpu_port = EXT_PORT_FOR_HOST;
static unsigned int valid_utp_portmask;
static unsigned int chip_type;
const char chip_name[6][10] = {
    "8367C",
    "8370B",
    "8364B",
    "8363SC_VB",
    "8367D",
    "None"
};

int RTL83XX_init(void)
{
    rtk_port_mac_ability_t mac_cfg;
    rtk_mode_ext_t mode;
    rtk_portmask_t portmask;
    unsigned int ret, port;

    /* Initial Chip */
    SW_API2(rtk_switch_init());

    chip_type = rtk_switch_chipType_get();
    dprintf("Switch v1.4 API: switchChip= %s\n", chip_name[chip_type]);

    //#ifdef CONFIG_SW_8367R
    if (chip_type == CHIP_RTL8367D)
        logical_cpu_port = EXT_PORT1; // 8367RB-VC only has EXT_PORT1 (RGMII)
    //#endif

    SW_API(rtk_switch_utpPortMask_get(&portmask));
    valid_utp_portmask = portmask.bits[0];

#if 1
    portmask.bits[0] = (valid_utp_portmask | BIT(logical_cpu_port)) & ~(BIT(RTL83XX_WAN));
    for (port=0; port<5; port++) {
        if (portmask.bits[0] & (1<<port))
            SW_API(rtk_port_isolation_set(port, &portmask));
    }
    portmask.bits[0] = BIT(logical_cpu_port) | (BIT(RTL83XX_WAN));
    SW_API(rtk_port_isolation_set(RTL83XX_WAN, &portmask));
#endif

    /* Enable LED Group 0&2 from P0 to P4 */
//    SW_API(rtk_led_enable_set(LED_GROUP_0, &portmask));
//    SW_API(rtk_led_enable_set(LED_GROUP_2, &portmask));

    /* note: rtk_port_rgmiiDelayExt_set should be called before rtk_port_macForceLinkExt_set
             to avoid the rxc glitch */
    SW_API(rtk_port_rgmiiDelayExt_set(logical_cpu_port, TX_DELAY, RX_DELAY));

    /* Set logical_cpu_port to RGMII with Force mode, 1000M, Full-duplex, enable TX&RX pause */
    mode = MODE_EXT_RGMII ;
    mac_cfg.forcemode = MAC_FORCE;
    mac_cfg.speed = SPD_1000M;
    mac_cfg.duplex = FULL_DUPLEX;
    mac_cfg.link = PORT_LINKUP;
    mac_cfg.nway = DISABLED;
    mac_cfg.txpause = ENABLED;
    mac_cfg.rxpause = ENABLED;
    SW_API(rtk_port_macForceLinkExt_set(logical_cpu_port, mode, &mac_cfg));
 
    /* set logical_cpu_port as CPU port */
    SW_API(rtk_cpu_enable_set(ENABLED));
    SW_API(rtk_cpu_tagPort_set(logical_cpu_port, CPU_INSERT_TO_NONE));

    portmask.bits[0]=(valid_utp_portmask | BIT(logical_cpu_port));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNDA, &portmask));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_UNKNOWNMC, &portmask));
    SW_API(rtk_l2_floodPortMask_set(FLOOD_BC, &portmask));

#if 0
    portmask.bits[0] = (valid_utp_portmask | BIT(logical_cpu_port)) & ~(BIT(RTL83XX_WAN));
    for (port=0; port<5; port++) {
        if (portmask.bits[0] & (1<<port))
            SW_API(rtk_port_isolation_set(port, &portmask));
    }
    portmask.bits[0] = BIT(logical_cpu_port) | (BIT(RTL83XX_WAN));
    SW_API(rtk_port_isolation_set(RTL83XX_WAN, &portmask));
#endif

//    SW_API(rtk_led_groupConfig_set(LED_GROUP_0, LED_CONFIG_SPD1000ACT));
//    SW_API(rtk_led_groupConfig_set(LED_GROUP_2, LED_CONFIG_SPD10010ACT));
    SW_API(rtk_led_modeForce_set(0, LED_GROUP_0, LED_FORCE_OFF));
    SW_API(rtk_led_modeForce_set(1, LED_GROUP_0, LED_FORCE_OFF));
    SW_API(rtk_led_modeForce_set(2, LED_GROUP_0, LED_FORCE_OFF));

fail:
    return ret; 
}

