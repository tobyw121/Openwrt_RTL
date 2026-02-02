#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wrapper
PKG_VERSION:=1.0.0

PKG_SOURCE:=
PKG_SOURCE_URL:=
PKG_HASH:=

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk

TOOLS_BIN_LIST:= aclocal \
aclocal-1.10 \
aclocal-1.11 \
aclocal-1.11.6 \
aclocal-1.15 \
aclocal-1.9 \
aclocal.real \
add_header \
addpattern \
ar-tool \
asustrx \
autoconf \
autoheader \
autom4te \
automake \
automake-1.15 \
autoreconf \
autoscan \
autoupdate \
badblocks \
bc \
bcm4908asus \
bcm4908kernel \
bison \
blkdeactivate \
blkid \
buffalo-enc \
buffalo-tag \
buffalo-tftp \
ccmake \
chattr \
cmake \
compile_et \
cpack \
ctest \
dc \
debugfs \
dgfirmware \
dgn3500sum \
dmsetup \
dmstats \
dns313-header \
dumpe2fs \
e2freefrag \
e2fsck \
e2image \
e2label \
e2mmpstatus \
e2scrub \
e2scrub_all \
e2undo \
e4crypt \
edimax_fw_header \
encode_crc \
faked \
fakeroot \
fatlabel \
filefrag \
findfs \
fix-u-media-header \
flex \
flex++ \
flock \
fsck \
fsck.ext2 \
fsck.ext3 \
fsck.ext4 \
fsck.fat \
fwtool \
gengetopt \
guards \
hcsmakeimage \
help2man \
ifnames \
imagetag \
jcgimage \
jshn \
lex \
libtool \
libtoolize \
libtool-ucxx \
locate \
logsave \
lsattr \
lxlfw \
lzcmp \
lzdiff \
lzegrep \
lzfgrep \
lzgrep \
lzless \
lzma \
lzma2eva \
lzmadec \
lzmainfo \
lzmore \
m4 \
makeamitbin \
make_ext4fs \
makeinfo \
mcopy \
mkbrncmdline \
mkbrnimg \
mkbuffaloimg \
mkcameofw \
mkcasfw \
mkchkimg \
mk_cmds \
mkcsysimg \
mkdapimg \
mkdapimg2 \
mkdhpimg \
mkdlinkfw \
mkdniimg \
mke2fs \
mkedimaximg \
mkenvimage \
mkfs.ext2 \
mkfs.ext3 \
mkfs.ext4 \
mkfs.fat \
mkfs.jffs2 \
mkfs.ubifs \
mkfwimage \
mkfwimage2 \
mkheader_gemtek \
mkhilinkfw \
mkimage \
mklibs \
mklibs-copy \
mklibs-readelf \
mklost+found \
mkmerakifw \
mkmerakifw-old \
mkmylofw \
mkplanexfw \
mkporayfw \
mkrasimage \
mkrtn56uimg \
mksenaofw \
mksercommfw \
mksquashfs4 \
mktitanimg \
mktplinkfw \
mktplinkfw2 \
mkwrggimg \
mkwrgimg \
mkzcfw \
mkzynfw \
mmd \
motorola-bin \
nand_ecc \
nec-enc \
ocspcheck \
openssl \
opkg \
osbridge-crc \
oseama \
otrx \
padjffs2 \
patch-cmdline \
patch-dtb \
patchelf \
pc1crypt \
pkg-config \
pkg-config.real \
ptgen \
quilt \
resize2fs \
seama \
sed \
sign_dlink_ru \
spw303v \
srec2bin \
sstrip \
tplink-safeloader \
trx \
trx2edips \
trx2usr \
tune2fs \
ubinize \
ucert \
uimage_padhdr \
unsquashfs4 \
unxz \
unzstd \
updatedb \
usign \
uuidd \
uuidgen \
veritysetup \
wrt400n \
xmlwf \
xorimage \
xz \
xzcat \
xzcmp \
xzdec \
xzdiff \
xzegrep \
xzfgrep \
xzgrep \
xzless \
xzmore \
yacc \
zip \
zstd \
zstdcat \
zstdgrep \
zstdless \
zstdmt \
zyimage \
zyxbcm

# 1: config symbol
# 2: feature
define toolchain_test
	@echo -n "Testing external tool $(STAGING_DIR_HOST)/bin ... "
	for bin in $(1); do \
		if [ -f $(STAGING_DIR_HOST)/bin/$$$${bin} -o -L $(STAGING_DIR_HOST)/bin/$$$${bin} ]; then \
			echo "ok"; exit 0; \
		else \
			echo "failed"; \
			echo "ERROR: $$$${bin} is enabled but the external toolchain does not support it"; \
			exit 1; \
		fi \
	done
endef


define Host/Prepare
endef

define Host/Configure
endef

define Host/Compile
endef

define Host/Install
	$(call toolchain_test,$(TOOLS_BIN_LIST))
endef

define Host/Clean
	rm -rf $(TOOLCHAIN_DIR)/bin
endef

$(eval $(call HostBuild))
