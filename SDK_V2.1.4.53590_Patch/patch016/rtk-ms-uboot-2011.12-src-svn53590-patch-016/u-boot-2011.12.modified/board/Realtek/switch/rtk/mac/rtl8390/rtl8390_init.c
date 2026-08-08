/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose :
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <common.h>
#include <rtk_debug.h>
#include <rtk_switch.h>
#include <rtk_type.h>
#include <rtk_reg.h>
#include <init.h>
#include <interrupt.h>

#include <rtk/mac/rtl8390/rtl8390_init.h>

#if defined(CONFIG_SOFTWARE_CONTROL_LED)
#include <rtk/drv/swled/swctrl_led_main.h>
#endif

#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
#include <rtk/drv/rtl8231/rtl8231_drv.h>
#include <rtk/mac/rtl8390/rtl8390_mdc_mdio.h>
#endif

#include <rtk/mac/rtl8390/rtl8390_swcore_reg.h>
#include <rtk/mac/rtl8390/rtl8390_drv.h>
#include <rtk/mac/rtl8390/rtl8390_rtk.h>

#if (defined(CONFIG_RTL8214) || defined(CONFIG_RTL8214F) || defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
#include <rtk/phy/rtl8214f.h>
#endif

#if (defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8218FB))
#include <rtk/phy/rtl8218b.h>
#endif

#if (defined(CONFIG_RTL8214FC))
#include <rtk/phy/rtl8214fc.h>
#endif

#if (defined(CONFIG_RTL8208))
#include <rtk/phy/rtl8208.h>
#endif

#if defined(CONFIG_RTL8218C)
#include <rtk/phy/rtl8218c.h>
#endif  /* defined(CONFIG_RTL8218C) */

/*
 * Symbol Definition
 */
typedef struct {
    unsigned int    reg;
    unsigned int    val;
} confcode_mac_regval_t;

typedef struct {
    unsigned int    reg;
    unsigned int    offset;
    unsigned int    endBit;
    unsigned int    startBit;
    unsigned int    val;
} confcode_serdes_patch_t;

#define QSGMII_MODE

#define MAX_SERDES      14

/*
 * Macro Definition
 */
#define MAC_REG_SET_CHK         rtl8390_macReg_setChk
#define SERDES_PATCH_SET        rtl8390_sds_patch_set
#define SERDES_PATCH_SET_CHK    rtl8390_sds_patch_setChk

/*
 * Data Declaration
 */
#if defined(CONFIG_RTL8396M_DEMO)
static int rtl8396_10gMediaType[2];
static uint32 chip10gMP = 0;
#define PHY_8390_10G_85_OHM
#endif  /* CONFIG_RTL8396M_DEMO */

/* RTL835x MAC configuration code */
static confcode_mac_regval_t rtl835x_mac_conf[] = {
    { 0x0ff4, 0xa0000000 },
    { 0x0008, 0x44444444 },
    { 0x000c, 0x664444 },
    { 0x02ac, 0x80000084 },
    { 0x0058, 0x15C },
    { 0x0014, 0x4 },
    { 0x02a8, 0x3c324f40 },
    { 0x60f8, 0x12972561 },
    { 0x1184, 0x7FFFFFFF },
    { 0x1188, 0xFFFFF800 },
    { 0x1180, 0x38000 },
    { 0x0ff0, 0x83536800 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_pwr_save[] = {
    { 0xa0e0, 0x400 },
    { 0xa1e0, 0x400 },
    };

static confcode_mac_regval_t rtl835x_mac_2G5_serdes[] = {
    { 0xa32c, 0x12AA9482 },
    { 0xa3ac, 0x12AA9482 },
    };

static confcode_mac_regval_t rtl835x_mac_2G5_serdes_10[] = {
    { 0xa32c, 0x120F9482 },
    { 0xa3ac, 0x120F9482 },
    };

static confcode_mac_regval_t rtl835x_mac_dis_25m_sdsck_out[] = {
    { 0xa30c, 0x81C5C1F5 },
    { 0xa70c, 0x81C5C1F5 },
    { 0xab0c, 0x81C5C1F5 },
    { 0xaf0c, 0x81C5C1F5 },
    { 0xb70c, 0x81C5C1F5 },
    { 0xb304, 0x7FFB5000 },
    { 0xb384, 0x7FFB5000 },
    { 0xbb04, 0x7FFB5000 },
    { 0xbb84, 0x7FFB5000 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_0[] = {
    { 0xa300, 0xf3c70000 },
    { 0xa304, 0x0A0B838E },
    { 0xa308, 0x42097211 },
    { 0xa310, 0x5CCC8C65 },
    { 0xa320, 0xC8DB0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0xC0C78C62 },
    { 0xa330, 0xB0020000 },
    { 0xa340, 0x10FF04AA },
    { 0xa000, 0x0F009403 },
    { 0xa004, 0x71067080 },
    { 0xa008, 0x8E0F0749 },
    { 0xa00c, 0x13598F5F },
    { 0xa010, 0x0000524B },
    { 0xa014, 0x00000CA4 },
    { 0xa018, 0x466408E4 },
    { 0xa01c, 0x00002053 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_8[] = {
    { 0xB300, 0x4000D800 },
    { 0xB304, 0x7FFB5000 },
    { 0xB308, 0x60C1001F },
    { 0xB30C, 0xFFFF60C1 },
    { 0xB310, 0xDC6FFFFF },
    { 0xB314, 0x14A51004 },
    { 0xB318, 0x0514F514 },
    { 0xB31C, 0x48C0DA41 },
    { 0xB320, 0xD08BCC06 },
    { 0xB324, 0x510897B3 },
    { 0xB328, 0x00000F03 },
    { 0xB32C, 0x0F400F40 },
    { 0xB330, 0x3DEF3DEF },
    { 0xB334, 0x00000000 },
    { 0xB338, 0x3B203B20 },
    { 0xB33C, 0x007A007A },
    { 0xB340, 0x58F558F5 },
    { 0xB344, 0x41FF41FA },
    { 0xB348, 0x39FF3A04 },
    { 0xB34C, 0x40FF00FF },
    { 0xB350, 0x007F007F },
    { 0xB354, 0x619F619F },
    { 0xB358, 0x28FB29FB },
    { 0xB35C, 0x80788078 },
    { 0xB000, 0x0F009403 },
    { 0xB004, 0x71067080 },
    { 0xB008, 0x8E0F0749 },
    { 0xB00C, 0x53598F5F },
    { 0xB010, 0x0000524B },
    { 0xB014, 0x00000CA4 },
    { 0xB018, 0x466404E4 },
    { 0xB01C, 0x00002053 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_10[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x0a4b8388 },
    { 0xb708, 0x42097211 },
    { 0xb720, 0x261b0000 },
    { 0xb728, 0xc0c78c62 },
    { 0xb728, 0xc0c78e62 },
    { 0xb728, 0xc0c78c62 },
    { 0xb400, 0x0F009403 },
    { 0xb40c, 0x53598f5f },
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_12[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xD10F97B3 },
    { 0xbba4, 0xd10f9793 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x7AF07E0  },
    { 0xbb30, 0x3DEF3DEF },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x00100510 },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x806d806d },
    { 0xbbdc, 0x806d806d },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    };

static confcode_mac_regval_t rtl835x_mac_serdes_rst[] = {
    { 0xA004, 0x71467380 },
    { 0xA004, 0x71067380 },
    { 0xA104, 0x71467380 },
    { 0xA104, 0x71067380 },
    { 0xa340, 0xBF04AA   },
    { 0xa340, 0x10BF04AA },
    { 0xa340, 0x10FF04AA },
    { 0xA404, 0x71467380 },
    { 0xA404, 0x71067380 },
    { 0xA504, 0x71467380 },
    { 0xA504, 0x71067380 },
    { 0xa740, 0xBF04AA   },
    { 0xa740, 0x10BF04AA },
    { 0xa740, 0x10FF04AA },
    { 0xA804, 0x71467380 },
    { 0xA804, 0x71067380 },
    { 0xA904, 0x71467380 },
    { 0xA904, 0x71067380 },
    { 0xab40, 0xBF04AA   },
    { 0xab40, 0x10BF04AA },
    { 0xab40, 0x10FF04AA },
    { 0xAc04, 0x71467380 },
    { 0xAc04, 0x71067380 },
    { 0xAd04, 0x71467380 },
    { 0xAd04, 0x71067380 },
    { 0xaf40, 0xBF04AA   },
    { 0xaf40, 0x10BF04AA },
    { 0xaf40, 0x10FF04AA },
    { 0xb404, 0x71467380 },
    { 0xb404, 0x71067380 },
    { 0xb504, 0x71467380 },
    { 0xb504, 0x71067380 },
    { 0xb740, 0xBF04AA   },
    { 0xb740, 0x10BF04AA },
    { 0xb740, 0x10FF04AA },
    { 0xB004, 0x71467380 },
    { 0xB004, 0x71067380 },
    { 0xB104, 0x71467380 },
    { 0xB104, 0x71067380 },
    { 0xB378, 0x00008541 },
    { 0xB378, 0x00008543 },
    { 0xB378, 0x00008553 },
    { 0xb804, 0x71467380 },
    { 0xb804, 0x71067380 },
    { 0xb904, 0x71467380 },
    { 0xb904, 0x71067380 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    };

/* RTL839x MAC configuration code */
static confcode_mac_regval_t rtl839x_dis_ck25mo[] = {
    { 0xa30c, 0x81c5C1f5 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xb304, 0x7ffb5000 },
    { 0xb384, 0x7ffb5000 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb84, 0x7ffb5000 },
    };

static confcode_mac_regval_t rtl839x_qsgmii[] = {
    { 0x0008, 0x66666666 },
    { 0x000c, 0x00666666 },
    };

#ifdef QSGMII_MODE
static confcode_mac_regval_t rtl839x_serdes10_a2d_clk_edge[] = {
    { 0xb40c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_serdes11_a2d_clk_edge[] = {
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_0[] = {
    { 0xa300, 0xf3c10000 },
    { 0xa304, 0x0A0B838E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa310, 0x5ccc8c65 },
    { 0xa320, 0xC6DB0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0x0ec78c60 },
    { 0xa32c, 0x14aa9482 },
    { 0xa330, 0xB0020000 },
    { 0xa340, 0x20bf04aa },
    { 0xa340, 0x30bf04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa004, 0x71467080 },
    { 0xa004, 0x71067080 },
    { 0xa008, 0x8E0F0749 },
    { 0xa00c, 0x13598F5F },
    { 0xa010, 0x0000524B },
    { 0xa014, 0x00000CA4 },
    { 0xa018, 0x466408E4 },
    { 0xa01c, 0x00002053 },
    { 0xa0e0, 0x00000400 },
    { 0xa000, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_1[] = {
    { 0xa300, 0xf3c10000 },
    { 0xa304, 0x0A0B838E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa390, 0x5ccc0000 },
    { 0xa3a0, 0xC6DB0000 },
    { 0xa3a4, 0x79000003 },
    { 0xa3a8, 0x0ec78c60 },
    { 0xa3ac, 0x14aa9482 },
    { 0xa3b0, 0xB0020000 },
    { 0xa340, 0x107f04aa },
    { 0xa340, 0x307f04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa104, 0x71467080 },
    { 0xa104, 0x71067080 },
    { 0xa108, 0x8E0F0749 },
    { 0xa10c, 0x13598F5F },
    { 0xa110, 0x0000524B },
    { 0xa114, 0x00000CA4 },
    { 0xa118, 0x466408E4 },
    { 0xa11c, 0x00002053 },
    { 0xa1e0, 0x00000400 },
    { 0xa100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_2[] = {
    { 0xa700, 0xf3c10000 },
    { 0xa704, 0x0A0B838E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa710, 0x5ccc8c65 },
    { 0xa720, 0xC6DB0000 },
    { 0xa724, 0x79000003 },
    { 0xa728, 0x0ec78c60 },
    { 0xa72c, 0x14aa9482 },
    { 0xa730, 0xB0020000 },
    { 0xa740, 0x20bf04aa },
    { 0xa740, 0x30bf04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa404, 0x71467080 },
    { 0xa404, 0x71067080 },
    { 0xa408, 0x8E0F0749 },
    { 0xa40c, 0x13598F5F },
    { 0xa410, 0x0000524B },
    { 0xa414, 0x00000CA4 },
    { 0xa418, 0x466408E4 },
    { 0xa41c, 0x00002053 },
    { 0xa4e0, 0x00000400 },
    { 0xa400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_3[] = {
    { 0xa700, 0xf3c10000 },
    { 0xa704, 0x0A0B838E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa790, 0x5ccc0000 },
    { 0xa7a0, 0xC6DB0000 },
    { 0xa7a4, 0x79000003 },
    { 0xa7a8, 0x0ec78c60 },
    { 0xa7ac, 0x14aa9482 },
    { 0xa7b0, 0xB0020000 },
    { 0xa740, 0x107f04aa },
    { 0xa740, 0x307f04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa504, 0x71467080 },
    { 0xa504, 0x71067080 },
    { 0xa508, 0x8E0F0749 },
    { 0xa50c, 0x13598F5F },
    { 0xa510, 0x0000524B },
    { 0xa514, 0x00000CA4 },
    { 0xa518, 0x466408E4 },
    { 0xa51c, 0x00002053 },
    { 0xa5e0, 0x00000400 },
    { 0xa500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_4[] = {
    { 0xab00, 0xf3c10000 },
    { 0xab04, 0x0A0B838E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab10, 0x5ccc8c65 },
    { 0xab20, 0xC6DB0000 },
    { 0xab24, 0x79000003 },
    { 0xab28, 0x0ec78c60 },
    { 0xab2c, 0x14aa9482 },
    { 0xab30, 0xB0020000 },
    { 0xab40, 0x20bf04aa },
    { 0xab40, 0x30bf04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa804, 0x71467080 },
    { 0xa804, 0x71067080 },
    { 0xa808, 0x8E0F0749 },
    { 0xa80c, 0x13598F5F },
    { 0xa810, 0x0000524B },
    { 0xa814, 0x00000CA4 },
    { 0xa818, 0x466408E4 },
    { 0xa81c, 0x00002053 },
    { 0xa8e0, 0x00000400 },
    { 0xa800, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_5[] = {
    { 0xab00, 0xf3c10000 },
    { 0xab04, 0x0A0B838E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab90, 0x5ccc0000 },
    { 0xaba0, 0xC6DB0000 },
    { 0xaba4, 0x79000003 },
    { 0xaba8, 0x0ec78c60 },
    { 0xabac, 0x14aa9482 },
    { 0xabb0, 0xB0020000 },
    { 0xab40, 0x107f04aa },
    { 0xab40, 0x307f04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa904, 0x71467080 },
    { 0xa904, 0x71067080 },
    { 0xa908, 0x8E0F0749 },
    { 0xa90c, 0x13598F5F },
    { 0xa910, 0x0000524B },
    { 0xa914, 0x00000CA4 },
    { 0xa918, 0x466408E4 },
    { 0xa91c, 0x00002053 },
    { 0xa9e0, 0x00000400 },
    { 0xa900, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_6[] = {
    { 0xaf00, 0xf3c10000 },
    { 0xaf04, 0x0A0B838E },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf10, 0x5ccc8c65 },
    { 0xaf20, 0xC6DB0000 },
    { 0xaf24, 0x79000003 },
    { 0xaf28, 0x0ec78c60 },
    { 0xaf2c, 0x14aa9482 },
    { 0xaf30, 0xB0020000 },
    { 0xaf40, 0x20bf04aa },
    { 0xaf40, 0x30bf04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xac04, 0x71467080 },
    { 0xac04, 0x71067080 },
    { 0xac08, 0x8E0F0749 },
    { 0xac0c, 0x13598F5F },
    { 0xac10, 0x0000524B },
    { 0xac14, 0x00000CA4 },
    { 0xac18, 0x466408E4 },
    { 0xac1c, 0x00002053 },
    { 0xace0, 0x00000400 },
    { 0xac00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_7[] = {
    { 0xaf00, 0xf3c10000 },
    { 0xaf04, 0x0A0B838E },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf90, 0x5ccc0000 },
    { 0xafa0, 0xC6DB0000 },
    { 0xafa4, 0x79000003 },
    { 0xafa8, 0x0ec78c60 },
    { 0xafac, 0x14aa9482 },
    { 0xafb0, 0xB0020000 },
    { 0xaf40, 0x107f04aa },
    { 0xaf40, 0x307f04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xad04, 0x71467080 },
    { 0xad04, 0x71067080 },
    { 0xad08, 0x8E0F0749 },
    { 0xad0c, 0x13598F5F },
    { 0xad10, 0x0000524B },
    { 0xad14, 0x00000CA4 },
    { 0xad18, 0x466408E4 },
    { 0xad1c, 0x00002053 },
    { 0xade0, 0x00000400 },
    { 0xad00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_10[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x4A0B839E },
    { 0xb708, 0x42095211 },
    { 0xb720, 0x6db0000  },
    { 0xb728, 0xd0c78c22 },
    { 0xb730, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x20bf080f },
    { 0xb740, 0x30bf080f },
    { 0xb740, 0x30ff080f },
    { 0xb404, 0x71467080 },
    { 0xb404, 0x71067080 },
    { 0xb4e0, 0x00000400 },
    { 0xb400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_11[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x4A0B839E },
    { 0xb708, 0x42095211 },
    { 0xb7a0, 0x165b0000 },
    { 0xb7a8, 0xd0c78c22 },
    { 0xb7b0, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x107f080f },
    { 0xb740, 0x307f080f },
    { 0xb740, 0x30ff080f },
    { 0xb504, 0x71467080 },
    { 0xb504, 0x71067080 },
    { 0xb5e0, 0x00000400 },
    { 0xb500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_8_9[] = {
    { 0xB300, 0x4000d800 },
    { 0xB304, 0x7ffb5000 },
    { 0xb308, 0x60c1001f },
    { 0xb30c, 0xffff60c1 },
    { 0xb310, 0xdc6fffff },
    { 0xb314, 0x14a51004 },
    { 0xb318, 0x514e514  },
    { 0xb31c, 0x48c08a41 },
    { 0xb320, 0xf04af216 },
    { 0xb324, 0xd10f9793 },
    { 0xb328, 0x0f03     },
    { 0xb32c, 0x27af27af },
    { 0xb330, 0x3cc03cc0 },
    { 0xb334, 0xffffffff },
    { 0xb338, 0x78037803 },
    { 0xb33c, 0xf01af01a },
    { 0xb340, 0xf500f5   },
    { 0xb344, 0x41ff41ff },
    { 0xb348, 0x39ff39ff },
    { 0xb34c, 0x00100010 },
    { 0xb350, 0x7f007f   },
    { 0xb354, 0x619f619f },
    { 0xb358, 0x29fb29fb },
    { 0xb35c, 0x806d806d },
    { 0xB380, 0x4000d800 },
    { 0xB384, 0x7ffb5000 },
    { 0xb388, 0x60c1001f },
    { 0xb38c, 0xffff60c1 },
    { 0xb390, 0xdc6fffff },
    { 0xb394, 0x14a51004 },
    { 0xb398, 0x514e514  },
    { 0xb39c, 0x48c08a41 },
    { 0xb3a0, 0xf04af216 },
    { 0xb3a4, 0xd10f9793 },
    { 0xb3a8, 0x0f03     },
    { 0xb3ac, 0x27af27af },
    { 0xb3b0, 0x3c403c40 },
    { 0xb3b4, 0xffffffff },
    { 0xb3b8, 0x78037803 },
    { 0xb3bc, 0xf01af01a },
    { 0xb3c0, 0xf500f5   },
    { 0xb3c4, 0x41ff41ff },
    { 0xb3c8, 0x39ff39ff },
    { 0xb3cc, 0x00100010 },
    { 0xb3d0, 0x7f007f   },
    { 0xb3d4, 0x619f619f },
    { 0xb3d8, 0x29fb29fb },
    { 0xb3dc, 0x806d806d },
    { 0xb378, 0x18ff3    },
    { 0xb37c, 0x80000000 },
    { 0xb004, 0x71467080 },
    { 0xb004, 0x71067080 },
    { 0xb104, 0x71467080 },
    { 0xb104, 0x71067080 },
    { 0xb00c, 0x53598f5f },
    { 0xb10c, 0x53598f5f },
    { 0xb378, 0x8553     },
    { 0xb37c, 0x8000     },
    { 0xb378, 0x1c553    },
    { 0xb37c, 0x80008000 },
    { 0xb018, 0x466408ec },
    { 0xb118, 0x466408ec },
    { 0xb340, 0x80f580f5 },
    { 0xb340, 0x00f500f5 },
    { 0xb340, 0x40f540f5 },
    { 0xb3c0, 0x80f580f5 },
    { 0xb3c0, 0x00f500f5 },
    { 0xb3c0, 0x40f540f5 },
    { 0xb378, 0x0001c541 },
    { 0xb378, 0x0001c543 },
    { 0xb378, 0x0001c553 },
    { 0xb0e0, 0x00000400 },
    { 0xb1e0, 0x00000400 },
    { 0xb000, 0x0f009403 },
    { 0xb100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_12[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xd10f9793 },
    { 0xbba4, 0xd10f9793 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x87968796 },
    { 0xbb30, 0x3dea3dea },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x100010   },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x806d806d },
    { 0xbbdc, 0x806d806d },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    { 0, 0},
    };
#if 0
static confcode_mac_regval_t rtl839x_fiber_serdes_12_13[] = {
    {0x0ff4, 0xa0000000},
    /* configure serdes#12,13 speed to 1000base-x */
    {0x000c, 0x00776666},
    /* configure chip mode=0x1; 48G+2*sgmii */
    {0x02ac, 0x81},
    /* patch serdes12,13 analog registers */
    {0xbb00, 0x4000D800},
    {0xbb04, 0x7FFB5000},
    {0xbb08, 0x60C1001F},
    {0xbb0C, 0xFFFF60C1},
    {0xbb10, 0xDC6FFFFF},
    {0xbb14, 0x14A51004},
    {0xbb18, 0x0514F514},
    {0xbb1C, 0x48C0DA41},
    {0xbb20, 0xC08BCC00},
    {0xbb24, 0x510F3493},
    {0xbb28, 0x00000F03},
    {0xbb2C, 0x0F400F40},
    {0xbb30, 0x62E162E1},
    {0xbb34, 0x000F000F},
    {0xbb38, 0x2D182D18},
    {0xbb3C, 0x007A007A},
    {0xbb40, 0x4F577A65},
    {0xbb44, 0x41FF41FA},
    {0xbb48, 0x39FF7A04},
    {0xbb4C, 0x401004E3},
    {0xbb50, 0x007F007F},
    {0xbb54, 0x619F619F},
    {0xbb58, 0x29FB29FB},
    {0xbb5C, 0xA078A078},
    {0xbb80, 0x4000D800},
    {0xbb84, 0x7FFB5000},
    {0xbb88, 0x60C1001F},
    {0xbb8C, 0xFFFF60C1},
    {0xbb90, 0xDC6FFFFF},
    {0xbb94, 0x14A51004},
    {0xbb98, 0x0514F514},
    {0xbb9C, 0x48C0DA41},
    {0xbba0, 0xC08BCC00},
    {0xbba4, 0x510F3493},
    {0xbba8, 0x00000F03},
    {0xbbaC, 0x0F400F40},
    {0xbbb0, 0x62E162E1},
    {0xbbb4, 0x000F000F},
    {0xbbb8, 0x2D182D18},
    {0xbbbC, 0x007A007A},
    {0xbbc0, 0x4F577A65},
    {0xbbc4, 0x41FF41FA},
    {0xbbc8, 0x39FF7A04},
    {0xbbcC, 0x401004E3},
    {0xbbd0, 0x007F007F},
    {0xbbd4, 0x619F619F},
    {0xbbd8, 0x29FB29FB},
    {0xbbdC, 0xA078A078},
    {0xbb78, 0x8553    },
    {0xbb7C, 0x80008000},
    {0xb804, 0x71467080},
    {0xb804, 0x71067080},
    {0xb904, 0x71467080},
    {0xb904, 0x71067080},
    {0xb80C, 0x53598F5F},
    {0xb800, 0x0f009403},
    {0xb8e0, 0x00000400},
    {0xb880, 0x61091140},
    {0xb980, 0x61091140},
    {0xbb78, 0x8541    },
    {0xbb78, 0x8543    },
    {0xbb78, 0x8553    },
};
#endif
#else   /* RSGMII+ MODE (the following configuration code) */
static confcode_mac_regval_t rtl839x_serdes10_a2d_clk_edge[] = {
    { 0xb40c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_serdes11_a2d_clk_edge[] = {
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_0[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x4A0B839E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa310, 0x5ccc8c65 },
    { 0xa320, 0xB61B0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0x0ec78c60 },
    { 0xa32c, 0x14aa9482 },
    { 0xa330, 0xf0020000 },
    { 0xa340, 0x20bf04aa },
    { 0xa340, 0x30bf04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa004, 0x71467080 },
    { 0xa004, 0x71067080 },
    { 0xa0e0, 0x00000400 },
    { 0xa000, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_1[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x4A0B839E },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa390, 0x5ccc0000 },
    { 0xa3a0, 0xB61B0000 },
    { 0xa3a4, 0x79000003 },
    { 0xa3a8, 0x0ec78c60 },
    { 0xa3ac, 0x14aa9482 },
    { 0xa3b0, 0xf0020000 },
    { 0xa340, 0x107f04aa },
    { 0xa340, 0x307f04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa104, 0x71467080 },
    { 0xa104, 0x71067080 },
    { 0xa1e0, 0x00000400 },
    { 0xa100, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_2[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x4A0B839E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa710, 0x5ccc8c65 },
    { 0xa720, 0xB61B0000 },
    { 0xa724, 0x79000003 },
    { 0xa728, 0x0ec78c60 },
    { 0xa72c, 0x14aa9482 },
    { 0xa730, 0xf0020000 },
    { 0xa740, 0x20bf04aa },
    { 0xa740, 0x30bf04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa404, 0x71467080 },
    { 0xa404, 0x71067080 },
    { 0xa4e0, 0x00000400 },
    { 0xa400, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_3[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x4A0B839E },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa790, 0x5ccc0000 },
    { 0xa7a0, 0xB61B0000 },
    { 0xa7a4, 0x79000003 },
    { 0xa7a8, 0x0ec78c60 },
    { 0xa7ac, 0x14aa9482 },
    { 0xa7b0, 0xf0020000 },
    { 0xa740, 0x107f04aa },
    { 0xa740, 0x307f04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa504, 0x71467080 },
    { 0xa504, 0x71067080 },
    { 0xa5e0, 0x00000400 },
    { 0xa500, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_4[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x4A0B839E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab10, 0x5ccc8c65 },
    { 0xab20, 0xB61B0000 },
    { 0xab24, 0x79000003 },
    { 0xab28, 0x0ec78c60 },
    { 0xab2c, 0x14aa9482 },
    { 0xab30, 0xf0020000 },
    { 0xab40, 0x20bf04aa },
    { 0xab40, 0x30bf04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa804, 0x71467080 },
    { 0xa804, 0x71067080 },
    { 0xa8e0, 0x00000400 },
    { 0xa800, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_5[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x4A0B839E },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab90, 0x5ccc0000 },
    { 0xaba0, 0xB61B0000 },
    { 0xaba4, 0x79000003 },
    { 0xaba8, 0x0ec78c60 },
    { 0xabac, 0x14aa9482 },
    { 0xabb0, 0xf0020000 },
    { 0xab40, 0x107f04aa },
    { 0xab40, 0x307f04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa904, 0x71467080 },
    { 0xa904, 0x71067080 },
    { 0xa9e0, 0x00000400 },
    { 0xa900, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_6[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf10, 0x5ccc8c65 },
    { 0xaf20, 0xB61B0000 },
    { 0xaf24, 0x79000003 },
    { 0xaf28, 0x0ec78c60 },
    { 0xaf2c, 0x14aa9482 },
    { 0xaf30, 0xf0020000 },
    { 0xaf40, 0x20bf04aa },
    { 0xaf40, 0x30bf04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xac04, 0x71467080 },
    { 0xac04, 0x71067080 },
    { 0xace0, 0x00000400 },
    { 0xac00, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_7[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf90, 0x5ccc0000 },
    { 0xafa0, 0xB61B0000 },
    { 0xafa4, 0x79000003 },
    { 0xafa8, 0x0ec78c60 },
    { 0xafac, 0x14aa9482 },
    { 0xafb0, 0xf0020000 },
    { 0xaf40, 0x107f04aa },
    { 0xaf40, 0x307f04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xad04, 0x71467080 },
    { 0xad04, 0x71067080 },
    { 0xade0, 0x00000400 },
    { 0xad00, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_10[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb720, 0x6db0000  },
    { 0xb728, 0xd0c78c22 },
    { 0xb730, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x20bf080f },
    { 0xb740, 0x30bf080f },
    { 0xb740, 0x30ff080f },
    { 0xb404, 0x71467080 },
    { 0xb404, 0x71067080 },
    { 0xb4e0, 0x00000400 },
    { 0xb400, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_11[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb7a0, 0x169b0000 },
    { 0xb7a8, 0xd0c78c22 },
    { 0xb7b0, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x107f080f },
    { 0xb740, 0x307f080f },
    { 0xb740, 0x30ff080f },
    { 0xb504, 0x71467080 },
    { 0xb504, 0x71067080 },
    { 0xb5e0, 0x00000400 },
    { 0xb500, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_8_9[] = {
    { 0xB300, 0x4000d800 },
    { 0xB304, 0x7ffb5000 },
    { 0xb308, 0x60c1001f },
    { 0xb30c, 0xffff60c1 },
    { 0xb310, 0xdc6fffff },
    { 0xb314, 0x14a51004 },
    { 0xb318, 0x514e514  },
    { 0xb31c, 0x48c08a41 },
    { 0xb320, 0xf04af216 },
    { 0xb324, 0xd10f97b3 },
    { 0xb328, 0x0f03     },
    { 0xb32c, 0x27af27af },
    { 0xb330, 0x3cc03cc0 },
    { 0xb334, 0xffffffff },
    { 0xb338, 0x78037803 },
    { 0xb33c, 0xf01af01a },
    { 0xb340, 0xf500f5   },
    { 0xb344, 0x41ff41ff },
    { 0xb348, 0x39ff39ff },
    { 0xb34c, 0x00100010 },
    { 0xb350, 0x7f007f   },
    { 0xb354, 0x619f619f },
    { 0xb358, 0x29fb29fb },
    { 0xb35c, 0x80608060 },
    { 0xB380, 0x4000d800 },
    { 0xB384, 0x7ffb5000 },
    { 0xb388, 0x60c1001f },
    { 0xb38c, 0xffff60c1 },
    { 0xb390, 0xdc6fffff },
    { 0xb394, 0x14a51004 },
    { 0xb398, 0x514e514  },
    { 0xb39c, 0x48c08a41 },
    { 0xb3a0, 0xf04af216 },
    { 0xb3a4, 0xd10f97b3 },
    { 0xb3a8, 0x0f03     },
    { 0xb3ac, 0x27af27af },
    { 0xb3b0, 0x3c403c40 },
    { 0xb3b4, 0xffffffff },
    { 0xb3b8, 0x78037803 },
    { 0xb3bc, 0xf01af01a },
    { 0xb3c0, 0xf500f5   },
    { 0xb3c4, 0x41ff41ff },
    { 0xb3c8, 0x39ff39ff },
    { 0xb3cc, 0x00100010 },
    { 0xb3d0, 0x7f007f   },
    { 0xb3d4, 0x619f619f },
    { 0xb3d8, 0x29fb29fb },
    { 0xb3dc, 0x80608060 },
    { 0xb378, 0x18ff3    },
    { 0xb37c, 0x80000000 },
    { 0xb004, 0x71467080 },
    { 0xb004, 0x71067080 },
    { 0xb104, 0x71467080 },
    { 0xb104, 0x71067080 },
    { 0xb00c, 0x53598f5f },
    { 0xb10c, 0x53598f5f },
    { 0xb378, 0x8553     },
    { 0xb37c, 0x8000     },
    { 0xb378, 0x1c553    },
    { 0xb37c, 0x80008000 },
    { 0xb018, 0x466408ec },
    { 0xb118, 0x466408ec },
    { 0xb340, 0x80f580f5 },
    { 0xb340, 0x00f500f5 },
    { 0xb340, 0x40f540f5 },
    { 0xb3c0, 0x80f580f5 },
    { 0xb3c0, 0x00f500f5 },
    { 0xb3c0, 0x40f540f5 },
    { 0xb378, 0x0001c541 },
    { 0xb378, 0x0001c543 },
    { 0xb378, 0x0001c553 },
    { 0xb0e0, 0x00000400 },
    { 0xb1e0, 0x00000400 },
    { 0xb000, 0x0f009403 },
    { 0xb100, 0x0f009403 },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_12[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xd10f97b3 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x87968796 },
    { 0xbb30, 0x3dea3dea },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x100010   },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x80608060 },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    };
#endif  /* end of QSGMII MODE (else) */

/* for QA board */
static confcode_mac_regval_t rtl839x_serdes10_a2d_clk_edge_qa[] = {
    { 0xb40c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_serdes11_a2d_clk_edge_qa[] = {
    { 0xb50c, 0x53598f5f },
    };

static confcode_mac_regval_t rtl839x_5G_serdes_0_qa[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x8A0B83BE },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa310, 0x5ccc8c65 },
    { 0xa320, 0xB61B0000 },
    { 0xa324, 0x79000003 },
    { 0xa328, 0x0ec78c60 },
    { 0xa32c, 0x14aa9482 },
    { 0xa330, 0xf0020000 },
    { 0xa340, 0x20bf04aa },
    { 0xa340, 0x30bf04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa004, 0x71467080 },
    { 0xa004, 0x71067080 },
    { 0xa0e0, 0x00000400 },
    { 0xa000, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_1_qa[] = {
    { 0xa300, 0x03c10000 },
    { 0xa304, 0x8A0B83BE },
    { 0xa308, 0x42095211 },
    { 0xa30c, 0x81c5C1f5 },
    { 0xa390, 0x5ccc0000 },
    { 0xa3a0, 0xB61B0000 },
    { 0xa3a4, 0x79000003 },
    { 0xa3a8, 0x0ec78c60 },
    { 0xa3ac, 0x14aa9482 },
    { 0xa3b0, 0xf0020300 },
    { 0xa340, 0x107f04aa },
    { 0xa340, 0x307f04aa },
    { 0xa340, 0x30ff04aa },
    { 0xa104, 0x71467080 },
    { 0xa104, 0x71067080 },
    { 0xa1e0, 0x00000400 },
    { 0xa100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_2_qa[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x8A0B83BE },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa710, 0x5ccc8c65 },
    { 0xa720, 0xB61B0000 },
    { 0xa724, 0x79000003 },
    { 0xa728, 0x0ec78c60 },
    { 0xa72c, 0x14aa9482 },
    { 0xa730, 0xf0020000 },
    { 0xa740, 0x20bf04aa },
    { 0xa740, 0x30bf04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa404, 0x71467080 },
    { 0xa404, 0x71067080 },
    { 0xa4e0, 0x00000400 },
    { 0xa400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_3_qa[] = {
    { 0xa700, 0x03c10000 },
    { 0xa704, 0x8A0B83BE },
    { 0xa708, 0x42095211 },
    { 0xa70c, 0x81c5C1f5 },
    { 0xa790, 0x5ccc0000 },
    { 0xa7a0, 0xB61B0000 },
    { 0xa7a4, 0x79000003 },
    { 0xa7a8, 0x0ec78c60 },
    { 0xa7ac, 0x14aa9482 },
    { 0xa7b0, 0xf0020000 },
    { 0xa740, 0x107f04aa },
    { 0xa740, 0x307f04aa },
    { 0xa740, 0x30ff04aa },
    { 0xa504, 0x71467080 },
    { 0xa504, 0x71067080 },
    { 0xa5e0, 0x00000400 },
    { 0xa500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_4_qa[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x8A0B83BE },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab10, 0x5ccc8c65 },
    { 0xab20, 0xB61B0000 },
    { 0xab24, 0x79000003 },
    { 0xab28, 0x0ec78c60 },
    { 0xab2c, 0x14aa9482 },
    { 0xab30, 0xf0020000 },
    { 0xab40, 0x20bf04aa },
    { 0xab40, 0x30bf04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa804, 0x71467080 },
    { 0xa804, 0x71067080 },
    { 0xa8e0, 0x00000400 },
    { 0xa800, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_5_qa[] = {
    { 0xab00, 0x03c10000 },
    { 0xab04, 0x8A0B83BE },
    { 0xab08, 0x42095211 },
    { 0xab0c, 0x81c5C1f5 },
    { 0xab90, 0x5ccc0000 },
    { 0xaba0, 0xB61B0000 },
    { 0xaba4, 0x79000003 },
    { 0xaba8, 0x0ec78c60 },
    { 0xabac, 0x14aa9482 },
    { 0xabb0, 0xf0020000 },
    { 0xab40, 0x107f04aa },
    { 0xab40, 0x307f04aa },
    { 0xab40, 0x30ff04aa },
    { 0xa904, 0x71467080 },
    { 0xa904, 0x71067080 },
    { 0xa9e0, 0x00000400 },
    { 0xa900, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_6_qa[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf10, 0x5ccc8c65 },
    { 0xaf20, 0xB61B0000 },
    { 0xaf24, 0x79000003 },
    { 0xaf28, 0x0ec78c60 },
    { 0xaf2c, 0x14aa9482 },
    { 0xaf30, 0xf0020000 },
    { 0xaf40, 0x20bf04aa },
    { 0xaf40, 0x30bf04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xac04, 0x71467080 },
    { 0xac04, 0x71067080 },
    { 0xace0, 0x00000400 },
    { 0xac00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_7_qa[] = {
    { 0xaf00, 0x03c10000 },
    { 0xaf04, 0x8A0B83BE },
    { 0xaf08, 0x42095211 },
    { 0xaf0c, 0x81c5C1f5 },
    { 0xaf90, 0x5ccc0000 },
    { 0xafa0, 0xB61B0000 },
    { 0xafa4, 0x79000003 },
    { 0xafa8, 0x0ec78c60 },
    { 0xafac, 0x14aa9482 },
    { 0xafb0, 0xf0020000 },
    { 0xaf40, 0x107f04aa },
    { 0xaf40, 0x307f04aa },
    { 0xaf40, 0x30ff04aa },
    { 0xad04, 0x71467080 },
    { 0xad04, 0x71067080 },
    { 0xade0, 0x00000400 },
    { 0xad00, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_10_qa[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb720, 0x6db0000  },
    { 0xb728, 0xd0c78c22 },
    { 0xb730, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x20bf080f },
    { 0xb740, 0x30bf080f },
    { 0xb740, 0x30ff080f },
    { 0xb404, 0x71467080 },
    { 0xb404, 0x71067080 },
    { 0xb4e0, 0x00000400 },
    { 0xb400, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_11_qa[] = {
    { 0xb700, 0xa3c20000 },
    { 0xb704, 0x8A0B83BE },
    { 0xb708, 0x42095211 },
    { 0xb7a0, 0x169b0000 },
    { 0xb7a8, 0xd0c78c22 },
    { 0xb7b0, 0xf0020300 },
    { 0xb70c, 0x81c5C1f5 },
    { 0xb740, 0x107f080f },
    { 0xb740, 0x307f080f },
    { 0xb740, 0x30ff080f },
    { 0xb504, 0x71467080 },
    { 0xb504, 0x71067080 },
    { 0xb5e0, 0x00000400 },
    { 0xb500, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_8_9_qa[] = {
    { 0xB300, 0x4000d800 },
    { 0xB304, 0x7ffb5000 },
    { 0xb308, 0x60c1001f },
    { 0xb30c, 0xffff60c1 },
    { 0xb310, 0xdc6fffff },
    { 0xb314, 0x14a51004 },
    { 0xb318, 0x514e514  },
    { 0xb31c, 0x48c08a41 },
    { 0xb320, 0xf04af216 },
    { 0xb324, 0xd10f9793 },
    { 0xb328, 0x0f03     },
    { 0xb32c, 0x27b727b7 },
    { 0xb330, 0x3cc03cc0 },
    { 0xb334, 0xffffffff },
    { 0xb338, 0x78037803 },
    { 0xb33c, 0xf01af01a },
    { 0xb340, 0xf500f5   },
    { 0xb344, 0x41ff41ff },
    { 0xb348, 0x39ff39ff },
    { 0xb34c, 0x00100010 },
    { 0xb350, 0x7f007f   },
    { 0xb354, 0x619f619f },
    { 0xb358, 0x29fb29fb },
    { 0xb35c, 0x80608071 },
    { 0xB380, 0x4000d800 },
    { 0xB384, 0x7ffb5000 },
    { 0xb388, 0x60c1001f },
    { 0xb38c, 0xffff60c1 },
    { 0xb390, 0xdc6fffff },
    { 0xb394, 0x14a51004 },
    { 0xb398, 0x514e514  },
    { 0xb39c, 0x48c08a41 },
    { 0xb3a0, 0xf04af216 },
    { 0xb3a4, 0xd10f9793 },
    { 0xb3a8, 0x0f03     },
    { 0xb3ac, 0x27b727b7 },
    { 0xb3b0, 0x3c403c40 },
    { 0xb3b4, 0xffffffff },
    { 0xb3b8, 0x78037803 },
    { 0xb3bc, 0xf01af01a },
    { 0xb3c0, 0xf500f5   },
    { 0xb3c4, 0x41ff41ff },
    { 0xb3c8, 0x39ff39ff },
    { 0xb3cc, 0x00100010 },
    { 0xb3d0, 0x7f007f   },
    { 0xb3d4, 0x619f619f },
    { 0xb3d8, 0x29fb29fb },
    { 0xb3dc, 0x80608071 },
    { 0xb378, 0x18ff3    },
    { 0xb37c, 0x80000000 },
    { 0xb004, 0x71467080 },
    { 0xb004, 0x71067080 },
    { 0xb104, 0x71467080 },
    { 0xb104, 0x71067080 },
    { 0xb00c, 0x53598f5f },
    { 0xb10c, 0x53598f5f },
    { 0xb378, 0x8553     },
    { 0xb37c, 0x8000     },
    { 0xb378, 0x1c553    },
    { 0xb37c, 0x80008000 },
    { 0xb018, 0x466408ec },
    { 0xb118, 0x466408ec },
    { 0xb340, 0x80f580f5 },
    { 0xb340, 0x00f500f5 },
    { 0xb340, 0x40f540f5 },
    { 0xb3c0, 0x80f580f5 },
    { 0xb3c0, 0x00f500f5 },
    { 0xb3c0, 0x40f540f5 },
    { 0xb378, 0x0001c541 },
    { 0xb378, 0x0001c543 },
    { 0xb378, 0x0001c553 },
    { 0xb0e0, 0x00000400 },
    { 0xb1e0, 0x00000400 },
    { 0xb000, 0x0f009403 },
    { 0xb100, 0x0f009403 },
    { 0, 0},
    };

static confcode_mac_regval_t rtl839x_5G_serdes_12_qa[] = {
    { 0xbb00, 0x4000d800 },
    { 0xbb04, 0x7ffb5000 },
    { 0xbb08, 0x60c1001f },
    { 0xbb0c, 0xffff60c1 },
    { 0xbb10, 0xdc6fffff },
    { 0xbb14, 0x14a51004 },
    { 0xbb18, 0x514e514  },
    { 0xbb1c, 0x48c08a41 },
    { 0xbb20, 0xf04af216 },
    { 0xbb24, 0xd10f9793 },
    { 0xbb28, 0x0f03     },
    { 0xbb2c, 0x87968796 },
    { 0xbb30, 0x3dea3dea },
    { 0xbb34, 0xffffffff },
    { 0xbb38, 0x78037803 },
    { 0xbb3c, 0xf01af01a },
    { 0xbb40, 0xf500f5   },
    { 0xbb44, 0x41ff41ff },
    { 0xbb48, 0x39ff39ff },
    { 0xbb4c, 0x100010   },
    { 0xbb50, 0x7f007f   },
    { 0xbb54, 0x619f619f },
    { 0xbb58, 0x29fb29fb },
    { 0xbb5c, 0x80608070 },
    { 0xbb78, 0x18ff3    },
    { 0xbb7c, 0x80000000 },
    { 0xbbc0, 0x18f55a75 },
    { 0xb804, 0x71467080 },
    { 0xb804, 0x71067080 },
    { 0xb904, 0x71467080 },
    { 0xb904, 0x71067080 },
    { 0xb80c, 0x53598f5f },
    { 0xbb78, 0x8553     },
    { 0xbb7c, 0x8000     },
    { 0xbbc0, 0x18f51a75 },
    { 0xbb78, 0x1c553    },
    { 0xbb7c, 0x80008000 },
    { 0xb818, 0x466408ec },
    { 0xb918, 0x466408ec },
    { 0xbb40, 0xc0f5c0f5 },
    { 0xbb40, 0x40f540f5 },
    { 0xbb78, 0x1C541    },
    { 0xbb78, 0x1c543    },
    { 0xbb78, 0x1c553    },
    { 0xb8e0, 0x00000400 },
    { 0xb800, 0x0f009403 },
    { 0, 0},
    };

confcode_mac_regval_t rtl835x_serdes_powerOff_conf[] = {
    { 0xa340, 0xc400043f},
    { 0xa3c0, 0xc40043f },
    { 0xa740, 0xc400043f},
    { 0xa7c0, 0xc40043f },
    { 0xab40, 0xc400043f},
    { 0xabc0, 0xc40043f },
    { 0xaf40, 0xc400043f},
    { 0xafc0, 0xc40043f },
    { 0xb740, 0xc400043f},
    { 0xb7c0, 0xc40043f },
    { 0xb3f8, 0x4c00000 },
};

confcode_serdes_patch_t rtl839x_init_fiber_1g_frc_S12[] = {
    { 0x000c  , 0       , 19 , 16 , 0x7     },
    { 0xb818  , 0       , 3  , 3  , 0       },
    { 0xbbfc  , 0       , 16 , 16 , 1       },
    { 0xbbfc  , 0       , 19 , 18 , 0x2     },
    { 0xbb58  , 0       , 0  , 0  , 1       },
    { 0xbb58  , 0       , 4  , 4  , 1       },
    { 0xbb58  , 0       , 16 , 16 , 1       },
    { 0xbb58  , 0       , 20 , 20 , 1       },
    { 0xbb38  , 0       , 15 , 0  , 0x0722  },
    { 0xbb40  , 0       , 15 , 0  , 0x18f5  },
    { 0xbb00  , 0       , 11 , 11 , 0       },
    { 0xb804  , 0       , 9  , 8  , 0x0     },
    { 0xb880  , 0       , 13 , 13 , 0       },
    { 0xb880  , 0       , 6  , 6  , 1       },
    { 0xb880  , 0       , 12 , 12 , 1       },
    { 0xbb20  , 0       , 5  , 5  , 1       },
    { 0xbb1c  , 0       , 11 , 7  , 0x15    },
    { 0xb888  , 0       , 7  , 7  , 1       },
    { 0xb888  , 0       , 8  , 8  , 1       },
    { 0xbbf8  , 0       , 21 , 20 , 0x3     },
    { 0xbbf8  , 0       , 25 , 24 , 0x1     },
    { 0xbbf8  , 0       , 21 , 20 , 0x1     },
    { 0xbbf8  , 0       , 25 , 24 , 0x3     },
    { 0xbbf8  , 0       , 21 , 20 , 0x0     },
    { 0xbbf8  , 0       , 25 , 24 , 0x0     },
};

confcode_serdes_patch_t rtl839x_init_fiber_1g_frc_S13[] = {
    { 0x000c  , 0       , 23 , 20 , 0x7     },
    { 0xb918  , 0       , 3  , 3  , 0       },
    { 0xbbfc  , 0       , 17 , 17 , 1       },
    { 0xbbfc  , 0       , 21 , 20 , 0x2     },
    { 0xbbd8  , 0       , 0  , 0  , 1       },
    { 0xbbd8  , 0       , 4  , 4  , 1       },
    { 0xbbd8  , 0       , 16 , 16 , 1       },
    { 0xbbd8  , 0       , 20 , 20 , 1       },
    { 0xbbb8  , 0       , 15 , 0  , 0x0722  },
    { 0xbbc0  , 0       , 15 , 0  , 0x18f5  },
    { 0xbb80  , 0       , 11 , 11 , 0       },
    { 0xb904  , 0       , 9  , 8  , 0x0     },
    { 0xb980  , 0       , 13 , 13 , 0       },
    { 0xb980  , 0       , 6  , 6  , 1       },
    { 0xb980  , 0       , 12 , 12 , 1       },
    { 0xbb20  , 0x80    , 5  , 5  , 1       },
    { 0xbb1c  , 0x80    , 11 , 7  , 0x15    },
    { 0xb988  , 0       , 7  , 7  , 1       },
    { 0xb988  , 0       , 8  , 8  , 1       },
    { 0xbbf8  , 0       , 23 , 22 , 0x3     },
    { 0xbbf8  , 0       , 27 , 26 , 0x1     },
    { 0xbbf8  , 0       , 23 , 22 , 0x1     },
    { 0xbbf8  , 0       , 27 , 26 , 0x3     },
    { 0xbbf8  , 0       , 23 , 22 , 0x0     },
    { 0xbbf8  , 0       , 27 , 26 , 0x0     },
};

/*
 * Function Declaration
 */

void rtl8390_macReg_setChk(uint32 reg, uint32 val)
{
    MEM32_WRITE((0xbb000000) + reg, val);
    OSAL_MDELAY(1);
    if ((reg != 0x0ff4) && (reg != 0x0014) && (reg != 0x1180))
    {
        if (REG32((0xbb000000) + reg) != val)
        {
            printf("WARN: Reg 0x%08X != 0x%08X (real: 0x%08X)\n",
                    (0xbb000000) + reg, val, REG32((0xbb000000) + reg));
        }
    }

    return;
}

void rtl8390_sds_patch_set(confcode_serdes_patch_t in)
{
    uint32  reg, val, len, mask;
    uint32  i, startBit, endBit;

    reg = in.reg + in.offset;
    startBit = in.startBit;
    endBit = in.endBit;
    len = endBit - startBit + 1;

    if (32 == len)
        val = in.val;
    else
    {
        mask = 0;
        for (i = startBit; i <= endBit; ++i)
            mask |= (1 << i);

        val = MEM32_READ(SWCORE_BASE_ADDR| reg);
        val &= ~(mask);
        val |= (in.val << startBit);
    }

    MEM32_WRITE((0xbb000000) + reg, val);

    return;
}

void rtl8390_sds_patch_setChk(confcode_serdes_patch_t in)
{
    uint32  reg, val, len, mask;
    uint32  i, startBit, endBit;

    reg = in.reg + in.offset;
    startBit = in.startBit;
    endBit = in.endBit;
    len = endBit - startBit + 1;

    if (32 == len)
        val = in.val;
    else
    {
        mask = 0;
        for (i = startBit; i <= endBit; ++i)
            mask |= (1 << i);

        val = MEM32_READ(SWCORE_BASE_ADDR| reg);
        val &= ~(mask);
        val |= (in.val << startBit);
    }

    rtl8390_macReg_setChk(reg, val);

    return;
}

/* Function Name:
 *      rtl8390_platform_config_init
 * Description:
 *      Platform Configuration code in RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_platform_config_init(const rtk_switch_model_t *pModel)
{
    char *s_mac = NULL;
    int     i, j;

    /* write MAC addr to register */
    s_mac = getenv("ethaddr");
    if (s_mac != NULL)
    {
        char enetaddr[6], *e = NULL;

        for (i=0; i<6; i++)
        {
            enetaddr[i] = (unsigned char)(s_mac ? simple_strtoul(s_mac, &e, 16) : 0);
            if (s_mac)
            s_mac = (*e) ? e+1 : e;
        }

#if 0
        REG32(SWCORE_BASE_ADDR| RTL8390_MAC_ADDR_CTRL_ADDR) = (*(unsigned int *)&enetaddr[0] & 0x0000FFFFU);
        REG32(SWCORE_BASE_ADDR| RTL8390_MAC_ADDR_CTRL_ADDR+4) = (*(unsigned int *)&enetaddr[2] & 0xFFFFFFFFU);
#else
        REG32(SWCORE_BASE_ADDR| RTL8390_MAC_ADDR_CTRL_ADDR) = (((uint8)enetaddr[0]) << 8) | ((uint8)enetaddr[1]);
        REG32(SWCORE_BASE_ADDR| (RTL8390_MAC_ADDR_CTRL_ADDR+4)) = (((uint8)enetaddr[2]) << 24) | (((uint8)enetaddr[3]) << 16) | (((uint8)enetaddr[4]) << 8) | ((uint8)enetaddr[5]);
#endif
    }

    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_SMI_PORT_POLLING_CTRL_ADDR(0), 0);
    MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_SMI_PORT_POLLING_CTRL_ADDR(32), 0);

    for (i = 0; i < pModel->port.count; ++i)
    {
        j = pModel->port.list[i].mac_id;
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_PORT_POLLING_CTRL_ADDR(j), \
            RTL8390_SMI_PORT_POLLING_CTRL_SMI_POLLING_PMSK_OFFSET(j),
            RTL8390_SMI_PORT_POLLING_CTRL_SMI_POLLING_PMSK_MASK(j), 0x1);
    }

#ifdef CONFIG_RTL8390_FPGA
    /* set PHY start address to 1 */
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_SMI_GLB_CTRL_ADDR, 0x81);

    /* lower FC threshold to fit FPGA packet buffer size */
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_DROP_THR_ADDR)         = 0x000003FF;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_HI_THR_ADDR)       = 0x02320226;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_LO_THR_ADDR)       = 0x0168015E;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_FCOFF_HI_THR_ADDR) = 0x02320226;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_GLB_FCOFF_LO_THR_ADDR) = 0x0168015E;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_P_HI_THR_ADDR(0))      = 0x0064005E;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_P_LO_THR_ADDR(0))      = 0x00140014;
    REG32(SWCORE_BASE_ADDR | RTL8390_FC_P_FCOFF_HI_THR_ADDR(0))= 0x0064005E;
#endif

    return;
} /* end of rtl8390_platform_config_init */

/* Function Name:
 *      rtl8390_mac_config_init
 * Description:
 *      Mac Configuration code in RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_mac_config_init(const rtk_switch_model_t *pModel)
{
    uint32 chip_info = 0;
    uint32 idx;
    uint32 i, active;

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_CHIP_INFO_ADDR, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_OFFSET, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_MASK, \
        0xA);

    chip_info = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_CHIP_INFO_ADDR);

    if ((chip_info & RTL8390_CHIP_INFO_RL_ID_MASK) == 0x0399)
    {
        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {
            /* MAC basic configuration */
            for (idx=0; idx<(sizeof(rtl835x_mac_conf)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl835x_mac_conf[idx].reg, rtl835x_mac_conf[idx].val);
            }
        }   /* RTK_CHIP_RTL835xM */
        else
        {
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_MAC_EFUSE_CTRL_ADDR, 0x00000080);

            //set Queue_Rst {0x0014 0x4}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_RST_GLB_CTRL_ADDR, 0x00000004);

            //set IPGComp_En {0x02a8 0x3c324f40}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_MAC_GLB_CTRL_ADDR, 0x3c324f40);

            #if 0   /* depend on the setting of boardmodel */
            //set LED_En {0x00e4 0x260F56E}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR, 0x0260f56e);
            #endif

            //set egressRate_unlimit {0x60f8 0x12972561}
            MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, 0x1297b961);
        }

        /* L2 Table default entry */
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_TBL_ACCESS_L2_DATA_ADDR(0), 0x7FFFFFFF);
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_TBL_ACCESS_L2_DATA_ADDR(1), 0xFFFFF800);
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_TBL_ACCESS_L2_CTRL_ADDR, 0x38000);

        /* Initial (Tick, Token) value */
        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {   /* 50MHz */
            /* Scheduling */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                37);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_MASK, \
                97);

            /* Storm Control - pps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                240);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                5);
            /* 10G Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                240);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                5);

            /* Protocol Storm - pps */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                195313);

            /* ACL Policer - bps */
            for (idx=0; idx<16; idx++)
            {
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                    214);
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_MASK, \
                    561);

            }

            /* Input BW - bps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                214);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                561);
        }
        else
        {   /* 250MHz */
            /* Scheduling */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                185);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_MASK, \
                97);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_10G_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_TICK_PERIOD_10G_MASK, \
                18);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SCHED_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_10G_OFFSET, \
                RTL8390_SCHED_LB_TICK_TKN_CTRL_BYTE_PER_TKN_10G_MASK, \
                151);

            /* Storm Control - pps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                238);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                1);
            /* 10G Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                238);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_STORM_CTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                1);

            /* Protocol Storm - pps */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_ADDR, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_STORM_CTRL_SPCL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                976563);

            /* ACL Policer - bps */
            for (idx=0; idx<16; idx++)
            {
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                    288);
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_METER_LB_TICK_TKN_CTRL_ADDR(idx), \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                    RTL8390_METER_LB_TICK_TKN_CTRL_TKN_MASK, \
                    151);

            }

            /* Input BW - bps */
            /* Giga Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                246);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 4), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                129);
            /* 10G Port */
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TICK_PERIOD_MASK, \
                18);
            MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| (RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_ADDR + 0), \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_OFFSET, \
                RTL8390_IGR_BWCTRL_LB_TICK_TKN_CTRL_TKN_MASK, \
                151);
        }
    }

    /*
     * LED initialize
     */
    /* config LED interface */
    if (LED_IF_SEL_NONE != pModel->led.led_if_sel)
    {
        switch (pModel->led.led_if_sel)
        {
            case LED_IF_SEL_SERIAL:
                i = 0;
                active = 0; /* Low */
                break;
            case LED_IF_SEL_SINGLE_COLOR_SCAN:
                i = 1;
                active = 1; /* High */
                break;
            case LED_IF_SEL_BI_COLOR_SCAN:
                i = 2;
                active = 1; /* High */
                break;
            default:
                printf("Invalid LED IF SEL\n");
                return;
        }

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
                RTL8390_LED_GLB_CTRL_LED_IF_SEL_OFFSET,
                RTL8390_LED_GLB_CTRL_LED_IF_SEL_MASK, i);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
                RTL8390_LED_GLB_CTRL_LED_ACTIVE_OFFSET,
                RTL8390_LED_GLB_CTRL_LED_ACTIVE_MASK, active);
    }

    /* config LED number */
    if (0 != pModel->led.num &&
            pModel->led.num < (1 << RTL8390_LED_GLB_CTRL_LED_NUM_SEL_OFFSET))
    {
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
                RTL8390_LED_GLB_CTRL_LED_NUM_SEL_OFFSET,
                RTL8390_LED_GLB_CTRL_LED_NUM_SEL_MASK, pModel->led.num);
    }

    /* enable LED */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR,
            RTL8390_LED_GLB_CTRL_LED_EN_OFFSET,
            RTL8390_LED_GLB_CTRL_LED_EN_MASK, 1);

    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COPR_PMASK_CTRL_ADDR(0),  pModel->led.copr_pmask[0]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COPR_PMASK_CTRL_ADDR(32), pModel->led.copr_pmask[1]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_FIB_PMASK_CTRL_ADDR(0),  pModel->led.fib_pmask[0]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_FIB_PMASK_CTRL_ADDR(32), pModel->led.fib_pmask[1]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COMBO_CTRL_ADDR(0),  pModel->led.led_combo[0]);
    MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_LED_COMBO_CTRL_ADDR(32), pModel->led.led_combo[1]);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET0_LED0_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET0_LED0_SEL_MASK,
        pModel->led.led_definition_set[0].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET0_LED1_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET0_LED1_SEL_MASK,
        pModel->led.led_definition_set[0].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET0_LED2_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET0_LED2_SEL_MASK,
        pModel->led.led_definition_set[0].led[2]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET1_LED0_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET1_LED0_SEL_MASK,
        pModel->led.led_definition_set[1].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET1_LED1_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET1_LED1_SEL_MASK,
        pModel->led.led_definition_set[1].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_0_1_CTRL_ADDR, \
        RTL8390_LED_SET_0_1_CTRL_SET1_LED2_SEL_OFFSET,
        RTL8390_LED_SET_0_1_CTRL_SET1_LED2_SEL_MASK,
        pModel->led.led_definition_set[1].led[2]);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET2_LED0_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET2_LED0_SEL_MASK,
        pModel->led.led_definition_set[2].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET2_LED1_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET2_LED1_SEL_MASK,
        pModel->led.led_definition_set[2].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET2_LED2_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET2_LED2_SEL_MASK,
        pModel->led.led_definition_set[2].led[2]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET3_LED0_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET3_LED0_SEL_MASK,
        pModel->led.led_definition_set[3].led[0]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET3_LED1_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET3_LED1_SEL_MASK,
        pModel->led.led_definition_set[3].led[1]);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_SET_2_3_CTRL_ADDR, \
        RTL8390_LED_SET_2_3_CTRL_SET3_LED2_SEL_OFFSET,
        RTL8390_LED_SET_2_3_CTRL_SET3_LED2_SEL_MASK,
        pModel->led.led_definition_set[3].led[2]);

    for (idx=0; idx<pModel->port.count; idx++)
    {
        Tuint32 macid = pModel->port.list[idx].mac_id;
        Tuint32 value;

        //printf("[LED] port %u => mac_idx (%u)\n", idx, macid);

        value = ((pModel->led.led_copr_set_psel_bit1_pmask[(macid/32)] & (1<<(macid%32)))? 2:0) | \
                ((pModel->led.led_copr_set_psel_bit0_pmask[(macid/32)] & (1<<(macid%32)))? 1:0);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_COPR_SET_SEL_CTRL_ADDR(macid), \
            RTL8390_LED_COPR_SET_SEL_CTRL_LED_COPR_SET_PSEL_OFFSET(macid),
            RTL8390_LED_COPR_SET_SEL_CTRL_LED_COPR_SET_PSEL_MASK(macid),
            value);

        value = ((pModel->led.led_fib_set_psel_bit1_pmask[(macid/32)] & (1<<(macid%32)))? 2:0) | \
                ((pModel->led.led_fib_set_psel_bit0_pmask[(macid/32)] & (1<<(macid%32)))? 1:0);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_FIB_SET_SEL_CTRL_ADDR(macid), \
            RTL8390_LED_FIB_SET_SEL_CTRL_LED_FIB_SET_PSEL_OFFSET(macid),
            RTL8390_LED_FIB_SET_SEL_CTRL_LED_FIB_SET_PSEL_MASK(macid),
            value);
    }

    /* EEEP configuration */
    MEM32_WRITE(0xBB00047C, 0x1414FF15);
    MEM32_WRITE(0xBB000480, 0x1414FF0D);
    MEM32_WRITE(0xBB000488, 0x1414FF0B);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_CHIP_INFO_ADDR, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_OFFSET, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_MASK, \
        0x0);

    return;
} /* end of rtl8390_mac_config_init */

/* Function Name:
 *      rtl8390_phy_config_init
 * Description:
 *      PHY Configuration code that connect with RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_phy_config_init(const rtk_switch_model_t *pModel)
{
    unsigned int phy_idx;
    unsigned int port_id;

    /* Disable MAC polling PHY setting */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_GLB_CTRL_ADDR, \
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x0);

    for (phy_idx=0; phy_idx<pModel->phy.count; phy_idx++)
    {
        Tuint8 phy0_macid = pModel->phy.list[phy_idx].mac_id;
        #if (defined(CONFIG_RTL8214FC))
        Tuint8 portNum = pModel->phy.list[phy_idx].phy_max;
        #endif

        //printf("[INFO] config PHY (%u) => mac_id = %u\n", phy_idx, phy0_macid);

        switch (pModel->phy.list[phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                {
                    rtl8208_config(phy0_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218B))
            case RTK_CHIP_RTL8218B:
                {
                    rtl8218b_rtl8390_config(phy0_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                {
                    rtl8218fb_rtl8390_config(phy0_macid);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                {
                    rtl8214fc_rtl8390_config(phy0_macid, portNum);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
            {
                rtl8214fb_config(phy0_macid);
            }
            break;
            #endif
            #if (defined(CONFIG_RTL8218C))
            case RTK_CHIP_RTL8218C:
                {
                    rtl8218c_config(phy0_macid);
                }
                break;
            #endif
            default:
                break;
        }
    }

    /* Park Page to 0 */
    for (port_id=pModel->port.offset; port_id<(pModel->port.offset+pModel->port.count); port_id++)
    {
        int mac_id = pModel->port.list[port_id].mac_id;

        gMacDrv->drv_miim_write(mac_id, gMacDrv->miim_max_page, 31, 0);
    }

    for (port_id=pModel->port.offset; port_id<(pModel->port.offset+pModel->port.count); port_id++)
    {
        if (RTK_CHIP_NONE == pModel->phy.list[pModel->port.list[port_id].phy_idx].chip)
            continue;

        rtl8390_phyPortPowerOn(port_id);
    }

    OSAL_MDELAY(500);

    for (port_id=pModel->port.offset; port_id<(pModel->port.offset+pModel->port.count); port_id++)
    {
        if (RTK_CHIP_NONE == pModel->phy.list[pModel->port.list[port_id].phy_idx].chip)
            continue;

        rtl8390_phyPortPowerOff(port_id);
    }

    /* Restore MAC polling PHY setting */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SMI_GLB_CTRL_ADDR, \
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
            RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x1);

    return;
} /* end of rtl8390_phy_config_init */

confcode_serdes_patch_t rtl839x_eee_enable0[] = {
    { 0, 0, 14, 10, 0x0},
    { 0, 0,  9,  5, 0x10},
    { 0, 0,  4,  0, 0x10},
};

confcode_serdes_patch_t rtl839x_eee_enable1[] = {
    //#qsgmii_lpi_tx_en=1, qsgmii_lpi_rx_en=1
    { 0, 0,  9,  8, 0x3},
};

void rtl8390_serdes_eee_enable(const int sds_num)
{
    int eee_sds_addr0 = 0xa0e0;
    int eee_sds_addr1 = 0xa014;
    int eee_sds_addr_ofst = 0x400;
    int addr_ofst;
    int idx;

    addr_ofst = (eee_sds_addr_ofst * (sds_num / 2)) + (0x100 * (sds_num % 2));

    for (idx = 0; idx < (sizeof(rtl839x_eee_enable0)/sizeof(confcode_serdes_patch_t)); ++idx)
    {
        rtl839x_eee_enable0[idx].reg = eee_sds_addr0 + addr_ofst;
        SERDES_PATCH_SET_CHK(rtl839x_eee_enable0[idx]);
    }

    for (idx = 0; idx < (sizeof(rtl839x_eee_enable1)/sizeof(confcode_serdes_patch_t)); ++idx)
    {
        rtl839x_eee_enable1[idx].reg = eee_sds_addr1 + addr_ofst;
        SERDES_PATCH_SET_CHK(rtl839x_eee_enable1[idx]);
    }

    return ;
}   /* end of rtl8390_serdes_eee_enable */

static confcode_mac_regval_t *rtl839x_5G_serdesDB_qa[] =
{
    rtl839x_5G_serdes_0_qa,
    rtl839x_5G_serdes_1_qa,
    rtl839x_5G_serdes_2_qa,
    rtl839x_5G_serdes_3_qa,
    rtl839x_5G_serdes_4_qa,
    rtl839x_5G_serdes_5_qa,
    rtl839x_5G_serdes_6_qa,
    rtl839x_5G_serdes_7_qa,
    rtl839x_5G_serdes_8_9_qa,
    rtl839x_5G_serdes_8_9_qa,
    rtl839x_5G_serdes_10_qa,
    rtl839x_5G_serdes_11_qa,
    rtl839x_5G_serdes_12_qa,
    rtl839x_5G_serdes_12_qa,
};

static confcode_mac_regval_t *rtl839x_5G_serdesDB[] =
{
    rtl839x_5G_serdes_0,
    rtl839x_5G_serdes_1,
    rtl839x_5G_serdes_2,
    rtl839x_5G_serdes_3,
    rtl839x_5G_serdes_4,
    rtl839x_5G_serdes_5,
    rtl839x_5G_serdes_6,
    rtl839x_5G_serdes_7,
    rtl839x_5G_serdes_8_9,
    rtl839x_5G_serdes_8_9,
    rtl839x_5G_serdes_10,
    rtl839x_5G_serdes_11,
    rtl839x_5G_serdes_12,
};

int rtl8390_serdes_chk(const rtk_switch_model_t *pModel, const int sdsId)
{
    uint32  base = 0xbb00a078, chkPos;
    int     id;

    chkPos = base + (0x400 * (sdsId / 2)) + (0x100 * (sdsId % 2));

    if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
    {
        id = sdsId / 2;
    }
    else
        id = sdsId;

    if (MEM32_READ(chkPos) != 0x1ff0000)
    {
        OSAL_PRINTF("[WARN] Serdes %u initail fail %x %x\n", id, chkPos, MEM32_READ(chkPos));
        return 1;
    }
    else
    {
        //OSAL_PRINTF("Serdes %u initail OK\n", id);
        return 0;
    }

    return 0;
}

void
rtl8390_5G_serdes_config(const rtk_switch_model_t *pModel,
    const int sdsId, int phyIdx)
{
    confcode_mac_regval_t   *serdesConfig;
    int32                   try, idx;

    try = 1;
    do {
        if (RTK_CHIP_NONE == pModel->phy.list[phyIdx].chip)
        {
            #if 1
            MAC_REG_SET_CHK(0xbb78, 0x1c005);
            return;
            #else
            for (idx = 0; idx < (sizeof(rtl839x_fiber_serdes_12_13) / sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl839x_fiber_serdes_12_13[idx].reg, rtl839x_fiber_serdes_12_13[idx].val);
            }
            #endif
        }
        else if (NULL != strstr(pModel->name, "_QA"))
        {
            if (10 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes10_a2d_clk_edge_qa)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes10_a2d_clk_edge_qa[idx].reg, rtl839x_serdes10_a2d_clk_edge_qa[idx].val);
                }
            }
            else if (11 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes11_a2d_clk_edge_qa)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes11_a2d_clk_edge_qa[idx].reg, rtl839x_serdes11_a2d_clk_edge_qa[idx].val);
                }
            }

            serdesConfig = rtl839x_5G_serdesDB_qa[sdsId];
            idx = 0;
            while (serdesConfig[idx].reg != 0)
            {
                MAC_REG_SET_CHK(serdesConfig[idx].reg, serdesConfig[idx].val);
                ++idx;
            }
        }
        else
        {
            if (10 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes10_a2d_clk_edge)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes10_a2d_clk_edge[idx].reg, rtl839x_serdes10_a2d_clk_edge[idx].val);
                }
            }
            else if (11 == sdsId)
            {
                for (idx = 0; idx < (sizeof(rtl839x_serdes11_a2d_clk_edge)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl839x_serdes11_a2d_clk_edge[idx].reg, rtl839x_serdes11_a2d_clk_edge[idx].val);
                }
            }

            serdesConfig = rtl839x_5G_serdesDB[sdsId];
            idx = 0;
            while (serdesConfig[idx].reg != 0)
            {
                MAC_REG_SET_CHK(serdesConfig[idx].reg, serdesConfig[idx].val);
                ++idx;
            }
        }

        OSAL_MDELAY(500);

        if (0 == rtl8390_serdes_chk(pModel, sdsId))
            break;
    } while (1);

    return;
}   /* end of rtl8390_5G_serdes_config */

void rtl839x_serdes_rst(const uint32 sds_num)
{
    uint32  sdsReg[] = {0xA328, 0xA728, 0xAB28, 0xAF28, 0xB320, 0xB728, 0xBB20};
    uint32  addr_ofst = 0x400;
    uint32  ofst, sdsAddr;

    ofst = addr_ofst*(sds_num/2);
    sdsAddr = sdsReg[sds_num/2] + (0x80 * (sds_num % 2));
    //if {[expr int(fmod(sds_num,2))] == 1} {set ofst [expr addr_ofst*(sds_num/2) + 0x100]} {set ofst [expr addr_ofst*(sds_num/2)]}
    if (sds_num < 8 || sds_num == 10 || sds_num == 11) {
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x0050);
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x00f0);
        SERDES_SET(0xa3c0 + ofst,  31 , 16 , 0x0);

        SERDES_SET(sdsAddr,  0 , 0 , 0x0);
        SERDES_SET(sdsAddr,  9 , 9 , 0x1);
        OSAL_MDELAY(100);
        SERDES_SET(sdsAddr,  9 , 9 , 0x0);
    } else if (sds_num == 8 || sds_num == 9) {
        SERDES_SET(0xb3f8,  19 , 16 , 0x5);
        OSAL_MDELAY(500);
        SERDES_SET(0xb3f8,  19 , 16 , 0xf);
        SERDES_SET(0xb3f8,  19 , 16 , 0x0);

        SERDES_SET(0xb320,  3 , 3 , 0x0);
        SERDES_SET(0xb340,  15 , 15 , 0x1);
        OSAL_MDELAY(100);
        SERDES_SET(0xb340,  15 , 15 , 0x0);
    } else if (sds_num == 12 || sds_num == 13) {
        SERDES_SET(0xbbf8,  19 , 16 , 0x5);
        OSAL_MDELAY(500);
        SERDES_SET(0xbbf8,  19 , 16 , 0xf);
        SERDES_SET(0xbbf8,  19 , 16 , 0x0);

        SERDES_SET(0xbb20,  3 , 3 , 0x0);
        SERDES_SET(0xbb40,  15 , 15 , 0x1);
        OSAL_MDELAY(100);
        SERDES_SET(0xbb40,  15 , 15 , 0x0);
    } else {
        printf( "sds number doesn't exist");
        return;
    }
    SERDES_SET(0xa004 + ofst,  31 , 16 , 0x7146);
    OSAL_MDELAY(100);
    SERDES_SET(0xa004 + ofst,  31 , 16 , 0x7106);
    SERDES_SET(0xa004 + ofst + 0x100,  31 , 16 , 0x7146);
    OSAL_MDELAY(100);
    SERDES_SET(0xa004 + ofst + 0x100,  31 , 16 , 0x7106);
}   /* end of rtl839x_serdes_rst */

void rtl839x_serdes_rst_all(void)
{
    uint32 sdsId;

    for (sdsId = 0; sdsId < 13; ++sdsId)
        rtl839x_serdes_rst(sdsId);
}

void rtl839x_serdes_patch_init(void)
{
    uint32 ofst_list[] = {0,0x800};
    uint32 ofst_list1[] = {0x80,0x880};
    uint32 ofst_list2[] = {0,0x80,0x400,0x480,0x800,0x880,0xc00,0xc80,0x1400,0x1480};
    //uint32  sdsList[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
    uint32  i, ofst;

    SERDES_SET(0xb300,  15 , 0  , 0x5800);
    SERDES_SET(0xb300,  31 , 16 , 0x4000);
    SERDES_SET(0xb304,  15 , 0  , 0x5400);
    SERDES_SET(0xb304,  31 , 16 , 0x0000);
    SERDES_SET(0xb308,  15 , 0  , 0x0000);
    SERDES_SET(0xb308,  31 , 16 , 0x4000);
    SERDES_SET(0xb30c,  15 , 0  , 0x4000);
    SERDES_SET(0xb30c,  31 , 16 , 0xffff);
    SERDES_SET(0xb310,  15 , 0  , 0xffff);
    SERDES_SET(0xb310,  31 , 16 , 0x806f);
    SERDES_SET(0xb314,  15 , 0  , 0x0004);
    SERDES_SET(0xb314,  31 , 16 , 0x0000);
    SERDES_SET(0xb318,  15 , 0  , 0x0000);
    SERDES_SET(0xb318,  31 , 16 , 0x0000);
    SERDES_SET(0xb31c,  15 , 0  , 0x0a00);
    SERDES_SET(0xb31c,  31 , 16 , 0x2000);
    SERDES_SET(0xb320,  15 , 0  , 0xf00e);
    SERDES_SET(0xb320,  31 , 16 , 0xf04a);
    SERDES_SET(0xb324,  15 , 0  , 0x97b3);
    SERDES_SET(0xb324,  31 , 16 , 0x5318);
    SERDES_SET(0xb328,  15 , 0  , 0x0f03);
    SERDES_SET(0xb328,  31 , 16 , 0x0);
    SERDES_SET(0xb32c,  15 , 0  , 0x0000);
    SERDES_SET(0xb32c,  31 , 16 , 0x0000);
    SERDES_SET(0xb330,  15 , 0  , 0x0000);
    SERDES_SET(0xb330,  31 , 16 , 0x0000);
    SERDES_SET(0xb334,  15 , 0  , 0xffff);
    SERDES_SET(0xb334,  31 , 16 , 0x0000);
    SERDES_SET(0xb338,  15 , 0  , 0x1203);
    SERDES_SET(0xb338,  31 , 16 , 0x0000);
    SERDES_SET(0xb33c,  15 , 0  , 0xa052);
    SERDES_SET(0xb33c,  31 , 16 , 0x9a00);
    SERDES_SET(0xb340,  15 , 0  , 0x00f5);
    SERDES_SET(0xb340,  31 , 16 , 0xf000);
    SERDES_SET(0xb344,  15 , 0  , 0x41ff);
    SERDES_SET(0xb344,  31 , 16 , 0x0000);
    SERDES_SET(0xb348,  15 , 0  , 0x39ff);
    SERDES_SET(0xb348,  31 , 16 , 0x3340);
    SERDES_SET(0xb34c,  15 , 0  , 0x40aa);
    SERDES_SET(0xb34c,  31 , 16 , 0x0000);
    SERDES_SET(0xb350,  15 , 0  , 0x801f);
    SERDES_SET(0xb350,  31 , 16 , 0x0000);
    SERDES_SET(0xb354,  15 , 0  , 0x619c);
    SERDES_SET(0xb354,  31 , 16 , 0xffed);
    SERDES_SET(0xb358,  15 , 0  , 0x29ff);
    SERDES_SET(0xb358,  31 , 16 , 0x29ff);
    SERDES_SET(0xb35c,  15 , 0  , 0x4e10);
    SERDES_SET(0xb35c,  31 , 16 , 0x4e10);
    SERDES_SET(0xb360,  15 , 0  , 0x0000);
    SERDES_SET(0xb360,  31 , 16 , 0x0000);
    SERDES_SET(0xb380,  15 , 0  , 0x5800);
    SERDES_SET(0xb380,  31 , 16 , 0x4000);
    SERDES_SET(0xb384,  15 , 0  , 0x5000);
    SERDES_SET(0xb384,  31 , 16 , 0x0000);
    SERDES_SET(0xb388,  15 , 0  , 0x0000);
    SERDES_SET(0xb388,  31 , 16 , 0x4000);
    SERDES_SET(0xb38c,  15 , 0  , 0x4000);
    SERDES_SET(0xb38c,  31 , 16 , 0xffff);
    SERDES_SET(0xb390,  15 , 0  , 0xffff);
    SERDES_SET(0xb390,  31 , 16 , 0x806f);
    SERDES_SET(0xb394,  15 , 0  , 0x0004);
    SERDES_SET(0xb394,  31 , 16 , 0x0000);
    SERDES_SET(0xb398,  15 , 0  , 0x0000);
    SERDES_SET(0xb398,  31 , 16 , 0x0000);
    SERDES_SET(0xb39c,  15 , 0  , 0x0a00);
    SERDES_SET(0xb39c,  31 , 16 , 0x2000);
    SERDES_SET(0xb3a0,  15 , 0  , 0xf00e);
    SERDES_SET(0xb3a0,  31 , 16 , 0xfdab);
    SERDES_SET(0xb3a4,  15 , 0  , 0x96ea);
    SERDES_SET(0xb3a4,  31 , 16 , 0x5318);
    SERDES_SET(0xb3a8,  15 , 0  , 0x0f03);
    SERDES_SET(0xb3a8,  31 , 16 , 0);
    SERDES_SET(0xb3ac,  15 , 0  , 0x0000);
    SERDES_SET(0xb3ac,  31 , 16 , 0x0000);
    SERDES_SET(0xb3b0,  15 , 0  , 0x0000);
    SERDES_SET(0xb3b0,  31 , 16 , 0x0000);
    SERDES_SET(0xb3b4,  15 , 0  , 0xffff);
    SERDES_SET(0xb3b4,  31 , 16 , 0x0000);
    SERDES_SET(0xb3b8,  15 , 0  , 0x1203);
    SERDES_SET(0xb3b8,  31 , 16 , 0x0000);
    SERDES_SET(0xb3bc,  15 , 0  , 0xa052);
    SERDES_SET(0xb3bc,  31 , 16 , 0x9a00);
    SERDES_SET(0xb3c0,  15 , 0  , 0x00f5);
    SERDES_SET(0xb3c0,  31 , 16 , 0xf000);
    SERDES_SET(0xb3c4,  15 , 0  , 0x4079);
    SERDES_SET(0xb3c4,  31 , 16 , 0x0000);
    SERDES_SET(0xb3c8,  15 , 0  , 0x93fa);
    SERDES_SET(0xb3c8,  31 , 16 , 0x3340);
    SERDES_SET(0xb3cc,  15 , 0  , 0x4280);
    SERDES_SET(0xb3cc,  31 , 16 , 0x0000);
    SERDES_SET(0xb3d0,  15 , 0  , 0x801f);
    SERDES_SET(0xb3d0,  31 , 16 , 0x0000);
    SERDES_SET(0xb3d4,  15 , 0  , 0x619c);
    SERDES_SET(0xb3d4,  31 , 16 , 0xffed);
    SERDES_SET(0xb3d8,  15 , 0  , 0x29ff);
    SERDES_SET(0xb3d8,  31 , 16 , 0x29ff);
    SERDES_SET(0xb3dc,  15 , 0  , 0x4c50);
    SERDES_SET(0xb3dc,  31 , 16 , 0x4c50);
    SERDES_SET(0xb3e0,  15 , 0  , 0x0000);
    SERDES_SET(0xb3e0,  31 , 16 , 0x0000);
    SERDES_SET(0xbb00,  15 , 0  , 0x5800);
    SERDES_SET(0xbb00,  31 , 16 , 0x4000);
    SERDES_SET(0xbb04,  15 , 0  , 0x5400);
    SERDES_SET(0xbb04,  31 , 16 , 0x0000);
    SERDES_SET(0xbb08,  15 , 0  , 0x0000);
    SERDES_SET(0xbb08,  31 , 16 , 0x4000);
    SERDES_SET(0xbb0c,  15 , 0  , 0x4000);
    SERDES_SET(0xbb0c,  31 , 16 , 0xffff);
    SERDES_SET(0xbb10,  15 , 0  , 0xffff);
    SERDES_SET(0xbb10,  31 , 16 , 0x806f);
    SERDES_SET(0xbb14,  15 , 0  , 0x0004);
    SERDES_SET(0xbb14,  31 , 16 , 0x0000);
    SERDES_SET(0xbb18,  15 , 0  , 0x0000);
    SERDES_SET(0xbb18,  31 , 16 , 0x0000);
    SERDES_SET(0xbb1c,  15 , 0  , 0x0a00);
    SERDES_SET(0xbb1c,  31 , 16 , 0x2000);
    SERDES_SET(0xbb20,  15 , 0  , 0xf00e);
    SERDES_SET(0xbb20,  31 , 16 , 0xf04a);
    SERDES_SET(0xbb24,  15 , 0  , 0x97b3);
    SERDES_SET(0xbb24,  31 , 16 , 0x5318);
    SERDES_SET(0xbb28,  15 , 0  , 0x0f03);
    SERDES_SET(0xbb28,  31 , 16 , 0x0);
    SERDES_SET(0xbb2c,  15 , 0  , 0x0000);
    SERDES_SET(0xbb2c,  31 , 16 , 0x0000);
    SERDES_SET(0xbb30,  15 , 0  , 0x0000);
    SERDES_SET(0xbb30,  31 , 16 , 0x0000);
    SERDES_SET(0xbb34,  15 , 0  , 0xffff);
    SERDES_SET(0xbb34,  31 , 16 , 0x0000);
    SERDES_SET(0xbb38,  15 , 0  , 0x1203);
    SERDES_SET(0xbb38,  31 , 16 , 0x0000);
    SERDES_SET(0xbb3c,  15 , 0  , 0xa052);
    SERDES_SET(0xbb3c,  31 , 16 , 0x9a00);
    SERDES_SET(0xbb40,  15 , 0  , 0x00f5);
    SERDES_SET(0xbb40,  31 , 16 , 0xf000);
    SERDES_SET(0xbb44,  15 , 0  , 0x41ff);
    SERDES_SET(0xbb44,  31 , 16 , 0x0000);
    SERDES_SET(0xbb48,  15 , 0  , 0x39ff);
    SERDES_SET(0xbb48,  31 , 16 , 0x3340);
    SERDES_SET(0xbb4c,  15 , 0  , 0x40aa);
    SERDES_SET(0xbb4c,  31 , 16 , 0x0000);
    SERDES_SET(0xbb50,  15 , 0  , 0x801f);
    SERDES_SET(0xbb50,  31 , 16 , 0x0000);
    SERDES_SET(0xbb54,  15 , 0  , 0x619c);
    SERDES_SET(0xbb54,  31 , 16 , 0xffed);
    SERDES_SET(0xbb58,  15 , 0  , 0x29ff);
    SERDES_SET(0xbb58,  31 , 16 , 0x29ff);
    SERDES_SET(0xbb5c,  15 , 0  , 0x4e10);
    SERDES_SET(0xbb5c,  31 , 16 , 0x4e10);
    SERDES_SET(0xbb60,  15 , 0  , 0x0000);
    SERDES_SET(0xbb60,  31 , 16 , 0x0000);
    SERDES_SET(0xbb80,  15 , 0  , 0x5800);
    SERDES_SET(0xbb80,  31 , 16 , 0x4000);
    SERDES_SET(0xbb84,  15 , 0  , 0x5000);
    SERDES_SET(0xbb84,  31 , 16 , 0x0000);
    SERDES_SET(0xbb88,  15 , 0  , 0x0000);
    SERDES_SET(0xbb88,  31 , 16 , 0x4000);
    SERDES_SET(0xbb8c,  15 , 0  , 0x4000);
    SERDES_SET(0xbb8c,  31 , 16 , 0xffff);
    SERDES_SET(0xbb90,  15 , 0  , 0xffff);
    SERDES_SET(0xbb90,  31 , 16 , 0x806f);
    SERDES_SET(0xbb94,  15 , 0  , 0x0004);
    SERDES_SET(0xbb94,  31 , 16 , 0x0000);
    SERDES_SET(0xbb98,  15 , 0  , 0x0000);
    SERDES_SET(0xbb98,  31 , 16 , 0x0000);
    SERDES_SET(0xbb9c,  15 , 0  , 0x0a00);
    SERDES_SET(0xbb9c,  31 , 16 , 0x2000);
    SERDES_SET(0xbba0,  15 , 0  , 0xf00e);
    SERDES_SET(0xbba0,  31 , 16 , 0xfdab);
    SERDES_SET(0xbba4,  15 , 0  , 0x96ea);
    SERDES_SET(0xbba4,  31 , 16 , 0x5318);
    SERDES_SET(0xbba8,  15 , 0  , 0x0f03);
    SERDES_SET(0xbba8,  31 , 16 , 0);
    SERDES_SET(0xbbac,  15 , 0  , 0x0000);
    SERDES_SET(0xbbac,  31 , 16 , 0x0000);
    SERDES_SET(0xbbb0,  15 , 0  , 0x0000);
    SERDES_SET(0xbbb0,  31 , 16 , 0x0000);
    SERDES_SET(0xbbb4,  15 , 0  , 0xffff);
    SERDES_SET(0xbbb4,  31 , 16 , 0x0000);
    SERDES_SET(0xbbb8,  15 , 0  , 0x1203);
    SERDES_SET(0xbbb8,  31 , 16 , 0x0000);
    SERDES_SET(0xbbbc,  15 , 0  , 0xa052);
    SERDES_SET(0xbbbc,  31 , 16 , 0x9a00);
    SERDES_SET(0xbbc0,  15 , 0  , 0x00f5);
    SERDES_SET(0xbbc0,  31 , 16 , 0xf000);
    SERDES_SET(0xbbc4,  15 , 0  , 0x4079);
    SERDES_SET(0xbbc4,  31 , 16 , 0x0000);
    SERDES_SET(0xbbc8,  15 , 0  , 0x93fa);
    SERDES_SET(0xbbc8,  31 , 16 , 0x3340);
    SERDES_SET(0xbbcc,  15 , 0  , 0x4280);
    SERDES_SET(0xbbcc,  31 , 16 , 0x0000);
    SERDES_SET(0xbbd0,  15 , 0  , 0x801f);
    SERDES_SET(0xbbd0,  31 , 16 , 0x0000);
    SERDES_SET(0xbbd4,  15 , 0  , 0x619c);
    SERDES_SET(0xbbd4,  31 , 16 , 0xffed);
    SERDES_SET(0xbbd8,  15 , 0  , 0x29ff);
    SERDES_SET(0xbbd8,  31 , 16 , 0x29ff);
    SERDES_SET(0xbbdc,  15 , 0  , 0x4c50);
    SERDES_SET(0xbbdc,  31 , 16 , 0x4c50);
    SERDES_SET(0xbbe0,  15 , 0  , 0x0000);
    SERDES_SET(0xbbe0,  31 , 16 , 0x0000);
    SERDES_SET(0xb018,  15 , 0  , 0x08ec);
    SERDES_SET(0xb118,  15 , 0  , 0x08ec);
    SERDES_SET(0xb818,  15 , 0  , 0x08ec);
    SERDES_SET(0xb918,  15 , 0  , 0x08ec);
    SERDES_SET(0xb3fc,  31 , 16 , 0x3f);
    SERDES_SET(0xbbfc,  31 , 16 , 0x3f);
    SERDES_SET(0xb00c,  30 , 30 , 1);
    SERDES_SET(0xb10c,  30 , 30 , 1);
    SERDES_SET(0xb40c,  30 , 30 , 1);
    SERDES_SET(0xb50c,  30 , 30 , 1);
    SERDES_SET(0xb80c,  30 , 30 , 1);
    SERDES_SET(0xb90c,  30 , 30 , 1);
    for (i = 0; i < sizeof(ofst_list)/sizeof(uint32); ++i)
    {
        ofst = ofst_list[i];
        SERDES_SET(0xb350 + ofst,  31 , 16 , 0x417f);
        SERDES_SET(0xb338 + ofst,  9  , 9  , 0);
        SERDES_SET(0xb338 + ofst,  12 , 10 , 0x0);
        SERDES_SET(0xb338 + ofst,  5  , 3  , 0x5);
        SERDES_SET(0xb338 + ofst,  8  , 6  , 0x0);
        SERDES_SET(0xb338 + ofst,  2  , 0  , 0x2);
        SERDES_SET(0xb340 + ofst,  31 , 16 , 0xc440);
        SERDES_SET(0xb34c + ofst,  3  , 3  , 0);
        SERDES_SET(0xb308 + ofst,  31 , 16 , 0x8000);
        SERDES_SET(0xb30c + ofst,  15 , 0  , 0x8000);
        SERDES_SET(0xb314 + ofst,  15 , 0  , 0x0);
        SERDES_SET(0xb33c + ofst,  15 , 0  , 0x2);
        SERDES_SET(0xb33c + ofst,  31 , 16 , 0xbe00);
        SERDES_SET(0xb35c + ofst,  10 , 10 , 0);
        SERDES_SET(0xb35c + ofst,  26 , 26 , 0);
        SERDES_SET(0xb35c + ofst,  14 , 14 , 0);
        SERDES_SET(0xb35c + ofst,  30 , 30 , 0);
        SERDES_SET(0xb320 + ofst,  5  , 5  , 0);
        SERDES_SET(0xb350 + ofst,  24 , 24 , 0);
        SERDES_SET(0xb304 + ofst,  31 , 28 , 0xf);
        SERDES_SET(0xb33c + ofst,  29 , 28 , 0x3);
        SERDES_SET(0xb33c + ofst,  27 , 25 , 0x7);
        SERDES_SET(0xb340 + ofst,  31 , 31 , 1);
        SERDES_SET(0xb340 + ofst,  30 , 30 , 1);
        SERDES_SET(0xb340 + ofst,  29 , 29 , 0);
        SERDES_SET(0xb340 + ofst,  28 , 28 , 0);
        SERDES_SET(0xb340 + ofst,  27 , 25 , 0x2);
        SERDES_SET(0xb340 + ofst,  24 , 22 , 0x2);
        SERDES_SET(0xb340 + ofst,  21 , 19 , 0x0);
        SERDES_SET(0xb340 + ofst,  18 , 16 , 0x0);
        SERDES_SET(0xb358 + ofst,  9  , 9  , 1);
        SERDES_SET(0xb358 + ofst,  25 , 25 , 1);
        SERDES_SET(0xb350 + ofst,  5  , 5  , 1);
        SERDES_SET(0xb350 + ofst,  6  , 6  , 0);
        SERDES_SET(0xb338 + ofst,  15 , 15 , 0);
        SERDES_SET(0xb320 + ofst,  15 , 12 , 0x0);
        SERDES_SET(0xb324 + ofst,  20 , 20 , 0);
        SERDES_SET(0xb324 + ofst,  25 , 25 , 0);
        SERDES_SET(0xb324 + ofst,  19 , 16 , 0x8);
        SERDES_SET(0xb324 + ofst,  24 , 21 , 0x8);
    }

    for (i = 0; i < sizeof(ofst_list1)/sizeof(uint32); ++i)
    {
        ofst = ofst_list1[i];
        SERDES_SET(0xb350 + ofst,  31 , 16 , 0x417f);
        SERDES_SET(0xb338 + ofst,  9  , 9  , 0);
        SERDES_SET(0xb338 + ofst,  12 , 10 , 0x0);
        SERDES_SET(0xb338 + ofst,  5  , 3  , 0x5);
        SERDES_SET(0xb338 + ofst,  8  , 6  , 0x0);
        SERDES_SET(0xb338 + ofst,  2  , 0  , 0x2);
        SERDES_SET(0xb340 + ofst,  31 , 16 , 0xc440);
        SERDES_SET(0xb308 + ofst,  31 , 16 , 0x8000);
        SERDES_SET(0xb30c + ofst,  15 , 0  , 0x8000);
        SERDES_SET(0xb314 + ofst,  15 , 0  , 0x0);
        SERDES_SET(0xb33c + ofst,  15 , 0  , 0x2);
        SERDES_SET(0xb33c + ofst,  31 , 16 , 0xbe00);
        SERDES_SET(0xb320 + ofst,  5  , 5  , 0);
        SERDES_SET(0xb350 + ofst,  24 , 24 , 0);
        SERDES_SET(0xb304 + ofst,  31 , 28 , 0xf);
        SERDES_SET(0xb33c + ofst,  29 , 28 , 0x3);
        SERDES_SET(0xb33c + ofst,  27 , 25 , 0x7);
        SERDES_SET(0xb340 + ofst,  31 , 31 , 1);
        SERDES_SET(0xb340 + ofst,  30 , 30 , 1);
        SERDES_SET(0xb340 + ofst,  29 , 29 , 0);
        SERDES_SET(0xb340 + ofst,  28 , 28 , 0);
        SERDES_SET(0xb340 + ofst,  27 , 25 , 0x2);
        SERDES_SET(0xb340 + ofst,  24 , 22 , 0x2);
        SERDES_SET(0xb340 + ofst,  21 , 19 , 0x0);
        SERDES_SET(0xb340 + ofst,  18 , 16 , 0x0);
        SERDES_SET(0xb358 + ofst,  9  , 9  , 1);
        SERDES_SET(0xb358 + ofst,  25 , 25 , 1);
        SERDES_SET(0xb350 + ofst,  5  , 5  , 1);
        SERDES_SET(0xb350 + ofst,  6  , 6  , 0);
        SERDES_SET(0xb338 + ofst,  15 , 15 , 0);
        SERDES_SET(0xb320 + ofst,  15 , 12 , 0x0);
        SERDES_SET(0xb324 + ofst,  20 , 20 , 0);
        SERDES_SET(0xb324 + ofst,  25 , 25 , 0);
        SERDES_SET(0xb324 + ofst,  19 , 16 , 0x8);
        SERDES_SET(0xb324 + ofst,  24 , 21 , 0x8);
    }

    SERDES_SET(0xab10,  15 , 0  , 0x8c6a);
    SERDES_SET(0xb710,  15 , 0  , 0x8c6a);

    for (i = 0; i < sizeof(ofst_list2)/sizeof(uint32); ++i)
    {
        ofst = ofst_list2[i];
        SERDES_SET(0xa320 + ofst,  31 , 31 , 0);
        SERDES_SET(0xa320 + ofst,  30 , 28 , 0x1);
        SERDES_SET(0xa320 + ofst,  27 , 25 , 0x2);
        SERDES_SET(0xa320 + ofst,  24 , 22 , 0x3);
        SERDES_SET(0xa32c + ofst,  15 , 15 , 0);
        SERDES_SET(0xa310 + ofst,  3  , 3  , 0);
        SERDES_SET(0xa30c + ofst,  25 , 25 , 0);
        SERDES_SET(0xa30c + ofst,  24 , 24 , 0);
        SERDES_SET(0xa328 + ofst,  1  , 1  , 1);
        SERDES_SET(0xa328 + ofst,  31 , 28 , 0xc);
        SERDES_SET(0xa32c + ofst,  12 , 12 , 0);
        SERDES_SET(0xa330 + ofst,  5  , 0  , 0x6);
        SERDES_SET(0xa310 + ofst,  6  , 6  , 0);
        SERDES_SET(0xa310 + ofst,  11 , 11 , 0);
        SERDES_SET(0xa310 + ofst,  15 , 12 , 0x8);
        SERDES_SET(0xa310 + ofst,  10 , 7  , 0x8);
    }
}   /* end of rtl839x_serdes_patch_init */

//for RTL8353M serdes patch
void rtl839x_5x_serdes_patch_init(void)
{
    SERDES_SET(0xb018,  15 , 0  , 0x08e4);
    SERDES_SET(0xb118,  15 , 0  , 0x08e4);
    SERDES_SET(0xb3fc,  31 , 16 , 0x2b);
    SERDES_SET(0xb338,  15 , 0  , 0x0722);
    SERDES_SET(0xb3b8,  15 , 0  , 0x0722);
    SERDES_SET(0xb340,  15 , 0  , 0x18f5);
    SERDES_SET(0xb3c0,  15 , 0  , 0x18f5);

    SERDES_SET(0xa340, 31, 0, 0xc400043f);
    SERDES_SET(0xa3c0, 31, 0, 0xc40043f);
    SERDES_SET(0xa740, 31, 0, 0xc400043f);
    SERDES_SET(0xa7c0, 31, 0, 0xc40043f);
    SERDES_SET(0xab40, 31, 0, 0xc400043f);
    SERDES_SET(0xabc0, 31, 0, 0xc40043f);
    SERDES_SET(0xaf40, 31, 0, 0xc400043f);
    SERDES_SET(0xafc0, 31, 0, 0xc40043f);
    SERDES_SET(0xb740, 31, 0, 0xc400043f);
    SERDES_SET(0xb7c0, 31, 0, 0xc40043f);
    SERDES_SET(0xb3f8, 31, 0, 0x4c00000);

}   /* end of rtl839x_5x_serdes_patch_init */

void rtl839x_serdes_cmu(uint32 enable, uint32 sds_num)
{
    uint32 addr5g;
    uint32 addr10g;
    uint32 val;
    uint32 addr_ofst = 0x400;
    uint32 ofst;

    ofst = addr_ofst*(sds_num/2);
    if (sds_num % 2 == 0) {
        addr5g = 20;
        addr10g = 16;
    } else {
        addr5g = 22;
        addr10g = 18;
    }

    if (enable == 1) {
        val = 1;
    } else {
        val = 0;
    }

    if (sds_num < 8 || sds_num == 10 || sds_num == 11) {
        SERDES_SET(0xa3c0 + ofst,  addr5g, addr5g, 1);
        SERDES_SET(0xa3c0 + ofst,  (addr5g + 1), (addr5g + 1), val);
    } else if (sds_num == 8 || sds_num == 9) {
        SERDES_SET(0xb3f8,  addr10g, addr10g, 1);
        SERDES_SET(0xb3f8,  (addr10g + 1), (addr10g + 1), val);
    } else if (sds_num == 12 || sds_num == 13) {
        SERDES_SET(0xbbf8,  addr10g, addr10g, 1);
        SERDES_SET(0xbbf8,  (addr10g + 1), (addr10g + 1), val);
    } else {
        printf( "sds number doesn't exist");
        return;
    }
}

void rtl839x_93m_rst_sys (void)
{
    rtl8390_drv_macPhyPatch1();

    SERDES_SET(0x0014,  4  , 4  , 1);

    OSAL_MDELAY(500);
    SERDES_SET(0xb018,  15 , 0  , 0x08ec);
    SERDES_SET(0xb118,  15 , 0  , 0x08ec);
    SERDES_SET(0xb818,  15 , 0  , 0x08ec);
    SERDES_SET(0xb918,  15 , 0  , 0x08ec);
    SERDES_SET(0xb3fc,  31 , 16 , 0x3f);
    SERDES_SET(0xbbfc,  31 , 16 , 0x3f);
    SERDES_SET(0xb00c,  30 , 30 , 1);
    SERDES_SET(0xb10c,  30 , 30 , 1);
    SERDES_SET(0xb40c,  30 , 30 , 1);
    SERDES_SET(0xb50c,  30 , 30 , 1);
    SERDES_SET(0xb80c,  30 , 30 , 1);
    SERDES_SET(0xb90c,  30 , 30 , 1);

    rtl8390_drv_macPhyPatch2();
}

void rtl839x_53m_rst_sys (void)
{
    uint32 sdsList[] = {1, 3, 5, 7, 11};
    uint32 i;

    rtl8390_drv_macPhyPatch1();

    SERDES_SET(0x0014,  4  , 4  , 1);

    OSAL_MDELAY(500);
    SERDES_SET(0xb018,  15 , 0  , 0x08ec);
    SERDES_SET(0xb118,  15 , 0  , 0x08ec);
    SERDES_SET(0xb818,  15 , 0  , 0x08ec);
    SERDES_SET(0xb918,  15 , 0  , 0x08ec);
    SERDES_SET(0xb3fc,  31 , 16 , 0x3f);
    SERDES_SET(0xbbfc,  31 , 16 , 0x3f);
    SERDES_SET(0xb00c,  30 , 30 , 1);
    SERDES_SET(0xb10c,  30 , 30 , 1);
    SERDES_SET(0xb40c,  30 , 30 , 1);
    SERDES_SET(0xb50c,  30 , 30 , 1);
    SERDES_SET(0xb80c,  30 , 30 , 1);
    SERDES_SET(0xb90c,  30 , 30 , 1);

    SERDES_SET(0xb018,  15 , 0  , 0x08e4);
    SERDES_SET(0xb118,  15 , 0  , 0x08e4);
    SERDES_SET(0xb3fc,  21 , 16 , 0x2b);

    for (i = 0; i < sizeof(sdsList)/sizeof(uint32); ++i)
        rtl839x_serdes_cmu(0, sdsList[i]);

    SERDES_SET(0xb3f8,  23 , 23 , 1);
    SERDES_SET(0xb3f8,  22 , 22 , 1);
    SERDES_SET(0xb3f8,  26 , 26 , 1);
    SERDES_SET(0xb3f8,  27 , 27 , 0);
    SERDES_SET(0xbbf8,  23 , 23 , 1);
    SERDES_SET(0xbbf8,  22 , 22 , 1);
    SERDES_SET(0xbbf8,  26 , 26 , 1);
    SERDES_SET(0xbbf8,  27 , 27 , 0);

    rtl8390_drv_macPhyPatch2();
}

#if defined(CONFIG_RTL8396M_DEMO)
void rtl8396_serdes_10g_rx_rst (int sds_num)
{
    uint32 ofst;

    switch (sds_num)
    {
        case 8:
            ofst = 0x0;
            break;
        case 12:
            ofst = 0x800;
            break;
        default:
            return;
    }

    SERDES_SET(0xb320 + ofst,  3,  3, 0x0);
    SERDES_SET(0xb340 + ofst, 15, 15, 0x1);
    OSAL_MDELAY(100);
    SERDES_SET(0xb340 + ofst, 15, 15, 0x0);

    SERDES_SET(0xB3F8 + ofst, 29, 28, 0x3);
    SERDES_SET(0xB3F8 + ofst, 17, 16, 0x3);
    SERDES_SET(0xB3F8 + ofst, 21, 20, 0x1);
    SERDES_SET(0xB3F8 + ofst, 25, 24, 0x3);

    SERDES_SET(0xB284 + ofst,  6,  6, 0x1);
    SERDES_SET(0xB284 + ofst,  5,  5, 0x0);
    SERDES_SET(0xB284 + ofst,  4,  4, 0x1);
    SERDES_SET(0xB284 + ofst,  3,  3, 0x1);
    SERDES_SET(0xB284 + ofst,  2,  2, 0x0);
    SERDES_SET(0xB284 + ofst,  1,  1, 0x1);
    SERDES_SET(0xB284 + ofst,  0,  0, 0x1);

    SERDES_SET(0xB284 + ofst,  0,  0, 0x0);
    SERDES_SET(0xB3F8 + ofst, 29, 28, 0x0);
    SERDES_SET(0xB3F8 + ofst, 17, 16, 0x0);
    SERDES_SET(0xB3F8 + ofst, 21, 20, 0x0);
    SERDES_SET(0xB3F8 + ofst, 25, 24, 0x0);
}

void rtl8396_serdes_init_96m (void)
{
    if (1 >= chip10gMP)
    {
        SERDES_SET(0x8,  31 , 0  , 0x666666);
        SERDES_SET(0xc,  31 , 0  , 0x10001);
        SERDES_SET(0xab10,  15 , 0  , 0x8c6a);
        SERDES_SET(0xb710,  15 , 0  , 0x8c6a);
        SERDES_SET(0xb3fc,  16 , 16 , 1);
        SERDES_SET(0xb3fc,  17 , 17 , 1);
        SERDES_SET(0xb3fc,  19 , 18 , 0x3);
        SERDES_SET(0xb3fc,  21 , 20 , 0x3);
        SERDES_SET(0xbbfc,  16 , 16 , 1);
        SERDES_SET(0xbbfc,  17 , 17 , 1);
        SERDES_SET(0xbbfc,  19 , 18 , 0x3);
        SERDES_SET(0xbbfc,  21 , 20 , 0x3);
        SERDES_SET(0xb3fc,  8  , 8  , 1);
        SERDES_SET(0xb3fc,  9  , 9  , 0);
        SERDES_SET(0xb3fc,  10 , 10 , 1);
        SERDES_SET(0xb3fc,  11 , 11 , 0);
        SERDES_SET(0xb3fc,  7  , 4  , 0xf);
        SERDES_SET(0xbbfc,  8  , 8  , 1);
        SERDES_SET(0xbbfc,  9  , 9  , 0);
        SERDES_SET(0xbbfc,  10 , 10 , 1);
        SERDES_SET(0xbbfc,  11 , 11 , 0);
        SERDES_SET(0xbbfc,  7  , 4  , 0xf);
        SERDES_SET(0xb3f8,  22 , 22 , 1);
        SERDES_SET(0xb3f8,  23 , 23 , 1);
        SERDES_SET(0xbbf8,  22 , 22 , 1);
        SERDES_SET(0xbbf8,  23 , 23 , 1);
        SERDES_SET(0xba84,  10 , 9  , 0x1);
        SERDES_SET(0xb284,  10 , 9  , 0x1);
        SERDES_SET(0xb280,  25 , 24 , 0x3);
        SERDES_SET(0xba80,  25 , 24 , 0x3);
        SERDES_SET(0xb37c,  0  , 0  , 1);
        SERDES_SET(0xbb7c,  0  , 0  , 1);
        SERDES_SET(0xaf40,  27 , 24 , 0xf);

        rtl8396_serdes_10g_rx_rst(8);
        rtl8396_serdes_10g_rx_rst(12);

        SERDES_SET(0xb284,  14 , 14 , 1);
        SERDES_SET(0xba84,  14 , 14 , 1);
        SERDES_SET(0x3fc,  31 , 0  , 0xffffff);
        SERDES_SET(0x400,  31 , 0  , 0x0);
        SERDES_SET(0x031c,  7  , 0  , 0x9f);
        SERDES_SET(0x034c,  7  , 0  , 0x9f);
        SERDES_SET(0x71bc,  20 , 20 , 1);
        SERDES_SET(0x71ec,  20 , 20 , 1);
        SERDES_SET(0x71bc,  19 , 4  , 0xfff);
        SERDES_SET(0x71bc,  3  , 0  , 0xf);
        SERDES_SET(0x71ec,  19 , 4  , 0xfff);
        SERDES_SET(0x71ec,  3  , 0  , 0xf);

        SERDES_SET(0xb0b0,  3  , 3  , 1);
        SERDES_SET(0xb8b0,  3  , 3  , 1);
    }
    else
    {
        SERDES_SET(0x8,  31 , 0  , 0x666666);
        SERDES_SET(0xc,  31 , 0  , 0x10001);
        SERDES_SET(0xab10,  15 , 0  , 0x8c6a);
        SERDES_SET(0xb710,  15 , 0  , 0x8c6a);
        SERDES_SET(0xb3fc,  16 , 16 , 1);
        SERDES_SET(0xb3fc,  17 , 17 , 1);
        SERDES_SET(0xb3fc,  19 , 18 , 0x3);
        SERDES_SET(0xb3fc,  21 , 20 , 0x3);
        SERDES_SET(0xbbfc,  16 , 16 , 1);
        SERDES_SET(0xbbfc,  17 , 17 , 1);
        SERDES_SET(0xbbfc,  19 , 18 , 0x3);
        SERDES_SET(0xbbfc,  21 , 20 , 0x3);
        SERDES_SET(0xb3fc,  8  , 8  , 1);
        SERDES_SET(0xb3fc,  9  , 9  , 0);
        SERDES_SET(0xb3fc,  10 , 10 , 1);
        SERDES_SET(0xb3fc,  11 , 11 , 0);
        SERDES_SET(0xb3fc,  7  , 4  , 0xf);
        SERDES_SET(0xbbfc,  8  , 8  , 1);
        SERDES_SET(0xbbfc,  9  , 9  , 0);
        SERDES_SET(0xbbfc,  10 , 10 , 1);
        SERDES_SET(0xbbfc,  11 , 11 , 0);
        SERDES_SET(0xbbfc,  7  , 4  , 0xf);
        SERDES_SET(0xb3f8,  22 , 22 , 1);
        SERDES_SET(0xb3f8,  23 , 23 , 1);
        SERDES_SET(0xbbf8,  22 , 22 , 1);
        SERDES_SET(0xbbf8,  23 , 23 , 1);
        SERDES_SET(0xba84,  10 , 9  , 0x1);
        SERDES_SET(0xb284,  10 , 9  , 0x1);
        SERDES_SET(0xb280,  25 , 24 , 0x3);
        SERDES_SET(0xba80,  25 , 24 , 0x3);
        SERDES_SET(0xb37c,  0  , 0  , 1);
        SERDES_SET(0xbb7c,  0  , 0  , 1);
        SERDES_SET(0xaf40,  27 , 24 , 0xf);

        rtl8396_serdes_10g_rx_rst(8);
        rtl8396_serdes_10g_rx_rst(12);

        SERDES_SET(0xb284,  14 , 14 , 1);
        SERDES_SET(0xba84,  14 , 14 , 1);
        SERDES_SET(0x3fc,  31 , 0  , 0xffffff);
        SERDES_SET(0x400,  31 , 0  , 0x0);
        SERDES_SET(0x031c,  0  , 0  , 0x0);
        SERDES_SET(0x034c,  0  , 0  , 0x0);
        SERDES_SET(0x71bc,  20 , 20 , 1);
        SERDES_SET(0x71ec,  20 , 20 , 1);
        SERDES_SET(0x71bc,  19 , 4  , 0xfff);
        SERDES_SET(0x71bc,  3  , 0  , 0xf);
        SERDES_SET(0x71ec,  19 , 4  , 0xfff);
        SERDES_SET(0x71ec,  3  , 0  , 0xf);

        SERDES_SET(0xb0b0,  3  , 3  , 1);
        SERDES_SET(0xb8b0,  3  , 3  , 1);

        SERDES_SET(0xb3a0, 31  , 16 , 0xFDAB);
        SERDES_SET(0xbba0, 31  , 16 , 0xFDAB);
        SERDES_SET(0xb3cc, 7   , 7  , 0);
        SERDES_SET(0xbbcc, 7   , 7  , 0);
    }
}

void rtl8396_serdes_init_10gr_rx_current(int sds)
{
    if (8 == sds)
    {
      SERDES_SET(0xB324, 31, 31, 1);
      SERDES_SET(0xB328, 12, 12, 1);
    }
    else if (12 == sds)
    {
      SERDES_SET(0xBB24, 31, 31, 1);
      SERDES_SET(0xBB28, 12, 12, 1);
    }
    else
        return;

    rtl8396_serdes_10g_rx_rst(sds);
}

void rtl8396_serdes_init_10gr_leq (int sds)
{
    uint32 pole1 = 0x7;
    uint32 ofst = 0x800;
    uint32 pole3 = 0x1;
    uint32 pole2 = 0x5;
    uint32 addr0 = 0xb340;
    uint32 addr1 = 0xb348;
    uint32 boost1 = 0x0;
    uint32 boost3 = 0x0;
    uint32 boost2 = 0x0;

    if (8 == sds)
    {
        SERDES_SET(addr0,  27 , 25 , boost1);
        SERDES_SET(addr0,  24 , 22 , boost2);
        SERDES_SET(addr0,  21 , 19 , boost3);
        SERDES_SET(addr0,  18 , 16 , pole1);
        SERDES_SET(addr0,  30 , 28 , pole2);
        SERDES_SET(addr1,  25 , 23 , pole3);
    }
    else if (12 == sds)
    {
        SERDES_SET(addr0+ofst,  27 , 25 , boost1);
        SERDES_SET(addr0+ofst,  24 , 22 , boost2);
        SERDES_SET(addr0+ofst,  21 , 19 , boost3);
        SERDES_SET(addr0+ofst,  18 , 16 , pole1);
        SERDES_SET(addr0+ofst,  30 , 28 , pole2);
        SERDES_SET(addr1+ofst,  25 , 23 , pole3);
    }

    rtl8396_serdes_10g_rx_rst(sds);
}

void rtl8396_efusRead_config(int addr)
{
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_EFUSE_CMD_ADDR,
            RTL8390_EFUSE_CMD_EFUSE_ADDR_OFFSET,
            RTL8390_EFUSE_CMD_EFUSE_ADDR_MASK, addr);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_EFUSE_CMD_ADDR,
            RTL8390_EFUSE_CMD_EFUSE_RWOP_OFFSET,
            RTL8390_EFUSE_CMD_EFUSE_RWOP_MASK, 0x0);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_EFUSE_CMD_ADDR,
            RTL8390_EFUSE_CMD_EFUSE_CMD_OFFSET,
            RTL8390_EFUSE_CMD_EFUSE_CMD_MASK, 0x1);
}

int rtl8396_efuse_chk(void)
{
    uint32  val;
    int     i;

    for (i = 0; i < 30; ++i)
    {
        val = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EFUSE_CMD_ADDR);
        if ((val & 0x80000000) == 0x80000000)
        {
            return 1;
        }
    }

    return 0;
}

int rtl8396_serdes_init_efuseConfig_set (int reg, int addr)
{
    uint32  data, val;

    rtl8396_efusRead_config(++addr);
    val = rtl8396_efuse_chk();
    if (!val)
    {
        printf("RW efuse %d fail %x\n", addr, MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EFUSE_CMD_ADDR));
        return 0;
    }

    val = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EFUSE_RDATA_ADDR);
    data = val << 16;

    rtl8396_efusRead_config(++addr);
    val = rtl8396_efuse_chk();
    if (!val)
    {
        printf("RW efuse %d fail %x\n", addr, MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EFUSE_CMD_ADDR));
        return 0;
    }

    val = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EFUSE_RDATA_ADDR);
    data |= val;

    SERDES_SET(reg, 31, 0, data);

    return 1;
}

void rtl8396_serdes_init_efuseConfig (int sds)
{
    uint32  val;
    int     addr, reg, idx;

    for (idx = 0; idx <= 21; ++idx)
    {
        addr = idx * 3;

        /* reg */
        rtl8396_efusRead_config(addr);
        val = rtl8396_efuse_chk();
        if (!val)
        {
            printf("RW efuse %d fail %x\n", addr, MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EFUSE_CMD_ADDR));
            continue;
        }

        reg = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EFUSE_RDATA_ADDR);

        switch (reg)
        {
            case 0xb318:
            case 0xb31c:
                if (1 != chip10gMP)
                    break;
            case 0xb320:
                if (8 == sds)
                    rtl8396_serdes_init_efuseConfig_set(reg, addr);
                break;
            case 0xbb18:
            case 0xbb1c:
                if (1 != chip10gMP)
                    break;
            case 0xbb20:
                if (12 == sds)
                    rtl8396_serdes_init_efuseConfig_set(reg, addr);
                break;
        }
    }

    rtl8396_serdes_10g_rx_rst(sds);
}

void rtl8396_serdes_10g_leq_dc_gain_cali (int sds_num)
{
    uint32  val;
    int     ofst, Bin;
    int     binMap[] = {0, 1, 3, 2, 7, 6, 4, 5, 15, 14, \
                        12, 13, 8, 9, 11, 10, 31, 30, 28, 29,   \
                        24, 25, 27, 26, 16, 17, 19, 18, 23, 22, \
                        20, 21};

    switch (sds_num)
    {
        case 8:
            ofst = 0x0;
            break;
        case 12:
            ofst = 0x800;
            break;
        default:
            return;
    }

    SERDES_SET(0xb340 + ofst,  28  , 27  , 0x0);
    SERDES_SET(0xb318 + ofst,  26  , 26  , 0x0);

    SERDES_SET(0xb340 + ofst,  15  , 15  , 0x1);
    OSAL_MDELAY(100);
    SERDES_SET(0xb340 + ofst,  15  , 15  , 0x0);
    OSAL_MDELAY(500);
    SERDES_SET(0xb318 + ofst,  26  , 26  , 0x1);

    SERDES_SET(0xb320 + ofst,  4  , 4  , 1);
    SERDES_SET(0xb320 + ofst,  11  , 10  , 0x3);
    SERDES_SET(0xb34c + ofst,  31  , 28  , 0x0);
    SERDES_SET(0xb340 + ofst,  4  , 2  , 0x4);

    SERDES_SET(0xb038 + ofst,  9  , 0  , 0x2);
    SERDES_SET(0xb038 + ofst,  10  , 10  , 0x1);
    SERDES_SET(0xb038 + ofst,  11  , 11  , 0x1);

    /* [22:18] */
    val = ((MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst) >> 18) & 0x1F);
    Bin = binMap[val];

    if (Bin <= 5)
    {
        SERDES_SET(0xb340 + ofst,  28  , 27  , 0x3);
    }
    else if ((Bin > 5) && (Bin <= 11))
    {
        SERDES_SET(0xb340 + ofst,  28  , 27  , 0x2);
    }
    else if ((Bin > 11) && (Bin <= 17))
    {
        SERDES_SET(0xb340 + ofst,  28  , 27  , 0x1);
    }
    else
    {
        SERDES_SET(0xb340 + ofst,  28  , 27  , 0x0);
    }
}

void rtl8396_serdes_10g_foreground_offset_range_cali (int sds_num)
{
    uint32  cnt = 0, record;
    int     ofst, ofst_range = 0, fgcal_offset;

    switch (sds_num)
    {
        case 8:
            ofst = 0x0;
            break;
        case 12:
            ofst = 0x800;
            break;
        default:
            return;
    }

    //SERDES_SET(0xb318 + ofst,  26  , 26  , 0x1);
    SERDES_SET(0xb320 + ofst,  15  , 14  , 0x0);
    SERDES_SET(0xb320 + ofst,  4  , 4  , 0x1);
    SERDES_SET(0xb320 + ofst,  11  , 10  , 0x3);
    SERDES_SET(0xb34c + ofst,  31  , 28  , 0x0);

    SERDES_SET(0xb340 + ofst,  4  , 2  , 0x3);
    SERDES_SET(0xb038 + ofst,  9  , 0  , 0x0);
    SERDES_SET(0xb038 + ofst,  10  , 10  , 0x1);
    SERDES_SET(0xb038 + ofst,  11  , 11  , 0x1);

    while (cnt < 4)
    {
        SERDES_SET(0xb320 + ofst,  15  , 14  , ofst_range);

        for (record = 0; record < 10; ++record)
        {
            SERDES_SET(0xb340 + ofst,  15  , 15  , 0x1);
            OSAL_MDELAY(100);
            SERDES_SET(0xb340 + ofst,  15  , 15  , 0x0);

            SERDES_SET(0xb344 + ofst,  21  , 16  , 0xe);
            /* [19:16] */
            fgcal_offset = ((MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst) >> 16) & 0xF);
            ofst_range = ((MEM32_READ(SWCORE_BASE_ADDR + 0xb320 + ofst) >> 14) & 0x3);

            if ((fgcal_offset != 0x0) && (fgcal_offset != 0xf))
            {
                break;
            }
        }

        if (10 == record)
        {
            ofst_range += 1;
        }
        else
            break;

        ++cnt;
    }
}

void rtl8396_serdes_10g_leq_init (int sds_num)
{
    uint32  ofst;

    switch (sds_num)
    {
        case 8:
            ofst = 0x0;
            break;
        case 12:
            ofst = 0x800;
            break;
        default:
            return;
    }

    SERDES_SET(0xb300 + ofst,  24 , 24 , 0);
    SERDES_SET(0xb300 + ofst,  23 , 23 , 1);
    SERDES_SET(0xb300 + ofst,  22 , 18 , 0xf);

    SERDES_SET(0xb320 + ofst,  3  , 3  , 0);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 1);
    OSAL_MDELAY(100);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 0);

    rtl8396_serdes_10g_rx_rst(sds_num);
}

int rtl8396_serdes_10g_eq_dump (int sds_num)
{
    uint32  val;
    int     ofst, Bin;
    int     binMap[] = {0, 1, 3, 2, 7, 6, 4, 5, 15, 14, \
                        12, 13, 8, 9, 11, 10, 31, 30, 28, 29,   \
                        24, 25, 27, 26, 16, 17, 19, 18, 23, 22, \
                        20, 21};

    if (8 == sds_num)
    {
        ofst = 0;
    }
    else
    {
        ofst = 0x800;
    }

    SERDES_SET(0xb320 + ofst,  4  , 4  , 1);
    SERDES_SET(0xb320 + ofst,  11  , 10  , 0x3);
    SERDES_SET(0xb34c + ofst,  31  , 28  , 0x0);

    SERDES_SET(0xb340 + ofst,  4  , 2  , 0x4);

    SERDES_SET(0xb038 + ofst,  9  , 0  , 0x2);
    SERDES_SET(0xb038 + ofst,  10  , 10  , 0x1);
    SERDES_SET(0xb038 + ofst,  11  , 11  , 0x1);

    /* [22:18] */
    val = ((MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst) >> 18) & 0x1F);
    Bin = binMap[val];

    return Bin;
}

void rtl8396_serdes_10g_modify(int sds_num)
{
    uint32 ofst;

    if (8 == sds_num)
    {
        ofst = 0;
    }
    else
    {
        ofst = 0x800;
    }

    SERDES_SET(0xb32c + ofst,  11 , 11 , 0);
    SERDES_SET(0xb32c + ofst,  5  , 5  , 0);
    SERDES_SET(0xb318 + ofst,  15 , 15 , 1);
    SERDES_SET(0xb318 + ofst,  7  , 7  , 0);
    SERDES_SET(0xb354 + ofst,  30 , 30 , 0);
    SERDES_SET(0xb300 + ofst,  23 , 23 , 0);
    SERDES_SET(0xb300 + ofst,  22 , 22 , 0);
    SERDES_SET(0xb300 + ofst,  20 , 20 , 1);
    SERDES_SET(0xb318 + ofst,  28 , 28 , 0);
    SERDES_SET(0xb318 + ofst,  22 , 21 , 0x0);
    SERDES_SET(0xb308 + ofst,  21 , 21 , 0);
    SERDES_SET(0xb31c + ofst,  1  , 1  , 0);
    SERDES_SET(0xb304 + ofst,  27 , 27 , 0);
    SERDES_SET(0xb338 + ofst,  12 , 12 , 0);
    SERDES_SET(0xb338 + ofst,  8  , 8  , 1);
    SERDES_SET(0xb308 + ofst,  15 , 15 , 0);
    SERDES_SET(0xb34c + ofst,  26 , 23 , 0x0);
    SERDES_SET(0xb314 + ofst,  27 , 27 , 0);
    SERDES_SET(0xb314 + ofst,  21 , 21 , 0);
    SERDES_SET(0xb31c + ofst,  29 , 29 , 0);
    SERDES_SET(0xb354 + ofst,  8  , 8  , 0);
    SERDES_SET(0xb33c + ofst,  29 , 29 , 0);
    SERDES_SET(0xb350 + ofst,  30 , 30 , 0);
    SERDES_SET(0xb328 + ofst,  5  , 4  , 0x3);
}

void rtl8396_serdes_10g_fiber_dfe_adapt (int sds_num)
{
    int32   ofst;
    int32   leq;
    int32   val, i;
    int32   cnt = 10;
    int32   tap0_sum = 0;
    int32   tap0_avg;

    if (8 == sds_num)
    {
        ofst = 0;
    }
    else
    {
        ofst = 0x800;
    }

    SERDES_SET(0xb338 + ofst,  14 , 14 , 1);
    SERDES_SET(0xb360 + ofst,  31 , 31 , 1);
    SERDES_SET(0xb318 + ofst,  31 , 29 , 0x7);
    SERDES_SET(0xb31c + ofst,  27 , 25 , 0x7);
    SERDES_SET(0xb310 + ofst,  30 , 28 , 0x7);
    SERDES_SET(0xb310 + ofst,  27 , 25 , 0x7);
    SERDES_SET(0xb304 + ofst,  9  , 5  , 0x3);

    SERDES_SET(0xb32c + ofst,  21 , 16 , 0x1f);
    SERDES_SET(0xb32c + ofst,  5  , 0  , 0x0);
    SERDES_SET(0xb304 + ofst,  27 , 22 , 0x0);
    SERDES_SET(0xb304 + ofst,  21 , 16 , 0x0);
    SERDES_SET(0xb330 + ofst,  11 , 6  , 0x0);
    SERDES_SET(0xb318 + ofst,  10 , 10 , 1);
    SERDES_SET(0xb318 + ofst,  9  , 9  , 1);
    SERDES_SET(0xb318 + ofst,  8  , 8  , 1);
    SERDES_SET(0xb318 + ofst,  2  , 2  , 0);
    SERDES_SET(0xb318 + ofst,  1  , 1  , 0);
    SERDES_SET(0xb318 + ofst,  0  , 0  , 0);
    SERDES_SET(0xb350 + ofst,  26 , 26 , 0);
    SERDES_SET(0xb350 + ofst,  25 , 25 , 0);

    SERDES_SET(0xb340 + ofst,  28 , 27 , 0x2);

    SERDES_SET(0xb300 + ofst,  24 , 24 , 1);
    SERDES_SET(0xb300 + ofst,  23 , 23 , 0);
    SERDES_SET(0xb318 + ofst,  26 , 26 , 0);
    SERDES_SET(0xb320 + ofst,  3  , 3  , 0);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 0x1);
    OSAL_MDELAY(10);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 0x0);
    OSAL_MDELAY(500);
    leq = rtl8396_serdes_10g_eq_dump(sds_num);
    SERDES_SET(0xb300 + ofst,  24 , 24 , 0);
    SERDES_SET(0xb300 + ofst,  23 , 23 , 1);
    SERDES_SET(0xb300 + ofst,  22 , 18 , leq);

    SERDES_SET(0xb320 + ofst,  4  , 4  , 1);
    SERDES_SET(0xb320 + ofst,  11 , 10 , 0x3);
    SERDES_SET(0xb340 + ofst,  4  , 2  , 0x3);
    SERDES_SET(0xb34c + ofst,  31 , 28 , 0x0);
    SERDES_SET(0xb038 + ofst,  11 , 11 , 1);
    SERDES_SET(0xb038 + ofst,  10 , 10 , 1);
    SERDES_SET(0xb038 + ofst,  9  , 0  , 0x0);

    SERDES_SET(0xb338 + ofst,  8  , 8  , 1);
    SERDES_SET(0xb314 + ofst,  29 , 29 , 0);
    SERDES_SET(0xb330 + ofst,  15 , 15 , 0);
    SERDES_SET(0xb318 + ofst,  8  , 8  , 0);
    SERDES_SET(0xb318 + ofst,  0  , 0  , 1);
    SERDES_SET(0xb344 + ofst,  21 , 16 , 0x0);

    for (i = 1; i <= cnt; ++i)
    {
        val = MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst);
        tap0_sum += (val >> 16) & 0x1f;

        OSAL_MDELAY(1);
    }

    tap0_avg = tap0_sum/cnt;
    SERDES_SET(0xb32c + ofst,  21 , 16 , tap0_avg);
    SERDES_SET(0xb318 + ofst,  8  , 8  , 1);
    SERDES_SET(0xb318 + ofst,  0  , 0  , 0);

    SERDES_SET(0xb320 + ofst,  3  , 3  , 0);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 1);
    OSAL_MDELAY(10);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 0);

    SERDES_SET(0xB3F8 + ofst, 29, 28, 0x3);
    SERDES_SET(0xB3F8 + ofst, 17, 16, 0x3);
    SERDES_SET(0xB3F8 + ofst, 21, 20, 0x1);
    SERDES_SET(0xB3F8 + ofst, 25, 24, 0x3);

    SERDES_SET(0xB284 + ofst,  6,  6, 0x1);
    SERDES_SET(0xB284 + ofst,  5,  5, 0x0);
    SERDES_SET(0xB284 + ofst,  4,  4, 0x1);
    SERDES_SET(0xB284 + ofst,  3,  3, 0x1);
    SERDES_SET(0xB284 + ofst,  2,  2, 0x0);
    SERDES_SET(0xB284 + ofst,  1,  1, 0x1);
    SERDES_SET(0xB284 + ofst,  0,  0, 0x1);

    SERDES_SET(0xB284 + ofst,  0,  0, 0x0);
    SERDES_SET(0xB3F8 + ofst, 29, 28, 0x0);
    SERDES_SET(0xB3F8 + ofst, 17, 16, 0x0);
    SERDES_SET(0xB3F8 + ofst, 21, 20, 0x0);
    SERDES_SET(0xB3F8 + ofst, 25, 24, 0x0);
}

void rtl8396_serdes_10g_dac_dfe_adapt (int sds_num)
{
    int32   ofst;
    int32   leq;
    int32   val, i;
    int32   cnt = 10, half;
    int32   vthp_sum = 0;
    int32   vthn_sum = 0;
    int32   tap0_sum = 0;
    int32   tap0_final_avg;
    int32   tap1_even = 0, tap1_odd = 0;
    int32   tap1_even_sum = 0, tap1_odd_sum = 0;
    int32   tap1_even_final_avg, tap1_odd_final_avg;
    int32   tap2_even = 0, tap2_odd = 0;
    int32   tap2_even_sum = 0, tap2_odd_sum = 0;
    int32   tap2_even_final_avg, tap2_odd_final_avg;
    int32   vthp_final_avg, vthn_final_avg;
    int32   tap0_avg;
    int32   vthp_avg, vthn_avg;
    int32   tap1_even_avg, tap1_odd_avg;
    int32   tap2_even_avg, tap2_odd_avg;

    half = cnt / 2;

    if (8 == sds_num)
    {
        ofst = 0;
    }
    else
    {
        ofst = 0x800;
    }

    SERDES_SET(0xb338 + ofst,  14 , 14 , 0);
    SERDES_SET(0xb360 + ofst,  31 , 31 , 0);
    SERDES_SET(0xb304 + ofst,  9  , 5  , 0x6);
    SERDES_SET(0xb308 + ofst,  14 , 12 , 0x0);
    SERDES_SET(0xb32c + ofst,  14 , 12 , 0x0);
    SERDES_SET(0xb30c + ofst,  9  , 5  , 0x2);
    SERDES_SET(0xb30c + ofst,  4  , 0  , 0x2);
    SERDES_SET(0xb33c + ofst,  29 , 28 , 0x1);
    SERDES_SET(0xb354 + ofst,  8  , 8  , 0x1);
    SERDES_SET(0xb354 + ofst,  21 , 16 , 0x3);
    SERDES_SET(0xb340 + ofst,  28 , 27 , 0x3);

    SERDES_SET(0xb32c + ofst,  21 , 16 , 0x1f);
    SERDES_SET(0xb32c + ofst,  5  , 0  , 0x0);
    SERDES_SET(0xb31c + ofst,  21 , 19 , 0x7);
    SERDES_SET(0xb31c + ofst,  18 , 16 , 0x7);
    SERDES_SET(0xb304 + ofst,  27 , 22 , 0x0);
    SERDES_SET(0xb304 + ofst,  21 , 16 , 0x0);
    SERDES_SET(0xb330 + ofst,  11 , 6  , 0x0);
    SERDES_SET(0xb330 + ofst,  5  , 0  , 0x0);
    SERDES_SET(0xb308 + ofst,  27 , 22 , 0x0);
    SERDES_SET(0xb308 + ofst,  11 , 6  , 0x0);
    SERDES_SET(0xb308 + ofst,  5  , 0  , 0x0);
    SERDES_SET(0xb318 + ofst,  15 , 8  , 0xff);
    SERDES_SET(0xb318 + ofst,  7  , 0  , 0x0);
    SERDES_SET(0xb350 + ofst,  30 , 24 , 0x0);

    SERDES_SET(0xb338 + ofst,  8  , 8  , 1);
    SERDES_SET(0xb314 + ofst,  29 , 29 , 0);
    SERDES_SET(0xb330 + ofst,  15 , 15 , 0);
    SERDES_SET(0xb354 + ofst,  27 , 27 , 0);
    SERDES_SET(0xb360 + ofst,  25 , 21 , 0x0);
    SERDES_SET(0xb360 + ofst,  20 , 16 , 0x0);

    SERDES_SET(0xb300 + ofst,  24 , 24 , 1);
    SERDES_SET(0xb300 + ofst,  23 , 23 , 0);
    SERDES_SET(0xb318 + ofst,  26 , 26 , 0);
    SERDES_SET(0xb320 + ofst,  3  , 3  , 0);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 1);
    OSAL_MDELAY(100);
    SERDES_SET(0xb340 + ofst,  15 , 15 , 0);
    OSAL_MDELAY(500);
    leq = rtl8396_serdes_10g_eq_dump(sds_num);
    //printf( "LEQ cal val = %d\n", leq);
    SERDES_SET(0xb300 + ofst,  24 , 24 , 0);
    SERDES_SET(0xb300 + ofst,  23 , 23 , 1);
    SERDES_SET(0xb300 + ofst,  22 , 18 , leq);

    SERDES_SET(0xb320 + ofst,  4  , 4  , 1);
    SERDES_SET(0xb320 + ofst,  11 , 10 , 0x3);
    SERDES_SET(0xb340 + ofst,  4  , 2  , 0x3);
    SERDES_SET(0xb34c + ofst,  31 , 28 , 0x0);
    SERDES_SET(0xb038 + ofst,  11 , 11 , 1);
    SERDES_SET(0xb038 + ofst,  10 , 10 , 1);
    SERDES_SET(0xb038 + ofst,  9  , 0  , 0x0);


    SERDES_SET(0xb318 + ofst,  0  , 0  , 1);
    SERDES_SET(0xb318 + ofst,  7  , 7  , 1);
    SERDES_SET(0xb350 + ofst,  24 , 24 , 1);
    SERDES_SET(0xb318 + ofst,  8  , 8  , 0);
    SERDES_SET(0xb318 + ofst,  15 , 15 , 0);

    for (i = 1; i <= cnt; ++i)
    {
        SERDES_SET(0xb344 + ofst,  21 , 16 , 0x0);
        val = MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst);
        tap0_sum += (val >> 16) & 0x1f;

        SERDES_SET(0xb344+ofst,  21 , 16 , 0xc);
        val = MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst);
        vthp_sum += ((val >> 16) & 0x7);
        vthn_sum += ((val >> 19) & 0x7);

        OSAL_MDELAY(1);
    }
    tap0_avg = tap0_sum/cnt;
    if ((tap0_sum - (tap0_avg * cnt)) >= half)
        tap0_final_avg = tap0_avg + 1;
    else
        tap0_final_avg = tap0_avg;
    //printf( "tap0 avg val = %d\n", tap0_final_avg);
    SERDES_SET(0xb32c + ofst,  21 , 16 , tap0_final_avg);
    SERDES_SET(0xb318 + ofst,  8  , 8  , 1);
    SERDES_SET(0xb318 + ofst,  0  , 0  , 0);
    SERDES_SET(0xb350 + ofst,  24 , 24 , 0);

    vthp_avg = vthp_sum/cnt;
    if ((vthp_sum - (vthp_avg * cnt)) >= half)
        vthp_final_avg = vthp_avg + 1;
    else
        vthp_final_avg = vthp_avg;

    vthn_avg = vthn_sum/cnt;
    if ((vthn_sum - (vthn_avg * cnt)) >= half)
        vthn_final_avg = vthn_avg + 1;
    else
        vthn_final_avg = vthn_avg;

    //printf( "vthp avg val = %d\n", vthp_final_avg);
    //printf( "vthn avg val = %d\n", vthn_final_avg);
    SERDES_SET(0xb31c + ofst,  21 , 19 , vthp_final_avg);
    SERDES_SET(0xb31c + ofst,  18 , 16 , vthn_final_avg);
    SERDES_SET(0xb318 + ofst,  15 , 15 , 1);
    SERDES_SET(0xb318 + ofst,  7  , 7  , 0);

    //if (leq > 5)
    {
        SERDES_SET(0xb338 + ofst,  8  , 8  , 0);
        SERDES_SET(0xb318 + ofst,  4  , 1  , 0xf);
        SERDES_SET(0xb350 + ofst,  28 , 25 , 0xf);
        SERDES_SET(0xb318 + ofst,  12 , 9  , 0x0);

        SERDES_SET(0xb320 + ofst,  3  , 3  , 0);
        SERDES_SET(0xb340 + ofst,  15 , 15 , 1);
        OSAL_MDELAY(100);
        SERDES_SET(0xb340 + ofst,  15 , 15 , 0);

        for (i = 1; i <= cnt; ++i)
        {
            SERDES_SET(0xb344+ofst,  21 , 16 , 0x1);
            val = MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst);
            tap1_even = (val >> 16) & 0x1f;
            if (((val >> 21) & 0x1) == 1)
                tap1_even_sum -= tap1_even;
            else
                tap1_even_sum += tap1_even;

            SERDES_SET(0xb344+ofst,  21 , 16 , 0x6);
            val = MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst);
            tap1_odd = (val >> 16) & 0x1f;
            if (((val >> 21) & 0x1) == 1)
                tap1_odd_sum -= tap1_odd;
            else
                tap1_odd_sum += tap1_odd;

            SERDES_SET(0xb344+ofst,  21 , 16 , 0x2);
            val = MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst);
            tap2_even = (val >> 16) & 0x1f;
            if (((val >> 21) & 0x1) == 1)
                tap2_even_sum -= tap2_even;
            else
                tap2_even_sum += tap2_even;

            SERDES_SET(0xb344+ofst,  21 , 16 , 0x7);
            val = MEM32_READ(SWCORE_BASE_ADDR + 0xb038 + ofst);
            tap2_odd = (val >> 16) & 0x1f;
            if (((val >> 21) & 0x1) == 1)
                tap2_odd_sum -= tap2_odd;
            else
                tap2_odd_sum += tap2_odd;

            OSAL_MDELAY(1);
        }
        tap1_even_avg = tap1_even_sum/cnt;
        if ((tap1_even_sum - (tap1_even_avg * cnt)) >= half)
            tap1_even_final_avg = tap1_even_avg + 1;
        else
            tap1_even_final_avg = tap1_even_avg;

        tap1_odd_avg = tap1_odd_sum/cnt;
        if ((tap1_odd_sum - (tap1_odd_avg * cnt)) >= half)
            tap1_odd_final_avg = tap1_odd_avg + 1;
        else
            tap1_odd_final_avg = tap1_odd_avg;

        //printf( "tap1_even avg val = %d\n", tap1_even_final_avg);
        //printf( "tap1_odd  avg val = %d\n", tap1_odd_final_avg);
        if (tap1_even_final_avg < 0)
        {
            SERDES_SET(0xb304+ofst,  27 , 27 , 1);
            val = tap1_even_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb304+ofst,  27 , 27 , 0);
            val = tap1_even_final_avg;
        }
        SERDES_SET(0xb304+ofst,  26 , 22 , val);

        if (tap1_odd_final_avg < 0)
        {
            SERDES_SET(0xb32c+ofst,  5  , 5  , 1);
            val = tap1_odd_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb32c+ofst,  5  , 5  , 0);
            val = tap1_odd_final_avg;
        }
        SERDES_SET(0xb32c+ofst,  4  , 0  , val);

        if (tap1_even_final_avg < 0)
        {
            SERDES_SET(0xb308+ofst,  11 , 11 , 1);
            val = tap1_even_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb308+ofst,  11 , 11 , 0);
            val = tap1_even_final_avg;
        }
        SERDES_SET(0xb308+ofst,  10 , 6  , val);

        if (tap1_odd_final_avg < 0)
        {
            SERDES_SET(0xb330+ofst,  5  , 5  , 1);
            val = tap1_odd_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb330+ofst,  5  , 5  , 0);
            val = tap1_odd_final_avg;
        }
        SERDES_SET(0xb330+ofst,  4  , 0  , val);

        tap2_even_avg = tap2_even_sum/cnt;
        if ((tap2_even_sum - (tap2_even_avg * cnt)) >= half)
            tap2_even_final_avg = tap2_even_avg + 1;
        else
            tap2_even_final_avg = tap2_even_avg;

        tap2_odd_avg = tap2_odd_sum/cnt;
        if ((tap2_odd_sum - (tap2_odd_avg * cnt)) >= half)
            tap2_odd_final_avg = tap2_odd_avg + 1;
        else
            tap2_odd_final_avg = tap2_odd_avg;

        //printf( "tap2_even avg val = %d\n", tap2_even_final_avg);
        //printf( "tap2_odd  avg val = %d\n", tap2_odd_final_avg);
        if (tap2_even_final_avg < 0)
        {
            SERDES_SET(0xb304+ofst,  21 , 21 , 0);
            val = tap2_even_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb304+ofst,  21 , 21 , 0);
            val = tap2_even_final_avg;
        }
        SERDES_SET(0xb304+ofst,  20 , 16 , val);

        if (tap2_odd_final_avg < 0)
        {
            SERDES_SET(0xb330+ofst,  11 , 11 , 0);
            val = tap2_odd_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb330+ofst,  11 , 11 , 0);
            val = tap2_odd_final_avg;
        }
        SERDES_SET(0xb330+ofst,  10 , 6  , val);

        if (tap2_even_final_avg < 0)
        {
            SERDES_SET(0xb308+ofst,  5  , 5  , 0);
            val = tap2_even_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb308+ofst,  5  , 5  , 0);
            val = tap2_even_final_avg;
        }
        SERDES_SET(0xb308+ofst,  4  , 0  , val);

        if (tap2_odd_final_avg < 0)
        {
            SERDES_SET(0xb308+ofst,  27 , 27 , 0);
            val = tap2_odd_final_avg * -1;
        }
        else
        {
            SERDES_SET(0xb308+ofst,  27 , 27 , 0);
            val = tap2_odd_final_avg;
        }
        SERDES_SET(0xb308+ofst,  26 , 22 , val);

        SERDES_SET(0xb318 + ofst,  12 , 9  , 0xf);
        SERDES_SET(0xb318 + ofst,  4  , 1  , 0x0);
        SERDES_SET(0xb350 + ofst,  28 , 25 , 0x0);
        SERDES_SET(0xb360 + ofst,  19 , 16 , 0xf);
        SERDES_SET(0xb360 + ofst,  24 , 21 , 0xf);
        SERDES_SET(0xb354+ofst,  27 , 27 , 1);
    }

    SERDES_SET(0xB3F8 + ofst, 29, 28, 0x3);
    SERDES_SET(0xB3F8 + ofst, 17, 16, 0x3);
    SERDES_SET(0xB3F8 + ofst, 21, 20, 0x1);
    SERDES_SET(0xB3F8 + ofst, 25, 24, 0x3);

    SERDES_SET(0xB284 + ofst,  6,  6, 0x1);
    SERDES_SET(0xB284 + ofst,  5,  5, 0x0);
    SERDES_SET(0xB284 + ofst,  4,  4, 0x1);
    SERDES_SET(0xB284 + ofst,  3,  3, 0x1);
    SERDES_SET(0xB284 + ofst,  2,  2, 0x0);
    SERDES_SET(0xB284 + ofst,  1,  1, 0x1);
    SERDES_SET(0xB284 + ofst,  0,  0, 0x1);

    SERDES_SET(0xB284 + ofst,  0,  0, 0x0);
    SERDES_SET(0xB3F8 + ofst, 29, 28, 0x0);
    SERDES_SET(0xB3F8 + ofst, 17, 16, 0x0);
    SERDES_SET(0xB3F8 + ofst, 21, 20, 0x0);
    SERDES_SET(0xB3F8 + ofst, 25, 24, 0x0);
}

uint32 sdsDelayTime = 300;

void rtl8396_10gSds_restart(int port)
{
    int sds;
    int media;

    if (1 >= chip10gMP)
        return;

    switch (port)
    {
        case 24:
            sds = 8;
            break;
        case 36:
            sds = 12;
            break;
        default:
            return;
    }

    OSAL_MDELAY(sdsDelayTime);

    if (2 == chip10gMP)
    {
        rtl8396_serdes_10g_rx_rst(sds);
    }
    else
    {
        rtl8390_10gMedia_get(port, &media);
        switch (media)
        {
            case 0 ... 3:
                rtl8396_serdes_10g_fiber_dfe_adapt(sds);
                break;
        }
    }
}

void rtl8396_serdes_10g_trx_85ohm(int sds)
{
    uint32  ofst;

    if (8 == sds)
    {
        ofst = 0x0;
    }
    else
    {
        ofst = 0x800;
    }

    SERDES_SET(0xb324 + ofst,  25 , 25 , 0);
    SERDES_SET(0xb324 + ofst,  24 , 21 , 0xf);
    SERDES_SET(0xb324 + ofst,  20 , 20 , 0);
    SERDES_SET(0xb324 + ofst,  19 , 16 , 0xf);

    return;
}

void rtl8396_serdes_init_10gr_fiber (int sds)
{
    uint32  ofst, port;

    if (8 == sds)
    {
        ofst = 0x0;
        port = 24;
    }
    else
    {
        ofst = 0x800;
        port = 36;
    }

    SERDES_SET(0xb014 + ofst,  14  , 14  , 0x1);
    SERDES_SET(0xb004 + ofst,  30  , 30  , 0x1);
    SERDES_SET(0xb004 + ofst,  23  , 23  , 0x1);

    if (1 >= chip10gMP)
    {
        if (8 == sds)
        {
            SERDES_SET(0x000c,  3  , 0  , 0x1);
            SERDES_SET(0xb0b0,  3  , 3  , 1);
            SERDES_SET(0x031c,  7  , 0  , 0x9f);
            SERDES_SET(0xB300,  31 , 0  , 0xC000F800);
            SERDES_SET(0xB304,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xB308,  31 , 0  , 0x80001000);
            SERDES_SET(0xB30C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xB310,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xB314,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xB318,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xB31C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xB320,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xB324,  31 , 0  , 0x730096B2);
            SERDES_SET(0xB328,  31 , 0  , 0x00000F53);
            SERDES_SET(0xB32C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xB330,  31 , 0  , 0x00007000);
            SERDES_SET(0xB334,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xB338,  31 , 0  , 0x02000000);
            SERDES_SET(0xB33C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xB340,  31 , 0  , 0xb49244f1);
            SERDES_SET(0xB344,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xB348,  31 , 0  , 0x614FFE1A);
            SERDES_SET(0xB34C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xB350,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xB354,  31 , 0  , 0x2002619C);
            SERDES_SET(0xB358,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xB35C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xB360,  31 , 0  , 0x00000000);
            SERDES_SET(0xB364,  31 , 0  , 0x00000000);
            SERDES_SET(0xB368,  31 , 0  , 0x00000000);
            SERDES_SET(0xB36C,  31 , 0  , 0x00000000);
            SERDES_SET(0xB370,  31 , 0  , 0x00000000);
            SERDES_SET(0xB374,  31 , 0  , 0x00000000);
            SERDES_SET(0xB378,  31 , 0  , 0x00000000);
            SERDES_SET(0xB37C,  31 , 0  , 0x00000000);
        }
        else if (12 == sds)
        {
            SERDES_SET(0x000c,  19 , 16 , 0x1);
            SERDES_SET(0xb8b0,  3  , 3  , 1);
            SERDES_SET(0x034c,  7  , 0  , 0x9f);
            SERDES_SET(0xBB00,  31 , 0  , 0xC000F800);
            SERDES_SET(0xBB04,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xBB08,  31 , 0  , 0x80001000);
            SERDES_SET(0xBB0C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xBB10,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xBB14,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xBB18,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xBB1C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xBB20,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xBB24,  31 , 0  , 0x730096B2);
            SERDES_SET(0xBB28,  31 , 0  , 0x00000F53);
            SERDES_SET(0xBB2C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xBB30,  31 , 0  , 0x00007000);
            SERDES_SET(0xBB34,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xBB38,  31 , 0  , 0x02000000);
            SERDES_SET(0xBB3C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xBB40,  31 , 0  , 0xb49244f1);
            SERDES_SET(0xBB44,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xBB48,  31 , 0  , 0x614FFE1A);
            SERDES_SET(0xBB4C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xBB50,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xBB54,  31 , 0  , 0x2002619C);
            SERDES_SET(0xBB58,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xBB5C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xBB60,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB64,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB68,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB6C,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB70,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB74,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB78,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB7C,  31 , 0  , 0x00000000);
        }
        else
            return;

        rtl8396_serdes_10g_rx_rst(sds);

        rtl8396_serdes_init_10gr_rx_current(sds);

        rtl8396_serdes_init_10gr_leq(sds);

        rtl8396_serdes_init_efuseConfig(sds);
    }
    else if (2 == chip10gMP)
    {
        if (8 == sds)
        {
            SERDES_SET(0x000c,  3 , 0  , 0x0);
            SERDES_SET(0xb0b0,  3  , 3  , 1);
            SERDES_SET(0xb3c0,  24 , 23 , 0x3);
            SERDES_SET(0xB300,  31 , 0  , 0xFC14F800);
            SERDES_SET(0xB304,  31 , 0  , 0xF80057EF);
            SERDES_SET(0xB308,  31 , 0  , 0x80209000);
            SERDES_SET(0xB30C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xB310,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xB314,  31 , 0  , 0x482003E4);
            SERDES_SET(0xB318,  31 , 0  , 0x347F7E80);
            SERDES_SET(0xB31C,  31 , 0  , 0x242D8A00);
            SERDES_SET(0xB320,  31 , 0  , 0x984A3F16);
            SERDES_SET(0xB324,  31 , 0  , 0xF108B632);
            SERDES_SET(0xB328,  31 , 0  , 0x00001F53);
            SERDES_SET(0xB32C,  31 , 0  , 0xFFCF0820);
            SERDES_SET(0xB330,  31 , 0  , 0x00007000);
            SERDES_SET(0xB334,  31 , 0  , 0x3800FFFF);
            SERDES_SET(0xB338,  31 , 0  , 0x02000100);
            SERDES_SET(0xB33C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xB340,  31 , 0  , 0x97E044F1);
            SERDES_SET(0xB344,  31 , 0  , 0xFFCC1084);
            SERDES_SET(0xB348,  31 , 0  , 0x614FFC84);
            SERDES_SET(0xB34C,  31 , 0  , 0x07C704F7);
            SERDES_SET(0xB350,  31 , 0  , 0x407EC3E0);
            SERDES_SET(0xB354,  31 , 0  , 0x6004619C);
            SERDES_SET(0xB358,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xB35C,  31 , 0  , 0x0E4B4E4B);
            SERDES_SET(0xB360,  31 , 0  , 0x00000000);
            SERDES_SET(0xB364,  31 , 0  , 0x00000000);
            SERDES_SET(0xB368,  31 , 0  , 0x00000000);
            SERDES_SET(0xB36C,  31 , 0  , 0x00000000);
            SERDES_SET(0xB370,  31 , 0  , 0x00000000);
            SERDES_SET(0xB374,  31 , 0  , 0x00000000);
            SERDES_SET(0xB378,  31 , 0  , 0x00000000);
            SERDES_SET(0xB37C,  31 , 0  , 0x00000000);

            SERDES_SET(0x000c,  3 , 0  , 0x1);
            SERDES_SET(0xb3f8,  17 , 16 , 0x1);
            OSAL_MDELAY(500);
            SERDES_SET(0xb3f8,  17 , 16 , 0x3);
            SERDES_SET(0xb3f8,  17 , 16 , 0x0);
            rtl8396_10gSds_restart(24);
        }
        else
        {
            SERDES_SET(0x000c,  19 , 16  , 0x0);
            SERDES_SET(0xb8b0,  3  , 3  , 1);
            SERDES_SET(0xbbc0,  24 , 23 , 0x3);
            SERDES_SET(0xBB00,  31 , 0  , 0xFC14F800);
            SERDES_SET(0xBB04,  31 , 0  , 0xF80057EF);
            SERDES_SET(0xBB08,  31 , 0  , 0x80209000);
            SERDES_SET(0xBB0C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xBB10,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xBB14,  31 , 0  , 0x482003E4);
            SERDES_SET(0xBB18,  31 , 0  , 0x347F7E80);
            SERDES_SET(0xBB1C,  31 , 0  , 0x242D8A00);
            SERDES_SET(0xBB20,  31 , 0  , 0x984A3F16);
            SERDES_SET(0xBB24,  31 , 0  , 0xF108B632);
            SERDES_SET(0xBB28,  31 , 0  , 0x00001F53);
            SERDES_SET(0xBB2C,  31 , 0  , 0xFFCF0820);
            SERDES_SET(0xBB30,  31 , 0  , 0x00007000);
            SERDES_SET(0xBB34,  31 , 0  , 0x3800FFFF);
            SERDES_SET(0xBB38,  31 , 0  , 0x02000100);
            SERDES_SET(0xBB3C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xBB40,  31 , 0  , 0x97E044F1);
            SERDES_SET(0xBB44,  31 , 0  , 0xFFCC1084);
            SERDES_SET(0xBB48,  31 , 0  , 0x614FFC84);
            SERDES_SET(0xBB4C,  31 , 0  , 0x07C704F7);
            SERDES_SET(0xBB50,  31 , 0  , 0x407EC3E0);
            SERDES_SET(0xBB54,  31 , 0  , 0x6004619C);
            SERDES_SET(0xBB58,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xBB5C,  31 , 0  , 0x0E4B4E4B);
            SERDES_SET(0xBB60,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB64,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB68,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB6C,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB70,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB74,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB78,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB7C,  31 , 0  , 0x00000000);

            SERDES_SET(0x000c,  19 , 16 , 0x1);
            SERDES_SET(0xbbf8,  17 , 16 , 0x1);
            OSAL_MDELAY(100);
            SERDES_SET(0xbbf8,  17 , 16 , 0x3);
            SERDES_SET(0xbbf8,  17 , 16 , 0x0);
            rtl8396_10gSds_restart(36);
        }
    }
    else
    {
        if (8 == sds)
            SERDES_SET(0x000c,  3 , 0  , 0x0);
        else
            SERDES_SET(0x000c,  19 , 16 , 0x0);

        SERDES_SET(0xb0b0 + ofst,  3  , 3  , 0);
        SERDES_SET(0xb3c0 + ofst,  24 , 23 , 0x3);
        SERDES_SET(0xb39c + ofst,  29 , 29 , 0);
        SERDES_SET(0xb388 + ofst,  15 , 15 , 0);
        SERDES_SET(0xb3a4 + ofst,  23 , 16 , 0);
        SERDES_SET(0xB300 + ofst,  31 , 0  , 0xFE14F800);
        SERDES_SET(0xB304 + ofst,  31 , 0  , 0xF80053EF);
        SERDES_SET(0xB308 + ofst,  31 , 0  , 0x80201000);
        SERDES_SET(0xB30C + ofst,  31 , 0  , 0xFFFFBFFF);
        SERDES_SET(0xB310 + ofst,  31 , 0  , 0x806FFFFF);
        SERDES_SET(0xB314 + ofst,  31 , 0  , 0x482003E4);
        SERDES_SET(0xB318 + ofst,  31 , 0  , 0x241F7F80);
        SERDES_SET(0xB31C + ofst,  31 , 0  , 0x042D8980);
        SERDES_SET(0xB320 + ofst,  31 , 0  , 0x984A3F16);
        //rtl8396_serdes_init_efuseConfig(sds);
        SERDES_SET(0xB324 + ofst,  31 , 0  , 0xF108B632);
        SERDES_SET(0xB328 + ofst,  31 , 0  , 0x00001F53);
        SERDES_SET(0xB32C + ofst,  31 , 0  , 0xFFDF0820);
        SERDES_SET(0xB330 + ofst,  31 , 0  , 0x00007000);
        SERDES_SET(0xB334 + ofst,  31 , 0  , 0x3800FFFF);
        SERDES_SET(0xB338 + ofst,  31 , 0  , 0x06000100);
        SERDES_SET(0xB33C + ofst,  31 , 0  , 0x9E100003);
        SERDES_SET(0xB340 + ofst,  31 , 0  , 0x97E004F1);
        SERDES_SET(0xB344 + ofst,  31 , 0  , 0xFFCC1084);
        SERDES_SET(0xB348 + ofst,  31 , 0  , 0x614FFC84);
        SERDES_SET(0xB34C + ofst,  31 , 0  , 0x07C704F7);
        if (8 == sds)
        {
            SERDES_SET(0xB350,  31 , 0  , 0x407EB162);
        }
        else
        {
            SERDES_SET(0xBB50,  31 , 0  , 0x407EACE1);
        }
        SERDES_SET(0xB354 + ofst,  31 , 0  , 0x6004609C);
        SERDES_SET(0xB358 + ofst,  31 , 0  , 0xEFFAEAE2);
        SERDES_SET(0xB35C + ofst,  31 , 0  , 0x0E4B4E4B);
        SERDES_SET(0xB360 + ofst,  31 , 0  , 0x00000000);
        SERDES_SET(0xB364 + ofst,  31 , 0  , 0x00000000);
        SERDES_SET(0xB368 + ofst,  31 , 0  , 0x00000000);
        SERDES_SET(0xB36C + ofst,  31 , 0  , 0x00000000);
        SERDES_SET(0xB370 + ofst,  31 , 0  , 0x00000000);
        SERDES_SET(0xB374 + ofst,  31 , 0  , 0x00000000);
        SERDES_SET(0xB378 + ofst,  31 , 0  , 0x00000000);
        SERDES_SET(0xB37C + ofst,  31 , 0  , 0x00000000);

        SERDES_SET(0xB358 + ofst,  24 , 24 , 0);
        SERDES_SET(0xB350 + ofst,  14 , 10 , 0xC);
        SERDES_SET(0xB350 + ofst,  9  , 5  , 0xB);

        #ifdef PHY_8390_10G_85_OHM
            rtl8396_serdes_10g_trx_85ohm(sds);
        #endif
        SERDES_SET(0xb3f8 + ofst,  17 , 16 , 0x1);
        OSAL_MDELAY(100);
        SERDES_SET(0xb3f8 + ofst,  17 , 16 , 0x3);
        SERDES_SET(0xb3f8 + ofst,  17 , 16 , 0x0);

        if (8 == sds)
            SERDES_SET(0x000c,  3 , 0  , 0x1);
        else
            SERDES_SET(0x000c,  19 , 16 , 0x1);

        rtl8396_serdes_10g_foreground_offset_range_cali(sds);
        rtl8396_serdes_10g_rx_rst(sds);
        rtl8396_serdes_10g_leq_init(sds);
    }
}

void rtl8396_serdes_10g_lcpll (int sds)
{
    if (8 == sds)
    {
        SERDES_SET(0xb344,  9  , 0  , 0x200);
        SERDES_SET(0xb348,  15 , 15 , 1);
        SERDES_SET(0xb348,  14 , 10 , 0xe);
        SERDES_SET(0xb348,  9  , 0  , 0x200);
        SERDES_SET(0xb34c,  11 , 10 , 0x0);
        SERDES_SET(0xb34c,  9  , 8  , 0x0);
        SERDES_SET(0xb344,  9  , 0  , 0x200);
    }
    else if (12 == sds)
    {
        SERDES_SET(0xbb44,  9  , 0  , 0x200);
        SERDES_SET(0xbb48,  15 , 15 , 1);
        SERDES_SET(0xbb48,  14 , 10 , 0xe);
        SERDES_SET(0xbb48,  9  , 0  , 0x200);
        SERDES_SET(0xbb4c,  11 , 10 , 0x0);
        SERDES_SET(0xbb4c,  9  , 8  , 0x0);
        SERDES_SET(0xbb44,  9  , 0  , 0x200);
    }

    rtl839x_serdes_rst(sds);
}

void rtl8396_serdes_ES_10g_qsgmii_patch (int sds)
{
    if (8 == sds)
    {
        SERDES_SET(0xb018,  3  , 3  , 0);
        SERDES_SET(0xb3fc,  31 , 16 , 0x0);
        SERDES_SET(0xb300,  15 , 0  , 0xF800);
        SERDES_SET(0xb300,  31 , 16 , 0xC000);
        SERDES_SET(0xb304,  15 , 0  , 0x57EF);
        SERDES_SET(0xb304,  31 , 16 , 0xF000);
        SERDES_SET(0xb308,  15 , 0  , 0x1000);
        SERDES_SET(0xb308,  31 , 16 , 0x8000);
        SERDES_SET(0xb30c,  15 , 0  , 0xBFFF);
        SERDES_SET(0xb30c,  31 , 16 , 0xFFFF);
        SERDES_SET(0xb310,  15 , 0  , 0xFFFF);
        SERDES_SET(0xb310,  31 , 16 , 0x806F);
        SERDES_SET(0xb314,  15 , 0  , 0x03E4);
        SERDES_SET(0xb314,  31 , 16 , 0x7FFF);
        SERDES_SET(0xb318,  15 , 0  , 0x3CC3);
        SERDES_SET(0xb318,  31 , 16 , 0x23FF);
        SERDES_SET(0xb31c,  15 , 0  , 0x7A00);
        SERDES_SET(0xb31c,  31 , 16 , 0x2424);
        SERDES_SET(0xb320,  15 , 0  , 0x1F16);
        SERDES_SET(0xb320,  31 , 16 , 0xB84A);
        SERDES_SET(0xb324,  15 , 0  , 0x96B2);
        SERDES_SET(0xb324,  31 , 16 , 0xF300);
        SERDES_SET(0xb328,  15 , 0  , 0x1F53);
        SERDES_SET(0xb328,  31 , 16 , 0x0000);
        SERDES_SET(0xb32c,  15 , 0  , 0x0000);
        SERDES_SET(0xb32c,  31 , 16 , 0xFFFF);
        SERDES_SET(0xb330,  15 , 0  , 0x7000);
        SERDES_SET(0xb330,  31 , 16 , 0x0000);
        SERDES_SET(0xb334,  15 , 0  , 0xFFFF);
        SERDES_SET(0xb334,  31 , 16 , 0x0800);
        SERDES_SET(0xb338,  15 , 0  , 0x1018);
        SERDES_SET(0xb338,  31 , 16 , 0x0200);
        SERDES_SET(0xb33c,  15 , 0  , 0x0003);
        SERDES_SET(0xb33c,  31 , 16 , 0xBE10);
        SERDES_SET(0xb340,  15 , 0  , 0x78F1);
        SERDES_SET(0xb340,  31 , 16 , 0x9F87);
        SERDES_SET(0xb344,  15 , 0  , 0x4200);
        SERDES_SET(0xb344,  31 , 16 , 0xFFCD);
        SERDES_SET(0xb348,  15 , 0  , 0xBA00);
        SERDES_SET(0xb348,  31 , 16 , 0x604F);
        SERDES_SET(0xb34c,  15 , 0  , 0x40AA);
        SERDES_SET(0xb34c,  31 , 16 , 0x0057);
        SERDES_SET(0xb350,  15 , 0  , 0xC3E0);
        SERDES_SET(0xb350,  31 , 16 , 0x033E);
        SERDES_SET(0xb354,  15 , 0  , 0x619C);
        SERDES_SET(0xb354,  31 , 16 , 0x2002);
        SERDES_SET(0xb358,  15 , 0  , 0xFAE2);
        SERDES_SET(0xb358,  31 , 16 , 0xFEFA);
        SERDES_SET(0xb35c,  15 , 0  , 0x4E10);
        SERDES_SET(0xb35c,  31 , 16 , 0x4E10);
        SERDES_SET(0xb360,  15 , 0  , 0x0000);
        SERDES_SET(0xb360,  31 , 16 , 0x0000);
    }
    else if (12 == sds)
    {
        SERDES_SET(0xb818,  3  , 3  , 0);
        SERDES_SET(0xbbfc,  31 , 16 , 0x0);
        SERDES_SET(0xbb00,  15 , 0  , 0xF800);
        SERDES_SET(0xbb00,  31 , 16 , 0xC000);
        SERDES_SET(0xbb04,  15 , 0  , 0x57EF);
        SERDES_SET(0xbb04,  31 , 16 , 0xF000);
        SERDES_SET(0xbb08,  15 , 0  , 0x1000);
        SERDES_SET(0xbb08,  31 , 16 , 0x8000);
        SERDES_SET(0xbb0c,  15 , 0  , 0xBFFF);
        SERDES_SET(0xbb0c,  31 , 16 , 0xFFFF);
        SERDES_SET(0xbb10,  15 , 0  , 0xFFFF);
        SERDES_SET(0xbb10,  31 , 16 , 0x806F);
        SERDES_SET(0xbb14,  15 , 0  , 0x03E4);
        SERDES_SET(0xbb14,  31 , 16 , 0x7FFF);
        SERDES_SET(0xbb18,  15 , 0  , 0x3CC3);
        SERDES_SET(0xbb18,  31 , 16 , 0x23FF);
        SERDES_SET(0xbb1c,  15 , 0  , 0x7A00);
        SERDES_SET(0xbb1c,  31 , 16 , 0x2424);
        SERDES_SET(0xbb20,  15 , 0  , 0x1F16);
        SERDES_SET(0xbb20,  31 , 16 , 0xB84A);
        SERDES_SET(0xbb24,  15 , 0  , 0x96B2);
        SERDES_SET(0xbb24,  31 , 16 , 0xF300);
        SERDES_SET(0xbb28,  15 , 0  , 0x1F53);
        SERDES_SET(0xbb28,  31 , 16 , 0x0000);
        SERDES_SET(0xbb2c,  15 , 0  , 0x0000);
        SERDES_SET(0xbb2c,  31 , 16 , 0xFFFF);
        SERDES_SET(0xbb30,  15 , 0  , 0x7000);
        SERDES_SET(0xbb30,  31 , 16 , 0x0000);
        SERDES_SET(0xbb34,  15 , 0  , 0xFFFF);
        SERDES_SET(0xbb34,  31 , 16 , 0x0800);
        SERDES_SET(0xbb38,  15 , 0  , 0x1018);
        SERDES_SET(0xbb38,  31 , 16 , 0x0200);
        SERDES_SET(0xbb3c,  15 , 0  , 0x0003);
        SERDES_SET(0xbb3c,  31 , 16 , 0xBE10);
        SERDES_SET(0xbb40,  15 , 0  , 0x78F1);
        SERDES_SET(0xbb40,  31 , 16 , 0x9F87);
        SERDES_SET(0xbb44,  15 , 0  , 0x4200);
        SERDES_SET(0xbb44,  31 , 16 , 0xFFCD);
        SERDES_SET(0xbb48,  15 , 0  , 0xBA00);
        SERDES_SET(0xbb48,  31 , 16 , 0x604F);
        SERDES_SET(0xbb4c,  15 , 0  , 0x40AA);
        SERDES_SET(0xbb4c,  31 , 16 , 0x0057);
        SERDES_SET(0xbb50,  15 , 0  , 0xC3E0);
        SERDES_SET(0xbb50,  31 , 16 , 0x033E);
        SERDES_SET(0xbb54,  15 , 0  , 0x619C);
        SERDES_SET(0xbb54,  31 , 16 , 0x2002);
        SERDES_SET(0xbb58,  15 , 0  , 0xFAE2);
        SERDES_SET(0xbb58,  31 , 16 , 0xFEFA);
        SERDES_SET(0xbb5c,  15 , 0  , 0x4E10);
        SERDES_SET(0xbb5c,  31 , 16 , 0x4E10);
        SERDES_SET(0xbb60,  15 , 0  , 0x0000);
        SERDES_SET(0xbb60,  31 , 16 , 0x0000);
    }

    rtl839x_serdes_rst(sds);
}

void rtl8396_serdes_init_96m_1g_fiber (int sds)
{
    uint32  ofst;

    if (8 == sds)
    {
        ofst = 0x0;
    }
    else
    {
        ofst = 0x800;
    }

    SERDES_SET(0xb014 + ofst,  14  , 14  , 0x0);
    SERDES_SET(0xb004 + ofst,  30  , 30  , 0x0);
    SERDES_SET(0xb004 + ofst,  23  , 23  , 0x0);

    if (1 >= chip10gMP)
    {
        rtl8396_serdes_init_10gr_fiber(sds);
        rtl8396_serdes_10g_lcpll(sds);

        if (8 == sds)
        {
            SERDES_SET(0xb000,  8  , 8  , 1);
            SERDES_SET(0xb004,  9  , 8  , 0x0);
            SERDES_SET(0xb34c,  15 , 0  , 0x40aa);
            SERDES_SET(0xb35c,  15 , 0  , 0x4e10);
            SERDES_SET(0xb35c,  31 , 16 , 0x4e10);
            SERDES_SET(0x000c,  3  , 0  , 0x7);
            SERDES_SET(0xb018,  3  , 3  , 0);
            SERDES_SET(0xb3fc,  31 , 16 , 0x0);
            SERDES_SET(0xb358,  0  , 0  , 0);
            SERDES_SET(0xb358,  4  , 4  , 0);
            SERDES_SET(0xb358,  16 , 16 , 0);
            SERDES_SET(0xb358,  20 , 20 , 0);
            SERDES_SET(0xb338,  15 , 0  , 0x0722);
            SERDES_SET(0xb340,  15 , 0  , 0x18f5);
            SERDES_SET(0xb300,  11 , 11 , 0);
            SERDES_SET(0xb080,  13 , 13 , 0);
            SERDES_SET(0xb080,  6  , 6  , 1);
            SERDES_SET(0xb080,  12 , 12 , 1);
            SERDES_SET(0xb088,  7  , 7  , 1);
            SERDES_SET(0xb088,  8  , 8  , 1);
            SERDES_SET(0xb320,  5  , 5  , 1);
            SERDES_SET(0xb31c,  11 , 7  , 0x16);
            SERDES_SET(0x031c,  7  , 0  , 0x74);
        }
        else if (12 == sds)
        {
            SERDES_SET(0xb800,  8  , 8  , 1);
            SERDES_SET(0xb804,  9  , 8  , 0x0);

            SERDES_SET(0xbb4c,  15 , 0  , 0x40aa);
            SERDES_SET(0xbb5c,  15 , 0  , 0x4e10);
            SERDES_SET(0xbb5c,  31 , 16 , 0x4e10);

            SERDES_SET(0x000c,  19 , 16 , 0x7);

            SERDES_SET(0xb818,  3  , 3  , 0);
            SERDES_SET(0xbbfc,  31 , 16 , 0x0);
            SERDES_SET(0xbb58,  0  , 0  , 0);
            SERDES_SET(0xbb58,  4  , 4  , 0);
            SERDES_SET(0xbb58,  16 , 16 , 0);
            SERDES_SET(0xbb58,  20 , 20 , 0);
            SERDES_SET(0xbb38,  15 , 0  , 0x0722);
            SERDES_SET(0xbb40,  15 , 0  , 0x18f5);
            SERDES_SET(0xbb00,  11 , 11 , 0);
            SERDES_SET(0xb804,  9  , 8  , 0x0);
            SERDES_SET(0xb880,  13 , 13 , 0);
            SERDES_SET(0xb880,  6  , 6  , 1);
            SERDES_SET(0xb880,  12 , 12 , 1);
            SERDES_SET(0xb888,  7  , 7  , 1);
            SERDES_SET(0xb888,  8  , 8  , 1);
            SERDES_SET(0xbb20,  5  , 5  , 1);
            SERDES_SET(0xbb1c,  11 , 7  , 0x16);

            SERDES_SET(0x034c,  7  , 0  , 0x74);
        }

        rtl839x_serdes_rst(sds);

        rtl8396_serdes_init_efuseConfig(sds);
    }
    else
    {
        if (8 == sds)
        {
            SERDES_SET(0x000c,  3 , 0  , 0x0);
        }
        else
        {
            SERDES_SET(0x000c,  19 , 16  , 0x0);
        }

        rtl8396_serdes_ES_10g_qsgmii_patch(sds);

        if (8 == sds)
        {
            SERDES_SET(0xb004,  9  , 8  , 0x0);

            SERDES_SET(0xb080,  13 , 13 , 0);
            SERDES_SET(0xb080,  6  , 6  , 1);
            SERDES_SET(0xb080,  12 , 12 , 1);

            SERDES_SET(0xb088,  7  , 7  , 1);
            SERDES_SET(0xb088,  8  , 8  , 1);

            SERDES_SET(0xb320,  5  , 5  , 1);
            SERDES_SET(0xb31c,  11 , 7  , 0x16);

            SERDES_SET(0x000c,  3 , 0  , 0x7);
        }
        else
        {
            SERDES_SET(0xb804,  9  , 8  , 0x0);

            SERDES_SET(0xb880,  13 , 13 , 0);
            SERDES_SET(0xb880,  6  , 6  , 1);
            SERDES_SET(0xb880,  12 , 12 , 1);

            SERDES_SET(0xb888,  7  , 7  , 1);
            SERDES_SET(0xb888,  8  , 8  , 1);

            SERDES_SET(0xbb20,  5  , 5  , 1);
            SERDES_SET(0xbb1c,  11 , 7  , 0x16);

            SERDES_SET(0x000c,  19 , 16  , 0x7);
        }

        rtl839x_serdes_rst(sds);
    }
}

void rtl8396_serdes_init_10gr_50cm_copper (int sds)
{
    if (1 >= chip10gMP)
    {
        if (8 == sds)
        {
            SERDES_SET(0x000c,  3  , 0  , 0x1);
            SERDES_SET(0x031c,  7  , 0  , 0xff);
            SERDES_SET(0xB300,  31 , 0  , 0xC000F800);
            SERDES_SET(0xB304,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xB308,  31 , 0  , 0x80001000);
            SERDES_SET(0xB30C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xB310,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xB314,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xB318,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xB31C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xB320,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xB324,  31 , 0  , 0x730096B2);
            SERDES_SET(0xB328,  31 , 0  , 0x00000F53);
            SERDES_SET(0xB32C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xB330,  31 , 0  , 0x00007000);
            SERDES_SET(0xB334,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xB338,  31 , 0  , 0x02000000);
            SERDES_SET(0xB33C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xB340,  31 , 0  , 0xA11A44F1);
            SERDES_SET(0xB344,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xB348,  31 , 0  , 0x61CFFE1A);
            SERDES_SET(0xB34C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xB350,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xB354,  31 , 0  , 0x2002619C);
            SERDES_SET(0xB358,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xB35C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xB360,  31 , 0  , 0x00000000);
            SERDES_SET(0xB364,  31 , 0  , 0x00000000);
            SERDES_SET(0xB368,  31 , 0  , 0x00000000);
            SERDES_SET(0xB36C,  31 , 0  , 0x00000000);
            SERDES_SET(0xB370,  31 , 0  , 0x00000000);
            SERDES_SET(0xB374,  31 , 0  , 0x00000000);
            SERDES_SET(0xB378,  31 , 0  , 0x00000000);
            SERDES_SET(0xB37C,  31 , 0  , 0x00000000);
        }
        else if (12 == sds)
        {
            SERDES_SET(0x000c,  19 , 16  , 0x1);
            SERDES_SET(0x034c,  7  , 0  , 0xff);
            SERDES_SET(0xBB00,  31 , 0  , 0xC000F800);
            SERDES_SET(0xBB04,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xBB08,  31 , 0  , 0x80001000);
            SERDES_SET(0xBB0C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xBB10,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xBB14,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xBB18,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xBB1C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xBB20,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xBB24,  31 , 0  , 0x730096B2);
            SERDES_SET(0xBB28,  31 , 0  , 0x00000F53);
            SERDES_SET(0xBB2C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xBB30,  31 , 0  , 0x00007000);
            SERDES_SET(0xBB34,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xBB38,  31 , 0  , 0x02000000);
            SERDES_SET(0xBB3C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xBB40,  31 , 0  , 0xA11A44F1);
            SERDES_SET(0xBB44,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xBB48,  31 , 0  , 0x61CFFE1A);
            SERDES_SET(0xBB4C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xBB50,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xBB54,  31 , 0  , 0x2002619C);
            SERDES_SET(0xBB58,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xBB5C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xBB60,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB64,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB68,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB6C,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB70,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB74,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB78,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB7C,  31 , 0  , 0x00000000);
        }
        else
            return;

        rtl8396_serdes_10g_rx_rst(sds);

        rtl8396_serdes_init_10gr_rx_current(sds);

        rtl8396_serdes_init_efuseConfig(sds);
    }
}

void rtl8396_serdes_init_10gr_100cm_copper (int sds)
{
    if (1 >= chip10gMP)
    {
        if (8 == sds)
        {
            SERDES_SET(0x000c,  3  , 0  , 0x1);
            SERDES_SET(0x031c,  7  , 0  , 0xff);
            SERDES_SET(0xB300,  31 , 0  , 0xC000F800);
            SERDES_SET(0xB304,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xB308,  31 , 0  , 0x80001000);
            SERDES_SET(0xB30C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xB310,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xB314,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xB318,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xB31C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xB320,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xB324,  31 , 0  , 0x730096B2);
            SERDES_SET(0xB328,  31 , 0  , 0x00000F53);
            SERDES_SET(0xB32C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xB330,  31 , 0  , 0x00007000);
            SERDES_SET(0xB334,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xB338,  31 , 0  , 0x02000000);
            SERDES_SET(0xB33C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xB340,  31 , 0  , 0xC00344F1);
            SERDES_SET(0xB344,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xB348,  31 , 0  , 0x634FFE1A);
            SERDES_SET(0xB34C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xB350,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xB354,  31 , 0  , 0x2002619C);
            SERDES_SET(0xB358,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xB35C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xB360,  31 , 0  , 0x00000000);
            SERDES_SET(0xB364,  31 , 0  , 0x00000000);
            SERDES_SET(0xB368,  31 , 0  , 0x00000000);
            SERDES_SET(0xB36C,  31 , 0  , 0x00000000);
            SERDES_SET(0xB370,  31 , 0  , 0x00000000);
            SERDES_SET(0xB374,  31 , 0  , 0x00000000);
            SERDES_SET(0xB378,  31 , 0  , 0x00000000);
            SERDES_SET(0xB37C,  31 , 0  , 0x00000000);
        }
        else if (12 == sds)
        {
            SERDES_SET(0x000c,  19 , 16 , 0x1);
            SERDES_SET(0x034c,  7  , 0  , 0xff);
            SERDES_SET(0xBB00,  31 , 0  , 0xC000F800);
            SERDES_SET(0xBB04,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xBB08,  31 , 0  , 0x80001000);
            SERDES_SET(0xBB0C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xBB10,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xBB14,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xBB18,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xBB1C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xBB20,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xBB24,  31 , 0  , 0x730096B2);
            SERDES_SET(0xBB28,  31 , 0  , 0x00000F53);
            SERDES_SET(0xBB2C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xBB30,  31 , 0  , 0x00007000);
            SERDES_SET(0xBB34,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xBB38,  31 , 0  , 0x02000000);
            SERDES_SET(0xBB3C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xBB40,  31 , 0  , 0xC00344F1);
            SERDES_SET(0xBB44,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xBB48,  31 , 0  , 0x634FFE1A);
            SERDES_SET(0xBB4C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xBB50,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xBB54,  31 , 0  , 0x2002619C);
            SERDES_SET(0xBB58,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xBB5C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xBB60,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB64,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB68,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB6C,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB70,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB74,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB78,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB7C,  31 , 0  , 0x00000000);
        }
        else
            return;

        rtl8396_serdes_10g_rx_rst(sds);

        rtl8396_serdes_init_10gr_rx_current(sds);

        rtl8396_serdes_init_efuseConfig(sds);
    }
}

void rtl8396_serdes_init_10gr_300cm_copper (int sds)
{
    if (1 >= chip10gMP)
    {
        if (8 == sds)
        {
            SERDES_SET(0x000c,  3  , 0  , 0x1);
            SERDES_SET(0x031c,  7  , 0  , 0xff);
            SERDES_SET(0xB300,  31 , 0  , 0xC000F800);
            SERDES_SET(0xB304,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xB308,  31 , 0  , 0x80001000);
            SERDES_SET(0xB30C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xB310,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xB314,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xB318,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xB31C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xB320,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xB324,  31 , 0  , 0x730096B2);
            SERDES_SET(0xB328,  31 , 0  , 0x00000F53);
            SERDES_SET(0xB32C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xB330,  31 , 0  , 0x00007000);
            SERDES_SET(0xB334,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xB338,  31 , 0  , 0x02000000);
            SERDES_SET(0xB33C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xB340,  31 , 0  , 0xC21744F1);
            SERDES_SET(0xB344,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xB348,  31 , 0  , 0x62CFFE1A);
            SERDES_SET(0xB34C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xB350,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xB354,  31 , 0  , 0x2002619C);
            SERDES_SET(0xB358,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xB35C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xB360,  31 , 0  , 0x00000000);
            SERDES_SET(0xB364,  31 , 0  , 0x00000000);
            SERDES_SET(0xB368,  31 , 0  , 0x00000000);
            SERDES_SET(0xB36C,  31 , 0  , 0x00000000);
            SERDES_SET(0xB370,  31 , 0  , 0x00000000);
            SERDES_SET(0xB374,  31 , 0  , 0x00000000);
            SERDES_SET(0xB378,  31 , 0  , 0x00000000);
            SERDES_SET(0xB37C,  31 , 0  , 0x00000000);
        }
        else if (12 == sds)
        {
            SERDES_SET(0x000c,  19 , 16 , 0x1);
            SERDES_SET(0x034c,  7  , 0  , 0xff);
            SERDES_SET(0xBB00,  31 , 0  , 0xC000F800);
            SERDES_SET(0xBB04,  31 , 0  , 0xF00057EF);
            SERDES_SET(0xBB08,  31 , 0  , 0x80001000);
            SERDES_SET(0xBB0C,  31 , 0  , 0xFFFFBFFF);
            SERDES_SET(0xBB10,  31 , 0  , 0x806FFFFF);
            SERDES_SET(0xBB14,  31 , 0  , 0x7FFF03E4);
            SERDES_SET(0xBB18,  31 , 0  , 0x23FF3CC3);
            SERDES_SET(0xBB1C,  31 , 0  , 0x24247A00);
            SERDES_SET(0xBB20,  31 , 0  , 0xB84A9F16);
            SERDES_SET(0xBB24,  31 , 0  , 0x730096B2);
            SERDES_SET(0xBB28,  31 , 0  , 0x00000F53);
            SERDES_SET(0xBB2C,  31 , 0  , 0xFFFF0000);
            SERDES_SET(0xBB30,  31 , 0  , 0x00007000);
            SERDES_SET(0xBB34,  31 , 0  , 0x0800FFFF);
            SERDES_SET(0xBB38,  31 , 0  , 0x02000000);
            SERDES_SET(0xBB3C,  31 , 0  , 0xBE100003);
            SERDES_SET(0xBB40,  31 , 0  , 0xC21744F1);
            SERDES_SET(0xBB44,  31 , 0  , 0xFFCD4206);
            SERDES_SET(0xBB48,  31 , 0  , 0x62CFFE1A);
            SERDES_SET(0xBB4C,  31 , 0  , 0x0057C4F7);
            SERDES_SET(0xBB50,  31 , 0  , 0x033E97E0);
            SERDES_SET(0xBB54,  31 , 0  , 0x2002619C);
            SERDES_SET(0xBB58,  31 , 0  , 0xEEFAEAE2);
            SERDES_SET(0xBB5C,  31 , 0  , 0x0E4B4A50);
            SERDES_SET(0xBB60,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB64,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB68,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB6C,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB70,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB74,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB78,  31 , 0  , 0x00000000);
            SERDES_SET(0xBB7C,  31 , 0  , 0x00000000);
        }
        else
            return;

        rtl8396_serdes_10g_rx_rst(sds);

        rtl8396_serdes_init_10gr_rx_current(sds);

        rtl8396_serdes_init_efuseConfig(sds);
    }
}
#endif  /* CONFIG_RTL8396M_DEMO */

/* Function Name:
 *      rtl8390_serdes_config_init
 * Description:
 *      Serdes Configuration code that connect with RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_serdes_config_init(const rtk_switch_model_t *pModel)
{
    uint32  val;
    uint32  sdsId, idx;
    int8    sdsMask[MAX_SERDES];

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_CHIP_INFO_ADDR, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_OFFSET, RTL8390_CHIP_INFO_CHIP_INFO_EN_MASK, 0xA);

    if ((MEM32_READ(SWCORE_BASE_ADDR | RTL8390_CHIP_INFO_ADDR) & 0xffff) == 0x0399)
    {
        DBG_PRINT(1, "### rtl8390_serdes_config_init: Set CFG_NEG_CLKWR_A2D to 1 of Serdes 8~13 ###\n");
#if 0   /* the width of register is over 32bits, need to fix it */
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS8_9_XSG0_ADDR, \
            RTL8390_SDS8_9_XSG0_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG0_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS8_9_XSG1_ADDR, \
            RTL8390_SDS8_9_XSG1_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG1_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS10_11_XSG0_ADDR, \
            RTL8390_SDS10_11_XSG0_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG0_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS10_11_XSG1_ADDR, \
            RTL8390_SDS10_11_XSG1_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS8_9_XSG1_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS12_13_XSG0_ADDR, \
            RTL8390_SDS12_13_XSG0_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS12_13_XSG0_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_SDS12_13_XSG1_ADDR, \
            RTL8390_SDS12_13_XSG1_SR7_CFG_NEG_CLKWR_A2D_OFFSET, RTL8390_SDS12_13_XSG1_SR7_CFG_NEG_CLKWR_A2D_MASK, 1);
#endif

        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {
            uint32 sds;
            uint32 idx, i;
            uint32 try;

            /* serdes_PWR_save */
            for (i=0; i<=6; i++)
            {
                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_pwr_save)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_pwr_save[idx].reg + (i * 0x400), \
                                    rtl835x_mac_serdes_pwr_save[idx].val);
                }
            }

            /* Serdes Reset: ew 0xbb000014=0x00000010 */
            MEM32_WRITE(SWCORE_BASE_ADDR | RTL8390_RST_GLB_CTRL_ADDR, 0x00000010);

            for (idx=0; idx<(sizeof(rtl835x_mac_dis_25m_sdsck_out)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl835x_mac_dis_25m_sdsck_out[idx].reg, \
                                rtl835x_mac_dis_25m_sdsck_out[idx].val);
            }

            for (sds=0; sds<=6; sds+=2)
            {
                for (idx=0; idx<(sizeof(rtl835x_mac_2G5_serdes)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_2G5_serdes[idx].reg + (0x400 * (sds/2)), \
                                    rtl835x_mac_2G5_serdes[idx].val);
                }

                try = 1;
                do {
                    for (idx=0; idx<(sizeof(rtl835x_mac_serdes_0)/sizeof(confcode_mac_regval_t)); idx++)
                    {
                        MAC_REG_SET_CHK(rtl835x_mac_serdes_0[idx].reg + (0x400 * (sds/2)), \
                                        rtl835x_mac_serdes_0[idx].val);
                    }
                } while (0);
            }


            try = 1;
            do {
                for (idx=0; idx<(sizeof(rtl835x_mac_2G5_serdes_10)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_2G5_serdes_10[idx].reg + (0x400 * (10/2)), \
                                    rtl835x_mac_2G5_serdes_10[idx].val);
                }

                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_10)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_10[idx].reg, \
                                    rtl835x_mac_serdes_10[idx].val);
                }
            } while (0);


            try = 1;
            do {
                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_8)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_8[idx].reg, \
                                    rtl835x_mac_serdes_8[idx].val);
                }
            } while (0);

            try = 1;
            do {
                for (idx=0; idx<(sizeof(rtl835x_mac_serdes_12)/sizeof(confcode_mac_regval_t)); idx++)
                {
                    MAC_REG_SET_CHK(rtl835x_mac_serdes_12[idx].reg, \
                                    rtl835x_mac_serdes_12[idx].val);
                }
            } while (0);

            /* Reset Serdes */
            for (idx=0; idx<(sizeof(rtl835x_mac_serdes_rst)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl835x_mac_serdes_rst[idx].reg, \
                                rtl835x_mac_serdes_rst[idx].val);
            }

            /* Check Serdes Status */
            OSAL_MDELAY(500);
            /* sds = 0 ~ 12*/
            for (sds=0; sds<=12; sds+=2)
            {
                if (MEM32_READ(0xbb00a078 + (0x400 * (sds/2))) != 0x1ff0000)
                {
                    OSAL_PRINTF("[WARN] Serdes %u initail fail\n", (sds / 2));
                }
                else
                {
                    OSAL_PRINTF("[INFO] Serdes %u OK\n", (sds / 2));
                }
            }
        }
        else
        {
            uint32  idx;
            int8    sdsMask[MAX_SERDES];

            for (idx=0; idx<(sizeof(rtl839x_dis_ck25mo)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl839x_dis_ck25mo[idx].reg, rtl839x_dis_ck25mo[idx].val);
            }
            OSAL_MDELAY(500);

    #ifdef  QSGMII_MODE
            for (idx=0; idx<(sizeof(rtl839x_qsgmii)/sizeof(confcode_mac_regval_t)); idx++)
            {
                MAC_REG_SET_CHK(rtl839x_qsgmii[idx].reg, rtl839x_qsgmii[idx].val);
            }
    #endif

            memset(sdsMask, MAX_SERDES, MAX_SERDES);

            for (idx = 0; idx < pModel->serdes.count; ++idx)
            {
                sdsMask[pModel->serdes.list[idx].sds_id] = pModel->serdes.list[idx].phy_idx;
            }

            for (idx = 0; idx < MAX_SERDES; ++idx)
            {
                if (MAX_SERDES != sdsMask[idx])
                {
                    rtl8390_5G_serdes_config(pModel, idx, sdsMask[idx]);
                    if (8 == idx)
                    {
                        ++idx;
                    }
                }
                /* #power down */
                else
                {
                    switch (idx)
                    {
                        case 6:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xaf40, 0xcf0104aa);
                                ++idx;
                            }
                            break;
                        case 8:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xb378, 0x1c005);
                                ++idx;
                            }
                            break;
                        case 10:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xb740, 0xcf01080f);
                                ++idx;
                            }
                            break;
                        case 12:
                            if (MAX_SERDES != sdsMask[idx + 1])
                            {
                                MAC_REG_SET_CHK(0xbb78, 0x1c005);
                                ++idx;
                            }
                            break;
                    }
                }
            }
        }
    }
    else
    {
        rtl839x_serdes_patch_init();

        if ((pModel->chip == RTK_CHIP_RTL8352M) || \
            (pModel->chip == RTK_CHIP_RTL8353M))
        {
            rtl839x_5x_serdes_patch_init();
        }

        memset(sdsMask, MAX_SERDES, MAX_SERDES);

        for (idx = 0; idx < pModel->serdes.count; ++idx)
        {
            sdsMask[pModel->serdes.list[idx].sds_id] = pModel->serdes.list[idx].phy_idx;
        }

        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
            {
                if (RTK_CHIP_NONE == pModel->phy.list[(int)sdsMask[sdsId]].chip &&
                        (pModel->chip != RTK_CHIP_RTL8396M))
                {
                    switch (sdsId)
                    {
                        case 12:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S12[idx]);
                            break;
                        case 13:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S13[idx]);
                            break;
                        default:
                            printf("The serdes can't pure fiber\n");
                    }
                }
            }
            else
            {
                /* power off */
                MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_SERDES_IF_CTRL_ADDR(sdsId), \
                        RTL8390_MAC_SERDES_IF_CTRL_SERDES_SPD_SEL_OFFSET(sdsId), \
                        RTL8390_MAC_SERDES_IF_CTRL_SERDES_SPD_SEL_MASK(sdsId), 0);
            }
        }

        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
                rtl839x_serdes_rst(sdsId);
        }

        if ((pModel->chip == RTK_CHIP_RTL8391M) || \
                (pModel->chip == RTK_CHIP_RTL8392M) || \
                (pModel->chip == RTK_CHIP_RTL8393M) || \
                (pModel->chip == RTK_CHIP_RTL8396M))
        {
            rtl839x_93m_rst_sys();
        }
        else
            rtl839x_53m_rst_sys();

        /* digit serdes config must be after MAC serdes reset */
        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
            {
                if (RTK_CHIP_NONE == pModel->phy.list[(int)sdsMask[sdsId]].chip &&
                        (pModel->chip != RTK_CHIP_RTL8396M))
                {
                    switch (sdsId)
                    {
                        case 12:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S12[idx]);
                            break;
                        case 13:
                            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                                SERDES_PATCH_SET_CHK(rtl839x_init_fiber_1g_frc_S13[idx]);
                            break;
                        default:
                            printf("The serdes can't pure fiber\n");
                    }
                }

                rtl8390_serdes_eee_enable(sdsId);
            }
        }

        rtl8390_phyPowerOff();

        /* power off */
        if ((pModel->chip == RTK_CHIP_RTL8352M) || (pModel->chip == RTK_CHIP_RTL8353M))
        {
            for (idx = 0; idx < (sizeof(rtl835x_serdes_powerOff_conf)/sizeof(confcode_mac_regval_t)); ++idx)
            {
                MAC_REG_SET_CHK(rtl835x_serdes_powerOff_conf[idx].reg, rtl835x_serdes_powerOff_conf[idx].val);
            }
        }

        if (pModel->chip == RTK_CHIP_RTL8392M ||
                pModel->chip == RTK_CHIP_RTL8391M)
        {
            val = MEM32_READ(SWCORE_BASE_ADDR + 0xAF40);
            val |= (0xF << 24);

            MEM32_WRITE(SWCORE_BASE_ADDR + 0xAF40, val);
        }

        /* serdes link check */
        OSAL_MDELAY(500);

        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
            {
                if (RTK_CHIP_NONE == pModel->phy.list[(int)sdsMask[sdsId]].chip)
                {
                    continue;
                }

                rtl8390_serdes_chk(pModel, pModel->serdes.list[sdsId].sds_id);
            }
        }
    }

    #if defined(CONFIG_RTL8396M_DEMO)
    if (pModel->chip == RTK_CHIP_RTL8396M)
    {
        rtl8396_serdes_init_96m();

        for (sdsId = 0; sdsId < MAX_SERDES; ++sdsId)
        {
            if (MAX_SERDES != sdsMask[sdsId])
            {
                if (RTK_CHIP_NONE == pModel->phy.list[(int)sdsMask[sdsId]].chip)
                {
                    rtl8396_serdes_init_10gr_fiber(sdsId);
                }
            }
        }

        if (1 == chip10gMP)
        {
            rtl8396_serdes_init_efuseConfig(8);
            rtl8396_serdes_init_efuseConfig(12);
        }

        rtl8390_phyPortPowerOff(24);
        rtl8390_phyPortPowerOff(36);

        rtl8396_10gMediaType[0] = 0;
        rtl8396_10gMediaType[1] = 0;
    }
    #endif  /* CONFIG_RTL8396M_DEMO */

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_CHIP_INFO_ADDR, \
        RTL8390_CHIP_INFO_CHIP_INFO_EN_OFFSET, RTL8390_CHIP_INFO_CHIP_INFO_EN_MASK, 0x0);

    return;
} /* end of rtl8390_serdes_config_init */

confcode_serdes_patch_t rtl839x_init_fiber_100_S12[] = {
    { 0x000c  , 0       , 19 , 16 , 0x8     },
    { 0xbbfc  , 0       , 16 , 16 , 0       },
    { 0xbbfc  , 0       , 19 , 18 , 0x0     },
    { 0xbb58  , 0       , 0  , 0  , 0       },
    { 0xbb58  , 0       , 4  , 4  , 0       },
    { 0xbb58  , 0       , 16 , 16 , 0       },
    { 0xbb58  , 0       , 20 , 20 , 0       },

    { 0xb880  , 0       , 12 , 12 , 0       },
    { 0xb880  , 0       , 13 , 13 , 1       },
    { 0xb880  , 0       , 6  , 6  , 0       },
    { 0xbb20  , 0       , 5  , 5  , 1       },
    { 0xbb1c  , 0       , 11 , 7  , 0x15    },
};

confcode_serdes_patch_t rtl839x_init_fiber_100_S13[] = {
    { 0x000c  , 0       , 23 , 20 , 0x8     },
    { 0xbbfc  , 0       , 17 , 17 , 0       },
    { 0xbbfc  , 0       , 21 , 20 , 0x0     },
    { 0xbbd8  , 0       , 0  , 0  , 0       },
    { 0xbbd8  , 0       , 4  , 4  , 0       },
    { 0xbbd8  , 0       , 16 , 16 , 0       },
    { 0xbbd8  , 0       , 20 , 20 , 0       },

    { 0xb980  , 0       , 12 , 12 , 0       },
    { 0xb980  , 0       , 13 , 13 , 1       },
    { 0xb980  , 0       , 6  , 6  , 0       },
    { 0xbb20  , 0x80    , 5  , 5  , 1       },
    { 0xbb1c  , 0x80    , 11 , 7  , 0x15    },
};

confcode_serdes_patch_t rtl839x_sfp_rst_S12[] = {
    #if 1
    { 0xbbf8  , 0       , 21 , 20 , 0x0003  },
    { 0xbbf8  , 0       , 25 , 24 , 0x0001  },
    { 0xbbf8  , 0       , 21 , 20 , 0x0001  },
    { 0xbbf8  , 0       , 25 , 24 , 0x0003  },
    { 0xbbf8  , 0       , 21 , 20 , 0x0000  },
    { 0xbbf8  , 0       , 25 , 24 , 0x0000  },
    #else
    { 0xbbf8  , 0       , 19 , 16 , 0x0005  },
    { 0xbbf8  , 0       , 19 , 16 , 0x000f  },
    #endif
    { 0xb804, 0, 22,  22,  1},
    { 0xb804, 0, 22,  22,  0},
};

confcode_serdes_patch_t rtl839x_sfp_rst_S13[] = {
    #if 1
    { 0xbbf8  , 0       , 23 , 22 , 0x0003  },
    { 0xbbf8  , 0       , 27 , 26 , 0x0001  },
    { 0xbbf8  , 0       , 23 , 22 , 0x0001  },
    { 0xbbf8  , 0       , 27 , 26 , 0x0003  },
    { 0xbbf8  , 0       , 23 , 22 , 0x0000  },
    { 0xbbf8  , 0       , 27 , 26 , 0x0000  },
    #else
    { 0xbbf8  , 0       , 19 , 16 , 0x0005  },
    { 0xbbf8  , 0       , 19 , 16 , 0x000f  },
    #endif
    { 0xb904, 0, 22,  22,  1},
    { 0xb904, 0, 22,  22,  0},
};

/* Function Name:
 *      rtl8390_misc_config_init
 * Description:
 *      Misc Configuration code in the RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void rtl8390_misc_config_init(const rtk_switch_model_t *pModel)
{
    uint32 port;

#if defined(CONFIG_SOFTWARE_CONTROL_LED)
    if ((pModel->chip == RTK_CHIP_RTL8352M) || (pModel->chip == RTK_CHIP_RTL8353M))
    {
        /*Setup the board LED information to swCtrl_led module*/
        swCtrl_led_init();

        uboot_isr_register(RTK_DEV_TC0, swCtrl_led_intr_handler, NULL);
        common_enable_irq(29);/* TC0_IRQ */
        common_enable_interrupt();
    }
#endif

    rtk_eee_off(pModel);

    /* enable special congest and set congest timer to 2 sec */
    for (port=0; port < 52; port++)
    {
        MEM32_WRITE(SWCORE_BASE_ADDR| RTL8390_SC_P_EN_ADDR(port), (0x2<<0) | (0x2<<16));
    }

    /* Disable 48 pass 1,
       so high speed to low speed Tx experiment in half mode won't drop packet by this function */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_MAC_GLB_CTRL_ADDR, \
            RTL8390_MAC_GLB_CTRL_MAC_DROP_48PASS1_EN_OFFSET,        \
            RTL8390_MAC_GLB_CTRL_MAC_DROP_48PASS1_EN_MASK, 0x0);

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_MAC_GLB_CTRL_ADDR, \
            RTL8390_MAC_GLB_CTRL_HALF_48PASS1_EN_OFFSET,            \
            RTL8390_MAC_GLB_CTRL_HALF_48PASS1_EN_MASK, 0x0);

    /* half duplex backpressure collision method */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR | RTL8390_MAC_GLB_CTRL_ADDR, \
            RTL8390_MAC_GLB_CTRL_BKPRES_MTHD_SEL_OFFSET,            \
            RTL8390_MAC_GLB_CTRL_BKPRES_MTHD_SEL_MASK, 0x0);

    #if defined(CONFIG_RTL8396M_DEMO)
    if (pModel->chip == RTK_CHIP_RTL8396M)
    {
        uint32  val;

        /* Table read for egress bandwidth of 10G port 24 to be 10Gbps */
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x611c, 0x218);

        /* Data correct to two registers */
        val = MEM32_READ(SWCORE_BASE_ADDR | 0x613c);
        val &= ~(0xFF);
        val |= 0x98;
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x613c, val);

        val = MEM32_READ(SWCORE_BASE_ADDR | 0x6140);
        val &= ~(0xFFF << 20);
        val |= (0x968 << 20);
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x6140, val);

        val = MEM32_READ(SWCORE_BASE_ADDR | 0x60f8);
        val &= ~(0xFFFF << 16);
        val |= (0x52a << 16);
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x60f8, val);

        /* Table write */
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x611c, 0x318);

        /* Table read for egress bandwidth of 10G port 36 to be 10Gbps */
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x611c, 0x224);

        /* Data correct to two registers */
        val = MEM32_READ(SWCORE_BASE_ADDR | 0x613c);
        val &= ~(0xFF);
        val |= 0x98;
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x613c, val);

        val = MEM32_READ(SWCORE_BASE_ADDR | 0x6140);
        val &= ~(0xFFF << 20);
        val |= (0x968 << 20);
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x6140, val);

        val = MEM32_READ(SWCORE_BASE_ADDR | 0x60f8);
        val &= ~(0xFFFF << 16);
        val |= (0x52a << 16);
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x60f8, val);

        /* Table write */
        MEM32_WRITE(SWCORE_BASE_ADDR | 0x611c, 0x324);
    }
    #endif  /* CONFIG_RTL8396M_DEMO */

    return;
} /* end of rtl8390_misc_config_init */

#if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
void rtl8231_init(void)
{
    uint32 val;

    /*RTL838xM internal GPIO_A1 is defined to reset rtl8231,
        so we need to set internal GPIO_A1 to be 0x1 before rtl8231
        init*/
    /*configure GPIO_A1 direction as output*/
    val = REG32(GPIO_PABC_DIR);
    REG32(GPIO_PABC_DIR) = val | (1<<GPIO_A1);

    /*configure GPIO_A1 data as 0x1*/
    val = REG32(GPIO_PABC_DATA);
    REG32(GPIO_PABC_DATA) = val | (1<<GPIO_A1);

    /*init rtl8231*/
    rtl8390_smiInit(GPIO_A2,GPIO_A3);
    rtl8390_smiWrite(RTL8231_ADDR, RTL8231_PIN_SEL_REG, 0xffff);
    rtl8390_smiWrite(RTL8231_ADDR, RTL8231_PIN_SEL_REG+1, 0xffff);

    rtl8231_pin_direction_set(RTL8231_PIN_5,rtl8231_PIN_DIR_OUT);
    rtl8231_pin_direction_set(RTL8231_PIN_6,rtl8231_PIN_DIR_OUT);
    rtl8231_pin_direction_set(RTL8231_PIN_8,rtl8231_PIN_DIR_OUT);

    rtl8231_pin_data_set(RTL8231_PIN_5,0x1);
    rtl8231_pin_data_set(RTL8231_PIN_6,0x1);
    rtl8231_pin_data_set(RTL8231_PIN_8,0x1);

    rtl8390_smiRead(RTL8231_ADDR, 0, &val);
    val |= 0x2;
    rtl8390_smiWrite(RTL8231_ADDR, 0, val);
}
#endif

/* Function Name:
 *      rtl8390_config
 * Description:
 *      Configuration code for RTL8390
 * Input:
 *      pModel - pointer to switch model of platform
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtl8390_config(const rtk_switch_model_t *pModel)
{
    uint32  model_info;

    model_info = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_MODEL_NAME_INFO_ADDR);

    #if defined(CONFIG_RTL8396M_DEMO)
    chip10gMP = (model_info & 0xf) >> 1;
    #endif

    OSAL_PRINTF("Model Info: %x\n", model_info);

    #if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
    rtl8231_init();
    #endif

    DBG_PRINT(1, "### Reset PHY ###\n");
    rtl8390_phyReset(gSwitchModel);

    DBG_PRINT(1, "### Platform Config ###\n");
    rtl8390_platform_config_init(gSwitchModel);

    DBG_PRINT(1, "### MAC Config ###\n");
    rtl8390_mac_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer MAC Config ###\n");
    customer_mac_config_init(gSwitchModel);
#endif

    DBG_PRINT(1, "### Power down PHY (RTL82XX) ###\n");
    rtl8390_phyPowerOff();

    DBG_PRINT(1, "### PHY Config (RTL82XX) ###\n");
    rtl8390_phy_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer PHY Config (RTL82XX) ###\n");
    customer_phy_config_init(gSwitchModel);
#endif

    DBG_PRINT(1, "### Serdes Config ###\n");
    rtl8390_serdes_config_init(gSwitchModel);

    DBG_PRINT(1, "### Misc Config ###\n");
    rtl8390_misc_config_init(gSwitchModel);

#if defined(CONFIG_CUSTOMER_BOARD)
    DBG_PRINT(1, "### Customer Misc Config ###\n");
    customer_mac_misc_config_init(gSwitchModel);
#endif

    return;
} /* end of rtl8390_config */

void rtl8390_sfp_speed_set(int port, int speed)
{
    int idx;

    if (1000 == speed)
    {
        if (0 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_1g_frc_S12[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S12[idx]);
        }
        else if (1 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_1g_frc_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_1g_frc_S13[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S13[idx]);
        }
    }
    else if (100 == speed)
    {
        if (0 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_100_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_100_S12[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S12)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S12[idx]);
        }
        else if (1 == port)
        {
            for (idx = 0; idx < (sizeof(rtl839x_init_fiber_100_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_init_fiber_100_S13[idx]);

            for (idx = 0; idx < (sizeof(rtl839x_sfp_rst_S13)/sizeof(confcode_serdes_patch_t)); ++idx)
                SERDES_PATCH_SET(rtl839x_sfp_rst_S13[idx]);
        }
    }

    return;
}

#if defined(CONFIG_RTL8396M_DEMO)
/* 0: 10G fiber, 1: 50cm, 2: 100cm, 3: 300cm, 4: 1G fiber */
void rtl8390_10gMedia_set(int port, int media)
{
    int sds, idx;

    switch (port)
    {
        case 24:
            sds = 8;
            idx = 0;
            break;
        case 36:
            sds = 12;
            idx = 1;
            break;
        default:
            return;
    }

    switch (media)
    {
        case 0:
            rtl8396_serdes_init_10gr_fiber(sds);
            break;
        case 1:
            rtl8396_serdes_init_10gr_50cm_copper(sds);
            break;
        case 2:
            rtl8396_serdes_init_10gr_100cm_copper(sds);
            break;
        case 3:
            rtl8396_serdes_init_10gr_300cm_copper(sds);
            break;
        case 4:
            rtl8396_serdes_init_96m_1g_fiber(sds);
            break;
        default:
            return;
    }

    rtl8396_10gMediaType[idx] = media;

    return;
}

void rtl8390_10gMedia_get(int port, int *media)
{
    int idx;

    if (24 == port)
        idx = 0;
    else
        idx = 1;

    *media = rtl8396_10gMediaType[idx];

    return;
}

void rtl8390_10gSds_restart(int port)
{
    int media;

    if (1 >= chip10gMP)
        return;

    rtl8390_10gMedia_get(port, &media);
    if (0 != media)
        return;

    if (24 != port && 36 != port)
        return;

    rtl8396_10gSds_restart(port);

    return;
}

void rtl8390_10gSds_init(int port)
{
    if (2 >= chip10gMP)
        return;

    if (24 != port && 36 != port)
        return;

    if (24 == port)
        rtl8396_serdes_10g_leq_init(8);
    else
        rtl8396_serdes_10g_leq_init(12);

    return;
}
#endif  /* CONFIG_RTL8396M_DEMO */
