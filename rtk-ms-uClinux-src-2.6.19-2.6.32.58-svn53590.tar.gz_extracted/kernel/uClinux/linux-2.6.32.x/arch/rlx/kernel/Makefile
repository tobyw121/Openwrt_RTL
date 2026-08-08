#
# Makefile for the Linux/MIPS kernel.
#

extra-y		:= head.o init_task.o

obj-y		+= cpu-probe.o branch.o entry.o genex.o irq.o process.o \
		   ptrace.o reset.o setup.o signal.o syscall.o \
		   time.o topology.o traps.o unaligned.o watch.o vdso.o

obj-$(CONFIG_CEVT_EXT)		+= cevt-ext.o
obj-$(CONFIG_CEVT_RLX)		+= cevt-rlx.o csrc-rlx.o

obj-$(CONFIG_STACKTRACE)	+= stacktrace.o
obj-$(CONFIG_MODULES)		+= mips_ksyms.o module.o

obj-y				+= rlx-switch.o

obj-$(CONFIG_IRQ_CPU)		+= irq-cpu.o
obj-$(CONFIG_IRQ_VEC)		+= irq-vec.o

obj-$(CONFIG_32BIT)		+= scall32-o32.o

obj-$(CONFIG_KGDB)		+= kgdb.o
obj-$(CONFIG_PROC_FS)		+= proc.o

obj-$(CONFIG_KEXEC)		+= machine_kexec.o relocate_kernel.o
obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o

EXTRA_CFLAGS += -Werror
