/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * Purpose : RTL83xx SOC and SWCORE commands for U-Boot.
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <rtk_osal.h>
#include <config.h>
#include <common.h>
#include <command.h>
#include <common/util.h>

#if defined(CONFIG_RTL8328)
#include <rtk/mac/rtl8328/rtl8328_rtk.h>
#endif

#if defined(CONFIG_RTL8380)
#include <rtk/mac/rtl8380/rtl8380_rtk.h>
#endif

#if defined(CONFIG_RTL8390)
#include <rtk/mac/rtl8390/rtl8390_rtk.h>
#endif

#ifdef CONFIG_RTL8214FC
#include <rtk/phy/rtl8214fc.h>
#endif

#include <rtk/drv/gpio/ext_gpio.h>
#include <rtk/drv/smi/smi.h>
#include <rtk/drv/gpio/gpio.h>
#include <rtk/drv/poe/pd69100.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */

/*
 * Macro Definition
 */
#ifndef REG32
#define REG32(reg)   (*(volatile unsigned int *)((unsigned int)reg))
#endif
#ifndef PORTMASK_COUNT
#define PORTMASK_COUNT(port_count)  ((0x1 << (port_count)) - 1)
#endif


/*
 * Function Declaration
 */

int32 is_ctrlc(void)
{
	if (tstc()) {
		switch (getc ()) {
		case 0x03:		/* ^C - Control C */
			return 1;
		default:
			break;
		}
	}
	return 0;
}

void _phy_testmode(unsigned int port, unsigned int mode, unsigned int chn)
{
    unsigned int phyid;
    unsigned int phy0_macid;
    unsigned int val;
    int          portIdx;


    rtk_portIdxFromMacId(port, &portIdx);
    phyid = gSwitchModel->port.list[portIdx].phy;
    phy0_macid = gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].mac_id;

    printf("test mode: %d\n", mode);
    printf("test port: %d [PHY%01d]\n", port, phyid);



    switch (gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].chip)
    {
    case RTK_CHIP_RTL8214FB:
    case RTK_CHIP_RTL8214B:
    case RTK_CHIP_RTL8212B:
        {
            printf("RTL8214FB/RTL8214B/RTL8212B Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

            switch (mode)
            {
            case 1:
                {
                    /* Disable LDPS */
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // disable LDPS
                    /* Test Mode 1 */
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,0,0x1140);  // power on PHY
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // test mode 1
                    OSAL_MDELAY(100);    /* delay 100mS and wait for Mirco-P completed */
                    /* Adjust Amplitude */
                    gMacDrv->drv_miim_write(phy0_macid+phyid,2,21,0xAA00); // adjust amplitude
                }
                break;

            case 4:
                {
                    /* Disable LDPS */
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // disable LDPS
                    /* Test Mode 4 */
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,0,0x1140);  // power on PHY
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00);  // Enable Test mode 4
                    OSAL_MDELAY(100);    /* delay 100mS and wait for Mirco-P completed */
                    /* Adjust LDVbias */
                    gMacDrv->drv_miim_write(phy0_macid+phyid,2,5,0xCE68);  // adjust LDVbias
                }
                break;

            default:
                printf("The mode (%d) is not be suppoted yet.\n", mode);
                break;
            }
        }
        break;

    case RTK_CHIP_RTL8214:
    case RTK_CHIP_RTL8214F:
        {
            printf("RTL8214/RTL8214F Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

            switch (mode)
            {
            case 1:
                {
                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Set Test mode: normal
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Set Test mode: normal
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Set Test mode: normal
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Set Test mode: normal

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // Disable Power-Saving mode
                    gMacDrv->drv_miim_write(phy0_macid+phyid,2,21,0xAA00); // Amp+
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // Enable Test mode 1
                }
                break;

            case 4:
                {
                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Set Test mode: normal
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Set Test mode: normal
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Set Test mode: normal
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Set Test mode: normal

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,21,0x0006); // Disable Power-Saving mode
                    gMacDrv->drv_miim_write(phy0_macid+phyid,2,21,0x5500); // Set back to normal value
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00);  // Enable Test mode 4
                }
                break;

            default:
                printf("The mode (%d) is not be suppoted yet.\n", mode);
                break;
            }
        }
        break;

    case RTK_CHIP_RTL8212F:
        {
            unsigned int val;
            printf("RTL8212F Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

            switch (mode)
            {
            case 1:
                {
                    gMacDrv->drv_miim_read(phy0_macid+0, 0, 9, (unsigned int *)&val);
                    val &= ~(7<<13);
                    gMacDrv->drv_miim_write(phy0_macid+0, 0, 9, val);  // Set Test mode: normal
                    gMacDrv->drv_miim_read(phy0_macid+1, 0, 9, (unsigned int *)&val);
                    val &= ~(7<<13);
                    gMacDrv->drv_miim_write(phy0_macid+1, 0, 9, val);  // Set Test mode: normal

                    gMacDrv->drv_miim_read(phy0_macid+phyid, 0, 9, (unsigned int *)&val);
                    val |= (1 << 13);  // Enable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+phyid, 0, 9, val);  // Enable Test mode 1

                }
                break;

            case 4:
                {
                    gMacDrv->drv_miim_read(phy0_macid+0, 0, 9, (unsigned int *)&val);
                    val &= ~(7<<13);
                    gMacDrv->drv_miim_write(phy0_macid+0, 0, 9, val);  // Set Test mode: normal
                    gMacDrv->drv_miim_read(phy0_macid+1, 0, 9, (unsigned int *)&val);
                    val &= ~(7<<13);
                    gMacDrv->drv_miim_write(phy0_macid+1, 0, 9, val);  // Set Test mode: normal

                    gMacDrv->drv_miim_read(phy0_macid+phyid, 0, 9, (unsigned int *)&val);
                    val |= (4 << 13);  // Enable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+phyid, 0, 9,val);  // Enable Test mode 4
                }
                break;

            default:
                printf("The mode (%d) is not be suppoted yet.\n", mode);
                break;
            }
        }
        break;

    case RTK_CHIP_RTL8218:
        {
            printf("RTL8218 Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

            /* recovery test mode 2 setting to default */
            gMacDrv->drv_miim_write(phy0_macid+1,2,0x11,0x7E00);
            gMacDrv->drv_miim_write(phy0_macid+1,0,0x1f,0x0000);
            gMacDrv->drv_miim_write(phy0_macid+4,2,0x11,0x7E00);
            gMacDrv->drv_miim_write(phy0_macid+4,0,0x1f,0x0000);

            switch (mode)
            {
            case 1:
                {
                    gMacDrv->drv_miim_write(phy0_macid+4,2,31,0x0002);  // Page 2
                    gMacDrv->drv_miim_write(phy0_macid+4,2,19,0xAA00);  // PHYReg wi 4 19 0xAA00, PHY 0 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,2,20,0xAA00);  // PHYReg wi 4 20 0xAA00, PHY 1 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,2,21,0xAA00);  // PHYReg wi 4 21 0xAA00, PHY 2 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,2,22,0xFA00);  // PHYReg wi 4 22 0xFA00, PHY 3 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,2,23,0xAF00);  // PHYReg wi 4 23 0xAF00, PHY 4 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,2,24,0xAA00);  // PHYReg wi 4 24 0xAA00, PHY 5 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,2,25,0xAA00);  // PHYReg wi 4 25 0xAA00, PHY 6 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,2,26,0xAA00);  // PHYReg wi 4 26 0xAA00, PHY 7 ==> Giga / 10M +2.5%
                    gMacDrv->drv_miim_write(phy0_macid+4,0,31,0x0000);  // Page 0

                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode 1
                    gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode 1

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // Test mode 1
                }
                break;

            case 2:
                {
                    /* change the parameters for test mode 2 */
                    gMacDrv->drv_miim_write(phy0_macid+1,2,0x11,0x5E00);
                    gMacDrv->drv_miim_write(phy0_macid+1,0,0x1f,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+4,2,0x11,0x5E00);
                    gMacDrv->drv_miim_write(phy0_macid+4,0,0x1f,0x0000);

                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x4E00);  // Test mode 2
                }
                break;

            case 3:
                {
                    printf(" test channel: %d\n", chn);

                    if (phyid < 4)
                    {
                        //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+1, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                        gMacDrv->drv_miim_write(phy0_macid+1, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                        //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+1, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                        gMacDrv->drv_miim_write(phy0_macid+1, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                    } else {
                        //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+4, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                        gMacDrv->drv_miim_write(phy0_macid+4, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                        //printf("PHY:%d, page:%d, reg:%d, val:0x%04X\n", phy0_macid+4, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                        gMacDrv->drv_miim_write(phy0_macid+4, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                    }

                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x6E00);  // Test mode 3
                }
                break;

            case 4:
                {
                    gMacDrv->drv_miim_write(phy0_macid+0,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY0)
                    gMacDrv->drv_miim_write(phy0_macid+0,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+1,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY1)
                    gMacDrv->drv_miim_write(phy0_macid+1,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+2,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY2)
                    gMacDrv->drv_miim_write(phy0_macid+2,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+3,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY3)
                    gMacDrv->drv_miim_write(phy0_macid+3,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+4,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY4)
                    gMacDrv->drv_miim_write(phy0_macid+4,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+5,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY5)
                    gMacDrv->drv_miim_write(phy0_macid+5,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+6,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY6)
                    gMacDrv->drv_miim_write(phy0_macid+6,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode 4
                    gMacDrv->drv_miim_write(phy0_macid+7,2,7,0x3678);  // Page 2 Reg 7 = 0x3678, Ldvbias = 4 (PHY7)
                    gMacDrv->drv_miim_write(phy0_macid+7,0,31,0x0000); // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode 4

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,31,0x0000);  // Page 0
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00);  // PHYReg wi 0 9 0x8E00, PHY0 Test mode 4
                }
                break;

            default:
                printf("The mode (%d) is not be suppoted yet.\n", mode);
                break;
            }
        }
        break;

    case RTK_CHIP_RTL8214FC:
    case RTK_CHIP_RTL8218B:
    case RTK_CHIP_RTL8218FB:
        {
            printf("RTL8218 Test Mode (PHYID: %d + %d)\n", phy0_macid, phyid);

            switch (mode)
            {
            case 1:
                {

                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,18,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,19,0x01c0);
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x2E00);  // Test mode 1
                }
                break;

            case 2:
                {

                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,18,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,19,0x01c0);
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x4E00);  // Test mode 2
                }
                break;

            case 3:
                {
                    printf(" test channel: %d\n", chn);


                    if (phyid < 4)
                    {
                        gMacDrv->drv_miim_write(phy0_macid+1,0xbc0,18,0x0000);
                        gMacDrv->drv_miim_write(phy0_macid+1,0xbc0,19,0x01c0);
                        gMacDrv->drv_miim_write(phy0_macid+1, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                        gMacDrv->drv_miim_write(phy0_macid+1, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                    } else {
                        gMacDrv->drv_miim_write(phy0_macid+4,0xbc0,18,0x0000);
                        gMacDrv->drv_miim_write(phy0_macid+4,0xbc0,19,0x01c0);
                        gMacDrv->drv_miim_write(phy0_macid+4, 2, 17, 0x4000 | ((phyid & 0x7) << 9));
                        gMacDrv->drv_miim_write(phy0_macid+4, 2, 16, 0x1100 | ((chn & 0x3) << 9));
                    }

                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                    if (RTK_CHIP_RTL8214FC != gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].chip)
                    {
                        gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                        gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                        gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                        gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode
                    }

                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,18,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,19,0x01c0);
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x6E00);  // Test mode 3
                }
                break;

            case 4:
                {
                    gMacDrv->drv_miim_write(phy0_macid+0,0xbc0,18,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+0,0xbc0,19,0x01c0);
                    gMacDrv->drv_miim_write(phy0_macid+0,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+1,0xbc0,18,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+1,0xbc0,19,0x01c0);
                    gMacDrv->drv_miim_write(phy0_macid+1,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+2,0xbc0,18,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+2,0xbc0,19,0x01c0);
                    gMacDrv->drv_miim_write(phy0_macid+2,0,9,0x0E00);  // Disable Test mode
                    gMacDrv->drv_miim_write(phy0_macid+3,0xbc0,18,0x0000);
                    gMacDrv->drv_miim_write(phy0_macid+3,0xbc0,19,0x01c0);
                    gMacDrv->drv_miim_write(phy0_macid+3,0,9,0x0E00);  // Disable Test mode
                    if (RTK_CHIP_RTL8214FC != gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].chip)
                    {
                        gMacDrv->drv_miim_write(phy0_macid+4,0xbc0,18,0x0000);
                        gMacDrv->drv_miim_write(phy0_macid+4,0xbc0,19,0x01c0);
                        gMacDrv->drv_miim_write(phy0_macid+4,0,9,0x0E00);  // Disable Test mode
                        gMacDrv->drv_miim_write(phy0_macid+5,0xbc0,18,0x0000);
                        gMacDrv->drv_miim_write(phy0_macid+5,0xbc0,19,0x01c0);
                        gMacDrv->drv_miim_write(phy0_macid+5,0,9,0x0E00);  // Disable Test mode
                        gMacDrv->drv_miim_write(phy0_macid+6,0xbc0,18,0x0000);
                        gMacDrv->drv_miim_write(phy0_macid+6,0xbc0,19,0x01c0);
                        gMacDrv->drv_miim_write(phy0_macid+6,0,9,0x0E00);  // Disable Test mode
                        gMacDrv->drv_miim_write(phy0_macid+7,0xbc0,18,0x0000);
                        gMacDrv->drv_miim_write(phy0_macid+7,0xbc0,19,0x01c0);
                        gMacDrv->drv_miim_write(phy0_macid+7,0,9,0x0E00);  // Disable Test mode
                    }


                    gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00); // PHYReg wi 0 9 0x8E00, PHY0 Test mode 4
                    val = 0x11 << chn;
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,18, val);
                    gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,19,0x0150);

                }
                break;

            default:
                printf("The mode (%d) is not be suppoted yet.\n", mode);
                break;
            }
        }
        break;

    default:
        printf("This chip of the port is not support the test mode yet.\n");
        break;
    }
}

void _phy_testmode_allPort(unsigned int mode, unsigned int chn)
{
    unsigned int phyid;
    unsigned int phy0_macid;
    unsigned int val;
    unsigned int portIdx;

    printf("Test Mode: %d   port: all  channel: %d\n", mode, chn);

    /* Disable Test mode */
    for (portIdx = 0; portIdx < gSwitchModel->port.count; portIdx++)
    {
        phyid = gSwitchModel->port.list[portIdx].phy;
        phy0_macid = gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].mac_id;

        switch (gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].chip)
        {
        case RTK_CHIP_RTL8214FC:
        case RTK_CHIP_RTL8218B:
        case RTK_CHIP_RTL8218FB:
            if (4 == mode)
            {
                gMacDrv->drv_miim_write(phy0_macid+phyid, 0xbc0,18,0x0000);
                gMacDrv->drv_miim_write(phy0_macid+phyid, 0xbc0,19,0x01c0);
                gMacDrv->drv_miim_write(phy0_macid+phyid, 0,9,0x0E00);  // Disable Test mode
            }
            else
            {
                printf("Not support all port configuring in mode:%d\n", mode);
                return;
            }
            break;
        default:
            printf("Not support all port configuring in this PHY\n");
            return;
        }
    }


    /* Enable Test mode */
    for (portIdx = 0; portIdx < gSwitchModel->port.count; portIdx++)
    {
        phyid = gSwitchModel->port.list[portIdx].phy;
        phy0_macid = gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].mac_id;

        switch (gSwitchModel->phy.list[gSwitchModel->port.list[portIdx].phy_idx].chip)
        {
        case RTK_CHIP_RTL8214FC:
        case RTK_CHIP_RTL8218B:
        case RTK_CHIP_RTL8218FB:
            if (4 == mode)
            {
                gMacDrv->drv_miim_write(phy0_macid+phyid,0,9,0x8E00); // PHYReg wi 0 9 0x8E00, PHY0 Test mode 4
                val = 0x11 << chn;
                gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,18, val);
                gMacDrv->drv_miim_write(phy0_macid+phyid,0xbc0,19,0x0150);
            }
            break;
        default:
            printf("Not support all port configuring in this PHY\n");
            return;
        }
    }

}

#define PORT_LOOPBACK_TEST_PKT_MAX_LEN (1500)
uint8 pkt[] = { /* BPDU*/
    0x01, 0x80, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x44, 0x33, 0x55, 0xaa,
    0xcc, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
    0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
    0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0xff, 0x00,
    };
uint8 pkt_recv[PORT_LOOPBACK_TEST_PKT_MAX_LEN];
void rkt_printPkt(uint8 *p, uint32 length)
{
    uint32 i=0;
    printf("length=%d\n",length);
    while(i<(length-4)){
        printf("%02x",p[i++]);
    }
    printf("\n");
}

extern void NetInit(void);
extern int NetRecvPacket(volatile uchar ** pkt, int max_len,int *port);

void pkt_buffClean(void)
{
    int rx_pkt_len=0,pkt_clear=0,pkt_port;
    uint8 *recv_pkt;

    recv_pkt = malloc(PORT_LOOPBACK_TEST_PKT_MAX_LEN);
    if(recv_pkt==0){
        printf("allocate memory failed.(%s)\n",__FUNCTION__);
        return;
    }
    while(pkt_clear==0){
        rx_pkt_len = NetRecvPacket((volatile uchar ** )&recv_pkt,PORT_LOOPBACK_TEST_PKT_MAX_LEN,&pkt_port);
        if (rx_pkt_len==0)
            pkt_clear=1;
        else
            printf("clear garbage packet,port%d, size = %d\n",pkt_port,rx_pkt_len);
    }
    if(recv_pkt!=0)
        free(recv_pkt);
    return;
}

int32 rtk_port_loopback_test(int internal, uint32 port_start,uint32 port_end,int req_round)
{
    int     ret = 0;
    uint32  i;
    int32   link;
    int     port,pkt_port,rx_port,tx_port;
    uint32  rx_pkt_len=0;
    uint8   *recv_pkt = NULL;
    int     all_linkup;
    int     round;

    if (gSwitchModel == NULL)
        return 1;

    NetInit();
    rtk_network_off();
    rtk_saLearning(0);

    for(port=port_start;port<port_end;port++)
    {
        rtk_phyPortPowerOn(port);
        if(internal)
            rtk_phy_selfLoop_on(port);
        rtk_portIsolationToCPU(port);
    }
    udelay(2000000);

    pkt_buffClean();

    all_linkup=1;
    printf("Check port link status:\n");
    for(port=port_start;port<port_end;port++)
    {
        for(i=0;i<0xF;i++)
        {
            rtk_portlink_get(0,port,&link);
            if(link == 0x1)
            {
                printf("%d,",port);
                break;
            }
            udelay(100000);
        }

        if(i == 0xF)
        {
            printf("\nport %2d LINK DOWN!\n",port);
            all_linkup=0;
        }

        if (is_ctrlc())
        {
            printf("\n");
            goto test_end;
        }
    }
    printf("\n");
    if(all_linkup==0){
        printf("There are some ports link-down!\n");
        goto test_end;
    }

    recv_pkt = malloc(PORT_LOOPBACK_TEST_PKT_MAX_LEN);
    if(recv_pkt==0){
        printf("allocate memory failed.\n");
        goto test_end;
    }

    round=1;
    while((req_round==0)||(round<=req_round)){
        printf("ROUND(%d)\n",round);

        for(tx_port=port_start ; tx_port<port_end ; tx_port++){

            rtk_portIsolationCPUgoto(tx_port);

            if(internal){
                rx_port = tx_port;
            }else{
                rx_port = (tx_port%2)?tx_port-1:tx_port+1;
                rtk_portIsolation(tx_port,rx_port);
            }


            NetInit();
            NetSendPacket(pkt,sizeof(pkt));
            printf("port%2d tx, ",tx_port);

            rx_pkt_len = NetRecvPacket((volatile uchar ** )&recv_pkt,PORT_LOOPBACK_TEST_PKT_MAX_LEN,&pkt_port);
            eth_halt();

            if(rx_pkt_len != 0)
            {

                ret = OSAL_MEMCMP(pkt,recv_pkt,(rx_pkt_len-4));

                if(ret == 0){
                    if(pkt_port!=rx_port){
                        printf("Wrong Receiving port!\n");
                        rkt_printPkt(recv_pkt,rx_pkt_len);
                        goto test_end;
                    }
                    printf("port%2d rx, ",pkt_port);
              		printf("PASS!\n");
                }else{
                    printf("Wrong packet content!\n");
                    rkt_printPkt(recv_pkt,rx_pkt_len);
                    goto test_end;
                }
            }else{
                printf("Receive no packet!\n");
                goto test_end;
            }

            if(!internal){
                rtk_portIsolationToCPU(tx_port);
            }

            if (is_ctrlc())
            {
                printf("\n");
                goto test_end;
            }

        }
        round++;
    }
    pkt_buffClean();

test_end:
    rtk_port_isolation_off();
    if(recv_pkt!=0)
        free(recv_pkt);
    if(internal){
        for(port=port_start;port<port_end;port++){
            rtk_phy_selfLoop_off(port);
        }
    }

    rtk_saLearning(1);
    printf("Network on...\n");
    rtk_network_on();


    return 0;
}

/* Function Name:
 *      do_rtk
 * Description:
 *      Main function of RTK commands.
 * Input:
 *      cmdtp, flag, argc, argv handled by the parser
 * Output:
 *      None
 * Return:
 *      [FIXME]
 * Note:
 *      None
 */
int do_rtk(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
    unsigned int pinNum;
    unsigned int pinStatus;
    int index;
    int port,led_index;
    unsigned int clk_pin,data_pin,data;
    if (argc < 2)
    {
        goto usage;
    }

    /* default */
    if (0 == strcmp(argv[1], "default"))
    {
        printf("Resetting the confinguration ...\n");
        rtk_default();
        return 0;
    }

    /* network */
    if (0 == strcmp(argv[1], "network"))
    {
        if (0 == strcmp(argv[2], "on"))        /* network on */
        {
            printf("Enable network\n");
            rtk_network_on();

            return 0;
        }
        else if (0 == strcmp(argv[2], "off"))  /* network off */
        {
            printf("Disable network\n");
            rtk_network_off();

            return 0;
        }

        goto usage;
    }

#if defined(CONFIG_RTL8328)
    if (0 == strcmp(argv[1], "linkdown-patch")) /* For RTL8328M only */
    {
        rtk_linkdown_powersaving_patch();

        return 0;
    }
#endif

#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || (defined(CONFIG_RTL8214F)) \
    || (defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB)))
    /* comboport */
    if (0 == strcmp(argv[1], "comboport"))
    {
        if (0 == strcmp(argv[2], "copper"))        /* comboport copper */
        {
            printf("ComboPort: Copper mode\n");
            rtk_comboport_copper();

            return 0;
        }
        else if (0 == strcmp(argv[2], "fiber"))  /* comboport fiber */
        {
            printf("ComboPort: Fiber mode\n");
            rtk_comboport_fiber();

            return 0;
        }
#if defined(CONFIG_RTL8390)
        else if (0 == strcmp(argv[2], "auto"))
        {
            printf("ComboPort: Auto mode\n");
            rtk_comboport_auto();
            return 0;
        }
        else if (0 == strcmp(argv[2], "port"))  /* per port */
        {
            int portid = simple_strtoul(argv[3], NULL, 10);
            if (0 == strcmp(argv[4], "copper"))        /* comboport copper */
            {
                printf("ComboPort %d: Copper mode\n", portid);
                rtk_comboport_portcopper(portid);

                return 0;
            }
            else if (0 == strcmp(argv[4], "fiber"))  /* comboport fiber */
            {
                printf("ComboPort %d: Fiber mode\n", portid);
                rtk_comboport_portfiber(portid);

                return 0;
            }
        }
#endif
        goto usage;
    }
#endif  /* (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || (defined(CONFIG_RTL8214F)) \
    || (defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB))) */

    /* phyreg get portid page reg */
    if ((0 == strcmp(argv[1], "phyreg")) && (0 == strcmp(argv[2], "get")))
    {
        int portid = simple_strtoul(argv[3], NULL, 10);
        int page = simple_strtoul(argv[4], NULL, 10);
        int reg = simple_strtoul(argv[5], NULL, 10);
        unsigned int val;

        gMacDrv->drv_miim_read(portid, page, reg, &val);
        printf("Get Port %02d page %02d reg %02d val: 0x%04X \n", portid, page, reg, val);

        return 1;
    }

    /* phyreg get portid page reg */
    if ((0 == strcmp(argv[1], "phyreg")) && (0 == strcmp(argv[2], "dump-top")))
    {
        #ifdef CONFIG_RTL8214FC
        int portid = simple_strtoul(argv[3], NULL, 10);

        rtk_8214fc_dumpTop(portid);
        #endif

        return 1;
    }

    /* phyreg set */
    if ((0 == strcmp(argv[1], "phyreg")) && (0 == strcmp(argv[2], "set")))
    {
        int portid = simple_strtoul(argv[3], NULL, 10);
        int page = simple_strtoul(argv[4], NULL, 10);
        int reg = simple_strtoul(argv[5], NULL, 10);
        int val = simple_strtoul(argv[6], NULL, 10);

        gMacDrv->drv_miim_write(portid, page, reg, val);
        printf("Set Port %02d page %02d reg %02d val: 0x%04X \n", portid, page, reg, val);

        return 1;
    }

    /* phyreg setbymask */
    if ((strcmp(argv[1], "phyreg") == 0) && (strcmp(argv[2], "setbymask") == 0)) {
        unsigned int port_mask = simple_strtoul(argv[3], NULL, 10);
        int page = simple_strtoul(argv[4], NULL, 10);
        int reg = simple_strtoul(argv[5], NULL, 10);
        int val = simple_strtoul(argv[6], NULL, 10);

        gMacDrv->drv_miim_portmask_write(port_mask, page, reg, val);
        printf("Set PortMask %08x page %02d reg %02d val: 0x%04X \n", port_mask, page, reg, val);

        return 1;
    }

    /* testmode */
    if (strcmp(argv[1], "testmode") == 0)
    {
        unsigned int mode;
        unsigned int port;
        unsigned int chn;
        int          portIdx;

        if (argc < 4)
        {
            printf("Usage: testmode <mode> <port> [channel]\n channel: 0=A,1=B,2=C,3=D\n\n");
        }
        else
        {
            mode = simple_strtoul(argv[2], NULL, 10);
            port = simple_strtoul(argv[3], NULL, 10);
            chn = (argc < 5)? 0 : simple_strtoul(argv[4], NULL, 10);

            if (0 == strcmp(argv[3], "all"))
            {
                _phy_testmode_allPort(mode, chn);

                return 1;
            }

            if (rtk_portIdxFromMacId(port, &portIdx) < 0)
            {
                printf("The port (%d) is invalid.\n", port);
            }
            else
            {
                _phy_testmode(port, mode, chn);
            }
        }

        return 1;
    }

#ifdef CONFIG_EEE
    /* eee */
    if (0 == strcmp(argv[1], "eee"))
    {
        if (0 == strcmp(argv[2], "on"))        /* eee on */
        {
            printf("Enable EEE function\n");
            rtk_eee_on(gSwitchModel);

            return 0;
        }
        else if (0 == strcmp(argv[2], "off"))  /* eee off */
        {
            printf("Disable EEE function\n");
            rtk_eee_off(gSwitchModel);

            return 0;
        }

        goto usage;
    }
#endif

#if defined(CONFIG_RTL8328) || defined(CONFIG_RTL8380)
    if ((0 == strcmp(argv[1], "l2-testmode")) && (0 == strcmp(argv[2], "on")))
    {
        rtk_l2testmode_on();
        return 0;
    }

    if ((0 == strcmp(argv[1], "l2-testmode")) && (0 == strcmp(argv[2], "off")))
    {
        rtk_l2testmode_off();
        return 0;
    }
#endif

#if defined(CONFIG_SOFTWARE_CONTROL_LED)
    if (0 == strcmp(argv[1], "software-control-led"))
    {
        if (0 == strcmp(argv[2], "on"))
        {
            printf("Enable Software LED Control function\n");
            rtk_softwareControlLed_on();

            return 0;
        }
        else if (0 == strcmp(argv[2], "off"))
        {
            printf("Disable Software LED Control function\n");
            rtk_softwareControlLed_off();

            return 0;
        }

        goto usage;
    }
#endif

#if  defined(CONFIG_RTL8380)
    if ((0 == strcmp(argv[1], "port-isolation")) && (0 == strcmp(argv[2], "on")))
    {
        rtk_port_isolation_on();
        return 0;
    }

    if ((0 == strcmp(argv[1], "port-isolation")) && (0 == strcmp(argv[2], "off")))
    {
        rtk_port_isolation_off();
        return 0;
    }
#endif

#if defined(CONFIG_RTL8390)
    if ((0 == strcmp(argv[1], "sfp-speed")) && (0 == strcmp(argv[2], "set")))
    {
        int port = simple_strtoul(argv[3], NULL, 10);
        int speed = simple_strtoul(argv[4], NULL, 10);
        rtk_sfp_speed_set(port, speed);
        return 0;
    }
    else if ((0 == strcmp(argv[1], "sys-esd")))
    {
        if (0 == strcmp(argv[2], "on"))
            rtk_sysEsd_set(1);
        else
            rtk_sysEsd_set(0);

        return 0;
    }
    else if ((0 == strcmp(argv[1], "parameter")) && (0 == strcmp(argv[2], "version")))
    {
        printf("V1.1\n");
        printf("V1.2\n");
        printf("update SYS ESD\n");
        printf("update green in short cable\n");
        printf("add SYS ESD command (rtk sys-esd <on|off>)\n");
        printf("add fiber command:\n");
        printf("    rtk fiber down-speed [enable | disable]\n");
        printf("    rtk fiber nway [enable | disable] speed [100 | 1000 | auto]\n");
        printf("    rtk fiber nway-force-link [enable | disable]\n");
        printf("    rtk fiber get speed\n");
        printf("    rtk fiber port [port] loopback [enable | disable]\n");
        printf("V1.3\n");
        printf("add 10g serdes and command (rtk 10g PORT [10gFiber | 1gFiber])\n");
        printf("add 10g serdes restart and command (rtk 10g PORT restart)\n");
        printf("add 10g serdes init and command (rtk 10g PORT init)\n");
        return 0;
    }
    #if defined(CONFIG_RTL8396M_DEMO)
    else if (0 == strcmp(argv[1], "10g"))
    {
        int media = 0, order = 2;
        int port;

        port = simple_strtoul(argv[order++], NULL, 10);

        if (0 == strcmp(argv[order], "10gFiber"))
            media = 0;
        else if (0 == strcmp(argv[order], "50cmUtp"))
            media = 1;
        else if (0 == strcmp(argv[order], "100cmUtp"))
            media = 2;
        else if (0 == strcmp(argv[order], "300cmUtp"))
            media = 3;
        else if (0 == strcmp(argv[order], "1gFiber"))
            media = 4;
        else if (0 == strcmp(argv[order], "restart"))
        {
            rtk_10gSds_restart(port);
            return 0;
        }
        else if (0 == strcmp(argv[order], "init"))
        {
            rtk_10gSds_init(port);
            return 0;
        }

        rtk_10gMedia_set(port, media);

        return 0;
    }
    #endif  /* CONFIG_RTL8396M_DEMO */
    #if ((defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB)))
    else if (0 == strcmp(argv[1], "fiber"))
    {
        if (argc < 3)
        {
            printf("rtk fiber down-speed [enable | disable]\n");
            printf("rtk fiber nway [enable | disable] speed [100 | 1000 | auto]\n");
            printf("rtk fiber nway-force-link [enable | disable]\n");
            printf("rtk fiber get speed\n");
            printf("rtk fiber port [port] loopback [enable | disable]\n");
            return 1;
        }

        int order = 2;
        if (0 == strcmp(argv[order], "down-speed"))
        {
            ++order;
            if (0 == strcmp(argv[order], "enable"))
                rtk_fiber_downSpeed_set(1);
            else
                rtk_fiber_downSpeed_set(0);
            return 0;
        }
        else if (0 == strcmp(argv[order], "nway"))
        {
            int speed, sts;

            ++order;
            if (0 == strcmp(argv[order], "enable"))
                sts = 1;
            else
                sts = 0;

            order += 2;
            if (argc < (order + 1))
            {
                printf("paramter num is incorrect\n");
                return 1;
            }

            if (0 == strcmp(argv[order], "auto"))
                speed = 10;
            else
                speed = simple_strtoul(argv[order], NULL, 10);
            rtk_fiber_speed_set(sts, speed);

            return 0;
        }
        else if (0 == strcmp(argv[order], "nway-force-link"))
        {
            ++order;
            if (0 == strcmp(argv[order], "enable"))
                rtk_fiber_nwayForceLink_set(1);
            else
                rtk_fiber_nwayForceLink_set(0);
            return 0;
        }
        else if (0 == strcmp(argv[order], "get"))
        {
            ++order;
            if (0 == strcmp(argv[order], "speed"))
                rtk_fiber_speed_get();
            return 0;
        }
        else if (0 == strcmp(argv[order], "port"))
        {
            int portid;

            ++order;
            portid = simple_strtoul(argv[order], NULL, 10);

            order += 2;
            if (0 == strcmp(argv[order], "enable"))
                rtk_fiber_portLoopback(portid, 1);
            else
                rtk_fiber_portLoopback(portid, 0);
            return 0;
        }
    }
    #endif  /* ((defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB))) */
#endif

		/*get internal GPIO pin status*/
    if (strcmp(argv[1], "pinGet") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: rtk pinGet <pinNum> \n pinNum: 0~32\n\n");
            return 1;
        }
        else
        {
            pinNum = simple_strtoul(argv[2], NULL, 10);

            if ((pinNum < 0) || (pinNum > 32))
            {
                printf("The pinNum (%d) is invalid.\n", pinNum);
                return 1;
            }
            else
            {
                intGpio_PinGet(pinNum,&pinStatus);
                printf("pin%d:\t%d\n\n",pinNum,pinStatus);
                return 0;
            }
        }
    }

    /*set internal GPIO pin status*/
    if (strcmp(argv[1], "pinSet") == 0)
    {
        if (argc < 4)
        {
            printf("Usage: rtk pinSet <pinNum> <status> \n pinNum: 0~32, status: 0/1\n\n");
            return 1;
        }
        else
        {
            pinNum = simple_strtoul(argv[2], NULL, 10);
            pinStatus = simple_strtoul(argv[3], NULL, 10);

            if ((pinNum < 0) || (pinNum > 32))
            {
                printf("The pinNum (%d) is invalid.\n", pinNum);
                return 1;
            }
            else if((pinStatus < 0) || (pinStatus > 1))
            {
                printf("The status (%d) is invalid.\n", pinStatus);
                return 1;
            }
            else
            {
                intGpio_PinSet(pinNum,pinStatus);
                printf("pin%d:\t%d\n\n",pinNum,pinStatus);
                return 0;
            }
        }
    }

        /*get external 8231 pin status*/
    if (strcmp(argv[1], "ext-pinGet") == 0)
    {
        if (argc < 3)
        {
            printf("Usage: rtk ext-pinGet <pinNum> \n pinNum: 0~36\n\n");
            return 1;
        }
        else
        {
            pinNum = simple_strtoul(argv[2], NULL, 10);

            if ((pinNum < 0) || (pinNum > 36))
            {
                printf("The pinNum (%d) is invalid.\n", pinNum);
                return 1;
            }
            else
            {
                rtl8231_pin_status_get(pinNum,&pinStatus);
                printf("pin%d:\t%d\n\n",pinNum,pinStatus);
                return 0;
            }
        }
    }

    /*set external 8231 pin status*/
    if (strcmp(argv[1], "ext-pinSet") == 0)
    {
        if (argc < 4)
        {
            printf("Usage: rtk ext-pinSet <pinNum> <status> \n pinNum: 0~36, status: 0/1\n\n");
            return 1;
        }
        else
        {
            pinNum = simple_strtoul(argv[2], NULL, 10);
            pinStatus = simple_strtoul(argv[3], NULL, 10);

            if ((pinNum < 0) || (pinNum > 36))
            {
                printf("The pinNum (%d) is invalid.\n", pinNum);
                return 1;
            }
            else if((pinStatus < 0) || (pinStatus > 1))
            {
                printf("The status (%d) is invalid.\n", pinStatus);
                return 1;
            }
            else
            {
                rtl8231_pin_status_set(pinNum,pinStatus);
                printf("pin%d:\t%d\n\n",pinNum,pinStatus);
                return 0;
            }
        }
    }
    if (strcmp(argv[1], "smi") == 0)
    {
    	if (argc < 3)
			{
            printf("cst smi parameter error\n\n");
            return 1;
      }

    	if (strcmp(argv[2], "list") == 0)
	    {
	        drv_smi_list();
	        return 0;
	    }
	    if (strcmp(argv[2], "init") == 0)
	    {
	    	  int type;
	    	  int delay;
	    	  int chipid;
	    	  if (argc < 10)
					{
            printf("cst smi parameter error\n\n");
            return 1;
      		}
      		index = simple_strtoul(argv[3], NULL, 10);
	    	  clk_pin = simple_strtoul(argv[4], NULL, 10);
      		data_pin = simple_strtoul(argv[5], NULL, 10);
      		if (strcmp(argv[6], "8") == 0)
      			type=SMI_TYPE_8BITS_DEV;
      		else if (strcmp(argv[6], "16") == 0)
      			type=SMI_TYPE_16BITS_DEV;
      		else
      		{
      			printf("type mode %s is invalid.\n", argv[6]);
        		return 1;
      		}
      		chipid= simple_strtoul(argv[7], NULL, 10);
      		delay = simple_strtoul(argv[8], NULL, 10);

	    		drv_smi_init(0, clk_pin, 0, data_pin, index);
	    		drv_smi_type_set(type, chipid, delay, index,(uint8 *)argv[9]);
	        return 0;
	    }

	    if (strcmp(argv[2], "read") == 0)
	    {
	    	  int reg;
	    	  if (argc < 5)
					{
            printf("cst smi parameter error\n\n");
            return 1;
      		}
      		index = simple_strtoul(argv[3], NULL, 10);
	    	  reg = simple_strtoul(argv[4], NULL, 10);
      		#if 1
	    		drv_smi_read(reg,&data,index);
	        #endif
	        return 0;
	    }
	    if (strcmp(argv[2], "write") == 0)
	    {
	    		int reg;
	    		if (argc < 6)
					{
            printf("cst smi parameter error\n\n");
            return 1;
      		}
      		index = simple_strtoul(argv[3], NULL, 10);
	    	  reg = simple_strtoul(argv[4], NULL, 10);
      		data = simple_strtoul(argv[5], NULL, 10);
	    		drv_smi_write(reg,data,index);
	        return 0;
	    }
    }

    if (strcmp(argv[1], "poe") == 0)
    {
	    if (strcmp(argv[2], "probe") == 0)
	    {
	        poe_pd69100_init();
	        return 0;
	    }
    }
    /* led test */
extern void swledtest_on(int port,int index);
extern void run_ledtest(void);

    if (strcmp(argv[1], "ledtest") == 0)
    {
    	if (argc < 3)
   		{
				run_ledtest();
				return 0;
			}
			if (argc != 4)
      {
          printf("Usage: cst ledtest [port] [led_index]\n\n");
          return 1;
      }
      port = simple_strtoul(argv[2], NULL, 10);
      led_index = simple_strtoul(argv[3], NULL, 10);
      if ((port < 0) || (port > 52))
      {
      	printf("The port (%d) is invalid.\n", port);
        return 1;
      }

      if ((led_index < 0) || (led_index > gSwitchModel->led.num))
      {
      	printf("The led index (%d) is invalid.\n", led_index);
        return 1;
      }
      swledtest_on(port,led_index);
      return 0;
    }

    if (0 == strcmp(argv[1], "loopback"))
    {
        int port_start,port_end, round;
	int internal = 0;

        if(argc>=3){
            if (0 == strcmp(argv[2], "int"))
                internal=1;
            else if(0 == strcmp(argv[2], "ext"))
                internal=0;
            else
                goto usage;
        }

        if (argc == 3){
            port_start=0;
            port_end=gSwitchModel->port.count-1;
            round = 1;
        }
        else if ((argc < 6)||(argc>6))
        {
            goto usage;
        }
        else
        {
            port_start = simple_strtoul(argv[3], NULL, 10);
            port_end = simple_strtoul(argv[4], NULL, 10);
            round = simple_strtoul(argv[5], NULL, 10);

            if ((port_start < 0) || (port_start >= gSwitchModel->port.count))
            {
                printf("The port (%d) is invalid.\n", port_start);
                return 1;
            }

            if ((port_end < 0) || (port_end >= gSwitchModel->port.count) || (port_end < port_start))
            {
                printf("The port (%d) is invalid.\n", port_end);
                return 1;
            }
        }

        rtk_port_loopback_test(internal,port_start,port_end+1,round);

        return 0;
    }

usage:
    printf("Usage:\n%s\n", (char *)cmdtp->usage);
    return 1;
} /* end of do_rtk */

U_BOOT_CMD(
    rtk, 10, 0, do_rtk,
    "rtk     - Realtek commands\n",
    "object action\n"
    "        - SOC commands.\n"
    "rtk network on\n"
    "        - Enable the networking function\n"
    "rtk netowkr off\n"
    "        - Disable the networking function\n"
    "rtk testmode [mode] [port]\n"
    "        - Set default value for specific testing\n"
    "rtk ext-pinGet [pinNum]\n"
    "        - get external 8231 GPIO pin status\n"
    "rtk ext-pinSet [pinNum] [status]\n"
    "        - set external 8231 GPIO pin status\n"
    "rtk smi list\n"
    "				 - list all smi group\n"
    "rtk smi init [group_id] [sck_pin] [sda_pin] "
    "[8/16 access type] [chipid] [delay] [name]\n"
    "        - create a smi group and init\n"
    "rtk smi read [group_id] [reg]\n"
    "rtk smi write [group_id] [reg] [data]\n"
    "rtk pinGet [pinNum]\n"
    "        - get internal GPIO pin status\n"
    "rtk pinSet [pinNum] [status]\n"
    "        - set internal GPIO pin status\n"
    "rtk poe probe \n"
    "        - probe poe device \n"
    "rtk ledtest [port] [led_index]\n"
    "        - led test\n"
    "rtk loopback ext [port-start] [port-end] [round]\n"
    "        - port traffic external loopback test\n"
    "rtk loopback int [port-start] [port-end] [round]\n"
    "        - port traffic internal loopback test\n"
);

