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
  DEVICE_PACKAGES += ip-full ip-bridge ethtool bridge dnsmasq odhcpd-ipv6only firewall4 luci uhttpd uhttpd-mod-ubus rpcd rtl8367d-compat
  # Keep first-boot images minimal; RTL8197F WLAN/Ethernet need separate drivers.
  # kernel.bin is the raw self-extracting loader payload.
  # kernel-cs6c.bin adds the Realtek/Xiaomi bootmiwifi-style header for
  # flash kernel-partition experiments; do not flash without serial recovery.
  IMAGES += kernel.bin kernel-cs6c.bin rootfs.bin fullflash-test.bin
  IMAGE/kernel.bin := append-kernel | pad-to 64k | check-size $$$$(KERNEL_SIZE)
  IMAGE/kernel-cs6c.bin := append-kernel | rtl8197f-image | pad-to 64k | check-size $$$$(KERNEL_SIZE)
  IMAGE/rootfs.bin := append-rootfs | pad-rootfs | check-size $$$$(ROOTFS_SIZE)
  IMAGE/fullflash-test.bin := append-kernel | rtl8197f-image | pad-to 64k | check-size $$$$(KERNEL_SIZE) | rtl8197f-rd05-fullflash
endef
TARGET_DEVICES += xiaomi_r4_rd05


define Device/tenda_lynx_8197f
  DEVICE_VENDOR := Tenda
  DEVICE_MODEL := Lynx RTL8197F-VG
  DEVICE_DTS := rtl8197f_tenda_lynx
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
  IMAGE_SIZE := 7872k
  KERNEL_SIZE := 3080192
  ROOTFS_SIZE := 4980736
  RTL8197F_IMAGE_SIGNATURE := cr6c
  RTL8197F_LOADADDR := 0x80a00000
  RTL8197F_LZMA_TEXT_START := 0x80a00000
  RTL8197F_BURNADDR := 0x00030000
  RTL8197F_KERNEL_CMDLINE := console=ttyS0,115200n8 lpj=5000000 rootfstype=squashfs,jffs2 clk_ignore_unused
  RTL8197F_HEADER_SIZE := 64
  RTL8197F_KERNEL_OFFSET := 0x00030000
  RTL8197F_ROOTFS_OFFSET := 0x00320000
  RTL8197F_ROOTFS_DATA_OFFSET := 0x007e0000
  SUPPORTED_DEVICES += tenda,lynx-rtl8197f
  DEVICE_PACKAGES += ip-full ip-bridge ethtool bridge dnsmasq odhcpd-ipv6only firewall4
  IMAGES += kernel.bin kernel-cr6c.bin rootfs.bin squashfs-fullflash-test.bin
  IMAGE/kernel.bin := append-kernel | pad-to 64k | check-size $$$$(KERNEL_SIZE)
  IMAGE/kernel-cr6c.bin := append-kernel | rtl8197f-image | pad-to 64k | check-size $$$$(KERNEL_SIZE)
  IMAGE/rootfs.bin := append-rootfs | pad-rootfs | check-size $$$$(ROOTFS_SIZE)
  IMAGE/squashfs-fullflash-test.bin := append-kernel | rtl8197f-image | pad-to 64k | check-size $$$$(KERNEL_SIZE) | rtl8197f-tenda-lynx-fullflash
endef
TARGET_DEVICES += tenda_lynx_8197f
