/*
 * Copyright(c) Realtek Semiconductor Corporation, 2009
 * All rights reserved.
 *
 * $Revision: 21568 $
 * $Date: 2011-08-26 19:16:38 +0800 (Fri, 26 Aug 2011) $
 *
 * Purpose : Definition those public OAM routing APIs and its data type in the SDK.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) OAM (802.3ah) configuration
 *           2) CFM (802.1ag) configuration
 */

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_bitop.h>
#include <common/util/rt_util.h>
#include <common/debug/rt_log.h>
#include <osal/sem.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/time.h>
#include <hal/chipdef/allmem.h>
#include <hal/chipdef/esw/rtk_esw_table_struct.h>
#include <hal/mac/reg.h>
#include <hal/mac/mem.h>
#include <hal/chipdef/allreg.h>
#include <hal/chipdef/esw/rtk_esw_reg_struct.h>
#include <hal/common/miim.h>
#include <hal/common/halctrl.h>
#include <dal/esw/dal_esw_oam.h>
#include <dal/esw/dal_esw_vlan.h>
#include <rtk/oam.h>
#include <rtk/default.h>

#define CFM_MAX_IDX             1
#define DAL_ESW_MEPID_ENTRY     16
#define DAL_MAX_CCM_PKTLEN      144

/*
 * Symbol Definition
 */
enum dal_esw_spg_action_e
{
    SPG_START_TX = 1,
    SPG_STOP_TX,
    SPG_PAUSE_TX,
    SPG_ACTION_END,
};

enum dal_esw_ps_e
{
    PS_BLOCKED = 1,
    PS_UPED,
    PS_END,
};

enum dal_esw_oamtf_act_e
{
    OAMTF_FOLLOW_OAMPDU,
    OAMTF_LOOPBACK,
    OAMTF_DROP,
    OAMTF_END,
};

enum dal_esw_oampduparser_act_e
{
    OAMPARSER_FORWARD,
    OAMPARSER_LOOPBACK,
    OAMPARSER_DROP,
    OAMPARSER_TRAP2CPU,
    OAMPARSER_END,
};

enum dal_tx_pkt_type_e
{
    SPG_PKT_NORMAL_ETHII = 0,
    SPG_PKT_OAMPDU_TEST,
    SPG_PKT_CCM,
    SPG_PKT_END,
};

typedef struct dal_esw_mepid_s
{
    uint32 mepid;
    uint32 valid;
} dal_esw_mepid_t;

typedef struct dal_esw_mepid_info_s
{
    dal_esw_mepid_t     mepid[DAL_ESW_MEPID_ENTRY];
} dal_esw_mepid_info_t;

/* 
 * Data Declaration 
 */
static uint32               oam_init[RTK_MAX_NUM_OF_UNIT] = {INIT_NOT_COMPLETED};
static osal_mutex_t         oam_sem[RTK_MAX_NUM_OF_UNIT];
static dal_esw_mepid_info_t *pMepid_info[RTK_MAX_NUM_OF_UNIT];

/*
 * Macro Definition
 */
/* vlan semaphore handling */
#define OAM_SEM_LOCK(unit)    \
do {\
    if (osal_sem_mutex_take(oam_sem[unit], OSAL_SEM_WAIT_FOREVER) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_LOCK_FAILED, (MOD_DAL|MOD_OAM),"semaphore lock failed");\
        return RT_ERR_SEM_LOCK_FAILED;\
    }\
} while(0)
#define OAM_SEM_UNLOCK(unit)   \
do {\
    if (osal_sem_mutex_give(oam_sem[unit]) != RT_ERR_OK)\
    {\
        RT_ERR(RT_ERR_SEM_UNLOCK_FAILED, (MOD_DAL|MOD_OAM),"semaphore unlock failed");\
        return RT_ERR_SEM_UNLOCK_FAILED;\
    }\
} while(0)


/*
 * Macro Declaration
 */


/*
 * Function Declaration
 */

/* Function Name:
 *      dal_esw_oam_init
 * Description:
 *      Initialize OAM module of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 * Note:
 *      Must initialize OAM module before calling any OAM APIs.
 */
int32
dal_esw_oam_init(uint32 unit)
{
    oam_init[unit] = INIT_NOT_COMPLETED;

    /* create semaphore */
    oam_sem[unit] = osal_sem_mutex_create();
    if (0 == oam_sem[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_OAM), "semaphore create failed");
        return RT_ERR_FAILED;
    }

    pMepid_info[unit] = (dal_esw_mepid_info_t *)osal_alloc(sizeof(dal_esw_mepid_info_t));
    if (NULL == pMepid_info[unit])
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_PORT), "memory allocate failed");
        return RT_ERR_FAILED;
    }        
    
    osal_memset(pMepid_info[unit], 0, sizeof(dal_esw_mepid_info_t));
        
    /* set init flag to complete init */
    oam_init[unit] = INIT_COMPLETED;
    
    return RT_ERR_OK;
} /* end of dal_ssw_oam_init */

/* Module Name    : OAM               */
/* Sub-module Name: OAM configuration */

/* Function Name:
 *      dal_esw_oam_oamCounter_get
 * Description:
 *      Get number of received OAM PDU on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pNumber - number of received OAM PDU
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_oamCounter_get(uint32 unit, rtk_port_t port, uint32 *pNumber)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pNumber), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_OAMPDU_PACKET_COUNTERr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAMPDUCNTf, pNumber) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pNumber=%d", *pNumber);
    
    return RT_ERR_OK;
}/*end of dal_esw_oam_oamCounter_get*/

/* Function Name:
 *      dal_esw_oam_txTestFrame_start
 * Description:
 *      Start transmitting OAM test frame.
 * Input:
 *      unit          - unit id
 *      pTestFrameCfg - configuration of test frame transmitting
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_txTestFrame_start(uint32 unit, rtk_oam_testFrameCfg_t *pTestFrameCfg)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pTestFrameCfg), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pTestFrameCfg->continuousTx=%d, \
        pTestFrameCfg->frameCount=%d, pTestFrameCfg->frameInterval=%d, pTestFrameCfg->length=%d, \
        pTestFrameCfg->txPortmask=0x%x", pTestFrameCfg->continuousTx,
        pTestFrameCfg->frameCount, pTestFrameCfg->frameInterval, pTestFrameCfg->length,
        pTestFrameCfg->txPortmask.bits[0]);

    OAM_SEM_LOCK(unit);

    val = ENABLED;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_EN_SPG_MODf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*set test pkt*/
    val = SPG_PKT_NORMAL_ETHII;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_PKT_TYPEf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    /*First Stop SPG*/
    val = SPG_STOP_TX;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_CMDf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    
    /* Set value to CHIP*/
    if(pTestFrameCfg->continuousTx)
    {
        val = 0;
        /*Set SPG Continue mode*/
        if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_PKT_CNT_MODEf, &val) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }
    }
    else
    {
        /*Set SPG Pkt Count mode*/
        val = 1;
        if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_PKT_CNT_MODEf, &val) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }

        /*Set SPG Tx Packet Count*/
        if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_COUNT_CONTROLr, 
                        ESW_TEST_PKT_CNTf, &(pTestFrameCfg->frameCount)) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }
    }

    /*Set SPG Pkt Length: fix packet length*/
    val = 0;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_LEN_TYPEf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set SPG Tx Packet Length*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL4r, 
                    ESW_LENGTH0f, &(pTestFrameCfg->length)) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Pkt Interval*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_TX_INTERVALr, 
                    ESW_TXINTERVALf, &(pTestFrameCfg->frameInterval)) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Pkt tx port mask*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL1r, 
                    ESW_TXPMf, &(pTestFrameCfg->txPortmask.bits[0])) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    
    /*Last Start SPG*/
    val = SPG_START_TX;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_CMDf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}/*end of dal_esw_oam_txTestFrame_start*/


/* Function Name:
 *      dal_esw_oam_txTestFrame_stop
 * Description:
 *      stop transmitting OAM test frame.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 * Note:
 *      None
 */
int32
dal_esw_oam_txTestFrame_stop(uint32 unit)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);

    OAM_SEM_LOCK(unit);

    /*Stop SPG*/
    val = SPG_STOP_TX;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_CMDf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "FAILED to Stop SPG!");
        return ret;
    }

    val = DISABLED;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_EN_SPG_MODf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}/*end of dal_esw_oam_txTestFrame_stop*/

/* Function Name:
 *      dal_esw_oam_txStatusOfTestFrame_get
 * Description:
 *      Get transmitting status of test frame.
 * Input:
 *      unit      - unit id
 *      pTxStatus - transmitting status
 *      pTxCount  - number of transmitted packets.
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Transmitting status is as following:
 *      - TX_FINISH
 *      - TX_NOT_FINISH
 */
int32
dal_esw_oam_txStatusOfTestFrame_get(uint32 unit, rtk_oam_testFrameTxStatus_t *pTxStatus, uint64 *pTxCount)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pTxStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pTxCount), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_DONE_FLGf, pTxStatus) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    if(*pTxStatus == 0)
        *pTxStatus = TX_NOT_FINISH;
    else
        *pTxStatus = TX_FINISH;

    *pTxCount = 0;
    if(*pTxStatus == TX_FINISH)
    {        
        /*Get Low 32 bit counter*/
        if ((ret = reg_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL7r, &val) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }

        *pTxCount = val;

        /*Get High 32 bit counter*/
        if ((ret = reg_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL8r, &val) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }

        *pTxCount |= ((uint64)val << 32);
    }

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pTxStatus=%d, pTxCount=%llu", *pTxStatus, *pTxCount);
    
    return RT_ERR_OK;
}/*end of dal_esw_oam_oamCounter_get*/

/* Function Name:
 *      dal_esw_oam_loopbackMode_get
 * Description:
 *      Get OAM loopback mode on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pLoopbackMode - pointer to OAM loopback mode
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      OAM loopback mode is as following:
 *      - OAM_LOOPBACK_ALL_FRAME
 *      - OAM_LOOPBACK_TEST_FRAME
 *      - OAM_LOOPBACK_DROP_TEST_FRAME
 *      - OAM_LOOPBACK_DISABLE
 */
int32
dal_esw_oam_loopbackMode_get(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackMode_t  *pLoopbackMode)
{
    int32   ret;
    uint32 oamTfAc;
    uint32 oamParserAct;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pLoopbackMode), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    osal_memset(pLoopbackMode, 0, sizeof(rtk_oam_loopbackMode_t));

    OAM_SEM_LOCK(unit);
    
    /* Get Oampdu test frame action*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_PDUTFACTf, &oamTfAc) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /* Get normal packet action*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_PARSER_ACTf, &oamParserAct) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);

    /*oam test frame: follow oampdu action; normal packet:forward*/
    if((oamTfAc == OAMTF_FOLLOW_OAMPDU) && (oamParserAct == OAMPARSER_FORWARD))
        *pLoopbackMode = OAM_LOOPBACK_DISABLE;

    if(oamParserAct == OAMPARSER_LOOPBACK)
        *pLoopbackMode = OAM_LOOPBACK_ALL_FRAME;
    else
    {
        if(oamTfAc == OAMTF_LOOPBACK)
            *pLoopbackMode = OAM_LOOPBACK_TEST_FRAME;
        else if(oamTfAc == OAMTF_DROP)
            *pLoopbackMode = OAM_LOOPBACK_DROP_TEST_FRAME;            
    } 
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pLoopbackMode=%d", *pLoopbackMode);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_oamCounter_get*/

/* Function Name:
 *      dal_esw_oam_loopbackMode_set
 * Description:
 *      Set OAM loopback mode on specified port.
 * Input:
 *      unit         - unit id
 *      port         - port id
 *      loopbackMode - OAM loopback mode
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      OAM loopback mode is as following:
 *      - OAM_LOOPBACK_ALL_FRAME
 *      - OAM_LOOPBACK_TEST_FRAME
 *      - OAM_LOOPBACK_DROP_TEST_FRAME
 *      - OAM_LOOPBACK_DISABLE
 */
int32
dal_esw_oam_loopbackMode_set(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackMode_t  loopbackMode)
{
    int32   ret;
    uint32 oamTfAc = 0;
    uint32 oamParserAct = 0;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d, loopbackMode=%d", 
            unit, port, loopbackMode);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((loopbackMode >= OAM_LOOPBACK_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    /*Translate loopback mode to asic mode*/
    if(loopbackMode == OAM_LOOPBACK_ALL_FRAME)
    {
        oamParserAct = OAMPARSER_LOOPBACK;
        oamTfAc = OAMTF_FOLLOW_OAMPDU;
    }
    else if(loopbackMode == OAM_LOOPBACK_TEST_FRAME)
    {
        oamParserAct = OAMPARSER_FORWARD;
        oamTfAc = OAMTF_LOOPBACK;        
    }
    else if(loopbackMode == OAM_LOOPBACK_DROP_TEST_FRAME)
    {
        oamParserAct = OAMPARSER_FORWARD;
        oamTfAc = OAMTF_DROP;
    }
    else
    {
        /*Default value*/
        oamTfAc = OAMTF_FOLLOW_OAMPDU;
        oamParserAct = OAMPARSER_FORWARD;
    }    

    OAM_SEM_LOCK(unit);
    
    /* Set Oampdu test frame action*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_PDUTFACTf, &oamTfAc) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /* Set normal packet action*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_PARSER_ACTf, &oamParserAct) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_oamCounter_set*/

/* Function Name:
 *      dal_esw_oam_loopbackCtrl_get
 * Description:
 *      Get control of loopback action on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      pLoopbackCtrl - pointer to control of loopback action
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_loopbackCtrl_get(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pLoopbackCtrl), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    osal_memset(pLoopbackCtrl, 0, sizeof(rtk_oam_loopbackCtrl_t));

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBSAf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Get SA Info*/
    switch(val)
    {
        case 0:
            pLoopbackCtrl->sa_action = SA_ACTION_KEEP_MAC;
            break;
        case 1:
            pLoopbackCtrl->sa_action = SA_ACTION_USE_SWITCH_MAC;
            break;
        case 2:
            pLoopbackCtrl->sa_action = SA_ACTION_USE_DA;
            break;
        case 3:
            pLoopbackCtrl->sa_action = SA_ACTION_USE_USER_DEFINE_MAC;
            /*Get User Defined Smac info*/
            if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL0r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_SA_31_0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            pLoopbackCtrl->user_defined_src_mac.octet[5] = val & BITMASK_8B;
            pLoopbackCtrl->user_defined_src_mac.octet[4] = (val >> 8) & BITMASK_8B;
            pLoopbackCtrl->user_defined_src_mac.octet[3] = (val >> 16) & BITMASK_8B;
            pLoopbackCtrl->user_defined_src_mac.octet[2] = (val >> 24) & BITMASK_8B;

            if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL1r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_SA_47_32f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            pLoopbackCtrl->user_defined_src_mac.octet[1] = val & BITMASK_8B;
            pLoopbackCtrl->user_defined_src_mac.octet[0] = (val >> 8) & BITMASK_8B;            
            break;
        default:
            OAM_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_OAM), "");
            return RT_ERR_FAILED;
    }

    if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBDAf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Get DA Info*/
    switch(val)
    {
        case 0:
            pLoopbackCtrl->da_action = DA_ACTION_KEEP_MAC;
            break;
        case 1:
            pLoopbackCtrl->da_action = DA_ACTION_USE_SWITCH_MAC;
            break;
        case 2:
            pLoopbackCtrl->da_action = DA_ACTION_USE_SA;
            break;
        case 3:
            pLoopbackCtrl->da_action = DA_ACTION_USE_USER_DEFINE_MAC;
            /*Get User Defined Smac info*/
            if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL1r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_DA_47_32f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            pLoopbackCtrl->user_defined_dst_mac.octet[1] = val & BITMASK_8B;
            pLoopbackCtrl->user_defined_dst_mac.octet[0] = (val >> 8) & BITMASK_8B;           

            if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL2r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_DA_31_0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            pLoopbackCtrl->user_defined_dst_mac.octet[5] = val & BITMASK_8B;
            pLoopbackCtrl->user_defined_dst_mac.octet[4] = (val >> 8) & BITMASK_8B;
            pLoopbackCtrl->user_defined_dst_mac.octet[3] = (val >> 16) & BITMASK_8B;
            pLoopbackCtrl->user_defined_dst_mac.octet[2] = (val >> 24) & BITMASK_8B;
            break;
        default:
            OAM_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_OAM), "");
            return RT_ERR_FAILED;
    }

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pLoopbackCtrl->sa_action=%d,pLoopbackCtrl->da_action=%d", 
                pLoopbackCtrl->sa_action, pLoopbackCtrl->da_action);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLoopbackCtrl->user_defined_src_mac=%x-%x-%x-%x-%x-%x",
           pLoopbackCtrl->user_defined_src_mac.octet[0], pLoopbackCtrl->user_defined_src_mac.octet[1], 
           pLoopbackCtrl->user_defined_src_mac.octet[2], pLoopbackCtrl->user_defined_src_mac.octet[3], 
           pLoopbackCtrl->user_defined_src_mac.octet[4], pLoopbackCtrl->user_defined_src_mac.octet[5]); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLoopbackCtrl->user_defined_dst_mac=%x-%x-%x-%x-%x-%x",
           pLoopbackCtrl->user_defined_dst_mac.octet[0], pLoopbackCtrl->user_defined_dst_mac.octet[1], 
           pLoopbackCtrl->user_defined_dst_mac.octet[2], pLoopbackCtrl->user_defined_dst_mac.octet[3], 
           pLoopbackCtrl->user_defined_dst_mac.octet[4], pLoopbackCtrl->user_defined_dst_mac.octet[5]); 
    
    return RT_ERR_OK;
}/*end of dal_esw_oam_loopbackCtrl_get*/    
    

/* Function Name:
 *      dal_esw_oam_loopbackCtrl_set
 * Description:
 *      Set control of loopback action on specified port.
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      pLoopbackCtrl - control of loopback action
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_loopbackCtrl_set(
    uint32                  unit, 
    rtk_port_t              port, 
    rtk_oam_loopbackCtrl_t  *pLoopbackCtrl)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pLoopbackCtrl), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pLoopbackCtrl->sa_action=%d,pLoopbackCtrl->da_action=%d", 
                pLoopbackCtrl->sa_action, pLoopbackCtrl->da_action);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLoopbackCtrl->user_defined_src_mac=%x-%x-%x-%x-%x-%x",
           pLoopbackCtrl->user_defined_src_mac.octet[0], pLoopbackCtrl->user_defined_src_mac.octet[1], 
           pLoopbackCtrl->user_defined_src_mac.octet[2], pLoopbackCtrl->user_defined_src_mac.octet[3], 
           pLoopbackCtrl->user_defined_src_mac.octet[4], pLoopbackCtrl->user_defined_src_mac.octet[5]); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pLoopbackCtrl->user_defined_dst_mac=%x-%x-%x-%x-%x-%x",
           pLoopbackCtrl->user_defined_dst_mac.octet[0], pLoopbackCtrl->user_defined_dst_mac.octet[1], 
           pLoopbackCtrl->user_defined_dst_mac.octet[2], pLoopbackCtrl->user_defined_dst_mac.octet[3], 
           pLoopbackCtrl->user_defined_dst_mac.octet[4], pLoopbackCtrl->user_defined_dst_mac.octet[5]); 

    OAM_SEM_LOCK(unit);    

    /*Set SA Info*/
    switch(pLoopbackCtrl->sa_action)
    {
        case SA_ACTION_KEEP_MAC:
            val = 0;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBSAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            break;
        case SA_ACTION_USE_SWITCH_MAC:
            val = 1;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBSAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            break;
        case SA_ACTION_USE_DA:
            val = 2;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBSAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            break;
        case SA_ACTION_USE_USER_DEFINE_MAC:
            val = 3;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBSAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = pLoopbackCtrl->user_defined_src_mac.octet[5] | (pLoopbackCtrl->user_defined_src_mac.octet[4] << 8)|
                    (pLoopbackCtrl->user_defined_src_mac.octet[3] << 16)|(pLoopbackCtrl->user_defined_src_mac.octet[2] << 24);
            
            /*Set User Defined Smac info*/
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL0r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_SA_31_0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = pLoopbackCtrl->user_defined_src_mac.octet[1] | (pLoopbackCtrl->user_defined_src_mac.octet[0] << 8);
            
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL1r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_SA_47_32f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            break;
        default:
            OAM_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_OAM), "");
            return RT_ERR_FAILED;
    }


   /*Set DA Info*/
    switch(pLoopbackCtrl->da_action)
    {
        case DA_ACTION_KEEP_MAC:
            val = 0;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBDAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            break;
        case DA_ACTION_USE_SWITCH_MAC:
            val = 1;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBDAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            break;
        case DA_ACTION_USE_SA:
            val = 2;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBDAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            break;
        case DA_ACTION_USE_USER_DEFINE_MAC:
            val = 3;
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_LBDAf, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = pLoopbackCtrl->user_defined_dst_mac.octet[1] | (pLoopbackCtrl->user_defined_dst_mac.octet[0] << 8);
            
            /*Set User Defined Smac info*/
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL1r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_DA_47_32f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = pLoopbackCtrl->user_defined_dst_mac.octet[5] | (pLoopbackCtrl->user_defined_dst_mac.octet[4] << 8)|
                    (pLoopbackCtrl->user_defined_dst_mac.octet[3] << 16)|(pLoopbackCtrl->user_defined_dst_mac.octet[2] << 24);
            
            if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_REPLACED_MAC_ADDRESS_CONTROL2r, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_RP_DA_31_0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
         
            break;
        default:
            OAM_SEM_UNLOCK(unit);
            RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_OAM), "");
            return RT_ERR_FAILED;
    }

    OAM_SEM_UNLOCK(unit); 
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_loopbackCtrl_set*/ 

/* Function Name:
 *      dal_esw_oam_DyingGaspSend_start
 * Description:
 *      Start sending dying gasp frame to specified ports.
 * Input:
 *      unit      - unit id
 *      pPortmask - ports for sending dying gasp
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_MASK        - invalid portmask
 * Note:
 *      This API will be used when CPU want to send dying gasp by itself.
 */
int32
dal_esw_oam_DyingGaspSend_start(uint32 unit, rtk_portmask_t *pPortmask)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pPortmask=%d", pPortmask->bits[0]);

    OAM_SEM_LOCK(unit);

     /*Set Dying Gasp Port Mask*/
    if ((ret = reg_field_write(unit, ESW_DYING_GASP_PORT_MASKr, ESW_DYPMf, 
                   &(pPortmask->bits[0])) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Trigger Dying Gasp Function*/
    val = ENABLED;
    if ((ret = reg_field_write(unit, ESW_DYING_GASP_POWER__MONITOR_CIRCUIT_DEGLITCHr, 
                    ESW_DYINGGASPTRIGGERf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}/*end of dal_esw_oam_DyingGaspSend_start*/

/* Function Name:
 *      dal_esw_oam_autoDyingGaspEnable_get
 * Description:
 *      Get enable status of sending dying gasp automatically on specified port 
 *      when voltage is lower than expected.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of sending dying gasp automatically
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_autoDyingGaspEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_DYING_GASPf, pEnable) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_autoDyingGaspEnable_get*/

/* Function Name:
 *      dal_esw_oam_autoDyingGaspEnable_set
 * Description:
 *      Set enable status of sending dying gasp automatically on specified port 
 *      when voltage is lower than expected.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of sending dying gasp automatically
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_autoDyingGaspEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d, enable=%d", unit, port, enable);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_OAM_PARSER_CONTROLr, 
                    port, REG_ARRAY_INDEX_NONE, ESW_POAM_DYING_GASPf, &enable) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_autoDyingGaspEnable_set*/

/* Function Name:
 *      dal_esw_oam_dyingGaspTLV_get
 * Description:
 *      Get TLV content of dying gasp frame on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pTLV - pointer to content of TLV
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_dyingGaspTLV_get(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pTLV), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    osal_memset(pTLV, 0, sizeof(rtk_oam_dyingGaspTLV_t));

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_read(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 3, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->localTLV[0] = (val >> 24) & BITMASK_8B;
    pTLV->localTLV[1] = (val >> 16) & BITMASK_8B;
    pTLV->localTLV[2] = (val >> 8) & BITMASK_8B;
    pTLV->localTLV[3] = (val >> 0) & BITMASK_8B;

    if ((ret = reg_array_read(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 2, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->localTLV[4] = (val >> 24) & BITMASK_8B;
    pTLV->localTLV[5] = (val >> 16) & BITMASK_8B;
    pTLV->localTLV[6] = (val >> 8) & BITMASK_8B;
    pTLV->localTLV[7] = (val >> 0) & BITMASK_8B;

    if ((ret = reg_array_read(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 1, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->localTLV[8] = (val >> 24) & BITMASK_8B;
    pTLV->localTLV[9] = (val >> 16) & BITMASK_8B;
    pTLV->localTLV[10] = (val >> 8) & BITMASK_8B;
    pTLV->localTLV[11] = (val >> 0) & BITMASK_8B;

    if ((ret = reg_array_read(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 0, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->localTLV[12] = (val >> 24) & BITMASK_8B;
    pTLV->localTLV[13] = (val >> 16) & BITMASK_8B;
    pTLV->localTLV[14] = (val >> 8) & BITMASK_8B;
    pTLV->localTLV[15] = (val >> 0) & BITMASK_8B;

    /*Get Remote Dying Gasp TLV*/
    if ((ret = reg_array_read(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 3, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->remoteTLV[0] = (val >> 24) & BITMASK_8B;
    pTLV->remoteTLV[1] = (val >> 16) & BITMASK_8B;
    pTLV->remoteTLV[2] = (val >> 8) & BITMASK_8B;
    pTLV->remoteTLV[3] = (val >> 0) & BITMASK_8B;

    if ((ret = reg_array_read(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 2, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->remoteTLV[4] = (val >> 24) & BITMASK_8B;
    pTLV->remoteTLV[5] = (val >> 16) & BITMASK_8B;
    pTLV->remoteTLV[6] = (val >> 8) & BITMASK_8B;
    pTLV->remoteTLV[7] = (val >> 0) & BITMASK_8B;

    if ((ret = reg_array_read(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 1, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->remoteTLV[8] = (val >> 24) & BITMASK_8B;
    pTLV->remoteTLV[9] = (val >> 16) & BITMASK_8B;
    pTLV->remoteTLV[10] = (val >> 8) & BITMASK_8B;
    pTLV->remoteTLV[11] = (val >> 0) & BITMASK_8B;

    if ((ret = reg_array_read(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 0, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    pTLV->remoteTLV[12] = (val >> 24) & BITMASK_8B;
    pTLV->remoteTLV[13] = (val >> 16) & BITMASK_8B;
    pTLV->remoteTLV[14] = (val >> 8) & BITMASK_8B;
    pTLV->remoteTLV[15] = (val >> 0) & BITMASK_8B;

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_dyingGaspTLV_get*/

/* Function Name:
 *      dal_esw_oam_dyingGaspTLV_set
 * Description:
 *      Set TLV content of dying gasp frame on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 *      pTLV - content of TLV
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_dyingGaspTLV_set(uint32 unit, rtk_port_t port, rtk_oam_dyingGaspTLV_t *pTLV)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pTLV), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /*Set Remote Dying Gasp TLV*/
    val = (pTLV->localTLV[0] << 24) | (pTLV->localTLV[1] << 16) | (pTLV->localTLV[2] << 8) | pTLV->localTLV[3];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 3, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pTLV->localTLV[4] << 24) | (pTLV->localTLV[5] << 16) | (pTLV->localTLV[6] << 8) | pTLV->localTLV[7];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 2, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pTLV->localTLV[8] << 24) | (pTLV->localTLV[9] << 16) | (pTLV->localTLV[10] << 8) | pTLV->localTLV[11];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 1, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pTLV->localTLV[12] << 24) | (pTLV->localTLV[13] << 16) | (pTLV->localTLV[14] << 8) | pTLV->localTLV[15];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_LOCAL_DYING_GASPr, 
                    port, 0, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Remote Dying Gasp TLV*/
    val = (pTLV->remoteTLV[0] << 24) | (pTLV->remoteTLV[1] << 16) | (pTLV->remoteTLV[2] << 8) | pTLV->remoteTLV[3];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 3, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pTLV->remoteTLV[4] << 24) | (pTLV->remoteTLV[5] << 16) | (pTLV->remoteTLV[6] << 8) | pTLV->remoteTLV[7];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 2, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pTLV->remoteTLV[8] << 24) | (pTLV->remoteTLV[9] << 16) | (pTLV->remoteTLV[10] << 8) | pTLV->remoteTLV[11];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 1, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pTLV->remoteTLV[12] << 24) | (pTLV->remoteTLV[13] << 16) | (pTLV->remoteTLV[14] << 8) | pTLV->remoteTLV[15];
    if ((ret = reg_array_write(unit, ESW_PORT_OAM_REMOTE_DYING_GASPr, 
                    port, 0, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_dyingGaspTLV_set*/

/* Function Name:
 *      dal_esw_oam_dyingGaspWaitTime_get
 * Description:
 *      Get waiting time of sending dying gasp after voltage is lower than expeted.
 * Input:
 *      unit - unit id
 * Output:
 *      time - waiting time
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      (1) Granularity of waiting time is 10 ns.
 *      (2) The valid range of time value is 0~0xFFFF
 */
int32
dal_esw_oam_dyingGaspWaitTime_get(uint32 unit, uint32 *pTime)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pTime), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_DYING_GASP_POWER__MONITOR_CIRCUIT_DEGLITCHr, 
                  ESW_TBPVALUEf, pTime) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pTime=%d", *pTime);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_dyingGaspWaitTime_get*/

/* Function Name:
 *      dal_esw_oam_dyingGaspWaitTime_set
 * Description:
 *      Set waiting time of sending dying gasp after voltage is lower than expeted.
 * Input:
 *      unit - unit id
 *      time - waiting time
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_OUT_OF_RANGE     - input parameter out of range
 * Note:
 *      (1) Granularity of waiting time is 10 ns.
 *      (2) The valid range of time value is 0~0xFFFF
 */
int32
dal_esw_oam_dyingGaspWaitTime_set(uint32 unit, uint32 time)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, time=%d", unit, time);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_DYING_GASP_POWER__MONITOR_CIRCUIT_DEGLITCHr, 
                  ESW_TBPVALUEf, &time) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_dyingGaspWaitTime_get*/

/* Module Name    : OAM               */
/* Sub-module Name: CFM configuration */

/* Function Name:
 *      dal_esw_oam_cfmEntry_get
 * Description:
 *      Get configuration of specified CFM group.
 * Input:
 *      unit    - unit id
 *      cfm_idx - CFM index
 * Output:
 *      pCfm    - pointer to configuration of specified CFM group
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      CFM group = MA
 */
int32
dal_esw_oam_cfmEntry_get(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm)
{
    int32   ret, idx;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, cfm_idx=%d", unit, cfm_idx);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((cfm_idx > CFM_MAX_IDX), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCfm), RT_ERR_NULL_POINTER);

    osal_memset(pCfm, 0, sizeof(rtk_oam_cfm_t));

    OAM_SEM_LOCK(unit);
    
    /* get md level*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL2r, 
                  ESW_MDLEVEL_VER0f - cfm_idx, &pCfm->md_level) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    /*Get MAID*/
    if(cfm_idx == 0)
    {
        for(idx =0; idx < RTK_MAX_LEN_OF_CFM_MAID/4; idx++)
        {
            if ((ret = reg_array_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_MAID0r, 11 - idx, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            } 

            pCfm->maid[idx*4] = (val >> 24) & BITMASK_8B; 
            pCfm->maid[idx*4 + 1] = (val >> 16) & BITMASK_8B; 
            pCfm->maid[idx*4 + 2] = (val >> 8) & BITMASK_8B; 
            pCfm->maid[idx*4 + 3] = val & BITMASK_8B; 
        }
    }
    else
    {
        for(idx =0; idx < RTK_MAX_LEN_OF_CFM_MAID/4; idx++)
        {
            if ((ret = reg_array_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_MAID1r, 11 - idx, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            } 

            pCfm->maid[idx*4] = (val >> 24) & BITMASK_8B; 
            pCfm->maid[idx*4 + 1] = (val >> 16) & BITMASK_8B; 
            pCfm->maid[idx*4 + 2] = (val >> 8) & BITMASK_8B; 
            pCfm->maid[idx*4 + 3] = val & BITMASK_8B; 
        }
    }    

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pCfm->md_level=%d", pCfm->md_level);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMFlag_get*/

/* Function Name:
 *      dal_esw_oam_cfmEntry_set
 * Description:
 *      Set configuration of specified CFM group.
 * Input:
 *      unit    - unit id
 *      cfm_idx - CFM index
 *      pCfm    - configuration of specified CFM group
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      CFM group = MA
 */
int32
dal_esw_oam_cfmEntry_set(uint32 unit, uint32 cfm_idx, rtk_oam_cfm_t *pCfm)
{
    int32   ret, idx;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, cfm_idx=%d", unit, cfm_idx);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((cfm_idx > CFM_MAX_IDX), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCfm), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pCfm->md_level=%d", pCfm->md_level);

    OAM_SEM_LOCK(unit);
    
    /*Set SPG Register*/
    val = pCfm->md_level;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL2r, 
                  ESW_MDLEVEL_VER0f - cfm_idx, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    /*Set ALE Register*/
    val = pCfm->md_level;
    if ((ret = reg_field_write(unit, ESW_MEPID_RECORD_GLOBAL_CONTROLr, 
                  ESW_MDLEVEL0f + cfm_idx, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    /*Set MAID*/
    if(cfm_idx == 0) /*entry 0*/
    {
        /*Set SPG MAID0*/
        for(idx =0; idx < RTK_MAX_LEN_OF_CFM_MAID/4; idx++)
        {
            val = (pCfm->maid[idx*4] << 24) |(pCfm->maid[idx*4 + 1] << 16) |
                    (pCfm->maid[idx*4 + 2] << 8) | (pCfm->maid[idx*4 + 3]);
            if ((ret = reg_array_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_MAID0r, 11 - idx, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }

        /*Set ALE MAID0*/
        val = (pCfm->maid[0] << 8) |pCfm->maid[1] ;
        if ((ret = reg_field_write(unit, ESW_CFM_MAID0_COMPARE8r, ESW_MAID0f, &val)) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }

        for(idx =0; idx < 8; idx++)
        {
            val = (pCfm->maid[idx*4 + 2] << 24) |(pCfm->maid[idx*4 + 3] << 16) |
                    (pCfm->maid[idx*4 + 4] << 8) | (pCfm->maid[idx*4 + 5]);
            if ((ret = reg_write(unit, ESW_CFM_MAID0_COMPARE0r  + 7 - idx, &val)) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
        
    }
    else    /*entry 1*/
    {
        /*Set SPG MAID1*/
        for(idx =0; idx < RTK_MAX_LEN_OF_CFM_MAID/4; idx++)
        {
            val = (pCfm->maid[idx*4] << 24) |(pCfm->maid[idx*4 + 1] << 16) |
                    (pCfm->maid[idx*4 + 2] << 8) | (pCfm->maid[idx*4 + 3]);
            if ((ret = reg_array_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_MAID1r, 11 - idx, REG_ARRAY_INDEX_NONE, &val)) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            } 
        }

        /*Set ALE MAID1*/
        val = (pCfm->maid[0] << 8) |pCfm->maid[1] ;
        if ((ret = reg_field_write(unit, ESW_CFM_MAID1_COMPARE8r, ESW_MAID1f, &val)) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }

        for(idx =0; idx < 8; idx++)
        {
            val = (pCfm->maid[idx*4 + 2] << 24) |(pCfm->maid[idx*4 + 3] << 16) |
                    (pCfm->maid[idx*4 + 4] << 8) | (pCfm->maid[idx*4 + 5]);
            if ((ret = reg_write(unit, ESW_CFM_MAID1_COMPARE0r  + 7 - idx, &val)) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMFlag_get*/

/* Function Name:
 *      dal_esw_oam_cfmPortEntry_get
 * Description:
 *      Get CFM configuration of specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pPortCfg - pointer to CFM configuration of port
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Each port can be only belong to one CFM group.
 *      mepid of configuration is used when transmit CCM packets.
 */
int32
dal_esw_oam_cfmPortEntry_get(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortCfg), RT_ERR_NULL_POINTER);

    osal_memset(pPortCfg, 0, sizeof(rtk_oam_cfmPort_t));

    OAM_SEM_LOCK(unit);
    
    /* get md level*/
    if ((ret = reg_array_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_MEPID_CONTROLr, port, 
               REG_ARRAY_INDEX_NONE, ESW_P_MEPIDf, &pPortCfg->mepid)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    /*Get Port Group*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_CCM_RECORD_CONTROL0r, 
                  port, REG_ARRAY_INDEX_NONE, ESW_CGROUPSELf, &pPortCfg->cfm_idx) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }  

    OAM_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pPortCfg->cfm_idx=%d, pPortCfg->mepid=%d", 
                pPortCfg->cfm_idx, pPortCfg->mepid);

    return RT_ERR_OK;    
}   /*end of dal_esw_oam_cfmPortEntry_get*/

/* Function Name:
 *      dal_esw_oam_cfmPortEntry_set
 * Description:
 *      Set CFM configuration of specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 *      pPortCfg - CFM configuration of port
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      Each port can be only belong to one CFM group.
 *      mepid of configuration is used when transmit CCM packets.
 */
int32
dal_esw_oam_cfmPortEntry_set(uint32 unit, rtk_port_t port, rtk_oam_cfmPort_t *pPortCfg)
{
    int32 ret;
    rtk_portmask_t portMsk;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortCfg), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pPortCfg->cfm_idx > CFM_MAX_IDX), RT_ERR_INPUT);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pPortCfg->cfm_idx=%d, pPortCfg->mepid=%d", 
                pPortCfg->cfm_idx, pPortCfg->mepid);

    OAM_SEM_LOCK(unit);
    
    /* get md level*/
    if ((ret = reg_array_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_MEPID_CONTROLr, port, 
               REG_ARRAY_INDEX_NONE, ESW_P_MEPIDf, &(pPortCfg->mepid))) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    /*Set Port Group(ALE)*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_CCM_RECORD_CONTROL0r, 
                  port, REG_ARRAY_INDEX_NONE, ESW_CGROUPSELf, &(pPortCfg->cfm_idx)) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }  

    /*Get Port Group(SPG)*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_GROUP_SELECTION_CONTROLr, 
                  ESW_TXGROUPSELMASKf, &portMsk.bits[0]) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    if(pPortCfg->cfm_idx == 0)
        BITMAP_CLEAR(portMsk.bits, port);
    else
        BITMAP_SET(portMsk.bits, port);

    /*Set Port Group(SPG)*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_GROUP_SELECTION_CONTROLr, 
                  ESW_TXGROUPSELMASKf, &(portMsk.bits[0])) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    OAM_SEM_UNLOCK(unit);   

    return RT_ERR_OK;    
}   /*end of dal_esw_oam_cfmPortEntry_set*/

/* Function Name:
 *      dal_esw_oam_cfmMepEnable_get
 * Description:
 *      Get enable status of MEP function on specified port.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status of MEP function
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmMepEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_CCM_RECORD_CONTROL0r, 
                  port, REG_ARRAY_INDEX_NONE, ESW_PMEPIDCENf, pEnable) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pEnable=%d", *pEnable);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmMepEnable_get*/

/* Function Name:
 *      dal_esw_oam_cfmMepEnable_set
 * Description:
 *      Set enable status of MEP function on specified port.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status of MEP function
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmMepEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_CCM_RECORD_CONTROL0r, 
                  port, REG_ARRAY_INDEX_NONE, ESW_PMEPIDCENf, &enable) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmMepEnable_set*/

/* Function Name:
 *      dal_esw_oam_txCCMFrame_start
 * Description:
 *      Start sending CCM packet on specified ports.
 * Input:
 *      unit      - unit id
 *      pPortmask - portmask for sending CCM frame
 * Output:
 *      None.
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_txCCMFrame_start(uint32 unit, rtk_portmask_t *pPortmask)
{
    int32   ret;
    uint32 isSnap, val;
    uint32 hasOuterVlanTag, hasInnerVlanTag, length;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pPortmask), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pPortmask=%d", pPortmask->bits[0]);

    val = ENABLED;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_EN_SPG_MODf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*First Stop SPG*/
    val = SPG_STOP_TX;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_CMDf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    
    /*Set SPG Continue mode*/
    val = 0;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_PKT_CNT_MODEf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set SPG Pkt Length: fix packet length*/
    val = 0;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_LEN_TYPEf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Default EtherType:0x8902*/
    val = 1;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_EN_ETHER_TYPEf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = 0x8902;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL1r, 
                    ESW_ETHER_TYPEf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    val = SPG_PKT_CCM;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_PKT_TYPEf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    if((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_CCMPKTTYPEf, &isSnap)) != RT_ERR_OK)    
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    if((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_EN_OVLAN_TAG0f, &hasOuterVlanTag)) != RT_ERR_OK)    
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    if((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_EN_IVLAN_TAG0f, &hasInnerVlanTag)) != RT_ERR_OK)    
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    } 

    length = DAL_MAX_CCM_PKTLEN;
    if(!isSnap)
        length -= 8;
    if(!hasOuterVlanTag)
        length -= 4;
    if(!hasInnerVlanTag)
        length -= 4;

    /*Set SPG Tx Packet Length: Max Length = DAL_MAX_CCM_PKTLEN 
    (CCM Packet Length = ethernet header+outer vlan tag + inner vlan tag + snap header + packet payload + fcs)*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL4r, 
                    ESW_LENGTH0f, &length) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Length1*/
    if((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_EN_OVLAN_TAG1f, &hasOuterVlanTag)) != RT_ERR_OK)    
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    if((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_EN_IVLAN_TAG1f, &hasInnerVlanTag)) != RT_ERR_OK)    
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
    } 

    length = DAL_MAX_CCM_PKTLEN;
    if(!isSnap)
        length -= 8;
    if(!hasOuterVlanTag)
        length -= 4;
    if(!hasInnerVlanTag)
        length -= 4;

    /*Set Packet Length1*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL6r, 
                    ESW_LENGTH1f, &length) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    /*Set Pkt tx port mask*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL1r, 
                    ESW_TXPMf, &(pPortmask->bits[0])) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    
    /*Then Start SPG*/
    val = SPG_START_TX;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_CMDf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}/*end of dal_esw_oam_txTestFrame_start*/

/* Function Name:
 *      dal_esw_oam_txCCMFrame_stop
 * Description:
 *      Stop sending CCM packet.
 * Input:
 *      unit   - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 * Note:
 *      CCM transmit and OAM's test frame transmit can't be enabled at same time.
 */
int32
dal_esw_oam_txCCMFrame_stop(uint32 unit)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);

    OAM_SEM_LOCK(unit);

    /*First Stop SPG*/
    val = SPG_STOP_TX;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_TX_CMDf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = DISABLED;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL0r, 
                    ESW_EN_SPG_MODf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}/*end of rtk_oam_txCCMFrame_stop*/

/* Function Name:
 *      dal_esw_oam_cfmCCMFrame_get
 * Description:
 *      Get the content of CCM frame for specific CFM group.
 * Input:
 *      unit      - unit id
 *      cfm_idx   - CFM index
 * Output:
 *      pCcmFrame - pointer to content of CCM frame
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMFrame_get(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((cfm_idx > CFM_MAX_IDX), RT_ERR_INPUT);
    RT_PARAM_CHK((NULL == pCcmFrame), RT_ERR_NULL_POINTER);

    osal_memset(pCcmFrame, 0, sizeof(rtk_oam_ccmFrame_t));

    OAM_SEM_LOCK(unit);

    /*Get Cfm Pkt type*/
    if((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                ESW_CCMPKTTYPEf, &pCcmFrame->pktType)) != RT_ERR_OK)    
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    /*Get Destination mac address*/
    if ((ret = reg_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_DMAC_CONTROL0r, &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    pCcmFrame->dest_mac.octet[0] = (val >> 24) & BITMASK_8B;
    pCcmFrame->dest_mac.octet[1] = (val >> 16) & BITMASK_8B;
    pCcmFrame->dest_mac.octet[2] = (val >> 8) & BITMASK_8B;
    pCcmFrame->dest_mac.octet[3] = (val) & BITMASK_8B;

    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_DMAC_CONTROL1r, 
                   ESW_SPG_DA0_15_0f, &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    pCcmFrame->dest_mac.octet[4] = (val >> 8) & BITMASK_8B;
    pCcmFrame->dest_mac.octet[5] = (val) & BITMASK_8B;
    
    /*Get Source Mac address*/
    if ((ret = reg_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_SMAC_CONTROL0r, &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    pCcmFrame->source_mac.octet[0] = (val >> 24) & BITMASK_8B;
    pCcmFrame->source_mac.octet[1] = (val >> 16) & BITMASK_8B;
    pCcmFrame->source_mac.octet[2] = (val >> 8) & BITMASK_8B;
    pCcmFrame->source_mac.octet[3] = (val) & BITMASK_8B;

    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_SMAC_CONTROL1r, 
                  ESW_SPG_SA_15_0f, &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    pCcmFrame->source_mac.octet[4] = (val >> 8) & BITMASK_8B;
    pCcmFrame->source_mac.octet[5] = (val) & BITMASK_8B;

    /*Get Vlan Info*/
    /*Get Outer vlan*/
    if(cfm_idx == 0)
    {
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                ESW_EN_OVLAN_TAG0f, &pCcmFrame->enable_outerTag) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    if(pCcmFrame->enable_outerTag)
    {
        if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL0r, 
            ESW_OVLAN_HEADER0f, &val) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }
        
        pCcmFrame->outer_tpid = (val >> 16) & BITMASK_16B;
        pCcmFrame->outer_pri = (val >> 13) & BITMASK_3B;
        pCcmFrame->outer_dei = (val >> 12) & BITMASK_1B;
        pCcmFrame->outer_vid = (val >> 0) & BITMASK_12B;
        
    }

    /*Get Inner vlan*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                ESW_EN_IVLAN_TAG0f, &pCcmFrame->enable_innerTag) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    if(pCcmFrame->enable_innerTag)
    {
        if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL2r, 
            ESW_IVLAN_HEADER0f, &val) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }
        
        pCcmFrame->inner_tpid= (val >> 16) & BITMASK_16B;
        pCcmFrame->inner_pri= (val >> 13) & BITMASK_3B;
        pCcmFrame->inner_cfi= (val >> 12) & BITMASK_1B;
        pCcmFrame->inner_vid= (val >> 0) & BITMASK_12B;
        
    }    
    }
    else
    {
        if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_EN_OVLAN_TAG1f, &pCcmFrame->enable_outerTag) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }

        if(pCcmFrame->enable_outerTag)
        {
            if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL1r, 
                ESW_OVLAN_HEADER1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            
            pCcmFrame->outer_tpid = (val >> 16) & BITMASK_16B;
            pCcmFrame->outer_pri = (val >> 13) & BITMASK_3B;
            pCcmFrame->outer_dei = (val >> 12) & BITMASK_1B;
            pCcmFrame->outer_vid = (val >> 0) & BITMASK_12B;
            
        }

        /*Get Inner vlan*/
        if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_EN_IVLAN_TAG1f, &pCcmFrame->enable_innerTag) ) != RT_ERR_OK)
        {
            OAM_SEM_UNLOCK(unit);
            RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
            return ret;
        }

        if(pCcmFrame->enable_innerTag)
        {
            if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL3r, 
                ESW_IVLAN_HEADER1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
            
            pCcmFrame->inner_tpid= (val >> 16) & BITMASK_16B;
            pCcmFrame->inner_pri= (val >> 13) & BITMASK_3B;
            pCcmFrame->inner_cfi= (val >> 12) & BITMASK_1B;
            pCcmFrame->inner_vid= (val >> 0) & BITMASK_12B;
            
        }    
    }
    
    
    OAM_SEM_UNLOCK(unit);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pCcmFrame->pktType=%d, pCcmFrame->enable_outerTag=%d, \
                pCcmFrame->enable_innerTag=%d, pCcmFrame->outer_dei=%d, pCcmFrame->outer_pri=%d, \
                pCcmFrame->outer_tpid=0x%x, pCcmFrame->outer_vid=%d, pCcmFrame->inner_cfi=%d, pCcmFrame->inner_pri=%d, \
                pCcmFrame->inner_tpid=0x%x, pCcmFrame->inner_vid=%d", pCcmFrame->pktType,
                pCcmFrame->enable_outerTag,  pCcmFrame->enable_innerTag,
                pCcmFrame->outer_dei, pCcmFrame->outer_pri, pCcmFrame->outer_tpid, pCcmFrame->outer_vid,
                pCcmFrame->inner_cfi, pCcmFrame->inner_pri, pCcmFrame->inner_tpid, pCcmFrame->inner_vid);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pCcmFrame->dest_mac=%x-%x-%x-%x-%x-%x",
           pCcmFrame->dest_mac.octet[0], pCcmFrame->dest_mac.octet[1], pCcmFrame->dest_mac.octet[2],
           pCcmFrame->dest_mac.octet[3], pCcmFrame->dest_mac.octet[4], pCcmFrame->dest_mac.octet[5]); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pCcmFrame->source_mac=%x-%x-%x-%x-%x-%x",
           pCcmFrame->source_mac.octet[0], pCcmFrame->source_mac.octet[1], pCcmFrame->source_mac.octet[2],
           pCcmFrame->source_mac.octet[3], pCcmFrame->source_mac.octet[4], pCcmFrame->source_mac.octet[5]); 

    
    return RT_ERR_OK;
}

/* Function Name:
 *      dal_esw_oam_cfmCCMFrame_set
 * Description:
 *      Fill the content of CCM frame for specific CFM group.
 * Input:
 *      unit      - unit id
 *      cfm_idx   - CFM index
 *      pCcmFrame - content of CCM frame
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMFrame_set(uint32 unit, uint32 cfm_idx, rtk_oam_ccmFrame_t *pCcmFrame)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, cfm_idx=%d", unit, cfm_idx);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pCcmFrame), RT_ERR_NULL_POINTER);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pCcmFrame->pktType=%d, pCcmFrame->enable_outerTag=%d, \
                pCcmFrame->enable_innerTag=%d, pCcmFrame->outer_dei=%d, pCcmFrame->outer_pri=%d, \
                pCcmFrame->outer_tpid=0x%x, pCcmFrame->outer_vid=%d, pCcmFrame->inner_cfi=%d, pCcmFrame->inner_pri=%d, \
                pCcmFrame->inner_tpid=0x%x, pCcmFrame->inner_vid=%d", pCcmFrame->pktType,
                pCcmFrame->enable_outerTag,  pCcmFrame->enable_innerTag,
                pCcmFrame->outer_dei, pCcmFrame->outer_pri, pCcmFrame->outer_tpid, pCcmFrame->outer_vid,
                pCcmFrame->inner_cfi, pCcmFrame->inner_pri, pCcmFrame->inner_tpid, pCcmFrame->inner_vid);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pCcmFrame->dest_mac=%x-%x-%x-%x-%x-%x",
           pCcmFrame->dest_mac.octet[0], pCcmFrame->dest_mac.octet[1], pCcmFrame->dest_mac.octet[2],
           pCcmFrame->dest_mac.octet[3], pCcmFrame->dest_mac.octet[4], pCcmFrame->dest_mac.octet[5]); 

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_L2), "pCcmFrame->source_mac=%x-%x-%x-%x-%x-%x",
           pCcmFrame->source_mac.octet[0], pCcmFrame->source_mac.octet[1], pCcmFrame->source_mac.octet[2],
           pCcmFrame->source_mac.octet[3], pCcmFrame->source_mac.octet[4], pCcmFrame->source_mac.octet[5]); 

    RT_PARAM_CHK((cfm_idx > CFM_MAX_IDX), RT_ERR_INPUT);    
    RT_PARAM_CHK((pCcmFrame->pktType >= PKT_TYPE_END), RT_ERR_INPUT);

    RT_PARAM_CHK(((pCcmFrame->source_mac.octet[0] & BITMASK_1B)) != 0, RT_ERR_MAC);
    if(pCcmFrame->enable_innerTag)
    {
        RT_PARAM_CHK((pCcmFrame->inner_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
        RT_PARAM_CHK((pCcmFrame->inner_pri > RTK_DOT1P_PRIORITY_MAX), RT_ERR_INPUT);
        RT_PARAM_CHK((pCcmFrame->inner_cfi > RTK_DOT1P_DEI_MAX), RT_ERR_INPUT);
    }

    if(pCcmFrame->enable_outerTag)
    {
        RT_PARAM_CHK((pCcmFrame->outer_vid > RTK_VLAN_ID_MAX), RT_ERR_VLAN_VID);
        RT_PARAM_CHK((pCcmFrame->outer_pri> RTK_DOT1P_PRIORITY_MAX), RT_ERR_INPUT);
        RT_PARAM_CHK((pCcmFrame->outer_dei > RTK_DOT1P_DEI_MAX), RT_ERR_INPUT);
    }   

    OAM_SEM_LOCK(unit);

    /*Set Cfm Pkt type*/
    if((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                ESW_CCMPKTTYPEf, &(pCcmFrame->pktType))) != RT_ERR_OK)    
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_SWITCH), "");
        return ret;
     }

    /*Set Dmac mode:fix*/
    val = 0;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_DA_MODf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Dmac0 address*/
    val = (pCcmFrame->dest_mac.octet[0] << 24) | (pCcmFrame->dest_mac.octet[1] << 16) |
                    (pCcmFrame->dest_mac.octet[2] << 8)|pCcmFrame->dest_mac.octet[3];
    if ((ret = reg_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_DMAC_CONTROL0r, 
                    &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pCcmFrame->dest_mac.octet[4] << 8)|pCcmFrame->dest_mac.octet[5];
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_DMAC_CONTROL1r, 
                    ESW_SPG_DA0_15_0f, &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Dmac1 address*/
    val = (pCcmFrame->dest_mac.octet[0] << 8)|pCcmFrame->dest_mac.octet[1];
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_DMAC_CONTROL1r, 
                    ESW_SPG_DA1_47_32f, &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pCcmFrame->dest_mac.octet[2] << 24) | (pCcmFrame->dest_mac.octet[3] << 16) |
                    (pCcmFrame->dest_mac.octet[4] << 8)|pCcmFrame->dest_mac.octet[5];
    if ((ret = reg_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_DMAC_CONTROL2r, 
                    &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Smac mode:fix*/
    val = 0;
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                    ESW_SA_MODf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Set Smac address*/
    val = (pCcmFrame->source_mac.octet[0] << 24) | (pCcmFrame->source_mac.octet[1] << 16) |
                    (pCcmFrame->source_mac.octet[2] << 8)|pCcmFrame->source_mac.octet[3];
    if ((ret = reg_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_SMAC_CONTROL0r, 
                    &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    val = (pCcmFrame->source_mac.octet[4] << 8)|pCcmFrame->source_mac.octet[5];
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_SMAC_CONTROL1r, 
                    ESW_SPG_SA_15_0f, &val)) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    /*Config Vlan setting*/
    /*Outer vlan*/
    if(cfm_idx == 0)
    {
        if(pCcmFrame->enable_outerTag)
        {
            /*Enable Outer vlan*/
            val = ENABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_EN_OVLAN_TAG0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = ((pCcmFrame->outer_tpid & BITMASK_16B) << 16) | ((pCcmFrame->outer_pri & BITMASK_3B )<< 13) | 
                    ((pCcmFrame->outer_dei & BITMASK_1B )<< 12) | (pCcmFrame->outer_vid & BITMASK_12B);

            /*Set Vlan Data*/
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL0r, 
                        ESW_OVLAN_HEADER0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

        }
        else
        {
            /*Disable Outer vlan*/
            val = DISABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_EN_OVLAN_TAG0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
    }
    else
    {
        if(pCcmFrame->enable_outerTag)
        {
            /*Enable Outer vlan*/
            val = ENABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_EN_OVLAN_TAG1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = ((pCcmFrame->outer_tpid & BITMASK_16B) << 16) | ((pCcmFrame->outer_pri & BITMASK_3B )<< 13) | 
                    ((pCcmFrame->outer_dei & BITMASK_1B )<< 12) | (pCcmFrame->outer_vid & BITMASK_12B);

            /*Set Vlan Data*/
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL1r, 
                        ESW_OVLAN_HEADER1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

        }
        else
        {
            /*Disable Outer vlan*/
            val = DISABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                            ESW_EN_OVLAN_TAG1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
    }
    

    /*Inner vlan*/
    if(cfm_idx == 0)
    {
        if(pCcmFrame->enable_innerTag)
        {
            /*Enable Inner vlan*/
            val = ENABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_EN_IVLAN_TAG0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = ((pCcmFrame->inner_tpid & BITMASK_16B) << 16) | ((pCcmFrame->inner_pri & BITMASK_3B )<< 13) | 
                    ((pCcmFrame->inner_cfi & BITMASK_1B )<< 12) | (pCcmFrame->inner_vid & BITMASK_12B);

            /*Set Vlan Data*/
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL2r, 
                        ESW_IVLAN_HEADER0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
        else
        {
            /*Disable Inner vlan*/
            val = DISABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_EN_IVLAN_TAG0f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
    }
    else
    {
        if(pCcmFrame->enable_innerTag)
        {
            /*Enable Inner vlan*/
            val = ENABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                        ESW_EN_IVLAN_TAG1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }

            val = ((pCcmFrame->inner_tpid & BITMASK_16B) << 16) | ((pCcmFrame->inner_pri & BITMASK_3B )<< 13) | 
                    ((pCcmFrame->inner_cfi & BITMASK_1B )<< 12) | (pCcmFrame->inner_vid & BITMASK_12B);

            /*Set Vlan Data*/
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_VLAN_CONTROL3r, 
                        ESW_IVLAN_HEADER1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
        else
        {
            /*Disable Inner vlan*/
            val = DISABLED;
            if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_CONTROL2r, 
                            ESW_EN_IVLAN_TAG1f, &val) ) != RT_ERR_OK)
            {
                OAM_SEM_UNLOCK(unit);
                RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                return ret;
            }
        }
    }    
    
    OAM_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;

}   /*end of dal_esw_oam_cfmCCMFrame_set*/

/* Function Name:
 *      dal_esw_oam_cfmCCMSnapOui_get
 * Description:
 *      Get OUI of SNAP packets when packet type of CCM packet is SNAP.
 * Input:
 *      unit     - unit id
 * Output:
 *      pSnapoui - pointer to OUI of SNAP packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMSnapOui_get(uint32 unit, rtk_snapOui_t *pSnapoui)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pSnapoui), RT_ERR_NULL_POINTER);

    osal_memset(pSnapoui, 0, sizeof(rtk_snapOui_t));

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL3r, 
                  ESW_SNAPOUIf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);

    pSnapoui->snapOui[0] = (val >> 16) & BITMASK_8B;
    pSnapoui->snapOui[1] = (val >> 8) & BITMASK_8B;
    pSnapoui->snapOui[2] = (val >> 0) & BITMASK_8B;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pSnapoui=0x%06x", val);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMSnapOui_get*/

/* Function Name:
 *      dal_esw_oam_cfmCCMSnapOui_set
 * Description:
 *      Set OUI of SNAP packets when packet type of CCM packet is SNAP.
 * Input:
 *      unit     - unit id
 *      pSnapoui - OUI of SNAP packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMSnapOui_set(uint32 unit, rtk_snapOui_t *pSnapoui)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pSnapoui), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* Set value to CHIP*/
    val = (pSnapoui->snapOui[0] << 16) | (pSnapoui->snapOui[1] << 8)|pSnapoui->snapOui[2];
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pSnapoui=0x%06x", val);
    
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL3r, 
                  ESW_SNAPOUIf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);    
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMSnapOui_set*/

/* Function Name:
 *      dal_esw_oam_cfmCCMEtype_get
 * Description:
 *      Get ethernet type of CCM packets for specified CFM group.
 * Input:
 *      unit       - unit id
 * Output:
 *      pEtherType - pointer to ethernet type of CCM packets
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMEtype_get(uint32 unit, uint32 *pEtherType)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEtherType), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL1r, 
                  ESW_ETHER_TYPEf, pEtherType) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pEtherType=0x%04x", pEtherType);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMEtype_get*/

/* Function Name:
 *      dal_esw_oam_cfmCCMEtype_set
 * Description:
 *      Set ethernet type of CCM packets for specified CFM group.
 * Input:
 *      unit      - unit id
 *      etherType - ethernet type of CCM packets
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Default ethernet type of CCM packets is 0x8902
 */
int32
dal_esw_oam_cfmCCMEtype_set(uint32 unit, uint32 etherType)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, etherType=0x%x", unit, etherType);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    OAM_SEM_LOCK(unit);
    
    /*Set Mac EtherType*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL1r, 
                  ESW_ETHER_TYPEf, &etherType) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    /*Set Ale EtherType*/
    if ((ret = reg_field_write(unit, ESW_MEPID_RECORD_GLOBAL_CONTROLr, 
                  ESW_CCMRTHERTYPEf, &etherType) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMEtype_set*/

/* Function Name:
 *      dal_esw_oam_cfmCCMOpcode_get
 * Description:
 *      Get opcode of CFM header.
 * Input:
 *      unit    - unit id
 * Output:
 *      pOpcode - pointer to opcode of CCM header
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMOpcode_get(uint32 unit, uint32 *pOpcode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pOpcode), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL1r, 
                  ESW_CCMOPCODEf, pOpcode) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pOpcode=0x%x", pOpcode);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMOpcode_get*/

/* Function Name:
 *      dal_esw_oam_cfmCCMOpcode_set
 * Description:
 *      Set opcode of CFM header.
 * Input:
 *      unit   - unit id
 *      opcode - opcode of CCM header
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      Default opcode is 0x1
 */
int32
dal_esw_oam_cfmCCMOpcode_set(uint32 unit, uint32 opcode)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, opcode=0x%x", unit, opcode);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL1r, 
                  ESW_CCMOPCODEf, &opcode) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMOpcode_set*/

/* Function Name:
 *      dal_esw_oam_cfmCCMOpcode_get
 * Description:
 *      Get flag of CFM header for specified CFM group.
 * Input:
 *      unit     - unit id
 *      cfm_idx  - CFM index
 * Output:
 *      pCcmFlag - pointer to clag of CCM header
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMFlag_get(uint32 unit, uint32 cfm_idx, uint32 *pCcmFlag)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, cfm_idx=%d", unit, cfm_idx);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((cfm_idx > CFM_MAX_IDX), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pCcmFlag), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL2r, 
                  ESW_CFMFLAGS0f - cfm_idx, pCcmFlag) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pCcmFlag=0x%x", *pCcmFlag);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMFlag_get*/

/* Function Name:
 *      dal_esw_oam_cfmCCMFlag_set
 * Description:
 *      Set flag of CFM header for specified CFM group.
 * Input:
 *      unit    - unit id
 *      cfm_idx - CFM index
 *      ccmFlag - flag of CCM header
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMFlag_set(uint32 unit, uint32 cfm_idx, uint32 ccmFlag)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, cfm_idx=%d, ccmFlag=0x%x", 
                 unit, cfm_idx, ccmFlag);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((cfm_idx > CFM_MAX_IDX), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_CONTROL2r, 
                  ESW_CFMFLAGS0f - cfm_idx, &ccmFlag) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMFlag_get*/

/* Function Name:
 *      dal_esw_oam_cfmCCMInterval_get
 * Description:
 *      Get transmit interval of CCM packets.
 * Input:
 *      unit      - unit id
 * Output:
 *      pInterval - pointer to transmit interval
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      CCM interval is as following
 *      - INTERVAL_3_3_MS
 *      - INTERVAL_10_MS
 *      - INTERVAL_100_MS
 *      - INTERVAL_1_S
 *      - INTERVAL_10_S
 *      - INTERVAL_1_MIN
 *      - INTERVAL_10_MIN
 */
int32
dal_esw_oam_cfmCCMInterval_get(uint32 unit, rtk_oam_ccmInterval_t *pInterval)
{
    int32   ret;
    uint32 val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pInterval), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_field_read(unit, ESW_MEPID_RECORD_GLOBAL_CONTROLr, 
                  ESW_CCMINTERVALf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);

    switch(val)
    {
        case 1:
            *pInterval = INTERVAL_3_3_MS;
            break;
        case 2:
            *pInterval = INTERVAL_10_MS;
            break; 
        case 3:
            *pInterval = INTERVAL_100_MS;
            break;
        case 4:
            *pInterval = INTERVAL_1_S;
            break;
        case 5:
            *pInterval = INTERVAL_10_S;
            break;
        case 6:
            *pInterval = INTERVAL_1_MIN;
            break; 
        case 7:
            *pInterval = INTERVAL_10_MIN;
            break;
        default:
            *pInterval = INTERVAL_END; /*invalid*/
            break;    
    }
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pInterval=%d", pInterval);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMInterval_get*/

/* Function Name:
 *      dal_esw_oam_cfmCCMInterval_set
 * Description:
 *      Set transmit interval of CCM packets.
 * Input:
 *      unit     - unit id
 *      interval - transmit interval
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCMInterval_set(uint32 unit, rtk_oam_ccmInterval_t interval)
{
    int32   ret;
    uint32 val, time;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    switch(interval)
    {
        case INTERVAL_3_3_MS:
            val = 1;
            time = 33; /*1 = 0.1ms*/
            break;
        case INTERVAL_10_MS:
            val = 2;
            time = 100;
            break; 
        case INTERVAL_100_MS:
            val = 3;
            time = 1000;
            break;
        case INTERVAL_1_S:
            val = 4;
            time = 10000;
            break;
        default:
            RT_ERR(RT_ERR_INPUT, (MOD_DAL|MOD_OAM), "Not Support By Asic");
            return RT_ERR_INPUT;    
    }

    OAM_SEM_LOCK(unit);
    
    /*Set ale register*/
    if ((ret = reg_field_write(unit, ESW_MEPID_RECORD_GLOBAL_CONTROLr, 
                  ESW_CCMINTERVALf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    /*Set SPG register*/
    if ((ret = reg_field_write(unit, ESW_SMART_PACKET_GENERATOR_PACKET_TX_INTERVALr, 
                  ESW_TXINTERVALf, &time) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCMInterval_set*/

/* Function Name:
 *      dal_esw_oam_cfmIntfStatus_get
 * Description:
 *      Get interface status of CCM TLV on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pIntfStatus - pointer to interface status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmIntfStatus_get(uint32 unit, rtk_port_t port, uint32 *pIntfStatus)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pIntfStatus), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_SMART_PACKET_GENERATOR_INTERFACE_STATUS_CONTROLr, port, 
              REG_ARRAY_INDEX_NONE, ESW_P_ISTLVVALf, pIntfStatus) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pIntfStatus=%d", *pIntfStatus);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmIntfStatus_get*/

/* Function Name:
 *      dal_esw_oam_cfmIntfStatus_set
 * Description:
 *      Set interface status of CCM TLV on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      intfStatus - interface status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmIntfStatus_set(uint32 unit, rtk_port_t port, uint32 intfStatus)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d, intfStatus=%d", 
                unit, port, intfStatus);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((intfStatus > 7) || (intfStatus == 0), RT_ERR_INPUT);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_SMART_PACKET_GENERATOR_INTERFACE_STATUS_CONTROLr, port, 
               REG_ARRAY_INDEX_NONE, ESW_P_ISTLVVALf, &intfStatus) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmIntfStatus_set*/

/* Function Name:
 *      dal_esw_oam_cfmPortStatus_get
 * Description:
 *      Get port status of CCM TLV on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 * Output:
 *      pPortStatus - pointer to port status
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmPortStatus_get(uint32 unit, rtk_port_t port, uint32 *pPortStatus)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d", unit, port);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((NULL == pPortStatus), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_SMART_PACKET_GENERATOR_PORT_STATUS_CONTROLr, port, 
               REG_ARRAY_INDEX_NONE, ESW_P_PSTLVVALf, pPortStatus) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pPortStatus=%d", *pPortStatus);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmPortStatus_get*/

/* Function Name:
 *      dal_esw_oam_cfmPortStatus_set
 * Description:
 *      Set port status of CCM TLV on specified port.
 * Input:
 *      unit       - unit id
 *      port       - port id
 *      portStatus - port status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmPortStatus_set(uint32 unit, rtk_port_t port, uint32 portStatus)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d, portStatus=%d", 
                unit, port, portStatus);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);
    RT_PARAM_CHK((portStatus != PS_BLOCKED) &&(portStatus != PS_UPED), RT_ERR_INPUT);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_SMART_PACKET_GENERATOR_PORT_STATUS_CONTROLr, port, 
              REG_ARRAY_INDEX_NONE, ESW_P_PSTLVVALf, &portStatus) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);
     
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmPortStatus_set*/

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_del
 * Description:
 *      Delete remote MEP.
 * Input:
 *      unit  - unit id
 *      mepid - remote MEP id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_ENTRY_NOTFOUND   - remote mep not found
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmRemoteMep_del(uint32 unit, uint32 mepid)
{
    int32   ret;
    uint32 index, val;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);

    OAM_SEM_LOCK(unit);

    for(index = 0; index < DAL_ESW_MEPID_ENTRY; index++)
    {
        if(pMepid_info[unit]->mepid[index].valid)
        {
            /*Find entry in the database*/
            if(pMepid_info[unit]->mepid[index].mepid == mepid)
            {
                /*Set to asic chip*/
                val = DISABLED;
                if ((ret = reg_array_field_write(unit, ESW_CFM_MEPID_RECORD_COMPARE_CONTROLr,
                            index, REG_ARRAY_INDEX_NONE, ESW_MEPIDVf, &val) ) != RT_ERR_OK)
                {
                    OAM_SEM_UNLOCK(unit);
                    RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
                    return ret;
                } 

                pMepid_info[unit]->mepid[index].valid = 0;
                OAM_SEM_UNLOCK(unit);
                return RT_ERR_OK;
            }
        }
    }    

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_ENTRY_NOTFOUND;
}   /*end of dal_esw_oam_cfmRemoteMep_del*/

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_add
 * Description:
 *      Add remote MEP.
 * Input:
 *      unit  - unit id
 *      mepid - remote MEP id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmRemoteMep_add(uint32 unit, uint32 mepid)
{
    int32   ret = RT_ERR_FAILED;
    uint32 index, val;
    int32 firstIdx = -1; /*-1: the database is full; other: the first invalid position*/
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);

    /*Search in the database*/
    for(index = 0; index < DAL_ESW_MEPID_ENTRY; index++)
    {
        /*record the first invalid position*/
        if(!pMepid_info[unit]->mepid[index].valid && (firstIdx == -1))
            firstIdx = index;
        
        if(pMepid_info[unit]->mepid[index].valid)
        {
            /*Mepid Already in the Database*/
            if(pMepid_info[unit]->mepid[index].mepid == mepid)
            {
                RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_OAM), "Mepid Already in the Database");
                return RT_ERR_FAILED;
            }
        }
    }

    /*Check the database has space*/
    if(firstIdx == -1)
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DAL|MOD_OAM), "The Database is Full");
        return RT_ERR_ENTRY_NOTFOUND;
    }

    /*Fill he Mepid in the database*/
    OAM_SEM_LOCK(unit);        
    /*Set to asic chip*/
    if ((ret = reg_array_field_write(unit, ESW_CFM_MEPID_RECORD_COMPARE_CONTROLr,
                    firstIdx, REG_ARRAY_INDEX_NONE, ESW_MEPIDf, &mepid) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    
    val = ENABLED;
    if ((ret = reg_array_field_write(unit, ESW_CFM_MEPID_RECORD_COMPARE_CONTROLr,
                  firstIdx, REG_ARRAY_INDEX_NONE, ESW_MEPIDVf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    pMepid_info[unit]->mepid[firstIdx].valid = TRUE;
    pMepid_info[unit]->mepid[firstIdx].mepid = mepid;
    
    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmRemoteMep_del*/

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_get
 * Description:
 *      Get remote MEP Info.
 * Input:
 *      unit  - unit id
 *      index - remote MEP id index
 * Output:
 *      pMepid - the pointer of the remote MEP id
 *      pValid - the pointer of the valid bit
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmRemoteMep_get(uint32 unit, uint32 index, uint32* pMepid, uint32 *pValid)
{  
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, index=%d", unit, index);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    RT_PARAM_CHK((index >= DAL_ESW_MEPID_ENTRY), RT_ERR_OUT_OF_RANGE);
    RT_PARAM_CHK((NULL == pMepid), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pValid), RT_ERR_NULL_POINTER);

    *pValid = pMepid_info[unit]->mepid[index].valid;
    *pMepid = pMepid_info[unit]->mepid[index].mepid;    

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pMepid=%d, pValid=%d", *pMepid, *pValid);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmRemoteMep_get*/

/* Function Name:
 *      dal_esw_oam_cfmRemoteMep_set
 * Description:
 *      Set remote MEP Info.
 * Input:
 *      unit  - unit id
 *      index - remote MEP id index
 *      mepid -  remote MEP id
 *      valid -  the valid bit 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmRemoteMep_set(uint32 unit, uint32 index, uint32 mepid, uint32 valid)
{
    int32   ret = RT_ERR_FAILED;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, index=%d, mepid=0x%x, valid=%d", 
                  unit, index, mepid, valid);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    RT_PARAM_CHK((index >= DAL_ESW_MEPID_ENTRY), RT_ERR_OUT_OF_RANGE);

    /*Fill he Mepid in the database*/
    OAM_SEM_LOCK(unit);        
    /*Set to asic chip*/
    if ((ret = reg_array_field_write(unit, ESW_CFM_MEPID_RECORD_COMPARE_CONTROLr,
                    index, REG_ARRAY_INDEX_NONE, ESW_MEPIDf, &mepid) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }
    
    if ((ret = reg_array_field_write(unit, ESW_CFM_MEPID_RECORD_COMPARE_CONTROLr,
                  index, REG_ARRAY_INDEX_NONE, ESW_MEPIDVf, &valid) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    pMepid_info[unit]->mepid[index].valid = valid;
    pMepid_info[unit]->mepid[index].mepid = mepid;
    
    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmRemoteMep_set*/


/* Function Name:
 *      dal_esw_oam_cfmCCStatus_get
 * Description:
 *      Get continuity check status of remote MEP on specified port.
 * Input:
 *      unit        - unit id
 *      port        - port id
 *      remoteMepid - remote mep id
 * Output:
 *      pRdi        - pointer to RDI of remote mep
 *      pCcStatus   - pointer to status of remote mep aging
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCStatus_get(
    uint32              unit, 
    rtk_port_t          port, 
    uint32              remoteMepid,
    uint32              *pRdi,
    rtk_oam_ccStatus_t  *pCcStatus)
{
    int32   ret;
    uint32 index;
    uint32 found = FALSE;
    uint32 foundIdx=0;
    rtk_enable_t enable;    
    rtk_portmask_t status;    
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, port=%d, remoteMepid=0x%x", 
                 unit, port, remoteMepid);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);

    /* parameter check */
    RT_PARAM_CHK((NULL == pRdi), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((NULL == pCcStatus), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    /*Search in the database*/
    for(index = 0; index < DAL_ESW_MEPID_ENTRY; index++)
    {        
        if(pMepid_info[unit]->mepid[index].valid)
        {
            /*Mepid Already in the Database*/
            if(pMepid_info[unit]->mepid[index].mepid == remoteMepid)
            {
                found = TRUE;
                foundIdx = index;
                break;
            }
        }
    }

    if(found == FALSE)
    {
        RT_ERR(RT_ERR_ENTRY_NOTFOUND, (MOD_DAL|MOD_OAM), "Not Found the remote mepid in the database");
        return RT_ERR_ENTRY_NOTFOUND;
    }

    OAM_SEM_LOCK(unit);    

    /*Get RDI Info*/
    if ((ret = reg_array_field_read(unit, ESW_CFM_MEPID_RECORD_COMPARE_CONTROLr,
                    foundIdx, REG_ARRAY_INDEX_NONE, ESW_RDIf, pRdi) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }

    OAM_SEM_UNLOCK(unit);

    if((ret = dal_esw_oam_cfmMepEnable_get(unit,  port, &enable)) != RT_ERR_OK)
    {
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "The Mepid is not enable int the port");
        return ret;
    }  

    OAM_SEM_LOCK(unit);  

    /*Get from asic chip*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_CCM_RECORD_CONTROL2r, 
                  port, REG_ARRAY_INDEX_NONE, ESW_MEPIDSTATUSf, &status.bits[0]) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }  
    
    OAM_SEM_UNLOCK(unit);

    if(BITMAP_IS_SET(status.bits, foundIdx))
        *pCcStatus = CC_STATUS_AGEOUT;
    else
        *pCcStatus = CC_STATUS_NOT_AGE;

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pRdi=%d, pCcStatus=%d", 
                 *pRdi, *pCcStatus);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCStatus_get*/

/* Function Name:
 *      dal_esw_oam_cfmCCStatus_reset
 * Description:
 *      Reset continuity check status of all remote mep on specified port.
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmCCStatus_reset(uint32 unit, rtk_port_t port)
{
    uint32  val;
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /*Clear CCM Information of the port*/
    val = BITMASK_16B;
    if ((ret = reg_array_field_write(unit, ESW_PORT_CCM_RECORD_CONTROL2r, 
                  port, REG_ARRAY_INDEX_NONE, ESW_MEPIDINFOCLRf, &val) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmCCStatus_reset*/

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_get
 * Description:
 *      Get enable status of auto reply for CFM loopback packets.
 * Input:
 *      unit    - unit id
 *      port    - port id
 * Output:
 *      pEnable - pointer to enable status 
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmLoopbackReplyEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_read(unit, ESW_PORT_CFM_LOOPBACK_CONTROLr, 
                  port, REG_ARRAY_INDEX_NONE, ESW_PAUTOLBRENf, pEnable) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pEnable=%d", pEnable);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmLoopbackReplyEnable_get*/

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_set
 * Description:
 *      Set enable status of auto reply for CFM loopback packets.
 * Input:
 *      unit   - unit id
 *      port   - port id
 *      enable - enable status 
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmLoopbackReplyEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d, enable=%d", unit, enable);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((enable >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK(!HAL_IS_ETHER_PORT(unit, port), RT_ERR_PORT_ID);

    OAM_SEM_LOCK(unit);
    
    /* get value from CHIP*/
    if ((ret = reg_array_field_write(unit, ESW_PORT_CFM_LOOPBACK_CONTROLr, 
                  port, REG_ARRAY_INDEX_NONE, ESW_PAUTOLBRENf, &enable) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
        
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmLoopbackReplyEnable_set*/

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_get
 * Description:
 *      Get control of CFM loopback action.
 * Input:
 *      unit  - unit id
 * Output:
 *      pCtrl - pointer to loopback control
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmLoopbackReplyCtrl_get(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pCtrl), RT_ERR_NULL_POINTER);

    OAM_SEM_LOCK(unit);
    
    /*Get Loopback reply ivid control bit*/
    if ((ret = reg_field_read(unit, ESW_CFM_LOOPBACK_CONTROLr, 
                  ESW_LBRKEEPITAGORIGf, &pCtrl->keep_innerTag) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    /*Get Loopback reply ovid control bit*/
    if ((ret = reg_field_read(unit, ESW_CFM_LOOPBACK_CONTROLr, 
                  ESW_LBRKEEPOTAGORIGf, &pCtrl->keep_outerTag) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pCtrl->keep_innerTag=%d, pCtrl->keep_outerTag=%d", 
                    pCtrl->keep_innerTag, pCtrl->keep_outerTag);
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmLoopbackReplyCtrl_get*/

/* Function Name:
 *      dal_esw_oam_cfmLoopbackReplyEnable_set
 * Description:
 *      Set control of CFM loopback action.
 * Input:
 *      unit  - unit id
 *      pCtrl - loopback control
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_NOT_INIT         - The module is not initial
 *      RT_ERR_UNIT_ID          - invalid unit id
 *      RT_ERR_PORT_ID          - invalid port id
 *      RT_ERR_NULL_POINTER     - input parameter may be null pointer
 *      RT_ERR_INPUT            - invalid input parameter
 * Note:
 *      None
 */
int32
dal_esw_oam_cfmLoopbackReplyCtrl_set(uint32 unit, rtk_oam_cfmLoopbackCtrl_t *pCtrl)
{
    int32   ret;
    
    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "unit=%d", unit);
    
    /* check Init status */
    RT_INIT_CHK(oam_init[unit]);
    
    /* parameter check */
    RT_PARAM_CHK((NULL == pCtrl), RT_ERR_NULL_POINTER);
    RT_PARAM_CHK((pCtrl->keep_innerTag >= RTK_ENABLE_END), RT_ERR_INPUT);
    RT_PARAM_CHK((pCtrl->keep_outerTag>= RTK_ENABLE_END), RT_ERR_INPUT);

    RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_OAM), "pCtrl->keep_innerTag=%d, pCtrl->keep_outerTag=%d", 
                    pCtrl->keep_innerTag, pCtrl->keep_outerTag);

    OAM_SEM_LOCK(unit);
    
    /*Get Loopback reply ivid control bit*/
    if ((ret = reg_field_write(unit, ESW_CFM_LOOPBACK_CONTROLr, 
                  ESW_LBRKEEPITAGORIGf, &(pCtrl->keep_innerTag)) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    } 

    /*Get Loopback reply ovid control bit*/
    if ((ret = reg_field_write(unit, ESW_CFM_LOOPBACK_CONTROLr, 
                  ESW_LBRKEEPOTAGORIGf, &(pCtrl->keep_outerTag)) ) != RT_ERR_OK)
    {
        OAM_SEM_UNLOCK(unit);
        RT_ERR(ret, (MOD_DAL|MOD_OAM), "");
        return ret;
    }   

    OAM_SEM_UNLOCK(unit);   
    
    return RT_ERR_OK;
}   /*end of dal_esw_oam_cfmLoopbackReplyCtrl_set*/

