@ECHO OFF

ECHO "### Platform dependent config - <core_conf.bat>                    ###"

ECHO "### Copy target include\autoconf.h and os_intf\windows\sources     ###"

ECHO "### Set platform independent environment variables                 ###"
SET _DRIVER_ROOT=%cd%
SET DRIVER_NAME=RTL8192C

set DEVICE_ID=
set DRIVER_ID=
set DRIVER_NAME=
set BUILD_OPTIONS=
set MKDRIVER_TARGET=
set MKDRIVER_DRIVER_PATH=

ECHO "### Set platform dependent environment variables                   ###"
IF %_TGTOS%==CE GOTO CORE_CONF_LABEL_CE

:CORE_CONF_LABEL_XP

SET SRC=sources_ddk
SET _WINBUILDROOT=%NTMAKEENV%

GOTO CORE_CONF_LABEL_OS_DETECT_EXIT

:CORE_CONF_LABEL_CE

SET SRC=sources_ce
SET WINCEREL=1
	
SET _COMMONOAKROOT=%_WINCEROOT%\PUBLIC\COMMON\OAK
SET _COMMONSDKROOT=%_WINCEROOT%\PUBLIC\COMMON\SDK
SET _DRIVER_TARGET_DIR=%_COMMONOAKROOT%\lib\%_TGTCPU%\%WINCEDEBUG%
SET _WINBUILDROOT=%_MAKEENVROOT%

SET X86_CPU=0
IF %_TGTCPU% == x86 SET X86_CPU=1

REM DIRECT CHECK %_WINCEOSVER% equ 600/500
	
SET _BUILD_MODULE_DEF=-DWINDOWS_CE -DX86_CPU=%X86_CPU%

SET _INCLUDES_DIRS=%_COMMONOAKROOT%\inc;%_COMMONSDKROOT%\inc;%_DRIVER_ROOT%\include;
	
REM SET _INCLUDES_DIRS=%_DRIVER_ROOT%\include;
REM Setup Library CE5.0 different from CE 4.2

SET _WINCE_LIBS=%_PROJECTROOT%\cesysgen\sdk\lib\%_TGTCPU%\%WINCEDEBUG%\ndis.lib
SET _WINCE_LIBS=%_WINCE_LIBS% %_PROJECTROOT%\cesysgen\sdk\lib\%_TGTCPU%\%WINCEDEBUG%\coredll.lib 
SET _WINCE_LIBS=%_WINCE_LIBS% %_PROJECTROOT%\cesysgen\sdk\lib\%_TGTCPU%\%WINCEDEBUG%\ceddk.lib

GOTO CORE_CONF_LABEL_OS_DETECT_EXIT

:CORE_CONF_LABEL_OS_DETECT_EXIT

ECHO "### Driver function object files declaration                       ###"
REM SET _DRV_LIB=%_DRIVER_TARGET_DIR%\rtw_core_common.lib
REM SET _DRV_LIB=%_DRV_LIB% %_DRIVER_TARGET_DIR%\rtw_core_efuse.lib
REM SET _DRV_LIB=%_DRV_LIB% %_DRIVER_TARGET_DIR%\rtw_core_led.lib
REM SET _DRV_LIB=%_DRV_LIB% %_DRIVER_TARGET_DIR%\rtw_hal_common.lib
REM SET _DRV_LIB=%_DRV_LIB% %_DRIVER_TARGET_DIR%\rtw_hal_rtl8192c_common.lib
REM SET _DRV_LIB=%_DRV_LIB% %_DRIVER_TARGET_DIR%\rtw_hal_rtl8192c_usb.lib

REM SET _DRV_LIB=%_DRV_LIB% %_DRIVER_TARGET_DIR%\rtl871x_mp.lib

IF "%_TGTOS%"=="CE" SET _DRIVER_TARGET_LIBS=%_DRV_LIB% %_WINCE_LIBS%
