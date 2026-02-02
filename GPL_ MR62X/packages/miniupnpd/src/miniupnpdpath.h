/* $Id: miniupnpdpath.h,v 1.9 2012/09/27 15:47:15 nanard Exp $ */
/* MiniUPnP project
 * http://miniupnp.free.fr/ or http://miniupnp.tuxfamily.org/
 * (c) 2006-2011 Thomas Bernard
 * This software is subject to the conditions detailed
 * in the LICENCE file provided within the distribution */

#ifndef MINIUPNPDPATH_H_INCLUDED
#define MINIUPNPDPATH_H_INCLUDED

#include "config.h"

/* Paths and other URLs in the miniupnpd http server */

#define ROOTDESC_PATH 		rootDescPath

#ifdef HAS_DUMMY_SERVICE
#define DUMMY_PATH			dummyPath
#endif

#define WANCFG_PATH			wancfgPath
#define WANCFG_CONTROLURL	wancfgControlURL
#define WANCFG_EVENTURL		wancfgEventURL

#define WANIPC_PATH			wanIPCPath
#define WANIPC_CONTROLURL	wanIPCControlURL
#define WANIPC_EVENTURL		wanIPCEventURL

#ifdef ENABLE_L3F_SERVICE
#define L3F_PATH			L3FPath
#define L3F_CONTROLURL		L3FControlURL
#define L3F_EVENTURL		L3FEventURL
#endif

#ifdef ENABLE_6FC_SERVICE
#define WANIP6FC_PATH		wanIP6FCPath
#define WANIP6FC_CONTROLURL	wanIP6FCControlURL
#define WANIP6FC_EVENTURL	wanIP6FCEventURL
#endif

#ifdef ENABLE_DP_SERVICE
/* For DeviceProtection introduced in IGD v2 */
#define DP_PATH				DPPath
#define DP_CONTROLURL		DPControlURL
#define DP_EVENTURL			DPEventURL
#endif

#endif

