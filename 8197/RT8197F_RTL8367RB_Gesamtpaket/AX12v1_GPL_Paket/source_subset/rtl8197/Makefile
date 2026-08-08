############################################################################
#
# Realtek Semiconductor Corp.
#
# Makefile -- Top level dist makefile.
#
# Tony Wu (tonywu@realtek.com)
# Oct. 12, 2008
#

ifneq (.config,$(wildcard .config))
TARGET := config
#
# DIR_ROOT Used when .config not Exist
#
DIR_ROOT = $(shell pwd)
DIR_USERS = $(DIR_ROOT)/users
DIR_BZBOX = $(DIR_ROOT)/users/busybox-1.13
DIR_LINUX = $(DIR_ROOT)/linux-2.6.30
export DIR_ROOT DIR_USERS DIR_BZBOX DIR_LINUX
else

include .config

#
# variables
#
DIR_ROOT = $(shell pwd)
DIR_RSDK = $(DIR_ROOT)/$(CONFIG_RSDKDIR)

ifeq ($(CONFIG_ARCH_CPU_MIPS),y)
VAR_ARCH:= mips
CROSS_TARGET := mips-linux
CROSS_COMPILE:= msdk-linux-
ifeq ($(CONFIG_BOARD_rtl8197F),y)
ifeq ($(CONFIG_LINUX_3.10),y)
VAR_ARCH:= mips-ori
WIFI_DRV_PATH:= drivers/net/wireless/rtl8192cd
else
WIFI_DRV_PATH:= drivers/net/wireless/realtek/rtl8192cd
endif
endif
else
VAR_ARCH:= rlx
CROSS_TARGET := rsdk-linux
CROSS_COMPILE := rsdk-linux-
endif
export VAR_ARCH CROSS_COMPILE CROSS_TARGET WIFI_DRV_PATH

HOST_CPU_NUM = $(shell cat /proc/cpuinfo | grep -c processor)

ifeq ($(CONFIG_ARCH_CPU_EL),y)
VAR_ENDIAN := EL
else
VAR_ENDIAN := EB
endif

DIR_BOARD = $(DIR_ROOT)/target
DIR_ROMFS = $(DIR_ROOT)/target/romfs
DIR_TMPFS = $(DIR_ROOT)/target/tmpfs
DIR_IMAGE = $(DIR_ROOT)/target/image
DIR_TOOLS = $(DIR_ROOT)/target/tools
DIR_USERS = $(DIR_ROOT)/users
DIR_BZBOX = $(DIR_ROOT)/$(CONFIG_BZBOXDIR)
DIR_LINUX = $(DIR_ROOT)/$(CONFIG_LINUXDIR)

ROMFSINST = $(DIR_ROOT)/config/romfsinst

RCSCONFIG = $(DIR_ROMFS)/etc/init.d/rcS.conf

### BACKPORT
BACKPORT_DIR = backports-5.2.8-1
BACKPORT_LOCAL_SYM = local-symbols
BACKPORT_LOCAL_SYM_OLD = local-symbols.old
BACKPORT_LOCAL_SYM_UPDATED = .local-sym-updated
BACKPORT_CONFIG = $(DIR_ROOT)/$(BACKPORT_DIR)/.config
BACKPORT_AUTOCONF_H = $(BACKPORT_DIR)/backport-include/backport/autoconf.h
WIFI_DRIVER_DIR = $(DIR_LINUX)/drivers/net/wireless/realtek/g6_wifi_driver
FW_IMG = $(WIFI_DRIVER_DIR)/phl/hal_g6/mac/fw_ax/rtl8852a/RTL8852Afw.bin
LINUX_CONFIG = $(DIR_LINUX)/.config
DIR_ROMFS_ETC = $(DIR_ROMFS)/etc

#### BACKPORT MODULE ####
BACKPORT_KERNEL_MODULE_DIR :=
BACKPORT_SUPPORT := $(shell if grep -q 'CONFIG_RTL8852AE_BACKPORTS=y' $(LINUX_CONFIG) ; \
                                then echo 1; else echo 0; fi)
WFO_VIRT_SUPPORT := $(shell if grep -q 'CONFIG_RTK_WFO=y' $(LINUX_CONFIG) ; \
                                then echo 1; else echo 0; fi)
THER_CTRL_SUPPORT := $(shell if grep -q 'CPTCFG_THER_CTRL=m' $(BACKPORT_CONFIG) ; \
                                then echo 1; else echo 0; fi)

ifeq ($(BACKPORT_SUPPORT), 1)
BACKPORT_KERNEL_MODULE_DIR += backports-5.2.8-1/
BACKPORT_KERNEL_MODULE_DIR += $(BACKPORT_DIR)/drivers/net/wireless/realtek/g6_wifi_driver/
#BACKPORT_KERNEL_MODULE_DIR += $(BACKPORT_DIR)/drivers/net/wireless/realtek/rtl8821cu/
#BACKPORT_KERNEL_MODULE_DIR += $(BACKPORT_DIR)/drivers/net/wireless/realtek/rtl8723fu
endif
ifeq ($(WFO_VIRT_SUPPORT), 1)
BACKPORT_KERNEL_MODULE_DIR += linux-4.4.x/drivers/net/wireless/realtek/wfo_virt/
endif
ifeq ($(THER_CTRL_SUPPORT), 1)
BACKPORT_KERNEL_MODULE_DIR += $(DIR_LINUX)/drivers/net/wireless/realtek/ther_ctrl/
endif
#### BACKPORT END

MAKE = PATH=$(DIR_RSDK)/bin:$(PATH); make
LSTRIP = PATH=$(DIR_RSDK)/bin:$(PATH); rsdk-linux-lstrip
SSTRIP = PATH=$(DIR_RSDK)/bin:$(PATH); rsdk-linux-sstrip

ifeq ($(CONFIG_ARCH_CPU_MIPS),y)
LSTRIP = PATH=$(DIR_RSDK)/bin:$(PATH); msdk-linux-lstrip
SSTRIP = PATH=$(DIR_RSDK)/bin:$(PATH); msdk-linux-sstrip
endif

export DIR_ROOT DIR_RSDK DIR_ROMFS DIR_IMAGE DIR_BOARD DIR_TMPFS
export DIR_LINUX DIR_BZBOX ROMFSINST DIR_TOOLS DIR_USERS 
export RCSCONFIG

ERROR :=
ifneq ($(CONFIG_BZBOXDIR),$(wildcard $(CONFIG_BZBOXDIR)))
ERROR += '$(CONFIG_BZBOXDIR) does not exist '
endif

ifneq ($(CONFIG_LINUXDIR),$(wildcard $(CONFIG_LINUXDIR)))
ERROR += '$(CONFIG_LINUXDIR) does not exist '
endif

ifneq ($(CONFIG_RSDKDIR),$(wildcard $(CONFIG_RSDKDIR)))
ERROR += '$(CONFIG_RSDKDIR) does not exist'
endif

ifeq ($(ERROR),)
# for ramfs or MP build, rootfs must be ready before kernel
TARGET := backports_autoconf bins romfs modules external_modules linux backports image
#TARGET := modules linux bins romfs image
else
TARGET := error
endif

endif

all: $(TARGET)

error:
	@echo
	@echo "=== NOTICE ===" 
	@echo
	@for X in $(ERROR) ; do \
		echo ERROR: $$X; \
	done
	@echo
	@echo "Please run 'make config' to reconfigure"
	@echo

#
# 0. target selection
#
.PHONY: config menuconfig silentconfig
config menuconfig:
#	@chmod u+x config/genconfig
#	@chmod u+x config/setconfig
#	@config/genconfig > Kconfig
#	@config/mconf Kconfig
#	@if egrep "^CONFIG_BOARD_rtl8196eu=y" .config > /dev/null; then \
#        cp $(shell pwd)/linux-2.6.30/drivers/usb/Kconfig-otg $(shell pwd)/linux-2.6.30/drivers/usb/Kconfig; \
#    else \
#        cp $(shell pwd)/linux-2.6.30/drivers/usb/Kconfig-not-otg $(shell pwd)/linux-2.6.30/drivers/usb/Kconfig; \
#	fi	
#	@config/setconfig defaults
#	@if egrep "^CONFIG_MCONF_LINUX=y" .config > /dev/null; then \
#			$(MAKE) -C $(DIR_LINUX) menuconfig ARCH=$(VAR_ARCH); \
#	fi
#	if egrep "^CONFIG_SLOT_0_8192FE=y" $(shell pwd)/linux-3.10/.config > /dev/null; then \
#        rm $(shell pwd)/linux-3.10/drivers/net/wireless/rtl8192cd; \
#		ln -s rtl8192cd_92f $(shell pwd)/linux-3.10/drivers/net/wireless/rtl8192cd; \
#    else \
#		rm $(shell pwd)/linux-3.10/drivers/net/wireless/rtl8192cd; \
#		ln -s ../../../../linux-2.6.30/drivers/net/wireless/rtl8192cd $(shell pwd)/linux-3.10/drivers/net/wireless/rtl8192cd; \
#	fi		
#	@if egrep "^CONFIG_MCONF_USERS=y" .config > /dev/null; then \
#		$(MAKE) -C $(DIR_USERS) menuconfig; \
#	fi
#	@if egrep "^CONFIG_MCONF_BZBOX=y" .config > /dev/null; then \
#		$(MAKE) -C $(DIR_BZBOX) menuconfig; \
#	fi
	$(MAKE) menuconfig_phase1
	$(MAKE) menuconfig_backport
	@config/setconfig final
	@config/hdrconfig $(DIR_ROOT)

silentconfig:
	$(MAKE) menuconfig_phase1_silent
	$(MAKE) menuconfig_backport
	@config/setconfig final
	@config/hdrconfig $(DIR_ROOT)


.PHONY: menuconfig_phase1 menuconfig_phase1_silent
menuconfig_phase1:
	@chmod u+x config/genconfig
	@chmod u+x config/setconfig
	@config/genconfig > Kconfig
	@config/mconf Kconfig
	@config/setconfig defaults
	@if egrep "^CONFIG_MCONF_LINUX=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_LINUX) menuconfig ARCH=$(VAR_ARCH); \
	fi
	@if egrep "^CONFIG_MCONF_USERS=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_USERS) menuconfig; \
	fi
	@if egrep "^CONFIG_MCONF_BZBOX=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_BZBOX) menuconfig; \
	fi

menuconfig_phase1_silent:
	@chmod u+x config/genconfig
	@chmod u+x config/setconfig
	@config/genconfig > Kconfig
	@config/setconfig defaults
	@if egrep "^CONFIG_MCONF_LINUX=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_LINUX) menuconfig ARCH=$(VAR_ARCH); \
	fi
	@if egrep "^CONFIG_MCONF_USERS=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_USERS) menuconfig; \
	fi
	@if egrep "^CONFIG_MCONF_BZBOX=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_BZBOX) menuconfig; \
	fi


#
# 1. user lib
#
#.PHONY: libs
#libs:
#	$(MAKE) -C $(DIR_USERS) lib || exit $$?

#
# 2. user app
#
.PHONY: users bins
users bins:
	if egrep "^CONFIG_BOARD_rtl8197F=y" .config > /dev/null; then \
		$(MAKE) -C $(DIR_LINUX) include/config/auto.conf ARCH=$(VAR_ARCH) > /dev/null; \
	else \
		$(MAKE) -C $(DIR_LINUX) dep > /dev/null; \
	fi

	if egrep "^CONFIG_RTL8852AE_BACKPORTS=y" $(LINUX_CONFIG) > /dev/null; then \
		if egrep "^CONFIG_APP_HOSTAPD=y" $(DIR_USERS)/.config > /dev/null; then \
			cd $(DIR_USERS)/hostapd-2.9/hostapd; \
			cp Makefile.97G Makefile; \
			cd -; \
		fi; \
		if egrep "^CONFIG_APP_WPA_SUPPLC=y" $(DIR_USERS)/.config > /dev/null; then \
			cd $(DIR_USERS)/wpa_supplicant-2.9/wpa_supplicant;\
			cp Makefile.97G Makefile; \
			cd -;\
		fi; \
	fi \

	if  egrep "^CONFIG_APP_BOA=y" $(DIR_USERS)/.config > /dev/null; then \
		cd $(DIR_USERS)/; \
		rm -rf boa; \
		if egrep "^CONFIG_RTL8852AE_BACKPORTS=y" $(LINUX_CONFIG) > /dev/null; then \
			ln -s boa_indep_stru boa; \
		else \
			ln -s boa_share_stru boa; \
		fi; \
		cd -; \
	fi \

	$(MAKE) -C $(DIR_USERS) || exit $$?

#
#2.1 user menuconfig
#
users_menuconfig:
	$(MAKE) -C $(DIR_USERS) menuconfig; 

#
# 3. romfs
#
#	[ -d $(DIR_ROMFS) ] || mkdir $(DIR_ROMFS)
.PHONY: romfs
romfs:
	$(MAKE) -C $(DIR_BOARD) romfs || exit $$?
	cp -R $(DIR_RSDK)/lib/*.so $(DIR_ROMFS)/lib
	cp -R $(DIR_RSDK)/lib/*.so.* $(DIR_ROMFS)/lib
	chmod 755 $(DIR_ROMFS)/lib/*.so
	$(MAKE) -C $(DIR_USERS) romfs || exit $$?
	$(LSTRIP) $(DIR_ROMFS)

#
# 4.0 kernel modules (if any)
#
.PHONY: modules
modules:
	$(MAKE) -C $(DIR_LINUX) ARCH=$(VAR_ARCH) modules || exit $$?; \
	$(MAKE) -C $(DIR_LINUX) ARCH=$(VAR_ARCH) modules_install INSTALL_MOD_PATH=$(DIR_ROMFS) || exit $$?; \

#
# 4.1 kernel image
#
.PHONY: linux
linux:
	. $(DIR_LINUX)/.config; if [ "$$CONFIG_INITRAMFS_SOURCE" != "" ]; then \
        cd $(DIR_LINUX); \
        mkdir -p $(DIR_ROMFS); \
        touch $$CONFIG_INITRAMFS_SOURCE | awk '{print $2}' || exit 1; \
        cd $(DIR_ROOT); \
	fi
	$(MAKE) -C $(DIR_LINUX) ARCH=$(VAR_ARCH) -j $(HOST_CPU_NUM)|| exit $$?; \
#		$(MAKE) -C $(DIR_USERS)/MAP-master/modules || exit $$?; \
	if [ -f $(DIR_LINUX)/vmlinux ]; then \
        ln -f $(DIR_LINUX)/vmlinux $(DIR_LINUX)/linux ; \
	fi

uImage:
	$(MAKE) -C $(DIR_LINUX) ARCH=$(VAR_ARCH) uImage;

#
#4.2 kernel menuconfig
#
linux_menuconfig:
	$(MAKE) -C $(DIR_LINUX) ARCH=$(VAR_ARCH) menuconfig;

#
#4.3 external modules
#
.PHONY: external_modules
external_modules:
	$(MAKE) -C $(DIR_USERS) ex_mod || exit $$?

#
# 5. image
#
.PHONY: image
image:
	[ -d $(DIR_IMAGE) ] || mkdir $(DIR_IMAGE)
	$(MAKE) -C $(DIR_BOARD) image


# 6.0 backports menuconfig
#
.PHONY: menuconfig_backport
menuconfig_backport:
	@. $(LINUX_CONFIG); if [ "$$CONFIG_RTL8852AE_BACKPORTS" = "y" ]; then \
	if [ -d $(BACKPORT_DIR) ]; then \
		cd $(BACKPORT_DIR)/drivers/net/wireless/realtek; \
		rm -f g6_wifi_driver && ln -sf $(DIR_LINUX)/drivers/net/wireless/realtek/g6_wifi_driver g6_wifi_driver; \
		rm -f g6_wifi_driver_mp && ln -sf $(DIR_LINUX)/drivers/net/wireless/realtek/g6_wifi_driver_mp g6_wifi_driver_mp; \
		rm -f rtl8723fu && ln -sf $(DIR_LINUX)/drivers/net/wireless/realtek/rtl8723fu rtl8723fu; \
		rm -f ther_ctrl && ln -sf $(DIR_LINUX)/drivers/net/wireless/realtek/ther_ctrl ther_ctrl; \
		rm -f wfo_virt && ln -sf $(DIR_LINUX)/drivers/net/wireless/realtek/wfo_virt wfo_virt; \
		cd -; \
		if [ ! -d $(BACKPORT_DIR)/drivers/net/wireless/realtek/rtl8723fu ]; then \
			sed -i '/realtek\/rtl8723fu/s/^/#/' $(BACKPORT_DIR)/drivers/net/wireless/Kconfig; \
		else \
			sed -i '/realtek\/rtl8723fu/s/^#+//' $(BACKPORT_DIR)/drivers/net/wireless/Kconfig; \
		fi; \
		if [ ! -d $(BACKPORT_DIR)/drivers/net/wireless/realtek/rtl8192fe ]; then \
			sed -i '/realtek\/rtl8192fe/s/^/#/' $(BACKPORT_DIR)/drivers/net/wireless/realtek/Kconfig; \
		else \
			sed -i '/realtek\/rtl8192fe/s/^#+//' $(BACKPORT_DIR)/drivers/net/wireless/realtek/Kconfig; \
		fi; \
		if [ ! -d $(BACKPORT_DIR)/drivers/net/wireless/realtek/rtl8197g ]; then \
			sed -i '/realtek\/rtl8197g/s/^/#/' $(BACKPORT_DIR)/drivers/net/wireless/realtek/Kconfig; \
		else \
			sed -i '/realtek\/rtl8197g/s/^#+//' $(BACKPORT_DIR)/drivers/net/wireless/realtek/Kconfig; \
		fi; \
		unset CC && \
		cd $(BACKPORT_DIR) && sed -i "/^[^#]*g6_wifi_driver_mp/ s/^/#/g" drivers/net/wireless/realtek/Kconfig && make oldconfig; \
	fi \
	fi

#
#6.1 backports build
#	
backports_autoconf:
	@. $(LINUX_CONFIG); \
	if [ "$$CONFIG_RTL8852AE_BACKPORTS" = "y" ]; then \
		if [ -d $(BACKPORT_DIR) ]; then \
			make -C $(BACKPORT_DIR)  backport-include/backport/autoconf.h; \
		fi \
	fi
backports:
	@. $(LINUX_CONFIG); \
	if [ "$$CONFIG_RTL8852AE_BACKPORTS" = "y" ]; then \
		if [ -d $(BACKPORT_DIR) ]; then \
			unset CC && \
			cd $(BACKPORT_DIR) && sed -i "/^[^#]*g6_wifi_driver_mp/ s/^/#/g" drivers/net/wireless/realtek/Kconfig && cd - && \
			if [ ! -f $(BACKPORT_DIR)/$(BACKPORT_LOCAL_SYM_UPDATED) ]; then \
				echo -n "Update backports local-symbols..."; \
				cp $(BACKPORT_DIR)/$(BACKPORT_LOCAL_SYM) $(BACKPORT_DIR)/$(BACKPORT_LOCAL_SYM_OLD); \
				cd $(BACKPORT_DIR) && sed -n "/^[\t| ]*config/p" ./drivers/net/wireless/realtek/g6_wifi_driver/Kconfig > .kconf.tmp && sed -i "s/^[\t| ]*config /CPTCFG_/" .kconf.tmp && sed -i "s/$$/&=/g" .kconf.tmp && cat .kconf.tmp >> local-symbols && rm -f .kconf.tmp && touch $(BACKPORT_LOCAL_SYM_UPDATED) && cd - ;\
echo "done."; \
			fi && \
			[ -d $(DIR_ROOT)/include/rtk ] || mkdir -p $(DIR_ROOT)/include/rtk && \
			[ -d $(DIR_ROOT)/include/backports ] || mkdir -p $(DIR_ROOT)/include/backports && \
			ln -fs $(DIR_ROOT)/$(BACKPORT_DIR)/backport-include/backport/autoconf.h include/rtk/backport_autoconf.h && \
			ln -fs $(DIR_ROOT)/$(BACKPORT_DIR)/backport-include/backport/autoconf.h include/backports/autoconf.h && \
			$(MAKE) -C $(BACKPORT_DIR) -j $(HOST_CPU_NUM) || exit $$? && \
			[ $(shell egrep "^CONFIG_FILE_FWIMG" $(WIFI_DRIVER_DIR)/Makefile | sed 's/[[:blank:]]//g; s/CONFIG_FILE_FWIMG=//g') = "y" ] && [ -f $(FW_IMG) ] && cp $(FW_IMG) $(DIR_ROMFS_ETC)/conf/rtl8852ae/ || : && \
			find $(BACKPORT_KERNEL_MODULE_DIR) -name "*.ko" -exec cp {} $(DIR_ROMFS_ETC)/conf \;;\
		fi \
	else \
		echo "CONFIG_RTL8852AE_BACKPORTS not set in linux.config, skip backports" ;\
	fi
#
# x. short-cut to build and clean one application
#
linux-clean:
	$(MAKE) -C $(DIR_LINUX) ARCH=$(VAR_ARCH) clean;

%_build:
	$(MAKE) -C users $(@:_build=)

%-build:
	$(MAKE) -C users $(@:-build=)

%_clean:
	$(MAKE) -C users/$(@:_clean=) clean

%-clean:
	$(MAKE) -C users/$(@:-clean=) clean

#.PHONY: version
#version:
#	$(MAKE) -C $(DIR_USERS)/goahead-2.1.1/LINUX release

.PHONY: clean  distclean
clean:
	$(MAKE) -C $(DIR_USERS) clean
	$(MAKE) -C $(DIR_LINUX) ARCH=$(VAR_ARCH) clean;
#	$(MAKE) -C $(DIR_USERS)/MAP-master/modules clean;
	if [ -f $(BACKPORT_DIR)/$(BACKPORT_LOCAL_SYM_UPDATED) ]; then \
		cd $(BACKPORT_DIR) ; mv $(BACKPORT_LOCAL_SYM_OLD) $(BACKPORT_LOCAL_SYM); rm $(BACKPORT_LOCAL_SYM_UPDATED) -f; \
	fi
	if [ -f $(BACKPORT_AUTOCONF_H) ];then \
		rm -f $(BACKPORT_AUTOCONF_H); \
	fi
	rm -rf $(DIR_ROMFS)
	mkdir -p $(DIR_ROMFS)
	mkdir -p $(DIR_IMAGE)

distclean:
	$(MAKE) -C $(DIR_USERS) distclean
