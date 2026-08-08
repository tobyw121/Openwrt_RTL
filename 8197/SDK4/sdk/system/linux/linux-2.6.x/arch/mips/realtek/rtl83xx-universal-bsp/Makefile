#
# Makefile for the RTL8390 specific parts of the kernel
#

EXTRA_CFLAGS += -I$(SDK_INCLUDE) -I$(SYS_INCLUDE) -DRTK_UNIVERSAL_BSP -Werror

obj-y	 	:= setup.o printf.o int.o mem.o chip_probe.o

EXTRA_AFLAGS := $(CFLAGS)

