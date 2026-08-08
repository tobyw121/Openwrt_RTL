/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose :
 *
 * Feature :
 *
 */

#ifndef __RTL8390_INIT_H__
#define __RTL8390_INIT_H__

/*
 * Include Files
 */
#include <rtk_type.h>
#include <rtk_switch.h>


/*
 * Function Declaration
 */
extern void rtl8390_config(const rtk_switch_model_t *pModel);

void rtl8390_sfp_speed_set(int port, int speed);

#if defined(CONFIG_RTL8396M_DEMO)
void rtl8390_10gMedia_set(int port, int media);
void rtl8390_10gMedia_get(int port, int *media);

void rtl8396_serdes_10g_rx_rst (int sds_num);
void rtl8396_10gSds_restart(int port);
void rtl8390_10gSds_restart(int port);
void rtl8390_10gSds_init(int port);
#endif  /* CONFIG_RTL8396M_DEMO */

#endif  /* __RTL8390_INIT_H__ */

