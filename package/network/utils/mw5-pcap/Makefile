include $(TOPDIR)/rules.mk

PKG_NAME:=mw5-pcap
PKG_RELEASE:=1
PKG_LICENSE:=ISC

include $(INCLUDE_DIR)/package.mk

define Package/mw5-pcap
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Tiny AF_PACKET to PCAP capture helper for Tenda MW5 diagnostics
endef

define Package/mw5-pcap/description
 A dependency-free packet capture helper used by mw5-wifi sniffing. It binds
 directly to an existing WLAN netdev with AF_PACKET and writes Ethernet PCAP.
endef

define Build/Configure
endef

define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) $(TARGET_CPPFLAGS) -Wall -Wextra \
		-o $(PKG_BUILD_DIR)/mw5-pcap $(PKG_BUILD_DIR)/mw5-pcap.c $(TARGET_LDFLAGS)
endef

define Package/mw5-pcap/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mw5-pcap $(1)/usr/sbin/mw5-pcap
endef

$(eval $(call BuildPackage,mw5-pcap))
