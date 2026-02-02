#ifdef BCM_MCAST_SUPPORT
#include <netinet/icmp6.h>
#include <linux/netlink.h>
#include "bcm_acceleration.h"
#include "membership.h"


extern mcast_proxy mproxy;
char *g_sock_buff[BCM_MCAST_NL_RX_BUF_SIZE] = {0};
typedef T_BCM_MCAST_PKT_INFO T_MCPD_PKT_INFO;
T_MCPD_PKT_INFO * pkt_info = NULL;
T_MCPD_PKT_INFO * pkt6_info = NULL;

int bcm_mcast_api_socket_create(int *nl_sock, int portid)
{
    struct sockaddr_nl src_addr;
    int                sd;
    int                rc;

    if ( NULL == nl_sock )
    {
       return -1;
    }

    *nl_sock = -1;
    sd = socket(PF_NETLINK, SOCK_RAW, NETLINK_BCM_MCAST);
    if(sd < 0)
    {
        return sd;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = portid;
    src_addr.nl_groups = 0;
    rc = bind(sd, (struct sockaddr *)&src_addr, sizeof(src_addr));
    if(rc < 0)
    {
        close(sd);
        return -1;
    }

    *nl_sock = sd;
    return 0;
} /* bcm_mcast_api_socket_create */

int bcm_mcast_api_nl_recv(int nl_sock, char *rx_buf, int rx_bufsize, bcm_mcast_api_rcv_func process_func)
{
   int                   recvLen;
   struct iovec          iov;
   struct msghdr         msg;
   struct nlmsghdr      *nl_msgHdr;
   unsigned char        *pdata;
   struct sockaddr_nl    sa;
   int                   data_len;

   if ( (nl_sock < 0) ||
        (NULL == rx_buf) ||
        (0 == rx_bufsize) || 
        (NULL == process_func) )
   {
      return -EINVAL;
   }
   memset(&msg, 0, sizeof(struct msghdr));
   msg.msg_name    = (void*)&sa;
   msg.msg_namelen = sizeof(sa);
   msg.msg_iov     = &iov;
   msg.msg_iovlen  = 1;
 
   iov.iov_base = (void *)rx_buf;
   iov.iov_len = rx_bufsize;
   recvLen = recvmsg(nl_sock, &msg, 0);
   if(recvLen <= 0)
   {
       return -EIO;
   }

   if (msg.msg_flags & MSG_TRUNC)
   {
      return -ENOMEM;
   }

   for(nl_msgHdr = (struct nlmsghdr *)rx_buf; 
       NLMSG_OK(nl_msgHdr, (unsigned int)recvLen);
       nl_msgHdr = NLMSG_NEXT(nl_msgHdr, recvLen))
   {
      if ((nl_msgHdr->nlmsg_type == NLMSG_NOOP) ||
          (nl_msgHdr->nlmsg_type == NLMSG_DONE) ||
          (nl_msgHdr->nlmsg_type == NLMSG_OVERRUN) )
      {
         continue;
      }

      if (nl_msgHdr->nlmsg_type == NLMSG_ERROR)
      {
         struct nlmsgerr *errmsg = NLMSG_DATA(nl_msgHdr);
         printErr("ERROR Message %d, sent by portid %x generated an error %d\n",
                errmsg->msg.nlmsg_type, errmsg->msg.nlmsg_pid, errmsg->error);
         continue;
      }

      if ((nl_msgHdr->nlmsg_type < BCM_MCAST_MSG_BASE) ||
          (nl_msgHdr->nlmsg_type >= BCM_MCAST_MSG_MAX))
      {
         continue;
      }

      pdata = NLMSG_DATA(nl_msgHdr);
      data_len = nl_msgHdr->nlmsg_len;
      process_func(nl_msgHdr->nlmsg_type, pdata, data_len);
   }
   return 0;
}

static int bcm_mcast_api_nl_tx(int sock_nl_in, int msg_type, unsigned char *pBuf, unsigned int len, unsigned char *pBuf2, unsigned int len2, int set)
{
   struct sockaddr_nl addr, dest_addr;
   socklen_t          addrLen = sizeof(addr);
   unsigned int       bufSize;
   struct nlmsghdr   *nlh;
   struct iovec       iov[5];
   int                iovcnt;
   struct msghdr      msg;
   int                ret = 0;
   int                sock_nl_local;
   char               buf[NLMSG_HDRLEN];
   char               pad[NLMSG_ALIGNTO];

   if ( ((len > 0)  && (NULL == pBuf)) || 
        ((len == 0) && (NULL != pBuf)) )
   {
       return -EINVAL;
   }

   if ((len2 > 0)  && (NULL == pBuf2))
   {
       return -EINVAL;
   }

   if ( sock_nl_in < 0 )
   {
      if ( bcm_mcast_api_socket_create(&sock_nl_local, 0) < 0 )
      {
         return -EIO;
      }
   }
   else
   {
      sock_nl_local = sock_nl_in;
   }

   /* retrieve the port id from the socket so it can be used in nlh */
   memset(&addr, 0, sizeof(struct sockaddr_nl));
   if ( getsockname(sock_nl_local, (struct sockaddr *)&addr, &addrLen) < 0)
   {
      return -EINVAL;
   }
   if ( 0 == addr.nl_pid )
   {
      return -EINVAL;
   }

   memset(&dest_addr, 0, sizeof(struct sockaddr_nl));
   dest_addr.nl_family = AF_NETLINK;
   dest_addr.nl_pid    = 0;
   dest_addr.nl_groups = 0;

   /* Fill the netlink message header */
   bufSize = NLMSG_SPACE(len+len2);
   memset(&buf[0], 0, NLMSG_HDRLEN);
   nlh = (struct nlmsghdr *)&buf[0];
   nlh->nlmsg_len   = bufSize;
   nlh->nlmsg_type  = msg_type;
   nlh->nlmsg_seq   = 0;
   nlh->nlmsg_pid   = addr.nl_pid;
   nlh->nlmsg_flags = NLM_F_REQUEST;
   if ( set ) 
   {
      nlh->nlmsg_flags |= NLM_F_CREATE|NLM_F_REPLACE;
   }

   memset(&iov, 0, sizeof(iov));
   memset(&msg, 0, sizeof(msg));
   iovcnt = 0;
   iov[iovcnt].iov_base = (void *)nlh;
   iov[iovcnt].iov_len  = NLMSG_HDRLEN;
   iovcnt++;
   iov[iovcnt].iov_base = (void *)pBuf;
   iov[iovcnt].iov_len  = len;
   iovcnt++;

   if (len2)
   {
      iov[iovcnt].iov_base = (void *)pBuf2;
      iov[iovcnt].iov_len  = len2;
      iovcnt++;
   }

   /* make sure message is aligned */
   if ( bufSize > (NLMSG_HDRLEN + len + len2) )
   {
      iov[iovcnt].iov_base = (void *)&pad;
      iov[iovcnt].iov_len  = bufSize - (NLMSG_HDRLEN + len + len2);
      iovcnt++;
   }
   msg.msg_name       = (void *)&dest_addr;
   msg.msg_namelen    = sizeof(dest_addr);
   msg.msg_iov        = iov;
   msg.msg_iovlen     = iovcnt;
   msg.msg_control    = NULL;
   msg.msg_controllen = 0;
   msg.msg_flags      = 0;

   if (sendmsg(sock_nl_local, &msg, 0) < 0)
   {
      ret = -EIO;
   }

   if ( sock_nl_in < 0 )
   {
      close(sock_nl_local);
   }

   return ret;
} /* bcm_mcast_api_nl_tx */

int bcm_mcast_api_register(int sock_nl, int primary)
{
   T_BCM_MCAST_REGISTER msg;

   msg.primary = primary;
   msg.result = 0;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_REGISTER, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_register */

int bcm_mcast_api_unregister(int sock_nl, int primary)
{
   T_BCM_MCAST_REGISTER msg;

   msg.primary = primary;
   msg.result = 0;

   return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_UNREGISTER, (unsigned char *)&msg, sizeof(msg), NULL, 0, 1);
} /* bcm_mcast_api_unregister */

int bcm_mcast_api_send_group_timeout (int sock_nl, int proto, int generalMembershipTimeoutSecs)
{
  T_BCM_MCAST_TIMEOUT_ENTRY timeoutEntry;
  timeoutEntry.proto = proto;
  timeoutEntry.generalMembershipTimeoutSecs = generalMembershipTimeoutSecs;

  return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_SET_TIMEOUT, 
                             (unsigned char *) &timeoutEntry, sizeof(timeoutEntry), 
                             NULL, 0, 1 );
}

int mcpd_netlink_init(void)
{
    /* create socket with portid "MCPD" */
    if ( bcm_mcast_api_socket_create(&mproxy.mcpd_netlink_socket, 0x4D435044) < 0 )
    {
        mproxy.mcpd_netlink_socket = -1;
        return MCPD_RET_GENERR;
    }

    /* register for notifications */
    bcm_mcast_api_register(mproxy.mcpd_netlink_socket, 0);

    return MCPD_RET_OK;
} /* mcpd_netlink_init */

int mcpd_netlink_shutdown(void)
{
    bcm_mcast_api_unregister(mproxy.mcpd_netlink_socket, 0);

    close(mproxy.mcpd_netlink_socket);

    return MCPD_RET_OK;
} /* mcpd_netlink_shutdown */

void mcpd_update_multicast_info()
{
    int snoopagingtime=0;

    snoopagingtime = TIMER_GMI;
    /* Set snoop aging timeout in multicast driver snooping table */
    bcm_mcast_api_send_group_timeout (mproxy.mcpd_netlink_socket, IPPROTO_IGMP,
                                      snoopagingtime);

#ifdef ENABLE_IMP_MLD

    bcm_mcast_api_send_group_timeout (mproxy.mcpd_netlink_socket, IPPROTO_ICMP,
                                      snoopagingtime);
#endif

}

void mcpd_netlink_process_msg(int type, unsigned char *pdata, int data_len)
{

    MCPD_TRACE(MCPD_TRC_LOG, "Received message of type %d", type);

    switch(type)
    {
        case BCM_MCAST_MSG_UNREGISTER:
            break;
        case BCM_MCAST_MSG_IGMP_PKT:
            MCPD_TRACE(MCPD_TRC_LOG, "BCM_MCAST_MSG_IGMP_PKT");
            pkt_info = (T_BCM_MCAST_PKT_INFO *)pdata;
            /* printWar("pkt_info->parent_ifi=%d %s pkt_info->rxdev_ifi=%d %s pkt_info->to_acceldev_ifi=%d %s pkt_info->tci=%d pkt_info->lan_ppp=%d pkt_info->ipv4rep.s_addr=%s rep_MAC=%02x-%02x-%02x-%02x-%02x-%02x\n", 
                pkt_info->parent_ifi, parent_name, pkt_info->rxdev_ifi, rx_name, pkt_info->to_acceldev_ifi, tx_name, pkt_info->tci, pkt_info->lan_ppp, imp_inet_ntoa(pkt_info->ipv4_rep.s_addr),
                pkt_info->rep_mac[0], pkt_info->rep_mac[1], pkt_info->rep_mac[2], pkt_info->rep_mac[3], pkt_info->rep_mac[4], pkt_info->rep_mac[5]);*/
            break;

#ifdef ENABLE_IMP_MLD
        case BCM_MCAST_MSG_MLD_PKT:
            MCPD_TRACE(MCPD_TRC_LOG, "BCM_MCAST_MSG_MLD_PKT");
            pkt6_info = (T_BCM_MCAST_PKT_INFO *)pdata;
            /*printWar("pkt_info->parent_ifi=%d %s pkt_info->rxdev_ifi=%d %s pkt_info->to_acceldev_ifi=%d %s pkt_info->tci=%d pkt_info->lan_ppp=%d pkt_info->ipv6_rep.s_addr=%s rep_MAC=%02x-%02x-%02x-%02x-%02x-%02x\n", 
                pkt_info->parent_ifi, parent_name, pkt_info->rxdev_ifi, rx_name, pkt_info->to_acceldev_ifi, tx_name, pkt_info->tci, pkt_info->lan_ppp, imp_inet6_ntoa(pkt_info->ipv6_rep.s6_addr),
                pkt_info->rep_mac[0], pkt_info->rep_mac[1], pkt_info->rep_mac[2], pkt_info->rep_mac[3], pkt_info->rep_mac[4], pkt_info->rep_mac[5]);*/
            break;
#endif

        case BCM_MCAST_MSG_IGMP_PURGE_ENTRY:
            pkt_info = NULL;
            break;

        case BCM_MCAST_MSG_MLD_PURGE_ENTRY:
            pkt6_info = NULL;
            break;

        default:
           pkt_info = NULL;
           pkt6_info = NULL;
           MCPD_TRACE(MCPD_TRC_ERR, "mcpd_nl msg %d not supported", type);
           break;
    }

    return;
} /* mcpd_netlink_process_msg */

int mcpd_netlink_recv_mesg(void)
{
    if ( bcm_mcast_api_nl_recv(mproxy.mcpd_netlink_socket, g_sock_buff, BCM_MCAST_NL_RX_BUF_SIZE, mcpd_netlink_process_msg) < 0 )
    {
        MCPD_TRACE(MCPD_TRC_ERR,"Error receiving message\n");
        return MCPD_RET_GENERR;
    }

    return MCPD_RET_OK;
} /* mcpd_netlink_recv_mesg */

//TX
/*
 * Add/Update IGMP snooping entry
 */
int bcm_mcast_api_update_igmp_snoop(int                    sock_nl,
                                    int                    parent_ifi,
                                    int                    dstdev_ifi,
                                    int                    to_acceldev_ifi,
                                    unsigned short         vid,
                                    int                    lan_ppp,
                                    unsigned short         grp_vid,
                                    const struct in_addr  *rx_grp,
                                    const struct in_addr  *tx_grp,
                                    const struct in_addr  *src,
                                    const struct in_addr  *rep,
                                    unsigned char         *rep_mac,
                                    unsigned char          rep_proto_ver,
                                    int                    filter_mode,
                                    T_BCM_MCAST_WAN_INFO_ARRAY *wan_info,
                                    int                    exclude_port,
                                    char                   en_rtp_seq_check)
{
    T_BCM_MCAST_IGMP_SNOOP_ENTRY snoopEntry;

    if (!rx_grp || !tx_grp || !src || !rep || !wan_info)
    {
        return -EINVAL;
    }

    //don't accelerate mulitcast groups: 239.255.255.250/32, 224.0.0.0/24, 224.0.255.135/32

    if (!(IN_MULTICAST(ntohl(tx_grp->s_addr)) ||
        SNOOP_IN_IS_ADDR_L2_MCAST(tx_grp)) ||
        ((tx_grp->s_addr & ntohl(0xFFFFFF00)) == ntohl(0xE0000000)) ||
        ((tx_grp->s_addr & ntohl(0xFFFFFFFF)) ==  ntohl(0xEFFFFFFA)) ||
        ((tx_grp->s_addr & ntohl(0xFFFFFFFF)) ==  ntohl(0xE000FF87)))
    {
        return -EINVAL;
    }

    if (!(IN_MULTICAST(ntohl(rx_grp->s_addr)) ||
        SNOOP_IN_IS_ADDR_L2_MCAST(rx_grp)) ||
        ((rx_grp->s_addr & ntohl(0xFFFFFF00)) == ntohl(0xE0000000)) ||
        ((rx_grp->s_addr & ntohl(0xFFFFFFFF)) ==  ntohl(0xEFFFFFFA)) ||
        ((rx_grp->s_addr & ntohl(0xFFFFFFFF)) ==  ntohl(0xE000FF87)))
    {
        return -EINVAL;
    }

    /* printWar("parent_ifi=%d dstdev_ifi=%d to_acceldev_ifi=%d vid=%d lan_ppp=%d grp_vid=%d "
    "rx_grp=%s tx_grp=%s src=%s rep=%s rep_mac=rep_MAC=%02x-%02x-%02x-%02x-%02x-%02x rep_proto_ver=%d filter_mode=%s "
    "exclude_port=%d en_rtp_seq_check=%d\n",
    parent_ifi, dstdev_ifi, to_acceldev_ifi, vid, lan_ppp, grp_vid,
    imp_inet_ntoa(rx_grp->s_addr), imp_inet_ntoa(tx_grp->s_addr), imp_inet_ntoa(src->s_addr), imp_inet_ntoa(rep->s_addr), 
    rep_mac[0], rep_mac[1], rep_mac[2], rep_mac[3], rep_mac[4], rep_mac[5],
    rep_proto_ver, filter_mode>2 ? (filter_mode==3 ? "exclude add" : "exclude clear") : (filter_mode==1 ? "include add" : "include clear")
    , exclude_port, en_rtp_seq_check);

    int i = 0;
    T_BCM_MCAST_WAN_INFO      wan[BCM_MCAST_MAX_SRC_IF];
    memcpy(&wan, wan_info, sizeof(T_BCM_MCAST_WAN_INFO_ARRAY));
    for (i = 0; i < 10; i++) {
        printErr("%d: %d %d\n", i + 1, wan[i].ifi, wan[i].if_ops);
    }*/

    memset(&snoopEntry, 0, sizeof(T_BCM_MCAST_IGMP_SNOOP_ENTRY));
    snoopEntry.parent_ifi = parent_ifi;
    snoopEntry.dstdev_ifi = dstdev_ifi;
    snoopEntry.to_acceldev_ifi = to_acceldev_ifi;
    memcpy(&snoopEntry.rx_grp, rx_grp, sizeof(struct in_addr));
    memcpy(&snoopEntry.tx_grp, tx_grp, sizeof(struct in_addr));
    memcpy(&snoopEntry.src, src, sizeof(struct in_addr));
    memcpy(&snoopEntry.rep, rep, sizeof(struct in_addr));
    memcpy(&snoopEntry.rep_mac, rep_mac, ETH_ALEN);
    memcpy(&snoopEntry.wan_info, wan_info, sizeof(T_BCM_MCAST_WAN_INFO_ARRAY));
    snoopEntry.exclude_port = exclude_port;
    snoopEntry.en_rtp_seq_check = en_rtp_seq_check;
    snoopEntry.mode = filter_mode;
    snoopEntry.tci = vid;
    snoopEntry.grp_vid = grp_vid;
    snoopEntry.lan_ppp = lan_ppp;
    snoopEntry.rep_proto_ver = rep_proto_ver;

    return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_IGMP_SNOOP_ENTRY, (unsigned char *)&snoopEntry, sizeof(snoopEntry),  NULL, 0, 1);
}

int bcm_mcast_update_igmp_snoop(const struct in_addr *rx_grp, const struct in_addr *tx_grp,
                                struct in_addr *src, int filter_mode, int iif_index)
{
    int type = 0;
    if(mproxy.snooping != IMP_ENABLE)
    {
        return 1;
    }
    if(!pkt_info)
    {
        return -EINVAL;
    }
    if(mproxy.igmp_version != IM_IGMPv3_MLDv2)
    {
        T_IGMPv12_REPORT *report = (T_IGMPv12_REPORT *)&pkt_info->pkt[0];
        type = report->type;
    }
    else
    {
        T_IGMPv3_REPORT *report = (T_IGMPv3_REPORT *)&pkt_info->pkt[0];
        type = report->type;
    }
    struct in_addr zero_addr = {0};
    T_MCPD_WAN_INFO_ARRAY wan_info;

    bzero((char *)&wan_info, sizeof(T_MCPD_WAN_INFO_ARRAY));
    wan_info[0].ifi = iif_index;
    if(wan_info[0].ifi)
        wan_info[0].if_ops = MCPD_IF_TYPE_ROUTED;
    else
        wan_info[0].if_ops = MCPD_IF_TYPE_UNKWN;

    if (!src || mproxy.igmp_version != IM_IGMPv3_MLDv2) {
        src = &zero_addr;
    }

    return bcm_mcast_api_update_igmp_snoop(mproxy.mcpd_netlink_socket,
                                         pkt_info->parent_ifi,
                                         pkt_info->rxdev_ifi,
                                         pkt_info->to_acceldev_ifi,
                                         pkt_info->tci,
                                         pkt_info->lan_ppp,
                                         BCM_MCAST_INVALID_VID, /*not set, will be added by blog vlan rules*/
                                         rx_grp,
                                         tx_grp,
                                         src,
                                         (const struct in_addr *)&pkt_info->ipv4_rep,
                                         (unsigned char *)pkt_info->rep_mac,
                                         type,
                                         filter_mode,
                                         wan_info,
                                         -1,
                                         0);
}

#ifdef ENABLE_IMP_MLD
int bcm_mcast_api_update_mld_snoop(int                    sock_nl,
                                   int                    parent_ifi,
                                   int                    dstdev_ifi,
                                   int                    to_acceldev_ifi,
                                   unsigned short         vid,
                                   int                    lan_ppp,
                                   unsigned short         grp_vid,
                                   const struct in6_addr *grp,
                                   const struct in6_addr *src,
                                   const struct in6_addr *rep,
                                   unsigned char         *rep_mac,
                                   unsigned char          rep_proto_ver,
                                   int                    filter_mode,
                                   T_BCM_MCAST_WAN_INFO_ARRAY *wan_info)
{
    T_BCM_MCAST_MLD_SNOOP_ENTRY  snoopEntry6;

    if (!grp || !src || !rep || !wan_info)
    {
        return -EINVAL;
    }

    if(!(SNOOP_IN6_IS_ADDR_MULTICAST(grp) ||
        SNOOP_IN6_IS_ADDR_L2_MCAST(grp)) ||
        (SNOOP_IN6_IS_ADDR_MC_SCOPE0(grp)) ||
        (SNOOP_IN6_IS_ADDR_MC_NODELOCAL(grp)) ||
        (SNOOP_IN6_IS_ADDR_MC_LINKLOCAL(grp)))
    {
        return -EINVAL;
    }

    //don't accelerate multicast groups: FF01::0000/112, FF02::0/112, FF05::0001:0003/128

    if ((((grp->s6_addr32[0] & ntohl(0xFFFFFFFF)) == ntohl(0xFF010000)) || ((grp->s6_addr32[0] & ntohl(0xFFFFFFFF)) == ntohl(0xFF020000))) &&
       ((grp->s6_addr32[1] & ntohl(0xFFFFFFFF)) == ntohl(0x00000000)) &&
       ((grp->s6_addr32[2] & ntohl(0xFFFFFFFF)) == ntohl(0x00000000)) &&
       ((grp->s6_addr32[3] & ntohl(0xFFFF0000)) == ntohl(0x00000000)))
    {
        return -EINVAL;
    }

    if (((grp->s6_addr32[0] & ntohl(0xFFFFFFFF)) == ntohl(0xFF050000)) &&
       ((grp->s6_addr32[1] & ntohl(0xFFFFFFFF)) == ntohl(0x00000000)) &&
       ((grp->s6_addr32[2] & ntohl(0xFFFFFFFF)) == ntohl(0x00000000)) &&
       ((grp->s6_addr32[3] & ntohl(0xFFFFFFFF)) == ntohl(0x00010003)))
    {
        return -EINVAL;
    }

    /*printWar("parent_ifi=%d dstdev_ifi=%d to_acceldev_ifi=%d vid=%d lan_ppp=%d grp_vid=%d "
    "grp=%s src=%s rep=%s rep_mac=%02x-%02x-%02x-%02x-%02x-%02x rep_proto_ver=%d filter_mode=%s \n",
    parent_ifi, dstdev_ifi, to_acceldev_ifi, vid, lan_ppp, grp_vid,
    imp_inet6_ntoa(grp->s6_addr), imp_inet6_ntoa(src->s6_addr), imp_inet6_ntoa(rep->s6_addr), 
    rep_mac[0], rep_mac[1], rep_mac[2], rep_mac[3], rep_mac[4], rep_mac[5], rep_proto_ver,
    filter_mode>2 ? (filter_mode==3 ? "exclude add" : "exclude clear") : (filter_mode==1 ? "include add" : "include clear"));
    

    int i = 0;
    T_BCM_MCAST_WAN_INFO      wan[BCM_MCAST_MAX_SRC_IF];
    memcpy(&wan, wan_info, sizeof(T_BCM_MCAST_WAN_INFO_ARRAY));
    for (i = 0; i < 10; i++) {
        printErr("%d: %d %d\n", i + 1, wan[i].ifi, wan[i].if_ops);
    }*/

    memset(&snoopEntry6, 0, sizeof(T_BCM_MCAST_MLD_SNOOP_ENTRY));
    snoopEntry6.parent_ifi = parent_ifi;
    snoopEntry6.dstdev_ifi = dstdev_ifi;
    snoopEntry6.to_acceldev_ifi = to_acceldev_ifi;
    memcpy(&snoopEntry6.grp, grp, sizeof(struct in6_addr));
    memcpy(&snoopEntry6.src, src, sizeof(struct in6_addr));
    memcpy(&snoopEntry6.rep, rep, sizeof(struct in6_addr));
    memcpy(&snoopEntry6.rep_mac, rep_mac, ETH_ALEN);
    memcpy(&snoopEntry6.wan_info, wan_info, sizeof(T_BCM_MCAST_WAN_INFO_ARRAY));
    snoopEntry6.mode = filter_mode;
    snoopEntry6.tci = vid;
    snoopEntry6.grp_vid = grp_vid;
    snoopEntry6.lan_ppp = lan_ppp;
    snoopEntry6.rep_proto_ver = rep_proto_ver;

    return bcm_mcast_api_nl_tx(sock_nl, BCM_MCAST_MSG_MLD_SNOOP_ENTRY, (unsigned char *)&snoopEntry6, sizeof(snoopEntry6), NULL, 0, 1);
}

int bcm_mcast_update_mld_snooping(const struct in6_addr *grp, struct in6_addr *src, int filter_mode, int iif_index)
{
    int type = 0;
    if(mproxy.snooping != IMP_ENABLE)
    {
        return 1;
    }
    if(!pkt6_info)
    {
        return -EINVAL;
    }
    if(mproxy.mld_version == IM_IGMPv3_MLDv2)
    {
        T_MLDv2_REPORT *report = (T_MLDv2_REPORT *)&pkt6_info->pkt[0];
        type = report->type;
    }
    else
    {
        T_MLDv1_REPORT *report = (T_MLDv1_REPORT *)&pkt6_info->pkt[0];
        type = report->icmp6_hdr.icmp6_type == ICMPV6_MLD_V1_REPORT ? BCM_MCAST_SNOOP_EX_ADD : BCM_MCAST_SNOOP_IN_CLEAR;
    }
    struct in6_addr zero_addr = {.s6_addr32 = {0,0,0,0}};
    T_MCPD_WAN_INFO_ARRAY wan_info;

    bzero((char *)&wan_info, sizeof(T_MCPD_WAN_INFO_ARRAY));
    wan_info[0].ifi = iif_index;
    if(wan_info[0].ifi)
        wan_info[0].if_ops = MCPD_IF_TYPE_ROUTED;
    else
        wan_info[0].if_ops = MCPD_IF_TYPE_UNKWN;

    if (!src || mproxy.mld_version != IM_IGMPv3_MLDv2)
    {
       src = &zero_addr;
    }

    return bcm_mcast_api_update_mld_snoop(mproxy.mcpd_netlink_socket,
                                       pkt6_info->parent_ifi,
                                       pkt6_info->rxdev_ifi,
                                       pkt6_info->to_acceldev_ifi,
                                       pkt6_info->tci,
                                       pkt6_info->lan_ppp,
                                       BCM_MCAST_INVALID_VID, /*not set, will be added by blog vlan rules*/
                                       grp,
                                       src,
                                       (const struct in6_addr *)&pkt6_info->ipv6_rep,
                                       (unsigned char *)pkt6_info->rep_mac,
                                       type,
                                       filter_mode,
                                       &wan_info);

} /* mcpd_mld_update_snooping_info */
#endif

/*-----------------------------------------------------------------------
 * Name         : bcm_mcast_update_snooping
 *
 * Brief        : use Multicast API to add/remove MLD snooping and acceleration entry
 * Params       : [in] p_grp          -- Source IPv4/Ipv6 address for the group
 *                [in] p_src_list     -- sourse list for SSM groups and NULL for ASM groups
 *                [in] rep_type       -- group report type
 *
 * Return       : NULL
*------------------------------------------------------------------------
*/
void bcm_mcast_update_snooping(pi_addr *p_grp, pa_list *p_src_list, unsigned char rep_type)
{
    int mode = 0;
    int count = 0;
    int in_count = 0;
    int i = 0;

    imp_membership_db *p_md;
    if ((p_md = imp_membership_db_find(p_grp)) == NULL)
    {
        printErr("group %s's memship db don't existed\n", imp_pi_ntoa(p_grp));
        return;
    }

    imp_mfc *p_mfc = LIST_FIRST(&p_md->mfc_list);
    if (!p_mfc)
    {
        /*printErr("no existed src in %s\n", imp_pi_ntoa(p_grp));*/
        return;
    }

    src_in_mfc_list src_list[MCPD_MAX_UPSTREAM_SSM_SRS] = {0};
    for (p_mfc = LIST_FIRST(&p_md->mfc_list); p_mfc; p_mfc = LIST_NEXT(p_mfc, link))
    {
        while(p_src_list)
        {
            if (memcmp(&p_mfc->pia, &p_src_list->addr, sizeof(pi_addr)) == 0)
            {
                src_list[count].if_in = TRUE;
                ++in_count;
                break;
            }
            p_src_list = p_src_list->next;
        }
        src_list[count].src_addr = p_mfc->pia;
        src_list[count].if_index = p_mfc->iif_index;
        ++count;
        if(count >= MCPD_MAX_UPSTREAM_SSM_SRS)
        {
            printErr("too many src error\n");
            return;
        }
    }

    switch (rep_type)
    {
        case 2:
            mode = MCPD_SNOOP_ONLY_EX_ADD;
            break;

        case 3:
            mode = MCPD_SNOOP_PART_IN_ADD;
            break;

        case 4:
            mode = MCPD_SNOOP_PART_EX_ADD;
            break;

        case 1:
        case 5:
            if (!in_count)
            {
                printErr("\nsrc is not in mfc\n");
                return;
            }
            mode = MCPD_SNOOP_ONLY_IN_ADD;
            break;

        case 6:
            if (!in_count)
            {
                printErr("\nsrc is not in mfc\n");
                return;
            }
            mode = MCPD_SNOOP_ONLY_CLEAR;
            break;

        default:
            printErr("\nrecord type error\n");
            return;
            break;
    }

    switch(mode)
    {
        case MCPD_SNOOP_PART_EX_ADD:
            for(i = 0 ; i < count ; ++i)
            {
                if (!src_list[i].if_in)
                {
                    if (p_grp->ss.ss_family == AF_INET &&
                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                        (const struct in_addr *)&p_grp->v4.sin_addr,
                        (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                        MCPD_SNOOP_EX_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_update_igmp_snoop error...\n");
                    }

#ifdef ENABLE_IMP_MLD
                    else if (p_grp->ss.ss_family == AF_INET6 &&
                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                        (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                        MCPD_SNOOP_EX_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_mld_update_snoop error...\n");
                    }
#endif
                }
                else
                {
                    if (p_grp->ss.ss_family == AF_INET)
                    {
                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                            (const struct in_addr *)&p_grp->v4.sin_addr,
                            (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                            MCPD_SNOOP_EX_CLEAR, src_list[i].if_index);

                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                            (const struct in_addr *)&p_grp->v4.sin_addr,
                            (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                            MCPD_SNOOP_IN_CLEAR, src_list[i].if_index);
                    }

#ifdef ENABLE_IMP_MLD
                    else if (p_grp->ss.ss_family == AF_INET6)
                    {
                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                            (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                            MCPD_SNOOP_EX_CLEAR, src_list[i].if_index);

                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                            (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                            MCPD_SNOOP_IN_CLEAR, src_list[i].if_index);
                    }
#endif
                }
            }
            break;

        case MCPD_SNOOP_PART_IN_ADD:

            for(i = 0 ; i < count ; ++i)
            {
                if (src_list[i].if_in)
                {
                    if (p_grp->ss.ss_family == AF_INET &&
                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                        (const struct in_addr *)&p_grp->v4.sin_addr,
                        (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                        MCPD_SNOOP_IN_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_update_igmp_snoop error...\n");
                    }

#ifdef ENABLE_IMP_MLD
                    else if (p_grp->ss.ss_family == AF_INET6 &&
                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                        (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                        MCPD_SNOOP_IN_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_mld_update_snoop error...\n");
                    }
#endif
                }
                else
                {
                    if (p_grp->ss.ss_family == AF_INET)
                    {
                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                            (const struct in_addr *)&p_grp->v4.sin_addr,
                            (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                            MCPD_SNOOP_EX_CLEAR, src_list[i].if_index);

                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                            (const struct in_addr *)&p_grp->v4.sin_addr,
                            (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                            MCPD_SNOOP_IN_CLEAR, src_list[i].if_index);
                    }

#ifdef ENABLE_IMP_MLD
                    else if (p_grp->ss.ss_family == AF_INET6)
                    {
                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                            (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                            MCPD_SNOOP_EX_CLEAR, src_list[i].if_index);

                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                            (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                            MCPD_SNOOP_IN_CLEAR, src_list[i].if_index);
                    }
#endif
                }
            }
            break;

        case MCPD_SNOOP_ONLY_IN_ADD:
            for(i = 0 ; i < count ; ++i)
            {
                if (src_list[i].if_in)
                {
                    if (p_grp->ss.ss_family == AF_INET &&
                    bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                        (const struct in_addr *)&p_grp->v4.sin_addr,
                        (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                        MCPD_SNOOP_IN_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_update_igmp_snoop error...\n");
                    }

#ifdef ENABLE_IMP_MLD
                    else if (p_grp->ss.ss_family == AF_INET6 &&
                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                        (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                        MCPD_SNOOP_IN_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_mld_update_snoop error...\n");
                    }
#endif
                }
            }
            break;

        case MCPD_SNOOP_ONLY_EX_ADD:
            for(i = 0 ; i < count ; ++i)
            {
                if (!src_list[i].if_in)
                {
                    if (p_grp->ss.ss_family == AF_INET &&
                    bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                        (const struct in_addr *)&p_grp->v4.sin_addr,
                        (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                        MCPD_SNOOP_EX_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_update_igmp_snoop error...\n");
                    }

#ifdef ENABLE_IMP_MLD
                    else if (p_grp->ss.ss_family == AF_INET6 &&
                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                        (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                        MCPD_SNOOP_EX_ADD, src_list[i].if_index) < 0)
                    {
                        printErr("bcm_mcast_mld_update_snoop error...\n");
                    }
#endif
                }
            }
            break;
        case MCPD_SNOOP_ONLY_CLEAR:
            for(i = 0 ; i < count ; ++i)
            {
                if (src_list[i].if_in)
                {
                    if (p_grp->ss.ss_family == AF_INET)
                    {
                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                            (const struct in_addr *)&p_grp->v4.sin_addr,
                            (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                            MCPD_SNOOP_EX_CLEAR, src_list[i].if_index);

                        bcm_mcast_update_igmp_snoop((const struct in_addr *)&p_grp->v4.sin_addr,
                            (const struct in_addr *)&p_grp->v4.sin_addr,
                            (struct in_addr *)&src_list[i].src_addr.v4.sin_addr,
                            MCPD_SNOOP_IN_CLEAR, src_list[i].if_index);
                    }

#ifdef ENABLE_IMP_MLD
                    else if (p_grp->ss.ss_family == AF_INET6)
                    {
                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                            (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                            MCPD_SNOOP_EX_CLEAR, src_list[i].if_index);

                        bcm_mcast_update_mld_snooping((const struct in6_addr *)&p_grp->v6.sin6_addr,
                            (struct in6_addr *)&src_list[i].src_addr.v6.sin6_addr,
                            MCPD_SNOOP_IN_CLEAR, src_list[i].if_index);
                    }
#endif
                }
            }
            break;
    }
}
#endif