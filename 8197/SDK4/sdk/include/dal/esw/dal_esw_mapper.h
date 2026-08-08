/*
 * Copyright(c) Realtek Semiconductor Corporation, 2008
 * All rights reserved.
 *
 * $Revision: 10406 $
 * $Date: 2010-06-23 19:26:54 +0800 (Wed, 23 Jun 2010) $
 *
 * Purpose : Enterprise Switch RTK API mapper table
 *
 * Feature : 
 *
 */

#ifndef __DAL_ESW_MAPPER_H__
#define __DAL_ESW_MAPPER_H__

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
extern dal_mapper_t dal_esw_mapper;

/*
 * Macro Declaration
 */ 

/*
 * Function Declaration
 */


/* Module Name    :  */

/* Function Name: 
 *      dal_esw_init
 * Description: 
 *      Initilize DAL of enterprise switch 
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
extern int32
dal_esw_init(uint32 unit);

#endif  /* __DAL_ESW_MAPPER_H__ */
