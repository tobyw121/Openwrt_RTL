#ifndef __IMP_ACCELERATION_H_
#define __IMP_ACCELERATION_H_

#ifdef BCM_MCAST_SUPPORT
#include "utils.h"

/* MLD defines and message types */
#define ICMPV6_MLD_V1V2_QUERY           130
#define ICMPV6_MLD_V1_REPORT            131
#define ICMPV6_MLD_V1_DONE              132
#define ICMPV6_MLD_V2_REPORT            143

#define MCPD_TRACE(level, args...)
#define NETLINK_BCM_MCAST	30	/* for multicast */
#define BCM_MCAST_MAX_SRC_IF      10

#define BCM_MCAST_IN_CLEAR 3
#define BCM_MCAST_EX_ADD   2

#define MCPD_MAX_UPSTREAM_SSM_SRS   24
#define MCPD_SNOOP_ONLY_IN_ADD    1
#define MCPD_SNOOP_ONLY_EX_ADD    2
#define MCPD_SNOOP_PART_IN_ADD    3
#define MCPD_SNOOP_PART_EX_ADD    4
#define MCPD_SNOOP_ONLY_CLEAR     5


#define BCM_MCAST_SNOOP_IN_ADD    1
#define BCM_MCAST_SNOOP_IN_CLEAR  2
#define BCM_MCAST_SNOOP_EX_ADD    3
#define BCM_MCAST_SNOOP_EX_CLEAR  4
#define BCM_MCAST_IF_UNKNOWN      0
#define BCM_MCAST_IF_BRIDGED      1
#define BCM_MCAST_IF_ROUTED       2

#define MCPD_SNOOP_IN_ADD         BCM_MCAST_SNOOP_IN_ADD
#define MCPD_SNOOP_IN_CLEAR       BCM_MCAST_SNOOP_IN_CLEAR
#define MCPD_SNOOP_EX_ADD         BCM_MCAST_SNOOP_EX_ADD
#define MCPD_SNOOP_EX_CLEAR       BCM_MCAST_SNOOP_EX_CLEAR
#define MCPD_IF_TYPE_UNKWN      BCM_MCAST_IF_UNKNOWN
#define MCPD_IF_TYPE_BRIDGED    BCM_MCAST_IF_BRIDGED
#define MCPD_IF_TYPE_ROUTED     BCM_MCAST_IF_ROUTED

typedef struct bcm_mcast_wan_info
{
   int                       ifi;
   int                       if_ops;
} T_BCM_MCAST_WAN_INFO;

typedef T_BCM_MCAST_WAN_INFO T_BCM_MCAST_WAN_INFO_ARRAY[BCM_MCAST_MAX_SRC_IF];
typedef T_BCM_MCAST_WAN_INFO_ARRAY T_MCPD_WAN_INFO_ARRAY;

typedef void (*bcm_mcast_api_rcv_func)(int type, unsigned char *pdata, int data_len);

typedef struct bcm_mcast_register 
{
    int primary;
    int result;
} T_BCM_MCAST_REGISTER;

typedef enum bcm_mcast_msgtype 
{
    BCM_MCAST_MSG_BASE = 0x10, /* NLMSG_MIN_TYPE, */
    BCM_MCAST_MSG_REGISTER = BCM_MCAST_MSG_BASE, /* usr - > krnl -> usr */
    BCM_MCAST_MSG_UNREGISTER, /* usr - > krnl -> usr */
    BCM_MCAST_MSG_IGMP_PKT, /* krnl -> usr */
    BCM_MCAST_MSG_IGMP_SNOOP_ENTRY,
    BCM_MCAST_MSG_MLD_PKT, /* krnl -> usr */
    BCM_MCAST_MSG_MLD_SNOOP_ENTRY,
    BCM_MCAST_MSG_IGMP_PURGE_ENTRY,
    BCM_MCAST_MSG_MLD_PURGE_ENTRY,
    BCM_MCAST_MSG_IF_CHANGE,
    BCM_MCAST_MSG_MC_FDB_CLEANUP, /* clean up for MIB RESET */
    BCM_MCAST_MSG_SET_PRI_QUEUE,
    BCM_MCAST_MSG_UPLINK_INDICATION,
    BCM_MCAST_MSG_IGMP_PURGE_REPORTER,
    BCM_MCAST_MSG_MLD_PURGE_REPORTER,
    BCM_MCAST_MSG_CONTROLS_ADMISSION,
    BCM_MCAST_MSG_ADMISSION_RESULT,
    BCM_MCAST_MSG_SNOOP_CFG,
#ifdef BCM_MCAST_HOSTCTL    /*used after bcm504L04 p1test3*/
    BCM_MCAST_MSG_HOSTCTL_CFG,
#endif
    BCM_MCAST_MSG_PROTO_RATE_LIMIT_CFG,
    BCM_MCAST_MSG_IGNORE_GROUP_LIST,
    BCM_MCAST_MSG_IGMP_DROP_GROUP,
    BCM_MCAST_MSG_MLD_DROP_GROUP,
    BCM_MCAST_MSG_SET_TIMEOUT,
    BCM_MCAST_MSG_BLOG_ENABLE,
    BCM_MCAST_MSG_QUERY_TRIGGER,
    BCM_MCAST_MSG_OVS_BRINFO_UPDATE,
    BCM_MCAST_MSG_PORT_DOWN,
    BCM_MCAST_MSG_IGMP_PURGE_GRP_REPORTER,
    BCM_MCAST_MSG_MLD_PURGE_GRP_REPORTER,
#ifdef BCM_MCAST_HOSTCTL    /*used after bcm504L04 p1test3*/
    BCM_MCAST_MSG_HOST_CLIENT_ENTRY_UPDATE,
#endif
    BCM_MCAST_MSG_MAX
} T_BCM_MCAST_MSGTYPES;

typedef enum mcpd_ret_code
{
    MCPD_RET_OK = 0,
    MCPD_RET_GENERR = 1,
    MCPD_RET_MEMERR = 2,
    MCPD_RET_ACCEPT = 3,
    MCPD_RET_DROP   = 4
} T_MCPD_RET_CODE;

typedef struct bcm_mcast_pkt_info
{
    int                       parent_ifi;
    int                       rxdev_ifi;
    int                       to_acceldev_ifi;
    int                       data_len;
    int                       lan_ppp;
    int                       packet_index;
    union {
       struct in6_addr        ipv6_rep;
       struct in_addr         ipv4_rep;
    };
    unsigned short            tci; /* vlan id */
    unsigned char             rep_mac[6];
    unsigned char             pkt[0];
} T_BCM_MCAST_PKT_INFO;

typedef struct bcm_mcast_timeout_entry
{
   int                       proto;
   int                       generalMembershipTimeoutSecs;
} T_BCM_MCAST_TIMEOUT_ENTRY;

typedef struct bcm_mcast_igmp_snoop_entry
{
   int                       parent_ifi;
   int                       dstdev_ifi;
   /* Internal, ignore endianness */
   int                       to_acceldev_ifi;
   unsigned int              mode;
   unsigned int              code;
   unsigned short            tci;/* vlan id */
   T_BCM_MCAST_WAN_INFO      wan_info[BCM_MCAST_MAX_SRC_IF];
   int                       lan_ppp;
   int                       exclude_port;
   char                      en_rtp_seq_check;
   /* Standard, use big endian */
   unsigned short            grp_vid; /*network-side VLAN ID carrying the multicast group downstream*/
   struct in_addr            rx_grp;
   struct in_addr            tx_grp;
   struct in_addr            src;
   struct in_addr            rep;
   unsigned char             rep_mac[6];
   unsigned char             rep_proto_ver;
} T_BCM_MCAST_IGMP_SNOOP_ENTRY;

/* IGMPv1/IGMPv2 report and query format */
typedef struct igmpv12_report
{
   unsigned char             type;  /* version & type */
   unsigned char             code;  /* unused */
   unsigned short            cksum; /* IP-style checksum */
   struct in_addr            group; /* group address being reported */
} T_IGMPv12_REPORT;

/* IGMPv3 group record format */
typedef struct igmp_grp_record
{
   unsigned char             type;			/* record type */
   unsigned char             datalen;		/* amount of aux data */
   unsigned short            numsrc;		/* number of sources */
   struct in_addr            group;		/* the group being reported */
   struct in_addr            sources[0];	/* source addresses */
} T_IGMP_GRP_RECORD;

/* IGMPv3 report format */
typedef struct igmp_v3_report
{
   unsigned char             type;	/* version & type of IGMP message */
   unsigned char             rsv0;	/* reserved */
   unsigned short            cksum; 	/* IP-style checksum */
   unsigned short            rsv1;	/* reserved */
   unsigned short            numgrps;   /* number of groups*/
   T_IGMP_GRP_RECORD         group[0];  /* group records */
} T_IGMPv3_REPORT;

typedef struct bcm_mcast_mld_snoop_entry 
{
   int                       parent_ifi;
   int                       dstdev_ifi;
   /* Internal, ignore endianness */
   int                       to_acceldev_ifi;
   unsigned int              mode;
   unsigned int              code;
   unsigned short            tci;
   T_BCM_MCAST_WAN_INFO      wan_info[BCM_MCAST_MAX_SRC_IF];
   int                       lan_ppp;
   /* External, use big endian */
   unsigned short            grp_vid; /*network-side VLAN ID carrying the multicast group downstream*/
   struct in6_addr           grp;
   struct in6_addr           src;
   struct in6_addr           rep;
   unsigned char             rep_mac[6];
   unsigned char             rep_proto_ver;
} T_BCM_MCAST_MLD_SNOOP_ENTRY;

/* MLDv1 report */
typedef struct mldv1_report
{
   struct icmp6_hdr          icmp6_hdr; /* ICMPv6 header */
   struct in6_addr           grp_addr; /* multicast group addr */
} T_MLDv1_REPORT;

/* MLDv2 report record format */
typedef struct mld_grp_record
{
   unsigned char             type; /* record type */
   unsigned char             datalen; /* auxiliary data len */
   unsigned short            numsrc; /* number of sources */
   struct in6_addr           group; /* multicast group address */
   struct in6_addr           sources[0]; /* source addresses */
} T_MLD_GRP_RECORD;

/* MLDv2 report */
typedef struct mldv2_report
{
   unsigned char               type;
   unsigned char               code; 
   unsigned short              cksum;
   unsigned short              rsvd;
   unsigned short              numgrps; /* number of group records */
   T_MLD_GRP_RECORD            group[0]; /* multicast groups */
} T_MLDv2_REPORT;

#define BCM_MCAST_NL_RX_BUF_SIZE  2048
#define ETH_ALEN	6		/* Octets in one ethernet addr	 */
#define BCM_MCAST_INVALID_VID 0xFFFF 

/* Identify IPV4 L2 multicast by checking whether the most bytes is 0 */
#define SNOOP_IN_IS_ADDR_L2_MCAST(a)        \
    !(((__const unsigned char *) (a))[0])
#define SNOOP_IN6_IS_ADDR_MULTICAST(a) (((__const unsigned char *) (a))[0] == 0xff)
#define SNOOP_IN6_IS_ADDR_L2_MCAST(a)       \
    !((((__const uint32_t *) (a))[0])       \
        || (((__const uint32_t *) (a))[1])  \
        || (((__const uint32_t *) (a))[2]))
#define SNOOP_IN6_IS_ADDR_MC_SCOPE0(a) \
	(SNOOP_IN6_IS_ADDR_MULTICAST(a) \
	 && ((((__const unsigned char *) (a))[1] & 0xf) == 0x0))
#define SNOOP_IN6_IS_ADDR_MC_NODELOCAL(a) \
	(SNOOP_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const unsigned char *) (a))[1] & 0xf) == 0x1))
#define SNOOP_IN6_IS_ADDR_MC_LINKLOCAL(a) \
	(SNOOP_IN6_IS_ADDR_MULTICAST(a)					      \
	 && ((((__const unsigned char *) (a))[1] & 0xf) == 0x2))

typedef struct src_in_mfc_list 
{
    pi_addr             src_addr;
    int                 if_index;
    unsigned char       if_in;
}src_in_mfc_list;

#define printErr(fmt, args...) printf("\033[31m[ %s ] %03d: "fmt"\033[0m", __FUNCTION__, __LINE__, ##args)
#define printWar(fmt, args...) printf("\033[1;33m[ %s ] %03d: "fmt"\033[0m", __FUNCTION__, __LINE__, ##args)

int bcm_mcast_api_socket_create(int *nl_sock, int portid);
int bcm_mcast_api_nl_recv(int nl_sock, char *rx_buf, int rx_bufsize, bcm_mcast_api_rcv_func process_func);
static int bcm_mcast_api_nl_tx(int sock_nl_in, int msg_type, unsigned char *pBuf, unsigned int len, unsigned char *pBuf2, unsigned int len2, int set);
int bcm_mcast_api_register(int sock_nl, int primary);
int bcm_mcast_api_unregister(int sock_nl, int primary);
int bcm_mcast_api_send_group_timeout (int sock_nl, int proto, int generalMembershipTimeoutSecs);
int mcpd_netlink_init(void);
int mcpd_netlink_shutdown(void);
void mcpd_update_multicast_info(void);
void mcpd_netlink_process_msg(int type, unsigned char *pdata, int data_len);
int mcpd_netlink_recv_mesg(void);
/*
 *-----------------------------------------------------------------------------
 * Function    : bcm_mcast_api_update_igmp_snoop
 * Description : Multicast API to add/remove IGMP snooping
 *               and acceleration entry. Refer to
 *               mcpd_igmp_update_snooping_info() for
 *               sample usage.
 * Input       : sock_nl_in - Netlink socket ID for MCPD to
 *                  Multicast driver communication.
 *               parent_ifi - Interface index for the bridge
 *                  associated with the client device.
 *               dstdev_ifi - Interface index for the client device.
 *               to_acceldev_ifi - Interface index for the flow
 *                  destination device or the client device.
 *                  This is the root device where the join was
 *                  received (for example: eth1).
 *               vid - VLAN tag from the Join report
 *               lanppp - Flag that indicates the presence of
 *                  a PPP header on the Join report
 *               grpVid - VLAN tag used for GPON/EPON
 *                  SFU Host Control mode
 *               rxGrp - Multicast IGMP group address subscribed
 *                  by the client
 *               txGrp - Multicast IGMP group address translated
 *                  to before transmission. This is usually the same
 *                  as RxGrp unless it is a Multicast DNAT scenario.
 *               src - Source IP address for the group. This value
 *                  is 0 for ASM groups and non-zero for SSM groups.
 *               rep - Client IP address 
 *               repMac - Client MAC address
 *               rep_proto_ver - IGMP v1/v2/v3 Report Type
 *               filter_mode - Takes values 1 thru 4
 *                  MCPD_SNOOP_IN_ADD (1)  - Add Snoop entry
 *                  MCPD_SNOOP_IN_CLEAR (2) - Remove snoop entry
 *                  MCPD_SNOOP_EX_ADD (3) - Add Snoop entry
 *                  MCPD_SNOOP_EX_CLEAR (4) - Remove Snoop entry
 *               wan_info - List of WAN interfaces that are configured
 *                  to be a multicast source. Each record in the list
 *                  has the ifindex of the WAN interface and a flag
 *                  that indicates if it is a BRIDGED/ROUTED interface.
 *               excludePort - UDP Exclude port. Multicast traffic
 *                  destined to this UDP port will not be accelerated.
 *               enRtpSeqCheck - Enable/Disable RTP Sequencing Check
 *                  for this flow. Note that the flow will not be
 *                  hardware accelerated if RTP sequencing check is
 *                  enabled.
 * Return Value : 0 - Success, Failure otherwise
 *-----------------------------------------------------------------------------
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
                                    char                   en_rtp_seq_check);

/*
 *-----------------------------------------------------------------------------
 * Function    : bcm_mcast_api_update_mld_snoop
 * Description : Multicast API to add/remove MLD snooping and acceleration 
 *               entry. Refer to mcpd_mld_update_snooping_info() for sample 
 *               usage.
 * Input       : sock_nl_in - Netlink socket ID for MCPD to
 *                  Multicast driver communication.
 *               parent_ifi - Interface index for the bridge
 *                  associated with the client device.
 *               dstdev_ifi - Interface index for the client device.
 *               to_acceldev_ifi - Interface index for the flow
 *                  destination device or the client device.
 *                  This is the root device where the join was
 *                  received (for example: eth1).
 *               vid - VLAN tag from the Join report
 *               lanppp - Flag that indicates the presence of
 *                  a PPP header on the Join report
 *               grpVid - VLAN tag used for GPON/EPON
 *                  SFU Host Control mode
 *               grp - Multicast MLD group address subscribed
 *                  by the client
 *               src - Source IPv6 address for the group. This value
 *                  is 0 for ASM groups and non-zero for SSM groups.
 *               rep - Client IPv6 address 
 *               repMac - Client MAC address
 *               rep_proto_ver - MLD v1/v2 Report Type
 *               filter_mode - Takes values 1 thru 4
 *                  MCPD_SNOOP_IN_ADD (1)  - Add Snoop entry
 *                  MCPD_SNOOP_IN_CLEAR (2) - Remove snoop entry
 *                  MCPD_SNOOP_EX_ADD (3) - Add Snoop entry
 *                  MCPD_SNOOP_EX_CLEAR (4) - Remove Snoop entry
 *               wan_info - List of WAN interfaces that are configured
 *                  to be a multicast source. Each record in the list
 *                  has the ifindex of the WAN interface and a flag
 *                  that indicates if it is a BRIDGED/ROUTED interface.
 * Return Value : 0 - Success, Failure otherwise
 *-----------------------------------------------------------------------------
 */
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
                                   T_BCM_MCAST_WAN_INFO_ARRAY *wan_info);
int bcm_mcast_update_igmp_snoop(const struct in_addr *rxGrp, const struct in_addr *txGrp, 
							struct in_addr *src, int filter_mode, int iif_index);
int bcm_mcast_update_mld_snooping(const struct in6_addr *grp, struct in6_addr *src, int filter_mode, int iif_index);
void bcm_mcast_update_snooping(pi_addr *p_grp, pa_list *p_src_list, unsigned char rep_type);
#endif
#endif