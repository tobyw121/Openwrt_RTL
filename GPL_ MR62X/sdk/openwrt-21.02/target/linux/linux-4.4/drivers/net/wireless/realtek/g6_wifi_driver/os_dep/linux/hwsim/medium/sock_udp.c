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

#include "../rtw_hwsim.h"

#include <linux/inet.h>         /* for in_aton() */
#include <linux/kfifo.h>

static u8 fake_icv[8] = {0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE};

struct rtw_hwsim_tx_frame {
	struct xmit_frame *x;
	struct rtw_hwsim_tx_metadata m;
	u8 *buf;
	u8 *icv;
};

struct medium_spec {
	struct socket *sock;
	struct task_struct *server_thread;
	struct rtw_hwsim_frame *rxf;
	size_t rxf_size;

	struct socket *client_sock;
	struct kmem_cache *frame_cache;
	spinlock_t frame_cache_lock;
	size_t txf_size;
	struct workqueue_struct *client_wq;
	struct work_struct client_work;
	DECLARE_KFIFO_PTR(tx_fifo, struct rtw_hwsim_tx_frame *);
};

#define med_to_mspec(m) ((struct medium_spec *)m->priv)

#define PORT_NUM 12345

static int create_bcst_udp_sock(struct socket **sock, u16 port)
{
	int so_bcst = 1;
	struct sockaddr_in s_addr;
	int err;

	err = sock_create_kern(&init_net, AF_INET, SOCK_DGRAM, 0, sock);
	if (err < 0) {
		RTW_ERR("%s(): sock_create_kern failed: %d\n", __func__, err);
		return err;
	}

	err = kernel_setsockopt(*sock, SOL_SOCKET, SO_BROADCAST,
	                        (char *)&so_bcst, sizeof(so_bcst));
	if (err < 0) {
		RTW_ERR("%s(): broadcast failed: %d\n", __func__, err);
		goto fail;
	}

	err = kernel_setsockopt(*sock, SOL_SOCKET, SO_REUSEADDR,
	                        (char *)&so_bcst, sizeof(so_bcst));
	if (err < 0) {
		RTW_ERR("%s(): set reuse addr failed: %d\n", __func__, err);
		goto fail;
	}

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = INADDR_ANY;

	err = kernel_bind(*sock, (struct sockaddr *)&s_addr, sizeof(s_addr));
	if (err < 0) {
		RTW_ERR("%s(): kernel_bind failed: %d\n", __func__, err);
		goto fail;
	}

	return 0;

fail:
	sock_release(*sock);
	return err;
}

static void release_bcst_udp_sock(struct socket *sock)
{
	kernel_sock_shutdown(sock, SHUT_RDWR);
	sock_release(sock);
}

static int create_bcst_udp_client_sock(struct socket **sock, u32 ip, u16 port)
{
	int err;
	int so_bcst = 1;
	struct sockaddr_in s_addr;

	err = sock_create_kern(&init_net, AF_INET, SOCK_DGRAM, 0, sock);
	if (err < 0) {
		RTW_ERR("%s(): kernel_accept failed: %d\n", __func__, err);
		return err;
	}

	err = kernel_setsockopt(*sock, SOL_SOCKET, SO_BROADCAST,
	                        (char *)&so_bcst, sizeof(so_bcst));

	if (err < 0) {
		RTW_ERR("%s(): set broadcast failed: %d\n", __func__, err);
		goto fail;
	}

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = htons(port);
	s_addr.sin_addr.s_addr = ip; //0xc0a801ff;//INADDR_BROADCAST;

	err = kernel_connect(*sock, (struct sockaddr *)&s_addr,
	                     sizeof(s_addr), 0);
	if (err < 0) {
		RTW_ERR("%s(): kernel_connect failed: %d\n", __func__, err);
		goto fail;
	}

	return 0;

fail:
	sock_release(*sock);
	return err;
}

static int process_rx_frame(struct rtw_hwsim_vif *vif,
                            struct rtw_hwsim_frame *f)
{
	struct _ADAPTER *adapter;
	union recv_frame *r;
	u8 *buf, *data;
	size_t buflen;
	struct wireless_dev *wdev;

	adapter = vif_to_adap(vif);

	buf = f->buf;
	buflen = le16_to_cpu(f->m.buf_len);

	/* append fake icv, m.buflen contains icv length */
	memcpy(buf + buflen, fake_icv, f->m.icv_len);
	buflen += f->m.icv_len;
	f->m.buf_len = cpu_to_le16(buflen);

	/* append crc32 for monitor mode */
	wdev = vif_to_wdev(vif);
	if (wdev->iftype == NL80211_IFTYPE_MONITOR) {
		u32 crc;

		crc = rtw_hwsim_crc32(buf, buflen);
		_rtw_memcpy(buf + buflen, &crc, sizeof(crc));
		buflen += sizeof(crc);
		f->m.buf_len = cpu_to_le16(buflen);
	}

	r = rtw_alloc_recvframe(&adapter->recvpriv.free_recv_queue);
	if (!r) {
		RTW_ERR("%s(): alloc memory for recvframe failed\n", __func__);
		return -1;
	}

	rtw_init_recvframe(r, &adapter->recvpriv);

	rtw_hwsim_rx_translate(adapter, f, r);

	if (rtw_os_alloc_recvframe(adapter, r, buf, NULL) == _FAIL) {
		RTW_ERR("%s(): failed to allocate recv frame\n", __func__);
		rtw_free_recvframe(r, &adapter->recvpriv.free_recv_queue);
		return -1;
	}

	recvframe_put(r, buflen);

	pre_recv_entry(r, NULL);

	return 0;
}

static int server_threadfn(void *data)
{
	int err;
	struct rtw_hwsim_medium *med;
	struct medium_spec *mspec;
	struct socket *sock;
	struct rtw_hwsim_frame *f;
	struct kvec vec[3];
	struct msghdr msg;
	ssize_t msglen;

	med = (struct rtw_hwsim_medium *)data;
	mspec = med_to_mspec(med);
	sock = mspec->sock;
	f = mspec->rxf;

	allow_signal(SIGINT);

	while (unlikely(!kthread_should_stop())) {
		memset(f, 0, mspec->rxf_size);
		memset(&vec, 0, sizeof(vec));
		memset(&msg, 0, sizeof(msg));

		/* metadata */
		vec[0].iov_base = &f->m;
		vec[0].iov_len = sizeof(f->m);

		/* wlan frame */
		vec[1].iov_base = &f->buf;
		vec[1].iov_len = RTW_HWSIM_RXF_QUANTUM;

		/* icv, skip */
		vec[2].iov_base = NULL;
		vec[2].iov_len = 0;

		err = kernel_recvmsg(sock, &msg, vec, 3, mspec->rxf_size, 0);
		if (err < 0) {
			RTW_ERR("%s(): recvmsg failed: %d\n", __func__, err);
			if (kthread_should_stop())
				break;
			else
				continue;
		}

		process_rx_frame(med_to_vif(med), f);
	}

	return 0;
}

static void client_worker(struct work_struct *work)
{
	int err;
	struct medium_spec *mspec;
	struct socket *sock;
	struct rtw_hwsim_tx_frame *txf;
	struct kvec vec[3];
	struct msghdr msg;
	size_t buflen;

	mspec = container_of(work, struct medium_spec, client_work);
	sock = mspec->client_sock;

	/* send each tx frame in the tx fifo*/
	while (kfifo_get(&mspec->tx_fifo, &txf)) {
		memset(&vec, 0, sizeof(vec));
		memset(&msg, 0, sizeof(msg));

		/* metadata */
		vec[0].iov_base = &txf->m;
		vec[0].iov_len = sizeof(txf->m);

		/* wlan frame */
		vec[1].iov_base = txf->buf;
		vec[1].iov_len = le16_to_cpu(txf->m.buf_len);

		/* icv */
		vec[2].iov_base = txf->icv;
		vec[2].iov_len = txf->m.icv_len;

		buflen = vec[0].iov_len + vec[1].iov_len + vec[2].iov_len;

		err = kernel_sendmsg(sock, &msg, vec, 3, buflen);

		rtw_hwsim_post_tx(txf->x->padapter, txf->x);

		spin_lock_bh(&mspec->frame_cache_lock);
		kmem_cache_free(mspec->frame_cache, txf);
		spin_unlock_bh(&mspec->frame_cache_lock);

		if (err < 0 || err != (int)buflen) {
			RTW_ERR("%s(): sendmsg failed, err=%d, buflen=%zd\n",
			        __func__, err, buflen);
			continue;
		}
	}
}

static int init(struct rtw_hwsim_medium *med)
{
	struct medium_spec *mspec;
	int err;

	RTW_INFO("%s(): initializing sock medium\n", __func__);

	mspec = rtw_malloc(sizeof(*mspec));
	if (!mspec)
		goto fail;
	med->priv = mspec;

	mspec->rxf_size = sizeof(struct rtw_hwsim_frame) +
		RTW_HWSIM_RXF_QUANTUM;
	mspec->rxf = rtw_malloc(mspec->rxf_size);
	if (!mspec->rxf)
		goto fail_alloc_rxf;

	if (create_bcst_udp_sock(&mspec->sock, PORT_NUM) < 0)
		goto fail_create_sock;

	if (create_bcst_udp_client_sock(&mspec->client_sock, med->vif->data->udp, PORT_NUM) < 0)
		goto fail_create_client_sock;

	/*
	 * Note: it is ok to use same name as it may merge the slab and use
	 * refcount to manage it.
	 */
	mspec->txf_size = sizeof(struct rtw_hwsim_tx_frame);
	mspec->frame_cache = kmem_cache_create("rtw_hwsim-udp-sock",
	                                       mspec->txf_size, 0,
	                                       SLAB_HWCACHE_ALIGN, NULL);
	if (!mspec->frame_cache) {
		RTW_ERR("%s(): kmem_cache_create failed\n", __func__);
		goto fail_create_cache;
	}

	spin_lock_init(&mspec->frame_cache_lock);

	err = kfifo_alloc(&mspec->tx_fifo, 256, GFP_ATOMIC);
	if (err < 0) {
		RTW_ERR("%s(): alloc tx fifo failed\n", __func__);
		goto fail_alloc_tx_fifo;
	}

	mspec->client_wq = create_singlethread_workqueue("rtw_hwsim_cli_wq");
	if (!mspec->client_wq) {
		RTW_ERR("%s(): failed to start client wq\n", __func__);
		goto fail_create_client_wq;
	}

	mspec->server_thread = kthread_run(server_threadfn, med,
	                                   "hwsim-server-thread");
	if (IS_ERR(mspec->server_thread)) {
		RTW_ERR("%s(): failed to start server thread\n", __func__);
		goto fail_create_server_thread;
	}

	INIT_WORK(&mspec->client_work, client_worker);

	return 0;

fail_create_server_thread:
	destroy_workqueue(mspec->client_wq);
fail_create_client_wq:
	kfifo_free(&mspec->tx_fifo);
fail_alloc_tx_fifo:
	kmem_cache_free(mspec->frame_cache, mspec->rxf);
fail_create_cache:
	release_bcst_udp_sock(mspec->client_sock);
fail_create_client_sock:
	release_bcst_udp_sock(mspec->sock);
fail_create_sock:
	rtw_mfree(mspec->rxf, mspec->rxf_size);
fail_alloc_rxf:
	rtw_mfree(mspec, sizeof(*mspec));
fail:
	return -1;
}

static void deinit(struct rtw_hwsim_medium *med)
{
	struct medium_spec *mspec;
	struct rtw_hwsim_frame *f;
	struct rtw_hwsim_tx_frame *txf;

	mspec = med_to_mspec(med);

	/* tx cleanup */
	cancel_work_sync(&mspec->client_work);
	flush_workqueue(mspec->client_wq);
	destroy_workqueue(mspec->client_wq);
	release_bcst_udp_sock(mspec->client_sock);

	spin_lock_bh(&mspec->frame_cache_lock);
	while (kfifo_get(&mspec->tx_fifo, &txf))
		kmem_cache_free(mspec->frame_cache, txf);
	spin_unlock_bh(&mspec->frame_cache_lock);
	kfifo_free(&mspec->tx_fifo);

	/* make kernel_recvmsg() returns */
	send_sig(SIGINT, mspec->server_thread, 0);
	kthread_stop(mspec->server_thread);
	release_bcst_udp_sock(mspec->sock);

	rtw_mfree(mspec->rxf, mspec->rxf_size);
	/* has to be done after all cache freed */
	kmem_cache_destroy(mspec->frame_cache);
	rtw_mfree(mspec, sizeof(*mspec));
	RTW_INFO("%s(): sock medium deinitialized\n", __func__);
}

static netdev_tx_t tx_handler(struct rtw_hwsim_medium *med, struct sk_buff *skb)
{
	netdev_tx_t ret;
	struct rtw_hwsim_vif *vif;
	struct security_priv *sec;

	vif = med_to_vif(med);
	sec = &vif->adapter.securitypriv;

	/*
	 * Note: We assume that when socket medium is used, the entities are
	 * spread on different machines and therefore there's no need to tweak
	 * the IP address for local loopback to work
	 */

	sec->hw_decrypted = _TRUE;

	ret = rtw_xmit_entry(skb, vif_to_netdev(vif));

	return ret;
}

static int medium_tx(struct rtw_hwsim_medium *med, const void *tx_ctx,
                     u8 *buf, size_t buflen)
{
	int err;
	struct xmit_frame *x;
	struct medium_spec *mspec;
	struct xmit_buf *xbuf;
	struct rtw_hwsim_tx_frame *txf;

	x = (struct xmit_frame *)tx_ctx;
	mspec = med_to_mspec(med);
	xbuf = x->pxmitbuf;

	/*
	 * Set done before we do actual tx anyway.
	 */
	rtw_sctx_done(&xbuf->sctx);

	spin_lock_bh(&mspec->frame_cache_lock);
	txf = kmem_cache_alloc(mspec->frame_cache, GFP_ATOMIC);
	spin_unlock_bh(&mspec->frame_cache_lock);
	if (!txf) {
		RTW_ERR("%s(): kmem_cache_alloc failed\n", __func__);
		return -ENOMEM;
	}

	txf->x = x;
	rtw_hwsim_build_metadata(&txf->m, x, buflen);
	txf->buf = buf;
	txf->icv = (txf->m.icv_len) ? fake_icv : NULL;

	err = kfifo_put(&mspec->tx_fifo, txf);
	if (!err) {
		RTW_ERR("%s(): tx fifo full\n", __func__);
		goto fail;
	}

	/*
	 * If the work is already queued (queue_work() returns 0), we're fine,
	 * as all frames in the tx fifo will be processed by the worker.  If
	 * the work is processing, i.e. in client_worker(), queue_work() does
	 * not fail.
	 */
	queue_work(mspec->client_wq, &mspec->client_work);

	return 0;

fail:
	kmem_cache_free(mspec->frame_cache, txf);
	rtw_hwsim_post_tx(x->padapter, x);
	return err;
}

static void pre_netif_rx(struct rtw_hwsim_medium *med, struct sk_buff *skb)
{
	/* to avoid L3 checsum validation */
	skb->ip_summed = CHECKSUM_UNNECESSARY;
}

static struct rtw_hwsim_medium_ops ops = {
	.init = init,
	.deinit = deinit,
	.tx_handler = tx_handler,
	.medium_tx = medium_tx,
	.pre_netif_rx = pre_netif_rx,
};

void rtw_hwsim_medium_sock_udp_ops(struct rtw_hwsim_medium *med)
{
	med->ops = &ops;
}
