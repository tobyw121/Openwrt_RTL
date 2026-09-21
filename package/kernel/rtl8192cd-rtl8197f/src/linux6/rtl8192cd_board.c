// SPDX-License-Identifier: GPL-2.0-only
/* Linux 6.x DT/NVMEM board-data glue for MW5 RTL8197FS/RTL8812BRH. */
#include <linux/device.h>
#include <linux/etherdevice.h>
#include <linux/nvmem-consumer.h>
#include <linux/property.h>
#include <linux/slab.h>

#include "rtl8192cd_linux6.h"

static void rtl8192cd_linux6_kfree(void *data)
{
	kfree(data);
}

static int rtl8192cd_linux6_read_cell(struct device *dev, const char *name,
				      void **data, size_t *data_len)
{
	struct nvmem_cell *cell;
	void *buf;
	size_t len;
	int ret;

	cell = devm_nvmem_cell_get(dev, name);
	if (IS_ERR(cell)) {
		ret = PTR_ERR(cell);
		if (ret == -ENOENT || ret == -ENODEV || ret == -ENODATA)
			return 0;
		return dev_err_probe(dev, ret, "failed to get %s NVMEM cell\n", name);
	}

	buf = nvmem_cell_read(cell, &len);
	if (IS_ERR(buf))
		return dev_err_probe(dev, PTR_ERR(buf), "failed to read %s NVMEM cell\n", name);

	ret = devm_add_action_or_reset(dev, rtl8192cd_linux6_kfree, buf);
	if (ret)
		return ret;

	*data = buf;
	*data_len = len;
	return 0;
}

static int rtl8192cd_hex_nibble(u8 c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -EINVAL;
}

/* Tenda's MW5 factory partition is a binary header followed by NUL-separated
 * ASCII KEY=value records.  Keep parsing bounded so a malformed partition can
 * never turn into an unbounded string operation. */
static const u8 *rtl8192cd_factory_value(const void *factory, size_t factory_len,
					 const char *key, size_t *value_len)
{
	const u8 *buf = factory;
	size_t key_len = strlen(key);
	size_t i, end;

	if (!buf || !factory_len || !key_len || factory_len <= key_len + 1)
		return NULL;

	for (i = 0; i + key_len + 1 < factory_len; i++) {
		if (memcmp(buf + i, key, key_len) || buf[i + key_len] != '=')
			continue;

		end = i + key_len + 1;
		while (end < factory_len && buf[end] != '\0' &&
		       buf[end] != '\n' && buf[end] != '\r' && buf[end] != 0xff)
			end++;
		*value_len = end - (i + key_len + 1);
		return buf + i + key_len + 1;
	}

	return NULL;
}

static int rtl8192cd_factory_base_mac(const void *factory, size_t factory_len,
				      u8 *addr)
{
	const u8 *value;
	size_t len;
	int hi, lo;
	int i;

	value = rtl8192cd_factory_value(factory, factory_len, "HW_NIC0_ADDR", &len);
	if (!value || len != ETH_ALEN * 2)
		return -ENOENT;

	for (i = 0; i < ETH_ALEN; i++) {
		hi = rtl8192cd_hex_nibble(value[i * 2]);
		lo = rtl8192cd_hex_nibble(value[i * 2 + 1]);
		if (hi < 0 || lo < 0)
			return -EINVAL;
		addr[i] = (hi << 4) | lo;
	}

	return is_valid_ether_addr(addr) ? 0 : -EINVAL;
}

static int rtl8192cd_linux6_read_mac_cell(struct device *dev, u8 *addr)
{
	struct nvmem_cell *cell;
	void *buf;
	size_t len;
	int ret;

	cell = devm_nvmem_cell_get(dev, "mac-address");
	if (IS_ERR(cell))
		return PTR_ERR(cell);

	buf = nvmem_cell_read(cell, &len);
	if (IS_ERR(buf))
		return PTR_ERR(buf);

	if (len != ETH_ALEN || !is_valid_ether_addr(buf))
		ret = -EINVAL;
	else {
		ether_addr_copy(addr, buf);
		ret = 0;
	}

	kfree(buf);
	return ret;
}

int rtl8192cd_linux6_parse_board_data(struct device *dev,
				      struct rtl8192cd_linux6_board_data *board,
				      u8 default_radio, u8 default_rfe,
				      int default_mac_increment)
{
	u32 value;
	int mac_inc = default_mac_increment;
	int ret;

	memset(board, 0, sizeof(*board));
	board->radio_index = default_radio;
	board->rfe_type = default_rfe;

	if (!device_property_read_u32(dev, "realtek,radio-index", &value)) {
		if (value > 1)
			return dev_err_probe(dev, -EINVAL, "realtek,radio-index must be 0 or 1\n");
		board->radio_index = value;
	}

	if (!device_property_read_u32(dev, "realtek,rfe-type", &value)) {
		if (value > U8_MAX)
			return dev_err_probe(dev, -EINVAL, "realtek,rfe-type is out of range\n");
		board->rfe_type = value;
	}

	if (!device_property_read_u32(dev, "realtek,mac-address-increment", &value))
		mac_inc = (int)value;

	/* Read the complete vendor factory store once.  The RF layer consumes its
	 * radio-specific records through the vendor eFuse command table. */
	ret = rtl8192cd_linux6_read_cell(dev, "factory",
					&board->factory, &board->factory_len);
	if (ret)
		return ret;

	ret = rtl8192cd_linux6_read_mac_cell(dev, board->mac_addr);
	if (ret == -ENOENT || ret == -ENODEV || ret == -ENODATA) {
		ret = rtl8192cd_factory_base_mac(board->factory, board->factory_len,
						 board->mac_addr);
	}
	if (!ret) {
		if (mac_inc)
			eth_addr_add(board->mac_addr, mac_inc);
		if (!is_valid_ether_addr(board->mac_addr))
			return dev_err_probe(dev, -EINVAL, "invalid derived WLAN MAC address\n");
		board->mac_valid = true;
	} else if (ret != -ENOENT && ret != -ENODEV && ret != -ENODATA) {
		return dev_err_probe(dev, ret, "failed to obtain WLAN MAC address\n");
	}

	/* Optional generic cell: a complete logical vendor eFuse shadow map.  It
	 * remains supported for boards that provide one; MW5 uses the factory blob. */
	ret = rtl8192cd_linux6_read_cell(dev, "calibration",
					&board->calibration, &board->calibration_len);
	if (ret)
		return ret;

	dev_info(dev,
		 "board-data: radio=%u rfe=%u mac=%s calibration=%zu factory=%zu bytes\n",
		 board->radio_index, board->rfe_type,
		 board->mac_valid ? "NVMEM/factory" : "vendor/eFuse fallback",
		 board->calibration_len, board->factory_len);
	return 0;
}
