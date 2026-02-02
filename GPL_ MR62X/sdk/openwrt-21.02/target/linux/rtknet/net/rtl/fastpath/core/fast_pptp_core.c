#include "linux/compiler.h"
#include "linux/ip.h"
#include "linux/jiffies.h"
#include "linux/mutex.h"
#include "linux/printk.h"
#include "linux/rtnetlink.h"
#include "linux/seq_file.h"
#include "linux/skbuff.h"
#include "linux/unaligned/le_struct.h"
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/kernel_stat.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
//#include <linux/brlock.h>
#include <linux/net.h>
#include <linux/socket.h>
#include <linux/version.h>

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/string.h>
#include <net/ip.h>
#include <net/protocol.h>
#include <net/route.h>
#include <net/sock.h>
#include <net/arp.h>
#include <net/raw.h>
#include <net/checksum.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netlink.h>

#include <linux/in.h>
#include <linux/netfilter/nf_conntrack_proto_gre.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
#include <uapi/linux/netfilter/nf_conntrack_common.h>
#endif
#include <linux/netfilter_ipv4/ip_conntrack_pptp.h>
#include <linux/if_tunnel.h>
#include <linux/if_ether.h>
#include <linux/tcp.h>
#include <net/rtl/rtl_types.h>
#include <net/rtl/fastpath/fastpath_core.h>
#include <net/rtl/rtl865x_netif.h>
#include <net/rtl/rtl_nic.h>

#if defined(FAST_PPTP)
#include <linux/inetdevice.h>
#include <linux/ppp_channel.h>
#include <linux/if_pppox.h>
#include <linux/if_bridge.h>
#include <linux/if_vlan.h>
#include <net/rtl/features/rtl_features.h>
#include <net/rtl/features/rtl_ps_hooks.h>

static struct pptp_acc_info pptpAccInfo;
int pptp_tcp_finished=0;

#ifdef CONFIG_FAST_PATH_MODULE
static int pptp_conn_check=0;
int fast_pptp_fw=0;
EXPORT_SYMBOL(fast_pptp_fw);
int Get_fast_pptp_fw(void)
{
	return fast_pptp_fw;

}
#else
static int pptp_conn_check=0;
int fast_pptp_fw=0;
#endif
#if defined(CONFIG_RTL_PPTP_DIRECT_PPP_REPLY)
unsigned int pptp_ppp_imagic = 0 ;
#endif

#if 1	// sync from voip customer for multiple ppp
int is_pptp_device(char *ppp_device)
{
	if (pptpAccInfo.pppDev && !strcmp(rtl_get_ppp_dev_name(pptpAccInfo.pppDev), ppp_device))
	{
		FAST_PPTP_PRINT("%s: pptp device = %s\n", __FUNCTION__, ppp_device);
		return 1;
	}
	return 0;
}

#endif

static inline void bcopy(unsigned char *dst, unsigned char *src, int len)
{	int i;
	for (i=0; i<len; i++)
		dst[i] = src[i];
}

static inline unsigned int rtl_get_skb_pptp_flag(struct sk_buff *skb)
{
	return skb->pptp_flag & 0xffu;
}

static inline void rtl_set_skb_pptp_flag(struct sk_buff *skb, unsigned int pptp_flag)
{
	skb->pptp_flag = (pptp_flag & 0xffu) | (skb->pptp_flag & 0xffffff00u);
}

static inline unsigned int rtl_get_skb_pptp_len(struct sk_buff *skb)
{
	return skb->pptp_flag >> 16;
}

static inline void rtl_set_skb_pptp_len(struct sk_buff *skb, unsigned int len)
{
	skb->pptp_flag = (len << 16) | (skb->pptp_flag & 0xffffu);
}

static inline struct net_device *rtl_get_skb_rx_pptp_dev(struct sk_buff *skb)
{
	return skb->rx_pptp_dev;
}

static inline void rtl_set_skb_rx_pptp_dev(struct sk_buff *skb, struct net_device *dev)
{
	if (skb->rx_pptp_dev == NULL)
		skb->rx_pptp_dev = dev;
}

int Check_GRE_rx_net_device(void *skb)
{
	struct iphdr *iph;
	struct pptp_gre_hdr *greh;
	iph = rtl_ip_hdr(skb);
	greh = (struct pptp_gre_hdr *)(rtl_get_skb_data(skb) + iph->ihl*4);

	if((pptpAccInfo.wanDevSlave))
	{
		//fastpptp:add the check of callid for pptp client dial up session
		if(pptpAccInfo.wanDevSlave == (void *)rtl_get_skb_dev(skb)&& greh->call_id == pptpAccInfo.peerCallID)
			return 1;
		else
			return 0;
	}else
		return 0;
 }

static void pptp_snt_acc_func(struct net *net, struct sock *sk, struct sk_buff *skb)
{
	struct ethhdr *eth;
	struct vlan_dev_priv *vlan;
	struct vlan_ethhdr *veth;

	if (!pptpAccInfo.valid || !pptpAccInfo.pppDev || !pptpAccInfo.wanDevSlave) {
		ip_local_out(net, sk, skb);
		return;
	}
	eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);
	memcpy(eth->h_dest, pptpAccInfo.peerMac, ETH_ALEN);
	memcpy(eth->h_source, pptpAccInfo.ourMac, ETH_ALEN);
	eth->h_proto = htons(0x0800);
	rtl_set_skb_dev(skb, pptpAccInfo.wanDevSlave);
	vlan = vlan_dev_priv(((struct sk_buff *)skb)->dev);
	veth = (struct vlan_ethhdr *)(((struct sk_buff *)skb)->data);
	if ((((struct sk_buff *)skb)->dev->priv_flags & IFF_802_1Q_VLAN) && (vlan->flags & VLAN_FLAG_REORDER_HDR)) {
		u16 vlan_tci;
		vlan_tci = vlan->vlan_id;
		vlan_tci |= vlan_dev_get_egress_qos_mask(((struct sk_buff *)skb)->dev, ((struct sk_buff *)skb)->priority);
		__vlan_hwaccel_put_tag(skb, vlan->vlan_proto, vlan_tci);
		__vlan_hwaccel_push_inside(skb);
		((struct sk_buff *)skb)->dev = vlan->real_dev;
	}
	if (gQosEnabled)
		dev_queue_xmit(skb);
	else
		rtl_call_skb_ndo_start_xmit(skb);
}

static void pptp_rcv_acc_func(struct ppp_channel *chan, struct sk_buff *skb)
{
	if (rtl_get_skb_pptp_flag(skb)) {
		if (enter_fast_path((void *)skb) == NET_RX_DROP)
			return;
		skb_push(skb, 2);
		skb->dev = pptpAccInfo.wanDev;
		ppp_input(chan, skb);
	} else {
		ppp_input(chan, skb);
	}
}

// Filter PPTP Outgoing-Call-Reply to log IP and call id
void fast_pptp_filter(void *skb)
{
   	struct pptp_pkt_hdr		*pptph;
   	struct PptpControlHeader	*ctlh;		
   	struct PptpOutCallReply	*ocack;
   	struct PptpOutCallRequest *ocreq;
   	struct PptpCallDisconnectNotify *occlrnotify;
   	struct PptpClearCallRequest *occlrreq;
   	struct iphdr *iph;
   	iph=rtl_ip_hdr(skb);

	if (rtl_get_skb_len(skb) >= (sizeof(struct iphdr)+sizeof(struct tcphdr)+sizeof(*pptph)+sizeof(*ctlh)+ sizeof(*occlrreq))) {
   		struct tcphdr *th = (struct tcphdr *)(((char *)iph) + iph->ihl*4);
    	
   		if (iph->protocol == IPPROTO_TCP && th->source == htons(PPTP_CONTROL_PORT)) {
	    		pptph = (struct pptp_pkt_hdr *)(((char *)th)+sizeof(struct tcphdr)+(th->doff-5)*4);
    			ctlh = (struct PptpControlHeader *)(((char *)pptph) + sizeof(struct pptp_pkt_hdr));
	    		ocack = (struct PptpOutCallReply *)(((char*)ctlh) + sizeof(struct PptpControlHeader));
    			
	   		if (ntohs(pptph->packetType) == PPTP_PACKET_CONTROL &&    				
   					ntohs(ctlh->messageType) == PPTP_OUT_CALL_REPLY &&
   					ocack->resultCode == PPTP_OUTCALL_CONNECT) {

				FAST_PPTP_PRINT("Rx PPTP Call-reply, from:%s, server callid=%d\n", skb->dev->name, ocack->callID);
    				
	    			//memset(&pptpAccInfo, 0, sizeof(pptpAccInfo));  
	    			//fastpptp:add the check of callid for pptp client dial up session
	    			
	    			if(pptpAccInfo.ourIp != 0x0 && pptpAccInfo.peerIp != 0x0){
	    				if ((pptpAccInfo.ourCallID != ocack->callID) || (pptpAccInfo.peerCallID !=ocack->peersCallID)){
	    					return;
	    				}
	    			}
#if 1
	    			//fastpptp:save server's callid and our call id from peersCallID field when parse out going reply packet 
    				pptp_tcp_finished=1;
				pptpAccInfo.ourCallID= ocack->callID;
    				pptpAccInfo.peerCallID = ocack->peersCallID;
//				printk("%s(%d): pptpAccInfo.ourCallID=%d, pptpAccInfo.peerCallID=%d\n",__FUNCTION__,__LINE__,
//					pptpAccInfo.ourCallID,pptpAccInfo.peerCallID);//Added for test

	    			pptpAccInfo.ourIp= iph->daddr;
	    			pptpAccInfo.peerIp= iph->saddr;
    
    				pptpAccInfo.wanDev= (void*)rtl_get_skb_dev(skb);
					pptpAccInfo.wanDevSlave = br_get_first_slave(pptpAccInfo.wanDev);
					pptpAccInfo.wanDevSlave = pptpAccInfo.wanDevSlave ? pptpAccInfo.wanDevSlave : pptpAccInfo.wanDev;
//				pptpAccInfo.lan_dev =__dev_get_by_name(CONFIG_TP_LAN_BRIDGE_NAME);

	    			memcpy(&pptpAccInfo.peerMac[0], rtl_get_skb_data(skb) -ETH_HLEN+ETH_ALEN, ETH_ALEN); // da of tx pkt
				memcpy(&pptpAccInfo.ourMac[0], rtl_get_skb_data(skb)-ETH_HLEN, ETH_ALEN); // sa of tx pkt
/*
				FAST_PPTP_PRINT("********************************************************************\n");
				FAST_PPTP_PRINT("ourCallID %d, peer callID %d.\n", pptpAccInfo.ourCallID, 
					pptpAccInfo.peerCallID);
				FAST_PPTP_PRINT("peerMac %x:%x:%x:%x:%x:%x.\n", pptpAccInfo.peerMac[0]
					, pptpAccInfo.peerMac[1], pptpAccInfo.peerMac[2]
					, pptpAccInfo.peerMac[3], pptpAccInfo.peerMac[4]
					, pptpAccInfo.peerMac[5]);
				FAST_PPTP_PRINT("ourMac %x:%x:%x:%x:%x:%x.\n", pptpAccInfo.ourMac[0], 
					pptpAccInfo.ourMac[1],pptpAccInfo.ourMac[2],
					pptpAccInfo.ourMac[3], pptpAccInfo.ourMac[4],
					pptpAccInfo.ourMac[5]);
				FAST_PPTP_PRINT("Dev %s %s.\n", pptpAccInfo.wanDev->name,pptpAccInfo.pppDev->name);
				FAST_PPTP_PRINT("Sip %4u.%4u.%4u.%4u  dip %4u.%4u.%4u.%4u.\n", 
					NIPQUAD(pptpAccInfo.ourIp), 
					NIPQUAD(pptpAccInfo.peerIp));
				FAST_PPTP_PRINT("********************************************************************\n");
*/
				pptpAccInfo.valid = 1;
				WRITE_ONCE(pptp_snt_acc, pptp_snt_acc_func);
				WRITE_ONCE(pptp_rcv_acc, pptp_rcv_acc_func);
//    			memcpy(&pptpInfo.mac_header[0], skb->data-ETH_HLEN+ETH_ALEN, ETH_ALEN); // da of tx pkt
//				memcpy(&pptpInfo.mac_header[ETH_ALEN], skb->data-ETH_HLEN, ETH_ALEN); // sa of tx pkt   									
// 				memcpy(&pptpInfo.mac_header[ETH_ALEN*2], skb->data-2, 2); // type    				    										
#endif
    			}		
			else if (ntohs(pptph->packetType) == PPTP_PACKET_CONTROL &&    				
   					ntohs(ctlh->messageType) == PPTP_CALL_DISCONNECT_NOTIFY) {
   						occlrnotify = (struct PptpCallDisconnectNotify *)(((char*)ctlh) + sizeof(struct PptpControlHeader));
   						FAST_PPTP_PRINT("Rx PPTP Call-Disconnect notify, from:%s, callid=%d\n", skb->dev->name, occlrnotify->callID);
   						
   						//fastpptp:add the check of callid for pptp client dial up session
   						if(pptpAccInfo.ourIp != 0x0 && pptpAccInfo.peerIp != 0x0){
	    						if ((pptpAccInfo.ourCallID != occlrnotify->callID)){
	    							return;
	    						}
	    					}
	    					
   						pptp_tcp_finished=0;
						pptpAccInfo.valid = 0;
						WRITE_ONCE(pptp_snt_acc, NULL);
						WRITE_ONCE(pptp_rcv_acc, NULL);
    			}		
	    	}
		else if (iph->protocol == IPPROTO_TCP && th->dest == htons(PPTP_CONTROL_PORT)) {
    			pptph = (struct pptp_pkt_hdr *)(((char *)th)+sizeof(struct tcphdr)+(th->doff-5)*4);
    			ctlh = (struct PptpControlHeader *)(((char *)pptph) + sizeof(struct pptp_pkt_hdr));
    			if (ntohs(pptph->packetType) == PPTP_PACKET_CONTROL &&    				
   					ntohs(ctlh->messageType) == PPTP_OUT_CALL_REQUEST) {
   						ocreq = (struct PptpOutCallRequest *)(((char*)ctlh) + sizeof(struct PptpControlHeader));
   						FAST_PPTP_PRINT("Rx PPTP Out-Call-request, from:%s, callid=%d\n", rtl_get_skb_dev_name(skb), ocreq->callID);
/*   						if(pptpAccInfo.tx_seqno !=0 && pptpAccInfo.ourIp != 0x0 && pptpAccInfo.peerIp != 0x0){
   							FAST_PPTP_PRINT("Rx PPTP Out Call-request dst is :%u.%u.%u.%u, PPTP server IP=%u.%u.%u.%u\n", NIPQUAD(iph->daddr), NIPQUAD(pptpAccInfo.peerIp));							
   							if((fastpptp_br0_ip !=0x0) && ((fastpptp_br0_ip & fastpptp_br0_mask) == (iph->saddr & fastpptp_br0_mask)) && (iph->daddr==pptpAccInfo.peerIp) && !strcmp(skb->dev->name, CONFIG_TP_LAN_BRIDGE_NAME)){
	   							if(ocreq->callID == pptpAccInfo.peerCallID){//conflict PPTP client call id from LAN host
	   								for(i=0;i<MAX_CONFLICT;i++){
	   									if(pptp_passthru_entry[i].isValid==0 && pptp_passthru_entry[i].LANHost==0){
	   										pptp_passthru_entry[i].greCallID_ToLAN=ocreq->callID;
	   										ocreq->callID = pptpAccInfo.peerCallID+i+1;
	   										pptp_passthru_entry[i].greCallID_ToWAN =ocreq->callID;
	   										pptp_passthru_entry[i].isValid=1;
	   										pptp_passthru_entry[i].LANHost= iph->saddr;
	   										FAST_PPTP_PRINT("Rx PPTP out Call-request,, from:%s, callid=0x%x, replace to CallID=%d, LANHost=%u.%u.%u.%u\n", skb->dev->name, pptp_passthru_entry[i].greCallID_ToLAN,pptp_passthru_entry[i].greCallID_ToWAN, NIPQUAD(pptp_passthru_entry[i].LANHost));
	   										tcplen = iph->tot_len-(iph->ihl*4);
	     										th->check = 0;
		    									th->check=csum_tcpudp_magic(skb->nh.iph->saddr, skb->nh.iph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)th, tcplen, 0));
		    									break;
	   									}else{
	   										if(pptp_passthru_entry[i].isValid==1 && pptp_passthru_entry[i].LANHost==iph->saddr){
	   											FAST_PPTP_PRINT("Rx PPTP out Call-request,, from:%s, callid=0x%x, replace to CallID=%d, LANHost=%u.%u.%u.%u already exist\n", skb->dev->name, pptp_passthru_entry[i].greCallID_ToLAN,pptp_passthru_entry[i].greCallID_ToWAN, NIPQUAD(pptp_passthru_entry[i].LANHost));
	   											break;
	   										}
	    									}
									}			
								}
   							}	
   						}
  */
	    			}else if (ntohs(pptph->packetType) == PPTP_PACKET_CONTROL &&    				
   						ntohs(ctlh->messageType) == PPTP_CALL_CLEAR_REQUEST) {
   						occlrreq = (struct PptpClearCallRequest *)(((char*)ctlh) + sizeof(struct PptpControlHeader));
   						FAST_PPTP_PRINT("Rx PPTP Call-clear-request, from:%s, callid=0x%x\n", rtl_get_skb_dev_name(skb), occlrreq->callID);
/*   						if(pptpAccInfo.tx_seqno !=0 && pptpAccInfo.ourIp != 0x0 && pptpAccInfo.peerIp != 0x0){
   							if((fastpptp_br0_ip !=0x0) && ((fastpptp_br0_ip & fastpptp_br0_mask) == (iph->saddr & fastpptp_br0_mask)) && (iph->daddr==pptpAccInfo.peerIp) && !strcmp(skb->dev->name, CONFIG_TP_LAN_BRIDGE_NAME)){
   								if(occlrreq->callID == pptpAccInfo.peerCallID){//conflict PPTP client call id from LAN host
	   								if(Last_pptp_passthru_entry.isValid==1 && Last_pptp_passthru_entry.LANHost==iph->saddr){
			   								if(Last_pptp_passthru_entry.greCallID_ToLAN==occlrreq->callID){
			   									FAST_PPTP_PRINT("Rx PPTP Call-clear-request, from:%s, callid=0x%x, replace to CallID=%d, LANHost=%u.%u.%u.%u\n", skb->dev->name, occlrreq->callID,Last_pptp_passthru_entry.greCallID_ToWAN, NIPQUAD(Last_pptp_passthru_entry.LANHost));
			   									occlrreq->callID = Last_pptp_passthru_entry.greCallID_ToWAN;
				   								tcplen = iph->tot_len-(iph->ihl*4);
				     								th->check = 0;
					    							th->check=csum_tcpudp_magic(skb->nh.iph->saddr, skb->nh.iph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)th, tcplen, 0));
					    							
					    							Last_pptp_passthru_entry.isValid=0; 
												Last_pptp_passthru_entry.LANHost=0;
												Last_pptp_passthru_entry.greCallID_ToLAN=0;
												Last_pptp_passthru_entry.greCallID_ToWAN=0;
				    							}
		   							}
	   							}
	   						}	
   						}	
 */
	    				}else if (ntohs(pptph->packetType) == PPTP_PACKET_CONTROL &&    				
   						ntohs(ctlh->messageType) == PPTP_CALL_DISCONNECT_NOTIFY) {
   						occlrnotify = (struct PptpCallDisconnectNotify *)(((char*)ctlh) + sizeof(struct PptpControlHeader));
   						FAST_PPTP_PRINT("Rx PPTP Call-Disconnect notify, from:%s, callid=0x%x\n", rtl_get_skb_dev_name(skb), occlrnotify->callID);
/*   						if(pptpAccInfo.tx_seqno !=0 && pptpAccInfo.ourIp != 0x0 && pptpAccInfo.peerIp != 0x0){
   							if((fastpptp_br0_ip !=0x0) && ((fastpptp_br0_ip & fastpptp_br0_mask) == (iph->saddr & fastpptp_br0_mask)) && (iph->daddr==pptpAccInfo.peerIp) && !strcmp(skb->dev->name, CONFIG_TP_LAN_BRIDGE_NAME)){
   								if(occlrnotify->callID == pptpAccInfo.peerCallID){//conflict PPTP client call id from LAN host
		   							if(Last_pptp_passthru_entry.isValid==1 && Last_pptp_passthru_entry.LANHost==iph->saddr){
			   								if(Last_pptp_passthru_entry.greCallID_ToLAN==occlrnotify->callID){
			   									FAST_PPTP_PRINT("Rx PPTP Call-Disconnect notify, from:%s, callid=0x%x, replace to CallID=%d, LANHost=%u.%u.%u.%u\n", skb->dev->name, occlrnotify->callID,Last_pptp_passthru_entry.greCallID_ToWAN, NIPQUAD(Last_pptp_passthru_entry.LANHost));
			   									occlrnotify->callID = Last_pptp_passthru_entry.greCallID_ToWAN;
				   								tcplen = iph->tot_len-(iph->ihl*4);
				     								th->check = 0;
					    							th->check=csum_tcpudp_magic(skb->nh.iph->saddr, skb->nh.iph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)th, tcplen, 0));
					    							
					    							Last_pptp_passthru_entry.isValid=0; 
												Last_pptp_passthru_entry.LANHost=0;
												Last_pptp_passthru_entry.greCallID_ToLAN=0;
												Last_pptp_passthru_entry.greCallID_ToWAN=0;
				    							}
		   							}
								}
							}
	    					}
*/						
    					}
    			}		
		
	}			
}
/*
void fast_pptp_map_rx_callID(struct pptp_gre_hdr *check_greh, unsigned int LANHost)
{
	int i;
	unsigned char *ppp_data;
	unsigned short ppp_protocol=0;
	int offset = sizeof(*check_greh) - 8;	// delete seq and ack no first
	
	if (!fast_pptp_fw)
		return;
		
			
		for(i=0;i<MAX_CONFLICT;i++){
			if(pptp_passthru_entry[i].isValid==1 && pptp_passthru_entry[i].LANHost==LANHost){
	   			if(pptp_passthru_entry[i].greCallID_ToWAN==check_greh->call_id){
	   				FAST_PPTP_PRINT("fast_pptp_map_rx_callID Got CallID=%d, replace to original Call ID to lan:%d, for LAN Host:%u.%u.%u.%u\n", check_greh->call_id, pptp_passthru_entry[i].greCallID_ToLAN,NIPQUAD(LANHost));
	   				check_greh->call_id=pptp_passthru_entry[i].greCallID_ToLAN;
	   				if (GRE_IS_S(check_greh->flags))	
						offset += 4;
						
					if (GRE_IS_A(check_greh->version))
						offset += 4;
						
					ppp_data = ((char *)check_greh) + offset;	
					if((ppp_data[0]==0xFF) && (ppp_data[1] == 0x03)){
						ppp_protocol=(ppp_data[2] << 8) + ppp_data[3];
						if(ppp_protocol == 0xC021){//LCP protocol
							if(ppp_data[4]==0x06 || ppp_data[4]==0x05){//LCP termination ack or termination
								FAST_PPTP_PRINT("fast_pptp_map_rx_callID Got termination, set entry %d to invalid, callid=%d\n", i, pptp_passthru_entry[i].greCallID_ToLAN);
								Last_pptp_passthru_entry.isValid=pptp_passthru_entry[i].isValid; 
								Last_pptp_passthru_entry.LANHost=pptp_passthru_entry[i].LANHost;
								Last_pptp_passthru_entry.greCallID_ToLAN=pptp_passthru_entry[i].greCallID_ToLAN;
								Last_pptp_passthru_entry.greCallID_ToWAN=pptp_passthru_entry[i].greCallID_ToWAN;
								pptp_passthru_entry[i].isValid=0;
	   							pptp_passthru_entry[i].LANHost=0;
	   							pptp_passthru_entry[i].greCallID_ToLAN=0;
	   							pptp_passthru_entry[i].greCallID_ToWAN=0;
							}
						}
					}
	    				break;
		    		}
	   		}
		}
}

void fast_pptp_map_rx_callID_pptp(const struct iphdr *iph, unsigned int LANHost)
{
	struct pptp_pkt_hdr		*pptph;
   	struct PptpControlHeader	*ctlh;		
   	struct PptpOutCallReply	*ocack;
	int i, tcplen=0;
	struct tcphdr *th = (void *)iph + iph->ihl * 4;
	
	if (!fast_pptp_fw)
		return;
		
	pptph = (struct pptp_pkt_hdr *)(((char *)th)+sizeof(struct tcphdr)+(th->doff-5)*4);
    	ctlh = (struct PptpControlHeader *)(((char *)pptph) + sizeof(struct pptp_pkt_hdr));
	ocack = (struct PptpOutCallReply *)(((char*)ctlh) + sizeof(struct PptpControlHeader));
    			
	   		if (ntohs(pptph->packetType) == PPTP_PACKET_CONTROL &&    				
   					ntohs(ctlh->messageType) == PPTP_OUT_CALL_REPLY &&
   					ocack->resultCode == PPTP_OUTCALL_CONNECT) {
		   		for(i=0;i<MAX_CONFLICT;i++){
					if(pptp_passthru_entry[i].isValid==1 && pptp_passthru_entry[i].LANHost==LANHost){
			   			if(pptp_passthru_entry[i].greCallID_ToWAN==ocack->peersCallID){
			   				FAST_PPTP_PRINT("fast_pptp_map_rx_callID_pptp Got CallID=%d, replace to original Call ID to lan:%d\n", ocack->peersCallID, pptp_passthru_entry[i].greCallID_ToLAN);
			   				ocack->peersCallID=pptp_passthru_entry[i].greCallID_ToLAN;
			   				tcplen = iph->tot_len-(iph->ihl*4);
	     						th->check = 0;
		    					th->check=csum_tcpudp_magic(iph->saddr, iph->daddr, tcplen, IPPROTO_TCP, csum_partial((char *)th, tcplen, 0));
				    			break;
				    		}
			   		}
				}
   		}		
}
*/

#if defined(CONFIG_RTL_PPTP_DIRECT_PPP_REPLY)

struct ppp_reply_hdr
{
//	__u16 addr_control;
	__u16 protocol;
	unsigned char code;
	unsigned char id;
	__u16 lcp_length;
	unsigned int imagicNumber;
};

int PPTP_Direct_Send_PPP_Reply(struct sk_buff * skb, int offset)
{	
	int	header_len;
	struct iphdr *iph_new, iph_newone;
	struct pptp_gre_hdr	*greh, grehone;
	struct ppp_reply_hdr *ppph, pppheader;
	unsigned char tos;
	struct ethhdr *eth;
	int ppp_hdr_len=0;		
	int wanip1=0, wanip2=0,wanip3=0;

	// var define ---------
	unsigned char *data;
	data = rtl_get_skb_data(skb);	
	unsigned char req_id;
	unsigned short req_len;

	//if the info is integrity, conitune to contruct the ptk.
	if ((pptpAccInfo.pppDev == NULL)||((rtl_get_ppp_dev_name(pptpAccInfo.pppDev) == NULL))||(strcmp(rtl_get_ppp_dev_name(pptpAccInfo.pppDev), RTL_PS_PPP0_DEV_NAME)!=0))
			return 0;	
	
	//extract necessary info	
	{
		req_id = data[offset+1];
		req_len = *((unsigned short*)(&data[offset+2]));
	}
	if (pptpAccInfo.valid && pptpAccInfo.pppDev && rtl_get_ppp_dev_priv(pptpAccInfo.pppDev))
	{
		eth = (struct ethhdr *)skb_push(skb,ETH_HLEN); 		//mac header push 	
		memcpy(eth->h_dest, pptpAccInfo.peerMac, ETH_ALEN);
		memcpy(eth->h_source, pptpAccInfo.ourMac, ETH_ALEN);
		eth->h_proto = htons(0x0800);
		// build ip header							
		iph_new = &iph_newone;	
		iph_new->version	=	4;
		iph_new->ihl		=	sizeof(struct iphdr) >> 2;
		iph_new->frag_off	=	htons(0x4000); 	/* don't do fragement */
		iph_new->protocol	=	IPPROTO_GRE;
		iph_new->tos		=	0;	//need confirm
		iph_new->daddr	=	pptpAccInfo.peerIp;
		iph_new->saddr	=	pptpAccInfo.ourIp;
		iph_new->ttl		=	IPDEFTTL;
		rtl_set_skb_ip_summed(skb, CHECKSUM_NONE);
		iph_new->tot_len	=	htons(rtl_get_skb_len(skb) - ETH_HLEN);
		iph_new->id 	=	htons(++pptpAccInfo.tx_ipID);
		iph_new->check	=	0;
		iph_new->check	=	ip_fast_csum((unsigned char *)iph_new, iph_new->ihl);	
		pptpAccInfo.ipID=	iph_new->id; // save id to check in sync_pptp_gre_seqno()
		memcpy(rtl_get_skb_data(skb) + ETH_HLEN, &iph_newone, sizeof(iph_newone));
		
		// build gre header
		greh			= &grehone;
		greh->flags 	= GRE_FLAG_K | GRE_FLAG_S;
		greh->version	= GRE_VERSION_PPTP | GRE_FLAG_A;
		greh->protocol	= htons(GRE_PROTOCOL_PPTP);

		greh->payload_len	= htons(10);	//ppp reply length
		greh->call_id		= pptpAccInfo.ourCallID;
		greh->seq		= htonl(pptpAccInfo.tx_seqno++);
		greh->ack		= htonl(pptpAccInfo.rx_seqno);
		memcpy(rtl_get_skb_data(skb)+ETH_HLEN+sizeof(struct iphdr), &grehone, sizeof(struct pptp_gre_hdr));

		//	build ppp header
		ppph 			= &pppheader;
		ppph->protocol 	= htons(0xc021);
		ppph->code		= 0x0a;
		ppph->id 		= req_id;
		ppph->lcp_length= req_len;
		ppph->imagicNumber	= pptp_ppp_imagic;		
		memcpy(skb->data+ETH_HLEN+sizeof(struct iphdr)+sizeof(struct pptp_gre_hdr), &pppheader, sizeof(struct ppp_reply_hdr));
		
		pptpAccInfo.fast_pptp_lastxmit = jiffies;
		rtl_set_skb_dev(skb, pptpAccInfo.wanDev);
		dev_queue_xmit(skb);
		//panic_printk("[%s],%d -kernel send reply ptk!!!!\n",__FUNCTION__,__LINE__);
		return -1;
	}
	return 0;
}
#endif

struct channel {
	struct ppp_file	file;		/* stuff for read/write/poll */
	struct list_head list;		/* link in all/new_channels list */
	struct ppp_channel *chan;	/* public channel data structure */
	struct rw_semaphore chan_sem;	/* protects `chan' during chan ioctl */
	spinlock_t	downl;		/* protects `chan', file.xq dequeue */
	struct ppp	*ppp;		/* ppp unit we're connected to */
	struct net	*chan_net;	/* the net channel belongs to */
	struct list_head clist;		/* link in list of channels per unit */
	rwlock_t	upl;		/* protects `ppp' */
#ifdef CONFIG_PPP_MULTILINK
	u8		avail;		/* flag used in multilink stuff */
	u8		had_frag;	/* >= 1 fragments have been sent */
	u32		lastseq;	/* MP: last sequence # received */
	int		speed;		/* speed of the corresponding ppp channel*/
#endif /* CONFIG_PPP_MULTILINK */
#if defined(CONFIG_RTL_PPPOE_HWACC) || defined (CONFIG_RTL_FAST_PPPOE)
	u8		pppoe;
	u8		rsv1;
	u16		rsv2;
#endif /* CONFIG_RTL865X_HW_TABLES */
};

struct pptp_opt *get_pptp_opt(struct sock **sk_out)
{
	struct net_device *pptpInf = (struct net_device *)pptpAccInfo.pppDev;
	struct ppp *ppp = NULL;
	struct list_head *list = NULL;
	struct channel *pch = NULL;
	struct sock *sk = NULL;
	struct pppox_sock *po = NULL;

	if (IS_ERR_OR_NULL(pptpInf))
		return NULL;
	ppp = netdev_priv(pptpInf);
	if (IS_ERR_OR_NULL(ppp)) {
		printk("[%s-%u] ppp is invalid, return\n", __func__, __LINE__);
		return NULL;
	}
	if (ppp->n_channels != 1) {
		printk("[%s-%u] ppp %s ->n_channels = %d\n",
			__func__, __LINE__, pptpInf->name,  ppp->n_channels);
		return NULL;
	}
	list = &ppp->channels;
	list = list->next;
	pch = list_entry(list, struct channel, clist);
	if (IS_ERR_OR_NULL(pch)) {
		printk("[%s-%u] pch is invalid, return\n", __func__, __LINE__);
		return NULL;
	}
	if (IS_ERR_OR_NULL(pch->chan)) {
		printk("[%s-%u] pch->chan is invalid, return\n", __func__, __LINE__);
		return NULL;
	}
	sk = (struct sock *)pch->chan->private;
	if (sk_out)
		*sk_out = sk;
	po = pppox_sk(sk);
	return &po->proto.pptp;
}

void set_pptp_device(char *ppp_device)
{
	if (fast_pptp_fw)
	{
		FAST_PPTP_PRINT("%s: pptp device = %s\n", __FUNCTION__, ppp_device);
		pptpAccInfo.pppDev = (void*)rtl_get_dev_by_name(ppp_device);
		pptpAccInfo.opt = get_pptp_opt(&pptpAccInfo.sk);
	}
}

#ifdef CONFIG_RTL_VLAN_8021Q
extern int linux_vlan_enable;
#endif
// Packet come from WAN, and it is GRE data
//	  delete IP+GRE+PPP header
int fast_pptp_to_lan(void **pskb)
{
	struct iphdr *iph;
	struct pptp_gre_hdr *greh;					
	void *ppp;
	struct sk_buff *skb = (struct sk_buff *)(*pskb), *skb1, *last_skb;
//Brad add ---------	
	unsigned char *data;
	__u32 ack, seq;
	struct pptp_opt *opt;
	unsigned char *ppp_data;	
	int offset = sizeof(*greh);	// delete seq and ack no
	int ppp_offset = 0;
	struct pppox_sock *po;
	struct sock *sk;
	s32 order, diff;
	struct sk_buff_head rcv_list;

	rtl_set_skb_pptp_flag(skb, 0);
//Brad add end-------
//	extern struct sk_buff *ppp_receive_nonmp_frame(struct ppp *ppp, struct sk_buff *skb, int is_fast_fw);
	if (ip_hdr(skb)->protocol != IPPROTO_GRE || rtl_get_skb_len(skb) < sizeof(struct iphdr))
		return 0;

	iph = rtl_ip_hdr(skb);
	data = rtl_get_skb_data(skb);
	greh = (struct pptp_gre_hdr *)(data + iph->ihl*4);

	if ((greh->version&7) == GRE_VERSION_PPTP &&
						ntohs(greh->protocol) == GRE_PROTOCOL_PPTP) {

		opt = pptpAccInfo.opt;
		sk = pptpAccInfo.sk;
		if (greh->call_id != pptpAccInfo.peerCallID || !opt || !opt->mtx_init)
			return 0;
		if (!GRE_IS_A(greh->version))
			offset -= sizeof(greh->ack);
		ppp_data = ((char *)greh) + offset;
		if (ntohs(greh->payload_len) > 0) {
			if (ppp_data[0] == PPP_ALLSTATIONS && ppp_data[1] == PPP_UI &&
				get_unaligned_be16(ppp_data + 2) == PPP_IP)
				ppp_offset = 4;
			else
			 	return 0;
		} else {
			if (!GRE_IS_A(greh->version))
				return 0;
		}
		mutex_lock(&opt->mtx);
		if (GRE_IS_A(greh->version)) {
			/* ack in different place if S = 0 */
			ack = GRE_IS_S(greh->flags) ? greh->ack : greh->seq;
			ack = ntohl(ack);
			order = ack - opt->ack_recv - 1;
			do {
				if ((s32)(ack - opt->seq_sent) > 0) {
					continue;
				}
				if (ack - opt->ack_recv > PPTP_XMIT_WIN) {
					continue;
				}
				opt->ack_recv = ack;
				opt->sample = (s32)(pptp_get_time() - opt->time_sent[order]);
				diff = opt->sample - opt->rtt;
				opt->rtt += PPTP_ACK_ALPHA(diff);
				if (diff < 0)
					diff = -diff;
				opt->dev += PPTP_ACK_BETA(diff - opt->dev);
					/* +2 to compensate low precision of int math */
				opt->ato = opt->rtt + PPTP_ACK_CHI(opt->dev + 2);
				if (opt->ato > PPTP_MAX_TIMEOUT)
					opt->ato = PPTP_MAX_TIMEOUT;
				else if (opt->ato < PPTP_MIN_TIMEOUT)
					opt->ato = PPTP_MIN_TIMEOUT;

				/* Shift packet transmit times in our transmit window */
				memmove(opt->time_sent, opt->time_sent + order + 1,
					sizeof(*opt->time_sent) * (PPTP_XMIT_WIN - (order + 1)));

				/* If we sent an entire window, increase window size */
				if ((s32)(ack - opt->win_ack) >= 0) {
					if (opt->win_sent < PPTP_XMIT_WIN)
						opt->win_sent++;
					opt->win_ack = ack + opt->win_sent;
				}

				/* Stop/(re)start receive ACK timer as necessary */
				del_timer(&opt->rack_timer);
				if (opt->ack_recv != opt->seq_sent)
					pptp_start_recv_ack_timer(opt);
			} while (0);
		}
		if (!GRE_IS_S(greh->flags))
			goto drop;
		seq = ntohl(greh->seq);
		offset = iph->ihl*4 + offset;
		if (!pskb_may_pull(skb, offset + ntohs(greh->payload_len))) {
			opt->stats.rx_length_errors++;
			goto drop;
		}
		offset += ppp_offset;
		rtl_set_skb_pptp_len((struct sk_buff *)skb, offset);
		skb_pull(skb, offset);
		//skb->transport_header= skb->network_header= skb->data;
		rtl_skb_reset_network_header(skb);
		rtl_skb_reset_transport_header(skb);
		rtl_set_skb_rx_pptp_dev((struct sk_buff *)skb, ((struct sk_buff *)skb)->dev);
//Brad add --------
		rtl_set_skb_pptp_flag((struct sk_buff *)skb, 1);
		if(rtl_get_ppp_dev_priv(pptpAccInfo.pppDev)){
			ppp = rtl_get_ppp_dev_priv(pptpAccInfo.pppDev);
			rtl_inc_ppp_stats(ppp, 0, rtl_get_skb_len(skb)-2);
			rtl_set_skb_pptp_flag((struct sk_buff *)skb, 2);
			//++ppp->stats.rx_packets;
			//ppp->stats.rx_bytes += rtl_get_skb_len(skb) - 2;
		}//else{
		//		printk("ppp0_dev priv==NULL\n");
		//	}
//Brad add end--------
		rtl_set_skb_dev(skb, pptpAccInfo.pppDev);
		po = pppox_sk(sk);
		/* Insert the packet into receive queue in order. */
		skb_queue_head_init(&rcv_list);
		skb_meta(skb)->sequence = seq;
		diff = seq - opt->seq_recv;
		if (diff <= 0) {
			opt->stats.rx_missed_errors++;
			goto drop;
		}
		else if (diff == 1) {
			/* the packet came in order, place it at the start of rcv_list */
			__skb_queue_tail(&rcv_list, skb);
			opt->seq_recv = seq;
			goto deliver;
		}
		/* The packet came too early, try to enqueue it.
		*/
		skb_queue_walk(&sk->sk_receive_queue, skb1) {
			diff = (s32)(skb_meta(skb1)->sequence - seq);
			if (diff == 0) {
				opt->stats.rx_frame_errors++;
				goto drop;
			}
			if (diff > 0) {
				break;
			}
		}
		if (sk->sk_receive_queue.qlen < PPTP_RCV_WIN) {
			__skb_insert(skb, skb1->prev, skb1, &sk->sk_receive_queue);
			skb_set_owner_r(skb, sk);
		} else {
			if (skb_queue_is_first(&sk->sk_receive_queue, skb1)) {
				__skb_queue_tail(&rcv_list, skb);
				opt->seq_recv = seq;
			} else {
				last_skb = __skb_dequeue(&sk->sk_receive_queue);
				opt->seq_recv = skb_meta(last_skb)->sequence;
				__skb_queue_tail(&rcv_list, last_skb);
				__skb_insert(skb, skb1->prev, skb1, &sk->sk_receive_queue);
				skb_set_owner_r(skb, sk);
			}
		}
deliver:
		/* Look if we have some packets in sequence after sendq. */
		skb_queue_walk_safe(&sk->sk_receive_queue, skb, skb1) {
			if ((s32)(skb_meta(skb)->sequence - opt->seq_recv) > 1)
				break;
			__skb_unlink(skb, &sk->sk_receive_queue);
			opt->seq_recv =  skb_meta(skb)->sequence;
			skb_orphan(skb);
			__skb_queue_tail(&rcv_list, skb);
		}
		if (skb_queue_empty(&sk->sk_receive_queue)) {
			if (timer_pending(&opt->reorder_timer))
				del_timer(&opt->reorder_timer);
		} else {
			if (!timer_pending(&opt->reorder_timer))
				pptp_start_reorder_timer(opt);
		}
		if (!skb_queue_empty(&rcv_list)) {
			pptp_ack(opt);
		}
		mutex_unlock(&opt->mtx);
		while ((skb = __skb_dequeue(&rcv_list))) {
			pptp_rcv_acc_func(&po->chan, skb);
		}
		return -1;
	}
	return 0;
drop:
	mutex_unlock(&opt->mtx);
	kfree_skb(skb);
	return -1;
}

void check_and_restore_pptp_hdr(struct sk_buff *skb)
{
	if (fast_pptp_fw == 0)
	{
		return;
	}

	if (rtl_get_skb_pptp_flag(skb))
	{
		if (rtl_get_skb_pptp_flag(skb) == 2)
		{
			rtl_get_skb_dev(skb)->stats.rx_packets--;
			rtl_get_skb_dev(skb)->stats.rx_bytes -= rtl_get_skb_len(skb) + 2;
		}
		rtl_skb_push(skb, rtl_get_skb_pptp_len(skb));
		rtl_set_skb_network_header(skb, rtl_get_skb_data(skb));
		rtl_set_skb_transport_header(skb, rtl_get_skb_data(skb));
		rtl_set_skb_dev(skb, rtl_get_skb_rx_pptp_dev(skb));
	}
}

unsigned long get_fastpptp_lastxmit(void)
{
	if(fast_pptp_fw && pptpAccInfo.valid == 1)
		return pptpAccInfo.fast_pptp_lastxmit;
	
	return 0;
}

// Packet come from LAN and dst dev is ppp0, 
// add IP+GRE+PPTP header
int fast_pptp_to_wan(struct sk_buff *skb)
{
	int	header_len, len;
	struct iphdr *iph_new;
	struct pptp_gre_hdr	*greh;
	int max_headroom;
	unsigned char *data;
	struct pptp_opt *opt;
	struct sock *sk = NULL;
	u32 seq_recv;

	if (pptpAccInfo.valid && pptpAccInfo.pppDev)
	{
		opt = pptpAccInfo.opt;
		sk = pptpAccInfo.sk;
		if (!sk || !opt || !opt->mtx_init) {
			printk("[%s-%u] opt(%p) sk(%p) is null\n", __func__, __LINE__,
				opt, sk);
			return 0;
		}

		max_headroom = ETH_HLEN + sizeof(*iph_new) + sizeof(*greh) + 4; // mac-header+ip+gre
		if (rtl_skb_headroom(skb) < max_headroom || rtl_skb_cloned(skb) || rtl_skb_shared(skb)) {
			void *new_skb = (void*)skb_realloc_headroom(skb, max_headroom);
			if (!new_skb) {
			//	printk("%s: skb_realloc_headroom failed!\n", __FUNCTION__);
				opt->stats.tx_dropped++;
				dev_kfree_skb(skb);
				return 1;		
			}									
			dev_kfree_skb(skb);
			skb = new_skb;
		}
		mutex_lock(&opt->mtx);
		if (opt->seq_sent - opt->ack_recv >= opt->win_sent) {
			do {
				opt->ack_recv = opt->seq_sent;
				opt->win_ack = opt->ack_recv + opt->win_sent;
				break;
				opt->stats.tx_window_errors++;
				mutex_unlock(&opt->mtx);
				kfree_skb(skb);
				return 1;
			} while (0);
		}
		opt->time_sent[opt->seq_sent - opt->ack_recv] = pptp_get_time();
		data = skb_push(skb, 4);
		data[0] = PPP_ALLSTATIONS;
		data[1] = PPP_UI;
		data[2] = 0x0;
		data[3] = PPP_IP;
		len = ((struct sk_buff *)skb)->len;

		header_len = sizeof(*greh);
		seq_recv = opt->seq_recv;

		if (opt->ack_sent == seq_recv)
			header_len -= sizeof(greh->ack);

		/* Push down and install GRE header */
		greh = (struct pptp_gre_hdr *)skb_push(skb, header_len);
		greh->flags = GRE_FLAG_K;
		greh->version = GRE_VERSION_PPTP;
		greh->protocol = htons(GRE_PROTOCOL_PPTP);
		greh->call_id = pptpAccInfo.ourCallID;
	
		greh->flags |= GRE_FLAG_S;
		greh->seq = htonl(++opt->seq_sent);
		if (opt->ack_sent != seq_recv) {
			/* send ack with this message */
			greh->version |= GRE_FLAG_A;
			greh->ack  = htonl(seq_recv);
			opt->ack_sent = seq_recv;
		}
		greh->payload_len = htons(len);
		/*	Push down and install the IP header. */

		skb_reset_transport_header(skb);
		skb_push(skb, sizeof(*iph_new));
		skb_reset_network_header(skb);

		iph_new = ip_hdr(skb);
		iph_new->version = 4;
		iph_new->ihl = sizeof(struct iphdr) >> 2;
		iph_new->frag_off =	htons(IP_DF);
		iph_new->protocol =	IPPROTO_GRE;
		iph_new->tos = 0;
		iph_new->daddr = pptpAccInfo.peerIp;
		iph_new->saddr = pptpAccInfo.ourIp;
		iph_new->ttl = IPDEFTTL;
		iph_new->tot_len = htons(((struct sk_buff *)skb)->len);

		((struct sk_buff *)skb)->ip_summed = CHECKSUM_NONE;
		iph_new->id = ntohs(opt->ip_id++);
		ip_send_check(iph_new);
		if (opt->seq_sent == opt->ack_recv + 1)
			pptp_start_recv_ack_timer(opt);
		mutex_unlock(&opt->mtx);
		pptp_snt_acc_func(sock_net(sk), skb->sk, skb);
		return 1;
	}
	else {
		if (pptpAccInfo.pppDev== NULL)
			printk("pptpInfo.ppp0_dev == NULL\n");
		else if (rtl_get_ppp_dev_priv(pptpAccInfo.pppDev) == NULL)
			printk("pptpInfo.ppp0_dev->priv == NULL\n");
	}

	return 0;	
}
EXPORT_SYMBOL(fast_pptp_to_wan);
#if defined(RTL_FASTPATH_FRAGMENT_SUPPORT)
int fast_pptp_to_wan_check(void *skb)
{
	if (!pptpAccInfo.valid || !pptpAccInfo.pppDev ||
		!pptpAccInfo.opt || !pptpAccInfo.sk)
		return 0;
	return 1;
}
int fast_pptp_to_wan2(struct sk_buff *skb)
{
	int	header_len, len;
	struct iphdr *iph_new;
	struct pptp_gre_hdr	*greh;
	int max_headroom;
	unsigned char *data;
	struct pptp_opt *opt;
	struct sock *sk = NULL;
	u32 seq_recv;

	if (pptpAccInfo.valid && pptpAccInfo.pppDev)
	{
		opt = pptpAccInfo.opt;
		sk = pptpAccInfo.sk;
		if (!sk || !opt || !opt->mtx_init) {
			printk("[%s-%u] opt(%p) sk(%p) is null\n", __func__, __LINE__,
				opt, sk);
			goto fail;
		}

		max_headroom = ETH_HLEN + sizeof(*iph_new) + sizeof(*greh) + 4; // mac-header+ip+gre
		if (rtl_skb_headroom(skb) < max_headroom || rtl_skb_cloned(skb) || rtl_skb_shared(skb)) {
			void *new_skb = (void*)skb_realloc_headroom(skb, max_headroom);
			if (!new_skb) {
			//	printk("%s: skb_realloc_headroom failed!\n", __FUNCTION__);
				opt->stats.tx_dropped++;
				dev_kfree_skb(skb);
				return 1;		
			}									
			dev_kfree_skb(skb);
			skb = new_skb;
		}
		mutex_lock(&opt->mtx);
		if (opt->seq_sent - opt->ack_recv >= opt->win_sent) {
			do {
				opt->ack_recv = opt->seq_sent;
				opt->win_ack = opt->ack_recv + opt->win_sent;
				break;
				opt->stats.tx_window_errors++;
				mutex_unlock(&opt->mtx);
				kfree_skb(skb);
				return 1;
			} while (0);
		}
		opt->time_sent[opt->seq_sent - opt->ack_recv] = pptp_get_time();
		data = skb_push(skb, 4);
		data[0] = PPP_ALLSTATIONS;
		data[1] = PPP_UI;
		data[2] = 0x0;
		data[3] = PPP_IP;
		len = ((struct sk_buff *)skb)->len;

		header_len = sizeof(*greh);
		seq_recv = opt->seq_recv;

		if (opt->ack_sent == seq_recv)
			header_len -= sizeof(greh->ack);

		/* Push down and install GRE header */
		greh = (struct pptp_gre_hdr *)skb_push(skb, header_len);
		greh->flags = GRE_FLAG_K;
		greh->version = GRE_VERSION_PPTP;
		greh->protocol = htons(GRE_PROTOCOL_PPTP);
		greh->call_id = pptpAccInfo.ourCallID;
	
		greh->flags |= GRE_FLAG_S;
		greh->seq = htonl(++opt->seq_sent);
		if (opt->ack_sent != seq_recv) {
			/* send ack with this message */
			greh->version |= GRE_FLAG_A;
			greh->ack  = htonl(seq_recv);
			opt->ack_sent = seq_recv;
		}
		greh->payload_len = htons(len);
		/*	Push down and install the IP header. */

		skb_reset_transport_header(skb);
		skb_push(skb, sizeof(*iph_new));
		skb_reset_network_header(skb);

		iph_new = ip_hdr(skb);
		iph_new->version = 4;
		iph_new->ihl = sizeof(struct iphdr) >> 2;
		iph_new->frag_off =	htons(IP_DF);
		iph_new->protocol =	IPPROTO_GRE;
		iph_new->tos = 0;
		iph_new->daddr = pptpAccInfo.peerIp;
		iph_new->saddr = pptpAccInfo.ourIp;
		iph_new->ttl = IPDEFTTL;
		iph_new->tot_len = htons(((struct sk_buff *)skb)->len);

		((struct sk_buff *)skb)->ip_summed = CHECKSUM_NONE;
		iph_new->id = ntohs(opt->ip_id++);
		ip_send_check(iph_new);
		if (opt->seq_sent == opt->ack_recv + 1)
			pptp_start_recv_ack_timer(opt);
		mutex_unlock(&opt->mtx);
		pptp_snt_acc_func(sock_net(sk), skb->sk, skb);
		return 1;
	}
fail:
	kfree_skb(skb);
	return 0;	
}
#endif
// Sync Rx GRE seq no to daemon.
//		Replace tx-seq and ack-seq.
void fast_pptp_sync_rx_seq(void *skb)
{

}


// Sync tx GRE seq no. 
//   replace gre tx seq, and ack number if packet is not sent from upper layer
void sync_tx_pptp_gre_seqno(void *skb)
{

}

static unsigned long atoi_dec(char *s)
{
	unsigned long k = 0;

	k = 0;
	while (*s != '\0' && *s >= '0' && *s <= '9') {
		k = 10 * k + (*s - '0');
		s++;
	}
	return k;
}

int pptpconn_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char tmpbuf[200];	

	if (count < 2) 
		return -EFAULT;

	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {
		pptp_conn_check = atoi_dec(tmpbuf);
	}
	return -EFAULT;				
}

int ppfw_write_proc(struct file *file, const char *buffer,
		      unsigned long count, void *data)
{
	char *tmpbuf;
	struct pptp_opt *opt;

	opt = pptpAccInfo.opt;
/*
	int i;
	struct in_device *in_dev;
	struct in_ifaddr **ifap = NULL;
	struct in_ifaddr *ifa = NULL;
	struct net_device *landev;
*/
	
	if (count < 2) 
		return -EFAULT;

	tmpbuf = kmalloc(count+32, GFP_ATOMIC);
	if (tmpbuf==NULL) 
		return -EFAULT;
	
	if (buffer && !copy_from_user(tmpbuf, buffer, count)) {
		fast_pptp_fw = atoi_dec(tmpbuf);

		//fastpptp:init pptpInfo struct when system re-init(init.sh) and pppd disconnect(disconnect.sh)
	    memset((void *)&pptpAccInfo, 0, sizeof(pptpAccInfo));
		if (opt && opt->mtx_init) {
			mutex_lock(&opt->mtx);
			memset(&opt->stats, 0, sizeof(opt->stats));
			mutex_unlock(&opt->mtx);
		}
/*		
		for(i=0;i<MAX_CONFLICT;i++){
              	memset(&pptp_passthru_entry[i], 0x0, sizeof(_gre_callId_map_entry)); 
              }
              memset(&Last_pptp_passthru_entry, 0x0, sizeof(_gre_callId_map_entry));

              if ((landev = __dev_get_by_name(CONFIG_TP_LAN_BRIDGE_NAME)) != NULL){
			ifa =NULL;
			if ((in_dev=__in_dev_get_rtnl(landev)) != NULL) {
				for (ifap=(&(in_dev->ifa_list)); 
						(ifap!=NULL) && ((ifa=*ifap) != NULL); 
						ifap=&(ifa->ifa_next)) 
				{
					if (strcmp(CONFIG_TP_LAN_BRIDGE_NAME, ifa->ifa_label) == 0){
						break; 
					}
				}
				if(ifa != NULL){
					fastpptp_br0_ip = ifa->ifa_address;
					fastpptp_br0_mask = ifa->ifa_mask;
				}
			}
		}
*/
		sync_tx_pptp_gre_seqno_hook = NULL;
		kfree(tmpbuf);
		return count;
	}

	kfree(tmpbuf);
	return -EFAULT;				
}		


#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
int ppfw_read_proc(struct seq_file *s, void *v)
{
	struct sock *sk;
	struct pptp_opt *opt, bkopt = {0};

	opt = pptpAccInfo.opt;
	sk = pptpAccInfo.sk;
	if (opt && opt->mtx_init) {
		mutex_lock(&opt->mtx);
		memcpy(&bkopt, opt, sizeof(bkopt));
		mutex_unlock(&opt->mtx);
	}
	seq_printf(s, "fast_pptp_fw: %d\n", fast_pptp_fw);
	seq_printf(s, "fast_pptp_lastxmit: %lu\n", pptpAccInfo.fast_pptp_lastxmit);
	seq_printf(s, "valid: %d\n", pptpAccInfo.valid);
	seq_printf(s, "wanDev: %s\n",
		pptpAccInfo.wanDev ? ((struct net_device *)pptpAccInfo.wanDev)->name : "null");
	seq_printf(s, "wanDevSlave: %s\n",
		pptpAccInfo.wanDevSlave ? ((struct net_device *)pptpAccInfo.wanDevSlave)->name : "null");
	seq_printf(s, "pppDev: %s\n",
		pptpAccInfo.pppDev ? ((struct net_device *)pptpAccInfo.pppDev)->name : "null");
	seq_printf(s, "ourMac: %02X-%02X-%02X-%02X-%02X-%02X\n",
		pptpAccInfo.ourMac[0], pptpAccInfo.ourMac[1], pptpAccInfo.ourMac[2],
		pptpAccInfo.ourMac[3], pptpAccInfo.ourMac[4], pptpAccInfo.ourMac[5]);
	seq_printf(s, "peerMac: %02X-%02X-%02X-%02X-%02X-%02X\n",
		pptpAccInfo.peerMac[0], pptpAccInfo.peerMac[1], pptpAccInfo.peerMac[2],
		pptpAccInfo.peerMac[3], pptpAccInfo.peerMac[4], pptpAccInfo.peerMac[5]);
	seq_printf(s, "ourCallID: %u\n", ntohs(pptpAccInfo.ourCallID));
	seq_printf(s, "peerCallID: %u\n", ntohs(pptpAccInfo.peerCallID));
	seq_printf(s, "ourIp: %u.%u.%u.%u\n", NIPQUAD(pptpAccInfo.ourIp));
	seq_printf(s, "peerIp: %u.%u.%u.%u\n", NIPQUAD(pptpAccInfo.peerIp));
	if (opt && opt->mtx_init) {
		seq_printf(s, "seq->s(%u),r(%u) ack->s(%u),r(%u)\n", bkopt.seq_sent, bkopt.seq_recv,
			bkopt.ack_sent, bkopt.ack_recv);
		seq_printf(s, "win_sent: %d win_ack: %d\n", bkopt.win_sent, bkopt.win_ack);
		seq_printf(s, "sample: %d rtt: %d dev: %d ato: %u | rtt_max: %d\n", bkopt.sample, bkopt.rtt,
			bkopt.dev, bkopt.ato, bkopt.rtt_max);
		seq_printf(s, "TX -> PPPOX_DEAD: %lu err: %lu win_err: %lu\n", bkopt.stats.tx_aborted_errors,
			bkopt.stats.tx_dropped, bkopt.stats.tx_window_errors);
		seq_printf(s, "RX -> NOT CONN: %lu length err: %lu out of time: %lu repeat: %lu\n",
			bkopt.stats.rx_fifo_errors, bkopt.stats.rx_length_errors, bkopt.stats.rx_missed_errors,
			bkopt.stats.rx_frame_errors);
		seq_printf(s, "ACK(R): %lu R reorder: %lu recv and send ack: %lu\n", bkopt.stats.tx_heartbeat_errors, 
			bkopt.stats.rx_over_errors, bkopt.stats.rx_crc_errors);
	}
	if (sk) {
		seq_printf(s, "rmem: %d rcvbuf: %u\n", sk->sk_rmem_alloc.counter, sk->sk_rcvbuf);
		seq_printf(s, "wmem: %d sntbuf: %u\n", sk->sk_wmem_alloc.counter, sk->sk_sndbuf);
	}
	return 0;
}

int pptpconn_read_proc(struct seq_file *s, void *v)
{
      seq_printf(s, "%d\n", pptp_conn_check);

      return 0;
}

int fastpath_pptp_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, ppfw_read_proc, NULL));
}

static ssize_t fastpath_pptp_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return ppfw_write_proc(file, userbuf,count, off);
}


struct file_operations fastpath_pptp_proc_fops = {
        .open           = fastpath_pptp_single_open,
	 .write		= fastpath_pptp_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};

int pptp_conn_single_open(struct inode *inode, struct file *file)
{
        return(single_open(file, pptpconn_read_proc, NULL));
}

static ssize_t pptp_conn_single_write(struct file * file, const char __user * userbuf,
		     size_t count, loff_t * off)
{
	return pptpconn_write_proc(file, userbuf,count, off);
}


struct file_operations pptp_conn_proc_fops = {
        .open           = pptp_conn_single_open,
	 .write		= pptp_conn_single_write,
        .read           = seq_read,
        .llseek         = seq_lseek,
        .release        = single_release,
};
#else
int ppfw_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
      int len;

      len = sprintf(page, "%d\n", fast_pptp_fw);
      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}

int pptpconn_read_proc(char *page, char **start, off_t off,
		     int count, int *eof, void *data)
{
      int len;

      len = sprintf(page, "%d\n", pptp_conn_check);
      if (len <= off+count) *eof = 1;
      *start = page + off;
      len -= off;
      if (len>count) len = count;
      if (len<0) len = 0;
      return len;
}
#endif

#if defined(CONFIG_PROC_FS)
static struct proc_dir_entry *res1=NULL;
static struct proc_dir_entry *res_check_pptp=NULL;
#endif

#ifdef CONFIG_FAST_PATH_MODULE
int  fast_pptp_init(void)
#else
int __init fast_pptp_init(void)
#endif
{
#if defined(CONFIG_PROC_FS)
    #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
	res1 = proc_create_data("fast_pptp", 0, &proc_root,
			 &fastpath_pptp_proc_fops, NULL);

	res_check_pptp = proc_create_data("pptp_conn_ck", 0, &proc_root,
			 &pptp_conn_proc_fops, NULL);
	#else
	res1 = create_proc_entry("fast_pptp", 0, NULL);
	if (res1) {
		res1->read_proc = ppfw_read_proc;
		res1->write_proc = ppfw_write_proc;
	}

	res_check_pptp = create_proc_entry("pptp_conn_ck", 0, NULL);
	if (res_check_pptp) {
		res_check_pptp->read_proc = pptpconn_read_proc;
		res_check_pptp->write_proc = pptpconn_write_proc;
	}
	#endif
#endif

	memset((void*)&pptpAccInfo, 0, sizeof(pptpAccInfo));
	return 0;
}

#ifdef CONFIG_FAST_PATH_MODULE
void fast_pptp_exit(void)
#else
void __exit fast_pptp_exit(void)
#endif
{
#if defined(CONFIG_PROC_FS)
	if (res1) {
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("fast_pptp", &proc_root);	
		#else
		remove_proc_entry("fast_pptp", res1);		
		#endif
		res1 = NULL;
	}

	if (res_check_pptp) {
        #if LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)
		remove_proc_entry("pptp_conn_ck", &proc_root);	
		#else
		remove_proc_entry("pptp_conn_ck", res_check_pptp);		
		#endif
		res_check_pptp = NULL;
	}
#endif
}

#endif
#if 0
#ifdef CONFIG_FAST_PATH_MODULE
EXPORT_SYMBOL(fast_pptp_exit);
EXPORT_SYMBOL(fast_pptp_init);
EXPORT_SYMBOL(fast_pptp_filter);
EXPORT_SYMBOL(fast_pptp_to_wan);
EXPORT_SYMBOL(Get_fast_pptp_fw);
EXPORT_SYMBOL(fast_pptp_sync_rx_seq);
EXPORT_SYMBOL(Check_GRE_rx_net_device);
EXPORT_SYMBOL(fast_pptp_to_lan);
#endif
#endif

