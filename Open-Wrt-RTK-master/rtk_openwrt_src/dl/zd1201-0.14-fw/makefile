#
#	Makefile for zd1201 firmware
#

FIRMWARE=zd1201.fw zd1201-ap.fw

INSTALL = cp -p -f

default: install

install:
	@echo -n "  INSTALL" ${FIRMWARE}
	@test -d /etc/firmware \
	 && $(INSTALL) ${FIRMWARE} /etc/firmware \
	 && echo " =>" /etc/firmware/ \
	 || test -d /usr/lib/hotplug/firmware \
	 && $(INSTALL) ${FIRMWARE} /usr/lib/hotplug/firmware \
	 && echo " =>" /usr/lib/hotplug/firmware/ \
	 || ( echo && echo "Failed:  No firmware directory found" && false )
