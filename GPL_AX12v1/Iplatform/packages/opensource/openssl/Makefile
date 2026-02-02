#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openssl

ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2d")
PKG_VERSION:=1.0.2d
else ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2u")
PKG_VERSION:=1.0.2u
else
PKG_VERSION:=1.1.1t
endif

PKG_RELEASE:=1

PKG_LICENSE:=SSLEAY OPENSSL
PKG_LICENSE_FILES:=LICENSE
PKG_BUILD_DEPENDS:=ocf-crypto-headers
PKG_CONFIG_DEPENDS:=CONFIG_OPENSSL_ENGINE_CRYPTO CONFIG_OPENSSL_ENGINE_DIGEST

include $(INCLUDE_DIR)/package.mk

define Package/openssl/Default
  TITLE:=Open source SSL toolkit
  URL:=http://www.openssl.org/
endef

define Package/openssl
  TITLE:=Open source SSL toolkit
  URL:=http://www.openssl.org/
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=SSL
endef

define Package/libopenssl/config
source "$(SOURCE)/Config.in"
endef

define Package/openssl/Default/description
The OpenSSL Project is a collaborative effort to develop a robust,
commercial-grade, full-featured, and Open Source toolkit implementing the Secure
Sockets Layer (SSL v2/v3) and Transport Layer Security (TLS v1) protocols as well
as a full-strength general purpose cryptography library.
endef

define Package/openssl/config
	config PACKAGE_OPENSSL_VERSION
		string "openssl compile version"
		depends PACKAGE_openssl
		default "1.0.2d"
		help
			current support valid value "1.0.2d" and "1.0.2u" and "1.1.1t"

	config PACKAGE_OPENSSL_SUPPORT_BRCM_NEON
		bool "openssl patch to support brcm armv7 NEON feature"
		depends PACKAGE_openssl
		default n
		help
			BRCM openssl patch which using runner to  improve performance.

	config PACKAGE_OPENSSL_SUPPORT_AARCH64_NEON
		bool "openssl compile target for aarch64 to support mtk armv8 NEON feature( ex:mt7986 )"
		depends PACKAGE_openssl
		default n
		help
			openssl patch which using NEON to improve openvpn performance.
endef


define Package/libopenssl
$(call Package/openssl/Default)
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=SSL
  DEPENDS:=+zlib
  TITLE+= (libraries)
  MENU:=1
endef

define Package/libopenssl/description
$(call Package/openssl/Default/description)
This package contains the OpenSSL shared libraries, needed by other programs.
endef

define Package/openssl-util
  $(call Package/openssl/Default)
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=SSL
  DEPENDS:=+libopenssl
  TITLE+= (utility)
endef

define Package/openssl-util/conffiles
/etc/ssl/openssl.cnf
endef

define Package/openssl-util/description
$(call Package/openssl/Default/description)
This package contains the OpenSSL command-line utility.
endef

ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2d")
OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 no-sha0 no-smime \
	no-rmd160 no-aes192 no-ripemd no-camellia no-ans1 no-krb5
else ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2u")
OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 \
	no-rmd160 no-camellia
else
OPENSSL_NO_CIPHERS:= no-idea no-md2 no-mdc2 no-rc5 \
	no-rmd160 no-camellia no-sm2 no-sm3 no-sm4 \
	no-aria no-gost no-blake2
endif
OPENSSL_OPTIONS:= shared no-err no-hw zlib-dynamic no-sse2
OPENSSL_DOPTION:=-DOPENSSL_SMALL_FOOTPRINT

ifdef CONFIG_OPENSSL_ENGINE_CRYPTO
  OPENSSL_OPTIONS += -DHAVE_CRYPTODEV
  ifdef CONFIG_OPENSSL_ENGINE_DIGEST
    OPENSSL_OPTIONS += -DUSE_CRYPTODEV_DIGESTS
  endif
else
  ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2d")
  OPENSSL_OPTIONS += no-engines
  endif
endif

ifeq ($(CONFIG_x86_64),y)
  OPENSSL_TARGET:=linux-x86_64
else
  OPENSSL_OPTIONS+=no-sse2
  ifeq ($(CONFIG_mips)$(CONFIG_mipsel),y)
    OPENSSL_TARGET:=linux-mips-openwrt
  else
    ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2d")
    OPENSSL_TARGET:=linux-generic-openwrt
    OPENSSL_OPTIONS+=no-perlasm
    else ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2u")
    OPENSSL_TARGET:=linux-generic-openwrt
    OPENSSL_OPTIONS+=no-tls1
	else
	OPENSSL_TARGET:=linux-armv4
    OPENSSL_OPTIONS+=no-tls1 no-tests
    endif
  endif
endif

ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2d")
PATCH_DIR:=
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	if [ "$(CONFIG_PACKAGE_OPENSSL_SUPPORT_BRCM_NEON)" == "y" ]; then \
	   patch -p1 -d $(PKG_BUILD_DIR) < ./src/brcm_spu_support_1.0.2d.patch; \
	fi
endef
else ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2u")
PATCH_DIR:=
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src_102u/* $(PKG_BUILD_DIR)
	if [ "$(CONFIG_PACKAGE_OPENSSL_SUPPORT_BRCM_NEON)" == "y" ]; then \
	    patch -p1 -d $(PKG_BUILD_DIR) < ./src_102u/brcm_neon_support_1.0.2u.patch; \
	fi
endef
else
PATCH_DIR:=
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src_111t/* $(PKG_BUILD_DIR)
endef
endif

OPENSSL_TARGET_FLAGS:="$(TARGET_LDFLAGS) -ldl -DDOPENSSL_SMALL_FOOTPRINT"

ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2d")
ifdef CONFIG_PACKAGE_OPENSSL_SUPPORT_BRCM_NEON
	IPF_OPENSSL_NO_CIPHERS := no-idea no-md2 no-mdc2 no-rc5 no-sha0 no-smime no-rmd160 no-aes192 no-ripemd no-camellia no-ans1 no-krb5
	IPF_OPENSSL_OPTIONS := shared no-err no-hw zlib-dynamic no-sse2 no-engines no-sse2 no-perlasm

	BCM_OPENSSL_TARGET := linux-bcmarm
	BCM_OPENSSL_OPTIONS := no-hw shared no-md2 no-md4 no-mdc2 no-ripemd no-rc2 no-krb5 \
						no-idea no-unit-test no-cast no-bf no-rc5 no-err no-ssl2 
	BCM_OPENSSL_ARCH_OPTS := -DOPENSSL_NO_HEARTBEATS -DL_ENDIAN no-engine -Os \
						-march=armv7-a -fomit-frame-pointer -mno-thumb-interwork -mabi=aapcs-linux \
						-marm -fno-common -ffixed-r8 -msoft-float -D__ARM_ARCH_7A__ \
						-Werror=return-type -Werror=uninitialized -Wno-date-time
	OPENSSL_TARGET:=$(BCM_OPENSSL_TARGET)
	OPENSSL_NO_CIPHERS:=$(IPF_OPENSSL_NO_CIPHERS)
	OPENSSL_OPTIONS:= $(IPF_OPENSSL_OPTIONS) $(BCM_OPENSSL_ARCH_OPTS)
	OPENSSL_TARGET_FLAGS:="$(TARGET_LDFLAGS) -ldl"
endif
endif

ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.0.2u")
ifdef CONFIG_PACKAGE_OPENSSL_SUPPORT_BRCM_NEON
	BCM_OPENSSL_TARGET := linux-bcmarm
	OPENSSL_TARGET:=$(BCM_OPENSSL_TARGET)
endif
ifdef CONFIG_PACKAGE_OPENSSL_SUPPORT_AARCH64_NEON
	OPENSSL_TARGET:= linux-aarch64
endif
endif

define Build/Configure
	(cd $(PKG_BUILD_DIR); \
		./Configure $(OPENSSL_TARGET) \
			--prefix=/usr \
			--openssldir=/etc/ssl \
			$(TARGET_CPPFLAGS) \
			$(OPENSSL_TARGET_FLAGS) \
			$(OPENSSL_NO_CIPHERS) \
			$(OPENSSL_OPTIONS) \
		&& sed -i 's/^CFLAG=/CFLAG=-Os -DGNU/g' Makefile \
		&& sed -i '/^CFLAG=/  s/-O3//g' Makefile \
	)
endef

TARGET_CFLAGS += $(FPIC)

ifeq ($(CONFIG_LIBC_USE_MUSL),y)
	TARGET_CFLAGS += -DCONFIG_LIBC_USE_MUSL=y
endif

ifeq ($(CONFIG_PACKAGE_OPENSSL_VERSION),"1.1.1t")
define Build/Compile
	# XXX: OpenSSL "make depend" will look for installed headers before its own,
	# so remove installed stuff first
	-$(SUBMAKE) -j1 clean-staging
	
	$(_SINGLE)$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		OPENWRT_OPTIMIZATION_FLAGS="$(TARGET_CFLAGS)" \
		$(OPENSSL_MAKEFLAGS) \
		all
	
	# Work around openssl_111t build bug to link libssl.so with libcrypto.so.
	-rm $(PKG_BUILD_DIR)/libssl.so.*.*.*
	
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		$(OPENSSL_MAKEFLAGS) \
		install
endef
else
define Build/Compile
	# XXX: OpenSSL "make depend" will look for installed headers before its own,
	# so remove installed stuff first
	-$(SUBMAKE) -j1 clean-staging
	$(MAKE) -C $(PKG_BUILD_DIR) \
		MAKEDEPPROG="$(TARGET_CROSS)gcc" \
		OPENWRT_OPTIMIZATION_FLAGS="$(TARGET_CFLAGS)" \
		$(OPENSSL_MAKEFLAGS) \
		depend
	$(_SINGLE)$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar r" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		OPENWRT_OPTIMIZATION_FLAGS="$(TARGET_CFLAGS)" \
		$(OPENSSL_MAKEFLAGS) \
		all
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_CROSS)ar r" \
		RANLIB="$(TARGET_CROSS)ranlib" \
		OPENWRT_OPTIMIZATION_FLAGS="$(TARGET_CFLAGS)" \
		$(OPENSSL_MAKEFLAGS) \
		build-shared
	# Work around openssl build bug to link libssl.so with libcrypto.so.
	-rm $(PKG_BUILD_DIR)/libssl.so.*.*.*
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		OPENWRT_OPTIMIZATION_FLAGS="$(TARGET_CFLAGS)" \
		$(OPENSSL_MAKEFLAGS) \
		do_linux-shared
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		$(OPENSSL_MAKEFLAGS) \
		install
endef
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/openssl $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib{crypto,ssl}.{a,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/{openssl,libcrypto,libssl}.pc $(1)/usr/lib/pkgconfig/
	[ -n "$(TARGET_LDFLAGS)" ] && $(SED) 's#$(TARGET_LDFLAGS)##g' $(1)/usr/lib/pkgconfig/{openssl,libcrypto,libssl}.pc || true
endef

define Package/libopenssl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libcrypto.so.* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libssl.so.* $(1)/usr/lib/
endef

define Package/openssl-util/install
	$(INSTALL_DIR) $(1)/etc/ssl
	$(CP) $(PKG_INSTALL_DIR)/etc/ssl/openssl.cnf $(1)/etc/ssl/
	$(INSTALL_DIR) $(1)/etc/ssl/certs
	$(INSTALL_DIR) $(1)/etc/ssl/private
	chmod 0700 $(1)/etc/ssl/private
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/openssl $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libopenssl))
$(eval $(call BuildPackage,openssl-util))
$(eval $(call BuildPackage,openssl))
