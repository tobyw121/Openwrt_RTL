# LuCI owns /www/index.html when selected. The standalone MW5 WebUI only
# supplies the landing page for MW5 images built without LuCI.
define Package/base-files/install-target
	rm -f $(1)/www/index.html
	$(if $(and $(filter y,$(CONFIG_TARGET_realtek_rtl8197f_DEVICE_tenda_nova_mw5)),$(if $(filter y m,$(CONFIG_PACKAGE_luci-base)),,y)), \
		$(INSTALL_DIR) $(1)/www; \
		$(INSTALL_DATA) $(PLATFORM_DIR)/rtl8197f/mw5-index.html $(1)/www/index.html)
endef
