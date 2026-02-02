/*
 * Copyright(c) 2018 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 */

#ifndef __RTW_HWSIM_H_
#define __RTW_HWSIM_H_

#include <linux/platform_device.h> /* for struct platform_driver */
#include <net/cfg80211.h>
#include <linux/netdevice.h>
#include <linux/slab.h>

#include <drv_types.h>	      /* for staruct _ADAPTER, RTW_DBG, ... */

extern struct list_head g_data_list;
extern spinlock_t rtw_hwsim_data_lock;
extern u16 sim_machine_id;

#define RTW_HWSIM_MEDIUM_LOCAL 1
#define RTW_HWSIM_MEDIUM_SOCKET_UDP 2
#define RTW_HWSIM_MEDIUM_LOOPBACK 3

#define RTW_HWSIM_RXF_QUANTUM 1600

/* forward declaration */
struct rtw_hwsim_vif;
struct rtw_hwsim_rx_frame;
struct rtw_hwsim_medium;

/**
 * struct rtw_hwsim_data - hwsim virtual device description
 *
 * This structure describes a hwsim virtual device. It is derived from
 * &struct rtw_wiphy_data by placing it at the beginning of the structure so
 * that when the driver access the private wiphy context with rtw_wiphy_priv(),
 * it gets correct content.
 *
 * @super: the private wiphy context used by the driver
 * @medium: the medium (local/sock/loopback) to be used for tx/rx
 * @list: used to maintain a list of hwsim virtual devices
 * @dev: the platform device
 * @wiphy: the @wiphy of the device
 * @addresses: available addresses of the device
 * @bands: available bands of the device
 * @devobj: the device object used by the driver
 * @vifs: a list virtual interfaces of the device
 * @n_vifs: number of virtual interfaces of the device
 */
struct rtw_hwsim_data {
	struct rtw_wiphy_data super;

	int medium;

	unsigned int udp; //udp client connect to this ip address

	struct list_head list;
	struct device *dev;
	struct wiphy *wiphy;
	struct mac_address addresses[2];

	struct ieee80211_supported_band bands[NUM_NL80211_BANDS];

	struct dvobj_priv *devobj;

	struct rtw_hwsim_vif *vifs[1];
	int n_vifs;
};

/**
 * struct rtw_hwsim_medium_ops - hwsim medium operations
 *
 * @init: the function for initializing medium specific data
 * @deinit: the function for deinitializing medium specific data
 * @tx_handler: the function for handling tx requests
 * @medium_tx: the function used to transmit the frame on the pseudo medium
 * @pre_netif_rx: function called before the driver is going to report the
 * received frame
 */
struct rtw_hwsim_medium_ops {
	int (*init)(struct rtw_hwsim_medium *med);
	void (*deinit)(struct rtw_hwsim_medium *med);
	netdev_tx_t (*tx_handler)(struct rtw_hwsim_medium *med,
	                          struct sk_buff *skb);
	int (*medium_tx)(struct rtw_hwsim_medium *med, const void *tx_ctx,
	                 u8 *buf, size_t buflen);
	void (*pre_netif_rx)(struct rtw_hwsim_medium *med, struct sk_buff *skb);
};

/**
 * struct rtw_hwsim_medium - the general medium structure
 *
 * @medium: the medium, one of RTW_HWSIM_MEDIUM_XXX
 * @vif: the vif of the medium
 * @ops: operations for the medium
 * @priv: the private data of the medium
 */
struct rtw_hwsim_medium {
	int medium;
	struct rtw_hwsim_vif *vif;
	struct rtw_hwsim_medium_ops *ops;
	void *priv;
};

/**
 * struct rtw_hwsim_vif - describes a hwsim virtual interface
 *
 * This structure describes a virtual interface used by hwsim. It is derived
 * from &struct rtw_netdev_priv_indicator by placing it at the beginning of the
 * structure so that when the driver access the private net device context with
 * rtw_netdev_priv(), it gets correct content.
 *
 * @super: the private net device context used by the driver
 * @wdev: used for &net_device->ieee80211_ptr
 * @adapter_wdev: the wdev allocated by adapter
 * @ndev: the net device of the virtual interface
 * @address: the mac address of the virtual interface
 * @adapter: the adapter of the virtual interface
 * @medium: the medium of the vif
 */
struct rtw_hwsim_vif {
	struct rtw_netdev_priv_indicator super;

	struct wireless_dev wdev;
	struct wireless_dev *adapter_wdev;
	struct rtw_hwsim_data *data;
	struct net_device *ndev;
	struct mac_address address;
	struct _ADAPTER adapter;
	struct rtw_hwsim_medium medium;
	__be32 ifa_addr;
};

struct rtw_hwsim_tx_metadata {
	__le16 hdr_len;
	__le16 pkt_len;
	__le16 buf_len;
	__le16 mid;
	__le16 seq;
	u8 icv_len;
	u8 qos_en;
	u8 priority;
	u8 rate;
	u8 encrypt;
	u8 nr_frags;
} __packed __aligned(2);

struct rtw_hwsim_frame {
	struct rtw_hwsim_tx_metadata m;
	u8 buf[0];
} __packed __aligned(2);

#define rtw_hwsim_dump_frame(f, quote) {	  \
		RTW_INFO("%s(): %s(%d)\n", __func__, quote, \
		         le16_to_cpu(f->m.buf_len)); \
		RTW_INFO("----------------\n"); \
		RTW_INFO_DUMP("", f->buf, le16_to_cpu(f->m.buf_len)); \
	}

#define data_to_devobj(d) (d->devobj)
#define data_to_wiphy(d) (d->wiphy)
#define ndev_to_vif(n) (struct rtw_hwsim_vif *)(netdev_priv(n))
#define vif_to_adapter(i) (&(i)->adapter)
#define vif_to_data(i) (i->data)
#define vif_to_devobj(i) (vif_to_data(i)->devobj)
#define vif_to_netdev(i) (vif->ndev)
#define vif_to_wiphy(i) (data_to_wiphy(vif_to_data(i)))
#define vif_to_wdev(i) (&i->wdev)
#define adap_to_vif(a) ((struct rtw_hwsim_vif *)netdev_priv(a->pnetdev))
#define vif_to_adap(v) (&v->adapter)
#define vif_to_med(v) (&v->medium)
#define med_to_vif(m) (m->vif)

/* cfg80211.c */
struct rtw_hwsim_data *rtw_hwsim_cfg80211_alloc(struct device *dev, int index,
                                                int mon);
void rtw_hwsim_cfg80211_free(struct rtw_hwsim_data *data);
int rtw_hwsim_cfg80211_register(struct rtw_hwsim_data *data);
void rtw_hwsim_cfg80211_unregister(struct rtw_hwsim_data *data);

/* txrx.c */
int rtw_hwsim_medium_init(struct rtw_hwsim_vif *vif, int medium);
void rtw_hwsim_medium_deinit(struct rtw_hwsim_vif *vif);
netdev_tx_t rtw_hwsim_medium_tx_handler(struct rtw_hwsim_vif *vif,
                                        struct sk_buff *skb);
int rtw_hwsim_tweak_ip_addr(struct sk_buff *skb);
void rtw_hwsim_rx_translate(struct _ADAPTER *adapter, struct rtw_hwsim_frame *f,
                            union recv_frame *r);
void rtw_hwsim_build_metadata(struct rtw_hwsim_tx_metadata *m,
                              const struct xmit_frame *x, size_t buflen);
void rtw_hwsim_release_frame(struct rtw_hwsim_frame *f);
u32 rtw_hwsim_crc32(const u8 *in, size_t byte_num);
void rtw_hwsim_post_tx(struct _ADAPTER *adapter, struct xmit_frame *x);

/* netdev.c */
struct rtw_hwsim_vif *rtw_hwsim_interface_add(struct rtw_hwsim_data *data);
void rtw_hwsim_interface_del(struct rtw_hwsim_vif *vif);
int rtw_hwsim_inetaddr_notifier_call(struct notifier_block *nb,
                                     unsigned long action, void *data);
/* core.c */
struct dvobj_priv *rtw_hwsim_alloc_devobj(struct device *dev);
void rtw_hwsim_free_devobj(struct dvobj_priv *devobj);
int rtw_hwsim_init_adapter(struct rtw_hwsim_vif *vif);
void rtw_hwsim_deinit_adapter(struct _ADAPTER *padapter);

/* medium */
void rtw_hwsim_medium_loopback_ops(struct rtw_hwsim_medium *med);
void rtw_hwsim_medium_local_ops(struct rtw_hwsim_medium *med);
void rtw_hwsim_medium_sock_tcp_ops(struct rtw_hwsim_medium *med);
void rtw_hwsim_medium_sock_udp_ops(struct rtw_hwsim_medium *med);

#endif	/* __RTW_HWSIM_H_ */
