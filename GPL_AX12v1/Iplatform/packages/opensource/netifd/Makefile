include $(TOPDIR)/rules.mk

PKG_NAME:=netifd
PKG_VERSION:=2013-05-13
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://nbd.name/luci2/netifd.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=bc4a4bb127622c76085ecec7fd20448aad7bafaf
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
PKG_MAINTAINER:=Felix Fietkau <nbd@openwrt.org>
# PKG_MIRROR_MD5SUM:=
# CMAKE_INSTALL:=1

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

PKG_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

# For mips arch case, extern libopenat is required.
ifneq ($(CONFIG_OPENAT),)
  USELIBOPENAT=-lopenat
  EXTERN_OPENAT=-DEXTERN_OPENAT
endif

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

# provide for non-arm arch support
ifeq ($(CONFIG_LINUX_KARCH),"arm")
  arch_options=-D__ARM__=1
endif

define Package/netifd
  SECTION:=base
  # CATEGORY:=Base system
  CATEGORY:=TP-LINK iplatform apps
  SUBMENU:=Network
  DEPENDS:=+libuci +PACKAGE_libnl-tiny:libnl-tiny +libubus +ubus +ubusd +jshn +kmod-bonding +OPENAT:libopenat +libc-ares
  TITLE:=OpenWrt Network Interface Configuration Daemon
endef

ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"10.3")
TARGET_CFLAGS += -fcommon
endif

ifneq ($(CONFIG_FEATURE_3G4G_MODULES),)
	export FEATURE_3G4G_MODULES := 1
	ifeq ($(CONFIG_FEATURE_3G4G_MODULES),"v2")
		3g4g_options=-D__3G4G_V2__=1
		TARGET_CFLAGS += -DFEATURE_3G4G_MODULES=2
	else
		3g4g_options=-D__3G4G__=1
		TARGET_CFLAGS += -DFEATURE_3G4G_MODULES
	endif
endif

ifneq ($(CONFIG_NETIFD_SUPPORT_WIRELESS_DEVICE),)
	export SUPPORT_WIRELESS_DEVICE := 1
	wireless_options=-D__WIRELESS__=1
	TARGET_CFLAGS += -DSUPPORT_WIRELESS_DEVICE
endif

ifeq ($(CONFIG_PACKAGE_libc-ares), y)
    TARGET_CFLAGS += -D_HAVE_CARES_
    TARGET_LDFLAGS += -lcares
endif

ifneq ($(CONFIG_3G4G_INDEPENDENT),)
	TARGET_CFLAGS += -DFEATURE_3G4G_MODULES_INDEPENDENT
endif

ifneq ($(CONFIG_EXTEND_SOCKETOPT_BASE_NUM),)
	TARGET_CFLAGS += -DEXTEND_SOCKETOPT_BASE_NUM
endif

ifneq ($(CONFIG_SUPPORT_VPN_CLIENT_IN_3G4G_MODULES),)
	TARGET_CFLAGS += -DSUPPORT_VPN_CLIENT_IN_3G4G_MODULES
endif

ifneq (,$(findstring bcm490x,$(BOARD)))
	TARGET_CFLAGS += "-DBCM_490X"
endif

ifneq (,$(findstring intel_grx350,$(BOARD)))
	TARGET_CFLAGS += "-DINTEL_GRX350"
endif

ifneq (,$(findstring ipq50xx,$(BOARD)))
	TARGET_CFLAGS += -DQCA_IPQ50XX
endif

ifneq (,$(findstring mt762x,$(BOARD)))
	TARGET_CFLAGS += "-DMTK_MT762X"
endif

ifeq ($(CONFIG_LIBC_USE_MUSL),y)
	TARGET_CFLAGS += -DCONFIG_LIBC_USE_MUSL=y
endif

TARGET_CFLAGS += \
	-I$(STAGING_DIR)/usr/include $(USELIBOPENAT) \
	$(EXTERN_OPENAT)

CMAKE_OPTIONS += \
	-DDEBUG=1 $(arch_options) -DKEEP_BRIDGE=1 $(3g4g_options) $(wireless_options)\

ifeq ($(CONFIG_USE_SDK_NL_TINY),y)
TARGET_CFLAGS += -I$(SDK_STAGING_DIR)/usr/include/libnl-tiny
CMAKE_OPTIONS += '-DLIBNL_LIBS=-L$(SDK_STAGING_DIR)/usr/lib -lnl-tiny'
else
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/libnl-tiny
CMAKE_OPTIONS += -DLIBNL_LIBS=-lnl-tiny
endif

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/json

ifneq ($(CONFIG_NETIFD_HAS_DOWNUP_BUG),)
	TARGET_CFLAGS += -DNETIFD_HAS_DOWNUP_BUG
	export NETIFD_HAS_DOWNUP_BUG := 1
	CMAKE_OPTIONS += -DNETIFD_HAS_DOWNUP_BUG=1
endif

ifneq ($(CONFIG_NETIFD_WITH_BCM_IPTV),)
	TARGET_CFLAGS += -DNETIFD_WITH_BCM_IPTV
	export NETIFD_WITH_BCM_IPTV := 1
	CMAKE_OPTIONS += -DNETIFD_WITH_BCM_IPTV=1
endif

ifneq ($(CONFIG_NETIFD_BCM_FFS),)
	TARGET_CFLAGS += -DNETIFD_BCM_FFS
	export NETIFD_BCM_FFS := 1
	CMAKE_OPTIONS += -DNETIFD_BCM_FFS=1
endif

ifneq ($(CONFIG_PACKAGE_v6plus),)
	TARGET_CFLAGS += -DNETIFD_WITH_V6PLUS
	TARGET_CFLAGS += -DCONFIG_TP_IMAGE -DCONFIG_V6PLUS_DSLITE_SUPPORT
	export NETIFD_WITH_V6PLUS := 1
	CMAKE_OPTIONS += -DNETIFD_WITH_V6PLUS=1
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Package/netifd/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/netifd $(1)/sbin/
	$(CP) ./filesystem/* $(1)/
	$(CP) $(PKG_BUILD_DIR)/dummy/netifd-proto.sh $(1)/lib/netifd/
endef

$(eval $(call BuildPackage,netifd))
