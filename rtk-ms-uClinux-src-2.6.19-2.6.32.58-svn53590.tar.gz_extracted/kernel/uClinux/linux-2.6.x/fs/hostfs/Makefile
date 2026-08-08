#
# Copyright (C) 2000 Jeff Dike (jdike@karaya.com)
# Licensed under the GPL
#

hostfs-objs := hostfs_kern.o hostfs_user.o

obj-y :=
obj-$(CONFIG_HOSTFS) += hostfs.o

ifeq (arch/um/scripts/Makefile.rules,$(wildcard arch/um/scripts/Makefile.rules))
include arch/um/scripts/Makefile.rules
endif

