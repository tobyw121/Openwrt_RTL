/*
 * Copyright (C) 2009-2010 Realtek Semiconductor Corp.
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated,
 * modified or distributed under the authorized license from Realtek.
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED.
 *
 * $Revision: 53218 $
 * $Date: 2014-11-19 19:52:01 +0800 (Wed, 19 Nov 2014) $
 *
 * Purpose : PHY 8212B/8214B/8214BF Driver APIs.
 *
 * Feature : PHY 8212B/8214B/8214BF Driver APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <osal/time.h>
#include <osal/memory.h>
#include <ioal/mem32.h>
#include <hal/common/halctrl.h>
#include <hal/common/miim.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_common.h>
#include <hal/phy/phy_8214f.h>
#include <hal/phy/phy_8214fb.h>
#include <rtk/default.h>
#include <hal/phy/phy_8214fb_patch.h>

/*
 * Symbol Definition
 */
#if 0 /* U-Boot have do it, not need to do again */
typedef struct phy_8214fb_confcode_global_s {
    uint16  phy_id;
    uint16  reg_no;
    uint16  reg_val;
} phy_8214fb_confcode_global_t;
#endif

typedef struct phy_8214fb_confcode_port_s {
    uint16  page_no;
    uint16  reg_no;
    uint16  reg_val;
} phy_8214fb_confcode_port_t;

#if 0 /* U-Boot have do it, not need to do again */
/* The following configuration is 2010-12-22 version */
phy_8214fb_confcode_global_t rtl8214fb_perchip[] = {\
    /* Reset Ser-Des0~5 and interface selection to RSGMII with 4 ports 1000Base-T/(1000Base-X or 100Base-FX) and change LED polarity */ \
    {3, 0x1f, 0x0008}, {3, 0x1c, 0xff00}, {3, 0x1f, 0x0008}, \
    {0, 0x1f, 0x0008}, {0, 0x11, 0x3197}, {0, 0x1f, 0x0008}, \
    {0, 0x1f, 0x000f}, {0, 0x1e, 0x0013}, {0, 0x1a, 0xE46A}, {0, 0x1f, 0x0008}, \
    {1, 0x1f, 0x000f}, {1, 0x1e, 0x0013}, {1, 0x1a, 0xE46A}, {1, 0x1f, 0x0008}, \
    {2, 0x1f, 0x000f}, {2, 0x1e, 0x0013}, {2, 0x1a, 0xE46A}, {2, 0x1f, 0x0008}, \
    {3, 0x1f, 0x000f}, {3, 0x1e, 0x0013}, {3, 0x1a, 0xE46A}, {3, 0x1f, 0x0008}, \
    {3, 0x1f, 0x0008}, {3, 0x1c, 0x9000}, {3, 0x1c, 0x0000}, {3, 0x11, 0x3117}, \
    /* Serial LED Active High to Low */ \
    {0, 0x1f, 0x0008}, {0, 0x1b, 0x000A}, {0, 0x1f, 0x0008}, \
    /* FIFO Setting */ \
    {1, 0x1f, 0x0008}, {1, 0x1b, 0x00D0}, {1, 0x1f, 0x0008}, \
    /* Force Select Copper Standard Register */ \
    {1, 0x1f, 0x0008}, {1, 0x10, 0x0F00}, {1, 0x1f, 0x0008}, \
    /* Enable Parallel Write */ \
    {3, 0x1f, 0x0008}, {3, 0x18, 0x0001}, {3, 0x1f, 0x0008}, \
    /* Modify Ser-Des0,1 DC Offset Calibration Tuning Range */ \
    {3, 0x1f, 0x000F}, {3, 0x1e, 0x000C}, {3, 0x1b, 0xBE03}, {3, 0x1c, 0x8C42}, \
    {3, 0x15, 0x4343}, {3, 0x1f, 0x0008}, \
    /* Modify Ser-Des0,1 Analog Parameter */ \
    {3, 0x1f, 0x000F}, {3, 0x1e, 0x000D}, {3, 0x16, 0x0900}, {3, 0x15, 0x856A}, \
    {3, 0x18, 0x80C8}, {3, 0x17, 0x5B93}, {3, 0x17, 0x5393}, {3, 0x17, 0x5B93}, \
    {3, 0x1f, 0x0008}, \
    /* Modify Ser-Des2~5 Register MSB/LSB Swap */ \
    {3, 0x1f, 0x000F}, {3, 0x1e, 0x001D}, {3, 0x16, 0x0C20}, {3, 0x17, 0xACE5}, \
    {3, 0x1f, 0x0008}, \
    /* Power Down PHY */ \
    {3, 0x1f, 0x0008}, {3, 0x00, 0x1940}, {3, 0x1f, 0x0008}, \
    /* Lock Naro-C */ \
    {3, 0x1f, 0x0007}, {3, 0x1e, 0x0023}, {3, 0x17, 0x1111}, {3, 0x1f, 0x0008}, \
    /* Naro-C Patch */ \
    {3, 0x1f, 0x0007}, {3, 0x1e, 0x0023}, {3, 0x16, 0x0306}, {3, 0x16, 0x0307}, \
    {3, 0x15, 0x0181}, {3, 0x19, 0x6C09}, {3, 0x15, 0x0285}, {3, 0x19, 0x4400}, \
    {3, 0x15, 0x0286}, {3, 0x19, 0x4020}, {3, 0x15, 0x0287}, {3, 0x19, 0x4480}, \
    {3, 0x15, 0x0288}, {3, 0x19, 0x9E00}, {3, 0x15, 0x0289}, {3, 0x19, 0x4891}, \
    {3, 0x15, 0x028A}, {3, 0x19, 0x4CC0}, {3, 0x15, 0x028B}, {3, 0x19, 0x4801}, \
    {3, 0x15, 0x03E5}, {3, 0x19, 0x4478}, {3, 0x15, 0x0000}, {3, 0x16, 0x0306}, \
    {3, 0x16, 0x0300}, {3, 0x1f, 0x0008}, \
    /* Micro-C Patch */ \
    {3, 0x1f, 0x0005}, {3, 0x05, 0xFFF6}, {3, 0x06, 0x0080}, {3, 0x05, 0x8B5C}, \
    {3, 0x06, 0x0000}, {3, 0x0F, 0x0100}, \
    {3, 0x05, 0x8000}, {3, 0x06, 0x0280}, {3, 0x06, 0x48F7}, \
    {3, 0x06, 0x00E0}, {3, 0x06, 0xFFF7}, {3, 0x06, 0xA080}, {3, 0x06, 0x02AE}, \
    {3, 0x06, 0xF602}, {3, 0x06, 0x00F7}, {3, 0x06, 0x0201}, {3, 0x06, 0x0402}, \
    {3, 0x06, 0x0111}, {3, 0x06, 0x0201}, {3, 0x06, 0x2102}, {3, 0x06, 0x8072}, \
    {3, 0x06, 0x0201}, {3, 0x06, 0x4702}, {3, 0x06, 0x8085}, {3, 0x06, 0xE08B}, \
    {3, 0x06, 0x88E1}, {3, 0x06, 0x8B89}, {3, 0x06, 0x1E01}, {3, 0x06, 0xE18B}, \
    {3, 0x06, 0x8A1E}, {3, 0x06, 0x01E1}, {3, 0x06, 0x8B8B}, {3, 0x06, 0x1E01}, \
    {3, 0x06, 0xE18B}, {3, 0x06, 0x8C1E}, {3, 0x06, 0x01E1}, {3, 0x06, 0x8B8D}, \
    {3, 0x06, 0x1E01}, {3, 0x06, 0xE18B}, {3, 0x06, 0x8E1E}, {3, 0x06, 0x01A0}, \
    {3, 0x06, 0x00C7}, {3, 0x06, 0xAEBB}, {3, 0x06, 0xEE8B}, {3, 0x06, 0x5702}, \
    {3, 0x06, 0x0281}, {3, 0x06, 0xC8BF}, {3, 0x06, 0x8B88}, {3, 0x06, 0xEC00}, \
    {3, 0x06, 0x19A9}, {3, 0x06, 0x8B90}, {3, 0x06, 0xF9EE}, {3, 0x06, 0xFFF6}, \
    {3, 0x06, 0x00EE}, {3, 0x06, 0xFFF7}, {3, 0x06, 0xFCD1}, {3, 0x06, 0x00BF}, \
    {3, 0x06, 0x85D9}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAD1}, {3, 0x06, 0x01BF}, \
    {3, 0x06, 0x85DC}, {3, 0x06, 0x022A}, {3, 0x06, 0xFA04}, {3, 0x06, 0xF8E0}, \
    {3, 0x06, 0x8B8C}, {3, 0x06, 0xAD20}, {3, 0x06, 0x0AEE}, {3, 0x06, 0x8B8C}, \
    {3, 0x06, 0x0002}, {3, 0x06, 0x1678}, {3, 0x06, 0x0282}, {3, 0x06, 0xE3FC}, \
    {3, 0x06, 0x04F8}, {3, 0x06, 0xE08B}, {3, 0x06, 0x8EAD}, {3, 0x06, 0x2020}, \
    {3, 0x06, 0xF620}, {3, 0x06, 0xE48B}, {3, 0x06, 0x8E02}, {3, 0x06, 0x81DF}, \
    {3, 0x06, 0x0281}, {3, 0x06, 0x0D02}, {3, 0x06, 0x8251}, {3, 0x06, 0x0202}, \
    {3, 0x06, 0xB802}, {3, 0x06, 0x228F}, {3, 0x06, 0x0226}, {3, 0x06, 0xC702}, \
    {3, 0x06, 0x0372}, {3, 0x06, 0x021E}, {3, 0x06, 0x9C02}, {3, 0x06, 0x82FD}, \
    {3, 0x06, 0xE08B}, {3, 0x06, 0x8EAD}, {3, 0x06, 0x2108}, {3, 0x06, 0xF621}, \
    {3, 0x06, 0xE48B}, {3, 0x06, 0x8E02}, {3, 0x06, 0x0338}, {3, 0x06, 0xE08B}, \
    {3, 0x06, 0x8EAD}, {3, 0x06, 0x2208}, {3, 0x06, 0xF622}, {3, 0x06, 0xE48B}, \
    {3, 0x06, 0x8E02}, {3, 0x06, 0x1DF9}, {3, 0x06, 0xE08B}, {3, 0x06, 0x8EAD}, \
    {3, 0x06, 0x2308}, {3, 0x06, 0xF623}, {3, 0x06, 0xE48B}, {3, 0x06, 0x8E02}, \
    {3, 0x06, 0x282D}, {3, 0x06, 0xE08B}, {3, 0x06, 0x8EAD}, {3, 0x06, 0x2405}, \
    {3, 0x06, 0xF624}, {3, 0x06, 0xE48B}, {3, 0x06, 0x8EE0}, {3, 0x06, 0x8B8E}, \
    {3, 0x06, 0xAD25}, {3, 0x06, 0x05F6}, {3, 0x06, 0x25E4}, {3, 0x06, 0x8B8E}, \
    {3, 0x06, 0xE08B}, {3, 0x06, 0x8EAD}, {3, 0x06, 0x260B}, {3, 0x06, 0xF626}, \
    {3, 0x06, 0xE48B}, {3, 0x06, 0x8E02}, {3, 0x06, 0x84F8}, {3, 0x06, 0x0283}, \
    {3, 0x06, 0x90E0}, {3, 0x06, 0x8B8E}, {3, 0x06, 0xAD27}, {3, 0x06, 0x05F6}, \
    {3, 0x06, 0x27E4}, {3, 0x06, 0x8B8E}, {3, 0x06, 0x0202}, {3, 0x06, 0x64FC}, \
    {3, 0x06, 0x04F8}, {3, 0x06, 0xFAEF}, {3, 0x06, 0x69E0}, {3, 0x06, 0x8B85}, \
    {3, 0x06, 0xAD21}, {3, 0x06, 0x47E0}, {3, 0x06, 0xE022}, {3, 0x06, 0xE1E0}, \
    {3, 0x06, 0x2358}, {3, 0x06, 0xC059}, {3, 0x06, 0x021E}, {3, 0x06, 0x01E1}, \
    {3, 0x06, 0x8B60}, {3, 0x06, 0x1F10}, {3, 0x06, 0x9E34}, {3, 0x06, 0xE48B}, \
    {3, 0x06, 0x60AD}, {3, 0x06, 0x212B}, {3, 0x06, 0xE18B}, {3, 0x06, 0x84F7}, \
    {3, 0x06, 0x29E5}, {3, 0x06, 0x8B84}, {3, 0x06, 0xAC27}, {3, 0x06, 0x18AC}, \
    {3, 0x06, 0x2605}, {3, 0x06, 0x0281}, {3, 0x06, 0xA8AE}, {3, 0x06, 0x1BD1}, \
    {3, 0x06, 0x02BF}, {3, 0x06, 0x2535}, {3, 0x06, 0x022A}, {3, 0x06, 0xFA02}, \
    {3, 0x06, 0x858B}, {3, 0x06, 0x0224}, {3, 0x06, 0xCEAE}, {3, 0x06, 0x0B02}, \
    {3, 0x06, 0x8564}, {3, 0x06, 0x0224}, {3, 0x06, 0xE7AE}, {3, 0x06, 0x0302}, \
    {3, 0x06, 0x8163}, {3, 0x06, 0xEF96}, {3, 0x06, 0xFEFC}, {3, 0x06, 0x0402}, \
    {3, 0x06, 0x81B7}, {3, 0x06, 0x0285}, {3, 0x06, 0xB2D1}, {3, 0x06, 0x03BF}, \
    {3, 0x06, 0x2535}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAD1}, {3, 0x06, 0x00BF}, \
    {3, 0x06, 0x2538}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAD1}, {3, 0x06, 0x00BF}, \
    {3, 0x06, 0x253B}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAD1}, {3, 0x06, 0x08BF}, \
    {3, 0x06, 0x252C}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAD1}, {3, 0x06, 0x01BF}, \
    {3, 0x06, 0x252F}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAD1}, {3, 0x06, 0x01BF}, \
    {3, 0x06, 0x2532}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAE1}, {3, 0x06, 0x8B86}, \
    {3, 0x06, 0xAD2A}, {3, 0x06, 0x08D1}, {3, 0x06, 0x00BF}, {3, 0x06, 0x253E}, \
    {3, 0x06, 0x022A}, {3, 0x06, 0xFA04}, {3, 0x06, 0xE18B}, {3, 0x06, 0x86AD}, \
    {3, 0x06, 0x2A08}, {3, 0x06, 0xD101}, {3, 0x06, 0xBF25}, {3, 0x06, 0x3E02}, \
    {3, 0x06, 0x2AFA}, {3, 0x06, 0x04F8}, {3, 0x06, 0xE0E0}, {3, 0x06, 0x38E1}, \
    {3, 0x06, 0xE039}, {3, 0x06, 0xAC2E}, {3, 0x06, 0x05D0}, {3, 0x06, 0x0302}, \
    {3, 0x06, 0x20E7}, {3, 0x06, 0xFC04}, {3, 0x06, 0xF8FA}, {3, 0x06, 0xEF69}, \
    {3, 0x06, 0xE08B}, {3, 0x06, 0x87AD}, {3, 0x06, 0x2008}, {3, 0x06, 0xD101}, \
    {3, 0x06, 0xBF27}, {3, 0x06, 0xE302}, {3, 0x06, 0x2AFA}, {3, 0x06, 0xEF96}, \
    {3, 0x06, 0xFEFC}, {3, 0x06, 0x04F8}, {3, 0x06, 0xF9FA}, {3, 0x06, 0xEF69}, \
    {3, 0x06, 0xE08B}, {3, 0x06, 0x87AD}, {3, 0x06, 0x2061}, {3, 0x06, 0xD200}, \
    {3, 0x06, 0xBF27}, {3, 0x06, 0xDA02}, {3, 0x06, 0x2ACD}, {3, 0x06, 0x1E21}, \
    {3, 0x06, 0xBF22}, {3, 0x06, 0x5602}, {3, 0x06, 0x2ACD}, {3, 0x06, 0x0C11}, \
    {3, 0x06, 0x1E21}, {3, 0x06, 0xBF85}, {3, 0x06, 0xDF02}, {3, 0x06, 0x2ACD}, \
    {3, 0x06, 0x0C12}, {3, 0x06, 0x1E21}, {3, 0x06, 0xBF85}, {3, 0x06, 0xE202}, \
    {3, 0x06, 0x2ACD}, {3, 0x06, 0x0C13}, {3, 0x06, 0x1E21}, {3, 0x06, 0xBF85}, \
    {3, 0x06, 0xE502}, {3, 0x06, 0x2ACD}, {3, 0x06, 0x0C14}, {3, 0x06, 0x1E21}, \
    {3, 0x06, 0xE08A}, {3, 0x06, 0xE21F}, {3, 0x06, 0x029E}, {3, 0x06, 0x28E6}, \
    {3, 0x06, 0x8AE2}, {3, 0x06, 0xAD30}, {3, 0x06, 0x1CAC}, {3, 0x06, 0x3107}, \
    {3, 0x06, 0x5A30}, {3, 0x06, 0xA200}, {3, 0x06, 0x14AE}, {3, 0x06, 0x0AEF}, \
    {3, 0x06, 0x0258}, {3, 0x06, 0x0C9E}, {3, 0x06, 0x045A}, {3, 0x06, 0x309F}, \
    {3, 0x06, 0x08BF}, {3, 0x06, 0x85F1}, {3, 0x06, 0x022A}, {3, 0x06, 0x5CAE}, \
    {3, 0x06, 0x06BF}, {3, 0x06, 0x85FA}, {3, 0x06, 0x022A}, {3, 0x06, 0x5CEF}, \
    {3, 0x06, 0x96FE}, {3, 0x06, 0xFDFC}, {3, 0x06, 0x04F8}, {3, 0x06, 0xE08B}, \
    {3, 0x06, 0x85AD}, {3, 0x06, 0x2629}, {3, 0x06, 0xE0E0}, {3, 0x06, 0x36E1}, \
    {3, 0x06, 0xE037}, {3, 0x06, 0xE18B}, {3, 0x06, 0x631F}, {3, 0x06, 0x109E}, \
    {3, 0x06, 0x1CE4}, {3, 0x06, 0x8B63}, {3, 0x06, 0xAC20}, {3, 0x06, 0x08AC}, \
    {3, 0x06, 0x210B}, {3, 0x06, 0xAC27}, {3, 0x06, 0x0DAE}, {3, 0x06, 0x0EEE}, \
    {3, 0x06, 0x8B56}, {3, 0x06, 0x00AE}, {3, 0x06, 0x0802}, {3, 0x06, 0x8283}, \
    {3, 0x06, 0xAE03}, {3, 0x06, 0x0225}, {3, 0x06, 0x6AFC}, {3, 0x06, 0x04F8}, \
    {3, 0x06, 0xFAEF}, {3, 0x06, 0x6902}, {3, 0x06, 0x82A0}, {3, 0x06, 0xE0E0}, \
    {3, 0x06, 0x14E1}, {3, 0x06, 0xE015}, {3, 0x06, 0xAD26}, {3, 0x06, 0x08D1}, \
    {3, 0x06, 0x1EBF}, {3, 0x06, 0x25CD}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAEF}, \
    {3, 0x06, 0x96FE}, {3, 0x06, 0xFC04}, {3, 0x06, 0xF8F9}, {3, 0x06, 0xE08B}, \
    {3, 0x06, 0x85AD}, {3, 0x06, 0x2738}, {3, 0x06, 0xD00B}, {3, 0x06, 0x0228}, \
    {3, 0x06, 0xFF58}, {3, 0x06, 0x8278}, {3, 0x06, 0x829F}, {3, 0x06, 0x2DE0}, \
    {3, 0x06, 0x8B56}, {3, 0x06, 0xE18B}, {3, 0x06, 0x571F}, {3, 0x06, 0x109E}, \
    {3, 0x06, 0x2310}, {3, 0x06, 0xE48B}, {3, 0x06, 0x56E0}, {3, 0x06, 0xE000}, \
    {3, 0x06, 0xE1E0}, {3, 0x06, 0x01F7}, {3, 0x06, 0x27E4}, {3, 0x06, 0xE000}, \
    {3, 0x06, 0xE5E0}, {3, 0x06, 0x01E2}, {3, 0x06, 0xE020}, {3, 0x06, 0xE3E0}, \
    {3, 0x06, 0x21AD}, {3, 0x06, 0x30F7}, {3, 0x06, 0xF627}, {3, 0x06, 0xE4E0}, \
    {3, 0x06, 0x00E5}, {3, 0x06, 0xE001}, {3, 0x06, 0xFDFC}, {3, 0x06, 0x04F8}, \
    {3, 0x06, 0xE0E5}, {3, 0x06, 0x14E1}, {3, 0x06, 0xE515}, {3, 0x06, 0xAD27}, \
    {3, 0x06, 0x0EE0}, {3, 0x06, 0x8B01}, {3, 0x06, 0xAC27}, {3, 0x06, 0x08F7}, \
    {3, 0x06, 0x27E4}, {3, 0x06, 0x8B01}, {3, 0x06, 0x0284}, {3, 0x06, 0xBAFC}, \
    {3, 0x06, 0x04F8}, {3, 0x06, 0xE08B}, {3, 0x06, 0x01AD}, {3, 0x06, 0x2717}, \
    {3, 0x06, 0xAC23}, {3, 0x06, 0x14BF}, {3, 0x06, 0x1DE7}, {3, 0x06, 0x022A}, \
    {3, 0x06, 0xCDAD}, {3, 0x06, 0x280B}, {3, 0x06, 0xE08B}, {3, 0x06, 0x01F7}, \
    {3, 0x06, 0x23E4}, {3, 0x06, 0x8B01}, {3, 0x06, 0x0283}, {3, 0x06, 0x1DFC}, \
    {3, 0x06, 0x04F8}, {3, 0x06, 0xF9E0}, {3, 0x06, 0xE514}, {3, 0x06, 0xE1E5}, \
    {3, 0x06, 0x15E1}, {3, 0x06, 0x8B01}, {3, 0x06, 0xAC29}, {3, 0x06, 0x62AC}, \
    {3, 0x06, 0x2604}, {3, 0x06, 0xF62A}, {3, 0x06, 0xAE02}, {3, 0x06, 0xF72A}, \
    {3, 0x06, 0xE58B}, {3, 0x06, 0x01AD}, {3, 0x06, 0x2713}, {3, 0x06, 0xF728}, \
    {3, 0x06, 0xE58B}, {3, 0x06, 0x01E2}, {3, 0x06, 0xE518}, {3, 0x06, 0xE3E5}, \
    {3, 0x06, 0x19F6}, {3, 0x06, 0x36E6}, {3, 0x06, 0xE518}, {3, 0x06, 0xE7E5}, \
    {3, 0x06, 0x19AD}, {3, 0x06, 0x283D}, {3, 0x06, 0xF8BF}, {3, 0x06, 0x1CE7}, \
    {3, 0x06, 0x022A}, {3, 0x06, 0xCDE5}, {3, 0x06, 0x8B02}, {3, 0x06, 0xD101}, \
    {3, 0x06, 0xBF1C}, {3, 0x06, 0xE702}, {3, 0x06, 0x2AFA}, {3, 0x06, 0xFCE2}, \
    {3, 0x06, 0xE0C0}, {3, 0x06, 0xE3E0}, {3, 0x06, 0xC1AD}, {3, 0x06, 0x3708}, \
    {3, 0x06, 0x5A7F}, {3, 0x06, 0xE6E0}, {3, 0x06, 0xC0E7}, {3, 0x06, 0xE0C1}, \
    {3, 0x06, 0xEF01}, {3, 0x06, 0x5830}, {3, 0x06, 0x5BCC}, {3, 0x06, 0x1E30}, \
    {3, 0x06, 0x0D04}, {3, 0x06, 0x1E30}, {3, 0x06, 0x6A80}, {3, 0x06, 0xE6E0}, \
    {3, 0x06, 0xC0E7}, {3, 0x06, 0xE0C1}, {3, 0x06, 0xF729}, {3, 0x06, 0xE58B}, \
    {3, 0x06, 0x01FD}, {3, 0x06, 0xFC04}, {3, 0x06, 0xF8F9}, {3, 0x06, 0xFAFB}, \
    {3, 0x06, 0xE0E0}, {3, 0x06, 0xFAE1}, {3, 0x06, 0xE0FB}, {3, 0x06, 0xAD24}, \
    {3, 0x06, 0x18E2}, {3, 0x06, 0x8B01}, {3, 0x06, 0xAD31}, {3, 0x06, 0x12F8}, \
    {3, 0x06, 0xE18B}, {3, 0x06, 0x02BF}, {3, 0x06, 0x1CE7}, {3, 0x06, 0x022A}, \
    {3, 0x06, 0xFAFC}, {3, 0x06, 0x580F}, {3, 0x06, 0xAD34}, {3, 0x06, 0x15AE}, \
    {3, 0x06, 0x03AF}, {3, 0x06, 0x84B5}, {3, 0x06, 0xEF30}, {3, 0x06, 0x5B03}, \
    {3, 0x06, 0xAD23}, {3, 0x06, 0x026B}, {3, 0x06, 0x04AD}, {3, 0x06, 0x2202}, \
    {3, 0x06, 0x6B08}, {3, 0x06, 0xEF03}, {3, 0x06, 0xD300}, {3, 0x06, 0xEF10}, \
    {3, 0x06, 0x590E}, {3, 0x06, 0xA108}, {3, 0x06, 0x04D3}, {3, 0x06, 0x02AE}, \
    {3, 0x06, 0x2DEF}, {3, 0x06, 0x1059}, {3, 0x06, 0x0EA1}, {3, 0x06, 0x0404}, \
    {3, 0x06, 0xD302}, {3, 0x06, 0xAE22}, {3, 0x06, 0xEF10}, {3, 0x06, 0x590E}, \
    {3, 0x06, 0xA106}, {3, 0x06, 0x04D3}, {3, 0x06, 0x04AE}, {3, 0x06, 0x17A0}, \
    {3, 0x06, 0x0A04}, {3, 0x06, 0xD308}, {3, 0x06, 0xAE10}, {3, 0x06, 0xA00B}, \
    {3, 0x06, 0x04D3}, {3, 0x06, 0x01AE}, {3, 0x06, 0x09EF}, {3, 0x06, 0x1059}, \
    {3, 0x06, 0x0CA1}, {3, 0x06, 0x0C02}, {3, 0x06, 0xD308}, {3, 0x06, 0xEF02}, \
    {3, 0x06, 0x0D04}, {3, 0x06, 0x5803}, {3, 0x06, 0xD600}, {3, 0x06, 0x00D7}, \
    {3, 0x06, 0x1111}, {3, 0x06, 0xAD3A}, {3, 0x06, 0x0516}, {3, 0x06, 0x0C6C}, \
    {3, 0x06, 0xAE14}, {3, 0x06, 0xAD3B}, {3, 0x06, 0x0516}, {3, 0x06, 0x0C68}, \
    {3, 0x06, 0xAE0C}, {3, 0x06, 0xAD39}, {3, 0x06, 0x0516}, {3, 0x06, 0x0C64}, \
    {3, 0x06, 0xAE04}, {3, 0x06, 0xAD38}, {3, 0x06, 0x0116}, {3, 0x06, 0xA000}, \
    {3, 0x06, 0x02AE}, {3, 0x06, 0x060C}, {3, 0x06, 0x610C}, {3, 0x06, 0x71B0}, \
    {3, 0x06, 0xFAE0}, {3, 0x06, 0xE516}, {3, 0x06, 0xE1E5}, {3, 0x06, 0x177F}, \
    {3, 0x06, 0xFFFF}, {3, 0x06, 0x1D47}, {3, 0x06, 0x1E46}, {3, 0x06, 0xE4E5}, \
    {3, 0x06, 0x16E5}, {3, 0x06, 0xE517}, {3, 0x06, 0xEF02}, {3, 0x06, 0x0D04}, \
    {3, 0x06, 0x5803}, {3, 0x06, 0xBFE5}, {3, 0x06, 0x180C}, {3, 0x06, 0x011A}, \
    {3, 0x06, 0x90E0}, {3, 0x06, 0xE0F8}, {3, 0x06, 0xE1E0}, {3, 0x06, 0xF9EF}, \
    {3, 0x06, 0x645E}, {3, 0x06, 0x1FFF}, {3, 0x06, 0xD819}, {3, 0x06, 0xD989}, \
    {3, 0x06, 0x5CE0}, {3, 0x06, 0x001E}, {3, 0x06, 0x46DC}, {3, 0x06, 0x19DD}, \
    {3, 0x06, 0x2A10}, {3, 0x06, 0xF631}, {3, 0x06, 0xEF02}, {3, 0x06, 0x5824}, \
    {3, 0x06, 0x3824}, {3, 0x06, 0x9E03}, {3, 0x06, 0xAD36}, {3, 0x06, 0x34AD}, \
    {3, 0x06, 0x3210}, {3, 0x06, 0xE0E5}, {3, 0x06, 0x16E1}, {3, 0x06, 0xE517}, \
    {3, 0x06, 0x5833}, {3, 0x06, 0x5933}, {3, 0x06, 0xE4E5}, {3, 0x06, 0x16E5}, \
    {3, 0x06, 0xE517}, {3, 0x06, 0xD200}, {3, 0x06, 0xE0E5}, {3, 0x06, 0x14E1}, \
    {3, 0x06, 0xE515}, {3, 0x06, 0xF627}, {3, 0x06, 0xE4E5}, {3, 0x06, 0x14E5}, \
    {3, 0x06, 0xE515}, {3, 0x06, 0xE0E5}, {3, 0x06, 0x18E1}, {3, 0x06, 0xE519}, \
    {3, 0x06, 0xF726}, {3, 0x06, 0xE4E5}, {3, 0x06, 0x18E5}, {3, 0x06, 0xE519}, \
    {3, 0x06, 0x0284}, {3, 0x06, 0xE1E6}, {3, 0x06, 0x8B01}, {3, 0x06, 0x0283}, \
    {3, 0x06, 0x1DFF}, {3, 0x06, 0xFEFD}, {3, 0x06, 0xFC04}, {3, 0x06, 0xF8E0}, \
    {3, 0x06, 0x8B64}, {3, 0x06, 0xF723}, {3, 0x06, 0xE48B}, {3, 0x06, 0x64D1}, \
    {3, 0x06, 0x01BF}, {3, 0x06, 0x36CB}, {3, 0x06, 0x022A}, {3, 0x06, 0xFAE0}, \
    {3, 0x06, 0xE2F4}, {3, 0x06, 0xE1E2}, {3, 0x06, 0xF5E4}, {3, 0x06, 0x8B58}, \
    {3, 0x06, 0xE58B}, {3, 0x06, 0x59EE}, {3, 0x06, 0xE2F4}, {3, 0x06, 0xD8EE}, \
    {3, 0x06, 0xE2F5}, {3, 0x06, 0x20FC}, {3, 0x06, 0x04F8}, {3, 0x06, 0xE08B}, \
    {3, 0x06, 0x58E1}, {3, 0x06, 0x8B59}, {3, 0x06, 0xE4E2}, {3, 0x06, 0xF4E5}, \
    {3, 0x06, 0xE2F5}, {3, 0x06, 0xE08B}, {3, 0x06, 0x64F6}, {3, 0x06, 0x23E4}, \
    {3, 0x06, 0x8B64}, {3, 0x06, 0xFC04}, {3, 0x06, 0xF8F9}, {3, 0x06, 0xFAEF}, \
    {3, 0x06, 0x69AC}, {3, 0x06, 0x1B5E}, {3, 0x06, 0xE0E0}, {3, 0x06, 0x12E1}, \
    {3, 0x06, 0xE013}, {3, 0x06, 0xBF26}, {3, 0x06, 0xBC02}, {3, 0x06, 0x2ACD}, \
    {3, 0x06, 0xEF01}, {3, 0x06, 0xE28A}, {3, 0x06, 0xCAE4}, {3, 0x06, 0x8ACA}, \
    {3, 0x06, 0x1F12}, {3, 0x06, 0x9E46}, {3, 0x06, 0xEF12}, {3, 0x06, 0x5907}, \
    {3, 0x06, 0x9F1A}, {3, 0x06, 0xF8E0}, {3, 0x06, 0x8B64}, {3, 0x06, 0xF721}, \
    {3, 0x06, 0xE48B}, {3, 0x06, 0x64D0}, {3, 0x06, 0x0302}, {3, 0x06, 0x20E7}, \
    {3, 0x06, 0x0226}, {3, 0x06, 0x96D1}, {3, 0x06, 0x00BF}, {3, 0x06, 0x27E3}, \
    {3, 0x06, 0x022A}, {3, 0x06, 0xFAFC}, {3, 0x06, 0xA000}, {3, 0x06, 0x1402}, \
    {3, 0x06, 0x2645}, {3, 0x06, 0x0226}, {3, 0x06, 0xADEE}, {3, 0x06, 0x8AE2}, \
    {3, 0x06, 0xFFE0}, {3, 0x06, 0x8B64}, {3, 0x06, 0xF621}, {3, 0x06, 0xE48B}, \
    {3, 0x06, 0x64AE}, {3, 0x06, 0x0FBF}, {3, 0x06, 0x36C5}, {3, 0x06, 0x022A}, \
    {3, 0x06, 0x5CBF}, {3, 0x06, 0x36C2}, {3, 0x06, 0x022A}, {3, 0x06, 0x5C02}, \
    {3, 0x06, 0x264E}, {3, 0x06, 0xEF96}, {3, 0x06, 0xFEFD}, {3, 0x06, 0xFC04}, \
    {3, 0x06, 0xF8FA}, {3, 0x06, 0xEF69}, {3, 0x06, 0xE08B}, {3, 0x06, 0x87AD}, \
    {3, 0x06, 0x2118}, {3, 0x06, 0xD000}, {3, 0x06, 0x0229}, {3, 0x06, 0x78AD}, \
    {3, 0x06, 0x2F10}, {3, 0x06, 0xD101}, {3, 0x06, 0xBF85}, {3, 0x06, 0xEE02}, \
    {3, 0x06, 0x2AFA}, {3, 0x06, 0xD101}, {3, 0x06, 0xBF85}, {3, 0x06, 0xE802}, \
    {3, 0x06, 0x2AFA}, {3, 0x06, 0xEF96}, {3, 0x06, 0xFEFC}, {3, 0x06, 0x04F8}, \
    {3, 0x06, 0xFAEF}, {3, 0x06, 0x69E0}, {3, 0x06, 0x8B87}, {3, 0x06, 0xAD21}, \
    {3, 0x06, 0x18D0}, {3, 0x06, 0x0002}, {3, 0x06, 0x2978}, {3, 0x06, 0xAD2E}, \
    {3, 0x06, 0x10D1}, {3, 0x06, 0x01BF}, {3, 0x06, 0x85EB}, {3, 0x06, 0x022A}, \
    {3, 0x06, 0xFAD1}, {3, 0x06, 0x01BF}, {3, 0x06, 0x85E8}, {3, 0x06, 0x022A}, \
    {3, 0x06, 0xFAEF}, {3, 0x06, 0x96FE}, {3, 0x06, 0xFC04}, {3, 0x06, 0xF8FA}, \
    {3, 0x06, 0xEF69}, {3, 0x06, 0xE08B}, {3, 0x06, 0x87AD}, {3, 0x06, 0x2118}, \
    {3, 0x06, 0xD100}, {3, 0x06, 0xBF85}, {3, 0x06, 0xE802}, {3, 0x06, 0x2AFA}, \
    {3, 0x06, 0xD100}, {3, 0x06, 0xBF85}, {3, 0x06, 0xEB02}, {3, 0x06, 0x2AFA}, \
    {3, 0x06, 0xD100}, {3, 0x06, 0xBF85}, {3, 0x06, 0xEE02}, {3, 0x06, 0x2AFA}, \
    {3, 0x06, 0xEF96}, {3, 0x06, 0xFEFC}, {3, 0x06, 0x0400}, {3, 0x06, 0xE140}, \
    {3, 0x06, 0x77E1}, {3, 0x06, 0x4099}, {3, 0x06, 0xE036}, {3, 0x06, 0x88E0}, \
    {3, 0x06, 0x36FE}, {3, 0x06, 0xE022}, {3, 0x06, 0x99E2}, {3, 0x06, 0x00DD}, \
    {3, 0x06, 0xE200}, {3, 0x06, 0xEEE2}, {3, 0x06, 0x00E0}, {3, 0x06, 0x4B10}, \
    {3, 0x06, 0xE04B}, {3, 0x06, 0x23E1}, {3, 0x06, 0x006C}, {3, 0x06, 0xE100}, \
    {3, 0x06, 0x1CE0}, {3, 0x06, 0x4B13}, {3, 0x06, 0xE04B}, {3, 0x06, 0x6000}, \
    {3, 0x05, 0xE142}, {3, 0x06, 0x0201}, {3, 0x05, 0xE140}, {3, 0x06, 0x0005}, \
    {3, 0x0f, 0x0000}, {3, 0x1f, 0x0008}, \
    /* Unlock Naro-C */ \
    {3, 0x1f, 0x0007}, {3, 0x1e, 0x0023}, {3, 0x17, 0x1910}, {3, 0x1f, 0x0008}, \
    /* Micro-C Modify Radom Seed Cause Not Link */ \
    {3, 0x1f, 0x0005}, {3, 0x05, 0x8B85}, {3, 0x06, 0xE286}, {3, 0x1f, 0x0008}, \
    /* Enable EEE 100/1000M */ \
    {3, 0x1f, 0x0007}, {3, 0x1e, 0x0020}, {3, 0x15, 0x0000}, {3, 0x1b, 0xA0BA}, \
    {3, 0x1f, 0x0008}, \
    /* Enable Down Speed */ \
    {3, 0x1f, 0x0007}, {3, 0x1e, 0x002D}, {3, 0x18, 0xF010}, {3, 0x1f, 0x0008}, \
    /* Disable the Solution of Force 10M to 100M Issue */ \
    {3, 0x1f, 0x0007}, {3, 0x1e, 0x002D}, {3, 0x16, 0x0024}, {3, 0x1f, 0x0008}, \
    /* Modify Analog PHY Parameter */ \
    {3, 0x1f, 0x0002}, {3, 0x08, 0x3602}, {3, 0x12, 0x00DD}, {3, 0x0c, 0x5C25}, \
    {3, 0x1f, 0x0008}, \
    /* Modify EEE Unformatted Page2 the Same as Page1 */ \
    {3, 0x1f, 0x0000}, {3, 0x0d, 0x0003}, {3, 0x0e, 0x0015}, {3, 0x0d, 0x4003}, \
    {3, 0x0e, 0x0006}, {3, 0x1f, 0x0008}, \
    /* Micro-C Select Calibration Mode */ \
    {3, 0x1f, 0x0005}, {3, 0x05, 0x8B82}, {3, 0x06, 0x05EB}, {3, 0x1f, 0x0008}, \
    /* Restart Auto-Negotiation */ \
    {3, 0x1f, 0x0008}, {3, 0x00, 0x1340}, {3, 0x1f, 0x0008}, \
    /* Disable Parallel Write */ \
    {3, 0x1f, 0x0008}, {3, 0x18, 0x0000}, {3, 0x1f, 0x0008}, \
    /* Auto Mode Standard Register */ \
    {1, 0x1f, 0x0008}, {1, 0x10, 0x0000}, {1, 0x1f, 0x0008}, \
    /* Reset Ser-Des */ \
    {3, 0x1f, 0x0008}, {3, 0x1c, 0xFF00}, {3, 0x1c, 0x9000}, {3, 0x1c, 0x0000}, \
    {3, 0x1f, 0x0008}, \
};
#endif

typedef struct hal_phy_info_s
{
    uint8   auto_1000f[RTK_MAX_NUM_OF_PORTS]; /* copper, reg[9].bit[9] shadow for patch down-speed mechanism */
    uint8   ldps_state[RTK_MAX_NUM_OF_PORTS]; /* copper, reg[21].bit[12] shadow for link-down power saving state */
} hal_phy_info_t;

static hal_phy_info_t   *pPhy_info[RTK_MAX_NUM_OF_UNIT];
static uint32           phyInfo_alloc[RTK_MAX_NUM_OF_UNIT];

/*
 * Data Declaration
 */
rt_phydrv_t phy_8214FBdrv_ge =
{
    RT_PHYDRV_RTL8214FB,
    phy_8214fb_init,
    phy_8214fb_media_get,
    phy_8214fb_media_set,
    phy_8214fb_autoNegoEnable_get,
    phy_8214fb_autoNegoEnable_set,
    phy_8214fb_autoNegoAbility_get,
    phy_8214fb_autoNegoAbility_set,
    phy_8214fb_duplex_get,
    phy_8214fb_duplex_set,
    phy_8214fb_speed_get,
    phy_8214fb_speed_set,
    phy_8214fb_enable_set,
    phy_8214fb_rtctResult_get,
    phy_8214fb_rtct_start,
    phy_8214fb_greenEnable_get,
    phy_8214fb_greenEnable_set,
    phy_8214fb_eeeEnable_get,
    phy_8214fb_eeeEnable_set,
    phy_8214fb_crossOverMode_get,
    phy_8214fb_crossOverMode_set,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,       
    phy_8214fb_fiber_media_get,
    phy_8214fb_fiber_media_set,
    phy_8214fb_linkDownPowerSavingEnable_get,
    phy_8214fb_linkDownPowerSavingEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8214fb_gigaLiteEnable_get,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8214fb_downSpeedEnable_get,
    phy_8214fb_downSpeedEnable_set,   
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8214fb_fiberOAMLoopBack_set,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8214FBdrv_ge */

rt_phydrv_t phy_8214Bdrv_ge =
{
    RT_PHYDRV_RTL8214B,
    phy_8214fb_init,
    phy_8214fb_media_get,
    phy_8214fb_media_set,
    phy_8214fb_autoNegoEnable_get,
    phy_8214fb_autoNegoEnable_set,
    phy_8214fb_autoNegoAbility_get,
    phy_8214fb_autoNegoAbility_set,
    phy_8214fb_duplex_get,
    phy_8214fb_duplex_set,
    phy_8214fb_speed_get,
    phy_8214fb_speed_set,
    phy_8214fb_enable_set,
    phy_8214fb_rtctResult_get,
    phy_8214fb_rtct_start,
    phy_8214fb_greenEnable_get,
    phy_8214fb_greenEnable_set,
    phy_8214fb_eeeEnable_get,
    phy_8214fb_eeeEnable_set,
    phy_8214fb_crossOverMode_get,
    phy_8214fb_crossOverMode_set,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,       
    phy_8214fb_fiber_media_get,
    phy_8214fb_fiber_media_set,
    phy_8214fb_linkDownPowerSavingEnable_get,
    phy_8214fb_linkDownPowerSavingEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8214fb_gigaLiteEnable_get,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8214fb_downSpeedEnable_get,
    phy_8214fb_downSpeedEnable_set,   
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8214fb_fiberOAMLoopBack_set,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8214Bdrv_ge */

rt_phydrv_t phy_8212Bdrv_ge =
{
    RT_PHYDRV_RTL8212B,
    phy_8214fb_init,
    phy_8214fb_media_get,
    phy_8214fb_media_set,
    phy_8214fb_autoNegoEnable_get,
    phy_8214fb_autoNegoEnable_set,
    phy_8214fb_autoNegoAbility_get,
    phy_8214fb_autoNegoAbility_set,
    phy_8214fb_duplex_get,
    phy_8214fb_duplex_set,
    phy_8214fb_speed_get,
    phy_8214fb_speed_set,
    phy_8214fb_enable_set,
    phy_8214fb_rtctResult_get,
    phy_8214fb_rtct_start,
    phy_8214fb_greenEnable_get,
    phy_8214fb_greenEnable_set,
    phy_8214fb_eeeEnable_get,
    phy_8214fb_eeeEnable_set,
    phy_8214fb_crossOverMode_get,
    phy_8214fb_crossOverMode_set,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,       
    phy_8214fb_fiber_media_get,
    phy_8214fb_fiber_media_set,
    phy_8214fb_linkDownPowerSavingEnable_get,
    phy_8214fb_linkDownPowerSavingEnable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8214fb_gigaLiteEnable_get,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    phy_8214fb_downSpeedEnable_get,
    phy_8214fb_downSpeedEnable_set,       
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8214fb_fiberOAMLoopBack_set,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_mac_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32, rtk_time_timeStamp_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_time_ptpIdentifier_t, rtk_time_timeStamp_t *))phy_common_unavail,
    phy_common_masterSlave_get,
    phy_common_masterSlave_set,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8212Bdrv_ge */

#define PHY_MEDIA_LINKDOWN          (0)
#define PHY_MEDIA_LINKUP            (1)

/*
 * Function Declaration
 */
/* Function Name:
 *      phy_8214fb_media_get
 * Description:
 *      Get PHY 8212B/8214FB/8214B media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 */
int32
phy_8214fb_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    uint32  val;
    uint32  base_port = 0;
    int32   ret = RT_ERR_FAILED;

    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    base_port = port - (port % PORT_NUM_IN_8214FB);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
        return ret;

    if (0x2000 == (val & 0x2000))
    {
        if (0x1000 == (val & 0x1000))
            *pMedia = PORT_MEDIA_FIBER;
        else
            *pMedia = PORT_MEDIA_COPPER;
    }
    else
    {
        if (0x0020 == (val & 0x0020))
            *pMedia = PORT_MEDIA_COPPER_AUTO;
        else
            *pMedia = PORT_MEDIA_FIBER_AUTO;
    }

    return RT_ERR_OK;
} /* end of phy_8214fb_media_get */

/* Function Name:
 *      phy_8214fb_media_set
 * Description:
 *      Get PHY 8212B/8214FB/8214B media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      1. media type is PORT_MEDIA_COPPER or PORT_MEDIA_FIBER
 *      2. there are 3 major steps for change the media from original to new
 *         - turn off original media
 *         - turn on new media
 *         - set new media to first priority
 *      3. PORT_MEDIA_COPPER_AUTO & PORT_MEDIA_FIBER_AUTO return RT_ERR_CHIP_NOT_SUPPORTED.
 */
int32
phy_8214fb_media_set(uint32 unit, rtk_port_t port, rtk_port_media_t media)
{
    uint32  val, phyReg0, is_phyDown;
    uint32  base_port = 0;
    int32   ret = RT_ERR_FAILED;

    base_port = port - (port % PORT_NUM_IN_8214FB);

    switch (media)
    {
        case PORT_MEDIA_COPPER:
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 0))
            {
                /* no change media */
                break;
            }
            /* If the original media is PORT_MEDIA_FIBER, need to apply
             * reg0.bit11 power down state from fiber to copper
             */
            if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 1))
            {
                if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 0, &phyReg0)) != RT_ERR_OK)
                    return ret;
                is_phyDown = (phyReg0 >> 11) & 0x1;

                /* turn-on copper */
                val &= ~(1<<12); /* select UTP */
                val |= (0x2000); /* disable auto-sensing */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 0, &phyReg0)) != RT_ERR_OK)
                    return ret;
                phyReg0 &= 0xF7FF;
                phyReg0 |= (is_phyDown << 11);
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 0, phyReg0)) != RT_ERR_OK)
                    return ret;
            }
            else
            {
                /* turn-on copper */
                val &= ~(1<<12); /* select UTP */
                val |= (0x2000); /* disable auto-sensing */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                    return ret;
            }
            /* Force TX 2.5G */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 25, 0x3749)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                return ret;
            /* force power */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 21, 0xC129)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                return ret;

            break; /* end of switch-case PORT_MEDIA_COPPER */

        case PORT_MEDIA_FIBER:
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 1))
            {
                /* no change media */
                 break;
            }
            /* If the original media is PORT_MEDIA_COPPER, need to apply
             * reg0.bit11 power down state from fiber to copper
             */
            if ((((val >> 13) & 0x1) == 1) && (((val >> 12) & 0x1) == 0))
            {
                if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 0, &phyReg0)) != RT_ERR_OK)
                    return ret;
                is_phyDown = (phyReg0 >> 11) & 0x1;

                /* turn-on fiber */
                val |= (0x3000); /* disable auto-sensing + select FIBER */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 0, &phyReg0)) != RT_ERR_OK)
                    return ret;
                phyReg0 &= 0xF7FF;
                phyReg0 |= (is_phyDown << 11);
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 0, phyReg0)) != RT_ERR_OK)
                    return ret;
                if (is_phyDown)
                {
                    /* Force TX 2.5G */
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 25, 0x374D)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                        return ret;
                    /* force power */
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 21, 0xC129)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                        return ret;
                }
                else
                {
                    /* Default setting */
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 25, 0x074D)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                        return ret;
                    /* Default setting */
                    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 21, 0xC178)) != RT_ERR_OK)
                        return ret;
                    if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                        return ret;
                }
            }
            else
            {
                /* turn-on fiber */
                val |= (0x3000); /* disable auto-sensing + select FIBER */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                    return ret;
                /* Default setting */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 25, 0x074D)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                    return ret;
                /* Default setting */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 21, 0xC178)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                    return ret;
            }
            break; /* end of switch-case PORT_MEDIA_FIBER */

        case PORT_MEDIA_COPPER_AUTO:
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            if ((((val >> 13) & 0x1) == 0) && (((val >> 5) & 0x1) == 1))
            {
                /* no change media */
                break;
            }
            /* turn-on copper */
            val &= ~(1<<13); /* enable auto-sensing */
            val |= (1<<5); /* preferred copper */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            /* Default setting */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 25, 0x074D)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                return ret;
            /* Default setting */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 21, 0xC178)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_MEDIA_COPPER_AUTO */

        case PORT_MEDIA_FIBER_AUTO:
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            if ((((val >> 13) & 0x1) == 0) && (((val >> 5) & 0x1) == 0))
            {
                /* no change media */
                 break;
            }
            /* turn-on fiber */
            val &= ~(1<<13); /* enable auto-sensing */
            val &= ~(1<<5); /* preferred fiber */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            /* Default setting */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 25, 0x074D)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                return ret;
            /* Default setting */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 21, 0xC178)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_MEDIA_FIBER_AUTO */

        default:
            break;
    }

    return RT_ERR_OK;
} /* end of phy_8214fb_media_set */

/* Function Name:
 *      _phy_8214fb_intMedia_get
 * Description:
 *      Get 8214fb internal media
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      media         - internal media
 *      link_status  - media link status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
_phy_8214fb_intMedia_get(uint32 unit, rtk_port_t port, rtk_port_media_t *media, uint32 *link_status)
{
    uint32              phyData1;
    int32               ret;
    uint8               is_fiber_linkup = 0, is_copper_linkup = 0;

    /* get media */
    if ((ret = phy_media_get(unit, port, media)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData1)) != RT_ERR_OK)
        goto ERR;
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData1)) != RT_ERR_OK)
        goto ERR;

    if (phyData1 & LinkStatus_MASK)
    {
        *link_status = PHY_MEDIA_LINKUP;
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_EXTENDED_STATUS_REG, &phyData1)) != RT_ERR_OK)
            goto ERR;
        if((phyData1 & _1000BaseX_FullDuplex_R15_MASK) || (phyData1 & _1000BaseX_HalfDuplex_R15_MASK))
        {
            is_fiber_linkup = PHY_MEDIA_LINKUP; /*Link up media is Fiber*/
        }
        if((phyData1 & _1000Base_TFullDuplex_R15_MASK) || (phyData1 & _1000Base_THalfDuplex_R15_MASK))
        {
            is_copper_linkup = PHY_MEDIA_LINKUP; /*Link up media is Copper*/
        }

    }else{
        is_fiber_linkup = PHY_MEDIA_LINKDOWN;
        is_copper_linkup = PHY_MEDIA_LINKDOWN;
        *link_status = PHY_MEDIA_LINKDOWN;
    }

    if (PORT_MEDIA_FIBER == *media || PORT_MEDIA_COPPER == *media)
        return RT_ERR_OK;

    /* nego media */
    if (is_copper_linkup == is_fiber_linkup)
    {
        if (PORT_MEDIA_FIBER_AUTO == *media)
            *media = PORT_MEDIA_FIBER;
        else if (PORT_MEDIA_COPPER_AUTO == *media)
            *media = PORT_MEDIA_COPPER;
    }
    else if (is_fiber_linkup)
        *media = PORT_MEDIA_FIBER;
    else
        *media = PORT_MEDIA_COPPER;

    return RT_ERR_OK;
ERR:
    return ret;
}   /* end of _phy_8214fb_intMedia_get */

/* Function Name:
 *      phy_8214fb_utpDownSpeedEnable_get
 * Description:
 *      Get UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of UTP down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8214fb_utpDownSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    uint32  phyData;
    int32   ret;
	
    if ((ret = hal_miim_write(unit, port, 0, 31, 7)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_write(unit, port, 7, 30, 0x2d)) != RT_ERR_OK)
        return ret;
	
    if ((ret = hal_miim_read(unit, port, 7, 24, &phyData)) != RT_ERR_OK)
        goto ERR;

    if (phyData & (1 << 4))
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    if ((ret = hal_miim_write(unit, port, 0, 31, 0)) != RT_ERR_OK)
	    return ret;
	
    ERR:
        hal_miim_write(unit, port, 0, 31, 0);

    return ret;
}

/* Function Name:
 *      phy_8214fb_utpDownSpeedEnable_set
 * Description:
 *      Set UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of UTP down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8214fb_utpDownSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret;

    if ((ret = hal_miim_write(unit, port, 0, 31, 7)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_write(unit, port, 7, 30, 0x2d)) != RT_ERR_OK)
        return ret;
	
    if ((ret = hal_miim_read(unit, port, 7, 24, &phyData)) != RT_ERR_OK)
        goto ERR;

	/* DownSpeed to 100M*/
	phyData &= ~(1 << 4);
	
    if (ENABLED == enable)
        phyData |= (1 << 4);
    else
        phyData &= ~(1 << 4);
	
    if ((ret = hal_miim_write(unit, port, 7, 24, phyData)) != RT_ERR_OK)
        goto ERR;

    if ((ret = hal_miim_write(unit, port, 0, 31, 0)) != RT_ERR_OK)
        return ret;

    return ret;
ERR:
    hal_miim_write(unit, port, 0, 31, 0);
    return ret;
}

/* Function Name:
 *      phy_8214fb_downSpeedEnable_get
 * Description:
 *      Get UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of down speed
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8214fb_downSpeedEnable_get(uint32 unit, rtk_port_t port,
    rtk_enable_t *pEnable)
{
    int32   ret;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fb_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if(PORT_MEDIA_FIBER == media)
    {
        ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }else{
        ret = phy_8214fb_utpDownSpeedEnable_get(unit, port, pEnable);
    }
    return ret;
}

/* Function Name:
 *      phy_8214fb_downSpeedEnable_set
 * Description:
 *      Set UTP down speed 1000M --> 100M status of the specific port
 * Input:
 *      unit    - unit id
 *      port    - port id
 *      enable  - status of down speed
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      None
 */
int32
phy_8214fb_downSpeedEnable_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    int32   ret;
    rtk_port_media_t    media;
    uint32              link_sts;

    if ((ret = _phy_8214fb_intMedia_get(unit, port, &media, &link_sts)) != RT_ERR_OK)
        return ret;

    if(PORT_MEDIA_FIBER == media)
    {
        ret = RT_ERR_CHIP_NOT_SUPPORTED;
    }else{
        ret = phy_8214fb_utpDownSpeedEnable_set(unit, port, enable);
    }
    return ret;
}


/* Function Name:
 *      phy_8214fb_init
 * Description:
 *      Initialize PHY 8212B/8214B/8214FB.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_init(uint32 unit, rtk_port_t port)
{
    uint32  val;
    uint32  base_port = 0;
    uint32  org_media;
    uint32  def_port_state = RTK_DEFAULT_PORT_ADMIN_ENABLE; /* enable(1), disable(0) */
    int32   ret = RT_ERR_FAILED;
    uint32  serdes_link, retry_serdes_count = 0;
    uint32  version, is_fb, is_12b = 0;

    base_port = port - (port % PORT_NUM_IN_8214FB);

    /* Initialize the PHY software shadow */
    if (phyInfo_alloc[unit] == 0)
    {
        pPhy_info[unit] = (hal_phy_info_t *)osal_alloc(sizeof(hal_phy_info_t));
        if (NULL == pPhy_info[unit])
        {
            RT_ERR(RT_ERR_FAILED, (MOD_HAL), "memory allocate failed");
            return RT_ERR_FAILED;
        }
        osal_memset(pPhy_info[unit], 0, sizeof(hal_phy_info_t));
        phyInfo_alloc[unit] = 1;
    }


    /* Get the currently media */
    if ((ret = phy_8214fb_media_get(unit, port, &org_media)) != RT_ERR_OK)
        return ret;

#if 0 /* U-Boot have do it, not need to do again */
    if (port == base_port)
    {
        /* Disable all ports in the chip before starting do patch */
        for (i = base_port; i < base_port+PORT_NUM_IN_8214FB; i++)
        {
            if ((ret = hal_miim_read(unit, i, 0, 0, &val)) != RT_ERR_OK)
            {
                RT_LOG(LOG_DEBUG, MOD_HAL, "hal_miim_read(unit %d, port %d) page 0 reg 0 fail!!", unit, port);
                return ret;
            }
            if ((ret = hal_miim_write(unit, i, 0, 0, (val | 0x0800))) != RT_ERR_OK)
            {
                RT_LOG(LOG_DEBUG, MOD_HAL, "hal_miim_read(unit %d, port %d) power down PHY fail!!", unit, port);
                return ret;
            }
        }

        /* Configuration per-chip based */
        conf_size = sizeof(rtl8214fb_perchip)/sizeof(phy_8214fb_confcode_global_t);
        for (i = 0; i < conf_size; i++)
        {
            if ((ret = hal_miim_write(unit, base_port+rtl8214fb_perchip[i].phy_id, fixed_page, rtl8214fb_perchip[i].reg_no, rtl8214fb_perchip[i].reg_val)) != RT_ERR_OK)
            {
                RT_DBG(LOG_DEBUG, MOD_HAL, "hal_miim_write(unit %d, port %d) conf[%d]: reg %d value 0x%x!!", unit, base_port+rtl8214fb_perchip[i].phy_id, i, rtl8214fb_perchip[i].reg_no, rtl8214fb_perchip[i].reg_val);
                return ret;
            }
        }
    }
#endif

    /* Disable the auto senseing and force the media to fiber */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
        return ret;
    val |= 0x2000;
    val |= 0x1000;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
        return ret;

    /* Patch some 8328 MAC auto config phy register back to chip default
     * PHY X, page 0, reg 4, value 0x01A0 (for fiber-1000)
     * PHY X, page 0, reg 9, value 0x0000 (for fiber-1000)
     */
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 4, 0x01A0)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 9, 0x0000)) != RT_ERR_OK)
        return ret;

    /* Set the default port state */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
        return ret;
    if (def_port_state)
        val &= ~(0x1 << 11);
    else
        val |= (0x1 << 11);
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, val)) != RT_ERR_OK)
        return ret;

    /* Disable the auto senseing and force the media to copper */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
        return ret;
    val |= 0x2000;
    val &= ~(1<<12);
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
        return ret;

    /* Read the chip copper Auto-1000F ability value and initial the shadow value */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 9, &val)) != RT_ERR_OK)
        return ret;
    /* store to shadow */
    (*pPhy_info[unit]).auto_1000f[port] = (val >> _1000Base_TFullDuplex_OFFSET) & 0x1;

    /* Read the chip copper LDPS state value and initial the shadow value */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 21, &val)) != RT_ERR_OK)
        return ret;
    /* store ldps state to shadow */
    (*pPhy_info[unit]).ldps_state[port] = (val >> 12) & 0x1;

    /* Set the default port state */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &val)) != RT_ERR_OK)
        return ret;
    if (def_port_state)
        val &= ~(0x1 << 11);
    else
        val |= (0x1 << 11);
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, val)) != RT_ERR_OK)
        return ret;

    /* Restore the original media */
    if ((ret = phy_8214fb_media_set(unit, port, org_media)) != RT_ERR_OK)
        return ret;

    /* Configure option when 1000-X Nway is failure, try force mode */
    if ((ret = hal_miim_write(unit, port, 8, 31, 0x000F)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_write(unit, port, 15, 25, 0x074D)) != RT_ERR_OK)
        return ret;


#if defined(CONFIG_SDK_RTL8380)
    if ((HAL_IS_RTL8380_FAMILY_ID(unit)) || (HAL_IS_RTL8330_FAMILY_ID(unit)))
    {
        uint32 phy_data;

        /* For 8380, enable serdes always linkup */
        if((port % 8) == 0)
        {
            /*14B*/
            phy_data = 8;
            ret = hal_miim_write(unit,  port,  15,  30, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0x8403;
            ret = hal_miim_write(unit,  port,  15,  21, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 8;
            ret = hal_miim_write(unit,  port+1,  15,  30, phy_data);
            if(ret != RT_ERR_OK)
                return ret;

            phy_data = 0x8403;
            ret = hal_miim_write(unit,  port+1,  15,  21, phy_data);
            if(ret != RT_ERR_OK)
                return ret;
        }

        /*Enable PktGen for PHY error counter patch*/
        phy_data = 0x5a06;
        ret = hal_miim_write(unit,  port,  6,  0, phy_data);
        if(ret != RT_ERR_OK)
            return ret;
     }
#endif

    /* check the serdes is linkup or not?
     * If serdes is not linkup, reset the serdes by disable/enable serdes.
     * retry 5 time for this mechanism
     */
    if (port == base_port)
    {
        if ((ret = hal_miim_read(unit, base_port, 10, 18, &version)) != RT_ERR_OK)
            return ret;

        is_fb = version & 0xF;
        if (is_fb == 0)
        {
            if ((ret = hal_miim_write(unit, base_port+3, 8, 18, 0x93f0)) != RT_ERR_OK)
            {
                return ret;
            }

            if ((ret = hal_miim_read(unit, base_port+3, 8, 19, &val)) != RT_ERR_OK)
                return ret;

            if (((val & 0xF) == 0xE) || ((val & 0xF) == 0x8))
                is_12b = 1;
            else
                is_12b = 0;
        }

        if (is_fb == 0 && is_12b == 1)
        {
            //osal_printf("### RTL8212B Recover Serdes Link - Phy0Id = %d\n", base_port);
            if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0xE000)) != RT_ERR_OK) /* disable SerDes */
                return ret;
            osal_time_mdelay(100);
            if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0xC000)) != RT_ERR_OK) /* enable SerDes */
                return ret;
        }
        else
        {
            //osal_printf("### RTL8214B/RTL8214FB Recover Serdes Link - Phy0Id = %d\n", base_port);
            if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0xE000)) != RT_ERR_OK) /* disable SerDes */
                return ret;
            osal_time_mdelay(100);
            if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0x8000)) != RT_ERR_OK) /* enable SerDes */
                 return ret;
        }

        do {
            osal_time_mdelay(100);
            if ((ret = hal_miim_read(unit, base_port+2, 8, 18, &serdes_link)) != RT_ERR_OK)
                return ret;
            if (is_fb == 0 && is_12b == 1)
            {
                if (serdes_link != 0x0003)
                {
                    //osal_printf("### RTL8212B Recover Serdes Link - Phy0Id = %d, serdes_link = 0x%0x\n", base_port, serdes_link);
                    if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0xE000)) != RT_ERR_OK) /* disable SerDes */
                        return ret;
                    osal_time_mdelay(100);
                    if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0xC000)) != RT_ERR_OK) /* enable SerDes */
                        return ret;
                }
                else
                {
                    //osal_printf("### RTL8212B Serdes Link OK - Phy0Id = %d ###\n", base_port);
                    break;
                }
            }
            else
            {
                if (serdes_link != 0x0033)
                {
                    //osal_printf("### RTL8214B/RTL8214FB Recover Serdes Link - Phy0Id = %d, serdes_link = 0x%0x\n", base_port, serdes_link);
                    if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0xE000)) != RT_ERR_OK) /* disable SerDes */
                        return ret;
                    osal_time_mdelay(100);
                    if ((ret = hal_miim_write(unit, base_port+3, 8, 28, 0x8000)) != RT_ERR_OK) /* enable SerDes */
                        return ret;
                }
                else
                {
                    //osal_printf("### RTL8214B/RTL8214FB Serdes Link OK - Phy0Id = %d ###\n", base_port);
                    break;
                }
            }
        } while (retry_serdes_count++ < 5);
    }

#if defined(CONFIG_SDK_RTL8390)
    if (HAL_IS_RTL8350_FAMILY_ID(unit))
    {
        if (HAL_IS_ESCHIP(unit))
        {
            uint32  idx = 0;
            typedef struct {
                unsigned int    reg;
                unsigned int    val;
            } confcode_mac_regval_t;

            static confcode_mac_regval_t rtl835x_es_mac_serdes_rst[] =
            {
                { 0xA004, 0x71467380 },
                { 0xA004, 0x71067380 },
                { 0xA104, 0x71467380 },
                { 0xA104, 0x71067380 },
                { 0xa340, 0x8A17080F },
                { 0xa340, 0x9A17080F },
                { 0xa340, 0x9A57080F },
                { 0xA404, 0x71467380 },
                { 0xA404, 0x71067380 },
                { 0xA504, 0x71467380 },
                { 0xA504, 0x71067380 },
                { 0xa740, 0x8A17080F },
                { 0xa740, 0x9A17080F },
                { 0xa740, 0x9A57080F },
                { 0xA804, 0x71467380 },
                { 0xA804, 0x71067380 },
                { 0xA904, 0x71467380 },
                { 0xA904, 0x71067380 },
                { 0xab40, 0x8A17080F },
                { 0xab40, 0x9A17080F },
                { 0xab40, 0x9A57080F },
                { 0xAc04, 0x71467380 },
                { 0xAc04, 0x71067380 },
                { 0xAd04, 0x71467380 },
                { 0xAd04, 0x71067380 },
                { 0xaf40, 0x8A17080F },
                { 0xaf40, 0x9A17080F },
                { 0xaf40, 0x9A57080F },
                { 0xb404, 0x71467380 },
                { 0xb404, 0x71067380 },
                { 0xb504, 0x71467380 },
                { 0xb504, 0x71067380 },
                { 0xb740, 0x8A17080F },
                { 0xb740, 0x9A17080F },
                { 0xb740, 0x9A57080F },
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

            /* Reset Serdes */
            //osal_printf("### Reset 835x mac serdes - Phy0Id = %d\n", base_port);
            for (idx=0; idx<(sizeof(rtl835x_es_mac_serdes_rst)/sizeof(confcode_mac_regval_t)); idx++)
            {
                ioal_mem32_write(unit, rtl835x_es_mac_serdes_rst[idx].reg, \
                                rtl835x_es_mac_serdes_rst[idx].val);
            }
        }
        else
        {
            uint32  idx = 0;
            typedef struct {
                unsigned int    reg;
                unsigned int    val;
            } confcode_mac_regval_t;

            static confcode_mac_regval_t rtl835x_mp_mac_serdes_rst[] =
            {
                /* Serdes 0 Reset */
                { 0xa3c0, 0x50043f },
                { 0xa3c0, 0xf0043f },
                { 0xa3c0, 0x43f },
                { 0xa004, 0x714670c0 },
                { 0xa004, 0x710670c0 },
                { 0xa104, 0x714670c0 },
                { 0xa104, 0x710670c0 },
                /* Serdes 2 Reset */
                { 0xa7c0, 0x50043f },
                { 0xa7c0, 0xf0043f },
                { 0xa7c0, 0x43f },
                { 0xa404, 0x714670c0 },
                { 0xa404, 0x710670c0 },
                { 0xa504, 0x714670c0 },
                { 0xa504, 0x710670c0 },
                /* Serdes 4 Reset */
                { 0xabc0, 0x50043f },
                { 0xabc0, 0xf0043f },
                { 0xabc0, 0x43f },
                { 0xa804, 0x714670c0 },
                { 0xa804, 0x710670c0 },
                { 0xa904, 0x714670c0 },
                { 0xa904, 0x710670c0 },
                /* Serdes 6 Reset */
                { 0xafc0, 0x50043f },
                { 0xafc0, 0xf0043f },
                { 0xafc0, 0x43f },
                { 0xac04, 0x714670c0 },
                { 0xac04, 0x710670c0 },
                { 0xad04, 0x714670c0 },
                { 0xad04, 0x710670c0 },
                /* Serdes 8 Reset */
                { 0xb3f8, 0x50000 },
                { 0xb3f8, 0xf0000 },
                { 0xb3f8, 0x0 },
                { 0xb004, 0x714670c0 },
                { 0xb004, 0x710670c0 },
                { 0xb104, 0x714670c0 },
                { 0xb104, 0x710670c0 },
                /* Serdes 10 Reset */
                { 0xb7c0, 0x50043f },
                { 0xb7c0, 0xf0043f },
                { 0xb7c0, 0x43f },
                { 0xb404, 0x714670c0 },
                { 0xb404, 0x710670c0 },
                { 0xb504, 0x714670c0 },
                { 0xb504, 0x710670c0 },
                /* Serdes 12 Reset */
                { 0xbbf8, 0x50000 },
                { 0xbbf8, 0xf0000 },
                { 0xbbf8, 0x0 },
                { 0xb804, 0x714670c0 },
                { 0xb804, 0x710670c0 },
                { 0xb904, 0x714670c0 },
                { 0xb904, 0x710670c0 },
            };

            /* Reset Serdes */
            //osal_printf("### Reset 835x mac serdes - Phy0Id = %d\n", base_port);
            for (idx=0; idx<(sizeof(rtl835x_mp_mac_serdes_rst)/sizeof(confcode_mac_regval_t)); idx++)
            {
                ioal_mem32_write(unit, rtl835x_mp_mac_serdes_rst[idx].reg, \
                                rtl835x_mp_mac_serdes_rst[idx].val);
            }
        }
    }
#endif

    return RT_ERR_OK;
} /* end of phy_8214fb_init */

/* Function Name:
 *      phy_8214fb_autoNegoAbility_get
 * Description:
 *      Get ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pAbility - pointer to PHY auto negotiation ability
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData4;
    uint32  phyData9;
    uint32  phyData;
    uint32  restore_val, val;
    rtk_port_media_t media;
    hal_control_t   *pHalCtrl;
    uint32  phyExtStatus15, is_fiber_linkup = 0, is_copper_linkup = 0;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        /* get value from internal status register */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 23, &phyData)) != RT_ERR_OK)
            return ret;

        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
            return ret;

        if (is_fiber_linkup)
        {
            if ((phyData & 0x0188) == 0x0088)
            {   /* 100Base-FX Linkup */
                pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
                pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;
                pAbility->Half_10 = 0;
                pAbility->Full_10 = 0;
                pAbility->Half_100 = 0;
                pAbility->Full_100 = 0;
                pAbility->Half_1000 = 0;
                pAbility->Full_1000 = 0;
            }
            else
            {   /* 1000Base-X Linkup */
                pAbility->FC = (phyData4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
                pAbility->AsyFC = (phyData4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
                pAbility->Half_10 = 0;
                pAbility->Full_10 = 0;
                pAbility->Half_100 = 0;
                pAbility->Full_100 = 0;
                pAbility->Half_1000 = (phyData4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
                pAbility->Full_1000 = (phyData4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;
            }
        }
        else if (is_copper_linkup)
        {
            pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
            pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;

            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
                return ret;

            pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
            pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
            pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
            pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
            pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
            pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;
        }
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
        if (restore_val & 0x0020)
            media = PORT_MEDIA_COPPER;
        else
            media = PORT_MEDIA_FIBER;
    }
    else
    {
        if (restore_val & 0x1000)
            media = PORT_MEDIA_FIBER;
        else
            media = PORT_MEDIA_COPPER;
    }

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        if ((restore_val & 0x0c00) == 0x0c00)
        {   /* 100-fx */
            pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
            pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;
            pAbility->Half_10 = 0;
            pAbility->Full_10 = 0;
            pAbility->Half_100 = 0;
            pAbility->Full_100 = 0;
            pAbility->Half_1000 = 0;
            pAbility->Full_1000 = 0;
        }
        else
        {   /* 1000-x */
            pAbility->FC = (phyData4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
            pAbility->AsyFC = (phyData4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
            pAbility->Half_10 = 0;
            pAbility->Full_10 = 0;
            pAbility->Half_100 = 0;
            pAbility->Full_100 = 0;
            pAbility->Half_1000 = (phyData4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
            pAbility->Full_1000 = (phyData4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;
        }
    }
    else if (PORT_MEDIA_COPPER == media)
    {
        pAbility->FC = (phyData4 & Pause_R4_MASK) >> Pause_R4_OFFSET;
        pAbility->AsyFC = (phyData4 & AsymmetricPause_R4_MASK) >> AsymmetricPause_R4_OFFSET;

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
            return ret;

        pAbility->Full_100= (phyData4 & _100Base_TX_FD_R4_MASK) >> _100Base_TX_FD_R4_OFFSET;
        pAbility->Half_100= (phyData4 & _100Base_TX_R4_MASK) >> _100Base_TX_R4_OFFSET;
        pAbility->Full_10= (phyData4 & _10Base_T_FD_R4_MASK) >> _10Base_T_FD_R4_OFFSET;
        pAbility->Half_10= (phyData4 & _10Base_T_R4_MASK) >> _10Base_T_R4_OFFSET;
        pAbility->Half_1000 = (phyData9 & _1000Base_THalfDuplex_MASK) >> _1000Base_THalfDuplex_OFFSET;
        pAbility->Full_1000 = (phyData9 & _1000Base_TFullDuplex_MASK) >> _1000Base_TFullDuplex_OFFSET;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fb_autoNegoAbility_get */

/* Function Name:
 *      phy_8214fb_autoNegoAbility_set
 * Description:
 *      Set ability advertisement for auto negotiation of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 *      pAbility  - auto negotiation ability that is going to set to PHY
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32   ret;
    uint32  phyData0;
    uint32  phyData4;
    uint32  phyData9;
    uint32  phyData;
    uint32  restore_val, val;
    rtk_enable_t     enable;
    rtk_port_media_t media;
    hal_control_t   *pHalCtrl;
    uint32  phyExtStatus15, is_fiber_linkup = 0, is_copper_linkup = 0;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        phy_8214fb_autoNegoEnable_get(unit, port, &enable);

        /* get value from internal status register */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 23, &phyData)) != RT_ERR_OK)
            return ret;

        /* get value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
            return ret;

        if (is_fiber_linkup)
        {
            if ((phyData & 0x0188) == 0x0088)
            {   /* 100Base-FX Linkup */
                phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
                phyData4 = phyData4
                        | (pAbility->FC << Pause_R4_OFFSET)
                        | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);
            }
            else
            {   /* 1000Base-X Linkup */
                phyData4 = phyData4 & ~(_1000BaseX_Pause_R4_MASK | _1000BaseX_AsymmetricPause_R4_MASK);
                phyData4 = phyData4
                        | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET)
                        | (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET);
                phyData4 = phyData4 & ~(_1000BaseX_HalfDuplex_R4_MASK | _1000BaseX_FullDuplex_R4_MASK);
                phyData4 = phyData4 | (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET)
                        | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);
            }
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
                return ret;
        }
        else if (is_copper_linkup)
        {
            phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->FC << Pause_R4_OFFSET)
                    | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
                return ret;

            phyData4 = phyData4 &
                    ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
                    | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
                    | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
                    | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

            phyData9 = phyData9 & ~(_1000Base_TFullDuplex_MASK | _1000Base_THalfDuplex_MASK);
            phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                       | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
                return ret;

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
                return ret;
        }

        /* Force re-autonegotiation if AN is on */
        if (ENABLED == enable)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
                return ret;

            phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
            phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
                return ret;
        }
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
        if (restore_val & 0x0020)
            media = PORT_MEDIA_COPPER;
        else
            media = PORT_MEDIA_FIBER;
    }
    else
    {
        if (restore_val & 0x1000)
            media = PORT_MEDIA_FIBER;
        else
            media = PORT_MEDIA_COPPER;
    }

    phy_8214fb_autoNegoEnable_get(unit, port, &enable);

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, &phyData4)) != RT_ERR_OK)
        return ret;

    if (PORT_MEDIA_FIBER == media)
    {
        if ((restore_val & 0x0c00) == 0x0c00)
        {   /* 100-fx */
            phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->FC << Pause_R4_OFFSET)
                    | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);
        }
        else
        {   /* 1000-x */
            phyData4 = phyData4 & ~(_1000BaseX_Pause_R4_MASK | _1000BaseX_AsymmetricPause_R4_MASK);
            phyData4 = phyData4
                    | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET)
                    | (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET);
            phyData4 = phyData4 & ~(_1000BaseX_HalfDuplex_R4_MASK | _1000BaseX_FullDuplex_R4_MASK);
            phyData4 = phyData4 | (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET)
                    | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);
        }
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
            return ret;
    }
    else if (PORT_MEDIA_COPPER == media)
    {
        phyData4 = phyData4 & ~(Pause_R4_MASK | AsymmetricPause_R4_MASK);
        phyData4 = phyData4
                | (pAbility->FC << Pause_R4_OFFSET)
                | (pAbility->AsyFC << AsymmetricPause_R4_OFFSET);

        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, &phyData9)) != RT_ERR_OK)
            return ret;

        phyData4 = phyData4 &
                ~(_100Base_TX_FD_R4_MASK | _100Base_TX_R4_MASK | _10Base_T_FD_R4_MASK | _10Base_T_R4_MASK);
        phyData4 = phyData4
                | (pAbility->Full_100 << _100Base_TX_FD_R4_OFFSET)
                | (pAbility->Half_100 << _100Base_TX_R4_OFFSET)
                | (pAbility->Full_10 << _10Base_T_FD_R4_OFFSET)
                | (pAbility->Half_10 << _10Base_T_R4_OFFSET);

        phyData9 = phyData9 & ~(_1000Base_THalfDuplex_MASK | _1000Base_TFullDuplex_MASK);
        phyData9 = phyData9 | (pAbility->Full_1000 << _1000Base_TFullDuplex_OFFSET)
                   | (pAbility->Half_1000 << _1000Base_THalfDuplex_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_AN_ADVERTISEMENT_REG, phyData4)) != RT_ERR_OK)
            return ret;

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_1000_BASET_CONTROL_REG, phyData9)) != RT_ERR_OK)
            return ret;
    }

    /* Force re-autonegotiation if AN is on*/
    if (ENABLED == enable)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | (enable << RestartAutoNegotiation_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8214fb_autoNegoAbility_set */


/* Function Name:
 *      phy_8214fb_speed_get
 * Description:
 *      Get link speed status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pSpeed - pointer to PHY link speed
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
                  | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
    }

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    *pSpeed = ((phyData0 & SpeedSelection1_MASK) >> (SpeedSelection1_OFFSET -1))
              | ((phyData0 & SpeedSelection0_MASK) >> SpeedSelection0_OFFSET);

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8214fb_speed_get */


/* Function Name:
 *      phy_8214fb_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32   ret;
    uint32  phyData0;
    rtk_port_media_t media;
    uint32  restore_val, val;
    hal_control_t   *pHalCtrl;
    uint32  phyExtStatus15, is_fiber_linkup = 0, is_copper_linkup = 0;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        if (is_fiber_linkup)
        {
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
                return ret;
            val = restore_val & (~(3<<10));
            if (speed == PORT_SPEED_1000M)
            {
                val |= (2<<10);
            }
            else if (speed == PORT_SPEED_100M)
            {
                val |= (3<<10);
            }
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
        }

        /* set value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
        phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
        if (restore_val & 0x0020)
            media = PORT_MEDIA_COPPER;
        else
            media = PORT_MEDIA_FIBER;
    }
    else
    {
        if (restore_val & 0x1000)
            media = PORT_MEDIA_FIBER;
        else
            media = PORT_MEDIA_COPPER;
    }

    if (PORT_MEDIA_FIBER == media)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
            return ret;
        val &= ~(3<<10);
        restore_val &= ~(3<<10);
        if (speed == PORT_SPEED_1000M)
        {
            val |= (2<<10);
            restore_val |= (2<<10);
        }
        else if (speed == PORT_SPEED_100M)
        {
            val |= (3<<10);
            restore_val |= (3<<10);
        }
        if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
    }

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(SpeedSelection1_MASK | SpeedSelection0_MASK);
    phyData0 = phyData0 | (((speed & 2) << (SpeedSelection1_OFFSET - 1)) | ((speed & 1) << SpeedSelection0_OFFSET));

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8214fb_speed_set */

/* Function Name:
 *      phy_8214fb_rtctResult_get
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
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_PHY_RTCT_NOT_FINISH   - RTCT not finish. Need to wait a while.
 *      RT_ERR_TIMEOUT      - RTCT test timeout in this port.
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_8214fb_rtctResult_get(uint32 unit, rtk_port_t port, rtk_rtctResult_t *pRtctResult)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData;
    rtk_port_media_t  media;
    uint32  speed;

    if ((ret = phy_8214fb_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    if (media == PORT_MEDIA_COPPER)
    {
        /* Check the port is link up or not? */
        ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData);
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData)) != RT_ERR_OK)
            return ret;

        phy_common_speed_get(unit, port, &speed);

        if ((phyData & LinkStatus_MASK) && speed != 0)
        {
            /* If the port is link up,
             * return cable length from green function
             */
            if ((ret = hal_miim_read(unit, port, 1, 4, &phyData)) != RT_ERR_OK)
                return ret;
            if ((phyData & 0x1) == 0x0000)
                return RT_ERR_PHY_RTCT_NOT_FINISH;
            if ((ret = hal_miim_read(unit, port, 1, 30, &phyData)) != RT_ERR_OK)
                return ret;
            pRtctResult->linkType = PORT_SPEED_1000M;
            pRtctResult->ge_result.channelALen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelBLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelCLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelDLen = (phyData & 0x00FF)*100;
            pRtctResult->ge_result.channelAShort = 0;
            pRtctResult->ge_result.channelBShort = 0;
            pRtctResult->ge_result.channelCShort = 0;
            pRtctResult->ge_result.channelDShort = 0;
            pRtctResult->ge_result.channelAOpen = 0;
            pRtctResult->ge_result.channelBOpen = 0;
            pRtctResult->ge_result.channelCOpen = 0;
            pRtctResult->ge_result.channelDOpen = 0;
            pRtctResult->ge_result.channelAMismatch = 0;
            pRtctResult->ge_result.channelBMismatch = 0;
            pRtctResult->ge_result.channelCMismatch = 0;
            pRtctResult->ge_result.channelDMismatch = 0;
            pRtctResult->ge_result.channelALinedriver = 0;
            pRtctResult->ge_result.channelBLinedriver = 0;
            pRtctResult->ge_result.channelCLinedriver = 0;
            pRtctResult->ge_result.channelDLinedriver = 0;
        }
        else
        {
            if ((ret = hal_miim_read(unit, port, 11, 27, &phyData)) != RT_ERR_OK)
                return ret;

            if (0 == (phyData & 0x4000))
                return RT_ERR_PHY_RTCT_NOT_FINISH;

            pRtctResult->linkType = PORT_SPEED_1000M;
            /* Length = (Index/64)*8ns*(0.2m/ns) = Index/40 (m) = (2.5) * Index (cm) */
            pRtctResult->ge_result.channelALen = (phyData & 0x1FFF)*5/2;

            if ((ret = hal_miim_read(unit, port, 11, 26, &phyData)) != RT_ERR_OK)
                return ret;
            pRtctResult->ge_result.channelAShort = phyData & 0x1000;
            pRtctResult->ge_result.channelBShort = phyData & 0x2000;
            pRtctResult->ge_result.channelCShort = phyData & 0x4000;
            pRtctResult->ge_result.channelDShort = phyData & 0x8000;
            pRtctResult->ge_result.channelAOpen  = phyData & 0x0100;
            pRtctResult->ge_result.channelBOpen  = phyData & 0x0200;
            pRtctResult->ge_result.channelCOpen  = phyData & 0x0400;
            pRtctResult->ge_result.channelDOpen  = phyData & 0x0800;
            pRtctResult->ge_result.channelAMismatch  = phyData & 0x0010;
            pRtctResult->ge_result.channelBMismatch  = phyData & 0x0020;
            pRtctResult->ge_result.channelCMismatch  = phyData & 0x0040;
            pRtctResult->ge_result.channelDMismatch  = phyData & 0x0080;
            pRtctResult->ge_result.channelALinedriver  = phyData & 0x0001;
            pRtctResult->ge_result.channelBLinedriver  = phyData & 0x0002;
            pRtctResult->ge_result.channelCLinedriver  = phyData & 0x0004;
            pRtctResult->ge_result.channelDLinedriver  = phyData & 0x0008;

            if ((ret = hal_miim_read(unit, port, 11, 28, &phyData)) != RT_ERR_OK)
                return ret;
            pRtctResult->ge_result.channelBLen = (phyData & 0x1FFF)*5/2;

            if ((ret = hal_miim_read(unit, port, 11, 29, &phyData)) != RT_ERR_OK)
                return ret;
            pRtctResult->ge_result.channelCLen = (phyData & 0x1FFF)*5/2;

            if ((ret = hal_miim_read(unit, port, 11, 30, &phyData)) != RT_ERR_OK)
                return ret;
            pRtctResult->ge_result.channelDLen = (phyData & 0x1FFF)*5/2;
        }
    }
    else
    {
        /* RTCT function is not supoprted in fiber media */
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return ret;
} /* end of phy_8214fb_rtctResult_get */

/* Function Name:
 *      phy_8214fb_rtct_start
 * Description:
 *      Start PHY interface RTCT test of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - chip not supported
 * Note:
 *      None
 */
int32
phy_8214fb_rtct_start(uint32 unit, rtk_port_t port)
{
    int32   ret = RT_ERR_FAILED;
    uint32  phyData, phyData21, i;
    rtk_port_media_t  media;
    uint32  speed;

    if ((ret = phy_8214fb_media_get(unit, port, &media)) != RT_ERR_OK)
        return ret;

    if (media == PORT_MEDIA_COPPER)
    {
        /* Check the port is link up or not? */
        ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData);
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &phyData)) != RT_ERR_OK)
            return ret;

        phy_common_speed_get(unit, port, &speed);

        if ((phyData & LinkStatus_MASK) && speed != 0)
        {
            /* If the port is link up,
             * enable the green function to know the cable length
             */
            if ((ret = hal_miim_read(unit, port, 1, 4, &phyData)) != RT_ERR_OK)
            {
                RT_LOG(LOG_EVENT, MOD_HAL, "hal_miim_read(unit %d, port %d) page 1 reg 4 value 0x%x!!", unit, port, phyData);
                return ret;
            }
            phyData |= 0x1;
            if ((ret = hal_miim_write(unit, port, 1, 4, phyData)) != RT_ERR_OK)
            {
                RT_LOG(LOG_EVENT, MOD_HAL, "hal_miim_write(unit %d, port %d) page 1 reg 4 value 0x%x!!", unit, port, phyData);
                return ret;
            }
        }
        else
        {
            /* Disable Power Saving Mode First */
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 21, &phyData21)) != RT_ERR_OK)
                return ret;
            phyData = phyData21 & ~(0x1000);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 21, phyData)) != RT_ERR_OK)
                return ret;
            osal_time_udelay(100 * 1000);
            /* Change to ext. page 47 for force giga power */
            if ((ret = hal_miim_write(unit, port, 7, 30, 0x002F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 7, 23, 0xD820)) != RT_ERR_OK)
                return ret;
            /* Page 11, PHY 0-3, Register 25 (Per-port RTCT Configuration Register)
             * bit[15:15]: Enable port 0-3 cable test. When enable port x RTCT,
             *             RTL8214 will scan port x cable status from channel A
             *             to channel D sequentially.
             *             1: Enable
             *             0: Disable
             */
            if ((ret = hal_miim_read(unit, port, 11, 25, &phyData)) != RT_ERR_OK)
                return ret;

            phyData &= ~(0x8000);
            if ((ret = hal_miim_write(unit, port, 11, 25, phyData)) != RT_ERR_OK)
                return ret;

            osal_time_udelay(100 * 1000);
            phyData |= (0x8000);
            if ((ret = hal_miim_write(unit, port, 11, 25, phyData)) != RT_ERR_OK)
                return ret;

            for (i = 0; i < RTCT_CHECKBUSY_TIMES; i++)
            {
                osal_time_udelay(500 * 1000);
                if ((ret = hal_miim_read(unit, port, 11, 27, &phyData)) != RT_ERR_OK)
                    return ret;
                if (phyData & 0x4000)
                    break;
            }

            /* Change to ext. page 47 for normal power */
            if ((ret = hal_miim_write(unit, port, 7, 30, 0x002F)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_write(unit, port, 7, 23, 0xD88F)) != RT_ERR_OK)
                return ret;
            /* Restore Power Saving Mode */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 21, phyData21)) != RT_ERR_OK)
                return ret;

            if (RTCT_CHECKBUSY_TIMES == i)
                return RT_ERR_PHY_RTCT_TIMEOUT;
        }
    }
    else
    {
        /* RTCT function is not supoprted in fiber media */
        return RT_ERR_CHIP_NOT_SUPPORTED;
    }

    return ret;
} /* end of phy_8214fb_rtct_start */

/* Function Name:
 *      phy_8214fb_greenEnable_get
 * Description:
 *      Get the status of link-up green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pEnable - pointer to status of link-up green feature
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      1. The link-up green feature. (Cable Length Power Saving)
 */
int32
phy_8214fb_greenEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  phyData;
    int32   ret = RT_ERR_FAILED;

    /* getting page 5, register 1, bit[8] for enable/disable green */
    if ((ret = hal_miim_read(unit, port, 5, 1, &phyData)) != RT_ERR_OK)
        return ret;
    if ((phyData >> 8) & 0x1)
        *pEnable = DISABLED;
    else
        *pEnable = ENABLED;

    return RT_ERR_OK;
} /* end of phy_8214fb_greenEnable_get */

/* Function Name:
 *      phy_8214fb_greenEnable_set
 * Description:
 *      Set the status of link-up green feature of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-up green feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 * Note:
 *      1. The link-up green feature. (Cable Length Power Saving)
 */
int32
phy_8214fb_greenEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData;
    int32   ret = RT_ERR_FAILED;

    /* For Link-On and Cable Length Power Saving (per-port) */
    /* setting page 5, register 1, bit[8] for enable/disable green */
    if ((ret = hal_miim_read(unit, port, 5, 1, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= 0xFEFF;
    if (ENABLED == enable)
        phyData = 0x0201;
    else
        phyData = 0x0301;
    if ((ret = hal_miim_write(unit, port, 5, 1, phyData)) != RT_ERR_OK)
        return ret;
    /* setting the GreenEthernet TX/RX Threshold */
    if (ENABLED == enable)
    {
        /* enable RX GreenEthernet threshold */
        if ((ret = hal_miim_write(unit, port, 3, 24, 0xBC3D)) != RT_ERR_OK)
            return ret;
        /* enable TX GreenEthernet threshold */
        if ((ret = hal_miim_write(unit, port, 3, 25, 0x3C3D)) != RT_ERR_OK)
            return ret;
    }
    else
    {
        /* disable RX GreenEthernet threshold */
        if ((ret = hal_miim_write(unit, port, 3, 24, 0xBC3C)) != RT_ERR_OK)
            return ret;
        /* disable TX GreenEthernet threshold */
        if ((ret = hal_miim_write(unit, port, 3, 25, 0x3C3C)) != RT_ERR_OK)
            return ret;
    }

    /* For Link-Down Power Saving (per-port) */
    if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(0x1 << 12);
    if (ENABLED == enable)
    {
        phyData |= (0x1 << 12);
    }
    if ((ret = hal_miim_write(unit, port, 0, 21, phyData)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fb_greenEnable_set */

/* Function Name:
 *      phy_8214fb_eeeEnable_get
 * Description:
 *      Get enable status of EEE function in the specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of EEE
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      None
 */
int32
phy_8214fb_eeeEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  phyData;

    /* get value from CHIP*/
    if ((ret = hal_miim_write(unit, port, 7, 0x1e, 0x0020)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 7, 0x15, &phyData)) != RT_ERR_OK)
        return ret;

    if ((phyData >> 8) & 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    return ret;
} /* end of phy_8214fb_eeeEnable_get */

/* Function Name:
 *      phy_8214fb_eeeEnable_set
 * Description:
 *      Set enable status of EEE function in the specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of EEE
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8214fb_eeeEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData0;
    int32   ret = RT_ERR_FAILED;
    rtk_enable_t    an_enable;
    uint32  phyData, restore_phyData;
    uint32  base_port, val, phyExtStatus15;
    uint32  is_fiber_linkup, is_copper_linkup, is_phyDown;

    base_port = port - (port % PORT_NUM_IN_8214FB);

    phy_8214fb_autoNegoEnable_get(unit, port, &an_enable);

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;

    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        if (is_copper_linkup)
        {

        }
        else
        {
            /* eee register is in copper page and can't access when fiber is linkup
             * If access via page 8 register 16 with force access copper PHY, it will cause
             * the fiber link down.
             */
            return RT_ERR_PHY_FIBER_LINKUP;
        }
    }

    /* linkdown or is_copper_linkup */
    if ((ret = hal_miim_read(unit, base_port+1, 8, 16, &restore_phyData)) != RT_ERR_OK)
        return ret;

    phyData = restore_phyData & ~(1<<(12+port-base_port));
    phyData |= (1<<(8+port-base_port));

    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, phyData)) != RT_ERR_OK)
        return ret;

    /* Power Down PHY */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;
    is_phyDown = (phyData0 >> 11) & 0x1;
    phyData0 |= (1 << 11);
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if (ENABLED == enable)
    {
        /* Micro-C Enable or Disable Auto Turn off EEE */
        if ((ret = hal_miim_write(unit, port, 5, 0x05, 0x8B85)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 5, 0x06, 0xE286)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* Micro-C Control 10M EEE */
        if ((ret = hal_miim_write(unit, port, 5, 0x05, 0x8B86)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 5, 0x06, 0x8600)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* Enable or Disable EEE */
        if ((ret = hal_miim_write(unit, port, 7, 0x1e, 0x0020)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 7, 0x15, 0x0100)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 7, 0x1b, 0xA03A)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* 100/1000M EEE Capability */
        if ((ret = hal_miim_write(unit, port, 0, 0x0d, 0x0007)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0e, 0x003C)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0d, 0x4007)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0e, 0x0006)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0d, 0x0000)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* 10M EEE Amplitude */
        if ((ret = hal_miim_write(unit, port, 2, 0x0b, 0x17A7)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;
    }
    else
    {
        /* Micro-C Enable or Disable Auto Turn off EEE */
        if ((ret = hal_miim_write(unit, port, 5, 0x05, 0x8B85)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 5, 0x06, 0xC286)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* Micro-C Control 10M EEE */
        if ((ret = hal_miim_write(unit, port, 5, 0x05, 0x8B86)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 5, 0x06, 0x8600)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* Enable or Disable EEE */
        if ((ret = hal_miim_write(unit, port, 7, 0x1e, 0x0020)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 7, 0x15, 0x0000)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 7, 0x1b, 0xA03A)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* 100/1000M EEE Capability */
        if ((ret = hal_miim_write(unit, port, 0, 0x0d, 0x0007)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0e, 0x003C)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0d, 0x4007)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0e, 0x0000)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 0, 0x0d, 0x0000)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;

        /* 10M EEE Amplitude */
        if ((ret = hal_miim_write(unit, port, 2, 0x0b, 0x17A7)) != RT_ERR_OK)
            return ret;
        if ((ret = hal_miim_write(unit, port, 8, 0x1f, 0x0008)) != RT_ERR_OK)
            return ret;
    }

    /* Force re-autonegotiation if AN is on */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;
    if (ENABLED == an_enable)
    {
        phyData0 = phyData0 & ~(RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | (an_enable << RestartAutoNegotiation_OFFSET);
    }

    if (!is_phyDown)
        phyData0 &= ~(1<<11);
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    /* restore PHY 1, page 8, register 16 */
    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, restore_phyData)) != RT_ERR_OK)
        return ret;

    osal_time_usleep(1000 * 1000); /* delay 1000mS */

    return ret;
} /* end of phy_8214fb_eeeEnable_set */

/* Function Name:
 *      phy_8214fb_autoNegoEnable_get
 * Description:
 *      Get autonegotiation enable status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        if (phyData0 & AutoNegotiationEnable_MASK)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
    }

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    if (phyData0 & AutoNegotiationEnable_MASK)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8214fb_autoNegoEnable_get */

/* Function Name:
 *      phy_8214fb_autoNegoEnable_set
 * Description:
 *      Set autonegotiation enable status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    uint32  phyExtStatus15, is_fiber_linkup = 0, is_copper_linkup = 0;
    rtk_port_media_t media;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;

    if (val & LinkStatus_MASK)
    {   /* Link-up */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        /* configure the Nway and Restart-Nway bit to PHY chip */
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(AutoNegotiationEnable_MASK | RestartAutoNegotiation_MASK);
        phyData0 = phyData0 | ((enable << AutoNegotiationEnable_OFFSET) | (1 << RestartAutoNegotiation_OFFSET));

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;

		/*Remove : No need to set the fiber media mode to FIBER-AUTO*/
//        if (is_fiber_linkup && (ENABLED == enable))
//        {
//            if ((ret = phy_8214fb_fiber_media_set(unit, port, PORT_FIBER_MEDIA_AUTO)) != RT_ERR_OK)
//                return ret;
//        }
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
        if (restore_val & 0x0020)
            media = PORT_MEDIA_COPPER;
        else
            media = PORT_MEDIA_FIBER;
    }
    else
    {
        if (restore_val & 0x1000)
            media = PORT_MEDIA_FIBER;
        else
            media = PORT_MEDIA_COPPER;
    }


    /* configure the Nway and Restart-Nway bit to PHY chip */
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;
    phyData0 = phyData0 & ~(AutoNegotiationEnable_MASK | RestartAutoNegotiation_MASK);
    phyData0 = phyData0 | ((enable << AutoNegotiationEnable_OFFSET) | (1 << RestartAutoNegotiation_OFFSET));
    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if ((PORT_MEDIA_FIBER == media) && (ENABLED == enable))
    {
        if ((ret = phy_8214fb_fiber_media_set(unit, port, PORT_FIBER_MEDIA_AUTO)) != RT_ERR_OK)
            return ret;
	   /*Remove : No need to set the fiber media mode to FIBER-AUTO*/
//        restore_val = restore_val & 0xf3ff;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8214fb_autoNegoEnable_set */

/* Function Name:
 *      phy_8214fb_duplex_get
 * Description:
 *      Get duplex mode status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pDuplex - pointer to PHY duplex mode status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* get value from CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        *pDuplex = (phyData0 & DuplexMode_MASK) >> DuplexMode_OFFSET;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
    }

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    *pDuplex = (phyData0 & DuplexMode_MASK) >> DuplexMode_OFFSET;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8214fb_duplex_get */

/* Function Name:
 *      phy_8214fb_duplex_set
 * Description:
 *      Set duplex mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      duplex        - duplex mode of the port, full or half
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex)
{
    int32   ret;
    uint32  phyData0;
    uint32  restore_val, val;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        /* set value to CHIP*/
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
            return ret;

        phyData0 = phyData0 & ~(DuplexMode_MASK);
        phyData0 = phyData0 | (duplex << DuplexMode_OFFSET);

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
            return ret;
        return RT_ERR_OK;
    }

    /* Link-down */
    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if (restore_val & 0x0020)
            val &= ~(0x1000);
        else
            val |= (0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
    }

    /* get value from CHIP*/
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData0)) != RT_ERR_OK)
        return ret;

    phyData0 = phyData0 & ~(DuplexMode_MASK);
    phyData0 = phyData0 | (duplex << DuplexMode_OFFSET);

    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData0)) != RT_ERR_OK)
        return ret;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;
    return ret;
} /* end of phy_8214fb_duplex_set */

/* Function Name:
 *      phy_8214fb_enable_set
 * Description:
 *      Set PHY interface status of the specific port
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData, phyData_fiber, phyData_copper;
    uint32  val, restore_val;
    int32   ret;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;

        if ((val & 0x0020) == 0x0000)
        {   /* Original media: Auto-Fiber */
            val &= ~(0x1000);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_copper)) != RT_ERR_OK)
                return ret;
            phyData_copper &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_copper |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_copper)) != RT_ERR_OK)
                return ret;

            val |= (0x1000);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_fiber)) != RT_ERR_OK)
                return ret;
            phyData_fiber &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_fiber |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_fiber)) != RT_ERR_OK)
                return ret;
        }
        else
        {   /* Original media: Auto-Copper */
            val |= (0x1000);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_fiber)) != RT_ERR_OK)
                return ret;
            phyData_fiber &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_fiber |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_fiber)) != RT_ERR_OK)
                return ret;

            val &= ~(0x1000);
            if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData_copper)) != RT_ERR_OK)
                return ret;
            phyData_copper &= ~(PowerDown_MASK);
            if (DISABLED == enable)
                phyData_copper |= (1 << PowerDown_OFFSET);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData_copper)) != RT_ERR_OK)
                return ret;
        }

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
            return ret;
    }
    else
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, &phyData)) != RT_ERR_OK)
            return ret;

        phyData &= ~(PowerDown_MASK);
        if (DISABLED == enable)
            phyData |= (1 << PowerDown_OFFSET);

        if ((restore_val & 0x1000) == 0x1000)
        {   /* force-fiber media */
            if (DISABLED == enable)
            {
                /* Force TX 2.5G */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 25, 0x374D)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                    return ret;
                /* force power */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 21, 0xC129)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                    return ret;
            }
            else
            {
                /* Default setting */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 30, 0x0018)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 25, 0x074D)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                    return ret;
                /* Default setting */
                if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 31, 0x000F)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 30, 0x001E)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 21, 0xC178)) != RT_ERR_OK)
                    return ret;
                if ((ret = hal_miim_write(unit, port, 15, 31, 0x0008)) != RT_ERR_OK)
                    return ret;
            }
        }

        if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, PHY_CONTROL_REG, phyData)) != RT_ERR_OK)
            return ret;
    }
    osal_time_udelay(500000);

    return RT_ERR_OK;
} /* end of phy_8214fb_enable_set */

/* Function Name:
 *      phy_8214fb_crossOverMode_get
 * Description:
 *      Get cross over mode in the specified port.
 * Input:
 *      unit  - unit id
 *      port  - port id
 * Output:
 *      pMode - pointer to cross over mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID      - invalid unit id
 *      RT_ERR_PORT_ID      - invalid port id
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
phy_8214fb_crossOverMode_get(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t *pMode)
{
    int32   ret;
    uint32  phyData, force_mode, mdi;
    uint32  restore_phyData, base_port, val, phyExtStatus15, cross_config_sts;
    uint32  is_fiber_linkup, is_copper_linkup;

    base_port = port - (port % PORT_NUM_IN_8214FB);
    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;
    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        if (is_copper_linkup)
        {
            /* get value from CHIP*/
            if ((ret = hal_miim_write(unit, port, 7, 0x1e, 0x002d)) != RT_ERR_OK)
                return ret;

            if ((ret = hal_miim_read(unit, port, 7, 0x18, &phyData)) != RT_ERR_OK)
                return ret;
            force_mode = (phyData >> 5) & 0x1;

            if ((ret = hal_miim_read(unit, port, 0, 0x10, &phyData)) != RT_ERR_OK)
                return ret;
            mdi = (phyData >> 5) & 0x1;

	    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 31, 0x7)) != RT_ERR_OK)
		return ret;
	    if ((ret = hal_miim_write(unit, port, 0x7, 30, 0x24)) != RT_ERR_OK)
		return ret;
	    if ((ret = hal_miim_read(unit, port, 0x7, 26, &cross_config_sts)) != RT_ERR_OK)
		return ret;
	    if((cross_config_sts & 0x1) == 1) /*Check uc_use_force_nxc bit*/
		force_mode = 0;

	    if ((ret = hal_miim_write(unit, port, PHY_PAGE_0, 31, 0x0)) != RT_ERR_OK)
		return ret;

            if (force_mode)
            {
                if (mdi)
                    *pMode = PORT_CROSSOVER_MODE_MDI;
                else
                    *pMode = PORT_CROSSOVER_MODE_MDIX;
            }
            else
                *pMode = PORT_CROSSOVER_MODE_AUTO;
            return ret;
        }
        else
        {
            /* cross over register is in copper page and can't access when fiber is linkup
             * If access via page 8 register 16 with force access copper PHY, it will cause
             * the fiber link down.
             */
            return RT_ERR_PHY_FIBER_LINKUP;
        }
    }

    if ((ret = hal_miim_read(unit, base_port+1, 8, 16, &restore_phyData)) != RT_ERR_OK)
        return ret;

    phyData = restore_phyData & ~(1<<(12+port-base_port));
    phyData |= (1<<(8+port-base_port));

    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, phyData)) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_write(unit, port, 7, 0x1e, 0x002d)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 7, 0x18, &phyData)) != RT_ERR_OK)
        return ret;
    force_mode = (phyData >> 5) & 0x1;

    if ((ret = hal_miim_read(unit, port, 0, 0x10, &phyData)) != RT_ERR_OK)
        return ret;
    mdi = (phyData >> 5) & 0x1;

    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, restore_phyData)) != RT_ERR_OK)
        return ret;

    if (force_mode)
    {
        if (mdi)
            *pMode = PORT_CROSSOVER_MODE_MDI;
        else
            *pMode = PORT_CROSSOVER_MODE_MDIX;
    }
    else
        *pMode = PORT_CROSSOVER_MODE_AUTO;

    return ret;
} /* end of phy_8214fb_crossOverMode_get */

/* Function Name:
 *      phy_8214fb_crossOverMode_set
 * Description:
 *      Set cross over mode in the specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      mode - cross over mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 *      RT_ERR_PORT_ID - invalid port id
 *      RT_ERR_INPUT   - invalid input parameter
 * Note:
 *      Following value is valid
 *      - PORT_CROSSOVER_MODE_AUTO
 *      - PORT_CROSSOVER_MODE_MDI
 *      - PORT_CROSSOVER_MODE_MDIX
 */
int32
phy_8214fb_crossOverMode_set(uint32 unit, rtk_port_t port, rtk_port_crossOver_mode_t mode)
{
    int32   ret;
    uint32  phyData, force_mode, mdi;
    uint32  restore_phyData, base_port, val, phyExtStatus15;
    uint32  is_fiber_linkup, is_copper_linkup;
	rtk_enable_t portEnable;

    base_port = port - (port % PORT_NUM_IN_8214FB);

    switch (mode)
    {
        case PORT_CROSSOVER_MODE_AUTO:
            force_mode = 0;
            mdi = 1;
            break;
        case PORT_CROSSOVER_MODE_MDI:
            force_mode = 1;
            mdi = 1;
            break;
        case PORT_CROSSOVER_MODE_MDIX:
            force_mode = 1;
            mdi = 0;
            break;
        default:
            return RT_ERR_INPUT;
    }

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;

    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        if (is_copper_linkup)
        {
        	/*Power Down*/
			portEnable = DISABLED;
			phy_8214fb_enable_set(unit, port, portEnable);


			osal_time_mdelay(1000);

		    if ((ret = hal_miim_read(unit, base_port+1, 8, 16, &restore_phyData)) != RT_ERR_OK)
		        goto set_crossOver_failed;

		    phyData = restore_phyData & ~(1<<(12+port-base_port));
		    phyData |= (1<<(8+port-base_port));

		    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, phyData)) != RT_ERR_OK)
		        goto set_crossOver_failed;

            /* get value from CHIP*/
            if ((ret = hal_miim_write(unit, port, 7, 0x1e, 0x002d)) != RT_ERR_OK)
                goto set_crossOver_failed;
            if ((ret = hal_miim_read(unit, port, 7, 0x18, &phyData)) != RT_ERR_OK)
                goto set_crossOver_failed;
            phyData &= ~(1 << 5);
            phyData |= (force_mode << 5);
            if ((ret = hal_miim_write(unit, port, 7, 0x18, phyData)) != RT_ERR_OK)
                goto set_crossOver_failed;

            if ((ret = hal_miim_read(unit, port, 0, 0x10, &phyData)) != RT_ERR_OK)
                goto set_crossOver_failed;
            phyData &= ~(1 << 5);
            phyData |= (mdi << 5);
            if ((ret = hal_miim_write(unit, port, 0, 0x10, phyData)) != RT_ERR_OK)
                goto set_crossOver_failed;
set_crossOver_failed:
		    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, restore_phyData)) != RT_ERR_OK)
        		return ret;
			/*Power Up*/
			portEnable = ENABLED;
			phy_8214fb_enable_set(unit, port, portEnable);

			return ret;
        }
        else
        {
            /* cross over register is in copper page and can't access when fiber is linkup
             * If access via page 8 register 16 with force access copper PHY, it will cause
             * the fiber link down.
             */
            return RT_ERR_PHY_FIBER_LINKUP;
        }
    }

    if ((ret = hal_miim_read(unit, base_port+1, 8, 16, &restore_phyData)) != RT_ERR_OK)
        return ret;

    phyData = restore_phyData & ~(1<<(12+port-base_port));
    phyData |= (1<<(8+port-base_port));

    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, phyData)) != RT_ERR_OK)
        return ret;

    /* get value from CHIP*/
    if ((ret = hal_miim_write(unit, port, 7, 0x1e, 0x002d)) != RT_ERR_OK)
        return ret;
    if ((ret = hal_miim_read(unit, port, 7, 0x18, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(1 << 5);
    phyData |= (force_mode << 5);
    if ((ret = hal_miim_write(unit, port, 7, 0x18, phyData)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 0, 0x10, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(1 << 5);
    phyData |= (mdi << 5);
    if ((ret = hal_miim_write(unit, port, 0, 0x10, phyData)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_write(unit, base_port+1, 8, 16, restore_phyData)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fb_crossOverMode_set */

/* Function Name:
 *      phy_8214fb_fiber_media_get
 * Description:
 *      Get PHY 8212B/8214FB/8214B fiber media type.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      pMedia - pointer buffer of phy fiber media type
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
phy_8214fb_fiber_media_get(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t *pMedia)
{
    uint32  val;
    int32   ret = RT_ERR_FAILED;

    if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
        return ret;
    if (((val >> 11) & 0x1) == 0)
        *pMedia = PORT_FIBER_MEDIA_AUTO;
    else if (((val >> 10) & 0x1) == 0)
        *pMedia = PORT_FIBER_MEDIA_1000;
    else
        *pMedia = PORT_FIBER_MEDIA_100;

    return RT_ERR_OK;
} /* end of phy_8214fb_fiber_media_get */

/* Function Name:
 *      phy_8214fb_fiber_media_set
 * Description:
 *      Get PHY 8212B/8214FB/8214B fiber media type.
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      media - phy media fiber type
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      1. fiber media type value is as following:
 *      - PORT_FIBER_MEDIA_1000
 *      - PORT_FIBER_MEDIA_100
 *      - PORT_FIBER_MEDIA_AUTO
 */
int32
phy_8214fb_fiber_media_set(uint32 unit, rtk_port_t port, rtk_port_fiber_media_t media)
{
    uint32  val, phyReg28, phyReg16;
    uint32  base_port = 0;
    int32   ret = RT_ERR_FAILED;

    base_port = port - (port % PORT_NUM_IN_8214FB);

    switch (media)
    {
        case PORT_FIBER_MEDIA_1000:
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            if ((((val >> 11) & 0x1) == 1) && (((val >> 10) & 0x1) == 0))
            {
                /* no change fiber media */
                break;
            }
            /* Power Down the Fiber Port */
            if ((ret = hal_miim_read(unit, base_port+3, PHY_PAGE_8, 28, &phyReg28)) != RT_ERR_OK)
                return ret;
            val = (phyReg28 | (1<<12) | (1<<(port-base_port+8)));
            if ((ret = hal_miim_write(unit, base_port+3, PHY_PAGE_8, 28, val)) != RT_ERR_OK)
                return ret;
            /* Force Select Fiber Standard Register */
            if ((ret = hal_miim_read(unit, base_port+1, PHY_PAGE_8, 16, &phyReg16)) != RT_ERR_OK)
                return ret;
            val = (phyReg16 | (0x11<<(port-base_port+8)));
            if ((ret = hal_miim_write(unit, base_port+1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            /* Enable or Disable 100Base-FX/1000Base-X Auto Sensing */
            /* change to ext. page 19 */
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x0013)) != RT_ERR_OK)
                return ret;
            /* enable or disable 100Base-FX/1000Base-X auto sensing (enable: 0xE46A, disable: 0x646A) */
            if ((ret = hal_miim_write(unit, port, 15, 26, 0x646A)) != RT_ERR_OK)
                return ret;
            /* Fiber Interface Setting */
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(3<<10);
            val |= (2<<10);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            /* Fiber Capability Setting */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 0, 0x1140)) != RT_ERR_OK)
                return ret;
            /* Auto Mode Standard Register */
            if ((ret = hal_miim_read(unit, base_port+1, PHY_PAGE_8, 16, &phyReg16)) != RT_ERR_OK)
                return ret;
            val = (phyReg16 & (~(0x11<<(port-base_port+8))));
            if ((ret = hal_miim_write(unit, base_port+1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            /* Power On Fiber */
            if ((ret = hal_miim_read(unit, base_port+3, PHY_PAGE_8, 28, &phyReg28)) != RT_ERR_OK)
                return ret;
            val = (phyReg28 & (~(1<<12)) & (~(1<<(port-base_port+8))));
            if ((ret = hal_miim_write(unit, base_port+3, PHY_PAGE_8, 28, val)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_FIBER_MEDIA_1000 */

        case PORT_FIBER_MEDIA_100:
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            if ((((val >> 11) & 0x1) == 1) && (((val >> 10) & 0x1) == 1))
            {
                /* no change fiber media */
                break;
            }
            /* Power Down the Fiber Port */
            if ((ret = hal_miim_read(unit, base_port+3, PHY_PAGE_8, 28, &phyReg28)) != RT_ERR_OK)
                return ret;
            val = (phyReg28 | (1<<12) | (1<<(port-base_port+8)));
            if ((ret = hal_miim_write(unit, base_port+3, PHY_PAGE_8, 28, val)) != RT_ERR_OK)
                return ret;
            /* Force Select Fiber Standard Register */
            if ((ret = hal_miim_read(unit, base_port+1, PHY_PAGE_8, 16, &phyReg16)) != RT_ERR_OK)
                return ret;
            val = (phyReg16 | (0x11<<(port-base_port+8)));
            if ((ret = hal_miim_write(unit, base_port+1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            /* Enable or Disable 100Base-FX/1000Base-X Auto Sensing */
            /* change to ext. page 19 */
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x0013)) != RT_ERR_OK)
                return ret;
            /* enable or disable 100Base-FX/1000Base-X auto sensing (enable: 0xE46A, disable: 0x646A) */
            if ((ret = hal_miim_write(unit, port, 15, 26, 0x646A)) != RT_ERR_OK)
                return ret;
            /* Fiber Interface Setting */
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(3<<10);
            val |= (3<<10);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            /* Fiber Capability Setting */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 0, 0x2100)) != RT_ERR_OK)
                return ret;
            /* Auto Mode Standard Register */
            if ((ret = hal_miim_read(unit, base_port+1, PHY_PAGE_8, 16, &phyReg16)) != RT_ERR_OK)
                return ret;
            val = (phyReg16 & (~(0x11<<(port-base_port+8))));
            if ((ret = hal_miim_write(unit, base_port+1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            /* Power On Fiber */
            if ((ret = hal_miim_read(unit, base_port+3, PHY_PAGE_8, 28, &phyReg28)) != RT_ERR_OK)
                return ret;
            val = (phyReg28 & (~(1<<12)) & (~(1<<(port-base_port+8))));
            if ((ret = hal_miim_write(unit, base_port+3, PHY_PAGE_8, 28, val)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_FIBER_MEDIA_100 */

        case PORT_FIBER_MEDIA_AUTO:
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            if (((val >> 11) & 0x1) == 0)
            {
                /* no change fiber media */
                break;
            }
            /* Power Down the Fiber Port */
            if ((ret = hal_miim_read(unit, base_port+3, PHY_PAGE_8, 28, &phyReg28)) != RT_ERR_OK)
                return ret;
            val = (phyReg28 | (1<<12) | (1<<(port-base_port+8)));
            if ((ret = hal_miim_write(unit, base_port+3, PHY_PAGE_8, 28, val)) != RT_ERR_OK)
                return ret;
            /* Force Select Fiber Standard Register */
            if ((ret = hal_miim_read(unit, base_port+1, PHY_PAGE_8, 16, &phyReg16)) != RT_ERR_OK)
                return ret;
            val = (phyReg16 | (0x11<<(port-base_port+8)));
            if ((ret = hal_miim_write(unit, base_port+1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            /* Enable or Disable 100Base-FX/1000Base-X Auto Sensing */
            /* change to ext. page 19 */
            if ((ret = hal_miim_write(unit, port, 15, 30, 0x0013)) != RT_ERR_OK)
                return ret;
            /* enable or disable 100Base-FX/1000Base-X auto sensing (enable: 0xE46A, disable: 0x646A) */
            if ((ret = hal_miim_write(unit, port, 15, 26, 0xE46A)) != RT_ERR_OK)
                return ret;
            /* Fiber Interface Setting */
            if ((ret = hal_miim_read(unit, port, PHY_PAGE_8, 30, &val)) != RT_ERR_OK)
                return ret;
            val &= ~(3<<10);
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
                return ret;
            /* Fiber Capability Setting */
            if ((ret = hal_miim_write(unit, port, PHY_PAGE_8, 0, 0x1140)) != RT_ERR_OK)
                return ret;
            /* Auto Mode Standard Register */
            if ((ret = hal_miim_read(unit, base_port+1, PHY_PAGE_8, 16, &phyReg16)) != RT_ERR_OK)
                return ret;
            val = (phyReg16 & (~(0x11<<(port-base_port+8))));
            if ((ret = hal_miim_write(unit, base_port+1, PHY_PAGE_8, 16, val)) != RT_ERR_OK)
                return ret;
            /* Power On Fiber */
            if ((ret = hal_miim_read(unit, base_port+3, PHY_PAGE_8, 28, &phyReg28)) != RT_ERR_OK)
                return ret;
            val = (phyReg28 & (~(1<<12)) & (~(1<<(port-base_port+8))));
            if ((ret = hal_miim_write(unit, base_port+3, PHY_PAGE_8, 28, val)) != RT_ERR_OK)
                return ret;
            break; /* end of switch-case PORT_FIBER_MEDIA_100 */

        default:
            break;
    }

    return RT_ERR_OK;
} /* end of phy_8214fb_fiber_media_set */


/* Function Name:
 *      phy_8214fb_auto_1000f_get
 * Description:
 *      Get PHY 8212B/8214FB/8214B copper 1000f ability from shadow.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pAbility - pointer to copper 1000f ability
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. copper 1000f ability value is as following:
 *      - 0: not the ability
 *      - 1: have the ability
 */
int32
phy_8214fb_auto_1000f_get(uint32 unit, rtk_port_t port, uint32 *pAbility)
{
    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pPhy_info[unit]), RT_ERR_NULL_POINTER);

    *pAbility = (*pPhy_info[unit]).auto_1000f[port];
    return RT_ERR_OK;
} /* end of phy_8214fb_auto_1000f_get */

/* Function Name:
 *      phy_8214fb_auto_1000f_set
 * Description:
 *      Set PHY 8212B/8214FB/8214B copper 1000f ability to shadow.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      ability  - copper 1000f ability
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK           - OK
 *      RT_ERR_FAILED       - invalid parameter
 *      RT_ERR_INPUT        - invalid input parameter
 *      RT_ERR_NULL_POINTER - input parameter is null pointer
 * Note:
 *      1. copper 1000f ability value is as following:
 *      - 0: not the ability
 *      - 1: have the ability
 */
int32
phy_8214fb_auto_1000f_set(uint32 unit, rtk_port_t port, uint32 ability)
{
    RT_PARAM_CHK((ability != 0) && (ability != 1), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pPhy_info[unit]), RT_ERR_NULL_POINTER);

    (*pPhy_info[unit]).auto_1000f[port] = ability;
    return RT_ERR_OK;
} /* end of phy_8214fb_auto_1000f_set */

/* Function Name:
 *      phy_8214fb_linkDownPowerSavingEnable_get
 * Description:
 *      Get the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of link-down power saving
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. The RTL8214FB is supported the per-port link-down power saving.
 *      2. The function should be called when media is copper.
 *      3. Error code will be return when fiber media is link up.
 */
int32
phy_8214fb_linkDownPowerSavingEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    uint32  phyData;
    uint32  val, phyExtStatus15;
    uint32  is_fiber_linkup, is_copper_linkup;
    uint32  restore_val;
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;

    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        if (is_copper_linkup)
        {
            /* getting page 0, register 21, bit[12] for link-down power saving state */
            if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
                return ret;
            if (((phyData >> 12) & 0x1) == 0x1)
                *pEnable = ENABLED;
            else
                *pEnable = DISABLED;
            return RT_ERR_OK;
        }
        else
        {
            /* return the value from shadow when fiber is linkup. */
            *pEnable = (*pPhy_info[unit]).ldps_state[port];
            return RT_ERR_OK;
        }
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    /* Link-down */
    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        val &= ~(0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
    }

    /* getting page 0, register 21, bit[12] for link-down power saving state */
    if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
        return ret;
    if (((phyData >> 12) & 0x1) == 0x1)
        *pEnable = ENABLED;
    else
        *pEnable = DISABLED;

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;

    return RT_ERR_OK;
} /* end of phy_8214fb_linkDownPowerSavingEnable_get */

/* Function Name:
 *      phy_8214fb_linkDownPowerSavingEnable_set
 * Description:
 *      Set the status of link-down power saving of the specific port in the specific unit
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - status of link-down power saving
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_PORT_ID - invalid port id
 * Note:
 *      1. The RTL8214FB is supported the per-port link-down power saving.
 *      2. The function should be called when media is copper.
 *      3. Error code will be return when fiber media is link up.
 */
int32
phy_8214fb_linkDownPowerSavingEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32  phyData;
    uint32  val, phyExtStatus15;
    uint32  is_fiber_linkup, is_copper_linkup;
    uint32  restore_val;
    int32   ret = RT_ERR_FAILED;
    hal_control_t   *pHalCtrl;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
        return RT_ERR_FAILED;

    ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val);
    if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, PHY_STATUS_REG, &val)) != RT_ERR_OK)
        return ret;

    if (val & LinkStatus_MASK)
    {
        if ((ret = hal_miim_read(unit, port, PHY_PAGE_0, 15, &phyExtStatus15)) != RT_ERR_OK)
            return ret;
        is_fiber_linkup = (phyExtStatus15 >> 15) & 0x1;
        is_copper_linkup = (phyExtStatus15 >> 13) & 0x1;

        if (is_copper_linkup)
        {
            /* get value from CHIP*/
            if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
                return ret;
            if (ENABLED == enable)
            {
                phyData |= (0x1 << 12);
            }
            else
            {
                phyData &= ~(0x1 << 12);
            }
            if ((ret = hal_miim_write(unit, port, 0, 21, phyData)) != RT_ERR_OK)
                return ret;

            /* Update shadow value */
            if (ENABLED == enable)
            {
                (*pPhy_info[unit]).ldps_state[port] = ENABLED;
            }
            else
            {
                (*pPhy_info[unit]).ldps_state[port] = DISABLED;
            }

            return RT_ERR_OK;
        }
        else
        {
            /* register is in copper page and can't access when fiber is linkup. */
            return RT_ERR_PHY_FIBER_LINKUP;
        }
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_read(unit, port, PHY_PAGE_8, 30, &restore_val)) != RT_ERR_OK)
        return ret;

    /* Link-down */
    if ((restore_val & 0x2000) == 0x0000)
    {
        val = (restore_val | 0x2000);
        val &= ~(0x1000);

        if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, val)) != RT_ERR_OK)
            return ret;
    }

    /* For Link-Down Power Saving (per-port) */
    if ((ret = hal_miim_read(unit, port, 0, 21, &phyData)) != RT_ERR_OK)
        return ret;
    phyData &= ~(0x1 << 12);
    if (ENABLED == enable)
    {
        phyData |= (0x1 << 12);
    }
    if ((ret = hal_miim_write(unit, port, 0, 21, phyData)) != RT_ERR_OK)
        return ret;

    /* Update shadow value */
    if (ENABLED == enable)
    {
        (*pPhy_info[unit]).ldps_state[port] = ENABLED;
    }
    else
    {
        (*pPhy_info[unit]).ldps_state[port] = DISABLED;
    }

    if ((ret = MACDRV(pHalCtrl)->fMdrv_miim_write(unit, port, PHY_PAGE_8, 30, restore_val)) != RT_ERR_OK)
        return ret;

    return ret;
} /* end of phy_8214fb_linkDownPowerSavingEnable_set */

/* Function Name:
 *      phy_8214fb_gigaLiteEnable_get
 * Description:
 *      Get the status of Giga Lite of the specific port in the specific unit
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to status of Giga Lite
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      1. The RTL8214fb is not supported the per-port Giga Lite feature.
 */
int32
phy_8214fb_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    *pEnable = DISABLED;
    return RT_ERR_OK;
} /* end of phy_8214fb_gigaLiteEnable_get */

/* Function Name:
 *      phy_8214fb_patch_set
 * Description:
 *      Set patch to PHY.
 * Input:
 *      unit   - unit id
 *      port   - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_UNIT_ID  - invalid unit id
 *      RT_ERR_PORT_ID  - invalid port id
 *      RT_ERR_INPUT    - invalid input parameter
 * Note:
 *      None
 */
int32
phy_8214fb_patch_set(uint32 unit, rtk_port_t port)
{
    int32 ret;
    ret = sub_phy_8214fb_patch_set(unit, port);
    return ret;
} /* end of phy_8214fb_patch_set */



/* Function Name:
 *      phy_8214fb_fiberOAMLoopBack_set
 * Description:
 *      Set Fiber-Port OAM Loopback feature,
 * Input:
 *      unit  - unit id
 *      port  - port id
 *      enable - Fiber-Port OAM Loopback feature
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8214fb_fiberOAMLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  phy_data;
    uint32  reg_val;
    int32    ret;

    /*Backup Media*/
    if ((ret = hal_miim_read(unit, port, 8, 30, &phy_data)) != RT_ERR_OK)
        return ret;

    /*Force Fiber*/
    reg_val = phy_data | (0x3<<12);
    if ((ret = hal_miim_write(unit, port, 8, 30, reg_val)) != RT_ERR_OK)
        return ret;

    if ((ret = hal_miim_read(unit, port, 0, 0, &reg_val)) != RT_ERR_OK)
        return ret;

    if(enable == ENABLED)
        reg_val |= (1UL<<14);
    else
        reg_val &= ~(1UL<<14);

    if ((ret = hal_miim_write(unit, port, 0, 0, reg_val)) != RT_ERR_OK)
        return ret;

    /*Restore  Media*/
    if ((ret = hal_miim_write(unit, port, 8, 30, phy_data)) != RT_ERR_OK)
        return ret;


    if(enable == ENABLED)
    {
	    /*Delay until PHY Linkup*/
	    osal_time_usleep(100 * 1000); /* delay 10mS */
    }

    return ret;
}   /* end of phy_8214fb_fiberOAMLoopBack_set */


