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
 * $Revision: 57050 $
 * $Date: 2015-03-23 14:36:24 +0800 (Mon, 23 Mar 2015) $
 *
 * Purpose : register service APIs in the SDK.
 *
 * Feature : register service APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_autoconf.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <ioal/mem32.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/ssw/rtk_ssw_reg_struct.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/chipdef/cypress/rtk_cypress_reg_struct.h>
#include <hal/chipdef/maple/rtk_maple_reg_struct.h>
#include <hal/common/halctrl.h>
#include <hal/mac/reg.h>
#include <hal/mac/mac_debug.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/cypress/rtk_cypress_table_struct.h>

/*
 * Data Declaration
 */
#define MAX_STRING_LEN 256
#define MAX_PHY_PORT    52
#define MAX_BUF_LEN   16
#if defined(CONFIG_SDK_RTL8390)
const static uint16 outQ_usedPage_fieldidx[] = {CYPRESS_OUT_Q_Q0_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q1_USED_PAGE_CNTtf,
                                                CYPRESS_OUT_Q_Q2_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q3_USED_PAGE_CNTtf,
                                                CYPRESS_OUT_Q_Q4_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q5_USED_PAGE_CNTtf,
                                                CYPRESS_OUT_Q_Q6_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q7_USED_PAGE_CNTtf};
const static uint16 outQ_maxUsedPage_fieldidx[] = {CYPRESS_OUT_Q_Q0_MAX_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q1_MAX_USED_PAGE_CNTtf,
                                                CYPRESS_OUT_Q_Q2_MAX_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q3_MAX_USED_PAGE_CNTtf,
                                                CYPRESS_OUT_Q_Q4_MAX_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q5_MAX_USED_PAGE_CNTtf,
                                                CYPRESS_OUT_Q_Q6_MAX_USED_PAGE_CNTtf, CYPRESS_OUT_Q_Q7_MAX_USED_PAGE_CNTtf};
#endif

enum LAYER34FMT
{
	LAYER34_UKONWN = 0,
	IPPRO_ARP,
	IPV4_ICMP,
	IPV4_IGMP,
	IPV4_TCP,
	IPV4_UDP,
	IPV4_UNKNOWN, /*6*/
	IPV6_ICMPV6,
	IPV6_TCP,
	IPV6_UDP,
	IPV6_UNKNOWN,  /*10*/
	L34FMT_MAX
};

extern uint32 pktBuf_watchdog_cnt;
extern uint32 macSerdes_watchdog_cnt;
extern uint32 phy_watchdog_cnt;
extern uint32 fiber_rx_watchdog_cnt;

/*
 * Function Declaration
 */
#if defined(CONFIG_SDK_RTL8328)
static int32
_diag_util_port2LPortMask_get(rtk_portmask_t *pstLPortMask, unsigned char ucPortId)
{
    if (ucPortId > RTK_MAX_NUM_OF_PORTS - 1)
    {
        return RT_ERR_FAILED;
    }

    if (pstLPortMask->bits[ucPortId / MASK_BIT_LEN] & (1 << (ucPortId % MASK_BIT_LEN)))
    {
        return RT_ERR_OK;
    }
    else
    {
        return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

/* convert logical port mask to string, separated by ","s */
static int32 lPortMask2str (uint8 *comma, rtk_portmask_t *pstLPortMask)
{
    int32   first = 0;
    int32   begin = 0;
    int32   end = 0;
    uint32  i = 0;
    uint8   buf[MAX_BUF_LEN];

    if ((NULL == comma) || (NULL == pstLPortMask))
    {
        return RT_ERR_FAILED;
    }

    memset(buf, 0, MAX_BUF_LEN);

    comma[0] = '\0';

    first = 1;
    begin = -1;
    end = -1;

    for (i = 0; i <= MAX_PHY_PORT; ++i)
    {
        if (RT_ERR_OK == _diag_util_port2LPortMask_get(pstLPortMask, i))
        {

            if (1 == first)
            {
                first = 0;
            }

            if (-1 == begin)
            {
                begin = end = i;
            }
            else
            {
                end = i;
            }

        }
        else
        {
            if ((0 == first) && (begin != -1))
            {
                first = -1;
            }
            else if ((-1 == first) && (begin != -1))
            {
                sprintf(buf, ",");
                strcat(comma, buf);
            }

            if ((begin != -1) && (begin == end))
            {
                sprintf(buf, "%d", begin);
                strcat(comma, buf);
            }
            else if (begin != -1)
            {
                sprintf(buf, "%d-%d", begin, end);
                strcat(comma, buf);
            }

            begin = -1;
            end = -1;
        }
    }

    if ((begin != -1) || (end != -1))
    {
        if (-1 == first)
        {
            sprintf(buf, ",");
            strcat(comma, buf);
        }
        if (begin == end)
        {
            sprintf(buf, "%d", begin);
            strcat(comma, buf);
        }
        else
        {
            sprintf(buf, "%d-%d", begin, end);
            strcat(comma, buf);
        }
    }
#if 0
    for (i = MAX_PHY_N_CPU_PORT; i < MAX_LOGIC_PORT; ++i)
    {
        if (RT_ERR_OK == _diag_util_port2LPortMask_get(pstLPortMask, i))
        {
            if (1 == first)
            {
                first = 0;
                sprintf(buf, "Trunk%d", i - MAX_PORT - 1 + 1);

            }
            else
            {
                sprintf(buf, ",Trunk%d", i - MAX_PORT - 1 + 1);
            }
            strcat(comma, buf);
        }
    }
#endif
    return RT_ERR_OK;
} /* end of lPortMask2str */
#endif

#if defined(CONFIG_SDK_RTL8328)
int32 _rtk_getPpi(uint32 unit, uint32 index, ppi_param_t * ppi)
{
    uint32 value, tmpVal;
    uint32 baseAddr;

    if(NULL == ppi)
    {
        return RT_ERR_FAILED;
    }

    memset(ppi, 0, sizeof(ppi_param_t));

    switch (index)
    {
    case 0:
        baseAddr = ESW_PACKET_PROCESS_INFORMATION0_CR0r;
        break;
    case 1:
        baseAddr = ESW_PACKET_PROCESS_INFORMATION1_CR0r;
        break;
    case 2:
        baseAddr = ESW_PACKET_PROCESS_INFORMATION2_CR0r;
        break;
    case 3:
        baseAddr = ESW_PACKET_PROCESS_INFORMATION3_CR0r;
        break;
    case 4:
        baseAddr = ESW_PACKET_PROCESS_INFORMATION4_CR0r;
        break;
    case 5:
        baseAddr = ESW_PACKET_PROCESS_INFORMATION5_CR0r;
        break;
    case 6:
        baseAddr = ESW_PACKET_PROCESS_INFORMATION6_CR0r;
        break;
    default:
        return RT_ERR_FAILED;
    }

    reg_read(unit, baseAddr, &value);
    reg_field_get(unit, baseAddr, ESW_SPHYf, &tmpVal, &value);
    ppi->sphy = tmpVal;
    reg_field_get(unit, baseAddr, ESW_CRSVLANf, &tmpVal, &value);
    ppi->crsvlan = tmpVal;
    reg_field_get(unit, baseAddr, ESW_KEEPORIGIVIDf, &tmpVal, &value);
    ppi->keeporigivid = tmpVal;
    reg_field_get(unit, baseAddr, ESW_KEEPORIGOVIDf, &tmpVal, &value);
    ppi->keeporigovid = tmpVal;
    reg_field_get(unit, baseAddr, ESW_PBIVIDf, &tmpVal, &value);
    ppi->pbivid = tmpVal;
    reg_field_get(unit, baseAddr, ESW_PBOVIDf, &tmpVal, &value);
    ppi->pbovid = tmpVal;

    reg_read(unit, baseAddr + 1, &value);
    reg_field_get(unit, baseAddr + 1, ESW_PBIPRIf, &tmpVal, &value);
    ppi->pbipri = tmpVal;
    reg_field_get(unit, baseAddr + 1, ESW_PBODEIf, &tmpVal, &value);
    ppi->pbodei = tmpVal;
    reg_field_get(unit, baseAddr + 1, ESW_IPUCf, &tmpVal, &value);
    ppi->ipuc = tmpVal;
    reg_field_get(unit, baseAddr + 1, ESW_PBOPRIf, &tmpVal, &value);
    ppi->pbopri = tmpVal;
    reg_field_get(unit, baseAddr + 1, ESW_PPBIVIDf, &tmpVal, &value);
    ppi->ppbivid = tmpVal;
    reg_field_get(unit, baseAddr + 1, ESW_PPBOVIDf, &tmpVal, &value);
    ppi->ppbovid = tmpVal;

    reg_read(unit, baseAddr + 2, &value);
    reg_field_get(unit, baseAddr + 2, ESW_PPBIPRIf, &tmpVal, &value);
    ppi->ppbipri = tmpVal;
    reg_field_get(unit, baseAddr + 2, ESW_PPBODEIf, &tmpVal, &value);
    ppi->ppbodei = tmpVal;
    reg_field_get(unit, baseAddr + 2, ESW_UPLINKMAC_0f, &tmpVal, &value);
    ppi->upLinkMac_0 = tmpVal;
    reg_field_get(unit, baseAddr + 2, ESW_PPBOPRIf, &tmpVal, &value);
    ppi->ppbopri = tmpVal;
    reg_field_get(unit, baseAddr + 2, ESW_FCIVIDf, &tmpVal, &value);
    ppi->fcivid = tmpVal;
    reg_field_get(unit, baseAddr + 2, ESW_FCOVIDf, &tmpVal, &value);
    ppi->fcovid = tmpVal;

    reg_read(unit, baseAddr + 3, &value);
    reg_field_get(unit, baseAddr + 3, ESW_TTLDECf, &tmpVal, &value);
    ppi->ttldec = tmpVal;
    reg_field_get(unit, baseAddr + 3, ESW_FCSELDOT1QIPRIf, &tmpVal, &value);
    ppi->fcseldot1qipri = tmpVal;
    reg_field_get(unit, baseAddr + 3, ESW_FCSELDOT1QOPRIf, &tmpVal, &value);
    ppi->fcseldot1qopri = tmpVal;
    reg_field_get(unit, baseAddr + 3, ESW_RRCPTYPEf, &tmpVal, &value);
    ppi->rrcpType = tmpVal;
    reg_field_get(unit, baseAddr + 3, ESW_RRCPAUTHf, &tmpVal, &value);
    ppi->rrcpAuth = tmpVal;
    reg_field_get(unit, baseAddr + 3, ESW_IVIDf, &tmpVal, &value);
    ppi->ivid = tmpVal;
    reg_field_get(unit, baseAddr + 3, ESW_OVIDf, &tmpVal, &value);
    ppi->ovid = tmpVal;
    reg_field_get(unit, baseAddr + 3, ESW_DROPf, &tmpVal, &value);
    ppi->drop = tmpVal;

    reg_read(unit, baseAddr + 4, &value);
    reg_field_get(unit, baseAddr + 4, ESW_CPUf, &tmpVal, &value);
    ppi->cpu = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_CPUTAGf, &tmpVal, &value);
    ppi->cputag = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_TOGVLANf, &tmpVal, &value);
    ppi->toGVlan = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_IPMCPKTf, &tmpVal, &value);
    ppi->ipmcPkt = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_DOT1QODPf, &tmpVal, &value);
    ppi->dot1qodp = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_SMLf, &tmpVal, &value);
    ppi->sml = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_REDIRf, &tmpVal, &value);
    ppi->redir = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_LKMISSf, &tmpVal, &value);
    ppi->lkmiss = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_FTIDXf, &tmpVal, &value);
    ppi->ftidx = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_DPNf, &tmpVal, &value);
    ppi->dpn = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_PBDPf, &tmpVal, &value);
    ppi->pbdp = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_PBPRIf, &tmpVal, &value);
    ppi->pbpri = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_FTIDXVALIDf, &tmpVal, &value);
    ppi->ftIdxValid = tmpVal;
    reg_field_get(unit, baseAddr + 4, ESW_DOT1QIDPf, &tmpVal, &value);
    ppi->dot1qidp = tmpVal;

    reg_read(unit, baseAddr + 5, &value);
    reg_field_get(unit, baseAddr + 5, ESW_DPMf, &tmpVal, &value);
    ppi->dpm = tmpVal;
    reg_field_get(unit, baseAddr + 5, ESW_DOT1QIPRIf, &tmpVal, &value);
    ppi->dot1qipri = tmpVal;

    reg_read(unit, baseAddr + 6, &value);
    reg_field_get(unit, baseAddr + 6, ESW_DSCPPRIEXISTf, &tmpVal, &value);
    ppi->dscppriexist = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_DSCPDPf, &tmpVal, &value);
    ppi->dscpdp = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_DSCPPRIf, &tmpVal, &value);
    ppi->dscppri = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_FBPRIEXISTf, &tmpVal, &value);
    ppi->fbpriexist = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_FBDPf, &tmpVal, &value);
    ppi->fbdp = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_FBPRIf, &tmpVal, &value);
    ppi->fbpri = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_IPRIEXISTf, &tmpVal, &value);
    ppi->ipriexist = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_IDPf, &tmpVal, &value);
    ppi->idp = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_IPRIf, &tmpVal, &value);
    ppi->ipri = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_OPRIEXISTf, &tmpVal, &value);
    ppi->opriexist = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_ODPf, &tmpVal, &value);
    ppi->odp = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_OPRIf, &tmpVal, &value);
    ppi->opri = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_DPf, &tmpVal, &value);
    ppi->dp = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_PRIf, &tmpVal, &value);
    ppi->pri = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_MACDOT1Xf, &tmpVal, &value);
    ppi->macdot1x = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_FCDFIf, &tmpVal, &value);
    ppi->fcdfi = tmpVal;
    reg_field_get(unit, baseAddr + 6, ESW_FCDFOf, &tmpVal, &value);
    ppi->fcdfo = tmpVal;

    reg_read(unit, baseAddr + 7, &value);
    reg_field_get(unit, baseAddr + 7, ESW_TMIDXf, &tmpVal, &value);
    ppi->tmidx = tmpVal;
    reg_field_get(unit, baseAddr + 7, ESW_VIDRCHf, &tmpVal, &value);
    ppi->vidrch = tmpVal;
    reg_field_get(unit, baseAddr + 7, ESW_FWDf, &tmpVal, &value);
    ppi->fwd = tmpVal;
    reg_field_get(unit, baseAddr + 7, ESW_MIRf, &tmpVal, &value);
    ppi->mir = tmpVal;
    reg_field_get(unit, baseAddr + 7, ESW_MIRORGf, &tmpVal, &value);
    ppi->mirorg = tmpVal;
    reg_field_get(unit, baseAddr + 7, ESW_SPMf, &tmpVal, &value);
    ppi->spm = tmpVal;

    reg_read(unit, baseAddr + 8, &value);
    reg_field_get(unit, baseAddr + 8, ESW_MSTIf, &tmpVal, &value);
    ppi->msti = tmpVal;
    reg_field_get(unit, baseAddr + 8, ESW_FIDf, &tmpVal, &value);
    ppi->fid = tmpVal;
    reg_field_get(unit, baseAddr + 8, ESW_MIRIUTAGf, &tmpVal, &value);
    ppi->miriutag = tmpVal;
    reg_field_get(unit, baseAddr + 8, ESW_MIROUTAGf, &tmpVal, &value);
    ppi->miroutag = tmpVal;

    reg_read(unit, baseAddr + 9, &value);
    reg_field_get(unit, baseAddr + 9, ESW_FLOODf, &tmpVal, &value);
    ppi->flood = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_PPBDFIf, &tmpVal, &value);
    ppi->ppbdfi = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_PPBDFOf, &tmpVal, &value);
    ppi->ppbdfo = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_TMf, &tmpVal, &value);
    ppi->tm = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_IPMCf, &tmpVal, &value);
    ppi->ipmc = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_COPYTOCPUf, &tmpVal, &value);
    ppi->copyToCpu = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_VIDRCf, &tmpVal, &value);
    ppi->vidrc = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_REASONf, &tmpVal, &value);
    ppi->reason = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_DSCPRMKf, &tmpVal, &value);
    ppi->dscprmk = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_DOT1QOPRIf, &tmpVal, &value);
    ppi->dot1qopri = tmpVal;
    reg_field_get(unit, baseAddr + 9, ESW_CEf, &tmpVal, &value);
    ppi->ce = tmpVal;

    reg_read(unit, baseAddr + 10, &value);
    reg_field_get(unit, baseAddr + 10, ESW_IUTAGSTATUSf, &tmpVal, &value);
    ppi->iutagstatus = tmpVal;
    reg_field_get(unit, baseAddr + 10, ESW_IUNTAGVALIDf, &tmpVal, &value);
    ppi->iuntagValid = tmpVal;
    reg_field_get(unit, baseAddr + 10, ESW_OUNTAGVALIDf, &tmpVal, &value);
    ppi->ountagValid = tmpVal;

    reg_read(unit, baseAddr + 11, &value);
    reg_field_get(unit, baseAddr + 11, ESW_OUTAGSTATUSf, &tmpVal, &value);
    ppi->outagstatus = tmpVal;

    reg_read(unit, baseAddr + 12, &value);
    reg_field_get(unit, baseAddr + 12, ESW_DSCPf, &tmpVal, &value);
    ppi->dscp = tmpVal;
    reg_field_get(unit, baseAddr + 12, ESW_ORGVIDf, &tmpVal, &value);
    ppi->orgvid = tmpVal;
    reg_field_get(unit, baseAddr + 12, ESW_DMACIDXf, &tmpVal, &value);
    ppi->dmacidx = tmpVal;

    reg_read(unit, baseAddr + 13, &value);
    reg_field_get(unit, baseAddr + 13, ESW_RRCPREGDATAf, &tmpVal, &value);
    ppi->rrcpRegData = tmpVal;

    reg_read(unit, baseAddr + 14, &value);
    reg_field_get(unit, baseAddr + 14, ESW_PRCf, &tmpVal, &value);
    ppi->prc = tmpVal;
    ppi->iprc[6] = value & 0xff;
    ppi->iprc[7] = (value >> 8) & 0xff;

    reg_read(unit, baseAddr + 15, &value);
    ppi->iprc[2] = value & 0xff;
    ppi->iprc[3] = (value >> 8) & 0xff;
    ppi->iprc[4] = (value >> 16) & 0xff;
    ppi->iprc[5] = (value >> 24) & 0xff;

    reg_read(unit, baseAddr + 16, &value);
    ppi->iprc[0] = (value >> 16) & 0xff;
    ppi->iprc[1] = (value >> 24) & 0xff;
    reg_field_get(unit, baseAddr + 16, ESW_ACLIDX0f, &tmpVal, &value);
    ppi->aclidx0 = tmpVal;
    reg_field_get(unit, baseAddr + 16, ESW_ACLIDX1f, &tmpVal, &value);
    ppi->aclidx1 = tmpVal;

    reg_read(unit, baseAddr + 17, &value);
    reg_field_get(unit, baseAddr + 17, ESW_ACLIDX2f, &tmpVal, &value);
    ppi->aclidx2 = tmpVal;
    reg_field_get(unit, baseAddr + 17, ESW_ACLIDX3f, &tmpVal, &value);
    ppi->aclidx3 = tmpVal;
    reg_field_get(unit, baseAddr + 17, ESW_ACLIDX4f, &tmpVal, &value);
    ppi->aclidx4 = tmpVal;
    reg_field_get(unit, baseAddr + 17, ESW_ACLIDX5f, &tmpVal, &value);
    ppi->aclidx5 = tmpVal;

    reg_read(unit, baseAddr + 18, &value);
    reg_field_get(unit, baseAddr + 18, ESW_ACLIDX6f, &tmpVal, &value);
    ppi->aclidx6 = tmpVal;
    reg_field_get(unit, baseAddr + 18, ESW_ACLIDX7f, &tmpVal, &value);
    ppi->aclidx7 = tmpVal;
    reg_field_get(unit, baseAddr + 18, ESW_ACLIDX8f, &tmpVal, &value);
    ppi->aclidx8 = tmpVal;
    reg_field_get(unit, baseAddr + 18, ESW_ACLIDX9f, &tmpVal, &value);
    ppi->aclidx9 = tmpVal;

    reg_read(unit, baseAddr + 19, &value);
    reg_field_get(unit, baseAddr + 19, ESW_ACLIDX10f, &tmpVal, &value);
    ppi->aclidx10 = tmpVal;
    reg_field_get(unit, baseAddr + 19, ESW_ACLIDX11f, &tmpVal, &value);
    ppi->aclidx11 = tmpVal;
    reg_field_get(unit, baseAddr + 19, ESW_ACLIDX12f, &tmpVal, &value);
    ppi->aclidx12 = tmpVal;
    reg_field_get(unit, baseAddr + 19, ESW_ACLIDX13f, &tmpVal, &value);
    ppi->aclidx13 = tmpVal;

    reg_read(unit, baseAddr + 20, &value);
    reg_field_get(unit, baseAddr + 20, ESW_ACLIDX14f, &tmpVal, &value);
    ppi->aclidx14 = tmpVal;
    reg_field_get(unit, baseAddr + 20, ESW_ACLIDX15f, &tmpVal, &value);
    ppi->aclidx15 = tmpVal;

    return RT_ERR_OK;
}
#endif

#if defined(CONFIG_SDK_RTL8328)
int32 _rtk_getPmi(uint32 unit, pmi_param_t * pmi)
{
    uint32 value, tmpVal;

    if(NULL == pmi)
    {
        return RT_ERR_FAILED;
    }

    memset(pmi, 0, sizeof(pmi_param_t));

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR0r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR0r, ESW_DPMf, &tmpVal, &value);
    pmi->dpm = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR0r, ESW_L2ERRf, &tmpVal, &value);
    pmi->l2err = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR0r, ESW_L3ERRf, &tmpVal, &value);
    pmi->l3err = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR0r, ESW_PPPOEf, &tmpVal, &value);
    pmi->pppoe = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_FWDf, &tmpVal, &value);
    pmi->fwd = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_MIRf, &tmpVal, &value);
    pmi->mir = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_MIRORGf, &tmpVal, &value);
    pmi->mirorg = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_PRIf, &tmpVal, &value);
    pmi->pri = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_RXRSPANf, &tmpVal, &value);
    pmi->rxrspan = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_MIRIUTAGf, &tmpVal, &value);
    pmi->miriutag = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_MIROUTAGf, &tmpVal, &value);
    pmi->miroutag = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_STPHYf, &tmpVal, &value);
    pmi->stphy = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_RRCPTYPEf, &tmpVal, &value);
    pmi->rrcpType = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR1r, ESW_RXCPUTAGf, &tmpVal, &value);
    pmi->rxcpuTag = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR2r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR2r, ESW_DPCNTf, &tmpVal, &value);
    pmi->dpcnt = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR2r, ESW_PNXTf, &tmpVal, &value);
    pmi->pnxt = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR2r, ESW_REASONf, &tmpVal, &value);
    pmi->reason = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_FRAMELENGTHf, &tmpVal, &value);
    pmi->framelength = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_DPf, &tmpVal, &value);
    pmi->dp = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_LBf, &tmpVal, &value);
    pmi->lb = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_OCCUPYf, &tmpVal, &value);
    pmi->occupy = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_SPHYf, &tmpVal, &value);
    pmi->sphy = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_EXTRATAGf, &tmpVal, &value);
    pmi->extraTag = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_ASDPf, &tmpVal, &value);
    pmi->asdp = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_ASDPRMKf, &tmpVal, &value);
    pmi->asdprmk = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_EVTENf, &tmpVal, &value);
    pmi->evten = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_TTLDECf, &tmpVal, &value);
    pmi->ttldec = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_TXCPUTAGf, &tmpVal, &value);
    pmi->txcputag = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR3r, ESW_DOPRIf, &tmpVal, &value);
    pmi->dopri = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR4r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR4r, ESW_DIVIDf, &tmpVal, &value);
    pmi->divid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR4r, ESW_L2FMTf, &tmpVal, &value);
    pmi->l2fmt = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR4r, ESW_DOVIDf, &tmpVal, &value);
    pmi->dovid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR4r, ESW_DIPRIf, &tmpVal, &value);
    pmi->dipri = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR4r, ESW_L4ERRf, &tmpVal, &value);
    pmi->l4err = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR5r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR5r, ESW_IUTAGSTATUSf, &tmpVal, &value);
    pmi->iutagStatus = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR5r, ESW_DSCPRMKf, &tmpVal, &value);
    pmi->dscprmk = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR5r, ESW_OAM_LBSAf, &tmpVal, &value);
    pmi->oam_lbsa = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, ESW_IPUCf, &tmpVal, &value);
    pmi->ipuc = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, ESW_IPMCf, &tmpVal, &value);
    pmi->ipmc = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, ESW_L34FMTf, &tmpVal, &value);
    pmi->l34fmt = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, ESW_DMACIDXf, &tmpVal, &value);
    pmi->dmacidx = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, ESW_DSCPf, &tmpVal, &value);
    pmi->dscp = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, ESW_CEf, &tmpVal, &value);
    pmi->ce = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR6r, ESW_RRCPREGDATA31_27f, &tmpVal, &value);
    pmi->rrcpRegData31_27 = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR7r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR7r, ESW_OUTAGSTATUSf, &tmpVal, &value);
    pmi->outagStatus = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR7r, ESW_RRCPf, &tmpVal, &value);
    pmi->rrcp = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR7r, ESW_OAM_LBDAf, &tmpVal, &value);
    pmi->oam_lbda = tmpVal;

    reg_read(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, &value);
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_KEEPORIGIVIDf, &tmpVal, &value);
    pmi->keeporigivid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_KEEPORIGOVIDf, &tmpVal, &value);
    pmi->keeporigovid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_DIPRIVALIDf, &tmpVal, &value);
    pmi->diprivalid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_DCFIVALIDf, &tmpVal, &value);
    pmi->dcfivalid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_DCFIf, &tmpVal, &value);
    pmi->dcfi = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_DOPRIVALIDf, &tmpVal, &value);
    pmi->doprivalid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_DDEIVALIDf, &tmpVal, &value);
    pmi->ddeivalid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_DDEIf, &tmpVal, &value);
    pmi->ddei = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_SPIDIDXVALIDf, &tmpVal, &value);
    pmi->spididxvalid = tmpVal;
    reg_field_get(unit, ESW_PACKET_MODIFICATION_INFORMATION_CR8r, ESW_SPIDIDXf, &tmpVal, &value);
    pmi->spididx = tmpVal;

    return RT_ERR_OK;
}
#endif

int32 _rtk_getHsb(uint32 unit, hsb_param_t * hsb)
{
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8328) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    uint32 value, tmpVal;
#endif
    hal_control_t *pInfo;

    if(NULL == hsb)
    {
        return RT_ERR_FAILED;
    }

    memset(hsb, 0, sizeof(hsb_param_t));
    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8389M_CHIP_ID:
        case RTL8389L_CHIP_ID:
        case RTL8329M_CHIP_ID:
        case RTL8377M_CHIP_ID:
#if defined(CONFIG_SDK_RTL8389)
        {
            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL0r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL0r, SSW_SEL_HSBf, &tmpVal, &value);
            hsb->r8389.sel_hsb = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL0r, SSW_VALID_HSBf, &tmpVal, &value);
            hsb->r8389.valid_hsb = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL0r, SSW_CFIf, &tmpVal, &value);
            hsb->r8389.cfi = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL0r, SSW_PATTERNMATCHf, &tmpVal, &value);
            hsb->r8389.patternmatch = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL0r, SSW_FLOWLABELf, &tmpVal, &value);
            hsb->r8389.flowlabel = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL1r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL1r, SSW_DSTPORTf, &tmpVal, &value);
            hsb->r8389.dstport = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL1r, SSW_SRCPORTf, &tmpVal, &value);
            hsb->r8389.srcport = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL2r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL2r, SSW_TCPFLAGSf, &tmpVal, &value);
            hsb->r8389.tcpflags = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL2r, SSW_IPPROTOf, &tmpVal, &value);
            hsb->r8389.ipproto = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL2r, SSW_SVIDf, &tmpVal, &value);
            hsb->r8389.svid = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL2r, SSW_SPRIf, &tmpVal, &value);
            hsb->r8389.spri = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL2r, SSW_RXDROPf, &tmpVal, &value);
            hsb->r8389.rxdrop = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL3r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL3r, SSW_CPUTAGIFf, &tmpVal, &value);
            hsb->r8389.cputagif = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL3r, SSW_CPUINTPRIf, &tmpVal, &value);
            hsb->r8389.cpuintpri = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL3r, SSW_CPUPORTMASKf, &tmpVal, &value);
            hsb->r8389.cpuportmask = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL4r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL4r, SSW_ETHTYPEf, &tmpVal, &value);
            hsb->r8389.ethtype = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL4r, SSW_IPV6MLDf, &tmpVal, &value);
            hsb->r8389.ipv6mld = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL4r, SSW_CPRIf, &tmpVal, &value);
            hsb->r8389.cpri = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL4r, SSW_CVIDf, &tmpVal, &value);
            hsb->r8389.cvid = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL5r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL5r, SSW_DIPf, &tmpVal, &value);
            hsb->r8389.dip = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL6r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL6r, SSW_SIPf, &tmpVal, &value);
            hsb->r8389.sip = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL7r, &value);
            hsb->r8389.dmac[3] = value & 0xff;
            hsb->r8389.dmac[2] = (value >> 8) & 0xff;
            hsb->r8389.dmac[1] = (value >> 16) & 0xff;
            hsb->r8389.dmac[0] = (value >> 24) & 0xff;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL8r, &value);
            hsb->r8389.smac[1] = value & 0xff;
            hsb->r8389.smac[0] = (value >> 8) & 0xff;
            hsb->r8389.dmac[5] = (value >> 16) & 0xff;
            hsb->r8389.dmac[4] = (value >> 24) & 0xff;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL9r, &value);
            hsb->r8389.smac[5] = value & 0xff;
            hsb->r8389.smac[4] = (value >> 8) & 0xff;
            hsb->r8389.smac[3] = (value >> 16) & 0xff;
            hsb->r8389.smac[2] = (value >> 24) & 0xff;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_TOSf, &tmpVal, &value);
            hsb->r8389.tos = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_CPUASDPf, &tmpVal, &value);
            hsb->r8389.cpuasdp = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_CPUASDPMf, &tmpVal, &value);
            hsb->r8389.cpuasdpm = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_CPUASDPRMKf, &tmpVal, &value);
            hsb->r8389.cpuasdprmk = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_IPV6f, &tmpVal, &value);
            hsb->r8389.ipv6 = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW___IPV4f, &tmpVal, &value);
            hsb->r8389.ipv4 = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_PPPOEf, &tmpVal, &value);
            hsb->r8389.pppoe = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_STAGIFf, &tmpVal, &value);
            hsb->r8389.stagif = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_CTAGIFf, &tmpVal, &value);
            hsb->r8389.ctagif = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_FRAMETYPEf, &tmpVal, &value);
            hsb->r8389.frametype = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL10r, SSW_PKTLENf, &tmpVal, &value);
            hsb->r8389.pktlen = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_BEFORE_CONTROL11r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL11r, SSW_L4CSOKf, &tmpVal, &value);
            hsb->r8389.l4csok = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL11r, SSW_L3CSOKf, &tmpVal, &value);
            hsb->r8389.l3csok = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL11r, SSW_ENDPAGEf, &tmpVal, &value);
            hsb->r8389.endpage = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL11r, SSW_STARTPAGEf, &tmpVal, &value);
            hsb->r8389.startpage = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL11r, SSW_STARTBANKf, &tmpVal, &value);
            hsb->r8389.startbank = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_BEFORE_CONTROL11r, SSW_SPAf, &tmpVal, &value);
            hsb->r8389.spa = tmpVal;
        }
#endif
            break;

        case RTL8328M_CHIP_ID:
        case RTL8328S_CHIP_ID:
        case RTL8328L_CHIP_ID:
#if defined(CONFIG_SDK_RTL8328)
        {
            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR0r, &value);
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_SPHYf, &tmpVal, &value);
            hsb->r8328.sphy = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_L2FMTf, &tmpVal, &value);
            hsb->r8328.l2fmt = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_PPPOEf, &tmpVal, &value);
            hsb->r8328.pppoe = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_L34FMTf, &tmpVal, &value);
            hsb->r8328.l34fmt = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_L4OFFf, &tmpVal, &value);
            hsb->r8328.l4off = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_IPFO0_Nf, &tmpVal, &value);
            hsb->r8328.ipfo0_n = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_DATYPEf, &tmpVal, &value);
            hsb->r8328.datype = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_OAMPDUf, &tmpVal, &value);
            hsb->r8328.oampdu = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_CPUTAGf, &tmpVal, &value);
            hsb->r8328.cputag = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_OTAGEXISTf, &tmpVal, &value);
            hsb->r8328.otagExist = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_OPRIf, &tmpVal, &value);
            hsb->r8328.opri = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR0r, ESW_DEIf, &tmpVal, &value);
            hsb->r8328.dei = tmpVal;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR1r, &value);
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR1r, ESW_OVIDf, &tmpVal, &value);
            hsb->r8328.ovid = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR1r, ESW_ITAGEXISTf, &tmpVal, &value);
            hsb->r8328.itagExist = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR1r, ESW_IPRIf, &tmpVal, &value);
            hsb->r8328.ipri = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR1r, ESW_CFIf, &tmpVal, &value);
            hsb->r8328.cfi = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR1r, ESW_FRAMELENf, &tmpVal, &value);
            hsb->r8328.frameLen = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR1r, ESW_IPV4OPHf, &tmpVal, &value);
            hsb->r8328.ipv4oph = tmpVal;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR2r, &value);
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR2r, ESW_IVIDf, &tmpVal, &value);
            hsb->r8328.ivid = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR2r, ESW_TYPELENf, &tmpVal, &value);
            hsb->r8328.typeLen = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR2r, ESW_RTKPROf, &tmpVal, &value);
            hsb->r8328.rtkPro = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR2r, ESW_LBf, &tmpVal, &value);
            hsb->r8328.lb = tmpVal;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR3r, &value);
            hsb->r8328.fs1[8] = value >> 31;
            hsb->r8328.fs1[6] = value >> 23;
            hsb->r8328.fs1[7] = value >> 15;
            hsb->r8328.fs1[4] = value >> 7;
            hsb->r8328.fs1[5] = (value << 1) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR4r, &value);
            hsb->r8328.fs1[5] |= (value >> 31) & 0xff;
            hsb->r8328.fs1[2] = value >> 23;
            hsb->r8328.fs1[3] = value >> 15;
            hsb->r8328.fs1[0] = value >> 7;
            hsb->r8328.fs1[1] = value << 1;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR5r, &value);
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_FS1_64f, &tmpVal, &value);
            hsb->r8328.fs1[1] |= tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_L2ERRf, &tmpVal, &value);
            hsb->r8328.l2err = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_L3ERRf, &tmpVal, &value);
            hsb->r8328.l3err = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_L4ERRf, &tmpVal, &value);
            hsb->r8328.l4err = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_L2CALCf, &tmpVal, &value);
            hsb->r8328.l2calc = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_L3CALCf, &tmpVal, &value);
            hsb->r8328.l3calc = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_L4CALCf, &tmpVal, &value);
            hsb->r8328.l4calc = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_DFPRIf, &tmpVal, &value);
            hsb->r8328.dfPri = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_OAMTFf, &tmpVal, &value);
            hsb->r8328.oamTf = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_TRAPTOCPUf, &tmpVal, &value);
            hsb->r8328.trapToCpu = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_INSERTCPUTAGf, &tmpVal, &value);
            hsb->r8328.insertCpuTag = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_OAM_LBSAf, &tmpVal, &value);
            hsb->r8328.oam_lbsa = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_OAM_LBDAf, &tmpVal, &value);
            hsb->r8328.oam_lbda = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR5r, ESW_FLAGf, &tmpVal, &value);
            hsb->r8328.flag = tmpVal;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR6r, &value);
            hsb->r8328.fs2[8] = value >> 31;
            hsb->r8328.fs2[6] = value >> 23;
            hsb->r8328.fs2[7] = value >> 15;
            hsb->r8328.fs2[4] = value >> 7;
            hsb->r8328.fs2[5] = value << 1;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR7r, &value);
            hsb->r8328.fs2[5] |= (value >> 31);
            hsb->r8328.fs2[2] = value >> 23;
            hsb->r8328.fs2[3] = value >> 15;
            hsb->r8328.fs2[0] = value >> 7;
            hsb->r8328.fs2[1] = value << 1;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR8r, &value);
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR8r, ESW_FS2_64f, &tmpVal, &value);
            hsb->r8328.fs2[1] |= tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR8r, ESW_DPMf, &tmpVal, &value);
            hsb->r8328.dpm = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR8r, ESW_IPV6OPHf, &tmpVal, &value);
            hsb->r8328.ipv6oph = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR8r, ESW_RXRSPANf, &tmpVal, &value);
            hsb->r8328.rxrspan = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR8r, ESW_EXTRATAGf, &tmpVal, &value);
            hsb->r8328.extraTag = tmpVal;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR9r, &value);
            hsb->r8328.dmac[3] = value & 0xff;
            hsb->r8328.dmac[2] = (value >> 8) & 0xff;
            hsb->r8328.dmac[1] = (value >> 16) & 0xff;
            hsb->r8328.dmac[0] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR10r, &value);
            hsb->r8328.smac[1] = value & 0xff;
            hsb->r8328.smac[0] = (value >> 8) & 0xff;
            hsb->r8328.dmac[5] = (value >> 16) & 0xff;
            hsb->r8328.dmac[4] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR11r, &value);
            hsb->r8328.smac[5] = value & 0xff;
            hsb->r8328.smac[4] = (value >> 8) & 0xff;
            hsb->r8328.smac[3] = (value >> 16) & 0xff;
            hsb->r8328.smac[2] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR12r, &value);
            hsb->r8328.payload[0] = value & 0xff;
            hsb->r8328.payload[1] = (value >> 8) & 0xff;
            hsb->r8328.payload[2] = (value >> 16) & 0xff;
            hsb->r8328.payload[3] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR13r, &value);
            hsb->r8328.payload[4] = value & 0xff;
            hsb->r8328.payload[5] = (value >> 8) & 0xff;
            hsb->r8328.payload[6] = (value >> 16) & 0xff;
            hsb->r8328.payload[7] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR14r, &value);
            hsb->r8328.payload[8] = value & 0xff;
            hsb->r8328.payload[9] = (value >> 8) & 0xff;
            hsb->r8328.payload[10] = (value >> 16) & 0xff;
            hsb->r8328.payload[11] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR15r, &value);
            hsb->r8328.payload[12] = value & 0xff;
            hsb->r8328.payload[13] = (value >> 8) & 0xff;
            hsb->r8328.payload[14] = (value >> 16) & 0xff;
            hsb->r8328.payload[15] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR16r, &value);
            hsb->r8328.payload[16] = value & 0xff;
            hsb->r8328.payload[17] = (value >> 8) & 0xff;
            hsb->r8328.payload[18] = (value >> 16) & 0xff;
            hsb->r8328.payload[19] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR17r, &value);
            hsb->r8328.payload[20] = value & 0xff;
            hsb->r8328.payload[21] = (value >> 8) & 0xff;
            hsb->r8328.payload[22] = (value >> 16) & 0xff;
            hsb->r8328.payload[23] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR18r, &value);
            hsb->r8328.payload[24] = value & 0xff;
            hsb->r8328.payload[25] = (value >> 8) & 0xff;
            hsb->r8328.payload[26] = (value >> 16) & 0xff;
            hsb->r8328.payload[27] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR19r, &value);
            hsb->r8328.payload[28] = value & 0xff;
            hsb->r8328.payload[29] = (value >> 8) & 0xff;
            hsb->r8328.payload[30] = (value >> 16) & 0xff;
            hsb->r8328.payload[31] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR20r, &value);
            hsb->r8328.payload[32] = value & 0xff;
            hsb->r8328.payload[33] = (value >> 8) & 0xff;
            hsb->r8328.payload[34] = (value >> 16) & 0xff;
            hsb->r8328.payload[35] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR21r, &value);
            hsb->r8328.payload[36] = value & 0xff;
            hsb->r8328.payload[37] = (value >> 8) & 0xff;
            hsb->r8328.payload[38] = (value >> 16) & 0xff;
            hsb->r8328.payload[39] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR22r, &value);
            hsb->r8328.payload[40] = value & 0xff;
            hsb->r8328.payload[41] = (value >> 8) & 0xff;
            hsb->r8328.payload[42] = (value >> 16) & 0xff;
            hsb->r8328.payload[43] = (value >> 24) & 0xff;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR23r, &value);
            hsb->r8328.payload[44] = value & 0xff;
            hsb->r8328.payload[45] = (value >> 8) & 0xff;
            hsb->r8328.payload[46] = (value >> 16) & 0x3;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR23r, ESW_DROPf, &tmpVal, &value);
            hsb->r8328.drop = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR23r, ESW_ASDPMf, &tmpVal, &value);
            hsb->r8328.asdpm = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR23r, ESW_ASDPf, &tmpVal, &value);
            hsb->r8328.asdp = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR23r, ESW_DPf, &tmpVal, &value);
            hsb->r8328.dp = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR23r, ESW_PRIf, &tmpVal, &value);
            hsb->r8328.pri = tmpVal;
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR23r, ESW_ASDPRMKf, &tmpVal, &value);
            hsb->r8328.asdprmk = tmpVal;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR24r, &value);
            //reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR24r, ESW_PMf, &tmpVal, value);
            hsb->r8328.pm = value;

            reg_read(unit, ESW_HEADER_STAMP_BEFORE_CR25r, &value);
            reg_field_get(unit, ESW_HEADER_STAMP_BEFORE_CR25r, ESW_REASONf, &tmpVal, &value);
            hsb->r8328.reason = tmpVal;
        }
#endif
            break;

        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8390)
        {
            /* HSB_DATA0 */
            reg_read(0, CYPRESS_HSB_DATA0r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA0r, CYPRESS_PAGE_CNTf, &tmpVal, &value);
            hsb->r8390.page_cnt = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA0r, CYPRESS_ENDSCf, &tmpVal, &value);
            hsb->r8390.endsc = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA0r, CYPRESS_BGDSCf, &tmpVal, &value);
            hsb->r8390.bgsc = tmpVal;

            /* HSB_DATA1 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA1r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA1r, CYPRESS_FIELD_SELTOR11f, &tmpVal, &value);
            hsb->r8390.field_seltor11 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA1r, CYPRESS_FIELD_SELTOR10f, &tmpVal, &value);
            hsb->r8390.field_seltor10 = tmpVal;

            /* HSB_DATA2 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA2r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA2r, CYPRESS_FIELD_SELTOR9f, &tmpVal, &value);
            hsb->r8390.field_seltor9 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA2r, CYPRESS_FIELD_SELTOR8f, &tmpVal, &value);
            hsb->r8390.field_seltor8 = tmpVal;

            /* HSB_DATA3 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA3r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA3r, CYPRESS_FIELD_SELTOR7f, &tmpVal, &value);
            hsb->r8390.field_seltor7 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA3r, CYPRESS_FIELD_SELTOR6f, &tmpVal, &value);
            hsb->r8390.field_seltor6 = tmpVal;

            /* HSB_DATA4 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA4r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA4r, CYPRESS_FIELD_SELTOR5f, &tmpVal, &value);
            hsb->r8390.field_seltor5 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA4r, CYPRESS_FIELD_SELTOR4f, &tmpVal, &value);
            hsb->r8390.field_seltor4 = tmpVal;

            /* HSB_DATA5 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA5r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA5r, CYPRESS_FIELD_SELTOR3f, &tmpVal, &value);
            hsb->r8390.field_seltor3 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA5r, CYPRESS_FIELD_SELTOR2f, &tmpVal, &value);
            hsb->r8390.field_seltor2 = tmpVal;

            /* HSB_DATA6 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA6r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA6r, CYPRESS_FIELD_SELTOR1f, &tmpVal, &value);
            hsb->r8390.field_seltor1 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA6r, CYPRESS_FIELD_SELTOR0f, &tmpVal, &value);
            hsb->r8390.field_seltor0 = tmpVal;

            /* HSB_DATA7 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA7r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA7r, CYPRESS_FIELD_SEL_VLDf, &tmpVal, &value);
            hsb->r8390.field_sel_vld = tmpVal;

            /* HSB_DATA8 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA8r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA8r, CYPRESS_IMPLS_EXPf, &tmpVal, &value);
            hsb->r8390.impls_exp = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA8r, CYPRESS_IMPLS_LABELf, &tmpVal, &value);
            hsb->r8390.impls_label = tmpVal;

            /* HSB_DATA9 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA9r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA9r, CYPRESS_OMPLS_EXPf, &tmpVal, &value);
            hsb->r8390.ompls_exp = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA9r, CYPRESS_OMPLS_LABELf, &tmpVal, &value);
            hsb->r8390.ompls_label = tmpVal;

            /* HSB_DATA10 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA10r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA10r, CYPRESS_ARPOPCODEf, &tmpVal, &value);
            hsb->r8390.arpopcode = tmpVal;

            /* HSB_DATA11 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA11r, CYPRESS_TGT_MAC_47_32f, &value);
            hsb->r8390.target_mac[0] = (uint8)(value >> 8) & 0xff;
            hsb->r8390.target_mac[1] = (uint8)(value >> 0) & 0xff;

            /* HSB_DATA12 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA12r, CYPRESS_TGT_MAC_31_0f, &value);
            hsb->r8390.target_mac[2] = (uint8)(value >> 24) & 0xff;
            hsb->r8390.target_mac[3] = (uint8)(value >> 16) & 0xff;
            hsb->r8390.target_mac[4] = (uint8)(value >> 8) & 0xff;
            hsb->r8390.target_mac[5] = (uint8)(value >> 0) & 0xff;

            /* HSB_DATA13 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA13r, CYPRESS_SENDER_MAC_47_32f, &value);
            hsb->r8390.sender_mac[0] = (value >> 8) & 0xff;
            hsb->r8390.sender_mac[1] = (value >> 0) & 0xff;

            /* HSB_DATA14 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA14r, CYPRESS_SENDER_MAC_31_0f, &value);
            hsb->r8390.sender_mac[2] = (value >> 24) & 0xff;
            hsb->r8390.sender_mac[3] = (value >> 16) & 0xff;
            hsb->r8390.sender_mac[4] = (value >> 8) & 0xff;
            hsb->r8390.sender_mac[5] = (value >> 0) & 0xff;

            /* HSB_DATA15 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA15r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA15r, CYPRESS_DPORTf, &tmpVal, &value);
            hsb->r8390.dport = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA15r, CYPRESS_SPORTf, &tmpVal, &value);
            hsb->r8390.sport = tmpVal;

            /* HSB_DATA16 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA16r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA16r, CYPRESS_TCP_FLAGf, &tmpVal, &value);
            hsb->r8390.tcp_flag = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA16r, CYPRESS_IP_VERf, &tmpVal, &value);
            hsb->r8390.ipver = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA16r, CYPRESS_IP_TTLf, &tmpVal, &value);
            hsb->r8390.ipttl = tmpVal;

            /* HSB_DATA17 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA17r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA17r, CYPRESS_IP_TOSf, &tmpVal, &value);
            hsb->r8390.ip_tos = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA17r, CYPRESS_IP_PROTOCOLf, &tmpVal, &value);
            hsb->r8390.ip_protocol = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA17r, CYPRESS_IP_FLAGf, &tmpVal, &value);
            hsb->r8390.ip_flag = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA17r, CYPRESS_IP_OFFSETf, &tmpVal, &value);
            hsb->r8390.ip_offset = tmpVal;

            /* HSB_DATA18 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA18r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA18r, CYPRESS_DIPf, &tmpVal, &value);
            hsb->r8390.dip = tmpVal;

            /* HSB_DATA19 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA19r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA19r, CYPRESS_SIPf, &tmpVal, &value);
            hsb->r8390.sip = tmpVal;

            /* HSB_DATA20 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA20r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA20r, CYPRESS_OTAGf, &tmpVal, &value);
            hsb->r8390.otag = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA20r, CYPRESS_ITAGf, &tmpVal, &value);
            hsb->r8390.itag = tmpVal;

            /* HSB_DATA21 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA21r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA21r, CYPRESS_OTPID_IDXf, &tmpVal, &value);
            hsb->r8390.otpid_idx = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA21r, CYPRESS_ITPID_IDXf, &tmpVal, &value);
            hsb->r8390.itpid_idx = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA21r, CYPRESS_ETPf, &tmpVal, &value);
            hsb->r8390.etp = tmpVal;

            /* HSB_DATA22 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA22r, CYPRESS_SMAC_47_32f, &value);
            hsb->r8390.smac[0] = (value >> 8) & 0xff;
            hsb->r8390.smac[1] = (value >> 0) & 0xff;

            /* HSB_DATA23 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA23r, CYPRESS_SMAC_31_0f, &value);
            hsb->r8390.smac[2] = (value >> 24) & 0xff;
            hsb->r8390.smac[3] = (value >> 16) & 0xff;
            hsb->r8390.smac[4] = (value >> 8) & 0xff;
            hsb->r8390.smac[5] = (value >> 0) & 0xff;

            /* HSB_DATA24 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA24r, CYPRESS_DMAC_47_32f, &value);
            hsb->r8390.dmac[0] = (value >> 8) & 0xff;
            hsb->r8390.dmac[1] = (value >> 0) & 0xff;

            /* HSB_DATA25 */
            value = 0;
            reg_field_read(0, CYPRESS_HSB_DATA25r, CYPRESS_DMAC_31_0f, &value);
            hsb->r8390.dmac[2] = (value >> 24) & 0xff;
            hsb->r8390.dmac[3] = (value >> 16) & 0xff;
            hsb->r8390.dmac[4] = (value >> 8) & 0xff;
            hsb->r8390.dmac[5] = (value >> 0) & 0xff;

            /* HSB_DATA26 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA26r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPLENf, &tmpVal, &value);
            hsb->r8390.iplen = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV4HDLENf, &tmpVal, &value);
            hsb->r8390.ipv4hdlen = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_TCPSEQZEROf, &tmpVal, &value);
            hsb->r8390.tcpseqzero = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV6HOPf, &tmpVal, &value);
            hsb->r8390.ipv6hop = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV6ROUTf, &tmpVal, &value);
            hsb->r8390.ipv6rout = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV6FRAGf, &tmpVal, &value);
            hsb->r8390.ipv6frag = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV6DESTf, &tmpVal, &value);
            hsb->r8390.ipv6dest = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV6AUTHf, &tmpVal, &value);
            hsb->r8390.ipv6auth = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPCSOKf, &tmpVal, &value);
            hsb->r8390.ipcsok = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV6EXT_LONGf, &tmpVal, &value);
            hsb->r8390.ipv6ext_long = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV6f, &tmpVal, &value);
            hsb->r8390.ipv6 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA26r, CYPRESS_IPV4f, &tmpVal, &value);
            hsb->r8390.ipv4 = tmpVal;

            /* HSB_DATA27 */
            value = 0;
            reg_read(0, CYPRESS_HSB_DATA27r, &value);
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_OMPLS_IFf, &tmpVal, &value);
            hsb->r8390.ompls_if = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_IMPLS_IFf, &tmpVal, &value);
            hsb->r8390.impls_if = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_ETAG_IFf, &tmpVal, &value);
            hsb->r8390.etag_if = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_CPUTAG_IFf, &tmpVal, &value);
            hsb->r8390.cputag_if = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_OTAG_IFf, &tmpVal, &value);
            hsb->r8390.otag_if = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_ITAG_IFf, &tmpVal, &value);
            hsb->r8390.itag_if = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_OAMPDUf, &tmpVal, &value);
            hsb->r8390.oampdu = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_LLC_OTHERf, &tmpVal, &value);
            hsb->r8390.llc_other = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_PPPOE_IFf, &tmpVal, &value);
            hsb->r8390.pppoe_if = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_RFC_1042f, &tmpVal, &value);
            hsb->r8390.rfc1042 = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_SPNf, &tmpVal, &value);
            hsb->r8390.spa = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_ERRPKTf, &tmpVal, &value);
            hsb->r8390.errpkt = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_L4HDCHKf, &tmpVal, &value);
            hsb->r8390.l4hdchk = tmpVal;
            reg_field_get(0, CYPRESS_HSB_DATA27r, CYPRESS_PKTLENf, &tmpVal, &value);
            hsb->r8390.pktlen = tmpVal;
        }
#endif
            break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8380)
        {
            /* HSB_DATA0 */
            reg_read(0, MAPLE_HSB_DATA0r, &value);
            reg_field_get(0, MAPLE_HSB_DATA0r, MAPLE_IP_FRG_OFFSETf, &tmpVal, &value);
            hsb->r8380.ip_offset = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA0r, MAPLE_IP_PROTOf, &tmpVal, &value);
            hsb->r8380.ip_protocol = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA0r, MAPLE_IP_TOSf, &tmpVal, &value);
            hsb->r8380.ip_dstype = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA0r, MAPLE_IP_MORE_FLAGf, &tmpVal, &value);
            hsb->r8380.ip_mf = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA0r, MAPLE_RTK_PPf, &tmpVal, &value);
            hsb->r8380.rtkpp = tmpVal;

            /* HSB_DATA1 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA1r, &value);
            reg_field_get(0, MAPLE_HSB_DATA1r, MAPLE_CPU_TAG_EXISTf, &tmpVal, &value);
            hsb->r8380.ctag_exist = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA1r, MAPLE_ETAG_EXISTf, &tmpVal, &value);
            hsb->r8380.etag_exist = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA1r, MAPLE_RTAG_EXISTf, &tmpVal, &value);
            hsb->r8380.rtag_exist = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA1r, MAPLE_OTAG_EXISTf, &tmpVal, &value);
            hsb->r8380.otag_exist = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA1r, MAPLE_ITAG_EXISTf, &tmpVal, &value);
            hsb->r8380.itag_exist = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA1r, MAPLE_IP_TTLf, &tmpVal, &value);
            hsb->r8380.ip_ttl = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA1r, MAPLE_IP_LENGTHf, &tmpVal, &value);
            hsb->r8380.ip_length = tmpVal;

            /* HSB_DATA2 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA2r, &value);
            reg_field_get(0, MAPLE_HSB_DATA2r, MAPLE_SIP_31_0f, &tmpVal, &value);
            hsb->r8380.sip[15] = tmpVal & 0xff;
            hsb->r8380.sip[14] = (tmpVal >> 8) & 0xff;
            hsb->r8380.sip[13] = (tmpVal >> 16) & 0xff;
            hsb->r8380.sip[12] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA3 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA3r, &value);
            reg_field_get(0, MAPLE_HSB_DATA3r, MAPLE_SIP_63_32f, &tmpVal, &value);
            hsb->r8380.sip[11] = tmpVal & 0xff;
            hsb->r8380.sip[10] = (tmpVal >> 8) & 0xff;
            hsb->r8380.sip[9] = (tmpVal >> 16) & 0xff;
            hsb->r8380.sip[8] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA4 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA4r, &value);
            reg_field_get(0, MAPLE_HSB_DATA4r, MAPLE_SIP_95_64f, &tmpVal, &value);
            hsb->r8380.sip[7] = tmpVal & 0xff;
            hsb->r8380.sip[6] = (tmpVal >> 8) & 0xff;
            hsb->r8380.sip[5] = (tmpVal >> 16) & 0xff;
            hsb->r8380.sip[4] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA5 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA5r, &value);
            reg_field_get(0, MAPLE_HSB_DATA5r, MAPLE_SIP_127_96f, &tmpVal, &value);
            hsb->r8380.sip[3] = tmpVal & 0xff;
            hsb->r8380.sip[2] = (tmpVal >> 8) & 0xff;
            hsb->r8380.sip[1] = (tmpVal >> 16) & 0xff;
            hsb->r8380.sip[0] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA6 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA6r, &value);
            reg_field_get(0, MAPLE_HSB_DATA6r, MAPLE_DIP_31_0f, &tmpVal, &value);
            hsb->r8380.dip[15] = tmpVal & 0xff;
            hsb->r8380.dip[14] = (tmpVal >> 8) & 0xff;
            hsb->r8380.dip[13] = (tmpVal >> 16) & 0xff;
            hsb->r8380.dip[12] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA7 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA7r, &value);
            reg_field_get(0, MAPLE_HSB_DATA7r, MAPLE_DIP_63_32f, &tmpVal, &value);
            hsb->r8380.dip[11] = tmpVal & 0xff;
            hsb->r8380.dip[10] = (tmpVal >> 8) & 0xff;
            hsb->r8380.dip[9] = (tmpVal >> 16) & 0xff;
            hsb->r8380.dip[8] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA8 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA8r, &value);
            reg_field_get(0, MAPLE_HSB_DATA8r, MAPLE_DIP_95_64f, &tmpVal, &value);
            hsb->r8380.dip[7] = tmpVal & 0xff;
            hsb->r8380.dip[6] = (tmpVal >> 8) & 0xff;
            hsb->r8380.dip[5] = (tmpVal >> 16) & 0xff;
            hsb->r8380.dip[4] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA9 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA9r, &value);
            reg_field_get(0, MAPLE_HSB_DATA9r, MAPLE_DIP_127_96f, &tmpVal, &value);
            hsb->r8380.dip[3] = tmpVal & 0xff;
            hsb->r8380.dip[2] = (tmpVal >> 8) & 0xff;
            hsb->r8380.dip[1] = (tmpVal >> 16) & 0xff;
            hsb->r8380.dip[0] = (tmpVal >> 24) & 0xff;

            /* HSB_DATA10 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA10r, &value);
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_FRAME_TYPEf, &tmpVal, &value);
            hsb->r8380.frame_type = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_IPV4_PKTf, &tmpVal, &value);
            hsb->r8380.ipv4_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_IPV6_PKTf, &tmpVal, &value);
            hsb->r8380.ipv6_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_ARP_PKTf, &tmpVal, &value);
            hsb->r8380.arp_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_PPPOE_PKTf, &tmpVal, &value);
            hsb->r8380.pppoe_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_TCP_PKTf, &tmpVal, &value);
            hsb->r8380.tcp_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_UDP_PKTf, &tmpVal, &value);
            hsb->r8380.udp_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_IGMP_PKTf, &tmpVal, &value);
            hsb->r8380.igmp_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_ICMP_PKTf, &tmpVal, &value);
            hsb->r8380.icmp_pkt = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_RSPAN_RMf, &tmpVal, &value);
            hsb->r8380.rx_rm_rtag = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_TCP_SN_EQ_0f, &tmpVal, &value);
            hsb->r8380.tcp_sn_eq_0 = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_TCP_FLAGf, &tmpVal, &value);
            hsb->r8380.tcp_flag = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_TCP_HDR_LENf, &tmpVal, &value);
            hsb->r8380.tcp_hdrlen = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_IPV4_DFf, &tmpVal, &value);
            hsb->r8380.ipv4_df = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA10r, MAPLE_IPV4_HDR_LENf, &tmpVal, &value);
            hsb->r8380.ipv4_hdrlen = tmpVal;

            /* HSB_DATA11 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA11r, &value);
            reg_field_get(0, MAPLE_HSB_DATA11r, MAPLE_L4_CONTENTf, &tmpVal, &value);
            hsb->r8380.l4_content = tmpVal;

            /* HSB_DATA12 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA12r, &value);
            reg_field_get(0, MAPLE_HSB_DATA12r, MAPLE_OTAG_CONTENTf, &tmpVal, &value);
            hsb->r8380.otag_content = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA12r, MAPLE_ITAG_CONTENTf, &tmpVal, &value);
            hsb->r8380.itag_content = tmpVal;

            /* HSB_DATA13 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA13r, &value);
            reg_field_get(0, MAPLE_HSB_DATA13r, MAPLE_CPU_TAG_CONTENT_31_0f, &tmpVal, &value);
            hsb->r8380.ctag_dpm = tmpVal & 0x1fffffff;
            hsb->r8380.ctag_as_dpm = (tmpVal >> 30) & 0x1;
            hsb->r8380.ctag_physical_dpm = (tmpVal >> 29) & 0x1;

            /* HSB_DATA14 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA14r, &value);
            reg_field_get(0, MAPLE_HSB_DATA14r, MAPLE_FS_VALIDf, &tmpVal, &value);
            hsb->r8380.fs3_valid = (tmpVal & 0x8) >> 3;
            hsb->r8380.fs2_valid = (tmpVal & 0x4) >> 2;
            hsb->r8380.fs1_valid = (tmpVal & 0x2) >> 1;
            hsb->r8380.fs0_valid = tmpVal & 0x1;
            reg_field_get(0, MAPLE_HSB_DATA14r, MAPLE_TYPELENf, &tmpVal, &value);
            hsb->r8380.typelen = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA14r, MAPLE_CRC_EQf, &tmpVal, &value);
            hsb->r8380.crceq = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA14r, MAPLE_CPU_TAG_CONTENT_42_32f, &tmpVal, &value);
            hsb->r8380.ctag_bp_fltr_1 = (tmpVal >> 10) & 0x1;
            hsb->r8380.ctag_bp_fltr_2 = (tmpVal >> 9) & 0x1;
            hsb->r8380.ctag_as_tagsts = (tmpVal >> 8) & 0x1;
            hsb->r8380.ctag_acl_act = (tmpVal >> 7) & 0x1;
            hsb->r8380.ctag_rvid_sel = (tmpVal >> 6) & 0x1;
            hsb->r8380.ctag_l2learning = (tmpVal >> 5) & 0x1;
            hsb->r8380.ctag_as_pri = (tmpVal >> 4) & 0x1;
            hsb->r8380.ctag_pri = (tmpVal >> 1) & 0x7;

            /* HSB_DATA15 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA15r, &value);
            reg_field_get(0, MAPLE_HSB_DATA15r, MAPLE_FS1_DATAf, &tmpVal, &value);
            hsb->r8380.fs1_data = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA15r, MAPLE_FS0_DATAf, &tmpVal, &value);
            hsb->r8380.fs0_data = tmpVal;

            /* HSB_DATA16 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA16r, &value);
            reg_field_get(0, MAPLE_HSB_DATA16r, MAPLE_FS3_DATAf, &tmpVal, &value);
            hsb->r8380.fs3_data = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA16r, MAPLE_FS2_DATAf, &tmpVal, &value);
            hsb->r8380.fs2_data = tmpVal;

            /* HSB_DATA17 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA17r, &value);
            reg_field_get(0, MAPLE_HSB_DATA17r, MAPLE_SA_31_0f, &tmpVal, &value);
            hsb->r8380.smac[2] = (tmpVal >> 24) & 0xff;
            hsb->r8380.smac[3] = (tmpVal >> 16) & 0xff;
            hsb->r8380.smac[4] = (tmpVal >> 8) & 0xff;
            hsb->r8380.smac[5] = tmpVal & 0xff;

            /* HSB_DATA18 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA18r, &value);
            reg_field_get(0, MAPLE_HSB_DATA18r, MAPLE_PKT_LENf, &tmpVal, &value);
            hsb->r8380.pktlen = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA18r, MAPLE_SA_47_32f, &tmpVal, &value);
            hsb->r8380.smac[0] = (tmpVal >> 8) & 0xff;
            hsb->r8380.smac[1] = tmpVal & 0xff;
            reg_field_get(0, MAPLE_HSB_DATA18r, MAPLE_IPV6_MOB_EXT_HDRf, &tmpVal, &value);
            hsb->r8380.ipv6_mob_ext_hdr = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA18r, MAPLE_IPV6_HBH_EXT_HDR_ERRf, &tmpVal, &value);
            hsb->r8380.ipv6_hbh_ext_hdr_err = tmpVal;

            /* HSB_DATA19 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA19r, &value);
            reg_field_get(0, MAPLE_HSB_DATA19r, MAPLE_DA_31_0f, &tmpVal, &value);
            hsb->r8380.dmac[2] = (tmpVal >> 24) & 0xff;
            hsb->r8380.dmac[3] = (tmpVal >> 16) & 0xff;
            hsb->r8380.dmac[4] = (tmpVal >> 8) & 0xff;
            hsb->r8380.dmac[5] = tmpVal & 0xff;

            /* HSB_DATA20 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA20r, &value);
            reg_field_get(0, MAPLE_HSB_DATA20r, MAPLE_DA_47_32f, &tmpVal, &value);
            hsb->r8380.dmac[0] = (tmpVal >> 8) & 0xff;
            hsb->r8380.dmac[1] = tmpVal & 0xff;

            /* HSB_DATA21 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA21r, &value);
            reg_field_get(0, MAPLE_HSB_DATA21r, MAPLE_IPV6_EXT_HDR_LENf, &tmpVal, &value);
            hsb->r8380.ipv6_extension_hdrlen = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA21r, MAPLE_OTAG_IDXf, &tmpVal, &value);
            hsb->r8380.otag_index = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA21r, MAPLE_ITAG_IDXf, &tmpVal, &value);
            hsb->r8380.itag_index = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA21r, MAPLE_RX_DROPf, &tmpVal, &value);
            hsb->r8380.rxdrop = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA21r, MAPLE_OAM_PKTf, &tmpVal, &value);
            hsb->r8380.oampdu = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA21r, MAPLE_SRC_PHYf, &tmpVal, &value);
            hsb->r8380.sphy = tmpVal;

            /* HSB_DATA22 */
            value = 0;
            reg_read(0, MAPLE_HSB_DATA22r, &value);
            reg_field_get(0, MAPLE_HSB_DATA22r, MAPLE_HSB_RSVDf, &tmpVal, &value);

            reg_field_get(0, MAPLE_HSB_DATA22r, MAPLE_IPV6_FLOW_LABELf, &tmpVal, &value);
            hsb->r8380.ipv6_flow_label = tmpVal;
            reg_field_get(0, MAPLE_HSB_DATA22r, MAPLE_IPV6_UNKNOWN_HDRf, &tmpVal, &value);
            hsb->r8380.ipv6_unknown_hdr = tmpVal;
        }
#endif
            break;

        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

void _dumpHsbPayloadInfo(uint32 unit, hsb_param_t * hsb)
{
#if defined(CONFIG_SDK_RTL8328)
    osal_printf("=================================Payload Info==================================\n");
    if (hsb->r8328.pppoe)
    {
        switch (hsb->r8328.l34fmt)
        {
        case LAYER34_UKONWN:
            break;
        case IPPRO_ARP:
            break;
        case IPV4_ICMP:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[5]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[4]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);

            osal_printf("icmp_type:0x%x\t", hsb->r8328.payload[19]);
            osal_printf("icmp_code:0x%x\n", hsb->r8328.payload[18]);
            break;
        case IPV4_IGMP:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[5]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[4]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);
            osal_printf("igmp_gip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16]);
            osal_printf("igmp_type:0x%x\n", hsb->r8328.payload[7]);
            break;
        case IPV4_TCP:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[5]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[4]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);

            osal_printf("tcp_sport:0x%x\t", (hsb->r8328.payload[19] << 8) | (hsb->r8328.payload[18]));
            osal_printf("tcp_dport:0x%x\t", (hsb->r8328.payload[17] << 8) | (hsb->r8328.payload[16]));

            osal_printf("tcp_flag:0x%x\n", hsb->r8328.payload[20]);
            break;
        case IPV4_UDP:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[5]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[4]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);

            osal_printf("udp_sport:0x%x\t", (hsb->r8328.payload[19] << 8) | (hsb->r8328.payload[18]));
            osal_printf("udp_dport:0x%x\n", (hsb->r8328.payload[17] << 8) | (hsb->r8328.payload[16]));
            break;
        case IPV4_UNKNOWN:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[5]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[4]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);
            break;
        case IPV6_ICMPV6:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[4]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[5] | (hsb->r8328.payload[6] << 8) | (hsb->r8328.payload[7] << 16)) >> 4);

            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ipv6_hopLimit:0x%x\n", hsb->r8328.payload[8]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[15], hsb->r8328.payload[14],
                           hsb->r8328.payload[13], hsb->r8328.payload[12], hsb->r8328.payload[19], hsb->r8328.payload[18],
                           hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20], hsb->r8328.payload[25], hsb->r8328.payload[24]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[31], hsb->r8328.payload[30],
                           hsb->r8328.payload[29], hsb->r8328.payload[28], hsb->r8328.payload[35], hsb->r8328.payload[34],
                           hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36], hsb->r8328.payload[41], hsb->r8328.payload[40]);
            osal_printf("icmpv6_code:0x%x\t", hsb->r8328.payload[43]);
            osal_printf("icmpv6_type:0x%x\n", hsb->r8328.payload[42]);
            break;
        case IPV6_TCP:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[4]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[5] | (hsb->r8328.payload[6] << 8) | (hsb->r8328.payload[7] << 16)) >> 4);

            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ipv6_hopLimit:0x%x\n", hsb->r8328.payload[8]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[15], hsb->r8328.payload[14],
                           hsb->r8328.payload[13], hsb->r8328.payload[12], hsb->r8328.payload[19], hsb->r8328.payload[18],
                           hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20], hsb->r8328.payload[25], hsb->r8328.payload[24]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[31], hsb->r8328.payload[30],
                           hsb->r8328.payload[29], hsb->r8328.payload[28], hsb->r8328.payload[35], hsb->r8328.payload[34],
                           hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36], hsb->r8328.payload[41], hsb->r8328.payload[40]);

            osal_printf("tcp_sport:0x%x\t", (hsb->r8328.payload[43] << 8) | (hsb->r8328.payload[42]));
            osal_printf("tcp_dport:0x%x\t", (hsb->r8328.payload[45] << 8) | (hsb->r8328.payload[44]));
            osal_printf("tcp_flag:0x%x\n", (hsb->r8328.payload[46] << 4) | (hsb->r8328.payload[5]));
            break;
        case IPV6_UDP:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[4]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[5] | (hsb->r8328.payload[6] << 8) | (hsb->r8328.payload[7] << 16)) >> 4);

            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ipv6_hopLimit:0x%x\n", hsb->r8328.payload[8]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[15], hsb->r8328.payload[14],
                           hsb->r8328.payload[13], hsb->r8328.payload[12], hsb->r8328.payload[19], hsb->r8328.payload[18],
                           hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20], hsb->r8328.payload[25], hsb->r8328.payload[24]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[31], hsb->r8328.payload[30],
                           hsb->r8328.payload[29], hsb->r8328.payload[28], hsb->r8328.payload[35], hsb->r8328.payload[34],
                           hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36], hsb->r8328.payload[41], hsb->r8328.payload[40]);

            osal_printf("tcp_sport:0x%x\t", (hsb->r8328.payload[43] << 8) | (hsb->r8328.payload[42]));
            osal_printf("tcp_dport:0x%x\n", (hsb->r8328.payload[45] << 8) | (hsb->r8328.payload[44]));
            break;
        case IPV6_UNKNOWN:
            osal_printf("ppp_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("ppp_sessionID:0x%x\t", (hsb->r8328.payload[1] << 8) | (hsb->r8328.payload[0]));
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[4]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[5] | (hsb->r8328.payload[6] << 8) | (hsb->r8328.payload[7] << 16)) >> 4);

            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ipv6_hopLimit:0x%x\n", hsb->r8328.payload[8]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[15], hsb->r8328.payload[14],
                           hsb->r8328.payload[13], hsb->r8328.payload[12], hsb->r8328.payload[19], hsb->r8328.payload[18],
                           hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20], hsb->r8328.payload[25], hsb->r8328.payload[24]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[31], hsb->r8328.payload[30],
                           hsb->r8328.payload[29], hsb->r8328.payload[28], hsb->r8328.payload[35], hsb->r8328.payload[34],
                           hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36], hsb->r8328.payload[41], hsb->r8328.payload[40]);
            break;
        default:
            break;
        }
    }
    else
    {
        switch (hsb->r8328.l34fmt)
        {
        case LAYER34_UKONWN:
            break;
        case IPPRO_ARP:
            osal_printf("arp_op:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("arp_sendMac:0x%x:%x:%x:%x:%x:%x\t", hsb->r8328.payload[1], hsb->r8328.payload[0], hsb->r8328.payload[7], hsb->r8328.payload[6], hsb->r8328.payload[5], hsb->r8328.payload[4]);
            osal_printf("arp_sip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8]);
            osal_printf("arp_targetMac:0x%x:%x:%x:%x:%x:%x\t", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12], hsb->r8328.payload[19], hsb->r8328.payload[18]);
            osal_printf("arp_tip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20]);
            break;
        case IPV4_ICMP:
            osal_printf("ip_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("iph_len:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[8]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[1]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t\t", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16]);

            osal_printf("ip_fragOff:0x%x\t", (hsb->r8328.payload[7] << 8) | (hsb->r8328.payload[6]));
            osal_printf("ip_id:0x%x\t", (hsb->r8328.payload[5] << 8) | (hsb->r8328.payload[4]));
            osal_printf("icmp_type:0x%x\t", hsb->r8328.payload[23]);
            osal_printf("icmp_code:0x%x\n", hsb->r8328.payload[22]);
            break;
        case IPV4_IGMP:
            osal_printf("ip_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("iph_len:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[8]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[1]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\t", hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16]);
            osal_printf("igmp_gip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20]);

            osal_printf("ip_fragOff:0x%x\t", (hsb->r8328.payload[7] << 8) | (hsb->r8328.payload[6]));
            osal_printf("ip_id:0x%x\t", (hsb->r8328.payload[5] << 8) | (hsb->r8328.payload[4]));
            osal_printf("igmp_type:0x%x\n", hsb->r8328.payload[10]);
            break;
        case IPV4_TCP:
            osal_printf("ip_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("iph_len:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[8]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[1]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t\t", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16]);

            osal_printf("ip_fragOff:0x%x\t", (hsb->r8328.payload[7] << 8) | (hsb->r8328.payload[6]));
            osal_printf("ip_id:0x%x\t", (hsb->r8328.payload[5] << 8) | (hsb->r8328.payload[4]));
            osal_printf("tcp_sport:0x%x\t", (hsb->r8328.payload[23] << 8) | (hsb->r8328.payload[22]));
            osal_printf("tcp_dport:0x%x\t", (hsb->r8328.payload[21] << 8) | (hsb->r8328.payload[20]));
            osal_printf("tcp_flag:0x%x\n", hsb->r8328.payload[24]);
            break;
        case IPV4_UDP:
            osal_printf("ip_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("iph_len:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[8]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[1]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t\t", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16]);

            osal_printf("ip_fragOff:0x%x\t", (hsb->r8328.payload[7] << 8) | (hsb->r8328.payload[6]));
            osal_printf("ip_id:0x%x\t", (hsb->r8328.payload[5] << 8) | (hsb->r8328.payload[4]));
            osal_printf("udp_sport:0x%x\t", (hsb->r8328.payload[23] << 8) | (hsb->r8328.payload[22]));
            osal_printf("udp_dport:0x%x\n", (hsb->r8328.payload[21] << 8) | (hsb->r8328.payload[20]));
            break;
        case IPV4_UNKNOWN:
            osal_printf("ip_len:0x%x\t", (hsb->r8328.payload[3] << 8) | (hsb->r8328.payload[2]));
            osal_printf("iph_len:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ip_pro:0x%x\t", hsb->r8328.payload[9]);
            osal_printf("ip_ttl:0x%x\t", hsb->r8328.payload[8]);
            osal_printf("ip_tos:0x%x\n", hsb->r8328.payload[1]);

            osal_printf("ipv4_sip:0x%2x.%2x.%2x.%2x\t\t", hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12]);
            osal_printf("ipv4_dip:0x%2x.%2x.%2x.%2x\n", hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16]);

            osal_printf("ip_fragOff:0x%x\t", (hsb->r8328.payload[7] << 8) | (hsb->r8328.payload[6]));
            osal_printf("ip_id:0x%x\n", (hsb->r8328.payload[5] << 8) | (hsb->r8328.payload[4]));
            break;
        case IPV6_ICMPV6:
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[1] | (hsb->r8328.payload[2] << 8) | (hsb->r8328.payload[3] << 16)) >> 4);
            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[7]);
            osal_printf("ipv6_hopLimit:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ipv6_payload_len:0x%x\n", (hsb->r8328.payload[5] << 8) | hsb->r8328.payload[4]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8],
                           hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12],
                           hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[25], hsb->r8328.payload[24],
                           hsb->r8328.payload[31], hsb->r8328.payload[30], hsb->r8328.payload[29], hsb->r8328.payload[28],
                           hsb->r8328.payload[35], hsb->r8328.payload[34], hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36]);

            osal_printf("icmpv6_code:0x%x\t", hsb->r8328.payload[41]);
            osal_printf("icmpv6_type:0x%x\n", hsb->r8328.payload[42]);

            break;
        case IPV6_TCP:
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[1] | (hsb->r8328.payload[2] << 8) | (hsb->r8328.payload[3] << 16)) >> 4);
            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[7]);
            osal_printf("ipv6_hopLimit:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ipv6_payload_len:0x%x\n", (hsb->r8328.payload[5] << 8) | hsb->r8328.payload[4]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8],
                           hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12],
                           hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[25], hsb->r8328.payload[24],
                           hsb->r8328.payload[31], hsb->r8328.payload[30], hsb->r8328.payload[29], hsb->r8328.payload[28],
                           hsb->r8328.payload[35], hsb->r8328.payload[34], hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36]);

            osal_printf("tcp_sport:0x%x\t", (hsb->r8328.payload[41] << 8) | hsb->r8328.payload[40]);
            osal_printf("tcp_dport:0x%x\t", (hsb->r8328.payload[43] << 8) | hsb->r8328.payload[42]);
            osal_printf("tcp_flag:0x%x\n", hsb->r8328.payload[1] | (hsb->r8328.payload[44] << 4));
            break;
        case IPV6_UDP:
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[1] | (hsb->r8328.payload[2] << 8) | (hsb->r8328.payload[3] << 16)) >> 4);
            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[7]);
            osal_printf("ipv6_hopLimit:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ipv6_payload_len:0x%x\n", (hsb->r8328.payload[5] << 8) | hsb->r8328.payload[4]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8],
                           hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12],
                           hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[25], hsb->r8328.payload[24],
                           hsb->r8328.payload[31], hsb->r8328.payload[30], hsb->r8328.payload[29], hsb->r8328.payload[28],
                           hsb->r8328.payload[35], hsb->r8328.payload[34], hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36]);

            osal_printf("udp_sport:0x%x\t", (hsb->r8328.payload[41] << 8) | hsb->r8328.payload[40]);
            osal_printf("udp_dport:0x%x\n", (hsb->r8328.payload[43] << 8) | hsb->r8328.payload[42]);
            break;
        case IPV6_UNKNOWN:
            osal_printf("ipv6_dscp:0x%x\t", hsb->r8328.payload[0]);
            osal_printf("ipv6_flabel:0x%x\t", (hsb->r8328.payload[1] | (hsb->r8328.payload[2] << 8) | (hsb->r8328.payload[3] << 16)) >> 4);
            osal_printf("ipv6_pro:0x%x\t", hsb->r8328.payload[7]);
            osal_printf("ipv6_hopLimit:0x%x\t", hsb->r8328.payload[6]);
            osal_printf("ipv6_payload_len:0x%x\n", (hsb->r8328.payload[5] << 8) | hsb->r8328.payload[4]);

            osal_printf("ipv6_sip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[11], hsb->r8328.payload[10], hsb->r8328.payload[9], hsb->r8328.payload[8],
                           hsb->r8328.payload[15], hsb->r8328.payload[14], hsb->r8328.payload[13], hsb->r8328.payload[12],
                           hsb->r8328.payload[19], hsb->r8328.payload[18], hsb->r8328.payload[17], hsb->r8328.payload[16], hsb->r8328.payload[23], hsb->r8328.payload[22], hsb->r8328.payload[21], hsb->r8328.payload[20]);

            osal_printf("ipv6_dip:0x%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x:%x\n",
                           hsb->r8328.payload[27], hsb->r8328.payload[26], hsb->r8328.payload[25], hsb->r8328.payload[24],
                           hsb->r8328.payload[31], hsb->r8328.payload[30], hsb->r8328.payload[29], hsb->r8328.payload[28],
                           hsb->r8328.payload[35], hsb->r8328.payload[34], hsb->r8328.payload[33], hsb->r8328.payload[32], hsb->r8328.payload[39], hsb->r8328.payload[38], hsb->r8328.payload[37], hsb->r8328.payload[36]);
            break;
        default:
            break;
        }
    }
#endif
}

/* Function Name:
 *      hal_dumpHsb
 * Description:
 *      Dump hsb paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_dumpHsb(uint32 unit)
{
    uint32  index;
    int32   i;
    hsb_param_t  hsb;
    hal_control_t *pInfo;

    if(RT_ERR_OK != _rtk_getHsb(unit, &hsb))
    {
        return RT_ERR_FAILED;
    }

    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8389M_CHIP_ID:
        case RTL8389L_CHIP_ID:
        case RTL8329M_CHIP_ID:
        case RTL8377M_CHIP_ID:
        {
            osal_printf("\n====================================HSB========================================\n");
            osal_printf("sel_hsb:0x%x\t", hsb.r8389.sel_hsb);
            osal_printf("valid_hsb:0x%x\t", hsb.r8389.valid_hsb);
            osal_printf("cfi:0x%x\t\t", hsb.r8389.cfi);
            osal_printf("patternmatch:0x%x\n", hsb.r8389.patternmatch);
            osal_printf("flowlabel:0x%x\t", hsb.r8389.flowlabel);
            osal_printf("dstport:0x%x\t", hsb.r8389.dstport);
            osal_printf("srcport:0x%x\t", hsb.r8389.srcport);
            osal_printf("tcpflags:0x%x\n", hsb.r8389.tcpflags);
            osal_printf("ipproto:0x%x\t", hsb.r8389.ipproto);
            osal_printf("svid:0x%x\t", hsb.r8389.svid);
            osal_printf("spri:0x%x\t", hsb.r8389.spri);
            osal_printf("rxdrop:0x%x\n", hsb.r8389.rxdrop);
            osal_printf("cputagif:0x%x\t", hsb.r8389.cputagif);
            osal_printf("cpuintpri:0x%x\t", hsb.r8389.cpuintpri);
            osal_printf("cpuportmask:0x%x\t", hsb.r8389.cpuportmask);
            osal_printf("ethtype:0x%x\n", hsb.r8389.ethtype);
            osal_printf("ipv6mld:0x%x\t", hsb.r8389.ipv6mld);
            osal_printf("cpri:0x%x\t", hsb.r8389.cpri);
            osal_printf("cvid:0x%x\t", hsb.r8389.cvid);
            osal_printf("dip:0x%x\n", hsb.r8389.dip);
            osal_printf("sip:0x%x\t", hsb.r8389.sip);
            osal_printf("tos:0x%x\t\t", hsb.r8389.tos);
            osal_printf("cpuasdp:0x%x\t", hsb.r8389.cpuasdp);
            osal_printf("cpuasdpm:0x%x\n", hsb.r8389.cpuasdpm);
            osal_printf("cpuasdprmk:0x%x\t", hsb.r8389.cpuasdprmk);
            osal_printf("ipv6:0x%x\t", hsb.r8389.ipv6);
            osal_printf("ipv4:0x%x\t", hsb.r8389.ipv4);
            osal_printf("pppoe:0x%x\n", hsb.r8389.pppoe);
            osal_printf("stagif:0x%x\t", hsb.r8389.stagif);
            osal_printf("ctagif:0x%x\t", hsb.r8389.ctagif);
            osal_printf("frametype:0x%x\t", hsb.r8389.frametype);
            osal_printf("pktlen:0x%x\n", hsb.r8389.pktlen);
            osal_printf("l4csok:0x%x\t", hsb.r8389.l4csok);
            osal_printf("l3csok:0x%x\t", hsb.r8389.l3csok);
            osal_printf("endpage:0x%x\t", hsb.r8389.endpage);
            osal_printf("startpage:0x%x\n", hsb.r8389.startpage);
            osal_printf("startbank:0x%x\t", hsb.r8389.startbank);
            osal_printf("spa:0x%x\n", hsb.r8389.spa);

            osal_printf("DMAC: ");
            for (i = 0; i < 6; i++)
                if (i == 5)
                    osal_printf("%02x  ", hsb.r8389.dmac[i]);
                else
                    osal_printf("%02x:", hsb.r8389.dmac[i]);

            osal_printf("\tSMAC: ");
            for (i = 0; i < 6; i++)
                if (i == 5)
                    osal_printf("%02x\n", hsb.r8389.smac[i]);
                else
                    osal_printf("%02x:", hsb.r8389.smac[i]);
            osal_printf("===============================================================================\n");
        }
            break;

        case RTL8328M_CHIP_ID:
        case RTL8328S_CHIP_ID:
        case RTL8328L_CHIP_ID:
        {
            osal_printf("\n====================================HSB========================================\n");
            osal_printf("asdp:0x%01x        asdpm:0x%01x       asdprmk:0x%01x    cfi:0x%01x         cputag:0x%01x\n", hsb.r8328.asdp, hsb.r8328.asdpm, hsb.r8328.asdprmk, hsb.r8328.cfi, hsb.r8328.cputag);
            osal_printf("datype:0x%01x      dei:0x%01x         dfPri:0x%01x      dp:0x%01x          dpm:0x%08x\n", hsb.r8328.datype, hsb.r8328.dei, hsb.r8328.dfPri, hsb.r8328.dp, hsb.r8328.dpm);
            osal_printf("drop:0x%01x        extra-tag:0x%01x   flag:0x%04x    framelen:0x%04x insertCpuTag:0x%01x\n", hsb.r8328.drop, hsb.r8328.extraTag, hsb.r8328.flag, hsb.r8328.frameLen, hsb.r8328.insertCpuTag);
            osal_printf("ipfo0_n:0x%01x     ipri:0x%01x        ipv4oph:0x%01x    ipv6oph:0x%01x     itagExist:0x%01x\n", hsb.r8328.ipfo0_n, hsb.r8328.ipri, hsb.r8328.ipv4oph, hsb.r8328.ipv6oph, hsb.r8328.itagExist);
            osal_printf("ivid:%-4d       l2calc:0x%01x      l3calc:0x%01x     l4calc:0x%01x      l2err:0x%01x\n", hsb.r8328.ivid, hsb.r8328.l2calc, hsb.r8328.l3calc, hsb.r8328.l4calc, hsb.r8328.l2err);
            osal_printf("l3err:0x%01x       l4err:0x%01x       l2fmt:0x%01x      l34fmt:0x%01x      l4off:0x%02x\n", hsb.r8328.l3err, hsb.r8328.l4err, hsb.r8328.l2fmt, hsb.r8328.l34fmt, hsb.r8328.l4off);
            osal_printf("lb:0x%01x          oam_lbda:0x%01x    oam_lbsa:0x%01x   oampdu:0x%01x      oamTf:0x%01x\n", hsb.r8328.lb, hsb.r8328.oam_lbda, hsb.r8328.oam_lbsa, hsb.r8328.oampdu, hsb.r8328.oamTf);
            osal_printf("opri:0x%01x        otagExist:0x%01x   ovid:%-4d      PM:0x%08x   pppoe:0x%01x\n", hsb.r8328.opri, hsb.r8328.otagExist, hsb.r8328.ovid, hsb.r8328.pm, hsb.r8328.pppoe);
            osal_printf("pri:0x%01x         reason:0x%01x      rtkpro:0x%01x     rxrspan:0x%01x     sphy:%2d\n", hsb.r8328.pri, hsb.r8328.reason, hsb.r8328.rtkPro, hsb.r8328.rxrspan, hsb.r8328.sphy);
            osal_printf("trapToCpu:0x%01x   typelen:0x%04x\n", hsb.r8328.trapToCpu, hsb.r8328.typeLen);

            osal_printf("DMAC: ");
            for (i = 0; i < 6; i++)
                if (i == 5)
                    osal_printf("%02x  ", hsb.r8328.dmac[i]);
                else
                    osal_printf("%02x:", hsb.r8328.dmac[i]);

            osal_printf("SMAC: ");
            for (i = 0; i < 6; i++)
                if (i == 5)
                    osal_printf("%02x\n", hsb.r8328.smac[i]);
                else
                    osal_printf("%02x:", hsb.r8328.smac[i]);

            osal_printf("fs1: 0x");
            for (i = 0; i < 8; i++)
                osal_printf("%x:", hsb.r8328.fs1[i]);
            osal_printf("%x\n", hsb.r8328.fs1[8]);

            osal_printf("fs2: 0x");
            for (i = 0; i < 8; i++)
                osal_printf("%x:", hsb.r8328.fs2[i]);
            osal_printf("%x\n", hsb.r8328.fs2[8]);

            osal_printf("\nPayload:\n");
            /*for (i = 0; i < 47; i++)
               osal_printf("%x ", hsb.r8328.payload[i]); */
            for (i = 0; i < 11; i++)
            {
                osal_printf("W[%2u]:0x%08x   ", i, (hsb.r8328.payload[4 * i + 3] << 24) | (hsb.r8328.payload[4 * i + 2] << 16) | (hsb.r8328.payload[4 * i + 1] << 8) | (hsb.r8328.payload[4 * i]));
                if ((i % 4) == 3)
                    osal_printf("\n");
            }
            osal_printf("W[%2u]:0x%08x\n", i, (hsb.r8328.payload[46] << 16) | (hsb.r8328.payload[45] << 8) | (hsb.r8328.payload[44]));
            _dumpHsbPayloadInfo(unit, &hsb);
            osal_printf("===============================================================================\n");
        }
            break;

        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
        {
            osal_printf("\n===========================HSB===============================\n");

            osal_printf("page_cnt : 0x%x\t\t", hsb.r8390.page_cnt);
            osal_printf("endsc : 0x%x\t\t", hsb.r8390.endsc);
            osal_printf("bgsc : 0x%x\n", hsb.r8390.bgsc);

            osal_printf("field_seltor11 : 0x%x\t", hsb.r8390.field_seltor11);
            osal_printf("field_seltor10 : 0x%x\t", hsb.r8390.field_seltor10);
            osal_printf("field_seltor9 : 0x%x\n", hsb.r8390.field_seltor9);
            osal_printf("field_seltor8 : 0x%x\t", hsb.r8390.field_seltor8);
            osal_printf("field_seltor7 : 0x%x\t", hsb.r8390.field_seltor7);
            osal_printf("field_seltor6 : 0x%x\n", hsb.r8390.field_seltor6);
            osal_printf("field_seltor5 : 0x%x\t", hsb.r8390.field_seltor5);
            osal_printf("field_seltor4 : 0x%x\t", hsb.r8390.field_seltor4);
            osal_printf("field_seltor3 : 0x%x\n", hsb.r8390.field_seltor3);
            osal_printf("field_seltor2 : 0x%x\t", hsb.r8390.field_seltor2);
            osal_printf("field_seltor1 : 0x%x\t", hsb.r8390.field_seltor1);
            osal_printf("field_seltor0 : 0x%x\n", hsb.r8390.field_seltor0);
            osal_printf("field_sel_vld : 0x%x\t", hsb.r8390.field_sel_vld);
            osal_printf("impls_exp : 0x%x\t\t", hsb.r8390.impls_exp);
            osal_printf("impls_label : 0x%x\n", hsb.r8390.impls_label);

            osal_printf("ompls_exp : 0x%x\t\t", hsb.r8390.ompls_exp);
            osal_printf("ompls_label : 0x%x\t", hsb.r8390.ompls_label);
            osal_printf("arpopcode : 0x%x\n", hsb.r8390.arpopcode);

            osal_printf("target_mac : %02x:%02x:%02x:%02x:%02x:%02x\n", hsb.r8390.target_mac[0], hsb.r8390.target_mac[1], hsb.r8390.target_mac[2], hsb.r8390.target_mac[3], hsb.r8390.target_mac[4], hsb.r8390.target_mac[5]);
            osal_printf("sender_mac : %02x:%02x:%02x:%02x:%02x:%02x\n", hsb.r8390.sender_mac[0], hsb.r8390.sender_mac[1], hsb.r8390.sender_mac[2], hsb.r8390.sender_mac[3], hsb.r8390.sender_mac[4], hsb.r8390.sender_mac[5]);
            osal_printf("dport : 0x%x\t\t", hsb.r8390.dport);
            osal_printf("sport : 0x%x\t\t", hsb.r8390.sport);
            osal_printf("tcp_flag : 0x%x\n", hsb.r8390.tcp_flag);

            osal_printf("ipver : 0x%x\t\t", hsb.r8390.ipver);
            osal_printf("ipttl : 0x%x\t\t", hsb.r8390.ipttl);
            osal_printf("ip_tos : 0x%x\n", hsb.r8390.ip_tos);

            osal_printf("ip_protocol : 0x%x\t", hsb.r8390.ip_protocol);
            osal_printf("ip_flag : 0x%x\t\t", hsb.r8390.ip_flag);
            osal_printf("ip_offset : 0x%x\n", hsb.r8390.ip_offset);

            osal_printf("dip : 0x%x\t\t", hsb.r8390.dip);
            osal_printf("sip : 0x%x\t\t", hsb.r8390.sip);
            osal_printf("otag : 0x%x\n", hsb.r8390.otag);

            osal_printf("itag : 0x%x\t\t", hsb.r8390.itag);
            osal_printf("otpid_idx : 0x%x\t\t", hsb.r8390.otpid_idx);
            osal_printf("itpid_idx : 0x%x\n", hsb.r8390.itpid_idx);

            osal_printf("smac : %02x:%02x:%02x:%02x:%02x:%02x\n", hsb.r8390.smac[0], hsb.r8390.smac[1], hsb.r8390.smac[2], hsb.r8390.smac[3], hsb.r8390.smac[4], hsb.r8390.smac[5]);
            osal_printf("dmac : %02x:%02x:%02x:%02x:%02x:%02x\n", hsb.r8390.dmac[0], hsb.r8390.dmac[1], hsb.r8390.dmac[2], hsb.r8390.dmac[3], hsb.r8390.dmac[4], hsb.r8390.dmac[5]);
            osal_printf("etp : 0x%x\t\t", hsb.r8390.etp);
            osal_printf("iplen : 0x%x\t\t", hsb.r8390.iplen);
            osal_printf("ipv4hdlen : 0x%x\n", hsb.r8390.ipv4hdlen);

            osal_printf("tcpseqzero : 0x%x\t", hsb.r8390.tcpseqzero);
            osal_printf("ipv6hop : 0x%x\t\t", hsb.r8390.ipv6hop);
            osal_printf("ipv6rout : 0x%x\n", hsb.r8390.ipv6rout);

            osal_printf("ipv6frag : 0x%x\t\t", hsb.r8390.ipv6frag);
            osal_printf("ipv6dest : 0x%x\t\t", hsb.r8390.ipv6dest);
            osal_printf("ipv6auth : 0x%x\n", hsb.r8390.ipv6auth);

            osal_printf("ipcsok : 0x%x\t\t", hsb.r8390.ipcsok);
            osal_printf("ipv6ext_long : 0x%x\t", hsb.r8390.ipv6ext_long);
            osal_printf("ipv6 : 0x%x\n", hsb.r8390.ipv6);

            osal_printf("ipv4 : 0x%x\t\t", hsb.r8390.ipv4);
            osal_printf("ompls_if : 0x%x\t\t", hsb.r8390.ompls_if);
            osal_printf("impls_if : 0x%x\n", hsb.r8390.impls_if);

            osal_printf("etag_if : 0x%x\t\t", hsb.r8390.etag_if);
            osal_printf("cputag_if : 0x%x\t\t", hsb.r8390.cputag_if);
            osal_printf("otag_if : 0x%x\n", hsb.r8390.otag_if);

            osal_printf("itag_if : 0x%x\t\t", hsb.r8390.itag_if);
            osal_printf("oampdu : 0x%x\t\t", hsb.r8390.oampdu);
            osal_printf("llc_other : 0x%x\n", hsb.r8390.llc_other);

            osal_printf("pppoe_if : 0x%x\t\t", hsb.r8390.pppoe_if);
            osal_printf("rfc1042 : 0x%x\t\t", hsb.r8390.rfc1042);
            osal_printf("spa : 0x%x\n", hsb.r8390.spa);

            osal_printf("errpkt : 0x%x\t\t", hsb.r8390.errpkt);
            osal_printf("l4hdchk : 0x%x\t\t", hsb.r8390.l4hdchk);
            osal_printf("pktlen : 0x%x\n", hsb.r8390.pktlen);
            osal_printf("=============================================================\n");
        }
            break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
        {
            osal_printf("\n=======================================   HSB   =======================================\n");

            /*CPU TAG*/
            osal_printf("--------------------------------------cpu tag-----------------------------------------\n");
            osal_printf("%-14s = 0x%-10x", "ctag_exist", hsb.r8380.ctag_exist);
            osal_printf("%-14s = 0x%-10x", "bp_fltr_1", hsb.r8380.ctag_bp_fltr_1);
            osal_printf("%-14s = 0x%-10x\n", "bp_fltr_2", hsb.r8380.ctag_bp_fltr_2);

            osal_printf("%-14s = 0x%-10x", "as_tagsts", hsb.r8380.ctag_as_tagsts);
            osal_printf("%-14s = 0x%-10x", "acl_act", hsb.r8380.ctag_acl_act);
            osal_printf("%-14s = 0x%-10x\n", "rvid_sel", hsb.r8380.ctag_rvid_sel);

            osal_printf("%-14s = 0x%-10x", "l2learning", hsb.r8380.ctag_l2learning);
            osal_printf("%-14s = 0x%-10x", "as_pri", hsb.r8380.ctag_as_pri);
            osal_printf("%-14s = 0x%-10x\n", "tag_pri", hsb.r8380.ctag_pri);

            osal_printf("%-14s = 0x%-10x", "as_dpm", hsb.r8380.ctag_as_dpm);
            osal_printf("%-14s = 0x%-10x", "physical_dpm", hsb.r8380.ctag_physical_dpm);
            osal_printf("%-14s = 0x%-10x\n", "dpm", hsb.r8380.ctag_dpm);
            /*tag status*/
            osal_printf("-----------------------------------otag/itag/extra tag-----------------------------------\n");
            osal_printf("%-14s = 0x%-10x", "rtag_exist", hsb.r8380.rtag_exist);
            osal_printf("%-14s = 0x%-10x", "otag_exist", hsb.r8380.otag_exist);
            osal_printf("%-14s = 0x%-10x\n", "otag_content", hsb.r8380.otag_content);

            osal_printf("%-14s = 0x%-10x", "otag_index", hsb.r8380.otag_index);
            osal_printf("%-14s = 0x%-10x", "itag_exist", hsb.r8380.itag_exist);
            osal_printf("%-14s = 0x%-10x\n", "itag_content", hsb.r8380.itag_content);

            osal_printf("%-14s = 0x%-10x", "itag_index", hsb.r8380.itag_index);
            osal_printf("%-14s = 0x%-10x\n", "etag_exist", hsb.r8380.etag_exist);
            osal_printf("----------------------------------------pkt type----------------------------------------\n");
            osal_printf("%-14s = 0x%-10x\n", "pppoe_pkt", hsb.r8380.pppoe_pkt);

            osal_printf("%-14s = 0x%-10x", "rtkpp", hsb.r8380.rtkpp);
            osal_printf("%-14s = 0x%-10x", "frame_type", hsb.r8380.frame_type);
            osal_printf("%-14s = 0x%-10x\n", "typelen", hsb.r8380.typelen);

            osal_printf("%-14s = 0x%-10x", "ipv4_pkt", hsb.r8380.ipv4_pkt);
            osal_printf("%-14s = 0x%-10x", "ipv6_pkt", hsb.r8380.ipv6_pkt);
            osal_printf("%-14s = 0x%-10x\n", "arp_pkt", hsb.r8380.arp_pkt);

            osal_printf("%-14s = 0x%-10x", "icmp_pkt", hsb.r8380.icmp_pkt);
            osal_printf("%-14s = 0x%-10x", "igmp_pkt", hsb.r8380.igmp_pkt);
            osal_printf("%-14s = 0x%-10x\n", "udp_pkt", hsb.r8380.udp_pkt);

            osal_printf("%-14s = 0x%-10x", "tcp_pkt", hsb.r8380.tcp_pkt);
            osal_printf("%-14s = 0x%-10x", "ip_mf", hsb.r8380.ip_mf);
            osal_printf("%-14s = 0x%-10x\n", "ip_dstype", hsb.r8380.ip_dstype);

            osal_printf("%-14s = 0x%-10x", "ip_protocol", hsb.r8380.ip_protocol);
            osal_printf("%-14s = 0x%-10x", "ip_offset", hsb.r8380.ip_offset);
            osal_printf("%-14s = 0x%-10x\n", "ip_length", hsb.r8380.ip_length);

            osal_printf("%-14s = 0x%-10x", "ip_ttl", hsb.r8380.ip_ttl);
            osal_printf("%-14s = 0x%-10x", "ipv4_hdrlen", hsb.r8380.ipv4_hdrlen);
            osal_printf("%-14s = 0x%-10x\n", "ipv4_df", hsb.r8380.ipv4_df);

            osal_printf("%-14s = 0x%-10x", "exten_hdrlen", hsb.r8380.ipv6_extension_hdrlen);
            osal_printf("%-14s = 0x%-10x", "tcp_hdrlen", hsb.r8380.tcp_hdrlen);
            osal_printf("%-14s = 0x%-10x\n", "tcp_flag", hsb.r8380.tcp_flag);

            osal_printf("%-14s = 0x%-10x", "tcp_sn_eq_0", hsb.r8380.tcp_sn_eq_0);
            osal_printf("%-14s = 0x%-10x", "l4_content", hsb.r8380.l4_content);
            osal_printf("%-14s = 0x%-10x\n", "flow_label", hsb.r8380.ipv6_flow_label);

            osal_printf("%-14s = 0x%-10x", "mob_ext_hdr", hsb.r8380.ipv6_mob_ext_hdr);
            osal_printf("%-14s = 0x%-10x", "hbh_ext_err", hsb.r8380.ipv6_hbh_ext_hdr_err);
            osal_printf("%-14s = 0x%-10x\n\n", "unknown_hdr", hsb.r8380.ipv6_unknown_hdr);

            /*MAC Address*/
            osal_printf("%-14s = 0x", "DMAC:");
            for(index=0; index<6; index++)
                osal_printf("%02x ", hsb.r8380.dmac[index]);
            osal_printf("\n");
            osal_printf("%-14s = 0x", "SMAC:");
            for(index=0; index<6; index++)
                osal_printf("%02x ", hsb.r8380.smac[index]);
            osal_printf("\n\n");

            /*dip address*/
            osal_printf("%-14s = 0x", "dip");
            for(index=0; index<16; index++)
                osal_printf("%02x ", hsb.r8380.dip[index]);
            osal_printf("\n");
            /*sip address*/
            osal_printf("%-14s = 0x", "sip");
            for(index=0; index<16; index++)
                osal_printf("%02x ", hsb.r8380.sip[index]);
            osal_printf("\n");

            //osal_printf("\n");
            osal_printf("--------------------------------------field selectors-----------------------------------\n");
            osal_printf("%-14s = 0x%-10x", "fs3_valid", hsb.r8380.fs3_valid);
            osal_printf("%-14s = 0x%-10x", "fs2_valid", hsb.r8380.fs2_valid);
            osal_printf("%-14s = 0x%-10x\n", "fs1_valid", hsb.r8380.fs1_valid);

            osal_printf("%-14s = 0x%-10x", "fs0_valid", hsb.r8380.fs0_valid);
            osal_printf("%-14s = 0x%-10x", "fs3_data", hsb.r8380.fs3_data);
            osal_printf("%-14s = 0x%-10x\n", "fs2_data", hsb.r8380.fs2_data);

            osal_printf("%-14s = 0x%-10x", "fs1_data", hsb.r8380.fs1_data);
            osal_printf("%-14s = 0x%-10x\n", "fs0_data", hsb.r8380.fs0_data);
            osal_printf("-------------------------------------------misc-----------------------------------------\n");
            osal_printf("%-14s = 0x%-10x", "sphy", hsb.r8380.sphy);
            osal_printf("%-14s = 0x%-10x", "pktlen", hsb.r8380.pktlen);
            osal_printf("%-14s = 0x%-10x\n", "crceq", hsb.r8380.crceq);
            osal_printf("%-14s = 0x%-10x", "rxdrop", hsb.r8380.rxdrop);
            osal_printf("%-14s = 0x%-10x", "oampdu", hsb.r8380.oampdu);
            osal_printf("%-14s = 0x%-10x", "rx_rm_rtag", hsb.r8380.rx_rm_rtag);
            osal_printf("\n====================================================================================\n");
        }
            break;

        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}

#if defined(CONFIG_SDK_RTL8328)
/* Function Name:
 *      hal_dumpPpi
 * Description:
 *      Dump ppi paramter of the specified device.
 * Input:
 *      unit - unit id
 *      index -ppi index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_dumpPpi(uint32 unit, uint32 index)
{
    uint32 i;
    ppi_param_t   ppi;

    if(RT_ERR_OK != _rtk_getPpi(unit, index, &ppi))
    {
        return RT_ERR_FAILED;
    }

    osal_printf("\n====================================PPI========================================\n");
    osal_printf("sphy:%-2d\t\t", ppi.sphy);
    osal_printf("crsvlan:%d\t", ppi.crsvlan);
    osal_printf("keeporigivid:%d\t", ppi.keeporigivid);
    osal_printf("keeporigovid:%d\n", ppi.keeporigovid);

    osal_printf("pbivid:%d\t", ppi.pbivid);
    osal_printf("pbovid:%d\t", ppi.pbovid);
    osal_printf("pbipri:%d\t", ppi.pbipri);
    osal_printf("pbodei:%d\n", ppi.pbodei);

    osal_printf("ipuc:%d\t\t", ppi.ipuc);
    osal_printf("pbopri:0x%x\t", ppi.pbopri);
    osal_printf("ppbivid:%d\t", ppi.ppbivid);
    osal_printf("ppbovid:%d\n", ppi.ppbovid);


    osal_printf("ppbipri:%d\t", ppi.ppbipri);
    osal_printf("ppbodei:%d\t", ppi.ppbodei);
    osal_printf("upLinkMac_0:%d\t", ppi.upLinkMac_0);
    osal_printf("ppbopri:%d\n", ppi.ppbopri);

    osal_printf("fcivid:%d\t", ppi.fcivid);
    osal_printf("fcovid:%d\t", ppi.fcovid);
    osal_printf("rrcpType:%d\t", ppi.rrcpType);
    osal_printf("fcseldot1qipri:%d\n", ppi.fcseldot1qipri);

    osal_printf("rrcpAuth:%d\t", ppi.rrcpAuth);
    osal_printf("ivid:%d\t\t", ppi.ivid);
    osal_printf("ovid:%d\t\t", ppi.ovid);
    osal_printf("fcseldot1qopri:%d\n", ppi.fcseldot1qopri);

    osal_printf("drop:%d\t\t", ppi.drop);
    osal_printf("dpm:0x%-8x\t", ppi.dpm);
    osal_printf("dot1qipri:%d\t", ppi.dot1qipri);
    osal_printf("cpu:0x%x\n", ppi.cpu);

    osal_printf("cputag:%d\t", ppi.cputag);
    osal_printf("toGVlan:%d\t", ppi.toGVlan);
    osal_printf("ipmcPkt:0x%x\t", ppi.ipmcPkt);
    osal_printf("dot1qodp:%d\n", ppi.dot1qodp);

    osal_printf("sml:%d\t\t", ppi.sml);
    osal_printf("redir:0x%x\t", ppi.redir);
    osal_printf("lkmiss:%d\t", ppi.lkmiss);
    osal_printf("ftidx:%d\n", ppi.ftidx);

    osal_printf("vidrch:0x%x\t", ppi.vidrch);
    osal_printf("dpn:%d\t\t", ppi.dpn);
    osal_printf("pbdp:%d\t\t", ppi.pbdp);
    osal_printf("pbpri:0x%x\n", ppi.pbpri);

    osal_printf("ftIdxValid:%d\t", ppi.ftIdxValid);
    osal_printf("dot1qidp:0x%x\t", ppi.dot1qidp);
    osal_printf("dscppriexist:%d\t", ppi.dscppriexist);
    osal_printf("dscpdp:%d\n", ppi.dscpdp);

    osal_printf("dscppri:%d\t", ppi.dscppri);
    osal_printf("fbpriexist:%d\t", ppi.fbpriexist);
    osal_printf("fbdp:%d\t\t", ppi.fbdp);
    osal_printf("fbpri:%d\n", ppi.fbpri);

    osal_printf("ipriexist:%d\t", ppi.ipriexist);
    osal_printf("idp:%d\t\t", ppi.idp);
    osal_printf("ipri:%d\t\t", ppi.ipri);
    osal_printf("opriexist:%d\n", ppi.opriexist);

    osal_printf("odp:%d\t\t", ppi.odp);
    osal_printf("opri:%d\t\t", ppi.opri);
    osal_printf("dp:%d\t\t", ppi.dp);
    osal_printf("pri:%d\t\n", ppi.pri);

    osal_printf("macdot1x:%d\t", ppi.macdot1x);
    osal_printf("fcdfi:%d\t\t", ppi.fcdfi);
    osal_printf("fcdfo:%d\t\t", ppi.fcdfo);
    osal_printf("msti:%d\n", ppi.msti);

    osal_printf("fid:%d\t\t", ppi.fid);
    osal_printf("miriutag:%d\t", ppi.miriutag);
    osal_printf("miroutag:%d\t", ppi.miroutag);
    osal_printf("flood:%d\t\n", ppi.flood);

    osal_printf("ppbdfi:%d\t", ppi.ppbdfi);
    osal_printf("ppbdfo:%d\t", ppi.ppbdfo);
    osal_printf("tm:%d\t\t", ppi.tm);
    osal_printf("mir:%d\t\n", ppi.mir);

    osal_printf("ipmc:%d\t\t", ppi.ipmc);
    osal_printf("copyToCpu:%d\t", ppi.copyToCpu);
    osal_printf("vidrc:%2u\t", ppi.vidrc);
    osal_printf("reason:0x%x\n", ppi.reason);

    osal_printf("dscprmk:%d\t", ppi.dscprmk);
    osal_printf("dot1qopri:%d\t", ppi.dot1qopri);
    osal_printf("ce:%d\t\t", ppi.ce);
    osal_printf("iutagstatus:0x%x\n", ppi.iutagstatus);

    osal_printf("iuntagValid:%d\t", ppi.iuntagValid);
    osal_printf("ountagValid:%d\t", ppi.ountagValid);
    osal_printf("outagstatus:0x%x\t", ppi.outagstatus);
    osal_printf("dscp:%2d\n", ppi.dscp);

    osal_printf("orgvid:%d\t", ppi.orgvid);
    osal_printf("dmacidx:%d\t", ppi.dmacidx);
    osal_printf("rrcpRegData:%d\t", ppi.rrcpRegData);
    osal_printf("prc:%d\n", ppi.prc);

    osal_printf("tmidx:%d\t\t", ppi.tmidx);
    osal_printf("ttldec:%d\t", ppi.ttldec);
    osal_printf("aclidx0:0x%x\t", ppi.aclidx0);
    osal_printf("aclidx1:0x%x\n", ppi.aclidx1);

    osal_printf("aclidx2:0x%x\t", ppi.aclidx2);
    osal_printf("aclidx3:0x%x\t", ppi.aclidx3);
    osal_printf("aclidx4:0x%x\t", ppi.aclidx4);
    osal_printf("aclidx5:0x%x\n", ppi.aclidx5);

    osal_printf("aclidx6:0x%x\t", ppi.aclidx6);
    osal_printf("aclidx7:0x%x\t", ppi.aclidx7);
    osal_printf("aclidx8:0x%x\t", ppi.aclidx8);
    osal_printf("aclidx9:0x%x\n", ppi.aclidx9);

    osal_printf("aclidx10:0x%x\t", ppi.aclidx10);
    osal_printf("aclidx11:0x%x\t", ppi.aclidx11);
    osal_printf("aclidx12:0x%x\t", ppi.aclidx12);
    osal_printf("aclidx13:0x%x\n", ppi.aclidx13);

    osal_printf("aclidx14:0x%x\t", ppi.aclidx14);
    osal_printf("aclidx15:0x%x \n", ppi.aclidx15);
    osal_printf("iprc: 0x");
    for (i = 0; i < 8; i++)
        osal_printf(" %x", ppi.iprc[i]);

    osal_printf("\n===============================================================================\n");
    return RT_ERR_OK;
}

/* Function Name:
 *      hal_dumpPmi
 * Description:
 *      Dump pmi paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_dumpPmi(uint32 unit)
{
    pmi_param_t   pmi;
    rtk_portmask_t portmask;
    uint8 portlist[MAX_STRING_LEN];

    if(RT_ERR_OK != _rtk_getPmi(unit, &pmi))
    {
        return RT_ERR_FAILED;
    }

    osal_printf("\n====================================PMI========================================\n");
    if ((pmi.rrcp) && (pmi.rrcpType == 3))
    {
        osal_printf("asdp:0x%x\t\t", pmi.asdp);
        osal_printf("asdprmk:0x%x\t\t", pmi.asdprmk);
        osal_printf("dipri:0x%x\t\t", pmi.dipri);
        osal_printf("divid:0x%x\t", pmi.divid);
        osal_printf("dopri:0x%x\n", pmi.dopri);
        osal_printf("dovid:0x%x\t\t", pmi.dovid);
        osal_printf("dp:0x%x\t\t\t", pmi.dp);
        osal_printf("dpcnt:0x%x\t", pmi.dpcnt);
        portmask.bits[0] = pmi.dpm;
        lPortMask2str(portlist, &portmask);
        osal_printf("dpm: %4s\t\t", portlist);
        osal_printf("dscprmk:0x%x\n", pmi.dscprmk);
        osal_printf("evten:0x%x\t\t", pmi.evten);
        osal_printf("extraTag:0x%x\t\t", pmi.extraTag);
        osal_printf("framelength:0x%x\t", pmi.framelength);
        osal_printf("fwd:0x%x\t\t\t", pmi.fwd);
        osal_printf("iutagStatus:0x%x\n", pmi.iutagStatus);
        osal_printf("keeporigivid:0x%x\t", pmi.keeporigivid);
        osal_printf("keeporigovid:0x%x\t", pmi.keeporigovid);
        osal_printf("l2fmt:0x%x\t\t", pmi.l2fmt);
        osal_printf("l2err:0x%x\t\t", pmi.l2err);
        osal_printf("l3err:0x%x\n", pmi.l3err);
        osal_printf("l4err:0x%x\t\t", pmi.l4err);
        osal_printf("lb:0x%x\t\t", pmi.lb);
        osal_printf("mir:0x%x\t\t\t", pmi.mir);
        osal_printf("miriutag:0x%x\t", pmi.miriutag);
        osal_printf("mirorg:0x%x\n", pmi.mirorg);
        osal_printf("miroutag:0x%x\t\t", pmi.miroutag);
        osal_printf("oam_lbsa:0x%x\t\t", pmi.oam_lbsa);
        osal_printf("oam_lbda:0x%x\t\t", pmi.oam_lbda);
        osal_printf("occupy:0x%x\t\t", pmi.occupy);
        osal_printf("outagStatus:0x%x\n", pmi.outagStatus);
        osal_printf("pnxt:0x%x\t\t", pmi.pnxt);
        osal_printf("pppoe:0x%x\t\t", pmi.pppoe);
        osal_printf("pri:0x%x\t\t\t", pmi.pri);
        osal_printf("reason:0x%x\t\t", pmi.reason);
        osal_printf("rrcp:0x%x\n", pmi.rrcp);
        osal_printf("rrcp register data:0x%x\t", pmi.ipuc | (pmi.ipmc << 1) | (pmi.l34fmt << 2) |
                     (pmi.dmacidx << 6) | (pmi.dscp << 20) | (pmi.ce << 26) | (pmi.rrcpRegData31_27 << 27));
        osal_printf("rrcpType:0x%x\t\t", pmi.rrcpType);
        osal_printf("rxcpuTag:0x%x\t\t", pmi.rxcpuTag);
        osal_printf("rxrspan:0x%x\t\t", pmi.rxrspan);
        osal_printf("sphy:0x%x\n", pmi.sphy);
        osal_printf("stphy:0x%x\t\t", pmi.stphy);
        osal_printf("ttldec:0x%x\t\t", pmi.ttldec);
        osal_printf("txcputag:0x%x\t\n", pmi.txcputag);
    }
    else
    {
        osal_printf("asdp:0x%01x          asdprmk:0x%01x       ce:0x%01x        dipri:0x%01x     divid:0x%03x\n", pmi.asdp, pmi.asdprmk, pmi.ce, pmi.dipri, pmi.divid);
        osal_printf("dmacidx:0x%04x    dopri:0x%01x         dovid:0x%03x   dp:0x%01x        dpcnt:0x%02x\n", pmi.dmacidx, pmi.dopri, pmi.dovid, pmi.dp, pmi.dpcnt);
        //portmask.bits[0] = pmi.dpm;
        //lPortMask2str(portlist, &portmask);
        //osal_printf("dpm: %4s\t\t", portlist);
        osal_printf("dpm:0x%08x    dscp:0x%02x         dscprmk:0x%01x   evten:0x%01x     extraTag:0x%01x\n", pmi.dpm, pmi.dscp, pmi.dscprmk, pmi.evten, pmi.extraTag);
        osal_printf("iutagStatus:0x%08x              fwd:0x%01x       ipmc:0x%01x      ipuc:0x%01x\n", pmi.iutagStatus, pmi.fwd, pmi.ipmc, pmi.ipuc);
        osal_printf("keeporigivid:0x%01x  keeporigovid:0x%01x  l2fmt:0x%01x     l34fmt:0x%01x    l2err:0x%01x\n", pmi.keeporigivid, pmi.keeporigovid, pmi.l2fmt, pmi.l34fmt, pmi.l2err);
        osal_printf("l3err:0x%01x         l4err:0x%01x         lb:0x%01x        mir:0x%01x       miriutag:0x%01x\n", pmi.l3err, pmi.l4err, pmi.lb, pmi.mir, pmi.miriutag);
        osal_printf("mirorg:0x%01x        miroutag:0x%01x      oam_lbsa:0x%01x  oam_lbda:0x%01x  occupy:0x%01x\n", pmi.mirorg, pmi.miroutag, pmi.oam_lbsa, pmi.oam_lbda, pmi.occupy);
        osal_printf("outagStatus:0x%08x              pnxt:0x%03x    pppoe:0x%01x     reason:0x%04x\n", pmi.outagStatus, pmi.pnxt, pmi.pppoe, pmi.reason);
        osal_printf("rrcp:0x%01x          rrcpType:0x%01x      rxcpuTag:0x%01x  rxrspan:0x%01x   sphy:0x%02x\n", pmi.rrcp, pmi.rrcpType, pmi.rxcpuTag, pmi.rxrspan, pmi.sphy);
        osal_printf("stphy:0x%02x        ttldec:0x%01x        txcputag:0x%01x  pri:0x%01x       framelen:0x%04x\n", pmi.stphy, pmi.ttldec, pmi.txcputag, pmi.pri, pmi.framelength);
    }

    osal_printf("===============================================================================\n");
    return RT_ERR_OK;
}
#endif

#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
int32 _rtk_getHsa(uint32 unit, hsa_param_t * hsa)
{
#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    uint32 value, tmpVal = 0;
#endif
#if defined(CONFIG_SDK_RTL8390)
    uint32 port, reg, field;
    uint32 tmpVal1 = 0, tmpVal2 = 0;
#endif
    hal_control_t *pInfo;

    if(NULL == hsa)
    {
        return RT_ERR_FAILED;
    }

    memset(hsa, 0, sizeof(hsa_param_t));
    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8389M_CHIP_ID:
        case RTL8389L_CHIP_ID:
        case RTL8329M_CHIP_ID:
        case RTL8377M_CHIP_ID:
#if defined(CONFIG_SDK_RTL8389)
        {
            reg_read(unit, SSW_HEADER_STAMP_AFTER_CONTROL0r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL0r, SSW_HSA_BUSYf, &tmpVal, &value);
            hsa->r8389.hsa_busy = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL0r, SSW_NEWSVIDf, &tmpVal, &value);
            hsa->r8389.newsvid = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL0r, SSW_NEWVIDf, &tmpVal, &value);
            hsa->r8389.newvid = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL0r, SSW_CPUTAGIFf, &tmpVal, &value);
            hsa->r8389.cputagif = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL0r, SSW_DPCNTf, &tmpVal, &value);
            hsa->r8389.dpcnt_4 = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_AFTER_CONTROL1r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL1r, SSW_DPCNTf, &tmpVal, &value);
            hsa->r8389.dpcnt_3_0 = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL1r, SSW_RVIDf, &tmpVal, &value);
            hsa->r8389.rvid = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL1r, SSW_REASONf, &tmpVal, &value);
            hsa->r8389.reason = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_AFTER_CONTROL2r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL2r, SSW_INTPRIf, &tmpVal, &value);
            hsa->r8389.intpri = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL2r, SSW_DPMASKf, &tmpVal, &value);
            hsa->r8389.dpmask = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_MIR1DPAf, &tmpVal, &value);
            hsa->r8389.mir1dpa = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_MIR0DPAf, &tmpVal, &value);
            hsa->r8389.mir0dpa = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_CPUASDPRMKf, &tmpVal, &value);
            hsa->r8389.cpuasdprmk = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_IPV6f, &tmpVal, &value);
            hsa->r8389.ipv6 = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW___IPV4f, &tmpVal, &value);
            hsa->r8389.ipv4 = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_PPPOEf, &tmpVal, &value);
            hsa->r8389.pppoe = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_STAGIFf, &tmpVal, &value);
            hsa->r8389.stagif = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_CTAGIFf, &tmpVal, &value);
            hsa->r8389.ctagif = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_FRAMETYPEf, &tmpVal, &value);
            hsa->r8389.frametype = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL3r, SSW_PKTLENf, &tmpVal, &value);
            hsa->r8389.pktlen = tmpVal;

            reg_read(unit, SSW_HEADER_STAMP_AFTER_CONTROL4r, &value);
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL4r, SSW_L4CSOKf, &tmpVal, &value);
            hsa->r8389.l4csok = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL4r, SSW_L3CSOKf, &tmpVal, &value);
            hsa->r8389.l3csok = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL4r, SSW_ENDPAGEf, &tmpVal, &value);
            hsa->r8389.endpage = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL4r, SSW_STARTPAGEf, &tmpVal, &value);
            hsa->r8389.startpage = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL4r, SSW_STARTBANKf, &tmpVal, &value);
            hsa->r8389.startbank = tmpVal;
            reg_field_get(unit, SSW_HEADER_STAMP_AFTER_CONTROL4r, SSW_SPAf, &tmpVal, &value);
            hsa->r8389.spa = tmpVal;
        }
#endif
            break;

        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8390)
        {
            /* HSA_DATA0 */
            reg_read(unit, CYPRESS_HSA_DATA0r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_PAGE_CNTf, &tmpVal, &value);
            hsa->r8390.page_cnt = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_ACL_MPLS_HITf, &tmpVal, &value);
            hsa->r8390.acl_mpls_hit = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_ACL_MPLS_ACTf, &tmpVal, &value);
            hsa->r8390.acl_mpls_act = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_MPLS_INDEXf, &tmpVal, &value);
            hsa->r8390.mpls_index = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_ASTAGSTSf, &tmpVal, &value);
            hsa->r8390.astagsts = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_CTAG_DMf, &tmpVal, &value);
            hsa->r8390.ctag_dm = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_CTAG_DGPKTf, &tmpVal, &value);
            hsa->r8390.ctag_dgpkt = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_TX_PTP_LOGf, &tmpVal, &value);
            hsa->r8390.tx_ptp_log = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_TX_PTP_OFFLOADf, &tmpVal, &value);
            hsa->r8390.tx_ptp_offload = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_NEW_SAf, &tmpVal, &value);
            hsa->r8390.new_sa = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_MAC_CSTf, &tmpVal, &value);
            hsa->r8390.mac_cst = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA0r, CYPRESS_STC_L2_PMVf, &tmpVal, &value);
            hsa->r8390.l2_pmv = tmpVal;

            /* HSA_DATA1 */
            value = 0;
            reg_read(0, CYPRESS_HSA_DATA1r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA1r, CYPRESS_ATK_TYPEf, &tmpVal, &value);
            hsa->r8390.atk_type = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA1r, CYPRESS_DM_RXIDXf, &tmpVal, &value);
            hsa->r8390.dm_rxidx = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA1r, CYPRESS_ACL_HITf, &tmpVal, &value);
            hsa->r8390.acl_hit = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA1r, CYPRESS_ACL_IDf, &tmpVal, &value);
            hsa->r8390.acl_id = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA1r, CYPRESS_EAV_CLASS_Bf, &tmpVal, &value);
            hsa->r8390.eav_class_b = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA1r, CYPRESS_EAV_CLASS_Af, &tmpVal, &value);
            hsa->r8390.eav_class_a = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA1r, CYPRESS_REASONf, &tmpVal, &value);
            hsa->r8390.reason = tmpVal;

            /* HSA_DATA2 */
            value = 0;
            reg_read(0, CYPRESS_HSA_DATA2r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_ACL_TOSf, &tmpVal, &value);
            hsa->r8390.acl_tos = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_NEW_TOSf, &tmpVal, &value);
            hsa->r8390.new_tos = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_INTERNAL_PRIf, &tmpVal, &value);
            hsa->r8390.internal_pri = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_OPRI_ACTf, &tmpVal, &value);
            hsa->r8390.opri_act = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_IPRI_ACTf, &tmpVal, &value);
            hsa->r8390.ipri_act = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_C2SC_PRIf, &tmpVal, &value);
            hsa->r8390.c2sc_pri = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_EACL_ITAGf, &tmpVal, &value);
            hsa->r8390.eacl_itag = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_EACL_OTAGf, &tmpVal, &value);
            hsa->r8390.eacl_otag = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_C2SC_ITAGf, &tmpVal, &value);
            hsa->r8390.c2sc_itag = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_C2SC_OTAGf, &tmpVal, &value);
            hsa->r8390.c2sc_otag = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_ITAG_STATUSf, &tmpVal, &value);
            hsa->r8390.itag_status = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_OTAG_STATUSf, &tmpVal, &value);
            hsa->r8390.otag_status = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_EACL_IVIDf, &tmpVal, &value);
            hsa->r8390.eacl_ivid = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_C2SC_IVIDf, &tmpVal, &value);
            hsa->r8390.c2sc_ivid = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_EACL_OTPIDf, &tmpVal, &value);
            hsa->r8390.eacl_otpid = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_OTPID_IDXf, &tmpVal, &value);
            hsa->r8390.otpid_idx = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_EACL_ITPIDf, &tmpVal, &value);
            hsa->r8390.eacl_itpid = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_C2SC_ITPIDf, &tmpVal, &value);
            hsa->r8390.c2sc_itpid = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA2r, CYPRESS_ITPID_IDXf, &tmpVal, &value);
            hsa->r8390.itpid_idx = tmpVal;

            /* HSA_DATA3 */
            value = 0;
            reg_read(0, CYPRESS_HSA_DATA3r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA3r, CYPRESS_NEW_OTAGf, &tmpVal, &value);
            hsa->r8390.new_otag = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA3r, CYPRESS_NEW_ITAGf, &tmpVal, &value);
            hsa->r8390.new_itag = tmpVal;

            /* HSA_DATA4 */
            value = 0;
            reg_read(unit, CYPRESS_HSA_DATA4r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA4r, CYPRESS_ALE_OVIDf, &tmpVal, &value);
            hsa->r8390.ale_ovid = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA4r, CYPRESS_L3_ROUTEf, &tmpVal, &value);
            hsa->r8390.l3_route = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA4r, CYPRESS_DA_PIDf, &tmpVal, &value);
            hsa->r8390.da_pid = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA4r, CYPRESS_NORMAL_FWDf, &tmpVal, &value);
            hsa->r8390.normal_fwd = tmpVal;

            /* HSA_DATA5 */
            value = 0;
            reg_read(0, CYPRESS_HSA_DATA5r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA5r, CYPRESS_SWAP_MACf, &tmpVal, &value);
            hsa->r8390.swap_mac = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA5r, CYPRESS_CPU_FWDf, &tmpVal, &value);
            hsa->r8390.cpu_fwd = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA5r, CYPRESS_SFLOW_INFOf, &tmpVal, &value);
            hsa->r8390.sflow_info = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA5r, CYPRESS_MIRROR_1_INFOf, &tmpVal, &value);
            hsa->r8390.mirror_1_info = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA5r, CYPRESS_MIRROR_2_INFOf, &tmpVal, &value);
            hsa->r8390.mirror_2_info = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA5r, CYPRESS_MIRROR_3_INFOf, &tmpVal, &value);
            hsa->r8390.mirror_3_info = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA5r, CYPRESS_MIRROR_4_INFOf, &tmpVal, &value);
            hsa->r8390.mirror_4_info = tmpVal;

            /* HSA_DATA6 ~ HSA_DATA11*/
            for (port = 0; port < 53; port++)
            {
                reg = CYPRESS_HSA_DATA6r + (port / 10);
                if (0 == ((53 - port) / 10))
                    field = CYPRESS_PORT50_QIDf - (port % 10);
                else
                    field = CYPRESS_PORT9_QIDf + ((port / 10) * 10) + (9 - (port % 10));

                tmpVal1 = 0;
                reg_field_read(0, reg, field, &tmpVal1);
                hsa->r8390.qid[port].qid = tmpVal1;
            }

            /* HSA_DATA12 */
            value = 0;
            reg_read(unit, CYPRESS_HSA_DATA12r, &value);  /* FIX ME */
            reg_field_get(unit, CYPRESS_HSA_DATA12r, CYPRESS_COLORf, &tmpVal, &value);
            hsa->r8390.color = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA12r, CYPRESS_RVID_SELf, &tmpVal, &value);
            hsa->r8390.rvid_sel = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA12r, CYPRESS_RVIDf, &tmpVal, &value);
            hsa->r8390.rvid = tmpVal;

            /* HSA_DATA13, 14 */
            tmpVal1 = 0;
            tmpVal2 = 0;
            reg_field_read(unit, CYPRESS_HSA_DATA13r, CYPRESS_DPM_52_32f, &tmpVal2);
            reg_field_read(unit, CYPRESS_HSA_DATA14r, CYPRESS_DPM_31_0f, &tmpVal1);
            hsa->r8390.dpm.bits[1] = tmpVal2;
            hsa->r8390.dpm.bits[0] = tmpVal1;

            tmpVal = 0;
            reg_field_read(unit, CYPRESS_HSA_DATA13r, CYPRESS_DPCf, &tmpVal);
            hsa->r8390.dpc = tmpVal;

            /* HSA_DATA15 */
            value = 0;
            reg_read(unit, CYPRESS_HSA_DATA15r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA15r, CYPRESS_ENDSCf, &tmpVal, &value);
            hsa->r8390.endsc = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA15r, CYPRESS_BGDSCf, &tmpVal, &value);
            hsa->r8390.bgdsc = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA15r, CYPRESS_STPID_IDXf, &tmpVal, &value);
            hsa->r8390.stpid_idx = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA15r, CYPRESS_CTPID_IDXf, &tmpVal2, &value);
            hsa->r8390.ctpid_idx = tmpVal2;

            /* HSA_DATA16 */
            value = 0;
            reg_read(unit, CYPRESS_HSA_DATA16r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA16r, CYPRESS_ORG_OTAGf, &tmpVal, &value);
            hsa->r8390.org_otag = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA16r, CYPRESS_ORG_ITAGf, &tmpVal, &value);
            hsa->r8390.org_itag = tmpVal;

            /* HSA_DATA17 */
            value = 0;
            reg_read(0, CYPRESS_HSA_DATA17r, &value);
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_OMPLS_IFf, &tmpVal, &value);
            hsa->r8390.ompls_if = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_IMPLS_IFf, &tmpVal, &value);
            hsa->r8390.impls_if = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_ORG_ETAG_IFf, &tmpVal, &value);
            hsa->r8390.org_etag_if = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_ORG_CPUTAG_IFf, &tmpVal, &value);
            hsa->r8390.org_cputag_if = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_ORG_OTAG_IFf, &tmpVal, &value);
            hsa->r8390.org_otag_if = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_ORG_ITAG_IFf, &tmpVal, &value);
            hsa->r8390.org_itag_if = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_IPV6f, &tmpVal, &value);
            hsa->r8390.ipv6 = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_LLC_OTHERf, &tmpVal, &value);
            hsa->r8390.llc_other = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_PPPOE_IFf, &tmpVal, &value);
            hsa->r8390.pppoe_if = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_RFC1042f, &tmpVal, &value);
            hsa->r8390.rfc1042 = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_SPNf, &tmpVal, &value);
            hsa->r8390.spa = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_ERRPKTf, &tmpVal, &value);
            hsa->r8390.errpkt = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_IPV4f, &tmpVal, &value);
            hsa->r8390.ipv4 = tmpVal;
            reg_field_get(unit, CYPRESS_HSA_DATA17r, CYPRESS_PKTLENf, &tmpVal, &value);
            hsa->r8390.pktlen = tmpVal;
        }
#endif
            break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8380)
        {
            /* HSA_DATA0 */
            reg_read(0, MAPLE_HSA_DATA0r, &value);
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ORG_ITPID_IDXf, &tmpVal, &value);
            hsa->r8380.org_itpid_idx = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ORG_OTPID_IDXf, &tmpVal, &value);
            hsa->r8380.org_otpid_idx = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_HSA_ORG_OCFIf, &tmpVal, &value);
            hsa->r8380.org_ocfi = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ORG_OPRIf, &tmpVal, &value);
            hsa->r8380.org_opri = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ORG_OVIDf, &tmpVal, &value);
            hsa->r8380.org_ovid = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ORG_ITAG_IFf, &tmpVal, &value);
            hsa->r8380.org_itag_if = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ORG_OTAG_IFf, &tmpVal, &value);
            hsa->r8380.org_otag_if = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ALE_ITAG_STSf, &tmpVal, &value);
            hsa->r8380.ale_itag_sts = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ALE_OTAG_STSf, &tmpVal, &value);
            hsa->r8380.ale_otag_sts = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ALE_ITAG_HITf, &tmpVal, &value);
            hsa->r8380.ale_itag_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ALE_OTAG_HITf, &tmpVal, &value);
            hsa->r8380.ale_otag_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_ASTAGSTSf, &tmpVal, &value);
            hsa->r8380.astagsts = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA0r, MAPLE_CPU_TAG_IFf, &tmpVal, &value);
            hsa->r8380.cpu_tag_if = tmpVal;

            /* HSA_DATA1 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA1r, &value);
            reg_field_get(0, MAPLE_HSA_DATA1r, MAPLE_ALE_INT_PRIf, &tmpVal, &value);
            hsa->r8380.ale_int_pri = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA1r, MAPLE_FWD_VIDf, &tmpVal, &value);
            hsa->r8380.fwd_vid = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA1r, MAPLE_FWD_BASEf, &tmpVal, &value);
            hsa->r8380.fwd_base = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA1r, MAPLE_ORG_ICFIf, &tmpVal, &value);
            hsa->r8380.org_icfi = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA1r, MAPLE_ORG_IPRIf, &tmpVal, &value);
            hsa->r8380.org_ipri = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA1r, MAPLE_ORG_IVIDf, &tmpVal, &value);
            hsa->r8380.org_ivid = tmpVal;

            /* HSA_DATA2 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA2r, &value);
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ALE_IVID_HITf, &tmpVal, &value);
            hsa->r8380.ale_ivid_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ALE_OVID_HITf, &tmpVal, &value);
            hsa->r8380.ale_ovid_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ACL_ITPID_IDXf, &tmpVal, &value);
            hsa->r8380.acl_itpid_idx = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ACL_ITPID_HITf, &tmpVal, &value);
            hsa->r8380.acl_itpid_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ACL_OTPID_IDXf, &tmpVal, &value);
            hsa->r8380.acl_otpid_idx = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ACL_OTPID_HITf, &tmpVal, &value);
            hsa->r8380.acl_otpid_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ALE_IVIDf, &tmpVal, &value);
            hsa->r8380.ale_ivid = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA2r, MAPLE_ALE_OVIDf, &tmpVal, &value);
            hsa->r8380.ale_ovid = tmpVal;

            /* HSA_DATA3 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA3r, &value);
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_IPV6f, &tmpVal, &value);
            hsa->r8380.ipv6 = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_IPV4f, &tmpVal, &value);
            hsa->r8380.ipv4 = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_PPPOEf, &tmpVal, &value);
            hsa->r8380.pppoe = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_RFC1042f, &tmpVal, &value);
            hsa->r8380.rfc1042 = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_ETAGf, &tmpVal, &value);
            hsa->r8380.etag = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_ACL_HITf, &tmpVal, &value);
            hsa->r8380.acl_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_ACL_IDX_9_0f, &tmpVal, &value);
            hsa->r8380.acl_idx_9_0 = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_ACL_IDX_10f, &tmpVal, &value);
            hsa->r8380.acl_idx_10 = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_MAC_CSTf, &tmpVal, &value);
            hsa->r8380.mac_cst = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_ATK_HITf, &tmpVal, &value);
            hsa->r8380.atk_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_ATK_TYPEf, &tmpVal, &value);
            hsa->r8380.atk_type = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_NEW_SAf, &tmpVal, &value);
            hsa->r8380.new_sa = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_STC_L2_PMVf, &tmpVal, &value);
            hsa->r8380.l2_pmv = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA3r, MAPLE_REASONf, &tmpVal, &value);
            hsa->r8380.reason = tmpVal;

            /* HSA_DATA4 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA4r, &value);
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_CPU_AS_PRIf, &tmpVal, &value);
            hsa->r8380.cpu_as_pri = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_ACL_AS_PRIf, &tmpVal, &value);
            hsa->r8380.acl_as_pri = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_DA_LK_HITf, &tmpVal, &value);
            hsa->r8380.da_lk_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_IS_EAV_Bf, &tmpVal, &value);
            hsa->r8380.is_eav_b = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_IS_EAV_Af, &tmpVal, &value);
            hsa->r8380.is_eav_a = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_DPCf, &tmpVal, &value);
            hsa->r8380.dpc = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_SPf, &tmpVal, &value);
            hsa->r8380.sp = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_L3_ROUTINGf, &tmpVal, &value);
            hsa->r8380.l3_routing = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_L3_ROUTING_IDXf, &tmpVal, &value);
            hsa->r8380.l3_routing_idx = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA4r, MAPLE_ACL_RMKf, &tmpVal, &value);
            hsa->r8380.acl_rmk = tmpVal;

            /* HSA_DATA5 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA5r, &value);
            reg_field_get(0, MAPLE_HSA_DATA5r, MAPLE_RSPAN_RMf, &tmpVal, &value);
            hsa->r8380.rspan_rm = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA5r, MAPLE_MIR_NOR_FWDf, &tmpVal, &value);
            hsa->r8380.mir_nor_fwd = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA5r, MAPLE_MIR4_INFOf, &tmpVal, &value);
            hsa->r8380.mir4_info = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA5r, MAPLE_MIR3_INFOf, &tmpVal, &value);
            hsa->r8380.mir3_info = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA5r, MAPLE_MIR2_INFOf, &tmpVal, &value);
            hsa->r8380.mir2_info = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA5r, MAPLE_MIR1_INFOf, &tmpVal, &value);
            hsa->r8380.mir1_info = tmpVal;

            /* HSA_DATA6 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA6r, &value);
            reg_field_get(0, MAPLE_HSA_DATA6r, MAPLE_INBW_PKTLENf, &tmpVal, &value);
            hsa->r8380.inbw_pktlen = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA6r, MAPLE_PKTLENf, &tmpVal, &value);
            hsa->r8380.pktlen = tmpVal;


            /* HSA_DATA7 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA7r, &value);  /* FIX ME */
            reg_field_get(0, MAPLE_HSA_DATA7r, MAPLE_CTAG_HITf, &tmpVal, &value);
            hsa->r8380.ale_ctag_hit = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA7r, MAPLE_RXQIDf, &tmpVal, &value);
            hsa->r8380.rxqid = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA7r, MAPLE_TXQID_CPUf, &tmpVal, &value);
            hsa->r8380.txqid_cpu = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA7r, MAPLE_TXQID_NRMf, &tmpVal, &value);
            hsa->r8380.txqid_nrm = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA7r, MAPLE_DP_BF_RXQf, &tmpVal, &value);
            hsa->r8380.dp_bf_rxq = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA7r, MAPLE_LST_DSCf, &tmpVal, &value);
            hsa->r8380.lst_dsc = tmpVal;
            reg_field_get(0, MAPLE_HSA_DATA7r, MAPLE_FST_DSCf, &tmpVal, &value);
            hsa->r8380.fst_dsc = tmpVal;

            /* HSA_DATA8 */
            value = 0;
            reg_read(0, MAPLE_HSA_DATA8r, &value);
            reg_field_get(0, MAPLE_HSA_DATA8r, MAPLE_DPMf, &tmpVal, &value);
            hsa->r8380.dpm = tmpVal;
        }
#endif
            break;

        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of _rtk_getHsa */

/* Function Name:
 *      hal_dumpHsa
 * Description:
 *      Dump hsa paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_dumpHsa(uint32 unit)
{
    uint32  i;
    hsa_param_t  hsa;
    hal_control_t *pInfo;

    if(RT_ERR_OK != _rtk_getHsa(unit, &hsa))
    {
        return RT_ERR_FAILED;
    }

    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8389M_CHIP_ID:
        case RTL8389L_CHIP_ID:
        case RTL8329M_CHIP_ID:
        case RTL8377M_CHIP_ID:
        {
            osal_printf("\n====================================HSA========================================\n");
            osal_printf("hsa_busy:0x%x\t\t", hsa.r8389.hsa_busy);
            osal_printf("newsvid:0x%x\t", hsa.r8389.newsvid);
            osal_printf("newvid:0x%x\t", hsa.r8389.newvid);
            osal_printf("cputagif:0x%x\n", hsa.r8389.cputagif);
            osal_printf("dpcnt:0x%x\t\t", (hsa.r8389.dpcnt_4 << 4) | hsa.r8389.dpcnt_3_0);
            osal_printf("rvid:0x%x\t", hsa.r8389.rvid);
            osal_printf("reason:0x%x\t", hsa.r8389.reason);
            osal_printf("intpri:0x%x\n", hsa.r8389.intpri);
            osal_printf("dpmask:0x%08x\t", hsa.r8389.dpmask);
            osal_printf("mir1dpa:0x%x\t", hsa.r8389.mir1dpa);
            osal_printf("mir0dpa:0x%x\t", hsa.r8389.mir0dpa);
            osal_printf("cpuasdprmk:0x%x\n", hsa.r8389.cpuasdprmk);
            osal_printf("ipv6:0x%x\t\t", hsa.r8389.ipv6);
            osal_printf("ipv4:0x%x\t", hsa.r8389.ipv4);
            osal_printf("pppoe:0x%x\t", hsa.r8389.pppoe);
            osal_printf("stagif:0x%x\n", hsa.r8389.stagif);
            osal_printf("ctagif:0x%x\t\t", hsa.r8389.ctagif);
            osal_printf("frametype:0x%x\t", hsa.r8389.frametype);
            osal_printf("pktlen:0x%x\t", hsa.r8389.pktlen);
            osal_printf("l4csok:%d\t\n", hsa.r8389.l4csok);
            osal_printf("l3csok:0x%x\t\t", hsa.r8389.l3csok);
            osal_printf("endpage:0x%x\t", hsa.r8389.endpage);
            osal_printf("startpage:0x%x\t", hsa.r8389.startpage);
            osal_printf("startbank:0x%x\n", hsa.r8389.startbank);
            osal_printf("spa:0x%x\n", hsa.r8389.spa);
            osal_printf("===============================================================================\n");
        }
            break;

        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
        {
            osal_printf("\n===========================HSA===============================\n");

            osal_printf("page_cnt : 0x%x\t\t", hsa.r8390.page_cnt);
            osal_printf("acl_mpls_hit : 0x%x\t", hsa.r8390.acl_mpls_hit);
            osal_printf("acl_mpls_act : 0x%x\n", hsa.r8390.acl_mpls_act);

            osal_printf("mpls_index : 0x%x\t", hsa.r8390.mpls_index);
            osal_printf("astagsts : 0x%x\t\t", hsa.r8390.astagsts);
            osal_printf("ctag_dm : 0x%x\n", hsa.r8390.ctag_dm);

            osal_printf("ctag_dgpkt : 0x%x\t", hsa.r8390.ctag_dgpkt);
            osal_printf("tx_ptp_log : 0x%x\t", hsa.r8390.tx_ptp_log);
            osal_printf("tx_ptp_offload : 0x%x\n", hsa.r8390.tx_ptp_offload);

            osal_printf("new_sa : 0x%x\t\t", hsa.r8390.new_sa);
            osal_printf("mac_cst : 0x%x\t\t", hsa.r8390.mac_cst);
            osal_printf("l2_pmv : 0x%x\n", hsa.r8390.l2_pmv);

            osal_printf("atk_type : 0x%x\t\t", hsa.r8390.atk_type);
            osal_printf("dm_rxidx : 0x%x\t\t", hsa.r8390.dm_rxidx);
            osal_printf("acl_hit : 0x%x\n", hsa.r8390.acl_hit);

            osal_printf("acl_id : 0x%x\t\t", hsa.r8390.acl_id);
            osal_printf("eav_class_b : 0x%x\t", hsa.r8390.eav_class_b);
            osal_printf("eav_class_a : 0x%x\n", hsa.r8390.eav_class_a);

            osal_printf("reason : 0x%x\t\t", hsa.r8390.reason);
            osal_printf("acl_tos : 0x%x\t\t", hsa.r8390.acl_tos);
            osal_printf("new_tos : 0x%x\n", hsa.r8390.new_tos);

            osal_printf("internal_pri : 0x%x\t", hsa.r8390.internal_pri);
            osal_printf("opri_act : 0x%x\t\t", hsa.r8390.opri_act);
            osal_printf("ipri_act : 0x%x\n", hsa.r8390.ipri_act);

            osal_printf("c2sc_pri : 0x%x\t\t", hsa.r8390.c2sc_pri);
            osal_printf("eacl_itag : 0x%x\t\t", hsa.r8390.eacl_itag);
            osal_printf("eacl_otag : 0x%x\n", hsa.r8390.eacl_otag);

            osal_printf("c2sc_itag : 0x%x\t\t", hsa.r8390.c2sc_itag);
            osal_printf("c2sc_otag : 0x%x\t\t", hsa.r8390.c2sc_otag);
            osal_printf("itag_status : 0x%x\n", hsa.r8390.itag_status);

            osal_printf("otag_status : 0x%x\t", hsa.r8390.otag_status);
            osal_printf("eacl_ivid : 0x%x\t\t", hsa.r8390.eacl_ivid);
            osal_printf("c2sc_ivid : 0x%x\n", hsa.r8390.c2sc_ivid);

            osal_printf("eacl_otpid : 0x%x\t", hsa.r8390.eacl_otpid);
            osal_printf("otpid_idx : 0x%x\t\t", hsa.r8390.otpid_idx);
            osal_printf("eacl_itpid : 0x%x\n", hsa.r8390.eacl_itpid);

            osal_printf("c2sc_itpid : 0x%x\t", hsa.r8390.c2sc_itpid);
            osal_printf("itpid_idx : 0x%x\t\t", hsa.r8390.itpid_idx);
            osal_printf("new_otag : 0x%x\n", hsa.r8390.new_otag);

            osal_printf("new_itag : 0x%x\t\t", hsa.r8390.new_itag);
            osal_printf("ale_ovid : 0x%x\t\t", hsa.r8390.ale_ovid);
            osal_printf("l3_route : 0x%x\n", hsa.r8390.l3_route);

            osal_printf("da_pid : 0x%x\t\t", hsa.r8390.da_pid);
            osal_printf("normal_fwd : 0x%x\t", hsa.r8390.normal_fwd);
            osal_printf("swap_mac : 0x%x\n", hsa.r8390.swap_mac);

            osal_printf("cpu_fwd : 0x%x\t\t", hsa.r8390.cpu_fwd);
            osal_printf("sflow_info : 0x%x\t", hsa.r8390.sflow_info);
            osal_printf("mirror_1_info : 0x%x\n", hsa.r8390.mirror_1_info);

            osal_printf("mirror_2_info : 0x%x\t", hsa.r8390.mirror_2_info);
            osal_printf("mirror_3_info : 0x%x\t", hsa.r8390.mirror_3_info);
            osal_printf("mirror_4_info : 0x%x\n", hsa.r8390.mirror_4_info);


            for (i = 0; i < 53; i++)
            {
                if (0 != ((i+1) % 3))
                    osal_printf("qid[%2d] : 0x%x\t\t", i, hsa.r8390.qid[i].qid);
                else
                    osal_printf("qid[%2d] : 0x%x\n", i, hsa.r8390.qid[i].qid);
            }
            osal_printf("\n");  /* end of QID */

            osal_printf("color : 0x%x\t\t", hsa.r8390.color);
            osal_printf("rvid_sel : 0x%x\t\t", hsa.r8390.rvid_sel);
            osal_printf("rvid : 0x%x\n", hsa.r8390.rvid);
            osal_printf("dpc : 0x%x\t\t", hsa.r8390.dpc);
            osal_printf("dpm[52:32] : 0x%x\t", hsa.r8390.dpm.bits[1]);
            osal_printf("dpm[31: 0] : 0x%x\n", hsa.r8390.dpm.bits[0]);
            osal_printf("endsc : 0x%x\t\t", hsa.r8390.endsc);
            osal_printf("bgdsc : 0x%x\t\t", hsa.r8390.bgdsc);
            osal_printf("stpid_idx : 0x%x\n", hsa.r8390.stpid_idx);

            osal_printf("ctpid_idx : 0x%x\t\t", hsa.r8390.ctpid_idx);
            osal_printf("org_otag : 0x%x\t\t", hsa.r8390.org_otag);
            osal_printf("org_itag : 0x%x\n", hsa.r8390.org_itag);

            osal_printf("ompls_if : 0x%x\t\t", hsa.r8390.ompls_if);
            osal_printf("impls_if : 0x%x\t\t", hsa.r8390.impls_if);
            osal_printf("org_etag_if : 0x%x\n", hsa.r8390.org_etag_if);

            osal_printf("org_cputag_if : 0x%x\t", hsa.r8390.org_cputag_if);
            osal_printf("org_otag_if : 0x%x\t", hsa.r8390.org_otag_if);
            osal_printf("org_itag_if : 0x%x\n", hsa.r8390.org_itag_if);

            osal_printf("ipv6 : 0x%x\t\t", hsa.r8390.ipv6);
            osal_printf("llc_other : 0x%x\t\t", hsa.r8390.llc_other);
            osal_printf("pppoe_if : 0x%x\n", hsa.r8390.pppoe_if);
            osal_printf("rfc1042 : 0x%x\t\t", hsa.r8390.rfc1042);
            osal_printf("spa : 0x%x\t\t", hsa.r8390.spa);
            osal_printf("errpkt : 0x%x\n", hsa.r8390.errpkt);
            osal_printf("ipv4 : 0x%x\t\t", hsa.r8390.ipv4);
            osal_printf("pktlen : 0x%x\n", hsa.r8390.pktlen);
            osal_printf("=============================================================\n");
        }
            break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
        {
            osal_printf("\n===============================================  HSA  ==================================================\n");

            osal_printf("%-13s = 0x%-9x", "cpu_tag_if", hsa.r8380.cpu_tag_if);
            osal_printf("%-13s = 0x%-9x\n", "astagsts", hsa.r8380.astagsts);

            osal_printf("---------------------------------------------- ALE DECISION --------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "ale_otag_hit", hsa.r8380.ale_otag_hit);
            osal_printf("%-13s = 0x%-9x", "ale_otag_sts", hsa.r8380.ale_otag_sts);
            osal_printf("%-13s = 0x%-9x", "ale_ovid_hit", hsa.r8380.ale_ovid_hit);
            osal_printf("%-13s = 0x%-9x\n", "ale_ovid", hsa.r8380.ale_ovid);

            osal_printf("%-13s = 0x%-9x", "ale_itag_hit", hsa.r8380.ale_itag_hit);
            osal_printf("%-13s = 0x%-9x", "ale_itag_sts", hsa.r8380.ale_itag_sts);
            osal_printf("%-13s = 0x%-9x", "ale_ivid_hit", hsa.r8380.ale_ivid_hit);
            osal_printf("%-13s = 0x%-9x\n", "ale_ivid", hsa.r8380.ale_ivid);

            osal_printf("%-13s = 0x%-9x\n", "ale_ctag_hit", hsa.r8380.ale_ctag_hit);

            osal_printf("---------------------------------------------- ACL DECISION --------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "acl_otpid_hit", hsa.r8380.acl_otpid_hit);
            osal_printf("%-13s = 0x%-9x", "acl_otpid_idx", hsa.r8380.acl_otpid_idx);
            osal_printf("%-13s = 0x%-9x", "acl_itpid_hit", hsa.r8380.acl_itpid_hit);
            osal_printf("%-13s = 0x%-9x\n", "acl_itpid_idx", hsa.r8380.acl_itpid_idx);
            osal_printf("%-13s = 0x%-9x", "acl_rmk", hsa.r8380.acl_rmk);
            osal_printf("%-13s = 0x%-9x", "l3_routing", hsa.r8380.l3_routing);
            osal_printf("%-13s = 0x%-9x\n", "l3_routing_idx", hsa.r8380.l3_routing_idx);

            osal_printf("---------------------------------------------- ORG TAG INFO --------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "org_otag_if", hsa.r8380.org_otag_if);
            osal_printf("%-13s = 0x%-9x", "org_ovid", hsa.r8380.org_ovid);
            osal_printf("%-13s = 0x%-9x", "org_opri", hsa.r8380.org_opri);
            osal_printf("%-13s = 0x%-9x\n", "org_ocfi", hsa.r8380.org_ocfi);

            osal_printf("%-13s = 0x%-9x", "org_itag_if", hsa.r8380.org_itag_if);
            osal_printf("%-13s = 0x%-9x", "org_ivid", hsa.r8380.org_ivid);
            osal_printf("%-13s = 0x%-9x", "org_ipri", hsa.r8380.org_ipri);
            osal_printf("%-13s = 0x%-9x\n", "org_icfi", hsa.r8380.org_icfi);

            osal_printf("%-13s = 0x%-9x", "org_otpid_idx", hsa.r8380.org_otpid_idx);
            osal_printf("%-13s = 0x%-9x\n", "org_itpid_idx", hsa.r8380.org_itpid_idx);

            osal_printf("---------------------------------------------- PACKET INFO ---------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "rfc1042", hsa.r8380.rfc1042);
            osal_printf("%-13s = 0x%-9x", "pppoe", hsa.r8380.pppoe);
            osal_printf("%-13s = 0x%-9x", "ipv4", hsa.r8380.ipv4);
            osal_printf("%-13s = 0x%-9x\n", "ipv6", hsa.r8380.ipv6);

            osal_printf("%-13s = 0x%-9x", "etag", hsa.r8380.etag);
            osal_printf("%-13s = 0x%-9x", "sp", hsa.r8380.sp);
            osal_printf("%-13s = 0x%-9x", "dpm", hsa.r8380.dpm);
            osal_printf("%-13s = 0x%-9x\n", "pktlen", hsa.r8380.pktlen);

            osal_printf("%-13s = 0x%-9x", "dp_bf_rxq", hsa.r8380.dp_bf_rxq);
            osal_printf("%-13s = 0x%-9x", "reason", hsa.r8380.reason);
            osal_printf("%-13s = 0x%-9x", "fwd_base", hsa.r8380.fwd_base);
            osal_printf("%-13s = 0x%-9x\n", "dpc", hsa.r8380.dpc);

            osal_printf("--------------------------------------------- MIRROR INFO ----------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "mir1_info", hsa.r8380.mir1_info);
            osal_printf("%-13s = 0x%-9x", "mir2_info", hsa.r8380.mir2_info);
            osal_printf("%-13s = 0x%-9x", "mir3_info", hsa.r8380.mir3_info);
            osal_printf("%-13s = 0x%-9x\n", "mir4_info", hsa.r8380.mir4_info);
            osal_printf("%-13s = 0x%-9x", "rspan_rm", hsa.r8380.rspan_rm);
            osal_printf("%-13s = 0x%-9x\n", "mir_nor_fwd", hsa.r8380.mir_nor_fwd);

            osal_printf("-------------------------------------------- FOR RX CPU TAG --------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "fwd_vid", hsa.r8380.fwd_vid);
            osal_printf("%-13s = 0x%-9x", "ale_int_pri", hsa.r8380.ale_int_pri);
            osal_printf("%-13s = 0x%-9x", "l2_pmv", hsa.r8380.l2_pmv);
            osal_printf("%-13s = 0x%-9x\n", "new_sa", hsa.r8380.new_sa);
            osal_printf("%-13s = 0x%-9x", "atk_type", hsa.r8380.atk_type);
            osal_printf("%-13s = 0x%-9x", "atk_hit", hsa.r8380.atk_hit);
            osal_printf("%-13s = 0x%-9x\n", "mac_cst", hsa.r8380.mac_cst);
            osal_printf("%-13s = 0x%-9x", "acl_hit", hsa.r8380.acl_hit);
            osal_printf("%-13s = 0x%-11x\n", "acl_idx", (hsa.r8380.acl_idx_10 << 10) | hsa.r8380.acl_idx_9_0);

            osal_printf("------------------------------------------------- MISC -------------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "is_eav_a", hsa.r8380.is_eav_a);
            osal_printf("%-13s = 0x%-9x\n", "is_eav_b", hsa.r8380.is_eav_b);
            osal_printf("%-13s = 0x%-9x", "da_lk_hit", hsa.r8380.da_lk_hit);
            osal_printf("%-13s = 0x%-9x", "acl_as_pri", hsa.r8380.acl_as_pri);
            osal_printf("%-13s = 0x%-9x\n", "cpu_as_pri", hsa.r8380.cpu_as_pri);

            osal_printf("---------------------------------------------- TX/RX QUEUE ----------------------------------------------\n");
            osal_printf("%-13s = 0x%-9x", "fst_dsc", hsa.r8380.fst_dsc);
            osal_printf("%-13s = 0x%-9x", "lst_dsc", hsa.r8380.lst_dsc);
            osal_printf("%-13s = 0x%-9x\n", "inbw_pktlen", hsa.r8380.inbw_pktlen);
            osal_printf("%-13s = 0x%-9x", "txqid_nrm", hsa.r8380.txqid_nrm);
            osal_printf("%-13s = 0x%-9x", "txqid_cpu", hsa.r8380.txqid_cpu);
            osal_printf("%-13s = 0x%-9x\n", "rxqid", hsa.r8380.rxqid);

            osal_printf("\n========================================================================================================\n");
        }
            break;

        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of hal_dumpHsa */
#endif

#if defined(CONFIG_SDK_RTL8390)
int32 _hal_getHsm(uint32 unit, hsm_param_t *hsm)
{
    uint32 val = 0, tmpVal = 0;
    hal_control_t *pInfo;

    if(NULL == hsm)
    {
        return RT_ERR_FAILED;
    }

    memset(hsm, 0, sizeof(hsm_param_t));
    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
        {
            /* HSM PRE */
            /* HSM_PRE_DATA0 */
            reg_read(0, CYPRESS_HSM_PRE_DATA0r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_PPPOEf, &tmpVal, &val);
            hsm->r8390.pre_data.pppoe_ef = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_CMACf, &tmpVal, &val);
            hsm->r8390.pre_data.gmac = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_GMACf, &tmpVal, &val);
            hsm->r8390.pre_data.cmac = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_IPV4_HDR_ERRf, &tmpVal, &val);
            hsm->r8390.pre_data.ipv4_hdr_err = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_IPV4_TTL_ZEROf, &tmpVal, &val);
            hsm->r8390.pre_data.ipv4_ttl_zero = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_IPV4_OPTf, &tmpVal, &val);
            hsm->r8390.pre_data.ipv4_opt = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_IPV6_HDR_ERRf, &tmpVal, &val);
            hsm->r8390.pre_data.ipv4_hdr_err = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_IPV6_HL_ZEROf, &tmpVal, &val);
            hsm->r8390.pre_data.ipv6_hl_zero = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_IPV6_HOPf, &tmpVal, &val);
            hsm->r8390.pre_data.ipv6_hop = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_OAM_ACTf, &tmpVal, &val);
            hsm->r8390.pre_data.oam_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_STPf, &tmpVal, &val);
            hsm->r8390.pre_data.stp = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_VLAN_ZEROf, &tmpVal, &val);
            hsm->r8390.pre_data.vlan_zero = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_VLAN_SPMf, &tmpVal, &val);
            hsm->r8390.pre_data.vlan_spm = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_UNKEYf, &tmpVal, &val);
            hsm->r8390.pre_data.unkey = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_MCKEYf, &tmpVal, &val);
            hsm->r8390.pre_data.mckey = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_VLAN_PROFILEf, &tmpVal, &val);
            hsm->r8390.pre_data.vlan_profile = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_FIDf, &tmpVal, &val);
            hsm->r8390.pre_data.fid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_METER_DROPf, &tmpVal, &val);
            hsm->r8390.pre_data.meter_drop = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA0r, CYPRESS_CFM_ACTf, &tmpVal, &val);
            hsm->r8390.pre_data.cfm_act = tmpVal;

            /* HSM_PRE_DATA1 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA1r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA1r, CYPRESS_CFM_IDXf, &tmpVal, &val);
            hsm->r8390.pre_data.cfm_idx = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA1r, CYPRESS_CFI_ACTf, &tmpVal, &val);
            hsm->r8390.pre_data.cfi_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA1r, CYPRESS_TAG_ACTf, &tmpVal, &val);
            hsm->r8390.pre_data.tag_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA1r, CYPRESS_NIVIDf, &tmpVal, &val);
            hsm->r8390.pre_data.nivid = tmpVal;

            /* HSM_PRE_DATA2 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA2r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA2r, CYPRESS_ACL_BYPASSf, &tmpVal, &val);
            hsm->r8390.pre_data.acl_bypass = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA2r, CYPRESS_NOVIDf, &tmpVal, &val);
            hsm->r8390.pre_data.novid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA2r, CYPRESS_RVIDf, &tmpVal, &val);
            hsm->r8390.pre_data.rvid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA2r, CYPRESS_ACL_MEETf, &tmpVal, &val);
            hsm->r8390.pre_data.acl_meet = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA2r, CYPRESS_ACLC2SCf, &tmpVal, &val);
            hsm->r8390.pre_data.aclc2sc = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA2r, CYPRESS_ACL_HITf, &tmpVal, &val);
            hsm->r8390.pre_data.acl_hit = tmpVal;

            /* HSM_PRE_DATA3 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA3r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA3r, CYPRESS_ACL_IDXf, &tmpVal, &val);
            hsm->r8390.pre_data.acl_idx = tmpVal;

            /* HSM_PRE_DATA4 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA4r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA4r, CYPRESS_ACL_DATAf, &tmpVal, &val);
            hsm->r8390.pre_data.acl_data = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA4r, CYPRESS_ORG_IVIF_IFf, &tmpVal, &val);
            hsm->r8390.pre_data.org_ivif_if = tmpVal;

            /* HSM_PRE_DATA5 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA5r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_ORG_IVIDf, &tmpVal, &val);
            hsm->r8390.pre_data.org_ivid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_SPE_TRAP_RSNf, &tmpVal, &val);
            hsm->r8390.pre_data.spe_trap_rsn = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_SPE_TRAP_ACTf, &tmpVal, &val);
            hsm->r8390.pre_data.spe_trap_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_EAV_CLASSAf, &tmpVal, &val);
            hsm->r8390.pre_data.eav_classa = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_EAV_CLASSBf, &tmpVal, &val);
            hsm->r8390.pre_data.eav_classb = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_HWATT_RSNf, &tmpVal, &val);
            hsm->r8390.pre_data.hwatt_rsn = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_HWATT_ACTf, &tmpVal, &val);
            hsm->r8390.pre_data.hwatt_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_RMA_PKTf, &tmpVal, &val);
            hsm->r8390.pre_data.rma_pkt= tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_RMA_ACTf, &tmpVal, &val);
            hsm->r8390.pre_data.rma_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_RMA_LEARNf, &tmpVal, &val);
            hsm->r8390.pre_data.rma_learn = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_RMA_BYPASSf, &tmpVal, &val);
            hsm->r8390.pre_data.rma_bypass = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_C2SC_HIT_MODEf, &tmpVal, &val);
            hsm->r8390.pre_data.c2sc_hit_mode = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA5r, CYPRESS_C2SC_HITf, &tmpVal, &val);
            hsm->r8390.pre_data.c2sc_hit = tmpVal;

            /* HSM_PRE_DATA6 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA6r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA6r, CYPRESS_C2SC_DATAf, &tmpVal, &val);
            hsm->r8390.pre_data.c2sc_data = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA6r, CYPRESS_INTERNAL_PRIOf, &tmpVal, &val);
            hsm->r8390.pre_data.internal_prio = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA6r, CYPRESS_DP_PREf, &tmpVal, &val);
            hsm->r8390.pre_data.dp_pre = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA6r, CYPRESS_PROTOCOL_STORMf, &tmpVal, &val);
            hsm->r8390.pre_data.protocol_storm = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA6r, CYPRESS_IBW_PASSf, &tmpVal, &val);
            hsm->r8390.pre_data.ibw_pass = tmpVal;

            /* HSM_PRE_DATA7 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA7r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA7r, CYPRESS_SLIDf, &tmpVal, &val);
            hsm->r8390.pre_data.slid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA7r, CYPRESS_TRUNK_GRPf, &tmpVal, &val);
            hsm->r8390.pre_data.trunk_grp = tmpVal;

            /* HSM_PRE_DATA8 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA8r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA8r, CYPRESS_SIPf, &tmpVal, &val);
            hsm->r8390.pre_data.sip = tmpVal;

            /* HSM_PRE_DATA9 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA9r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA9r, CYPRESS_DIPf, &tmpVal, &val);
            hsm->r8390.pre_data.dip = tmpVal;

            /* HSM_PRE_DATA10 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA10r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA10r, CYPRESS_IPV6f, &tmpVal, &val);
            hsm->r8390.pre_data.ipv6 = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA10r, CYPRESS_IPV4f, &tmpVal, &val);
            hsm->r8390.pre_data.ipv4 = tmpVal;

            /* HSM_PRE_DATA11 */
            val = 0;
            reg_field_read(0, CYPRESS_HSM_PRE_DATA11r, CYPRESS_SMAC_47_32f, &val);
            hsm->r8390.pre_data.smac[0] = (uint8)(val >> 8) & 0xff;
            hsm->r8390.pre_data.smac[1] = (uint8)(val >> 0) & 0xff;

            /* HSM_PRE_DATA12 */
            val = 0;
            reg_field_read(0, CYPRESS_HSM_PRE_DATA12r, CYPRESS_SMAC_31_0f, &val);
            hsm->r8390.pre_data.smac[2] = (uint8)(val >> 24) & 0xff;
            hsm->r8390.pre_data.smac[3] = (uint8)(val >> 16) & 0xff;
            hsm->r8390.pre_data.smac[4] = (uint8)(val >> 8) & 0xff;
            hsm->r8390.pre_data.smac[5] = (uint8)(val >> 0) & 0xff;

            /* HSM_PRE_DATA13 */
            val = 0;
            reg_field_read(0, CYPRESS_HSM_PRE_DATA13r, CYPRESS_DMAC_47_32f, &val);
            hsm->r8390.pre_data.dmac[0] = (val >> 8) & 0xff;
            hsm->r8390.pre_data.dmac[1] = (val >> 0) & 0xff;

            /* HSM_PRE_DATA14 */
            val = 0;
            reg_field_read(0, CYPRESS_HSM_PRE_DATA14r, CYPRESS_DMAC_31_0f, &val);
            hsm->r8390.pre_data.dmac[2] = (val >> 24) & 0xff;
            hsm->r8390.pre_data.dmac[3] = (val >> 16) & 0xff;
            hsm->r8390.pre_data.dmac[4] = (val >> 8) & 0xff;
            hsm->r8390.pre_data.dmac[5] = (val >> 0) & 0xff;

            /* HSM_PRE_DATA15 */
            val = 0;
            reg_read(0, CYPRESS_HSM_PRE_DATA15r, &val);
            reg_field_get(0, CYPRESS_HSM_PRE_DATA15r, CYPRESS_CTAG_LNf, &tmpVal, &val);
            hsm->r8390.pre_data.ctag_ln = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA15r, CYPRESS_CTAG_IFf, &tmpVal, &val);
            hsm->r8390.pre_data.ctag_if = tmpVal;
            reg_field_get(0, CYPRESS_HSM_PRE_DATA15r, CYPRESS_PKTLENf, &tmpVal, &val);
            hsm->r8390.pre_data.pktlen = tmpVal;

            /* HSM LU */
            /* HSM_LU_DATA0 */
            reg_read(0, CYPRESS_HSM_LU_DATA0r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA0r, CYPRESS_ACL_BYPASSf, &tmpVal, &val);
            hsm->r8390.lu_data.acl_bypass = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA0r, CYPRESS_DPORT_MOVEf, &tmpVal, &val);
            hsm->r8390.lu_data.dpmove_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA0r, CYPRESS_SPORT_FILTER_ENf, &tmpVal, &val);
            hsm->r8390.lu_data.sport_filter_en = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA0r, CYPRESS_GMAC_ERRf, &tmpVal, &val);
            hsm->r8390.lu_data.gmac_err = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA0r, CYPRESS_MC_SAf, &tmpVal, &val);
            hsm->r8390.lu_data.mc_sa = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA0r, CYPRESS_VLAN_SPFILTERf, &tmpVal, &val);
            hsm->r8390.lu_data.vlan_spfilter = tmpVal;

            /* HSM_LU_DATA1 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA1r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA1r, CYPRESS_DPM_52_32f, &tmpVal, &val);
            hsm->r8390.lu_data.dpm.bits[1] = tmpVal;

            /* HSM_LU_DATA2 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA2r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA2r, CYPRESS_DPM_31_0f, &tmpVal, &val);
            hsm->r8390.lu_data.dpm.bits[0] = tmpVal;

            /* HSM_LU_DATA3 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA3r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA3r, CYPRESS_LOOKUP_TYPEf, &tmpVal, &val);
            hsm->r8390.lu_data.lookup_type = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA3r, CYPRESS_FWD_TYPEf, &tmpVal, &val);
            hsm->r8390.lu_data.fwd_type = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA3r, CYPRESS_NEWSA_ACf, &tmpVal, &val);
            hsm->r8390.lu_data.newsa_ac = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA3r, CYPRESS_PMOVE_ACTf, &tmpVal, &val);
            hsm->r8390.lu_data.pmove_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA3r, CYPRESS_MACLIMIT_ACTf, &tmpVal, &val);
            hsm->r8390.lu_data.maclimit_act = tmpVal;

            /* HSM_LU_DATA4 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA4r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA4r, CYPRESS_ROUTE_DATAf, &tmpVal, &val);
            hsm->r8390.lu_data.route_data = tmpVal;

            /* HSM_LU_DATA5 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA5r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA5r, CYPRESS_DA_HITf, &tmpVal, &val);
            hsm->r8390.lu_data.da_hit = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA5r, CYPRESS_DA_DATAf, &tmpVal, &val);
            hsm->r8390.lu_data.da_data = tmpVal;

            /* HSM_LU_DATA6 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA6r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA6r, CYPRESS_SA_HITf, &tmpVal, &val);
            hsm->r8390.lu_data.sa_hit = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA6r, CYPRESS_SA_DATAf, &tmpVal, &val);
            hsm->r8390.lu_data.sa_data = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA6r, CYPRESS_CMACf, &tmpVal, &val);
            hsm->r8390.lu_data.cmac = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA6r, CYPRESS_GMACf, &tmpVal, &val);
            hsm->r8390.lu_data.gmac = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA6r, CYPRESS_IPV4_HDR_ERRf, &tmpVal, &val);
            hsm->r8390.lu_data.ipv4_hdr_err = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA6r, CYPRESS_IPV4_OPTf, &tmpVal, &val);
            hsm->r8390.lu_data.ipv4_opt = tmpVal;

            /* HSM_LU_DATA7 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA7r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_IPV4_TTL_ZEROf, &tmpVal, &val);
            hsm->r8390.lu_data.ipv4_ttl_zero = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_IPV6_HDR_ERRf, &tmpVal, &val);
            hsm->r8390.lu_data.ipv6_hdr_err = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_IPV6_HL_ZEROf, &tmpVal, &val);
            hsm->r8390.lu_data.ipv6_hl_zero = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_IPV6_HOPf, &tmpVal, &val);
            hsm->r8390.lu_data.ipv6_hop = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_OAM_ACTf, &tmpVal, &val);
            hsm->r8390.lu_data.oam_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_STPf, &tmpVal, &val);
            hsm->r8390.lu_data.stp = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_VLAN_ZEROf, &tmpVal, &val);
            hsm->r8390.lu_data.vlan_zero = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_VLAN_SPMf, &tmpVal, &val);
            hsm->r8390.lu_data.vlan_spm = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_FIDf, &tmpVal, &val);
            hsm->r8390.lu_data.fid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_METER_DROPf, &tmpVal, &val);
            hsm->r8390.lu_data.meter_drop = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_CFM_ACTf, &tmpVal, &val);
            hsm->r8390.lu_data.cfm_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_CFM_IDXf, &tmpVal, &val);
            hsm->r8390.lu_data.cfm_idx = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_CFI_ACTf, &tmpVal, &val);
            hsm->r8390.lu_data.cfi_act = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA7r, CYPRESS_TAG_ACTf, &tmpVal, &val);
            hsm->r8390.lu_data.tag_act = tmpVal;

            /* HSM_LU_DATA8 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA8r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA8r, CYPRESS_NIVIDf, &tmpVal, &val);
            hsm->r8390.lu_data.nivid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA8r, CYPRESS_NOVIDf, &tmpVal, &val);
            hsm->r8390.lu_data.novid = tmpVal;

            /* HSM_LU_DATA9 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA9r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA9r, CYPRESS_RVIDf, &tmpVal, &val);
            hsm->r8390.lu_data.rvid = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA9r, CYPRESS_ACL_MEETf, &tmpVal, &val);
            hsm->r8390.lu_data.acl_meet = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA9r, CYPRESS_ACLC2SCf, &tmpVal, &val);
            hsm->r8390.lu_data.aclc2sc = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA9r, CYPRESS_ACL_HITf, &tmpVal, &val);
            hsm->r8390.lu_data.acl_hit = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA9r, CYPRESS_ACL_IDXf, &tmpVal, &val);
            hsm->r8390.lu_data.acl_idx = tmpVal;

            /* HSM_LU_DATA10 */
            val = 0;
            reg_read(0, CYPRESS_HSM_LU_DATA10r, &val);
            reg_field_get(0, CYPRESS_HSM_LU_DATA10r, CYPRESS_ACL_DATAf, &tmpVal, &val);
            hsm->r8390.lu_data.acl_data = tmpVal;
            reg_field_get(0, CYPRESS_HSM_LU_DATA10r, CYPRESS_SPE_TRAP_RSNf, &tmpVal, &val);
            hsm->r8390.lu_data.spe_trap_rsn = tmpVal;

            /* HSM_LU_DATA11 */
            val = 0;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_RMA_BYP_VLANf, &val);
            hsm->r8390.lu_data.rma_byp_vlan = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_SPE_TRAP_ACTf, &val);
            hsm->r8390.lu_data.spe_trap_act = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_EAV_CLASSAf, &val);
            hsm->r8390.lu_data.eav_classa = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_EAV_CLASSBf, &val);
            hsm->r8390.lu_data.eav_classb = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_HWATT_RSNf, &val);
            hsm->r8390.lu_data.hwatt_rsn = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_HWATT_ACTf, &val);
            hsm->r8390.lu_data.hwatt_act = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_RMA_PKTf, &val);
            hsm->r8390.lu_data.rma_pkt = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_RMA_ACTf, &val);
            hsm->r8390.lu_data.rma_act = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_RMA_BYPASSf, &val);
            hsm->r8390.lu_data.rma_bypass = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_C2SC_HIT_MODEf, &val);
            hsm->r8390.lu_data.c2sc_hit_mode = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA11r, CYPRESS_C2SC_HITf, &val);
            hsm->r8390.lu_data.c2sc_hit = tmpVal;

            /* HSM_LU_DATA12 */
            val = 0;
            reg_field_read(0, CYPRESS_HSM_LU_DATA12r, CYPRESS_C2SC_DATAf, &val);
            hsm->r8390.lu_data.c2sc_data = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA12r, CYPRESS_INTERNAL_PRIOf, &val);
            hsm->r8390.lu_data.internal_prio = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA12r, CYPRESS_DP_PREf, &val);
            hsm->r8390.lu_data.dp_pre = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA12r, CYPRESS_PROTOCOL_STORMf, &val);
            hsm->r8390.lu_data.protocol_storm = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA12r, CYPRESS_IBW_PASSf, &val);
            hsm->r8390.lu_data.ibw_pass = tmpVal;

            /* HSM_LU_DATA13 */
            val = 0;
            reg_field_read(0, CYPRESS_HSM_LU_DATA13r, CYPRESS_SLIDf, &val);
            hsm->r8390.lu_data.slid = tmpVal;
            reg_field_read(0, CYPRESS_HSM_LU_DATA13r, CYPRESS_TRUNK_GRPf, &val);
            hsm->r8390.lu_data.trunk_grp = tmpVal;
        }
            break;

        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
} /* end of _hal_getHsm */
#endif

#if defined(CONFIG_SDK_RTL8390)
int32 _hal_dumpHsm_8390(uint32 unit)
{
    hsm_param_t hsm;

    if (RT_ERR_OK != _hal_getHsm(unit, &hsm))
    {
        return RT_ERR_FAILED;
    }

    osal_printf("\n=========================HSM PRE=============================\n");
    osal_printf("PPPOE : 0x%x\t\t", hsm.r8390.pre_data.pppoe_ef);
    osal_printf("CMAC : 0x%x\t\t", hsm.r8390.pre_data.cmac);
    osal_printf("GMAC : 0x%x\n", hsm.r8390.pre_data.gmac);

    osal_printf("IPV4_HDR_ERR : 0x%x\t", hsm.r8390.pre_data.ipv4_hdr_err);
    osal_printf("IPV4_TTL_ZERO : 0x%x\t", hsm.r8390.pre_data.ipv4_ttl_zero);
    osal_printf("IPV4_OPT : 0x%x\n", hsm.r8390.pre_data.ipv4_opt);

    osal_printf("IPV6_HDR_ERR : 0x%x\t", hsm.r8390.pre_data.ipv6_hdr_err);
    osal_printf("IPV6_HL_ZERO : 0x%x\t", hsm.r8390.pre_data.ipv6_hl_zero);
    osal_printf("IPV6_HOP : 0x%x\n", hsm.r8390.pre_data.ipv6_hop);

    osal_printf("OAM_ACT : 0x%x\t\t", hsm.r8390.pre_data.oam_act);
    osal_printf("STP : 0x%x\t\t", hsm.r8390.pre_data.stp);
    osal_printf("VLAN_ZERO : 0x%x\n", hsm.r8390.pre_data.vlan_zero);

    osal_printf("VLAN_SPM : 0x%x\t\t", hsm.r8390.pre_data.vlan_spm);
    osal_printf("UNKEY : 0x%x\t\t", hsm.r8390.pre_data.unkey);
    osal_printf("MCKEY : 0x%x\n", hsm.r8390.pre_data.mckey);

    osal_printf("VLAN_PROFILE : 0x%x\t", hsm.r8390.pre_data.vlan_profile);
    osal_printf("FID : 0x%x\t\t", hsm.r8390.pre_data.fid);
    osal_printf("METER_DROP : 0x%x\n", hsm.r8390.pre_data.meter_drop);

    osal_printf("CFM_ACT : 0x%x\t\t", hsm.r8390.pre_data.cfm_act);
    osal_printf("CFM_IDX : 0x%x\t\t", hsm.r8390.pre_data.cfm_idx);
    osal_printf("CFI_ACT : 0x%x\n", hsm.r8390.pre_data.cfi_act);

    osal_printf("TAG_ACT : 0x%x\t\t", hsm.r8390.pre_data.tag_act);
    osal_printf("NIVID : 0x%x\t\t", hsm.r8390.pre_data.nivid);
    osal_printf("ACL_BYPASS : 0x%x\n", hsm.r8390.pre_data.acl_bypass);

    osal_printf("NOVID : 0x%x\t\t", hsm.r8390.pre_data.novid);
    osal_printf("RVID : 0x%x\t\t", hsm.r8390.pre_data.rvid);
    osal_printf("ACL_MEET : 0x%x\n", hsm.r8390.pre_data.acl_meet);

    osal_printf("ACLC2SC : 0x%x\t\t", hsm.r8390.pre_data.aclc2sc);
    osal_printf("ACL_HIT : 0x%x\t\t", hsm.r8390.pre_data.acl_hit);
    osal_printf("ACL_IDX : 0x%x\n", hsm.r8390.pre_data.acl_idx);

    osal_printf("ACL_DATA : 0x%x\t\t", hsm.r8390.pre_data.acl_data);
    osal_printf("ORG_IVIF_IF : 0x%x\t", hsm.r8390.pre_data.org_ivif_if);
    osal_printf("ORG_IVID : 0x%x\n", hsm.r8390.pre_data.org_ivid);

    osal_printf("SPE_TRAP_RSN : 0x%x\t", hsm.r8390.pre_data.spe_trap_rsn);
    osal_printf("SPE_TRAP_ACT : 0x%x\t", hsm.r8390.pre_data.spe_trap_act);
    osal_printf("EAV_CLASSA : 0x%x\n", hsm.r8390.pre_data.eav_classa);

    osal_printf("EAV_CLASSB : 0x%x\t", hsm.r8390.pre_data.eav_classb);
    osal_printf("HWATT_RSN : 0x%x\t\t", hsm.r8390.pre_data.hwatt_rsn);
    osal_printf("HWATT_ACT : 0x%x\n", hsm.r8390.pre_data.hwatt_act);

    osal_printf("RMA_PKT : 0x%x\t\t", hsm.r8390.pre_data.rma_pkt);
    osal_printf("RMA_ACT : 0x%x\t\t", hsm.r8390.pre_data.rma_act);
    osal_printf("RMA_LEARN : 0x%x\n", hsm.r8390.pre_data.rma_learn);

    osal_printf("RMA_BYPASS : 0x%x\t", hsm.r8390.pre_data.rma_bypass);
    osal_printf("C2SC_HIT_MODE : 0x%x\t", hsm.r8390.pre_data.c2sc_hit_mode);
    osal_printf("C2SC_HIT : 0x%x\n", hsm.r8390.pre_data.c2sc_hit);

    osal_printf("C2SC_DATA : 0x%x\t\t", hsm.r8390.pre_data.c2sc_data);
    osal_printf("INTERNAL_PRIO : 0x%x\t", hsm.r8390.pre_data.internal_prio);
    osal_printf("DP_PRE : 0x%x\n", hsm.r8390.pre_data.dp_pre);

    osal_printf("PROTOCOL_STORM : 0x%x\t", hsm.r8390.pre_data.protocol_storm);
    osal_printf("IBW_PASS : 0x%x\t\t", hsm.r8390.pre_data.ibw_pass);
    osal_printf("SLID : 0x%x\n", hsm.r8390.pre_data.slid);

    osal_printf("TRUNK_GRP : 0x%x\t\t", hsm.r8390.pre_data.trunk_grp);
    osal_printf("SIP : 0x%x\t\t", hsm.r8390.pre_data.sip);
    osal_printf("DIP : 0x%x\n", hsm.r8390.pre_data.dip);

    osal_printf("IPv6 : 0x%x\t\t", hsm.r8390.pre_data.ipv6);
    osal_printf("IPv4 : 0x%x\t\t", hsm.r8390.pre_data.ipv4);
    osal_printf("CTAG_LN : 0x%x\n", hsm.r8390.pre_data.ctag_ln);
    osal_printf("CTAG_IF : 0x%x\t\t", hsm.r8390.pre_data.ctag_if);
    osal_printf("PKTLEN : 0x%x\n", hsm.r8390.pre_data.pktlen);

    osal_printf("SMAC : %02x:%02x:%02x:%02x:%02x:%02x\n", hsm.r8390.pre_data.smac[0], hsm.r8390.pre_data.smac[1], hsm.r8390.pre_data.smac[2], hsm.r8390.pre_data.smac[3], hsm.r8390.pre_data.smac[4], hsm.r8390.pre_data.smac[5]);
    osal_printf("DMAC : %02x:%02x:%02x:%02x:%02x:%02x\n", hsm.r8390.pre_data.dmac[0], hsm.r8390.pre_data.dmac[1], hsm.r8390.pre_data.dmac[2], hsm.r8390.pre_data.dmac[3], hsm.r8390.pre_data.dmac[4], hsm.r8390.pre_data.dmac[5]);
    osal_printf("=============================================================\n");

    osal_printf("\n=========================HSM LU==============================\n");
    osal_printf("ACL_BYPASS : 0x%x\t", hsm.r8390.lu_data.acl_bypass);
    osal_printf("DPORT_MOVE : 0x%x\t", hsm.r8390.lu_data.dpmove_act);
    osal_printf("SPORT_FILTER_EN : 0x%x\n", hsm.r8390.lu_data.sport_filter_en);

    osal_printf("GMAC_ERR : 0x%x\t\t", hsm.r8390.lu_data.gmac_err);
    osal_printf("MC_SA : 0x%x\t\t", hsm.r8390.lu_data.mc_sa);
    osal_printf("VLAN_SPFILTER : 0x%x\n", hsm.r8390.lu_data.vlan_spfilter);

    osal_printf("DPM[52:32] : 0x%x\t", hsm.r8390.lu_data.dpm.bits[1]);
    osal_printf("DPM[31: 0] : 0x%x\t", hsm.r8390.lu_data.dpm.bits[0]);
    osal_printf("LOOKUP_TYPE : 0x%x\n", hsm.r8390.lu_data.lookup_type);

    osal_printf("FWD_TYPE : 0x%x\t\t", hsm.r8390.lu_data.fwd_type);
    osal_printf("NEWSA_AC : 0x%x\t\t", hsm.r8390.lu_data.newsa_ac);
    osal_printf("PMOVE_ACT : 0x%x\n", hsm.r8390.lu_data.pmove_act);

    osal_printf("MACLIMIT_ACT : 0x%x\t", hsm.r8390.lu_data.maclimit_act);
    osal_printf("ROUTE_DATA : 0x%x\t", hsm.r8390.lu_data.route_data);
    osal_printf("DA_HIT : 0x%x\n", hsm.r8390.lu_data.da_hit);

    osal_printf("DA_DATA : 0x%x\t\t", hsm.r8390.lu_data.da_data);
    osal_printf("SA_HIT : 0x%x\t\t", hsm.r8390.lu_data.sa_hit);
    osal_printf("SA_DATA : 0x%x\n", hsm.r8390.lu_data.sa_data);

    osal_printf("CMAC : 0x%x\t\t", hsm.r8390.lu_data.cmac);
    osal_printf("GMAC : 0x%x\t\t", hsm.r8390.lu_data.gmac);
    osal_printf("IPV4_HDR_ERR : 0x%x\n", hsm.r8390.lu_data.ipv4_hdr_err);

    osal_printf("IPV4_OPT : 0x%x\t\t", hsm.r8390.lu_data.ipv4_opt);
    osal_printf("IPV4_TTL_ZERO : 0x%x\t", hsm.r8390.lu_data.ipv4_ttl_zero);
    osal_printf("IPV6_HDR_ERR : 0x%x\n", hsm.r8390.lu_data.ipv6_hdr_err);

    osal_printf("IPV6_HL_ZERO : 0x%x\t", hsm.r8390.lu_data.ipv6_hl_zero);
    osal_printf("IPV6_HOP : 0x%x\t\t", hsm.r8390.lu_data.ipv6_hop);
    osal_printf("OAM_ACT : 0x%x\n", hsm.r8390.lu_data.oam_act);

    osal_printf("STP : 0x%x\t\t", hsm.r8390.lu_data.stp);
    osal_printf("VLAN_ZERO : 0x%x\t\t", hsm.r8390.lu_data.vlan_zero);
    osal_printf("VLAN_SPM : 0x%x\n", hsm.r8390.lu_data.vlan_spm);

    osal_printf("FID : 0x%x\t\t", hsm.r8390.lu_data.fid);
    osal_printf("METER_DROP : 0x%x\t", hsm.r8390.lu_data.meter_drop);
    osal_printf("CFM_ACT : 0x%x\n", hsm.r8390.lu_data.cfm_act);

    osal_printf("CFM_IDX : 0x%x\t\t", hsm.r8390.lu_data.cfm_idx);
    osal_printf("CFI_ACT : 0x%x\t\t", hsm.r8390.lu_data.cfi_act);
    osal_printf("TAG_ACT : 0x%x\n", hsm.r8390.lu_data.tag_act);

    osal_printf("NIVID : 0x%x\t\t", hsm.r8390.lu_data.nivid);
    osal_printf("NOVID : 0x%x\t\t", hsm.r8390.lu_data.novid);
    osal_printf("RVID : 0x%x\n", hsm.r8390.lu_data.rvid);

    osal_printf("ACL_MEET : 0x%x\t\t", hsm.r8390.lu_data.acl_meet);
    osal_printf("ACLC2SC : 0x%x\t\t", hsm.r8390.lu_data.aclc2sc);
    osal_printf("ACL_HIT : 0x%x\n", hsm.r8390.lu_data.acl_hit);

    osal_printf("ACL_IDX : 0x%x\t\t", hsm.r8390.lu_data.acl_idx);
    osal_printf("ACL_DATA : 0x%x\t\t", hsm.r8390.lu_data.acl_data);
    osal_printf("SPE_TRAP_RSN : 0x%x\n", hsm.r8390.lu_data.spe_trap_rsn);

    osal_printf("RMA_BYP_VLAN : 0x%x\t", hsm.r8390.lu_data.rma_byp_vlan);
    osal_printf("SPE_TRAP_ACT : 0x%x\t", hsm.r8390.lu_data.spe_trap_act);
    osal_printf("EAV_CLASSA : 0x%x\n", hsm.r8390.lu_data.eav_classa);

    osal_printf("EAV_CLASSB : 0x%x\t", hsm.r8390.lu_data.eav_classb);
    osal_printf("HWATT_RSN : 0x%x\t\t", hsm.r8390.lu_data.hwatt_rsn);
    osal_printf("HWATT_ACT : 0x%x\n", hsm.r8390.lu_data.hwatt_act);

    osal_printf("RMA_PKT : 0x%x\t\t", hsm.r8390.lu_data.rma_pkt);
    osal_printf("RMA_ACT : 0x%x\t\t", hsm.r8390.lu_data.rma_act);
    osal_printf("RMA_BYPASS : 0x%x\n", hsm.r8390.lu_data.rma_bypass);

    osal_printf("C2SC_HIT_MODE : 0x%x\t", hsm.r8390.lu_data.c2sc_hit_mode);
    osal_printf("C2SC_HIT : 0x%x\t\t", hsm.r8390.lu_data.c2sc_hit);
    osal_printf("C2SC_DATA : 0x%x\n", hsm.r8390.lu_data.c2sc_data);

    osal_printf("INTERNAL_PRIO : 0x%x\t", hsm.r8390.lu_data.internal_prio);
    osal_printf("DP_PRE : 0x%x\t\t", hsm.r8390.lu_data.dp_pre);
    osal_printf("PROTOCOL_STORM : 0x%x\n", hsm.r8390.lu_data.protocol_storm);

    osal_printf("IBW_PASS : 0x%x\t\t", hsm.r8390.lu_data.ibw_pass);
    osal_printf("SLID : 0x%x\t\t", hsm.r8390.lu_data.slid);
    osal_printf("TRUNK_GRP : 0x%x\n", hsm.r8390.lu_data.trunk_grp);
    osal_printf("=============================================================\n");

    return RT_ERR_OK;
}
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_dumpHsm
 * Description:
 *      Dump hsm paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_dumpHsm(uint32 unit)
{
    hal_control_t *pHalCtrl = NULL;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

#if defined(CONFIG_SDK_RTL8390)
    switch (pHalCtrl->chip_id)
    {
        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
            if (RT_ERR_OK != _hal_dumpHsm_8390(unit))
            {
                return RT_ERR_FAILED;
            }
            break;

        default:
            return RT_ERR_FAILED;
    }
#endif

    return RT_ERR_OK;
} /* end of hal_dumpHsm */
#endif

#if defined(CONFIG_SDK_RTL8380)
int32 _hal_getHsm_8380(uint32 unit, uint32 index, hsm_param_t *hsm)
{
    uint32 val = 0;
    uint32 tmpVal = 0;
    uint32 MAPLE_HSM_DATA[17];

    memset(hsm, 0, sizeof(hsm_param_t));

    switch(index)
    {
    case 0:
        for(val = 0; val < 17; val++)
            MAPLE_HSM_DATA[val] = MAPLE_HSM0_DATA0r + val;
        break;

    case 1:
        for(val = 0; val < 17; val++)
            MAPLE_HSM_DATA[val] = MAPLE_HSM1_DATA0r + val;
        break;

    case 2:
        for(val = 0; val < 17; val++)
            MAPLE_HSM_DATA[val] = MAPLE_HSM2_DATA0r + val;
        break;

    default:
        break;
    }

    /* HSM_DATA0 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[0], &val);
    reg_field_get(0, MAPLE_HSM_DATA[0], MAPLE_ACL_IDX0f, &tmpVal, &val);
    hsm->r8380.acl_idx0 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[0], MAPLE_ACL_IDX1f, &tmpVal, &val);
    hsm->r8380.acl_idx1 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[0], MAPLE_ACL_IDX2f, &tmpVal, &val);
    hsm->r8380.acl_idx2 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[0], MAPLE_ACL_IDX3f, &tmpVal, &val);
    hsm->r8380.acl_idx3 = tmpVal;

    /* HSM_DATA1 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[1], &val);
    reg_field_get(0, MAPLE_HSM_DATA[1], MAPLE_ACL_IDX4f, &tmpVal, &val);
    hsm->r8380.acl_idx4 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[1], MAPLE_ACL_IDX5f, &tmpVal, &val);
    hsm->r8380.acl_idx5 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[1], MAPLE_ACL_IDX6f, &tmpVal, &val);
    hsm->r8380.acl_idx6 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[1], MAPLE_ACL_IDX7f, &tmpVal, &val);
    hsm->r8380.acl_idx7 = tmpVal;

    /* HSM_DATA2 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[2], &val);
    reg_field_get(0, MAPLE_HSM_DATA[2], MAPLE_ACL_ROUTf, &tmpVal, &val);
    hsm->r8380.acl_rout = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[2], MAPLE_ACL_REDIRf, &tmpVal, &val);
    hsm->r8380.acl_redir = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[2], MAPLE_ACL_COPY_HITf, &tmpVal, &val);
    hsm->r8380.acl_copy_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[2], MAPLE_ACL_DPMf, &tmpVal, &val);
    hsm->r8380.acl_dpm = tmpVal;

    /* HSM_DATA3 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[3], &val);
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_MIR_ACTf, &tmpVal, &val);
    hsm->r8380.acl_mir_act = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_MIR_HITf, &tmpVal, &val);
    hsm->r8380.acl_mir_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_CPU_PRIf, &tmpVal, &val);
    hsm->r8380.acl_cpuPri = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_CPUPRI_HITf, &tmpVal, &val);
    hsm->r8380.acl_cpuPri_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_NORMALPRI_HITf, &tmpVal, &val);
    hsm->r8380.acl_normalPri_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_EGRTAGSTS_HITf, &tmpVal, &val);
    hsm->r8380.acl_egr_tagSts_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_RMK_VALf, &tmpVal, &val);
    hsm->r8380.acl_rmk_val = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_RMK_ACTf, &tmpVal, &val);
    hsm->r8380.acl_rmk_act = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_RMK_HITf, &tmpVal, &val);
    hsm->r8380.acl_rmk_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_LOG_HITf, &tmpVal, &val);
    hsm->r8380.acl_log_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[3], MAPLE_ACL_FWD_INFOf, &tmpVal, &val);
    hsm->r8380.acl_fwd_info = tmpVal;

    /* HSM_DATA4 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[4], &val);
    reg_field_get(0, MAPLE_HSM_DATA[4], MAPLE_ACL_OVID_HITf, &tmpVal, &val);
    hsm->r8380.acl_ovid_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[4], MAPLE_ACL_IVID_HITf, &tmpVal, &val);
    hsm->r8380.acl_ivid_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[4], MAPLE_ACL_FLT_HITf, &tmpVal, &val);
    hsm->r8380.acl_fltr_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[4], MAPLE_ACL_FLT_DPMf, &tmpVal, &val);
    hsm->r8380.acl_fltr_dpm = tmpVal;

    /* HSM_DATA5 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[5], &val);
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_COPY_TO_CPUf, &tmpVal, &val);
    hsm->r8380.copyToCpu = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ALE_TRAPf, &tmpVal, &val);
    hsm->r8380.ale_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ALE_DROPf, &tmpVal, &val);
    hsm->r8380.ale_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_RNG_CHK_IPf, &tmpVal, &val);
    hsm->r8380.rng_chk_ip = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_METER_HITf, &tmpVal, &val);
    hsm->r8380.acl_meter = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_ITPID_IDXf, &tmpVal, &val);
    hsm->r8380.acl_itpid_idx = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_ITPID_HITf, &tmpVal, &val);
    hsm->r8380.acl_itpid_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_OTPID_IDXf, &tmpVal, &val);
    hsm->r8380.acl_otpid_idx = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_OTPID_HITf, &tmpVal, &val);
    hsm->r8380.acl_otpid_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_MIR_IDXf, &tmpVal, &val);
    hsm->r8380.acl_mir_idx = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_CPUTAGf, &tmpVal, &val);
    hsm->r8380.acl_cpuTag = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_FORCEf, &tmpVal, &val);
    hsm->r8380.acl_force = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[5], MAPLE_ACL_IDX_9_0f, &tmpVal, &val);
    hsm->r8380.acl_idx_9_0 = tmpVal;

    /* HSM_DATA6 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[6], &val);
    reg_field_get(0, MAPLE_HSM_DATA[6], MAPLE_RNG_CHK_SPMf, &tmpVal, &val);
    hsm->r8380.rng_chk_spm = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[6], MAPLE_RNG_CHK_COMMf, &tmpVal, &val);
    hsm->r8380.rng_chk_comm = tmpVal;

    /* HSM_DATA7 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[7], &val);
    reg_field_get(0, MAPLE_HSM_DATA[7], MAPLE_RMA_FLOODf, &tmpVal, &val);
    hsm->r8380.rma_fld = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[7], MAPLE_L2_DA_IDXf, &tmpVal, &val);
    hsm->r8380.l2_da_idx = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[7], MAPLE_L2_DA_HITf, &tmpVal, &val);
    hsm->r8380.l2_da_hit = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[7], MAPLE_L2_DA_LKMISSf, &tmpVal, &val);
    hsm->r8380.l2_da_lm = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[7], MAPLE_L2_SA_IDXf, &tmpVal, &val);
    hsm->r8380.l2_sa_idx = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[7], MAPLE_L2_SA_HITf, &tmpVal, &val);
    hsm->r8380.l2_sa_hit = tmpVal;

    /* HSM_DATA8 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[8], &val);
    reg_field_get(0, MAPLE_HSM_DATA[8], MAPLE_INT_PRIf, &tmpVal, &val);
    hsm->r8380.int_pri = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[8], MAPLE_DPMf, &tmpVal, &val);
    hsm->r8380.dpm = tmpVal;

    /* HSM_DATA9 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[9], &val);
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_FIDf, &tmpVal, &val);
    hsm->r8380.fid = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_VLAN_FWD_BASEf, &tmpVal, &val);
    hsm->r8380.vlan_fwd_base = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_COPY_BYP_VLAN_IFILTERf, &tmpVal, &val);
    hsm->r8380.copy_igrVlan_lky = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_BYP_STPf, &tmpVal, &val);
    hsm->r8380.byp_igr_stp = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_BYP_STORMf, &tmpVal, &val);
    hsm->r8380.byp_storm = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_CPU_PRIf, &tmpVal, &val);
    hsm->r8380.cpu_pri = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_SRC_LOGIC_PORTf, &tmpVal, &val);
    hsm->r8380.slp = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_EGR_VLAN_LKYf, &tmpVal, &val);
    hsm->r8380.egr_vlan_lky = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_EAV_CLASS_Bf, &tmpVal, &val);
    hsm->r8380.eav_class_b = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_EAV_CLASS_Af, &tmpVal, &val);
    hsm->r8380.eav_class_a = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[9], MAPLE_ATK_TYPEf, &tmpVal, &val);
    hsm->r8380.atk_prvnt_rsn = tmpVal;

    /* HSM_DATA10 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[10], &val);
    reg_field_get(0, MAPLE_HSM_DATA[10], MAPLE_L2_DIS_SA_LRNf, &tmpVal, &val);
    hsm->r8380.dis_sa_lrn = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[10], MAPLE_IGR_VLAN_LKYf, &tmpVal, &val);
    hsm->r8380.igr_vlan_lky = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[10], MAPLE_INPUT_QIDf, &tmpVal, &val);
    hsm->r8380.input_qid = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[10], MAPLE_OTAG_STSf, &tmpVal, &val);
    hsm->r8380.otag_sts = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[10], MAPLE_ITAG_STSf, &tmpVal, &val);
    hsm->r8380.itag_sts = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[10], MAPLE_ALE_IVIDf, &tmpVal, &val);
    hsm->r8380.ale_ivid = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[10], MAPLE_ALE_OVIDf, &tmpVal, &val);
    hsm->r8380.ale_ovid = tmpVal;

    /* HSM_DATA11 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[11], &val);
    reg_field_get(0, MAPLE_HSM_DATA[11], MAPLE_COPY_BYP_STPf, &tmpVal, &val);
    hsm->r8380.copy_igrStp_lky = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[11], MAPLE_BYP_EGR_STPf, &tmpVal, &val);
    hsm->r8380.byp_egr_stp_pmsk = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[11], MAPLE_BYP_IGR_BWCTRLf, &tmpVal, &val);
    hsm->r8380.byp_igr_bwctrl = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[11], MAPLE_KNMC_BYP_VLAN_EFILTERf, &tmpVal, &val);
    hsm->r8380.knmc_byp_vlan_efilter = tmpVal;

    /* HSM_DATA12 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[12], &val);
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_MIR_FLTR_DROPf, &tmpVal, &val);
    hsm->r8380.mir_filter_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_TRUNK_DROPf, &tmpVal, &val);
    hsm->r8380.trunk_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_ISOLATION_DROPf, &tmpVal, &val);
    hsm->r8380.isolation_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_SELF_FLTR_DROPf, &tmpVal, &val);
    hsm->r8380.src_port_filter_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_STORM_DROPf, &tmpVal, &val);
    hsm->r8380.storm_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_STP_EFILTER_DROPf, &tmpVal, &val);
    hsm->r8380.mstp_egr_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_VLAN_EFILTER_DROPf, &tmpVal, &val);
    hsm->r8380.vlan_egr_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_INVLD_MCASTDPM_DROPf, &tmpVal, &val);
    hsm->r8380.invld_mcast_dpm_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_INVLD_DPM_DROPf, &tmpVal, &val);
    hsm->r8380.invld_dpm_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_DA_LKMISS_DROPf, &tmpVal, &val);
    hsm->r8380.l2_lm_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_DA_BLK_DROPf, &tmpVal, &val);
    hsm->r8380.l2_da_blk = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_VLAN_MAC_CONSTRT_DROPf, &tmpVal, &val);
    hsm->r8380.l2_vlan_constrt_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_PORT_MAC_CONSTRT_DROPf, &tmpVal, &val);
    hsm->r8380.l2_port_constrt_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_SYS_MAC_CONSTRT_DROPf, &tmpVal, &val);
    hsm->r8380.l2_sys_constrt_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_NEW_SA_DROPf, &tmpVal, &val);
    hsm->r8380.l2_newSA_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_PORT_MV_DROPf, &tmpVal, &val);
    hsm->r8380.l2_portMove_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_SA_BLK_DROPf, &tmpVal, &val);
    hsm->r8380.l2_sa_blk = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_SELFMAC_DROPf, &tmpVal, &val);
    hsm->r8380.sTrap_selfMac_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_L2_INVLD_SA_DROPf, &tmpVal, &val);
    hsm->r8380.l2_invldSA_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_STP_IFILTER_DROPf, &tmpVal, &val);
    hsm->r8380.mstp_igr_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_VLAN_IFILTER_DROPf, &tmpVal, &val);
    hsm->r8380.vlan_igr_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_VLAN_CFI_DROPf, &tmpVal, &val);
    hsm->r8380.vlan_cfi_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_VLAN_AFT_DROPf, &tmpVal, &val);
    hsm->r8380.vlan_aft_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_ACL_METER_DROPf, &tmpVal, &val);
    hsm->r8380.acl_meter_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_ACL_REDIR_DROPf, &tmpVal, &val);
    hsm->r8380.acl_redir_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_INPUT_Q_DROPf, &tmpVal, &val);
    hsm->r8380.input_q_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_ROUT_DROPf, &tmpVal, &val);
    hsm->r8380.l3_rout_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_ACL_DROPf, &tmpVal, &val);
    hsm->r8380.acl_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_ACL_LKMISS_DROPf, &tmpVal, &val);
    hsm->r8380.acl_lm_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_ATK_DROPf, &tmpVal, &val);
    hsm->r8380.atk_prvnt_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_RMA_DROPf, &tmpVal, &val);
    hsm->r8380.rma_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[12], MAPLE_RSPAN_DROPf, &tmpVal, &val);
    hsm->r8380.rspan_drop = tmpVal;

    /* HSM_DATA13 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[13], &val);
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_REASON_COPYf, &tmpVal, &val);
    hsm->r8380.reason_n_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_ACL_COPYf, &tmpVal, &val);
    hsm->r8380.acl_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_L2_DA_LKMISS_COPYf, &tmpVal, &val);
    hsm->r8380.l2_lm_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_L2_PORT_MV_COPYf, &tmpVal, &val);
    hsm->r8380.l2_portMove_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_L2_NEW_SA_COPYf, &tmpVal, &val);
    hsm->r8380.l2_newSA_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_L2_VLAN_MAC_CONSTRT_COPYf, &tmpVal, &val);
    hsm->r8380.l2_vlan_constrt_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_L2_PORT_MAC_CONSTRT_COPYf, &tmpVal, &val);
    hsm->r8380.l2_port_constrt_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_L2_SYS_MAC_CONSTRT_COPYf, &tmpVal, &val);
    hsm->r8380.l2_sys_constrt_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_SPECIAL_COPYf, &tmpVal, &val);
    hsm->r8380.sTrap_copy = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_REASON_DROPf, &tmpVal, &val);
    hsm->r8380.reason_n_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_MAX_LEN_DROPf, &tmpVal, &val);
    hsm->r8380.tx_maxLen_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_FLOWCTRL_DROPf, &tmpVal, &val);
    hsm->r8380.flowCtrl_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_LINK_DOWN_DROPf, &tmpVal, &val);
    hsm->r8380.link_down_drop = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[13], MAPLE_ACL_FLTR_DROPf, &tmpVal, &val);
    hsm->r8380.acl_filter_drop = tmpVal;

    /* HSM_DATA14 */
    val = 0;
    reg_read(0, MAPLE_HSM_DATA[14], &val);
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_REASON_TRAPf, &tmpVal, &val);
    hsm->r8380.reason_n_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_L2_DA_LKMISS_TRAPf, &tmpVal, &val);
    hsm->r8380.l2_lm_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_L2_VLAN_MAC_CONSTRT_TRAPf, &tmpVal, &val);
    hsm->r8380.l2_vlan_constrt_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_L2_PORT_MAC_CONSTRT_TRAPf, &tmpVal, &val);
    hsm->r8380.l2_port_constrt_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_L2_SYS_MAC_CONSTRT_TRAPf, &tmpVal, &val);
    hsm->r8380.l2_sys_constrt_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_L2_NEW_SA_TRAPf, &tmpVal, &val);
    hsm->r8380.l2_newSA_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_L2_PORT_MV_TRAPf, &tmpVal, &val);
    hsm->r8380.l2_portMove_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_SELFMAC_TRAPf, &tmpVal, &val);
    hsm->r8380.sTrap_selfMac_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_VLAN_IFILTER_TRAPf, &tmpVal, &val);
    hsm->r8380.vlan_igr_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_VLAN_CFI_TRAPf, &tmpVal, &val);
    hsm->r8380.vlan_cfi_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_ROUT_TRAPf, &tmpVal, &val);
    hsm->r8380.l3_rout_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_ACL_TRAPf, &tmpVal, &val);
    hsm->r8380.acl_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_SPECIAL_TRAPf, &tmpVal, &val);
    hsm->r8380.sTrap_comm_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_RLDP_RLPP_TRAPf, &tmpVal, &val);
    hsm->r8380.rldp_rlpp_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_ATK_TRAPf, &tmpVal, &val);
    hsm->r8380.atk_prvnt_trap = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[14], MAPLE_RMA_TRAPf, &tmpVal, &val);
    hsm->r8380.rma_trap = tmpVal;

    /* HSM_DATA15 */

    val = 0;
    reg_read(0, MAPLE_HSM_DATA[15], &val);
    reg_field_get(0, MAPLE_HSM_DATA[15], MAPLE_ACL_IDX8f, &tmpVal, &val);
    hsm->r8380.acl_idx8 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[15], MAPLE_ACL_IDX9f, &tmpVal, &val);
    hsm->r8380.acl_idx9 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[15], MAPLE_ACL_IDX10f, &tmpVal, &val);
    hsm->r8380.acl_idx10 = tmpVal;
    reg_field_get(0, MAPLE_HSM_DATA[15], MAPLE_ACL_IDX11f, &tmpVal, &val);
    hsm->r8380.acl_idx11 = tmpVal;

    /* HSM_DATA16 */

    val = 0;
    reg_read(0, MAPLE_HSM_DATA[16], &val);
    reg_field_get(0, MAPLE_HSM_DATA[16], MAPLE_ACL_IDX_10f, &tmpVal, &val);
    hsm->r8380.acl_idx_10 = tmpVal;
    
    return RT_ERR_OK;
}
#endif

#if defined(CONFIG_SDK_RTL8380)
int32 _hal_dumpHsm_8380(uint32 unit, uint32 index)
{
    hsm_param_t   hsm;
    uint32        tmp;
    if(RT_ERR_OK != _hal_getHsm_8380(unit, index, &hsm))
    {
        return RT_ERR_FAILED;
    }

    osal_printf("\n========================================  HSM %d  ========================================\n", index);
    tmp = hsm.r8380.acl_idx0;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx0", tmp);
    tmp = hsm.r8380.acl_idx1;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx1", tmp);
    tmp = hsm.r8380.acl_idx2;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x\n", "acl_idx2", tmp);

    tmp = hsm.r8380.acl_idx3;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx3", tmp);
    tmp = hsm.r8380.acl_idx4;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx4", tmp);
    tmp = hsm.r8380.acl_idx5;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x\n", "acl_idx5", tmp);

    tmp = hsm.r8380.acl_idx6;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx6", tmp);
    tmp = hsm.r8380.acl_idx7;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx7", tmp);
    tmp = hsm.r8380.acl_idx8;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x\n", "acl_idx8", tmp);

    tmp = hsm.r8380.acl_idx9;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx9", tmp);
    tmp = hsm.r8380.acl_idx10;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x", "acl_idx10", tmp);
    tmp = hsm.r8380.acl_idx11;
    tmp = ((tmp << 1) & 0x100) | (tmp & 0x7f);
    osal_printf("%-18s = 0x%-8x\n", "acl_idx11", tmp);
    
    osal_printf("%-18s = 0x%-8x", "acl_dpm", hsm.r8380.acl_dpm);
    osal_printf("%-18s = 0x%-8x", "acl_idx", (hsm.r8380.acl_idx_10 << 10) | hsm.r8380.acl_idx_9_0);
    osal_printf("%-18s = 0x%-8x\n", "acl_copy_hit", hsm.r8380.acl_copy_hit);
    
    osal_printf("%-18s = 0x%-8x", "acl_redir", hsm.r8380.acl_redir);
    osal_printf("%-18s = 0x%-8x", "acl_fwd_info", hsm.r8380.acl_fwd_info);
    osal_printf("%-18s = 0x%-8x\n", "acl_log_hit", hsm.r8380.acl_log_hit);
    
    osal_printf("%-18s = 0x%-8x", "acl_rout", hsm.r8380.acl_rout);
    osal_printf("%-18s = 0x%-8x", "acl_rmk_hit", hsm.r8380.acl_rmk_hit);
    osal_printf("%-18s = 0x%-8x\n", "acl_rmk_act", hsm.r8380.acl_rmk_act);
    
    osal_printf("%-18s = 0x%-8x", "acl_rmk_val", hsm.r8380.acl_rmk_val);
    osal_printf("%-18s = 0x%-8x", "acl_cpuPri_hit", hsm.r8380.acl_cpuPri_hit);
    osal_printf("%-18s = 0x%-8x\n", "acl_cpuPri", hsm.r8380.acl_cpuPri);
    
    osal_printf("%-18s = 0x%-8x", "acl_normalPri_hit", hsm.r8380.acl_normalPri_hit);
    osal_printf("%-18s = 0x%-8x", "acl_mir_hit", hsm.r8380.acl_mir_hit);
    osal_printf("%-18s = 0x%-8x\n", "acl_mir_act", hsm.r8380.acl_mir_act);
    
    osal_printf("%-18s = 0x%-8x", "acl_mir_idx", hsm.r8380.acl_mir_idx);
    osal_printf("%-18s = 0x%-8x", "acl_fltr_dpm", hsm.r8380.acl_fltr_dpm);
    osal_printf("%-18s = 0x%-8x\n", "acl_fltr_hit", hsm.r8380.acl_fltr_hit);
    
    osal_printf("%-18s = 0x%-8x", "acl_ivid_hit", hsm.r8380.acl_ivid_hit);
    osal_printf("%-18s = 0x%-8x", "acl_ovid_hit", hsm.r8380.acl_ovid_hit);
    osal_printf("%-18s = 0x%-8x\n", "acl_egr_tagSts_hit", hsm.r8380.acl_egr_tagSts_hit);

    osal_printf("%-18s = 0x%-8x", "acl_meter_hit", hsm.r8380.acl_meter);
    osal_printf("%-18s = 0x%-8x", "acl_force", hsm.r8380.acl_force);
    osal_printf("%-18s = 0x%-8x\n", "acl_cpuTag", hsm.r8380.acl_cpuTag);
    
    osal_printf("%-18s = 0x%-8x", "acl_otpid_hit", hsm.r8380.acl_otpid_hit);    
    osal_printf("%-18s = 0x%-8x", "acl_otpid_idx", hsm.r8380.acl_otpid_idx);
    osal_printf("%-18s = 0x%-8x\n", "acl_itpid_hit", hsm.r8380.acl_itpid_hit);
    
    osal_printf("%-18s = 0x%-8x", "acl_itpid_idx", hsm.r8380.acl_itpid_idx);    
    osal_printf("%-18s = 0x%-8x", "rng_chk_comm", hsm.r8380.rng_chk_comm);
    osal_printf("%-18s = 0x%-8x\n", "rng_chk_spm", hsm.r8380.rng_chk_spm);
    
    osal_printf("%-18s = 0x%-8x", "rng_chk_ip", hsm.r8380.rng_chk_ip);
    osal_printf("%-18s = 0x%-8x", "l2_sa_hit", hsm.r8380.l2_sa_hit);
    osal_printf("%-18s = 0x%-8x\n", "l2_sa_idx", hsm.r8380.l2_sa_idx);
    
    osal_printf("%-18s = 0x%-8x", "l2_da_lm", hsm.r8380.l2_da_lm);
    osal_printf("%-18s = 0x%-8x", "l2_da_hit", hsm.r8380.l2_da_hit);
    osal_printf("%-18s = 0x%-8x\n", "l2_da_idx", hsm.r8380.l2_da_idx);

    osal_printf("%-18s = 0x%-8x", "dis_sa_lrn", hsm.r8380.dis_sa_lrn);    
    osal_printf("%-18s = 0x%-8x", "rma_fld", hsm.r8380.rma_fld);
    osal_printf("%-18s = 0x%-8x\n", "ale_ovid", hsm.r8380.ale_ovid);
    
    osal_printf("%-18s = 0x%-8x", "ale_ivid", hsm.r8380.ale_ivid);    
    osal_printf("%-18s = 0x%-8x", "itag_sts", hsm.r8380.itag_sts);
    osal_printf("%-18s = 0x%-8x\n", "otag_sts", hsm.r8380.otag_sts);

    osal_printf("%-18s = 0x%-8x", "ale_drop", hsm.r8380.ale_drop);    
    osal_printf("%-18s = 0x%-8x", "ale_trap", hsm.r8380.ale_trap);
    osal_printf("%-18s = 0x%-8x\n", "copyToCpu", hsm.r8380.copyToCpu);
    
    osal_printf("%-18s = 0x%-8x", "int_pri", hsm.r8380.int_pri);    
    osal_printf("%-18s = 0x%-8x", "cpu_pri", hsm.r8380.cpu_pri);
    osal_printf("%-18s = 0x%-8x\n", "dpm", hsm.r8380.dpm);
    
    osal_printf("%-18s = 0x%-8x", "slp", hsm.r8380.slp);    
    osal_printf("%-18s = 0x%-8x", "fid", hsm.r8380.fid);
    osal_printf("%-18s = 0x%-8x\n", "input_qid", hsm.r8380.input_qid);
    
    osal_printf("%-18s = 0x%-8x", "vlan_fwd_base", hsm.r8380.vlan_fwd_base);    
    osal_printf("%-18s = 0x%-8x", "atk_prvnt_rsn", hsm.r8380.atk_prvnt_rsn);
    osal_printf("%-18s = 0x%-8x\n", "eav_class_a", hsm.r8380.eav_class_a);
    
    osal_printf("%-18s = 0x%-8x", "eav_class_b", hsm.r8380.eav_class_b);    
    osal_printf("%-18s = 0x%-8x", "egr_vlan_lky", hsm.r8380.egr_vlan_lky);
    osal_printf("%-18s = 0x%-8x\n", "byp_igr_bwctrl", hsm.r8380.byp_igr_bwctrl);
    
    osal_printf("%-18s = 0x%-8x", "byp_storm", hsm.r8380.byp_storm);    
    osal_printf("%-18s = 0x%-8x", "byp_igr_stp", hsm.r8380.byp_igr_stp);
    osal_printf("%-18s = 0x%-8x\n", "igr_vlan_lky", hsm.r8380.igr_vlan_lky);
    
    osal_printf("%-18s = 0x%-8x", "copy_igrVlan_lky", hsm.r8380.copy_igrVlan_lky);   
    osal_printf("%-18s = 0x%-8x", "copy_igrStp_lky", hsm.r8380.copy_igrStp_lky);
    osal_printf("%-18s = 0x%-8x\n", "byp_egr_stp_pmsk", hsm.r8380.byp_egr_stp_pmsk);
    
    osal_printf("%-18s = 0x%-8x\n", "knmc_byp_vlan_efilter", hsm.r8380.knmc_byp_vlan_efilter);

    osal_printf("-----------------------------------------drop-----------------------------------------\n");
    osal_printf("%-21s = 0x%-3x", "rspan_drop", hsm.r8380.rspan_drop);
    osal_printf("%-21s = 0x%-3x", "rma_drop", hsm.r8380.rma_drop);
    osal_printf("%-21s = 0x%-3x\n", "atk_prvnt_drop", hsm.r8380.atk_prvnt_drop);

    osal_printf("%-21s = 0x%-3x", "acl_lm_drop", hsm.r8380.acl_lm_drop);
    osal_printf("%-21s = 0x%-3x", "acl_drop", hsm.r8380.acl_drop);
    osal_printf("%-21s = 0x%-3x\n", "acl_redir_drop", hsm.r8380.acl_redir_drop);

    osal_printf("%-21s = 0x%-3x", "acl_meter_drop", hsm.r8380.acl_meter_drop);
    osal_printf("%-21s = 0x%-3x", "acl_filter_drop", hsm.r8380.acl_filter_drop);
    osal_printf("%-21s = 0x%-3x\n", "vlan_aft_drop", hsm.r8380.vlan_aft_drop);

    osal_printf("%-21s = 0x%-3x", "vlan_cfi_drop", hsm.r8380.vlan_cfi_drop);
    osal_printf("%-21s = 0x%-3x", "vlan_igr_drop", hsm.r8380.vlan_igr_drop);
    osal_printf("%-21s = 0x%-3x\n", "vlan_egr_drop", hsm.r8380.vlan_egr_drop);

    osal_printf("%-21s = 0x%-3x", "mstp_igr_drop", hsm.r8380.mstp_igr_drop);
    osal_printf("%-21s = 0x%-3x", "l2_invldSA_drop", hsm.r8380.l2_invldSA_drop);
    osal_printf("%-21s = 0x%-3x\n", "l2_sa_blk", hsm.r8380.l2_sa_blk);

    osal_printf("%-21s = 0x%-3x", "l2_portMove_drop", hsm.r8380.l2_portMove_drop);
    osal_printf("%-21s = 0x%-3x", "l2_newSA_drop", hsm.r8380.l2_newSA_drop);
    osal_printf("%-21s = 0x%-3x\n", "l2_sys_constrt_drop", hsm.r8380.l2_sys_constrt_drop);

    osal_printf("%-21s = 0x%-3x", "l2_port_constrt_drop", hsm.r8380.l2_port_constrt_drop);
    osal_printf("%-21s = 0x%-3x", "l2_vlan_constrt_drop", hsm.r8380.l2_vlan_constrt_drop);
    osal_printf("%-21s = 0x%-3x\n", "l2_da_blk", hsm.r8380.l2_da_blk);

    osal_printf("%-21s = 0x%-3x", "l2_lm_drop", hsm.r8380.l2_lm_drop);
    osal_printf("%-21s = 0x%-3x", "l3_rout_drop", hsm.r8380.l3_rout_drop);
    osal_printf("%-21s = 0x%-3x\n", "input_q_drop", hsm.r8380.input_q_drop);

    osal_printf("%-21s = 0x%-3x", "invld_dpm_drop", hsm.r8380.invld_dpm_drop);
    osal_printf("%-21s = 0x%-3x", "invld_mcast_dpm_drop", hsm.r8380.invld_mcast_dpm_drop);
    osal_printf("%-21s = 0x%-3x\n", "mstp_egr_drop", hsm.r8380.mstp_egr_drop);

    osal_printf("%-21s = 0x%-3x", "storm_drop", hsm.r8380.storm_drop);
    osal_printf("%-21s = 0x%-3x", "src_port_filter_drop", hsm.r8380.src_port_filter_drop);
    osal_printf("%-21s = 0x%-3x\n", "isolation_drop", hsm.r8380.isolation_drop);

    osal_printf("%-21s = 0x%-3x", "trunk_drop", hsm.r8380.trunk_drop);
    osal_printf("%-21s = 0x%-3x", "mir_filter_drop", hsm.r8380.mir_filter_drop);
    osal_printf("%-21s = 0x%-3x\n", "sTrap_selfMac_drop", hsm.r8380.sTrap_selfMac_drop);

    osal_printf("%-21s = 0x%-3x", "link_down_drop", hsm.r8380.link_down_drop);
    osal_printf("%-21s = 0x%-3x", "flowCtrl_drop", hsm.r8380.flowCtrl_drop);
    osal_printf("%-21s = 0x%-3x\n", "tx_maxLen_drop", hsm.r8380.tx_maxLen_drop);

    osal_printf("%-21s = 0x%-3x\n", "reason_n_drop", hsm.r8380.reason_n_drop);
    osal_printf("-----------------------------------------copy-----------------------------------------\n");
    osal_printf("%-21s = 0x%-3x", "sTrap_copy", hsm.r8380.sTrap_copy);
    osal_printf("%-21s = 0x%-3x", "l2_sys_constrt_copy", hsm.r8380.l2_sys_constrt_copy);
    osal_printf("%-21s = 0x%-3x\n", "l2_port_constrt_copy", hsm.r8380.l2_port_constrt_copy);

    osal_printf("%-21s = 0x%-3x", "l2_vlan_constrt_copy", hsm.r8380.l2_vlan_constrt_copy);
    osal_printf("%-21s = 0x%-3x", "l2_newSA_copy", hsm.r8380.l2_newSA_copy);
    osal_printf("%-21s = 0x%-3x\n", "l2_portMove_copy", hsm.r8380.l2_portMove_copy);

    osal_printf("%-21s = 0x%-3x", "l2_lm_copy", hsm.r8380.l2_lm_copy);
    osal_printf("%-21s = 0x%-3x", "acl_copy", hsm.r8380.acl_copy);
    osal_printf("%-21s = 0x%-3x\n", "reason_n_copy", hsm.r8380.reason_n_copy);
    osal_printf("-----------------------------------------trap-----------------------------------------\n");
    osal_printf("%-21s = 0x%-3x", "rma_trap", hsm.r8380.rma_trap);
    osal_printf("%-21s = 0x%-3x", "atk_prvnt_trap", hsm.r8380.atk_prvnt_trap);
    osal_printf("%-21s = 0x%-3x\n", "rldp_rlpp_trap", hsm.r8380.rldp_rlpp_trap);

    osal_printf("%-21s = 0x%-3x", "sTrap_comm_trap", hsm.r8380.sTrap_comm_trap);
    osal_printf("%-21s = 0x%-3x", "acl_trap", hsm.r8380.acl_trap);
    osal_printf("%-21s = 0x%-3x\n", "l3_rout_trap", hsm.r8380.l3_rout_trap);

    osal_printf("%-21s = 0x%-3x", "vlan_cfi_trap", hsm.r8380.vlan_cfi_trap);
    osal_printf("%-21s = 0x%-3x", "vlan_igr_trap", hsm.r8380.vlan_igr_trap);
    osal_printf("%-21s = 0x%-3x\n", "sTrap_selfMac_trap", hsm.r8380.sTrap_selfMac_trap);

    osal_printf("%-21s = 0x%-3x", "l2_portMove_trap", hsm.r8380.l2_portMove_trap);
    osal_printf("%-21s = 0x%-3x", "l2_newSA_trap", hsm.r8380.l2_newSA_trap);
    osal_printf("%-21s = 0x%-3x\n", "l2_sys_constrt_trap", hsm.r8380.l2_sys_constrt_trap);

    osal_printf("%-21s = 0x%-3x", "l2_port_constrt_trap", hsm.r8380.l2_port_constrt_trap);
    osal_printf("%-21s = 0x%-3x", "l2_vlan_constrt_trap", hsm.r8380.l2_vlan_constrt_trap);
    osal_printf("%-21s = 0x%-3x\n", "l2_lm_trap", hsm.r8380.l2_lm_trap);

    osal_printf("%-21s = 0x%-3x\n", "reason_n_trap", hsm.r8380.reason_n_trap);
    osal_printf("====================================================================================\n");

    return RT_ERR_OK;
}
#endif

#if defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_dumpHsmIdx
 * Description:
 *      Dump hsm paramter of the specified device.
 * Input:
 *      unit - unit id
 *      index -hsm index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_dumpHsmIdx(uint32 unit, uint32 index)
{
    hal_control_t *pHalCtrl = NULL;

    if ((pHalCtrl = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

#if defined(CONFIG_SDK_RTL8380)
    if ((RTL8380M_CHIP_ID == pHalCtrl->chip_id) || (RTL8330M_CHIP_ID == pHalCtrl->chip_id) ||
        (RTL8382M_CHIP_ID == pHalCtrl->chip_id) || (RTL8332M_CHIP_ID == pHalCtrl->chip_id) ||
        (RTL8380MES_CHIP_ID == pHalCtrl->chip_id) || (RTL8330MES_CHIP_ID == pHalCtrl->chip_id) ||
        (RTL8382MES_CHIP_ID == pHalCtrl->chip_id) || (RTL8332MES_CHIP_ID == pHalCtrl->chip_id))
    {
        if(RT_ERR_OK != _hal_dumpHsm_8380(unit, index))
        {
            return RT_ERR_FAILED;
        }
    }
#endif

    return RT_ERR_OK;
} /* end of hal_dumpHsmIdx */
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getDbgCntr
 * Description:
 *      Get debug counter of the specified device.
 * Input:
 *      unit  - unit id
 *      type  - debug counter type
 * Output:
 *      pCntr - value of counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_getDbgCntr(uint32 unit, rtk_dbg_mib_dbgType_t type, uint32 *pCntr)
{
    int32 ret = RT_ERR_FAILED;
	
    hal_control_t *pInfo;

    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }
	

#if defined(CONFIG_SDK_RTL8390)
    switch (pInfo->chip_id)
    {
        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
			
    	 switch (type)
        {
            case RTK_DBG_MIB_ALE_TX_GOOD_PKTS:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER0r,
                        CYPRESS_REASON_0f, pCntr);
                break;
            case RTK_DBG_MIB_ERROR_PKTS:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER1r,
                        CYPRESS_REASON_1f, pCntr);
                break;
            case RTK_DBG_MIB_EGR_ACL_DROP:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER2r,
                        CYPRESS_REASON_2f, pCntr);
                break;
            case RTK_DBG_MIB_EGR_METER_DROP:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER3r,
                        CYPRESS_REASON_3f, pCntr);
                break;
            case RTK_DBG_MIB_OAM:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER4r,
                        CYPRESS_REASON_4f, pCntr);
                break;
            case RTK_DBG_MIB_CFM:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER5r,
                        CYPRESS_REASON_5f, pCntr);
                break;
            case RTK_DBG_MIB_VLAN_IGR_FLTR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER6r,
                        CYPRESS_REASON_6f, pCntr);
                break;
            case RTK_DBG_MIB_VLAN_ERR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER7r,
                        CYPRESS_REASON_7f, pCntr);
                break;
            case RTK_DBG_MIB_INNER_OUTER_CFI_EQUAL_1:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER8r,
                        CYPRESS_REASON_8f, pCntr);
                break;
            case RTK_DBG_MIB_VLAN_TAG_FORMAT:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER9r,
                        CYPRESS_REASON_9f, pCntr);
                break;
            case RTK_DBG_MIB_SRC_PORT_SPENDING_TREE:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER10r,
                        CYPRESS_REASON_10f, pCntr);
                break;
            case RTK_DBG_MIB_INBW:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER11r,
                        CYPRESS_REASON_11f, pCntr);
                break;
            case RTK_DBG_MIB_RMA:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER12r,
                        CYPRESS_REASON_12f, pCntr);
                break;
            case RTK_DBG_MIB_HW_ATTACK_PREVENTION:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER13r,
                        CYPRESS_REASON_13f, pCntr);
                break;
            case RTK_DBG_MIB_PROTO_STORM:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER14r,
                        CYPRESS_REASON_14f, pCntr);
                break;
            case RTK_DBG_MIB_MCAST_SA:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER15r,
                        CYPRESS_REASON_15f, pCntr);
                break;
            case RTK_DBG_MIB_IGR_ACL_DROP:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER16r,
                        CYPRESS_REASON_16f, pCntr);
                break;
            case RTK_DBG_MIB_IGR_METER_DROP:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER17r,
                        CYPRESS_REASON_17f, pCntr);
                break;
            case RTK_DBG_MIB_DFLT_ACTION_FOR_MISS_ACL_AND_C2SC:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER18r,
                        CYPRESS_REASON_18f, pCntr);
                break;
            case RTK_DBG_MIB_NEW_SA:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER19r,
                        CYPRESS_REASON_19f, pCntr);
                break;
            case RTK_DBG_MIB_PORT_MOVE:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER20r,
                        CYPRESS_REASON_20f, pCntr);
                break;
            case RTK_DBG_MIB_SA_BLOCKING:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER21r,
                        CYPRESS_REASON_21f, pCntr);
                break;
            case RTK_DBG_MIB_ROUTING_EXCEPTION:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER22r,
                        CYPRESS_REASON_22f, pCntr);
                break;
            case RTK_DBG_MIB_SRC_PORT_SPENDING_TREE_NON_FWDING:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER23r,
                        CYPRESS_REASON_23f, pCntr);
                break;
            case RTK_DBG_MIB_MAC_LIMIT:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER24r,
                        CYPRESS_REASON_24f, pCntr);
                break;
            case RTK_DBG_MIB_UNKNOW_STORM:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER25r,
                        CYPRESS_REASON_25f, pCntr);
                break;
            case RTK_DBG_MIB_MISS_DROP:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER26r,
                        CYPRESS_REASON_26f, pCntr);
                break;
            case RTK_DBG_MIB_CPU_MAC_DROP:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER27r,
                        CYPRESS_REASON_27f, pCntr);
                break;
            case RTK_DBG_MIB_DA_BLOCKING:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER28r,
                        CYPRESS_REASON_28f, pCntr);
                break;
            case RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_EGR_ACL:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER29r,
                        CYPRESS_REASON_29f, pCntr);
                break;
            case RTK_DBG_MIB_VLAN_EGR_FILTER:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER30r,
                        CYPRESS_REASON_30f, pCntr);
                break;
            case RTK_DBG_MIB_SPANNING_TRE:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER31r,
                        CYPRESS_REASON_31f, pCntr);
                break;
            case RTK_DBG_MIB_PORT_ISOLATION:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER32r,
                        CYPRESS_REASON_32f, pCntr);
                break;
            case RTK_DBG_MIB_OAM_EGRESS_DROP:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER33r,
                        CYPRESS_REASON_33f, pCntr);
                break;
            case RTK_DBG_MIB_MIRROR_ISOLATION:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER34r,
                        CYPRESS_REASON_34f, pCntr);
                break;
            case RTK_DBG_MIB_MAX_LEN_BEFORE_EGR_ACL:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER35r,
                        CYPRESS_REASON_35f, pCntr);
                break;
            case RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER36r,
                        CYPRESS_REASON_36f, pCntr);
                break;
            case RTK_DBG_MIB_MAX_LEN_BEFORE_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER37r,
                        CYPRESS_REASON_37f, pCntr);
                break;
            case RTK_DBG_MIB_SPECIAL_CONGEST_BEFORE_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER38r,
                        CYPRESS_REASON_38f, pCntr);
                break;
            case RTK_DBG_MIB_LINK_STATUS_BEFORE_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER39r,
                        CYPRESS_REASON_39f, pCntr);
                break;
            case RTK_DBG_MIB_WRED_BEFORE_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER40r,
                        CYPRESS_REASON_40f, pCntr);
                break;
            case RTK_DBG_MIB_MAX_LEN_AFTER_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER41r,
                        CYPRESS_REASON_41f, pCntr);
                break;
            case RTK_DBG_MIB_SPECIAL_CONGEST_AFTER_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER42r,
                        CYPRESS_REASON_42f, pCntr);
                break;
            case RTK_DBG_MIB_LINK_STATUS_AFTER_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER43r,
                        CYPRESS_REASON_43f, pCntr);
                break;
            case RTK_DBG_MIB_WRED_AFTER_MIRROR:
                ret = reg_field_read(unit, CYPRESS_STAT_PRVTE_DROP_COUNTER44r,
                        CYPRESS_REASON_44f, pCntr);
                break;
            default:
                break;
        }
		 
        break;

        default:
                break;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    switch (pInfo->chip_id)
    {
        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:

        switch (type)
        {
            case RTK_DBG_MIB_ALE_TX_GOOD_PKTS_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER0r,
                        MAPLE_REASON_0f, pCntr);
                break;
            case RTK_DBG_MIB_MAC_RX_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER1r,
                        MAPLE_REASON_1f, pCntr);
                break;
            case RTK_DBG_MIB_ACL_FWD_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER2r,
                        MAPLE_REASON_2f, pCntr);
                break;
            case RTK_DBG_MIB_HW_ATTACK_PREVENTION_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER3r,
                        MAPLE_REASON_3f, pCntr);
                break;
            case RTK_DBG_MIB_RMA_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER4r,
                        MAPLE_REASON_4f, pCntr);
                break;
            case RTK_DBG_MIB_VLAN_IGR_FLTR_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER5r,
                        MAPLE_REASON_5f, pCntr);
                break;
            case RTK_DBG_MIB_INNER_OUTER_CFI_EQUAL_1_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER6r,
                        MAPLE_REASON_6f, pCntr);
                break;
            case RTK_DBG_MIB_PORT_MOVE_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER7r,
                        MAPLE_REASON_7f, pCntr);
                break;
            case RTK_DBG_MIB_NEW_SA_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER8r,
                        MAPLE_REASON_8f, pCntr);
                break;
            case RTK_DBG_MIB_MAC_LIMIT_SYS_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER9r,
                        MAPLE_REASON_9f, pCntr);
                break;
            case RTK_DBG_MIB_MAC_LIMIT_VLAN_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER10r,
                        MAPLE_REASON_10f, pCntr);
                break;
            case RTK_DBG_MIB_MAC_LIMIT_PORT_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER11r,
                        MAPLE_REASON_11f, pCntr);
                break;
            case RTK_DBG_MIB_SWITCH_MAC_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER12r,
                        MAPLE_REASON_12f, pCntr);
                break;
            case RTK_DBG_MIB_ROUTING_EXCEPTION_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER13r,
                        MAPLE_REASON_13f, pCntr);
                break;
            case RTK_DBG_MIB_DA_LKMISS_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER14r,
                        MAPLE_REASON_14f, pCntr);
                break;
            case RTK_DBG_MIB_RSPAN_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER15r,
                        MAPLE_REASON_15f, pCntr);
                break;
            case RTK_DBG_MIB_ACL_LKMISS_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER16r,
                        MAPLE_REASON_16f, pCntr);
                break;
            case RTK_DBG_MIB_ACL_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER17r,
                        MAPLE_REASON_17f, pCntr);
                break;
            case RTK_DBG_MIB_INBW_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER18r,
                        MAPLE_REASON_18f, pCntr);
                break;
            case RTK_DBG_MIB_IGR_METER_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER19r,
                        MAPLE_REASON_19f, pCntr);
                break;
				
            case RTK_DBG_MIB_ACCEPT_FRAME_TYPE_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER20r,
                        MAPLE_REASON_20f, pCntr);
                break;
            case RTK_DBG_MIB_STP_IGR_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER21r,
                        MAPLE_REASON_21f, pCntr);
                break;
            case RTK_DBG_MIB_INVALID_SA_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER22r,
                        MAPLE_REASON_22f, pCntr);
                break;
            case RTK_DBG_MIB_SA_BLOCKING_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER23r,
                        MAPLE_REASON_23f, pCntr);
                break;
            case RTK_DBG_MIB_DA_BLOCKING_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER24r,
                        MAPLE_REASON_24f, pCntr);
                break;
            case RTK_DBG_MIB_L2_INVALID_DPM_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER25r,
                        MAPLE_REASON_25f, pCntr);
                break;
            case RTK_DBG_MIB_MCST_INVALID_DPM_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER26r,
                        MAPLE_REASON_26f, pCntr);
                break;
            case RTK_DBG_MIB_ROUTE_INVALID_NHP_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER27r,
                        MAPLE_REASON_27f, pCntr);
                break;
            case RTK_DBG_MIB_STORM_SPPRS_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER28r,
                        MAPLE_REASON_28f, pCntr);
                break;
            case RTK_DBG_MIB_LALS_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER29r,
                        MAPLE_REASON_29f, pCntr);
                break;
            case RTK_DBG_MIB_VLAN_EGR_FILTER_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER30r,
                        MAPLE_REASON_30f, pCntr);
                break;
            case RTK_DBG_MIB_STP_EGR_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER31r,
                        MAPLE_REASON_31f, pCntr);
                break;
            case RTK_DBG_MIB_SRC_PORT_FILTER_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER32r,
                        MAPLE_REASON_32f, pCntr);
                break;
            case RTK_DBG_MIB_PORT_ISOLATION_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER33r,
                        MAPLE_REASON_33f, pCntr);
                break;
            case RTK_DBG_MIB_ACL_FLTR_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER34r,
                        MAPLE_REASON_34f, pCntr);
                break;
				
            case RTK_DBG_MIB_MIRROR_FLTR_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER35r,
                        MAPLE_REASON_35f, pCntr);
                break;
            case RTK_DBG_MIB_TX_MAX_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER36r,
                        MAPLE_REASON_36f, pCntr);
                break;
            case RTK_DBG_MIB_LINK_DOWN_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER37r,
                        MAPLE_REASON_37f, pCntr);
                break;
            case RTK_DBG_MIB_FLOW_CONTROL_DROP_RTL8380:
                ret = reg_field_read(unit, MAPLE_STAT_PRVTE_DROP_COUNTER38r,
                        MAPLE_REASON_38f, pCntr);
                break;
				
            default:
                break;
        }

        break;
        default:
                break;
    }
#endif


    return ret;
}
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlIgrPortUsedPageCnt
 * Description:
 *      Get flow control ingress used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pCntr    - value of used page count
 *      pMaxCntr - value of maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_getFlowCtrlIgrPortUsedPageCnt(uint32 unit, rtk_port_t port, uint32 * pCntr, uint32 * pMaxCntr)
{
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    int32 ret = RT_ERR_FAILED;
    uint32 value;
#endif

    hal_control_t *pInfo;

    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8390)
            ret = reg_array_read(unit, CYPRESS_FC_P_USED_PAGE_CNTr, port, REG_ARRAY_INDEX_NONE, &value);

            *pCntr = (value & 0xFFFF);
            *pMaxCntr = (value >> 16);

            return ret;
#endif
        break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8380)
            ret = reg_array_read(unit, MAPLE_FC_P_PAGE_CNTr, port, REG_ARRAY_INDEX_NONE, &value);
            if(ret != RT_ERR_OK)
                return ret;
            *pCntr = (value & 0x3FF);
            

            ret = reg_array_read(unit, MAPLE_FC_P_PAGE_PEAKCNTr, port, REG_ARRAY_INDEX_NONE, &value);
            if(ret != RT_ERR_OK)
                return ret;
            *pMaxCntr = (value & 0x3FF);

            return ret;
#endif
        break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;

}
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlEgrPortUsedPageCnt
 * Description:
 *      Get flow control egress used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pCntr    - value of used page count
 *      pMaxCntr - value of maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_getFlowCtrlEgrPortUsedPageCnt(uint32 unit, rtk_port_t port, uint32 *pCntr, uint32 *pMaxCntr)
{
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    int32 ret = RT_ERR_FAILED;
#endif

#if defined(CONFIG_SDK_RTL8390)
    out_q_entry_t outQ;
#endif

#if defined(CONFIG_SDK_RTL8380)
    uint32 value;
#endif

    hal_control_t *pInfo;

    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8390)
            osal_memset(&outQ, 0, sizeof(outQ));

            if ((ret = table_read(unit, CYPRESS_OUT_Qt, port, (uint32 *) &outQ)) != RT_ERR_OK)
                return ret;
            ret = table_field_get(unit, CYPRESS_OUT_Qt, CYPRESS_OUT_Q_PE_USED_PAGE_CNTtf,
                        pCntr, (uint32 *) &outQ);
            ret = table_field_get(unit, CYPRESS_OUT_Qt, CYPRESS_OUT_Q_PE_MAX_USED_PAGE_CNTtf,
                        pMaxCntr, (uint32 *) &outQ);

            return ret;
#endif
        break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8380)
            ret = reg_array_read(unit, MAPLE_FC_P_EGR_PAGE_CNTr, port, REG_ARRAY_INDEX_NONE, &value);
            *pCntr = (value & 0x3FF);
            *pMaxCntr = (value >> 16) & 0x3FF;

            return ret;
#endif
        break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlSystemUsedPageCnt
 * Description:
 *      Get flow control system used page count of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pCntr    - value of used page count
 *      pMaxCntr - value of maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_getFlowCtrlSystemUsedPageCnt(uint32 unit, uint32 *pCntr, uint32 *pMaxCntr)
{
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    int32 ret = RT_ERR_FAILED;
    uint32 value;
#endif
    
    hal_control_t *pInfo;

    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8390)
            ret = reg_read(unit, CYPRESS_FC_TL_USED_PAGE_CNTr, &value);

            *pCntr = (value & 0xFFFF);
            *pMaxCntr = (value >> 16);

            return ret;
#endif
        break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8380)
            ret = reg_read(unit, MAPLE_FC_GLB_PAGE_CNTr, &value);
            if(ret != RT_ERR_OK)
                return ret;
            *pCntr = (value & 0x3FF);
            

            ret = reg_read(unit, MAPLE_FC_GLB_PAGE_PEAKCNTr, &value);
            if(ret != RT_ERR_OK)
                return ret;
            *pMaxCntr = (value & 0x3FF);

            return ret;
#endif
        break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;    
}
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlPortQueueUsedPageCnt
 * Description:
 *      Get flow control egress queue used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pQCntr    - value of queue used page count
 *      pQMaxCntr - value of queue maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_getFlowCtrlPortQueueUsedPageCnt(uint32 unit, rtk_port_t port, rtk_dbg_queue_usedPageCnt_t *pQCntr, rtk_dbg_queue_usedPageCnt_t *pQMaxCntr)
{
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
    int32 ret = RT_ERR_FAILED;
    rtk_qid_t qid;
#endif

#if defined(CONFIG_SDK_RTL8390)
    out_q_entry_t outQ;
#endif

#if defined(CONFIG_SDK_RTL8380)
    uint32 value;
#endif
   
    hal_control_t *pInfo;

    if ((pInfo = hal_ctrlInfo_get(unit)) == NULL)
    {
        return RT_ERR_FAILED;
    }

    switch (pInfo->chip_id)
    {
        case RTL8352M_CHIP_ID:
        case RTL8353M_CHIP_ID:
        case RTL8391M_CHIP_ID:
        case RTL8392M_CHIP_ID:
        case RTL8393M_CHIP_ID:
        case RTL8396M_CHIP_ID:
        case RTL8352MES_CHIP_ID:
        case RTL8353MES_CHIP_ID:
        case RTL8392MES_CHIP_ID:
        case RTL8393MES_CHIP_ID:
        case RTL8396MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8390)
            osal_memset(&outQ, 0, sizeof(outQ));

            if ((ret = table_read(unit, CYPRESS_OUT_Qt, port, (uint32 *) &outQ)) != RT_ERR_OK)
                return ret;

            for (qid = 0; qid <= (HAL_MAX_NUM_OF_QUEUE(unit) - 1); qid++)
            {
                ret = table_field_get(unit, CYPRESS_OUT_Qt, (uint32)outQ_usedPage_fieldidx[qid],
                            &pQCntr->cntr[qid], (uint32 *) &outQ);
                ret = table_field_get(unit, CYPRESS_OUT_Qt, (uint32)outQ_maxUsedPage_fieldidx[qid],
                            &pQMaxCntr->cntr[qid], (uint32 *) &outQ);
            }

            return ret;
#endif
        break;

        case RTL8380M_CHIP_ID:
        case RTL8330M_CHIP_ID:
        case RTL8382M_CHIP_ID:
        case RTL8332M_CHIP_ID:
        case RTL8380MES_CHIP_ID:
        case RTL8330MES_CHIP_ID:
        case RTL8382MES_CHIP_ID:
        case RTL8332MES_CHIP_ID:
#if defined(CONFIG_SDK_RTL8380)
            for (qid = 0; qid <= (HAL_MAX_NUM_OF_QUEUE(unit) - 1); qid++)
            {
                ret = reg_array_read(unit, MAPLE_FC_PQ_EGR_PAGE_CNT0r+port/8, port, qid, &value);
                if(ret != RT_ERR_OK)
                    return ret;

                pQCntr->cntr[qid] = value & 0x3FF;
                pQMaxCntr->cntr[qid] = (value>>16) & 0x3FF;
            }
            return ret;
#endif
        break;
        default:
            return RT_ERR_FAILED;
    }

    return RT_ERR_OK;
}
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_resetFlowCtrlIgrPortUsedPageCnt
 * Description:
 *      Reset flow control ingress used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_resetFlowCtrlIgrPortUsedPageCnt(uint32 unit, rtk_port_t port)
{
    int32 ret = RT_ERR_FAILED;
    uint32 value = 1;

    ret = reg_array_write(unit, CYPRESS_FC_P_USED_PAGE_CNTr, port, REG_ARRAY_INDEX_NONE, &value);

    return ret;
}
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_resetFlowCtrlEgrPortUsedPageCnt
 * Description:
 *      Reset flow control egress used page count including port and queue of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_resetFlowCtrlEgrPortUsedPageCnt(uint32 unit, rtk_port_t port)
{
    int32 ret = RT_ERR_FAILED;
    uint32 value = 1;
    out_q_entry_t outQ;

    osal_memset(&outQ, 0, sizeof(outQ));

    if ((ret = table_read(unit, CYPRESS_OUT_Qt, port, (uint32 *) &outQ)) != RT_ERR_OK)
        return ret;
    ret = table_field_set(unit, CYPRESS_OUT_Qt, CYPRESS_OUT_Q_PE_USED_PAGE_CNTtf,
                &value, (uint32 *) &outQ);
    if ((ret = table_write(unit, CYPRESS_OUT_Qt, port, (uint32 *) &outQ)) != RT_ERR_OK)
        return ret;

    return ret;
}
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_resetFlowCtrlSystemUsedPageCnt
 * Description:
 *      Reset flow control system used page count of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
int32 hal_resetFlowCtrlSystemUsedPageCnt(uint32 unit)
{
    int32 ret = RT_ERR_FAILED;
    uint32 value = 1;
    
    ret = reg_write(unit, CYPRESS_FC_TL_USED_PAGE_CNTr, &value);

    return ret;
}
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380) 
/* Function Name:
 *      hal_getWatchdogCnt
 * Description:
 *      Display phyWatchdog, serdesWatchdog and PktbufWatchdog.
 * Input:
 *      unit     - unit id
 *      type    - Which Watchdog value 
 * Output:
 *      count   - counter's value
 * Return:
 *      RT_ERR_OK
 */
int32 hal_getWatchdogCnt(uint32 unit, uint32 type, uint32 * count)

{
	int32 ret = RT_ERR_OK;

	switch(type)
	{
		case 0:
			*count = pktBuf_watchdog_cnt;
			break;
		case 1:
			*count = macSerdes_watchdog_cnt;
			break;
		case 2:
			*count = phy_watchdog_cnt;
			break;		
 		case 3:
			*count = fiber_rx_watchdog_cnt;
			break;				
		default:
			osal_printf("\nUnknown Watchdog type(%d)!!!\n", type);

	}
	return ret;
}
#endif 

