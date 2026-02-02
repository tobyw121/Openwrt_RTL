#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=udt-dropbear
PKG_VERSION:=2015.67
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE libtomcrypt/LICENSE libtommath/LICENSE

PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

PKG_CONFIG_DEPENDS:=CONFIG_DROPBEAR_ECC

include $(INCLUDE_DIR)/package.mk

export CFLAGS+=-I$(STAGING_DIR)/usr/include
export LDFLAGS+=-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib

define Package/$(PKG_NAME)/Default
  SUBMENU:=P2P
  DEPENDS:=+libuci +udt4 +libstdcpp +libpthread
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  SECTION:=TP-LINK
  CATEGORY:=TP-LINK iplatform apps
  TITLE:=udt-dropbear
endef

define Package/$(PKG_NAME)/description
 dropbear over udt
endef

define Package/$(PKG_NAME)/conffiles
/etc/udt-dropbear/dropbear_rsa_host_key
/etc/udt-dropbear/dropbear_dss_host_key
/etc/config/udt_dropbear
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

CONFIGURE_ARGS += \
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
TARGET_LDFLAGS += -luci -Wl,--gc-sections

#define Build/Configure
#	$(Build/Configure/Default)

	# Enforce that all replacements are made, otherwise options.h has changed
	# format and this logic is broken.
#	for OPTION in DROPBEAR_ECDSA DROPBEAR_ECDH DROPBEAR_CURVE25519; do \
#	  awk 'BEGIN { rc = 1 } \
#	       /'$$$$OPTION'/ { $$$$0 = "$(if $(CONFIG_DROPBEAR_ECC),,// )#define '$$$$OPTION'"; rc = 0 } \
#	       { print } \
#	       END { exit(rc) }' $(PKG_BUILD_DIR)/options.h \
#	       >$(PKG_BUILD_DIR)/options.h.new && \
#	  mv $(PKG_BUILD_DIR)/options.h.new $(PKG_BUILD_DIR)/options.h || exit 1; \
#	done
#endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		$(TARGET_CONFIGURE_OPTS) \
		PROGRAMS="libudtdb dropbear dbclient dropbearkey scp" \
		SCPPROGRESS=1  #MULTI=1 
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib/
#	$(INSTALL_DIR) $(1)/usr/sbin
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbear $(1)/usr/sbin/udt_dropbear
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbear $(1)/usr/bin/udtdbsvr
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dbclient $(1)/usr/bin/udtdbcli
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scp $(1)/usr/bin/scp
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dropbearkey $(1)/usr/bin/udtdbkey
#	$(LN) ../sbin/udt_dropbear $(1)/usr/bin/udt_scp
#	$(LN) ../sbin/udt_dropbear $(1)/usr/bin/udt_ssh
#	$(LN) ../sbin/udt_dropbear $(1)/usr/bin/udt_dbclient
#	$(LN) ../sbin/udt_dropbear $(1)/usr/bin/udt_dropbearkey
#	$(INSTALL_DIR) $(1)/etc/config
#	$(INSTALL_DATA) ./files/dropbear.config $(1)/etc/config/udt_dropbear
#	$(INSTALL_DIR) $(1)/etc/init.d
#	$(INSTALL_BIN) ./files/dropbear.init $(1)/etc/init.d/udt_dropbear
	$(INSTALL_DIR) $(1)/usr/lib/opkg/info
#	$(INSTALL_DIR) $(1)/etc/udt-dropbear
#	touch $(1)/etc/udt-dropbear/dropbear_rsa_host_key
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/udt-dropbear $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
