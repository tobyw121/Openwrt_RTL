#!/bin/sh
#
#	sdk-linked-kmod.sh
#		1.Remove the source files of SDK package that belong rtnic, rtdrv and rtk modules.
#		2.Backup the module files(XXX.ko to XXX.kmod) for SDK package.
#

BASENAME=`basename $0`
DIRNAME=`dirname $0`
ORIGPWD=`pwd`

# The unused files of SDK
RMF_SDK="
	sdk/system/linux/rtnic/*.c
	sdk/system/linux/rtnic/*.h
	sdk/system/linux/rtnic/Makefile
	sdk/system/linux/rtdrv/*.c
	sdk/system/linux/rtdrv/*.h
	sdk/system/linux/rtdrv/Makefile
	sdk/system/linux/rtk
	sdk/src/rtk
	sdk/src/dal
	sdk/src/hal
	sdk/src/ioal
	sdk/src/common
	sdk/src/osal
	sdk/src/app/diag_shell/src
	sdk/src/app/diag_shell/Makefile
	sdk/src/app/lib
	sdk/unittest/Makefile
	sdk/unittest/dal
	sdk/unittest/hal
	sdk/unittest/nic
	sdk/unittest/osal
	sdk/unittest/sdk
	sdk/include/app
	sdk/include/common/debug
	sdk/include/common/util
	sdk/include/dal
	sdk/include/hal
	sdk/include/ioal
	sdk/include/osal
	sdk/doc
"

# The unused files of RTL8389
RMF_RTL8389="
	linux-2.6.x/drivers/net/switch/rtl8389/model
	linux-2.6.x/drivers/net/switch/rtl8389/driver/*.c
	linux-2.6.x/drivers/net/switch/rtl8389/driver/bsp/*.c
	linux-2.6.x/drivers/net/switch/rtl8389/driver/bsp/linux	
	linux-2.6.x/drivers/net/switch/rtl8389/driver/doc
	linux-2.6.x/drivers/net/switch/rtl8389/driver/osal/*.c
	linux-2.6.x/drivers/net/switch/rtl8389/driver/osal/linux	
	linux-2.6.x/drivers/net/switch/rtl8389/driver/rtk/*.c
	linux-2.6.x/drivers/net/switch/rtl8389/driver/sample
	linux-2.6.x/drivers/net/switch/rtl8389/driver/sdkTest/*.c
	linux-2.6.x/drivers/net/switch/rtl8389/driver/soc/*.c
	linux-2.6.x/drivers/net/switch/rtl8389/krnsal/src/*.c
	linux-2.6.x/net/switch/rtl8389/802.1x/*.c	
	linux-2.6.x/net/switch/rtl8389/ccldriver/*.c	
	linux-2.6.x/net/switch/rtl8389/cclmx/*.c	
	linux-2.6.x/net/switch/rtl8389/debug/*.c	
	linux-2.6.x/net/switch/rtl8389/extension/*.c	
	linux-2.6.x/net/switch/rtl8389/garp/*.c
	linux-2.6.x/net/switch/rtl8389/igmp/*.c	
	linux-2.6.x/net/switch/rtl8389/krntest
	linux-2.6.x/net/switch/rtl8389/lacp/*.c	
	linux-2.6.x/net/switch/rtl8389/mstp/*.c	
	linux-2.6.x/net/switch/rtl8389/rstp/*.c	
	linux-2.6.x/net/switch/rtl8389/stp/*.c	
	linux-2.6.x/net/switch/rtl8389/stp_uni/*.c	
	user/switch/rtl8316s
	vendors/Realtek/RTL8316S
"

# The unused files of RTL8316S
RMF_RTL8316S="
	vendors/Realtek/RTL8389
"


# Main
case "$1" in
  sdk)
	cd ${DIRNAME}/..
	cp -f linux-2.6.x/net/switch/sdk/rtnic/rtnic.ko linux-2.6.x/net/switch/sdk/rtnic/rtnic.kmod
	cp -f linux-2.6.x/net/switch/sdk/rtdrv/rtdrv.ko linux-2.6.x/net/switch/sdk/rtdrv/rtdrv.kmod
	cp -f linux-2.6.x/drivers/net/switch/sdk/rtk/rtk.ko linux-2.6.x/drivers/net/switch/sdk/rtk/rtk.kmod
	cp -f linux-2.6.x/drivers/net/switch/sdk/unittest/sdktest.ko linux-2.6.x/drivers/net/switch/sdk/unittest/sdktest.kmod
	cp -f sdk/src/app/diag_shell/bin/cli sdk/src/app/diag_shell/bin/cli_exe
	make clean
	for FILE in ${RMF_SDK}
	do
		rm -fr ${FILE} >& /dev/null &
	done
	cd ${ORIGPWD}
	;;
  rtl8389)
	cd ${DIRNAME}/..
	cp -f linux-2.6.x/drivers/net/switch/rtl8389/driver/soc/rtl8389_errno.c user/switch/rtl8389/usrsal/src
	cp -f linux-2.6.x/drivers/net/switch/rtl8389/driver/rtk_sdk.ko linux-2.6.x/drivers/net/switch/rtl8389/driver/rtk_sdk.kmod
	cp -f linux-2.6.x/drivers/net/switch/rtl8389/krnsal/src/krnsal.ko linux-2.6.x/drivers/net/switch/rtl8389/krnsal/src/krnsal.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/cclmx/cclmx.ko linux-2.6.x/net/switch/rtl8389/cclmx/cclmx.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/ccldriver/ccl_driver.ko linux-2.6.x/net/switch/rtl8389/ccldriver/ccl_driver.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/stp/stp.ko linux-2.6.x/net/switch/rtl8389/stp/stp.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/rstp/r_stp.ko linux-2.6.x/net/switch/rtl8389/rstp/r_stp.kmod		
	cp -f linux-2.6.x/net/switch/rtl8389/mstp/m_stp.ko linux-2.6.x/net/switch/rtl8389/mstp/m_stp.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/lacp/lacp.ko linux-2.6.x/net/switch/rtl8389/lacp/lacp.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/garp/garp_protocols.ko linux-2.6.x/net/switch/rtl8389/garp/garp_protocols.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/igmp/igmp_snooping.ko linux-2.6.x/net/switch/rtl8389/igmp/igmp_snooping.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/802.1x/802_1x.ko linux-2.6.x/net/switch/rtl8389/802.1x/802_1x.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/debug/debug.ko linux-2.6.x/net/switch/rtl8389/debug/debug.kmod
	cp -f linux-2.6.x/net/switch/rtl8389/extension/extension.ko linux-2.6.x/net/switch/rtl8389/extension/extension.kmod
	make distclean
	for FILE in ${RMF_RTL8389}
	do
		rm -fr ${FILE} >& /dev/null &
	done
	cd ${ORIGPWD}
	;;
  rtl8316s)
	cd ${DIRNAME}/..
	for FILE in ${RMF_RTL8316S}
	do
		rm -fr ${FILE} >& /dev/null &
	done
	cd ${ORIGPWD}
	;;
  *)
	echo $"Usage: $0 {sdk|rtl8389|rtl8316s}"
	exit 1
esac

exit 0

