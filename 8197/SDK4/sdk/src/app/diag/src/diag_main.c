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
 * $Revision: 27613 $
 * $Date: 2012-03-30 17:33:46 +0800 (Fri, 30 Mar 2012) $
 *
 * Purpose : Define diag shell main function.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) main function.
 */

/*  
 * Include Files 
 */
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <common/rt_autoconf.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser.h>
#include <parser/cparser_priv.h>
#include <parser/cparser_token.h>
#include <parser/cparser_tree.h>
#if defined(CONFIG_X86_I2C)
#include <io.h>
#include <I2Clib.h>
#include <rtk/init.h>
#endif

#define DIAG_HOSTNAME_DEFAULT        "RTK.0> "    /* Default hostname. */

/* 
 * Function Declaration 
 */
int
diag_main(int argc,	char** argv)
{
    cparser_t parser;



#if defined(CONFIG_X86_I2C)

    /*check if io.dll exists, set up x86 printport as i2c */
    if(LoadIODLL())
    {
        printf(" - Load io.dll error");
        return 0;
    }
    I2C_PIN_DEF(3,0,4,1,2);  
    MDC_PIN_DEF(3,0,4,1,2);
    PrintPortPinDefine(3,0,4,1,2,0);
    
    /*rtk driver init*/
    diag_util_printf("Init RTK Driver ....");

    if(RT_ERR_OK != rtk_init(DEFAULT_INIT_UNIT_ID))
        diag_util_printf("FAIL\n");
    else
        diag_util_printf("OK\n");
    

#endif

    parser.cfg.root = &cparser_root;
    parser.cfg.ch_complete = '\t';
    /* 
     * Instead of making sure the terminal setting of the target and 
     * the host are the same. ch_erase and ch_del both are treated
     * as backspace.
     */
    parser.cfg.ch_erase = '\b';
    parser.cfg.ch_del = 127;
    parser.cfg.ch_help = '?';
    parser.cfg.flags = 0;
    strcpy(parser.cfg.prompt, DIAG_HOSTNAME_DEFAULT);
    diag_om_set_chip_id(DIAG_OM_CHIP_ID_DEFAULT);
    if (diag_om_set_deviceInfo(DIAG_OM_CHIP_ID_DEFAULT) != RT_ERR_OK)
    {
        diag_util_printf("diag_main: set deviceInfo error.\n");
        return -1;
    }
    
    parser.cfg.fd = STDOUT_FILENO;
    
    cparser_io_config(&parser);
    
    /* Initialization */
    if (CPARSER_OK != cparser_init(&parser.cfg, &parser)) {
        diag_util_printf("Fail to initialize parser.\n");
	return -1;
    }
    
    /* Main command loop */
    cparser_run(&parser);

	diag_util_printf("\n");
    
	return 0;
} /* end of diag_main */

#if (defined(CONFIG_SDK_KERNEL_LINUX) && !defined(CONFIG_SDK_MODEL_MODE_USER) && !defined(CONFIG_SDK_KERNEL_LINUX_USER_MODE) || defined(CONFIG_X86_I2C))
int
main(int argc, char** argv)
{
    /* Ignore Ctrl+C  signal. */
    signal(SIGINT, SIG_IGN);	
    return diag_main(argc, argv);
} /* end of main */
#endif
