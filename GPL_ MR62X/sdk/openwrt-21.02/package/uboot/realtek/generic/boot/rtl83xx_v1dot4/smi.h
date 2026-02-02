
#ifndef __SMI_H__
#define __SMI_H__

#include <rtk_types.h>
#include "rtk_error.h"

rtk_int32 smi_reset(rtk_uint32 port, rtk_uint32 pinRST);

//#define CONFIG_RTL_97F_SVN_TRUNK	1

#ifdef CONFIG_RTL_97F_SVN_TRUNK
rtk_int32 smi_init(rtk_uint32 portSCK, rtk_uint32 pinSCK, rtk_uint32 portSDA, rtk_uint32 pinSDA);
#else
rtk_int32 smi_init(rtk_uint32 portSCK, rtk_uint32 pinSCK, rtk_uint32 pinSDA);
#endif

rtk_int32 smi_init_83xx(rtk_uint32 portSCK, rtk_uint32 portSDA, rtk_uint32 pinSCK, rtk_uint32 pinSDA);

rtk_int32 smi_read(rtk_uint32 mAddrs, rtk_uint32 *rData);
rtk_int32 smi_write(rtk_uint32 mAddrs, rtk_uint32 rData);

#endif /* __SMI_H__ */


