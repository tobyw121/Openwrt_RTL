#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/package.mk
ifeq ($(CONFIG_PACKAGE_DROPBEAR_VERSION),"2019.78")
PKG_VERSION:=2019.78
else
PKG_VERSION:=2011.54
endif

define Package/dropbearconvert
  $(call Package/dropbear/Default)
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=Utility for converting SSH keys
endef

define Package/dropbear/description
 A small SSH2 server/client designed for small memory environments.
endef

define Package/dropbear
  $(call Package/dropbear/Default)
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=Small SSH2 client/server
ifeq ($(PKG_VERSION),2019.78)
  DEPENDS:= +DROPBEAR_ZLIB:zlib +libuci
else
  DEPENDS:=+liblua
endif
endef

define Package/dropbear/Default
  URL:=http://matt.ucc.asn.au/dropbear/
ifeq ($(PKG_VERSION),2019.78)
  DEPENDS:=+libuci
endif
endef

PKG_NAME:=dropbear
PKG_RELEASE:=2
PKG_BUILD_PARALLEL:=1


ifeq ($(PKG_VERSION),2019.78)
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_USE_MIPS16:=0
PKG_FIXUP:=autoreconf

PKG_CONFIG_DEPENDS:= \
	CONFIG_TARGET_INIT_PATH CONFIG_DROPBEAR_ECC CONFIG_DROPBEAR_ECC_FULL \
	CONFIG_DROPBEAR_CURVE25519 CONFIG_DROPBEAR_ZLIB \
	CONFIG_DROPBEAR_UTMP CONFIG_DROPBEAR_PUTUTLINE \
	CONFIG_DROPBEAR_DBCLIENT

ifneq ($(DUMP),1)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell echo $(CONFIG_TARGET_INIT_PATH) | mkhash md5)
endif

CONFIG_DROPBEAR_DBCLIENT:=1

define Package/dropbear/config
	source "$(SOURCE)/Config.in"
endef

define Package/dropbear/conffiles
$(if $(CONFIG_DROPBEAR_ECC),/etc/dropbear/dropbear_ecdsa_host_key)
/etc/dropbear/dropbear_rsa_host_key
/etc/config/dropbear
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src_2019_78/* $(PKG_BUILD_DIR)
endef

ESED:=$(STAGING_DIR_HOST)/bin/sed -E -i -e

CONFIGURE_ARGS += \
	--disable-pam \
	--enable-openpty \
	--enable-syslog \
	--disable-lastlog \
	--disable-utmpx \
	$(if $(CONFIG_DROPBEAR_UTMP),,--disable-utmp) \
	--disable-wtmp \
	--disable-wtmpx \
	--disable-loginfunc \
	--disable-harden \
	$(if $(CONFIG_DROPBEAR_PUTUTLINE),,--disable-pututline) \
	--disable-pututxline \
	$(if $(CONFIG_DROPBEAR_ZLIB),,--disable-zlib) \
	--enable-bundled-libtom

TARGET_CFLAGS += -DARGTYPE=3 -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections
TARGET_LDFLAGS += -L$(STAGING_DIR)/usr/lib -luci 

ifneq ($(CONFIG_DROPBEAR_FORK_LIMIT),)
	TARGET_CPPFLAGS += -DFORK_LIMIT
endif

ifeq ($(CONFIG_PACKAGE_tmp-server_func_service_auth),y)
	TARGET_CPPFLAGS += -DALLOW_SERVICE_AUTH=1
endif

ifeq ($(CONFIG_FAILED_LOGIN_ATTEMPTS_LIMIT),y)
TARGET_CFLAGS += -DFAILED_LOGIN_ATTEMPTS_LIMIT
endif

ifeq ($(CONFIG_SUPPORT_UPDATE_SSH_IDENT),y)
TARGET_CFLAGS += -DINCLUDE_UPDATE_SSH_IDENT
endif

ifeq ($(CONFIG_SUPPORT_PASSWD_IN_SHADOW),y)
TARGET_CFLAGS += -DPASSWD_IN_SHADOW
endif

ifeq ($(CONFIG_SUPPORT_SSH_DISABLE_DEFAULT),y)
TARGET_CFLAGS += -DSSH_DISABLE_DEFAULT
endif

ifneq (,$(findstring ipq95xx,$(BOARD)))
TARGET_CFLAGS += -DINCLUDE_IPQ95XX
endif

define Build/Configure
	: > $(PKG_BUILD_DIR)/localoptions.h

	$(Build/Configure/Default)

	#echo '#define DEFAULT_PATH "$(TARGET_INIT_PATH)"' >> \
		#$(PKG_BUILD_DIR)/localoptions.h

	echo '#define DROPBEAR_FAIL_COUNT "/var/dropbear/fail_count"' >> \
		$(PKG_BUILD_DIR)/localoptions.h

	echo '#define DROPBEAR_ARP_TABLE  "/proc/net/arp"' >> \
		$(PKG_BUILD_DIR)/localoptions.h

	echo '#define DROPBEAR_CURVE25519 $(if $(CONFIG_DROPBEAR_CURVE25519),1,0)' >> \
		$(PKG_BUILD_DIR)/localoptions.h

	for OPTION in DROPBEAR_ECDSA DROPBEAR_ECDH; do \
		echo "#define $$$$OPTION $(if $(CONFIG_DROPBEAR_ECC),1,0)" >> \
			$(PKG_BUILD_DIR)/localoptions.h; \
	done

	# remove protocol idented software version number
	echo "$(ESED)"
	$(ESED) 's,^(#define LOCAL_IDENT) .*$$$$,\1 $(if $(CONFIG_SUPPORT_UPDATE_SSH_IDENT),"TPS-2.0-dropbear_\" DROPBEAR_VERSION,"SSH-2.0-dropbear_\" DROPBEAR_VERSION),g' \
		$(PKG_BUILD_DIR)/sysoptions.h

	# disable legacy/unsafe methods and unused functionality
	for OPTION in INETD_MODE DROPBEAR_CLI_NETCAT \
	DROPBEAR_3DES DROPBEAR_DSS DROPBEAR_ENABLE_CBC_MODE \
	DROPBEAR_SHA1_96_HMAC DROPBEAR_USE_PASSWORD_ENV; do \
		echo "#define $$$$OPTION 0" >> \
			$(PKG_BUILD_DIR)/localoptions.h; \
	done

	# enable nistp384 and nistp521 only if full ECC support was requested
	for OPTION in DROPBEAR_ECC_384 DROPBEAR_ECC_521; do \
		$(ESED) 's,^(#define '$$$$OPTION') .*$$$$,\1 $(if $(CONFIG_DROPBEAR_ECC_FULL),1,0),g' \
		$(PKG_BUILD_DIR)/sysoptions.h; \
	done

	# Enforce rebuild of svr-chansession.c
	rm -f $(PKG_BUILD_DIR)/svr-chansession.o
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		PROGRAMS="dropbear $(if $(CONFIG_DROPBEAR_DBCLIENT),dbclient,) dropbearkey scp" \
		MULTI=1 SCPPROGRESS=1
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		PROGRAMS="dropbearconvert"
endef

define Package/dropbear/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbearmulti $(1)/usr/sbin/dropbear
	$(INSTALL_DIR) $(1)/usr/bin
	$(LN) ../sbin/dropbear $(1)/usr/bin/scp
	$(LN) ../sbin/dropbear $(1)/usr/bin/ssh
	$(if $(CONFIG_DROPBEAR_DBCLIENT),$(LN) ../sbin/dropbear $(1)/usr/bin/dbclient,)
	$(LN) ../sbin/dropbear $(1)/usr/bin/dropbearkey
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./filesystem_2019_78/dropbear.config $(1)/etc/config/dropbear
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem_2019_78/dropbear.init $(1)/etc/init.d/dropbear
	$(call init_order,$(1)/etc/init.d/dropbear)
	$(INSTALL_DIR) $(1)/usr/lib/opkg/info
	$(INSTALL_DIR) $(1)/etc/dropbear
	$(if $(CONFIG_DROPBEAR_ECC),touch $(1)/etc/dropbear/dropbear_ecdsa_host_key)
	touch $(1)/etc/dropbear/dropbear_rsa_host_key
endef

else
#2011.54
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:= \
	http://matt.ucc.asn.au/dropbear/releases/ \
	http://www.mirrors.wiretapped.net/security/cryptography/apps/ssh/dropbear/
PKG_MD5SUM:=c627ffe09570fad7aa94d8eac2b9320c

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/dropbear/conffiles
/etc/dropbear/dropbear_rsa_host_key
/etc/dropbear/dropbear_dss_host_key 
/etc/config/dropbear 
endef

define Package/dropbear/config
	config DROPBEAR_SIMPLE
		bool "Simplification for dropbear"
		depends PACKAGE_dropbear
		default n
		help
			cut dbclient and scp
endef

CONFIGURE_ARGS += \
	--with-shared \
	--disable-pam \
	--enable-openpty \
	--enable-syslog \
	$(if $(CONFIG_SHADOW_PASSWORDS),,--disable-shadow) \
	--disable-lastlog \
	--disable-utmp \
	--disable-utmpx \
	--disable-wtmp \
	--disable-wtmpx \
	--disable-loginfunc \
	--disable-pututline \
	--disable-pututxline \
	--disable-zlib \
	--enable-bundled-libtom

TARGET_CFLAGS += -DARGTYPE=3 -ffunction-sections -fdata-sections
TARGET_LDFLAGS += -Wl,--gc-sections

ifeq ($(CONFIG_DROPBEAR_SIMPLE),y)
TARGET_CFLAGS += -DDROPBEAR_SIMPLE
endif

ifneq ($(CONFIG_DROPBEAR_FORK_LIMIT),)
	TARGET_CPPFLAGS += -DFORK_LIMIT
endif

ifeq ($(CONFIG_PACKAGE_tmp-server_func_service_auth),y)
	TARGET_CPPFLAGS += -DALLOW_SERVICE_AUTH=1
endif

ifeq ($(CONFIG_FAILED_LOGIN_ATTEMPTS_LIMIT),y)
TARGET_CFLAGS += -DFAILED_LOGIN_ATTEMPTS_LIMIT
endif

define Build/Configure
	$(SED) 's,^/\* #define PKG_MULTI.*,#define PKG_MULTI,g' $(PKG_BUILD_DIR)/options.h
	$(SED) 's,^#define DO_HOST_LOOKUP,/* & */,g' $(PKG_BUILD_DIR)/options.h
	$(call Build/Configure/Default)
endef

ifeq ($(CONFIG_DROPBEAR_SIMPLE),y)
define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		LD="$(TARGET_CC)" \
		PROGRAMS="dropbear dropbearkey" \
		MULTI=1 SCPPROGRESS=0
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		LD="$(TARGET_CC)" \
		PROGRAMS="dropbearconvert"
endef

define Package/dropbear/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbearmulti $(1)/usr/sbin/dropbear
	$(INSTALL_DIR) $(1)/usr/bin
	#ln -sf ../sbin/dropbear $(1)/usr/bin/scp
	#ln -sf ../sbin/dropbear $(1)/usr/bin/ssh
	#ln -sf ../sbin/dropbear $(1)/usr/bin/dbclient
	ln -sf ../sbin/dropbear $(1)/usr/bin/dropbearkey
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./filesystem/etc/config/dropbear $(1)/etc/config/dropbear
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/etc/init.d/dropbear $(1)/etc/init.d/dropbear
	$(INSTALL_DIR) $(1)/usr/lib/opkg/info
	$(INSTALL_DIR) $(1)/etc/dropbear
	touch $(1)/etc/dropbear/dropbear_rsa_host_key
	touch $(1)/etc/dropbear/dropbear_dss_host_key
endef
else
define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		LD="$(TARGET_CC)" \
		PROGRAMS="dropbear dbclient dropbearkey scp" \
		MULTI=1 SCPPROGRESS=1
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		LD="$(TARGET_CC)" \
		PROGRAMS="dropbearconvert"
endef

define Package/dropbear/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbearmulti $(1)/usr/sbin/dropbear
	$(INSTALL_DIR) $(1)/usr/bin
	ln -sf ../sbin/dropbear $(1)/usr/bin/scp
	ln -sf ../sbin/dropbear $(1)/usr/bin/ssh
	ln -sf ../sbin/dropbear $(1)/usr/bin/dbclient
	ln -sf ../sbin/dropbear $(1)/usr/bin/dropbearkey
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_DATA) ./filesystem/etc/config/dropbear $(1)/etc/config/dropbear
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/etc/init.d/dropbear $(1)/etc/init.d/dropbear
	$(INSTALL_DIR) $(1)/usr/lib/opkg/info
	$(INSTALL_DIR) $(1)/etc/dropbear
	touch $(1)/etc/dropbear/dropbear_rsa_host_key
	touch $(1)/etc/dropbear/dropbear_dss_host_key
endef
endif
endif

define Package/dropbearconvert/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbearconvert $(1)/usr/bin/dropbearconvert
endef

$(eval $(call BuildPackage,dropbear))
$(eval $(call BuildPackage,dropbearconvert))
