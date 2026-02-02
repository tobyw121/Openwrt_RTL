ifeq ($(wildcard $(DIR_LINUX)/.config),)
include ../../../linux-2.6.30/.config
else
include $(DIR_LINUX)/.config
endif

ifeq ($(wildcard $(DIR_USERS)/.config),)
include ../../.config
else
include $(DIR_USERS)/.config
endif

BACKPORT_DIR = backports-5.2.8-1
BACKPORT_CONFIG ?= $(DIR_ROOT)/$(BACKPORT_DIR)/.config
-include $(BACKPORT_CONFIG)

ifdef CONFIG_RTL_8198_AP_ROOT
CONFIG_RTL_AP = CONFIG_RTL_8198_AP_ROOT
endif

ifdef CONFIG_RTL_8197D_AP
CONFIG_RTL_AP = CONFIG_RTL_8197D_AP
endif

ifdef CONFIG_RTL_AP_PACKAGE 
CONFIG_RTL_AP = CONFIG_RTL_AP_PACKAGE
endif

ifdef CONFIG_RTL_AP
AP=1
SKUTAG=ap
else
GW=1
SKUTAG=gw
endif

APMIB_SHARED = 1

MTDAPI_DIR=$(DIR_ROOT)/users/lib/lib
#------------------------------------------------------------------------------
#ifndef COMMON_CFLAGS
#$(error COMMON_CFLAGS is not defined)
#endif

#ifndef APMIB_SHARED
#$(error APMIB_SHARED is not defined)
#endif

#ifndef APMIB_LIB
#$(error APMIB_LIB is not defined)
#endif

#------------------------------------------------------------------------------
.SUFFIXES:
.SUFFIXES: .o .c
.PHONY: clean all depend

LDFLAGS =  -g
CFLAGS = -Os -pipe
DEPEND = ./.depend

#------------------------------------------------------------------------------
APMIB_DIR=../apmib

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =

#------------------------------------------------------------------------------
# Files
#------------------------------------------------------------------------------
INCLUDE_SYSCONF = 1

ifeq ($(INCLUDE_SYSCONF),1)
SYSCONFIG = sysconf
TIMELYCHECK = timelycheck
SYSTEMD_ACTION	= systemd_action
SYSTEMD	= systemd

ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
AUTO_DHCP_CHECK = Auto_DHCP_Check
endif
INTERNET_PPP=ppp_inet
INTERNET_NTP=ntp_inet
INTERNET_DDNS=ddns_inet
RTK_EVENTD=rtk_eventd
ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
INTERNET_IPV6_MANAGE=ipv6_manage_inet
endif
ifeq ($(CONFIG_IPV6_DNSV6_MODE_SUPPORT),y)
INTERNET_IPV6_MANAGE=ipv6_manage_inet
endif
else
SYSCONFIG =
TIMELYCHECK = 
SYSTEMD_ACTION =
SYSTEMD =
ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
AUTO_DHCP_CHECK =
endif
INTERNET_PPP = 
INTERNET_NTP= 
INTERNET_DDNS=
RTK_EVENTD=
ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
INTERNET_IPV6_MANAGE=
endif
ifeq ($(CONFIG_IPV6_DNSV6_MODE_SUPPORT),y)
INTERNET_IPV6_MANAGE=
endif
endif

ifeq ($(INCLUDE_SYSCONF),1)
REALTEK_SYS_FILES = sysconf.c set_init.c set_bridge.c set_linux_vlan.c set_wlanapp.c sys_utility.c
ifeq ($(GW),1)
REALTEK_SYS_FILES += set_firewall.c set_wan.c
ifneq ($(CONFIG_APP_APPLE_MFI_WAC),y)
ifeq ($(CONFIG_IPV6), y)
REALTEK_SYS_FILES += set_ipv6.c
endif
endif
endif

ifeq ($(CPTCFG_RTL8852AE_BACKPORTS), y)
REALTEK_SYS_FILES += wifi6_priv_conf.c hapd_conf.c set_init_wifi6.c
endif

REALTEK_INTERNET_PPP_FILES = ppp_inet.c sys_utility.c
REALTEK_INTERNET_NTP_FILES = ntp_inet.c sys_utility.c
REALTEK_INTERNET_DDNS_FILES = ddns_inet.c sys_utility.c
ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
REALTEK_INTERNET_IPV6_MANAGE_FILES= ipv6_manage_inet.c sys_utility.c
endif
ifeq ($(CONFIG_IPV6_DNSV6_MODE_SUPPORT),y)
REALTEK_INTERNET_IPV6_MANAGE_FILES= ipv6_manage_inet.c sys_utility.c
endif

REALTEK_SYSTEMD_ACTION_FILES = systemd_action.c systemd_hostapd.c
REALTEK_SYSTEMD_FILES = systemd_hostapd.c systemd_main.c

REALTEK_TIMELY_CHECK_FILES = timelycheck.c
REALTEK_TIMELY_CHECK_FILES += sys_utility.c set_init.c set_bridge.c set_linux_vlan.c set_wlanapp.c

ifeq ($(CPTCFG_RTL8852AE_BACKPORTS), y)
REALTEK_TIMELY_CHECK_FILES += wifi6_priv_conf.c hapd_conf.c set_init_wifi6.c
endif

ifeq ($(CONFIG_USER_RTK_WLAN_MANAGER),y)
REALTEK_SYS_FILES += wlan_manager_conf.c
REALTEK_TIMELY_CHECK_FILES += wlan_manager_conf.c
endif

RTK_EVENTD_FILES += rtk_eventd.c sys_utility.c

ifeq ($(GW),1)
REALTEK_TIMELY_CHECK_FILES += set_firewall.c set_wan.c
ifneq ($(CONFIG_APP_APPLE_MFI_WAC),y)
ifeq ($(CONFIG_IPV6), y)
REALTEK_TIMELY_CHECK_FILES += set_ipv6.c
endif
endif
endif
ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
REALTEK_AUTO_DHCP_CHECK_FILES = Auto_DHCP_Check.c
REALTEK_AUTO_DHCP_CHECK_FILES += sys_utility.c discovery.c if.c
endif
else
REALTEK_SYS_FILES =
REALTEK_INTERNET_PPP_FILES =
REALTEK_INTERNET_NTP_FILES =
REALTEK_INTERNET_DDNS_FILES =
REALTEK_TIMELY_CHECK_FILES =
ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
REALTEK_AUTO_DHCP_CHECK_FILES =
endif
ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
REALTEK_INTERNET_IPV6_MANAGE_FILES=
endif
ifeq ($(CONFIG_IPV6_DNSV6_MODE_SUPPORT),y)
REALTEK_INTERNET_IPV6_MANAGE_FILES=
endif
endif

ifeq ($(CONFIG_DOMAIN_NAME_QUERY_SUPPORT),y)
REALTEK_SYS_FILES += set_domain_query.c
REALTEK_TIMELY_CHECK_FILES += set_domain_query.c
endif

ifeq ($(CONFIG_RTL_P2P_SUPPORT),y)
REALTEK_SYS_FILES += set_domain_query.c
endif

ifeq ($(CONFIG_APP_MULTI_AP),y)
ifeq ($(CPTCFG_RTL8852AE_BACKPORTS), y)
REALTEK_SYS_FILES += map_init_wifi6.c
REALTEK_TIMELY_CHECK_FILES += map_init_wifi6.c
else
REALTEK_SYS_FILES += map_init.c
REALTEK_TIMELY_CHECK_FILES += map_init.c
endif
endif

ifeq ($(CONFIG_RTL_ULINKER),y)
REALTEK_SYS_FILES += set_ulinker.c
## for pppoe discover
REALTEK_SYS_FILES += discovery.c if.c
endif

ifeq ($(CONFIG_RTL_MESH_CROSSBAND),y)
CFLAGS += -DCONFIG_RTL_MESH_CROSSBAND
endif

ifeq ($(CONFIG_RTL_MESH_SUPPORT),y)
CFLAGS += -DCONFIG_RTK_MESH
ifeq ($(CONFIG_RTL_MESH_SINGLE_IFACE),y)
CFLAGS += -DCONFIG_RTL_MESH_SINGLE_IFACE
endif
endif

ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
COMMON_CFLAGS += -DCONFIG_AUTO_DHCP_CHECK
endif

ifeq ($(CONFIG_APP_AVAHI),y)
CFLAGS += -DCONFIG_AVAHI
endif

ifeq ($(CONFIG_APP_APPLE_MFI_WAC),y)
CFLAGS += -DCONFIG_APP_APPLE_MFI_WAC
endif

ifeq ($(CONFIG_APP_APPLE_HOMEKIT),y)
CFLAGS += -DCONFIG_APPLE_HOMEKIT
endif

ifeq ($(CONFIG_APP_RTK_BLUETOOTH_FM),y)
CFLAGS += -DCONFIG_APP_RTK_BLUETOOTH_FM
endif

ifeq ($(CONFIG_RTL_8197F_VG),y)
CFLAGS += -DCONFIG_RTL_8197F_VG
endif

#------------------------------------------------------------------------------
# CFLAGS
#------------------------------------------------------------------------------
DEBUG	= -g -Wall
IFLAGS 	= -I. -I$(APMIB_DIR)
CFLAGS += $(COMMON_CFLAGS) $(EXTRA_CFLAGS)

#include $(DIR_USERS)/.config
ifdef CONFIG_APP_MULTIPPPOE
	CFLAGS += -D MULTI_PPPOE
endif
SOURCES = $(REALTEK_SYS_FILES) $(REALTEK_INTERNET_PPP_FILES) \
		$(REALTEK_INTERNET_NTP_FILES) $(REALTEK_INTERNET_DDNS_FILES)\
		$(REALTEK_TIMELY_CHECK_FILES) $(REALTEK_AUTO_DHCP_CHECK_FILES)
ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
SOURCES += $(REALTEK_INTERNET_IPV6_MANAGE_FILES)
endif
ifeq ($(CONFIG_IPV6_DNSV6_MODE_SUPPORT),y)
SOURCES += $(REALTEK_INTERNET_IPV6_MANAGE_FILES)
endif
SOURCES += $(REALTEK_SYSTEMD_ACTION_FILES)
SOURCES += $(REALTEK_SYSTEMD_FILES)

REALTEK_SYS_OBJS = $(REALTEK_SYS_FILES:.c=.o)
REALTEK_INTERNET_PPP_OBJS = $(REALTEK_INTERNET_PPP_FILES:.c=.o)
REALTEK_INTERNET_NTP_OBJS = $(REALTEK_INTERNET_NTP_FILES:.c=.o)
REALTEK_INTERNET_DDNS_OBJS = $(REALTEK_INTERNET_DDNS_FILES:.c=.o)
ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
REALTEK_INTERNET_IPV6_MANAGE_OBJS = $(REALTEK_INTERNET_IPV6_MANAGE_FILES)
endif
ifeq ($(CONFIG_IPV6_DNSV6_MODE_SUPPORT),y)
REALTEK_INTERNET_IPV6_MANAGE_OBJS = $(REALTEK_INTERNET_IPV6_MANAGE_FILES)
endif
REALTEK_TIMELY_CHECK_OBJS = $(REALTEK_TIMELY_CHECK_FILES:.c=.o)
REALTEK_SYSTEMD_ACTION_OBJS = $(REALTEK_SYSTEMD_ACTION_FILES:.c=.o)
REALTEK_SYSTEMD_OBJS = $(REALTEK_SYSTEMD_FILES:.c=.o)

ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
REALTEK_AUTO_DHCP_CHECK_OBJS = $(REALTEK_AUTO_DHCP_CHECK_FILES:.c=.o)
endif
RTK_EVENTD_OBJS = $(RTK_EVENTD_FILES:.c=.o)
ifeq ($(CONFIG_APP_RTK_VOIP),y)
LDFLAGS += -lpthread
endif

ifeq ($(CONFIG_MTD_NAME_MAPPING),y)
LDFLAGS += -L$(MTDAPI_DIR) -lmtdapi
endif
#------------------------------------------------------------------------------
# targets
#------------------------------------------------------------------------------
TARGET = $(SYSCONFIG) $(TIMELYCHECK) $(RTK_EVENTD) $(SYSTEMD_ACTION) $(SYSTEMD)
ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
TARGET += $(AUTO_DHCP_CHECK)
endif
ifeq ($(GW),1)
TARGET += $(INTERNET_PPP) $(INTERNET_NTP) $(INTERNET_DDNS)
endif
ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
TARGET += $(INTERNET_IPV6_MANAGE)
endif
ifeq ($(CONFIG_IPV6_DNSV6_MODE_SUPPORT),y)
TARGET += $(INTERNET_IPV6_MANAGE)
endif
all: depend $(TARGET)

$(SYSCONFIG): Makefile $(REALTEK_SYS_OBJS) $(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o $@ $(REALTEK_SYS_OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@

$(INTERNET_PPP): Makefile $(REALTEK_INTERNET_PPP_OBJS)	$(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o $@ $(REALTEK_INTERNET_PPP_OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@

$(INTERNET_NTP): Makefile $(REALTEK_INTERNET_NTP_OBJS)
	$(CC) -o $@ $(REALTEK_INTERNET_NTP_OBJS) $(LDFLAGS)
	$(STRIP) $@

$(INTERNET_DDNS): Makefile $(REALTEK_INTERNET_DDNS_OBJS)
	$(CC) -o $@ $(REALTEK_INTERNET_DDNS_OBJS) $(LDFLAGS)
	$(STRIP) $@

$(INTERNET_IPV6_MANAGE): Makefile $(REALTEK_INTERNET_IPV6_MANAGE_OBJS) $(APMIB_DIR)/$(APMIB_LIB)
ifeq ($(KERNEL_VER),linux-2.6.30)
	$(CC) -c -o ipv6_manage_inet.o $(DEBUG) $(CFLAGS) $(IFLAGS) -I$(DIR_LINUX)/include/ ipv6_manage_inet.c
else
	$(CC) -c -o ipv6_manage_inet.o $(DEBUG) $(CFLAGS) $(IFLAGS) -I$(DIR_LINUX)/include/uapi/ ipv6_manage_inet.c 
endif
	$(CC) -o $@ ipv6_manage_inet.o sys_utility.o $(LDFLAGS)  -L$(APMIB_DIR) -lapmib 
	$(STRIP) $@

$(TIMELYCHECK): Makefile $(REALTEK_TIMELY_CHECK_OBJS) $(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o $@ $(REALTEK_TIMELY_CHECK_OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@

$(SYSTEMD_ACTION): Makefile $(REALTEK_SYSTEMD_ACTION_OBJS) $(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o $@ $(REALTEK_SYSTEMD_ACTION_OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@

$(SYSTEMD): Makefile $(REALTEK_SYSTEMD_OBJS) $(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o $@ $(REALTEK_SYSTEMD_OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@

ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
$(AUTO_DHCP_CHECK): Makefile $(REALTEK_AUTO_DHCP_CHECK_OBJS) $(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o $@ $(REALTEK_AUTO_DHCP_CHECK_OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@
endif

$(RTK_EVENTD): Makefile $(RTK_EVENTD_OBJS) $(APMIB_DIR)/$(APMIB_LIB)
	$(CC) -o $@ $(RTK_EVENTD_OBJS) $(LDFLAGS) -L$(APMIB_DIR) -lapmib
	$(STRIP) $@

clean:
	rm -f $(DEPEND)
	rm -f *.o $(SYSCONFIG) $(INTERNET_PPP) $(INTERNET_NTP) $(INTERNET_DDNS) $(INTERNET_IPV6_MANAGE) $(TIMELYCHECK) $(RTK_EVENTD) $(SYSTEMD_ACTION)
	rm -f $(SYSTEMD)

ifeq ($(CONFIG_AUTO_DHCP_CHECK),y)
	rm -f $(AUTO_DHCP_CHECK)
endif
depend: $(SOURCES)
	if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(DEBUG) $(CFLAGS) $(IFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.c.o:
	${CC} -c -o $@ $(DEBUG) $(CFLAGS) $(IFLAGS) $<
