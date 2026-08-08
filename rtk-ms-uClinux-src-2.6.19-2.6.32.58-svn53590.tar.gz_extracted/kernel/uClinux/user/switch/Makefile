#***********************************************************
#
# Copyright (C) 2010 Realtek Semiconductor Corp.
#
# Makefile -- Build instructions for switch global setting
#
#***********************************************************

#
# Include architecture specific build rules
#
ifneq ($(CONFIG_SYS_CODEBASE), SDK)
ifndef SYS_CONFIG
include $(SYS_ROOTDIR)/make.config.mk
endif
endif

#
# Define build variables
#

#
# Define build flags
#

#
# Define build directories
#
dir_y :=

dir_$(CONFIG_SYS_LIB) += lib/
dir_$(CONFIG_SYS_APP) += app/
dir_$(CONFIG_SYS_UT) += ut/

ifneq ($(CONFIG_SDK_LINKED_KMOD),y)
dir_$(CONFIG_SDK_APP_COMPILE) += sdk/
endif

#
# Define build targets
#
all:
	for i in $(dir_y) ; do \
		$(MAKE) -C $$i || exit $? ; \
	done

romfs:
	for i in $(dir_y) ; do \
		$(MAKE) -C $$i romfs || exit $? ; \
	done

clean:
	for i in $(dir_y) ; do \
		$(MAKE) -C $$i clean ; \
	done

