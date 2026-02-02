IWINFO_BACKENDS    = $(BACKENDS)
BCM_SDK	           = $(BCMFLAGS)
IWINFO_CFLAGS      = $(CFLAGS) -std=gnu99 -fstrict-aliasing -Iinclude

IWINFO_LIB         = libiwinfo.so
IWINFO_LIB_LDFLAGS = $(LDFLAGS) -shared
IWINFO_LIB_OBJ     = iwinfo_utils.o iwinfo_wext.o iwinfo_wext_scan.o iwinfo_lib.o

IWINFO_LUA         = iwinfo.so
IWINFO_LUA_LDFLAGS = $(LDFLAGS) -shared -L. -liwinfo -llua
IWINFO_LUA_OBJ     = iwinfo_lua.o

IWINFO_CLI         = iwinfo
IWINFO_CLI_LDFLAGS = $(LDFLAGS) -L. -liwinfo
IWINFO_CLI_OBJ     = iwinfo_cli.o

ifneq ($(filter wl,$(IWINFO_BACKENDS)),)
	IWINFO_CFLAGS  += -DUSE_WL
	IWINFO_LIB_OBJ += iwinfo_wl.o
	ifneq ($(filter 7X,$(BCM_SDK)),)
		IWINFO_CFLAGS += -DUSE_WL_SDK7X
	endif
	ifneq ($(filter SUPPORT_BRCM_2G_AX,$(BCM_SDK)),)
		IWINFO_CFLAGS += -DSUPPORT_BRCM_2G_AX
	endif
	ifneq ($(filter SUPPORT_BRCM_5G_AX,$(BCM_SDK)),)
		IWINFO_CFLAGS += -DSUPPORT_BRCM_5G_AX
	endif
		ifneq ($(filter SUPPORT_BRCM_2G_BE,$(BCM_SDK)),)
		IWINFO_CFLAGS += -DSUPPORT_BRCM_2G_BE
	endif
	ifneq ($(filter SUPPORT_BRCM_5G_BE,$(BCM_SDK)),)
		IWINFO_CFLAGS += -DSUPPORT_BRCM_5G_BE
	endif
	ifneq ($(filter AX,$(BCM_SDK)),)
		IWINFO_CFLAGS += -DUSE_WL_SDKAX
	endif


endif

ifneq ($(filter madwifi,$(IWINFO_BACKENDS)),)
	IWINFO_CFLAGS  += -DUSE_MADWIFI
	IWINFO_LIB_OBJ += iwinfo_madwifi.o
endif

ifneq ($(filter mtk,$(IWINFO_BACKENDS)),)
	IWINFO_CFLAGS  += -DUSE_MTK
	IWINFO_LIB_OBJ += iwinfo_mtk.o
endif

ifneq ($(filter qcawifi,$(IWINFO_BACKENDS)),)
	IWINFO_CFLAGS  += -DUSE_QCAWIFI
	IWINFO_LIB_OBJ += iwinfo_qcawifi.o
endif

ifneq ($(filter nl80211,$(IWINFO_BACKENDS)),)
	IWINFO_CFLAGS      += -DUSE_NL80211
	IWINFO_CLI_LDFLAGS += -lnl-tiny
	IWINFO_LIB_LDFLAGS += -lnl-tiny
	IWINFO_LIB_OBJ     += iwinfo_nl80211.o
endif

ifneq ($(filter intelwifi,$(IWINFO_BACKENDS)),)
	IWINFO_CFLAGS      += -DUSE_INTEL
	IWINFO_LIB_OBJ     += iwinfo_intel.o
	IWINFO_CLI_LDFLAGS += -lnl-tiny
	IWINFO_LIB_LDFLAGS += -lnl-tiny
	IWINFO_LIB_OBJ     += iwinfo_nl80211.o
endif

ifneq ($(filter rtk,$(IWINFO_BACKENDS)),)
	IWINFO_CFLAGS      += -DUSE_RTK
	IWINFO_LIB_OBJ     += iwinfo_rtk.o
endif

ifneq ($(filter mtk7915,$(DBDCCFLAGS)),)
	IWINFO_CFLAGS      += -DPACKAGE_WIFI_DBDC_MODE -DPACKAGE_WIFI_DBDC_CHIP_MTK7915 -DPACKAGE_WIFI_HE_AX
endif

ifneq ($(filter mtk7986,$(DBDCCFLAGS)),)
	IWINFO_CFLAGS      += -DPACKAGE_WIFI_DBDC_MODE -DPACKAGE_WIFI_DBDC_CHIP_MTK7986 -DPACKAGE_WIFI_HE_AX
endif

ifneq ($(filter mtk7981,$(DBDCCFLAGS)),)
	IWINFO_CFLAGS      += -DPACKAGE_WIFI_DBDC_MODE -DPACKAGE_WIFI_DBDC_CHIP_MTK7981 -DPACKAGE_WIFI_HE_AX
endif

%.o: %.c
	$(CC) $(IWINFO_CFLAGS) $(FPIC) -c -o $@ $<

compile: clean $(IWINFO_LIB_OBJ) $(IWINFO_LUA_OBJ) $(IWINFO_CLI_OBJ)
	$(CC) $(IWINFO_LIB_LDFLAGS) -o $(IWINFO_LIB) $(IWINFO_LIB_OBJ)
	$(CC) $(IWINFO_LUA_LDFLAGS) -o $(IWINFO_LUA) $(IWINFO_LUA_OBJ)
	$(CC) $(IWINFO_CLI_LDFLAGS) -o $(IWINFO_CLI) $(IWINFO_CLI_OBJ)

clean:
	rm -f *.o $(IWINFO_LIB) $(IWINFO_LUA) $(IWINFO_CLI)
