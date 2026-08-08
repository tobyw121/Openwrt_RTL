/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 57334 $
 * $Date: 2015-03-30 14:13:45 +0800 (Mon, 30 Mar 2015) $
 *
 * Purpose : Realtek Switch SDK Rtdrv Netfilter Module.
 *
 * Feature : Realtek Switch SDK Rtdrv Netfilter Module
 *
 */

/*
 * Include Files
 */
#include <asm/uaccess.h>
#include <linux/netfilter.h>
#include <common/rt_autoconf.h>
#include <common/rt_error.h>
#include <common/debug/mem.h>
#include <osal/print.h>
#include <hal/mac/mem.h>
#include <ioal/mem32.h>
#include <drv/nic/nic.h>
#include <drv/nic/diag.h>
#include <drv/watchdog/watchdog.h>
#include <drv/rtl8231/rtl8231.h>
#include <drv/gpio/ext_gpio.h>
#if defined(CONFIG_SDK_UART1)
#include <drv/uart/uart.h>
#endif
#include <rtdrv/rtdrv_netfilter.h>
#if (defined(CONFIG_SDK_DRIVER_TEST) || defined(CONFIG_SDK_DRIVER_TEST_MODULE))
#include <sdk/sdk_test.h>
#include <common/unittest_util.h>
#endif
#include <drv/gpio/gpio.h>
#include <drv/smi/smi.h>
#include <drv/smi/ext_smi.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
extern struct nf_sockopt_ops rtdrv_sockopts;

#if defined(CONFIG_SDK_APP_DIAG_EXT)
extern struct nf_sockopt_ops rtdrv_ext_sockopts;

#if defined(CONFIG_SDK_RTL8390)
extern drv_nic_pkt_t *pDiagExtPacket;
#endif

#endif

/*
 * Macro Declaration
 */

/*
 * Function Declaration
 */

/* Function Name:
 *      do_rtdrv_set_ctl
 * Description:
 *      This function is called whenever a process tries to do setsockopt
 * Input:
 *      *sk   - network layer representation of sockets
 *      cmd   - ioctl commands
 *      *user - data buffer handled between user and kernel space
 *      len   - data length
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 do_rtdrv_set_ctl(struct sock *sk, int cmd, void *user, unsigned int len)
{
    int32                           ret = RT_ERR_FAILED;
    rtdrv_union_t                   buf;

    switch(cmd)
    {
    /** INIT **/
        case RTDRV_INIT_RTKAPI:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_init(buf.unit_cfg.unit);
            break;

    /** L2 **/
        case RTDRV_L2_ADDR_DELALL:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_addr_delAll(buf.l2_data.unit, buf.l2_data.static_flag);
            break;

        case RTDRV_L2_ADDR_ADD:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_addr_add(buf.l2_data.unit, &buf.l2_data.data);
            break;

        case RTDRV_L2_ADDR_DEL:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_addr_del(buf.l2_data.unit, buf.l2_data.vid, &buf.l2_data.mac);
            break;

        case RTDRV_L2_AGING_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_l2_aging_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_L2_PORT_AGING_ENABLE_SET:
            copy_from_user(&buf.age_cfg, user, sizeof(rtdrv_l2_ageCfg_t));
            ret = rtk_l2_portAgingEnable_set(buf.unit_cfg.unit, buf.age_cfg.port, buf.age_cfg.enable);
            break;

        case RTDRV_L2_EN_FLUSH_LINK_DOWN_PORT_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_l2_flushLinkDownPortAddrEnable_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_L2_UCASTADDR_FLUSH:
            copy_from_user(&buf.flush_type, user, sizeof(rtdrv_flushType_t));
            ret = rtk_l2_ucastAddr_flush(buf.flush_type.unit, &buf.flush_type.config);
            break;

        case RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ENABLE_SET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLimitLearningCntEnable_set(buf.l2_learn.unit, buf.l2_learn.port, buf.l2_learn.enable);
            break;

        case RTDRV_L2_EN_TRAP_IGMP_CTRL_2CPU_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_igmpCtrlPkt2CpuEnable_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_L2_EN_TRAP_1X_MAC_CHANGE_2CPU_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_1xMacChangePort2CpuEnable_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_L2_EN_TRAP_IP_MCAST_2CPU_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_ipMcastPkt2CpuEnable_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_L2_EN_TRAP_L2_MCAST_2CPU_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_l2McastPkt2CpuEnable_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_L2_IP6_MCAST_ADDR_ADD:
            copy_from_user(&buf.ip6Mcast_data, user, sizeof(rtdrv_l2_ip6McstAddrData_t));
            ret = rtk_l2_ip6McastAddr_add(buf.ip6Mcast_data.unit, &buf.ip6Mcast_data.ip6_m_data);
            break;

        case RTDRV_L2_IP_MCAST_ADDR_ADD:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_ipMcastAddr_add(buf.mcast_data.unit, &buf.mcast_data.ip_m_data);
            break;

        case RTDRV_L2_IP_MCAST_ADDR_DEL:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_ipMcastAddr_del(buf.mcast_data.unit, buf.mcast_data.ip_m_data.sip, buf.mcast_data.ip_m_data.dip,
                                         buf.mcast_data.ip_m_data.rvid);
            break;

        case RTDRV_L2_IP_MCAST_ADDR_SET:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_ipMcastAddr_set(buf.mcast_data.unit, &buf.mcast_data.ip_m_data);
            break;

        case RTDRV_L2_IPMCASTADDR_ADD_WITH_INDEX:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_ipMcastAddr_add_with_index(buf.mcast_data.unit, &buf.mcast_data.ip_m_data);
            break;

        case RTDRV_L2_IP6MCASTMODE_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_l2_ip6mcMode_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_L2_IP6_MCAST_ADDR_DEL:
            copy_from_user(&buf.ip6Mcast_data, user, sizeof(rtdrv_l2_ip6McstAddrData_t));
            ret = rtk_l2_ip6McastAddr_del(buf.ip6Mcast_data.unit, buf.ip6Mcast_data.ip6_m_data.sip, buf.ip6Mcast_data.ip6_m_data.dip,
                                         buf.ip6Mcast_data.ip6_m_data.rvid);
            break;

        case RTDRV_L2_MCAST_ADDR_ADD:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_mcastAddr_add(buf.mcast_data.unit, &buf.mcast_data.m_data);
            break;

        case RTDRV_L2_MCAST_ADDR_DEL:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_mcastAddr_del(buf.mcast_data.unit, buf.mcast_data.m_data.rvid, &buf.mcast_data.m_data.mac);
            break;

        case RTDRV_L2_MCASTADDR_ADD_WITH_INDEX:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_mcastAddr_add_with_index(buf.mcast_data.unit, &(buf.mcast_data.m_data));
            break;

        case RTDRV_L2_MCASTFWDINDEX_FREE:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_mcastFwdIndex_free(buf.mcast_data.unit, buf.mcast_data.fwdIndex);
            break;

        case RTDRV_L2_CPU_MAC_ADDR_ADD:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_cpuMacAddr_add(buf.l2_data.unit, buf.l2_data.vid, &buf.l2_data.mac);
            break;

        case RTDRV_L2_CPU_MAC_ADDR_DEL:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_cpuMacAddr_del(buf.l2_data.unit, buf.l2_data.vid, &buf.l2_data.mac);
            break;

        case RTDRV_L2_LIMIT_LEARNING_CNT_SET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_limitLearningCnt_set(buf.l2_learn.unit, buf.l2_learn.mac_cnt);
            break;

        case RTDRV_L2_PORT_LIMIT_LEARNING_CNT_SET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLimitLearningCnt_set(buf.l2_learn.unit, buf.l2_learn.port, buf.l2_learn.mac_cnt);
            break;

        case RTDRV_L2_LIMIT_LEARNING_CNT_ACTION_SET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_limitLearningCntAction_set(buf.l2_learn.unit, buf.l2_learn.action);
            break;

        case RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ACTION_SET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLimitLearningCntAction_set(buf.l2_learn.unit, buf.l2_learn.port, buf.l2_learn.action);
            break;

        case RTDRV_L2_LOOKUP_MISS_FLOOD_SET:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_lookupMissFloodPortMask_set(buf.l2_data.unit, DLF_TYPE_ANY, &buf.l2_data.portmask);
            break;

        case RTDRV_L2_SRC_PORT_EGR_FILTER_SET:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_srcPortEgrFilterMask_set(buf.l2_data.unit, &buf.l2_data.portmask);
            break;

        case RTDRV_L2_LEGAL_MOVETO_PORTMASK_SET:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_legalMoveToPorts_set(buf.l2_data.unit, buf.l2_data.port, &buf.l2_data.portmask);
            break;

        case RTDRV_L2_LEGAL_MOVETO_ACTION_SET:
            copy_from_user(&buf.l2_action, user, sizeof(rtdrv_l2_portAct_t));
            ret = rtk_l2_legalPortMoveAction_set(buf.l2_action.unit, buf.l2_action.port, buf.l2_action.action);
            break;

        case RTDRV_L2_LEGAL_MOVETO_FLUSH_ENABLE_SET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_legalPortMoveFlushAddrEnable_set(buf.l2_common.unit, buf.l2_common.port, buf.l2_common.value);
            break;

        case RTDRV_L2_ILLEGAL_MOVETO_ACTION_SET:
            copy_from_user(&buf.l2_action, user, sizeof(rtdrv_l2_portAct_t));
            ret = rtk_l2_illegalPortMoveAction_set(buf.l2_action.unit, buf.l2_action.port, buf.l2_action.action);
            break;

        case RTDRV_L2_STTC_PORT_MOVE_ACTION_SET:
            copy_from_user(&buf.l2_action, user, sizeof(rtdrv_l2_portAct_t));
            ret = rtk_l2_staticPortMoveAction_set(buf.l2_action.unit, buf.l2_action.port, buf.l2_action.action);
            break;

        case RTDRV_L2_LOOKUP_MISS_FLOODPORTMASK_IDX_SET:
            copy_from_user(&buf.l2_lkMiss, user, sizeof(rtdrv_l2_lkMiss_t));
            ret = rtk_l2_lookupMissFloodPortMaskIdx_set(buf.l2_lkMiss.unit, buf.l2_lkMiss.type, buf.l2_lkMiss.index);
            break;

        case RTDRV_L2_LOOKUP_MISS_FLOOD_PMSK_SET_WITH_IDX:
            copy_from_user(&buf.l2_lkMiss, user, sizeof(rtdrv_l2_lkMiss_t));
            ret = rtk_l2_lookupMissFloodPortMask_set_with_idx(buf.l2_lkMiss.unit, buf.l2_lkMiss.type, buf.l2_lkMiss.index, &buf.l2_lkMiss.portMask);
            break;

        case RTDRV_L2_LOOKUP_MISS_ACTION_SET:
            copy_from_user(&buf.l2_lkMiss, user, sizeof(rtdrv_l2_lkMiss_t));
            ret = rtk_l2_lookupMissAction_set(buf.l2_lkMiss.unit, buf.l2_lkMiss.type, buf.l2_lkMiss.action);
            break;

        case RTDRV_L2_PORT_LOOKUP_MISS_ACTION_SET:
            copy_from_user(&buf.l2_lkMiss, user, sizeof(rtdrv_l2_lkMiss_t));
            ret = rtk_l2_portLookupMissAction_set(buf.l2_lkMiss.unit, buf.l2_lkMiss.port, buf.l2_lkMiss.type, buf.l2_lkMiss.action);
            break;

        case RTDRV_L2_EXCEPTION_SA_ACTION_SET:
            copy_from_user(&buf.l2_exceptSa, user, sizeof(rtdrv_l2_exceptSa_t));
            ret = rtk_l2_exceptionAddrAction_set(buf.l2_exceptSa.unit, buf.l2_exceptSa.type, buf.l2_exceptSa.action);
            break;

        case RTDRV_L2_LOOKUPMISSPRI_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissPri_set(buf.l2_pri.unit, buf.l2_pri.pri);
            break;

        case RTDRV_L2_LOOKUPMISSPRIENABLE_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissPriEnable_set(buf.l2_pri.unit, buf.l2_pri.enable);
            break;

        case RTDRV_L2_FIDLIMITLEARNINGENTRY_SET :
            copy_from_user(&buf.l2_FidLearn, user, sizeof(rtdrv_l2_learnFidCnt_t));
            ret = rtk_l2_fidLimitLearningEntry_set(buf.l2_FidLearn.unit, buf.l2_FidLearn.entryIdx, &buf.l2_FidLearn.fidMacLimitEntry);
            break;

        case RTDRV_L2_FIDLEARNINGCNT_RESET:
            copy_from_user(&buf.l2_FidLearn, user, sizeof(rtdrv_l2_learnFidCnt_t));
            ret = rtk_l2_fidLearningCnt_reset(buf.l2_FidLearn.unit, buf.l2_FidLearn.entryIdx);
            break;

        case RTDRV_L2_FID_LIMIT_LEARNING_CNT_ACTION_SET:
            copy_from_user(&buf.l2_FidLearn, user, sizeof(rtdrv_l2_learnFidCnt_t));
            ret = rtk_l2_fidLearningCntAction_set(buf.l2_FidLearn.unit, buf.l2_FidLearn.action);
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPPRI_SET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapPri_set(buf.l2_learn_priDp.unit, buf.l2_learn_priDp.priority);
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPPRIENABLE_SET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapPriEnable_set(buf.l2_learn_priDp.unit, buf.l2_learn_priDp.enable);
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPDP_SET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapDP_set(buf.l2_learn_priDp.unit, buf.l2_learn_priDp.dpValue);
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPDPENABLE_SET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapDPEnable_set(buf.l2_learn_priDp.unit, buf.l2_learn_priDp.enable);
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPADDCPUTAGENABLE_SET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapAddCPUTagEnable_set(buf.l2_learn_priDp.unit, buf.l2_learn_priDp.insertCpuTag);
            break;

        case RTDRV_L2_CAMENABLE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_camEnable_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.enable);
            break;

        case RTDRV_L2_HASHALGO_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_hashAlgo_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.hash_algo);
            break;

        case RTDRV_L2_HASHCAREBYTE_SET:
            copy_from_user(&buf.l2_hashCareByte, user, sizeof(rtdrv_l2_hashCareByte_t));
            ret = rtk_l2_ip6CareByte_set(buf.l2_hashCareByte.unit, buf.l2_hashCareByte.type, buf.l2_hashCareByte.value);
            break;

        case RTDRV_L2_VLANMODE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_vlanMode_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.port, buf.l2_learnCfg.vlanMode);
            break;

        case RTDRV_L2_LEARNINGENABLE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_learningEnable_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.port, buf.l2_learnCfg.enable);
            break;

        case RTDRV_L2_NEWMACOP_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_newMacOp_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.port, buf.l2_learnCfg.lrnMode, buf.l2_learnCfg.fwdAction);
            break;

        case RTDRV_L2_LRUENABLE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_LRUEnable_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.enable);
            break;

        case RTDRV_L2_UCASTLOOKUPMODE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ucastLookupMode_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.ucast_lookupMode);
            break;

        case RTDRV_L2_MCASTLOOKUPMODE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_mcastLookupMode_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.mcast_lookupMode, buf.l2_learnCfg.fixed_fid);
            break;

        case RTDRV_L2_IPMCENABLE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ipmcEnable_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.enable);
            break;

        case RTDRV_L2_IPMCMODE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ipmcMode_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.ipmcMode);
            break;

        case RTDRV_L2_IPMC_DIP_CHK_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ipMcastAddrChkEnable_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.dip_check);
            break;

        case RTDRV_L2_IPMC_VLAN_COMPARE_SET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_ipMcstFidVidCompareEnable_set(buf.l2_common.unit, buf.l2_common.value);
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHACTION_SET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchAction_set(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, buf.l2_ipmcCfg.mismatch_action);
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHPRI_SET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchPri_set(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, buf.l2_ipmcCfg.priority);
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHPRIENABLE_SET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchPriEnable_set(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, buf.l2_ipmcCfg.enable);
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHDP_SET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchDP_set(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, buf.l2_ipmcCfg.dpValue);
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHDPENABLE_SET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchDPEnable_set(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, buf.l2_ipmcCfg.enable);
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHADDCPUTAGENABLE_SET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_set(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, buf.l2_ipmcCfg.insertCpuTag);
            break;

        case RTDRV_L2_LOOKUPMISSDP_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissDP_set(buf.l2_pri.unit, buf.l2_pri.dpValue);
            break;

        case RTDRV_L2_LOOKUPMISSDPENABLE_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissDPEnable_set(buf.l2_pri.unit, buf.l2_pri.enable);
            break;

        case RTDRV_L2_LOOKUPMISSADDCPUTAGENABLE_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissAddCPUTagEnable_set(buf.l2_pri.unit, buf.l2_pri.insertCpuTag);
            break;

        case RTDRV_L2_IPMC_ROUTERPORTS_SET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmc_routerPorts_set(buf.l2_ipmcCfg.unit, &buf.l2_ipmcCfg.router_portMask);
            break;

        case RTDRV_L2_TRAPPRI_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_trapPri_set(buf.l2_pri.unit, buf.l2_pri.pri);
            break;

        case RTDRV_L2_TRAPPRIENABLE_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_trapPriEnable_set(buf.l2_pri.unit, buf.l2_pri.enable);
            break;

        case RTDRV_L2_TRAPADDCPUTAGENABLE_SET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_trapAddCPUTagEnable_set(buf.l2_pri.unit, buf.l2_pri.insertCpuTag);
            break;

        case RTDRV_L2_MCASTFWDPORTMASK_SET:
            copy_from_user(&buf.l2_fwdEntryContent, user, sizeof(rtdrv_l2_fwdTblEntry_t));
            ret = rtk_l2_mcastFwdPortmask_set(buf.l2_fwdEntryContent.unit,
                buf.l2_fwdEntryContent.entryIdx, &buf.l2_fwdEntryContent.portMask, buf.l2_fwdEntryContent.crossVlan);
            break;

        case RTDRV_L2_MCASTBLOCKPORTMASK_SET:
            copy_from_user(&buf.l2_portmaskCfg, user, sizeof(rtdrv_l2_portmaskCfg_t));
            ret = rtk_l2_mcastBlockPortmask_set(buf.l2_portmaskCfg.unit, &buf.l2_portmaskCfg.portMask);
            break;

        case RTDRV_L2_ZEROSALEARNINGENABLE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_zeroSALearningEnable_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.enable);
            break;

        case RTDRV_L2_NOTIFICATIONENABLE_SET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_notificationEnable_set(buf.l2_learnCfg.unit, buf.l2_learnCfg.enable);
            break;

        case RTDRV_L2_NOTIFICATIONENABLE_EVENT_TYPE_SET:
            copy_from_user(&buf.l2_notifyEventCfg, user, sizeof(rtdrv_l2_notifyEventCfg_t));
            ret = rtk_l2_notificationEventEnable_set(buf.l2_notifyEventCfg.unit, buf.l2_notifyEventCfg.event, buf.l2_notifyEventCfg.enable);
            break;

        case RTDRV_L2_NOTIFICATION_BACKPRESSURE_THRESH_SET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_notificationBackPressureThresh_set(buf.l2_common.unit, buf.l2_common.value);
            break;

        case RTDRV_L2_SECURE_MAC_MODE_SET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_secureMacMode_set(buf.l2_common.unit, buf.l2_common.value);
            break;

        case RTDRV_L2_PORT_DYNM_PORTMOVE_FORBID_ENABLE_SET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_portDynamicPortMoveForbidEnable_set(buf.l2_common.unit, buf.l2_common.port, buf.l2_common.value);
            break;

        case RTDRV_L2_DYNM_PORTMOVE_FORBID_ACTION_SET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_dynamicPortMoveForbidAction_set(buf.l2_common.unit, buf.l2_common.value);
            break;

        case RTDRV_L2_PORT_MAC_FILTER_ENABLE_SET:
            copy_from_user(&buf.l2_macFilterCfg, user, sizeof(rtdrv_l2_mac_filter_t));
            ret = rtk_l2_portMacFilterEnable_set(buf.l2_macFilterCfg.unit, buf.l2_macFilterCfg.port, buf.l2_macFilterCfg.filterMode, buf.l2_macFilterCfg.enable);
            break;

    /*L3*/
        case RTDRV_L3_INIT :
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_l3_init(buf.unit_cfg.unit);
            break;

        case RTDRV_L3_TTLEXPIREACTION_SET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireAction_set(buf.l3_config.unit, buf.l3_config.type, buf.l3_config.action);
            break;

        case RTDRV_L3_TTLEXPIRETRAPPRI_SET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapPri_set(buf.l3_config.unit, buf.l3_config.type, buf.l3_config.priority);
            break;

        case RTDRV_L3_TTLEXPIRETRAPPRIENABLE_SET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapPriEnable_set(buf.l3_config.unit, buf.l3_config.type, buf.l3_config.dfPri);
            break;

        case RTDRV_L3_TTLEXPIRETRAPDP_SET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapDP_set(buf.l3_config.unit, buf.l3_config.type, buf.l3_config.dpValue);
            break;

        case RTDRV_L3_TTLEXPIRETRAPDPENABLE_SET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapDPEnable_set(buf.l3_config.unit, buf.l3_config.type, buf.l3_config.dfDp);
            break;

        case RTDRV_L3_TTLEXPIREADDCPUTAGENABLE_SET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireAddCPUTagEnable_set(buf.l3_config.unit, buf.l3_config.type, buf.l3_config.insertCpuTag);
            break;

        case RTDRV_L3_ROUTE_ROUTEENTRY_SET:
            copy_from_user(&buf.l3_route_entry, user, sizeof(rtdrv_l3_routeEntry_t));
            ret = rtk_l3_routeEntry_set(buf.l3_route_entry.unit, buf.l3_route_entry.index, &buf.l3_route_entry.entry);
            break;

        case RTDRV_L3_ROUTE_SWITCHMACADDR_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_l3_routeSwitchMacAddr_set(buf.l3_config.unit, buf.l3_config.index, &buf.l3_config.mac);
            break;

    /** PORT **/
        case RTDRV_PORT_EN_AUTONEGO_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyAutoNegoEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_AUTONEGO_ABIL_SET:
            copy_from_user(&buf.autonego_ability, user, sizeof(rtdrv_port_autoNegoAbility_t));
            ret = rtk_port_phyAutoNegoAbility_set(buf.autonego_ability.unit, buf.autonego_ability.port,
                                                  &buf.autonego_ability.ability);
            break;

        case RTDRV_PORT_FORCE_MODE_ABIL_SET:
            copy_from_user(&buf.forcemode_ability, user, sizeof(rtdrv_port_forceModeAbility_t));
            ret = rtk_port_phyForceModeAbility_set(buf.forcemode_ability.unit, buf.forcemode_ability.port,
                                                   buf.forcemode_ability.speed, buf.forcemode_ability.duplex,
                                                   buf.forcemode_ability.flowctrl);
            break;

        case RTDRV_PORT_PHY_REG_SET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyReg_set(buf.phy_data.unit, buf.phy_data.port, buf.phy_data.page,
                                      buf.phy_data.reg, buf.phy_data.data);
            break;

        case RTDRV_PORT_PHY_REG_BROADCAST_SET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyReg_broadcast_set(buf.phy_data.unit, buf.phy_data.page, buf.phy_data.reg, buf.phy_data.data);
            break;

        case RTDRV_PORT_PHY_REG_BROADCAST_ID_SET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyReg_broadcastID_set(buf.phy_data.unit, buf.phy_data.broadcastID);
            break;

        case RTDRV_PORT_PHY_EXT_PARK_PAGE_REG_SET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyExtParkPageReg_set(buf.phy_data.unit, buf.phy_data.port, buf.phy_data.page,
                                      buf.phy_data.extPage, buf.phy_data.parkPage, buf.phy_data.reg, buf.phy_data.data);
            break;

        case RTDRV_PORT_PHYMASK_EXT_PARK_PAGE_REG_SET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phymaskExtParkPageReg_set(buf.phy_data.unit, &buf.phy_data.portmask, buf.phy_data.page,
                                      buf.phy_data.extPage, buf.phy_data.parkPage, buf.phy_data.reg, buf.phy_data.data);
            break;

        case RTDRV_PORT_PHY_MMD_REG_SET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyMmdReg_set(buf.phy_data.unit, buf.phy_data.port, buf.phy_data.mmdAddr,
                                      buf.phy_data.reg, buf.phy_data.data);
            break;

        case RTDRV_PORT_PHYMASK_MMD_REG_SET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phymaskMmdReg_set(buf.phy_data.unit, &buf.phy_data.portmask, buf.phy_data.mmdAddr,
                                      buf.phy_data.reg, buf.phy_data.data);
            break;

        case RTDRV_PORT_MASTER_SLAVE_SET:
            copy_from_user(&buf.masterSlave_cfg, user, sizeof(rtdrv_port_masterSlave_t));
            ret = rtk_port_phyMasterSlave_set(buf.masterSlave_cfg.unit, buf.masterSlave_cfg.port, buf.masterSlave_cfg.masterSlaveCfg);
            break;

        case RTDRV_PORT_ISOLATION_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_isolation_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.portmask);
            break;

         case RTDRV_PORT_ISOLATION_ADD:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_isolation_add(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.targetPort);
            break;

        case RTDRV_PORT_ISOLATION_DEL:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_isolation_del(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.targetPort);
            break;

        case RTDRV_PORT_EN_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_adminEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_BACK_PRESSURE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_backpressureEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_PHY_MEDIA_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyComboPortMedia_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.media);
            break;

        case RTDRV_PORT_GREEN_ENABLE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_greenEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

		case RTDRV_PORT_GIGA_LITE_ENABLE_SET:
		    copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
	    	ret = rtk_port_gigaLiteEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
	    break;

        case RTDRV_PORT_UDLDENABLE_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldEnable_set(buf.udld_cfg.unit, buf.udld_cfg.port, buf.udld_cfg.enable);
            break;

        case RTDRV_PORT_UDLDLINKUPAUTOTRIGGERENABLE_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldLinkUpAutoTriggerEnable_set(buf.udld_cfg.unit, buf.udld_cfg.autoTriggerEnable);
            break;

        case RTDRV_PORT_UDLDTRIGGER_START:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldTrigger_start(buf.udld_cfg.unit, buf.udld_cfg.port);
            break;

        case RTDRV_PORT_UDLDECHOACTION_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldEchoAction_set(buf.udld_cfg.unit, buf.udld_cfg.action);
            break;

        case RTDRV_PORT_UDLDLINKSTATUS_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldLinkStatus_set(buf.udld_cfg.unit, buf.udld_cfg.port, buf.udld_cfg.linkStatus);
            break;

        case RTDRV_PORT_UDLDAUTODISABLEFAILEDPORTENABLE_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldAutoDisableFailedPortEnable_set(buf.udld_cfg.unit, buf.udld_cfg.enable);
            break;

        case RTDRV_PORT_UDLDINTERVAL_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldInterval_set(buf.udld_cfg.unit, buf.udld_cfg.interval);
            break;

        case RTDRV_PORT_UDLDRETRYCOUNT_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldRetryCount_set(buf.udld_cfg.unit, buf.udld_cfg.retryCount);
            break;

        case RTDRV_PORT_UDLDLEDINDICATEENABLE_SET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldLedIndicateEnable_set(buf.udld_cfg.unit, buf.udld_cfg.enable);
            break;

        case RTDRV_PORT_RLDPENABLE_SET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpEnable_set(buf.rldp_cfg.unit, buf.rldp_cfg.port, buf.rldp_cfg.enable);
            break;

        case RTDRV_PORT_RLDPAUTOBLOCKENABLE_SET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpAutoBlockEnable_set(buf.rldp_cfg.unit, buf.rldp_cfg.port, buf.rldp_cfg.enable);
            break;

        case RTDRV_PORT_RLDPINTERVAL_SET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpInterval_set(buf.rldp_cfg.unit, buf.rldp_cfg.port, buf.rldp_cfg.interval);
            break;

        case RTDRV_PORT_RLDPSELFLOOPAGINGTIME_SET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpSelfLoopAgingTime_set(buf.rldp_cfg.unit, buf.rldp_cfg.port, buf.rldp_cfg.agingTime);
            break;

        case RTDRV_PORT_RLDPNORMALLOOPAGINGTIME_SET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpNormalLoopAgingTime_set(buf.rldp_cfg.unit, buf.rldp_cfg.port, buf.rldp_cfg.agingTime);
            break;

        case RTDRV_PORT_PHY_CROSSOVERMODE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyCrossOverMode_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_TX_EN_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_txEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_RX_EN_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_rxEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_PHY_FIBER_MEDIA_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyComboPortFiberMedia_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.fiber_media);
            break;

        case RTDRV_PORT_LINKDOWN_POWERSAVING_ENABLE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_linkDownPowerSavingEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_VLAN_ISOLATION_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_vlanBasedIsolationEntry_set(buf.port_cfg.unit, buf.port_cfg.index, &buf.port_cfg.vlanIsoEntry);
            break;

        case RTDRV_PORT_VLAN_ISOLATION_VLANSOURCE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_vlanBasedIsolation_vlanSource_set(buf.port_cfg.unit, buf.port_cfg.vlanIsoSrc);
            break;
        case RTDRV_PORT_DOWNSPEEDENABLE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_downSpeedEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;
        case RTDRV_PORT_FIBERDOWNSPEEDENABLE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_fiberDownSpeedEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;
        case RTDRV_PORT_FIBERNWAYFORCELINKENABLE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_fiberNwayForceLinkEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;
        case RTDRV_PORT_FIBEROAMLOOPBACKENABLE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_fiberOAMLoopBackEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;
        case RTDRV_PORT_10GMEDIA_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_10gMedia_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.media_10g);
            break;
        case RTDRV_PORT_FIBERINTERNALLOOPBACKENABLE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_fiberInternalLoopBackEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_PORT_10GSDS_RESTART:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_10gSds_restart(buf.port_cfg.unit, buf.port_cfg.port);
            break;

        case RTDRV_PORT_10G_INIT:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_10g_init(buf.port_cfg.unit, buf.port_cfg.port);
            break;

    /*OAM*/
        case RTDRV_OAM_INIT :
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_oam_init(buf.port_cfg.unit);
            break;

        case RTDRV_OAM_TXTESTFRAME_START:
            copy_from_user(&buf.spd_cfg, user, sizeof(rtdrv_oamSpgCfg_t));
            ret = rtk_oam_txTestFrame_start(buf.spd_cfg.unit, &buf.spd_cfg.testFrameCfg);
            break;

        case RTDRV_OAM_TXTESTFRAME_STOP:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_oam_txTestFrame_stop(buf.port_cfg.unit);
            break;

        case RTDRV_OAM_LOOPBACKMODE_SET:
            copy_from_user(&buf.loopback_cfg, user, sizeof(rtdrv_oamLoopbackCfg_t));
            ret = rtk_oam_loopbackMode_set(buf.loopback_cfg.unit, buf.loopback_cfg.port, buf.loopback_cfg.lpbackMode);
            break;

        case RTDRV_OAM_LOOPBACKCTRL_SET:
            copy_from_user(&buf.loopback_cfg, user, sizeof(rtdrv_oamLoopbackCfg_t));
            ret = rtk_oam_loopbackCtrl_set(buf.loopback_cfg.unit, buf.loopback_cfg.port, &buf.loopback_cfg.lpbackCtrl);
            break;

        case RTDRV_OAM_PORTDYINGGASPPAYLOAD_SET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_portDyingGaspPayload_set(buf.dyingGasp_cfg.unit,
                    buf.dyingGasp_cfg.port, buf.dyingGasp_cfg.payload,
                    buf.dyingGasp_cfg.cnt);
            break;

        case RTDRV_OAM_DYINGGASPSEND_START:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_DyingGaspSend_start(buf.dyingGasp_cfg.unit, &buf.dyingGasp_cfg.portMask);
            break;

        case RTDRV_OAM_DYINGGASPSEND_SET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_dyingGaspSend_set(buf.dyingGasp_cfg.unit, buf.dyingGasp_cfg.enable);
            break;

        case RTDRV_OAM_AUTODYINGGASPENABLE_SET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_autoDyingGaspEnable_set(buf.dyingGasp_cfg.unit, buf.dyingGasp_cfg.port, buf.dyingGasp_cfg.enable);
            break;

        case RTDRV_OAM_DYINGGASPTLV_SET :
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_dyingGaspTLV_set(buf.dyingGasp_cfg.unit, buf.dyingGasp_cfg.port, &buf.dyingGasp_cfg.tlv);
            break;

        case RTDRV_OAM_DYINGGASPWAITTIME_SET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_dyingGaspWaitTime_set(buf.dyingGasp_cfg.unit, buf.dyingGasp_cfg.waitTime);
            break;

        case RTDRV_OAM_DYINGGASPPKTCNT_SET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_dyingGaspPktCnt_set(buf.dyingGasp_cfg.unit, buf.dyingGasp_cfg.cnt);
            break;

        case RTDRV_OAM_CFMENTRY_SET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmEntry_set(buf.cfm_cfg.unit, buf.cfm_cfg.cfmIdx, &buf.cfm_cfg.cfmCfg);
            break;

        case RTDRV_OAM_CFMPORTENTRY_SET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmPortEntry_set(buf.cfm_cfg.unit, buf.cfm_cfg.port, &buf.cfm_cfg.portCfg);
            break;

        case RTDRV_OAM_CFMMEPENABLE_SET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmMepEnable_set(buf.cfm_cfg.unit, buf.cfm_cfg.port, buf.cfm_cfg.enable);
            break;

        case RTDRV_OAM_TXCCMFRAME_START:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_txCCMFrame_start(buf.dyingGasp_cfg.unit, &buf.dyingGasp_cfg.portMask);
            break;

        case RTDRV_OAM_TXCCMFRAME_STOP:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_txCCMFrame_stop(buf.dyingGasp_cfg.unit);
            break;

        case RTDRV_OAM_CFMCCMFRAME_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMFrame_set(buf.ccm_cfg.unit, buf.ccm_cfg.cfmIdx, &buf.ccm_cfg.ccmFrame);
            break;

        case RTDRV_OAM_CFMCCMSNAPOUI_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMSnapOui_set(buf.ccm_cfg.unit, &buf.ccm_cfg.snapoui);
            break;

        case RTDRV_OAM_CFMCCMETYPE_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMEtype_set(buf.ccm_cfg.unit, buf.ccm_cfg.etherType);
            break;

        case RTDRV_OAM_CFMCCMOPCODE_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMOpcode_set(buf.ccm_cfg.unit, buf.ccm_cfg.opCode);
            break;

        case RTDRV_OAM_CFMCCMFLAG_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMFlag_set(buf.ccm_cfg.unit, buf.ccm_cfg.cfmIdx, buf.ccm_cfg.ccmFlag);
            break;

        case RTDRV_OAM_CFMCCMINTERVAL_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMInterval_set(buf.ccm_cfg.unit, buf.ccm_cfg.ccmInterval);
            break;

        case RTDRV_OAM_CFMINTFSTATUS_SET :
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmIntfStatus_set(buf.misc_cfg.unit, buf.misc_cfg.port, buf.misc_cfg.status);
            break;

        case RTDRV_OAM_CFMPORTSTATUS_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmPortStatus_set(buf.misc_cfg.unit, buf.misc_cfg.port, buf.misc_cfg.status);
            break;

        case RTDRV_OAM_CFMREMOTEMEP_DEL:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmRemoteMep_del(buf.misc_cfg.unit, buf.misc_cfg.mepid);
            break;

        case RTDRV_OAM_CFMREMOTEMEP_ADD:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmRemoteMep_add(buf.misc_cfg.unit, buf.misc_cfg.mepid);
            break;

        case RTDRV_OAM_CFMCCSTATUS_RESET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmCCStatus_reset(buf.misc_cfg.unit, buf.misc_cfg.port);
            break;

        case RTDRV_OAM_CFMLOOPBACKREPLYENABLE_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmLoopbackReplyEnable_set(buf.misc_cfg.unit, buf.misc_cfg.port, buf.misc_cfg.loopbackEnable);
            break;

        case RTDRV_OAM_CFMLOOPBACKREPLYCTRL_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmLoopbackReplyCtrl_set(buf.misc_cfg.unit, &buf.misc_cfg.ctrl);
            break;

        case RTDRV_OAM_LOOPBACKMACSWAPENABLE_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_loopbackMacSwapEnable_set(buf.misc_cfg.unit,
                                                    buf.misc_cfg.loopbackEnable);
            break;

        case RTDRV_OAM_PORTLOOPBACKMUXACTION_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_portLoopbackMuxAction_set(buf.misc_cfg.unit,
                    buf.misc_cfg.port, buf.misc_cfg.action);
            break;

        case RTDRV_OAM_CFMCCMPCP_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmPcp_set(buf.ccm_cfg.unit,
                                        buf.ccm_cfg.ccmFrame.outer_pri);
            break;

        case RTDRV_OAM_CFMCCMCFI_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmCfi_set(buf.ccm_cfg.unit,
                                        buf.ccm_cfg.ccmFrame.outer_dei);
            break;

        case RTDRV_OAM_CFMCCMTPID_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmTpid_set(buf.ccm_cfg.unit,
                                         buf.ccm_cfg.ccmFrame.outer_tpid);
            break;

        case RTDRV_OAM_CFMCCMRESETLIFETIME_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmInstLifetime_set(buf.ccm_cfg.unit,
                                                  buf.ccm_cfg.cfmIdx,
                                                  buf.ccm_cfg.ccmFlag);
            break;

        case RTDRV_OAM_CFMCCMMEPID_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmCcmMepid_set(buf.misc_cfg.unit,
                                          buf.misc_cfg.mepid);
            break;

        case RTDRV_OAM_CFMCCMINTERVALFIELD_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmIntervalField_set(buf.ccm_cfg.unit,
                                                  buf.ccm_cfg.ccmFlag);
            break;

        case RTDRV_OAM_CFMCCMMDL_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmMdl_set(buf.cfm_cfg.unit,
                                        buf.cfm_cfg.cfmCfg.md_level);
            break;

        case RTDRV_OAM_CFMCCMINSTTAGSTATUS_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmInstTagStatus_set(buf.cfm_cfg.unit,
                                                  buf.cfm_cfg.cfmIdx,
                                                  buf.cfm_cfg.enable);
            break;

        case RTDRV_OAM_CFMCCMINSTVID_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmInstVid_set(buf.ccm_cfg.unit,
                                            buf.ccm_cfg.cfmIdx,
                                            buf.ccm_cfg.ccmFrame.outer_vid);
            break;

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_OAM_CFMCCMINSTMAID_SET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmInstMaid_set(buf.cfm_cfg.unit,
                                             buf.cfm_cfg.cfmIdx,
                                             buf.cfm_cfg.maid);
            break;
#endif

        case RTDRV_OAM_CFMCCMINSTTXSTATUS_SET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmInstTxStatus_set(buf.cfm_cfg.unit,
                                                 buf.cfm_cfg.cfmIdx,
                                                 buf.cfm_cfg.enable);
            break;

        case RTDRV_OAM_CFMCCMINSTINTERVAL_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmInstInterval_set(buf.ccm_cfg.unit,
                                                 buf.ccm_cfg.cfmIdx,
                                                 buf.ccm_cfg.ccmInterval);
            break;

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_OAM_CFMCCMTXINSTPORT_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmTxInstPort_set(buf.ccm_cfg.unit,
                                               buf.ccm_cfg.cfmIdx,
                                               buf.ccm_cfg.portIdx,
                                               buf.ccm_cfg.port);
            break;
#endif

        case RTDRV_OAM_CFMCCMRXINSTVID_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmRxInstVid_set(buf.ccm_cfg.unit,
                                              buf.ccm_cfg.cfmIdx,
                                              buf.ccm_cfg.ccmFrame.outer_vid);
            break;

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_OAM_CFMCCMRXINSTPORT_SET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmRxInstPort_set(buf.ccm_cfg.unit,
                                               buf.ccm_cfg.cfmIdx,
                                               buf.ccm_cfg.portIdx,
                                               buf.ccm_cfg.port);
            break;
#endif

        case RTDRV_OAM_CFMETHDMPORTENABLE_SET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmPortEthDmEnable_set(buf.cfm_cfg.unit,
                                                 buf.cfm_cfg.port,
                                                 buf.cfm_cfg.enable);
            break;

        case RTDRV_OAM_LINKFAULTMONENABLE_SET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_linkFaultMonEnable_set(buf.cfm_cfg.enable);
            break;
    /** VLAN **/
        case RTDRV_VLAN_PORT_SET:
            copy_from_user(&buf.vlan_port_data, user, sizeof(rtdrv_vlan_port_t));
            ret = rtk_vlan_port_set(buf.vlan_port_data.unit, buf.vlan_port_data.vid, &buf.vlan_port_data.member,
                                    &buf.vlan_port_data.untag);
            break;

        case RTDRV_VLAN_PORT_PVID_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portPvid_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_VLAN_PORT_OUTER_PVID_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portOuterPvid_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_VLAN_PROTO_GROUP_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_protoGroup_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &(buf.vlan_cfg.protoGroup));
            break;

        case RTDRV_VLAN_PORT_PROTO_VLAN_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portProtoVlan_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.idx, &(buf.vlan_cfg.protoVlanCfg));
            break;

        case RTDRV_VLAN_PORT_OUTER_PROTO_VLAN_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portOuterProtoVlan_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.idx, &(buf.vlan_cfg.protoVlanCfg));
            break;

        case RTDRV_VLAN_PORT_TPID_ENTRY_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portTpidEntry_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.idx, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_TPID_MODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTpidMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_IGR_INNER_TPID_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrInnerTpid_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_TPID_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTpid_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_TPID_MODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTpidMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_IGR_OUTER_TPID_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrOuterTpid_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_TPID_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTpid_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_IGR_EXTRA_TPID_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrExtraTpid_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_IGR_IGNORE_INNER_TAG_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrIgnoreInnerTagEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_IGR_IGNORE_OUTER_TAG_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrIgnoreOuterTagEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_TAG_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTagEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_TAG_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTagEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_IGR_EXTRA_TAG_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrExtraTagEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_EXTRA_TAG_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrExtraTagEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_VID_SOURCE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerVidSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_PRI_SOURCE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerPriSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_VID_SOURCE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterVidSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_PRI_SOURCE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterPriSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_IGR_TAG_KEEP_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrTagKeepEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data, buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_PORT_EGR_TAG_KEEP_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrTagKeepEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data, buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_FWD_MODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_fwdMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.vid, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_EN_PORT_IGR_FILTER_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portIgrFilterEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_VLAN_TAG_MODE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_tagMode_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_VLAN_PORT_ACCEPT_FRAME_TYPE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portAcceptFrameType_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_VLAN_PORT_OUTER_ACCEPT_FRAME_TYPE_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portOuterAcceptFrameType_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_VLAN_EN_MCAST_LEAKY_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_vlan_mcastLeakyEnable_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_VLAN_EN_PORT_MCAST_LEAKY_SET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_mcastLeakyPortEnable_set(buf.port_cfg.unit, buf.port_cfg.port, buf.port_cfg.data);
            break;

        case RTDRV_VLAN_STG_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_stg_set(buf.vlan_cfg.unit, buf.vlan_cfg.vid, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_FID_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_fid_set(buf.vlan_cfg.unit, buf.vlan_cfg.vid, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_CREATE:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_create(buf.vlan_cfg.unit, buf.vlan_cfg.vid);
            break;

        case RTDRV_VLAN_DESTROY:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_destroy(buf.vlan_cfg.unit, buf.vlan_cfg.vid);
            break;

        case RTDRV_VLAN_DESTROY_ALL:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_vlan_destroyAll(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;

        case RTDRV_VLAN_EN_IGR_FILTER_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_igrFilterEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGR_FILTER_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrFilterEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_vlanFunctionEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_ADD:
            copy_from_user(&buf.vlan_port_data, user, sizeof(rtdrv_vlan_port_t));
            ret = rtk_vlan_port_add(buf.vlan_port_data.unit, buf.vlan_port_data.vid, buf.vlan_port_data.port,
                                    buf.vlan_port_data.is_untag);
            break;

        case RTDRV_VLAN_PORT_DEL:
            copy_from_user(&buf.vlan_port_data, user, sizeof(rtdrv_vlan_port_t));
            ret = rtk_vlan_port_del(buf.vlan_port_data.unit, buf.vlan_port_data.vid, buf.vlan_port_data.port);
            break;

        case RTDRV_VLAN_UCAST_LUTMODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_l2UcastLookupMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.vid, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_MCAST_LUTMODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_l2McastLookupMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.vid, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PROFILE_IDX_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_profileIdx_set(buf.vlan_cfg.unit, buf.vlan_cfg.vid, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PROFILE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_profile_set(buf.vlan_cfg.unit, buf.vlan_cfg.data, &buf.vlan_cfg.profile);
            break;

        case RTDRV_VLAN_PORT_IGR_FILTER_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrFilter_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_PVID_MODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portPvidMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_OPVID_MODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portOuterPvidMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_MAC_BASED_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_macBasedVlan_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    buf.vlan_cfg.data, &buf.vlan_cfg.mac, buf.vlan_cfg.vid,
                    buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_MAC_BASED_WITH_MSK_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_macBasedVlanWithMsk_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    buf.vlan_cfg.data, &buf.vlan_cfg.mac, &buf.vlan_cfg.msk, buf.vlan_cfg.vid,
                    buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_MAC_BASED_WITH_PORT_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_macBasedVlanWithPort_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    buf.vlan_cfg.data, &buf.vlan_cfg.mac, &buf.vlan_cfg.msk,
                    buf.vlan_cfg.port, buf.vlan_cfg.port_msk, buf.vlan_cfg.vid, buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_IP_SUBNET_BASED_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_ipSubnetBasedVlan_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    buf.vlan_cfg.data, buf.vlan_cfg.sip, buf.vlan_cfg.sip_msk,
                    buf.vlan_cfg.vid, buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_IP_SUBNET_BASED_WITH_PORT_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_ipSubnetBasedVlanWithPort_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    buf.vlan_cfg.data, buf.vlan_cfg.sip, buf.vlan_cfg.sip_msk,
                    buf.vlan_cfg.port, buf.vlan_cfg.port_msk, buf.vlan_cfg.vid, buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_ITPID_ENTRY_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_innerTpidEntry_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_OTPID_ENTRY_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_outerTpidEntry_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_ETPID_ENTRY_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_extraTpidEntry_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_EGR_ITAG_STS_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTagSts_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_EGR_OTAG_STS_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTagSts_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_IGRVLANCNVT_BLKMODE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_igrVlanCnvtBlkMode_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_IGRVLANCNVT_ENTRY_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_igrVlanCnvtEntry_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.igrCnvtEntry);
            break;

        case RTDRV_VLAN_EGRVLANCNVT_DBLTAG_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtDblTagEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_EGRVLANCNVT_VIDSRC_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtVidSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_EGRVLANCNVT_ENTRY_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtEntry_set(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.egrCnvtEntry);
            break;

        case RTDRV_VLAN_PORT_VLANAGGR_ENABLE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portVlanAggrEnable_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_LEAKYSTPFILTER_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_leakyStpFilter_set(buf.vlan_cfg.unit, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_EXCEPT_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_except_set(buf.vlan_cfg.unit, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORTIGRCNVTDFLTACT_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrCnvtDfltAct_set(buf.vlan_cfg.unit,
                    buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_IGRVLANCNVTENTRY_DELALL:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_igrVlanCnvtEntry_delAll(buf.vlan_cfg.unit);
            break;

        case RTDRV_VLAN_EGRVLANCNVTENTRY_DELALL:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtEntry_delAll(buf.vlan_cfg.unit);
            break;

        case RTDRV_VLAN_EGRVLANCNVTRANGECHECKVID_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtRangeCheckVid_set(buf.vlan_cfg.unit,
                    buf.vlan_cfg.idx, &buf.vlan_cfg.egrRangeCheck);
            break;

        case RTDRV_VLAN_PORT_IGRTAGKEEPTYPE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrTagKeepType_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data, buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_PORT_EGRTAGKEEPTYPE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrTagKeepType_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data, buf.vlan_cfg.data1);
            break;

        case RTDRV_VLAN_PORT_VLANAGGRVIDSOURCE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portVlanAggrVidSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_VLANAGGRPRITAGVIDSOURCE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portVlanAggrPriTagVidSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGRVLANCNVTVIDSOURCE_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrVlanCnvtVidSource_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGRVLANCNVTVIDTARGET_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrVlanCnvtVidTarget_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;

        case RTDRV_VLAN_PORT_EGRVLANCNVTLOOKUPMISSACT_SET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrVlanCnvtLookupMissAct_set(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.data);
            break;


    /** STP **/
        case RTDRV_STP_MSTP_STATE_SET:
            copy_from_user(&buf.stp_cfg, user, sizeof(rtdrv_stpCfg_t));
            ret = rtk_stp_mstpState_set(buf.stp_cfg.unit, buf.stp_cfg.msti, buf.stp_cfg.port, buf.stp_cfg.stp_state);
            break;

        case RTDRV_STP_MSTP_INSTANCE_CREATE:
            copy_from_user(&buf.stp_cfg, user, sizeof(rtdrv_stpCfg_t));
            ret = rtk_stp_mstpInstance_create(buf.stp_cfg.unit, buf.stp_cfg.msti);
            break;

        case RTDRV_STP_MSTP_INSTANCE_DESTROY:
            copy_from_user(&buf.stp_cfg, user, sizeof(rtdrv_stpCfg_t));
            ret = rtk_stp_mstpInstance_destroy(buf.stp_cfg.unit, buf.stp_cfg.msti);
            break;

        case RTDRV_STP_MSTP_MODE_SET:
            copy_from_user(&buf.stp_cfg, user, sizeof(rtdrv_stpCfg_t));
            ret = rtk_stp_mstpInstanceMode_set(buf.stp_cfg.unit, buf.stp_cfg.msti_mode);
            break;

    /** REG **/
        case RTDRV_REG_REGISTER_SET:
            copy_from_user(&buf.reg_cfg, user, sizeof(rtdrv_regCfg_t));
            ioal_mem32_write(buf.reg_cfg.unit, buf.reg_cfg.reg, buf.reg_cfg.value);
            ret = RT_ERR_OK; /*xxx_reg_register_set(buf.reg_cfg.unit, buf.reg_cfg.reg, buf.reg_cfg.value);*/
            break;

        case RTDRV_TABLE_WRITE:
            copy_from_user(&buf.tbl_cfg, user, sizeof(rtdrv_tblCfg_t));
            ret = table_write(buf.tbl_cfg.unit, buf.tbl_cfg.table, buf.tbl_cfg.addr, buf.tbl_cfg.value);
            break;


    /** COUNTER **/
        case RTDRV_COUNTER_GLOBAL_RESET:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_global_reset(buf.counter_cfg.unit);
            break;

        case RTDRV_COUNTER_PORT_RESET:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_port_reset(buf.counter_cfg.unit, buf.counter_cfg.port);
            break;

        case RTDRV_COUNTER_TAGLENCNT_SET:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_tagLenCntIncEnable_set(buf.counter_cfg.unit, buf.counter_cfg.tagCnt_type, buf.counter_cfg.enable);
            break;

    /** TIME **/
        case RTDRV_TIME_PORT_PTP_ENABLE_SET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_portPtpEnable_set(buf.time_cfg.unit, buf.time_cfg.port, buf.time_cfg.enable);
            break;

        case RTDRV_TIME_REF_TIME_SET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_refTime_set(buf.time_cfg.unit, buf.time_cfg.timeStamp);
            break;

        case RTDRV_TIME_REF_TIME_ADJUST_SET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_refTimeAdjust_set(buf.time_cfg.unit, buf.time_cfg.sign, buf.time_cfg.timeStamp);
            break;

        case RTDRV_TIME_REF_TIME_ENABLE_SET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_refTimeEnable_set(buf.time_cfg.unit, buf.time_cfg.enable);
            break;

    /** TRAP **/
        case RTDRV_TRAP_RMAACTION_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaAction_set(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, buf.trap_cfg.rma_action);
            break;

        case RTDRV_TRAP_RMAPRI_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaPri_set(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, buf.trap_cfg.priority);
            break;

        case RTDRV_TRAP_RMAPRIENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaPriEnable_set(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_RMACPUTAGADDENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaCpuTagAddEnable_set(buf.trap_cfg.unit, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_RMAVLANCHECKENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaVlanCheckEnable_set(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_BYPASS_STP_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_bypassStp_set(buf.trap_cfg.unit, buf.trap_cfg.bypassStp_frame, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_BYPASS_VLAN_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_bypassVlan_set(buf.trap_cfg.unit, buf.trap_cfg.bypassVlan_frame, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_USERDEFINERMA_SET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRma_set(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.rma_frame);
            break;

        case RTDRV_TRAP_USERDEFINERMAENABLE_SET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaEnable_set(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, buf.l2_trap_cfg.enable);
            break;

        case RTDRV_TRAP_USERDEFINERMAACTION_SET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaAction_set(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, buf.l2_trap_cfg.rma_action);
            break;

        case RTDRV_TRAP_USERDEFINERMAPRI_SET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaPri_set(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, buf.l2_trap_cfg.priority);
            break;

        case RTDRV_TRAP_USERDEFINERMAPRIENABLE_SET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaPriEnable_set(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, buf.l2_trap_cfg.enable);
            break;

        case RTDRV_TRAP_USERDEFINERMAVLANCHECKENABLE_SET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaVlanCheckEnable_set(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, buf.l2_trap_cfg.vlanCheck);
            break;

        case RTDRV_TRAP_USERDEFINERMASTPBLOCKENABLE_SET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaStpBlockEnable_set(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, buf.l2_trap_cfg.stpBlock);
            break;

        case RTDRV_TRAP_MGMTFRAMEACTION_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFrameAction_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.rma_action);
            break;

        case RTDRV_TRAP_MGMTFRAMEPRI_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFramePri_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.priority);
            break;

        case RTDRV_TRAP_MGMTFRAMEPRIENABLE_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFramePriEnable_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.enable);
            break;

        case RTDRV_TRAP_MGMTFRAMEVLANCHECK_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFrameVlanCheck_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.vlanCheck);
            break;

        case RTDRV_TRAP_USERDEFINEMGMT_SET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmt_set(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, &buf.mgmuser_trap_cfg.userDefine);
            break;

        case RTDRV_TRAP_USERDEFINEMGMTACTION_SET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtAction_set(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, buf.mgmuser_trap_cfg.rma_action);
            break;

        case RTDRV_TRAP_USERDEFINEMGMTPRI_SET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtPri_set(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, buf.mgmuser_trap_cfg.priority);
            break;

        case RTDRV_TRAP_USERDEFINEMGMTPRIENABLE_SET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtPriEnable_set(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, buf.mgmuser_trap_cfg.enable);
            break;

        case RTDRV_TRAP_USERDEFINEMGMTVLANCHECK_SET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtVlanCheck_set(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, buf.mgmuser_trap_cfg.vlanCheck);
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEACTION_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFrameAction_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.rma_action);
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEPRI_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFramePri_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.priority);
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEPRIENABLE_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFramePriEnable_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.enable);
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEVLANCHECK_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFrameVlanCheck_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.vlanCheck);
            break;

        case RTDRV_TRAP_PORTMGMTFRAMECROSSVLAN_SET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFrameCrossVlan_set(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.vlanCross);
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERACTION_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderAction_set(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        buf.other_trap_cfg.ipFamily, buf.other_trap_cfg.action);
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERPRI_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderPri_set(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        buf.other_trap_cfg.priority);
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERPRIENABLE_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderPriEnable_set(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        buf.other_trap_cfg.enable);
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERADDCPUTAGENABLE_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderAddCPUTagEnable_set(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        buf.other_trap_cfg.cputag);
            break;

        case RTDRV_TRAP_PKTWITHCFIACTION_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIAction_set(buf.other_trap_cfg.unit,
                    buf.other_trap_cfg.action);
            break;

        case RTDRV_TRAP_PKTWITHOUTERCFIACTION_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithOuterCFIAction_set(buf.other_trap_cfg.unit,
                    buf.other_trap_cfg.action);
            break;

        case RTDRV_TRAP_PORTPKTWITHCFIACTION_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_portPktWithCFIAction_set(buf.other_trap_cfg.unit,
                    buf.other_trap_cfg.port, buf.other_trap_cfg.action);
            break;

        case RTDRV_TRAP_PKTWITHCFIPRI_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIPri_set(buf.other_trap_cfg.unit,
                    buf.other_trap_cfg.priority);
            break;

        case RTDRV_TRAP_PORTPKTWITHCFIPRI_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_portPktWithCFIPri_set(buf.other_trap_cfg.unit,
                    buf.other_trap_cfg.port, buf.other_trap_cfg.priority);
            break;

        case RTDRV_TRAP_PKTWITHCFIPRIENABLE_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIPriEnable_set(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        buf.other_trap_cfg.enable);
            break;

        case RTDRV_TRAP_PKTWITHCFIADDCPUTAGENABLE_SET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIAddCPUTagEnable_set(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        buf.other_trap_cfg.cputag);
            break;

        case RTDRV_TRAP_CFMFRAMEACTION_SET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameAction_set(buf.cfm_trap_cfg.unit, buf.cfm_trap_cfg.md_level, buf.cfm_trap_cfg.action);
            break;

        case RTDRV_TRAP_CFMUNKNOWNFRAMEACT_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_trap_cfmUnknownFrameAct_set(buf.misc_cfg.unit,
                                                  buf.misc_cfg.action);
            break;

        case RTDRV_TRAP_CFMLOOPBACKACT_SET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmLoopbackLinkTraceAct_set(buf.cfm_trap_cfg.unit,
                                             buf.cfm_trap_cfg.md_level,
                                             buf.cfm_trap_cfg.action);
            break;

        case RTDRV_TRAP_CFMCCMACT_SET:
            copy_from_user(&buf.oam_trap_cfg, user, sizeof(rtdrv_trapOamCfg_t));
            ret = rtk_trap_cfmCcmAct_set(buf.oam_trap_cfg.unit,
                                         buf.oam_trap_cfg.md_level,
                                         buf.oam_trap_cfg.action);
            break;

        case RTDRV_TRAP_CFMETHDMACT_SET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmEthDmAct_set(buf.cfm_trap_cfg.unit,
                                           buf.cfm_trap_cfg.md_level,
                                           buf.cfm_trap_cfg.action);
            break;

        case RTDRV_TRAP_CFMFRAMETRAPPRI_SET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameTrapPri_set(buf.cfm_trap_cfg.unit, buf.cfm_trap_cfg.priority);
            break;

        case RTDRV_TRAP_CFMFRAMETRAPPRIENABLE_SET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameTrapPriEnable_set(buf.cfm_trap_cfg.unit, buf.cfm_trap_cfg.enable);
            break;

        case RTDRV_TRAP_CFMFRAMETRAPADDCPUTAGENABLE_SET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameTrapAddCPUTagEnable_set(buf.cfm_trap_cfg.unit, buf.cfm_trap_cfg.cputag);
            break;

        case RTDRV_TRAP_PORTOAMPDUACTION_SET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_portOamPDUAction_set(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, buf.port_trap_cfg.action);
            break;

        case RTDRV_TRAP_PORTOAMPDUPRI_SET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_portOamPDUPri_set(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, buf.port_trap_cfg.priority);
            break;

        case RTDRV_TRAP_OAMPDUACTION_SET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_oamPDUAction_set(buf.cfm_trap_cfg.unit, buf.cfm_trap_cfg.action);
            break;

        case RTDRV_TRAP_OAMPDUPRI_SET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_oamPDUPri_set(buf.cfm_trap_cfg.unit, buf.cfm_trap_cfg.priority);
            break;

        case RTDRV_TRAP_OAMPDUPRIENABLE_SET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_oamPDUPriEnable_set(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, buf.port_trap_cfg.enable);
            break;

        case RTDRV_TRAP_OAMPDUTRAPADDCPUTAGENABLE_SET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_oamPDUTrapAddCPUTagEnable_set(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, buf.port_trap_cfg.cputag);
            break;

        case RTDRV_TRAP_1XMACCHANGEPORT2CPUENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_1xMacChangePort2CpuEnable_set(buf.trap_cfg.unit, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_IGMPCTRLPKT2CPUENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_igmpCtrlPkt2CpuEnable_set(buf.trap_cfg.unit, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_L2MCASTPKT2CPUENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_l2McastPkt2CpuEnable_set(buf.trap_cfg.unit, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_IPMCASTPKT2CPUENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_ipMcastPkt2CpuEnable_set(buf.trap_cfg.unit, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_REASONTRAPTOCPUPRIORITY_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_reasonTrapToCPUPriority_set(buf.trap_cfg.unit, buf.trap_cfg.reason, buf.trap_cfg.priority);
            break;

        case RTDRV_TRAP_PKT2CPUENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_pkt2CpuEnable_set(buf.trap_cfg.unit, buf.trap_cfg.pkt_type, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_MGMTIPCHECK_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_mgmtIpCheck_set(buf.trap_cfg.unit, buf.trap_cfg.ip_type, buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_PORTOAMLOOPBACKPARACTION_SET:
            copy_from_user(&buf.oam_trap_cfg, user, sizeof(rtdrv_trapOamCfg_t));
            ret = rtk_trap_portOamLoopbackParAction_set(buf.oam_trap_cfg.unit,
                    buf.oam_trap_cfg.port, buf.oam_trap_cfg.action);
            break;

        case RTDRV_TRAP_ROUTEEXCEPTIONACTION_SET:
            copy_from_user(&buf.routeException_trap_cfg, user,
                    sizeof(rtdrv_trapRouteExceptionCfg_t));
            ret = rtk_trap_routeExceptionAction_set(
                    buf.routeException_trap_cfg.unit,
                    buf.routeException_trap_cfg.type,
                    buf.routeException_trap_cfg.action);
            break;

        case RTDRV_TRAP_ROUTEEXCEPTIONPRI_SET:
            copy_from_user(&buf.routeException_trap_cfg, user,
                    sizeof(rtdrv_trapRouteExceptionCfg_t));
            ret = rtk_trap_routeExceptionPri_set(
                    buf.routeException_trap_cfg.unit,
                    buf.routeException_trap_cfg.type,
                    buf.routeException_trap_cfg.priority);
            break;

        case RTDRV_TRAP_USERDEFINERMALEARNINGENABLE_SET:
            copy_from_user(&buf.mgmuser_trap_cfg, user,
                    sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineRmaLearningEnable_set(
                    buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx,
                    buf.mgmuser_trap_cfg.enable);
            break;

        case RTDRV_TRAP_RMALEARNINGENABLE_SET:
            copy_from_user(&buf.trap_cfg, user,
                    sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaLearningEnable_set(
                    buf.trap_cfg.unit, &buf.trap_cfg.rma_frame,
                    buf.trap_cfg.enable);
            break;

        case RTDRV_TRAP_MGMTFRAMELEARNINGENABLE_SET:
            copy_from_user(&buf.mgm_trap_cfg, user,
                    sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFrameLearningEnable_set(buf.mgm_trap_cfg.unit,
                    buf.mgm_trap_cfg.frameType, buf.mgm_trap_cfg.enable);
            break;

        case RTDRV_TRAP_MGMTFRAMEMGMTVLANENABLE_SET:
            copy_from_user(&buf.other_trap_cfg, user,
                    sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_mgmtFrameMgmtVlanEnable_set(
                    buf.other_trap_cfg.unit, buf.other_trap_cfg.enable);
            break;

        case RTDRV_TRAP_BPDUFLOODPORTMASK_SET:
            copy_from_user(&buf.bpdu_flood_pmsk_cfg, user,
                    sizeof(rtdrv_bpduFloodPmskCfg_t));
            ret = rtk_trap_bpduFloodPortmask_set(
                    buf.bpdu_flood_pmsk_cfg.unit, &buf.bpdu_flood_pmsk_cfg.pmsk);
            break;

        case RTDRV_TRAP_RMAGROUPACTION_SET:
            copy_from_user(&buf.rma_grp_act_cfg, user,
                    sizeof(rtdrv_rmaGroupType_t));
            ret = rtk_trap_rmaGroupAction_set(
                    buf.rma_grp_act_cfg.unit, buf.rma_grp_act_cfg.rmaGroup_frameType, buf.rma_grp_act_cfg.rma_action);
            break;

        case RTDRV_TRAP_RMAGROUPLEARNINGENABLE_SET:
            copy_from_user(&buf.rma_grp_lrn_cfg, user,
                    sizeof(rtdrv_rmaGroupLearn_t));
            ret = rtk_trap_rmaGroupLearningEnable_set(
                    buf.rma_grp_lrn_cfg.unit, buf.rma_grp_lrn_cfg.rmaGroup_frameType, buf.rma_grp_lrn_cfg.enable);
            break;

        case RTDRV_TRAP_MGMTFRAMESELFARPENABLE_SET:
            copy_from_user(&buf.other_trap_cfg, user,
                    sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_mgmtFrameSelfARPEnable_set(
                    buf.other_trap_cfg.unit, buf.other_trap_cfg.enable);
            break;

        case RTDRV_TRAP_RMALOOKUPMISSACTIONENABLE_SET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaLookupMissActionEnable_set(buf.trap_cfg.unit, buf.trap_cfg.enable);
            break;

    /** FILTER **/
        case RTDRV_FILTER_CUTLINE_SET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_blkCutline_set(buf.filter_cfg.unit, buf.filter_cfg.cutline);
            break;

        case RTDRV_FILTER_ENABLE_SET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_pieEnable_set(buf.filter_cfg.unit, buf.filter_cfg.enable);
            break;

        case RTDRV_FILTER_IGR_ACL_ADD:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_igrAcl_add(buf.filter_cfg.unit, buf.filter_cfg.filter_id, &buf.filter_cfg.acl_cfg,
                                        &buf.filter_cfg.action);
            break;

        case RTDRV_FILTER_FLOW_TBL_ADD:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_flowTbl_add(buf.filter_cfg.unit, buf.filter_cfg.filter_id, &buf.filter_cfg.flow_table_cfg,
                                         &buf.filter_cfg.action);
            break;

        case RTDRV_FILTER_IGR_ACL_DEL:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_igrAcl_del(buf.filter_cfg.unit, buf.filter_cfg.filter_id);
            break;

        case RTDRV_FILTER_FLOW_TBL_DEL:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_flowTbl_del(buf.filter_cfg.unit, buf.filter_cfg.filter_id);
            break;

        case RTDRV_FILTER_IGR_ACL_DELALL:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_igrAcl_delAll(buf.filter_cfg.unit);
            break;

        case RTDRV_FILTER_FLOW_TBL_DELALL:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_flowTbl_delAll(buf.filter_cfg.unit);
            break;

        case RTDRV_FILTER_LOG_COUNTER_SET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_stat_set(buf.filter_cfg.unit, buf.filter_cfg.index, buf.filter_cfg.packet_counter,
                                      buf.filter_cfg.byte_counter);
            break;

        case RTDRV_FILTER_PATTERN_MATCH_SET:
            copy_from_user(&buf.pattern_cfg, user, sizeof(rtdrv_patternCfg_t));
            ret = rtk_filter_patternMatch_set(buf.pattern_cfg.unit, buf.pattern_cfg.port, buf.pattern_cfg.mode,
                                              buf.pattern_cfg.pattern, buf.pattern_cfg.mask);
            break;

        case RTDRV_FILTER_RATE_LIMIT_SET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_igrAclRateLimit_set(buf.filter_cfg.unit, buf.filter_cfg.index, buf.filter_cfg.rate);
            break;

    /** PIE **/

//        case RTDRV_PIE_ENTRY_FIELD_SET:
//            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
//            ret = rtk_pie_pieRuleEntryField_set(buf.pie_cfg.unit, buf.pie_cfg.phase, buf.pie_cfg.index,
//                                                buf.pie_cfg.entry_buffer, buf.pie_cfg.field_type,
//                                                buf.pie_cfg.field_data, buf.pie_cfg.field_mask);
//            break;

        case RTDRV_PIE_ENTRY_FIELD_WRITE:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryField_write(buf.pie_cfg.unit, buf.pie_cfg.phase, buf.pie_cfg.index,
                                                  buf.pie_cfg.field_type, buf.pie_cfg.field_data,
                                                  buf.pie_cfg.field_mask);
            break;

//        case RTDRV_PIE_PREDEFINED_ENTRY_SET:
//            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
//            ret = rtk_pie_piePreDefinedRuleEntry_set(buf.pie_cfg.unit, buf.pie_cfg.entry_buffer,
//                                                     &buf.pie_cfg.predefined_entry);
//            break;

        case RTDRV_PIE_ENTRY_WRITE:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntry_write(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.entry_buffer);
            break;

        case RTDRV_PIE_ENTRY_DEL:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntry_del(buf.pie_cfg.unit, &buf.pie_cfg.block_content);
            break;

        case RTDRV_PIE_ENTRY_MOVE:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntry_move(buf.pie_cfg.unit, &buf.pie_cfg.move_content);
            break;

        case RTDRV_PIE_ENTRY_SWAP:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntry_swap(buf.pie_cfg.unit, &buf.pie_cfg.move_content);
            break;

        case RTDRV_PIE_ACTION_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleAction_set(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.action);
            break;

        case RTDRV_PIE_ACTION_DEL:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleAction_del(buf.pie_cfg.unit, &buf.pie_cfg.block_content);
            break;

        case RTDRV_PIE_ACTION_MOVE:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleAction_move(buf.pie_cfg.unit, &buf.pie_cfg.move_content);
            break;

        case RTDRV_PIE_ACTION_SWAP:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleAction_swap(buf.pie_cfg.unit, &buf.pie_cfg.move_content);
            break;

        case RTDRV_PIE_POLICER_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRulePolicer_set(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.policer);
            break;

        case RTDRV_PIE_COUNTER_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieStat_set(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.pkt_cnt, buf.pie_cfg.byte_cnt);
            break;

        case RTDRV_PIE_COUNTER_CLEARALL:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieStat_clearAll(buf.pie_cfg.unit);
            break;

        case RTDRV_PIE_TMP_SELECTOR_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieTemplateSelector_set(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.phase, buf.pie_cfg.index1);
            break;

        case RTDRV_PIE_USER_TMP_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieUserTemplate_set(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.template);
            break;

        case RTDRV_PIE_L34_CHECKSUN_ERR_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieL34ChecksumErr_set(buf.pie_cfg.unit, buf.pie_cfg.checksum_err_op);
            break;

        case RTDRV_PIE_TMP_PAYLOAD_OFFSET_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieUserTemplatePayloadOffset_set(buf.pie_cfg.unit, buf.pie_cfg.index,
                                                           buf.pie_cfg.index1, buf.pie_cfg.offset);
            break;

        case RTDRV_PIE_RESULT_REVERSE_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieResultReverse_set(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.reverse_op);
            break;

        case RTDRV_PIE_RESULT_AGGREGATOR_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieResultAggregator_set(buf.pie_cfg.unit, buf.pie_cfg.index,
                                                  buf.pie_cfg.index1, buf.pie_cfg.aggregator_type);
            break;

        case RTDRV_PIE_BLOCK_PRI_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieBlockPriority_set(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.priority);
            break;

        case RTDRV_PIE_GROUP_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieGroupCtrl_set(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.group_op);
            break;

        case RTDRV_PIE_EGR_ACL_CTRL_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieEgrAclLookupCtrl_set(buf.pie_cfg.unit, &buf.pie_cfg.egr_acl_ctrl);
            break;

        case RTDRV_PIE_PORT_LP_ENABLE_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePortLookupPhaseEnable_set(buf.pie_cfg.unit, buf.pie_cfg.port,
                                                       buf.pie_cfg.phase, buf.pie_cfg.enable);
            break;

        case RTDRV_PIE_PORT_LP_MISS_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePortLookupPhaseMiss_set(buf.pie_cfg.unit,  buf.pie_cfg.port,
                                                     buf.pie_cfg.phase, buf.pie_cfg.miss_action);
            break;

        case RTDRV_PIE_COUNTER_MODE_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieCounterIndicationMode_set(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.counter_mode);
            break;

        case RTDRV_PIE_POLICER_CTRL_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePolicerCtrl_set(buf.pie_cfg.unit, &buf.pie_cfg.policer_ctrl);
            break;

        case RTDRV_PIE_RC_L4PORT_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckL4Port_set(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_l4port_data);
            break;

        case RTDRV_PIE_RC_VID_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckVid_set(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_vid_data);
            break;

        case RTDRV_PIE_RC_IP_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckIp_set(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_ip_data);
            break;

        case RTDRV_PIE_RC_SRC_PORT_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckSrcPort_set(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_src_port_data);
            break;

        case RTDRV_PIE_FS_ENABLE_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_fieldSelectorEnable_set(buf.pie_cfg.unit, buf.pie_cfg.port, buf.pie_cfg.index, buf.pie_cfg.enable);
            break;

        case RTDRV_PIE_FS_CONTENT_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_fieldSelectorContent_set(buf.pie_cfg.unit, buf.pie_cfg.port, buf.pie_cfg.index, &buf.pie_cfg.fs_content);
            break;

        case RTDRV_PIE_PM_ENABLE_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_patternMatchEnable_set(buf.pie_cfg.unit, buf.pie_cfg.port, buf.pie_cfg.enable);
            break;

        case RTDRV_PIE_PM_CONTENT_SET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_patternMatchContent_set(buf.pie_cfg.unit, buf.pie_cfg.port, &buf.pie_cfg.pm_content);
            break;

        case RTDRV_PIE_ENTRY_ACTION_DEL:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryAction_del(buf.pie_cfg.unit, &buf.pie_cfg.block_content);
            break;

        case RTDRV_PIE_ENTRY_ACTION_MOVE:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryAction_move(buf.pie_cfg.unit, &buf.pie_cfg.move_content);
            break;

        case RTDRV_PIE_ENTRY_ACTION_SWAP:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryAction_swap(buf.pie_cfg.unit, &buf.pie_cfg.move_content);
            break;

    /** ACL **/
        case RTDRV_ACL_ENTRY_DATA_WRITE:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntryField_write(
                    buf.acl_cfg.unit,
                    buf.acl_cfg.phase,
                    buf.acl_cfg.index,
                    buf.acl_cfg.field_type,
                    buf.acl_cfg.field_data,
                    buf.acl_cfg.field_mask);
            break;

        case RTDRV_ACL_METER_MODE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterMode_set(buf.acl_cfg.unit, buf.acl_cfg.blockIdx, buf.acl_cfg.meterMode);
            break;

        case RTDRV_ACL_METER_INCLUDE_IFG_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterIncludeIfg_set(buf.acl_cfg.unit, buf.acl_cfg.ifg_include);
            break;

        case RTDRV_ACL_METER_BURST_SIZE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterBurstSize_set(buf.acl_cfg.unit, buf.acl_cfg.meterMode, &buf.acl_cfg.burstSize);
            break;

        case RTDRV_ACL_METER_ENTRY_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterEntry_set(buf.acl_cfg.unit, buf.acl_cfg.meterIdx, &buf.acl_cfg.meterEntry);
            break;

        case RTDRV_ACL_PARTITION_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_partition_set(buf.acl_cfg.unit, buf.acl_cfg.blockIdx);
            break;

        case RTDRV_ACL_BLOCKPWRENABLE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockPwrEnable_set(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, buf.acl_cfg.status);
            break;

        case RTDRV_ACL_BLOCKLOOKUPENABLE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockLookupEnable_set(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, buf.acl_cfg.status);
            break;

        case RTDRV_ACL_RULEVALIDATE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleValidate_set(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, buf.acl_cfg.status);
            break;

        case RTDRV_ACL_RULEENTRY_WRITE:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntry_write(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, buf.acl_cfg.entry_buffer);
            break;

        case RTDRV_ACL_RULEENTRYFIELD_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntryField_set(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, buf.acl_cfg.entry_buffer,
                    buf.acl_cfg.field_type, buf.acl_cfg.field_data,
                    buf.acl_cfg.field_mask);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RULEOPERATION_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleOperation_set(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, &buf.acl_cfg.oper);
            break;

        case RTDRV_ACL_RULEACTION_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleAction_set(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, &buf.acl_cfg.action);
            break;

        case RTDRV_ACL_RULE_DEL:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_rule_del(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    &buf.acl_cfg.clear);
            break;

        case RTDRV_ACL_RULE_MOVE:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_rule_move(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    &buf.acl_cfg.move);
            break;

        case RTDRV_ACL_TEMPLATESELECTOR_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_templateSelector_set(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, buf.acl_cfg.template_idx);
            break;

        case RTDRV_ACL_TEMPLATE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_template_set(buf.acl_cfg.unit, buf.acl_cfg.index,
                    &buf.acl_cfg.template);
            break;

        case RTDRV_ACL_BLOCKRESULTMODE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockResultMode_set(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, buf.acl_cfg.blk_mode);
            break;

        case RTDRV_ACL_BLOCKAGGREGATORENABLE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockGroupEnable_set(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, buf.acl_cfg.blk_group,
                    buf.acl_cfg.status);
            break;

        case RTDRV_ACL_STATPKTCNT_CLEAR:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_statPktCnt_clear(buf.acl_cfg.unit, buf.acl_cfg.index);
            break;

        case RTDRV_ACL_STATBYTECNT_CLEAR:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_statByteCnt_clear(buf.acl_cfg.unit, buf.acl_cfg.index);
            break;

        case RTDRV_ACL_STAT_CLEARALL:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_stat_clearAll(buf.acl_cfg.unit);
            break;

        case RTDRV_ACL_RANGECHECKL4PORT_SET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckL4Port_set(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_l4Port);
            break;

        case RTDRV_ACL_RANGECHECKVID_SET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckVid_set(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_vid);
            break;

        case RTDRV_ACL_RANGECHECKIP_SET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckIp_set(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_ip);
            break;

        case RTDRV_ACL_RANGECHECKSRCPORT_SET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckSrcPort_set(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_port);
            break;

        case RTDRV_ACL_RANGECHECKDSTPORT_SET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckDstPort_set(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_port);
            break;

        case RTDRV_ACL_RANGECHECKPACKETLEN_SET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckPacketLen_set(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_pktLen);
            break;

        case RTDRV_ACL_FIELDSELECTOR_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_fieldSelector_set(buf.acl_cfg.unit,
                    buf.acl_cfg.index, &buf.acl_cfg.fs);
            break;

        case RTDRV_ACL_PORTLOOKUPENABLE_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_portLookupEnable_set(buf.acl_cfg.unit,
                    buf.acl_cfg.port, buf.acl_cfg.status);
            break;

        case RTDRV_ACL_LOOKUPMISSACT_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_lookupMissAct_set(buf.acl_cfg.unit,
                    buf.acl_cfg.port, buf.acl_cfg.lmAct);
            break;

        case RTDRV_ACL_RANGECHECKFIELDSEL_SET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckFieldSelector_set(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_fieldSel);
            break;

        case RTDRV_ACL_TEMPLATEFIELDINTENTVLANTAG_SET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_templateFieldIntentVlanTag_set(buf.acl_cfg.unit, buf.acl_cfg.tagType);
            break;

    /** QOS **/
        case RTDRV_QOS_QUEUE_NUM_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_queueNum_set(buf.qos_cfg.unit, buf.qos_cfg.queue_num);
            break;

        case RTDRV_QOS_PRI_MAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_priMap_set(buf.qos_cfg.unit, buf.qos_cfg.queue_num, &buf.qos_cfg.pri2qid);
            break;

        case RTDRV_QOS_PORT_PRI_MAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriMap_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.int_pri, buf.qos_cfg.queue);
            break;

        case RTDRV_QOS_1P_PRI_REMAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pPriRemap_set(buf.qos_cfg.unit, buf.qos_cfg.dot1p_pri, buf.qos_cfg.int_pri);
            break;

        case RTDRV_QOS_1P_PRI_REMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pPriRemapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.dot1p_pri, buf.qos_cfg.int_pri, buf.qos_cfg.dp);
            break;

        case RTDRV_QOS_PORT_1P_PRI_REMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_port1pPriRemapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_OUTER_1P_PRI_REMAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pPriRemap_set(buf.qos_cfg.unit, buf.qos_cfg.dot1p_pri, buf.qos_cfg.dei, buf.qos_cfg.int_pri);
            break;

        case RTDRV_QOS_OUTER_1P_PRI_REMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pPriRemapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.dot1p_pri, buf.qos_cfg.dei
                                                , buf.qos_cfg.int_pri, buf.qos_cfg.dp);
            break;

        case RTDRV_QOS_PORT_OUTER_1P_PRI_REMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pPriRemapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_DEI_DP_REMAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_deiDpRemap_set(buf.qos_cfg.unit, buf.qos_cfg.dei, buf.qos_cfg.dp);
            break;

        case RTDRV_QOS_PORT_DEI_SRC_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDEISrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.deiSrc);
            break;

        case RTDRV_QOS_DSCP_DP_REMAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpDpRemap_set(buf.qos_cfg.unit, buf.qos_cfg.dscp, buf.qos_cfg.dp);
            break;

        case RTDRV_QOS_DSCP_PRI_REMAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpPriRemap_set(buf.qos_cfg.unit, buf.qos_cfg.dscp, buf.qos_cfg.int_pri);
            break;

        case RTDRV_QOS_DSCP_PRI_REMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpPriRemapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.dscp, buf.qos_cfg.int_pri, buf.qos_cfg.dp);
            break;

        case RTDRV_QOS_PORT_DSCP_PRI_REMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDscpPriRemapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_PORT_PRI_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPri_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.int_pri);
            break;

        case RTDRV_QOS_PORT_DP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDp_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.dp);
            break;

        case RTDRV_QOS_PORT_INNER_PRI_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portInnerPri_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.int_pri);
            break;

        case RTDRV_QOS_PORT_OUTER_PRI_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuterPri_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.int_pri);
            break;

        case RTDRV_QOS_PORT_OUTER_DEI_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuterDEI_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.dei);
            break;

        case RTDRV_QOS_DP_SRC_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dpSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.dpSrcType);
            break;

        case RTDRV_QOS_PRI_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_priSel_set(buf.qos_cfg.unit, buf.qos_cfg.port_pri, buf.qos_cfg.class_pri,
                                     buf.qos_cfg.acl_pri, buf.qos_cfg.dscp_pri);
            break;

        case RTDRV_QOS_PRI_SEL_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_priSelGroup_set(buf.qos_cfg.unit, buf.qos_cfg.index, &(buf.qos_cfg.priSelWeight));
            break;

        case RTDRV_QOS_PORT_PRI_SEL_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriSelGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_1P_REMARK_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemarkEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.remark_enable);
            break;

        case RTDRV_QOS_DSCP_REMARK_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemarkEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.remark_enable);
            break;

        case RTDRV_QOS_1P_DFLT_PRI_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pDfltPri_set(buf.qos_cfg.unit, buf.qos_cfg.dot1p_dflt_pri);
            break;

        case RTDRV_QOS_1P_REMARK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemark_set(buf.qos_cfg.unit, buf.qos_cfg.int_pri, buf.qos_cfg.dot1p_pri);
            break;

        case RTDRV_QOS_1P_REMARK_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemarkGroup_set(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.int_pri, buf.qos_cfg.dp, buf.qos_cfg.dot1p_pri);
            break;

        case RTDRV_QOS_1P_REMARK_SRC_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemarkSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.rmksrc_1p);
            break;

        case RTDRV_QOS_PORT_1P_REMARK_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_port1pRemarkGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_PORT_1P_PRIMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_port1pPriMapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_OUT_1P_REMARK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pRemark_set(buf.qos_cfg.unit, buf.qos_cfg.int_pri, buf.qos_cfg.dot1p_pri);
            break;

        case RTDRV_QOS_OUT_1P_REMARK_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_out1pRemarkEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.remark_enable);
            break;

        case RTDRV_QOS_OUT_1P_REMARK_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pRemarkGroup_set(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.int_pri
                                        , buf.qos_cfg.dp, buf.qos_cfg.dot1p_pri, buf.qos_cfg.dei);
            break;

        case RTDRV_QOS_OUT_1P_REMARK_SRC_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pRemarkSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.rmksrc_outer1p);
            break;

        case RTDRV_QOS_PORT_OUT_1P_DFLT_SRC_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pDfltPriSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.out1p_dflt_src);
            break;

        case RTDRV_QOS_PORT_OUT_1P_REMARK_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pRemarkGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_PORT_OUT_1P_PRIMAP_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pPriMapGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_DSCP_REMARK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemark_set(buf.qos_cfg.unit, buf.qos_cfg.int_pri, buf.qos_cfg.dscp);
            break;

        case RTDRV_QOS_DSCP2DOT1P_REMARK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscp2Dot1pRemark_set(buf.qos_cfg.unit, buf.qos_cfg.org_dscp, buf.qos_cfg.dot1p_pri);
            break;

        case RTDRV_QOS_DSCP2OUT1P_REMARK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscp2Outer1pRemark_set(buf.qos_cfg.unit, buf.qos_cfg.org_dscp, buf.qos_cfg.dot1p_pri);
            break;

        case RTDRV_QOS_DSCP2DSCP_REMARK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscp2DscpRemark_set(buf.qos_cfg.unit, buf.qos_cfg.org_dscp, buf.qos_cfg.dscp);
            break;

        case RTDRV_QOS_DSCP_REMARK_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemarkGroup_set(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.int_pri, buf.qos_cfg.dp
                                            , buf.qos_cfg.dscp);
            break;

        case RTDRV_QOS_DSCP_REMARK_SRC_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemarkSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.rmksrc_dscp);
            break;

        case RTDRV_QOS_PORT_DSCP_REMARK_GROUP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portdscpRemarkGroup_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.index);
            break;

        case RTDRV_QOS_DEI_REMARK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_deiRemark_set(buf.qos_cfg.unit, buf.qos_cfg.dp, buf.qos_cfg.dei);
            break;

        case RTDRV_QOS_DEI_REMARK_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_deiRemarkEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.remark_enable);
            break;

        case RTDRV_QOS_PORT_DEI_REMARK_TAG_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDEIRemarkTagSel_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.deiSrc);
            break;

        case RTDRV_QOS_SCHEDULING_ALGORITHM_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_schedulingAlgorithm_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.scheduling_type);
            break;

        case RTDRV_QOS_SCHEDULING_QUEUE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_schedulingQueue_set(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.qweights);
            break;

        case RTDRV_QOS_WFQ_FIXED_BANDWIDTH_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wfqFixedBandwidthEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_CONG_AVOID_ALGO_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidAlgo_set(buf.qos_cfg.unit, buf.qos_cfg.congAvoid_algo);
            break;

        case RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidQueueThreshEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_CONG_AVOID_PORT_THRESH_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidPortThreshEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_CONG_AVOID_SYS_THRESH_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidSysThreshEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_CONG_AVOID_SYS_THRESH_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidSysThresh_set(buf.qos_cfg.unit, buf.qos_cfg.dp, &(buf.qos_cfg.congAvoid_thresh));
            break;

        case RTDRV_QOS_CONG_AVOID_SYS_DROP_PROBABILITY_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidSysDropProbability_set(buf.qos_cfg.unit, buf.qos_cfg.dp, buf.qos_cfg.data);
            break;

        case RTDRV_QOS_CONG_AVOID_PORT_THRESH_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidPortThresh_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.dp, &(buf.qos_cfg.congAvoid_thresh));
            break;

        case RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidQueueThresh_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, &(buf.qos_cfg.congAvoid_thresh));
            break;

        case RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_THRESH_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidGlobalQueueThresh_set(buf.qos_cfg.unit, buf.qos_cfg.queue, buf.qos_cfg.dp, &(buf.qos_cfg.congAvoid_thresh));
            break;

        case RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_DROP_PROBABILITY_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidGlobalQueueDropProbability_set(buf.qos_cfg.unit, buf.qos_cfg.queue, buf.qos_cfg.dp, buf.qos_cfg.data);
            break;

        case RTDRV_QOS_WRED_SYS_THRESH_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredSysThresh_set(buf.qos_cfg.unit, buf.qos_cfg.dp, &(buf.qos_cfg.wred_thresh));
            break;

        case RTDRV_QOS_WEIGHT_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredWeight_set(buf.qos_cfg.unit, buf.qos_cfg.data);
            break;

        case RTDRV_QOS_MPD_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredMpd_set(buf.qos_cfg.unit, buf.qos_cfg.data);
            break;

        case RTDRV_QOS_ECN_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredEcnEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_CNT_REVERSE_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredCntReverseEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_AVB_SR_CLASS_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portAvbStreamReservationClassEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.srClass, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_AVB_SR_CONFIG_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_avbStreamReservationConfig_set(buf.qos_cfg.unit, &buf.qos_cfg.srConf);
            break;

        case RTDRV_QOS_PKT2CPU_PRI_REMAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_pkt2CpuPriRemap_set(buf.qos_cfg.unit, buf.qos_cfg.int_pri, buf.qos_cfg.new_pri);
            break;

        case RTDRV_QOS_PRI2IGR_QUEUE_MAP_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPri2IgrQMap_set(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.pri2qid);
            break;

        case RTDRV_QOS_PRI2IGR_QUEUE_MAP_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPri2IgrQMapEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_IGR_QUEUE_WEIGHT_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_igrQueueWeight_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, buf.qos_cfg.data);
            break;

        case RTDRV_QOS_1P_DFLT_PRI_SRC_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pDfltPriSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.dflt_src_1p);
            break;

        case RTDRV_QOS_PORT_OUTER_1P_REMARK_SRC_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pRemarkSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.rmksrc_outer1p);
            break;

        case RTDRV_QOS_OUTER_1P_DFLT_PRI_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pDfltPri_set(buf.qos_cfg.unit, buf.qos_cfg.out1p_dflt_pri);
            break;

        case RTDRV_QOS_OUTER_1P_DFLT_PRI_CFG_SRC_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pDfltPriCfgSrcSel_set(buf.qos_cfg.unit, buf.qos_cfg.out1p_dflt_cfg_dir);
            break;

        case RTDRV_QOS_INVLD_DSCP_VAL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_invldDscpVal_set(buf.qos_cfg.unit, buf.qos_cfg.dscp);
            break;

        case RTDRV_QOS_INVLD_DSCP_MASK_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_invldDscpMask_set(buf.qos_cfg.unit, buf.qos_cfg.dscp);
            break;

        case RTDRV_QOS_PORT_INVLD_DSCP_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portInvldDscpEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_INVLD_DSCP_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_invldDscpEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_PORT_PRI_REMAP_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriRemapEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_PORT_PRI_REMAP_SEL_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriRemapSel_set(buf.qos_cfg.unit, buf.qos_cfg.portPriRemap_type);
            break;

        case RTDRV_QOS_PORT_IPRI_REMAP_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portInnerPriRemapEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_PORT_OPRI_REMAP_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuterPriRemapEnable_set(buf.qos_cfg.unit, buf.qos_cfg.enable);
            break;

        case RTDRV_QOS_QUEUE_STRICT_ENABLE_SET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_queueStrictEnable_set(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, buf.qos_cfg.enable);
            break;

    /** TRUNK **/
        case RTDRV_TRUNK_PORT_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_port_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, &buf.trunk_cfg.trk_member);
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_BIND_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithmBind_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, buf.trunk_cfg.algo_id);
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_PARAM_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithmParam_set(buf.trunk_cfg.unit, buf.trunk_cfg.algo_id, buf.trunk_cfg.algo_bitmask);
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithm_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid,
                                                      buf.trunk_cfg.algo_bitmask);
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_SHIFT_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithmShift_set(buf.trunk_cfg.unit, buf.trunk_cfg.algo_id, &buf.trunk_cfg.shift);
            break;

        case RTDRV_TRUNK_HASH_MAPPING_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_hashMappingTable_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid,
                                                 &buf.trunk_cfg.hash2Port_array);
            break;

        case RTDRV_TRUNK_TRUNK_MODE_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_mode_set(buf.trunk_cfg.unit, buf.trunk_cfg.mode);
            break;

        case RTDRV_TRUNK_PORT_LINK_NOTIFICATION:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_port_link_notification(buf.trunk_cfg.unit, buf.trunk_cfg.trk_memberport, buf.trunk_cfg.linkStat);
            break;

        case RTDRV_TRUNK_REPRESENTPORT_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_representPort_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, buf.trunk_cfg.represPort);
            break;

        case RTDRV_TRUNK_FLOODMODE_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_floodMode_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, buf.trunk_cfg.floodMode);
            break;

        case RTDRV_TRUNK_FLOODPORT_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_floodPort_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, buf.trunk_cfg.floodPort);
            break;

        case RTDRV_TRUNK_TRAFFIC_SEPARATE_SET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_trafficSeparate_set(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, buf.trunk_cfg.separate);
            break;

    /** DOT1X **/
        case RTDRV_DOT1X_PORT_BASED_ENABLE_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portBasedEnable_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.enable);
            break;

        case RTDRV_DOT1X_MAC_BASED_ENABLE_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_macBasedEnable_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.enable);
            break;

        case RTDRV_DOT1X_FRAME_TO_CPU_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_eapolFrame2CpuEnable_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.enable);
            break;

        case RTDRV_DOT1X_PORT_BASED_AUTH_STATUS_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portBasedAuthStatus_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.port_auth);
            break;

        case RTDRV_DOT1X_AUTH_MAC_ADD:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_macBasedAuthMac_add(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.vid,
                                                &buf.dot1x_cfg.auth_mac);
            break;

        case RTDRV_DOT1X_AUTH_MAC_DEL:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_macBasedAuthMac_del(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.vid,
                                                &buf.dot1x_cfg.auth_mac);
            break;

        case RTDRV_DOT1X_UNAUTH_PACKET_OPER_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_unauthPacketOper_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.action);
            break;

        case RTDRV_DOT1X_PORT_UNAUTH_PACKET_OPER_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portUnauthPacketOper_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.action);
            break;

        case RTDRV_DOT1X_PORT_UNAUTH_TAG_PACKET_OPER_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portUnauthTagPacketOper_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.action);
            break;

        case RTDRV_DOT1X_PORT_UNAUTH_UNTAG_PACKET_OPER_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portUnauthUntagPacketOper_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.action);
            break;

        case RTDRV_DOT1X_PORT_BASED_DIRECTION_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portBasedDirection_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.direction);
            break;

        case RTDRV_DOT1X_MAC_BASED_DIRECTION_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_macBasedDirection_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.direction);
            break;

        case RTDRV_DOT1X_PORT_GUEST_VLAN_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portGuestVlan_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, buf.dot1x_cfg.vid);
            break;

        case RTDRV_DOT1X_GUEST_VLAN_BEHAVIOR_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_guestVlanBehavior_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.gv_behavior);
            break;

        case RTDRV_DOT1X_GUEST_VLAN_ROUTE_BEHAVIOR_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_guestVlanRouteBehavior_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.rt_action);
            break;

        case RTDRV_DOT1X_TRAP_PRI_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_trapPri_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.pri);
            break;

        case RTDRV_DOT1X_TRAP_PRI_ENABLE_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_trapPriEnable_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.enable);
            break;

        case RTDRV_DOT1X_TRAP_ADD_CPUTAG_ENABLE_SET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_trapAddCPUTagEnable_set(buf.dot1x_cfg.unit, buf.dot1x_cfg.enable);
            break;

    /** DEBUG **/
        case RTDRV_DEBUG_EN_LOG_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rt_log_enable_set(buf.unit_cfg.data);
            break;

        case RTDRV_DEBUG_LOGLV_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rt_log_level_set(buf.unit_cfg.data);
            break;

        case RTDRV_DEBUG_LOGLVMASK_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rt_log_mask_set(buf.unit_cfg.data);
            break;

        case RTDRV_DEBUG_LOGTYPE_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rt_log_type_set(buf.unit_cfg.data);
            break;

        case RTDRV_DEBUG_LOGFORMAT_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rt_log_format_set(buf.unit_cfg.data);
            break;

        case RTDRV_DEBUG_MODMASK_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rt_log_moduleMask_set(buf.unit_cfg.data64);
            break;

        case RTDRV_DEBUG_MEM_WRITE:
            copy_from_user(&buf.reg_cfg, user, sizeof(rtdrv_regCfg_t));
            ret = debug_mem_write(buf.reg_cfg.unit, buf.reg_cfg.reg, buf.reg_cfg.value);
            break;

        case RTDRV_DEBUG_HSB_DUMP:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_dumpHsb(buf.unit_cfg.unit);
            break;

#if defined(CONFIG_SDK_RTL8328)
        case RTDRV_DEBUG_PMI_DUMP:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_dumpPmi(buf.unit_cfg.unit);
            break;

        case RTDRV_DEBUG_PPI_DUMP:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_dumpPpi(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;
#endif

#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
        case RTDRV_DEBUG_HSA_DUMP:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_dumpHsa(buf.unit_cfg.unit);
            break;
#endif

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_DEBUG_HSM_DUMP:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_dumpHsm(buf.unit_cfg.unit);
            break;
#endif

#if defined(CONFIG_SDK_RTL8380)
        case RTDRV_DEBUG_HSM_IDX_DUMP:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_dumpHsmIdx(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;
#endif

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_INGRESS_RESET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_resetFlowCtrlIgrPortUsedPageCnt(buf.unit_cfg.unit, buf.unit_cfg.port);
            break;
        case RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_EGRESS_RESET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_resetFlowCtrlEgrPortUsedPageCnt(buf.unit_cfg.unit, buf.unit_cfg.port);
            break;
        case RTDRV_DEBUG_FLOWCTRL_SYSTEM_USED_PAGE_CNT_RESET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_resetFlowCtrlSystemUsedPageCnt(buf.unit_cfg.unit);
            break;
#endif

#if defined(CONFIG_SDK_UART1)
        case RTDRV_UART1_PUTC:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = drv_uart_putc(buf.unit_cfg.unit, buf.unit_cfg.data8);
            break;

        case RTDRV_UART1_BAUDRATE_SET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = drv_uart_baudrate_set(buf.unit_cfg.unit, buf.unit_cfg.data);
            break;
#endif

    /** MIRROR **/
        case RTDRV_MIRROR_ENTRY_CREATE:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portBased_create(buf.mirror_cfg.unit, buf.mirror_cfg.mirroring_port);
            break;

        case RTDRV_MIRROR_ENTRY_DESTROY:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portBased_destroy(buf.mirror_cfg.unit, buf.mirror_cfg.mirroring_port);
            break;

        case RTDRV_MIRROR_ENTRY_DESTROYALL:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portBased_destroyAll(buf.mirror_cfg.unit);
            break;

        case RTDRV_MIRROR_ENTRY_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portBased_set(buf.mirror_cfg.unit, buf.mirror_cfg.mirroring_port,
                                           &buf.mirror_cfg.rx_portmask, &buf.mirror_cfg.tx_portmask);
            break;

        case RTDRV_MIRROR_GROUP_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_group_set(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.mirrorEntry);
            break;

        case RTDRV_MIRROR_EGR_MODE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_egrMode_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_PORT_RSPAN_IGR_MODE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portRspanIgrMode_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_PORT_RSPAN_EGR_MODE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portRspanEgrMode_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_RSPAN_IGR_MODE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanIgrMode_set(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_RSPAN_EGR_MODE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanEgrMode_set(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_RSPAN_IGR_TAG_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanIgrTag_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.rspan_igrTag);
            break;

        case RTDRV_MIRROR_RSPAN_EGR_TAG_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanEgrTag_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.rspan_egrTag);
            break;

        case RTDRV_MIRROR_RSPAN_TAG_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanTag_set(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.rspan_tag);
            break;

        case RTDRV_MIRROR_SFLOW_MIRROR_SEED_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowMirrorSeed_set(buf.mirror_cfg.unit, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_ENABLE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowMirrorSampleEnable_set(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, buf.mirror_cfg.enable);
            break;

        case RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_RATE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowMirrorSampleRate_set(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_SFLOW_PORT_SEED_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortSeed_set(buf.mirror_cfg.unit, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_ENABLE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortIgrSampleEnable_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, buf.mirror_cfg.enable);
            break;

        case RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_RATE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortIgrSampleRate_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_ENABLE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortEgrSampleEnable_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, buf.mirror_cfg.enable);
            break;

        case RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_RATE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortEgrSampleRate_set(buf.mirror_cfg.unit, buf.mirror_cfg.port, buf.mirror_cfg.data);
            break;

        case RTDRV_MIRROR_SFLOW_ADD_CPU_TAG_ENABLE_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowAddCPUTagEnable_set(buf.mirror_cfg.unit, buf.mirror_cfg.enable);
            break;

        case RTDRV_MIRROR_SFLOW_SAMPLE_CTRL_SET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowSampleCtrl_set(buf.mirror_cfg.unit, buf.mirror_cfg.sample_ctrl);
            break;

    /** FLOWCTRL **/
        case RTDRV_FLOWCTRL_PORT_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_portEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.enable);
            break;

        case RTDRV_FLOWCTRL_PORT_PAUSE_FORCE_MODE_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_portPauseForceModeEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.enable);
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ACTION_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAction_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.pauseOn_action);
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PAGENUM_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAllowedPageNum_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.data);
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTLEN_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAllowedPktLen_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.data);
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTNUM_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAllowedPktNum_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.data);
            break;

        case RTDRV_FLOWCTRL_FC_ON_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrSystemPauseThresh_set(buf.flowctrl_cfg.unit, &buf.flowctrl_cfg.thresh);
            break;

        case RTDRV_FLOWCTRL_FC_OFF_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrSystemCongestThresh_set(buf.flowctrl_cfg.unit, &buf.flowctrl_cfg.thresh);
            break;

        case RTDRV_FLOWCTRL_PORT_FC_ON_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPortPauseThresh_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.thresh);
            break;

        case RTDRV_FLOWCTRL_GROUP_FC_ON_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPauseThreshGroup_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                      &buf.flowctrl_cfg.thresh);
            break;

        case RTDRV_FLOWCTRL_PORT_FC_ON_OFF_GROUP_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPortPauseThreshGroupSel_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.grp_idx);
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROPMODE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropMode_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.egrDropMode);
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROPFORCEMODE_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropForceModeEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.enable);
            break;

        case RTDRV_FLOWCTRL_PORT_FC_OFF_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPortCongestThresh_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                        &buf.flowctrl_cfg.thresh);
            break;

        case RTDRV_FLOWCTRL_GROUP_FC_OFF_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrCongestThreshGroup_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                        &buf.flowctrl_cfg.thresh);
            break;

        case RTDRV_FLOWCTRL_EGR_SYSTEM_DROP_THRESH_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrSystemDropThresh_set(buf.flowctrl_cfg.unit,
                                                        &buf.flowctrl_cfg.dropThresh);
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROP_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropThresh_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                     &buf.flowctrl_cfg.dropThresh);
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_THRESH_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortQueueDropThresh_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                     buf.flowctrl_cfg.queue, &buf.flowctrl_cfg.dropThresh);
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_QUEUE_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortQueueDropEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.enable);
            break;

        case RTDRV_FLOWCTRL_EGR_QUEUE_DROP_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrQueueDropThresh_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.queue,
                                                      &buf.flowctrl_cfg.dropThresh);
            break;

        case RTDRV_FLOWCTRL_EGR_CPU_QUEUE_DROP_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrCpuQueueDropThresh_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.queue,
                                                      &buf.flowctrl_cfg.dropThresh);
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROP_REFCONGEST_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropRefCongestEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.enable);
            break;

      case RTDRV_FLOWCTRL_EGR_PORT_GROUP_DROP_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropThreshGroup_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                      &buf.flowctrl_cfg.dropThresh);
            break;
      case RTDRV_FLOWCTRL_EGR_QUEUE_GROUP_DROP_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrQueueDropThreshGroup_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                      buf.flowctrl_cfg.queue, &buf.flowctrl_cfg.dropThresh);
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_DROP_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueueDropEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.queue,
                                                      buf.flowctrl_cfg.enable);
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_FC_ON_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueuePauseThreshGroup_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                        buf.flowctrl_cfg.queue, &buf.flowctrl_cfg.dropThresh);
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_FC_ON_DROP_GROUP_SEL_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueuePauseDropThreshGroupSel_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.grp_idx);
            break;
      case RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_GROUP_SEL_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortQueueDropThreshGroupSel_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.grp_idx);
            break;
      case RTDRV_FLOWCTRL_EGR_QUEUE_DROP_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrQueueDropEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                        buf.flowctrl_cfg.queue, buf.flowctrl_cfg.enable);
            break;
      case RTDRV_FLOWCTRL_EGR_PORT_DROP_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      buf.flowctrl_cfg.enable);
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_DROP_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueueDropThreshGroup_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx, buf.flowctrl_cfg.queue,
                                                      &buf.flowctrl_cfg.dropThresh);
            break;

      case RTDRV_FLOWCTRL_PORT_HOL_TRAFFIC_DROP_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_portHolTrafficDropEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,\
                                                            buf.flowctrl_cfg.enable);
            break;

      case RTDRV_FLOWCTRL_HOL_TRAFFIC_TYPE_DROP_ENABLE_SET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_holTrafficTypeDropEnable_set(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.data,\
                                                            buf.flowctrl_cfg.enable);
            break;

    /** RATE **/
        case RTDRV_RATE_IGR_ENABLE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlEnable_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_IGR_INCLUDE_IFG_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlIncludeIfg_set(buf.rate_cfg.unit, buf.rate_cfg.ifg_include);
            break;

        case RTDRV_RATE_PORT_IGR_INCLUDE_IFG_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portIgrBandwidthCtrlIncludeIfg_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.ifg_include);
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_ENABLE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthFlowctrlEnable_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.ifg_include);
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_THRESH_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthFlowctrlThresh_set(buf.rate_cfg.unit, buf.rate_cfg.port, &(buf.rate_cfg.rate_thresh));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_LOW_THRESH_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthLowThresh_set(buf.rate_cfg.unit, buf.rate_cfg.thresh);
            break;

        case RTDRV_RATE_PORT_IGR_BANDWIDTH_CTRL_EXCEED_RESET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portIgrBandwidthCtrlExceed_reset(buf.rate_cfg.unit, buf.rate_cfg.port);
            break;

        case RTDRV_RATE_PORT_IGR_BANDWIDTH_FLOWCTRL_HIGH_THRESH_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portIgrBandwidthHighThresh_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.thresh);
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_BURST_SIZE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBwCtrlBurstSize_set(buf.rate_cfg.unit, buf.rate_cfg.thresh);
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_CTRL_FPENTRY_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlFPEntry_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.index, &(buf.rate_cfg.fpEntry));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_CTRL_BYPASS_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlBypass_set(buf.rate_cfg.unit, buf.rate_cfg.igrBypassType, buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_IGR_RATE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlRate_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.rate);
            break;

        case RTDRV_RATE_EGR_ENABLE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrBandwidthCtrlEnable_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_EGR_INCLUDE_IFG_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrBandwidthCtrlIncludeIfg_set(buf.rate_cfg.unit, buf.rate_cfg.ifg_include);
            break;

        case RTDRV_RATE_CPU_EGR_BANDWIDTH_CTRL_RATE_MODE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_cpuEgrBandwidthCtrlRateMode_set(buf.rate_cfg.unit, buf.rate_cfg.data);
            break;

        case RTDRV_RATE_PORT_EGR_INCLUDE_IFG_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portEgrBandwidthCtrlIncludeIfg_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.ifg_include);
            break;

        case RTDRV_RATE_PORT_EGR_BURST_SIZE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portEgrBandwidthCtrlBurstSize_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.data);
            break;

        case RTDRV_RATE_EGR_QUEUE_BWCTRL_ENABLE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueBwCtrlEnable_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_EGR_QUEUE_BWCTRL_RATE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueBwCtrlRate_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, buf.rate_cfg.rate);
            break;

        case RTDRV_RATE_EGR_PORT_QUEUE_BWCTRL_BURST_SIZE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrPortQueueBwCtrlBurstSize_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, buf.rate_cfg.data);
            break;

        case RTDRV_RATE_EGR_QUEUE_BWCTRL_BURST_SIZE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueBwCtrlBurstSize_set(buf.rate_cfg.unit, buf.rate_cfg.data);
            break;

        case RTDRV_RATE_EGR_QUEUE_FIX_BW_ENABLE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueFixedBandwidthEnable_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_EGR_RATE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrBandwidthCtrlRate_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.rate);
            break;

        case RTDRV_RATE_STROM_CTRL_RATE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlRate_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                buf.rate_cfg.rate);
            break;

        case RTDRV_RATE_STROM_CTRL_PROTO_RATE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlProtoRate_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_proto_type,
                                                buf.rate_cfg.rate);
            break;

        case RTDRV_RATE_STORM_CONTROL_ENABLE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlEnable_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_PORT_STORM_CONTROL_RATE_MODE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portStormControlRateMode_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                buf.rate_cfg.data);
            break;

        case RTDRV_RATE_STORM_CONTROL_RATE_MODE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlRateMode_set(buf.rate_cfg.unit, buf.rate_cfg.data);
            break;

        case RTDRV_RATE_STORM_CONTROL_BURST_RATE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlBurstRate_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                buf.rate_cfg.rate);
            break;

        case RTDRV_RATE_STORM_CONTROL_INCLUDE_IFG_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlIncludeIfg_set(buf.rate_cfg.unit,
                                                buf.rate_cfg.ifg_include);
            break;

        case RTDRV_RATE_STORM_CONTROL_REFRESH_MODE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlRefreshMode_set(buf.rate_cfg.unit,
                                                buf.rate_cfg.data);
            break;

        case RTDRV_RATE_STORM_CONTROL_TYPE_SEL_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlTypeSel_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                buf.rate_cfg.storm_sel);
            break;

        case RTDRV_RATE_STORM_CONTROL_BYPASS_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlBypass_set(buf.rate_cfg.unit, buf.rate_cfg.stormBypassType,
                                                buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_STORM_CONTROL_EXCEED_RESET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlExceed_reset(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type);
            break;

        case RTDRV_RATE_STORM_CONTROL_PROTO_EXCEED_RESET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlProtoExceed_reset(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_proto_type);
            break;

        case RTDRV_RATE_STORM_CONTROL_BURST_SIZE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlBurstSize_set(buf.rate_cfg.unit, buf.rate_cfg.storm_type, buf.rate_cfg.data);
            break;

        case RTDRV_RATE_IGR_QUEUE_BWCTRL_ENABLE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrQueueBwCtrlEnable_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, buf.rate_cfg.enable);
            break;

        case RTDRV_RATE_IGR_QUEUE_BWCTRL_RATE_SET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrQueueBwCtrlRate_set(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, buf.rate_cfg.rate);
            break;

        case RTDRV_RATE_IGR_QUEUE_BWCTRL_EXCEED_RESET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrQueueBwCtrlExceed_reset(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue);
            break;

    /** SVLAN **/
        case RTDRV_SVLAN_SVID_CREATE:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_create(buf.svlan_cfg.unit, buf.svlan_cfg.svid);
            break;

        case RTDRV_SVLAN_SVID_DESTROY:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_destroy(buf.svlan_cfg.unit, buf.svlan_cfg.svid);
            break;

        case RTDRV_SVLAN_MEMBER_SET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_memberPort_set(buf.svlan_cfg.unit, buf.svlan_cfg.svid, &buf.svlan_cfg.svlan_portmask);
            break;

        case RTDRV_SVLAN_MEMBER_ENTRY_SET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_memberPortEntry_set(buf.svlan_cfg.unit, buf.svlan_cfg.svid_idx, buf.svlan_cfg.svid,
                                                &buf.svlan_cfg.svlan_portmask);
            break;

        case RTDRV_SVLAN_TPID_ENTRY_SET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_tpidEntry_set(buf.svlan_cfg.unit, buf.svlan_cfg.svid_idx, buf.svlan_cfg.svlan_tag_id);
            break;

        case RTDRV_SVLAN_PORT_SVID_SET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_portSvid_set(buf.svlan_cfg.unit, buf.svlan_cfg.port, buf.svlan_cfg.svid);
            break;

        case RTDRV_SVLAN_SERVICE_PORT_SET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_servicePort_set(buf.svlan_cfg.unit, &buf.svlan_cfg.svlan_portmask);
            break;

    /** SWITCH **/
        case RTDRV_SWITCH_CPU_MAX_PKTLEN_SET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_cpuMaxPktLen_set(buf.switch_cfg.unit, buf.switch_cfg.dir, buf.switch_cfg.len);
            break;

        case RTDRV_SWITCH_MAX_PKTLEN_SET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_maxPktLen_set(buf.switch_cfg.unit, buf.switch_cfg.len);
            break;

        case RTDRV_SWITCH_MAX_PKTLEN_LINK_SPEED_SET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_maxPktLenLinkSpeed_set(buf.switch_cfg.unit, buf.switch_cfg.speed, buf.switch_cfg.maxLen);
            break;

        case RTDRV_SWITCH_MAX_PKTLEN_TAGLENCNT_SET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_maxPktLenTagLenCntIncEnable_set(buf.switch_cfg.unit, buf.switch_cfg.enable);
            break;

        case RTDRV_SWITCH_PORTMAXPKTLEN_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_portMaxPktLen_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.port, buf.switch_cfgParam.maxLen);
            break;

        case RTDRV_SWITCH_PORTSNAPMODE_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_portSnapMode_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.port, buf.switch_cfgParam.snapMode);
            break;

        case RTDRV_SWITCH_CHKSUMFAILACTION_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_chksumFailAction_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.port,
                buf.switch_cfgParam.failType, buf.switch_cfgParam.action);
            break;

        case RTDRV_SWITCH_RECALCCRCENABLE_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_recalcCRCEnable_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.port, buf.switch_cfgParam.enable);
            break;

        case RTDRV_SWITCH_MGMTVLANID_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_mgmtVlanId_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.mgmIvid);
            break;

        case RTDRV_SWITCH_OUTERMGMTVLANID_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_outerMgmtVlanId_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.mgmOvid);
            break;

        case RTDRV_SWITCH_MGMTMACADDR_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_mgmtMacAddr_set(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.mac);
            break;

        case RTDRV_SWITCH_IPV4ADDR_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_IPv4Addr_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.ipv4Addr);
            break;

        case RTDRV_SWITCH_IPV6ADDR_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_IPv6Addr_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.ipv6Addr);
            break;

        case RTDRV_SWITCH_DELAY_ENABLE_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_hwInterfaceDelayEnable_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.type, buf.switch_cfgInfo.enable);
            break;

        case RTDRV_SWITCH_PKT2CPU_FORMAT_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_pkt2CpuFormat_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.data);
            break;

        case RTDRV_SWITCH_PPPOE_PASS_THROUGH_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_pppoePassthrough_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.enable);
            break;

        case RTDRV_SWITCH_WATCHDOG_ENABLE_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = drv_watchdog_enable_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.enable);
            break;

        case RTDRV_SWITCH_WATCHDOG_MODE_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = drv_watchdog_mode_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.data);
            break;

        case RTDRV_SWITCH_WATCHDOG_SCALE_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = drv_watchdog_scale_set(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.data);
            break;

        case RTDRV_SWITCH_PKT2CPUTYPEFORMAT_SET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_pkt2CpuTypeFormat_set(buf.switch_cfgInfo.unit,
                    buf.switch_cfgInfo.type, buf.switch_cfgInfo.format);
            break;

        case RTDRV_SWITCH_CPU_PKT_TRUNCATE_EN_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_cpuPktTruncateEnable_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.enable);
            break;

        case RTDRV_SWITCH_CPU_PKT_TRUNCATE_LEN_SET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_cpuPktTruncateLen_set(buf.switch_cfgParam.unit, buf.switch_cfgParam.maxLen);
            break;

    /** NIC **/
        case RTDRV_NIC_RX_START:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_rx_start(buf.nic_cfg.unit);
            break;

        case RTDRV_NIC_RX_STOP:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_rx_stop(buf.nic_cfg.unit);
            break;

        case RTDRV_NIC_DEBUG_SET:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_dbg_set(buf.nic_cfg.unit, buf.nic_cfg.flags);
            break;

        case RTDRV_NIC_COUNTER_DUMP:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_cntr_dump(buf.nic_cfg.unit);
            break;

        case RTDRV_NIC_COUNTER_CLEAR:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_cntr_clear(buf.nic_cfg.unit);
            break;

        case RTDRV_NIC_BUFFER_DUMP:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_ringbuf_dump(buf.nic_cfg.unit);
            break;

        case RTDRV_NIC_PKTHDR_MBUF_DUMP:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_pkthdr_mbuf_dump(buf.nic_cfg.unit, buf.nic_cfg.mode, buf.nic_cfg.start,
                                              buf.nic_cfg.end, buf.nic_cfg.flags);
            break;

#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
        case RTDRV_NIC_CPU_ENTRY_ADD:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_pieCpuEntry_add(buf.nic_cfg.unit, buf.nic_cfg.index, &buf.nic_cfg.cpu_entry);
            break;

        case RTDRV_NIC_CPU_ENTRY_DEL:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_pieCpuEntry_del(buf.nic_cfg.unit, buf.nic_cfg.index);
            break;

        case RTDRV_NIC_CPU_ENTRY_SET:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_pieCpuEntry_set(buf.nic_cfg.unit, buf.nic_cfg.index, &buf.nic_cfg.cpu_entry);
            break;
#endif

#if (defined(CONFIG_SDK_DRIVER_TEST) || defined(CONFIG_SDK_DRIVER_TEST_MODULE))
    /** SDK **/
        case RTDRV_SDK_TEST:
            copy_from_user(&buf.sdk_cfg, user, sizeof(rtdrv_sdkCfg_t));
            ret = sdktest_run(buf.sdk_cfg.unit, buf.sdk_cfg.item);
            break;

        case RTDRV_SDK_TEST_ID:
            copy_from_user(&buf.sdk_cfg, user, sizeof(rtdrv_sdkCfg_t));
            ret = sdktest_run_id(buf.sdk_cfg.unit, buf.sdk_cfg.start, buf.sdk_cfg.end);
            break;

        case RTDRV_SDK_TEST_MODE_SET:
            copy_from_user(&buf.sdk_cfg, user, sizeof(rtdrv_sdkCfg_t));
            ret = sdktest_mode_set(buf.sdk_cfg.mode);
            break;

#endif

    /** EEE **/
        case RTDRV_EEE_PORT_ENABLE_SET:
            copy_from_user(&buf.eee_cfg, user, sizeof(rtdrv_eeeCfg_t));
            ret = rtk_eee_portEnable_set(buf.eee_cfg.unit, buf.eee_cfg.port, buf.eee_cfg.enable);
            break;

        case RTDRV_EEEP_PORT_ENABLE_SET:
            copy_from_user(&buf.eee_cfg, user, sizeof(rtdrv_eeeCfg_t));
            ret = rtk_eeep_portEnable_set(buf.eee_cfg.unit, buf.eee_cfg.port, buf.eee_cfg.enable);
            break;

    /** SEC **/
        case RTDRV_SEC_PORT_ATTACK_PREVENT_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portAttackPrevent_set(buf.sec_cfg.unit, buf.sec_cfg.port, buf.sec_cfg.attack_type,
                                    buf.sec_cfg.action);
            break;

        case RTDRV_SEC_PORT_MIN_IPV6_FRAG_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portMinIPv6FragLen_set(buf.sec_cfg.unit, buf.sec_cfg.port, buf.sec_cfg.data);
            break;

        case RTDRV_SEC_PORT_MAX_PING_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portMaxPingLen_set(buf.sec_cfg.unit, buf.sec_cfg.port, buf.sec_cfg.data);
            break;

        case RTDRV_SEC_PORT_MIN_TCP_HDR_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portMinTCPHdrLen_set(buf.sec_cfg.unit, buf.sec_cfg.port, buf.sec_cfg.data);
            break;

        case RTDRV_SEC_PORT_SMURF_NETMASK_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portSmurfNetmaskLen_set(buf.sec_cfg.unit, buf.sec_cfg.port, buf.sec_cfg.data);
            break;

        case RTDRV_SEC_PORT_ATTACK_PREVENT_ENABLE_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portAttackPreventEnable_set(buf.sec_cfg.unit, buf.sec_cfg.port, buf.sec_cfg.enable);
            break;

        case RTDRV_SEC_ATTACK_PREVENT_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_attackPreventAction_set(buf.sec_cfg.unit, buf.sec_cfg.attack_type,
                                    buf.sec_cfg.action);
            break;

        case RTDRV_SEC_MIN_IPV6_FRAG_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_minIPv6FragLen_set(buf.sec_cfg.unit, buf.sec_cfg.data);
            break;

        case RTDRV_SEC_MAX_PING_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_maxPingLen_set(buf.sec_cfg.unit, buf.sec_cfg.data);
            break;

        case RTDRV_SEC_MIN_TCP_HDR_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_minTCPHdrLen_set(buf.sec_cfg.unit, buf.sec_cfg.data);
            break;

        case RTDRV_SEC_SMURF_NETMASK_LEN_SET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_smurfNetmaskLen_set(buf.sec_cfg.unit, buf.sec_cfg.data);
            break;

    /** LED **/
        case RTDRV_LED_SYS_ENABLE_SET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_sysEnable_set(buf.led_cfg.unit, buf.led_cfg.type, buf.led_cfg.enable);
            break;

        case RTDRV_LED_PORT_ENABLE_SET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_portEnable_set(buf.led_cfg.unit, buf.led_cfg.port, buf.led_cfg.enable);
            break;

        case RTDRV_LED_PORTLEDENTITYSWCTRLENABLE_SET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_portLedEntitySwCtrlEnable_set(buf.led_cfg.unit,
                    buf.led_cfg.port, buf.led_cfg.entity, buf.led_cfg.enable);
            break;

        case RTDRV_LED_SWCTRL_START:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_swCtrl_start(buf.led_cfg.unit);
            break;

        case RTDRV_LED_PORTLEDENTITYSWCTRLMODE_SET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_portLedEntitySwCtrlMode_set(buf.led_cfg.unit,
                    buf.led_cfg.port, buf.led_cfg.entity, buf.led_cfg.media,
                    buf.led_cfg.mode);
            break;

        case RTDRV_LED_SYSMODE_SET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_sysMode_set(buf.led_cfg.unit, buf.led_cfg.mode);
            break;

        /* MPLS */
        case RTDRV_MPLS_INIT:
            copy_from_user(&buf.mpls_cfg, user, sizeof(rtdrv_mplsCfg_t));
            ret = rtk_mpls_init(buf.mpls_cfg.unit);
            break;
        case RTDRV_MPLS_TTLINHERIT_SET:
            copy_from_user(&buf.mpls_cfg, user, sizeof(rtdrv_mplsCfg_t));
            ret = rtk_mpls_ttlInherit_set(buf.mpls_cfg.unit, buf.mpls_cfg.u.inherit);
            break;
        case RTDRV_MPLS_ENCAP_SET:
            copy_from_user(&buf.mpls_cfg, user, sizeof(rtdrv_mplsCfg_t));
            ret = rtk_mpls_encap_set(buf.mpls_cfg.unit, buf.mpls_cfg.lib_idx,
                    &buf.mpls_cfg.u.encap_info);
            break;
        case RTDRV_MPLS_ENABLE_SET:
            copy_from_user(&buf.mpls_cfg, user, sizeof(rtdrv_mplsCfg_t));
            ret = rtk_mpls_enable_set(buf.mpls_cfg.unit, buf.mpls_cfg.enable);
            break;

#if defined(CONFIG_SDK_RTL8231)
    /** RTL8231 **/
        case RTDRV_RTL8231_I2C_WRITE:
            copy_from_user(&buf.rtl8231_cfg, user, sizeof(rtdrv_rtl8231Cfg_t));
            ret = drv_rtl8231_i2c_write(buf.rtl8231_cfg.unit, buf.rtl8231_cfg.phyId_or_slaveAddr, buf.rtl8231_cfg.reg_addr, buf.rtl8231_cfg.data);
            break;

        case RTDRV_RTL8231_MDC_WRITE:
            copy_from_user(&buf.rtl8231_cfg, user, sizeof(rtdrv_rtl8231Cfg_t));
            ret = drv_rtl8231_mdc_write(buf.rtl8231_cfg.unit, buf.rtl8231_cfg.phyId_or_slaveAddr, buf.rtl8231_cfg.page, buf.rtl8231_cfg.reg_addr, buf.rtl8231_cfg.data);
            break;

        case RTDRV_EXTGPIO_DEV_INIT:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_dev_init(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, &buf.extGpio_cfg.extGpio_devConfData);
            break;

        case RTDRV_EXTGPIO_DEV_ENABLE_SET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_devEnable_set(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.data);
            break;

        case RTDRV_EXTGPIO_SYNC_ENABLE_SET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_syncEnable_set(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.data);
            break;

        case RTDRV_EXTGPIO_SYNC_START:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_sync_start(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev);
            break;

        case RTDRV_EXTGPIO_PIN_INIT:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_pin_init(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.gpioId, &buf.extGpio_cfg.extGpio_confData);
            break;

        case RTDRV_EXTGPIO_DATABIT_SET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_dataBit_set(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.gpioId, buf.extGpio_cfg.data);
            break;

        case RTDRV_EXTGPIO_REG_WRITE:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_reg_write(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.reg, buf.extGpio_cfg.data);
            break;

        case RTDRV_EXTGPIO_DIRECTION_SET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_direction_set(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.gpioId, buf.extGpio_cfg.data);
            break;

        case RTDRV_EXT_SMI_INIT:
            copy_from_user(&buf.extSmi_cfg, user, sizeof(rtdrv_extSmiCfg_t));
            ret = drv_extSmi_dev_init(buf.extSmi_cfg.unit, buf.extSmi_cfg.periferal, buf.extSmi_cfg.phyAddrSCK,
                                                                                    buf.extSmi_cfg.phyAddrSDA,
                                                                                    buf.extSmi_cfg.gpioIdSCK,
                                                                                    buf.extSmi_cfg.gpioIdSDA);
            break;

        case RTDRV_EXT_SMI_WRITE:
            copy_from_user(&buf.extSmi_cfg, user, sizeof(rtdrv_extSmiCfg_t));
            ret = drv_extSmi_write(buf.extSmi_cfg.unit, buf.extSmi_cfg.periferal, buf.extSmi_cfg.addrs, buf.extSmi_cfg.rdata);
            break;

        case RTDRV_EXTGPIO_I2C_INIT:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_i2c_init(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.gpioId, buf.extGpio_cfg.data);
            break;

        case RTDRV_EXTGPIO_I2C_WRITE:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_i2c_write(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.reg, buf.extGpio_cfg.data);
            break;
#endif

    /** Internal GPIO **/
        case RTDRV_GPIO_PIN_INIT:
            copy_from_user(&buf.gpio_cfg, user, sizeof(rtdrv_gpioCfg_t));
            ret = drv_gpio_init(buf.gpio_cfg.gpioId, buf.gpio_cfg.function, buf.gpio_cfg.direction, buf.gpio_cfg.interruptEnable);
            break;

        case RTDRV_GPIO_DATABIT_INIT:
            copy_from_user(&buf.gpio_cfg, user, sizeof(rtdrv_gpioCfg_t));
            ret = drv_gpio_dataBit_init(buf.gpio_cfg.gpioId, buf.gpio_cfg.data);
            break;

        case RTDRV_GPIO_DATABIT_SET:
            copy_from_user(&buf.gpio_cfg, user, sizeof(rtdrv_gpioCfg_t));
            ret = drv_gpio_dataBit_set(buf.gpio_cfg.gpioId, buf.gpio_cfg.data);
            break;

		case RTDRV_GENCTRL_GPIO_DEV_INIT:
			copy_from_user(&buf.genCtrlGPIO_cfg, user, sizeof(rtdrv_generalCtrlGpioCfg_t));
			ret = drv_generalCtrlGPIO_dev_init(buf.genCtrlGPIO_cfg.unit, buf.genCtrlGPIO_cfg.dev, buf.genCtrlGPIO_cfg.gpioId, &buf.genCtrlGPIO_cfg.genCtrl_gpioDev);
			break;
		case RTDRV_GENCTRL_GPIO_PIN_INIT:
			copy_from_user(&buf.genCtrlGPIO_cfg, user, sizeof(rtdrv_generalCtrlGpioCfg_t));
			ret = drv_generalCtrlGPIO_pin_init(buf.genCtrlGPIO_cfg.unit, buf.genCtrlGPIO_cfg.dev, buf.genCtrlGPIO_cfg.gpioId, &buf.genCtrlGPIO_cfg.genCtrl_gpioPin);
			break;
		case RTDRV_GENCTRL_GPIO_DEV_ENABLE:
			copy_from_user(&buf.genCtrlGPIO_cfg, user, sizeof(rtdrv_generalCtrlGpioCfg_t));
			ret = drv_generalCtrlGPIO_devEnable_set(buf.genCtrlGPIO_cfg.unit, buf.genCtrlGPIO_cfg.dev, buf.genCtrlGPIO_cfg.data);
			break;
		case RTDRV_GENCTRL_GPIO_DATABIT_SET:
			copy_from_user(&buf.genCtrlGPIO_cfg, user, sizeof(rtdrv_generalCtrlGpioCfg_t));
			ret = drv_generalCtrlGPIO_dataBit_set(buf.genCtrlGPIO_cfg.unit, buf.genCtrlGPIO_cfg.dev, buf.genCtrlGPIO_cfg.gpioId, buf.genCtrlGPIO_cfg.data);
			break;


    /**SMI**/
        case RTDRV_SMI_INIT:
            copy_from_user(&buf.smi_cfg, user, sizeof(rtdrv_smiCfg_t));
            ret = drv_smi_init(buf.smi_cfg.portSCK, buf.smi_cfg.pinSCK, buf.smi_cfg.portSDA, buf.smi_cfg.pinSDA, buf.smi_cfg.dev);
            break;

        case RTDRV_SMI_TYPE_SET:
            copy_from_user(&buf.smi_cfg, user, sizeof(rtdrv_smiCfg_t));
            ret = drv_smi_type_set(buf.smi_cfg.type, buf.smi_cfg.chipid, buf.smi_cfg.delay, buf.smi_cfg.dev);
            break;

        case RTDRV_SMI_WRITE:
            copy_from_user(&buf.smi_cfg, user, sizeof(rtdrv_smiCfg_t));
            ret = drv_smi_write(buf.smi_cfg.addrs, buf.smi_cfg.rdata, buf.smi_cfg.dev);
            break;

        /* DIAG */
        case RTDRV_DIAG_MAC_REMOTE_LOOPBACK_SET:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_portMacRemoteLoopbackEnable_set(buf.diag_cfg.unit, buf.diag_cfg.port, buf.diag_cfg.enable);
            break;
        case RTDRV_DIAG_MAC_LOCAL_LOOPBACK_SET:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_portMacLocalLoopbackEnable_set(buf.diag_cfg.unit, buf.diag_cfg.port, buf.diag_cfg.enable);
            copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
            break;
        case RTDRV_DIAG_RTCTENABLE_SET:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_rtctEnable_set(buf.diag_cfg.unit, &buf.diag_cfg.portmask);
            break;

        default:
            break;
    }

	return ret;
}

/* Function Name:
 *      do_rtdrv_get_ctl
 * Description:
 *      This function is called whenever a process tries to do getsockopt
 * Input:
 *      *sk   - network layer representation of sockets
 *      cmd   - ioctl commands
 * Output:
 *      *user - data buffer handled between user and kernel space
 *      len   - data length
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
int32 do_rtdrv_get_ctl(struct sock *sk, int cmd, void *user, int *len)
{
    int32                           ret = RT_ERR_FAILED;
    rtdrv_union_t       buf;


    switch(cmd)
    {
    /** L2 **/
        case RTDRV_L2_LCPORT_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_l2_portLearningCnt_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_L2_ADDR_GET:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_addr_get(buf.l2_data.unit, &buf.l2_data.data);
            copy_to_user(user, &buf.l2_data, sizeof(rtdrv_l2_addrData_t));
            break;

        case RTDRV_L2_ADDR_GETNEXT:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_nextValidAddr_get(buf.l2_data.unit, &buf.l2_data.index, buf.l2_data.static_flag,
                                           &buf.l2_data.data);
            copy_to_user(user, &buf.l2_data, sizeof(rtdrv_l2_addrData_t));
            break;

        case RTDRV_L2_AGING_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_l2_aging_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_L2_PORT_AGING_ENABLE_GET:
            copy_from_user(&buf.age_cfg, user, sizeof(rtdrv_l2_ageCfg_t));
            ret = rtk_l2_portAgingEnable_get(buf.age_cfg.unit, buf.age_cfg.port, &buf.age_cfg.enable);
            copy_to_user(user, &buf.age_cfg, sizeof(rtdrv_l2_ageCfg_t));
            break;

        case RTDRV_L2_EN_FLUSH_LINK_DOWN_PORT_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_l2_flushLinkDownPortAddrEnable_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ENABLE_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLimitLearningCntEnable_get(buf.l2_learn.unit, buf.l2_learn.port, &buf.l2_learn.enable);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_PORTLASTLEARNEDMAC_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLastLearnedMac_get(buf.l2_learn.unit, buf.l2_learn.port, &buf.l2_learn.fid, &buf.l2_learn.mac);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_EN_TRAP_IGMP_CTRL_2CPU_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_igmpCtrlPkt2CpuEnable_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_L2_EN_TRAP_1X_MAC_CHANGE_2CPU_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_1xMacChangePort2CpuEnable_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_L2_EN_TRAP_IP_MCAST_2CPU_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_ipMcastPkt2CpuEnable_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_L2_EN_TRAP_L2_MCAST_2CPU_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_trap_l2McastPkt2CpuEnable_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_L2_IP_MCAST_ADDR_GET:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_ipMcastAddr_get(buf.mcast_data.unit, &buf.mcast_data.ip_m_data);
            copy_to_user(user, &buf.mcast_data, sizeof(rtdrv_l2_mcastAddrData_t));
            break;

        case RTDRV_L2_IP_MCAST_ADDR_GETNEXT:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_nextValidIpMcastAddr_get(buf.mcast_data.unit, &buf.mcast_data.index, &buf.mcast_data.ip_m_data);
            copy_to_user(user, &buf.mcast_data, sizeof(rtdrv_l2_mcastAddrData_t));
            break;

        case RTDRV_L2_MCAST_ADDR_GET:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_mcastAddr_get(buf.mcast_data.unit, &buf.mcast_data.m_data);
            copy_to_user(user, &buf.mcast_data, sizeof(rtdrv_l2_mcastAddrData_t));
            break;

        case RTDRV_L2_MCAST_ADDR_GETNEXT:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_nextValidMcastAddr_get(buf.mcast_data.unit, &buf.mcast_data.index, &buf.mcast_data.m_data);
            copy_to_user(user, &buf.mcast_data, sizeof(rtdrv_l2_mcastAddrData_t));
            break;

        case RTDRV_L2_LEARNING_CNT_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_learningCnt_get(buf.l2_learn.unit, &buf.l2_learn.mac_cnt);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_PORT_LEARNING_CNT_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLearningCnt_get(buf.l2_learn.unit, buf.l2_learn.port, &buf.l2_learn.mac_cnt);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_LIMIT_LEARNING_CNT_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_limitLearningCnt_get(buf.l2_learn.unit, &buf.l2_learn.mac_cnt);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_PORT_LIMIT_LEARNING_CNT_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLimitLearningCnt_get(buf.l2_learn.unit, buf.l2_learn.port, &buf.l2_learn.mac_cnt);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_LIMIT_LEARNING_CNT_ACTION_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_limitLearningCntAction_get(buf.l2_learn.unit, &buf.l2_learn.action);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_PORT_LIMIT_LEARNING_CNT_ACTION_GET:
            copy_from_user(&buf.l2_learn, user, sizeof(rtdrv_l2_learnCnt_t));
            ret = rtk_l2_portLimitLearningCntAction_get(buf.l2_learn.unit, buf.l2_learn.port, &buf.l2_learn.action);
            copy_to_user(user, &buf.l2_learn, sizeof(rtdrv_l2_learnCnt_t));
            break;

        case RTDRV_L2_LOOKUP_MISS_FLOOD_GET:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_lookupMissFloodPortMask_get(buf.l2_data.unit, buf.l2_data.type, &buf.l2_data.portmask);
            copy_to_user(user, &buf.l2_data, sizeof(rtdrv_l2_addrData_t));
            break;

        case RTDRV_L2_SRC_PORT_EGR_FILTER_GET:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_srcPortEgrFilterMask_get(buf.l2_data.unit, &buf.l2_data.portmask);
            copy_to_user(user, &buf.l2_data, sizeof(rtdrv_l2_addrData_t));
            break;

        case RTDRV_L2_LEGAL_MOVETO_PORTMASK_GET:
            copy_from_user(&buf.l2_data, user, sizeof(rtdrv_l2_addrData_t));
            ret = rtk_l2_legalMoveToPorts_get(buf.l2_data.unit, buf.l2_data.port, &buf.l2_data.portmask);
            copy_to_user(user, &buf.l2_data, sizeof(rtdrv_l2_addrData_t));
            break;

        case RTDRV_L2_LEGAL_MOVETO_ACTION_GET:
            copy_from_user(&buf.l2_action, user, sizeof(rtdrv_l2_portAct_t));
            ret = rtk_l2_legalPortMoveAction_get(buf.l2_action.unit, buf.l2_action.port, &buf.l2_action.action);
            copy_to_user(user, &buf.l2_action, sizeof(rtdrv_l2_portAct_t));
            break;

        case RTDRV_L2_LEGAL_MOVETO_FLUSH_ENABLE_GET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_legalPortMoveFlushAddrEnable_get(buf.l2_common.unit, buf.l2_common.port, &buf.l2_common.value);
            copy_to_user(user, &buf.l2_common, sizeof(rtdrv_l2_common_t));
            break;

        case RTDRV_L2_ILLEGAL_MOVETO_ACTION_GET:
            copy_from_user(&buf.l2_action, user, sizeof(rtdrv_l2_portAct_t));
            ret = rtk_l2_illegalPortMoveAction_get(buf.l2_action.unit, buf.l2_action.port, &buf.l2_action.action);
            copy_to_user(user, &buf.l2_action, sizeof(rtdrv_l2_portAct_t));
            break;

        case RTDRV_L2_STTC_PORT_MOVE_ACTION_GET:
            copy_from_user(&buf.l2_action, user, sizeof(rtdrv_l2_portAct_t));
            ret = rtk_l2_staticPortMoveAction_get(buf.l2_action.unit, buf.l2_action.port, &buf.l2_action.action);
            copy_to_user(user, &buf.l2_action, sizeof(rtdrv_l2_portAct_t));
            break;

        case RTDRV_L2_LOOKUP_MISS_FLOODPORTMASK_IDX_GET:
            copy_from_user(&buf.l2_lkMiss, user, sizeof(rtdrv_l2_lkMiss_t));
            ret = rtk_l2_lookupMissFloodPortMaskIdx_get(buf.l2_lkMiss.unit, buf.l2_lkMiss.type, &buf.l2_lkMiss.index);
            copy_to_user(user, &buf.l2_lkMiss, sizeof(rtdrv_l2_lkMiss_t));
            break;

        case RTDRV_L2_EXCEPTION_SA_ACTION_GET:
            copy_from_user(&buf.l2_exceptSa, user, sizeof(rtdrv_l2_exceptSa_t));
            ret = rtk_l2_exceptionAddrAction_get(buf.l2_exceptSa.unit, buf.l2_exceptSa.type, &buf.l2_exceptSa.action);
            copy_to_user(user, &buf.l2_exceptSa, sizeof(rtdrv_l2_exceptSa_t));
            break;

        case RTDRV_L2_FIDLIMITLEARNINGENTRY_GET :
            copy_from_user(&buf.l2_FidLearn, user, sizeof(rtdrv_l2_learnFidCnt_t));
            ret = rtk_l2_fidLimitLearningEntry_get(buf.l2_FidLearn.unit, buf.l2_FidLearn.entryIdx, &buf.l2_FidLearn.fidMacLimitEntry);
            copy_to_user(user, &buf.l2_FidLearn, sizeof(rtdrv_l2_learnFidCnt_t));
            break;

        case RTDRV_L2_FIDLEARNINGCNT_GET:
            copy_from_user(&buf.l2_FidLearn, user, sizeof(rtdrv_l2_learnFidCnt_t));
            ret = rtk_l2_fidLearningCnt_get(buf.l2_FidLearn.unit, buf.l2_FidLearn.entryIdx, &buf.l2_FidLearn.mac_cnt);
            copy_to_user(user, &buf.l2_FidLearn, sizeof(rtdrv_l2_learnFidCnt_t));
            break;

        case RTDRV_L2_FIDLASTLEARNEDMAC_GET:
            copy_from_user(&buf.l2_FidLearn, user, sizeof(rtdrv_l2_learnFidCnt_t));
            ret = rtk_l2_fidLastLearnedMac_get(buf.l2_FidLearn.unit, buf.l2_FidLearn.entryIdx, &buf.l2_FidLearn.fid, &buf.l2_FidLearn.mac);
            copy_to_user(user, &buf.l2_FidLearn, sizeof(rtdrv_l2_learnFidCnt_t));
            break;

        case RTDRV_L2_FID_LIMIT_LEARNING_CNT_ACTION_GET:
            copy_from_user(&buf.l2_FidLearn, user, sizeof(rtdrv_l2_learnFidCnt_t));
            ret = rtk_l2_fidLearningCntAction_get(buf.l2_FidLearn.unit, &buf.l2_FidLearn.action);
            copy_to_user(user, &buf.l2_FidLearn, sizeof(rtdrv_l2_learnFidCnt_t));
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPPRI_GET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapPri_get(buf.l2_learn_priDp.unit, &buf.l2_learn_priDp.priority);
            copy_to_user(user, &buf.l2_learn_priDp, sizeof(rtdrv_l2_learnPriDp_t));
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPPRIENABLE_GET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapPriEnable_get(buf.l2_learn_priDp.unit, &buf.l2_learn_priDp.enable);
            copy_to_user(user, &buf.l2_learn_priDp, sizeof(rtdrv_l2_learnPriDp_t));
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPDP_GET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapDP_get(buf.l2_learn_priDp.unit, &buf.l2_learn_priDp.dpValue);
            copy_to_user(user, &buf.l2_learn_priDp, sizeof(rtdrv_l2_learnPriDp_t));
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPDPENABLE_GET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapDPEnable_get(buf.l2_learn_priDp.unit, &buf.l2_learn_priDp.enable);
            copy_to_user(user, &buf.l2_learn_priDp, sizeof(rtdrv_l2_learnPriDp_t));
            break;

        case RTDRV_L2_LIMITLEARNINGTRAPADDCPUTAGENABLE_GET:
            copy_from_user(&buf.l2_learn_priDp, user, sizeof(rtdrv_l2_learnPriDp_t));
            ret = rtk_l2_limitLearningTrapAddCPUTagEnable_get(buf.l2_learn_priDp.unit, &buf.l2_learn_priDp.insertCpuTag);
            copy_to_user(user, &buf.l2_learn_priDp, sizeof(rtdrv_l2_learnPriDp_t));
            break;

        case RTDRV_L2_CAMENABLE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_camEnable_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.enable);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_HASHALGO_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_hashAlgo_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.hash_algo);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_HASHCAREBYTE_GET:
            copy_from_user(&buf.l2_hashCareByte, user, sizeof(rtdrv_l2_hashCareByte_t));
            ret = rtk_l2_ip6CareByte_get(buf.l2_hashCareByte.unit, buf.l2_hashCareByte.type, &buf.l2_hashCareByte.value);
            copy_to_user(user, &buf.l2_hashCareByte, sizeof(rtdrv_l2_hashCareByte_t));
            break;

        case RTDRV_L2_VLANMODE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_vlanMode_get(buf.l2_learnCfg.unit, buf.l2_learnCfg.port, &buf.l2_learnCfg.vlanMode);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_LEARNINGENABLE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_learningEnable_get(buf.l2_learnCfg.unit, buf.l2_learnCfg.port, &buf.l2_learnCfg.enable);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_NEWMACOP_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_newMacOp_get(buf.l2_learnCfg.unit, buf.l2_learnCfg.port, &buf.l2_learnCfg.lrnMode, &buf.l2_learnCfg.fwdAction);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_LRUENABLE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_LRUEnable_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.enable);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_UCASTLOOKUPMODE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ucastLookupMode_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.ucast_lookupMode);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_MCASTLOOKUPMODE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_mcastLookupMode_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.mcast_lookupMode, &buf.l2_learnCfg.fixed_fid);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_IPMCENABLE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ipmcEnable_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.enable);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_IPMCMODE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ipmcMode_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.ipmcMode);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_IPMC_DIP_CHK_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_ipMcastAddrChkEnable_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.dip_check);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_IPMC_VLAN_COMPARE_GET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_ipMcstFidVidCompareEnable_get(buf.l2_common.unit, &buf.l2_common.value);
            copy_to_user(user, &buf.l2_common, sizeof(rtdrv_l2_common_t));
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHACTION_GET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchAction_get(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, &buf.l2_ipmcCfg.mismatch_action);
            copy_to_user(user, &buf.l2_ipmcCfg, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHPRI_GET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchPri_get(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, &buf.l2_ipmcCfg.priority);
            copy_to_user(user, &buf.l2_ipmcCfg, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            break;

            case RTDRV_L2_IPMCDSTADDRMISMATCHPRIENABLE_GET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchPriEnable_get(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, &buf.l2_ipmcCfg.enable);
            copy_to_user(user, &buf.l2_ipmcCfg, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHDP_GET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchDP_get(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, &buf.l2_ipmcCfg.dpValue);
            copy_to_user(user, &buf.l2_ipmcCfg, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHDPENABLE_GET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchDPEnable_get(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, &buf.l2_ipmcCfg.enable);
            copy_to_user(user, &buf.l2_ipmcCfg, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            break;

        case RTDRV_L2_IPMCDSTADDRMISMATCHADDCPUTAGENABLE_GET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmcDstAddrMismatchAddCPUTagEnable_get(buf.l2_ipmcCfg.unit, buf.l2_ipmcCfg.type, &buf.l2_ipmcCfg.insertCpuTag);
            copy_to_user(user, &buf.l2_ipmcCfg, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            break;

        case RTDRV_L2_LOOKUP_MISS_ACTION_GET:
            copy_from_user(&buf.l2_lkMiss, user, sizeof(rtdrv_l2_lkMiss_t));
            ret = rtk_l2_lookupMissAction_get(buf.l2_lkMiss.unit, buf.l2_lkMiss.type, &buf.l2_lkMiss.action);
            copy_to_user(user, &buf.l2_lkMiss, sizeof(rtdrv_l2_lkMiss_t));
            break;

        case RTDRV_L2_PORT_LOOKUP_MISS_ACTION_GET:
            copy_from_user(&buf.l2_lkMiss, user, sizeof(rtdrv_l2_lkMiss_t));
            ret = rtk_l2_portLookupMissAction_get(buf.l2_lkMiss.unit, buf.l2_lkMiss.port, buf.l2_lkMiss.type, &buf.l2_lkMiss.action);
            copy_to_user(user, &buf.l2_lkMiss, sizeof(rtdrv_l2_lkMiss_t));
            break;

        case RTDRV_L2_LOOKUPMISSPRI_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissPri_get(buf.l2_pri.unit, &buf.l2_pri.pri);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_LOOKUPMISSPRIENABLE_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissPriEnable_get(buf.l2_pri.unit, &buf.l2_pri.enable);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_LOOKUPMISSDP_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissDP_get(buf.l2_pri.unit, &buf.l2_pri.dpValue);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_LOOKUPMISSDPENABLE_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissDPEnable_get(buf.l2_pri.unit, &buf.l2_pri.enable);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_LOOKUPMISSADDCPUTAGENABLE_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_lookupMissAddCPUTagEnable_get(buf.l2_pri.unit, &buf.l2_pri.insertCpuTag);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_IPMC_ROUTERPORTS_GET:
            copy_from_user(&buf.l2_ipmcCfg, user, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            ret = rtk_l2_ipmc_routerPorts_get(buf.l2_ipmcCfg.unit, &buf.l2_ipmcCfg.router_portMask);
            copy_to_user(user, &buf.l2_ipmcCfg, sizeof(rtdrv_l2_ipmcMismatchCfg_t));
            break;

        case RTDRV_L2_TRAPPRI_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_trapPri_get(buf.l2_pri.unit, &buf.l2_pri.pri);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_TRAPPRIENABLE_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_trapPriEnable_get(buf.l2_pri.unit, &buf.l2_pri.enable);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_TRAPADDCPUTAGENABLE_GET:
            copy_from_user(&buf.l2_pri, user, sizeof(rtdrv_l2_pri_t));
            ret = rtk_l2_trapAddCPUTagEnable_get(buf.l2_pri.unit, &buf.l2_pri.insertCpuTag);
            copy_to_user(user, &buf.l2_pri, sizeof(rtdrv_l2_pri_t));
            break;

        case RTDRV_L2_MCASTFWDPORTMASK_GET:
            copy_from_user(&buf.l2_fwdEntryContent, user, sizeof(rtdrv_l2_fwdTblEntry_t));
            ret = rtk_l2_mcastFwdPortmask_get(buf.l2_fwdEntryContent.unit,
                buf.l2_fwdEntryContent.entryIdx, &buf.l2_fwdEntryContent.portMask, &buf.l2_fwdEntryContent.crossVlan);
            copy_to_user(user, &buf.l2_fwdEntryContent, sizeof(rtdrv_l2_fwdTblEntry_t));
            break;

        case RTDRV_L2_MCASTBLOCKPORTMASK_GET:
            copy_from_user(&buf.l2_portmaskCfg, user, sizeof(rtdrv_l2_portmaskCfg_t));
            ret = rtk_l2_mcastBlockPortmask_get(buf.l2_portmaskCfg.unit, &buf.l2_portmaskCfg.portMask);
            copy_to_user(user, &buf.l2_portmaskCfg, sizeof(rtdrv_l2_portmaskCfg_t));
            break;

        case RTDRV_L2_ZEROSALEARNINGENABLE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_zeroSALearningEnable_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.enable);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_NOTIFICATIONENABLE_GET:
            copy_from_user(&buf.l2_learnCfg, user, sizeof(rtdrv_l2_learnCfg_t));
            ret = rtk_l2_notificationEnable_get(buf.l2_learnCfg.unit, &buf.l2_learnCfg.enable);
            copy_to_user(user, &buf.l2_learnCfg, sizeof(rtdrv_l2_learnCfg_t));
            break;

        case RTDRV_L2_NOTIFICATIONENABLE_EVENT_TYPE_GET:
            copy_from_user(&buf.l2_notifyEventCfg, user, sizeof(rtdrv_l2_notifyEventCfg_t));
            ret = rtk_l2_notificationEventEnable_get(buf.l2_notifyEventCfg.unit, buf.l2_notifyEventCfg.event, &buf.l2_notifyEventCfg.enable);
            copy_to_user(user, &buf.l2_notifyEventCfg, sizeof(rtdrv_l2_notifyEventCfg_t));
            break;

        case RTDRV_L2_NOTIFICATION_BACKPRESSURE_THRESH_GET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_notificationBackPressureThresh_get(buf.l2_common.unit, &buf.l2_common.value);
            copy_to_user(user, &buf.l2_common, sizeof(rtdrv_l2_common_t));
            break;

        case RTDRV_L2_SECURE_MAC_MODE_GET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_secureMacMode_get(buf.l2_common.unit, &buf.l2_common.value);
            copy_to_user(user, &buf.l2_common, sizeof(rtdrv_l2_common_t));
            break;

        case RTDRV_L2_IPMCASTADDR_GET_WITH_INDEX:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_ipMcastAddr_get_with_index(buf.mcast_data.unit, &buf.mcast_data.ip_m_data);
            copy_to_user(user, &buf.mcast_data, sizeof(rtdrv_l2_mcastAddrData_t));
            break;

        case RTDRV_L2_IP6MCASTMODE_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_l2_ip6mcMode_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_L2_IP6_MCAST_ADDR_GET:
            copy_from_user(&buf.ip6Mcast_data, user, sizeof(rtdrv_l2_ip6McstAddrData_t));
            ret = rtk_l2_ip6McastAddr_get(buf.ip6Mcast_data.unit, &buf.ip6Mcast_data.ip6_m_data);
            copy_to_user(user, &buf.ip6Mcast_data, sizeof(rtdrv_l2_ip6McstAddrData_t));
            break;

        case RTDRV_L2_IP6_MCAST_ADDR_GETNEXT:
            copy_from_user(&buf.ip6Mcast_data, user, sizeof(rtdrv_l2_ip6McstAddrData_t));
            ret = rtk_l2_nextValidIp6McastAddr_get(buf.ip6Mcast_data.unit, &buf.ip6Mcast_data.index, &buf.ip6Mcast_data.ip6_m_data);
            copy_to_user(user, &buf.ip6Mcast_data, sizeof(rtdrv_l2_ip6McstAddrData_t));
            break;

        case RTDRV_L2_MCASTADDR_GET_WITH_INDEX:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_mcastAddr_get_with_index(buf.mcast_data.unit, &(buf.mcast_data.m_data));
            copy_to_user(user, &buf.mcast_data, sizeof(rtdrv_l2_mcastAddrData_t));
            break;

        case RTDRV_L2_MCASTFWDINDEX_ALLOC:
            copy_from_user(&buf.mcast_data, user, sizeof(rtdrv_l2_mcastAddrData_t));
            ret = rtk_l2_mcastFwdIndex_alloc(buf.mcast_data.unit, &(buf.mcast_data.fwdIndex));
            copy_to_user(user, &buf.mcast_data, sizeof(rtdrv_l2_mcastAddrData_t));
            break;

        case RTDRV_L2_MCASTFWDINDEXFREECOUNT_GET:
            copy_from_user(&buf.l2_fwdEntryContent, user, sizeof(rtdrv_l2_fwdTblEntry_t));
            ret = rtk_l2_mcastFwdIndexFreeCount_get(buf.l2_fwdEntryContent.unit, &buf.l2_fwdEntryContent.freeCount);
            copy_to_user(user, &buf.l2_fwdEntryContent, sizeof(rtdrv_l2_fwdTblEntry_t));
            break;

        case RTDRV_L2_ADDRENTRY_GET:
            copy_from_user(&buf.l2entry_data, user, sizeof(rtdrv_l2_entry_t));
            ret = rtk_l2_addrEntry_get(buf.l2entry_data.unit, buf.l2entry_data.index, &buf.l2entry_data.entry);
            copy_to_user(user, &buf.l2entry_data, sizeof(rtdrv_l2_entry_t));
            break;

        case RTDRV_L2_CONFLICT_ADDR_GET:
            copy_from_user(&buf.l2_conflict_data, user, sizeof(rtdrv_l2_conflict_t));
            ret = rtk_l2_conflictAddr_get(buf.l2_conflict_data.unit, &buf.l2_conflict_data.input, &buf.l2_conflict_data.output[0], buf.l2_conflict_data.size_of_output_buf, &buf.l2_conflict_data.ret_valid_cnt);
            copy_to_user(user, &buf.l2_conflict_data, sizeof(rtdrv_l2_conflict_t));
            break;

        case RTDRV_L2_PORT_DYNM_PORTMOVE_FORBID_ENABLE_GET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_portDynamicPortMoveForbidEnable_get(buf.l2_common.unit, buf.l2_common.port, &buf.l2_common.value);
            copy_to_user(user, &buf.l2_common, sizeof(rtdrv_l2_common_t));
            break;

        case RTDRV_L2_DYNM_PORTMOVE_FORBID_ACTION_GET:
            copy_from_user(&buf.l2_common, user, sizeof(rtdrv_l2_common_t));
            ret = rtk_l2_dynamicPortMoveForbidAction_get(buf.l2_common.unit, &buf.l2_common.value);
            copy_to_user(user, &buf.l2_common, sizeof(rtdrv_l2_common_t));
            break;

        case RTDRV_L2_PORT_MAC_FILTER_ENABLE_GET:
            copy_from_user(&buf.l2_macFilterCfg, user, sizeof(rtdrv_l2_mac_filter_t));
            ret = rtk_l2_portMacFilterEnable_get(buf.l2_macFilterCfg.unit, buf.l2_macFilterCfg.port, buf.l2_macFilterCfg.filterMode, &buf.l2_macFilterCfg.enable);
            copy_to_user(user, &buf.l2_macFilterCfg, sizeof(rtdrv_l2_mac_filter_t));
            break;

        case RTDRV_L2_HW_NEXT_VALID_ADDR_GET:
            copy_from_user(&buf.l2_entryGetCfg, user, sizeof(rtdrv_l2_entry_get_t));
            ret = rtk_l2_hwNextValidAddr_get(buf.l2_entryGetCfg.unit, &buf.l2_entryGetCfg.scan_idx, buf.l2_entryGetCfg.type, &buf.l2_entryGetCfg.entry);
            copy_to_user(user, &buf.l2_entryGetCfg, sizeof(rtdrv_l2_entry_get_t));
            break;

    /*L3*/
        case RTDRV_L3_TTLEXPIREACTION_GET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireAction_get(buf.l3_config.unit, buf.l3_config.type, &buf.l3_config.action);
            copy_to_user(user, &buf.l3_config, sizeof(rtdrv_l3_config_t));
            break;

        case RTDRV_L3_TTLEXPIRETRAPPRI_GET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapPri_get(buf.l3_config.unit, buf.l3_config.type, &buf.l3_config.priority);
            copy_to_user(user, &buf.l3_config, sizeof(rtdrv_l3_config_t));
            break;

        case RTDRV_L3_TTLEXPIRETRAPPRIENABLE_GET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapPriEnable_get(buf.l3_config.unit, buf.l3_config.type, &buf.l3_config.dfPri);
            copy_to_user(user, &buf.l3_config, sizeof(rtdrv_l3_config_t));
            break;

        case RTDRV_L3_TTLEXPIRETRAPDP_GET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapDP_get(buf.l3_config.unit, buf.l3_config.type, &buf.l3_config.dpValue);
            copy_to_user(user, &buf.l3_config, sizeof(rtdrv_l3_config_t));
            break;

        case RTDRV_L3_TTLEXPIRETRAPDPENABLE_GET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireTrapDPEnable_get(buf.l3_config.unit, buf.l3_config.type, &buf.l3_config.dfDp);
            copy_to_user(user, &buf.l3_config, sizeof(rtdrv_l3_config_t));
            break;

        case RTDRV_L3_TTLEXPIREADDCPUTAGENABLE_GET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_ttlExpireAddCPUTagEnable_get(buf.l3_config.unit, buf.l3_config.type, &buf.l3_config.insertCpuTag);
            copy_to_user(user, &buf.l3_config, sizeof(rtdrv_l3_config_t));
            break;

        case RTDRV_L3_ROUTE_ROUTEENTRY_GET:
            copy_from_user(&buf.l3_route_entry, user, sizeof(rtdrv_l3_routeEntry_t));
            ret = rtk_l3_routeEntry_get(buf.l3_route_entry.unit, buf.l3_route_entry.index, &buf.l3_route_entry.entry);
            copy_to_user(user, &buf.l3_route_entry, sizeof(rtdrv_l3_routeEntry_t));
            break;

        case RTDRV_L3_ROUTE_SWITCHMACADDR_GET:
            copy_from_user(&buf.l3_config, user, sizeof(rtdrv_l3_config_t));
            ret = rtk_l3_routeSwitchMacAddr_get(buf.l3_config.unit, buf.l3_config.index, &buf.l3_config.mac);
            copy_to_user(user, &buf.l3_config, sizeof(rtdrv_l3_config_t));
            break;

    /** PORT **/
        case RTDRV_PORT_LINK_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_link_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_SPEED_DUPLEX_GET:
            copy_from_user(&buf.speed_duplex, user, sizeof(rtdrv_port_speedDuplex_t));
            ret = rtk_port_speedDuplex_get(buf.speed_duplex.unit, buf.speed_duplex.port, &buf.speed_duplex.speed,
                                           &buf.speed_duplex.duplex);
            copy_to_user(user, &buf.speed_duplex, sizeof(rtdrv_port_speedDuplex_t));
            break;

        case RTDRV_PORT_FLOW_CTRL_GET:
            copy_from_user(&buf.port_flowctrl, user, sizeof(rtdrv_port_flowctrl_t));
            ret = rtk_port_flowctrl_get(buf.port_flowctrl.unit, buf.port_flowctrl.port, &buf.port_flowctrl.tx_status,
                                        &buf.port_flowctrl.rx_status);
            copy_to_user(user, &buf.port_flowctrl, sizeof(rtdrv_port_flowctrl_t));
            break;

        case RTDRV_PORT_EN_AUTONEGO_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyAutoNegoEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_AUTONEGO_ABIL_GET:
            copy_from_user(&buf.autonego_ability, user, sizeof(rtdrv_port_autoNegoAbility_t));
            ret = rtk_port_phyAutoNegoAbility_get(buf.autonego_ability.unit, buf.autonego_ability.port,
                                                  &buf.autonego_ability.ability);
            copy_to_user(user, &buf.autonego_ability, sizeof(rtdrv_port_autoNegoAbility_t));
            break;

        case RTDRV_PORT_FORCE_MODE_ABIL_GET:
            copy_from_user(&buf.forcemode_ability, user, sizeof(rtdrv_port_forceModeAbility_t));
            ret = rtk_port_phyForceModeAbility_get(buf.forcemode_ability.unit, buf.forcemode_ability.port,
                                                   &buf.forcemode_ability.speed, &buf.forcemode_ability.duplex,
                                                   &buf.forcemode_ability.flowctrl);
            copy_to_user(user, &buf.forcemode_ability, sizeof(rtdrv_port_forceModeAbility_t));
            break;

        case RTDRV_PORT_CPU_PORT_ID_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_port_cpuPortId_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_PORT_PHY_REG_GET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyReg_get(buf.phy_data.unit, buf.phy_data.port, buf.phy_data.page, buf.phy_data.reg,
                                      &buf.phy_data.data);
            copy_to_user(user, &buf.phy_data, sizeof(rtdrv_port_phyReg_t));
            break;

        case RTDRV_PORT_PHY_EXT_PARK_PAGE_REG_GET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyExtParkPageReg_get(buf.phy_data.unit, buf.phy_data.port, buf.phy_data.page, buf.phy_data.extPage,
                buf.phy_data.parkPage, buf.phy_data.reg, &buf.phy_data.data);
            copy_to_user(user, &buf.phy_data, sizeof(rtdrv_port_phyReg_t));
            break;

        case RTDRV_PORT_PHY_MMD_REG_GET:
            copy_from_user(&buf.phy_data, user, sizeof(rtdrv_port_phyReg_t));
            ret = rtk_port_phyMmdReg_get(buf.phy_data.unit, buf.phy_data.port, buf.phy_data.mmdAddr, buf.phy_data.reg, &buf.phy_data.data);
            copy_to_user(user, &buf.phy_data, sizeof(rtdrv_port_phyReg_t));
            break;

        case RTDRV_PORT_MASTER_SLAVE_GET:
            copy_from_user(&buf.masterSlave_cfg, user, sizeof(rtdrv_port_masterSlave_t));
            ret = rtk_port_phyMasterSlave_get(buf.masterSlave_cfg.unit, buf.masterSlave_cfg.port, &buf.masterSlave_cfg.masterSlaveCfg, &buf.masterSlave_cfg.masterSlaveActual);
            copy_to_user(user, &buf.masterSlave_cfg, sizeof(rtdrv_port_masterSlave_t));
            break;

        case RTDRV_PORT_ISOLATION_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_isolation_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.portmask);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_EN_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_adminEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_BACK_PRESSURE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_backpressureEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_PHY_MEDIA_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyComboPortMedia_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.media);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_GREEN_ENABLE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_greenEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

	case RTDRV_PORT_GIGA_LITE_ENABLE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_gigaLiteEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_UDLDENABLE_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldEnable_get(buf.udld_cfg.unit, buf.udld_cfg.port, &buf.udld_cfg.enable);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDLINKUPAUTOTRIGGERENABLE_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldLinkUpAutoTriggerEnable_get(buf.udld_cfg.unit, &buf.udld_cfg.autoTriggerEnable);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDSTATUS_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldStatus_get(buf.udld_cfg.unit, buf.udld_cfg.port, &buf.udld_cfg.status);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDECHOACTION_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldEchoAction_get(buf.udld_cfg.unit, &buf.udld_cfg.action);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDLINKSTATUS_GET :
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldLinkStatus_get(buf.udld_cfg.unit, buf.udld_cfg.port, &buf.udld_cfg.linkStatus);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDAUTODISABLEFAILEDPORTENABLE_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldAutoDisableFailedPortEnable_get(buf.udld_cfg.unit, &buf.udld_cfg.enable);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDINTERVAL_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldInterval_get(buf.udld_cfg.unit, &buf.udld_cfg.interval);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDRETRYCOUNT_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldRetryCount_get(buf.udld_cfg.unit, &buf.udld_cfg.retryCount);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_UDLDLEDINDICATEENABLE_GET:
            copy_from_user(&buf.udld_cfg, user, sizeof(rtdrv_portUdldCfg_t));
            ret = rtk_port_udldLedIndicateEnable_get(buf.udld_cfg.unit, &buf.udld_cfg.enable);
            copy_to_user(user, &buf.udld_cfg, sizeof(rtdrv_portUdldCfg_t));
            break;

        case RTDRV_PORT_RLDPENABLE_GET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpEnable_get(buf.rldp_cfg.unit, buf.rldp_cfg.port, &buf.rldp_cfg.enable);
            copy_to_user(user, &buf.rldp_cfg, sizeof(rtdrv_portRldpCfg_t));
            break;

        case RTDRV_PORT_RLDPSTATUS_GET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpStatus_get(buf.rldp_cfg.unit, buf.rldp_cfg.port, &buf.rldp_cfg.normalStatus, &buf.rldp_cfg.selfStatus);
            copy_to_user(user, &buf.rldp_cfg, sizeof(rtdrv_portRldpCfg_t));
            break;

        case RTDRV_PORT_RLDPAUTOBLOCKENABLE_GET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpAutoBlockEnable_get(buf.rldp_cfg.unit, buf.rldp_cfg.port, &buf.rldp_cfg.enable);
            copy_to_user(user, &buf.rldp_cfg, sizeof(rtdrv_portRldpCfg_t));
            break;

        case RTDRV_PORT_RLDPINTERVAL_GET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpInterval_get(buf.rldp_cfg.unit, buf.rldp_cfg.port, &buf.rldp_cfg.interval);
            copy_to_user(user, &buf.rldp_cfg, sizeof(rtdrv_portRldpCfg_t));
            break;

        case RTDRV_PORT_RLDPSELFLOOPAGINGTIME_GET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpSelfLoopAgingTime_get(buf.rldp_cfg.unit, buf.rldp_cfg.port, &buf.rldp_cfg.agingTime);
            copy_to_user(user, &buf.rldp_cfg, sizeof(rtdrv_portRldpCfg_t));
            break;

        case RTDRV_PORT_RLDPNORMALLOOPAGINGTIME_GET:
            copy_from_user(&buf.rldp_cfg, user, sizeof(rtdrv_portRldpCfg_t));
            ret = rtk_port_rldpNormalLoopAgingTime_get(buf.rldp_cfg.unit, buf.rldp_cfg.port, &buf.rldp_cfg.agingTime);
            copy_to_user(user, &buf.rldp_cfg, sizeof(rtdrv_portRldpCfg_t));
            break;

        case RTDRV_PORT_PHY_CROSSOVERMODE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyCrossOverMode_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_TX_EN_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_txEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_RX_EN_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_rxEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_PHY_FIBER_MEDIA_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyComboPortFiberMedia_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.fiber_media);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_LINKMEDIA_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_linkMedia_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data, &buf.port_cfg.media);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_LINKDOWN_POWERSAVING_ENABLE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_linkDownPowerSavingEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_VLAN_ISOLATION_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_vlanBasedIsolationEntry_get(buf.port_cfg.unit, buf.port_cfg.index, &buf.port_cfg.vlanIsoEntry);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_VLAN_ISOLATION_VLANSOURCE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_vlanBasedIsolation_vlanSource_get(buf.port_cfg.unit, &buf.port_cfg.vlanIsoSrc);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;
        case RTDRV_PORT_DOWNSPEEDENABLE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_downSpeedEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;
        case RTDRV_PORT_FIBERDOWNSPEEDENABLE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_fiberDownSpeedEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;
        case RTDRV_PORT_FIBERNWAYFORCELINKENABLE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_fiberNwayForceLinkEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;
        case RTDRV_PORT_10GMEDIA_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_10gMedia_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.media_10g);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_PORT_PHY_CROSSOVERSTATUS_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_port_phyCrossOverStatus_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
        break;

     /*OAM*/
        case RTDRV_OAM_OAMCOUNTER_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_oam_oamCounter_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_OAM_TXSTATUSOFTESTFRAME_GET:
            copy_from_user(&buf.spd_cfg, user, sizeof(rtdrv_oamSpgCfg_t));
            ret = rtk_oam_txStatusOfTestFrame_get(buf.spd_cfg.unit, &buf.spd_cfg.txStatus, &buf.spd_cfg.txCount);
            copy_to_user(user, &buf.spd_cfg, sizeof(rtdrv_oamSpgCfg_t));
            break;

        case RTDRV_OAM_LOOPBACKMODE_GET:
            copy_from_user(&buf.loopback_cfg, user, sizeof(rtdrv_oamLoopbackCfg_t));
            ret = rtk_oam_loopbackMode_get(buf.loopback_cfg.unit, buf.loopback_cfg.port, &buf.loopback_cfg.lpbackMode);
            copy_to_user(user, &buf.loopback_cfg, sizeof(rtdrv_oamLoopbackCfg_t));
            break;

        case RTDRV_OAM_LOOPBACKCTRL_GET:
            copy_from_user(&buf.loopback_cfg, user, sizeof(rtdrv_oamLoopbackCfg_t));
            ret = rtk_oam_loopbackCtrl_get(buf.loopback_cfg.unit, buf.loopback_cfg.port, &buf.loopback_cfg.lpbackCtrl);
            copy_to_user(user, &buf.loopback_cfg, sizeof(rtdrv_oamLoopbackCfg_t));
            break;

        case RTDRV_OAM_AUTODYINGGASPENABLE_GET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_autoDyingGaspEnable_get(buf.dyingGasp_cfg.unit, buf.dyingGasp_cfg.port, &buf.dyingGasp_cfg.enable);
            copy_to_user(user, &buf.dyingGasp_cfg, sizeof(rtdrv_oamDyingGaspCfg_t));
            break;

        case RTDRV_OAM_DYINGGASPTLV_GET :
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_dyingGaspTLV_get(buf.dyingGasp_cfg.unit, buf.dyingGasp_cfg.port, &buf.dyingGasp_cfg.tlv);
            copy_to_user(user, &buf.dyingGasp_cfg, sizeof(rtdrv_oamDyingGaspCfg_t));
            break;

        case RTDRV_OAM_DYINGGASPWAITTIME_GET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_dyingGaspWaitTime_get(buf.dyingGasp_cfg.unit, &buf.dyingGasp_cfg.waitTime);
            copy_to_user(user, &buf.dyingGasp_cfg, sizeof(rtdrv_oamDyingGaspCfg_t));
            break;

        case RTDRV_OAM_DYINGGASPPKTCNT_GET:
            copy_from_user(&buf.dyingGasp_cfg, user, sizeof(rtdrv_oamDyingGaspCfg_t));
            ret = rtk_oam_dyingGaspPktCnt_get(buf.dyingGasp_cfg.unit, &buf.dyingGasp_cfg.cnt);
            copy_to_user(user, &buf.dyingGasp_cfg, sizeof(rtdrv_oamDyingGaspCfg_t));
            break;

        case RTDRV_OAM_CFMENTRY_GET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmEntry_get(buf.cfm_cfg.unit, buf.cfm_cfg.cfmIdx, &buf.cfm_cfg.cfmCfg);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

        case RTDRV_OAM_CFMPORTENTRY_GET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmPortEntry_get(buf.cfm_cfg.unit, buf.cfm_cfg.port, &buf.cfm_cfg.portCfg);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

        case RTDRV_OAM_CFMMEPENABLE_GET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmMepEnable_get(buf.cfm_cfg.unit, buf.cfm_cfg.port, &buf.cfm_cfg.enable);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMFRAME_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMFrame_get(buf.ccm_cfg.unit, buf.ccm_cfg.cfmIdx, &buf.ccm_cfg.ccmFrame);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMSNAPOUI_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMSnapOui_get(buf.ccm_cfg.unit, &buf.ccm_cfg.snapoui);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMETYPE_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMEtype_get(buf.ccm_cfg.unit, &buf.ccm_cfg.etherType);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMOPCODE_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMOpcode_get(buf.ccm_cfg.unit, &buf.ccm_cfg.opCode);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMFLAG_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMFlag_get(buf.ccm_cfg.unit, buf.ccm_cfg.cfmIdx, &buf.ccm_cfg.ccmFlag);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMINTERVAL_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCCMInterval_get(buf.ccm_cfg.unit, &buf.ccm_cfg.ccmInterval);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMINTFSTATUS_GET :
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmIntfStatus_get(buf.misc_cfg.unit, buf.misc_cfg.port, &buf.misc_cfg.status);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_OAM_CFMPORTSTATUS_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmPortStatus_get(buf.misc_cfg.unit, buf.misc_cfg.port, &buf.misc_cfg.status);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_OAM_CFMCCSTATUS_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmCCStatus_get(buf.misc_cfg.unit, buf.misc_cfg.port, buf.misc_cfg.mepid, &buf.misc_cfg.rdi, &buf.misc_cfg.ccStatus);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_OAM_CFMLOOPBACKREPLYENABLE_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmLoopbackReplyEnable_get(buf.misc_cfg.unit, buf.misc_cfg.port, &buf.misc_cfg.loopbackEnable);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_OAM_CFMLOOPBACKREPLYCTRL_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmLoopbackReplyCtrl_get(buf.misc_cfg.unit, &buf.misc_cfg.ctrl);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_OAM_LOOPBACKMACSWAPENABLE_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_loopbackMacSwapEnable_get(buf.misc_cfg.unit,
                                                    &buf.misc_cfg.loopbackEnable);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;


        case RTDRV_OAM_PORTLOOPBACKMUXACTION_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_portLoopbackMuxAction_get(buf.misc_cfg.unit,
                    buf.misc_cfg.port, &buf.misc_cfg.action);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_OAM_CFMCCMPCP_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmPcp_get(buf.ccm_cfg.unit,
                                        &buf.ccm_cfg.ccmFrame.outer_pri);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMCFI_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmCfi_get(buf.ccm_cfg.unit,
                                        &buf.ccm_cfg.ccmFrame.outer_dei);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMTPID_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmTpid_get(buf.ccm_cfg.unit,
                                         &buf.ccm_cfg.ccmFrame.outer_tpid);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMRESETLIFETIME_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmInstLifetime_get(buf.ccm_cfg.unit,
                                                  buf.ccm_cfg.cfmIdx,
                                                  &buf.ccm_cfg.ccmFlag);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMMEPID_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_oam_cfmCcmMepid_get(buf.misc_cfg.unit,
                                          &buf.misc_cfg.mepid);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_OAM_CFMCCMINTERVALFIELD_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmIntervalField_get(buf.ccm_cfg.unit,
                                                  &buf.ccm_cfg.ccmFlag);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMMDL_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmMdl_get(buf.cfm_cfg.unit,
                                        &buf.cfm_cfg.cfmCfg.md_level);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMINSTTAGSTATUS_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmInstTagStatus_get(buf.cfm_cfg.unit,
                                                  buf.cfm_cfg.cfmIdx,
                                                  &buf.cfm_cfg.enable);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMINSTVID_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmInstVid_get(buf.ccm_cfg.unit,
                                            buf.ccm_cfg.cfmIdx,
                                            &buf.ccm_cfg.ccmFrame.outer_vid);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_OAM_CFMCCMINSTMAID_GET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmInstMaid_get(buf.cfm_cfg.unit,
                                             buf.cfm_cfg.cfmIdx,
                                             &buf.cfm_cfg.maid);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;
#endif

        case RTDRV_OAM_CFMCCMINSTTXSTATUS_GET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmCcmInstTxStatus_get(buf.cfm_cfg.unit,
                                                 buf.cfm_cfg.cfmIdx,
                                                 &buf.cfm_cfg.enable);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMINSTINTERVAL_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmInstInterval_get(buf.ccm_cfg.unit,
                                                 buf.ccm_cfg.cfmIdx,
                                                 &buf.ccm_cfg.ccmInterval);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_OAM_CFMCCMTXINSTPORT_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmTxInstPort_get(buf.ccm_cfg.unit,
                                               buf.ccm_cfg.cfmIdx,
                                               buf.ccm_cfg.portIdx,
                                               &buf.ccm_cfg.port);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;
#endif

        case RTDRV_OAM_CFMCCMRXINSTVID_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmRxInstVid_get(buf.ccm_cfg.unit,
                                              buf.ccm_cfg.cfmIdx,
                                              &buf.ccm_cfg.ccmFrame.outer_vid);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

#if defined(CONFIG_SDK_RTL8390)
        case RTDRV_OAM_CFMCCMRXINSTPORT_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmRxInstPort_get(buf.ccm_cfg.unit,
                                               buf.ccm_cfg.cfmIdx,
                                               buf.ccm_cfg.portIdx,
                                               &buf.ccm_cfg.port);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;

        case RTDRV_OAM_CFMCCMKEEPALIVE_GET:
            copy_from_user(&buf.ccm_cfg, user, sizeof(rtdrv_oamCcmCfg_t));
            ret = rtk_oam_cfmCcmInstAliveTime_get(buf.ccm_cfg.unit,
                                              buf.ccm_cfg.cfmIdx,
                                              buf.ccm_cfg.portIdx,
                                              &buf.ccm_cfg.ccmInterval);
            copy_to_user(user, &buf.ccm_cfg, sizeof(rtdrv_oamCcmCfg_t));
            break;
#endif

        case RTDRV_OAM_CFMETHDMPORTENABLE_GET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmPortEthDmEnable_get(buf.cfm_cfg.unit,
                                              buf.cfm_cfg.port,
                                              &buf.cfm_cfg.enable);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

        case RTDRV_OAM_CFMETHDMRXTIMESTAMP_GET:
            copy_from_user(&buf.cfm_cfg, user, sizeof(rtdrv_oamCfmCfg_t));
            ret = rtk_oam_cfmEthDmRxTimestamp_get(buf.cfm_cfg.unit,
                                              buf.cfm_cfg.index,
                                              &buf.cfm_cfg.timeStamp);
            copy_to_user(user, &buf.cfm_cfg, sizeof(rtdrv_oamCfmCfg_t));
            break;

    /** VLAN **/
        case RTDRV_VLAN_PORT_GET:
            copy_from_user(&buf.vlan_port_data, user, sizeof(rtdrv_vlan_port_t));
            ret = rtk_vlan_port_get(buf.vlan_port_data.unit, buf.vlan_port_data.vid, &buf.vlan_port_data.member,
                                    &buf.vlan_port_data.untag);
            copy_to_user(user, &buf.vlan_port_data, sizeof(rtdrv_vlan_port_t));
            break;

        case RTDRV_VLAN_PORT_PVID_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portPvid_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_VLAN_PORT_OUTER_PVID_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portOuterPvid_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_VLAN_PROTO_GROUP_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_protoGroup_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.protoGroup);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_PROTO_VLAN_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portProtoVlan_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.idx, &buf.vlan_cfg.protoVlanCfg);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_OUTER_PROTO_VLAN_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portOuterProtoVlan_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.idx, &buf.vlan_cfg.protoVlanCfg);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_TPID_ENTRY_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portTpidEntry_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, buf.vlan_cfg.idx, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_TPID_MODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTpidMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_INNER_TPID_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrInnerTpid_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_TPID_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTpid_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_TPID_MODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTpidMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_OUTER_TPID_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrOuterTpid_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_TPID_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTpid_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_EXTRA_TPID_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrExtraTpid_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_IGNORE_INNER_TAG_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrIgnoreInnerTagEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_IGNORE_OUTER_TAG_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrIgnoreOuterTagEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_TAG_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTagEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_TAG_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTagEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_EXTRA_TAG_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrExtraTagEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_EXTRA_TAG_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrExtraTagEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_VID_SOURCE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerVidSource_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_INNER_PRI_SOURCE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerPriSource_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_VID_SOURCE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterVidSource_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_OUTER_PRI_SOURCE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterPriSource_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_TAG_KEEP_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrTagKeepEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data, &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_TAG_KEEP_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrTagKeepEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data, &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_FWD_MODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_fwdMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.vid, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EN_PORT_IGR_FILTER_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portIgrFilterEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_VLAN_TAG_MODE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_tagMode_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_VLAN_PORT_ACCEPT_FRAME_TYPE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portAcceptFrameType_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_VLAN_PORT_OUTER_ACCEPT_FRAME_TYPE_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_portOuterAcceptFrameType_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_VLAN_EN_MCAST_LEAKY_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = rtk_vlan_mcastLeakyEnable_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_VLAN_EN_PORT_MCAST_LEAKY_GET:
            copy_from_user(&buf.port_cfg, user, sizeof(rtdrv_portCfg_t));
            ret = rtk_vlan_mcastLeakyPortEnable_get(buf.port_cfg.unit, buf.port_cfg.port, &buf.port_cfg.data);
            copy_to_user(user, &buf.port_cfg, sizeof(rtdrv_portCfg_t));
            break;

        case RTDRV_VLAN_STG_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_stg_get(buf.vlan_cfg.unit, buf.vlan_cfg.vid, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_FID_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_fid_get(buf.vlan_cfg.unit, buf.vlan_cfg.vid, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EN_IGR_FILTER_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_igrFilterEnable_get(buf.vlan_cfg.unit, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGR_FILTER_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrFilterEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_vlanFunctionEnable_get(buf.vlan_cfg.unit, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_UCAST_LUTMODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_l2UcastLookupMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.vid, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_MCAST_LUTMODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_l2McastLookupMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.vid, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PROFILE_IDX_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_profileIdx_get(buf.vlan_cfg.unit, buf.vlan_cfg.vid, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PROFILE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_profile_get(buf.vlan_cfg.unit, buf.vlan_cfg.data, &buf.vlan_cfg.profile);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGR_FILTER_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrFilter_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_IGRTAGKEEPTYPE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrTagKeepType_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data, &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGRTAGKEEPTYPE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrTagKeepType_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data, &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_PVID_MODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portPvidMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_OPVID_MODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portOuterPvidMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_MAC_BASED_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_macBasedVlan_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    &buf.vlan_cfg.data, &buf.vlan_cfg.mac, &buf.vlan_cfg.vid,
                    &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_MAC_BASED_WITH_MSK_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_macBasedVlanWithMsk_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    &buf.vlan_cfg.data, &buf.vlan_cfg.mac, &buf.vlan_cfg.msk, &buf.vlan_cfg.vid,
                    &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_MAC_BASED_WITH_PORT_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_macBasedVlanWithPort_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    &buf.vlan_cfg.data, &buf.vlan_cfg.mac, &buf.vlan_cfg.msk,
                    &buf.vlan_cfg.port, &buf.vlan_cfg.port_msk, &buf.vlan_cfg.vid, &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_IP_SUBNET_BASED_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_ipSubnetBasedVlan_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    &buf.vlan_cfg.data, &buf.vlan_cfg.sip, &buf.vlan_cfg.sip_msk, &buf.vlan_cfg.vid,
                    &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_IP_SUBNET_BASED_WITH_PORT_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_ipSubnetBasedVlanWithPort_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx,
                    &buf.vlan_cfg.data, &buf.vlan_cfg.sip, &buf.vlan_cfg.sip_msk,
                    &buf.vlan_cfg.port, &buf.vlan_cfg.port_msk, &buf.vlan_cfg.vid, &buf.vlan_cfg.data1);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_ITPID_ENTRY_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_innerTpidEntry_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_OTPID_ENTRY_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_outerTpidEntry_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_ETPID_ENTRY_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_extraTpidEntry_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EGR_ITAG_STS_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrInnerTagSts_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EGR_OTAG_STS_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrOuterTagSts_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_IGRVLANCNVT_BLKMODE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_igrVlanCnvtBlkMode_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_IGRVLANCNVT_ENTRY_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_igrVlanCnvtEntry_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.igrCnvtEntry);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EGRVLANCNVT_DBLTAG_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtDblTagEnable_get(buf.vlan_cfg.unit, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EGRVLANCNVT_VIDSRC_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtVidSource_get(buf.vlan_cfg.unit, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EGRVLANCNVT_ENTRY_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtEntry_get(buf.vlan_cfg.unit, buf.vlan_cfg.idx, &buf.vlan_cfg.egrCnvtEntry);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_VLANAGGR_ENABLE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portVlanAggrEnable_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_LEAKYSTPFILTER_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_leakyStpFilter_get(buf.vlan_cfg.unit, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_EXCEPT_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_except_get(buf.vlan_cfg.unit, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORTIGRCNVTDFLTACT_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portIgrCnvtDfltAct_get(buf.vlan_cfg.unit,
                    buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;
        case RTDRV_VLAN_EGRVLANCNVTRANGECHECKVID_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_egrVlanCnvtRangeCheckVid_get(buf.vlan_cfg.unit,
                    buf.vlan_cfg.idx, &buf.vlan_cfg.egrRangeCheck);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_VLANAGGRVIDSOURCE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portVlanAggrVidSource_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_VLANAGGRPRITAGVIDSOURCE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portVlanAggrPriTagVidSource_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGRVLANCNVTVIDSOURCE_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrVlanCnvtVidSource_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGRVLANCNVTVIDTARGET_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrVlanCnvtVidTarget_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

        case RTDRV_VLAN_PORT_EGRVLANCNVTLOOKUPMISSACT_GET:
            copy_from_user(&buf.vlan_cfg, user, sizeof(rtdrv_vlanCfg_t));
            ret = rtk_vlan_portEgrVlanCnvtLookupMissAct_get(buf.vlan_cfg.unit, buf.vlan_cfg.port, &buf.vlan_cfg.data);
            copy_to_user(user, &buf.vlan_cfg, sizeof(rtdrv_vlanCfg_t));
            break;

    /** STP **/
        case RTDRV_STP_MSTP_STATE_GET:
            copy_from_user(&buf.stp_cfg, user, sizeof(rtdrv_stpCfg_t));
            ret = rtk_stp_mstpState_get(buf.stp_cfg.unit, buf.stp_cfg.msti, buf.stp_cfg.port, &buf.stp_cfg.stp_state);
            copy_to_user(user, &buf.stp_cfg, sizeof(rtdrv_stpCfg_t));
            break;

        case RTDRV_STP_MSTP_MODE_GET:
            copy_from_user(&buf.stp_cfg, user, sizeof(rtdrv_stpCfg_t));
            ret = rtk_stp_mstpInstanceMode_get(buf.stp_cfg.unit, &buf.stp_cfg.msti_mode);
            copy_to_user(user, &buf.stp_cfg, sizeof(rtdrv_stpCfg_t));
            break;

    /** REG **/
        case RTDRV_REG_REGISTER_GET:
            copy_from_user(&buf.reg_cfg, user, sizeof(rtdrv_regCfg_t));
            ioal_mem32_read(buf.reg_cfg.unit, buf.reg_cfg.reg, &buf.reg_cfg.value);
            ret = RT_ERR_OK; /*xxx_reg_register_get(buf.reg_cfg.unit, buf.reg_cfg.reg, &buf.reg_cfg.value);*/
            copy_to_user(user, &buf.reg_cfg, sizeof(rtdrv_regCfg_t));
            break;

        case RTDRV_REG_IDX2ADDR_GET:
            copy_from_user(&buf.reg_cfg, user, sizeof(rtdrv_regCfg_t));
            ret = reg_idx2Addr_get(buf.reg_cfg.unit, (uint32)buf.reg_cfg.reg, &buf.reg_cfg.value);
            copy_to_user(user, &buf.reg_cfg, sizeof(rtdrv_regCfg_t));
            break;

        case RTDRV_REG_IDXMAX_GET:
            copy_from_user(&buf.reg_cfg, user, sizeof(rtdrv_regCfg_t));
            ret = reg_idxMax_get(buf.reg_cfg.unit, &buf.reg_cfg.value);
            copy_to_user(user, &buf.reg_cfg, sizeof(rtdrv_regCfg_t));
            break;

        case RTDRV_REG_INFO_GET:
            copy_from_user(&buf.reg_cfg, user, sizeof(rtdrv_regCfg_t));
            ret = reg_info_get(buf.reg_cfg.unit, (uint32)buf.reg_cfg.reg, &buf.reg_cfg.data);
            copy_to_user(user, &buf.reg_cfg, sizeof(rtdrv_regCfg_t));
            break;

        case RTDRV_TABLE_READ:
            copy_from_user(&buf.tbl_cfg, user, sizeof(rtdrv_tblCfg_t));
            ret = table_read(buf.tbl_cfg.unit, buf.tbl_cfg.table, buf.tbl_cfg.addr, buf.tbl_cfg.value);
            copy_to_user(user, &buf.reg_cfg, sizeof(rtdrv_tblCfg_t));
            break;

    /** COUNTER **/
        case RTDRV_COUNTER_GLOBAL_GET:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_global_get(buf.counter_cfg.unit, buf.counter_cfg.cntr_idx, &(buf.counter_cfg.cntr));
            copy_to_user(user, &buf.counter_cfg, sizeof(rtdrv_counterCfg_t));
            break;

        case RTDRV_COUNTER_GLOBAL_GETALL:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_global_getAll(buf.counter_cfg.unit, &buf.counter_cfg.global_cnt);
            copy_to_user(user, &buf.counter_cfg, sizeof(rtdrv_counterCfg_t));
            break;

        case RTDRV_COUNTER_PORT_GET:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_port_get(buf.counter_cfg.unit, buf.counter_cfg.port, buf.counter_cfg.cntr_idx, &(buf.counter_cfg.cntr));
            copy_to_user(user, &buf.counter_cfg, sizeof(rtdrv_counterCfg_t));
            break;

        case RTDRV_COUNTER_PORT_GETALL:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_port_getAll(buf.counter_cfg.unit, buf.counter_cfg.port, &buf.counter_cfg.port_cnt);
            copy_to_user(user, &buf.counter_cfg, sizeof(rtdrv_counterCfg_t));
            break;

        case RTDRV_COUNTER_SMON_GET:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_smon_get(buf.counter_cfg.unit, buf.counter_cfg.pri, buf.counter_cfg.cntr_idx, &(buf.counter_cfg.cntr));
            copy_to_user(user, &buf.counter_cfg, sizeof(rtdrv_counterCfg_t));
            break;

        case RTDRV_COUNTER_SMON_GETALL:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_smon_getAll(buf.counter_cfg.unit, buf.counter_cfg.pri, &buf.counter_cfg.smon_cnt);
            copy_to_user(user, &buf.counter_cfg, sizeof(rtdrv_counterCfg_t));
            break;

        case RTDRV_COUNTER_TAGLENCNT_GET:
            copy_from_user(&buf.counter_cfg, user, sizeof(rtdrv_counterCfg_t));
            ret = rtk_stat_tagLenCntIncEnable_get(buf.counter_cfg.unit, buf.counter_cfg.tagCnt_type, &buf.counter_cfg.enable);
            copy_to_user(user, &buf.counter_cfg, sizeof(rtdrv_counterCfg_t));
            break;

    /** TIME **/
        case RTDRV_TIME_PORT_PTP_ENABLE_GET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_portPtpEnable_get(buf.time_cfg.unit, buf.time_cfg.port, &buf.time_cfg.enable);
            copy_to_user(user, &buf.time_cfg, sizeof(rtdrv_timeCfg_t));
            break;

        case RTDRV_TIME_PORT_PTP_RX_TIME_GET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_portPtpRxTimestamp_get(buf.time_cfg.unit, buf.time_cfg.port, buf.time_cfg.identifier, &buf.time_cfg.timeStamp);
            copy_to_user(user, &buf.time_cfg, sizeof(rtdrv_timeCfg_t));
            break;

        case RTDRV_TIME_PORT_PTP_TX_TIME_GET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_portPtpTxTimestamp_get(buf.time_cfg.unit, buf.time_cfg.port, buf.time_cfg.identifier, &buf.time_cfg.timeStamp);
            copy_to_user(user, &buf.time_cfg, sizeof(rtdrv_timeCfg_t));
            break;

        case RTDRV_TIME_REF_TIME_GET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_refTime_get(buf.time_cfg.unit, &buf.time_cfg.timeStamp);
            copy_to_user(user, &buf.time_cfg, sizeof(rtdrv_timeCfg_t));
            break;

        case RTDRV_TIME_REF_TIME_ENABLE_GET:
            copy_from_user(&buf.time_cfg, user, sizeof(rtdrv_timeCfg_t));
            ret = rtk_time_refTimeEnable_get(buf.time_cfg.unit, &buf.time_cfg.enable);
            copy_to_user(user, &buf.time_cfg, sizeof(rtdrv_timeCfg_t));
            break;

    /** TRAP **/
        case RTDRV_TRAP_RMAACTION_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaAction_get(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, &buf.trap_cfg.rma_action);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_RMAPRI_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaPri_get(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, &buf.trap_cfg.priority);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_RMAPRIENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaPriEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_RMACPUTAGADDENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaCpuTagAddEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_RMAVLANCHECKENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaVlanCheckEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.rma_frame, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_BYPASS_STP_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_bypassStp_get(buf.trap_cfg.unit, buf.trap_cfg.bypassStp_frame, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_BYPASS_VLAN_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_bypassVlan_get(buf.trap_cfg.unit, buf.trap_cfg.bypassVlan_frame, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMA_GET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRma_get(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.rma_frame);
            copy_to_user(user, &buf.l2_trap_cfg, sizeof(rtdrv_trapL2userRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMAENABLE_GET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaEnable_get(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.enable);
            copy_to_user(user, &buf.l2_trap_cfg, sizeof(rtdrv_trapL2userRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMAACTION_GET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaAction_get(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.rma_action);
            copy_to_user(user, &buf.l2_trap_cfg, sizeof(rtdrv_trapL2userRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMAPRI_GET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaPri_get(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.priority);
            copy_to_user(user, &buf.l2_trap_cfg, sizeof(rtdrv_trapL2userRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMAPRIENABLE_GET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaPriEnable_get(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.enable);
            copy_to_user(user, &buf.l2_trap_cfg, sizeof(rtdrv_trapL2userRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMAVLANCHECKENABLE_GET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaVlanCheckEnable_get(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.vlanCheck);
            copy_to_user(user, &buf.l2_trap_cfg, sizeof(rtdrv_trapL2userRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMASTPBLOCKENABLE_GET:
            copy_from_user(&buf.l2_trap_cfg, user, sizeof(rtdrv_trapL2userRmaCfg_t));
            ret = rtk_trap_userDefineRmaStpBlockEnable_get(buf.l2_trap_cfg.unit, buf.l2_trap_cfg.rma_index, &buf.l2_trap_cfg.stpBlock);
            copy_to_user(user, &buf.l2_trap_cfg, sizeof(rtdrv_trapL2userRmaCfg_t));
            break;

        case RTDRV_TRAP_MGMTFRAMEACTION_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFrameAction_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, (rtk_action_t*)&buf.mgm_trap_cfg.rma_action);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_MGMTFRAMEPRI_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFramePri_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.priority);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_MGMTFRAMEPRIENABLE_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFramePriEnable_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.enable);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_MGMTFRAMEVLANCHECK_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFrameVlanCheck_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.vlanCheck);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINEMGMT_GET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmt_get(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, &buf.mgmuser_trap_cfg.userDefine);
            copy_to_user(user, &buf.mgmuser_trap_cfg, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINEMGMTACTION_GET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtAction_get(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, (rtk_action_t*)&buf.mgmuser_trap_cfg.rma_action);
            copy_to_user(user, &buf.mgmuser_trap_cfg, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINEMGMTPRI_GET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtPri_get(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, &buf.mgmuser_trap_cfg.priority);
            copy_to_user(user, &buf.mgmuser_trap_cfg, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINEMGMTPRIENABLE_GET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtPriEnable_get(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, &buf.mgmuser_trap_cfg.enable);
            copy_to_user(user, &buf.mgmuser_trap_cfg, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINEMGMTVLANCHECK_GET:
            copy_from_user(&buf.mgmuser_trap_cfg, user, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineMgmtVlanCheck_get(buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx, &buf.mgmuser_trap_cfg.vlanCheck);
            copy_to_user(user, &buf.mgmuser_trap_cfg, sizeof(rtdrv_trapUserMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEACTION_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFrameAction_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, (rtk_action_t*)&buf.mgm_trap_cfg.rma_action);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEPRI_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFramePri_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.priority);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEPRIENABLE_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFramePriEnable_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.enable);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_PORTMGMTFRAMEVLANCHECK_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFrameVlanCheck_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.vlanCheck);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_PORTMGMTFRAMECROSSVLAN_GET:
            copy_from_user(&buf.mgm_trap_cfg, user, sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_portMgmtFrameCrossVlan_get(buf.mgm_trap_cfg.unit, buf.mgm_trap_cfg.port,
                        buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.vlanCross);
            copy_to_user(user, &buf.mgm_trap_cfg, sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERACTION_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderAction_get(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        buf.other_trap_cfg.ipFamily, &buf.other_trap_cfg.action);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERPRI_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderPri_get(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        &buf.other_trap_cfg.priority);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERPRIENABLE_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderPriEnable_get(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        &buf.other_trap_cfg.enable);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_IPWITHOPTIONHEADERADDCPUTAGENABLE_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_ipWithOptionHeaderAddCPUTagEnable_get(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        &buf.other_trap_cfg.cputag);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_PKTWITHCFIACTION_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIAction_get(buf.other_trap_cfg.unit,
                    &buf.other_trap_cfg.action);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_PKTWITHOUTERCFIACTION_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithOuterCFIAction_get(buf.other_trap_cfg.unit,
                    &buf.other_trap_cfg.action);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_PORTPKTWITHCFIACTION_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_portPktWithCFIAction_get(buf.other_trap_cfg.unit,
                    buf.other_trap_cfg.port, &buf.other_trap_cfg.action);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_PKTWITHCFIPRI_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIPri_get(buf.other_trap_cfg.unit,
                    &buf.other_trap_cfg.priority);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_PORTPKTWITHCFIPRI_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_portPktWithCFIPri_get(buf.other_trap_cfg.unit,
                    buf.other_trap_cfg.port, &buf.other_trap_cfg.priority);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_PKTWITHCFIPRIENABLE_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIPriEnable_get(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        &buf.other_trap_cfg.enable);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_PKTWITHCFIADDCPUTAGENABLE_GET:
            copy_from_user(&buf.other_trap_cfg, user, sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_pktWithCFIAddCPUTagEnable_get(buf.other_trap_cfg.unit, buf.other_trap_cfg.port,
                        &buf.other_trap_cfg.cputag);
            copy_to_user(user, &buf.other_trap_cfg, sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_CFMFRAMEACTION_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameAction_get(buf.cfm_trap_cfg.unit, buf.cfm_trap_cfg.md_level, &buf.cfm_trap_cfg.action);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_CFMUNKNOWNFRAMEACT_GET:
            copy_from_user(&buf.misc_cfg, user, sizeof(rtdrv_oamCfmMiscCfg_t));
            ret = rtk_trap_cfmUnknownFrameAct_get(buf.misc_cfg.unit,
                                                  &buf.misc_cfg.action);
            copy_to_user(user, &buf.misc_cfg, sizeof(rtdrv_oamCfmMiscCfg_t));
            break;

        case RTDRV_TRAP_CFMLOOPBACKACT_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmLoopbackLinkTraceAct_get(buf.cfm_trap_cfg.unit,
                                              buf.cfm_trap_cfg.md_level,
                                              &buf.cfm_trap_cfg.action);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_CFMCCMACT_GET:
            copy_from_user(&buf.oam_trap_cfg, user, sizeof(rtdrv_trapOamCfg_t));
            ret = rtk_trap_cfmCcmAct_get(buf.oam_trap_cfg.unit,
                                         buf.oam_trap_cfg.md_level,
                                         &buf.oam_trap_cfg.action);
            copy_to_user(user, &buf.oam_trap_cfg, sizeof(rtdrv_trapOamCfg_t));
            break;

        case RTDRV_TRAP_CFMETHDMACT_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmEthDmAct_get(buf.cfm_trap_cfg.unit,
                                           buf.cfm_trap_cfg.md_level,
                                           &buf.cfm_trap_cfg.action);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_CFMFRAMETRAPPRI_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameTrapPri_get(buf.cfm_trap_cfg.unit, &buf.cfm_trap_cfg.priority);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_CFMFRAMETRAPPRIENABLE_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameTrapPriEnable_get(buf.cfm_trap_cfg.unit, &buf.cfm_trap_cfg.enable);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_CFMFRAMETRAPADDCPUTAGENABLE_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_cfmFrameTrapAddCPUTagEnable_get(buf.cfm_trap_cfg.unit, &buf.cfm_trap_cfg.cputag);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_PORTOAMPDUACTION_GET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_portOamPDUAction_get(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, &buf.port_trap_cfg.action);
            copy_to_user(user, &buf.port_trap_cfg, sizeof(rtdrv_trapPortCfg_t));
            break;

        case RTDRV_TRAP_PORTOAMPDUPRI_GET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_portOamPDUPri_get(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, &buf.port_trap_cfg.priority);
            copy_to_user(user, &buf.port_trap_cfg, sizeof(rtdrv_trapPortCfg_t));
            break;

        case RTDRV_TRAP_OAMPDUACTION_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_oamPDUAction_get(buf.cfm_trap_cfg.unit, &buf.cfm_trap_cfg.action);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_OAMPDUPRI_GET:
            copy_from_user(&buf.cfm_trap_cfg, user, sizeof(rtdrv_trapCfmCfg_t));
            ret = rtk_trap_oamPDUPri_get(buf.cfm_trap_cfg.unit, &buf.cfm_trap_cfg.priority);
            copy_to_user(user, &buf.cfm_trap_cfg, sizeof(rtdrv_trapCfmCfg_t));
            break;

        case RTDRV_TRAP_OAMPDUPRIENABLE_GET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_oamPDUPriEnable_get(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, &buf.port_trap_cfg.enable);
            copy_to_user(user, &buf.port_trap_cfg, sizeof(rtdrv_trapPortCfg_t));
            break;

        case RTDRV_TRAP_OAMPDUTRAPADDCPUTAGENABLE_GET:
            copy_from_user(&buf.port_trap_cfg, user, sizeof(rtdrv_trapPortCfg_t));
            ret = rtk_trap_oamPDUTrapAddCPUTagEnable_get(buf.port_trap_cfg.unit, buf.port_trap_cfg.port, &buf.port_trap_cfg.cputag);
            copy_to_user(user, &buf.port_trap_cfg, sizeof(rtdrv_trapPortCfg_t));
            break;

        case RTDRV_TRAP_1XMACCHANGEPORT2CPUENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_1xMacChangePort2CpuEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_IGMPCTRLPKT2CPUENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_igmpCtrlPkt2CpuEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_L2MCASTPKT2CPUENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_l2McastPkt2CpuEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_IPMCASTPKT2CPUENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_ipMcastPkt2CpuEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_REASONTRAPTOCPUPRIORITY_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_reasonTrapToCPUPriority_get(buf.trap_cfg.unit, buf.trap_cfg.reason, &buf.trap_cfg.priority);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_PKT2CPUENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_pkt2CpuEnable_get(buf.trap_cfg.unit, buf.trap_cfg.pkt_type, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_MGMTIPCHECK_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_mgmtIpCheck_get(buf.trap_cfg.unit, buf.trap_cfg.ip_type, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_PORTOAMLOOPBACKPARACTION_GET:
            copy_from_user(&buf.oam_trap_cfg, user, sizeof(rtdrv_trapOamCfg_t));
            ret = rtk_trap_portOamLoopbackParAction_get(buf.oam_trap_cfg.unit,
                    buf.oam_trap_cfg.port, &buf.oam_trap_cfg.action);
            copy_to_user(user, &buf.oam_trap_cfg, sizeof(rtdrv_trapOamCfg_t));
            break;

        case RTDRV_TRAP_ROUTEEXCEPTIONACTION_GET:
            copy_from_user(&buf.routeException_trap_cfg, user,
                    sizeof(rtdrv_trapRouteExceptionCfg_t));
            ret = rtk_trap_routeExceptionAction_get(
                    buf.routeException_trap_cfg.unit,
                    buf.routeException_trap_cfg.type,
                    &buf.routeException_trap_cfg.action);
            copy_to_user(user, &buf.routeException_trap_cfg,
                    sizeof(rtdrv_trapRouteExceptionCfg_t));
            break;

        case RTDRV_TRAP_ROUTEEXCEPTIONPRI_GET:
            copy_from_user(&buf.routeException_trap_cfg, user,
                    sizeof(rtdrv_trapRouteExceptionCfg_t));
            ret = rtk_trap_routeExceptionPri_get(
                    buf.routeException_trap_cfg.unit,
                    buf.routeException_trap_cfg.type,
                    &buf.routeException_trap_cfg.priority);
            copy_to_user(user, &buf.routeException_trap_cfg,
                    sizeof(rtdrv_trapRouteExceptionCfg_t));
            break;

        case RTDRV_TRAP_USERDEFINERMALEARNINGENABLE_GET:
            copy_from_user(&buf.mgmuser_trap_cfg, user,
                    sizeof(rtdrv_trapUserMgmRmaCfg_t));
            ret = rtk_trap_userDefineRmaLearningEnable_get(
                    buf.mgmuser_trap_cfg.unit, buf.mgmuser_trap_cfg.mgmt_idx,
                    &buf.mgmuser_trap_cfg.enable);
            copy_to_user(user, &buf.mgmuser_trap_cfg,
                    sizeof(rtdrv_trapUserMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_RMALEARNINGENABLE_GET:
            copy_from_user(&buf.trap_cfg, user,
                    sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaLearningEnable_get(
                    buf.trap_cfg.unit, &buf.trap_cfg.rma_frame,
                    &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

        case RTDRV_TRAP_MGMTFRAMELEARNINGENABLE_GET:
            copy_from_user(&buf.mgm_trap_cfg, user,
                    sizeof(rtdrv_trapMgmRmaCfg_t));
            ret = rtk_trap_mgmtFrameLearningEnable_get(buf.mgm_trap_cfg.unit,
                    buf.mgm_trap_cfg.frameType, &buf.mgm_trap_cfg.enable);
            copy_to_user(user, &buf.mgm_trap_cfg,
                    sizeof(rtdrv_trapMgmRmaCfg_t));
            break;

        case RTDRV_TRAP_MGMTFRAMEMGMTVLANENABLE_GET:
            copy_from_user(&buf.other_trap_cfg, user,
                    sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_mgmtFrameMgmtVlanEnable_get(buf.other_trap_cfg.unit,
                    &buf.other_trap_cfg.enable);
            copy_to_user(user, &buf.other_trap_cfg,
                    sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_BPDUFLOODPORTMASK_GET:
            copy_from_user(&buf.bpdu_flood_pmsk_cfg, user,
                    sizeof(rtdrv_bpduFloodPmskCfg_t));
            ret = rtk_trap_bpduFloodPortmask_get(
                    buf.bpdu_flood_pmsk_cfg.unit, &buf.bpdu_flood_pmsk_cfg.pmsk);
            copy_to_user(user, &buf.bpdu_flood_pmsk_cfg,
                    sizeof(rtdrv_bpduFloodPmskCfg_t));
            break;

        case RTDRV_TRAP_RMAGROUPACTION_GET:
            copy_from_user(&buf.rma_grp_act_cfg, user,
                    sizeof(rtdrv_rmaGroupType_t));
            ret = rtk_trap_rmaGroupAction_get(
                    buf.rma_grp_act_cfg.unit, buf.rma_grp_act_cfg.rmaGroup_frameType, &buf.rma_grp_act_cfg.rma_action);
            copy_to_user(user, &buf.rma_grp_act_cfg,
                    sizeof(rtdrv_rmaGroupType_t));
            break;

        case RTDRV_TRAP_RMAGROUPLEARNINGENABLE_GET:
            copy_from_user(&buf.rma_grp_lrn_cfg, user,
                    sizeof(rtdrv_rmaGroupLearn_t));
            ret = rtk_trap_rmaGroupLearningEnable_get(
                    buf.rma_grp_lrn_cfg.unit, buf.rma_grp_lrn_cfg.rmaGroup_frameType, &buf.rma_grp_lrn_cfg.enable);
            copy_to_user(user, &buf.rma_grp_lrn_cfg,
                    sizeof(rtk_enable_t));
            break;

        case RTDRV_TRAP_MGMTFRAMESELFARPENABLE_GET:
            copy_from_user(&buf.other_trap_cfg, user,
                    sizeof(rtdrv_trapOtherCfg_t));
            ret = rtk_trap_mgmtFrameSelfARPEnable_get(buf.other_trap_cfg.unit,
                    &buf.other_trap_cfg.enable);
            copy_to_user(user, &buf.other_trap_cfg,
                    sizeof(rtdrv_trapOtherCfg_t));
            break;

        case RTDRV_TRAP_RMALOOKUPMISSACTIONENABLE_GET:
            copy_from_user(&buf.trap_cfg, user, sizeof(rtdrv_trapCfg_t));
            ret = rtk_trap_rmaLookupMissActionEnable_get(buf.trap_cfg.unit, &buf.trap_cfg.enable);
            copy_to_user(user, &buf.trap_cfg, sizeof(rtdrv_trapCfg_t));
            break;

    /** FILTER **/
        case RTDRV_FILTER_CUTLINE_GET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_blkCutline_get(buf.filter_cfg.unit, &buf.filter_cfg.cutline);
            copy_to_user(user, &buf.filter_cfg, sizeof(rtdrv_filterCfg_t));
            break;

        case RTDRV_FILTER_ENABLE_GET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_pieEnable_get(buf.filter_cfg.unit, &buf.filter_cfg.enable);
            copy_to_user(user, &buf.filter_cfg, sizeof(rtdrv_filterCfg_t));
            break;

        case RTDRV_FILTER_FLOW_TBL_GET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_flowTbl_get(buf.filter_cfg.unit, buf.filter_cfg.filter_id, &buf.filter_cfg.flow_table_cfg,
                                         &buf.filter_cfg.action);
            copy_to_user(user, &buf.filter_cfg, sizeof(rtdrv_filterCfg_t));
            break;

        case RTDRV_FILTER_IGR_ACL_GET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_igrAcl_get(buf.filter_cfg.unit, buf.filter_cfg.filter_id, &buf.filter_cfg.acl_cfg,
                                        &buf.filter_cfg.action);
            copy_to_user(user, &buf.filter_cfg, sizeof(rtdrv_filterCfg_t));
            break;

        case RTDRV_FILTER_LOG_COUNTER_GET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_stat_get(buf.filter_cfg.unit, buf.filter_cfg.index, &buf.filter_cfg.packet_counter,
                                      &buf.filter_cfg.byte_counter);
            copy_to_user(user, &buf.filter_cfg, sizeof(rtdrv_filterCfg_t));
            break;

        case RTDRV_FILTER_PATTERN_MATCH_GET:
            copy_from_user(&buf.pattern_cfg, user, sizeof(rtdrv_patternCfg_t));
            ret = rtk_filter_patternMatch_get(buf.pattern_cfg.unit, buf.pattern_cfg.port, &buf.pattern_cfg.mode,
                                              buf.pattern_cfg.pattern, &buf.pattern_cfg.mask);
            copy_to_user(user, &buf.pattern_cfg, sizeof(rtdrv_patternCfg_t));
            break;

        case RTDRV_FILTER_RATE_LIMIT_GET:
            copy_from_user(&buf.filter_cfg, user, sizeof(rtdrv_filterCfg_t));
            ret = rtk_filter_igrAclRateLimit_get(buf.filter_cfg.unit, buf.filter_cfg.index, &buf.filter_cfg.rate);
            copy_to_user(user, &buf.filter_cfg, sizeof(rtdrv_filterCfg_t));
            break;

/** PIE **/
        case RTDRV_PIE_ENTRY_FIELD_SIZE_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryFieldSize_get(buf.pie_cfg.unit, buf.pie_cfg.field_type, &buf.pie_cfg.size);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_ENTRY_SIZE_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntrySize_get(buf.pie_cfg.unit, &buf.pie_cfg.size);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_ENTRY_FIELD_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryField_get(buf.pie_cfg.unit, buf.pie_cfg.phase, buf.pie_cfg.index,
                                                buf.pie_cfg.entry_buffer, buf.pie_cfg.field_type,
                                                buf.pie_cfg.field_data, buf.pie_cfg.field_mask);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_ENTRY_FIELD_SET_TO_BUFFER:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryField_set(buf.pie_cfg.unit, buf.pie_cfg.phase, buf.pie_cfg.index,
                                                buf.pie_cfg.entry_buffer, buf.pie_cfg.field_type,
                                                buf.pie_cfg.field_data, buf.pie_cfg.field_mask);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_ENTRY_FIELD_READ:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntryField_read(buf.pie_cfg.unit, buf.pie_cfg.phase, buf.pie_cfg.index,
                                                 buf.pie_cfg.field_type, buf.pie_cfg.field_data,
                                                 buf.pie_cfg.field_mask);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_PREDEFINED_ENTRY_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePreDefinedRuleEntry_get(buf.pie_cfg.unit, buf.pie_cfg.entry_buffer,
                                                     &buf.pie_cfg.predefined_entry);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_ENTRY_READ:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleEntry_read(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.entry_buffer);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_ACTION_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRuleAction_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.action);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_POLICER_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieRulePolicer_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.policer);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_HIT_INDICATION_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieHitIndication_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.hit_status);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_COUNTER_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieStat_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.pkt_cnt, &buf.pie_cfg.byte_cnt);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_TMP_SELECTOR_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieTemplateSelector_get(buf.pie_cfg.unit, buf.pie_cfg.index, buf.pie_cfg.phase, &buf.pie_cfg.index1);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_USER_TMP_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieUserTemplate_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.template);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_L34_CHECKSUN_ERR_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieL34ChecksumErr_get(buf.pie_cfg.unit, &buf.pie_cfg.checksum_err_op);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_TMP_PAYLOAD_OFFSET_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieUserTemplatePayloadOffset_get(buf.pie_cfg.unit, buf.pie_cfg.index,
                                                           buf.pie_cfg.index1, &buf.pie_cfg.offset);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_RESULT_REVERSE_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieResultReverse_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.reverse_op);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_RESULT_AGGREGATOR_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieResultAggregator_get(buf.pie_cfg.unit, buf.pie_cfg.index,
                                                           buf.pie_cfg.index1, &buf.pie_cfg.aggregator_type);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_BLOCK_PRI_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieBlockPriority_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.priority);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_GROUP_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieGroupCtrl_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.group_op);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_EGR_ACL_CTRL_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieEgrAclLookupCtrl_get(buf.pie_cfg.unit, &buf.pie_cfg.egr_acl_ctrl);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_PORT_LP_ENABLE_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePortLookupPhaseEnable_get(buf.pie_cfg.unit, buf.pie_cfg.port, buf.pie_cfg.phase, &buf.pie_cfg.enable);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_PORT_LP_MISS_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePortLookupPhaseMiss_get(buf.pie_cfg.unit, buf.pie_cfg.port, buf.pie_cfg.phase, &buf.pie_cfg.miss_action);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;
        case RTDRV_PIE_COUNTER_MODE_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_pieCounterIndicationMode_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.counter_mode);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_POLICER_CTRL_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePolicerCtrl_get(buf.pie_cfg.unit, &buf.pie_cfg.policer_ctrl);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_RC_L4PORT_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckL4Port_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_l4port_data);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;
        case RTDRV_PIE_RC_VID_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckVid_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_vid_data);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_RC_IP_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckIp_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_ip_data);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_RC_SRC_PORT_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_rangeCheckSrcPort_get(buf.pie_cfg.unit, buf.pie_cfg.index, &buf.pie_cfg.rc_src_port_data);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_FS_ENABLE_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_fieldSelectorEnable_get(buf.pie_cfg.unit, buf.pie_cfg.port, buf.pie_cfg.index, &buf.pie_cfg.enable);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_FS_CONTENT_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_fieldSelectorContent_get(buf.pie_cfg.unit, buf.pie_cfg.port, buf.pie_cfg.index, &buf.pie_cfg.fs_content);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_PM_ENABLE_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_patternMatchEnable_get(buf.pie_cfg.unit, buf.pie_cfg.port, &buf.pie_cfg.enable);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_PM_CONTENT_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_patternMatchContent_get(buf.pie_cfg.unit, buf.pie_cfg.port, &buf.pie_cfg.pm_content);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

        case RTDRV_PIE_BUFFER_ENTRY_FROM_PREDEF_GET:
            copy_from_user(&buf.pie_cfg, user, sizeof(rtdrv_pieCfg_t));
            ret = rtk_pie_piePreDefinedRuleEntry_set(buf.pie_cfg.unit, buf.pie_cfg.entry_buffer,
                                                     &buf.pie_cfg.predefined_entry);
            copy_to_user(user, &buf.pie_cfg, sizeof(rtdrv_pieCfg_t));
            break;

    /** ACL **/
        case RTDRV_ACL_ENTRY_FIELD_SIZE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntryFieldSize_get(buf.acl_cfg.unit, buf.acl_cfg.field_type, &buf.acl_cfg.size);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_ENTRY_SIZE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntrySize_get(buf.acl_cfg.unit, buf.acl_cfg.phase, &buf.acl_cfg.size);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_ENTRY_DATA_READ:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntry_read(buf.acl_cfg.unit, buf.acl_cfg.phase, buf.acl_cfg.index, buf.acl_cfg.entry_buffer);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_METER_MODE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterMode_get(buf.acl_cfg.unit, buf.acl_cfg.blockIdx, &buf.acl_cfg.meterMode);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_METER_INCLUDE_IFG_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterIncludeIfg_get(buf.acl_cfg.unit, &buf.acl_cfg.ifg_include);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_METER_BURST_SIZE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterBurstSize_get(buf.acl_cfg.unit, buf.acl_cfg.meterMode, &buf.acl_cfg.burstSize);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_METER_ENTRY_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterEntry_get(buf.acl_cfg.unit, buf.acl_cfg.meterIdx, &buf.acl_cfg.meterEntry);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_METER_EXCEED_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterExceed_get(buf.acl_cfg.unit, buf.acl_cfg.meterIdx, &buf.acl_cfg.isExceed);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_METER_EXCEED_AGGREGATION_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_meterExceedAggregation_get(buf.acl_cfg.unit, &buf.acl_cfg.exceedMask);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_PARTITION_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_partition_get(buf.acl_cfg.unit, &buf.acl_cfg.blockIdx);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_BLOCKPWRENABLE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockPwrEnable_get(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, &buf.acl_cfg.status);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_BLOCKLOOKUPENABLE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockLookupEnable_get(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, &buf.acl_cfg.status);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RULEVALIDATE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleValidate_get(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, &buf.acl_cfg.status);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RULEENTRYFIELD_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntryField_get(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, buf.acl_cfg.entry_buffer,
                    buf.acl_cfg.field_type, buf.acl_cfg.field_data,
                    buf.acl_cfg.field_mask);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RULEENTRYFIELD_READ:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntryField_read(buf.acl_cfg.unit,
                    buf.acl_cfg.phase, buf.acl_cfg.index,
                    buf.acl_cfg.field_type, buf.acl_cfg.field_data,
                    buf.acl_cfg.field_mask);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RULEENTRYFIELD_CHECK:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleEntryField_check(buf.acl_cfg.unit,
                    buf.acl_cfg.phase, buf.acl_cfg.field_type);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RULEOPERATION_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleOperation_get(buf.acl_cfg.unit,
                    buf.acl_cfg.phase, buf.acl_cfg.index,
                    &buf.acl_cfg.oper);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RULEACTION_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleAction_get(buf.acl_cfg.unit,
                    buf.acl_cfg.phase, buf.acl_cfg.index,
                    &buf.acl_cfg.action);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_TEMPLATESELECTOR_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_templateSelector_get(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, &buf.acl_cfg.template_idx);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_TEMPLATE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_template_get(buf.acl_cfg.unit,
                    buf.acl_cfg.index, &buf.acl_cfg.template);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_TEMPLATEFIELD_CHECK:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_templateField_check(buf.acl_cfg.unit,
                    buf.acl_cfg.phase, buf.acl_cfg.field_type);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_BLOCKRESULTMODE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockResultMode_get(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, &buf.acl_cfg.blk_mode);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_BLOCKAGGREGATORENABLE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_blockGroupEnable_get(buf.acl_cfg.unit,
                    buf.acl_cfg.blockIdx, buf.acl_cfg.blk_group,
                    &buf.acl_cfg.status);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_STATPKTCNT_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_statPktCnt_get(buf.acl_cfg.unit, buf.acl_cfg.index,
                    &buf.acl_cfg.size);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_STATBYTECNT_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_statByteCnt_get(buf.acl_cfg.unit, buf.acl_cfg.index,
                    &buf.acl_cfg.count);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RANGECHECKL4PORT_GET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckL4Port_get(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_l4Port);
            copy_to_user(user, &buf.rangeCheck_cfg, sizeof(rtdrv_rangeCheckCfg_t));
            break;

        case RTDRV_ACL_RANGECHECKVID_GET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckVid_get(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_vid);
            copy_to_user(user, &buf.rangeCheck_cfg, sizeof(rtdrv_rangeCheckCfg_t));
            break;

        case RTDRV_ACL_RANGECHECKIP_GET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckIp_get(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_ip);
            copy_to_user(user, &buf.rangeCheck_cfg, sizeof(rtdrv_rangeCheckCfg_t));
            break;

        case RTDRV_ACL_RANGECHECKSRCPORT_GET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckSrcPort_get(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_port);
            copy_to_user(user, &buf.rangeCheck_cfg, sizeof(rtdrv_rangeCheckCfg_t));
            break;

        case RTDRV_ACL_RANGECHECKDSTPORT_GET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckDstPort_get(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_port);
            copy_to_user(user, &buf.rangeCheck_cfg, sizeof(rtdrv_rangeCheckCfg_t));
            break;

        case RTDRV_ACL_RANGECHECKPACKETLEN_GET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckPacketLen_get(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_pktLen);
            copy_to_user(user, &buf.rangeCheck_cfg, sizeof(rtdrv_rangeCheckCfg_t));
            break;

        case RTDRV_ACL_FIELDSELECTOR_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_fieldSelector_get(buf.acl_cfg.unit, buf.acl_cfg.index,
                    &buf.acl_cfg.fs);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_PORTLOOKUPENABLE_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_portLookupEnable_get(buf.acl_cfg.unit,
                    buf.acl_cfg.port, &buf.acl_cfg.status);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_LOOKUPMISSACT_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_lookupMissAct_get(buf.acl_cfg.unit,
                    buf.acl_cfg.port, &buf.acl_cfg.lmAct);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_RANGECHECKFIELDSEL_GET:
            copy_from_user(&buf.rangeCheck_cfg, user, sizeof(rtdrv_rangeCheckCfg_t));
            ret = rtk_acl_rangeCheckFieldSelector_get(buf.rangeCheck_cfg.unit,
                    buf.rangeCheck_cfg.index, &buf.rangeCheck_cfg.range_fieldSel);
            copy_to_user(user, &buf.rangeCheck_cfg, sizeof(rtdrv_rangeCheckCfg_t));
            break;

        case RTDRV_ACL_RULEHITINDICATION_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_ruleHitIndication_get(buf.acl_cfg.unit, buf.acl_cfg.phase,
                    buf.acl_cfg.index, buf.acl_cfg.count, &buf.acl_cfg.status);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

        case RTDRV_ACL_TEMPLATEFIELDINTENTVLANTAG_GET:
            copy_from_user(&buf.acl_cfg, user, sizeof(rtdrv_aclCfg_t));
            ret = rtk_acl_templateFieldIntentVlanTag_get(buf.acl_cfg.unit, &buf.acl_cfg.tagType);
            copy_to_user(user, &buf.acl_cfg, sizeof(rtdrv_aclCfg_t));
            break;

    /** QOS **/
        case RTDRV_QOS_QUEUE_NUM_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_queueNum_get(buf.qos_cfg.unit, &buf.qos_cfg.queue_num);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PRI_MAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_priMap_get(buf.qos_cfg.unit, buf.qos_cfg.queue_num, &buf.qos_cfg.pri2qid);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_PRI_MAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriMap_get(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.int_pri, &buf.qos_cfg.queue);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_PRI_REMAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pPriRemap_get(buf.qos_cfg.unit, buf.qos_cfg.dot1p_pri, &buf.qos_cfg.int_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_PRI_REMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pPriRemapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.dot1p_pri, &buf.qos_cfg.int_pri, &(buf.qos_cfg.dp));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_1P_PRI_REMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_port1pPriRemapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.index));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUTER_1P_PRI_REMAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pPriRemap_get(buf.qos_cfg.unit, buf.qos_cfg.dot1p_pri, buf.qos_cfg.dei, &(buf.qos_cfg.int_pri));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUTER_1P_PRI_REMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pPriRemapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.dot1p_pri
                                                , buf.qos_cfg.dei, &(buf.qos_cfg.int_pri), &(buf.qos_cfg.dp));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OUTER_1P_PRI_REMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pPriRemapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.index));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DEI_DP_REMAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_deiDpRemap_get(buf.qos_cfg.unit, buf.qos_cfg.dei, &buf.qos_cfg.dp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_DEI_SRC_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDEISrcSel_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.deiSrc);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP_DP_REMAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpDpRemap_get(buf.qos_cfg.unit, buf.qos_cfg.dscp, &buf.qos_cfg.dp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP_PRI_REMAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpPriRemap_get(buf.qos_cfg.unit, buf.qos_cfg.dscp, &buf.qos_cfg.int_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP_PRI_REMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpPriRemapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.dscp, &buf.qos_cfg.int_pri, &(buf.qos_cfg.dp));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_DSCP_PRI_REMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDscpPriRemapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.index));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_PRI_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPri_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.int_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_DP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDp_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.dp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_INNER_PRI_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portInnerPri_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.int_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OUTER_PRI_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuterPri_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.int_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OUTER_DEI_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuterDEI_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.dei);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DP_SRC_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dpSrcSel_get(buf.qos_cfg.unit, &buf.qos_cfg.dpSrcType);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PRI_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_priSel_get(buf.qos_cfg.unit, &buf.qos_cfg.port_pri, &buf.qos_cfg.class_pri,
                                     &buf.qos_cfg.acl_pri, &buf.qos_cfg.dscp_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PRI_SEL_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_priSelGroup_get(buf.qos_cfg.unit, buf.qos_cfg.index, &(buf.qos_cfg.priSelWeight));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_PRI_SEL_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriSelGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.index));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_REMARK_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemarkEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.remark_enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP_REMARK_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemarkEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.remark_enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_DFLT_PRI_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pDfltPri_get(buf.qos_cfg.unit, &buf.qos_cfg.dot1p_dflt_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_REMARK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemark_get(buf.qos_cfg.unit, buf.qos_cfg.int_pri, &buf.qos_cfg.dot1p_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_REMARK_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemarkGroup_get(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.int_pri, buf.qos_cfg.dp, &buf.qos_cfg.dot1p_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_REMARK_SRC_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pRemarkSrcSel_get(buf.qos_cfg.unit, &buf.qos_cfg.rmksrc_1p);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_1P_REMARK_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_port1pRemarkGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.index);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_1P_PRIMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_port1pPriMapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.index);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUT_1P_REMARK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pRemark_get(buf.qos_cfg.unit, buf.qos_cfg.int_pri, &buf.qos_cfg.dot1p_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUT_1P_REMARK_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_out1pRemarkEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.remark_enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUTER_1P_REMARK_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pRemarkGroup_get(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.int_pri, buf.qos_cfg.dp
                                        , &(buf.qos_cfg.dot1p_pri), &(buf.qos_cfg.dei));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUT_1P_REMARK_SRC_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pRemarkSrcSel_get(buf.qos_cfg.unit, &buf.qos_cfg.rmksrc_outer1p);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OUTER_1P_DFLT_SRC_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pDfltPriSrcSel_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.out1p_dflt_src));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OUTER_1P_REMARK_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pRemarkGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.index));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OUTER_1P_PRIMAP_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pPriMapGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.index));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP_REMARK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemark_get(buf.qos_cfg.unit, buf.qos_cfg.int_pri, &buf.qos_cfg.dscp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP2DOT1P_REMARK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscp2Dot1pRemark_get(buf.qos_cfg.unit, buf.qos_cfg.org_dscp, &buf.qos_cfg.dot1p_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP2OUT1P_REMARK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscp2Outer1pRemark_get(buf.qos_cfg.unit, buf.qos_cfg.org_dscp, &buf.qos_cfg.dot1p_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP2DSCP_REMARK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscp2DscpRemark_get(buf.qos_cfg.unit, buf.qos_cfg.org_dscp, &buf.qos_cfg.dscp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP_REMARK_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemarkGroup_get(buf.qos_cfg.unit, buf.qos_cfg.index, buf.qos_cfg.int_pri, buf.qos_cfg.dp
                                            , &buf.qos_cfg.dscp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DSCP_REMARK_SRC_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_dscpRemarkSrcSel_get(buf.qos_cfg.unit, &buf.qos_cfg.rmksrc_dscp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_DSCP_REMARK_GROUP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portdscpRemarkGroup_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.index));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DEI_REMARK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_deiRemark_get(buf.qos_cfg.unit, buf.qos_cfg.dp, &(buf.qos_cfg.dei));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_DEI_REMARK_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_deiRemarkEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.remark_enable));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_DEI_REMARK_TAG_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portDEIRemarkTagSel_get(buf.qos_cfg.unit, buf.qos_cfg.port, &(buf.qos_cfg.deiSrc));
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_SCHEDULING_ALGORITHM_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_schedulingAlgorithm_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.scheduling_type);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_SCHEDULING_QUEUE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_schedulingQueue_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.qweights);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_WFQ_FIXED_BANDWIDTH_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wfqFixedBandwidthEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_ALGO_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidAlgo_get(buf.qos_cfg.unit, &buf.qos_cfg.congAvoid_algo);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidQueueThreshEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_PORT_THRESH_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidPortThreshEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_SYS_THRESH_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidSysThreshEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_SYS_THRESH_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidSysThresh_get(buf.qos_cfg.unit, buf.qos_cfg.dp, &buf.qos_cfg.congAvoid_thresh);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_SYS_DROP_PROBABILITY_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidSysDropProbability_get(buf.qos_cfg.unit, buf.qos_cfg.dp, &buf.qos_cfg.data);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_PORT_THRESH_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidPortThresh_get(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.dp, &buf.qos_cfg.congAvoid_thresh);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_QUEUE_THRESH_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidQueueThresh_get(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, &buf.qos_cfg.congAvoid_thresh);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_THRESH_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidGlobalQueueThresh_get(buf.qos_cfg.unit, buf.qos_cfg.queue, buf.qos_cfg.dp, &buf.qos_cfg.congAvoid_thresh);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_CONG_AVOID_GLOBAL_QUEUE_DROP_PROBABILITY_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_congAvoidGlobalQueueDropProbability_get(buf.qos_cfg.unit, buf.qos_cfg.queue, buf.qos_cfg.dp, &buf.qos_cfg.data);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_WRED_SYS_THRESH_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredSysThresh_get(buf.qos_cfg.unit, buf.qos_cfg.dp, &buf.qos_cfg.wred_thresh);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_WRED_WEIGHT_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredWeight_get(buf.qos_cfg.unit, &buf.qos_cfg.data);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_WRED_MPD_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredMpd_get(buf.qos_cfg.unit, &buf.qos_cfg.data);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_WRED_ECN_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredEcnEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_WRED_CNT_REVERSE_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_wredCntReverseEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_AVB_SR_CLASS_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portAvbStreamReservationClassEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.srClass, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_AVB_SR_CONFIG_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_avbStreamReservationConfig_get(buf.qos_cfg.unit, &buf.qos_cfg.srConf);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PKT2CPU_PRI_REMAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_pkt2CpuPriRemap_get(buf.qos_cfg.unit, buf.qos_cfg.int_pri, &buf.qos_cfg.new_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PRI2IGR_QUEUE_MAP_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPri2IgrQMap_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.pri2qid);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PRI2IGR_QUEUE_MAP_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPri2IgrQMapEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_IGR_QUEUE_WEIGHT_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_igrQueueWeight_get(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, &buf.qos_cfg.data);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_1P_DFLT_PRI_SRC_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_1pDfltPriSrcSel_get(buf.qos_cfg.unit, &buf.qos_cfg.dflt_src_1p);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OUTER_1P_REMARK_SRC_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuter1pRemarkSrcSel_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.rmksrc_outer1p);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUTER_1P_DFLT_PRI_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pDfltPri_get(buf.qos_cfg.unit, &buf.qos_cfg.out1p_dflt_pri);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_OUTER_1P_DFLT_PRI_CFG_SRC_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_outer1pDfltPriCfgSrcSel_get(buf.qos_cfg.unit,&buf.qos_cfg.out1p_dflt_cfg_dir);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_INVLD_DSCP_VAL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_invldDscpVal_get(buf.qos_cfg.unit, &buf.qos_cfg.dscp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_INVLD_DSCP_MASK_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_invldDscpMask_get(buf.qos_cfg.unit, &buf.qos_cfg.dscp);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_INVLD_DSCP_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portInvldDscpEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_INVLD_DSCP_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_invldDscpEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_PRI_REMAP_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriRemapEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_PRI_REMAP_SEL_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portPriRemapSel_get(buf.qos_cfg.unit, &buf.qos_cfg.portPriRemap_type);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_IPRI_REMAP_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portInnerPriRemapEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_PORT_OPRI_REMAP_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_portOuterPriRemapEnable_get(buf.qos_cfg.unit, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

        case RTDRV_QOS_QUEUE_STRICT_ENABLE_GET:
            copy_from_user(&buf.qos_cfg, user, sizeof(rtdrv_qosCfg_t));
            ret = rtk_qos_queueStrictEnable_get(buf.qos_cfg.unit, buf.qos_cfg.port, buf.qos_cfg.queue, &buf.qos_cfg.enable);
            copy_to_user(user, &buf.qos_cfg, sizeof(rtdrv_qosCfg_t));
            break;

    /** TRUNK **/
        case RTDRV_TRUNK_PORT_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_port_get(buf.trunk_cfg.unit,buf.trunk_cfg.trk_gid, &buf.trunk_cfg.trk_member);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_BIND_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithmBind_get(buf.trunk_cfg.unit,buf.trunk_cfg.trk_gid, &buf.trunk_cfg.algo_id);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithm_get(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid,
                                                      &buf.trunk_cfg.algo_bitmask);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_PARAM_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithmParam_get(buf.trunk_cfg.unit,buf.trunk_cfg.algo_id, &buf.trunk_cfg.algo_bitmask);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_DISTRIBUTION_ALGORITHM_SHIFT_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_distributionAlgorithmShift_get(buf.trunk_cfg.unit,buf.trunk_cfg.algo_id, &buf.trunk_cfg.shift);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_HASH_MAPPING_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_hashMappingTable_get(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid,
                                                 &buf.trunk_cfg.hash2Port_array);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_TRUNK_MODE_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_mode_get(buf.trunk_cfg.unit, &buf.trunk_cfg.mode);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_REPRESENTPORT_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_representPort_get(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, &buf.trunk_cfg.represPort);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_FLOODMODE_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_floodMode_get(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, &buf.trunk_cfg.floodMode);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_FLOODPORT_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_floodPort_get(buf.trunk_cfg.unit, buf.trunk_cfg.trk_gid, &buf.trunk_cfg.floodPort);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

        case RTDRV_TRUNK_TRAFFIC_SEPARATE_GET:
            copy_from_user(&buf.trunk_cfg, user, sizeof(rtdrv_trunkCfg_t));
            ret = rtk_trunk_trafficSeparate_get(buf.trunk_cfg.unit,buf.trunk_cfg.trk_gid, &buf.trunk_cfg.separate);
            copy_to_user(user, &buf.trunk_cfg, sizeof(rtdrv_trunkCfg_t));
            break;

    /** DOT1X **/
        case RTDRV_DOT1X_PORT_BASED_ENABLE_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portBasedEnable_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.enable);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_MAC_BASED_ENABLE_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_macBasedEnable_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.enable);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_FRAME_TO_CPU_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_eapolFrame2CpuEnable_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.enable);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_PORT_BASED_AUTH_STATUS_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portBasedAuthStatus_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.port_auth);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_UNAUTH_PACKET_OPER_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_unauthPacketOper_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.action);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_PORT_UNAUTH_PACKET_OPER_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portUnauthPacketOper_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.action);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_PORT_UNAUTH_TAG_PACKET_OPER_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portUnauthTagPacketOper_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.action);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_PORT_UNAUTH_UNTAG_PACKET_OPER_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portUnauthUntagPacketOper_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.action);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_PORT_BASED_DIRECTION_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portBasedDirection_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.direction);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_MAC_BASED_DIRECTION_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_macBasedDirection_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.direction);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_PORT_GUEST_VLAN_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_portGuestVlan_get(buf.dot1x_cfg.unit, buf.dot1x_cfg.port, &buf.dot1x_cfg.vid);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_GUEST_VLAN_BEHAVIOR_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_guestVlanBehavior_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.gv_behavior);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_GUEST_VLAN_ROUTE_BEHAVIOR_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_guestVlanRouteBehavior_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.rt_action);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_TRAP_PRI_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_trapPri_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.pri);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_TRAP_PRI_ENABLE_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_trapPriEnable_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.enable);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

        case RTDRV_DOT1X_TRAP_ADD_CPUTAG_ENABLE_GET:
            copy_from_user(&buf.dot1x_cfg, user, sizeof(rtdrv_dot1xCfg_t));
            ret = rtk_dot1x_trapAddCPUTagEnable_get(buf.dot1x_cfg.unit, &buf.dot1x_cfg.enable);
            copy_to_user(user, &buf.dot1x_cfg, sizeof(rtdrv_dot1xCfg_t));
            break;

    /** DEBUG **/
        case RTDRV_DEBUG_EN_LOG_GET:
            ret = rt_log_enable_get(&buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_LOGLV_GET:
            ret = rt_log_level_get(&buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_LOGLVMASK_GET:
            ret = rt_log_mask_get(&buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_LOGTYPE_GET:
            ret = rt_log_type_get(&buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_LOGFORMAT_GET:
            ret = rt_log_format_get(&buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_MODMASK_GET:
            ret = rt_log_moduleMask_get(&buf.unit_cfg.data64);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_LOGCFG_GET:
            ret = rt_log_config_get((uint32 *)&buf.log_cfg);
            copy_to_user(user, &buf.log_cfg, sizeof(rtdrv_logCfg_t));
            break;

        case RTDRV_DEBUG_MEM_READ:
            copy_from_user(&buf.reg_cfg, user, sizeof(rtdrv_regCfg_t));
            ret = debug_mem_read(buf.reg_cfg.unit, buf.reg_cfg.reg, &buf.reg_cfg.value);
            copy_to_user(user, &buf.reg_cfg, sizeof(rtdrv_regCfg_t));
            break;

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
        case RTDRV_DEBUG_MIB_DBG_CNTR_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_getDbgCntr(buf.unit_cfg.unit, buf.unit_cfg.mibType, &buf.unit_cfg.cntr);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;
#endif
#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
        case RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_INGRESS_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_getFlowCtrlIgrPortUsedPageCnt(buf.unit_cfg.unit, buf.unit_cfg.port, &buf.unit_cfg.cntr, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_FLOWCTRL_PORT_USED_PAGE_CNT_EGRESS_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_getFlowCtrlEgrPortUsedPageCnt(buf.unit_cfg.unit, buf.unit_cfg.port, &buf.unit_cfg.cntr, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_FLOWCTRL_SYSTEM_USED_PAGE_CNT_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_getFlowCtrlSystemUsedPageCnt(buf.unit_cfg.unit, &buf.unit_cfg.cntr, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_DEBUG_FLOWCTRL_PORT_QUEUE_USED_PAGE_CNT_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = hal_getFlowCtrlPortQueueUsedPageCnt(buf.unit_cfg.unit, buf.unit_cfg.port,
                &buf.unit_cfg.qCntr, &buf.unit_cfg.qMaxCntr);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

	case RTDRV_DEBUG_WATCHDOG_CNT_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
			ret = hal_getWatchdogCnt(buf.unit_cfg.unit, buf.unit_cfg.cntr, &buf.unit_cfg.data);
			copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
			break;
#endif

#if defined(CONFIG_SDK_UART1)
        case RTDRV_UART1_GETC:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = drv_uart_getc(buf.unit_cfg.unit, &buf.unit_cfg.data8, buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;

        case RTDRV_UART1_BAUDRATE_GET:
            copy_from_user(&buf.unit_cfg, user, sizeof(rtdrv_unitCfg_t));
            ret = drv_uart_baudrate_get(buf.unit_cfg.unit, &buf.unit_cfg.data);
            copy_to_user(user, &buf.unit_cfg, sizeof(rtdrv_unitCfg_t));
            break;
#endif

    /** MRIIOR **/
        case RTDRV_MIRROR_ENTRY_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portBased_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirroring_port,
                                           &buf.mirror_cfg.rx_portmask, &buf.mirror_cfg.tx_portmask);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_GROUP_INIT:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_group_init(buf.mirror_cfg.unit,&buf.mirror_cfg.mirrorEntry);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_GROUP_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_group_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.mirrorEntry);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_EGR_MODE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_egrMode_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_PORT_RSPAN_IGR_MODE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portRspanIgrMode_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_PORT_RSPAN_EGR_MODE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_portRspanEgrMode_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_RSPAN_IGR_MODE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanIgrMode_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_RSPAN_EGR_MODE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanEgrMode_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_RSPAN_IGR_TAG_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanIgrTag_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.rspan_igrTag);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_RSPAN_EGR_TAG_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanEgrTag_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.rspan_egrTag);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_RSPAN_TAG_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_rspanTag_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.rspan_tag);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_MIRROR_SEED_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowMirrorSeed_get(buf.mirror_cfg.unit, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_ENABLE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowMirrorSampleEnable_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.enable);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_RATE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowMirrorSampleRate_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_MIRROR_SAMPLE_STAT_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowMirrorSampleStat_get(buf.mirror_cfg.unit, buf.mirror_cfg.mirror_id, &buf.mirror_cfg.sample_stat);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_PORT_SEED_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortSeed_get(buf.mirror_cfg.unit, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_ENABLE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortIgrSampleEnable_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.enable);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_PORT_IGR_SAMPLE_RATE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortIgrSampleRate_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_ENABLE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortEgrSampleEnable_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.enable);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_PORT_EGR_SAMPLE_RATE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowPortEgrSampleRate_get(buf.mirror_cfg.unit, buf.mirror_cfg.port, &buf.mirror_cfg.data);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_ADD_CPU_TAG_ENABLE_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowAddCPUTagEnable_get(buf.mirror_cfg.unit, &buf.mirror_cfg.enable);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

        case RTDRV_MIRROR_SFLOW_SAMPLE_CTRL_GET:
            copy_from_user(&buf.mirror_cfg, user, sizeof(rtdrv_mirrorCfg_t));
            ret = rtk_mirror_sflowSampleCtrl_get(buf.mirror_cfg.unit, &buf.mirror_cfg.sample_ctrl);
            copy_to_user(user, &buf.mirror_cfg, sizeof(rtdrv_mirrorCfg_t));
            break;

    /** FLOWCRTL **/
        case RTDRV_FLOWCTRL_PORT_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_portEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PORT_PAUSE_FORCE_MODE_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_portPauseForceModeEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ACTION_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAction_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, &buf.flowctrl_cfg.pauseOn_action);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PAGENUM_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAllowedPageNum_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, &buf.flowctrl_cfg.data);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTLEN_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAllowedPktLen_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, &buf.flowctrl_cfg.data);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PAUSEON_ALLOWED_PKTNUM_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_pauseOnAllowedPktNum_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, &buf.flowctrl_cfg.data);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_FC_ON_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrSystemPauseThresh_get(buf.flowctrl_cfg.unit, &buf.flowctrl_cfg.thresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_FC_OFF_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrSystemCongestThresh_get(buf.flowctrl_cfg.unit, &buf.flowctrl_cfg.thresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PORT_FC_ON_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPortPauseThresh_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.thresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_GROUP_FC_ON_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPauseThreshGroup_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                      &buf.flowctrl_cfg.thresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PORT_FC_ON_OFF_GROUP_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPortPauseThreshGroupSel_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.grp_idx);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROPMODE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropMode_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.egrDropMode);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROPFORCEMODE_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropForceModeEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_PORT_FC_OFF_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrPortCongestThresh_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                        &buf.flowctrl_cfg.thresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_GROUP_FC_OFF_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrCongestThreshGroup_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                        &buf.flowctrl_cfg.thresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_SYSTEM_DROP_THRESH_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrSystemDropThresh_get(buf.flowctrl_cfg.unit,
                                                        &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROP_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropThresh_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                     &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_THRESH_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortQueueDropThresh_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                     buf.flowctrl_cfg.queue, &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_QUEUE_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortQueueDropEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_QUEUE_DROP_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrQueueDropThresh_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.queue,
                                                      &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_CPU_QUEUE_DROP_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrCpuQueueDropThresh_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.queue,
                                                      &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

        case RTDRV_FLOWCTRL_EGR_PORT_DROP_REFCONGEST_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropRefCongestEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

      case RTDRV_FLOWCTRL_EGR_PORT_GROUP_DROP_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropThreshGroup_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                      &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_EGR_QUEUE_GROUP_DROP_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrQueueDropThreshGroup_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                      buf.flowctrl_cfg.queue, &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_DROP_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueueDropEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port, buf.flowctrl_cfg.queue,
                                                      &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_FC_ON_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueuePauseThreshGroup_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx,
                                                        buf.flowctrl_cfg.queue, &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_FC_ON_DROP_GROUP_SEL_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueuePauseDropThreshGroupSel_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.grp_idx);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_EGR_PORT_QUEUE_DROP_GROUP_SEL_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortQueueDropThreshGroupSel_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.grp_idx);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_EGR_QUEUE_DROP_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrQueueDropEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                        buf.flowctrl_cfg.queue, &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_EGR_PORT_DROP_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_egrPortDropEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;
      case RTDRV_FLOWCTRL_IGR_QUEUE_GROUP_DROP_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_igrQueueDropThreshGroup_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.grp_idx, buf.flowctrl_cfg.queue,
                                                      &buf.flowctrl_cfg.dropThresh);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

      case RTDRV_FLOWCTRL_PORT_HOL_TRAFFIC_DROP_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_portHolTrafficDropEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.port,
                                                      &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

      case RTDRV_FLOWCTRL_HOL_TRAFFIC_TYPE_DROP_ENABLE_GET:
            copy_from_user(&buf.flowctrl_cfg, user, sizeof(rtdrv_flowctrlCfg_t));
            ret = rtk_flowctrl_holTrafficTypeDropEnable_get(buf.flowctrl_cfg.unit, buf.flowctrl_cfg.data,
                                                      &buf.flowctrl_cfg.enable);
            copy_to_user(user, &buf.flowctrl_cfg, sizeof(rtdrv_flowctrlCfg_t));
            break;

    /** RATE **/
        case RTDRV_RATE_IGR_ENABLE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlEnable_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_INCLUDE_IFG_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlIncludeIfg_get(buf.rate_cfg.unit, &buf.rate_cfg.ifg_include);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_PORT_IGR_INCLUDE_IFG_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portIgrBandwidthCtrlIncludeIfg_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.ifg_include);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_ENABLE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthFlowctrlEnable_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.ifg_include);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_THRESH_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthFlowctrlThresh_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.rate_thresh);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_FLOWCTRL_LOW_THRESH_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthLowThresh_get(buf.rate_cfg.unit, &buf.rate_cfg.thresh);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_PORT_IGR_BANDWIDTH_CTRL_EXCEED_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portIgrBandwidthCtrlExceed_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.isExceed);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_PORT_IGR_BANDWIDTH_FLOWCTRL_HIGH_THRESH_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portIgrBandwidthHighThresh_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.thresh);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_BURST_SIZE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBwCtrlBurstSize_get(buf.rate_cfg.unit, &buf.rate_cfg.thresh);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_CTRL_FPENTRY_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlFPEntry_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.index, &buf.rate_cfg.fpEntry);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_BANDWIDTH_CTRL_BYPASS_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlBypass_get(buf.rate_cfg.unit, buf.rate_cfg.igrBypassType, &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_RATE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrBandwidthCtrlRate_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.rate);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_ENABLE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrBandwidthCtrlEnable_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_INCLUDE_IFG_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrBandwidthCtrlIncludeIfg_get(buf.rate_cfg.unit, &buf.rate_cfg.ifg_include);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_CPU_EGR_BANDWIDTH_CTRL_RATE_MODE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_cpuEgrBandwidthCtrlRateMode_get(buf.rate_cfg.unit, &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_PORT_EGR_INCLUDE_IFG_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portEgrBandwidthCtrlIncludeIfg_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.ifg_include);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_PORT_EGR_BURST_SIZE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portEgrBandwidthCtrlBurstSize_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_QUEUE_BWCTRL_ENABLE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueBwCtrlEnable_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_QUEUE_BWCTRL_RATE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueBwCtrlRate_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, &buf.rate_cfg.rate);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_PORT_QUEUE_BWCTRL_BURST_SIZE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrPortQueueBwCtrlBurstSize_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_QUEUE_BWCTRL_BURST_SIZE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueBwCtrlBurstSize_get(buf.rate_cfg.unit, &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_QUEUE_FIX_BW_ENABLE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrQueueFixedBandwidthEnable_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_EGR_RATE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_egrBandwidthCtrlRate_get(buf.rate_cfg.unit, buf.rate_cfg.port, &buf.rate_cfg.rate);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STROM_CTRL_RATE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlRate_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                &buf.rate_cfg.rate);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STROM_CTRL_PROTO_RATE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlProtoRate_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_proto_type,
                                                &buf.rate_cfg.rate);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_ENABLE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlEnable_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_PORT_STORM_CONTROL_RATE_MODE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_portStormControlRateMode_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_RATE_MODE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlRateMode_get(buf.rate_cfg.unit, &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_BURST_RATE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlBurstRate_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                &buf.rate_cfg.rate);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_INCLUDE_IFG_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlIncludeIfg_get(buf.rate_cfg.unit,
                                                &buf.rate_cfg.ifg_include);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_EXCEED_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlExceed_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                &buf.rate_cfg.isExceed);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_PROTO_EXCEED_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlProtoExceed_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_proto_type,
                                                &buf.rate_cfg.isExceed);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_REFRESH_MODE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlRefreshMode_get(buf.rate_cfg.unit,
                                                &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_TYPE_SEL_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlTypeSel_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.storm_type,
                                                &buf.rate_cfg.storm_sel);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_BYPASS_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlBypass_get(buf.rate_cfg.unit, buf.rate_cfg.stormBypassType,
                                                &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_STORM_CONTROL_BURST_SIZE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_stormControlBurstSize_get(buf.rate_cfg.unit, buf.rate_cfg.storm_type,
                                                &buf.rate_cfg.data);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_QUEUE_BWCTRL_ENABLE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrQueueBwCtrlEnable_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, &buf.rate_cfg.enable);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_QUEUE_BWCTRL_RATE_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrQueueBwCtrlRate_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, &buf.rate_cfg.rate);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;

        case RTDRV_RATE_IGR_QUEUE_BWCTRL_EXCEED_GET:
            copy_from_user(&buf.rate_cfg, user, sizeof(rtdrv_rateCfg_t));
            ret = rtk_rate_igrQueueBwCtrlExceed_get(buf.rate_cfg.unit, buf.rate_cfg.port, buf.rate_cfg.queue, &buf.rate_cfg.isExceed);
            copy_to_user(user, &buf.rate_cfg, sizeof(rtdrv_rateCfg_t));
            break;


    /** SVLAN **/
        case RTDRV_SVLAN_MEMBER_GET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_memberPort_get(buf.svlan_cfg.unit, buf.svlan_cfg.svid, &buf.svlan_cfg.svlan_portmask);
            copy_to_user(user, &buf.svlan_cfg, sizeof(rtdrv_svlanCfg_t));
            break;

        case RTDRV_SVLAN_MEMBER_ENTRY_GET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_memberPortEntry_get(buf.svlan_cfg.unit, buf.svlan_cfg.svid_idx, &buf.svlan_cfg.svid,
                                                &buf.svlan_cfg.svlan_portmask);
            copy_to_user(user, &buf.svlan_cfg, sizeof(rtdrv_svlanCfg_t));
            break;

        case RTDRV_SVLAN_VALID_MEMBER_ENTRY_GETNEXT:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_nextValidMemberPortEntry_get(buf.svlan_cfg.unit, &buf.svlan_cfg.svid_idx,
                                                         &buf.svlan_cfg.svid, &buf.svlan_cfg.svlan_portmask);
            copy_to_user(user, &buf.svlan_cfg, sizeof(rtdrv_svlanCfg_t));
            break;

        case RTDRV_SVLAN_TPID_ENTRY_GET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_tpidEntry_get(buf.svlan_cfg.unit, buf.svlan_cfg.svid_idx, &buf.svlan_cfg.svlan_tag_id);
            copy_to_user(user, &buf.svlan_cfg, sizeof(rtdrv_svlanCfg_t));
            break;

        case RTDRV_SVLAN_PORT_SVID_GET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_portSvid_get(buf.svlan_cfg.unit, buf.svlan_cfg.port, &buf.svlan_cfg.svid);
            copy_to_user(user, &buf.svlan_cfg, sizeof(rtdrv_svlanCfg_t));
            break;

        case RTDRV_SVLAN_SERVICE_PORT_GET:
            copy_from_user(&buf.svlan_cfg, user, sizeof(rtdrv_svlanCfg_t));
            ret = rtk_svlan_servicePort_get(buf.svlan_cfg.unit, &buf.svlan_cfg.svlan_portmask);
            copy_to_user(user, &buf.svlan_cfg, sizeof(rtdrv_svlanCfg_t));
            break;

    /** SWITCH **/
        case RTDRV_SWITCH_CPU_MAX_PKTLEN_GET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_cpuMaxPktLen_get(buf.switch_cfg.unit, buf.switch_cfg.dir, &buf.switch_cfg.len);
            copy_to_user(user, &buf.switch_cfg, sizeof(rtdrv_switchCfg_t));
            break;

        case RTDRV_SWITCH_MAX_PKTLEN_GET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_maxPktLen_get(buf.switch_cfg.unit, &buf.switch_cfg.len);
            copy_to_user(user, &buf.switch_cfg, sizeof(rtdrv_switchCfg_t));
            break;

        case RTDRV_SWITCH_MAX_PKTLEN_LINK_SPEED_GET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_maxPktLenLinkSpeed_get(buf.switch_cfg.unit, buf.switch_cfg.speed, &buf.switch_cfg.maxLen);
            copy_to_user(user, &buf.switch_cfg, sizeof(rtdrv_switchCfg_t));
            break;

        case RTDRV_SWITCH_MAX_PKTLEN_TAGLENCNT_GET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_maxPktLenTagLenCntIncEnable_get(buf.switch_cfg.unit, &buf.switch_cfg.enable);
            copy_to_user(user, &buf.switch_cfg, sizeof(rtdrv_switchCfg_t));
            break;

        case RTDRV_SWITCH_DEVICE_INFO_GET:
            copy_from_user(&buf.switch_cfg, user, sizeof(rtdrv_switchCfg_t));
            ret = rtk_switch_deviceInfo_get(buf.switch_cfg.unit, &buf.switch_cfg.devInfo);
            copy_to_user(user, &buf.switch_cfg, sizeof(rtdrv_switchCfg_t));
            break;

        case RTDRV_SWITCH_PORTMAXPKTLEN_GET :
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_portMaxPktLen_get(buf.switch_cfgParam.unit, buf.switch_cfgParam.port, &buf.switch_cfgParam.maxLen);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

        case RTDRV_SWITCH_PORTSNAPMODE_GET :
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_portSnapMode_get(buf.switch_cfgParam.unit, buf.switch_cfgParam.port, &buf.switch_cfgParam.snapMode);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

        case RTDRV_SWITCH_CHKSUMFAILACTION_GET :
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_chksumFailAction_get(buf.switch_cfgParam.unit, buf.switch_cfgParam.port,
                buf.switch_cfgParam.failType, &buf.switch_cfgParam.action);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

        case RTDRV_SWITCH_RECALCCRCENABLE_GET :
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_recalcCRCEnable_get(buf.switch_cfgParam.unit, buf.switch_cfgParam.port,
                &buf.switch_cfgParam.enable);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

        case RTDRV_SWITCH_MGMTVLANID_GET :
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_mgmtVlanId_get(buf.switch_cfgParam.unit, &buf.switch_cfgParam.mgmIvid);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

        case RTDRV_SWITCH_OUTERMGMTVLANID_GET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_outerMgmtVlanId_get(buf.switch_cfgParam.unit, &buf.switch_cfgParam.mgmOvid);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

        case RTDRV_SWITCH_MGMTMACADDR_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_mgmtMacAddr_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.mac);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_IPV4ADDR_GET :
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_IPv4Addr_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.ipv4Addr);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_IPV6ADDR_GET :
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_IPv6Addr_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.ipv6Addr);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_DELAY_ENABLE_GET :
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_hwInterfaceDelayEnable_get(buf.switch_cfgInfo.unit, buf.switch_cfgInfo.type, &buf.switch_cfgInfo.enable);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_PKT2CPU_FORMAT_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_pkt2CpuFormat_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.data);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_PKT2CPUTYPEFORMAT_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_pkt2CpuTypeFormat_get(buf.switch_cfgInfo.unit,
                    buf.switch_cfgInfo.type, &buf.switch_cfgInfo.format);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_PPPOE_PASS_THROUGH_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_pppoePassthrough_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.enable);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_WATCHDOG_ENABLE_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = drv_watchdog_enable_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.enable);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_WATCHDOG_MODE_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = drv_watchdog_mode_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.data);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_WATCHDOG_SCALE_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = drv_watchdog_scale_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.data);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_SOFTWARE_RESET_COUNTER_GET:
            copy_from_user(&buf.switch_cfgInfo, user, sizeof(rtdrv_switchCfgInfo_t));
            ret = rtk_switch_softwareResetCounter_get(buf.switch_cfgInfo.unit, &buf.switch_cfgInfo.data);
            copy_to_user(user, &buf.switch_cfgInfo, sizeof(rtdrv_switchCfgInfo_t));
            break;

        case RTDRV_SWITCH_CPU_PKT_TRUNCATE_EN_GET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_cpuPktTruncateEnable_get(buf.switch_cfgParam.unit, &buf.switch_cfgParam.enable);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

        case RTDRV_SWITCH_CPU_PKT_TRUNCATE_LEN_GET:
            copy_from_user(&buf.switch_cfgParam, user, sizeof(rtdrv_switchCfgParam_t));
            ret = rtk_switch_cpuPktTruncateLen_get(buf.switch_cfgParam.unit, &buf.switch_cfgParam.maxLen);
            copy_to_user(user, &buf.switch_cfgParam, sizeof(rtdrv_switchCfgParam_t));
            break;

    /** NIC **/
        case RTDRV_NIC_DEBUG_GET:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_dbg_get(buf.nic_cfg.unit, &buf.nic_cfg.flags);
            copy_to_user(user, &buf.nic_cfg, sizeof(rtdrv_nicCfg_t));
            break;

        case RTDRV_NIC_RX_STATUS_GET:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_rx_status_get(buf.nic_cfg.unit, &buf.nic_cfg.rx_status);
            copy_to_user(user, &buf.nic_cfg, sizeof(rtdrv_nicCfg_t));
            break;

#if defined(CONFIG_SDK_SOFTWARE_RX_CPU_TAG)
        case RTDRV_NIC_CPU_ENTRY_GET:
            copy_from_user(&buf.nic_cfg, user, sizeof(rtdrv_nicCfg_t));
            ret = drv_nic_pieCpuEntry_get(buf.nic_cfg.unit, buf.nic_cfg.index,
                                          &buf.nic_cfg.cpu_entry);
            copy_to_user(user, &buf.nic_cfg, sizeof(rtdrv_nicCfg_t));
            break;
#endif

#if (defined(CONFIG_SDK_DRIVER_TEST) || defined(CONFIG_SDK_DRIVER_TEST_MODULE))
    /** SDK **/
        case RTDRV_SDK_TEST_MODE_GET:
            copy_from_user(&buf.sdk_cfg, user, sizeof(rtdrv_sdkCfg_t));
            ret = sdktest_mode_get(&buf.sdk_cfg.mode);
            copy_to_user(user, &buf.sdk_cfg, sizeof(rtdrv_sdkCfg_t));
            break;

#endif

    /** EEE **/
        case RTDRV_EEE_PORT_ENABLE_GET:
            copy_from_user(&buf.eee_cfg, user, sizeof(rtdrv_eeeCfg_t));
            ret = rtk_eee_portEnable_get(buf.eee_cfg.unit, buf.eee_cfg.port, &buf.eee_cfg.enable);
            copy_to_user(user, &buf.eee_cfg, sizeof(rtdrv_eeeCfg_t));
            break;

        case RTDRV_EEE_PORT_STATE_GET:
            copy_from_user(&buf.eee_cfg, user, sizeof(rtdrv_eeeCfg_t));
            ret = rtk_eee_portState_get(buf.eee_cfg.unit, buf.eee_cfg.port, &buf.eee_cfg.enable);
            copy_to_user(user, &buf.eee_cfg, sizeof(rtdrv_eeeCfg_t));
            break;

        case RTDRV_EEEP_PORT_ENABLE_GET:
            copy_from_user(&buf.eee_cfg, user, sizeof(rtdrv_eeeCfg_t));
            ret = rtk_eeep_portEnable_get(buf.eee_cfg.unit, buf.eee_cfg.port, &buf.eee_cfg.enable);
            copy_to_user(user, &buf.eee_cfg, sizeof(rtdrv_eeeCfg_t));
            break;

    /** SEC **/
        case RTDRV_SEC_PORT_ATTACK_PREVENT_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portAttackPrevent_get(buf.sec_cfg.unit, buf.sec_cfg.port, buf.sec_cfg.attack_type
                                        , &buf.sec_cfg.action);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_PORT_MIN_IPV6_FRAG_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portMinIPv6FragLen_get(buf.sec_cfg.unit, buf.sec_cfg.port, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_PORT_MAX_PING_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portMaxPingLen_get(buf.sec_cfg.unit, buf.sec_cfg.port, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_PORT_MIN_TCP_HDR_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portMinTCPHdrLen_get(buf.sec_cfg.unit, buf.sec_cfg.port, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_PORT_SMURF_NETMASK_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portSmurfNetmaskLen_get(buf.sec_cfg.unit, buf.sec_cfg.port, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_PORT_ATTACK_PREVENT_ENABLE_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_portAttackPreventEnable_get(buf.sec_cfg.unit, buf.sec_cfg.port, &buf.sec_cfg.enable);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_ATTACK_PREVENT_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_attackPreventAction_get(buf.sec_cfg.unit, buf.sec_cfg.attack_type
                                        , &buf.sec_cfg.action);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_MIN_IPV6_FRAG_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_minIPv6FragLen_get(buf.sec_cfg.unit, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_MAX_PING_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_maxPingLen_get(buf.sec_cfg.unit, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_MIN_TCP_HDR_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_minTCPHdrLen_get(buf.sec_cfg.unit, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

        case RTDRV_SEC_SMURF_NETMASK_LEN_GET:
            copy_from_user(&buf.sec_cfg, user, sizeof(rtdrv_secCfg_t));
            ret = rtk_sec_smurfNetmaskLen_get(buf.sec_cfg.unit, &buf.sec_cfg.data);
            copy_to_user(user, &buf.sec_cfg, sizeof(rtdrv_secCfg_t));
            break;

    /** LED **/
        case RTDRV_LED_SYS_ENABLE_GET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_sysEnable_get(buf.led_cfg.unit, buf.led_cfg.type, &buf.led_cfg.enable);
            copy_to_user(user, &buf.led_cfg, sizeof(rtdrv_ledCfg_t));
            break;

        case RTDRV_LED_PORT_ENABLE_GET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_portEnable_get(buf.led_cfg.unit, buf.led_cfg.port, &buf.led_cfg.enable);
            copy_to_user(user, &buf.led_cfg, sizeof(rtdrv_ledCfg_t));
            break;

        case RTDRV_LED_PORTLEDENTITYSWCTRLENABLE_GET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_portLedEntitySwCtrlEnable_get(buf.led_cfg.unit,
                    buf.led_cfg.port, buf.led_cfg.entity, &buf.led_cfg.enable);
            copy_to_user(user, &buf.led_cfg, sizeof(rtdrv_ledCfg_t));
            break;

        case RTDRV_LED_PORTLEDENTITYSWCTRLMODE_GET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_portLedEntitySwCtrlMode_get(buf.led_cfg.unit,
                    buf.led_cfg.port, buf.led_cfg.entity, buf.led_cfg.media,
                    &buf.led_cfg.mode);
            copy_to_user(user, &buf.led_cfg, sizeof(rtdrv_ledCfg_t));
            break;

        case RTDRV_LED_SYSMODE_GET:
            copy_from_user(&buf.led_cfg, user, sizeof(rtdrv_ledCfg_t));
            ret = rtk_led_sysMode_get(buf.led_cfg.unit, &buf.led_cfg.mode);
            copy_to_user(user, &buf.led_cfg, sizeof(rtdrv_ledCfg_t));
            break;

        /* MPLS */
        case RTDRV_MPLS_TTLINHERIT_GET:
            copy_from_user(&buf.mpls_cfg, user, sizeof(rtdrv_mplsCfg_t));
            ret = rtk_mpls_ttlInherit_get(buf.mpls_cfg.unit, &buf.mpls_cfg.u.inherit);
            copy_to_user(user, &buf.mpls_cfg, sizeof(rtdrv_mplsCfg_t));
            break;
        case RTDRV_MPLS_ENCAP_GET:
            copy_from_user(&buf.mpls_cfg, user, sizeof(rtdrv_mplsCfg_t));
            ret = rtk_mpls_encap_get(buf.mpls_cfg.unit, buf.mpls_cfg.lib_idx,
                    &buf.mpls_cfg.u.encap_info);
            copy_to_user(user, &buf.mpls_cfg, sizeof(rtdrv_mplsCfg_t));
            break;

        case RTDRV_MPLS_ENABLE_GET:
            copy_from_user(&buf.mpls_cfg, user, sizeof(rtdrv_mplsCfg_t));
            ret = rtk_mpls_enable_get(buf.mpls_cfg.unit, &buf.mpls_cfg.enable);
            copy_to_user(user, &buf.mpls_cfg, sizeof(rtdrv_mplsCfg_t));
            break;

#if defined(CONFIG_SDK_RTL8231)
    /** RTL8231 **/
        case RTDRV_RTL8231_I2C_READ:
            copy_from_user(&buf.rtl8231_cfg, user, sizeof(rtdrv_rtl8231Cfg_t));
            ret = drv_rtl8231_i2c_read(buf.rtl8231_cfg.unit, buf.rtl8231_cfg.phyId_or_slaveAddr, buf.rtl8231_cfg.reg_addr, &buf.rtl8231_cfg.data);
            copy_to_user(user, &buf.rtl8231_cfg, sizeof(rtdrv_rtl8231Cfg_t));
            break;

        case RTDRV_RTL8231_MDC_READ:
            copy_from_user(&buf.rtl8231_cfg, user, sizeof(rtdrv_rtl8231Cfg_t));
            ret = drv_rtl8231_mdc_read(buf.rtl8231_cfg.unit, buf.rtl8231_cfg.phyId_or_slaveAddr, buf.rtl8231_cfg.page, buf.rtl8231_cfg.reg_addr, &buf.rtl8231_cfg.data);
            copy_to_user(user, &buf.rtl8231_cfg, sizeof(rtdrv_rtl8231Cfg_t));
            break;

        case RTDRV_EXTGPIO_DEV_READY_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_devReady_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_DEV_ENABLE_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_devEnable_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_SYNC_ENABLE_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_syncEnable_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_SYNC_STATUS_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_syncStatus_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_DATABIT_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_dataBit_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.gpioId, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_REG_READ:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_reg_read(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.reg, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_DEV_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_dev_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, &buf.extGpio_cfg.extGpio_devConfData);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_PIN_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_pin_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.gpioId, &buf.extGpio_cfg.extGpio_confData);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_DIRECTION_GET:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_direction_get(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.gpioId, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;

        case RTDRV_EXTGPIO_I2C_READ:
            copy_from_user(&buf.extGpio_cfg, user, sizeof(rtdrv_extGpioCfg_t));
            ret = drv_extGpio_i2c_read(buf.extGpio_cfg.unit, buf.extGpio_cfg.dev, buf.extGpio_cfg.reg, &buf.extGpio_cfg.data);
            copy_to_user(user, &buf.extGpio_cfg, sizeof(rtdrv_extGpioCfg_t));
            break;
#endif

        case RTDRV_GPIO_DATABIT_GET:
            copy_from_user(&buf.gpio_cfg, user, sizeof(rtdrv_gpioCfg_t));
            ret = drv_gpio_dataBit_get(buf.gpio_cfg.gpioId, &buf.gpio_cfg.data);
            copy_to_user(user, &buf.gpio_cfg, sizeof(rtdrv_gpioCfg_t));
            break;

        case RTDRV_GENCTRL_GPIO_DATABIT_GET:
            copy_from_user(&buf.gpio_cfg, user, sizeof(rtdrv_gpioCfg_t));
            ret = drv_generalCtrlGPIO_dataBit_get(buf.genCtrlGPIO_cfg.unit, buf.genCtrlGPIO_cfg.dev, buf.genCtrlGPIO_cfg.gpioId, &buf.genCtrlGPIO_cfg.data);
			if(ret != RT_ERR_OK)
				printk("\nGet drv_generalCtrlGPIO_dataBit_get() ERROR = %d\n",ret);
            copy_to_user(user, &buf.gpio_cfg, sizeof(rtdrv_gpioCfg_t));
            break;

    /**SMI**/
#if defined(CONFIG_SDK_RTL8231)
        case RTDRV_EXT_SMI_READ:
            copy_from_user(&buf.extSmi_cfg, user, sizeof(rtdrv_extSmiCfg_t));
            ret = drv_extSmi_read(buf.extSmi_cfg.unit, buf.extSmi_cfg.periferal, buf.extSmi_cfg.addrs, &buf.extSmi_cfg.rdata);
            copy_to_user(user, &buf.extSmi_cfg, sizeof(rtdrv_extSmiCfg_t));
            break;
#endif

        case RTDRV_SMI_GROUP_GET:
            copy_from_user(&buf.smi_cfg, user, sizeof(rtdrv_smiCfg_t));
            ret = drv_smi_group_get(&buf.smi_cfg.portSCK, &buf.smi_cfg.pinSCK, &buf.smi_cfg.portSDA, &buf.smi_cfg.pinSDA, buf.smi_cfg.dev);
            copy_to_user(user, &buf.smi_cfg, sizeof(rtdrv_smiCfg_t));
            break;

        case RTDRV_SMI_TYPE_GET:
            copy_from_user(&buf.smi_cfg, user, sizeof(rtdrv_smiCfg_t));
            ret = drv_smi_type_get(&buf.smi_cfg.type, &buf.smi_cfg.chipid,&buf.smi_cfg.delay, buf.smi_cfg.dev);
            copy_to_user(user, &buf.smi_cfg, sizeof(rtdrv_smiCfg_t));
            break;

        case RTDRV_SMI_READ:
            copy_from_user(&buf.smi_cfg, user, sizeof(rtdrv_smiCfg_t));
            ret = drv_smi_read(buf.smi_cfg.addrs, &buf.smi_cfg.rdata, buf.smi_cfg.dev);
            copy_to_user(user, &buf.smi_cfg, sizeof(rtdrv_smiCfg_t));
            break;

        /* DIAG */
        case RTDRV_DIAG_MAC_REMOTE_LOOPBACK_GET:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_portMacRemoteLoopbackEnable_get(buf.diag_cfg.unit, buf.diag_cfg.port, &buf.diag_cfg.enable);
            copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
            break;
        case RTDRV_DIAG_MAC_LOCAL_LOOPBACK_GET:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_portMacLocalLoopbackEnable_get(buf.diag_cfg.unit, buf.diag_cfg.port, &buf.diag_cfg.enable);
            copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
            break;
        case RTDRV_DIAG_RTCTRESULT_GET:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_portRtctResult_get(buf.diag_cfg.unit, buf.diag_cfg.port, &buf.diag_cfg.rtctResult);
            copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
            break;
        case RTDRV_DIAG_TABLE_WHOLE_READ:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_table_whole_read((uint32)buf.diag_cfg.unit, (uint32)buf.diag_cfg.target_index);
			 copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
            break;
         case RTDRV_DIAG_REG_WHOLE_READ:
            copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
            ret = rtk_diag_reg_whole_read((uint32)buf.diag_cfg.unit);
			 copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
            break;
    	 case RTDRV_DIAG_PERIPHERAL_REG_READ:
		     copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
			 ret = rtk_diag_peripheral_register_dump((uint32)buf.diag_cfg.unit);
			 copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
			 break;
    	 case RTDRV_DIAG_PHY_REG_READ:
		     copy_from_user(&buf.diag_cfg, user, sizeof(rtdrv_diagCfg_t));
			 ret = rtk_diag_phy_reg_whole_read((uint32)buf.diag_cfg.unit);
			 copy_to_user(user, &buf.diag_cfg, sizeof(rtdrv_diagCfg_t));
			 break;

        default:
            break;
    }

	return ret;
}

/* Function Name:
 *      rtdrv_init
 * Description:
 *      Init driver and register netfilter socket option
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 * Note:
 *      None
 */
static int32 __init rtdrv_init(void)
{
	/* register netfilter socket option */
    if (nf_register_sockopt(&rtdrv_sockopts))
    {
        osal_printf("[%s]: nf_register_sockopt failed.\n", __FUNCTION__);
        return RT_ERR_FAILED;
    }

#if defined(__RTDRV_MODULE__)
    osal_printf(KERN_INFO"Init RTDRV Driver Module....OK\n");
#endif

#if defined(CONFIG_SDK_APP_DIAG_EXT)
    /* register netfilter socket option */
    if (nf_register_sockopt(&rtdrv_ext_sockopts))
    {
        osal_printf("[%s]: nf_register_sockopt failed.\n", __FUNCTION__);
        return RT_ERR_FAILED;
    }

#if defined(CONFIG_SDK_RTL8390)
	if (RT_ERR_OK == drv_nic_pkt_alloc(0, 1518, 0, &pDiagExtPacket))
	{
		pDiagExtPacket->length = 1518;
        pDiagExtPacket->tail = pDiagExtPacket->data + pDiagExtPacket->length;
		pDiagExtPacket->end = pDiagExtPacket->data + pDiagExtPacket->length;
	}
	else
	{
        osal_printf("[%s]: Alloc packet failed.\n", __FUNCTION__);
        return RT_ERR_FAILED;
	}
#endif
#if defined(__RTDRV_MODULE__)
    osal_printf(KERN_INFO"Init RTDRV EXT Driver Module....OK\n");
#endif
#endif /* CONFIG_SDK_APP_DIAG_EXT */

    return RT_ERR_OK;
}

/* Function Name:
 *      rtdrv_exit
 * Description:
 *      Exit driver and unregister netfilter socket option
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None
 */
static void __exit rtdrv_exit(void)
{
    nf_unregister_sockopt(&rtdrv_sockopts);

#if defined(__RTDRV_MODULE__)
    osal_printf(KERN_INFO"Exit RTDRV Driver Module....OK\n");
#endif

#if defined(CONFIG_SDK_APP_DIAG_EXT)
    nf_unregister_sockopt(&rtdrv_ext_sockopts);

#if defined(__RTDRV_MODULE__)
    osal_printf(KERN_INFO"Exit RTDRV EXT Driver Module....OK\n");
#endif

#endif /* CONFIG_SDK_APP_DIAG_EXT */
}

struct nf_sockopt_ops rtdrv_sockopts = {
	{ NULL, NULL }, PF_INET,
	RTDRV_BASE_CTL, RTDRV_SET_MAX+1, do_rtdrv_set_ctl, NULL,
	RTDRV_BASE_CTL, RTDRV_GET_MAX+1, do_rtdrv_get_ctl, NULL
};

module_init(rtdrv_init);
module_exit(rtdrv_exit);

MODULE_DESCRIPTION ("Switch SDK User/Kernel Driver Module");
MODULE_LICENSE("GPL");

