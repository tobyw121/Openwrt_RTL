#
# Makefile for the Linux/MIPS kernel.
#

extra-y		:= head.o init_task.o

obj-y		+= cpu-probe.o branch.o entry.o genex.o irq.o process.o \
		   ptrace.o reset.o setup.o signal.o syscall.o \
		   time.o topology.o traps.o unaligned.o watch.o vdso.o

obj-$(CONFIG_CEVT_R4K_LIB)	+= cevt-r4k.o
obj-$(CONFIG_CEVT_EXT)		+= cevt-ext.o
obj-$(CONFIG_MIPS_MT_SMTC)	+= cevt-smtc.o
obj-$(CONFIG_CSRC_R4K_LIB)	+= csrc-r4k.o
obj-$(CONFIG_SYNC_R4K)		+= sync-r4k.o

obj-$(CONFIG_STACKTRACE)	+= stacktrace.o
obj-$(CONFIG_MODULES)		+= mips_ksyms.o module.o

obj-$(CONFIG_CPU_MIPS32)	+= r4k_fpu.o r4k_switch.o

obj-$(CONFIG_SMP)		+= smp.o
obj-$(CONFIG_SMP_UP)		+= smp-up.o

obj-$(CONFIG_MIPS_MT)		+= mips-mt.o
obj-$(CONFIG_MIPS_MT_FPAFF)	+= mips-mt-fpaff.o
obj-$(CONFIG_MIPS_MT_SMTC)	+= smtc.o smtc-asm.o smtc-proc.o
obj-$(CONFIG_MIPS_MT_SMP)	+= smp-mt.o
obj-$(CONFIG_MIPS_CMP)		+= smp-cmp.o
obj-$(CONFIG_CPU_MIPSR2)	+= spram.o

obj-$(CONFIG_MIPS_VPE_LOADER)	+= vpe.o
obj-$(CONFIG_MIPS_VPE_APSP_API)	+= rtlx.o
obj-$(CONFIG_MIPS_APSP_KSPD)	+= kspd.o

obj-$(CONFIG_IRQ_CPU)		+= irq_cpu.o
obj-$(CONFIG_IRQ_GIC)		+= irq-gic.o

obj-$(CONFIG_32BIT)		+= scall32-o32.o

obj-$(CONFIG_KGDB)		+= kgdb.o
obj-$(CONFIG_PROC_FS)		+= proc.o

obj-$(CONFIG_KEXEC)		+= machine_kexec.o relocate_kernel.o
obj-$(CONFIG_EARLY_PRINTK)	+= early_printk.o

EXTRA_CFLAGS += -Werror
