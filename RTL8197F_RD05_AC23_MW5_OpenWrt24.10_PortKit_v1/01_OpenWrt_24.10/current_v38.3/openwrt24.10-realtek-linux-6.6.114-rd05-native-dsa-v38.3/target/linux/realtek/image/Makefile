# SPDX-License-Identifier: GPL-2.0-only

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

KERNEL_LOADADDR = 0x80100000

# Optional user-provided full SPI-NOR templates for dangerous fullflash test images.
# Override with:
#   make RD05_SPI_TEMPLATE=/absolute/path/to/original-rd05-spi-backup.bin
#   make TENDA_LYNX_SPI_TEMPLATE=/absolute/path/to/original-tenda-8m-spi-backup.bin
RD05_SPI_TEMPLATE ?= $(TOPDIR)/rd05-spi-template.bin
TENDA_LYNX_SPI_TEMPLATE ?= $(TOPDIR)/backup_bh25q64.bin

DEVICE_VARS += RTL8197F_LZMA_TEXT_START
DEVICE_VARS += RTL8197F_KERNEL_CMDLINE

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

# Realtek RTL8197F/Xiaomi RD05 bootmiwifi-compatible non-secure image
# wrapper.  This generates the small cs6c-style IMG_HEADER_T used by the
# RTL8197-class boot code before the payload and appends the additive checksum.
define Build/rtl8197f-image
	$(TOPDIR)/scripts/rtl8197f-image.py \
		--signature $(RTL8197F_IMAGE_SIGNATURE) \
		--start-addr $(RTL8197F_LOADADDR) \
		--burn-addr $(RTL8197F_BURNADDR) \
		--header-size $(RTL8197F_HEADER_SIZE) \
		$(if $(RTL8197F_IMAGE_PAD),--pad-to $(RTL8197F_IMAGE_PAD)) \
		$@ $@.new
	mv $@.new $@
endef


# Build a raw self-extracting MIPS LZMA loader for RTL8197F bootcode.
# The RD05 bootmiwifi path does not parse U-Boot uImage payloads. It copies
# IMG_HEADER_T.len-2 bytes to IMG_HEADER_T.startAddr and jumps directly there.
# Therefore the flash kernel payload must begin with executable MIPS code.
define Build/rtl8197f-loader-kernel
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



# Create a complete Xiaomi R4/RD05 16 MiB SPI-NOR test image from a local
# original SPI dump/template. The current $@ already contains the generated
# cs6c kernel payload; the rootfs image is passed through OpenWrt's IMAGE_ROOTFS helper.
define Build/rtl8197f-rd05-fullflash
	$(TOPDIR)/scripts/rtl8197f-rd05-fullflash.py \
		--template "$(RD05_SPI_TEMPLATE)" \
		--kernel "$@" \
		--rootfs "$(IMAGE_ROOTFS)" \
		--output "$@.new" \
		--kernel-offset $(RTL8197F_KERNEL_OFFSET) \
		--rootfs-offset $(RTL8197F_ROOTFS_OFFSET) \
		--rootfs-data-offset $(RTL8197F_ROOTFS_DATA_OFFSET)
	mv $@.new $@
endef



# Create a complete 8 MiB Tenda/Lynx RTL8197F SPI-NOR test image from a local
# original SPI dump/template.  The current $@ already contains the generated
# cr6c kernel payload; the rootfs image is passed through OpenWrt's IMAGE_ROOTFS helper.
define Build/rtl8197f-tenda-lynx-fullflash
	$(TOPDIR)/scripts/rtl8197f-tenda-lynx-fullflash.py \
		--template "$(TENDA_LYNX_SPI_TEMPLATE)" \
		--kernel "$@" \
		--rootfs "$(IMAGE_ROOTFS)" \
		--output "$@.new" \
		--kernel-offset $(RTL8197F_KERNEL_OFFSET) \
		--rootfs-offset $(RTL8197F_ROOTFS_OFFSET) \
		--config-offset $(RTL8197F_ROOTFS_DATA_OFFSET)
	mv $@.new $@
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
