#
#	Makefile -- Build instructions for RTL8389
#

ROMFSIMG = $(IMAGEDIR)/romfs.img
EMPFSIMG = $(IMAGEDIR)/empfs.img
IMAGE    = $(IMAGEDIR)/image.bin
IMAGEZ   = $(IMAGEDIR)/imagez.bin
ELFIMAGE = $(IMAGEDIR)/image.elf


ROMFS_DIRS = bin dev etc home lib mnt proc usr var sys
EMPFS_DIRS = dev etc root
ROFS_DIRS = var/config var/tmp var/log var/run var/lock

ROMFS_DIRS += $(ROFS_DIRS)

MTDDEVICES = \
	mtdblock0,b,31,0	mtdblock1,b,31,1	mtdchar0,c,90,0	mtdchar1,c,90,1	\
	mtdblock2,b,31,2	mtdblock3,b,31,3	mtdchar2,c,90,2	mtdchar3,c,90,3	\
	mtdblock4,b,31,4	mtdblock5,b,31,5	mtdchar4,c,90,4	mtdchar5,c,90,5	\
	mtdblock6,b,31,6	mtdblock7,b,31,7	mtdchar6,c,90,6		mtdchar7,c,90,7	\
	mtdblock8,b,31,8	mtdblock9,b,31,9	mtdchar8,c,90,8		mtdchar9,c,90,9 \
	mtdblock10,b,31,10	mtdblock11,b,31,11	mtdchar10,c,90,10	mtdchar11,c,90,11

DEVICES = \
	tty,c,5,0	console,c,5,1	cua0,c,5,64	cua1,c,5,65 \
	\
	mem,c,1,1	kmem,c,1,2	null,c,1,3 \
	zero,c,1,5	random,c,1,8	urandom,c,1,9  \
	\
	ptmx,c,5,2 \
	\
	ptyp0,c,2,0	ptyp1,c,2,1	ptyp2,c,2,2	ptyp3,c,2,3 \
	ptyp4,c,2,4	ptyp5,c,2,5	ptyp6,c,2,6	ptyp7,c,2,7 \
	ptyp8,c,2,8	ptyp9,c,2,9	ptypa,c,2,10	ptypb,c,2,11 \
	ptypc,c,2,12	ptypd,c,2,13	ptype,c,2,14	ptypf,c,2,15 \
	\
	tty0,c,4,0	tty1,c,4,1	tty2,c,4,2	tty3,c,4,3 \
	ttyS0,c,4,64	ttyS1,c,4,65 \
	\
	ttyp0,c,3,0	ttyp1,c,3,1	ttyp2,c,3,2	ttyp3,c,3,3 \
	ttyp4,c,3,4	ttyp5,c,3,5	ttyp6,c,3,6	ttyp7,c,3,7 \
	ttyp8,c,3,8	ttyp9,c,3,9	ttypa,c,3,10	ttypb,c,3,11 \
	ttypc,c,3,12	ttypd,c,3,13	ttype,c,3,14	ttypf,c,3,15 \
	\
	rtcore,c,200,0 \
	ski,c,201,0 \
	\
	$(MTDDEVICES)

clean:

romfs:
	chmod 755 $(ROOTDIR)/tools/romfs-inst.sh
	[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)
	for i in $(ROMFS_DIRS); do \
		[ -d $(ROMFSDIR)/$$i ] || mkdir -p $(ROMFSDIR)/$$i; \
	done
	@if egrep "^CONFIG_INITRAMFS_SOURCE=\"..\/empfs\"" $(LINUX_CONFIG) > /dev/null; then \
		for i in $(EMPFS_DIRS); do \
			[ -d $(EMPFSDIR)/$$i ] || mkdir -p $(EMPFSDIR)/$$i; \
		done; \
		rm -rf $(EMPFSDIR)/dev/*; \
		for i in $(MTDDEVICES); do \
			mknod $(EMPFSDIR)/dev/`echo $$i|sed 's/,/ /g'`; \
		done; \
	fi
	#User may not do 'make mrproper' to clean romfs/*, clean again here to avoid file exist error.
	rm -rf $(ROMFSDIR)/dev/*
	for i in $(DEVICES); do \
		mknod $(ROMFSDIR)/dev/`echo $$i|sed 's/,/ /g'`;\
	done
	$(ROMFSINST) -s var/tmp /tmp
	$(ROMFSINST) -s bin /sbin
	$(ROMFSINST) -e CONFIG_USER_BUSYBOX_INIT -s sbin/init /init
	$(ROMFSINST) -p 755 /etc/rc
	$(ROMFSINST) -p 755 /etc/inittab
	$(ROMFSINST) /etc/profile
	$(ROMFSINST) /etc/services	
	$(ROMFSINST) /etc/motd
	$(ROMFSINST) /etc/passwd
	echo "$(VERSIONSTR) -- " `date` > $(ROMFSDIR)/etc/version
	sed -n '/branches\/sdk\/sdk-integrate-8390-8380$$/s/\/svn\/CN\/Switch\/\!svn\/ver\///p' $(ROOTDIR)/../../.svn/all-wcprops | sed -n 's/\/branches\/sdk\/sdk-integrate-8390-8380//p' > $(ROMFSDIR)/etc/revision
	
image:
	[ -d $(IMAGEDIR) ] || mkdir -p $(IMAGEDIR)
	genromfs -v -V "ROMdisk" -f $(ROMFSIMG) -d $(ROMFSDIR)
	$(CROSS_COMPILE)ld -r -o $(ROOTDIR)/$(LINUXDIR)/romfs.o -b binary $(ROMFSIMG) 
	$(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S $(ROOTDIR)/$(LINUXDIR)/linux $(IMAGEDIR)/image.ram
	cat $(IMAGEDIR)/image.ram $(ROMFSIMG) > $(IMAGE)	
	$(ROOTDIR)/tools/cksum -b -o 2 $(IMAGE) >> $(IMAGE)
	BSS=`$(CROSS_COMPILE)objdump --headers $(ROOTDIR)/$(LINUXDIR)/linux | \
	  grep .bss` ; \
	ADDR=`set -- $${BSS} ; echo 0x$${4}` ; \
	$(CROSS_COMPILE)objcopy --add-section=.romfs=$(ROMFSIMG) \
	  --adjust-section-vma=.romfs=$${ADDR} --no-adjust-warnings \
	  --set-section-flags=.romfs=alloc,load,data   \
	  $(ROOTDIR)/$(LINUXDIR)/linux $(ELFIMAGE) 2> /dev/null
     
