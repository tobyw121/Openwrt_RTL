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
#define _XMIT_OSDEP_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#include <if_ether.h>
#include <ip.h>
#include <rtw_byteorder.h>
#include <wifi.h>
#include <mlme_osdep.h>
#include <xmit_osdep.h>
#include <osdep_intf.h>
#include <circ_buf.h>
#include <sys/mbuf.h>

uint rtw_remainder_len(struct pkt_file *pfile)
{
	return (pfile->buf_len - ((SIZE_PTR)(pfile->cur_addr) - (SIZE_PTR)(pfile->buf_start)));
}

void _rtw_open_pktfile (_pkt *pktptr, struct pkt_file *pfile)
{
	pfile->pkt = pktptr;
	pfile->cur_addr = pfile->buf_start=mtod(pktptr, u8 *);// = pktptr->data;
    pfile->pkt_len=pktptr->m_pkthdr.len;
    pfile->buf_len= pktptr->m_len;//pktptr->m_next->m_len;

	pfile->cur_buffer =pktptr ;
	
}

uint _rtw_pktfile_read (struct pkt_file *pfile, u8 *rmem, uint rlen)
{	
	uint	len = 0,t_len= 0;
	_buffer	*next_buffer;
	while(1)
	{

	       	len =  rtw_remainder_len(pfile);
	       	len = (rlen > len)? len: rlen;
		  	if(len){
		  		if(rmem){
	  					
	  				//	m_copydata(pfile->cur_buffer,0,len,rmem);
	  				_rtw_memcpy(rmem,pfile->cur_addr,len);
	  					{
	  						uint i;
	  						for(i=0;i<len;i=i+8)
	  						;//	printf("%s(),%d  i=%d  rmem=:%x:%x:%x:%x:%x:%x:%x:%x\n",__FUNCTION__,__LINE__,i,rmem[i],rmem[i+1],rmem[i+2],rmem[i+3],rmem[i+4],rmem[i+5],rmem[i+6],rmem[i+7]);
	  					}
				}
		  	}
	  		t_len += len;
			rlen -= len;
			rmem += len;
			pfile->cur_addr += len;
			pfile->pkt_len -= len;

			if (rlen)
			{
				next_buffer=pfile->cur_buffer->m_next;
				if (next_buffer == NULL)
					break;
			}
			else
				break;
		    
		    pfile->cur_buffer = next_buffer;
			pfile->buf_start=mtod(pfile->cur_buffer, u8 *);
			pfile->cur_addr = pfile->buf_start;
			pfile->buf_len= pfile->cur_buffer->m_len;
	} // end of while

	return t_len;	
}

sint rtw_endofpktfile(struct pkt_file *pfile)
{

	if (pfile->pkt_len == 0) {
		return _TRUE;
	}


	return _FALSE;
}

void rtw_set_tx_chksum_offload(_pkt *pkt, struct pkt_attrib *pattrib)
{

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	struct sk_buff *skb = (struct sk_buff *)pkt;
	pattrib->hw_tcp_csum = 0;
	
	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		if (skb_shinfo(skb)->nr_frags == 0)
		{	
                        const struct iphdr *ip = ip_hdr(skb);
                        if (ip->protocol == IPPROTO_TCP) {
                                // TCP checksum offload by HW
                                RTW_INFO("CHECKSUM_PARTIAL TCP\n");
                                pattrib->hw_tcp_csum = 1;
                                //skb_checksum_help(skb);
                        } else if (ip->protocol == IPPROTO_UDP) {
                                //RTW_INFO("CHECKSUM_PARTIAL UDP\n");
#if 1                       
                                skb_checksum_help(skb);
#else
                                // Set UDP checksum = 0 to skip checksum check
                                struct udphdr *udp = skb_transport_header(skb);
                                udp->check = 0;
#endif
                        } else {
				RTW_INFO("%s-%d TCP CSUM offload Error!!\n", __FUNCTION__, __LINE__);
                                WARN_ON(1);     /* we need a WARN() */
			    }
		}
		else { // IP fragmentation case
			RTW_INFO("%s-%d nr_frags != 0, using skb_checksum_help(skb);!!\n", __FUNCTION__, __LINE__);
                	skb_checksum_help(skb);
		}		
	}
#endif	
	
}

int rtw_os_xmit_resource_alloc(_adapter *padapter, struct xmit_buf *pxmitbuf,u32 alloc_sz, u8 flag)
{
	if (alloc_sz > 0) {
		pxmitbuf->pallocated_buf = rtw_zmalloc(alloc_sz);
		if (pxmitbuf->pallocated_buf == NULL)
		{
			return _FAIL;
		}

		pxmitbuf->pbuf = (u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(pxmitbuf->pallocated_buf), XMITBUF_ALIGN_SZ);
	}

	if (flag) {
#ifdef CONFIG_USB_HCI
		int i;

	       for(i=0; i<8; i++)
	      	{
			pxmitbuf->pxmit_urb[i] = rtw_usb_alloc_urb(0, GFP_KERNEL);
	             	if(pxmitbuf->pxmit_urb[i] == NULL) 
	             	{
	             		RTW_INFO("pxmitbuf->pxmit_urb[i]==NULL");
		        	return _FAIL;	 
	             	}
	      	}
#endif
	}

	return _SUCCESS;	
}

void rtw_os_xmit_resource_free(_adapter *padapter, struct xmit_buf *pxmitbuf,u32 free_sz, u8 flag)
{
	if (flag) {
#ifdef CONFIG_USB_HCI
		int i;

		for(i=0; i<8; i++)
		{
			if(pxmitbuf->pxmit_urb[i])
			{
	                        //rtw_usb_kill_urb(pxmitbuf->pxmit_urb[i]);
	                        rtw_usb_free_urb(pxmitbuf->pxmit_urb[i]);
			}
		}
#endif
	}

	if (free_sz > 0) {
		if(pxmitbuf->pallocated_buf)
			rtw_mfree(pxmitbuf->pallocated_buf, free_sz);
	}
}

void rtw_os_pkt_complete(_adapter *padapter, _pkt *pkt)
{


//	if (netif_queue_stopped(padapter->pnetdev))
//		netif_wake_queue(padapter->pnetdev);

	/*
	 * if there's a BPF listener, bounce a copy
	 * of this frame to him:
	 */
	BPF_MTAP(padapter->pifp, pkt);

	rtw_os_pkt_free(pkt);
}

void rtw_os_xmit_complete(_adapter *padapter, struct xmit_frame *pxframe)
{
	if(pxframe->pkt)
		rtw_os_pkt_complete(padapter, pxframe->pkt);

	pxframe->pkt = NULL;
}

void rtw_os_xmit_schedule(_adapter *padapter)
{
	_irqL  irqL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	_enter_critical_bh(&pxmitpriv->lock, &irqL);
	
	if(rtw_txframes_pending(padapter))	
	{
		taskqueue_enqueue_fast(taskqueue_fast, &pxmitpriv->xmit_tasklet);
	}

	_exit_critical_bh(&pxmitpriv->lock, &irqL);
}


int rtw_xmit_entry(_pkt *pkt, _nic_hdl pnetdev)
{
	_adapter *padapter = (_adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	s32 res = 0;



	if (rtw_if_up(padapter) == _FALSE) {
		#ifdef DBG_TX_DROP_FRAME
		RTW_INFO("DBG_TX_DROP_FRAME %s if_up fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	res = rtw_xmit(padapter, &pkt);
	if (res < 0) {
		#ifdef DBG_TX_DROP_FRAME
		RTW_INFO("DBG_TX_DROP_FRAME %s rtw_xmit fail\n", __FUNCTION__);
		#endif
		goto drop_packet;
	}

	goto exit;

drop_packet:
	pxmitpriv->tx_drop++;
	rtw_os_pkt_complete(padapter,pkt);
exit:


	return 0;
}

void	rtw_xmit_entry_wrap (struct ifnet * pifp)
{

	_adapter *padapter;
	struct mbuf		*m_head;
	int			queued;

	padapter = pifp->if_softc;

	mtx_lock(&padapter->xmitpriv.lock);

	if ((pifp->if_drv_flags & (IFF_DRV_RUNNING | IFF_DRV_OACTIVE)) !=
	    IFF_DRV_RUNNING ) {
	    
		mtx_unlock(&padapter->xmitpriv.lock);
		return;
	}

	for (queued = 0; !IFQ_DRV_IS_EMPTY(&pifp->if_snd) ;) {
		IFQ_DRV_DEQUEUE(&pifp->if_snd, m_head);
		if (m_head == NULL)
			break;

		if (rtw_xmit_entry(m_head, pifp) != 0) {
			if (m_head == NULL)
				break;
			IFQ_DRV_PREPEND(&pifp->if_snd, m_head);
			pifp->if_drv_flags |= IFF_DRV_OACTIVE;
			break;
		}

		queued++;
	}

	mtx_unlock(&padapter->xmitpriv.lock);

}


