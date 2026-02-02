#
# Copyright (C) 2010-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=net-snmp
PKG_VERSION:=5.7.2
PKG_RELEASE:=1

PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(INCLUDE_DIR)/package.mk

define Package/net-snmp/Default
  SECTION:=snmpd
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:= SNMP
  #URL:=http://www.net-snmp.org/
endef

define Package/net-snmp
  $(call Package/net-snmp/Default)
  TITLE:=Simple Network Management Protocol implement
endef

SNMP_MIB_MODULES_INCLUDED = \
	mibII/system_mib \
	mibII/ip \
	mibII/tcp \
	mibII/udp \
	mibII/icmp \

SNMP_MIB_MODULES_EXCLUDED = \
	agent_mibs \
	agentx \
	host \
	ieee802dot11 \
	notification \
	mibII \
	snmpv3mibs \
	ucd_snmp \
	utilities \
	target \

SNMP_TRANSPORTS_INCLUDED = Callback UDP

SNMP_TRANSPORTS_EXCLUDED = TCP TCPv6 UDPv6 Unix

SNMP_PREFIX=/tmp/snmp

SNMP_CONFIGURE_OPTIONS = \
	--prefix=$(SNMP_PREFIX) \
	--with-cc=$(TARGET_CC) \
	--with-ar=$(TARGET_AR) \
	--enable-shared=no \
	--enable-static=yes \
	--enable-internal-md5 \
	--enable-mini-agent \
	--disable-applications \
	--disable-debugging \
	--disable-ipv6 \
	--disable-manuals \
	--disable-mib-loading \
	--disable-mibs \
	--disable-scripts \
	--disable-ucd-snmp-compatibility \
	--disable-privacy \
	--disable-embedded-perl \
	--disable-snmptrapd-subagent \
	--with-default-snmp-version="1" \
	--with-out-mib-modules="$(SNMP_MIB_MODULES_EXCLUDED)" \
	--with-mib-modules="$(SNMP_MIB_MODULES_INCLUDED)" \
	--with-out-transports="$(SNMP_TRANSPORTS_EXCLUDED)" \
	--with-transports="$(SNMP_TRANSPORTS_INCLUDED)" \
	--without-opaque-special-types \
	--without-perl-modules \
	--without-kmem-usage \
	--without-openssl \
	--without-libwrap \
	--without-rpm \
	--without-zlib \
	--without-efence \
	--without-rsaref \
	--without-kmem-usage \
	--without-dmalloc\
	--with-sys-contact=root@localhost \
	--with-sys-location=Unknown \
	--with-logfile=/dev/null \
	--with-persistent-directory=$(SNMP_PREFIX)/persist \
	-sysconfdir=$(SNMP_PREFIX)

CONFIGURE_ARGS += $(SNMP_CONFIGURE_OPTIONS)	

define Package/net-snmp/conffiles
/etc/snmpd.conf
endef

#define Build/Configure
#	$(call Build/Configure/Default)
#endef
	
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/net-snmp/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/agent/snmpd $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./filesystem/etc/init.d/snmpd $(1)/etc/init.d/snmpd
	$(INSTALL_DIR) $(1)/etc/hotplug.d/firewall
	$(INSTALL_DATA) ./filesystem/etc/hotplug.d/firewall/50-snmp $(1)/etc/hotplug.d/firewall/
endef

$(eval $(call BuildPackage,net-snmp))
