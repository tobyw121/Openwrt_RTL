#
# Makefile for the Linux/MIPS-specific parts of the memory manager.
#

obj-y				+= cache.o dma-default.o extable.o fault.o \
				   init.o tlbex.o tlbex-fault.o uasm.o page.o

obj-$(CONFIG_32BIT)		+= ioremap.o pgtable-32.o
obj-$(CONFIG_HIGHMEM)		+= highmem.o
obj-$(CONFIG_HUGETLB_PAGE)	+= hugetlbpage.o

obj-y				+= cache-rlx.o tlb-rlx.o

EXTRA_CFLAGS += -Werror
