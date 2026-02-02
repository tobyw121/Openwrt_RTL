#
# Copyright (c) 2013 The Linux Foundation. All rights reserved.
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ncurses
ifeq ($(CONFIG_PACKAGE_NCURSE_VERSION),"6.2")
PKG_VERSION:=6.2
PKG_RELEASE:=1
PATCH_DIR?=./patches6.2
else
PKG_VERSION:=5.7
PKG_RELEASE:=5
PATCH_DIR?=./patches5.7
PKG_BUILD_DIR:=$(BUILD_DIR)/$(BUILD_VARIANT)/$(PKG_NAME)-$(PKG_VERSION)
endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)

ifeq ($(PKG_VERSION),6.2)
PKG_HASH:=30306e0c76e0f9f1f0de987cf1c82a5c21e1ce6568b9227f7da5b71cbea86c9d
else
PKG_MD5SUM:=cce05daf61a64501ef6cd8da1f727ec6
endif

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=README
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_BUILD_DEPENDS:=ncurses/host

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/host-build.mk

define Package/libncurses/config
	config PACKAGE_NCURSE_VERSION
	string "ncurse version"
	depends PACKAGE_libncurses
	default "6.2"
	
endef

define Package/terminfo
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Terminal Info Database (ncurses)
  URL:=http://www.gnu.org/software/ncurses/
ifeq ($(PKG_VERSION),5.7)
  VARIANT:=libncurses
endif
endef

define Package/libncurses
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Terminal handling library (Unicode)
  URL:=http://www.gnu.org/software/ncurses/
ifeq ($(PKG_VERSION),6.2)
  PROVIDES:=libncursesw
  DEPENDS:= +terminfo
  ABI_VERSION:=6
else
  DEPENDS:= +terminfo
  VARIANT:=libncurses
endif
endef

ifeq ($(PKG_VERSION),6.2)
define Package/libncurses-dev
  SECTION:=devel
  CATEGORY:=Development
  SUBMENU:=Libraries
  DEPENDS:=zlib
  TITLE:=Development files for the ncurses library
endef
endif

ifeq ($(PKG_VERSION),5.7)
define Package/libncursesw
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Terminal handling library (Unicode)
  URL:=http://www.gnu.org/software/ncurses/
  VARIANT:=libncursesw
endef
endif


TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--enable-echo \
	--enable-const \
	--enable-overwrite \
	--disable-rpath \
	--without-ada \
	--without-debug \
	--without-manpages \
	--without-profile \
	--without-progs \
	--without-tests \
	--disable-big-core \
	--disable-home-terminfo \
	--with-normal \
	--with-shared \
	--with-terminfo-dirs=/usr/share/terminfo \
	--with-default-terminfo-dir=/usr/share/terminfo

ifeq ($(PKG_VERSION),6.2)
CONFIGURE_ARGS += \
	     --enable-pc-files \
         --with-pkg-config-libdir=/usr/lib/pkgconfig \
         --enable-widec \
         --with-build-cppflags=-D_GNU_SOURCE

CONFIGURE_VARS += \
	cf_try_fPIC=no
endif

HOST_CFLAGS += $(HOST_FPIC)

HOST_CONFIGURE_ARGS += \
	--without-cxx \
	--without-cxx-binding \
	--without-ada \
	--without-debug \
	--without-manpages \
	--without-profile \
	--without-tests \
	--without-curses-h


ifeq ($(PKG_VERSION),5.7)
ifneq ($(CONFIG_INSTALL_LIBSTDCPP),y)
CONFIGURE_ARGS += \
	--without-cxx-binding
endif
endif

ifeq ($(HOST_OS),FreeBSD)
	CONFIGURE_ARGS +=
		--with-terminfo=/usr/share/terminfo.db 
endif

ifeq ($(PKG_VERSION),5.7)
ifeq ($(BUILD_VARIANT),libncursesw)
	CONFIGURE_ARGS += \
		--enable-widec \
		--with-build-cppflags=-D_GNU_SOURCE
endif
endif

MAKE_FLAGS += \
	BUILD_CC="$(HOSTCC)" \
	HOSTCC="$(HOSTCC)" \
	HOSTCCFLAGS="" \
	PKG_CONFIG_LIBDIR=/usr/lib/pkgconfig \
	libs

define Build/Install/Default
	$(MAKE_VARS) \
	$(MAKE) -C $(PKG_BUILD_DIR)/$(MAKE_PATH) \
		$(MAKE_INSTALL_FLAGS) \
		$(1) install.libs install.data;
endef

define Package/terminfo/install
	echo ""
ifneq ($(HOST_OS),FreeBSD)
	$(INSTALL_DIR) $(1)/usr/share/terminfo
	(cd $(PKG_INSTALL_DIR)/usr/share/terminfo; \
		for dir in ??; do \
			[ -d "$$$$dir" ] || continue; \
			mv $$$$dir $$$$(echo -ne "\x$$$$dir"); \
		done \
	)
	for file in a/ansi d/dumb l/linux r/rxvt r/rxvt-unicode s/screen v/vt100 v/vt102 x/xterm x/xterm-color x/xterm-256color; do \
		$(INSTALL_DIR) $(1)/usr/share/terminfo/`dirname $$$$file`; \
		$(CP) $(PKG_INSTALL_DIR)/usr/share/terminfo/$$$$file \
			$(1)/usr/share/terminfo/$$$$file; \
	done
endif
endef


define Package/libncurses/install
	$(INSTALL_DIR) $(1)/usr/lib
ifeq ($(PKG_VERSION),6.2)
	for lib in ncurses panel menu form; do \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib$$$${lib}w.so.* $(1)/usr/lib/; \
		for so in $(1)/usr/lib/lib$$$${lib}w.so.*; do \
			ln -s $$$${so##*/} $$$${so%w.so*}.so$$$${so##*w.so}; \
		done; \
	done
else
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{curses,ncurses,panel,menu,form}.so* $(1)/usr/lib/
endif
endef

ifeq ($(PKG_VERSION),6.2)
define Package/libncurses-dev/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
endef
endif

ifeq ($(PKG_VERSION),5.7)
define Package/libncursesw/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{ncurses,panel,menu,form}w.so* $(1)/usr/lib/
endef
endif

ifeq ($(PKG_VERSION),6.2)
define Build/InstallDev
	$(CP) $(PKG_INSTALL_DIR)/* $(1)
	for lib in ncurses panel menu form; do \
		for so in $(1)/usr/lib/lib$$$${lib}w.so*; do \
			ln -s $$$${so##*/} $$$${so%w.so*}.so$$$${so##*w.so}; \
		done; \
	done
	ln -s . $(1)/usr/include/ncursesw
	$(TARGET_CROSS)ar rc $(1)/usr/lib/libtinfo.a
	$(INSTALL_DIR) $(2)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ncursesw6-config $(2)/bin/
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' -e 's/$$$$INCS //g' \
		$(2)/bin/ncursesw6-config
	ln -sf $(STAGING_DIR)/host/bin/ncursesw6-config $(1)/usr/bin/ncursesw6-config
endef
endif

ifeq ($(PKG_VERSION),5.7)
ifeq ($(BUILD_VARIANT),libncursesw)
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/ncursesw/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/ncursesw/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{ncurses,panel,menu,form}w.{a,so*} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/bin $(2)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ncursesw5-config $(2)/bin/
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' -e 's/$$$$INCS //g' \
		$(2)/bin/ncursesw5-config
	ln -sf $(STAGING_DIR)/host/bin/ncursesw5-config $(1)/usr/bin/ncursesw5-config
endef
else
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{curses,ncurses,panel,menu,form}.{a,so*} $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/bin $(2)/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/ncurses5-config $(2)/bin/
	$(SED) 's,^\(prefix\|exec_prefix\)=.*,\1=$(STAGING_DIR)/usr,g' \
		$(2)/bin/ncurses5-config
	ln -sf $(STAGING_DIR)/host/bin/ncurses5-config $(1)/usr/bin/ncurses5-config
endef
endif
endif

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) libs
	$(MAKE) -C $(HOST_BUILD_DIR)/progs tic
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,terminfo))
$(eval $(call BuildPackage,libncurses))
ifeq ($(PKG_VERSION),6.2)
$(eval $(call BuildPackage,libncurses-dev))
else
$(eval $(call BuildPackage,libncursesw))
endif

