# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80100000

# RTL8197F full-SPI outputs are enabled by default in this PRIVATE tree.
# The bundled templates are complete device dumps and must never be published.
# Set RTL8197F_BUILD_FULLFLASH=0 only when intentionally disabling these images.
RTL8197F_BUILD_FULLFLASH ?= 1
RTL8197F_FULLFLASH_TEMPLATE_DIR ?= $(TOPDIR)/fullflash-templates
RD05_SPI_TEMPLATE ?= $(RTL8197F_FULLFLASH_TEMPLATE_DIR)/xiaomi-r4-rd05-spi.bin
TENDA_AC23_SPI_TEMPLATE ?= $(RTL8197F_FULLFLASH_TEMPLATE_DIR)/tenda-ac23-spi.bin
TENDA_MW5_SPI_TEMPLATE ?= $(RTL8197F_FULLFLASH_TEMPLATE_DIR)/tenda-nova-mw5-spi.bin
RTL8197F_FULLFLASH_ENABLED := $(filter 1 y yes true,$(RTL8197F_BUILD_FULLFLASH))
RD05_FULLFLASH_IMAGE := $(if $(RTL8197F_FULLFLASH_ENABLED),squashfs-spi-full.bin)
TENDA_AC23_FULLFLASH_IMAGE := $(if $(RTL8197F_FULLFLASH_ENABLED),squashfs-spi-full.bin)
TENDA_MW5_FULLFLASH_IMAGE := $(if $(RTL8197F_FULLFLASH_ENABLED),squashfs-spi-full.bin)

DEVICE_VARS += RTL8197F_LZMA_TEXT_START RTL8197F_KERNEL_CMDLINE

DEVICE_VARS += \
	BELKIN_HEADER \
	BELKIN_MODEL \
        CAMEO_BOARD_MODEL \
        CAMEO_BOARD_VERSION \
        CAMEO_CUSTOMER_SIGNATURE \
        CAMEO_KERNEL_PART \
        CAMEO_KERNEL_PART_SIZE \
        CAMEO_ROOTFS_PART \
        H3C_DEVICE_ID \
        H3C_PRODUCT_ID \
	LINKSYS_HEADER \
	RTL8197F_BURNADDR \
	RTL8197F_HEADER_SIZE \
	RTL8197F_IMAGE_PAD \
	RTL8197F_IMAGE_SIGNATURE \
	RTL8197F_LOADADDR \
	RTL8197F_LEN_INCLUDES_HEADER_PAD \
	RTL8197F_KERNEL_OFFSET \
	RTL8197F_ROOTFS_OFFSET \
	RTL8197F_ROOTFS_DATA_OFFSET \
        ZYXEL_VERS

define Build/zyxel-vers
       ( echo VERS;\
       for hw in $(ZYXEL_VERS); do\
               echo -n "V9.99($$hw.0) | ";\
               date -d @$(SOURCE_DATE_EPOCH) +%m/%d/%Y;\
       done ) >> $@
endef

define Build/cameo-tag
	$(SCRIPT_DIR)/cameo-tag.py $@ $(CAMEO_KERNEL_PART_SIZE)
endef

define Build/cameo-version
	echo -n "OpenWrt" >> $@
	dd if=/dev/zero bs=25 count=1 >> $@
endef

define Build/cameo-headers
        dd if=$@ bs=$(CAMEO_KERNEL_PART_SIZE) count=1 of=$@.kernel_part; \
        dd if=$@ bs=$(CAMEO_KERNEL_PART_SIZE) skip=1 of=$@.rootfs_part; \
        $(SCRIPT_DIR)/cameo-imghdr.py $@.kernel_part $@.kernel_part.hex \
                "$(CAMEO_BOARD_MODEL)" os $(CAMEO_KERNEL_PART) \
                $(CAMEO_CUSTOMER_SIGNATURE) \
                $(CAMEO_BOARD_VERSION) \
                $(KERNEL_LOADADDR); \
        $(SCRIPT_DIR)/cameo-imghdr.py $@.rootfs_part $@.rootfs_part.hex \
                "$(CAMEO_BOARD_MODEL)" squashfs $(CAMEO_ROOTFS_PART) \
                $(CAMEO_CUSTOMER_SIGNATURE) \
                $(CAMEO_BOARD_VERSION); \
        cat $@.kernel_part.hex $@.rootfs_part.hex > $@
endef

define Build/7z
	$(STAGING_DIR_HOST)/bin/7zr a $(@).new -t7z -m0=lzma $(@)
	mv $@.new $@
endef

define Build/h3c-image
	$(STAGING_DIR_HOST)/bin/mkh3cimg \
		-i $(@) \
		-o $(@).new \
		-c 7z \
		-p $(H3C_PRODUCT_ID) \
		-d $(H3C_DEVICE_ID)
	mv $@.new $@
endef

define Build/h3c-vfs
	$(STAGING_DIR_HOST)/bin/mkh3cvfs \
		-i $(@) \
		-o $(@).new \
		-f openwrt-kernel.bin
	mv $@.new $@
endef

define Build/belkin-header
	$(SCRIPT_DIR)/belkin-header.py $(@) $(@).new $(BELKIN_HEADER) ${BELKIN_MODEL}
	mv $@.new $@
endef

define Build/linksys-image
	$(SCRIPT_DIR)/linksys-image.sh $(@) $(@).new $(LINKSYS_MODEL)
	mv $@.new $@
endef


# Wrap a payload in the Realtek IMG_HEADER_T format used by RTL8197F boot code.
define Build/rtl8197f-image
	$(TOPDIR)/scripts/rtl8197f-image.py \
		--signature $(RTL8197F_IMAGE_SIGNATURE) \
		--start-addr $(RTL8197F_LOADADDR) \
		--burn-addr $(RTL8197F_BURNADDR) \
		--header-size $(RTL8197F_HEADER_SIZE) \
		$(if $(RTL8197F_LEN_INCLUDES_HEADER_PAD),--length-includes-header-padding) \
		$(if $(RTL8197F_IMAGE_PAD),--pad-to $(RTL8197F_IMAGE_PAD)) \
		$@ $@.new
	mv $@.new $@
endef

# Append a SquashFS image with the Realtek/Tenda bootloader checksum trailer.
# The checksum must be generated from the rootfs alone before pad-rootfs adds
# OpenWrt/JFFS2 alignment data to the complete firmware image.
define Build/append-rtl8197f-rootfs
	$(TOPDIR)/scripts/rtl8197f-rootfs-checksum.py build \
		"$(IMAGE_ROOTFS)" "$@.rtl-rootfs"
	cat "$@.rtl-rootfs" >> "$@"
	rm -f "$@.rtl-rootfs"
endef

# Fail before OpenWrt's non-fatal check-size removes the image.  The checker
# predicts the exact erase-block-aligned size that pad-rootfs will produce and
# leaves the input intact on failure so the report remains inspectable.
define Build/rtl8197f-firmware-budget
	python3 $(TOPDIR)/scripts/rtl8197f-firmware-budget.py \
		--image "$@" \
		--limit "$(IMAGE_SIZE)" \
		--erase-size "$(if $(BLOCKSIZE),$(BLOCKSIZE),64k)" \
		--label "$(DEVICE_NAME)"
endef

# Build the raw self-extracting MIPS LZMA loader expected by RTL8197F boot code.
define Build/rtl8197f-loader-kernel
	python3 $(TOPDIR)/scripts/rtl8197f-loader-overlap-check.py \
		--lzma "$@" \
		--kernel-loadaddr $(KERNEL_LOADADDR) \
		--loader-start $(RTL8197F_LZMA_TEXT_START) \
		--safety-margin 64k \
		--label "$(DEVICE_NAME)"
	rm -rf $@.src
	$(MAKE) -C lzma-loader \
		PKG_BUILD_DIR="$@.src" \
		TARGET_DIR="$(dir $@)" \
		LOADER_NAME="$(notdir $@)" \
		LZMA_TEXT_START=$(RTL8197F_LZMA_TEXT_START) \
		LOADADDR=$(KERNEL_LOADADDR) \
		KERNEL_CMDLINE="$(RTL8197F_KERNEL_CMDLINE)" \
		LOADER_DATA="$@" \
		BOARD="$(DEVICE_NAME)" \
		compile loader.bin
	mv "$@.bin" "$@"
	rm -rf $@.src
endef

# Full-SPI images preserve the bundled device-specific boot/factory regions.
# The Python builders validate template size and board markers before writing.
define Build/rtl8197f-rd05-fullflash
	$(TOPDIR)/scripts/rtl8197f-rd05-fullflash.py \
		--template "$(RD05_SPI_TEMPLATE)" \
		--kernel "$@" \
		--rootfs "$(IMAGE_ROOTFS)" \
		--output "$@.new" \
		--manifest "$@.spi-manifest.txt" \
		--kernel-offset $(RTL8197F_KERNEL_OFFSET) \
		--rootfs-offset $(RTL8197F_ROOTFS_OFFSET) \
		--rootfs-data-offset $(RTL8197F_ROOTFS_DATA_OFFSET)
	mv $@.new $@
	sha256sum $@ > $@.sha256
endef

define Build/rtl8197f-tenda-lynx-fullflash
	$(TOPDIR)/scripts/rtl8197f-tenda-lynx-fullflash.py \
		--template "$(TENDA_AC23_SPI_TEMPLATE)" \
		--firmware "$@" \
		--output "$@.new" \
		--manifest "$@.spi-manifest.txt"
	$(TOPDIR)/scripts/rtl8197f-fullflash-verify.py \
		--board ac23 \
		--template "$(TENDA_AC23_SPI_TEMPLATE)" \
		--image "$@.new"
	mv $@.new $@
	sha256sum $@ > $@.sha256
endef

define Build/rtl8197f-tenda-mw5-fullflash
	$(TOPDIR)/scripts/rtl8197f-tenda-mw5-auto-fullflash.sh \
		--topdir "$(TOPDIR)" \
		--template "$(TENDA_MW5_SPI_TEMPLATE)" \
		--firmware "$@" \
		--output "$@.new" \
		--manifest "$@.spi-manifest.txt"
	$(TOPDIR)/scripts/rtl8197f-fullflash-verify.py \
		--board mw5 \
		--template "$(TENDA_MW5_SPI_TEMPLATE)" \
		--image "$@.new"
	mv $@.new $@
	rm -f $@.new.sha256
	sha256sum $@ > $@.sha256
endef

define Device/Default
  PROFILES = Default
  KERNEL := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip
  KERNEL_INITRAMFS := \
	kernel-bin | \
	append-dtb | \
	libdeflate-gzip | \
	uImage gzip
  DEVICE_DTS_DIR := ../dts
  DEVICE_DTS = $$(SOC)_$(1)
  IMAGES := sysupgrade.bin
  IMAGE/sysupgrade.bin := \
	append-kernel | \
	pad-to 64k | \
	append-rootfs | \
	pad-rootfs | \
	check-size | \
	append-metadata
endef

include $(SUBTARGET).mk

$(eval $(call BuildImage))
