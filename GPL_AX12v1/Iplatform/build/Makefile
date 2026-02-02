#
# Generic Makefile to build images for various type of products.
# Author: TengFei
# Date: Aug 13th, 2014.
#

PRODUCT_NAME=AX12v1

#
# our make tool
#
SUBMAKE:=$(MAKE)

#
# path name of common dirs
#
BUILD_DIR:=$(PWD)
CONFIG_DIR:=$(BUILD_DIR)/product_configs/$(PRODUCT_NAME)
PLATFORM_DIR:=$(BUILD_DIR)/../openwrt
IMAGE_DIR:=$(BUILD_DIR)/../image/$(PRODUCT_NAME)

#
# basic Vars for special product
#
include $(CONFIG_DIR)/basic.config

# default target
platform:

#
# basic targets for special product
#
include $(CONFIG_DIR)/product.mk

export CONFIG_PRODUCT_NAME=$(PRODUCT_NAME)

platform: iplatform_prep sdk iplatform

toolchain:
	@if [ ! -d $(TOOLCHAIN_ROOT_DIR) ]; then \
		echo Uncompressing toolchain, please wait... && \
		tar -jxf $(TOOLCHAIN_DIR)/$(TOOLCHAIN_PACKAGE) -C $(TOOLCHAIN_DIR) ; \
	 fi
	@echo make $@ done.

iplatform.config:
	@cp -f $(CONFIG_DIR)/iplatform.config $(PLATFORM_DIR)/.config
	@cp -f $(CONFIG_DIR)/iplatform.config $(CONFIG_DIR)/.iplatform.config.tmp
	sed -e 's|^CONFIG_TOOLCHAIN_ROOT=.*|CONFIG_TOOLCHAIN_ROOT="$(TOOLCHAIN_DIR)"|' \
		-e 's|^CONFIG_LIBC_ROOT_DIR=.*|CONFIG_LIBC_ROOT_DIR="$(TOOLCHAIN_DIR)"|' \
		-e 's|^CONFIG_LIBGCC_ROOT_DIR=.*|CONFIG_LIBGCC_ROOT_DIR="$(TOOLCHAIN_DIR)"|' \
		-e 's|^CONFIG_LIBPTHREAD_ROOT_DIR=.*|CONFIG_LIBPTHREAD_ROOT_DIR="$(TOOLCHAIN_DIR)"|' \
		-e 's|^CONFIG_LIBRT_ROOT_DIR=.*|CONFIG_LIBRT_ROOT_DIR="$(TOOLCHAIN_DIR)"|' \
		-e 's|^CONFIG_LIBSTDCPP_ROOT_DIR=.*|CONFIG_LIBSTDCPP_ROOT_DIR="$(TOOLCHAIN_DIR)"|' \
			$(CONFIG_DIR)/.iplatform.config.tmp > $(CONFIG_DIR)/iplatform.config;
	@echo make $@ done.

iplatform_prep: feed.conf toolchain iplatform.config iplatform_package/symlinks $(EXT_PREPARE)

feed.conf:
	@if [ -e $(BUILD_DIR)/feeds.conf ]; then \
		cp -f $(BUILD_DIR)/feeds.conf $(PLATFORM_DIR)/feeds.conf; \
	fi

iplatform: iplatform_world

%::
	$(if $(filter iplatform_%,$@),\
		@$(MAKE) -C $(PLATFORM_DIR) $(patsubst iplatform_%, %, $@) $(PLATFORM_FLAGS),\
		@$(MAKE) -C $(SDK_BUILD_DIR) $@ $(SDK_FLAGS))

.PHONY: toolchain iplatform_prep iplatform platform sdk \
	iplatform.config $(EXT_PHONY)
