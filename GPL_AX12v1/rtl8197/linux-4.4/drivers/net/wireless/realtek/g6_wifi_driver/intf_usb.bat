@ECHO OFF

ECHO "### Platform dependent HCI config - <intf_usb.bat>                 ###"

SET _OS_INTFS_FILES=
SET _OS_INTFS_FILES=%_OS_INTFS_FILES% ndisloader_%HCI%.c
REM SET _OS_INTFS_FILES=%_OS_INTFS_FILES% %HCI%_intf.c
SET _OS_INTFS_FILES=%_OS_INTFS_FILES% %HCI%_intf_ce.c
SET _OS_INTFS_FILES=%_OS_INTFS_FILES% ce_osintf.c
SET _OS_INTFS_FILES=%_OS_INTFS_FILES% os_intfs.c

SET _OS_INTFS_FILES=%_OS_INTFS_FILES% ioctl_win.c
SET _OS_INTFS_FILES=%_OS_INTFS_FILES% xmit_win.c
SET _OS_INTFS_FILES=%_OS_INTFS_FILES% mlme_win.c
SET _OS_INTFS_FILES=%_OS_INTFS_FILES% recv_win.c

SET _OS_INTFS_FILES=%_OS_INTFS_FILES% osdep_service.c

SET _HAL_INTFS_FILES=
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% hal_init.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192c_d_hal_init.c

SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192c_dm.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192c_phycfg.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192c_rf6052.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192c_rxdesc.c

SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% %HCI%_ops_ce.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% %HCI%_halinit.c

SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% Hal8192CUHWImg.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192cu_recv.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192cu_xmit.c
SET _HAL_INTFS_FILES=%_HAL_INTFS_FILES% rtl8192c_cmd.c

SET _CORE_FILES=
SET _CORE_FILES=%_CORE_FILES% ieee80211.c
SET _CORE_FILES=%_CORE_FILES% rtw_cmd.c
SET _CORE_FILES=%_CORE_FILES% rtw_debug.c
SET _CORE_FILES=%_CORE_FILES% rtw_io.c
SET _CORE_FILES=%_CORE_FILES% rtw_ioctl_query.c
SET _CORE_FILES=%_CORE_FILES% rtw_ioctl_set.c
SET _CORE_FILES=%_CORE_FILES% rtw_mlme.c
SET _CORE_FILES=%_CORE_FILES% rtw_mlme_ext.c
SET _CORE_FILES=%_CORE_FILES% rtw_pwrctrl.c
SET _CORE_FILES=%_CORE_FILES% rtw_recv.c
SET _CORE_FILES=%_CORE_FILES% rtw_rf.c
SET _CORE_FILES=%_CORE_FILES% rtw_security.c
SET _CORE_FILES=%_CORE_FILES% rtw_sta_mgt.c
SET _CORE_FILES=%_CORE_FILES% rtw_wlan_util.c
SET _CORE_FILES=%_CORE_FILES% rtw_xmit.c

SET _CORE_FILES=%_CORE_FILES% rtl8712_efuse.c
SET _CORE_FILES=%_CORE_FILES% rtl8192c_led.c

IF %_TGTOS%==CE GOTO INTF_USB_LABEL_CE

:INTF_USB_LABEL_XP

copy autoconf_%HAL_RTL871X%_%HCI%_xp.h include\autoconf.h
copy os_intf\windows\sources_ddk_%HCI% os_intf\windows\sources_ddk

SET _USB_LIBS=%DDK_LIB_PATH%\usbd.lib

GOTO INTF_USB_LABEL_OS_DETECT_EXIT

:INTF_USB_LABEL_CE

ECHO "### COPY target os_intf\windows\sources_ce                         ###"

copy autoconf_%HAL_RTL871X%_%HCI%_ce.h include\autoconf.h
REM copy os_intf\windows\sources_ce_%HCI% os_intf\windows\sources_ce
	
REM SET _SDIO_LIBS=
REM SET _SDIO_LIBS=%_SDIO_LIBS% sdbus.lib
REM SET _SDIO_LIBS=%_SDIO_LIBS% sdcardlib.lib

SET _WINCE_LIBS=%_WINCE_LIBS% %_USB_LIBS%

GOTO INTF_USB_LABEL_OS_DETECT_EXIT

:INTF_USB_LABEL_OS_DETECT_EXIT
