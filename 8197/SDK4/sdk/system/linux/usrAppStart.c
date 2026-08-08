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
 * $Revision: 8992 $
 * $Date: 2010-04-12 14:18:51 +0800 (Mon, 12 Apr 2010) $
 *
 * Purpose : user main function for Linux
 *
 * Feature : 1) Init RTK Layer API
 *           2) Create SDK Main Thread
 *           3) Invoke SDK Diag Shell
 */

/*
 * Include Files
 */
#include <common/rt_error.h>
#include <common/debug/rt_log.h>
#include <ioal/ioal_init.h>
#include <drv/nic/probe.h>
#include <drv/nic/nic.h>
#include <osal/lib.h>
#include <osal/memory.h>
#include <osal/print.h>
#include <rtk/init.h>

/* 
 * Symbol Definition     
 */


/* 
 * Data Declaration      
 */

/* 
 * Function Declaration  
 */
#ifdef CONFIG_SDK_APP_DIAG
extern int diag_main(int argc, char** argv);
#endif

/* Function Name:
 *      sdk_main
 * Description:
 *      sdk main function
 * Input:
 *      data - may hold pointer or word
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None.
 */
void sdk_main(void)
{
#ifdef CONFIG_SDK_APP_DIAG

    /* Run diag shell */
    osal_printf("Start to run Diag Shell....\n\n"); 
    
    diag_main(0, NULL);
#else
    osal_printf("Please add your application here....\n\n");
#endif
} /* end of sdk_main */

/* Function Name:
 *      main
 * Description:
 *      the normal application entry point
 * Input:
 *      None
 * Output:
 *      None
 * Return:
 *      None
 * Note:
 *      None.
 */
int main(int argc, char** argv)
{
    int32 ret = RT_ERR_FAILED;

    /* Init RTK Layer API */
    osal_printf("Init RTK Layer....");
   
    RT_ERR_CHK(rt_log_init(), ret);
    RT_ERR_CHK(ioal_init(DEFAULT_INIT_UNIT_ID), ret);
    RT_ERR_CHK(rtk_init(DEFAULT_INIT_UNIT_ID), ret);
     
    osal_printf("OK\n"); 

#ifdef CONFIG_SDK_DRIVER_NIC_USER_MODE
    /* Probe the nic */
    RT_ERR_CHK(nic_probe(DEFAULT_INIT_UNIT_ID), ret);  
#endif
    
    sdk_main();
    
    return RT_ERR_OK;
} /* end of main */

