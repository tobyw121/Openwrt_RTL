# Realtek Semiconductor Corp.
#
# bsp/Makefile
#     bsp Makefile
#
# Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
#

CPPFLAGS_vmlinux.lds := $(KBUILD_CFLAGS)

#
# Sheipa
#
core-y   += arch/mips/bsp/
cflags-y += -Iinclude/asm-mips -Iarch/mips/bsp/ -DRTK_UNIVERSAL_BSP
load-y   += 0x80000000

extra-y  += vmlinux.lds

obj-y := prom.o setup.o irq.o timer.o serial.o chip_probe.o
obj-$(CONFIG_PCI) += pci-fixup.o pci-ops.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_USB) += usb.o
obj-$(CONFIG_MIPS_MT_SMTC) += smtc.o
obj-$(CONFIG_MIPS_MT_SMP) += vsmp.o
