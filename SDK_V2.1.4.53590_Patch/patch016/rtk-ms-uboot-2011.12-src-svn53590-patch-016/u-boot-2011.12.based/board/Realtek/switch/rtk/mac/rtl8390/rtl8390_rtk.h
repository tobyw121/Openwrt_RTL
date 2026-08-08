/*
 * Copyright(c) Realtek Semiconductor Corporation, 2012
 * All rights reserved.
 *
 * Purpose : Related definition of the RTL8328 driver for RTK command.
 *
 * Feature : RTL8328 driver for RTK command
 *
 */

#ifndef	__RTL8398_RTK_H__
#define	__RTL8398_RTK_H__

#include <rtk_switch.h>

extern const rtk_switch_model_t *gSwitchModel;
extern const rtk_mac_drv_t *gMacDrv;

extern void rtk_default(void);

extern void rtk_comboport_copper(void);
extern void rtk_comboport_fiber(void);
void rtk_comboport_portcopper(int portid);
void rtk_comboport_portfiber(int portid);
void rtk_comboport_auto(void);

extern void rtk_eee_off(const rtk_switch_model_t *pModel);

#ifdef CONFIG_EEE
extern void rtk_eee_on(const rtk_switch_model_t *pModel);
#endif

extern void rtk_network_on(void);
extern void rtk_network_off(void);

void rtk_phyPortPowerOn(int mac_idx);

extern void rtk_linkdown_powersaving_patch(void);

extern void rtk_phy_selfLoop_on(int portId);
extern void rtk_phy_selfLoop_off(int portId);

extern int32 rtk_htp_detect(void);
extern int32 rtk_rstDeftGpio_init(void);
extern int32 rtk_rstDeftGpio_detect(void);

void rtk_smiRead(uint32 phyad, uint32 regad, uint32* pData);
void rtk_smiWrite(uint32 phyad, uint32 regad, uint32 data);

void rtk_sys_led_on(void);
void rtk_sys_led_off(void);

void rtk_sfp_speed_set(int port, int speed);
void rtk_sysEsd_set(int state);

extern void rtk_mac_polling_enable(int port);
extern void rtk_mac_polling_disable(int port);
extern void rtk_serdes_fiber_watchdog(int port);

void rtk_port_isolation_on(void);
void rtk_port_isolation_off(void);
void rtk_portIsolation(int srcPort,int destPort);
void rtk_portIsolationCPUgoto(int port);
void rtk_portIsolationToCPU(int port);

void rtk_fiber_downSpeed_set(int status);
void rtk_fiber_nwayForceLink_set(int status);
void rtk_fiber_speed_set(int nway, int speed);
void rtk_fiber_nway_set(int status);
void rtk_fiber_rx_set(int status);
void rtk_fiber_speed_get(void);
void rtk_fiber_portLoopback(int port, int status);

void rtk_saLearning(int state);
int rtk_portlink_get(int unit,int port,int *link);

#if defined(CONFIG_RTL8396M_DEMO)
void rtk_10gMedia_set(int port, int media);
void rtk_10gSds_restart(int port);
void rtk_10gSds_init(int port);
#endif  /* CONFIG_RTL8396M_DEMO */

#endif	/* __RTL8398_RTK_H__ */

