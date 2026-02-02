#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=xtables-addons
ifeq ($(CONFIG_PACKAGE_XTABLES_ADDONS_VERSION),"v3")
PKG_VERSION:=3.0
PKG_RELEASE:=3.1
PKG_MD5SUM:=b37ed4d9c28cdcd5558c55934be8d051
PKG_BUILD_SRC:=./src-3.x/*
else ifeq ($(CONFIG_PACKAGE_XTABLES_ADDONS_VERSION),"v2")
PKG_VERSION:=2.10
PKG_RELEASE:=3.1
PKG_MD5SUM:=727bf0dd4a3d9c65724267bd0d5d80b0
PATCH_DIR:=./patches-2.x
PKG_BUILD_SRC:=./src-2.x/*
else
PKG_VERSION:=1.42
PKG_RELEASE:=3.3
PKG_MD5SUM:=7c996a0400667b57ab4fb53a013ae742
PATCH_DIR:=./patches
PKG_BUILD_SRC:=./src/*
endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@SF/xtables-addons
PKG_BUILD_DEPENDS:=iptables
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/package.mk

define Package/xtables-addons
  SECTION:=net
  CATEGORY:=Network
  SUBMENU:=Firewall
  TITLE:=Extensions not distributed in the main Xtables
  URL:=http://xtables-addons.sourceforge.net/
endef

define Package/xtables-addons/config
	config PACKAGE_XTABLES_ADDONS_VERSION
		string "xtables-addons compile version"
		depends PACKAGE_xtables-addons
		default "v1"
		help
			if kernel version > 3.7, please set the value to v2 to support it. otherwise let it to default value v1.
			current support valid value "v1" and "v2"
endef

# uses GNU configure

CONFIGURE_ARGS+= \
	--with-kbuild="$(LINUX_DIR)" \
	--with-xtables="$(STAGING_DIR)/usr" \
	--with-xtlibdir="/usr/lib/iptables" \

#add by wanghao
ifneq ($(CONFIG_PACKAGE_kmod-shortcut-fe),)
  EXTRA_CFLAGS+=-DSUPPORT_SHORTCUT_FE
endif
#add end

#add by CCy
ifneq ($(CONFIG_PACKAGE_kmod-blockingx),)
  EXTRA_CFLAGS+=-DSUPPORT_HOMECARE_PRO_BLOCKING
ifneq (,$(findstring ipq50xx,$(BOARD)))
  EXTRA_CFLAGS+=-DSUPPORT_QCA_NSS
endif
ifneq (,$(findstring ipq95xx,$(BOARD)))
  EXTRA_CFLAGS+=-DSUPPORT_QCA_NSS
endif
#add for home shield block url, by Cheng Ming
ifneq ($(CONFIG_BLOCK_URL),)
  EXTRA_CFLAGS+=-DPCTL_BLOCK_URL
endif
#add end
endif

ifneq ($(CONFIG_PACKAGE_libauc),)
  EXTRA_CFLAGS+=-DSUPPORT_HOMECARE_PRO_URL_CLASS
endif
#add end

#add by Wuzexuan
ifneq ($(CONFIG_PCTL_IPV6),)
  EXTRA_CFLAGS+=-DPCTL_SUPPORT_IPV6
endif
ifneq ($(CONFIG_USER_32BIT_COMPAT_KERNEL_64BIT),)
  EXTRA_CFLAGS+=-DUSER_32BIT_COMPAT_KERNEL_64BIT
endif
#add end

#add for xt_v6ports.c
ifneq (,$(findstring ipq50xx,$(BOARD)))
  EXTRA_CFLAGS+=-DQCA_IPQ50XX
endif
#add end

ifneq (,$(findstring ipq95xx,$(BOARD)))
  EXTRA_CFLAGS+=-Wno-discarded-qualifiers
  EXTRA_CFLAGS+=-Wno-incompatible-pointer-types
  EXTRA_CFLAGS+=-Wno-error=frame-larger-than=
endif

ifeq ($(CONFIG_MERCUSYS_TYPE_SUPPORT),y)
EXTRA_CFLAGS += -DMERCUSYS_TYPE_SUPPORT
endif
#for h files are not under kernel dir sometimes
ifdef CONFIG_LINUX_KERNEL_INCLUDE_PATH
	EXTRA_CFLAGS += -I$(LINUX_DIR)/../$(CONFIG_LINUX_KERNEL_INCLUDE_PATH)
endif

ifneq (,$(CONFIG_PACKAGE_kmod-ipt-nathelper-rtsp))
        export config_build_rtsp=m
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(PKG_BUILD_SRC) $(PKG_BUILD_DIR)
	$(call PatchDir,$(PKG_BUILD_DIR),$(PATCH_DIR),)
	echo -------$(CONFIG_PACKAGE_XTABLES_ADDONS_VERSION)--------
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(KERNEL_CROSS)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		DEPMOD="/bin/true" \
		all
endef

define Build/Install
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(LINUX_KARCH)" \
		CROSS_COMPILE="$(KERNEL_CROSS)" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		DEPMOD="/bin/true" \
		install
endef

# 1: extension/module suffix used in package name
# 2: extension/module display name used in package title/description
# 3: list of extensions to package
# 4: list of modules to package
# 5: module load priority
# 6: module depends
define BuildTemplate

 ifneq ($(3),)
  define Package/iptables-mod-$(1)
    $$(call Package/xtables-addons)
    CATEGORY:=Network
    TITLE:=$(2) iptables extension
    DEPENDS:=iptables $(if $(4),+kmod-ipt-$(1))
  endef

  define Package/iptables-mod-$(1)/install
	$(INSTALL_DIR) $$(1)/usr/lib/iptables
	for m in $(3); do \
		$(CP) \
			$(PKG_INSTALL_DIR)/usr/lib/iptables/lib$$$$$$$${m}.so \
			$$(1)/usr/lib/iptables/ ; \
	done
  endef

  $$(eval $$(call BuildPackage,iptables-mod-$(1)))
 endif

 ifneq ($(4),)
  define KernelPackage/ipt-$(1)
    SUBMENU:=Netfilter Extensions
    TITLE:=$(2) netfilter module
    DEPENDS:=kmod-ipt-core $(6)
    KCONFIG:=$(7)
    FILES:=$(foreach mod,$(4),$(PKG_BUILD_DIR)/extensions/$(mod).$(LINUX_KMOD_SUFFIX))
    AUTOLOAD:=$(call AutoLoad,$(5),$(notdir $(4)))
  endef

  ifneq ($(CONFIG_PACKAGE_iptables-mod-$(1)),)
	export config_build_$(1)=m
  endif

  $$(eval $$(call KernelPackage,ipt-$(1)))
 endif

endef


define Package/iptaccount
  $(call Package/xtables-addons)
  CATEGORY:=Network
  TITLE:=iptables-mod-account control utility
  DEPENDS:=iptables +iptables-mod-account
endef

define Package/iptaccount/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libxt_ACCOUNT_cl.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/sbin/iptaccount \
		$(1)/usr/sbin/
endef


#$(eval $(call BuildTemplate,SUFFIX,DESCRIPTION,EXTENSION,MODULE,PRIORITY,DEPENDS))

$(eval $(call BuildTemplate,compat-xtables,API compatibilty layer,,compat_xtables,45,,CONFIG_NF_CONNTRACK_MARK=y))
$(eval $(call BuildTemplate,rawpost,RAWPOST,,iptable_rawpost $(if $(CONFIG_IPV6),ip6table_rawpost),50,+kmod-ipt-compat-xtables))
ifeq ($(CONFIG_PACKAGE_XTABLES_ADDONS_VERSION),"v3")
#$(eval $(call BuildTemplate,nathelper-rtsp,RTSP Conntrack and NAT,,rtsp/nf_conntrack_rtsp rtsp/nf_nat_rtsp,46,+kmod-ipt-conntrack-extra))
else
#add by wanghao
ifeq (,$(findstring ipq50xx,$(BOARD))$(findstring ipq95xx,$(BOARD)))
$(eval $(call BuildTemplate,nathelper-rtsp,RTSP Conntrack and NAT,,rtsp/nf_conntrack_rtsp rtsp/nf_nat_rtsp,46,+kmod-ipt-conntrack-extra))
endif
#add end
endif

$(eval $(call BuildTemplate,account,ACCOUNT,xt_ACCOUNT,ACCOUNT/xt_ACCOUNT,46,+kmod-ipt-compat-xtables))

# add by huluyao, 2014 02 13
$(eval $(call BuildTemplate,httphost,httphost,xt_httphost,xt_httphost,46,))
$(eval $(call BuildTemplate,urlfilter,urlfilter,xt_urlfilter,xt_urlfilter,46,))
$(eval $(call BuildTemplate,app,app,xt_app,xt_app,46,,CONFIG_NF_CONNTRACK_TP_APP=y CONFIG_REG_PCRE=y))
$(eval $(call BuildTemplate,pctl,pctl,xt_pctl,xt_pctl,46,))
$(eval $(call BuildTemplate,blogskip,BLOGSKIP,xt_BLOGSKIP,xt_BLOGSKIP,46,))
$(eval $(call BuildTemplate,blog,blog,xt_blog,xt_blog,46,))
$(eval $(call BuildTemplate,blogtg,BLOGTG,xt_BLOGTG,xt_BLOGTG,46,))

$(eval $(call BuildTemplate,chaos,CHAOS,xt_CHAOS,xt_CHAOS,47,+kmod-ipt-compat-xtables +kmod-ipt-delude +kmod-ipt-tarpit))
$(eval $(call BuildTemplate,condition,Condition,xt_condition,xt_condition,46,))
$(eval $(call BuildTemplate,delude,DELUDE,xt_DELUDE,xt_DELUDE,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,dhcpmac,DHCPMAC,xt_DHCPMAC,xt_DHCPMAC,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,dnetmap,DNETMAP,xt_DNETMAP,xt_DNETMAP,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,fuzzy,fuzzy,xt_fuzzy,xt_fuzzy,46,))
$(eval $(call BuildTemplate,geoip,geoip,xt_geoip,xt_geoip,46,))
$(eval $(call BuildTemplate,iface,iface,xt_iface,xt_iface,46,))
$(eval $(call BuildTemplate,ipmark,IPMARK,xt_IPMARK,xt_IPMARK,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,ipp2p,IPP2P,xt_ipp2p,xt_ipp2p,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,ipv4options,ipv4options,xt_ipv4options,xt_ipv4options,46,))
$(eval $(call BuildTemplate,length2,length2,xt_length2,xt_length2,46,))
$(eval $(call BuildTemplate,logmark,LOGMARK,xt_LOGMARK,xt_LOGMARK,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,lscan,lscan,xt_lscan,xt_lscan,46,))
$(eval $(call BuildTemplate,lua,Lua PacketScript,xt_LUA,LUA/xt_LUA,46,+kmod-ipt-conntrack-extra))
$(eval $(call BuildTemplate,psd,psd,xt_psd,xt_psd,46,))
$(eval $(call BuildTemplate,quota2,quota2,xt_quota2,xt_quota2,46,))
$(eval $(call BuildTemplate,rawnat,RAWNAT,xt_RAWDNAT xt_RAWSNAT,xt_RAWNAT,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,steal,STEAL,xt_STEAL,xt_STEAL,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,sysrq,SYSRQ,xt_SYSRQ,xt_SYSRQ,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,trigger,TRIGGER,ipt_TRIGGER,ipt_TRIGGER,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,tarpit,TARPIT,xt_TARPIT,xt_TARPIT,46,+kmod-ipt-compat-xtables))
$(eval $(call BuildTemplate,LOOPBACKDNAT,LOOPBACKDNAT,ipt_LOOPBACKDNAT,xt_LOOPBACKDNAT,46,))
$(eval $(call BuildTemplate,V6PORTS,V6PORTS,ipt_V6PORTS,xt_V6PORTS,46,))
$(eval $(call BuildTemplate,CHECKPORTS,CHECKPORTS,ipt_CHECKPORTS,xt_CHECKPORTS,46,))

$(eval $(call BuildPackage,iptaccount))
$(eval $(call BuildPackage,xtables-addons))
