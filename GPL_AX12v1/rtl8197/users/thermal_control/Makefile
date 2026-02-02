-include $(LINUX_CONFIG)
ifeq ($(ROOTDIR),)
ROOTDIR=../..
endif
$(warning ROOTDIR=$(ROOTDIR))
-include $(ROOTDIR)/.config

CC=$(CROSS_COMPILE)gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=ther_control.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=ther_control

ifeq ($(CONFIG_RTL8852AE_BACKPORTS),y)
CFLAGS += -DBACKPORTS
CFLAGS += -I$(ROOTDIR)/include/
ifeq ($(CONFIG_RTL_8197F_VG),y)
CFLAGS += -I$(ROOTDIR)/linux-4.4/drivers/net/wireless/realtek/ther_ctrl/
else
CFLAGS += -I$(ROOTDIR)/linux-4.4.x/drivers/net/wireless/realtek/ther_ctrl/
endif
else ifeq ($(CONFIG_DEFAULTS_KERNEL_5_10),y)
CFLAGS += -I$(ROOTDIR)/include/
CFLAGS += -I$(ROOTDIR)/linux-5.10.x/drivers/net/wireless/realtek/ther_ctrl/
else
CFLAGS += -I$(ROOTDIR)/include/
CFLAGS += -I$(ROOTDIR)/linux-4.4.x/drivers/net/wireless/realtek/rtl8192cd/
endif

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

romfs:
	$(ROMFSINST) ./$(EXECUTABLE) /bin/$(EXECUTABLE)
	$(ROMFSINST) ./ther_config/ther.conf-unknow /etc/ther.conf
ifeq ($(CONFIG_RTK_SOC_RTL8198D),y)
ifeq ($(CONFIG_RTL_TRIBAND_SUPPORT),y)
	$(ROMFSINST) ./ther_config/ther.conf-98D-V100 /etc/ther.conf
endif
ifeq ($(CONFIG_SLOT_1_8814BE),y)
	$(ROMFSINST) ./ther_config/ther.conf-98D-V00B /etc/ther.conf
endif
endif
ifeq ($(CONFIG_RTL8198F_SERIES),y)
ifeq ($(CONFIG_SLOT_0_8814BE),y)
	$(ROMFSINST) ./ther_config/ther.conf-98F-8814B-V510 /etc/ther.conf
ifeq ($(CONFIG_SOC_EXT_PA),y)
	$(ROMFSINST) ./ther_config/ther.conf-98F-8814B-V601 /etc/ther.conf
endif
endif
endif
ifeq ($(CONFIG_RTL_8197F),y)
	$(ROMFSINST) ./ther_config/ther.conf-97F-V6A4G /etc/ther.conf
endif

.PHONY: rcX
rcX:
	echo "nothing"

clean: 
	rm -rf $(EXECUTABLE) *.o
