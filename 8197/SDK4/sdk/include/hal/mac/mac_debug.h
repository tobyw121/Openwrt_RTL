/*
 * Copyright (C) 2009 Realtek Semiconductor Corp. 
 * All Rights Reserved.
 *
 * This program is the proprietary software of Realtek Semiconductor
 * Corporation and/or its licensors, and only be used, duplicated, 
 * modified or distributed under the authorized license from Realtek. 
 *
 * ANY USE OF THE SOFTWARE OTHER THAN AS AUTHORIZED UNDER 
 * THIS LICENSE OR COPYRIGHT LAW IS PROHIBITED. 
 *
 * $Revision: 48836 $
 * $Date: 2014-06-24 15:26:00 +0800 (Tue, 24 Jun 2014) $
 *
 * Purpose : register service APIs in the SDK.
 *
 * Feature : register service APIs
 *
 */

#ifndef __HAL_MAC_DEBUG_H__
#define __HAL_MAC_DEBUG_H__

#include <common/rt_autoconf.h>

struct hsb_param_s
{
    union
    {
        struct
        {
            uint32  sphy:5;
            uint32  l2fmt:4;
            uint32  pppoe:1;
            uint32  l34fmt:4;
            uint32  l4off:8;
            uint32  ipfo0_n:1;
            uint32  datype:2;
            uint32  oampdu:1;
            uint32  cputag:1;
            uint32  otagExist:1;
            uint32  opri:3;
            uint32  dei:1;
            uint32  ovid:12;
            uint32  itagExist:1;
            uint32  ipri:3;
            uint32  cfi:1;
            uint32  frameLen:14;
            uint32  ipv4oph:1;
            uint32  ivid:12;
            uint32  typeLen:16;
            uint32  rtkPro:3;
            uint32  lb:1;
            uint32  l2err:1;
            uint32  l3err:1;
            uint32  l4err:1;
            uint32  l2calc:1;
            uint32  l3calc:1;  
            uint32  l4calc:1;  
            uint8   fs1[9];
            uint8   fs2[9];
            uint32  oamTf:1;
            uint32  trapToCpu:1;
            uint32  oam_lbsa:2;
            uint32  oam_lbda:2;
            uint32  flag:16;
            #define FC_EN_MASK                  (1<<0)
            #define INGRESS_ACL_EN_MASK         (1<<1)
            #define EGRESS_ACL_EN_MASK          (1<<2)
            #define EGRESS_VLAN_TRAN_EN_MASK    (1<<3)
            #define INGRESS_MAX1X_EN_MASK       (1<<4)
            #define EGRESS_MAX1X_EN_MASK        (1<<5)
            #define ATTACK_PREV_EN_MASK         (1<<6)
            #define TAG_STADE_EN_MASK           (1<<7)
            #define SML_EN_MASK                 (1<<8)
            #define INGRESS_VLAN_EN_MASK        (1<<9)
            #define EGRESS_VLAN_EN_MASK         (1<<10)
            #define LINKAGGR_LOADSHARE_EN_MASK  (1<<11)
            #define EGRESS_SPT_EN_MASK          (1<<12)
            #define EGRESS_LDP_EN_MASK          (1<<13)
            uint32  pm;  
            uint8   dmac[6];
            uint8   smac[6]; 
            uint8   payload[47];
            uint32  drop:1;
            uint32  asdpm:1;                  
            uint32  asdp:1;               
            uint32  dp:2;                 
            uint32  pri:3;                
            uint32  asdprmk:1;  
            uint32  ipv6oph:1;
            uint32  dpm:28;
            uint32  reason:4;
            uint32  rxrspan:1;
            uint32  extraTag:1;
            uint32  dfPri:1;
            uint32  insertCpuTag:1;
        } r8328;
        struct
        {
            uint32  sel_hsb:1;
            uint32  valid_hsb:1;
            uint32  reserved:7;
            uint32  cfi:1;
            uint32  patternmatch:2;
            uint32  flowlabel:20;

            uint32  dstport:16;
            uint32  srcport:16;

            uint32  tcpflags:8;
            uint32  ipproto:8;
            uint32  svid:12;
            uint32  spri:3;
            uint32  rxdrop:1;

            uint32  cputagif:1;
            uint32  cpuintpri:3;
            uint32  cpuportmask:28;

            uint32  ethtype:16;
            uint32  ipv6mld:1;
            uint32  cpri:3;
            uint32  cvid:12;

            uint32  dip;
            uint32  sip;

            uint8   dmac[6];
            uint8   smac[6]; 

            uint32  tos:8;
            uint32  cpuasdp:1;
            uint32  cpuasdpm:1;
            uint32  cpuasdprmk:1;
            uint32  ipv6:1;
            uint32  ipv4:1;
            uint32  pppoe:1;
            uint32  stagif:1;
            uint32  ctagif:1;
            uint32  frametype:2;
            uint32  pktlen:14;

            uint32  l4csok:1;
            uint32  l3csok:1;
            uint32  endpage:11;
            uint32  startpage:11;
            uint32  startbank:3;
            uint32  spa:5;
        } r8389;
        struct
        {
            uint32 page_cnt:6;
            uint32 endsc:12;            /* End page point */
            uint32 bgsc:12;             /* Start page point */
            uint32 field_seltor11:16;   /* Non IPv6 : User defined 16-bit field IPv6 : SIP[127:112] */
            uint32 field_seltor10:16;   /* Non IPv6 : User defined 16-bit field IPv6 : SIP[111:96] */
            uint32 field_seltor9:16;    /* Non IPv6 : User defined 16-bit field IPv6 : SIP[95:80] */
            uint32 field_seltor8:16;    /* Non IPv6 : User defined 16-bit field IPv6 : SIP[79:64] */
            uint32 field_seltor7:16;    /* Non IPv6 : User defined 16-bit field IPv6 : SIP[63:48] */
            uint32 field_seltor6:16;    /* Non IPv6 : User defined 16-bit field IPv6 : SIP[47:32] */
            uint32 field_seltor5:16;    /* User defined 16-bit field */
            uint32 field_seltor4:16;    /* User defined 16-bit field, CPUTAG[79:64] */
            uint32 field_seltor3:16;    /* User defined 16-bit field, CPUTAG[63:48] */
            uint32 field_seltor2:16;    /* User defined 16-bit field, CPUTAG[47:32] */
            uint32 field_seltor1:16;    /* User defined 16-bit field, shared for CPUTAG[31:16] */
            uint32 field_seltor0:16;    /* User defined 16-bit field, shared for CPUTAG[15:0] */
            uint32 field_sel_vld:12;    /* Indicates which field_selector is valid */
            uint32 impls_exp:3;         /* Inner MPLS EXP */
            uint32 impls_label:20;      /* Inner MPLS label */
            uint32 ompls_exp:3;         /* Outer MPLS EXP */
            uint32 ompls_label:20;      /* Outer MPLS label */
            uint32 arpopcode:16;        /* ARP/RARP : Operation field. IPv6 : Extented header length */
            uint8  target_mac[6];       /* ARP/RARP :Target MAC of ARP/RARP packet, for ACL. IPV6 : DIP[127:80] */
            uint8  sender_mac[6];       /* ARP/RARP: Sender MAC of ARP/RARP packet, for ACL. IPV6 : DIP[79:32] */
            uint32 dport:16;            /* TCP/UDP destination port, ICMP code B[7:0] */
            uint32 sport:16;            /* TCP/UDP source port, ICMP type B[7:0] */
            uint32 tcp_flag:8;          /* TCP : TCP Flag. IGMP : IGMP Type field */
            uint32 ipver:4;             /* IP header version (For Ipv4 and Ipv6) */
            uint32 ipttl:8;             /* IP header - TTL field */
            uint32 ip_tos:8;            /* IP header - DSCP/TOS */
            uint32 ip_protocol:8;       /* IP header - protocol field, IPv6: Next header */
            uint32 ip_flag:3;           /* IPV4 : IP header- flag. IPV6 : IP flag in fragment header */
            uint32 ip_offset:13;        /* IPV4 : IP header- offset. IPV6 : IP offset in fragment header */
            uint32 dip;                 /* IPv4: { DIP[31:0] }, IPv6: DIP[31:0], target IP for ARP/RARP
                                            B[15:0]=MEPID of CCM packet
                                            B[23:16]=OpCode of CFM packet
                                            B[26:24]=MD Level of CFM packet
                                            B[29:27]=CFM Interval Field
                                            B[31:30]=reserved */
            uint32 sip;                 /* IPv4: {SIP[31:0] }, IPv6: SIP[31:0], source IP for ARP/RARP */
            uint32 otag:16;             /* Outer VLAN Tag, B[15:13]=user-priority,B[12]=DEI,B[11:0]=VID */
            uint32 itag:16;             /* Inner VLAN Tag, B[15:13]=user-priority,B[12]=CFI,B[11:0]=VID */
            uint32 otpid_idx:2;         /* TPID index for outer tag */
            uint32 itpid_idx:2;         /* TPID index for inner tag */
            uint32 etp:16;              /* Ethernet/Length field(Ether Type) */
            uint8  smac[6];             /* Source MAC address */
            uint8  dmac[6];             /* Destination MAC address */
            uint32 iplen:16;            /* IPv4 Total length or IPv6 payload length(For Attack prevention) */
            uint32 ipv4hdlen:4;         /* IPv4 header length */
            uint32 tcpseqzero:1;        /* TCP sequence number is zero */
            uint32 ipv6hop:1;           /* IPv6 packet with Hop-by-Hop header */
            uint32 ipv6rout:1;          /* IPv6 packet with routing header */
            uint32 ipv6frag:1;          /* IPv6 packet with fragment header */
            uint32 ipv6dest:1;          /* Ipv6 packet with destination header */
            uint32 ipv6auth:1;          /* IPv6 packet with authentication header */
            uint32 ipcsok:1;            /* IP checksum is ok */
            uint32 ipv6ext_long:1;      /* IPv6 header is too long for parser */
            uint32 ipv6:1;              /* IPv6 Type */
            uint32 ipv4:1;              /* IPv4 Type */
            uint32 ompls_if:1;          /* Has outer MPLS interface */
            uint32 impls_if:1;          /* Has inner MPLS interface */
            uint32 etag_if:1;           /* Extra Tag */
            uint32 cputag_if:1;         /* Has CPU Tag */
            uint32 otag_if:1;           /* Has Outer VLAN Tag(Tag or Priority tag) */
            uint32 itag_if:1;           /* Has Inner VLAN Tag(Tag or priority tag) */
            uint32 oampdu:1;            /* OAM control frame */
            uint32 llc_other:1;         /* LEN < 0x0600, {DSAP, SSAP, CTL, OUI}  != 0xaaaa03_000000 */
            uint32 pppoe_if:1;          /* PPPoE packet */
            uint32 rfc1042:1;           /* LLC SNAP */
            uint32 spa:6;               /* Source Port ID */
            uint32 errpkt:1;            /* Error packet(CRC,Symbol) */
            uint32 l4hdchk:1;           /* Check the non-fragment or first fragment of IP packet(TCP/UDP/ICMP) has enough length for L4 header(For attack prevention) */
            uint32 pktlen:14;           /* Packet length (Include CRC) */
        } r8390;
        struct
        {
            uint32 rtkpp:2;             /*b11:this is a rldp/rlpp pkt; b01:this is a rrcp pkt; b00:this is not a realtek protocol pkt*/
            uint32 ip_mf:1;             /*ipv4 MF flag bit*/
            uint32 ip_dstype:8;         /*ipv4/ipv6 dstype value*/
            uint32 ip_protocol:8;       /*the protocol item in ipv4/v6 header or "next header" item in extension header*/
            uint32 ip_offset:13;        /*the offset item in ipv4 header or fragment extension header*/
            uint32 ip_length:16;        /*the payload length item in ipv4/v6 header*/
            uint32 ip_ttl:8;            /*the ttl item in ipv4/v6 header*/
            uint8  dip[16];              /*ipv6 pkt;(48bit don't care,DMAC,DIP)arp pkt;(32bit don't care,12bytes content following 0x8899)rtkpp pkt;
                                                            (96bit don't care, dip)ipv4 pkt*/
            uint8  sip[16];              /*128bit sip for ipv6 pkt;(32bit don't care,OP,SMAC,SIP) arp pkt;(96bit don't care ,sip) ipv4 pkt*/
            uint32 ipv4_hdrlen:4;       /*ipv4 header length for ipv4 pkt;{DEST,FRAG,ROUT,HOP}extension header exist for ipv6 pkt*/
            uint32 ipv4_df:1;           /*the "DF" iterm in ipv4 header for ipv4 pkt; AUTH extionsion header exist for ipv6 pkt*/
            uint32 tcp_hdrlen:4;        /*tcp header length in tcp header for tcp pkt;group ip [27:24] for igmp pkt*/
            uint32 tcp_flag:8;          /*tcp flag in tcp header for tcp pkt;group ip[23:16] for igmp pkt*/
            uint32 tcp_sn_eq_0:1;       /*tcp sequence number is all 0*/
            uint32 l4_content;       /*{sport,dport} for tcp/udp pkt;{type,code,16bit don¡¯t care}for icmp pkt;{type,8bit don¡¯t care,group ip[15:0]} for igmp pkt*/
            uint32 rx_rm_rtag:1;             /*rx remove rspan tag*/
            uint32 icmp_pkt:1;          /*this is a icmp pkt;the icmp header must be complete*/
            uint32 igmp_pkt:1;          /*this is a igmp pkt*/
            uint32 udp_pkt:1;           /*this is a udp pkt*/
            uint32 tcp_pkt:1;           /*this is a tcp pkt*/
            uint32 pppoe_pkt:1;         /*this is a pppoe pkt*/
            uint32 arp_pkt:1;           /*this a arp pkt*/
            uint32 ipv6_pkt:1;          /*this is a ipv6 pkt*/
            uint32 ipv4_pkt:1;          /*this is a ipv4 pkt*/
            uint32 frame_type:2;        /*ethernet II or snap LLC or length encapulation according to spec:
                                                           *Ethernet II:0b00;RFC_1042:0b01;LLC other:0b10;others:0b11
                                                           */
            uint32 itag_content:16;     /*the {pri, cfi, ivid} of itag*/
            uint32 otag_content:16;     /*the {pri, dei, ovid} of otag, the {pri, 13bit don't care} of rspan tag*/
        
            /*ctag content exclude 'proto' and 'reserve' is 41bit*/
            uint32 ctag_acl_act:1;      /*take effect of ingress ACL or not*/
            uint32 ctag_bp_fltr_1:1;    /*if value is 0b1, bypass the following filtering:(1)egress port isolation (2)egress mirror isolation*/
            uint32 ctag_bp_fltr_2:1;    /*if value is 0b1, bypass the following checking:(1)egress spanning tree port state in blocking/listening or learning (2)egress vlan filtering*/
            uint32 ctag_as_tagsts:1;    /*decide whether proceed outer.inner Tag*/
            uint32 ctag_rvid_sel:1;     /*relay vid selection:0b0 follow the inner tag;0b1 follow the outer tag*/
            uint32 ctag_l2learning:1;   /*decide whether proceed l2 learning*/
            uint32 ctag_as_pri:1;       /*if CPU provides the priority directly*/
            uint32 ctag_pri:3;          /*assigned priority*/
            uint32 ctag_as_dpm:1;       /*0b1:the address lookup does not take effect and the assigned DPM is used as the destination port mask*/
            uint32 ctag_physical_dpm:1; /*decide DPM property:0 logical port; 1 physical port*/
            uint32 ctag_dpm:29;         /*the assigned packet destination port mask*/
            
            uint32 itag_exist:1;        /*this pkt is with itag*/
            uint32 otag_exist:1;        /*this pkt is with otag*/
            uint32 rtag_exist:1;        /*this pkt is with rspan tag*/
            uint32 etag_exist:1;        /*this pkt is with extra tag*/
            uint32 ctag_exist:1;        /*this pkt is with cpu tag*/
            uint32 eav_b_pkt:1;         /*no use at present*/
            uint32 eav_a_pkt:1;         /*no use at present*/
            uint32 crceq:1;             /*no use at present*/
            uint32 typelen:16;          /*the type or length field of pkt*/
            uint32 fs3_data:16;         /*field selector 3 select data*/
            uint32 fs2_data:16;         /*field selector 2 select data*/
            uint32 fs1_data:16;         /*field selector 1 select data*/
            uint32 fs0_data:16;         /*field selector 0 select data*/
            uint32 fs3_valid:1;         /*field selector 3 select data is valid*/
            uint32 fs2_valid:1;         /*field selector 2 select data is valid*/
            uint32 fs1_valid:1;         /*field selector 1 select data is valid*/
            uint32 fs0_valid:1;         /*field selector 0 select data is valid*/
            uint8 smac[6];              /*source mac address*/
            uint8 dmac[6];              /*destination mac address*/
            uint32 pktlen:14;             /*the total bytes store in pkt buffer*/
            uint32 sphy:5;              /*the id of this rx port*/
            uint32 rxdrop:1;            /*the pkt is decieded to drop by rx*/
            uint32 oampdu:1;            /*the pkt is oampdu pkt*/
            uint32 itag_index:2;        /*the tpid index of itag pkt*/
            uint32 otag_index:2;        /*the tpid index of otag/rspan tag pkt*/
            uint32 ipv6_extension_hdrlen:14;      /*ipv6 extenstion header length*/
            uint32 ipv6_flow_label:20;          /*ipv6 flow label*/
            uint32 ipv6_mob_ext_hdr:1;      /*ipv6 mobility extension header*/
            uint32 ipv6_hbh_ext_hdr_err:1;  /*ipv6 hop by hop extension header error*/
            uint32 ipv6_unknown_hdr:1;   /*ipv6 unknown hdr*/
            uint32 reserved1 : 2;
            uint32 reserved2: 27;
        } r8380;
    };
};
typedef struct hsb_param_s hsb_param_t;


struct ppi_param_s
{
    uint32  sphy:5;         
    uint32  crsvlan:1;      
    uint32  keeporigivid:1;     
    uint32  keeporigovid:1;     
    uint32  pbivid:12;      
    uint32  pbovid:12;      


    uint32  pbipri:3;              
    uint32  pbodei:1;       
    uint32  ipuc:1;         
    uint32  pbopri:3;       
    uint32  ppbivid:12;     
    uint32  ppbovid:12;     


    uint32  ppbipri:3;      
    uint32  ppbodei:1;      
    uint32  upLinkMac_0:1;  
    uint32  ppbopri:3;      
    uint32  fcivid:12;      
    uint32  fcovid:12;      

    uint32  ttldec:1;
    uint32  fcseldot1qipri:1;   
    uint32  fcseldot1qopri:1;   
    uint32  rrcpType:3;     
    uint32  rrcpAuth:1;     
    uint32  ivid:12;            
    uint32  ovid:12;            
    uint32  drop:1;         


    uint32  cpu:1;          
    uint32  cputag:1;       
    uint32  toGVlan:1;      
    uint32  ipmcPkt:1;      
    uint32  dot1qodp:2;     
    uint32  sml:1;          
    uint32  redir:1;            
    uint32  lkmiss:1;       
    uint32  ftidx:10;                   
    uint32  dpn:5;              
    uint32  pbdp:2;         
    uint32  pbpri:3;            
    uint32  ftIdxValid:1;       
    uint32  dot1qidp:2;
    uint32  dpm:29;         
    uint32  dot1qipri:3;
    uint32  dscppriexist:1; 
    uint32  dscpdp:2;       
    uint32  dscppri:3;      
    uint32  fbpriexist:1;       
    uint32  fbdp:2;         
    uint32  fbpri:3;            
    uint32  ipriexist:1;        
    uint32  idp:2;          
    uint32  ipri:3;             
    uint32  opriexist:1;        
    uint32  odp:2;          
    uint32  opri:3;         
    uint32  dp:2;           
    uint32  pri:3;          
    uint32  macdot1x:1;      
    uint32  fcdfi:1;        
    uint32  fcdfo:1;    
    uint32  tmidx:2;    
    uint32  vidrch:1;
    uint32  fwd:4;          
    uint32  mir:4;          
    uint32  mirorg:4;       
    uint32  spm:16;     
    uint32  msti:7;         
    uint32  fid:12; 
    uint32  miriutag:4;         
    uint32  miroutag:4;  
    uint32  flood:1;            
    uint32  ppbdfi:1;       
    uint32  ppbdfo:1;       
    uint32  tm:1;           
    uint32  ipmc:1;         
    uint32  copyToCpu:1;    
    uint32  vidrc:5;            
    uint32  reason:16;      
    uint32  dscprmk:1;      
    uint32  dot1qopri:3;        
    uint32  ce:1;   
    uint32  iutagstatus:29;     
    uint32  iuntagValid:1;  
    uint32  ountagValid:1;  
    uint32  outagstatus:29;

    uint32  dscp:6;             
    uint32  orgvid:12;                  
    uint32  dmacidx:14; 

    uint32  rrcpRegData;            

    uint32  prc:16;                 
    uint8   iprc[8];

    uint8   bpdu:1; /*for sw use*/
    uint32  egressvid:12;

    uint32  aclidx0:8;          
    uint32  aclidx1:8;           

    uint32  aclidx2:8;          
    uint32  aclidx3:8;          
    uint32  aclidx4:8;           
    uint32  aclidx5:8;           

    uint32  aclidx6:8;          
    uint32  aclidx7:8;          
    uint32  aclidx8:8;          
    uint32  aclidx9:8;          

    uint32  aclidx10:8;         
    uint32  aclidx11:8;         
    uint32  aclidx12:8;         
    uint32  aclidx13:8;         

    uint32  aclidx14:8;         
    uint32  aclidx15:8;         

};
typedef struct ppi_param_s ppi_param_t;


struct pmi_param_s
{
    uint32  dpm:29;         /*destination port mask*/
    uint32  l2err:1;        /*Recalculate the layer2 CRC*/
    uint32  l3err:1;        /*Recalculate the IP checksum*/
    uint32  pppoe:1;        /*Whether PPPoE header exists*/

    uint32  fwd:4;          
    uint32  mir:4;          
    uint32  mirorg:4;       
    uint32  pri:3;          /*Priority*/
    uint32  rxrspan:1;      /*rspan tag exist*/
    uint32  miriutag:4;     
    uint32  miroutag:4;
    uint32  stphy:5;
    uint32  rrcpType:2;     /*rrcp packet type*/
    uint32  rxcpuTag:1;     /*cpu tag exist when packet is rx from cpu port*/  

    uint32  dpcnt:5;            
    uint32  pnxt:11;        
    uint32  reason:16;      /*cpu reason*/

    uint32  framelength:14; /*frame length,include CRC*/
    uint32  dp:2;           /*Drop precedence*/
    uint32  lb:1;           /*oam loopback*/
    uint32  occupy:1;       /*occupied or not*/
    uint32  sphy:5;         /*Source port number*/
    uint32  extraTag:1;     /*extra tag exist*/
    uint32  asdp:1;         /*Packet priority and drop precedence is assigned*/
    uint32  asdprmk:1;      /*Packet priority and drop precedence is assigned and need remarking*/
    uint32  evten:1;
    uint32  ttldec:1;       /*TTL decrease for routing */
    uint32  txcputag:1;     /*Insert CPU tag when packet is sent to CPU port*/
    uint32  dopri:3;        /*destination outer tag priority*/      

    uint32  divid:12;       /*Destination Inner VID*/
    uint32  l2fmt:4;        /*Layer 2 format*/
    uint32  dovid:12;       /*Destination Outer VID*/
    uint32  dipri:3;            
    uint32  l4err:1;        /*Recalculate the TCP, UDP, ICMP and IGMP checksum*/

    uint32  iutagStatus:29;     
    uint32  dscprmk:1;      /*Weather DSCP remarking or not*/
    uint32  oam_lbsa:2;     /*oam loopback smac address operation*/

    uint32  ipuc:1;         /*IPv4 unicast routing*/
    uint32  ipmc:1;         /*ip multicast routing*/
    uint32  l34fmt:4;       /*Layer 3/4 format*/
    uint32  dmacidx:14;     /*When perform IPv4 unicast routing, this field point to the DMAC position stored in address hash table*/
    uint32  dscp:6;         /*DSCP*/  
    uint32  ce:1;           /*Whether set CE bit in ip header to */
    uint32  rrcpRegData31_27:5; /*rrcp register data bit31-bit26*/

    uint32  outagStatus:29;             
    uint32  rrcp:1;         /*rrcp packet*/
    uint32  oam_lbda:2;     /*oam loopback dmac address operation*/

    uint32  keeporigivid:1;
    uint32  keeporigovid:1;

    uint32  diprivalid:1;
    uint32  dcfivalid:1;
    uint32  dcfi:1;
    uint32  doprivalid:1;
    uint32  ddeivalid:1;
    uint32  ddei:1;
    uint32  spididxvalid:1;
    uint32  spididx:4;
    uint32  reserved:19;

    /*used for model code, not real ASIC field*/
    uint32  drop:1;         /*1:dropped by last PIE lookup phase*/
    uint32  curPortNum:5;   /*current port number*/
    uint32  egvidiuvalid:1;
    uint32  egvidouvalid:1;
    uint32  egvidkeepi:1;
    uint32  egvidkeepo:1;
};
typedef struct pmi_param_s pmi_param_t;

typedef struct hsa_8390_qid_s
{
    uint32 qid:3;   /* per port queue ID 0~7 */
} hsa_8390_qid_t;

struct hsa_param_s
{
    union
    {
        struct
        {
            uint32  hsa_busy:1;
            uint32  reserved:5;
            uint32  newsvid:12;
            uint32  newvid:12;
            uint32  cputagif:1;
            uint32  dpcnt_4:1;
        
            uint32  dpcnt_3_0:4;
            uint32  rvid:12;           
            uint32  reason:16;         
        
            uint32  intpri:3;          
            uint32  dpmask:29;         
        
            uint32  mir1dpa:5;
            uint32  mir0dpa:5;
            uint32  cpuasdprmk:1;
            uint32  ipv6:1;
            uint32  ipv4:1;
            uint32  pppoe:1;
            uint32  stagif:1;
            uint32  ctagif:1;
            uint32  frametype:2;
            uint32  pktlen:14;
        
            uint32  l4csok:1;
            uint32  l3csok:1;
            uint32  endpage:11;
            uint32  startpage:11;
            uint32  startbank:3;
            uint32  spa:5;
        } r8389;
        struct
        {
            uint32 page_cnt:6;
            uint32 acl_mpls_hit:1;      /* Hit MPLS rule */
            uint32 acl_mpls_act:1;      /* 0=Push outer, 1=push inner & outer */
            uint32 mpls_index:9;        /* MPLS index, B[0]=is no used when push inner and outer */
            uint32 astagsts:1;          /* Decide whether processed Outer/Inner TAG(From CPUTAG)
                                            0: don't touch
                                            1: follow ALE decision */
            uint32 ctag_dm:1;           /* ETH-DM packet indicate:
                                            0b1: Notify Tx Modifier to fill the TxTimestamp field based-on OpCode
                                            0b0: This is NOT an ETH-DM packet */
            uint32 ctag_dgpkt:1;        /* Dying Gasp packet:
                                            0b1: It's a dying gasp packet
                                            0b0: It's not s dying gasp packet */
            uint32 tx_ptp_log:1;        /* PTP log function
                                            0b0: do not log the tx timestamp (normal)
                                            0b1: need to log the tx timestamp for this packet */
            uint32 tx_ptp_offload:2;    /* PTP offload mechanism
                                            0b00: None (disabled)
                                            0b01: Follow_Up (grandmaster)
                                            0b10: Follow_Up (transparent)
                                            0b11: Pdelay_Resp_Follow_Up (responder) */
            uint32 new_sa:1;            /* MAC constraint (learn limit) */
            uint32 mac_cst:1;           /* MAC constraint (learn limit) */
            uint32 l2_pmv:1;            /* L2 entry port move */
            uint32 atk_type:5;          /* Attack type */
            uint32 dm_rxidx:6;          /* (1) CFM packet : FIFO index for CFM DM packet
                                           (2) B[4:0] indicates the routing point ID, Total routing point={dm_rxidx[4:0],da_pid},
                                               B[5] is no used for routing point. The field is used for routing function only when the l3_route bit is 1 */
            uint32 acl_hit:1;           /* Hit ACL rule */
            uint32 acl_id:12;           /* ACL index */
            uint32 eav_class_b:1;       /* Incoming is EAV packet with class B */
            uint32 eav_class_a:1;       /* Incoming is EAV packet with class A */
            uint32 reason:5;            /* Reason for CPU */
            uint32 acl_tos:1;           /* Modify TOS field */
            uint32 new_tos:8;           /* New TOS field */
            uint32 internal_pri:3;      /* Internal priority, for egress remarking */
            uint32 opri_act:1;          /* Egress ACL force the priority of O-TAG or C2SC */
            uint32 ipri_act:1;          /* Egress ACL force the priority of I-TAG */
            uint32 c2sc_pri:1;          /* C2SC force inner priority */
            uint32 eacl_itag:1;         /* Egress ACL force inner tag status */
            uint32 eacl_otag:1;         /* Egress ACL force outer tag status */
            uint32 c2sc_itag:1;         /* C2SC force inner tag status */
            uint32 c2sc_otag:1;         /* C2SC force outer tag status */
            uint32 itag_status:2;       /* Tag status for inner VLAN. 0=untag, 1=tag,2=Keep format */
            uint32 otag_status:2;       /* Tag status for outer VLAN. 0=untag, 1=tag,2=Keep format */
            uint32 eacl_ivid:1;         /* Egress ACL force inner VLAN ID */
            uint32 c2sc_ivid:1;         /* C2SC force inner VLAN ID */
            uint32 eacl_otpid:1;        /* Egress ACL and C2SC force outer TPID, 1=force, 0=no */
            uint32 otpid_idx:2;         /* Index of Egress TPID(Combine result of Egress ACL and C2SC) */
            uint32 eacl_itpid:1;        /* Egress ACL force inner TPID, 1=force, 0=no */
            uint32 c2sc_itpid:1;        /* C2SC table force inner TPID, 1=force, 0=no */
            uint32 itpid_idx:2;         /* Index of Inner TPID(Combine result of ACL & C2SC) */
            uint32 new_otag:16;         /* New Outer tag (VLAN ID is combination result of Egressa ACL, C2SC, IACL, port-based) */
            uint32 new_itag:16;         /* New inner tag(VLAN ID is combinational result of ACL, C2SC, port-protocol) */
            uint32 ale_ovid:1;          /* ACL force outer VLAN ID */
            uint32 l3_route:1;          /* Do L3 routing function */
            uint32 da_pid:11;           /* Pointer ID for DA, L3 routing function */
            uint32 normal_fwd:4;        /* Send a normal packet for mirroring port */
            uint32 swap_mac:1;          /* Swap DA and SA for oam loopback */
            uint32 cpu_fwd:1;           /* Forwarding a normal packet to CPU */
            uint32 sflow_info:6;        /* TX sFlow sample information.
                                            0~53 Sampling for port-idxx
                                            62: Ingress sampling
                                            63: No sampling */
            uint32 mirror_1_info:6;     /* mirroring information for set 1
                                            0~53 Egress mirroring for port-Idxx
                                            62 : Ingress mirroring
                                            63 : no mirroring */
            uint32 mirror_2_info:6;     /* mirroring information for set 2
                                            0~53 Egress mirroring for port-Idxx
                                            62 : Ingress mirroring
                                            63 : no mirroring */
            uint32 mirror_3_info:6;     /* mirroring information for set 3
                                            0~53 Egress mirroring for port-Idxx
                                            62 : Ingress mirroring
                                            63 : no mirroring */
            uint32 mirror_4_info:6;     /* mirroring information for set 4
                                            0~53 Egress mirroring for port-Idxx
                                            62 : Ingress mirroring
                                            63 : no mirroring */
            hsa_8390_qid_t qid[53];     /* Queue ID per-port. 53 ports x 3bit */
            uint32 color:2;             /* Color of packet */
            uint32 rvid_sel:1;          /* The source of RVID, 1=RVID get from outer tag, 0= RVID get from inner tag */
            uint32 rvid:12;             /* Relayed VID */
            uint32 dpc:6;               /* destination copy counter(port counter) */
            rtk_portmask_t dpm;         /* destination port mask */
            uint32 endsc:12;            /* end page point */
            uint32 bgdsc:12;            /* start page point */
            uint32 stpid_idx:2;         /* original TPID index for outer tag */
            uint32 ctpid_idx:2;         /* original TPID index for inner tag */
            uint32 org_otag:16;         /* Outer VLAN Tag */
            uint32 org_itag:16;         /* Inner VLAN Tag */
            uint32 ompls_if:1;          /* Has outer MPLS interface */
            uint32 impls_if:1;          /* Has inner MPLS interface */
            uint32 org_etag_if:1;       /* Has Extra tag */
            uint32 org_cputag_if:1;     /* Has CPU Tag */
            uint32 org_otag_if:1;       /* Has outer VLAN Tag(Tag or priority tag) */
            uint32 org_itag_if:1;       /* Has inner VLAN Tag(Tag or priority tag) */
            uint32 ipv6:1;              /* IPv6 Type */
            uint32 llc_other:1;         /* LEN < 0x0600, {DSAP, SSAP, CTL, OUI} != 0xaaaa03_000000 */
            uint32 pppoe_if:1;          /* PPPoE packet */
            uint32 rfc1042:1;           /* LLC SNAP */
            uint32 spa:6;               /* Source Port ID */
            uint32 errpkt:1;            /* Error packet(CRC,Symbol) */
            uint32 ipv4:1;              /* IPv4 Type */
            uint32 pktlen:14;           /* Packet length (Include CRC) */
        } r8390;
        struct
        {
            uint32 cpu_tag_if:1;        /*has cpu tag*/
            uint32 astagsts:1;          /*cpu assign tag status*/
            uint32 ale_otag_hit:1;      /*ale hit outer tag status rule*/
            uint32 ale_itag_hit:1;      /*ale hit inner tag status rule*/
            uint32 ale_otag_sts:2;      /*ale outer tag status*/
            uint32 ale_itag_sts:2;      /*ale inner tag status*/
            uint32 org_otag_if:1;       /*incoming packet has outer tag*/
            uint32 org_itag_if:1;       /*incoming packet has inner tag*/
            uint32 org_ovid:12;         /*incoming packet's outer vlan vid*/
            uint32 org_opri:3;          /*incoming packet's outer vlan pri*/
            uint32 org_ocfi:1;          /*incoming packet's outer vlan cfi*/
            uint32 org_ivid:12;         /*incoming packet's inner vlan vid*/
            uint32 org_ipri:3;          /*incoming packet's inner vlan pri*/
            uint32 org_icfi:1;          /*incoming packet's inner vlan cfi*/
            uint32 ale_ovid:12;         /*outer vlan vid determined by ale*/
            uint32 ale_ivid:12;         /*inner vlan vid determined by ale*/
            uint32 fwd_base:1;          /*fwd base vid sel*/
            uint32 fwd_vid:12;          /*fwd vid*/
            uint32 acl_otpid_hit:1;     /*acl outer tpid rule hit*/
            uint32 acl_otpid_idx:2;     /*acl outer tpid rule index*/
            uint32 acl_itpid_hit:1;     /*acl inner tpid rule hit*/
            uint32 acl_itpid_idx:2;     /*acl inner tpid rule index*/
            uint32 org_otpid_idx:2;     /*org outer tpid index*/
            uint32 org_itpid_idx:2;     /*org inner tpid index*/
            uint32 ale_int_pri:3;       /*ale internal priority*/
            uint32 reason:6;            /*cpu tag: reason to cpu*/
            uint32 l2_pmv:1;            /*cpu tag: L2 entry port move*/
            uint32 new_sa:1;            /*cpu tag: new source mac address*/
            uint32 atk_type:5;          /*cpu tag: attack type*/
            uint32 atk_hit:1;           /*cpu tag: hit HW attack*/
            uint32 mac_cst:1;           /*cpu tag: mac constraint*/
            uint32 acl_idx_9_0:10;      /*cpu tag: acl index*/
            uint32 acl_idx_10:1;        /*cpu tag: acl index*/
            uint32 acl_hit:1;           /*cpu tag: acl hit*/
            uint32 etag:1;              /*has extra tag*/
            uint32 rfc1042:1;           /*has snap header*/
            uint32 pppoe:1;             /*has pppoe*/
            uint32 ipv4:1;              /*is ipv4 packet*/
            uint32 ipv6:1;              /*is ipv6 packet*/
            uint32 acl_rmk:7;           /*HSA_ACL_RMK[6:3] = 0b0010 not hit ACL remarking action
                                          HSA_ACL_RMK[6] = 1(dscp remarking value), HSA_ACL_RMK[5:0] is DSCP remarking value;
                                          HSA_ACL_RMK[5:0] is DSCP remarking value;
                                          HSA_ACL_RMK[6] = 0(other remarking action), 
                                          HSA_ACL_RMK[5:3] is other remarking action, 
                                          HSA_ACL_RMK[2:0] is other remarking action value
                                         */
            uint32 l3_routing_idx:9;    /*routing mac table*/
            uint32 l3_routing:1;        /*1:reg l3 routing*/
            uint32 ale_ovid_hit:1;      /*ale hit outer tag vid rule*/
            uint32 ale_ivid_hit:1;      /*ale hit inner tag vid rule*/
            uint32 sp:5;                /*source port*/
            uint32 pktlen:14;           /*pkt length*/
            uint32 dpc:5;               /*destination port number*/
            uint32 mir1_info:5;         /*mirror set 0 information:
                                            0~28:tx mirror
                                            29: rx mirror
                                            30: rspan mirror
                                            31: no mirror*/
            uint32 mir2_info:5;         /*mirror set 1 information*/
            uint32 mir3_info:5;         /*mirror set 2 information*/
            uint32 mir4_info:5;         /*mirror set 3 information*/
            uint32 mir_nor_fwd:4;       /*normal forward to mirroring port*/
            uint32 is_eav_a:1;          /*is eav a pkt*/
            uint32 is_eav_b:1;          /*is eav b pkt*/
            uint32 da_lk_hit:1;         /*da lookup hit*/
            uint32 acl_as_pri:1;        /*acl assign priority*/
            uint32 fst_dsc:11;          /*first dsc*/
            uint32 lst_dsc:11;          /*last dsc*/
            uint32 dp_bf_rxq:1;         /*pkt droped before RXQ*/
            uint32 dpm:29;              /*destination port mask*/
            uint32 inbw_pktlen:14;      /*pkt length for rxq/rxp rate control*/
            uint32 txqid_nrm:3;         /*tx qid for normal port*/
            uint32 txqid_cpu:3;         /*tx qid for cpu port*/
            uint32 rxqid:2;             /*rx qid*/
            uint32 cpu_as_pri:1;        /*cpu tag assign priority*/
            uint32 ale_ctag_hit:1;      /*ALE add cpu tag or not*/
            uint32 rspan_rm:1;          /*RSPAN TAG remove or not*/
        } r8380;
    };
};
typedef struct hsa_param_s hsa_param_t;

struct hsm_pre_8390_s
{
    uint32 rma_byp_vlan:1; /* RMA bypass VLAN(Ingress filter, VLAN error, Frame type, CFI) */
    uint32 acl_bypass:3;   /* ACL bypass Ingress bandwidth , stp, ingress/egress drop
                              B[0] : bypass ingress bandwidth & Storm
                              B[1] : STP source check
                              B[2] : ingress/Egress check" */
    uint32 pppoe_ef:1;      /* Has PPPOE header */
    uint32 cmac:1;          /* DA is CPU MAC & VID=CPU VID */
    uint32 gmac:1;          /* DA is me */
    uint32 ipv4_hdr_err:1;  /* IPv4 Header error */
    uint32 ipv4_ttl_zero:1; /* IPv4 & TTL is zero */
    uint32 ipv4_opt:1;      /* IPv4 has opertion header */
    uint32 ipv6_hdr_err:1;  /* IPv6 header error */
    uint32 ipv6_hl_zero:1;  /* IPv6 & TTL=0 */
    uint32 ipv6_hop:1;      /* IPv6 with hop-by-hop header */
    uint32 oam_act:2;       /* OAM actionm0=FWD,1=Trap,2=drop,3=LB */
    uint32 stp:2;           /* Source port port state */
    uint32 vlan_zero:1;     /* VLAN member set is all zero */
    uint32 vlan_spm:1;      /* Source port is member of VLAN */
    uint32 unkey:1;         /* Unicast key type */
    uint32 mckey:1;         /* Multicast key type */
    uint32 vlan_profile:3;  /* VLAN proile */
    uint32 fid:8;           /* VLAN FID */
    uint32 meter_drop:1;    /* Drop by meter; 1=drop, 0=pass */
    uint32 cfm_act:2;       /* CFM action; 0=drop, 1trap, 2=FWD */
    uint32 cfm_idx:6;       /* CFM index */
    uint32 cfi_act:2;       /* CFI action; 0=fwd, 1=drop, 2=trap, 3=no used */
    uint32 tag_act:1;       /* TAG type action, 0=pass, 1=drop */
    uint32 nivid:12;        /* New inner VLAN ID */
    uint32 novid:12;        /* New outer VLAN ID */
    uint32 rvid:12;         /* Relay VLAN ID */
    uint32 acl_meet:1;      /* Hit ACL Forwarding/IVID/OVID action */
    uint32 aclc2sc:1;       /* Default action if the packet does not hit ACL & C2SC rule; 0=FWD,1=Drop */
    uint32 acl_hit:3;       /* ACL hit ; B[2]=MPLS, B[1]=FWD act, B[0]=Mirror Act */
    uint32 acl_idx:12;      /* Hit the index of ACL FWD action */
    uint32 acl_data:29;     /* B[28]=MPLS act
                               B[27:19]=Label Information Base entry index
                               B[18:16]=Forwarding action
                               B[15:2]=Forwarding information
                               B[1:0]=Mirror index */
    uint32 org_ivif_if:1;   /* Has inner VLAN tag */
    uint32 org_ivid:12;     /* Inner VLAN Tag ID */
    uint32 spe_trap_rsn:3;  /* Special trap reason; IGMP=1, MDL=2, EAPOL=3, ARP request=4, IPv6 neighbor=5 */
    uint32 spe_trap_act:1;  /* Special packet trap action, 0=no act(fwd), 1=trap */
    uint32 eav_classa:1;    /* Is EAV & Class A packet */
    uint32 eav_classb:1;    /* Is EAV & Class B packet */
    uint32 hwatt_rsn:5;     /* trap reason of HW attack */
    uint32 hwatt_act:2;     /* HW attack action; 0=normal, 1=drop,2=trap */
    uint32 rma_pkt:1;       /* Is RMA packet */
    uint32 rma_act:2;       /* Action for RMA packet; 0=table lookup,1=Drop,2=Trap */
    uint32 rma_learn:1;     /* Enable learning for RMA packet */
    uint32 rma_bypass:1;    /* Bypass Blocking/listening state for RMA packet */
    uint32 c2sc_hit_mode:1; /* C2SC mode, 0=VLAN, 1=MAC */
    uint32 c2sc_hit:1;      /* Hit C2SC table */
    uint32 c2sc_data:25;    /* C2SC data */
    uint32 internal_prio:3; /* Internal priority */
    uint32 dp_pre:2;        /* Drop precedence/color */
    uint32 protocol_storm:1;/* Drop by protocol-based storm */
    uint32 ibw_pass:1;      /* Pass by input bandwidth controller */
    uint32 slid:6;          /* Source logical port ID */
    uint32 trunk_grp:5;     /* Source port trunk information
                               B[4]=Source port belong to a trunk group
                               B[3:0]=Trunk group ID */
    uint32 sip:32;          /* Source IP address */
    uint32 dip:32;          /* Destination IP address */
    uint32 ipv6:1;          /* Is IPv6 packet */
    uint32 ipv4:1;          /* Is IPv4 packet */
    uint8  smac[6];         /* Source MAC address */
    uint8  dmac[6];         /* Destination MAC address */
    uint32 ctag_ln:1;       /* enable to learn SA by CTAG  */
    uint32 ctag_if:1;       /* Has CPU tag */
    uint32 pktlen:14;       /* Packet length (Include CRC) */

} ;
typedef struct hsm_pre_8390_s hsm_pre_8390_t;

struct hsm_lu_8390_s
{
    uint32 rma_byp_vlan:1; /* RMA bypass VLAN(Ingress filter, VLAN error, Frame type, CFI) */
    uint32 acl_bypass:3;   /* ACL bypass Ingress bandwidth , stp, ingress/egress drop
                              B[0] : bypass ingress bandwidth & Storm
                              B[1] : STP source check
                              B[2] : ingress/Egress check" */


    uint32 dpmove_act:2;        /* Dynamic port-moving action;0=fwd,1=drop,2=trap,3=copy */
    uint32 sport_filter_en:1;   /* Enable source port filter function */
    uint32 gmac_err:1;          /* L3 routing flow & DA!=Me */
    uint32 mc_sa:1;             /* SA is MC */
    uint32 vlan_spfilter:2;     /* VLAN source port filter cfg; 0=FWD,1=drop,2=Trap */
    rtk_portmask_t dpm;         /* destination port mask */
    uint32 lookup_type:2;       /* Lookup type; 0=UC, 1=L2 MC, 2=IPMC, 3=L3 routing */
    uint32 fwd_type:2;          /* Forwading type, 0=UC, 1=MC, 2=IPv4 MC, 3=IPv6 MC */
    uint32 newsa_ac:2;          /* Action of new SA; 0=FWD,1=drop,2=trap,3=Copy */
    uint32 pmove_act:2;         /* port-move Action for static entry;0=FWD,1=drop,2=trap,3=copy */
    uint32 maclimit_act:2;      /* mac limit action, 0=fwd,1=drop,2=trap,3=copy */
    uint32 route_data:30;       /* Routing information 
                                    B[29:19]=DA index
                                    B[18]=Select replaced VLAN
                                    B[17:6]=New VID
                                    B[5:0]=Destination port ID */
    uint32 da_hit:1;            /* DA exist in BCAM or SRAM */
    uint32 da_data:26;          /* UC DA information in memory
                                    B[25:20] = Port ID
                                    B[19:17]=Aging value
                                    B[16]=SA blocking
                                    B[15]=DA blocking
                                    B[14]=static
                                    B[13]=Suppression
                                    B[12]=Next Host
                                    B[11:0]=inner VID
                                    MC DA information format
                                    B[25:14]=0
                                    B[13:2]=MC index
                                    B[1:0]=Reserved
                                    IPv4MC & DIP+SIP format
                                    B[25:14]=FRVID
                                    B[13:2]=MC index
                                    B[1:0]=reserved */
    uint32 sa_hit:1;            /* SA exist in BCAM or SRAM */
    uint32 sa_data:26;          /* "SA information in memoryB[12:7] = Port ID
                                    B[6:4]=Aging value
                                    B[3]=SA blocking
                                    B[2]=DA blocking
                                    B[1]=static
                                    B[0]=Suppression" */
    uint32 cmac:1;          /* DA is CPU MAC & VID=CPU VID */
    uint32 gmac:1;          /* DA is me */
    uint32 ipv4_hdr_err:1;  /* IPv4 Header error */
    uint32 ipv4_opt:1;      /* IPv4 has opertion header */
    uint32 ipv4_ttl_zero:1; /* IPv4 & TTL is zero */
    uint32 ipv6_hdr_err:1;  /* IPv6 header error */
    uint32 ipv6_hl_zero:1;  /* IPv6 & TTL=0 */
    uint32 ipv6_hop:1;      /* IPv6 with hop-by-hop header */
    uint32 oam_act:2;       /* OAM actionm0=FWD,1=Trap,2=drop,3=LB */
    uint32 stp:2;           /* Source port port state */
    uint32 vlan_zero:1;     /* VLAN member set is all zero */
    uint32 vlan_spm:1;      /* Source port is member of VLAN */
    uint32 fid:8;           /* VLAN profile */
    uint32 meter_drop:1;    /* Drop by meter; 1=drop, 0=pass */
    uint32 cfm_act:2;       /* CFM action; 0=drop, 1trap, 2=FWD */
    uint32 cfm_idx:6;       /* CFM index */
    uint32 cfi_act:2;       /* CFI action; 0=fwd, 1=drop, 2=trap, 3=no used */
    uint32 tag_act:1;       /* TAG type action, 0=pass, 1=drop */
    uint32 nivid:12;        /* New inner VLAN ID */
    uint32 novid:12;        /* New outer VLAN ID */
    uint32 rvid:12;         /* Relay VLAN ID */
    uint32 acl_meet:1;      /* Hit ACL Forwarding/IVID/OVID action */
    uint32 aclc2sc:1;       /* Default action if the packet does not hit ACL & C2SC rule; 0=FWD,1=Drop */
    uint32 acl_hit:3;       /* ACL hit ; B[2]=MPLS, B[1]=FWD act, B[0]=Mirror Act */
    uint32 acl_idx:12;      /* Hit the index of ACL FWD action */
    uint32 acl_data:29;     /* B[28]=MPLS act
                               B[27:19]=Label Information Base entry index
                               B[18:16]=Forwarding action
                               B[15:2]=Forwarding information
                               B[1:0]=Mirror index */

    uint32 spe_trap_rsn:3;  /* Special trap reason; IGMP=1, MDL=2, EAPOL=3, ARP request=4, IPv6 neighbor=5 */
    uint32 spe_trap_act:1;  /* Special packet trap action, 0=no act(fwd), 1=trap */
    uint32 eav_classa:1;    /* Is EAV & Class A packet */
    uint32 eav_classb:1;    /* Is EAV & Class B packet */
    uint32 hwatt_rsn:5;     /* trap reason of HW attack */
    uint32 hwatt_act:2;     /* HW attack action; 0=normal, 1=drop,2=trap */
    uint32 rma_pkt:1;       /* Is RMA packet */
    uint32 rma_act:2;       /* Action for RMA packet; 0=table lookup,1=Drop,2=Trap */
    uint32 rma_bypass:1;    /* Bypass Blocking/listening state for RMA packet */
    uint32 c2sc_hit_mode:1; /* C2SC mode, 0=VLAN, 1=MAC */
    uint32 c2sc_hit:1;      /* Hit C2SC table */
    uint32 c2sc_data:25;    /* C2SC data */
    uint32 internal_prio:3; /* Internal priority */
    uint32 dp_pre:2;        /* Drop precedence/color */
    uint32 protocol_storm:1;/* Drop by protocol-based storm */
    uint32 ibw_pass:1;      /* Pass by input bandwidth controller */
    uint32 slid:6;          /* Source logical port ID */
    uint32 trunk_grp:5;     /* Source port trunk information
                               B[4]=Source port belong to a trunk group
                               B[3:0]=Trunk group ID */
} ;
typedef struct hsm_lu_8390_s hsm_lu_8390_t;

struct hsm_param_s
{
    union
    {
        struct
        {
            hsm_pre_8390_t  pre_data;
            hsm_lu_8390_t   lu_data;
        } r8390;
        struct
        {
            /*data0*/
            uint32 acl_idx0:8;  /*acl block 0 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            uint32 acl_idx1:8;  /*acl block 1 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            uint32 acl_idx2:8;  /*acl block 2 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            uint32 acl_idx3:8;  /*acl block 3 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            /*data1*/    
            uint32 acl_idx4:8;  /*acl block 4 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            uint32 acl_idx5:8;  /*acl block 5 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            uint32 acl_idx6:8;  /*acl block 6 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            uint32 acl_idx7:8;  /*acl block 7 hit entry, bit7 is valid bit, and bit6-bit0 means index*/
            /*data2*/
            uint32 acl_copy_hit:1;  /*acl copy action hit*/
            uint32 acl_dpm:29;      /*acl copy dpm*/
            uint32 acl_redir:1;     /*acl redirect*/
            uint32 acl_rout:1;      /*acl routing*/
            /*data3*/
            uint32 acl_normalPri_hit:1; /*normal priority assign*/
            uint32 acl_cpuPri_hit:1;    /*cpu priority action hit*/
            uint32 acl_cpuPri:3;        /*internal priority for cpu port*/
            uint32 acl_mir_hit:1;       /*acl mirror action hit*/
            uint32 acl_mir_act:1;       /*mirror original pkt or modified pkt*/    
            uint32 acl_fwd_info:13;     /*port ID[4:0]/index to multicast table[8:0] /index to  L2 table[12:0]*/
            uint32 acl_log_hit:1;       /*acl log ation hit*/
            uint32 acl_rmk_hit:1;       /*acl remark action hit*/
            uint32 acl_rmk_act:3;       /*000: Remark 802.1p priority of I-TAG
                                          001: Remark 802.1p priority of O-TAG
                                          010: Remark DSCP value
                                          011: Remark IP Precedence bits of TOS byte[IPv4 packets only]
                                          100: Copy priority from I-TAG to O-TAG
                                          101: Copy priority from O-TAG to I-TAG
                                          110: Keep 802.1p priority of I-TAG
                                          111: Keep 802.1p priority of O-TAG*/    
            uint32 acl_rmk_val:6;       /*[2:0]: New 802.1p priority of I-TAG
                                          [2:0]: New 802.1p priority of O-TAG
                                          [5:0]: DSCP value
                                          [2:0]: IPv4 precedence*/     
            uint32 acl_egr_tagSts_hit:1;/*acl egress tag status hit*/
            /*data4*/                                                                
            uint32 acl_ovid_hit:1;  /*ovid action hit*/
            uint32 acl_ivid_hit:1;  /*ivid action hit*/
            uint32 acl_fltr_hit:1;  /*filter action hit*/
            uint32 acl_fltr_dpm:29; /*acl filter dpm*/
            /*data5*/
            uint32 acl_force:1;     /*acl force bit*/
            uint32 acl_cpuTag:1;    /*packet carry cpu tag*/
            uint32 acl_idx_9_0:10;  /*acl drop/trap/copy action hit index*/
            uint32 acl_mir_idx:2;   /*mirror index*/
            uint32 acl_otpid_hit:1; /*acl otpid hit*/
            uint32 acl_itpid_hit:1; /*acl itpid hit*/
            uint32 acl_otpid_idx:2; /*acl otpid index*/
            uint32 acl_itpid_idx:2; /*acl itpid index*/
            uint32 acl_meter:1;     /*acl meter action hit*/ 
            uint32 rng_chk_ip:8;    /*ipv4 sip(8-bit)/ipv4 dip(8 bit)/ipv6 sip full(2 bit)/ipv6 dip full(2 bit)/ipv6 sip suffix(4 bit)/ipv6 dip suffix(4 bit) range checking result*/
            uint32 ale_drop:1;      /*ALE drop*/
            uint32 ale_trap:1;      /*ALE trap*/
            uint32 copyToCpu:1;     /*copy to cpu*/
            /*data6*/
            uint32 rng_chk_comm:16; /*ivid/ovid/l4sport/l4dport/packet length/fs0/fs1 range checking result*/
            uint32 rng_chk_spm:16;  /*source port mask range checking result*/
            /*data7*/
            uint32 rma_fld:1;       /*rma flood */
            uint32 l2_sa_hit:1;     /*SA look up hit*/
            uint32 l2_sa_idx:14;    /*SA look up hit index
                                      bit[13]==0,bit[12:0] is hash table index
                                      bit[13]==1,bit[12:0] is hash CAM index*/
            uint32 l2_da_lm:1;      /*l2 da look up miss*/
            uint32 l2_da_hit:1;     /*l2 address table look up hit*/
            uint32 l2_da_idx:14;    /*SA look up hit index
                                      bit[13]==0,bit[12:0] is hash table index
                                      bit[13]==1,bit[12:0] is hash CAM index*/
            /*data8*/
            uint32 dpm:29;      /*dpm*/
            uint32 int_pri:3;   /*internal priority*/
            /*data9*/
            uint32 atk_prvnt_rsn:5; /*attack type or reason*/
            uint32 eav_class_a:1;   /*eav class a pkt*/
            uint32 eav_class_b:1;   /*eav class b pkt*/    
            uint32 egr_vlan_lky:1;  /* only vlan egress filter leaky*/
            uint32 slp:5;           /*source logic port*/
            uint32 cpu_pri:3;       /*to cpu priority*/
            uint32 byp_storm:1;     /*by pass storm filter*/
            uint32 byp_igr_stp:1;   /*by pass ingress stp filter*/
            uint32 copy_igrVlan_lky:1;
            uint32 vlan_fwd_base:1; /*forward base on ivid or ovid, 1:ovid, 0:ivid*/    
            uint32 fid:12;          /*vlan fid*/
            /*data10*/
            uint32 ale_ovid:12;     /*ovid the ALE determined*/
            uint32 ale_ivid:12;     /*ivid the ALE determined*/
            uint32 otag_sts:2;      /*00: keep, 01:untag, 10:tag, 11: ale no determined*/
            uint32 itag_sts:2;      /*00: keep, 01:untag, 10:tag, 11: ale no determined*/
            uint32 input_qid:2;     /*00: queue0, 01:queue1, 10:queue2,  11:special queue*/
            uint32 igr_vlan_lky:1;
            uint32 dis_sa_lrn:1;    /*disbale sa learning*/
            /*data11*/
            uint32 knmc_byp_vlan_efilter:1; /*known l2/ip4/ip6 multicast by pass vlan egress filter */
            uint32 byp_igr_bwctrl:1;        /*by pass ingress bandwidth control*/
            uint32 byp_egr_stp_pmsk:29;     /*bypass egress stp port mask*/
            uint32 copy_igrStp_lky:1;       /*acl copy bypass ingress stp filter*/
            /*data12*/
            uint32 rspan_drop:1;    /*rspan drop because of incrocrect mirror setting.*/
            uint32 rma_drop:1;      /*rma drop*/    
            uint32 atk_prvnt_drop:1;/*attack drop*/
            uint32 acl_lm_drop:1;   /*acl look up miss drop*/
            uint32 acl_drop:1;      /*acl drop*/
            uint32 l3_rout_drop:1;  /*routing exception drop*/
            uint32 input_q_drop:1;  /*input queue drop*/
            uint32 acl_redir_drop:1;/*acl redirect drop because invalid destination port*/
            uint32 acl_meter_drop:1;/*acl meter drop*/
            uint32 vlan_aft_drop:1; /*vlan accept type drop*/
            uint32 vlan_cfi_drop:1; /*cfi=1 drop*/
            uint32 vlan_igr_drop:1; /*vlan ingress filter drop*/
            uint32 mstp_igr_drop:1; /*mstp drop*/
            uint32 l2_invldSA_drop:1;       /*invalid SA drop*/
            uint32 sTrap_selfMac_drop:1;    /*Special drop switch MAC*/
            uint32 l2_sa_blk:1;             /*is sa block*/ 
            uint32 l2_portMove_drop:1;      /*port move drop*/
            uint32 l2_newSA_drop:1;         /*new SA drop*/
            uint32 l2_sys_constrt_drop:1;   /*sys mac constrain drop*/
            uint32 l2_port_constrt_drop:1;  /*port mac constrain drop*/
            uint32 l2_vlan_constrt_drop:1;  /*vlan mac constrain drop*/
            uint32 l2_da_blk:1;             /*is da block*/
            uint32 l2_lm_drop:1;            /*da look up miss drop*/
            uint32 invld_dpm_drop:1;        /*invalid dpm drop*/
            uint32 invld_mcast_dpm_drop:1;  /*invalid multicast dpm drop*/ 
            uint32 vlan_egr_drop:1;         /*vlan egress filter drop*/
            uint32 mstp_egr_drop:1;         /*stp egress filter drop*/
            uint32 storm_drop:1;            /*storm filter drop*/
            uint32 src_port_filter_drop:1;  /*source port filter drop*/
            uint32 isolation_drop:1;        /*port isolation drop*/
            uint32 trunk_drop:1;            /*incorrect representative port drop*/
            uint32 mir_filter_drop:1;       /*mirror filter drop*/
            /*data13*/    
            uint32 acl_filter_drop:1;       /*acl filter dorp*/
            uint32 link_down_drop:1;        /*port link down drop*/
            uint32 flowCtrl_drop:1;         /*flow control drop*/
            uint32 tx_maxLen_drop:1;        /*tx pkt max length drop*/
            uint32 reason_n_drop:12;        /*other reason drop, reserved for more drop*/
            uint32 sTrap_copy:1;            /*special copy*/
            uint32 l2_sys_constrt_copy:1;   /*sys mac constrain copy*/
            uint32 l2_port_constrt_copy:1;  /*port mac constrain copy*/
            uint32 l2_vlan_constrt_copy:1;  /*vlan mac constrain copy*/
            uint32 l2_newSA_copy:1;         /*new SA copy*/
            uint32 l2_portMove_copy:1;      /*port move copy*/
            uint32 l2_lm_copy:1;            /*da look up miss copy*/
            uint32 acl_copy:1;              /*acl copy*/
            uint32 reason_n_copy:8;         /*other reason copy, reserved for more copy*/
            /*data14*/ 
            uint32 rma_trap:1;              /*rma trap*/
            uint32 atk_prvnt_trap:1;        /*attack trap*/
            uint32 rldp_rlpp_trap:1;        /*RLDP & RLPP trap*/
            uint32 sTrap_comm_trap:1;       /*Special trap IGMP
                                              Special trap MLD
                                              Special trap EAPOL
                                              Special trap ARP
                                              Special trap  IPV6NB*/
            uint32 acl_trap:1;              /*acl trap*/  
            uint32 l3_rout_trap:1;          /*routing exception trap*/
            uint32 vlan_cfi_trap:1;         /*cfi=1 trap*/
            uint32 vlan_igr_trap:1;         /*vlan ingress filter trap*/    
            uint32 sTrap_selfMac_trap:1;    /*Special trap switch MAC*/
            uint32 l2_portMove_trap:1;      /*port move trap*/
            uint32 l2_newSA_trap:1;         /*new SA trap*/
            uint32 l2_sys_constrt_trap:1;   /*sys mac constrain trap*/
            uint32 l2_port_constrt_trap:1;  /*port mac constrain trap*/
            uint32 l2_vlan_constrt_trap:1;  /*vlan mac constrain trap*/
            uint32 l2_lm_trap:1;            /*da look up miss trap*/
            uint32 reason_n_trap:17;        /*other reason trap, reserved for more trap*/
            /*data15*/
            uint32 acl_idx8:8;
            uint32 acl_idx9:8;
            uint32 acl_idx10:8;
            uint32 acl_idx11:8;
            /*data16*/
           uint32 acl_idx_10:1; 
        } r8380;
    };
};
typedef struct hsm_param_s hsm_param_t;

/* flowctrl Queue used page cnt */
typedef struct rtk_dbg_queue_usedPageCnt_s
{
    uint32 cntr[RTK_MAX_NUM_OF_QUEUE];
} rtk_dbg_queue_usedPageCnt_t;

/* private statistic counter index */
typedef enum rtk_dbg_mib_dbgType_e
{
#if defined(CONFIG_SDK_RTL8390)
    RTK_DBG_MIB_ALE_TX_GOOD_PKTS = 0,
    RTK_DBG_MIB_ERROR_PKTS,
    RTK_DBG_MIB_EGR_ACL_DROP,
    RTK_DBG_MIB_EGR_METER_DROP,
    RTK_DBG_MIB_OAM,
    RTK_DBG_MIB_CFM,
    RTK_DBG_MIB_VLAN_IGR_FLTR,
    RTK_DBG_MIB_VLAN_ERR,
    RTK_DBG_MIB_INNER_OUTER_CFI_EQUAL_1,
    RTK_DBG_MIB_VLAN_TAG_FORMAT,
    RTK_DBG_MIB_SRC_PORT_SPENDING_TREE = 10,
    RTK_DBG_MIB_INBW,
    RTK_DBG_MIB_RMA,
    RTK_DBG_MIB_HW_ATTACK_PREVENTION,
    RTK_DBG_MIB_PROTO_STORM,
    RTK_DBG_MIB_MCAST_SA,
    RTK_DBG_MIB_IGR_ACL_DROP,
    RTK_DBG_MIB_IGR_METER_DROP,
    RTK_DBG_MIB_DFLT_ACTION_FOR_MISS_ACL_AND_C2SC,
    RTK_DBG_MIB_NEW_SA,
    RTK_DBG_MIB_PORT_MOVE = 20,
    RTK_DBG_MIB_SA_BLOCKING,
    RTK_DBG_MIB_ROUTING_EXCEPTION,
    RTK_DBG_MIB_SRC_PORT_SPENDING_TREE_NON_FWDING,
    RTK_DBG_MIB_MAC_LIMIT,
    RTK_DBG_MIB_UNKNOW_STORM,
    RTK_DBG_MIB_MISS_DROP,
    RTK_DBG_MIB_CPU_MAC_DROP,
    RTK_DBG_MIB_DA_BLOCKING,
    RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_EGR_ACL,
    RTK_DBG_MIB_VLAN_EGR_FILTER = 30,
    RTK_DBG_MIB_SPANNING_TRE,
    RTK_DBG_MIB_PORT_ISOLATION,
    RTK_DBG_MIB_OAM_EGRESS_DROP,
    RTK_DBG_MIB_MIRROR_ISOLATION,
    RTK_DBG_MIB_MAX_LEN_BEFORE_EGR_ACL,
    RTK_DBG_MIB_SRC_PORT_FILTER_BEFORE_MIRROR,
    RTK_DBG_MIB_MAX_LEN_BEFORE_MIRROR,
    RTK_DBG_MIB_SPECIAL_CONGEST_BEFORE_MIRROR,
    RTK_DBG_MIB_LINK_STATUS_BEFORE_MIRROR,
    RTK_DBG_MIB_WRED_BEFORE_MIRROR = 40,
    RTK_DBG_MIB_MAX_LEN_AFTER_MIRROR,
    RTK_DBG_MIB_SPECIAL_CONGEST_AFTER_MIRROR,
    RTK_DBG_MIB_LINK_STATUS_AFTER_MIRROR,
    RTK_DBG_MIB_WRED_AFTER_MIRROR,
#endif
#if defined(CONFIG_SDK_RTL8380)
    RTK_DBG_MIB_ALE_TX_GOOD_PKTS_RTL8380,
    RTK_DBG_MIB_MAC_RX_DROP_RTL8380,
    RTK_DBG_MIB_ACL_FWD_DROP_RTL8380,
    RTK_DBG_MIB_HW_ATTACK_PREVENTION_DROP_RTL8380,
    RTK_DBG_MIB_RMA_DROP_RTL8380,
    RTK_DBG_MIB_VLAN_IGR_FLTR_DROP_RTL8380,
    RTK_DBG_MIB_INNER_OUTER_CFI_EQUAL_1_DROP_RTL8380,
    RTK_DBG_MIB_PORT_MOVE_DROP_RTL8380,
    RTK_DBG_MIB_NEW_SA_DROP_RTL8380,
    RTK_DBG_MIB_MAC_LIMIT_SYS_DROP_RTL8380,
    
    RTK_DBG_MIB_MAC_LIMIT_VLAN_DROP_RTL8380,
    RTK_DBG_MIB_MAC_LIMIT_PORT_DROP_RTL8380,
    RTK_DBG_MIB_SWITCH_MAC_DROP_RTL8380,
    RTK_DBG_MIB_ROUTING_EXCEPTION_DROP_RTL8380,
    RTK_DBG_MIB_DA_LKMISS_DROP_RTL8380,
    RTK_DBG_MIB_RSPAN_DROP_RTL8380,
    RTK_DBG_MIB_ACL_LKMISS_DROP_RTL8380,
    RTK_DBG_MIB_ACL_DROP_RTL8380,
    RTK_DBG_MIB_INBW_DROP_RTL8380,
    RTK_DBG_MIB_IGR_METER_DROP_RTL8380,
    
    RTK_DBG_MIB_ACCEPT_FRAME_TYPE_DROP_RTL8380,
    RTK_DBG_MIB_STP_IGR_DROP_RTL8380,
    RTK_DBG_MIB_INVALID_SA_DROP_RTL8380,
    RTK_DBG_MIB_SA_BLOCKING_DROP_RTL8380,
    RTK_DBG_MIB_DA_BLOCKING_DROP_RTL8380,
    RTK_DBG_MIB_L2_INVALID_DPM_DROP_RTL8380,
    RTK_DBG_MIB_MCST_INVALID_DPM_DROP_RTL8380,
    RTK_DBG_MIB_ROUTE_INVALID_NHP_DROP_RTL8380,
    RTK_DBG_MIB_STORM_SPPRS_DROP_RTL8380,
    RTK_DBG_MIB_LALS_DROP_RTL8380,
    
    RTK_DBG_MIB_VLAN_EGR_FILTER_DROP_RTL8380,
    RTK_DBG_MIB_STP_EGR_DROP_RTL8380,
    RTK_DBG_MIB_SRC_PORT_FILTER_DROP_RTL8380,
    RTK_DBG_MIB_PORT_ISOLATION_DROP_RTL8380,
    RTK_DBG_MIB_ACL_FLTR_DROP_RTL8380,
    RTK_DBG_MIB_MIRROR_FLTR_DROP_RTL8380,
    RTK_DBG_MIB_TX_MAX_DROP_RTL8380,
    RTK_DBG_MIB_LINK_DOWN_DROP_RTL8380,
    RTK_DBG_MIB_FLOW_CONTROL_DROP_RTL8380,
#endif
    RTK_DBG_MIB_DBG_TYPE_END
}rtk_dbg_mib_dbgType_t;


/* Function Name:
 *      hal_dumpHsb
 * Description:
 *      Dump hsb paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_dumpHsb(uint32 unit);

#if defined(CONFIG_SDK_RTL8328)
/* Function Name:
 *      hal_dumpPpi
 * Description:
 *      Dump ppi paramter of the specified device.
 * Input:
 *      unit - unit id
 *      index -ppi index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_dumpPpi(uint32 unit, uint32 index);

/* Function Name:
 *      hal_dumpPmi
 * Description:
 *      Dump pmi paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_dumpPmi(uint32 unit);
#endif

#if defined(CONFIG_SDK_RTL8389) || defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_dumpHsa
 * Description:
 *      Dump hsa paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_dumpHsa(uint32 unit);
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_dumpHsm
 * Description:
 *      Dump hsm paramter of the specified device.
 * Input:
 *      unit - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_dumpHsm(uint32 unit);
#endif

#if defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_dumpHsmIdx
 * Description:
 *      Dump hsm paramter of the specified device.
 * Input:
 *      unit - unit id
 *      index -hsm index
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_dumpHsmIdx(uint32 unit, uint32 index);
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getDbgCntr
 * Description:
 *      Get debug counter of the specified device.
 * Input:
 *      unit  - unit id
 *      type  - debug counter type
 * Output:
 *      pCntr - value of counter
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_getDbgCntr(uint32 unit, rtk_dbg_mib_dbgType_t type, uint32 *pCntr);
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlIgrPortUsedPageCnt
 * Description:
 *      Get flow control ingress used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pCntr    - value of used page count
 *      pMaxCntr - value of maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_getFlowCtrlIgrPortUsedPageCnt(uint32 unit, rtk_port_t port, uint32 * pCntr, uint32 * pMaxCntr);
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlEgrPortUsedPageCnt
 * Description:
 *      Get flow control egress used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pCntr    - value of used page count
 *      pMaxCntr - value of maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_getFlowCtrlEgrPortUsedPageCnt(uint32 unit, rtk_port_t port, uint32 *pCntr, uint32 *pMaxCntr);
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlSystemUsedPageCnt
 * Description:
 *      Get flow control system used page count of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      pCntr    - value of used page count
 *      pMaxCntr - value of maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_getFlowCtrlSystemUsedPageCnt(uint32 unit, uint32 *pCntr, uint32 *pMaxCntr);
#endif

#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380)
/* Function Name:
 *      hal_getFlowCtrlPortQueueUsedPageCnt
 * Description:
 *      Get flow control egress queue used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      pQCntr    - value of queue used page count
 *      pQMaxCntr - value of queue maximum used page count
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_getFlowCtrlPortQueueUsedPageCnt(uint32 unit, rtk_port_t port, rtk_dbg_queue_usedPageCnt_t *pQCntr, rtk_dbg_queue_usedPageCnt_t *pQMaxCntr);
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_resetFlowCtrlIgrPortUsedPageCnt
 * Description:
 *      Reset flow control ingress used page count of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_resetFlowCtrlIgrPortUsedPageCnt(uint32 unit, rtk_port_t port);
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_resetFlowCtrlEgrPortUsedPageCnt
 * Description:
 *      Reset flow control egress used page count including port and queue of the specified port.
 * Input:
 *      unit     - unit id
 *      port     - port id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_resetFlowCtrlEgrPortUsedPageCnt(uint32 unit, rtk_port_t port);
#endif

#if defined(CONFIG_SDK_RTL8390)
/* Function Name:
 *      hal_resetFlowCtrlSystemUsedPageCnt
 * Description:
 *      Reset flow control system used page count of the specified device.
 * Input:
 *      unit     - unit id
 * Output:
 *      None
 * Return:
 *      RT_ERR_OK
 *      RT_ERR_FAILED
 *      RT_ERR_UNIT_ID - invalid unit id
 */
extern int32 hal_resetFlowCtrlSystemUsedPageCnt(uint32 unitr);
#endif


#if defined(CONFIG_SDK_RTL8390) || defined(CONFIG_SDK_RTL8380) 
/* Function Name:
 *      hal_getWatchdogCnt
 * Description:
 *      Display phyWatchdog, serdesWatchdog and PktbufWatchdog.
 * Input:
 *      unit     - unit id
 *      type    - Which Watchdog value 
 * Output:
 *      count   - counter's value
 * Return:
 *      RT_ERR_OK
 */
extern int32 hal_getWatchdogCnt(uint32 unit, uint32 type, uint32 * count);
#endif

#endif  /* __HAL_MAC_DEBUG_H__ */
