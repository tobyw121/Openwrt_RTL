#include <linux/kernel.h>

#include <8192cd.h>
#include <8192cd_cfg.h>

#include <ca_types.h>
#include <ca_ipc_pe.h>

#include <pe_fw.h>

#include <core/8192cd_core_rx.h>
#include <core/core_sync.h>

#include <pe_ipc.h>


struct list_head queued_ipc_cmd_list;

/*
*	RX CMD path
*/
extern ca_status_t ca_ipc_msg_handle_register(ca_ipc_session_id_t session_id,
	const ca_ipc_msg_handle_t * msg_handle_array,ca_uint32_t msg_handle_count);

/*
*	RX sta handler
*/
extern ca_status_t ca_ipc_sta_handle_register(const ca_ipc_sta_handler_t * sta_handler);

/*
*	TX CMD path
*/
extern ca_status_t ca_ipc_msg_async_send(ca_ipc_pkt_t* p_ipc_pkt);
extern ca_status_t ca_ipc_msg_sync_send(ca_ipc_pkt_t* p_ipc_pkt,void * result_data,ca_uint16_t * result_size);

/*
*	Check CPU is up or down
*/
extern ca_status_t ca_ipc_check_status(ca_uint8 cpu_id);
extern ca_status_t ca_ipc_print_status(ca_uint8 cpu_id);


int tx_pkt_recycle(unsigned char *buf);
int tx_sta_add_cmd(unsigned char *buf,unsigned int buf_len);
int tx_sta_del_cmd(unsigned char *buf,unsigned int buf_len);
int tx_sta_update_cmd(unsigned char *buf,unsigned int buf_len);
bool do_issue_add_ba_req(struct rtl8192cd_priv *priv, void* msg_data);
bool do_issue_deauth(struct rtl8192cd_priv *priv, void* msg_data);
bool do_mac_clone(struct rtl8192cd_priv *priv, void* msg_data);
bool do_add_a4_client(struct rtl8192cd_priv *priv, void* msg_data);
bool do_dummy(struct rtl8192cd_priv *priv, void* msg_data);

static struct func_handler_s func_handler[]={
	{WFO_IPC_FUNC_ADDBAREQ,		do_issue_add_ba_req},
	{WFO_IPC_FUNC_ISSUE_DEAUTH,	do_issue_deauth},
	{WFO_IPC_FUNC_ISSUE_PSPOLL,	do_dummy},
	{WFO_IPC_FUNC_FREE_STAINFO,	do_dummy},
	{WFO_IPC_FUNC_FREE_STAINFO,	do_mac_clone},
	{WFO_IPC_FUNC_ADD_A4_CLIENT,do_add_a4_client},
};

unsigned char* t2h_sram_addr_transfer(unsigned char *addr)
{
	unsigned int phys_mem_shift = 0;
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	unsigned char *data = NULL;

	//panic_printk("%s: B4 addr=0x%x\n", __FUNCTION__, addr);
	
	phys_mem_shift = addr - pe_addr->pe_sram_phys_addr;
	if(phys_mem_shift > pe_addr->pe_sram_mem_size){
		panic_printk("sram virt addr = %x\r\n", pe_addr->pe_sram_virt_addr);
		panic_printk("sram phys addr = %x\r\n", pe_addr->pe_sram_phys_addr);
		panic_printk("phys shift = %x\r\n", phys_mem_shift);
		return 0;
	}

	data = (unsigned char*)(pe_addr->pe_sram_virt_addr + phys_mem_shift);

	//panic_printk("%s: A4 addr=0x%x\n", __FUNCTION__, data);

	return data;
}

#if 0
unsigned char* t2h_dmem_addr_transfer(unsigned char *addr)
{
	unsigned int phys_mem_shift = 0;
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	unsigned char *data = NULL;

	//panic_printk("%s: B4 addr=0x%x\n", __FUNCTION__, addr);
	
	phys_mem_shift = addr - pe_addr->pe_dmem_phys_addr;
	if(phys_mem_shift > pe_addr->pe_dmem_mem_size){
		panic_printk("sram virt addr = %x\r\n", pe_addr->pe_dmem_virt_addr);
		panic_printk("sram phys addr = %x\r\n", pe_addr->pe_dmem_phys_addr);
		panic_printk("phys shift = %x\r\n", phys_mem_shift);
		return 0;
	}

	data = (unsigned char*)(pe_addr->pe_dmem_virt_addr + phys_mem_shift);

	//panic_printk("%s: A4 addr=0x%x\n", __FUNCTION__, data);

	return data;
}
#endif

unsigned char* t2h_addr_transfer(unsigned char *addr)
{
	unsigned int phys_mem_shift = 0;
	struct pe_addr_base* pe_addr = get_pe_addr_base();
	unsigned char *data = NULL;

	//panic_printk("%s: B4 addr=0x%x\n", __FUNCTION__, addr);
	
	phys_mem_shift = addr - (pe_addr->pe_fw_phys_addr & 0x0FFFFFFF);
	if(phys_mem_shift > pe_addr->pe_fw_mem_size){
		panic_printk("mem virt addr = %x\r\n", pe_addr->pe_fw_virt_addr);
		panic_printk("mem phys addr = %x\r\n", pe_addr->pe_fw_phys_addr);
		panic_printk("phys shift = %x\r\n", phys_mem_shift);
		return 0;
	}

	data = (unsigned char*)(pe_addr->pe_fw_virt_addr + phys_mem_shift);

	//panic_printk("%s: A4 addr=0x%x\n", __FUNCTION__, data);

	return data;
}

#if 0
unsigned int t2h_data_check_sum(unsigned char* data, unsigned int len)
{
	unsigned int i = 0, checksum = 0;
	
	for(i = 1; i <= len;i++){
		checksum += (*data);
		data++;		
	}

	return checksum;
}
#endif

int check_pe_status(void)
{
	return ca_ipc_check_status(CA_IPC_CPU_PE0);
}

void check_ipc_status(void)
{
	ca_ipc_print_status(0);
	ca_ipc_print_status(1);
}

int tx_init_cmd(void)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_INIT;
    send_date.msg_size = 0;
    send_date.msg_data = 0;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_start_process_cmd(struct rtl8192cd_priv *priv)
{
    ca_ipc_pkt_t send_date;
	unsigned int buf[1];
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	if(priv->vap_id >= 0)
		buf[0] = priv->vap_id;
	else if(IS_VXD_INTERFACE(priv))
		buf[0] = 0xAAAAAAAA;
	else
#endif
		buf[0] = 0xFFFFFFFF;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_START_PROCESS;
    send_date.msg_size = sizeof(buf);
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_stop_process_cmd(struct rtl8192cd_priv *priv)
{
    ca_ipc_pkt_t send_date;
	unsigned int buf[1];
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	if(priv->vap_id >= 0)
		buf[0] = priv->vap_id;
	else if(IS_VXD_INTERFACE(priv))
		buf[0] = 0xAAAAAAAA;
	else
#endif
		buf[0] = 0xFFFFFFFF;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_STOP_PROCESS;
    send_date.msg_size = sizeof(buf);
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);

	return 0;
}

#define RECYCLE_SIZE	32

struct recycle_cb
{
	unsigned char recycle_num;
	unsigned int recycle_skb[RECYCLE_SIZE];	
};

struct recycle_cb r_cb;

int tx_pkt_recycle(unsigned char *buf)
{
	r_cb.recycle_skb[r_cb.recycle_num] = *((unsigned int*)buf);
	r_cb.recycle_num++;

	if(r_cb.recycle_num == RECYCLE_SIZE)
	{
		tx_pkt_recycle_cmd();
		//r_cb.recycle_num = 0;
	}
}

int tx_pkt_recycle_cmd()
{
	ca_ipc_pkt_t send_date;
	
	send_date.dst_cpu_id = CA_IPC_CPU_PE0;
	send_date.session_id = CA_IPC_SESSION_WFO;
	send_date.msg_no = WFO_IPC_H2T_PKT_RECYCLE;
	send_date.msg_size = sizeof(r_cb);
	send_date.msg_data = &r_cb;
	send_date.priority = CA_IPC_PRIO_HIGH;

	ca_ipc_msg_async_send(&send_date);

	r_cb.recycle_num = 0;
	
	return 0;
}

int tx_sta_add_cmd(unsigned char *buf,unsigned int buf_len)
{
	ca_ipc_pkt_t send_date;	

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_STA_ADD;
    send_date.msg_size = buf_len;
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

	ca_ipc_msg_async_send(&send_date);

	return 0;
}

int tx_sta_del_cmd(unsigned char *buf,unsigned int buf_len)
{
    ca_ipc_pkt_t send_date;
	
	send_date.dst_cpu_id = CA_IPC_CPU_PE0;
	send_date.session_id = CA_IPC_SESSION_WFO;
	send_date.msg_no = WFO_IPC_H2T_STA_DEL;
	send_date.msg_size = buf_len;
	send_date.msg_data = buf;
	send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_stainfo_set_cmd(struct rtl8192cd_priv *priv, struct stat_info *pstat, unsigned short num, unsigned short len, unsigned char* buf)
{
    ca_ipc_pkt_t send_date;
	struct stainfo_cb var;
	
	if(len > MAX_VAR_LEN){
		//panic_printk("len is %d > %d\n", len, MAX_VAR_LEN);
		return 0;
	}
	
	memset(&var, 0x0, sizeof(struct stainfo_cb));
	
	memcpy(var.mac, pstat->cmn_info.mac_addr, 6);
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	if(IS_VXD_INTERFACE(priv))
		var.vap_id = -2;
	else
		var.vap_id = priv->vap_id;
#else
	var.vap_id = -1;
#endif
	var.num = num;
	var.len = len;	
	memcpy(var.data, buf, len);		

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_STA_UPDATE;
    send_date.msg_size = sizeof(var);
    send_date.msg_data = &var;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

#if 0
int tx_sta_update_cmd(unsigned char *buf,unsigned int buf_len)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_STA_UPDATE;
    send_date.msg_size = buf_len;
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}
#endif

int tx_var_set_cmd(struct rtl8192cd_priv *priv, struct var_arg* arg_list, unsigned char* buf)
{
    ca_ipc_pkt_t send_date;
	struct var_cb var;

	if(arg_list->len > MAX_VAR_LEN)
	{
		panic_printk("%s : num = %d, len is %d > %d\n", __func__, arg_list->num, arg_list->len, MAX_VAR_LEN);
		return 0;
	}

	memset(&var, 0, sizeof(struct var_cb));

#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	if(IS_VXD_INTERFACE(priv))
		var.vap_id = -2;
	else
		var.vap_id = priv->vap_id;
#else
	var.vap_id = -1;
#endif
	var.num = arg_list->num;
	var.len = arg_list->len;

	if(arg_list->num != VAR_t2h_sync)
		memcpy(var.data, buf, arg_list->len);

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_VAR_SET;
    send_date.msg_size = sizeof(var);
    send_date.msg_data = &var;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}

int tx_debug_cmd(unsigned char* buf, unsigned int len)
{
    ca_ipc_pkt_t send_date;

    send_date.dst_cpu_id = CA_IPC_CPU_PE0;
    send_date.session_id = CA_IPC_SESSION_WFO;
    send_date.msg_no = WFO_IPC_H2T_DEBUG;
    send_date.msg_size = len;
    send_date.msg_data = buf;
    send_date.priority = CA_IPC_PRIO_HIGH;

    ca_ipc_msg_async_send(&send_date);
	return 0;
}


int rx_pe_ready_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	//char byestr[]={"Bye from ARM , nice to meet you"};
	//int size_str = sizeof(byestr);
	//panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);
	if(netif_running(pe_offload_dev))
	{
		struct rtl8192cd_priv *priv = pe_offload_priv; 
		panic_printk("%s offload ready\r\n", priv->dev->name);
		priv->pe_ready = 1;
	}

	//memcpy(msg_data,byestr,size_str);
	//*msg_size = size_str;
	return 0;
}




int rx_pkt_send_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	if(netif_running(pe_offload_dev))
	{
		struct rtl8192cd_priv *priv = pe_offload_priv; 
		unsigned int pkt_size = 0;
		unsigned char *data = NULL;
		struct sk_buff	*pskb = NULL;
		unsigned long flags = 0;
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
		struct rtl8192cd_priv *priv_vap = NULL;
		unsigned int vap_id;
		struct sk_buff *newskb = NULL;
#endif

		if(priv == NULL)
		{
			panic_printk("priv is NULL\r\n");
			goto recycle;
		}

		if(!priv->pe_ready)
		{
			//panic_printk("pe is not ready");
			goto recycle;
		}
			
		
		SAVE_INT_AND_CLI(flags);
		SMP_LOCK(flags);	

		// < skb virt addr || skb->data phys addr || skb->len || vap_id > 

		data = t2h_addr_transfer(*((unsigned int*)msg_data + 1));

		if(data == NULL)
		{
			mem_dump("data = ", msg_data, msg_size);
			goto recycle;
		}
		
		pkt_size = *((unsigned int*)msg_data + 2);

		if(pkt_size == 0)
		{
			panic_printk("pkt_size is NULL\r\n");
			goto recycle;
		}

		pskb = dev_alloc_skb(RX_BUF_LEN);
		if (pskb != NULL) 
		{
			pskb->dev = priv->dev;
			skb_put(pskb, pkt_size);
			memcpy(pskb->data,data,pkt_size);
		}
		else
		{
			panic_printk("allocate skb fail\r\n");
			goto recycle;
		}

		if ((IP_MCAST_MAC(pskb->data) && IS_IGMP_PROTO(pskb->data)) || IPV6_MCAST_MAC(pskb->data)) {
			//mem_dump("IGMP rx_pkt_send_cmd", pskb->data, 48);
			//skb_pull(pskb, 14);			
			//panic_printk("MCAST & IGMP rx_pkt_send_cmd\n");
			skb_reset_mac_header(pskb);
			#ifdef _FULLY_WIFI_IGMP_SNOOPING_SUPPORT_
			extern void check_igmp_snooping_pkt(struct sk_buff *pskb);
			check_igmp_snooping_pkt( pskb);
			#endif

			#ifdef _FULLY_WIFI_MLD_SNOOPING_SUPPORT_
			extern void check_mld_snooping_pkt(struct sk_buff *pskb);
			check_mld_snooping_pkt( pskb);
			#endif
			rtl_kfree_skb(priv, pskb, _SKB_TX_);
			goto recycle;
		} else 
//TODO
#if 0//(defined(MBSSID) || defined(UNIVERSAL_REPEATER))
		if(IS_BCAST2(data))
		{
			int i;
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) 
			{
				if (IS_DRV_OPEN(priv->pvap_priv[i])) 
				{
					priv_vap = priv->pvap_priv[i];
					newskb = skb_clone(pskb, GFP_ATOMIC);
					newskb->dev = priv_vap->dev;
					if (rtl8192cd_start_xmit(newskb, priv_vap->dev)){			
						rtl_kfree_skb(priv, newskb, _SKB_TX_);
					}
				}
			}
			if (rtl8192cd_start_xmit(pskb, priv->dev)){			
				rtl_kfree_skb(priv, pskb, _SKB_TX_);
			}
		}
		else
#endif
		{
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
			vap_id = *((unsigned int*)msg_data+3);
			if(vap_id == 0xAAAAAAAA)
			{
				priv = priv->pvxd_priv;
				//panic_printk("TX VXD \r\n");
			}
			else if(vap_id != 0xFFFFFFFF)
			{
				priv = priv->pvap_priv[vap_id];
				//panic_printk("TX VAP id = %d \r\n", vap_id);
			}
			//else
			//	panic_printk("TX ROOT \r\n");
#endif

			if (rtl8192cd_start_xmit(pskb, priv->dev)){			
				rtl_kfree_skb(priv, pskb, _SKB_TX_);
			}
		}
		
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
	}

recycle:
	//RECYCLE 
	tx_pkt_recycle(msg_data);

	return 0;
}


int rx_pkt_recv_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	if(netif_running(pe_offload_dev))
	{
		struct rtl8192cd_priv *priv = pe_offload_priv; 
		unsigned int pkt_size;
		unsigned char *data = NULL;
		struct sk_buff	*pskb = NULL;
		unsigned long flags = 0;

		if(priv == NULL)
		{
			panic_printk("priv is NULL\r\n");
			goto recycle;
		}

		if(!priv->pe_ready)
		{
			//panic_printk("pe is not ready");
			goto recycle;
		}
		
		SAVE_INT_AND_CLI(flags);
		SMP_LOCK(flags);	
		
		//	panic_printk("skb = %x, skb->data = %x, skb->len = %d\r\n", ((int*)msg_data)[0], ((int*)msg_data)[1], ((int*)msg_data)[2]);

		// < skb virt addr || skb->data phys addr || skb->len > 

		data = t2h_addr_transfer(*((unsigned int*)msg_data + 1));

		if(data == NULL)
		{
			mem_dump("data = ", msg_data, msg_size);
			goto recycle;
		}
			
		pkt_size = *((unsigned int*)msg_data + 2);

		if(pkt_size < 1600){
			pskb = dev_alloc_skb(RX_BUF_LEN);
			if (pskb != NULL) {
				pskb->dev = priv->dev;
				init_rxdesc_88XX(priv, pskb, 0, NULL, NULL);
				skb_put(pskb, pkt_size);
				memcpy(pskb->data,data,pkt_size);

				rtl88XX_ipc_rx_isr(priv, pskb);
			}else{
				GDEBUG("!error pskb is null\n");
			}
		} else {
			;//GDEBUG("[%s] wrong pkt(size:%d)\n", __FUNCTION__, pkt_size);
		}

		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
	}
	
	//RECYCLE
recycle:

 	tx_pkt_recycle(msg_data);
	return 0;
}

struct rx_info_idx sw_idx;

int rx_rxinfo_report_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	//panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

#if 0

	/*
	*   30 RX_INFOs through one IPC MSG , total packet buffer is 1080 bytes
	*   Each Rx_INFO is pktinfo (4bytes) | driver_info (32bytes)
	*/
	if(!netif_running(pe_offload_dev))
		return -1;
	
	struct rtl8192cd_priv *priv = pe_offload_priv;
	unsigned char *rx_info_buf = NULL;
	struct stat_info *pstat = NULL, *pre_stat = NULL;
	unsigned int size = 0;
	unsigned int i = 0;

	if(!priv->pe_ready)
	{
		panic_printk("pe is not ready");
		return -1;
	}

	//for rx info parsing
	unsigned int aid, rate, pre_aid=0, offset=0;	
	struct rx_frinfo *pfrinfo = kmalloc(sizeof(struct rx_frinfo), GFP_ATOMIC);
	unsigned char *driverinfo_buf = kmalloc(DRIVER_INFO_SIZE, GFP_ATOMIC);

	if(pfrinfo == NULL || driverinfo_buf == NULL){
		panic_printk("%s:%d: pfrinfo/driverinfo_buf cant get buffer !!\n", __FUNCTION__, __LINE__);
		return -1;
	}
		
	rx_info_buf = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+0));

	if(rx_info_buf == NULL)
	{
		mem_dump("data = ", msg_data, msg_size);
		return -1;
	}
	
	size = *((unsigned int*)msg_data+1);
	//panic_printk("rx_info_buf=%x size=%d check_sum=%x\n", rx_info_buf, size, t2h_data_check_sum(rx_info_buf, size));

	extern unsigned char rxinfo_buf[NUM_RXINFO_PER_REPORT * RXINFO_SIZE_PER_PACKET];;
	memcpy(rxinfo_buf, rx_info_buf, size);

	//mem_dump("rxinfo_buf=", rxinfo_buf, 86);

	offset = AID_SIZE+RATE_SIZE+RXRATE_SIZE+RXBW_SIZE+RXSPLCP_SIZE;

	//| aid | rate | phy info | drive info |	
	for(i=0; i < NUM_RXINFO_PER_REPORT; i++){

		memset(pfrinfo, 0x0, sizeof(struct rx_frinfo));			
		offset = RXINFO_SIZE_PER_PACKET*i;

		
		aid = rxinfo_buf[offset+0];
		rate = rxinfo_buf[offset+1];
		pfrinfo->rx_rate = rxinfo_buf[offset+2];
		pfrinfo->rx_bw = rxinfo_buf[offset+3];
		pfrinfo->rx_splcp = rxinfo_buf[offset+4];

		offset += (AID_SIZE+RATE_SIZE+RXRATE_SIZE+RXBW_SIZE+RXSPLCP_SIZE);			
		
		if(aid > 0 && aid == pre_aid)
			pstat = pre_stat;
		else
			pstat = get_stainfo_from_aidarray(priv, aid);
		
		if(pstat){			
			memcpy(&pfrinfo->phy_info, (void *)(rxinfo_buf+offset), PHY_INFO_SIZE);
			memcpy(driverinfo_buf, (void *)(rxinfo_buf+offset+PHY_INFO_SIZE), DRIVER_INFO_SIZE);
			pfrinfo->driver_info = (struct RxFWInfo *)driverinfo_buf;
			pfrinfo->physt = TRUE;
			
			pe_translate_rssi_sq_outsrc(pstat, priv, pfrinfo, rate);
			update_sta_rssi(priv, pstat, pfrinfo);

			pre_aid = aid;
			pre_stat = pstat;
			pstat = NULL;
		}
	}
	
	kfree(pfrinfo);
	kfree(driverinfo_buf);
#else
	if(netif_running(pe_offload_dev))
	{
		struct pe_addr_base* pe_addr = get_pe_addr_base();
		struct pe_controlblock *pe_cb = pe_addr->pe_reserved_virt_addr;
		struct rtl8192cd_priv *priv = pe_offload_priv; 
		
		if(!priv->pe_ready)
		{
			//panic_printk("pe is not ready");
			return -1;
		}

		if(sw_idx.rx_info_wdx == sw_idx.rx_info_rdx)
		{
			//read index
			sw_idx.rx_info_wdx = pe_cb->rx_info.rx_info_wdx;
			if(sw_idx.rx_info_wdx == sw_idx.rx_info_rdx)
				return -1;
		}

		if(sw_idx.rx_info_wdx >= RX_INFO_ARRAY_SIZE)
		{
			panic_printk("rx_info_wdx error %d\r\n", sw_idx.rx_info_wdx);
			return -1;
		}

		//panic_printk("rdx = %d, wdx = %d\r\n", sw_idx.rx_info_rdx, sw_idx.rx_info_wdx);

		if(1)
		{
			unsigned int aid, rate, pre_aid = 0, offset = 0;	
			unsigned char *rxinfo_buf = NULL;
			struct stat_info *pstat = NULL, *pre_stat = NULL;
			unsigned int read_cnt = 0;
			int retry_cnt = 0;
			
			struct rx_frinfo *pfrinfo = kmalloc(sizeof(struct rx_frinfo), GFP_ATOMIC);
			unsigned char *driverinfo_buf = kmalloc(DRIVER_INFO_SIZE, GFP_ATOMIC);

			if(pfrinfo == NULL || driverinfo_buf == NULL){
				panic_printk("%s:%d: pfrinfo/driverinfo_buf cant get buffer !!\n", __FUNCTION__, __LINE__);
				return -1;
			}

			while(sw_idx.rx_info_rdx != sw_idx.rx_info_wdx)
			{
				rxinfo_buf = &pe_cb->rx_info.rx_info_buf[sw_idx.rx_info_rdx];
				
				memset(pfrinfo, 0x0, sizeof(struct rx_frinfo));			
				memset(driverinfo_buf, 0x0, DRIVER_INFO_SIZE);
				offset = 0;

				aid = rxinfo_buf[offset+0];
				rate = rxinfo_buf[offset+1];
				pfrinfo->rx_rate = rxinfo_buf[offset+2];
				pfrinfo->rx_bw = rxinfo_buf[offset+3];
				pfrinfo->rx_splcp = rxinfo_buf[offset+4];

				offset += (AID_SIZE+RATE_SIZE+RXRATE_SIZE+RXBW_SIZE+RXSPLCP_SIZE);			
			
				if(aid > 0 && aid == pre_aid)
					pstat = pre_stat;
				else
					pstat = get_stainfo_from_aidarray(priv, aid);
				
				if(pstat){		
					memcpy(&pfrinfo->phy_info, (void *)(rxinfo_buf+offset), PHY_INFO_SIZE);
					memcpy(driverinfo_buf, (void *)(rxinfo_buf+offset+PHY_INFO_SIZE), DRIVER_INFO_SIZE);
					pfrinfo->driver_info = (struct RxFWInfo *)driverinfo_buf;
					pfrinfo->physt = TRUE;
					
					pe_translate_rssi_sq_outsrc(pstat, priv, pfrinfo, rate);
					update_sta_rssi(priv, pstat, pfrinfo);

					pre_aid = aid;
					pre_stat = pstat;
					pstat = NULL;
				}

				read_cnt++;

				sw_idx.rx_info_rdx++;
				if(sw_idx.rx_info_rdx == RX_INFO_ARRAY_SIZE)
					sw_idx.rx_info_rdx = 0;

				retry_cnt++;
				if(retry_cnt > 2000)
				{
					panic_printk(" %s : LOOP\r\n", __func__);
					break;
				}

			}

			kfree(pfrinfo);
			kfree(driverinfo_buf);

			//update read index
			pe_cb->rx_info.rx_info_rdx = sw_idx.rx_info_rdx;
			
			//panic_printk("read rx info num = %d\r\n", read_cnt);
		}
	}
#endif
	return 0;
}

#if 0
int rx_counter_report_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);
	if(netif_running(pe_offload_dev))
	{
	}
	return 0;
}
#endif

bool do_issue_add_ba_req(struct rtl8192cd_priv *priv, void* msg_data)
{
	unsigned char mac_addr[ETH_ALEN];
	unsigned char tid;
	struct stat_info *pstat ;
	
	memcpy(mac_addr,((unsigned char*)msg_data+2),ETH_ALEN);
	tid = *((unsigned char*)msg_data+8);
	
	pstat = get_stainfo(priv,mac_addr);
	
	if(pstat!=NULL){
		issue_ADDBAreq(priv, pstat, tid);
		return TRUE;
	}
	else{
		printk("error: pstat is null \n");
		return FALSE;
	}
}

bool do_issue_deauth(struct rtl8192cd_priv *priv, void* msg_data)
{
	unsigned char mac_addr[ETH_ALEN];
	int reason;
	struct stat_info *pstat ;
	
	memcpy(mac_addr,((unsigned char*)msg_data+2),ETH_ALEN);
	reason = *(int*)((unsigned int)msg_data+8);
		
	issue_deauth(priv, mac_addr, reason);
	return TRUE;
}

bool do_mac_clone(struct rtl8192cd_priv *priv, void* msg_data)
{
	unsigned char mac_addr[ETH_ALEN];
	
	memcpy(mac_addr,((unsigned char*)msg_data+2),ETH_ALEN);

	mac_clone(priv, mac_addr);
	priv->macclone_completed = 1;
	indicate_wsc_mac_addr_has_changed(priv);
	return TRUE;
}

bool do_add_a4_client(struct rtl8192cd_priv *priv, void* msg_data)
{
	unsigned char mac_addr[ETH_ALEN];
	struct stat_info *pstat ;
	
	memcpy(mac_addr,((unsigned char*)msg_data+2),ETH_ALEN);
	
	pstat = get_stainfo(priv,mac_addr);
	
	if(pstat!=NULL){
#ifdef A4_STA
		add_a4_client(priv, pstat);
#endif
		return TRUE;
	}
	else{
		printk("error: pstat is null \n");
		return FALSE;
	}
}

bool do_dummy(struct rtl8192cd_priv *priv, void* msg_data)
{
}

int rx_func_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	
	if(netif_running(pe_offload_dev))
	{
		struct rtl8192cd_priv *priv = pe_offload_priv;	//hardcode for temporary
		unsigned long flags = 0;
		
		if(priv == NULL)
		{
			panic_printk("priv is NULL\r\n");
		}
		
		SAVE_INT_AND_CLI(flags);
		SMP_LOCK(flags);	
	
		unsigned char sub_type = *((unsigned char*)msg_data+0);

#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
		unsigned char vap_id = *((unsigned char*)msg_data+1);
		if(vap_id == 0xAA)
			priv = priv->pvxd_priv;
		else if(vap_id != 0xFF)
			priv = priv->pvap_priv[vap_id];
#endif

		if((func_handler[sub_type].do_func(priv, msg_data))!=TRUE)
			GDEBUG("run function fail (func id=%d) \n",sub_type);

		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
	}
	
	return 0;
}

int rx_pe_stainfo_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	//panic_printk( "async callback receives msg_no[%d] message[0x%x] size[0x%x]\n", msg_no, ((unsigned int*)msg_data),*msg_size);	

	if(!netif_running(pe_offload_dev))
		return -1;
	
	struct rtl8192cd_priv *priv = pe_offload_priv;
	unsigned char *pe_stat = NULL, *pestainfo_t2h_list = NULL, *mac = NULL;
	struct stat_info *pstat = NULL;
	unsigned int size = 0;
	int ret = 0;
#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	unsigned int vap_id = 0;
#endif

	pe_stat = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+0));
	mac = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+1));	
	pestainfo_t2h_list = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+2));
	size = *((unsigned int*)msg_data+3);

#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	vap_id = *((unsigned int*)msg_data+4);
	if(vap_id == 0xAAAAAAAA)
	{
		priv = priv->pvxd_priv;
	}
	else if(vap_id != 0xFFFFFFFF)
	{
		priv = priv->pvap_priv[vap_id];
	}
#endif


	if(pe_stat == NULL || mac == NULL || pestainfo_t2h_list == NULL)
	{
		panic_printk("[%s] t2h addr transfer error\n", __FUNCTION__);
		return 0;
	} else{
		panic_printk("mac=%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		panic_printk("size=%d\n", size);
	}

	//mem_dump("data=", pestainfo_t2h_list, 96);

	pstat = get_stainfo(priv, mac);
	if (pstat != NULL){
		pstat->pstat_pe = pe_stat;
		if(pestainfo_t2h_list != NULL && size !=0){
			//pstat->pe_stainfo_t2h_list = (unsigned char *)pestainfo_t2h_list;
			pstat->pe_stainfo_t2h_list = kmalloc(size, GFP_ATOMIC);
			if(pstat->pe_stainfo_t2h_list)
			{
				memcpy(pstat->pe_stainfo_t2h_list, pestainfo_t2h_list, size);
				//verify
				ret = pe_stainfo_sync(priv, pstat);
				if(!ret)
				{
					panic_printk("ERROR : memory issue\r\n");
					//recovery
					if(priv->pe_stainfo_t2h_list != NULL)
						memcpy(pstat->pe_stainfo_t2h_list, priv->pe_stainfo_t2h_list, size);
				}
				//backup one
				if(priv->pe_stainfo_t2h_list == NULL && ret)
				{
					priv->pe_stainfo_t2h_list = kmalloc(size, GFP_ATOMIC);
					if(priv->pe_stainfo_t2h_list != NULL)
						memcpy(priv->pe_stainfo_t2h_list, pstat->pe_stainfo_t2h_list, size);
					else
						panic_printk("%d : allocate pe_stainfo_t2h_list fail \r\n", __LINE__);
				}
			}
			else
				panic_printk("%d : allocate pe_stainfo_t2h_list fail \r\n", __LINE__);
		}
	} else {
		printk("Error : cannot find match pstat\r\n");
		return 0;
	}

	//pe_stainfo_sync(priv, pstat);
	
	return 0;
}

int rx_pe_priv_param_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	//panic_printk( "async callback receives msg_no[%d] message[0x%x] size[0x%x]\n", msg_no, ((unsigned int*)msg_data),*msg_size);	

	if(!netif_running(pe_offload_dev))
		return -1;
	
	struct rtl8192cd_priv *priv = pe_offload_priv;
	unsigned char *pe_net_stats = NULL, *pe_ext_stats = NULL, *pe_privparam_t2h_list = NULL, *pe_pshare_phw = NULL;
	unsigned int size = 0;
	unsigned int vap_id = 0;

#if (defined(MBSSID) || defined(UNIVERSAL_REPEATER))
	vap_id = *((unsigned int*)msg_data+5);
	if(vap_id == 0xAAAAAAAA)
	{
		panic_printk("Set VXD %u\r\n", vap_id);
		priv = priv->pvxd_priv;
	}
	else if(vap_id != 0xFFFFFFFF)
	{
		panic_printk("Set VAP %u\r\n", vap_id);
		priv = priv->pvap_priv[vap_id];
	}
#endif

	if(vap_id == 0xFFFFFFFF){
		pe_net_stats = (unsigned char *)t2h_sram_addr_transfer(*((unsigned int*)msg_data+0));//priv->net_stats
		pe_ext_stats = (unsigned char *)t2h_sram_addr_transfer(*((unsigned int*)msg_data+1));//priv->ext_stats
		pe_pshare_phw = (unsigned char *)t2h_sram_addr_transfer(*((unsigned int*)msg_data+2));//priv->pshare->phw
		pe_privparam_t2h_list = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+3));
		size = *((unsigned int*)msg_data+4);
	}else{
		pe_net_stats = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+0));//priv->net_stats
		pe_ext_stats = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+1));//priv->ext_stats
		pe_pshare_phw = (unsigned char *)t2h_sram_addr_transfer(*((unsigned int*)msg_data+2));//priv->pshare->phw
		pe_privparam_t2h_list = (unsigned char *)t2h_addr_transfer(*((unsigned int*)msg_data+3));
		size = *((unsigned int*)msg_data+4);
	}
	
	if(pe_net_stats == NULL || pe_ext_stats == NULL || 
		pe_privparam_t2h_list == NULL || pe_pshare_phw == NULL)
	{
		panic_printk("[%s] t2h addr transfer error\n", __FUNCTION__);
		return 0;
	} else{		
		//panic_printk("size=%d\n", size);

		priv->pe_net_stats = (unsigned char*)pe_net_stats;
		priv->pe_ext_stats = (unsigned char*)pe_ext_stats;
		priv->pe_pshare_phw = (unsigned char*)pe_pshare_phw; 
		
		if(size!=0 && pe_privparam_t2h_list != NULL){
			priv->pe_privparam_t2h_list = kmalloc(size, GFP_ATOMIC);
			memcpy(priv->pe_privparam_t2h_list, pe_privparam_t2h_list, size);

			//mem_dump("data=", pe_privparam_t2h_list, 96);
		}
	}	
	
	return 0;
}


int rx_debug_cmd(ca_ipc_addr_t peer, ca_uint16_t msg_no, ca_uint16_t trans_id ,const void *msg_data,
				ca_uint16_t* msg_size)
{
	//panic_printk( "async callback receives msg_no[%d] message[%s] size[%d]\n", msg_no,(const char *)msg_data,*msg_size);

	if(*(unsigned char*)msg_data == 0x1)
		rx_sta_handler();
	//panic_printk("%d\r\n", *(unsigned char*)msg_data);
	return 0;
}

void check_sta_state(struct rtl8192cd_priv *priv)
{
	struct list_head	*plist, *phead;
	unsigned char sta_state = 0;
	struct stat_info	*pstat;
	int retry_cnt = 0;
	
	phead = &priv->asoc_list;
	plist = phead->next;
	while(plist != phead)
	{
		pstat = list_entry(plist, struct stat_info, asoc_list);
		plist = plist->next;

		if(list_empty(&pstat->asoc_list))
			break;
	
		sta_state = pe_sta_state_get(pstat->cmn_info.aid);
		if((!(pstat->state & WIFI_SLEEP_STATE) && sta_state) ||
			((pstat->state & WIFI_SLEEP_STATE) && !sta_state))
			{
				//panic_printk("aid %d , state change from %d\r\n",pstat->cmn_info.aid, (pstat->state & WIFI_SLEEP_STATE));
				pe_pwr_state(priv, pstat, sta_state);
			}
		if(sta_state)
		{
			int sta_hw_queue_state = pe_sta_hw_queue_state_get(pstat->cmn_info.aid);
			if(sta_hw_queue_state)
			{
				//panic_printk("aid %d , hw queue set \r\n",pstat->cmn_info.aid);
				pe_sta_hw_queue_set(priv, pstat);
			}
		}

		retry_cnt++;
		if(retry_cnt > 1000)
		{
			panic_printk(" %s : LOOP\r\n", __func__);
			break;
		}
	}

}

void rx_sta_handler(void)
{
	if(netif_running(pe_offload_dev))
	{
		struct rtl8192cd_priv *priv = pe_offload_priv;

		unsigned long flags = 0;

		SAVE_INT_AND_CLI(flags);
		SMP_LOCK(flags);	
		
		check_sta_state(priv);
		
#if defined(MBSSID)
		{
			int i;
			struct rtl8192cd_priv *priv_vap = NULL;
			for (i=0; i<RTL8192CD_NUM_VWLAN; i++) 
			{
				if (IS_DRV_OPEN(priv->pvap_priv[i])) 
				{
					priv_vap = priv->pvap_priv[i];
					check_sta_state(priv_vap);
				}
			}
		}
#endif
		RESTORE_INT(flags);
		SMP_UNLOCK(flags);
	}
}

ca_ipc_msg_handle_t invoke_procs_wfo[]=
{
	{ .msg_no=WFO_IPC_T2H_PE_READY, 	.proc=rx_pe_ready_cmd },
	{ .msg_no=WFO_IPC_T2H_PKT_RECV, 	.proc=rx_pkt_recv_cmd },
	{ .msg_no=WFO_IPC_T2H_PKT_SEND, 	.proc=rx_pkt_send_cmd },	
	{ .msg_no=WFO_IPC_T2H_RXINFO_REPORT, 	.proc=rx_rxinfo_report_cmd },
//	{ .msg_no=WFO_IPC_T2H_COUNTER_REPORT, 	.proc=rx_counter_report_cmd },
	{ .msg_no=WFO_IPC_T2H_FUNC, 		.proc=rx_func_cmd },
	{ .msg_no=WFO_IPC_T2H_ADD_STAINFO, 	.proc=rx_pe_stainfo_cmd },
	{ .msg_no=WFO_IPC_T2H_ADD_PRIV_PARAM, 	.proc=rx_pe_priv_param_cmd },	
	{ .msg_no=WFO_IPC_T2H_DEBUG, 		.proc=rx_debug_cmd },
};

int register_wfo_ipc(void)
{
	int rc;
	//ca_uint16_t result_size = 0x10;
	panic_printk("Register IPC \r\n");

	//INIT_LIST_HEAD(&queued_ipc_cmd_list);
	
	rc= ca_ipc_msg_handle_register(CA_IPC_SESSION_WFO, invoke_procs_wfo, WFO_IPC_T2H_MAX);
	if( CA_E_OK != rc )
	{
		printk("%s Register Failed :%d \n", __FILE__,__LINE__);
		return 1;
	}

	/* Register STA handler */
	rc= ca_ipc_sta_handle_register(rx_sta_handler);
	if( CA_E_OK != rc )
	{
		printk("%s Register Failed :%d \n", __FILE__,__LINE__);
		return 1;
	}
	return 0;
}

