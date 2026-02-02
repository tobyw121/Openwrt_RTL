/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
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
 *
 *
 ******************************************************************************/
#define _RECV_OSDEP_C_

#include <drv_types.h>

int rtw_os_alloc_recvframe(_adapter *padapter, union recv_frame *precvframe, u8 *pdata, _pkt *pskb)
{
	int res = _SUCCESS;

	return res;
}

void rtw_os_free_recvframe(union recv_frame *precvframe)
{
	if (precvframe->u.hdr.pkt) {
		rtw_os_pkt_free(precvframe->u.hdr.pkt);
		precvframe->u.hdr.pkt = NULL;
	}
}

//init os related resource in struct recv_priv
int rtw_os_recv_resource_init(struct recv_priv *precvpriv, _adapter *padapter)
{	
	int	res=_SUCCESS;

	return res;
}

//alloc os related resource in union recv_frame
int rtw_os_recv_resource_alloc(_adapter *padapter, union recv_frame *precvframe)
{	
	int	res=_SUCCESS;
	
	precvframe->u.hdr.pkt = NULL;

	return res;

}

//free os related resource in union recv_frame
void rtw_os_recv_resource_free(struct recv_priv *precvpriv)
{
	sint i;
	union recv_frame *precvframe;
	precvframe = (union recv_frame*) precvpriv->precv_frame_buf;

	for (i = 0; i < NR_RECVFRAME; i++) {
		rtw_os_free_recvframe(precvframe);
		precvframe++;
	}
}


//alloc os related resource in struct recv_buf
int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf)
{
	int res=_SUCCESS;

#ifdef CONFIG_USB_HCI	
#ifdef CONFIG_USE_USB_BUFFER_ALLOC
	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;
#endif //CONFIG_USE_USB_BUFFER_ALLOC
	precvbuf->irp_pending = _FALSE;
        precvbuf->purb = rtw_usb_alloc_urb(0, GFP_KERNEL);
	if(precvbuf->purb == NULL){
		res = _FAIL;
	}

	precvbuf->pskb = NULL;

	precvbuf->pallocated_buf  = precvbuf->pbuf = NULL;

	precvbuf->pdata = precvbuf->phead = precvbuf->ptail = precvbuf->pend = NULL;

	precvbuf->transfer_len = 0;

	precvbuf->len = 0;
	
#ifdef CONFIG_USE_USB_BUFFER_ALLOC
	precvbuf->pallocated_buf = rtw_usb_buffer_alloc(pusbd, (size_t)precvbuf->alloc_sz, &precvbuf->dma_transfer_addr);
	precvbuf->pbuf = precvbuf->pallocated_buf;
	if(precvbuf->pallocated_buf == NULL)
		return _FAIL;

#endif //CONFIG_USE_USB_BUFFER_ALLOC
	
#endif
#ifdef CONFIG_SDIO_HCI
	precvbuf->pskb = NULL;

	precvbuf->pallocated_buf  = precvbuf->pbuf = NULL;

	precvbuf->pdata = precvbuf->phead = precvbuf->ptail = precvbuf->pend = NULL;

	precvbuf->len = 0;
#endif
	return res;
	
}

//free os related resource in struct recv_buf
int rtw_os_recvbuf_resource_free(_adapter *padapter, struct recv_buf *precvbuf)
{
	int ret = _SUCCESS;

#ifdef CONFIG_USB_HCI

#ifdef CONFIG_USE_USB_BUFFER_ALLOC

	struct dvobj_priv	*pdvobjpriv = &padapter->dvobjpriv;
	struct usb_device	*pusbd = pdvobjpriv->pusbdev;

	rtw_usb_buffer_free(pusbd, (size_t)precvbuf->alloc_sz, precvbuf->pallocated_buf, precvbuf->dma_transfer_addr);
	precvbuf->pallocated_buf =  NULL;
	precvbuf->dma_transfer_addr = 0;

#endif //CONFIG_USE_USB_BUFFER_ALLOC

	if(precvbuf->purb)
	{
                //rtw_usb_kill_urb(precvbuf->purb);
                rtw_usb_free_urb(precvbuf->purb);
	}
	
#endif //CONFIG_USB_HCI

	if(precvbuf->pskb)
		rtw_skb_free(precvbuf->pskb);
	
	return ret;

}

_pkt *rtw_os_alloc_msdu_pkt(union recv_frame *prframe, const u8 *da, const u8 *sa, u8 *msdu ,u16 msdu_len)
{
	u16	eth_type;
	u8	*ptr,offset;
	_pkt *sub_m = NULL;
	struct rx_pkt_attrib *pattrib = &prframe->u.hdr.attrib;
	_adapter *padapter = prframe->u.hdr.adapter;


	//Allocate a mbuff,  
	//sub_m =m_devget(pdata, nSubframe_Length+12, 12, padapter->pifp,NULL);
	sub_m = m_devget(pdata, msdu_len + ETH_HLEN, ETHER_ALIGN, padapter->pifp, NULL);

	if (sub_m) {
		ptr = mtod(sub_m, u8 *);
		offset = ETH_HLEN;
		/* convert hdr + possible LLC headers into Ethernet header */

		eth_type = RTW_GET_BE16(&ptr[offset+6]);

		if (sub_m->m_pkthdr.len >= ETH_HLEN + 8
			&& ((_rtw_memcmp(ptr + ETH_HLEN, rtw_rfc1042_header, SNAP_SIZE)
					&& eth_type != ETH_P_AARP && eth_type != ETH_P_IPX)
				|| _rtw_memcmp(ptr + ETH_HLEN, rtw_bridge_tunnel_header, SNAP_SIZE))
		) {
			/* remove RFC1042 or Bridge-Tunnel encapsulation and replace EtherType */
			offset += SNAP_SIZE;
			_rtw_memcpy(&ptr[offset - ETH_ALEN], sa, ETH_ALEN);
			offset -= ETH_ALEN;
			_rtw_memcpy(&ptr[offset - ETH_ALEN], da, ETH_ALEN);
			offset -= ETH_ALEN;
		} else {
			/* Leave Ethernet header part of hdr and full payload */
			u16 len;

			len = htons(sub_m->m_pkthdr.len - offset);
			_rtw_memcpy(&ptr[offset - 2], &len, 2);
			offset -= 2;
			_rtw_memcpy(&ptr[offset - ETH_ALEN], sa, ETH_ALEN);
			offset -= ETH_ALEN;
			_rtw_memcpy(&ptr[offset - ETH_ALEN], da, ETH_ALEN);
			offset -= ETH_ALEN;
		}

		m_adj(sub_m,offset);
	}

	return sub_m;
}

void rtw_os_recv_indicate_pkt(_adapter *padapter, _pkt *pkt, union recv_frame *rframe)
{
	struct recv_priv *precvpriv = &padapter->recvpriv;
	struct mlme_priv*pmlmepriv = &padapter->mlmepriv;

	/* Indicat the packets to upper layer */
	if (pkt) {
		if (MLME_IS_AP(padapter)) {
		 	struct sta_info *psta = NULL;
		 	struct sta_priv *pstapriv = &padapter->stapriv;
			_pkt *pskb2 = NULL;
			struct ethhdr *ehdr = mtod(pkt, struct ethhdr *);
			int bmcast = IS_MCAST(ehdr->h_dest);

			//RTW_INFO("bmcast=%d\n", bmcast);

			if (_rtw_memcmp(ehdr->h_dest, adapter_mac_addr(padapter), ETH_ALEN) == _FALSE) {
				//RTW_INFO("not ap psta=%p, addr=%pM\n", psta, ehdr->h_dest);

				if(bmcast)
				{
					psta = rtw_get_bcmc_stainfo(padapter);

					//pskb2 = m_copym(m, 0, M_COPYALL, M_DONTWAIT);
					pskb2 = m_copypacket(pkt, M_DONTWAIT);

				} else {
					psta = rtw_get_stainfo(pstapriv, ehdr->h_dest);
				}

				if(psta)
				{
					//RTW_INFO("directly forwarding to the rtw_xmit_entry\n");
					rtw_xmit_entry(pkt, padapter->pifp);

					if (bmcast && pskb2 != NULL)
						pkt = pskb2;
					else
						return;
				}
			}
			else// to APself
			{
				//RTW_INFO("to APSelf\n");
			}
		}

		if ( ((u32)(mtod(pkt, caddr_t) + 14) % 4) != 0)
			printf("%s()-%d: mtod(sub_m) = %p\n", __FUNCTION__, __LINE__, mtod(sub_m, caddr_t));

#ifdef CONFIG_RX_INDICATE_QUEUE
		IF_ENQUEUE(&precvpriv->rx_indicate_queue, pkt);
		if (_IF_QLEN(&precvpriv->rx_indicate_queue) <= 1) {
			taskqueue_enqueue(taskqueue_thread, &precvpriv->rx_indicate_tasklet);
		}
#else
		(*padapter->pifp->if_input)(padapter->pifp, pkt);	
#endif
	}
}

void rtw_handle_tkip_mic_err(_adapter *padapter, struct sta_info *sta, u8 bgroup)
{
#ifndef PLATFORM_FREEBSD	
    union iwreq_data wrqu;
    struct iw_michaelmicfailure    ev;
    struct mlme_priv*              pmlmepriv  = &padapter->mlmepriv;

    
    _rtw_memset( &ev, 0x00, sizeof( ev ) );
    if ( bgroup )
    {
        ev.flags |= IW_MICFAILURE_GROUP;
    }
    else
    {
        ev.flags |= IW_MICFAILURE_PAIRWISE;
    }
   
    ev.src_addr.sa_family = ARPHRD_ETHER;
    _rtw_memcpy( ev.src_addr.sa_data, &pmlmepriv->assoc_bssid[ 0 ], ETH_ALEN );

    _rtw_memset( &wrqu, 0x00, sizeof( wrqu ) );
    wrqu.data.length = sizeof( ev );

    wireless_send_event( padapter->pnetdev, IWEVMICHAELMICFAILURE, &wrqu, (char*) &ev );
#else
printf("%s,%d,not implemented\n",__FUNCTION__,__LINE__);
#endif    
}

#ifdef CONFIG_RX_INDICATE_QUEUE
void rtw_rx_indicate_tasklet(void *priv, int npending)
{
	_adapter		*padapter = (_adapter*)priv;
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	
	struct mbuf *m;
	for (;;) {
		IF_DEQUEUE(&precvpriv->rx_indicate_queue, m);
		if (m == NULL)
			break;
		(*padapter->pifp->if_input)(padapter->pifp, m);
	}

}
#endif	// CONFIG_RX_INDICATE_QUEUE

inline void rtw_rframe_set_os_pkt(union recv_frame *rframe)
{
	_pkt *m = rframe->u.hdr.pkt;

	m->m_data = rframe->u.hdr.rx_data;
	m->m_pkthdr.len = m->m_len = rframe->u.hdr.len;
}

int rtw_recv_indicatepkt(_adapter *padapter, union recv_frame *precv_frame)
{
	struct recv_priv *precvpriv;
	_queue *pfree_recv_queue;

	precvpriv = &(padapter->recvpriv);	
	pfree_recv_queue = &(precvpriv->free_recv_queue);	

	if (!precv_frame->u.hdr.pkt)
		goto _recv_indicatepkt_drop;

	rtw_os_recv_indicate_pkt(padapter, precv_frame->u.hdr.pkt, precv_frame);

_recv_indicatepkt_end:
	precv_frame->u.hdr.pkt = NULL;
	rtw_free_recvframe(precv_frame, pfree_recv_queue);
	return _SUCCESS;

_recv_indicatepkt_drop:
	rtw_free_recvframe(precv_frame, pfree_recv_queue);
	return _FAIL;
}

void rtw_os_read_port(_adapter *padapter, struct recv_buf *precvbuf)
{	
	struct recv_priv *precvpriv = &padapter->recvpriv;

#ifdef CONFIG_USB_HCI

	precvbuf->ref_cnt--;

	//free skb in recv_buf
	rtw_skb_free(precvbuf->pskb);

	precvbuf->pskb = NULL;

	if(precvbuf->irp_pending == _FALSE)
	{
		rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
	}	
		

#endif
#ifdef CONFIG_SDIO_HCI
		precvbuf->pskb = NULL;
#endif

}

