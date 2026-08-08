# SPDX-License-Identifier: GPL-2.0-only

include ./common.mk

define Device/realtek_rtl8197f_evb
  DEVICE_VENDOR := Realtek
  DEVICE_MODEL := RTL8197F Evaluation Board
  DEVICE_DTS := rtl8197f_rtl8197f-evb
  IMAGE_SIZE := 32000k
  SUPPORTED_DEVICES += realtek,rtl8197f-evb
  DEVICE_PACKAGES += kmod-usb2 kmod-usb-ohci kmod-usb-ehci
endef
TARGET_DEVICES += realtek_rtl8197f_evb

define Device/realtek_rtl8197f_uboot_test
  DEVICE_VENDOR := Realtek
  DEVICE_MODEL := RTL8197F U-Boot Test Board
  DEVICE_DTS := rtl8197f_rtl8197f-uboot-test
  IMAGE_SIZE := 32000k
  SUPPORTED_DEVICES += realtek,rtl8197f-uboot-test
  DEVICE_PACKAGES += kmod-usb2 kmod-usb-ohci kmod-usb-ehci
endef
TARGET_DEVICES += realtek_rtl8197f_uboot_test

define Device/xiaomi_r4_rd05
  DEVICE_VENDOR := Xiaomi
  DEVICE_MODEL := Mi WiFi R4 (RD05)
  DEVICE_DTS := rtl8197f_xiaomi_r4-rd05
  # bootmiwifi copies the payload to RTL8197F_LOADADDR and jumps directly.
  # Do not wrap the flash kernel payload as a U-Boot uImage; use a raw
  # self-extracting LZMA loader instead.
  KERNEL := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    rtl8197f-loader-kernel
  # Keep initramfs as uImage for manual U-Boot/TFTP bootm tests.
  KERNEL_INITRAMFS := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    uImage lzma
  IMAGE_SIZE := 14336k
  KERNEL_SIZE := 3080192
  ROOTFS_SIZE := 11599872
  RTL8197F_IMAGE_SIGNATURE := cs6c
  RTL8197F_LOADADDR := 0x80cf0000
  RTL8197F_LZMA_TEXT_START := 0x80cf0000
  RTL8197F_BURNADDR := 0x00060000
  RTL8197F_KERNEL_CMDLINE := console=ttyS0,115200n8 lpj=5000000 rootfstype=squashfs,jffs2 clk_ignore_unused
  RTL8197F_HEADER_SIZE := 16
  RTL8197F_KERNEL_OFFSET := 0x00060000
  RTL8197F_ROOTFS_OFFSET := 0x00350000
  RTL8197F_ROOTFS_DATA_OFFSET := 0x00e60000
  SUPPORTED_DEVICES += xiaomi,r4-rd05
  DEVICE_PACKAGES += ip-full ip-bridge ethtool dnsmasq odhcpd-ipv6only firewall4 luci uhttpd uhttpd-mod-ubus rpcd rtl8367d-compat kmod-rtl8192cd-rtl8197f-rd05 wireless-tools
  # Keep first-boot images minimal; RTL8197F WLAN/Ethernet need separate drivers.
  # kernel.bin is the raw self-extracting loader payload.
  # kernel-cs6c.bin adds the Realtek/Xiaomi bootmiwifi-style header for
  # flash kernel-partition experiments; do not flash without serial recovery.
  IMAGES += kernel.bin kernel-cs6c.bin rootfs.bin $(RD05_FULLFLASH_IMAGE)
  IMAGE/kernel.bin := append-kernel | pad-to 64k | check-size $$$$(KERNEL_SIZE)
  IMAGE/kernel-cs6c.bin := append-kernel | rtl8197f-image | pad-to 64k | check-size $$$$(KERNEL_SIZE)
  IMAGE/rootfs.bin := append-rootfs | pad-rootfs | check-size $$$$(ROOTFS_SIZE)
  IMAGE/squashfs-spi-full.bin := append-kernel | rtl8197f-image | pad-to 64k | check-size $$$$(KERNEL_SIZE) | rtl8197f-rd05-fullflash
endef
TARGET_DEVICES += xiaomi_r4_rd05


define Device/tenda_ac23
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := AC23 / Lynx 8000
  DEVICE_DTS := rtl8197f_tenda_ac23
  KERNEL := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    rtl8197f-loader-kernel
  KERNEL_INITRAMFS := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    uImage lzma
  # Complete writable firmware window: 0x030000..0x7e0000.
  IMAGE_SIZE := 7872k
  # Both collected 8 MiB Tenda dumps use 64 KiB SPI erase sectors.  Without
  # this explicit value pad-rootfs tries 4/8/16/64/128/256 KiB and can waste
  # up to 192 KiB beyond the hardware alignment requirement.
  BLOCKSIZE := 64k
  RTL8197F_IMAGE_SIGNATURE := cr6c
  RTL8197F_LOADADDR := 0x80a00000
  RTL8197F_LZMA_TEXT_START := 0x80a00000
  RTL8197F_BURNADDR := 0x00030000
  RTL8197F_KERNEL_CMDLINE := console=ttyS0,115200n8 lpj=5000000 rootfstype=squashfs,jffs2 clk_ignore_unused
  # Byte analysis of the stock SPI image proves that the MIPS loader starts at
  # firmware+0x3c: 16-byte IMG_HEADER_T plus 44 zero bytes.
  RTL8197F_HEADER_SIZE := 60
  RTL8197F_LEN_INCLUDES_HEADER_PAD := 1
  SUPPORTED_DEVICES += tenda,ac23 tenda,lynx-rtl8197f
  DEVICE_PACKAGES += ip-full ip-bridge ethtool dnsmasq odhcpd-ipv6only firewall4 kmod-rtl8192cd-rtl8197f-ac23 wireless-tools
  # v42 no longer assumes a fixed rootfs at 0x320000.  The combined cr6c image
  # uses IMG_HEADER_T.len and the Realtek/Tenda rootfs checksum trailer.
  IMAGES := sysupgrade.bin firmware-cr6c.bin $(TENDA_AC23_FULLFLASH_IMAGE)
  IMAGE/sysupgrade.bin := append-kernel | rtl8197f-image | append-rtl8197f-rootfs | rtl8197f-firmware-budget | pad-rootfs | rtl8197f-firmware-budget | append-metadata
  IMAGE/firmware-cr6c.bin := append-kernel | rtl8197f-image | append-rtl8197f-rootfs | rtl8197f-firmware-budget | pad-rootfs | rtl8197f-firmware-budget
  IMAGE/squashfs-spi-full.bin := append-kernel | rtl8197f-image | append-rtl8197f-rootfs | rtl8197f-firmware-budget | pad-rootfs | rtl8197f-firmware-budget | rtl8197f-tenda-lynx-fullflash
endef
TARGET_DEVICES += tenda_ac23


define Device/tenda_nova_mw5
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := Nova MW5
  DEVICE_DTS := rtl8197f_tenda_nova-mw5
  KERNEL := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    rtl8197f-loader-kernel
  KERNEL_INITRAMFS := \
    kernel-bin | \
    append-dtb | \
    lzma | \
    uImage lzma
  IMAGE_SIZE := 5696k
  # The verified GD25Q64-compatible dump reports 64 KiB erase sectors.  This
  # keeps pad-rootfs within the OEM 0x590000-byte KernelFS window instead of
  # rounding every image to a 256 KiB boundary.
  BLOCKSIZE := 64k
  RTL8197F_IMAGE_SIGNATURE := cr6c
  # Keep the self-extracting loader above the complete appended-DTB kernel.
  # A full v42.24 world build produced a 0x900d5b-byte kernel ending at
  # 0x80a00d5b and overwrote the old 0x80a00000 loader before flush_cache().
  # Keep the OEM header/staging address at 0x80a00000, but link the loader at
  # 0x80d00000.  Its startup relocation copies the full payload forward by
  # 3 MiB before decoding.  The distance is larger than the complete ~2.8 MiB
  # loader payload, so source and destination do not overlap.
  RTL8197F_LOADADDR := 0x80a00000
  RTL8197F_LZMA_TEXT_START := 0x80d00000
  RTL8197F_BURNADDR := 0x00030000
  RTL8197F_KERNEL_CMDLINE := console=ttyS0,115200n8 lpj=5000000 rootfstype=squashfs,jffs2 clk_ignore_unused
  RTL8197F_HEADER_SIZE := 16
  SUPPORTED_DEVICES += tenda,nova-mw5
  # Flash-safe 8 MiB profile.  Keep the functional IPv4 router path
  # (dnsmasq + firewall4), both WLAN radios and WEXT control, but remove tools
  # that are not required for boot/routing.  The rebuilt image must recover
  # at least 448 KiB; the strict budget gate measures the actual result.
  # Removed tools remain buildable/installable as separate IPKs.
  DEVICE_PACKAGES += \
    -ca-bundle \
    -ethtool \
    -ip-full \
    -ip-bridge \
    -libustream-mbedtls \
    -odhcp6c \
    -odhcpd-ipv6only \
    -procd-ujail \
    -uboot-envtools \
    -uclient-fetch \
    dnsmasq \
    firewall4 \
    kmod-rtl8192cd-rtl8197f-mw5 \
    wireless-tools
  # This PRIVATE tree automatically produces the personalized 8 MiB SPI image
  # from the bundled, board-matched MW5 template. Do not publish that output.
  IMAGES := sysupgrade.bin firmware-cr6c.bin $(TENDA_MW5_FULLFLASH_IMAGE)
  # Tenda's RTL8197F bootloader requires a board-specific SquashFS trailer:
  # preserve bytes_used, zero-pad after the filesystem to 4 KiB, then append
  # a BE16 additive checksum outside the SquashFS length.
  IMAGE/sysupgrade.bin := append-kernel | rtl8197f-image | append-rtl8197f-rootfs | rtl8197f-firmware-budget | pad-rootfs | rtl8197f-firmware-budget | append-metadata
  IMAGE/firmware-cr6c.bin := append-kernel | rtl8197f-image | append-rtl8197f-rootfs | rtl8197f-firmware-budget | pad-rootfs | rtl8197f-firmware-budget
  IMAGE/squashfs-spi-full.bin := append-kernel | rtl8197f-image | append-rtl8197f-rootfs | rtl8197f-firmware-budget | pad-rootfs | rtl8197f-firmware-budget | rtl8197f-tenda-mw5-fullflash
endef
TARGET_DEVICES += tenda_nova_mw5
