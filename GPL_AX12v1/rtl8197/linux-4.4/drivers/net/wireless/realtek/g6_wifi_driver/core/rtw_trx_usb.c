/******************************************************************************
 *
 * Copyright(c) 2019 Realtek Corporation.
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
 *****************************************************************************/
#define _RTW_TRX_USB_C_
#include <drv_types.h>		/* struct dvobj_priv and etc. */


/********************************xmit section*******************************/
s32 usb_dump_xframe(_adapter *padapter, struct xmit_frame *pxmitframe);

#ifdef CONFIG_USB_TX_AGGREGATION
#define IDEA_CONDITION 1	/* check all packets before enqueue */
static s32 usb_xmitframe_process(_adapter *padapter,
		struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{	
	return _TRUE;
}

#else /* CONFIG_USB_TX_AGGREGATION */

static s32 usb_xmitframe_process(_adapter *padapter,
		struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	return _TRUE;

}
#endif


static void usb_xmit_tasklet(_adapter *padapter)
{
#ifdef CONFIG_TX_AMSDU_SW_MODE
	core_tx_amsdu_tasklet(padapter);
#endif
}

s32 usb_init_xmit_priv(_adapter *adapter)
{
	s32 ret = _SUCCESS;
	struct xmit_priv *pxmitpriv = &adapter->xmitpriv;
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);

	rtw_tasklet_init(&pxmitpriv->xmit_tasklet,
		     (void(*)(unsigned long))usb_xmit_tasklet,
		     (unsigned long)adapter);

	return _SUCCESS;
}

void usb_free_xmit_priv(_adapter *adapter)
{
}

s32 usb_dump_xframe(_adapter *padapter, struct xmit_frame *pxmitframe)
{
	return _SUCCESS;
}


static s32 usb_xmit_direct(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	return _SUCCESS;
}

static s32 usb_data_xmit(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	return _TRUE;
}

s32 usb_mgnt_xmit(_adapter *adapter, struct xmit_frame *pmgntframe)
{
	return usb_dump_xframe(adapter, pmgntframe);
}

#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
/*
 * Description
 *	Transmit xmitbuf to hardware tx fifo
 *
 * Return
 *	_SUCCESS	ok
 *	_FAIL		something error
 */
s32 usb_xmit_buf_handler(_adapter *adapter)
{
	return _SUCCESS;
}
#endif /* CONFIG_XMIT_THREAD_MODE */

s32 usb_xmitframe_enqueue(_adapter *adapter, struct xmit_frame *pxmitframe)
{
	return _SUCCESS;
}

/******************************** recv section*******************************/
#ifdef CONFIG_PREALLOC_RECV_SKB
static void _usb_prealloc_recv_skb(_adapter *padapter)
{
	int i;
	SIZE_PTR tmpaddr = 0;
	SIZE_PTR alignment = 0;
	struct sk_buff *pskb = NULL;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_priv *precvpriv = &padapter->recvpriv;
	u8 recvbuf_nr = GET_HAL_RECVBUF_NR(dvobj);
	u16 recvbuf_sz = GET_HAL_RECVBUF_SZ(dvobj);

	RTW_INFO("NR_PREALLOC_RECV_SKB: %d\n", NR_PREALLOC_RECV_SKB);
	#ifdef CONFIG_FIX_NR_BULKIN_BUFFER
	RTW_INFO("Enable CONFIG_FIX_NR_BULKIN_BUFFER\n");
	#endif

	for (i = 0; i < NR_PREALLOC_RECV_SKB; i++) {
		#ifdef CONFIG_PREALLOC_RX_SKB_BUFFER
		pskb = rtw_alloc_skb_premem(recvbuf_sz);
		#else
		pskb = rtw_skb_alloc(recvbuf_sz + RECVBUFF_ALIGN_SZ);
		#endif /* CONFIG_PREALLOC_RX_SKB_BUFFER */

		if (pskb) {
			#ifdef PLATFORM_FREEBSD
			pskb->dev = padapter->pifp;
			#else
			pskb->dev = padapter->pnetdev;
			#endif /* PLATFORM_FREEBSD */

			#ifndef CONFIG_PREALLOC_RX_SKB_BUFFER
			tmpaddr = (SIZE_PTR)pskb->data;
			alignment = tmpaddr & (RECVBUFF_ALIGN_SZ - 1);
			skb_reserve(pskb, (RECVBUFF_ALIGN_SZ - alignment));
			#endif
			skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);
		}
	}
}
#endif/*CONFIG_PREALLOC_RECV_SKB*/
int usb_init_recv_priv(_adapter *padapter)
{
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_priv *precvpriv = &padapter->recvpriv;
	int	i, res = _SUCCESS;
	struct recv_buf *precvbuf;
	u8 recvbuf_nr = GET_HAL_RECVBUF_NR(dvobj);
	u16 recvbuf_sz = GET_HAL_RECVBUF_SZ(dvobj);
	/*GEORGIA_TODO_FIXIT*/
	u16 ini_in_buf_sz = 0; //= GET_HAL_INTBUF_SZ(dvobj);

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	#ifdef PLATFORM_LINUX
	precvpriv->int_in_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (precvpriv->int_in_urb == NULL) {
		res = _FAIL;
		RTW_INFO("alloc_urb for interrupt in endpoint fail !!!!\n");
		goto exit;
	}
	#endif /* PLATFORM_LINUX */
	precvpriv->int_in_buf = rtw_zmalloc(ini_in_buf_sz);
	if (precvpriv->int_in_buf == NULL) {
		res = _FAIL;
		RTW_INFO("alloc_mem for interrupt in endpoint fail !!!!\n");
		goto exit;
	}
#endif /* CONFIG_USB_INTERRUPT_IN_PIPE */

	/* init recv_buf */
	_rtw_init_queue(&precvpriv->free_recv_buf_queue);
	_rtw_init_queue(&precvpriv->recv_buf_pending_queue);
#ifndef CONFIG_USE_USB_BUFFER_ALLOC_RX
	/* this is used only when RX_IOBUF is sk_buff */
	skb_queue_head_init(&precvpriv->free_recv_skb_queue);
#endif

	RTW_INFO("NR_RECVBUFF: %d\n", recvbuf_nr);
	RTW_INFO("MAX_RECVBUF_SZ: %d\n", recvbuf_sz);
	precvpriv->pallocated_recv_buf = rtw_zmalloc(recvbuf_nr * sizeof(struct recv_buf) + 4);
	if (precvpriv->pallocated_recv_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	precvpriv->precv_buf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(precvpriv->pallocated_recv_buf), 4);

	precvbuf = (struct recv_buf *)precvpriv->precv_buf;

	for (i = 0; i < recvbuf_nr ; i++) {
		_rtw_init_listhead(&precvbuf->list);

		_rtw_spinlock_init(&precvbuf->recvbuf_lock);

		precvbuf->alloc_sz = recvbuf_sz;

		res = rtw_os_recvbuf_resource_alloc(padapter, precvbuf);
		if (res == _FAIL)
			break;

		precvbuf->ref_cnt = 0;
		precvbuf->adapter = padapter;

		/* rtw_list_insert_tail(&precvbuf->list, &(precvpriv->free_recv_buf_queue.queue)); */

		precvbuf++;
	}

	precvpriv->free_recv_buf_queue_cnt = recvbuf_nr;

#if defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD)
	skb_queue_head_init(&precvpriv->rx_skb_queue);

	#ifdef CONFIG_RX_INDICATE_QUEUE
	_rtw_memset(&precvpriv->rx_indicate_queue, 0, sizeof(struct ifqueue));
	mtx_init(&precvpriv->rx_indicate_queue.ifq_mtx, "rx_indicate_queue", NULL, MTX_DEF);
	#endif /* CONFIG_RX_INDICATE_QUEUE */

	#ifdef CONFIG_PREALLOC_RECV_SKB
	_usb_prealloc_recv_skb(padapter);
	#endif /* CONFIG_PREALLOC_RECV_SKB */

#endif /* defined(PLATFORM_LINUX) || defined(PLATFORM_FREEBSD) */

exit:

	return res;
}

void usb_free_recv_priv(_adapter *padapter)
{
	int i;
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
	struct recv_buf *precvbuf;
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	u8 recvbuf_nr = GET_HAL_RECVBUF_NR(dvobj);
	/*GEORGIA_TODO_FIXIT*/
	u16 ini_in_buf_sz = 0; //= GET_HAL_INTBUF_SZ(dvobj);

	precvbuf = (struct recv_buf *)precvpriv->precv_buf;

	for (i = 0; i < recvbuf_nr ; i++) {
		rtw_os_recvbuf_resource_free(padapter, precvbuf);
		precvbuf++;
	}

	if (precvpriv->pallocated_recv_buf)
		rtw_mfree(precvpriv->pallocated_recv_buf, recvbuf_nr * sizeof(struct recv_buf) + 4);

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	#ifdef PLATFORM_LINUX
	if (precvpriv->int_in_urb)
		usb_free_urb(precvpriv->int_in_urb);
	#endif
	if (precvpriv->int_in_buf)
		rtw_mfree(precvpriv->int_in_buf, ini_in_buf_sz);
#endif /* CONFIG_USB_INTERRUPT_IN_PIPE */

#ifdef PLATFORM_LINUX
	if (skb_queue_len(&precvpriv->rx_skb_queue))
		RTW_WARN("rx_skb_queue not empty\n");

	rtw_skb_queue_purge(&precvpriv->rx_skb_queue);

	if (skb_queue_len(&precvpriv->free_recv_skb_queue))
		RTW_WARN("free_recv_skb_queue not empty, %d\n", skb_queue_len(&precvpriv->free_recv_skb_queue));

	#if !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX)
	#if defined(CONFIG_PREALLOC_RECV_SKB) && defined(CONFIG_PREALLOC_RX_SKB_BUFFER)
	{
		struct sk_buff *skb;

		while ((skb = skb_dequeue(&precvpriv->free_recv_skb_queue)) != NULL) {
			if (rtw_free_skb_premem(skb) != 0)
				rtw_skb_free(skb);
		}
	}
	#else
	rtw_skb_queue_purge(&precvpriv->free_recv_skb_queue);
	#endif /* defined(CONFIG_PREALLOC_RX_SKB_BUFFER) && defined(CONFIG_PREALLOC_RECV_SKB) */
	#endif /* !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX) */
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	struct sk_buff  *pskb;
	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue)))
		rtw_skb_free(pskb);

	#if !defined(CONFIG_USE_USB_BUFFER_ALLOC_RX)
	rtw_skb_queue_purge(&precvpriv->free_recv_skb_queue);
	#endif

	#ifdef CONFIG_RX_INDICATE_QUEUE
	struct mbuf *m;
	for (;;) {
		IF_DEQUEUE(&precvpriv->rx_indicate_queue, m);
		if (m == NULL)
			break;
		rtw_skb_free(m);
	}
	mtx_destroy(&precvpriv->rx_indicate_queue.ifq_mtx);
	#endif /* CONFIG_RX_INDICATE_QUEUE */

#endif /* PLATFORM_FREEBSD */
}

struct rtw_intf_ops usb_ops = {
	.init_xmit_priv = usb_init_xmit_priv,
	.free_xmit_priv = usb_free_xmit_priv,
	.data_xmit	= usb_data_xmit,
	.xmitframe_enqueue = usb_xmitframe_enqueue,
	.mgnt_xmit	= usb_mgnt_xmit,
	#if 0 /*def CONFIG_XMIT_THREAD_MODE*/
	.xmit_buf_handler = usb_xmit_buf_handler,
	#endif

	.init_recv_priv = usb_init_recv_priv,
	.free_recv_priv = usb_free_recv_priv,

};

