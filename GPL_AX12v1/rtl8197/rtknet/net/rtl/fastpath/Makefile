#
# Makefile for the fastpath modules on top of IPv4.
#

include $(DIR_ROOT)/.config

#EXTRA_CFLAGS += -I $(DIR_LINUX)/drivers/net/rtl819x
EXTRA_CFLAGS += -I $(DIR_LINUX)/net
EXTRA_CFLAGS += -D__KERNEL__
#EXTRA_CFLAGS += -Wno-implicit -Werror

ifeq "$(CONFIG_MODEL_RTL89xxC_SDK)" "y"
EXTRA_CFLAGS += --save-temps
endif

FASTPATH_BUILD_DIR =

CORE_SRC_DIR = core
CORE_RELEASE_DIR = core_release
FASTPATH_BUILD_SRC = $(shell test -d $(DIR_LINUX)/net/rtl/fastpath/$(CORE_SRC_DIR) && echo -n y)

ifeq ($(FASTPATH_BUILD_SRC),y)
FASTPATH_BUILD_DIR = $(CORE_SRC_DIR)
else
FASTPATH_BUILD_DIR = $(CORE_RELEASE_DIR)
endif

ifeq ($(CONFIG_RTL_IPTABLES_FAST_PATH),m)
EXTRA_CFLAGS += -DCONFIG_RTK_IPTABLES_FAST_PATH
EXTRA_CFLAGS += -DCONFIG_FAST_PATH_MODULE

FASTPATH_OBJ=$(FASTPATH_BUILD_DIR)/fastpath_core.o fastpath_common.o $(FASTPATH_BUILD_DIR)/filter.o $(FASTPATH_BUILD_DIR)/fast_pptp_core.o $(FASTPATH_BUILD_DIR)/fast_l2tp_core.o
FASTPATH_MODULE=fastpath.o
obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_MODULE)
fastpath-objs := $(FASTPATH_OBJ)

else
obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_BUILD_DIR)/fastpath_core.o fastpath_common.o $(FASTPATH_BUILD_DIR)/filter.o filter_common.o $(FASTPATH_BUILD_DIR)/fast_pptp_core.o $(FASTPATH_BUILD_DIR)/fast_l2tp_core.o
ifeq ($(CONFIG_RTL_FAST_FILTER), y)
obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_BUILD_DIR)/fast_filter.o
else
obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_BUILD_DIR)/filter_v2.o
endif

obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_BUILD_DIR)/filter_ipv4.o
ifeq ($(CONFIG_IPV6), y)
obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_BUILD_DIR)/filter_ipv6.o
endif


ifeq ($(CONFIG_RTL_FAST_PPPOE), y)
obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_BUILD_DIR)/fast_pppoe_core.o
endif

ifeq ($(CONFIG_RTL_FAST_IPV6), y)
obj-$(CONFIG_RTL_IPTABLES_FAST_PATH) += $(FASTPATH_BUILD_DIR)/fast_ipv6_core.o
endif
endif

obj-$(CONFIG_RTL_FAST_BRIDGE) += $(FASTPATH_BUILD_DIR)/fast_br.o

clean:
	rm -rf *.o *.ko *.mod.c

EXTRA_AFLAGS += $(EXTRA_CFLAGS)
