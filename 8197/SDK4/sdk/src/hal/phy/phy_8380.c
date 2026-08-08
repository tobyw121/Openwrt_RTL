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
 * $Revision: 32092 $
 * $Date: 2012-08-24 15:55:16 +0800 (Fri, 24 Aug 2012) $
 *
 * Purpose : PHY 8328 intra serdes Driver APIs.
 *
 * Feature : PHY 8328 intra serdes Driver APIs
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <hal/phy/phydef.h>
#include <hal/phy/phy_common.h>
#include <hal/phy/phy_8380.h>

#include <osal/time.h>
#include <hal/common/halctrl.h>
#include <hal/phy/phydef.h>


#include <hal/mac/reg.h>
#include <hal/chipdef/maple/rtk_maple_reg_struct.h>

#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/debug/rt_log.h>

#include <ioal/mem32.h>

/*
 * Symbol Definition
 */

/*
 * Data Declaration
 */
rt_phydrv_t phy_8380_serdes_ge =
{
    RT_PHYDRV_RTL8380_SERDES_GE,
    phy_8380_init,
    phy_8380_media_get,
    (int32 (*)(uint32, rtk_port_t, rtk_port_media_t))phy_common_unavail,
    phy_8380_autoNegoEnable_get,
    phy_8380_autoNegoEnable_set,
    phy_8380_autoNegoAbility_get,
    phy_8380_autoNegoAbility_set,
    phy_8380_duplex_get,
    phy_8380_duplex_set,
    phy_8380_speed_get,
    phy_8380_speed_set,
    phy_8380_enable_set,
    (int32 (*)(uint32, rtk_port_t, rtk_rtctResult_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_mode_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_mode_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_crossOver_status_t *))phy_common_unavail,       
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_fiber_media_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, uint32))phy_common_unavail,
    phy_8380_gigaLiteEnable_get,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,      
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
    phy_8380_fiberOAMLoopBack_set,
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
    (int32 (*)(uint32, rtk_port_t, rtk_port_masterSlave_t *, rtk_port_masterSlave_t *))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_port_masterSlave_t))phy_common_unavail,
    (int32 (*)(uint32, rtk_port_t, rtk_enable_t))phy_common_unavail,
}; /* end of phy_8380_serdes_ge */


/* Function Name:
 *      phy_8380_reset
 * Description:
 *      Reset serdes
 * Input:
 *      unit          - unit id
 *      port          - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
static int32
_phy_8380_reset(uint32 unit, rtk_port_t port)
{
    uint32 serdes_reg1;
    uint32 serdes_reg2;

    uint32 serdes_val;

    /*Serdes Reset*/
    if (24 == port)
    {
        serdes_reg1 = MAPLE_SDS4_REG0r;
        serdes_reg2 = MAPLE_SDS4_REG3r;
    }
    else  if (26 == port)
    {
        serdes_reg1 = MAPLE_SDS5_REG0r;
        serdes_reg2 = MAPLE_SDS5_REG3r;
    }
    else
	 return RT_ERR_PORT_ID;

     /*Analog Reset*/
    serdes_val = 0;
    reg_field_write(unit, serdes_reg1, MAPLE_SDS_EN_RXf, &serdes_val);
    reg_field_write(unit, serdes_reg1, MAPLE_SDS_EN_TXf, &serdes_val);

    serdes_val = 1;
    reg_field_write(unit, serdes_reg1, MAPLE_SDS_EN_RXf, &serdes_val);
    reg_field_write(unit, serdes_reg1, MAPLE_SDS_EN_TXf, &serdes_val);

    /*Digital Reset*/
    serdes_val = 1;
    reg_field_write(unit, serdes_reg2, MAPLE_SOFT_RSTf, &serdes_val);

    serdes_val = 0;
    reg_field_write(unit, serdes_reg2, MAPLE_SOFT_RSTf, &serdes_val);

    return RT_ERR_OK;
}   /* end of _phy_8380_reset */



/* Function Name:
 *      phy_8380_init
 * Description:
 *      Initialize 8380 MAC internal serdes PHY.
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
phy_8380_init(uint32 unit, rtk_port_t port)
{
    uint32  reg0,reg1;
    uint32 val;
    int32   ret;

    if (24 == port)
    {
        reg0 = MAPLE_SDS4_REG4r;
        reg1 = MAPLE_SDS4_FIB_EXT_REG16r;
    }
    else if (26 == port)
    {
        reg0 = MAPLE_SDS5_REG4r;
        reg1 = MAPLE_SDS5_FIB_EXT_REG16r;
    }
    else
        return RT_ERR_PORT_ID;

    /* auto-nego state can link with link partner is force mode */
    val = 1;
    if ((ret = reg_field_write(unit, reg0, MAPLE_CFG_EN_LINK_FIB1Gf, &val)) != RT_ERR_OK)
        return ret;

    /* disable EEE */
    val = 0;
    if ((ret = reg_field_write(unit, reg1, MAPLE_EEE_RSG_FIB1Gf, &val)) != RT_ERR_OK)
        return ret;

    if ((ret = reg_field_write(unit, reg1, MAPLE_EEE_STD_FIB1Gf, &val)) != RT_ERR_OK)
        return ret;

    if ((ret = reg_field_write(unit, reg1, MAPLE_C1_PWRSAV_EN_FIB1Gf, &val)) != RT_ERR_OK)
        return ret;

    if ((ret = reg_field_write(unit, reg1, MAPLE_C2_PWRSAV_EN_FIB1Gf, &val)) != RT_ERR_OK)
        return ret;

    if ((ret = reg_field_write(unit, reg1, MAPLE_EEE_QUIET_FIB1Gf, &val)) != RT_ERR_OK)
        return ret;
    
    return RT_ERR_OK;
} /* end of phy_8328_init */

/* Function Name:
 *      phy_8380_media_get
 * Description:
 *      Get 8380 serdes PHY media type.
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
 *      1. media type is PORT_MEDIA_FIBER
 */
int32
phy_8380_media_get(uint32 unit, rtk_port_t port, rtk_port_media_t *pMedia)
{
    RT_PARAM_CHK((NULL == pMedia), RT_ERR_NULL_POINTER);

    *pMedia = PORT_MEDIA_FIBER;
    return RT_ERR_OK;
} /* end of phy_8380_media_get */

/* Function Name:
 *      phy_8380_gigaLiteEnable_get
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
 *      1. The RTL8380 is not supported the per-port Giga Lite feature.
 */
int32
phy_8380_gigaLiteEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    *pEnable = DISABLED;
    return RT_ERR_OK;
} /* end of phy_8380_gigaLiteEnable_get */


/* Function Name:
 *      phy_8380_speed_get
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
phy_8380_speed_get(uint32 unit, rtk_port_t port, uint32 *pSpeed)
{
    int32 ret;
    uint32 value;

    RT_PARAM_CHK((NULL == pSpeed), RT_ERR_NULL_POINTER);

    if(24 == port)
   {
         if ((ret = reg_read(unit, MAPLE_SDS4_FIB_REG0r
                             , &value)) != RT_ERR_OK)
         {
             RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
             return ret;
         }
   }
   else if(26 == port)
   {
        if ((ret = reg_read(unit, MAPLE_SDS5_FIB_REG0r
                            , &value)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
   }
   else
   {
         return RT_ERR_FAILED;
   }

    if ((((value >> 6) & 0x1) == 0) && (((value >> 13) & 0x1) == 1))
        *pSpeed = PORT_SPEED_100M;
    else
        *pSpeed = PORT_SPEED_1000M;

    return RT_ERR_OK;
} /* end of phy_8380_speed_get */

/* Function Name:
 *      phy_8380_speed_set
 * Description:
 *      Set speed mode status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      speed         - link speed status 10/100/1000
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK                 - OK
 *      RT_ERR_FAILED             - invalid parameter
 *      RT_ERR_CHIP_NOT_SUPPORTED - copper media chip is not supported Force-1000
 * Note:
 *      None
 */
int32
phy_8380_speed_set(uint32 unit, rtk_port_t port, uint32 speed)
{
    int32 ret;
    uint32 reg_idx;
    uint32 reg_val;

   if(24 == port)
   {
	reg_idx = MAPLE_SDS4_FIB_REG0r;
   }
   else if(26 == port)
   {
	reg_idx = MAPLE_SDS5_FIB_REG0r;
   }
   else
   {
         return RT_ERR_PORT_ID;
   }

    if ((ret = reg_read(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    reg_val &= ~((1UL<<6) | (1UL<<13));
    reg_val |= (((speed & 2)>>1) << 6) | ((speed & 1) << 13);

    if ((ret = reg_write(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

	_phy_8380_reset(unit, port);

    return RT_ERR_OK;
} /* end of phy_8380_speed_set */

/* Function Name:
 *      phy_8380_duplex_get
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
phy_8380_duplex_get(uint32 unit, rtk_port_t port, uint32 *pDuplex)
{
    int32 ret;
    uint32 value;

    RT_PARAM_CHK((NULL == pDuplex), RT_ERR_NULL_POINTER);

    if(24 == port)
    {
        if ((ret = reg_read(unit, MAPLE_SDS4_FIB_REG0r
                            , &value)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
   }
   else if(26 == port)
   {
        if ((ret = reg_read(unit, MAPLE_SDS5_FIB_REG0r
                            , &value)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
   }
   else
   {
         return RT_ERR_FAILED;
   }

    if (((value >> 8) & 0x1) == 1)
        *pDuplex = PORT_FULL_DUPLEX;
    else
        *pDuplex = PORT_HALF_DUPLEX;

    return RT_ERR_OK;
} /* end of phy_8380_duplex_get */

/* Function Name:
 *      phy_8380_duplex_set
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
phy_8380_duplex_set(uint32 unit, rtk_port_t port, uint32 duplex)
{
    int32 ret;
    uint32 reg_idx;
    uint32 reg_val;

   if(24 == port)
   {
	reg_idx = MAPLE_SDS4_FIB_REG0r;
   }
   else if(26 == port)
   {
	reg_idx = MAPLE_SDS5_FIB_REG0r;
   }
   else
   {
         return RT_ERR_PORT_ID;
   }

    if ((ret = reg_read(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if (PORT_HALF_DUPLEX == duplex)
        reg_val &= ~(0x1<<8);
    else
        reg_val |= (0x1<<8);

    if ((ret = reg_write(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    _phy_8380_reset(unit, port);

    return RT_ERR_OK;
} /* end of phy_8380_duplex_set */

/* Function Name:
 *      phy_8380_flowCtrl_get
 * Description:
 *      Get flow control status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable - pointer to flow control status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8380_flowCtrl_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;
    uint32 value;

    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if(24 == port)
    {
        if ((ret = reg_read(unit, MAPLE_SDS4_FIB_REG4r
                            , &value)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
    }
    else if(26 == port)
    {
        if ((ret = reg_read(unit, MAPLE_SDS5_FIB_REG4r
                            , &value)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
    }
    else
    {
         return RT_ERR_FAILED;
    }

    if (((value >> 7) & 0x3) == 0)
        *pEnable = DISABLED;
    else
        *pEnable = ENABLED;
    return ret;

    return RT_ERR_OK;
} /* end of phy_8380_flowCtrl_get */

/* Function Name:
 *      phy_8380_flowCtrl_set
 * Description:
 *      Set flow control status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - flow control status
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8380_flowCtrl_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 reg_val;
    uint32 reg_idx;

    if(24 == port)
   {
	reg_idx = MAPLE_SDS4_FIB_REG4r;
   }
   else if(26 == port)
   {
	reg_idx = MAPLE_SDS5_FIB_REG4r;
   }
   else
   {
         return RT_ERR_PORT_ID;
   }

    if ((ret = reg_read(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    if (DISABLED == enable)
        reg_val &= ~(0x3<<7);
    else
        reg_val |= (0x3<<7);

    if ((ret = reg_write(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    return RT_ERR_OK;

} /* end of phy_8380_flowCtrl_set */



/* Function Name:
 *      phy_8380_enable_set
 * Description:
 *      Set interface status of the specific port
 * Input:
 *      unit          - unit id
 *      port          - port id
 *      enable        - admin configuration of PHY interface
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8380_enable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    uint32 serdes_reg0;
    uint32 serdes_mode;

    uint32 serdes_reg1;
    uint32 serdes_pdwn;

    /*Check PortId*/
    if ((24 != port) && (26 != port))
        return RT_ERR_PORT_ID;

     /*Serdes Mode*/
     serdes_reg0 = MAPLE_SDS_MODE_SELr;

      if (DISABLED == enable)
          serdes_mode = 0x9;
      else
          serdes_mode = 0x4;

      if (24 == port)
      	   reg_field_write(unit, serdes_reg0, MAPLE_SDS4_MODE_SELf, &serdes_mode);
      else
      	   reg_field_write(unit, serdes_reg0, MAPLE_SDS5_MODE_SELf, &serdes_mode);

      /*Always Power Up because u-boot may close it*/
      if (24 == port)
	   serdes_reg1 = MAPLE_SDS4_FIB_REG0r;
      else
	   serdes_reg1 = MAPLE_SDS5_FIB_REG0r;

      serdes_pdwn = 0x0;
      reg_field_write(unit, serdes_reg1, MAPLE_CFG_FIB_PDOWNf, &serdes_pdwn);

	_phy_8380_reset(unit, port);

    return RT_ERR_OK;
} /* end of phy_8380_enable_set */


/* Function Name:
 *      phy_8380_autoNegoEnable_get
 * Description:
 *      Get autonegotiation enable status of the specific port
 * Input:
 *      unit - unit id
 *      port - port id
 * Output:
 *      pEnable -   auto negotiation status
 * Return:
 *      RT_ERR_OK     - OK
 *      RT_ERR_FAILED - invalid parameter
 * Note:
 *      None
 */
int32
phy_8380_autoNegoEnable_get(uint32 unit, rtk_port_t port, rtk_enable_t *pEnable)
{
    int32 ret;
    uint32 value;

    RT_PARAM_CHK((NULL == pEnable), RT_ERR_NULL_POINTER);

    if(24 == port)
   {
         if ((ret = reg_read(unit, MAPLE_SDS4_FIB_REG0r
                             , &value)) != RT_ERR_OK)
         {
             RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
             return ret;
         }
   }
   else if(26 == port)
   {
        if ((ret = reg_read(unit, MAPLE_SDS5_FIB_REG0r
                            , &value)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }
   }
   else
   {
         return RT_ERR_FAILED;
   }

   if (((value >> 12) & 0x1) == 1)
            *pEnable = ENABLED;
        else
            *pEnable = DISABLED;


    return RT_ERR_OK;
} /* end of phy_8380_autoNegoEnable_get */


/* Function Name:
 *      phy_8380_autoNegoEnable_set
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
phy_8380_autoNegoEnable_set(uint32 unit, rtk_port_t port, rtk_enable_t enable)
{
    int32 ret;
    uint32 reg_idx;
    uint32 reg_val;

   if(24 == port)
   {
	reg_idx = MAPLE_SDS4_FIB_REG0r;
   }
   else if(26 == port)
   {
	reg_idx = MAPLE_SDS5_FIB_REG0r;
   }
   else
   {
         return RT_ERR_PORT_ID;
   }

    if ((ret = reg_read(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }


   if (DISABLED == enable)
   {
       reg_val &= ~(1UL<<12);
   }
   else
   {
       reg_val |= (1UL<<12);
   }


    if ((ret = reg_write(unit, reg_idx
                        , &reg_val)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    return RT_ERR_OK;
} /* end of phy_8380_autoNegoEnable_set */

/* Function Name:
 *      phy_8380_autoNegoAbility_get
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
phy_8380_autoNegoAbility_get(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    int32 ret;
    uint32 reg_idx;
    uint32  phy_data4;

    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    osal_memset(pAbility, 0, sizeof(rtk_port_phy_ability_t));

    /* Fiber port read from intra serdes registers, not PHY */
   if(24 == port)
   {
	reg_idx = MAPLE_SDS4_FIB_REG4r;
   }
   else if(26 == port)
   {
	reg_idx = MAPLE_SDS5_FIB_REG4r;
   }
   else
   {
         return RT_ERR_PORT_ID;
   }

    if ((ret = reg_read(unit, reg_idx
                    , &phy_data4)) != RT_ERR_OK)
    {
        RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
        return ret;
    }

    pAbility->FC = (phy_data4 & _1000BaseX_Pause_R4_MASK) >> _1000BaseX_Pause_R4_OFFSET;
    pAbility->AsyFC = (phy_data4 & _1000BaseX_AsymmetricPause_R4_MASK) >> _1000BaseX_AsymmetricPause_R4_OFFSET;
    pAbility->Half_10 = 0;
    pAbility->Full_10 = 0;
    pAbility->Half_100 = 0;
    pAbility->Full_100 = 0;
    pAbility->Half_1000 = (phy_data4 & _1000BaseX_HalfDuplex_R4_MASK) >> _1000BaseX_HalfDuplex_R4_OFFSET;
    pAbility->Full_1000 = (phy_data4 & _1000BaseX_FullDuplex_R4_MASK) >> _1000BaseX_FullDuplex_R4_OFFSET;

    return RT_ERR_OK;

} /* end of phy_8380_autoNegoAbility_get */

/* Function Name:
 *      phy_8380_autoNegoAbility_set
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
phy_8380_autoNegoAbility_set(uint32 unit, rtk_port_t port, rtk_port_phy_ability_t *pAbility)
{
    rtk_enable_t    enable;
    uint32          reg_idx;
    uint32		 phy_data0;
    uint32		 phy_data4;
    int32           ret;

    RT_PARAM_CHK((NULL == pAbility), RT_ERR_NULL_POINTER);

    if(24 == port)
    {
 	reg_idx = MAPLE_SDS4_FIB_REG4r;
    }
    else if(26 == port)
    {
 	reg_idx = MAPLE_SDS5_FIB_REG4r;
    }
    else
    {
          return RT_ERR_PORT_ID;
    }


     if ((ret = reg_read(unit, reg_idx
                     , &phy_data4)) != RT_ERR_OK)
     {
         RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
         return ret;
     }

     phy_data4 &= ~(0xF << 5);
     phy_data4 |= (pAbility->AsyFC << _1000BaseX_AsymmetricPause_R4_OFFSET) | (pAbility->FC << _1000BaseX_Pause_R4_OFFSET);
     phy_data4 |= (pAbility->Half_1000 << _1000BaseX_HalfDuplex_R4_OFFSET) | (pAbility->Full_1000 << _1000BaseX_FullDuplex_R4_OFFSET);

     if ((ret = reg_write(unit, reg_idx
                     , &phy_data4)) != RT_ERR_OK)
     {
         RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
         return ret;
     }

      /* get value from CHIP*/
     if ((ret = phy_8380_autoNegoEnable_get(unit, port, &enable)) != RT_ERR_OK)
     	return ret;


     if (ENABLED == enable)
     {
        if(24 == port)
        {
        	reg_idx = MAPLE_SDS4_FIB_REG0r;
        }
        else if(26 == port)
        {
        	reg_idx = MAPLE_SDS5_FIB_REG0r;
        }
        else
        {
              return RT_ERR_PORT_ID;
        }

        if ((ret = reg_read(unit, reg_idx
                        , &phy_data0)) != RT_ERR_OK)
        {
            RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
            return ret;
        }

         phy_data0 |= (1 << 9);

         if ((ret = reg_write(unit, reg_idx
                             , &phy_data0)) != RT_ERR_OK)
         {
             RT_DBG(LOG_DEBUG, (MOD_DAL|MOD_PORT), "Error Code: 0x%X", ret);
             return ret;
         }

     }

    return RT_ERR_OK;
} /* end of phy_8380_autoNegoAbility_set */


/* Function Name:
 *      phy_8380_fiberOAMLoopBack_set
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
phy_8380_fiberOAMLoopBack_set(uint32 unit, rtk_port_t port,
    rtk_enable_t enable)
{
    uint32  reg_val;
    int32    ret;

    ret = RT_ERR_OK;
    if((port != 24) && (port != 26))
          return RT_ERR_PORT_ID;

    if(enable == ENABLED)
    {
        /*Enable Loopback*/
        if(24 == port)
        {
            ioal_mem32_read(unit, 0xef80, &reg_val);
            reg_val |= (1UL<<4);
            ioal_mem32_write(unit, 0xef80, reg_val);
        }
        else
        {
            ioal_mem32_read(unit, 0xf180, &reg_val);
            reg_val |= (1UL<<4);
            ioal_mem32_write(unit, 0xf180, reg_val);
        }       

        /*Delay until PHY Linkup*/
        osal_time_usleep(10 * 1000); /* delay 10mS */
    }
    else
    {
        /*Disable Loopback*/
        if(24 == port)
        {
            ioal_mem32_read(unit, 0xef80, &reg_val);
            reg_val &= ~(1UL<<4);
            ioal_mem32_write(unit, 0xef80, reg_val);
        }
        else
        {
            ioal_mem32_read(unit, 0xf180, &reg_val);
            reg_val &= ~(1UL<<4);
            ioal_mem32_write(unit, 0xf180, reg_val);
        }
    }
    return ret;
}   /* end of phy_8380_fiberOAMLoopBack_set */



