/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 39046 $
 * $Date: 2013-04-30 21:54:21 +0800 (Tue, 30 Apr 2013) $
 *
 * Purpose : Monitor workaround status of each port 
 *
 * Feature : 
 *
 */
#ifndef __DAL_WA_MON_H__ 
#define __DAL_WA_MON_H__

/*  
 * Include Files 
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
//#include <dal/dal_mapper.h>

/* 
 * Symbol Definition 
 */

/* 
 * Data Declaration 
 */


/*
 * Macro Definition
 */

/* 
 * Function Declaration 
 */

/* Module Name : */

/* Function Name: 
 *      dal_waMon_init
 * Description: 
 *      Initial Workaround Monitor component
 * Input:  
 *      None
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED - initialize fail
 *      RT_ERR_OK     - initialize success
 * Note: 
 *      None
 */ 
extern int32
dal_waMon_init(void);

/* Function Name: 
 *      dal_waMon_enable
 * Description: 
 *      Enable workaround monitor thread
 * Input:  
 *      scan_interval_us        - scan interval in us.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_OUT_OF_RANGE - scan interval is too small
 * Note:
 *      When enable workaround monitor thread
 */ 
extern int32
dal_waMon_enable(uint32 scan_interval_us);

/* Function Name: 
 *      dal_waMon_disable
 * Description: 
 *      Disable workaround scan thread
 * Input:  
 *      None.
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note: 
 *      When disable workaround monitor thread
 */ 
extern int32
dal_waMon_disable(void);

/* Function Name: 
 *      dal_waMon_phyReconfig_register
 * Description: 
 *      Register callback function for PHY need to reconfigure notification 
 * Input:  
 *      phyReconfig_callback    - callback function for link change
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 *      RT_ERR_NULL_POINTER - input parameter may be null pointer
 * Note: 
 *      
 */ 
extern int32 
dal_waMon_phyReconfig_register(rtk_port_phyReconfig_callback_t phyReconfig_callback);


/* Function Name: 
 *      dal_waMon_phyReconfig_unregister
 * Description: 
 *      UnRegister callback function for PHY need to reconfigure notification 
 * Input:  
 *      phyReconfig_callback    - callback function for link change
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 *      RT_ERR_NOT_INIT - The module is not initial
 * Note: 
 *      
 */ 
extern int32 
dal_waMon_phyReconfig_unregister(void);

#endif /* __DAL_WA_MON_H__ */
