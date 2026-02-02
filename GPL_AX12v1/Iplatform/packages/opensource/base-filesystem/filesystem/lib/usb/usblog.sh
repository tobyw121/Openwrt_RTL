#!/bin/sh

#syslog define
PROJ_LOG_ID_USB=294
#MSG(USB_DEVICE_STORAGE_FOUND, 51, INF, "[USB %1]New USB device #%2 founded - %3 - Storage")
USB_DEVICE_STORAGE_FOUND=51
#MSG(USB_DEVICE_HUB_FOUND, 52, INF, "[USB %1]New USB device #%2 founded - %3 - Hub)")
USB_DEVICE_HUB_FOUND=52
#MSG(USB_DEVICE_PRINTER_FOUND, 53, INF, "[USB %1]New USB device #%2 founded - %3 - Printer")
USB_DEVICE_PRINTER_FOUND=53
#MSG(USB_DEVICE_DISCONNECT, 54, INF, "[USB %1]USB device #%2 Disconnect")
USB_DEVICE_DISCONNECT=54
#MSG(USB_DEVICE_STORAGE_VOLUMN, 60, INF, "[USB %1]USB device #%2 Storage size %3")
USB_DEVICE_STORAGE_VOLUMN=60

# ddns_syslog log_id log_param
usb_syslog()
{
    local log_id=$1
    shift
    logx -p $$ $PROJ_LOG_ID_USB $log_id "$@"
}
