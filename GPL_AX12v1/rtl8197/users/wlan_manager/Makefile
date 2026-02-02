EXEC = wlan_manager
OBJS = wlan_manager.o band_steering.o

include $(LINUX_CONFIG)
ifeq ($(CONFIG_USER_RTK_CROSSBAND_REPEATER), y)
OBJS += crossband_daemon.o

CFLAGS := -s -Wall -c -Os $(CFLAGS)
LDFLAGS = -s -Wall

#CFLAGS  += -I$(ROOTDIR)/user/boa/src/LINUX
CFLAGS  += -I$(ROOTDIR)/include
ifneq ("$(wildcard $(ROOTDIR)/backports-5.2.8-1/drivers/net/wireless/realtek/rtl8192fe)","")
CFLAGS += $(ROOTDIR)/backports-5.2.8-1/drivers/net/wireless/realtek/rtl8192fe
endif
ifneq ("$(wildcard $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/rtl8192cd)","")
CFLAGS += $(ROOTDIR)/$(LINUXDIR)/drivers/net/wireless/realtek/rtl8192cd
endif
LDFLAGS += -L$(APOLLODIR)/object/src/app/lib
#LDFLAGS += -L$(ROOTDIR)/user/boa/src/LINUX $(LIBMIB) $(LIBVOIP)

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += --static -s -Wl,--gc-sections
endif
endif
CFLAGS += -DCONFIG_BAND_STEERING
ifeq ($(CONFIG_USER_RTK_COMMON_NETLINK),y)
CFLAGS += -DCONFIG_USER_RTK_COMMON_NETLINK
endif
ifeq ($(CONFIG_USER_RTK_CROSSBAND_REPEATER), y)
CFLAGS += -DCONFIG_CROSSBAND_REPEATER
endif

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

romfs:
	$(ROMFSINST)    /bin/$(EXEC)

clean:
	-rm -f $(EXEC) *.elf *.gdb *.o
