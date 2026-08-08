/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : RTL8390 SOC commands for U-Boot.
 *
 * Feature :
 *
 */


/*
 * Include Files
 */
#include <common.h>
#include <command.h>
#include <rtk_osal.h>
#include <rtk_reg.h>
#include <common/util.h>
#include <rtk/mac/rtl8390/rtl8390_init.h>
#include <rtk/mac/rtl8390/rtl8390_rtk.h>
#include <rtk/mac/rtl8390/rtl8390_drv.h>
#include <rtk/mac/rtl8390/rtl8390_mdc_mdio.h>
#include <rtk/mac/rtl8390/rtl8390_swcore_reg.h>
#include <rtk/drv/swled/swctrl_led_main.h>
#include <interrupt.h>

#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
#include <rtk/phy/conf/conftypes.h>
#endif

#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
#include <rtk/phy/rtl8214f.h>
#endif

#if (defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
#include <rtk/phy/rtl8218b.h>
#endif

/*
 * Symbol Definition
 */
#define FLASH_LAYOUT_JFFS2_START    (0xBD040000)
#define FLASH_LAYOUT_JFFS2_END      (0xBD0FFFFF)

#define MEDIATYPE_COPPER    (0)
#define MEDIATYPE_FIBER     (1)
#define MEDIATYPE_COPPER_AUTO   (2)
#define MEDIATYPE_FIBER_AUTO    (3)

/*
 * Data Declaration
 */
const char jffs2_pattern_default[] = {  \
    0x19, 0x85, 0x20, 0x03, 0x00, 0x00, 0x00, 0x0C, 0xF0, 0x60, 0xDC, 0x98, 0x19, 0x85, 0xE0, 0x01, \
    0x00, 0x00, 0x00, 0x2B, 0x3E, 0x42, 0x24, 0x27, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x02, 0x49, 0xC2, 0x63, 0x49, 0x03, 0x04, 0x00, 0x00, 0x0D, 0x75, 0xC6, 0xA9, \
    0x70, 0x7E, 0xB1, 0xD7, 0x6C, 0x6F, 0x67, 0xFF, 0x19, 0x85, 0xE0, 0x02, 0x00, 0x00, 0x00, 0x44, \
    0xA4, 0xEF, 0x22, 0x3E, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x41, 0xC9, \
    0x2E, 0xC4, 0x19, 0x06, 0x00, 0x00, 0x00, 0x00, 0x49, 0xC2, 0x63, 0x49, 0x49, 0xC2, 0x63, 0x49, \
    0x49, 0xC2, 0x63, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBA, 0x0A, 0xC7, 0xCE };

#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || (defined(CONFIG_RTL8214F)) \
    || (defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB)))
static unsigned int gComboPortMode = -1;               /* 0: Copper, 1: Fiber */
#endif  /* (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || (defined(CONFIG_RTL8214F)) \
    || (defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB))) */


#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
confcode_prv_t rtl8214fb_eee_disable[] = {
    //puts "=============================================================================="
    //puts "Start at: [clock format [clock seconds] -format %c]"
    //puts "=============================================================================="
    //set PHYID 0
    //####################################################################################################
    //#    Force Select Copper Standard Register
    //####################################################################################################
    //puts "Force Select Copper Standard Register ..."
    {1, 31, 0x0008}, // change to page 8
    {1, 16, 0x0F00}, // force select copper standard register
    {1, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Enable Parallel Write
    //####################################################################################################
    //puts "Enable Parallel Write ..."
    {3, 31, 0x0008}, // change to page 8
    {3, 24, 0x0001}, // enable parallel write
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Power Down PHY
    //####################################################################################################
    //puts "Power Down PHY ..."
    {3, 31, 0x0008}, // change to page 8
    {3,  0, 0x1940}, // power down PHY
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Micro-C Enable or Disable Auto Turn off EEE
    //####################################################################################################
    //puts "Micro-C Enable or Disable Auto Turn off EEE ..."
    {3, 31, 0x0005}, // change to page 5
    {3,  5, 0x8B85}, // set Micro-C memory address (enable or disable auto turn off EEE)
    {3,  6, 0xC286}, // set Micro-C memory data (enable or disable auto turn off EEE) (enable: 0xE286, disable: 0xC286)
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Micro-C Control 10M EEE
    //####################################################################################################
    //puts "Micro-C Control 10M EEE ..."
    {3, 31, 0x0005}, // change to page 5
    {3,  5, 0x8B86}, // set Micro-C memory address (control 10M EEE)
    {3,  6, 0x8600}, // set Micro-C memory data (control 10M EEE) (enable: 0x8601, disable: 0x8600)
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Enable or Disable EEE
    //####################################################################################################
    //puts "Enable or Disable EEE ..."
    {3, 31, 0x0007}, // change to page 7
    {3, 30, 0x0020}, // change to ext. page 32
    {3, 21, 0x0000}, // enable or disable EEE (enable: 0x0100, disable: 0x0000)
    {3, 27, 0xA03A}, // force EEE PHY mode (PHY mode: 0xA0BA, MAC mode: 0xA03A)
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    100/1000M EEE Capability
    //####################################################################################################
    //puts "100/1000M EEE Capability ..."
    {3, 31, 0x0000}, // change to page 0
    {3, 13, 0x0007}, // MMD register 7.60
    {3, 14, 0x003C}, // MMD register 7.60
    {3, 13, 0x4007}, // MMD register 7.60
    {3, 14, 0x0000}, // disable 100/1000M EEE capability
    {3, 13, 0x0000}, // MMD register
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    10M EEE Amplitude
    //####################################################################################################
    //puts "10M EEE Amplitude ..."
    {3, 31, 0x0002}, // change to page 2
    {3, 11, 0x17A7}, // 10M EEE amplitude
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Disable Parallel Write
    //####################################################################################################
    //puts "Disable Parallel Write ..."
    {3, 31, 0x0008}, // change to page 8
    {3, 24, 0x0000}, // disable parallel write
    {3, 31, 0x0008}, // change to page 8
};
#endif

#ifdef CONFIG_EEE
#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
confcode_prv_t rtl8214fb_eee_enable[] = {
    //puts "=============================================================================="
    //puts "Start at: [clock format [clock seconds] -format %c]"
    //puts "=============================================================================="
    //set PHYID 0
    //####################################################################################################
    //#    Force Select Copper Standard Register
    //####################################################################################################
    //puts "Force Select Copper Standard Register ..."
    {1, 31, 0x0008}, // change to page 8
    {1, 16, 0x0F00}, // force select copper standard register
    {1, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Enable Parallel Write
    //####################################################################################################
    //puts "Enable Parallel Write ..."
    {3, 31, 0x0008}, // change to page 8
    {3, 24, 0x0001}, // enable parallel write
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Power Down PHY
    //####################################################################################################
    //puts "Power Down PHY ..."
    {3, 31, 0x0008}, // change to page 8
    {3,  0, 0x1940}, // power down PHY
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Micro-C Enable or Disable Auto Turn off EEE
    //####################################################################################################
    //puts "Micro-C Enable or Disable Auto Turn off EEE ..."
    {3, 31, 0x0005}, // change to page 5
    {3,  5, 0x8B85}, // set Micro-C memory address (enable or disable auto turn off EEE)
    {3,  6, 0xE286}, // set Micro-C memory data (enable or disable auto turn off EEE) (enable: 0xE286, disable: 0xC286)
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Micro-C Control 10M EEE
    //####################################################################################################
    //puts "Micro-C Control 10M EEE ..."
    {3, 31, 0x0005}, // change to page 5
    {3,  5, 0x8B86}, // set Micro-C memory address (control 10M EEE)
    {3,  6, 0x8600}, // set Micro-C memory data (control 10M EEE) (enable: 0x8601, disable: 0x8600)
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Enable or Disable EEE
    //####################################################################################################
    //puts "Enable or Disable EEE ..."
    {3, 31, 0x0007}, // change to page 7
    {3, 30, 0x0020}, // change to ext. page 32
    {3, 21, 0x0100}, // enable or disable EEE (enable: 0x0100, disable: 0x0000)
    {3, 27, 0xA03A}, // force EEE PHY mode (PHY mode: 0xA0BA, MAC mode: 0xA03A)
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    100/1000M EEE Capability
    //####################################################################################################
    //puts "100/1000M EEE Capability ..."
    {3, 31, 0x0000}, // change to page 0
    {3, 13, 0x0007}, // MMD register 7.60
    {3, 14, 0x003C}, // MMD register 7.60
    {3, 13, 0x4007}, // MMD register 7.60
    {3, 14, 0x0006}, // enable 100/1000M EEE capability
    {3, 13, 0x0000}, // MMD register
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    10M EEE Amplitude
    //####################################################################################################
    //puts "10M EEE Amplitude ..."
    {3, 31, 0x0002}, // change to page 2
    {3, 11, 0x17A7}, // 10M EEE amplitude
    {3, 31, 0x0008}, // change to page 8
    //####################################################################################################
    //#    Disable Parallel Write
    //####################################################################################################
    //puts "Disable Parallel Write ..."
    {3, 31, 0x0008}, // change to page 8
    {3, 24, 0x0000}, // disable parallel write
    {3, 31, 0x0008}, // change to page 8
};

#endif /* (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B)) */
#endif  /* CONFIG_EEE */

/*
 * Macro Definition
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      rtk_network_on
 * Description:
 *      Enable network function.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_network_on(void)
{
    rtl8390_phyPowerOn();

    /* delay 2.0 sec for link-status stable */
    printf("Please wait for PHY init-time ...\n\n");
#ifdef CONFIG_RTL8390_FPGA
    OSAL_UDELAY(200000);
#else
    OSAL_UDELAY(2000000);
#endif

    return;
} /* end of rtk_network_on */

/* Function Name:
 *      rtk_phyPortPowerOn
 * Description:
 *      Power-On PHY by port.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_phyPortPowerOn(int mac_idx)
{
    if (gSwitchModel == NULL)
        return;

    rtl8390_phyPortPowerOn(gSwitchModel->port.list[mac_idx].mac_id);
}

/* Function Name:
 *      rtk_portlink_get
 * Description:
 *      Get port link status.
 * Input:
 *      int unit
 *      int port
 * Output:
 *      int *link
 * Return:
 *      always 0
 * Note:
 *      None
 */
int rtk_portlink_get(int unit,int port,int *link)
{
    unsigned int val;

    val = MEM32_READ(SWCORE_BASE_ADDR | RTL8390_MAC_LINK_STS_ADDR(port));
    val = MEM32_READ(SWCORE_BASE_ADDR | RTL8390_MAC_LINK_STS_ADDR(port));

    if (0x1 == ((val >> (port % 32)) & 0x1))
    {
        *link=1;
    }
    else
        *link=0;

    return 0;
}

/* Function Name:
 *      rtk_network_off
 * Description:
 *      Disable network function.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_network_off(void)
{
    rtl8390_phyPowerOff();

    return;
}/* end of rtk_network_off */

void rtk_default(void)
{
    cmd_tbl_t *run;
    int rc;
    char *argu[8];
    char addr_str1[12];
    char addr_str2[12];
    char len_str[4];

    /* erase 0xbd040000 0xbd0fffff; */
    run = find_cmd("erase");
    if (NULL == run)
    {
        printf("erase command is not supported!\n");
        return;
    }

    sprintf(addr_str1, "0x%08X", FLASH_LAYOUT_JFFS2_START);
    sprintf(addr_str2, "0x%08X", FLASH_LAYOUT_JFFS2_END);

    argu[0] = "erase";
    argu[1] = addr_str1;
    argu[2] = addr_str2;
    argu[3] = NULL;

    rc = run->cmd(run, 0, 3, argu);
    if (rc != 0)
    {
        printf("[DBG] %s %s %s\n", argu[0], argu[1], argu[2]);
        printf("Error: erase flash failed!\n");
        return;
    }

    /* cp.b 0x81000000 0xbd040000 7c */
    run = find_cmd("cp.b");
    if (NULL == run)
    {
        printf("erase command is not supported!\n");
        return;
    }

    sprintf(addr_str1, "0x%08X", (uint32)&jffs2_pattern_default);
    sprintf(addr_str2, "0x%08X", FLASH_LAYOUT_JFFS2_START);
    sprintf(len_str, "%X", sizeof(jffs2_pattern_default));

    argu[0] = "cp.b";
    argu[1] = addr_str1;
    argu[2] = addr_str2;
    argu[3] = len_str;
    argu[4] = NULL;

    rc = run->cmd(run, 0, 4, argu);
    if (rc != 0)
    {
        printf("[DBG] %s %s %s %s\n", argu[0], argu[1], argu[2], argu[3]);
        printf("Error: initial configuration failed!\n");
        return;
    }

    printf("Success.\n");

    return;
} /* end of rtk_default */

#ifdef CONFIG_EEE
void rtk_eee_on(const rtk_switch_model_t *pModel)
{
    int i, j, gval;
	unsigned int val;

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SMI_GLB_CTRL_ADDR,
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x0);

    for (i=0; i<pModel->phy.count; i++)
    {
        switch (pModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                {
                    unsigned int rtl8208d_phy0_id = pModel->phy.list[i].mac_id;
                    unsigned int phyData;

                    for (j=(rtl8208d_phy0_id); j<(rtl8208d_phy0_id+8); j++)
                    {
                        /* enable EEE function */
                        gMacDrv->drv_miim_read(j, 4, 16, &phyData);
                        phyData |= (3 << 12);
                        gMacDrv->drv_miim_write(j, 4, 16, phyData);

                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 31, 0x0);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x7);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 14, 0x3c);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x4007);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 14, 0x2);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x0);

                        /* Force re-autonegotiation if AN is on */
                        gMacDrv->drv_miim_read(j, 0, 0, &phyData);
                        if (phyData & 0x1000)
                        {
                            phyData |= (1 << 9);
                        }
                        gMacDrv->drv_miim_write(j, 0, 0, phyData);
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                {
                    unsigned int    rtl8214fb_phy0_macid = pModel->phy.list[i].mac_id;
                    unsigned int             forceReg, phyData;
                    int             idx;

                    /* store the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 16, &forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);

                    for (idx = 0; idx < (sizeof(rtl8214fb_eee_enable)/sizeof(confcode_prv_t)); ++idx)
                    {
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_eee_enable[idx].phy, \
                                gMacDrv->miim_max_page,
                                (int)rtl8214fb_eee_enable[idx].reg,
                                (int)rtl8214fb_eee_enable[idx].val);
                    }

                    /* Restart Auto-Negotiation */
                    for (idx = 0; idx < 4; ++idx)
                    {
                        gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, &phyData);

                        phyData |= (1 << 9);
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, phyData);
                    }

                    /* restore the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 16, forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    OSAL_MDELAY(1000);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218))
            case RTK_CHIP_RTL8218:
                {
                    unsigned int rtl8218_macid = pModel->phy.list[i].mac_id;
                    //int val;

                    /* enable main function */
                    for (j = rtl8218_macid; j < (rtl8218_macid + 8); j++)
                    {
                        //## ext page 32
                        gMacDrv->drv_miim_write(rtl8218_macid,7,0x1e,0x0020);
                        gMacDrv->drv_miim_write(rtl8218_macid,7,0x17,0x000a);
                        gMacDrv->drv_miim_write(rtl8218_macid,7,0x1b,0x2fca);
                        gMacDrv->drv_miim_write(rtl8218_macid,7,0x15,0x0100);

                        //###### negear EEE Nway ability autooff
                        gMacDrv->drv_miim_write(rtl8218_macid,31,0x1f,0x0005);
                        gMacDrv->drv_miim_write(rtl8218_macid,31,0x05,0x8b84);
                        //gMacDrv->drv_miim_write(rtl8218_macid_pm,31,0x06,0x0026);
                        gMacDrv->drv_miim_write(rtl8218_macid,31,0x06,0x0062);
                        gMacDrv->drv_miim_write(rtl8218_macid,31,0x1f,0x0000);
                    }

                    gMacDrv->drv_miim_write(rtl8218_macid,0,0x1f,0x0000);

                    for (j = rtl8218_macid; j < (rtl8218_macid + 8); j++)
                    {
                        gMacDrv->drv_miim_read(j, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            gMacDrv->drv_miim_write(j, 0, 0, (val | (1 << 9)));
                        }
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218B:
            case RTK_CHIP_RTL8214FC:
            case RTK_CHIP_RTL8218FB:
                {
                    int base = pModel->phy.list[i].mac_id;
                    int port, maxPort;

                    printf("PHY[%d]: enable EEE\n", i);

                    if (RTK_CHIP_RTL8214FC == pModel->phy.list[i].chip)
                        maxPort = 4;
                    else
                        maxPort = 8;

                    for (j = 0; j < maxPort; ++j)
                    {
                        port = base + j;

                        gMacDrv->drv_miim_write(port, 0xa42, 29, 0x1);
                        gMacDrv->drv_miim_read(port, 0xa43, 25, &val);
                        val &= ~(1 << 5);
                        gMacDrv->drv_miim_write(port, 0xa43, 25, val);

                        gMacDrv->drv_miim_write(port, 7, 60, 0x6);

                        /* Force re-autonegotiation if AN is on */
                        gMacDrv->drv_miim_read(port, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            val |= (1 << 9);
                            gMacDrv->drv_miim_write(port, 0, 0, val);
                        }

                        gMacDrv->drv_miim_write(port, 0xa42, 29, 0x0);
                    }
                }
                break;
            #endif
            default:
                printf("PHY[%d]: not supported in EEE\n", i);
                break;
        }
    }

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SMI_GLB_CTRL_ADDR, \
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x1);

    gval = 0;
    for (i = 0; i < pModel->port.count; ++i)
    {
        j = pModel->port.list[i].mac_id;

        val = 1;
        if (i >= 48)
            gval = 1;

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_100M_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_100M_EN_MASK, val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_500M_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_500M_EN_MASK, ~val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_1000M_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_1000M_EN_MASK, gval);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_10G_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_10G_EN_MASK, ~val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_EEE_CTRL_ADDR(j), \
                RTL8390_EEE_CTRL_EEE_TX_EN_OFFSET,
                RTL8390_EEE_CTRL_EEE_TX_EN_MASK, val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_EEE_CTRL_ADDR(j), \
                RTL8390_EEE_CTRL_EEE_RX_EN_OFFSET,
                RTL8390_EEE_CTRL_EEE_RX_EN_MASK, val);
    }

    return;
} /* end of rtk_eee_on */
#endif

void rtk_eee_off(const rtk_switch_model_t *pModel)
{
    int i, j;
    unsigned int val;

    /* Save & Disable MAC polling PHY setting */
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SMI_GLB_CTRL_ADDR,
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x0);

    for (i=0; i<pModel->phy.count; i++)
    {
        switch (pModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8208))
            case RTK_CHIP_RTL8208D:
            case RTK_CHIP_RTL8208L:
                {
                    unsigned int rtl8208d_phy0_id = pModel->phy.list[i].mac_id;
                    unsigned int phyData;

                    for (j=(rtl8208d_phy0_id); j<(rtl8208d_phy0_id+8); j++)
                    {
                        /* enable EEE function */
                        gMacDrv->drv_miim_read(j, 4, 16, &phyData);
                        phyData &= ~(3 << 12);
                        gMacDrv->drv_miim_write(j, 4, 16, phyData);

                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 31, 0x0);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x7);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 14, 0x3c);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x4007);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 14, 0x0);
                        gMacDrv->drv_miim_write(j, gMacDrv->miim_max_page, 13, 0x0);

                        /* Force re-autonegotiation if AN is on */
                        gMacDrv->drv_miim_read(j, 0, 0, &phyData);
                        if (phyData & 0x1000)
                        {
                            phyData |= (1 << 9);
                        }
                        gMacDrv->drv_miim_write(j, 0, 0, phyData);
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                {
                    unsigned int    rtl8214fb_phy0_macid = pModel->phy.list[i].mac_id;
                    unsigned int    forceReg, phyData;
                    int             idx;

                    /* store the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 16, &forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);

                    for (idx = 0; idx < (sizeof(rtl8214fb_eee_disable)/sizeof(confcode_prv_t)); ++idx)
                    {
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + rtl8214fb_eee_disable[idx].phy,
                                gMacDrv->miim_max_page,
                                (int)rtl8214fb_eee_disable[idx].reg,
                                (int)rtl8214fb_eee_disable[idx].val);
                    }

                    /* Restart Auto-Negotiation */
                    for (idx = 0; idx < 4; ++idx)
                    {
                        gMacDrv->drv_miim_read(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, &phyData);

                        phyData |= (1 << 9);
                        gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + idx,
                                gMacDrv->miim_max_page, 0, phyData);
                    }

                    /* restore the original register value */
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 16, forceReg);
                    gMacDrv->drv_miim_write(rtl8214fb_phy0_macid + 1,
                            gMacDrv->miim_max_page, 31, 8);
                    OSAL_MDELAY(1000);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8218))
            case RTK_CHIP_RTL8218:
                {
                    unsigned int rtl8218_macid = pModel->phy.list[i].mac_id;

                    for (j = rtl8218_macid; j < (rtl8218_macid + 8); j++)
                    {
                        //###### negear EEE Nway ability autooff
                        gMacDrv->drv_miim_write(j,127,0x1f,0x0005);
                        gMacDrv->drv_miim_write(j,127,0x05,0x8b84);
                        gMacDrv->drv_miim_write(j,127,0x06,0x0042);
                        gMacDrv->drv_miim_write(j,127,0x1f,0x0000);

                        //## ext page 32
                        gMacDrv->drv_miim_write(j,7,0x1e,0x0020);
                        gMacDrv->drv_miim_write(j,7,0x17,0x000a);
                        gMacDrv->drv_miim_write(j,7,0x1b,0x2f4a);
                        gMacDrv->drv_miim_write(j,7,0x15,0x0000);
                    }

                    /* Force re-autonegotiation if AN is on */
                    for (j = rtl8218_macid; j < (rtl8218_macid + 8); j++)
                    {
                        gMacDrv->drv_miim_read(j, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            gMacDrv->drv_miim_write(j, 0, 0, (val | (1 << 9)));
                        }
                    }
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218B) || defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218B:
            case RTK_CHIP_RTL8214FC:
            case RTK_CHIP_RTL8218FB:
                {
                    int base = pModel->phy.list[i].mac_id;
                    int port, maxPort;

                    printf("PHY[%d]: disable EEE\n", i);

                    if (RTK_CHIP_RTL8214FC == pModel->phy.list[i].chip)
                        maxPort = 4;
                    else
                        maxPort = 8;

                    for (j = 0; j < maxPort; ++j)
                    {
                        port = base + j;

                        gMacDrv->drv_miim_write(port, 0xa42, 29, 0x1);
                        gMacDrv->drv_miim_read(port, 0xa43, 25, &val);
                        val &= ~(1 << 5);
                        gMacDrv->drv_miim_write(port, 0xa43, 25, val);

                        gMacDrv->drv_miim_write(port, 7, 60, 0x0);

                        /* Force re-autonegotiation if AN is on */
                        gMacDrv->drv_miim_read(port, 0, 0, &val);
                        if (val & (1 << 12))
                        {
                            val |= (1 << 9);
                            gMacDrv->drv_miim_write(port, 0, 0, val);
                        }

                        gMacDrv->drv_miim_write(port, 0xa42, 29, 0x0);
                    }
                }
                break;
            #endif
            default:
                printf("PHY[%d]: not supported in EEE\n", i);
                break;
        }
    }

    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_SMI_GLB_CTRL_ADDR, \
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_OFFSET,
                RTL8390_SMI_GLB_CTRL_MDX_POLLING_EN_MASK, 0x1);

    for (i = 0; i < pModel->port.count; ++i)
    {
        j = pModel->port.list[i].mac_id;

        val = 0;
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_100M_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_100M_EN_MASK, val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_500M_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_500M_EN_MASK, val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_1000M_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_1000M_EN_MASK, val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_MAC_FORCE_MODE_CTRL_ADDR(j), \
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_10G_EN_OFFSET,
                RTL8390_MAC_FORCE_MODE_CTRL_EEE_10G_EN_MASK, val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_EEE_CTRL_ADDR(j), \
                RTL8390_EEE_CTRL_EEE_TX_EN_OFFSET,
                RTL8390_EEE_CTRL_EEE_TX_EN_MASK, val);

        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_EEE_CTRL_ADDR(j), \
                RTL8390_EEE_CTRL_EEE_RX_EN_OFFSET,
                RTL8390_EEE_CTRL_EEE_RX_EN_MASK, val);
    }
    return;
} /* end of rtk_eee_off */

#if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || (defined(CONFIG_RTL8214F)) \
    || (defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB)))
/* Function Name:
 *      rtk_comboport_copper
 * Description:
 *      Force comboport into copper mode.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_comboport_copper(void)
{
    int i;
    #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || defined(CONFIG_RTL8214F) || defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
    int portid;
    #endif

    if (gSwitchModel == NULL)
        return;

    if (MEDIATYPE_COPPER == gComboPortMode)
        return;

    gComboPortMode = MEDIATYPE_COPPER;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214F))
            case RTK_CHIP_RTL8214F:
                for (portid = 0; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                {
                    rtl8214f_media_set(portid + gSwitchModel->phy.list[i].mac_id, MEDIATYPE_COPPER);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                for (portid = 0; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                {
                    rtl8214fb_media_set(portid + gSwitchModel->phy.list[i].mac_id, MEDIATYPE_COPPER);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (portid = 0; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                    rtl8214fc_media_set(portid + gSwitchModel->phy.list[i].mac_id, gComboPortMode);
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                for (portid = 4; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                    rtl8214fc_media_set(portid + gSwitchModel->phy.list[i].mac_id, gComboPortMode);
                break;
            #endif
        }
    }

    return;
} /* end of rtk_comboport_copper */


/* Function Name:
 *      rtk_comboport_fiber
 * Description:
 *      Force comboport into fiber mode.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_comboport_fiber(void)
{
    int i;
    #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || defined(CONFIG_RTL8214F) || defined(CONFIG_RTL8214FC) || defined(CONFIG_RTL8218FB))
    int portid;
    #endif

    if (gSwitchModel == NULL)
        return;

    if (MEDIATYPE_FIBER == gComboPortMode)
        return;

    gComboPortMode = MEDIATYPE_FIBER;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214F))
            case RTK_CHIP_RTL8214F:
                for (portid = 0; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                {
                    rtl8214f_media_set(portid + gSwitchModel->phy.list[i].mac_id, MEDIATYPE_FIBER);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                for (portid = 0; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                {
                    rtl8214fb_media_set(portid + gSwitchModel->phy.list[i].mac_id, MEDIATYPE_FIBER);
                }
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (portid = 0; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                    rtl8214fc_media_set(portid + gSwitchModel->phy.list[i].mac_id, gComboPortMode);
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                for (portid = 4; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                    rtl8214fc_media_set(portid + gSwitchModel->phy.list[i].mac_id, gComboPortMode);
                break;
            #endif
        }
    }

    return;
} /* end of rtk_comboport_fiber */

void rtk_comboport_portcopper(int portid)
{
    int i;

    if (gSwitchModel == NULL)
        return;

    for (i=0; i<gSwitchModel->port.count; i++)
    {
        if (gSwitchModel->port.list[i].mac_id != portid)
            continue;

        switch (gSwitchModel->phy.list[gSwitchModel->port.list[i].phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8214F))
            case RTK_CHIP_RTL8214F:
                    rtl8214f_media_set(portid, MEDIATYPE_COPPER);
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                    rtl8214fb_media_set(portid, MEDIATYPE_COPPER);
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                    rtl8214fc_media_set(portid, MEDIATYPE_COPPER);
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                    rtl8214fc_media_set(portid, MEDIATYPE_COPPER);
                break;
            #endif
        }
    }

    return;
} /* end of rtk_comboport_copper */


/* Function Name:
 *      rtk_comboport_fiber
 * Description:
 *      Force comboport into fiber mode.
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
void rtk_comboport_portfiber(int portid)
{
    int i;

    if (gSwitchModel == NULL)
        return;

    for (i=0; i<gSwitchModel->port.count; i++)
    {
        if (gSwitchModel->port.list[i].mac_id != portid)
            continue;

        switch (gSwitchModel->phy.list[gSwitchModel->port.list[i].phy_idx].chip)
        {
            #if (defined(CONFIG_RTL8214F))
            case RTK_CHIP_RTL8214F:
                    rtl8214f_media_set(portid, MEDIATYPE_FIBER);
                break;
            #endif
            #if (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B))
            case RTK_CHIP_RTL8214FB:
            case RTK_CHIP_RTL8214B:
            case RTK_CHIP_RTL8212B:
                    rtl8214fb_media_set(portid, MEDIATYPE_FIBER);
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                    rtl8214fc_media_set(portid, MEDIATYPE_FIBER);
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                if ((portid % 8) >= 4)
                    rtl8214fc_media_set(portid, MEDIATYPE_FIBER);
                break;
            #endif
        }
    }

    return;
} /* end of rtk_comboport_fiber */

void rtk_comboport_auto(void)
{
    int i;
    int portid;

    if (gSwitchModel == NULL)
        return;

    if (MEDIATYPE_FIBER_AUTO == gComboPortMode)
        return;

    gComboPortMode = MEDIATYPE_FIBER_AUTO;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (portid = 0; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                    rtl8214fc_media_set(portid + gSwitchModel->phy.list[i].mac_id, gComboPortMode);
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                for (portid = 4; portid < gSwitchModel->phy.list[i].phy_max; ++portid)
                    rtl8214fc_media_set(portid + gSwitchModel->phy.list[i].mac_id, gComboPortMode);
                break;
            #endif
        }
    }

    return;
} /* end of rtk_comboport_auto */
#endif /* (defined(CONFIG_RTL8214FB) || defined(CONFIG_RTL8214B) || defined(CONFIG_RTL8212B) || (defined(CONFIG_RTL8214F))
(defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB))) */


void rtk_linkdown_powersaving_patch(void)
{
    unsigned char port, tryCount;
    unsigned int value, value1;
    unsigned int flag = FALSE;

    while(1)
    {
        for(port= 0; port < 16; port++)
        {
            gMacDrv->drv_miim_read(port, 0, 1, (unsigned int *)&value);
            gMacDrv->drv_miim_read(port, 0, 1, (unsigned int *)&value);

            if ((value & 0x4) == 0)
            {
                flag = TRUE;
                for (tryCount = 0; tryCount < 20; tryCount++)
                {
                    gMacDrv->drv_miim_read(port, 0, 19, (unsigned int *)&value1);
                    if (value1 & 0x8000)
                    {
                        gMacDrv->drv_miim_write(port, 0, 24, 0x0310);
                        break;
                    }
                }
            }
        }

        if(TRUE)
        {
            OSAL_UDELAY(800);
            gMacDrv->drv_miim_write(0xff, 0x0, 24, 0x8310);
        }
    }

} /* end of rtk_linkdown_powersaving_patch */
#ifdef CONFIG_SOFTWARE_CONTROL_LED
void rtk_softwareControlLed_on(void)
{
    /*Setup the board LED information to swCtrl_led module*/
    swCtrl_led_init();

    uboot_isr_register(RTK_DEV_TC0, swCtrl_led_intr_handler, NULL);
    common_enable_irq(29);/* TC0_IRQ */
    common_enable_interrupt();

    return;
} /* end of rtk_softwareControlLed_on */

void rtk_softwareControlLed_off(void)
{
    /*Setup the board LED information to swCtrl_led module*/
    //swCtrl_led_init();

    //uboot_isr_register(RTK_DEV_TC0, swCtrl_led_intr_handler, NULL);
    //common_enable_irq(29);/* TC0_IRQ */
    common_disable_interrupt();
    swCtrl_led_allOff();

    return;
} /* end of rtk_softwareControlLed_off */
#endif

void rtk_saLearning(int state)
{
    Tuint32 val;
    int     i, port;

    if (gSwitchModel == NULL)
        return;

    if (0 == state)
        val = 2;
    else
        val = 0;

    for (i = 0; i < gSwitchModel->port.count; ++i)
    {
        port = gSwitchModel->port.list[i].mac_id;

        /* Enable SA learning */
        MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_L2_PORT_NEW_SALRN_ADDR(port), \
                RTL8390_L2_PORT_NEW_SALRN_NEW_SALRN_OFFSET(port), \
                RTL8390_L2_PORT_NEW_SALRN_NEW_SALRN_MASK(port), val);
    }
}

void rtk_port_isolation_on(void)
{
    unsigned int    addr;
    unsigned char   i;
    int             port;

    if (gSwitchModel == NULL)
        return;

    /*Port Isolation settings: port 0<-->1, 2<-->3, 4<-->5.......*/
    for (i = 0; i < gSwitchModel->port.count; i += 2)
    {
        /* first port */
        port = gSwitchModel->port.list[i].mac_id;
        addr = SWCORE_BASE_ADDR | RTL8390_PORT_ISO_CTRL_ADDR(port);
        if ((port + 1) < 32)
            addr += 4;

        REG32(addr) = 1 << (port + 1);

        /* second port */
        port = gSwitchModel->port.list[i + 1].mac_id;
        addr = SWCORE_BASE_ADDR | RTL8390_PORT_ISO_CTRL_ADDR(port);
        if ((port - 1) < 32)
            addr += 4;

        REG32(addr) = 1 << (port - 1);
    }

    rtk_saLearning(1);
}

void rtk_port_isolation_off(void)
{
    unsigned int    addr;
    unsigned char   i;
    int             port;

    if (gSwitchModel == NULL)
        return;

    /*Port Isolation settings: port 0<-->1, 2<-->3, 4<-->5.......*/
    for (i = 0; i < gSwitchModel->port.count; i += 2)
    {
        /* first port */
        port = gSwitchModel->port.list[i].mac_id;
        addr = SWCORE_BASE_ADDR | RTL8390_PORT_ISO_CTRL_ADDR(port);
        if ((port + 1) < 32)
            addr += 4;

        REG32(addr) = 0xFFFFFFFF;

        /* second port */
        port = gSwitchModel->port.list[i + 1].mac_id;
        addr = SWCORE_BASE_ADDR | RTL8390_PORT_ISO_CTRL_ADDR(port);
        if ((port - 1) < 32)
            addr += 4;

        REG32(addr) = 0xFFFFFFFF;
    }

    rtk_saLearning(0);
}

void rtk_portIsolation(int srcPort,int destPort)
{
    unsigned int    addr;

    /*Port Isolation settings: CPU-->port */
    addr = SWCORE_BASE_ADDR | RTL8390_PORT_ISO_CTRL_ADDR(srcPort);
    //printf("src %d dst %d %x\n", srcPort, destPort, addr);
    if (destPort < 32)
    {
        REG32(addr) = 0;
        REG32(addr + 4) = 0x1 << destPort;
    }
    else
    {
        REG32(addr) = 0x1 << (destPort % 32);
        REG32(addr + 4) = 0;
    }
}

void rtk_portIsolationCPUgoto(int port)
{
    rtk_portIsolation(52, port);
}

void rtk_portIsolationToCPU(int port)
{
    rtk_portIsolation(port, 52);
}

void rtk_phy_selfLoop_on(int portId)
{
    unsigned int val;

    rtl8390_getPhyReg(portId, 0, 0, &val);
    val |= (1 << 14);
    rtl8390_setPhyReg(portId, 0, 0, val);
}

void rtk_phy_selfLoop_off(int portId)
{
    unsigned int val;

    rtl8390_getPhyReg(portId, 0, 0, &val);
    val &= ~(1 << 14);
    rtl8390_setPhyReg(portId, 0, 0, val);
}

int32 rtk_htp_detect(void)
{
    uint32  val;

    val = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EXT_GPIO_DATA_CTRL_ADDR(0));
    if ((val & (1 << 27)) == 0)
        return 1;

    return 0;
}

int32 rtk_rstDeftGpio_init(void)
{
    uint32  val;

    val = 0x2;
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR, \
        RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_OFFSET, \
        RTL8390_LED_GLB_CTRL_EXT_GPIO_EN_MASK, \
        val);

    val = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EXT_GPIO_DIR_CTRL_ADDR(0));
    val &= ~(1 << 23);
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_EXT_GPIO_DIR_CTRL_ADDR(0), \
        RTL8390_EXT_GPIO_DIR_CTRL_EXT_GPIO_DIR_OFFSET(0), \
        RTL8390_EXT_GPIO_DIR_CTRL_EXT_GPIO_DIR_MASK(0), \
        val);

    OSAL_MDELAY(500);

    return 0;
}   /* end of rtk_rstDeftGpio_init */

int32 rtk_rstDeftGpio_detect(void)
{
    uint32  val;

    val = MEM32_READ(SWCORE_BASE_ADDR| RTL8390_EXT_GPIO_DATA_CTRL_ADDR(0));
    if ((val & (1 << 23)) == 0)
        return 1;

    return 0;
}   /* end of rtk_rstDeftGpio_detect */

void rtk_smiRead(uint32 phyad, uint32 regad, uint32* pData)
{
    #if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
    rtl8390_smiRead(phyad, regad, pData);
    #endif
}

void rtk_smiWrite(uint32 phyad, uint32 regad, uint32 data)
{
    #if defined(CONFIG_MDC_MDIO_EXT_SUPPORT)
    rtl8390_smiWrite(phyad, regad, data);
    #endif
}

void rtk_sys_led_on(void)
{
    uint32  val;

    val = 0x3;
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR, \
        RTL8390_LED_GLB_CTRL_SYS_LED_MODE_OFFSET, \
        RTL8390_LED_GLB_CTRL_SYS_LED_MODE_MASK, \
        val);
}

void rtk_sys_led_off(void)
{
    uint32  val;

    val = 0x0;
    MEM32_WRITE_FIELD(SWCORE_BASE_ADDR| RTL8390_LED_GLB_CTRL_ADDR, \
        RTL8390_LED_GLB_CTRL_SYS_LED_MODE_OFFSET, \
        RTL8390_LED_GLB_CTRL_SYS_LED_MODE_MASK, \
        val);
}

void rtk_sfp_speed_set(int port, int speed)
{
    if (port > 2)
        return;

    if (speed != 100 && speed != 1000)
        return;

    rtl8390_sfp_speed_set(port, speed);

    return;
}

void rtk_sysEsd_set(int state)
{
    unsigned int    romId, val;
    int             phyIdx, basePort, portNum, portId;
    int             i, stsVal;

    if (1 == state)
        stsVal = 0x7;
    else
        stsVal = 0x0;

    for (phyIdx = 0; phyIdx < gSwitchModel->phy.count; ++phyIdx)
    {
        basePort = gSwitchModel->phy.list[phyIdx].mac_id;


        switch (gSwitchModel->phy.list[phyIdx].chip)
        {
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218B:
                portNum = 8;
                break;
            #endif
            #if (defined(CONFIG_RTL8218FB))
            case RTK_CHIP_RTL8218FB:
                portNum = 8;
                break;
            #endif
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                portNum = 4;
                break;
            #endif
            default:
                portNum = 0;
                break;
        }

        for (i = 0; i < portNum; ++i)
        {
            gMacDrv->drv_miim_write(basePort, gMacDrv->miim_max_page, 27, 0x0004);
            gMacDrv->drv_miim_read(basePort, gMacDrv->miim_max_page, 28, &romId);
            if (romId > 2)
            {
                printf("invalid rom id %d\n", romId);
                return;
            }

            portId = basePort + i;

            /* patch request */
            gMacDrv->drv_miim_read(portId, 0xb82, 16, &val);
            val |= (1 << 4);
            gMacDrv->drv_miim_write(portId, 0xb82, 16, val);

            /* polling patch ready */
            do
            {
                gMacDrv->drv_miim_read(portId, 0xb80, 16, &val);
            } while ((val & 0x40) == 0);

            gMacDrv->drv_miim_read(portId, 0xa00, 16, &val);
            val &= ~(0xF << 12);
            val |= (stsVal << 12);
            gMacDrv->drv_miim_write(portId, 0xa00, 16, val);

            /* patch release */
            gMacDrv->drv_miim_read(portId, 0xb82, 16, &val);
            val &= ~(1 << 4);
            gMacDrv->drv_miim_write(portId, 0xb82, 16, val);
        }
    }

    return;
}

#if ((defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB)))
void rtk_fiber_downSpeed_set(int status)
{
    int port, i;
    unsigned int val;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (port = 48; port < (48 + gSwitchModel->phy.list[i].phy_max); ++port)
                {
                    gMacDrv->drv_miim_write(port, 0, 29, 3);

                    gMacDrv->drv_miim_read(port, 8, 17, &val);
                    if (1 == status)
                        val |= (1 << 5);
                    else
                        val &= ~(1 << 5);
                    gMacDrv->drv_miim_write(port, 8, 17, val);

                    gMacDrv->drv_miim_write(port, 0, 29, 0);

                    printf("down speed p%d %d %x\n", port, status, val);
                }
                break;
            #endif
        }
    }

    return;
}

void rtk_fiber_nwayForceLink_set(int status)
{
    int port, i;
    unsigned int val;

    rtk_comboport_fiber();

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (port = 48; port < (48 + gSwitchModel->phy.list[i].phy_max); ++port)
                {
                    gMacDrv->drv_miim_write(port, 0, 29, 3);

                    gMacDrv->drv_miim_read(port, 8, 20, &val);
                    if (1 == status)
                        val |= (1 << 2);
                    else
                        val &= ~(1 << 2);
                    gMacDrv->drv_miim_write(port, 8, 20, val);

                    gMacDrv->drv_miim_write(port, 0, 29, 0);

                    printf("force link p%d %d %x\n", port, status, val);
                }
                break;
            #endif
        }
    }

    return;
}

void rtk_fiber_nway_set(int status)
{
    int port, i;
    unsigned int val;

    rtk_comboport_fiber();
    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (port = 48; port < (48 + gSwitchModel->phy.list[i].phy_max); ++port)
                {
                    gMacDrv->drv_miim_read(port, 0, 0, &val);
                    if (1 == status)
                        val |= ((1 << 12) | (1 <<9));
                    else
                        val &= ~(1 << 12);
                    gMacDrv->drv_miim_write(port, 0, 0, val);

                    printf("nway p%d %d %x\n", port, status, val);
                }
                break;
            #endif
        }
    }

    return;
}

void rtk_fiber_speed_set(int nway, int speed)
{
    int port, i;
    unsigned int val, val2;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (port = 48; port < (48 + gSwitchModel->phy.list[i].phy_max); ++port)
                {
                    /* check auto-det */
                    gMacDrv->drv_miim_write(port, 0, 29, 3);
                    gMacDrv->drv_miim_read(port, 8, 20, &val2);

                    val2 &= ~((1 << 12) | (7 << 13));
                    if (100 == speed)
                        val2 |= ((1 << 12) | (5 << 13));
                    else if (1000 == speed)
                        val2 |= ((1 << 12) | (4 << 13));

                    gMacDrv->drv_miim_write(port, 8, 20, val2);
                    gMacDrv->drv_miim_write(port, 0, 29, 0);
                    printf("force speed sds p%d %d %x\n", port, speed, val2);

                    gMacDrv->drv_miim_read(port, 0, 0, &val);
                    val &= ~((1 << 6) | (1 << 13) | (1 << 12));

                    if (nway)
                        val |= (1 << 12);

                    if (100 == speed)
                        val |= (1 << 13);
                    else
                        val |= (1 << 6);

                    gMacDrv->drv_miim_write(port, 0, 0, val);

                    gMacDrv->drv_miim_read(port, 0, 0, &val2);
                    printf("force speed p%d %d %x %x\n", port, speed, val, val2);
                }
                break;
            #endif
        }
    }

    return;
}

void rtk_fiber_rx_set(int status)
{
    int port, i;
    unsigned int val, val2;

    rtk_comboport_fiber();

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                for (port = 48; port < (48 + gSwitchModel->phy.list[i].phy_max); ++port)
                {
                    /* read speed */
                    gMacDrv->drv_miim_read(port, 0, 0, &val2);

                    gMacDrv->drv_miim_write(port, 0, 29, 3);

                    gMacDrv->drv_miim_read(port, 8, 20, &val);
                    val &= ~((1 << 12) | (7 << 13));
                    if (0 == status)
                    {
                        if (!(val2 & (1 << 6)) && (val2 & (1 << 13)) && !(val2 & (1 << 12)))
                        {
                            val |= (1 << 12);
                            val |= (5 << 13);
                        }
                        else
                        {
                            val |= (1 << 12);
                            val |= (4 << 13);
                        }
                    }
                    gMacDrv->drv_miim_write(port, 8, 20, val);

                    gMacDrv->drv_miim_write(port, 0, 29, 0);
                    printf("force rx sds p%d %d %x %x\n", port, status, val, val2);
                }
                break;
            #endif
        }
    }

    return;
}

void rtk_fiber_speed_get(void)
{
    int port, i;
    unsigned int val;

    for (i=0; i<gSwitchModel->phy.count; i++)
    {
        switch (gSwitchModel->phy.list[i].chip)
        {
            #if (defined(CONFIG_RTL8214FC))
            case RTK_CHIP_RTL8214FC:
                //for (port = 48; port < (48 + gSwitchModel->phy.list[i].phy_max); ++port)
                for (port = 50; port < 52; ++port)
                {

                    printf("port %d: ", port);

                    /* read reg 1 */
                    gMacDrv->drv_miim_read(port, 0, 1, &val);
                    if (0 == ((val >> 2) & 0x1))
                    {
                        printf("link down\n");
                    }
                    else
                    {
                        /* read reg 0 */
                        gMacDrv->drv_miim_read(port, 0, 0, &val);
                        if ((0 == ((val >> 6) & 0x1)) && (1 == ((val >> 13) & 0x1)))
                            printf("100M\n");
                        else if ((1 == ((val >> 6) & 0x1)) && (0 == ((val >> 13) & 0x1)))
                            printf("1G\n");
                        else
                            printf("Invalid\n");
                    }
                }
                break;
            #endif
        }
    }

    return;
}

void rtk_fiber_portLoopback(int port, int status)
{
    unsigned int val;

    rtk_comboport_portfiber(port);

    gMacDrv->drv_miim_read(port, 0, 0, &val);
    val &= ~(1 << 14);

    val |= (status << 14);
    gMacDrv->drv_miim_write(port, 0, 0, val);

    gMacDrv->drv_miim_read(port, 8, 16, &val);
    if (1 == status)
    {
        val &= ~(0xf << 6);
        val |= (0x1 << 4);
    }
    else
    {
        val &= ~(0x1 << 4);
        val |= (0xD << 6);
    }

    gMacDrv->drv_miim_write(port, 8, 16, val);
    gMacDrv->drv_miim_write(port, 0, 31, 0);

    return;
}
#endif  /* ((defined(CONFIG_RTL8214FC)) || (defined(CONFIG_RTL8218FB))) */

#if defined(CONFIG_RTL8396M_DEMO)
void rtk_10gMedia_set(int port, int media)
{
    rtl8390_10gMedia_set(port, media);

    return;
}

void rtk_10gSds_restart(int port)
{
    rtl8390_10gSds_restart(port);

    return;
}

void rtk_10gSds_init(int port)
{
    rtl8390_10gSds_init(port);

    return;
}
#endif  /* CONFIG_RTL8396M_DEMO */
