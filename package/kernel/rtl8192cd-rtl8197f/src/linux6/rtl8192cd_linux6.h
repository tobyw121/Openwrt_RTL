/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef __RTL8192CD_LINUX6_H__
#define __RTL8192CD_LINUX6_H__

#include <linux/device.h>
#include <linux/if_ether.h>
#include <linux/types.h>

struct dentry;
struct pci_dev;
struct rtl8192cd_priv;

enum rtl8192cd_linux6_bus_type {
	RTL8192CD_BUS_SOC = 0,
	RTL8192CD_BUS_PCI = 1,
};

struct rtl8192cd_linux6_board_data {
	u8 radio_index;
	u8 rfe_type;
	u8 mac_addr[ETH_ALEN];
	bool mac_valid;
	void *calibration;
	size_t calibration_len;
	void *factory;
	size_t factory_len;
};

struct rtl8192cd_linux6_hw {
	struct device *dev;
	struct pci_dev *pdev;
	void __iomem *mmio;
	resource_size_t mmio_len;
	int irq;
	enum rtl8192cd_linux6_bus_type bus_type;
	struct rtl8192cd_linux6_board_data board;
	struct rtl8192cd_priv *priv;
	struct dentry *debugfs_dir;
};

int rtl8192cd_linux6_parse_board_data(struct device *dev,
				      struct rtl8192cd_linux6_board_data *board,
				      u8 default_radio, u8 default_rfe,
				      int default_mac_increment);

int rtl8192cd_linux6_attach(struct rtl8192cd_linux6_hw *hw);
void rtl8192cd_linux6_detach(struct rtl8192cd_linux6_hw *hw);

int rtl8197f_soc_register(void);
void rtl8197f_soc_unregister(void);
int rtl8822b_pci_register(void);
void rtl8822b_pci_unregister(void);

int rtl8192cd_linux6_register_buses(void);
void rtl8192cd_linux6_unregister_buses(void);

#endif
