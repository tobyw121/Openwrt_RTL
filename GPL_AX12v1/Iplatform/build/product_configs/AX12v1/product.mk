
sdk_build:
	cd $(SDK_ROOT_DIR) && $(MAKE) all V=s

sdk: sdk_silent_config sdk_build kernel_header
	@echo "build sdk done"

sdk_silent_config:
	cd $(SDK_ROOT_DIR) && make silentconfig

kernel_header_clean:
	[ ! -d "$(PLATFORM_DIR)/staging_dir/usr-$(PRODUCT_NAME)" ] || rm -rf $(PLATFORM_DIR)/staging_dir/usr-$(PRODUCT_NAME)

kernel_header:kernel_header_clean
	cd $(SDK_KERNEL_DIR) && $(MAKE) ARCH=mips INSTALL_HDR_PATH=$(PLATFORM_DIR)/staging_dir/usr-$(PRODUCT_NAME) headers_install
