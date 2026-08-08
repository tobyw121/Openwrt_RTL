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
 * $Revision: 6401 $
 * $Date: 2009-10-14 16:03:12 +0800 (Wed, 14 Oct 2009) $
 *
 * Purpose : Smart Switch RTK API mapper table
 *
 * Feature : 
 *
 */

/*
 * Include Files
 */
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <dal/dal_mapper.h>	 


/* 
 * Symbol Definition
 */

/*
 * Data Declaration
 */
extern dal_mapper_t dal_ssw_mapper;

/*
 * Macro Declaration
 */ 

/*
 * Function Declaration
 */


/* Module Name    :  */

/* Function Name: 
 *      dal_ssw_init
 * Description: 
 *      Initilize DAL of smart switch 
 * Input:  
 *      unit    - unit id
 * Output: 
 *      None 
 * Return: 
 *      RT_ERR_FAILED   - initialize fail
 *      RT_ERR_OK       - initialize success
 * Note: 
 *      RTK must call this function before do other kind of action.
 */ 
extern int32 dal_ssw_init(uint32 unit);
