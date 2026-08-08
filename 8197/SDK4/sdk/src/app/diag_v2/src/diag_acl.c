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
 * $Revision: 46263 $
 * $Date: 2014-01-23 14:42:29 +0800 (Thu, 23 Jan 2014) $
 *
 * Purpose : Definition those ACL command and APIs in the SDK diagnostic shell.
 *
 * Feature : The file have include the following module and sub-modules
 *           1) acl command
 *
 */

/*
 * Include Files
 */
#include <common/debug/rt_log.h>
#include <common/rt_error.h>
#include <common/rt_type.h>
#include <common/rt_error.h>
#include <common/util/rt_util.h>
#include <osal/memory.h>
#include <rtk/switch.h>
#include <rtk/port.h>
#include <rtk/acl.h>
#include <diag_util.h>
#include <diag_om.h>
#include <parser/cparser_priv.h>


/*
 * Symbol Definition
 */
typedef struct diag_acl_field_s
{
    rtk_acl_fieldType_t type;
    char                user_info[32];
    char                desc[104];
} diag_acl_field_t;

typedef struct diag_acl_template_s
{
    rtk_acl_templateFieldType_t type;
    char                        user_info[32];
    char                        desc[120];
} diag_acl_template_t;

/*
 * Data Declaration
 */

#ifdef CMD_ACL_INIT_ENTRY_BUFFER
static uint8 *diag_acl_entry_buffer = NULL;
#endif

static diag_acl_field_t diag_acl_field_list[] =
{
    {USER_FIELD_TEMPLATE_ID,                "template-id",          "template ID the entry maps to "},
    {USER_FIELD_FRAME_TYPE,                 "frame-type",           "frame type (0b00: ARP, 0b01: L2 only, 0b10: IPv4, 0b11: IPv6)"},
    {USER_FIELD_SPN,                        "spn",                  "source port number"},
    {USER_FIELD_SPMM,                       "spmm",                 "source port mask configurations check result"},
    {USER_FIELD_SPMM_0_1,                   "spmm01",               "source port mask [1:0] of check result in fix field"},
    {USER_FIELD_SPM,                        "spm",                  "source port mask"},
    {USER_FIELD_DMAC,                       "dmac",                 "destination MAC address"},
    {USER_FIELD_SMAC,                       "smac",                 "source MAC address"},
    {USER_FIELD_ITAG_EXIST,                 "itag-exist",           "packet with inner tag"},
    {USER_FIELD_OTAG_EXIST,                 "otag-exist",           "packet with outer tag"},
    {USER_FIELD_ITAG_FMT,                   "itag-fmt",             "0b0: inner tag packet, 0b1: untag/priority tag packet"},
    {USER_FIELD_OTAG_FMT,                   "otag-fmt",             "0b0: outer tag packet, 0b1: untag/priority tag packet"},
    {USER_FIELD_FRAME_TYPE_L2,              "L2-frame-type",        "L2 frame type(0b00: Ethernet, 0b01: LLC_SNAP, 0b10: LLC_Other)"},
    {USER_FIELD_ETAG_EXIST,                 "etag-exist",           "packet with extra tag"},
    {USER_FIELD_ETHERTYPE,                  "ethertype",            "ethernet type/length"},
    {USER_FIELD_ARPOPCODE,                  "arp-opcode",           "ARP/RARP Opcode"},
    {USER_FIELD_OTAG_PRI,                   "opri",                 "O-TAG priority"},
    {USER_FIELD_DEI_VALUE,                  "dei",                  "O-TAG DEI field"},
    {USER_FIELD_OTAG_VID,                   "ovid",                 "O-TAG VID"},
    {USER_FIELD_ITAG_PRI,                   "ipri",                 "I-TAG priority"},
    {USER_FIELD_CFI_VALUE,                  "cfi",                  "I-TAG CFI field"},
    {USER_FIELD_ITAG_VID,                   "ivid",                 "I-TAG VID"},
    {USER_FIELD_MGNT_VLAN,                  "mgnt-vlan",            "mangement VLAN"},
    {USER_FIELD_FWD_VID,                    "fvid",                 "forward VID"},
    {USER_FIELD_SWITCHMAC,                  "switch-mac",           "destination MAC address is switch MAC address"},
    {USER_FIELD_L4_PROTO,                   "L4-frame-type",        "layer 4 format (0b000: UDP, 0b001: TCP, 0b010: ICMP/ICMPv6, 0b011: IGMP, 0x1XXX: L4 other)"},
    {USER_FIELD_IP4_SIP,                    "ip4-sip",              "IPv4 source IP"},
    {USER_FIELD_IP4_DIP,                    "ip4-dip",              "IPv4 destination IP"},
    {USER_FIELD_IP6_SIP,                    "ip6-sip",              "IPv6 srouce address"},
    {USER_FIELD_IP6_DIP,                    "ip6-dip",              "IPv6 destinaction address"},
    {USER_FIELD_IP4TOS_IP6TC,               "tos-tc",               "IPv4 TOS, IPv6 Traffic Class"},
    {USER_FIELD_IP4PROTO_IP6NH,             "proto-nh",             "IPv4 protocol, IPv6 Next Header"},
    {USER_FIELD_IP_FLAG,                    "ip-flag",              "IP flag"},
    {USER_FIELD_IP4_TTL_IP6_HOPLIMIT,       "ttl-hoplimit",         "IPv4 TTL, IPv6 hop limit (0b00: TTL = 0, 0b01: TTL = 1, 0b10: 2<= TTL < 255, 0b11: TTL = 255)"},
    {USER_FIELD_L4_SRC_PORT,                "l4-sport",             "TCP/UDP source port"},
    {USER_FIELD_L4_DST_PORT,                "l4-dport",             "TCP/UDP destination port"},
    {USER_FIELD_IP6_AUTH_HDR_EXIST,         "ip6-auth-hdr-exist",   "IPv6 packet with authentication header"},
    {USER_FIELD_IP6_DEST_HDR_EXIST,         "ip6-dest-hdr-exist",   "IPv6 packet with destination option header"},
    {USER_FIELD_IP6_FRAG_HDR_EXIST,         "ip6-frag-hdr-exist",   "IPv6 packet with fragment header"},
    {USER_FIELD_IP6_ROUTING_HDR_EXIST,      "ip6-routing-hdr-exist","IPv6 packet with routing header"},
    {USER_FIELD_IP6_HOP_HDR_EXIST,          "ip6-hop-hdr-exist",    "IPv6 packet with hop-by-hop header"},
    {USER_FIELD_IGMP_TYPE,                  "igmp-type",            "IGMP type"},
    {USER_FIELD_TCP_ECN,                    "tcp-ecn",              "TCP ECN"},
    {USER_FIELD_TCP_FLAG,                   "tcp-flag",             "TCP flag"},
    {USER_FIELD_TCP_NONZEROSEQ,             "tcp-nonzero-seq",      "TCP packet with non zero sequence"},
    {USER_FIELD_ICMP_CODE,                  "icmp-code",            "ICMP/ICMPv6 code"},
    {USER_FIELD_ICMP_TYPE,                  "icmp-type",            "ICMP/ICMPv6 type"},
    {USER_FIELD_IP_NONZEROOFFSET,           "ip-nonzero-offset",    "IPv4/IPv6 fragment offset isn't 0"},
    {USER_FIELD_VID_RANGE0,                 "range-vid0",           "VID range check configuration 15-0 result"},
    {USER_FIELD_PORT_RANGE,                 "range-l4port",         "TCP/UDP port range check result"},
    {USER_FIELD_IP_RANGE,                   "range-ip",             "IPv4/IPv6 range check result"},
    {USER_FIELD_LEN_RANGE,                  "range-len",            "Packet length(CRC included) range check result"},
    {USER_FIELD_FIELD_SELECTOR_VALID_MSK,   "field-selector-mask",  "Field selector valid mask"},
    {USER_FIELD_FIELD_SELECTOR0,            "field-selector0",      "Field selector 0 output"},
    {USER_FIELD_FIELD_SELECTOR1,            "field-selector1",      "Field selector 1 output"},
    {USER_FIELD_FIELD_SELECTOR2,            "field-selector2",      "Field selector 2 output"},
    {USER_FIELD_FIELD_SELECTOR3,            "field-selector3",      "Field selector 3 output"},
    {USER_FIELD_TELNET,                     "telnet",               "telnet packet, tcp and destination port=23"},
    {USER_FIELD_SSH,                        "ssh",                  "ssh packet, tcp and destination port=22"},
    {USER_FIELD_HTTP,                       "http",                 "http packet, tcp and destination port=80"},
    {USER_FIELD_HTTPS,                      "https",                "https packet, tcp and destination port=443"},
    {USER_FIELD_SNMP,                       "snmp",                 "snmp packet, udp and destination port=161, or destination port=10161"},
    {USER_FIELD_UNKNOWN_L7,                 "unknown-l7",           "packet is not TELNET/SSH/HTTP/HTTPS/SNMP"},
    {USER_FIELD_L2_CRC_ERROR,               "l2-crc-error",         "L2 crc error packet"},
#if defined(CONFIG_SDK_RTL8390)
    {USER_FIELD_OMPLS_LABEL,                "ompls-label",          "outer MPLS label"},
    {USER_FIELD_OMPLS_EXP,                  "ompls-exp",            "outer MPLS label EXP"},
    {USER_FIELD_OMPLS_LABEL_EXIST,          "ompls-exist",          "outer MPLS label exist"},
    {USER_FIELD_IMPLS_LABEL,                "impls-label",          "inner MPLS label"},
    {USER_FIELD_IMPLS_EXP,                  "impls-exp",            "inner MPLS label EXP"},
    {USER_FIELD_IMPLS_LABEL_EXIST,          "impls-exist",          "inner MPLS label exist"},
    {USER_FIELD_IP_FRAGMENT_OFFSET,         "ip-offset",            "IP fragment offset"},
    {USER_FIELD_IP_DSCP,                    "dscp",                 "IP DSCP"},
    {USER_FIELD_IP6_ESP_HDR_EXIST,          "ip6-esp-hdr-exist",    "IPv6 packet with ESP header"},
    {USER_FIELD_VID_RANGE1,                 "range-vid1",           "VID range check configuration 31-16 result"},
    {USER_FIELD_FIELD_SELECTOR4,            "field-selector4",      "Field selector 4 output"},
    {USER_FIELD_FIELD_SELECTOR5,            "field-selector5",      "Field selector 5 output"},
    {USER_FIELD_FIELD_SELECTOR6,            "field-selector6",      "Field selector 6 output"},
    {USER_FIELD_FIELD_SELECTOR7,            "field-selector7",      "Field selector 7 output"},
    {USER_FIELD_FIELD_SELECTOR8,            "field-selector8",      "Field selector 8 output"},
    {USER_FIELD_FIELD_SELECTOR9,            "field-selector9",      "Field selector 9 output"},
    {USER_FIELD_FIELD_SELECTOR10,           "field-selector10",     "Field selector 10 output"},
    {USER_FIELD_FIELD_SELECTOR11,           "field-selector11",     "Field selector 11 output"},
    {USER_FIELD_DPM,                        "dpm",                  "destination port mask decided before egress ACL"},
    {USER_FIELD_DPMM,                       "dpmm",                 "destination port mask configurations check result decided before egress ACL"},
    {USER_FIELD_DPN,                        "dpn",                  "destination port number decided before egress ACL"},
    {USER_FIELD_UCAST_DA,                   "ucast-da",             "unicast DA"},
    {USER_FIELD_MCAST_DA,                   "mcast-da",             "multicast DA"},
    {USER_FIELD_SA_LUT_RESULT,              "sa-l2hit",             "SA lookup result. 0: lookup miss 1: lookup hit"},
    {USER_FIELD_DA_LUT_RESULT,              "da-l2hit",             "DA lookup result. 0: lookup miss 1: lookup hit"},
    {USER_FIELD_NONZERO_DPM,                "nonzero-dpm",          "non-zero destination port mask"},
    {USER_FIELD_L2_DPM,                     "l2-dpm",               "destination port mask decided by LUT"},
    {USER_FIELD_L2_DPN,                     "l2-dpn",               "destination port number decided by LUT"},
    {USER_FIELD_ATTACK,                     "attack-pkt",           "packet hit attack prevention criteria"},
    {USER_FIELD_DP,                         "dp",                   "drop precedence"},
    {USER_FIELD_INT_PRI,                    "int-pri",              "internal priority"},
    {USER_FIELD_IVID,                       "inner-vlan",           "inner VID given by ingress VLAN decision"},
    {USER_FIELD_OVID,                       "outer-vlan",           "outer VID given by ingress VLAN decision"},
    {USER_FIELD_IGR_ACL_DROP_HIT,           "iacl-drop",            "Ingress ACL drop action hit"},
    {USER_FIELD_IGR_ACL_COPY_HIT,           "iacl-copy",            "Ingress ACL copy action hit"},
    {USER_FIELD_IGR_ACL_REDIRECT_HIT,       "iacl-redirect",        "Ingress ACL redirect action hit"},
    {USER_FIELD_IGR_ACL_ROUTING_HIT,        "iacl-routing",         "Ingress ACL routing action hit"},
    {USER_FIELD_IP4_CHKSUM_ERROR,           "ip-checksum-error",    "IPv4 checksum error packet"},
#endif
#if defined(CONFIG_SDK_RTL8380)
    {USER_FIELD_DATYPE,                     "datype",               "0b0: unicast dmac, 0b1: broadcast dmac, 0b10: reserved, 0b11: multicast dmac"},
    {USER_FIELD_PPPOE,                      "pppoe",                "pppoe packet"},
    {USER_FIELD_FLDSEL_RANGE,               "range-field-selector", "field selector range check result"},
    {USER_FIELD_IP6_MOB_HDR_EXIST,          "ip6-mob-hdr-exist",    "IPv6 packet with mobility header"},
    {USER_FIELD_IGMP_GROUPIP,               "igmp-group-ip",        "IGMP group IP address"},
    {USER_FIELD_DROPPED,                    "dropped",              "packet dropped before entering ACL"},
    {USER_FIELD_RTKPP,                      "rtkpp",                "0b00: unknown, 0b01: RRCP, 0b10: RLPP, 0b11: RLDP"},
    {USER_FIELD_RRCPHLNULLMAC,              "rrcp-nullmac",         "NULL uplink MAC in RRCP hello reply"},
    {USER_FIELD_RRCPOP,                     "rrcp-opcode",          "0b00: RRCP hello packet, 0b01: Reserved, 0b10: RRCP get, 0b11: RRCP set"},
    {USER_FIELD_RRCPREPLY,                  "rrcp-reply",           "RRCP reply"},
    {USER_FIELD_RRCPVER,                    "rrcp-version",         "0b00: RRCPv1, 0b01: RRCPv3"},
    {USER_FIELD_RRCPKEY,                    "rrcp-key",             "RRCP key match system setting"},
    {USER_FIELD_FLOW_LABEL,                 "flow-label",           "IPv6 flow label"},

#endif
};

static diag_acl_template_t diag_acl_template_list[] =
{
    {TMPLTE_FIELD_SPMMASK,              "spmm",                     "soure portmask configuration result"},
    {TMPLTE_FIELD_SPM0,                 "spm0",                     "source portmask for port 0-15"},
    {TMPLTE_FIELD_SPM1,                 "spm1",                     "source portmask for port 16-31"},
    {TMPLTE_FIELD_DMAC0,                "dmac0",                    "destination MAC [15:0"},
    {TMPLTE_FIELD_DMAC1,                "dmac1",                    "destination MAC [31:16]"},
    {TMPLTE_FIELD_DMAC2,                "dmac2",                    "destination MAC [47:32]"},
    {TMPLTE_FIELD_SMAC0,                "smac0",                    "source MAC [15:0]"},
    {TMPLTE_FIELD_SMAC1,                "smac1",                    "source MAC [31:16]"},
    {TMPLTE_FIELD_SMAC2,                "smac2",                    "source MAC [47:32]"},
    {TMPLTE_FIELD_ETHERTYPE,            "ethertype",                "ethernet type"},
    {TMPLTE_FIELD_OTAG,                 "otag",                     "outer tag"},
    {TMPLTE_FIELD_ITAG,                 "itag",                     "inner tag"},
    {TMPLTE_FIELD_SIP0,                 "sip0",                     "IPv4 or IPv6 source IP[15:0] or ARP/RARP source protocol address in header"},
    {TMPLTE_FIELD_SIP1,                 "sip1",                     "IPv4 or IPv6 source IP[31:16] or ARP/RARP source protocol address in header"},
    {TMPLTE_FIELD_SIP2,                 "sip2",                     "IPv6 source IP[47:32]"},
    {TMPLTE_FIELD_SIP3,                 "sip3",                     "IPv6 source IP[63:48]"},
    {TMPLTE_FIELD_SIP4,                 "sip4",                     "IPv6 source IP[79:64]"},
    {TMPLTE_FIELD_SIP5,                 "sip5",                     "IPv6 source IP[95:80]"},
    {TMPLTE_FIELD_SIP6,                 "sip6",                     "IPv6 source IP[111:96]"},
    {TMPLTE_FIELD_SIP7,                 "sip7",                     "IPv6 source IP[127:112]"},
    {TMPLTE_FIELD_DIP0,                 "dip0",                     "IPv4 or IPv6 destination IP[15:0] or ARP/RARP destination protocol address in header"},
    {TMPLTE_FIELD_DIP1,                 "dip1",                     "IPv4 or IPv6 destination IP[31:16] or ARP/RARP destination protocol address in header"},
    {TMPLTE_FIELD_DIP2,                 "dip2",                     "IPv6 destination IP[47:32]"},
    {TMPLTE_FIELD_DIP3,                 "dip3",                     "IPv6 destination IP[63:48]"},
    {TMPLTE_FIELD_DIP4,                 "dip4",                     "IPv6 destination IP[79:64]"},
    {TMPLTE_FIELD_DIP5,                 "dip5",                     "IPv6 destination IP[95:80]"},
    {TMPLTE_FIELD_DIP6,                 "dip6",                     "IPv6 destination IP[111:96]"},
    {TMPLTE_FIELD_DIP7,                 "dip7",                     "IPv6 destination IP[127:112]"},
    {TMPLTE_FIELD_IP_TOS_PROTO,         "ip-tos-proto",             "IPv4 TOS/IPv6 traffic class and IPv4 protocold/IPv6 next header"},
    {TMPLTE_FIELD_L4_SPORT,             "l4-sport",                 "TCP/UDP source port"},
    {TMPLTE_FIELD_L4_DPORT,             "l4-dport",                 "TCP/UDP destination port"},
    {TMPLTE_FIELD_L34_HEADER,           "l34-header",               "packet with extra tag and IPv6 with auth, dest, frag, route, hop-by-hop option header, IGMP type, TCP flag"},
    {TMPLTE_FIELD_ICMP_IGMP,            "icmp-igmp",                "ICMP/ICMPv6 and IGMP info"},
    {TMPLTE_FIELD_FIELD_SELECTOR_VALID, "field_selector_valid_msk", "field selector valid mask"},
    {TMPLTE_FIELD_FIELD_SELECTOR_0,     "field_selector0",          "field selector 0"},
    {TMPLTE_FIELD_FIELD_SELECTOR_1,     "field_selector1",          "field selector 1"},
    {TMPLTE_FIELD_FIELD_SELECTOR_2,     "field_selector2",          "field selector 2"},
    {TMPLTE_FIELD_FIELD_SELECTOR_3,     "field_selector3",          "field selector 3"},
    {TMPLTE_FIELD_FWD_VID,              "fwd-vid",                  "forwarding VID"},
#if defined(CONFIG_SDK_RTL8390)
    {TMPLTE_FIELD_SPM2,                 "spm2",                     "source portmask for port 32-47"},
    {TMPLTE_FIELD_SPM3,                 "spm3",                     "source portmask for port 48-52 and packet len range check"},
    {TMPLTE_FIELD_IP_FLAG_OFFSET,       "ip-flag-offset",           "IP falgs, IP fragment offset, IPv6 DIP[63:48]"},
    {TMPLTE_FIELD_VID_RANG0,            "vid-range0",               "VID range check0"},
    {TMPLTE_FIELD_VID_RANG1,            "vid-range1",               "VID range check1"},
    {TMPLTE_FIELD_IP_L4PORT_RANG,       "ip-l4port-range",          "IP address and TCP/UDP port range check"},
    {TMPLTE_FIELD_IP_LEN_RANG,          "ip-len-range",             "IP address and packet len range check"},
    {TMPLTE_FIELD_FIELD_SELECTOR_4,     "field_selector4",          "field selector 4"},
    {TMPLTE_FIELD_FIELD_SELECTOR_5,     "field_selector5",          "field selector 5"},
    {TMPLTE_FIELD_FIELD_SELECTOR_6,     "field_selector6",          "field selector 6"},
    {TMPLTE_FIELD_FIELD_SELECTOR_7,     "field_selector7",          "field selector 7"},
    {TMPLTE_FIELD_FIELD_SELECTOR_8,     "field_selector8",          "field selector 8"},
    {TMPLTE_FIELD_FIELD_SELECTOR_9,     "field_selector9",          "field selector 9"},
    {TMPLTE_FIELD_FIELD_SELECTOR_10,    "field_selector10",         "field selector 10"},
    {TMPLTE_FIELD_FIELD_SELECTOR_11,    "field_selector11",         "field selector 11"},
    {TMPLTE_FIELD_OLABEL,               "inner-label-lsb",          "outer MPLS label[15:0]"},
    {TMPLTE_FIELD_ILABEL,               "outer-label-lsb",          "inner MPLS label[15:0]"},
    {TMPLTE_FIELD_OILABEL,              "label_msb",                "inner/outer MPLS label[19:16], inner/outer exp"},
    {TMPLTE_FIELD_DPMMASK,              "dpmm",                     "dest. portmask configuration result"},
    {TMPLTE_FIELD_DPM0,                 "dpm0",                     "dest. portmask for port 0-15"},
    {TMPLTE_FIELD_DPM1,                 "dpm1",                     "dest. portmask for port 16-31"},
    {TMPLTE_FIELD_DPM2,                 "dpm2",                     "dest. portmask for port 32-47"},
    {TMPLTE_FIELD_DPM3,                 "dpm3",                     "dest. portmask for port 48-63"},
    {TMPLTE_FIELD_L2DPM0,               "l2-dpm0",                  "L2 looup result dest. portmask for port 0-15"},
    {TMPLTE_FIELD_L2DPM1,               "l2-dpm1",                  "L2 looup result dest. portmask for port 16-31"},
    {TMPLTE_FIELD_L2DPM2,               "l2-dpm2",                  "L2 looup result dest. portmask for port 32-47"},
    {TMPLTE_FIELD_L2DPM3,               "l2-dpm3",                  "L2 looup result dest. portmask for port 48-63"},
    {TMPLTE_FIELD_IVLAN,                "inner-vlan",               "inner VALN given by ingress inner VLAN decision"},
    {TMPLTE_FIELD_OVLAN,                "outer-vlan",               "outer VALN given by ingress outer VLAN decision"},
#endif
#if defined(CONFIG_SDK_RTL8380)
    {TMPLTE_FIELD_IP_RANGE,             "ip-range",                 "IPv4/v6 address range check and inner untag/priority-tag, outer untag/priority-tag, flow label MSB 4-bit"},
    {TMPLTE_FIELD_RANGE_CHK,            "range-check",              "layer4 port/VID/packet length/field selector range check"},
    {TMPLTE_FIELD_FLOW_LABEL,           "flow-label",               "flow label LSB 15-bit"},
#endif
};

/*
 * Macro Declaration
 */
#define DIAG_UTIL_ACL_PARAM_LEN_CHK(field_len, input)           \
do {                                                            \
    if ((((int)field_len + 3) / 4) < ((int)strlen(input) - 2))  \
    {                                                           \
        diag_util_printf("field data or mask is too long!\n");  \
        return CPARSER_NOT_OK;                                  \
    }                                                           \
} while(0)

/*
 * Function Declaration
 */

/* convert Integer from string to number */
int32
diag_acl_str2IntArray (uint8 *int_array, uint8 *str, uint32 field_size)
{
    uint8  value = 0;
    uint32 str_idx, array_idx, hi_bits;

    if (!(int_array && str))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
        return  RT_ERR_FAILED;
    }

    if (!((strlen((char *)str) > 2) && ('0' == str[0]) && ('x' == str[1])))
    {
        RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
        return  RT_ERR_FAILED;
    }

    value = 0;
    array_idx = field_size - 1;
    hi_bits = 0;

    /* exclude 0x head */
    for (str_idx = (strlen((char *)str) - 1); str_idx >= 2; str_idx--)
    {
        if (('0' <= str[str_idx]) && ('9' >= str[str_idx]))
        {
            value = str[str_idx] - '0';
        }
        else if (('a' <= str[str_idx]) && ('f' >= str[str_idx]))
        {
            value = str[str_idx] - 'a' + 10;
        }
        else if (('A' <= str[str_idx]) && ('F' >= str[str_idx]))
        {
            value = str[str_idx] - 'A' + 10;
        }
        else
        {
            RT_ERR(RT_ERR_FAILED, (MOD_DIAGSHELL), "");
            return  RT_ERR_FAILED;
        }

        if(hi_bits == 1)
        {
            int_array[array_idx] = int_array[array_idx] + (value  << 4);
            hi_bits = 0;
            array_idx--;
        }
        else
        {
            int_array[array_idx] = value;
            hi_bits = 1;
        }
    }

    return RT_ERR_OK;
}

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_FIELD_LIST
/*
 * acl get entry phase <UINT:phase> field-list
 */
cparser_result_t
cparser_cmd_acl_get_entry_phase_phase_field_list(cparser_context_t *context,
        uint32_t *phase_ptr)
{
    uint32  i;
    uint32  unit = 0;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("%-25s %s\n", "field keyword", "field desciption");
    diag_util_mprintf("=====================================================\n");

    for (i = 0; i < (sizeof(diag_acl_field_list)/sizeof(diag_acl_field_t)); ++i)
    {
        ret = rtk_acl_ruleEntryField_check(unit, *phase_ptr,
                diag_acl_field_list[i].type);
        if (RT_ERR_OK != ret)
            continue;

        diag_util_mprintf("%-25s %s\n", diag_acl_field_list[i].user_info,
                diag_acl_field_list[i].desc);
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX
/*
 * acl get entry phase <UINT:phase> entry <UINT:index>
 */
cparser_result_t cparser_cmd_acl_get_entry_phase_phase_entry_index(cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    int32  ret = RT_ERR_FAILED;
    uint32 unit = 0, i;
    uint32 entry_size = 0, entry_mask_position;
    uint8  *pEntry_buffer = NULL;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntrySize_get(unit, *phase_ptr, &entry_size), ret);

    if ((pEntry_buffer = malloc(entry_size)) == NULL)
    {
        diag_util_printf("malloc() fail!\n");
        return CPARSER_NOT_OK;
    }

    if ((ret = rtk_acl_ruleEntry_read( unit, *phase_ptr, *index_ptr, pEntry_buffer)) != RT_ERR_OK)
    {
        DIAG_ERR_PRINT(ret);
        free(pEntry_buffer);
        return CPARSER_NOT_OK;
    }

    entry_mask_position = (entry_size / 2);
    for (i = 0; i < entry_mask_position; i++)
    {
        if((i % 8) == 0)
        {
            diag_util_mprintf("\n");
        }
        diag_util_printf("byte %2d | data:%02x mask:%02x\n", i, *(pEntry_buffer + i),
            *(pEntry_buffer + i + entry_mask_position));
    }
    diag_util_mprintf("\n");

    free(pEntry_buffer);
    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX_FIELD_FIELD_NAME
/*
 * acl get entry phase <UINT:phase> entry <UINT:index> field <STRING:field_name>
 */
cparser_result_t
cparser_cmd_acl_get_entry_phase_phase_entry_index_field_field_name(
        cparser_context_t *context,
        uint32_t *phase_ptr,
        uint32_t *index_ptr,
        char **field_name_ptr)
{
    uint32  num_element;
    uint32  unit = 0;
    uint32  field_size = 0;
    int32   ret;
    uint32  i;
    uint8   field_data[RTK_MAX_SIZE_OF_ACL_USER_FIELD];
    uint8   field_mask[RTK_MAX_SIZE_OF_ACL_USER_FIELD];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    num_element = (sizeof(diag_acl_field_list)/sizeof(diag_acl_field_t));
    memset(field_data, 0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memset(field_mask, 0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);

    for (i = 0; i < num_element; ++i)
    {
        if (!strcasecmp(*field_name_ptr, diag_acl_field_list[i].user_info))
            break;
    }

    if (num_element <= i)
    {
        diag_util_printf("Invalid field %s!\n", *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    ret = rtk_acl_ruleEntryField_check(unit, *phase_ptr,
            diag_acl_field_list[i].type);
    if (RT_ERR_OK != ret)
    {
        diag_util_printf("Field %s isn't supported in the phase!\n",
                *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntryFieldSize_get(unit, diag_acl_field_list[i].type, &field_size), ret);

    if ((field_size % 8) == 0)
    {
        field_size = (field_size / 8);
    }
    else
    {
        field_size = (field_size / 8) + 1;
    }

    ret = rtk_acl_ruleEntryField_read(unit, *phase_ptr, *index_ptr,
            diag_acl_field_list[i].type, field_data, field_mask);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
        diag_util_mprintf("Ingress ");
    else
        diag_util_mprintf("Egress ");

    diag_util_mprintf("index %d field %s\n", *index_ptr, *field_name_ptr);

    diag_util_mprintf("\tdata=0x");
    for (i = 0; i < field_size ; ++i)
    {
        diag_util_mprintf("%02x", field_data[i]);
    }
    diag_util_mprintf("\n");

    diag_util_mprintf("\tmask=0x");
    for (i = 0; i < field_size ; ++i)
    {
        diag_util_mprintf("%02x", field_mask[i]);
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_FIELD_FIELD_NAME_DATA_DATA_MASK_MASK
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> field <STRING:field_name> data <STRING:data> mask <STRING:mask>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_field_field_name_data_data_mask_mask(
        cparser_context_t *context,
        uint32_t *phase_ptr,
        uint32_t *index_ptr,
        char **field_name_ptr,
        char **data_ptr,
        char **mask_ptr)
{
    uint32  num_element, i;
    uint32  unit = 0;
    uint32  field_size = 0;
    int32   ret;
    uint8   field_data[RTK_MAX_SIZE_OF_ACL_USER_FIELD];
    uint8   field_mask[RTK_MAX_SIZE_OF_ACL_USER_FIELD];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    num_element = (sizeof(diag_acl_field_list)/sizeof(diag_acl_field_t));

    for (i = 0; i < num_element; ++i)
    {
        if (!strcasecmp(*field_name_ptr, diag_acl_field_list[i].user_info))
            break;
    }

    if (num_element <= i)
    {
        diag_util_printf("Invalid field %s!\n", *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    ret = rtk_acl_ruleEntryField_check(unit, *phase_ptr,
            diag_acl_field_list[i].type);
    if (RT_ERR_OK != ret)
    {
        diag_util_printf("Field %s isn't supported in the phase!\n",
                *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntryFieldSize_get(unit, diag_acl_field_list[i].type, &field_size), ret);

    DIAG_UTIL_ACL_PARAM_LEN_CHK(field_size, *data_ptr);
    DIAG_UTIL_ACL_PARAM_LEN_CHK(field_size, *mask_ptr);
    memset(field_data, 0x0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memset(field_mask, 0x0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);

    field_size = ((field_size + 7) / 8);

    if (diag_acl_str2IntArray (field_data, (uint8 *)*data_ptr, field_size) != RT_ERR_OK)
    {
        diag_util_printf("field data error!\n");
        return CPARSER_NOT_OK;
    }

    if (diag_acl_str2IntArray (field_mask, (uint8 *)*mask_ptr, field_size) != RT_ERR_OK)
    {
        diag_util_printf("field mask error!\n");
        return CPARSER_NOT_OK;
    }

    ret = rtk_acl_ruleEntryField_write(unit, *phase_ptr, *index_ptr,
            diag_acl_field_list[i].type, field_data, field_mask);

    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_PARTITION
/*
 * acl get partition
 */
cparser_result_t cparser_cmd_acl_get_partition(cparser_context_t *context)
{
    uint32  cute_line;
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_partition_get(unit, &cute_line);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("The partition is %d\n", cute_line);

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_PARTITION_VALUE
/*
 * acl set partition <UINT:value>
 */
cparser_result_t
cparser_cmd_acl_set_partition_value(cparser_context_t *context,
    uint32_t *value_ptr)
{
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_acl_partition_set(unit, *value_ptr);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_BLOCK_INDEX_POWER_STATE
/*
 * acl get block <UINT:index> power state
 */
cparser_result_t
cparser_cmd_acl_get_block_index_power_state(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_blockPwrEnable_get(unit, *index_ptr, &enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("The block %d power state is ", *index_ptr);
    if (ENABLED == enable)
        diag_util_mprintf("Enabled\n");
    else
        diag_util_mprintf("Disabled\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_BLOCK_INDEX_POWER_STATE_DISABLE_ENABLE
/*
 * acl set block <UINT:index> power state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_block_index_power_state_disable_enable(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(6, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    ret = rtk_acl_blockPwrEnable_set(unit, *index_ptr, enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_PORT_PORT_LOOKUP_MISS_ACTION
/*
 * acl get port <UINT:port> lookup-miss action
 */
cparser_result_t
cparser_cmd_acl_get_port_port_lookup_miss_action(cparser_context_t *context,
    uint32_t *port_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_lookupMissAct_t     lmAct;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_lookupMissAct_get(unit, *port_ptr, &lmAct);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Port %d lookup miss action is ");
    if (ACL_LOOKUPMISS_ACTION_PERMIT == lmAct)
        diag_util_mprintf("Permit\n");
    else
        diag_util_mprintf("Drop\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_PORT_PORT_LOOKUP_MISS_ACTION_PERMIT_DROP
/*
 * acl set port <UINT:port> lookup-miss action ( permit  drop )
 */
cparser_result_t
cparser_cmd_acl_set_port_port_lookup_miss_action_permit_drop(
    cparser_context_t *context,
    uint32_t *port_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_lookupMissAct_t     lmAct;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('p' == TOKEN_CHAR(6, 0))
    {
        lmAct = ACL_LOOKUPMISS_ACTION_PERMIT;
    }
    else
    {
        lmAct = ACL_LOOKUPMISS_ACTION_DROP;
    }

    ret = rtk_acl_lookupMissAct_set(unit, *port_ptr, lmAct);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif


#ifdef CMD_ACL_GET_PORT_PORT_LOOKUP_STATE
/*
 * acl get port <UINT:port> lookup state
 */
cparser_result_t
cparser_cmd_acl_get_port_port_lookup_state(cparser_context_t *context,
    uint32_t *port_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_portLookupEnable_get(unit, *port_ptr, &enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Port %d lookup state is ", *port_ptr);
    if (ENABLED == enable)
        diag_util_mprintf("Enabled\n");
    else
        diag_util_mprintf("Disabled\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_PORT_PORT_LOOKUP_STATE_DISABLE_ENABLE
/*
 * acl set port <UINT:port> lookup state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_port_port_lookup_state_disable_enable(
    cparser_context_t *context,
    uint32_t *port_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(6, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    ret = rtk_acl_portLookupEnable_set(unit, *port_ptr, enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_BLOCK_INDEX_LOOKUP_STATE
/*
 * acl get block <UINT:index> lookup state
 */
cparser_result_t
cparser_cmd_acl_get_block_index_lookup_state(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_blockLookupEnable_get(unit, *index_ptr, &enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("The block %d lookup state is ", *index_ptr);
    if (ENABLED == enable)
        diag_util_mprintf("Enabled\n");
    else
        diag_util_mprintf("Disabled\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_BLOCK_INDEX_LOOKUP_STATE_DISABLE_ENABLE
/*
 * acl set block <UINT:index> lookup state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_block_index_lookup_state_disable_enable(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_enable_t    enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(6, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    ret = rtk_acl_blockLookupEnable_set(unit, *index_ptr, enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX_VALIDATE
/*
 * acl get entry phase <UINT:phase> entry <UINT:index> validate
 */
cparser_result_t
cparser_cmd_acl_get_entry_phase_phase_entry_index_validate(cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32  unit;
    uint32  valid;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_ruleValidate_get(unit, *phase_ptr, *index_ptr, &valid);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
        diag_util_mprintf("Ingress ");
    else
        diag_util_mprintf("Egress ");

    diag_util_mprintf("entry index %d is ", *index_ptr);

    if (0 == valid)
        diag_util_mprintf("Invalidate\n");
    else
        diag_util_mprintf("Validate\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_STATE_VALID_INVALID
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> state ( valid | invalid )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_state_valid_invalid(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32  unit;
    uint32  valid;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('v' == TOKEN_CHAR(8, 0))
    {
        valid = 1;
    }
    else
    {
        valid = 0;
    }

    ret = rtk_acl_ruleValidate_set(unit, *phase_ptr, *index_ptr, valid);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX_OPERATION_REVERSE_STATE
/*
 * acl get entry phase <UINT:phase> entry <UINT:index> operation reverse state
 */
cparser_result_t
cparser_cmd_acl_get_entry_phase_phase_entry_index_operation_reverse_state(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_operation_t oper;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&oper, 0, sizeof(rtk_acl_operation_t));

    ret = rtk_acl_ruleOperation_get(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
        diag_util_mprintf("Ingress ");
    else
        diag_util_mprintf("Egress ");

    diag_util_mprintf("entry index %d reverse state is ", *index_ptr);

    if(ENABLED == oper.reverse)
    {
        diag_util_mprintf("Enabled");
    }
    else
    {
        diag_util_mprintf("Disabled");
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_entry_phase_entry_operation_reverse_state */
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX_OPERATION_AGGREGATE1_STATE
/*
 * acl get entry phase <UINT:phase> entry <UINT:index> operation aggregate1 state
 */
cparser_result_t
cparser_cmd_acl_get_entry_phase_phase_entry_index_operation_aggregate1_state(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_operation_t oper;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&oper, 0, sizeof(rtk_acl_operation_t));

    ret = rtk_acl_ruleOperation_get(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
        diag_util_mprintf("Ingress ");
    else
        diag_util_mprintf("Egress ");

    diag_util_mprintf("entry index %d aggregate 1 state is ", *index_ptr);

    if(ENABLED == oper.aggr_1)
    {
        diag_util_mprintf("Enabled");
    }
    else
    {
        diag_util_mprintf("Disabled");
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_entry_phase_entry_operation_aggregate1_state */
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX_OPERATION_AGGREGATE2_STATE
/*
 * acl get entry phase <UINT:phase> entry <UINT:index> operation aggregate2 state
 */
cparser_result_t
cparser_cmd_acl_get_entry_phase_phase_entry_index_operation_aggregate2_state(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_operation_t oper;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&oper, 0, sizeof(rtk_acl_operation_t));

    ret = rtk_acl_ruleOperation_get(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
        diag_util_mprintf("Ingress ");
    else
        diag_util_mprintf("Egress ");

    diag_util_mprintf("entry index %d aggregate 2 state is ", *index_ptr);

    if(ENABLED == oper.aggr_2)
    {
        diag_util_mprintf("Enabled");
    }
    else
    {
        diag_util_mprintf("Disabled");
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_entry_phase_entry_operation_aggregate2_state */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_OPERATION_REVERSE_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> operation reverse state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_operation_reverse_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_operation_t oper;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&oper, 0, sizeof(rtk_acl_operation_t));

    ret = rtk_acl_ruleOperation_get(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        oper.reverse = ENABLED;
    }
    else
    {
        oper.reverse = DISABLED;
    }

    ret = rtk_acl_ruleOperation_set(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_operation_reverse_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_OPERATION_AGGREGATE1_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> operation aggregate1 state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_operation_aggregate1_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_operation_t oper;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&oper, 0, sizeof(rtk_acl_operation_t));

    ret = rtk_acl_ruleOperation_get(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        oper.aggr_1 = ENABLED;
    }
    else
    {
        oper.aggr_1 = DISABLED;
    }

    ret = rtk_acl_ruleOperation_set(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_operation_aggregate1_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_OPERATION_AGGREGATE2_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> operation aggregate2 state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_operation_aggregate2_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_operation_t oper;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&oper, 0, sizeof(rtk_acl_operation_t));

    ret = rtk_acl_ruleOperation_get(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        oper.aggr_2 = ENABLED;
    }
    else
    {
        oper.aggr_2 = DISABLED;
    }

    ret = rtk_acl_ruleOperation_set(unit, *phase_ptr, *index_ptr, &oper);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_operation_aggregate2_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FORWARD_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action forward state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_forward_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_en = enable;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.fwd_en = enable;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_forward_state_disable_enable */
#endif

/*
*   acl set entry phase <UINT:phase> entry <UINT:index> action drop ( drop | withdraw )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_DROP_DROP_WITHDRAW

cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_drop_drop_withdraw(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        if('d' == TOKEN_CHAR(9, 0))
        {
            action.igr_acl.drop_data = ACL_IGR_ACTION_DROP_DROP;
        }
        else
        {
            action.igr_acl.drop_data = ACL_IGR_ACTION_DROP_WITHDRAW;
        }
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_forward_state_disable_enable */
#endif


#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_PERMIT
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action permit
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_permit(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_PERMIT;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.fwd_data.fwd_type = ACL_EGR_ACTION_FWD_PERMIT;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_permit */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_DROP
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action drop
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_drop(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *entry_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *entry_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_DROP;
    }
    else
    {
        action.egr_acl.fwd_data.fwd_type = ACL_EGR_ACTION_FWD_DROP;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *entry_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_drop */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_COPY_PORT_PORT
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action copy port <UINT:port>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_copy_port_port(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *port_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_COPY_TO_PORTID;
            action.igr_acl.fwd_data.fwd_info = *port_ptr;
        }
        else
        {
            action.egr_acl.fwd_data.fwd_type = ACL_EGR_ACTION_FWD_COPY_TO_PORTID;
            action.egr_acl.fwd_data.fwd_info = *port_ptr;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_COPY_TO_PORTID;
            action.igr_acl.fwd_data.info.copy_redirect_port.fwd_port_id = *port_ptr;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_copy_uni_dpn */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_COPY_PORTMASK_INDEX_PORTMASK_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action copy portmask-index <UINT:portmask_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_copy_portmask_index_portmask_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *portmask_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_COPY_TO_PORTMASK;
            action.igr_acl.fwd_data.fwd_info = *portmask_index_ptr;
        }
        else
        {
            action.egr_acl.fwd_data.fwd_type = ACL_EGR_ACTION_FWD_COPY_TO_PORTMASK;
            action.egr_acl.fwd_data.fwd_info = *portmask_index_ptr;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_COPY_TO_PORTMASK;
            action.igr_acl.fwd_data.info.copy_redirect_portMsk.fwd_idx = *portmask_index_ptr;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_copy_multi_portmask_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REDIRECT_PORT_PORT
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action redirect port <UINT:port>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_redirect_port_port(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *entry_ptr,
    uint32_t  *dpn_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *entry_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_REDIRECT_TO_PORTID;
            action.igr_acl.fwd_data.fwd_info = *dpn_ptr;
        }
        else
        {
            action.egr_acl.fwd_data.fwd_type = ACL_EGR_ACTION_FWD_REDIRECT_TO_PORTID;
            action.egr_acl.fwd_data.fwd_info = *dpn_ptr;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_REDIRECT_TO_PORTID;
            action.igr_acl.fwd_data.info.copy_redirect_port.fwd_port_id = *dpn_ptr;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *entry_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_redirect_uni_dpn */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REDIRECT_PORTMASK_INDEX_PORTMASK_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action redirect portmask-index <UINT:portmask_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_redirect_portmask_index_portmask_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *portmask_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_REDIRECT_TO_PORTMASK;
            action.igr_acl.fwd_data.fwd_info = *portmask_index_ptr;
        }
        else
        {
            action.egr_acl.fwd_data.fwd_type = ACL_EGR_ACTION_FWD_REDIRECT_TO_PORTMASK;
            action.egr_acl.fwd_data.fwd_info = *portmask_index_ptr;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_REDIRECT_TO_PORTMASK;
            action.igr_acl.fwd_data.info.copy_redirect_portMsk.fwd_idx = *portmask_index_ptr;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_redirect_multi_portmask_index */
#endif


/*
* acl set entry phase <UINT:phase> entry <UINT:index> action copy skip-ingress-vlan ( disable | enable )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_COPY_SKIP_INGRESS_VLAN_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_copy_skip_ingress_vlan_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.info.copy_redirect_port.skip_storm_igrVlan = enable;
        action.igr_acl.fwd_data.info.copy_redirect_portMsk.skip_storm_igrVlan = enable;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

#endif

/*
*    acl set entry phase <UINT:phase> entry <UINT:index> action redirect skip-storm ( disable | enable )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REDIRECT_SKIP_STORM_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_redirect_skip_storm_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.info.copy_redirect_port.skip_storm_igrVlan = enable;
        action.igr_acl.fwd_data.info.copy_redirect_portMsk.skip_storm_igrVlan = enable;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

#endif

/*
*   acl set entry phase <UINT:phase> entry <UINT:index> action redirect force ( disable | enable )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REDIRECT_FORCE_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_redirect_force_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.info.copy_redirect_port.force = enable;
        action.igr_acl.fwd_data.info.copy_redirect_portMsk.force = enable;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

#endif

/*
*  acl set entry phase <UINT:phase> entry <UINT:index> action forward cpu-tag ( disable | enable )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FORWARD_CPU_TAG_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_forward_cpu_tag_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.info.copy_redirect_port.cpu_tag = enable;
        action.igr_acl.fwd_data.info.copy_redirect_portMsk.cpu_tag = enable;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

#endif

/*
* acl set entry phase <UINT:phase> entry <UINT:index> action forward skip-ingress-stp ( disable | enable )

*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FORWARD_SKIP_INGRESS_STP_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_forward_skip_ingress_stp_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.info.copy_redirect_port.skip_igrStpDrop = enable;
        action.igr_acl.fwd_data.info.copy_redirect_portMsk.skip_igrStpDrop = enable;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

#endif

/*
*  acl set entry phase <UINT:phase> entry <UINT:index> action forward vlan-leaky ( disable | enable )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FORWARD_VLAN_LEAKY_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_forward_vlan_leaky_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_VLAN_LEAKY;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

#endif
/*
*   acl set entry phase <UINT:phase> entry <UINT:index> action forward drop ( disable | enable )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FORWARD_DROP_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_forward_drop_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_DROP;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}

#endif


/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action routing unicast <UINT:next_hop_index>
 */
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_ROUTING_UNICAST_NEXT_HOP_INDEX
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_routing_unicast_next_hop_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *next_hop_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (1 == *phase_ptr)
    {
        diag_util_printf("Egress ACL isn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    action.igr_acl.fwd_data.fwd_type = ACL_IGR_ACTION_FWD_UNICAST_ROUTING;
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.igr_acl.fwd_data.fwd_info = *next_hop_index_ptr;
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.fwd_data.info.route.idx = *next_hop_index_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_routing_unicast_next_hop_entry_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_STATISTIC_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action statistic state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_statistic_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.stat_en = enable;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.stat_en = enable;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_statistic_state_disable_enable */
#endif


#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FILTERING_PORT_PORT
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action filtering port <UINT:port>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_filtering_port_port(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *port_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.filter_data.flt_act = ACL_IGR_ACTION_FLT_SINGLE_PORT;
        action.igr_acl.filter_data.flt_info = *port_ptr;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_filtering_portmask_index */
#endif


#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FILTERING_PORTMASK_INDEX_PORTMASK_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action filtering portmask-index <UINT:portmask_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_filtering_portmask_index_portmask_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *portmask_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif
    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.fwd_data.fwd_type = ACL_EGR_ACTION_FWD_FILTERING;
        action.egr_acl.fwd_data.fwd_info = *portmask_index_ptr;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.filter_data.flt_act = ACL_IGR_ACTION_FLT_MULTIPLE_PORTS;
        action.igr_acl.filter_data.flt_info = *portmask_index_ptr;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_filtering_portmask_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_FILTER_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action filter state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_filter_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.filter_en = enable;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_statistic_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_STATISTIC_PACKET32_COUNTER_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action statistic packet32 <UINT:counter_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_statistic_packet32_counter_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *counter_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.stat_data.stat_type = STAT_TYPE_PACKET_BASED_32BIT;
        action.igr_acl.stat_data.stat_idx = *counter_index_ptr;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.stat_data.stat_type = STAT_TYPE_PACKET_BASED_32BIT;
        action.egr_acl.stat_data.stat_idx = *counter_index_ptr;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_statistic_packet32_counter_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_STATISTIC_BYTE64_COUNTER_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action statistic byte64 <UINT:counter_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_statistic_byte64_counter_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *counter_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.stat_data.stat_type = STAT_TYPE_BYTE_BASED_64BIT;
        action.igr_acl.stat_data.stat_idx = *counter_index_ptr;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.stat_data.stat_type = STAT_TYPE_BYTE_BASED_64BIT;
        action.egr_acl.stat_data.stat_idx = *counter_index_ptr;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_statistic_byte64_counter_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_MIRROR_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action mirror state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_mirror_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.mirror_en = enable;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.mirror_en = enable;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_mirror_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_MIRROR_MIRROR_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action mirror <UINT:mirror_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_mirror_mirror_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *mirror_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.mirror_data.mirror_set_idx = *mirror_index_ptr;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.mirror_data.mirror_type = ACL_ACTION_MIRROR_ORIGINAL;
        action.egr_acl.mirror_data.mirror_set_idx = *mirror_index_ptr;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_mirror_mirror_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_MIRROR_MIRROR_INDEX_ORIGINAL_MODIFIED
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action mirror <UINT:mirror_index> ( original | modified )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_mirror_mirror_index_original_modified(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *mirror_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if('o' == TOKEN_CHAR(10, 0))
        {
            action.egr_acl.mirror_data.mirror_type = ACL_ACTION_MIRROR_ORIGINAL;
        }
        else
        {
            action.egr_acl.mirror_data.mirror_type = ACL_ACTION_MIRROR_MODIFIED;
        }

        action.egr_acl.mirror_data.mirror_set_idx = *mirror_index_ptr;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if('o' == TOKEN_CHAR(10, 0))
        {
            action.igr_acl.mirror_data.mirror_type = ACL_ACTION_MIRROR_ORIGINAL;
        }
        else
        {
            action.igr_acl.mirror_data.mirror_type = ACL_ACTION_MIRROR_MODIFIED;
        }

        action.igr_acl.mirror_data.mirror_set_idx = *mirror_index_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_mirror_mirror_index_original_modified */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_METER_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action meter state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_meter_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.meter_en = enable;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.meter_en = enable;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_meter_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_METER_METER_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action meter <UINT:meter_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_meter_meter_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *meter_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.meter_data.meter_idx = *meter_index_ptr;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.meter_data.meter_idx = *meter_index_ptr;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_meter_meter_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_INNER_OUTER_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate ( inner | outer ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_inner_outer_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(11, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == *phase_ptr)
    {
        if('i' == TOKEN_CHAR(9, 0))
        {
            action.igr_acl.inner_vlan_assign_en = enable;
        }
        else
        {
            action.igr_acl.outer_vlan_assign_en = enable;
        }
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        if('i' == TOKEN_CHAR(9, 0))
        {
            action.egr_acl.inner_vlan_assign_en = enable;
        }
        else
        {
            action.egr_acl.outer_vlan_assign_en = enable;
        }
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_inner_outer_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_INNER_TPID_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate inner-tpid state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_inner_tpid_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (0 == *phase_ptr)
    {
        diag_util_printf("Ingress ACL doesn't support the action\n");
        return CPARSER_NOT_OK;
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(11, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.inner_vlan_data.tpid_assign = enable;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.itpid_en = enable;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_inner_tpid_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_INNER_TPID_INDEX_TPID_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate inner-tpid-index <UINT:tpid_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_inner_tpid_index_tpid_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *tpid_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.inner_vlan_data.tpid_idx = *tpid_index_ptr;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.itpid_data.tpid_idx = *tpid_index_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_inner_tpid_tpid_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_INNER_VID_ASSIGN_SHIFT_POSITIVE_SHIFT_NEGATIVE_SHIFT_FROM_OUTER_VALUE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate inner-vid ( assign | shift-positive | shift-negative | shift-from-outer ) <UINT:value>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_inner_vid_assign_shift_positive_shift_negative_shift_from_outer_value(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *value_ptr)
{
    uint32                          unit;
    uint32                          shift_sel = 0;
    int32                           ret;
    rtk_acl_action_t                action;
    rtk_acl_actionVlanAssignType_t  type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('a' == TOKEN_CHAR(10, 0))
    {
        type = ACL_ACTION_VLAN_ASSIGN_NEW_VID;
    }
    else if (0 == osal_strcmp(TOKEN_STR(10), "shift-positive"))
    {
        type = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID;
    }
    else if (0 == osal_strcmp(TOKEN_STR(10), "shift-negative"))
    {
        type = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID;
        shift_sel = 1;
    }
    else
    {
        type = ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_OUTER_VID;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.inner_vlan_data.vid_assign_type = type;
        action.igr_acl.inner_vlan_data.vid_shift_sel = shift_sel;
        action.igr_acl.inner_vlan_data.vid_value = *value_ptr;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.inner_vlan_data.vid_assign_type = type;
        action.egr_acl.inner_vlan_data.vid_shift_sel = shift_sel;
        action.egr_acl.inner_vlan_data.vid_value = *value_ptr;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_inner_vid_assign_shift_positive_shift_negative_shift_from_outer_value */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_INNER_VID_ASSIGN_PORT_BASED
/*
 *    acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate inner-vid assign-port-based
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_inner_vid_assign_port_based(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_action_t                action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.inner_vlan_data.vid_assign_type = ACL_IGR_ACTION_IVLAN_ASSIGN_PORT_BASED_INNER_VID;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_inner_vid_assign_shift_shift_from_outer_value */
#endif


#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_INNER_VID_COPY_FROM_OUTER
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate inner-vid copy-from-outer
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_inner_vid_copy_from_outer(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (0 == *phase_ptr)
    {
        diag_util_printf("Ingress ACL doesn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    action.egr_acl.inner_vlan_data.vid_assign_type =
            ACL_EGR_ACTION_IVLAN_ASSIGN_COPY_FROM_OUTER_VID;

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_inner_vid_copy_from_outer */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_OUTER_TPID_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate outer-tpid state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_outer_tpid_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif
    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(11, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.outer_vlan_data.tpid_assign = enable;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.otpid_en = enable;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_outer_tpid_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_OUTER_TPID_TPID_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate outer-tpid-index <UINT:tpid_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_outer_tpid_index_tpid_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *tpid_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.outer_vlan_data.tpid_idx = *tpid_index_ptr;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.otpid_data.tpid_idx = *tpid_index_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_outer_tpid_tpid_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_OUTER_VID_ASSIGN_SHIFT_POSITIVE_SHIFT_NEGATIVE_SHIFT_FROM_INNER_VALUE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate outer-vid ( assign | shift-positive | shift-negative | shift-from-inner ) <UINT:value>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_outer_vid_assign_shift_positive_shift_negative_shift_from_inner_value(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *value_ptr)
{
    uint32                          unit;
    uint32                          shift_sel = 0;
    int32                           ret;
    rtk_acl_action_t                action;
    rtk_acl_actionVlanAssignType_t  type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('a' == TOKEN_CHAR(10, 0))
    {
        type = ACL_ACTION_VLAN_ASSIGN_NEW_VID;
    }
    else if (0 == osal_strcmp(TOKEN_STR(10), "shift-positive"))
    {
        type = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID;
    }
    else if (0 == osal_strcmp(TOKEN_STR(10), "shift-negative"))
    {
        type = ACL_ACTION_VLAN_ASSIGN_SHIFT_VID;
        shift_sel = 1;
    }
    else
    {
        type = ACL_ACTION_VLAN_ASSIGN_SHIFT_FROM_INNER_VID;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.outer_vlan_data.vid_assign_type = type;
        action.igr_acl.outer_vlan_data.vid_shift_sel = shift_sel;
        action.igr_acl.outer_vlan_data.vid_value = *value_ptr;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.outer_vlan_data.vid_assign_type = type;
        action.egr_acl.outer_vlan_data.vid_shift_sel = shift_sel;
        action.egr_acl.outer_vlan_data.vid_value = *value_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_outer_vid_assign_shift_positive_shift_negative_shift_from_inner_value */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_OUTER_VID_ASSIGN_PORT_BASED
/*
 *   acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate outer-vid assign-port-based
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_outer_vid_assign_port_based(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_action_t                action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.outer_vlan_data.vid_assign_type = ACL_IGR_ACTION_OVLAN_ASSIGN_PORT_BASED_OUTER_VID;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_inner_vid_assign_shift_shift_from_outer_value */
#endif


#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_VLAN_XLATE_OUTER_VID_COPY_FROM_INNER
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action vlan-xlate outer-vid copy-from-inner
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_vlan_xlate_outer_vid_copy_from_inner(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (0 == *phase_ptr)
    {
        diag_util_printf("Ingress ACL doesn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    action.egr_acl.outer_vlan_data.vid_assign_type =
            ACL_ACTION_VLAN_ASSIGN_COPY_FROM_INNER_VID;

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_vlan_xlate_outer_vid_copy_from_inner */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_PRIORITY_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action priority state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_priority_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *entry_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *entry_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == *phase_ptr)
    {
        action.igr_acl.pri_en = enable;
    }
#if defined(CONFIG_SDK_RTL8390)
    else
    {
        action.egr_acl.pri_en = enable;
    }
#endif
    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *entry_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_priority_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_PRIORITY_CPU_PORT_ONLY_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action priority cpu-port-only state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_priority_cpu_port_only_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(11, 0))
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.pri_data.pri_act = enable;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.cpu_pri_en = enable;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_priority_cpu_port_only_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_PRIORITY_CPU_PORT_ONLY_PRIORITY
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action priority cpu-port-only <UINT:priority>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_priority_cpu_port_only_priority(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *priority_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.cpu_pri_data.pri = *priority_ptr;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_priority_priority */
#endif


#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_PRIORITY_PRIORITY
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action priority <UINT:priority>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_priority_priority(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *priority_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.egr_acl.pri_data.pri_act = 0;
            action.igr_acl.pri_data.pri = *priority_ptr;
        }
        else
        {
            action.egr_acl.pri_data.pri = *priority_ptr;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.pri_data.pri = *priority_ptr;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_priority_priority */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_MPLS_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action mpls state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_mpls_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (1 == *phase_ptr)
    {
        diag_util_printf("Egress ACL isn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    action.igr_acl.mpls_en = enable;

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_mpls_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_MPLS_PUSH_OUTER_PUSH_INNER_OUTER_LIB_INDEX
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action mpls ( push-outer | push-inner-outer ) <UINT:lib_index>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_mpls_push_outer_push_inner_outer_lib_index(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *lib_index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (1 == *phase_ptr)
    {
        diag_util_printf("Egress ACL isn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == osal_strcmp(TOKEN_STR(9), "push-outer"))
    {
        action.igr_acl.mpls_data.mpls_act = 0;
    }
    else
    {
        action.igr_acl.mpls_data.mpls_act = 1;
    }

    action.igr_acl.mpls_data.mpls_idx = *lib_index_ptr;

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_mpls_push_outer_push_inner_outer_lib_index */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_BYPASS_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action bypass state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_bypass_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (1 == *phase_ptr)
    {
        diag_util_printf("Egress ACL isn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    action.igr_acl.bypass_en = enable;

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_bypass_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_BYPASS_INGRESS_BANDWIDTH_CONTROL_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action bypass ingress-bandwidth-control state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_bypass_ingress_bandwidth_control_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (1 == *phase_ptr)
    {
        diag_util_printf("Egress ACL isn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(11, 0))
    {
        action.igr_acl.bypass_data.ibc_sc = 1;
    }
    else
    {
        action.igr_acl.bypass_data.ibc_sc = 0;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_bypass_igr_bw_ctrl_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_BYPASS_INGRESS_STP_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action bypass ingress-stp state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_bypass_ingress_stp_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (1 == *phase_ptr)
    {
        diag_util_printf("Egress ACL isn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(11, 0))
    {
        action.igr_acl.bypass_data.igr_stp = 1;
    }
    else
    {
        action.igr_acl.bypass_data.igr_stp = 0;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_bypass_igr_stp_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_BYPASS_INGRESS_DROP_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action bypass ingress-drop state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_bypass_ingress_drop_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (1 == *phase_ptr)
    {
        diag_util_printf("Egress ACL isn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(11, 0))
    {
        action.igr_acl.bypass_data.all = 1;
    }
    else
    {
        action.igr_acl.bypass_data.all = 0;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_bypass_igr_drop_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_SHAPER_STATE_DISABLE_ENABLE
/*
 *    acl set entry phase <UINT:phase> entry <UINT:index> action shaper state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_shaper_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        if('e' == TOKEN_CHAR(10, 0))
        {
            enable = ENABLED;
        }
        else
        {
            enable = DISABLED;
        }

        action.igr_acl.shaper_en = enable;
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_SHAPER_QUEUE_0_QUEUE_1_QUEUE_2_QUEUE_3
/*
 *   acl set entry phase <UINT:phase> entry <UINT:index> action shaper ( queue-0 | queue-1 | queue-2 | queue-3 )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_shaper_queue_0_queue_1_queue_2_queue_3(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
    {
        if('0' == TOKEN_CHAR(9, 6))
        {
            action.igr_acl.shaper_data.shaper_act = ACL_IGR_ACTION_SHAPER_Q0;
        }
        else if('1' == TOKEN_CHAR(9, 6))
        {
            action.igr_acl.shaper_data.shaper_act = ACL_IGR_ACTION_SHAPER_Q1;
        }
        else if('2' == TOKEN_CHAR(9, 6))
        {
            action.igr_acl.shaper_data.shaper_act = ACL_IGR_ACTION_SHAPER_Q2;
        }
        else
        {
            action.igr_acl.shaper_data.shaper_act = ACL_IGR_ACTION_SHAPER_Q3;
        }
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_meter_meter_index */
#endif



#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.rmk_en = enable;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.remark_en = enable;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_INNER_PRIORITY_OUTER_PRIORITY_PRIORITY
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark ( inner-priority | outer-priority ) <UINT:priority>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_inner_priority_outer_priority_priority(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *priority_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if('i' == TOKEN_CHAR(9, 0))
        {
            action.egr_acl.rmk_data.rmk_type = ACL_ACTION_REMARK_INNER_USER_PRI;
        }
        else
        {
            action.egr_acl.rmk_data.rmk_type = ACL_ACTION_REMARK_OUTER_USER_PRI;
        }

        action.egr_acl.rmk_data.rmk_value = *priority_ptr;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if('i' == TOKEN_CHAR(9, 0))
        {
            action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_INNER_USER_PRI;
        }
        else
        {
            action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_OUTER_USER_PRI;
        }

        action.igr_acl.remark_data.rmk_info = *priority_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_inner_pri_outer_pri_priority */
#endif

/*
*  acl set entry phase <UINT:phase> entry <UINT:index> action remark keep ( inner-priority | outer-priority )
*/
#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_KEEP_INNER_PRIORITY_OUTER_PRIORITY

cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_keep_inner_priority_outer_priority(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
    if (0 == *phase_ptr)
    {
        if('i' == TOKEN_CHAR(10, 0))
        {
            action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_KEEP_INNER_USER_PRI;
        }
        else
        {
            action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_KEEP_OUTER_USER_PRI;
        }
    }

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_inner_pri_outer_pri_priority */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_DSCP_AND_INNER_DSCP_DSCP_INNER_PRIORITY_PRIORITY
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark dscp-and-inner dscp <UINT:dscp> inner-priority <UINT:priority>
 */
cparser_result_t cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_dscp_and_inner_dscp_dscp_inner_priority_priority(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr,
    uint32_t *dscp_ptr,
    uint32_t *priority_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (0 == *phase_ptr)
    {
        diag_util_printf("Ingress ACL doesn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    action.egr_acl.rmk_data.rmk_value = *dscp_ptr;
    action.egr_acl.rmk_data.rmk_value_ext = *priority_ptr;

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_INNER_AND_OUTER_INNER_PRIORITY_INNER_PRI_OUTER_PRIORITY_OUTER_PRI
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark inner-and-outer inner-priority <UINT:inner_pri> outer-priority <UINT:outer_pri>
 */
cparser_result_t cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_inner_and_outer_inner_priority_inner_pri_outer_priority_outer_pri(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr,
    uint32_t *inner_pri_ptr,
    uint32_t *outer_pri_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (0 == *phase_ptr)
    {
        diag_util_printf("Ingress ACL doesn't support the action\n");
        return CPARSER_NOT_OK;
    }

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    action.egr_acl.rmk_data.rmk_value = *inner_pri_ptr;
    action.egr_acl.rmk_data.rmk_value_ext = *outer_pri_ptr;

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_DSCP_DSCP
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark dscp <UINT:dscp>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_dscp_dscp(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *dscp_value_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.rmk_data.rmk_type = ACL_ACTION_REMARK_DSCP;
        action.egr_acl.rmk_data.rmk_value = *dscp_value_ptr;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_DSCP;
        action.igr_acl.remark_data.rmk_info = *dscp_value_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_dscp_dscp_value */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_IP_PRECEDENCE_PRECEDENCE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark ip-precedence <UINT:precedence>
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_ip_precedence_precedence(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr,
    uint32_t  *precedence_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.rmk_data.rmk_type = ACL_ACTION_REMARK_IP_PRECEDENCE;
        action.egr_acl.rmk_data.rmk_value = *precedence_ptr;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_IP_PRECEDENCE;
        action.igr_acl.remark_data.rmk_info = *precedence_ptr;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_ip_precedence_precedence */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_COPY_INNER_TO_OUTER
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark copy-inner-to-outer
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_copy_inner_to_outer(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.rmk_data.rmk_type = ACL_ACTION_REMARK_COPY_IPRI_TO_OPRI;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_COPY_IPRI_TO_OPRI;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_copy_inner_to_outer */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_REMARK_COPY_OUTER_TO_INNER
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action remark copy-outer-to-inner
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_remark_copy_outer_to_inner(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        action.egr_acl.rmk_data.rmk_type = ACL_ACTION_REMARK_COPY_OPRI_TO_IPRI;
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        action.igr_acl.remark_data.rmk_act = ACL_ACTION_REMARK_COPY_OPRI_TO_IPRI;
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_remark_copy_outer_to_inner */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_TAG_STATE_DISABLE_ENABLE
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action tag state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_tag_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;
    rtk_enable_t        enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(10, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
        else
        {
            action.egr_acl.tag_sts_en = enable;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            action.igr_acl.egrTagStat_en = enable;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_tag_state_disable_enable */
#endif

#ifdef CMD_ACL_SET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION_TAG_INNER_OUTER_UNTAG_TAG_KEEP_CONTENT_RESERVED
/*
 * acl set entry phase <UINT:phase> entry <UINT:index> action tag ( inner | outer ) ( untag | tag | keep-content | reserved )
 */
cparser_result_t
cparser_cmd_acl_set_entry_phase_phase_entry_index_action_tag_inner_outer_untag_tag_keep_content_reserved(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_action_t            action;
    rtk_acl_actionTagStsFmt_t   status;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (0 == *phase_ptr)
        {
            diag_util_printf("Ingress ACL doesn't support the action\n");
            return CPARSER_NOT_OK;
        }
    }
#endif

    memset(&action, 0, sizeof(rtk_acl_action_t));

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('u' == TOKEN_CHAR(10, 0))
    {
        status = ACL_ACTION_TAG_STS_UNTAG;
    }
    else if('t' == TOKEN_CHAR(10, 0))
    {
        status = ACL_ACTION_TAG_STS_TAG;
    }
    else if('k' == TOKEN_CHAR(10, 0))
    {
        status = ACL_ACTION_TAG_STS_KEEP_CONTENT;
    }
    else
    {
        status = ACL_ACTION_TAG_STS_NOP;
    }

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if('i' == TOKEN_CHAR(9, 0))
        {
            action.egr_acl.tag_sts_data.itag_sts = status;
        }
        else
        {
            action.egr_acl.tag_sts_data.otag_sts = status;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if('i' == TOKEN_CHAR(9, 0))
        {
            action.igr_acl.egrTagStat_data.itag_sts = status;
        }
        else
        {
            action.igr_acl.egrTagStat_data.otag_sts = status;
        }
    }
#endif

    ret = rtk_acl_ruleAction_set(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_phase_entry_action_tag_inner_outer_untag_tag_keep_content_reserved */
#endif

#ifdef CMD_ACL_DEL_ENTRY_PHASE_PHASE_START_START_END_END
/*
 * acl del entry phase <UINT:phase> start <UINT:start> end <UINT:end>
 */
cparser_result_t
cparser_cmd_acl_del_entry_phase_phase_start_start_end_end(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *start_ptr,
    uint32_t  *end_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_acl_clear_t clear_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    clear_info.start_idx = *start_ptr;
    clear_info.end_idx = *end_ptr;
    ret = rtk_acl_rule_del(unit, *phase_ptr, &clear_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_del_entry_phase_start_end */
#endif

#ifdef CMD_ACL_MOVE_ENTRY_PHASE_PHASE_FROM_FROM_TO_TO_ENTRY_NUM_ENTRY_NUM
/*
 * acl move entry phase <UINT:phase> from <UINT:from> to <UINT:to> entry_num <UINT:entry_num>
 */
cparser_result_t
cparser_cmd_acl_move_entry_phase_phase_from_from_to_to_entry_num_entry_num(
    cparser_context_t *context,
    uint32_t  *phase_ptr,
    uint32_t  *from_ptr,
    uint32_t  *to_ptr,
    uint32_t  *entry_num_ptr)
{
    uint32          unit;
    int32           ret;
    rtk_acl_move_t  move_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    move_info.move_from = *from_ptr;
    move_info.move_to = *to_ptr;
    move_info.length = *entry_num_ptr;
    ret = rtk_acl_rule_move(unit, *phase_ptr, &move_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_move_entry_phase_from_to_entry_num */
#endif

#ifdef CMD_ACL_SET_SELECTOR_BLOCK_INDEX_TEMPLATE_INDEX0_TEMPLATE_INDEX
/*
 * acl set selector block <UINT:index> template-index0 <UINT:template_index>
 */
cparser_result_t
cparser_cmd_acl_set_selector_block_index_template_index0_template_index(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *template_index_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtk_acl_templateIdx_t   temp_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_acl_templateSelector_get(unit, *index_ptr, &temp_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    temp_info.template_id[0] = *template_index_ptr;
    ret = rtk_acl_templateSelector_set(unit, *index_ptr, temp_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_selector_block_index_template_idx1_template_idx2 */
#endif

#ifdef CMD_ACL_SET_SELECTOR_BLOCK_INDEX_TEMPLATE_INDEX1_TEMPLATE_INDEX
/*
 * acl set selector block <UINT:index> template-index1 <UINT:template_index>
 */
cparser_result_t
cparser_cmd_acl_set_selector_block_index_template_index1_template_index(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *template_index_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtk_acl_templateIdx_t   temp_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_acl_templateSelector_get(unit, *index_ptr, &temp_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    temp_info.template_id[1] = *template_index_ptr;
    ret = rtk_acl_templateSelector_set(unit, *index_ptr, temp_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_selector_block_index_template_idx1_template_idx2 */
#endif

#ifdef CMD_ACL_SET_SELECTOR_BLOCK_INDEX_TEMPLATE_INDEX2_TEMPLATE_INDEX
/*
 * acl set selector block <UINT:index> template-index2 <UINT:template_index>
 */
cparser_result_t
cparser_cmd_acl_set_selector_block_index_template_index2_template_index(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *template_index_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtk_acl_templateIdx_t   temp_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_acl_templateSelector_get(unit, *index_ptr, &temp_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    temp_info.template_id[2] = *template_index_ptr;
    ret = rtk_acl_templateSelector_set(unit, *index_ptr, temp_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_selector_block_index_template_idx1_template_idx2 */
#endif


#ifdef CMD_ACL_GET_SELECTOR_INDEX
/*
 * acl get selector <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_get_selector_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                  unit;
    int32                   ret;
    uint32                  i, numOftemp;
    rtk_acl_templateIdx_t   temp_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_templateSelector_get(unit, *index_ptr, &temp_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_OM_GET_CHIP_CAPACITY(unit, numOftemp, max_num_of_acl_block_templateSelector);
    diag_util_mprintf("Block %d:\n", *index_ptr);

    for(i = 0; i < numOftemp; i ++)
    {
        diag_util_mprintf("\tTemplate idx %d: %d\n", i, temp_info.template_id[i]);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_selector_block_index */
#endif

#ifdef CMD_ACL_GET_TEMPLATE_PHASE_PHASE_FIELD_LIST
/*
 * acl get template phase <UINT:phase> field-list
 */
cparser_result_t
cparser_cmd_acl_get_template_phase_phase_field_list(
    cparser_context_t *context,
    uint32_t  *phase_ptr)
{
    uint32  unit, i;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("%-25s %s\n", "template field keyword", "desciption");
    diag_util_mprintf("=====================================================\n");

    for (i = 0; i < (sizeof(diag_acl_template_list)/sizeof(diag_acl_template_t)); ++i)
    {
        ret = rtk_acl_templateField_check(unit, *phase_ptr,
                diag_acl_template_list[i].type);
        if (RT_ERR_OK != ret)
            continue;

        diag_util_mprintf("%-25s %s\n", diag_acl_template_list[i].user_info,
                diag_acl_template_list[i].desc);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_template_phase_phase_field_list */
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX_HIT_INDICATION
/*
 * acl get entry phase <UINT:phase> entry <UINT:index> hit-indication
 */
cparser_result_t cparser_cmd_acl_get_entry_phase_phase_entry_index_hit_indication(cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32 unit;
    int32  ret;
    uint32 status;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_ruleHitIndication_get(unit, *phase_ptr, *index_ptr, FALSE, &status);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Hit Status [%d]: %s\n", *index_ptr, (status == TRUE)?"True":"False");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_RESET_ENTRY_PHASE_PHASE_ENTRY_INDEX_HIT_INDICATION
/*
 * acl reset entry phase <UINT:phase> entry <UINT:index> hit-indication
 */
cparser_result_t cparser_cmd_acl_reset_entry_phase_phase_entry_index_hit_indication(cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32          unit;
    int32           ret;
    uint32          status;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_ruleHitIndication_get(unit, *phase_ptr, *index_ptr, TRUE, &status);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_TEMPLATE_TEMPLATE_INDEX_FIELD_INDEX_INDEX_FIELD_TYPE_FIELD_NAME
/*
 * acl set template <UINT:template_index> field_index <UINT:index> field_type <STRING:field_name>
 */
cparser_result_t
cparser_cmd_acl_set_template_template_index_field_index_index_field_type_field_name(
    cparser_context_t *context,
    uint32_t  *template_index_ptr,
    uint32_t  *index_ptr,
    char * *field_name_ptr)
{
    uint32              unit;
    uint32              num_element, i;
    int32               ret;
    rtk_acl_template_t  template_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (*index_ptr >= RTK_MAX_NUM_OF_ACL_TEMPLATE_FIELD)
    {
        diag_util_printf("Invalid field index!\n");
        return CPARSER_NOT_OK;
    }

    num_element = (sizeof(diag_acl_template_list)/sizeof(diag_acl_template_t));

    for (i = 0; i < num_element; ++i)
    {
        if (!strcasecmp(*field_name_ptr, diag_acl_template_list[i].user_info))
            break;
    }

    if (num_element <= i)
    {
        diag_util_printf("Invalid field %s!\n", *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    memset(&template_info, 0, sizeof(rtk_acl_template_t));

    ret = rtk_acl_template_get(unit, *template_index_ptr, &template_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    template_info.field[*index_ptr] = diag_acl_template_list[i].type;

    ret = rtk_acl_template_set(unit, *template_index_ptr, &template_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_template_template_index_field_index_index_field_type_field_name */
#endif

#ifdef CMD_ACL_GET_TEMPLATE_INDEX
/*
 * acl get template <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_get_template_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32              unit;
    uint32              num_element, i, j;
    int32               ret;
    rtk_acl_template_t  template_info;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&template_info, 0, sizeof(rtk_acl_template_t));

    ret = rtk_acl_template_get(unit, *index_ptr, &template_info);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Template %d field information:\n", *index_ptr);

    for (j = 0; j < RTK_MAX_NUM_OF_ACL_TEMPLATE_FIELD; ++j)
    {
        num_element = (sizeof(diag_acl_template_list)/sizeof(diag_acl_template_t));

        diag_util_mprintf("\tfield %d: ", j);

        for (i = 0; i < num_element; ++i)
        {
            if (template_info.field[j] == diag_acl_template_list[i].type)
                break;
        }

        if (num_element > i)
            diag_util_mprintf("%s", diag_acl_template_list[i].user_info);

        diag_util_mprintf("\n");
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_template_index */
#endif

#ifdef CMD_ACL_SET_BLOCK_INDEX_MULTIPLE_RESULT_STATE_DISABLE_ENABLE
/*
 * acl set block <UINT:index> multiple-result state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_block_index_multiple_result_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_blockResultMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(6, 0))
    {
        mode = ACL_BLOCK_RESULT_MULTIPLE;
    }
    else
    {
        mode = ACL_BLOCK_RESULT_SINGLE;
    }

    ret = rtk_acl_blockResultMode_set(unit, *index_ptr, mode);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_block_index_multi_result_state_disable_enable */
#endif

#ifdef CMD_ACL_GET_BLOCK_INDEX_MULTI_RESULT_STATE
/*
 * acl get block <UINT:index> multi-result state
 */
cparser_result_t
cparser_cmd_acl_get_block_index_multi_result_state(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_blockResultMode_t   mode;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_blockResultMode_get(unit, *index_ptr, &mode);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Block %d multi-result state: ", *index_ptr);
    if (ACL_BLOCK_RESULT_SINGLE == mode)
        diag_util_mprintf("Disabled");
    else
        diag_util_mprintf("Enabled");

    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_block_index_multi_result_state */
#endif

/*
*acl set block <UINT:index> group group_1 state ( disable | enable )
*/
#ifdef CMD_ACL_SET_BLOCK_INDEX_GROUP_GROUP_1_STATE_DISABLE_ENABLE
cparser_result_t
cparser_cmd_acl_set_block_index_group_group_1_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_enable_t                enable;
    rtk_acl_blockGroup_t        group_type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(7, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    group_type = ACL_BLOCK_GROUP_1;

    ret = rtk_acl_blockGroupEnable_set(unit, *index_ptr, group_type, enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_block_index_group_group_2_group_4_group_8_all_state_disable_enable */
#endif

#ifdef CMD_ACL_GET_BLOCK_INDEX_GROUP_GROUP_1_STATE
/*
 * acl get block <UINT:index> group group_1 state
 */
cparser_result_t
cparser_cmd_acl_get_block_index_group_group_1_state(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_enable_t                enable;
    rtk_acl_blockGroup_t        group_type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    group_type = ACL_BLOCK_GROUP_1;

    ret = rtk_acl_blockGroupEnable_get(unit, *index_ptr, group_type, &enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Block %d grouping state: ", *index_ptr);
    if (ENABLED == enable)
        diag_util_mprintf("Enabled");
    else
        diag_util_mprintf("Disabled");

    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_block_index_group_group_2_group_4_group_8_all_state */
#endif


#ifdef CMD_ACL_SET_BLOCK_INDEX_GROUP_GROUP_2_GROUP_4_GROUP_8_ALL_STATE_DISABLE_ENABLE
/*
 * acl set block <UINT:index> group ( group_2 | group_4 | group_8 | all ) state ( disable | enable )
 */
cparser_result_t
cparser_cmd_acl_set_block_index_group_group_2_group_4_group_8_all_state_disable_enable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_enable_t                enable;
    rtk_acl_blockGroup_t        group_type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(7, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    if (0 == osal_strcmp(TOKEN_STR(5), "all"))
    {
        group_type = ACL_BLOCK_GROUP_ALL;
    }
    else if('2' == TOKEN_CHAR(5, 6))
    {
        group_type = ACL_BLOCK_GROUP_2;
    }
    else if('4' == TOKEN_CHAR(5, 6))
    {
        group_type = ACL_BLOCK_GROUP_4;
    }
    else
    {
        group_type = ACL_BLOCK_GROUP_8;
    }

    ret = rtk_acl_blockGroupEnable_set(unit, *index_ptr, group_type, enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_block_index_group_group_2_group_4_group_8_all_state_disable_enable */
#endif

#ifdef CMD_ACL_GET_BLOCK_INDEX_GROUP_GROUP_2_GROUP_4_GROUP_8_ALL_STATE
/*
 * acl get block <UINT:index> group ( group_2 | group_4 | group_8 | all ) state
 */
cparser_result_t
cparser_cmd_acl_get_block_index_group_group_2_group_4_group_8_all_state(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_enable_t                enable;
    rtk_acl_blockGroup_t        group_type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (0 == osal_strcmp(TOKEN_STR(5), "all"))
    {
        group_type = ACL_BLOCK_GROUP_ALL;
    }
    else if('2' == TOKEN_CHAR(5, 6))
    {
        group_type = ACL_BLOCK_GROUP_2;
    }
    else if('4' == TOKEN_CHAR(5, 6))
    {
        group_type = ACL_BLOCK_GROUP_4;
    }
    else
    {
        group_type = ACL_BLOCK_GROUP_8;
    }

    ret = rtk_acl_blockGroupEnable_get(unit, *index_ptr, group_type, &enable);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Block %d aggregator %s state: ", *index_ptr, TOKEN_STR(5));
    if (ENABLED == enable)
        diag_util_mprintf("Enabled");
    else
        diag_util_mprintf("Disabled");

    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_block_index_group_group_2_group_4_group_8_all_state */
#endif

#ifdef CMD_ACL_GET_COUNTER_BYTE_PACKET_INDEX
/*
 * acl get counter ( byte | packet ) <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_get_counter_byte_packet_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if('b' == TOKEN_CHAR(3, 0))
    {
        uint64  byte_cnt;

        ret = rtk_acl_statByteCnt_get(unit, *index_ptr, &byte_cnt);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("Byte counter index %d: %llu\n", *index_ptr, byte_cnt);
    }
    else
    {
        uint32  pkt_cnt;

        ret = rtk_acl_statPktCnt_get(unit, *index_ptr, (uint32 *)&pkt_cnt);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("Packet counter index %d: %lu\n", *index_ptr, pkt_cnt);
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_counter_byte_packet_index */
#endif

#ifdef CMD_ACL_CLEAR_COUNTER_BYTE_PACKET_INDEX
/*
 * acl clear counter ( byte | packet ) <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_clear_counter_byte_packet_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('b' == TOKEN_CHAR(3, 0))
    {
        ret = rtk_acl_statByteCnt_clear(unit, *index_ptr);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
    else
    {
        ret = rtk_acl_statPktCnt_clear(unit, *index_ptr);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_clear_counter_byte_packet_index */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_ENTRY_INDEX_LOW_BOUND_VID
/*
 * range-check set vid entry <UINT:index> low-bound <UINT:vid>
 */
cparser_result_t
cparser_cmd_range_check_set_vid_entry_index_low_bound_vid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *vid_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_vid_t        vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&vid_range, 0, sizeof(rtk_acl_rangeCheck_vid_t));

    ret = rtk_acl_rangeCheckVid_get(unit, *index_ptr, &vid_range);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif

    vid_range.vid_lower_bound   = *vid_ptr;
    ret = rtk_acl_rangeCheckVid_set(unit, *index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_vid_inner_outer_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_ENTRY_INDEX_UP_BOUND_VID
/*
 * range-check set vid entry <UINT:index> up-bound <UINT:vid>
 */
cparser_result_t
cparser_cmd_range_check_set_vid_entry_index_up_bound_vid(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *vid_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_vid_t        vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&vid_range, 0, sizeof(rtk_acl_rangeCheck_vid_t));

    ret = rtk_acl_rangeCheckVid_get(unit, *index_ptr, &vid_range);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif

    vid_range.vid_upper_bound   = *vid_ptr;
    ret = rtk_acl_rangeCheckVid_set(unit, *index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_vid_inner_outer_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_ENTRY_INDEX_TYPE_INNER_OUTER
/*
 * range-check set vid entry <UINT:index> type ( inner | outer )
 */
cparser_result_t
cparser_cmd_range_check_set_vid_entry_index_type_inner_outer(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_vid_t        vid_range;
    rtk_acl_rangeCheck_vid_type_t   type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('i' == TOKEN_CHAR(6, 0))
    {
        type = RNGCHK_VID_TYPE_INNER;
    }
    else
    {
        type = RNGCHK_VID_TYPE_OUTER;
    }

    memset(&vid_range, 0, sizeof(rtk_acl_rangeCheck_vid_t));

    ret = rtk_acl_rangeCheckVid_get(unit, *index_ptr, &vid_range);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif

    vid_range.vid_type          = type;
    ret = rtk_acl_rangeCheckVid_set(unit, *index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_vid_inner_outer_lower_upper */
#endif


#ifdef CMD_RANGE_CHECK_GET_ENTRY_INDEX_VID
/*
 * range-check get vid entry <UINT:index>
 */
cparser_result_t
cparser_cmd_range_check_get_vid_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_vid_t    vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&vid_range, 0, sizeof(rtk_acl_rangeCheck_vid_t));

    ret = rtk_acl_rangeCheckVid_get(unit, *index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("VLAN id range check index %d:\n", *index_ptr);

    diag_util_mprintf("\tType: ");
    if (RNGCHK_VID_TYPE_INNER == vid_range.vid_type)
    {
        diag_util_mprintf("Inner\n");
    }
    else
    {
        diag_util_mprintf("Outer\n");
    }

    diag_util_mprintf("\tLower: %d\n", vid_range.vid_lower_bound);
    diag_util_mprintf("\tUpper: %d\n", vid_range.vid_upper_bound);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf("\tReverse: ");
        if (0 == vid_range.reverse)
        {
            diag_util_mprintf("Disabled\n");
        }
        else
        {
            diag_util_mprintf("Enabled\n");
        }
    }
#endif

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_get_entry_index_vid */
#endif

#ifdef CMD_RANGE_CHECK_SET_VID_ENTRY_INDEX_REVERSE_DISABLE_ENABLE
/*
 * range-check set vid entry <UINT:index> reverse ( disable | enable )
 */
cparser_result_t
cparser_cmd_range_check_set_vid_entry_index_reverse_disable_enable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_vid_t    vid_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&vid_range, 0, sizeof(rtk_acl_rangeCheck_vid_t));

    ret = rtk_acl_rangeCheckVid_get(unit, *index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
    {
        vid_range.reverse = 1;
    }
    else
    {
        vid_range.reverse = 0;
    }

    ret = rtk_acl_rangeCheckVid_set(unit, *index_ptr, &vid_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_vid_reverse_state_disable_enable */
#endif

#ifdef CMD_RANGE_CHECK_SET_IP_ENTRY_INDEX_LOW_BOUND_IP
/*
 * range-check set ip entry <UINT:index> low-bound <IPV4ADDR:ip>
 */
cparser_result_t
cparser_cmd_range_check_set_ip_entry_index_low_bound_ip(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *ip_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_ip_t         ip_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&ip_range, 0, sizeof(rtk_acl_rangeCheck_ip_t));

    ret = rtk_acl_rangeCheckIp_get(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    ip_range.ip_lower_bound = *ip_ptr;
    ret = rtk_acl_rangeCheckIp_set(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_ip_ip4_sip_ip4_dip_ip6_sip_ip6_dip_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_IP_ENTRY_INDEX_UP_BOUND_IP
/*
 * range-check set ip entry <UINT:index> up-bound <IPV4ADDR:ip>
 */
cparser_result_t
cparser_cmd_range_check_set_ip_entry_index_up_bound_ip(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *ip_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_ip_t         ip_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&ip_range, 0, sizeof(rtk_acl_rangeCheck_ip_t));

    ret = rtk_acl_rangeCheckIp_get(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    ip_range.ip_upper_bound = *ip_ptr;
    ret = rtk_acl_rangeCheckIp_set(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_ip_ip4_sip_ip4_dip_ip6_sip_ip6_dip_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_IP_ENTRY_INDEX_TYPE_SIP_DIP_SIP6_DIP6
/*
 * range-check set ip entry <UINT:index> type ( sip | dip | sip6 | dip6 )
 */
cparser_result_t
cparser_cmd_range_check_set_ip_entry_index_type_sip_dip_sip6_dip6(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_ip_t         ip_range;
    rtk_acl_rangeCheck_ip_type_t    type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&ip_range, 0, sizeof(rtk_acl_rangeCheck_ip_t));

    ret = rtk_acl_rangeCheckIp_get(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('6' == TOKEN_CHAR(6, 3))
    {
        if('s' == TOKEN_CHAR(6, 0))
            type = RNGCHK_IP_TYPE_IPV6_SRC;
        else
            type = RNGCHK_IP_TYPE_IPV6_DST;
    }
    else if('s' == TOKEN_CHAR(6, 0))
    {
        type = RNGCHK_IP_TYPE_IPV4_SRC;
    }
    else
    {
        type = RNGCHK_IP_TYPE_IPV4_DST;
    }

    ip_range.ip_type = type;
    ret = rtk_acl_rangeCheckIp_set(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_ip_ip4_sip_ip4_dip_ip6_sip_ip6_dip_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_IP_ENTRY_INDEX_TYPE_SIP6_SUFFIX_DIP6_SUFFIX
/*
 * range-check set ip entry <UINT:index> type ( sip6-suffix | dip6-suffix )
 */
cparser_result_t
cparser_cmd_range_check_set_ip_entry_index_type_sip6_suffix_dip6_suffix(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_ip_t         ip_range;
    rtk_acl_rangeCheck_ip_type_t    type;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&ip_range, 0, sizeof(rtk_acl_rangeCheck_ip_t));

    ret = rtk_acl_rangeCheckIp_get(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('s' == TOKEN_CHAR(6, 0))
    {
        type = RNGCHK_IP_TYPE_IPV6_SRC_SUFFIX;
    }
    else
    {
        type = RNGCHK_IP_TYPE_IPV6_DST_SUFFIX;
    }

    ip_range.ip_type = type;
    ret = rtk_acl_rangeCheckIp_set(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_ip_ip4_sip_ip4_dip_ip6_sip_ip6_dip_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_IP_ENTRY_INDEX_REVERSE_DISABLE_ENABLE
/*
 * range-check set ip entry <UINT:index> reverse ( disable | enable )
 */
cparser_result_t
cparser_cmd_range_check_set_ip_entry_index_reverse_disable_enable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtk_acl_rangeCheck_ip_t ip_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&ip_range, 0, sizeof(rtk_acl_rangeCheck_ip_t));

    ret = rtk_acl_rangeCheckIp_get(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
    {
        ip_range.reverse = 1;
    }
    else
    {
        ip_range.reverse = 0;
    }

    ret = rtk_acl_rangeCheckIp_set(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_ip_reverse_state_disable_enable */
#endif

#ifdef CMD_RANGE_CHECK_GET_IP_ENTRY_INDEX
/*
 * range-check get ip entry <UINT:index>
 */
cparser_result_t
cparser_cmd_range_check_get_ip_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                  unit;
    int32                   ret;
    rtk_acl_rangeCheck_ip_t ip_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&ip_range, 0, sizeof(rtk_acl_rangeCheck_ip_t));

    ret = rtk_acl_rangeCheckIp_get(unit, *index_ptr, &ip_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("IP range check index %d:\n", *index_ptr);

    diag_util_mprintf("\tType: ");
    switch (ip_range.ip_type)
    {
        case RNGCHK_IP_TYPE_IPV4_SRC:
            diag_util_mprintf("IPv4 source IP");
            break;
        case RNGCHK_IP_TYPE_IPV4_DST:
            diag_util_mprintf("IPv4 destination IP");
            break;
        case RNGCHK_IP_TYPE_IPV6_SRC:
            diag_util_mprintf("IPv6 source IP");
            break;
        case RNGCHK_IP_TYPE_IPV6_DST:
            diag_util_mprintf("IPv6 destination IP");
            break;
        case RNGCHK_IP_TYPE_IPV6_SRC_SUFFIX:
            diag_util_mprintf("IPv6 suffix source IP");
            break;
        case RNGCHK_IP_TYPE_IPV6_DST_SUFFIX:
            diag_util_mprintf("IPv6 suffix destination IP");
            break;
        default:
            return CPARSER_NOT_OK;
    }

    diag_util_mprintf("\n");

    diag_util_mprintf("\tLower: %x\n", ip_range.ip_lower_bound);
    diag_util_mprintf("\tUpper: %x\n", ip_range.ip_upper_bound);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf("\tReverse: ");
        if (0 == ip_range.reverse)
        {
            diag_util_mprintf("Disabled\n");
        }
        else
        {
            diag_util_mprintf("Enabled\n");
        }
    }
#endif

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_get_entry_index_ip */
#endif

#ifdef CMD_RANGE_CHECK_SET_ENTRY_INDEX_L4PORT_SOURCE_DEST_LOWER_UPPER
/*
 * range-check set <UINT:entry_index> l4port ( source | dest ) <UINT:lower> <UINT:upper>
 */
cparser_result_t
cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper(
    cparser_context_t *context,
    uint32_t  *entry_index_ptr,
    uint32_t  *lower_ptr,
    uint32_t  *upper_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_l4Port_t l4port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l4port_range, 0, sizeof(rtk_acl_rangeCheck_l4Port_t));

    ret = rtk_acl_rangeCheckL4Port_get(unit, *entry_index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('s' == TOKEN_CHAR(4, 0))
    {
        l4port_range.l4port_dir = RNGCHK_L4PORT_DIRECTION_SRC;
    }
    else
    {
        l4port_range.l4port_dir = RNGCHK_L4PORT_DIRECTION_DST;
    }

    l4port_range.upper_bound = *upper_ptr;
    l4port_range.lower_bound = *lower_ptr;
    ret = rtk_acl_rangeCheckL4Port_set(unit, *entry_index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_GET_FIELD_SELECTOR_ENTRY_INDEX
/*
 * range-check get field selector entry <UINT:index>
 */
cparser_result_t
cparser_cmd_range_check_get_field_selector_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
#if defined(CONFIG_SDK_RTL8380)
    uint32                              unit;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        int32                               ret;
        rtk_acl_rangeCheck_fieldSelector_t  field_range;

        memset(&field_range, 0, sizeof(rtk_acl_rangeCheck_fieldSelector_t));

        ret = rtk_acl_rangeCheckFieldSelector_get(unit, *index_ptr, &field_range);
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }

        diag_util_mprintf("Field selector range check index %d:\n", *index_ptr);

        diag_util_mprintf("\tType: ");
        switch (field_range.fieldSelector_type)
        {
            case RNGCHK_FIELDSELECTOR_TYPE_FIELDSELECTOR0:
                diag_util_mprintf("Field selector 0");
                break;
            case RNGCHK_FIELDSELECTOR_TYPE_FIELDSELECTOR1:
                diag_util_mprintf("Field selector 1");
                break;
            default:
                return CPARSER_NOT_OK;
        }

        diag_util_mprintf("\n");

        diag_util_mprintf("\tLower: %x\n", field_range.lower_bound);
        diag_util_mprintf("\tUpper: %x\n", field_range.upper_bound);
        
    }
#endif

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_get_entry_index_ip */
#endif


#ifdef CMD_RANGE_CHECK_SET_FIELD_SELECTOR_ENTRY_INDEX_LOW_BOUND_FIELD_SELECTOR
/*
 * range-check set field-selector entry <UINT:index> low-bound <UINT:field-selector>
 */
cparser_result_t
cparser_cmd_range_check_set_field_selector_entry_index_low_bound_fieldsel(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *field_selector_ptr)
{
    uint32                              unit;
    int32                               ret;
    rtk_acl_rangeCheck_fieldSelector_t  field_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&field_range, 0, sizeof(rtk_acl_rangeCheck_fieldSelector_t));

    ret = rtk_acl_rangeCheckFieldSelector_get(unit, *index_ptr, &field_range);
    if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    field_range.lower_bound = *field_selector_ptr;
    ret = rtk_acl_rangeCheckFieldSelector_set(unit, *index_ptr, &field_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_FIELD_SELECTOR_ENTRY_INDEX_UP_BOUND_FIELD_SELECTOR
/*
 * range-check set field-selector entry <UINT:index> up-bound <UINT:field-selector>
 */
cparser_result_t
cparser_cmd_range_check_set_field_selector_entry_index_up_bound_fieldsel(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *field_selector_ptr)
{
    uint32                              unit;
    int32                               ret;
    rtk_acl_rangeCheck_fieldSelector_t  field_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&field_range, 0, sizeof(rtk_acl_rangeCheck_fieldSelector_t));

    ret = rtk_acl_rangeCheckFieldSelector_get(unit, *index_ptr, &field_range);
    if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    field_range.upper_bound = *field_selector_ptr;
    ret = rtk_acl_rangeCheckFieldSelector_set(unit, *index_ptr, &field_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_FIELD_SELECTOR_ENTRY_INDEX_TYPE_FIELD_SELECTOR_0_FIELD_SELECTOR_1
/*
 *range-check set field-selector entry <UINT:index> type ( field-selector-0 | field-selector-1 )
 */
cparser_result_t
cparser_cmd_range_check_set_field_selector_entry_index_type_field_selector_0_field_selector_1(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                              unit;
    int32                               ret;
    rtk_acl_rangeCheck_fieldSelector_t  field_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&field_range, 0, sizeof(rtk_acl_rangeCheck_fieldSelector_t));

    ret = rtk_acl_rangeCheckFieldSelector_get(unit, *index_ptr, &field_range);
    if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('0' == TOKEN_CHAR(6, 15))
    {
        field_range.fieldSelector_type = RNGCHK_FIELDSELECTOR_TYPE_FIELDSELECTOR0;
    }
    else
    {
        field_range.fieldSelector_type = RNGCHK_FIELDSELECTOR_TYPE_FIELDSELECTOR1;
    }

    ret = rtk_acl_rangeCheckFieldSelector_set(unit, *index_ptr, &field_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif


#ifdef CMD_RANGE_CHECK_SET_L4PORT_ENTRY_INDEX_LOW_BOUND_L4PORT
/*
 * range-check set l4port entry <UINT:index> low-bound <UINT:l4port>
 */
cparser_result_t
cparser_cmd_range_check_set_l4port_entry_index_low_bound_l4port(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *l4port_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_l4Port_t l4port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l4port_range, 0, sizeof(rtk_acl_rangeCheck_l4Port_t));

    ret = rtk_acl_rangeCheckL4Port_get(unit, *index_ptr, &l4port_range);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif

    l4port_range.lower_bound = *l4port_ptr;
    ret = rtk_acl_rangeCheckL4Port_set(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_L4PORT_ENTRY_INDEX_UP_BOUND_L4PORT
/*
 * range-check set l4port entry <UINT:index> up-bound <UINT:l4port>
 */
cparser_result_t
cparser_cmd_range_check_set_l4port_entry_index_up_bound_l4port(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *l4port_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_l4Port_t l4port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l4port_range, 0, sizeof(rtk_acl_rangeCheck_l4Port_t));

    ret = rtk_acl_rangeCheckL4Port_get(unit, *index_ptr, &l4port_range);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif

    l4port_range.upper_bound = *l4port_ptr;
    ret = rtk_acl_rangeCheckL4Port_set(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_L4PORT_ENTRY_INDEX_TYPE_SRC_PORT_DST_PORT
/*
 * range-check set l4port entry <UINT:index> type ( src-port | dst-port )
 */
cparser_result_t
cparser_cmd_range_check_set_l4port_entry_index_type_src_port_dst_port(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_l4Port_t l4port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l4port_range, 0, sizeof(rtk_acl_rangeCheck_l4Port_t));

    ret = rtk_acl_rangeCheckL4Port_get(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('s' == TOKEN_CHAR(6, 0))
    {
        l4port_range.l4port_dir = RNGCHK_L4PORT_DIRECTION_SRC;
    }
    else
    {
        l4port_range.l4port_dir = RNGCHK_L4PORT_DIRECTION_DST;
    }

    ret = rtk_acl_rangeCheckL4Port_set(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_L4PORT_ENTRY_INDEX_TYPE_SRC_PORT_DST_PORT_SRC_OR_DST_PORT
/*
 * range-check set l4port entry <UINT:index> type ( src-port | dst-port |src-or-dst-port  )
 */
cparser_result_t
cparser_cmd_range_check_set_l4port_entry_index_type_src_port_dst_port_src_or_dst_port(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_l4Port_t l4port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l4port_range, 0, sizeof(rtk_acl_rangeCheck_l4Port_t));

    ret = rtk_acl_rangeCheckL4Port_get(unit, *index_ptr, &l4port_range);
    if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if(('s' == TOKEN_CHAR(6, 0)) && ('o' == TOKEN_CHAR(6, 4)))
    {
        l4port_range.l4port_dir = RNGCHK_L4PORT_DIRECTION_SRC_DST;
    }
    else if('s' == TOKEN_CHAR(6, 0))
    {
        l4port_range.l4port_dir = RNGCHK_L4PORT_DIRECTION_SRC;
    }
    else
    {
        l4port_range.l4port_dir = RNGCHK_L4PORT_DIRECTION_DST;
    }

    ret = rtk_acl_rangeCheckL4Port_set(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_source_dest_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_L4PORT_ENTRY_INDEX_REVERSE_DISABLE_ENABLE
/*
 * range-check set l4port entry <UINT:index> reverse ( disable | enable )
 */
cparser_result_t
cparser_cmd_range_check_set_l4port_entry_index_reverse_disable_enable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_l4Port_t l4port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&l4port_range, 0, sizeof(rtk_acl_rangeCheck_l4Port_t));

    ret = rtk_acl_rangeCheckL4Port_get(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
    {
        l4port_range.reverse = 1;
    }
    else
    {
        l4port_range.reverse = 0;
    }

    ret = rtk_acl_rangeCheckL4Port_set(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_l4port_reverse_state_disable_enable */
#endif

#ifdef CMD_RANGE_CHECK_GET_L4PORT_ENTRY_INDEX
/*
 * range-check get l4port entry <UINT:index>
 */
cparser_result_t
cparser_cmd_range_check_get_l4port_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                      unit;
    int32                       ret;
    rtk_acl_rangeCheck_l4Port_t l4port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();
		
    memset(&l4port_range, 0, sizeof(rtk_acl_rangeCheck_l4Port_t));

    ret = rtk_acl_rangeCheckL4Port_get(unit, *index_ptr, &l4port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("L4 port range check index %d:\n", *index_ptr);

    diag_util_mprintf("\tType: ");
    if (RNGCHK_L4PORT_DIRECTION_SRC == l4port_range.l4port_dir)
    {
        diag_util_mprintf("source direction");
    }
    else if (RNGCHK_L4PORT_DIRECTION_DST == l4port_range.l4port_dir)
    {
        diag_util_mprintf("destination direction");
    }
	
#if defined(CONFIG_SDK_RTL8380)
    if (DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
	 if (RNGCHK_L4PORT_DIRECTION_SRC_DST == l4port_range.l4port_dir)
	 {
	    diag_util_mprintf("source or destination direction");
	 }
	
    }
#endif
	
    diag_util_mprintf("\n");

    diag_util_mprintf("\tLower: %d\n", l4port_range.lower_bound);
    diag_util_mprintf("\tUpper: %d\n", l4port_range.upper_bound);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf("\tReverse: ");
        if (0 == l4port_range.reverse)
        {
            diag_util_mprintf("Disabled\n");
        }
        else
        {
            diag_util_mprintf("Enabled\n");
        }
    }
#endif

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_get_entry_index_l4port */
#endif

#ifdef CMD_RANGE_CHECK_SET_LENGTH_ENTRY_INDEX_LOW_BOUND_LENGTH
/*
 * range-check set length entry <UINT:index> low-bound <UINT:length>
 */
cparser_result_t
cparser_cmd_range_check_set_length_entry_index_low_bound_length(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *length_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_packetLen_t  pktlen_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&pktlen_range, 0, sizeof(rtk_acl_rangeCheck_packetLen_t));

    ret = rtk_acl_rangeCheckPacketLen_get(unit, *index_ptr, &pktlen_range);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif

    pktlen_range.lower_bound = *length_ptr;
    ret = rtk_acl_rangeCheckPacketLen_set(unit, *index_ptr, &pktlen_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_pkt_len_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_LENGTH_ENTRY_INDEX_UP_BOUND_LENGTH
/*
 * range-check set length entry <UINT:index> up-bound <UINT:length>
 */
cparser_result_t
cparser_cmd_range_check_set_length_entry_index_up_bound_length(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *length_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_packetLen_t  pktlen_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&pktlen_range, 0, sizeof(rtk_acl_rangeCheck_packetLen_t));

    ret = rtk_acl_rangeCheckPacketLen_get(unit, *index_ptr, &pktlen_range);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        if (RT_ERR_OK != ret)
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif
#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        if ((RT_ERR_OK != ret) && (RT_ERR_RANGE_CHECK_TYPE != ret))
        {
            DIAG_ERR_PRINT(ret);
            return CPARSER_NOT_OK;
        }
    }
#endif

    pktlen_range.upper_bound = *length_ptr;
    ret = rtk_acl_rangeCheckPacketLen_set(unit, *index_ptr, &pktlen_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_pkt_len_lower_upper */
#endif

#ifdef CMD_RANGE_CHECK_SET_LENGTH_ENTRY_INDEX_REVERSE_DISABLE_ENABLE
/*
 * range-check set length entry <UINT:index> reverse ( disable | enable )
 */
cparser_result_t
cparser_cmd_range_check_set_length_entry_index_reverse_disable_enable(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_packetLen_t  pktlen_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    memset(&pktlen_range, 0, sizeof(rtk_acl_rangeCheck_packetLen_t));

    ret = rtk_acl_rangeCheckPacketLen_get(unit, *index_ptr, &pktlen_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('e' == TOKEN_CHAR(6, 0))
    {
        pktlen_range.reverse = 1;
    }
    else
    {
        pktlen_range.reverse = 0;
    }

    ret = rtk_acl_rangeCheckPacketLen_set(unit, *index_ptr, &pktlen_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_pkt_len_reverse_state_disable_enable */
#endif

#ifdef CMD_RANGE_CHECK_GET_LENGTH_ENTRY_INDEX
/*
 * range-check get length entry <UINT:index>
 */
cparser_result_t
cparser_cmd_range_check_get_length_entry_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_packetLen_t  pktlen_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&pktlen_range, 0, sizeof(rtk_acl_rangeCheck_packetLen_t));

    ret = rtk_acl_rangeCheckPacketLen_get(unit, *index_ptr, &pktlen_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Packet len range check index %d:\n", *index_ptr);

    diag_util_mprintf("\tLower: %d\n", pktlen_range.lower_bound);
    diag_util_mprintf("\tUpper: %d\n", pktlen_range.upper_bound);

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf("\tReverse: ");
        if (0 == pktlen_range.reverse)
        {
            diag_util_mprintf("Disabled\n");
        }
        else
        {
            diag_util_mprintf("Enabled\n");
        }
    }
#endif

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_get_entry_index_pkt_len */
#endif

#ifdef CMD_FIELD_SELECTOR_SET_INDEX_INDEX_FORMAT_RAW_L2_L3_L4_OFFSET_OFFSET
/*
 * field-selector set index <UINT:index> format ( raw | l2 | l3  | l4 ) offset <UINT:offset>
 */
cparser_result_t
cparser_cmd_field_selector_set_index_index_format_raw_l2_l3_l4_offset_offset(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *offset_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_fieldSelector_data_t    fs;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (0 == osal_strcmp(TOKEN_STR(5), "raw"))
    {
        fs.start = FS_START_POS_RAW;
    }
    else if (0 == osal_strcmp(TOKEN_STR(5), "l2"))
    {
        fs.start = FS_START_POS_L2;
    }
    else if (0 == osal_strcmp(TOKEN_STR(5), "l3"))
    {
        fs.start = FS_START_POS_L3;
    }
    else
    {
        fs.start = FS_START_POS_L4;
    }

    fs.offset = *offset_ptr;
    ret = rtk_acl_fieldSelector_set(unit, *index_ptr, &fs);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_field_selector_set_index_content_raw_llc_l3_arp_ipv4_ipv6_ip_l4_offset */
#endif


#ifdef CMD_FIELD_SELECTOR_SET_INDEX_INDEX_FORMAT_RAW_LLC_L3_ARP_IPV4_HEADER_IPV6_HEADER_IP_PAYLOAD_L4_PAYLOAD_OFFSET_OFFSET
/*
 * field-selector set index <UINT:index> format ( raw | llc | l3 | arp | ipv4-header | ipv6-header | ip-payload | l4-payload ) offset <UINT:offset>
 */
cparser_result_t
cparser_cmd_field_selector_set_index_index_format_raw_llc_l3_arp_ipv4_header_ipv6_header_ip_payload_l4_payload_offset_offset(
    cparser_context_t *context,
    uint32_t  *index_ptr,
    uint32_t  *offset_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_fieldSelector_data_t    fs;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (0 == osal_strcmp(TOKEN_STR(5), "raw"))
    {
        fs.start = FS_START_POS_RAW;
    }
    else if (0 == osal_strcmp(TOKEN_STR(5), "llc"))
    {
        fs.start = FS_START_POS_LLC;
    }
    else if (0 == osal_strcmp(TOKEN_STR(5), "l3"))
    {
        fs.start = FS_START_POS_L3;
    }
    else if (0 == osal_strcmp(TOKEN_STR(5), "arp"))
    {
        fs.start = FS_START_POS_ARP;
    }
    else if (0 == strncmp(TOKEN_STR(5), "ipv4", 4))
    {
        fs.start = FS_START_POS_IPV4;
    }
    else if (0 == strncmp(TOKEN_STR(5), "ipv6", 4))
    {
        fs.start = FS_START_POS_IPV6;
    }
    else if (0 == strncmp(TOKEN_STR(5), "ip", 2))
    {
        fs.start = FS_START_POS_IP;
    }
    else
    {
        fs.start = FS_START_POS_L4;
    }

    fs.offset = *offset_ptr;
    ret = rtk_acl_fieldSelector_set(unit, *index_ptr, &fs);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_field_selector_set_index_content_raw_llc_l3_arp_ipv4_ipv6_ip_l4_offset */
#endif

#ifdef CMD_FIELD_SELECTOR_GET_INDEX_INDEX
/*
 * field-selector get index <UINT:index>
 */
cparser_result_t
cparser_cmd_field_selector_get_index_index(
    cparser_context_t *context,
    uint32_t  *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_fieldSelector_data_t    fs;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    ret = rtk_acl_fieldSelector_get(unit, *index_ptr, &fs);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Field selector %d:\n", *index_ptr);

    diag_util_mprintf("\tcontent from: ");
    switch (fs.start)
    {
        case FS_START_POS_RAW:
            diag_util_mprintf("Raw");
            break;
#if defined(CONFIG_SDK_RTL8390)
        case FS_START_POS_LLC:
            diag_util_mprintf("LLC");
            break;
#endif
#if defined(CONFIG_SDK_RTL8380)
        case FS_START_POS_L2:
            diag_util_mprintf("L2");
            break;
#endif
        case FS_START_POS_L3:
            diag_util_mprintf("L3");
            break;
#if defined(CONFIG_SDK_RTL8390)
        case FS_START_POS_ARP:
            diag_util_mprintf("ARP");
            break;
        case FS_START_POS_IPV4:
            diag_util_mprintf("IPv4");
            break;
        case FS_START_POS_IPV6:
            diag_util_mprintf("IPv6");
            break;
        case FS_START_POS_IP:
            diag_util_mprintf("IP");
            break;
#endif
        case FS_START_POS_L4:
            diag_util_mprintf("L4");
            break;
        default:
            return CPARSER_NOT_OK;
    }
    diag_util_mprintf("\n");

    diag_util_mprintf("\toffset: %d\n", fs.offset);

    return CPARSER_OK;
}   /* end of cparser_cmd_field_selector_get_index */
#endif

#ifdef CMD_ACL_SET_METER_IFG_EXCLUDE_INCLUDE
/*
 * acl set meter ifg ( exclude | include )
 */
cparser_result_t cparser_cmd_acl_set_meter_ifg_exclude_include(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    includeIfg = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('e' == TOKEN_CHAR(4,0))
    {
        includeIfg = DISABLED;
    }
    else if ('i' == TOKEN_CHAR(4,0))
    {
        includeIfg = ENABLED;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_meterIncludeIfg_set(unit, includeIfg), ret);

    return CPARSER_OK;
} /* end of cparser_cmd_acl_set_meter_ifg_exclude_include */
#endif

#ifdef CMD_ACL_GET_METER_IFG
/*
 * acl get meter ifg
 */
cparser_result_t cparser_cmd_acl_get_meter_ifg(cparser_context_t *context)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_enable_t    includeIfg = DISABLED;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_acl_meterIncludeIfg_get(unit, &includeIfg), ret);

    if (ENABLED == includeIfg)
    {
        diag_util_mprintf("Meter Include IFG\n");
    }
    else
    {
        diag_util_mprintf("Meter Exclude IFG\n");
    }

    return CPARSER_OK;
} /* end of cparser_cmd_acl_get_meter_ifg */
#endif

#ifdef CMD_ACL_SET_METER_MODE_BLOCK_INDEX_BYTE_PACKET
/*
 * acl set meter mode block <UINT:index> ( byte | packet )
 */
cparser_result_t cparser_cmd_acl_set_meter_mode_block_index_byte_packet(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          blockIdx;
    rtk_acl_meterMode_t    mode = METER_MODE_BYTE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    blockIdx = *index_ptr;

    if ('b' == TOKEN_CHAR(6,0))
    {
        mode = METER_MODE_BYTE;
    }
    else if ('p' == TOKEN_CHAR(6,0))
    {
        mode = METER_MODE_PACKET;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_meterMode_set(unit, blockIdx, mode), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_METER_BURST_SIZE_BYTE_PACKET_SLB_LB0BS_SLB_LB1BS_LB0_BURST_SIZE_LB0_BURST_SIZE_LB1BS_BURST_SIZE_LB1BS_BURST_SIZE
/*
 * acl set meter burst-size ( byte | packet ) ( slb_lb0bs | slb_lb1bs ) lb0-burst-size <UINT:lb0_burst_size> lb1-burst-size <UINT:lb1_burst_size>
 */
cparser_result_t cparser_cmd_acl_set_meter_burst_size_byte_packet_slb_lb0bs_slb_lb1bs_lb0_burst_size_lb0_burst_size_lb1_burst_size_lb1_burst_size(
    cparser_context_t *context,
    uint32_t *lb0_burst_size_ptr,
    uint32_t *lb1_burst_size_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_acl_meterMode_t    mode = METER_MODE_BYTE;
    rtk_acl_meterBurstSize_t  burstSize;
    uint32          lb0bs, lb1bs;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(4,0))
    {
        mode = METER_MODE_BYTE;
    }
    else if ('p' == TOKEN_CHAR(4,0))
    {
        mode = METER_MODE_PACKET;
    }

    lb0bs = *lb0_burst_size_ptr;
    lb1bs = *lb1_burst_size_ptr;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterBurstSize_get(unit, mode, &burstSize), ret);

    burstSize.slb_lb0bs = lb0bs;
    burstSize.slb_lb1bs = lb1bs;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterBurstSize_set(unit, mode, &burstSize), ret);

    return CPARSER_OK;
}
#endif


#ifdef CMD_ACL_SET_METER_BURST_SIZE_BYTE_PACKET_DLB_SRTCM_TRTCM_LB0_BURST_SIZE_LB0_BURST_SIZE_LB1BS_BURST_SIZE_LB1BS_BURST_SIZE
/*
 * acl set meter burst-size ( byte | packet ) ( dlb | srtcm | trtcm ) lb0-burst-size <UINT:lb0_burst_size> lb1-burst-size <UINT:lb1_burst_size>
 */
cparser_result_t cparser_cmd_acl_set_meter_burst_size_byte_packet_dlb_srtcm_trtcm_lb0_burst_size_lb0_burst_size_lb1_burst_size_lb1_burst_size(
    cparser_context_t *context,
    uint32_t *lb0_burst_size_ptr,
    uint32_t *lb1_burst_size_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    rtk_acl_meterMode_t    mode = METER_MODE_BYTE;
    rtk_acl_meterType_t    type = METER_TYPE_DLB;
    rtk_acl_meterBurstSize_t  burstSize;
    uint32          lb0bs, lb1bs;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('b' == TOKEN_CHAR(4,0))
    {
        mode = METER_MODE_BYTE;
    }
    else if ('p' == TOKEN_CHAR(4,0))
    {
        mode = METER_MODE_PACKET;
    }

    if ('d' == TOKEN_CHAR(5,0))
    {
        type = METER_TYPE_DLB;
    }
    else if ('s' == TOKEN_CHAR(5,0))
    {
        type = METER_TYPE_SRTCM;
    }
    else if ('t' == TOKEN_CHAR(5,0))
    {
        type = METER_TYPE_TRTCM;
    }

    lb0bs = *lb0_burst_size_ptr;
    lb1bs = *lb1_burst_size_ptr;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterBurstSize_get(unit, mode, &burstSize), ret);
    switch(type)
    {
        case METER_TYPE_DLB:
            burstSize.dlb_lb0bs = lb0bs;
            burstSize.dlb_lb1bs = lb1bs;
            break;
        case METER_TYPE_SRTCM:
            burstSize.srtcm_cbs = lb0bs;
            burstSize.srtcm_ebs = lb1bs;
            break;
        case METER_TYPE_TRTCM:
            burstSize.trtcm_cbs = lb0bs;
            burstSize.trtcm_pbs = lb1bs;
            break;
        default:
            break;
    }
    DIAG_UTIL_ERR_CHK(rtk_acl_meterBurstSize_set(unit, mode, &burstSize), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_METER_ENTRY_INDEX_DLB_LB0_RATE_LB0_RATE_LB1_RATE_LB1_RATE
/*
 * acl set meter entry <UINT:index> dlb lb0-rate <UINT:lb0_rate> lb1-rate <UINT:lb1_rate>
 */
cparser_result_t cparser_cmd_acl_set_meter_entry_index_dlb_lb0_rate_lb0_rate_lb1_rate_lb1_rate(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *lb0_rate_ptr,
    uint32_t *lb1_rate_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          meterId;
    rtk_acl_meterEntry_t   meterEntry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    meterId = *index_ptr;

    osal_memset(&meterEntry, 0, sizeof(meterEntry));

    meterEntry.type = METER_TYPE_DLB;
    meterEntry.color_aware = FALSE;
    meterEntry.lb0_rate = *lb0_rate_ptr;
    meterEntry.lb1_rate = *lb1_rate_ptr;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterEntry_set(unit, meterId, &meterEntry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_METER_ENTRY_INDEX_SRTCM_COLOR_AWARE_COLOR_UNAWARE_CIR_CIR
/*
 * acl set meter entry <UINT:index> srtcm ( color-aware | color-unaware ) cir <UINT:cir>
 */
cparser_result_t cparser_cmd_acl_set_meter_entry_index_srtcm_color_aware_color_unaware_cir_cir(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *cir_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          meterId;
    rtk_acl_meterEntry_t   meterEntry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    meterId = *index_ptr;

    osal_memset(&meterEntry, 0, sizeof(meterEntry));

    if ('a' == TOKEN_CHAR(6,6))
    {
        meterEntry.color_aware = TRUE;
    }
    else if ('u' == TOKEN_CHAR(6,6))
    {
        meterEntry.color_aware = FALSE;
    }

    meterEntry.type = METER_TYPE_SRTCM;
    meterEntry.lb0_rate = *cir_ptr;
    meterEntry.lb1_rate = 0;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterEntry_set(unit, meterId, &meterEntry), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_METER_ENTRY_INDEX_TRTCM_COLOR_AWARE_COLOR_UNAWARE_CIR_CIR_PIR_PIR
/*
 * acl set meter entry <UINT:index> trtcm ( color-aware | color-unaware ) cir <UINT:cir> pir <UINT:pir>
 */
cparser_result_t cparser_cmd_acl_set_meter_entry_index_trtcm_color_aware_color_unaware_cir_cir_pir_pir(cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *cir_ptr,
    uint32_t *pir_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          meterId;
    rtk_acl_meterEntry_t   meterEntry;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    meterId = *index_ptr;

    osal_memset(&meterEntry, 0, sizeof(meterEntry));

    if ('a' == TOKEN_CHAR(6,6))
    {
        meterEntry.color_aware = TRUE;
    }
    else if ('u' == TOKEN_CHAR(6,6))
    {
        meterEntry.color_aware = FALSE;
    }

    meterEntry.type = METER_TYPE_TRTCM;
    meterEntry.lb0_rate = *cir_ptr;
    meterEntry.lb1_rate = *pir_ptr;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterEntry_set(unit, meterId, &meterEntry), ret);
    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_SET_METER_ENTRY_INDEX_SLB_STATE_DISABLE_ENABLE_RATE_RATE_GROUP_GROUP
/*
 * acl set meter entry <UINT:index> slb state ( disable | enable ) rate <UINT:rate> group <UINT:group>
 */
cparser_result_t cparser_cmd_acl_set_meter_entry_index_slb_state_disable_enable_rate_rate_group_group(
    cparser_context_t *context,
    uint32_t *index_ptr,
    uint32_t *rate_ptr,
    uint32_t *threshold_group_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          meterId;
    rtk_acl_meterEntry_t    meterEntry;
    rtk_enable_t            enable;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if('e' == TOKEN_CHAR(7, 0))
    {
        enable = ENABLED;
    }
    else
    {
        enable = DISABLED;
    }

    meterId = *index_ptr;

    osal_memset(&meterEntry, 0, sizeof(meterEntry));

    meterEntry.enable = enable;
    meterEntry.rate = *rate_ptr;
    meterEntry.thr_grp = *threshold_group_ptr;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterEntry_set(unit, meterId, &meterEntry), ret);

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_METER_MODE_INDEX
/*
 * acl get meter mode <UINT:index>
 */
cparser_result_t cparser_cmd_acl_get_meter_mode_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32          unit = 0;
    int32           ret = RT_ERR_FAILED;
    uint32          blockIdx;
    rtk_acl_meterMode_t    mode = METER_MODE_BYTE;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    blockIdx = *index_ptr;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterMode_get(unit, blockIdx, &mode), ret);

    diag_util_printf("Block %d meter mode: ", blockIdx);

    if(METER_MODE_BYTE == mode)
        diag_util_mprintf("Byte Based\n");
    else
        diag_util_mprintf("Packet Based\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_METER_BURST_SIZE
/*
 * acl get meter burst-size
 */
cparser_result_t cparser_cmd_acl_get_meter_burst_size(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    rtk_acl_meterBurstSize_t  burstSize;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_acl_meterBurstSize_get(unit, METER_MODE_BYTE, &burstSize), ret);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf("Meter Byte Based Mode\n");
        diag_util_mprintf(" Dual Leaky Bucket\n");
        diag_util_mprintf("     Leaky Bucket 0 Burst Size:%d\n", burstSize.dlb_lb0bs);
        diag_util_mprintf("     Leaky Bucket 1 Burst Size:%d\n", burstSize.dlb_lb1bs);
        diag_util_mprintf(" Single Rate Three Color Marker\n");
        diag_util_mprintf("     Committed Burst Size:%d\n", burstSize.srtcm_cbs);
        diag_util_mprintf("     Excess Burst Size:%d\n", burstSize.srtcm_ebs);
        diag_util_mprintf(" Two Rate Three Color Marker\n");
        diag_util_mprintf("     Committed Burst Size:%d\n", burstSize.trtcm_cbs);
        diag_util_mprintf("     Peak Burst Size:%d\n", burstSize.trtcm_pbs);
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Meter Byte Based Mode\n");
        diag_util_mprintf(" Single Leaky Bucket\n");
        diag_util_mprintf("     Leaky Bucket 0 Burst Size:%d\n", burstSize.slb_lb0bs);
        diag_util_mprintf("     Leaky Bucket 1 Burst Size:%d\n", burstSize.slb_lb1bs);
    }
#endif

    DIAG_UTIL_ERR_CHK(rtk_acl_meterBurstSize_get(unit, METER_MODE_PACKET, &burstSize), ret);
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        diag_util_mprintf("Meter Packet Based Mode\n");
        diag_util_mprintf(" Dual Leaky Bucket\n");
        diag_util_mprintf("     Leaky Bucket 0 Burst Size:%d\n", burstSize.dlb_lb0bs);
        diag_util_mprintf("     Leaky Bucket 1 Burst Size:%d\n", burstSize.dlb_lb1bs);
        diag_util_mprintf(" Single Rate Three Color Marker\n");
        diag_util_mprintf("     Committed Burst Size:%d\n", burstSize.srtcm_cbs);
        diag_util_mprintf("     Excess Burst Size:%d\n", burstSize.srtcm_ebs);
        diag_util_mprintf(" Two Rate Three Color Marker\n");
        diag_util_mprintf("     Committed Burst Size:%d\n", burstSize.trtcm_cbs);
        diag_util_mprintf("     Peak Burst Size:%d\n", burstSize.trtcm_pbs);
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Meter Packet Based Mode\n");
        diag_util_mprintf(" Single Leaky Bucket\n");
        diag_util_mprintf("     Leaky Bucket 0 Burst Size:%d\n", burstSize.slb_lb0bs);
        diag_util_mprintf("     Leaky Bucket 1 Burst Size:%d\n", burstSize.slb_lb1bs);
    }
#endif

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_METER_ENTRY_INDEX
/*
 * acl get meter entry <UINT:index>
 */
cparser_result_t cparser_cmd_acl_get_meter_entry_index(cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  meterId;
    rtk_acl_meterEntry_t    meterEntry;


    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    meterId = *index_ptr;

    DIAG_UTIL_ERR_CHK(rtk_acl_meterEntry_get(unit, meterId, &meterEntry), ret);

    diag_util_mprintf("Meter Entry %d\n", meterId);

    diag_util_printf("Meter Type: ");

#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        switch(meterEntry.type)
        {
            case METER_TYPE_DLB:
                diag_util_mprintf("Double Leaky Bucket\n");
                diag_util_mprintf("Leaky Bucket 0 Rate: %d\n", meterEntry.lb0_rate);
                diag_util_mprintf("Leaky Bucket 1 Rate: %d\n", meterEntry.lb1_rate);
                break;
            case METER_TYPE_SRTCM:
                diag_util_mprintf("Single Rate Three Color Marker\n");
                diag_util_printf("Color Aware: ");
                if(TRUE == meterEntry.color_aware)
                    diag_util_mprintf("Yes\n");
                else
                    diag_util_mprintf("No\n");

                diag_util_mprintf("Committed Information Rate: %d\n", meterEntry.lb0_rate);
                break;
            case METER_TYPE_TRTCM:
                diag_util_mprintf("Two Rate Three Color Marker\n");
                diag_util_printf("Color Aware: ");
                if(TRUE == meterEntry.color_aware)
                    diag_util_mprintf("Yes\n");
                else
                    diag_util_mprintf("No\n");

                diag_util_mprintf("Committed Information Rate: %d\n", meterEntry.lb0_rate);
                diag_util_mprintf("Peak Information Rate: %d\n", meterEntry.lb1_rate);
                break;
            default:
                diag_util_mprintf("Invalid Entry\n");
                break;
        }
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        diag_util_mprintf("Single Leaky Bucket\n");
        diag_util_mprintf("Enable: %d\n", meterEntry.enable);
        diag_util_mprintf("Leaky Bucket Rate: %d\n", meterEntry.rate);
        diag_util_mprintf("Threshold group: %d\n", meterEntry.thr_grp);
    }
#endif

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_METER_EXCEED_FLAG
/*
 * acl get meter exceed-flag
 */
cparser_result_t cparser_cmd_acl_get_meter_exceed_flag(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  meterId;
    uint32  isExceed;
    uint32  numOfmeter;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_OM_GET_CHIP_CAPACITY(unit, numOfmeter, max_num_of_metering);

    diag_util_mprintf("Exceed Meter Entry:");
    for(meterId = 0; meterId < numOfmeter; meterId ++)
    {
        DIAG_UTIL_ERR_CHK(rtk_acl_meterExceed_get(unit, meterId, &isExceed), ret);
        if(TRUE == isExceed)
        {
            diag_util_mprintf("%d\n", meterId);
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_METER_EXCEED_FLAG_AGGREGATION
/*
 * acl get meter exceed-flag aggregation
 */
cparser_result_t cparser_cmd_acl_get_meter_exceed_flag_aggregation(cparser_context_t *context)
{
    uint32  unit = 0;
    int32   ret = RT_ERR_FAILED;
    uint32  exceedIdx;
    uint32  exceedMask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    DIAG_UTIL_ERR_CHK(rtk_acl_meterExceedAggregation_get(unit, &exceedMask), ret);
    diag_util_mprintf("Exceed Meter Entry Index Range (one or more meter entries in the range exceed):");

    for(exceedIdx = 0; exceedIdx < 32; exceedIdx ++)
    {
        if(exceedMask & (1 << exceedIdx))
        {
            diag_util_mprintf("%d - %d\n", exceedIdx * 16, (exceedIdx + 1) * 16);
        }
    }

    diag_util_mprintf("\n");

    return CPARSER_OK;
}
#endif

#ifdef CMD_ACL_GET_ENTRY_PHASE_PHASE_ENTRY_INDEX_ACTION
static void
_diag_acl_igrAction_show(uint32 unit, rtk_acl_igrAction_t *action)
{
#if defined(CONFIG_SDK_RTL8390)
    if (DIAG_OM_GET_FAMILYID(RTL8390_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8350_FAMILY_ID))
    {
        /* forward action */
        diag_util_mprintf("\tForward action state: ");
        if (ENABLED == action->fwd_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\taction: ");
            switch (action->fwd_data.fwd_type)
            {
                case ACL_ACTION_FWD_PERMIT:
                    diag_util_mprintf("permit\n");
                    break;
                case ACL_ACTION_FWD_DROP:
                    diag_util_mprintf("drop\n");
                    break;
                case ACL_ACTION_FWD_COPY_TO_PORTID:
                    diag_util_mprintf("copy uni port:%d\n", action->fwd_data.fwd_info);
                    break;
                case ACL_ACTION_FWD_COPY_TO_PORTMASK:
                    diag_util_mprintf("copy multi index:%d\n", action->fwd_data.fwd_info);
                    break;
                case ACL_ACTION_FWD_REDIRECT_TO_PORTID:
                    diag_util_mprintf("redirect uni port:%d\n", action->fwd_data.fwd_info);
                    break;
                case ACL_ACTION_FWD_REDIRECT_TO_PORTMASK:
                    diag_util_mprintf("redirect multi index:%d\n", action->fwd_data.fwd_info);
                    break;
                case ACL_ACTION_FWD_UNICAST_ROUTING:
                    diag_util_mprintf("route unicast index:%d\n", action->fwd_data.fwd_info);
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->fwd_data.fwd_type) */
        }   /* if (ENABLED == action->fwd_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* Counter action */
        diag_util_mprintf("\tStatistic action state: ");
        if (ENABLED == action->stat_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\taction: ");
            switch (action->stat_data.stat_type)
            {
                case STAT_TYPE_PACKET_BASED_32BIT:
                    diag_util_mprintf("packet32 index:%d\n", action->stat_data.stat_idx);
                    break;
                case STAT_TYPE_BYTE_BASED_64BIT:
                    diag_util_mprintf("byte64 index:%d\n", action->stat_data.stat_idx);
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->stat_data.stat_type) */
        }   /* if (ENABLED == action->stat_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* Mirror action */
        diag_util_mprintf("\tMirror action state: ");
        if (ENABLED == action->mirror_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\tmirror index:%d\n", action->mirror_data.mirror_set_idx);
        }   /* end of if (ENABLED == action->mirror_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* Meter action */
        diag_util_mprintf("\tMeter action state: ");
        if (ENABLED == action->meter_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tmeter index:%d\n", action->meter_data.meter_idx);
        }   /* end of if (ENABLED == action->meter_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* inner vlan translate action */
        diag_util_mprintf("\tInner Vlan translate action state: ");
        if (ENABLED == action->inner_vlan_assign_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\tinner vlan-xlate action: ");
            switch (action->inner_vlan_data.vid_assign_type)
            {
                case ACL_IGR_ACTION_IVLAN_ASSIGN_NEW_VID:
                    diag_util_mprintf("new vid:%d\n", action->inner_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_IVLAN_ASSIGN_SHIFT_VID:
                    diag_util_mprintf("shift vid:%d\n", action->inner_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_IVLAN_ASSIGN_SHIFT_FROM_OUTER_VID:
                    diag_util_mprintf("shift from outer vid:%d\n", action->inner_vlan_data.vid_value);
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->inner_vlan_data.vid_assign_type) */
        }   /* end of if (ENABLED == action->inner_vlan_assign_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* outer vlan translate action */
        diag_util_mprintf("\tOuter Vlan translate action state: ");
        if (ENABLED == action->outer_vlan_assign_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\touter vlan-xlate action: ");
            switch (action->outer_vlan_data.vid_assign_type)
            {
                case ACL_IGR_ACTION_OVLAN_ASSIGN_NEW_VID:
                    diag_util_mprintf("new vid:%d\n", action->outer_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_OVLAN_ASSIGN_SHIFT_VID:
                    diag_util_mprintf("shift vid:%d\n", action->outer_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_OVLAN_ASSIGN_SHIFT_FROM_INNER_VID:
                    diag_util_mprintf("shift from inner vid:%d\n", action->outer_vlan_data.vid_value);
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->outer_vlan_data.vid_assign_type) */
        }   /* end of if (ENABLED == action->outer_vlan_assign_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* priority action */
        diag_util_mprintf("\tPriority action state: ");
        if (ENABLED == action->pri_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tpriority:%d\n", action->pri_data.pri);
        }   /* end of if (ENABLED == action->pri_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* MPLS action */
        diag_util_mprintf("\tMPLS action state: ");
        if (ENABLED == action->mpls_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tmpls action: ");

            if (0 == action->mpls_data.mpls_act)
                diag_util_mprintf("push outer ");
            else
                diag_util_mprintf("push inner and outer ");

            diag_util_mprintf("index:%d \n", action->mpls_data.mpls_idx);
        }   /* end of if (ENABLED == action->mpls_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* bypass action */
        diag_util_mprintf("\tBypass action state: ");
        if (ENABLED == action->bypass_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\tigr-bw-ctrl state: ");
            if (ENABLED == action->bypass_data.ibc_sc)
                diag_util_mprintf("Enabled\n");
            else
                diag_util_mprintf("Disabled\n");

            diag_util_mprintf("\t\tigr-stp state: ");
            if (ENABLED == action->bypass_data.igr_stp)
                diag_util_mprintf("Enabled\n");
            else
                diag_util_mprintf("Disabled\n");

            diag_util_mprintf("\t\tigr-drop state: ");
            if (ENABLED == action->bypass_data.all)
                diag_util_mprintf("Enabled\n");
            else
                diag_util_mprintf("Disabled\n");
        }   /* end of if (ENABLED == action->bypass_en) */
        else
            diag_util_mprintf("Disabled\n");
    }
#endif

#if defined(CONFIG_SDK_RTL8380)
    if(DIAG_OM_GET_FAMILYID(RTL8380_FAMILY_ID) || DIAG_OM_GET_FAMILYID(RTL8330_FAMILY_ID))
    {
        /* forward action */
        diag_util_mprintf("\tForward action state: ");
        if (ENABLED == action->fwd_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\taction: ");
            switch (action->fwd_data.fwd_type)
            {
                case ACL_ACTION_FWD_PERMIT:
                    diag_util_mprintf("permit\n");
                    break;
                case ACL_ACTION_FWD_DROP:
                    diag_util_mprintf("drop\n");
                    break;
                case ACL_ACTION_FWD_COPY_TO_PORTID:
                    diag_util_mprintf("copy uni port:%d\n", action->fwd_data.info.copy_redirect_port.fwd_port_id);
                    diag_util_mprintf("\t\t        cpu tag:%d\n", action->fwd_data.info.copy_redirect_port.cpu_tag);
                    diag_util_mprintf("\t\t        skip ingress stp:%d\n", action->fwd_data.info.copy_redirect_port.skip_igrStpDrop);
                    diag_util_mprintf("\t\t        skip ingress vlan filter:%d\n", action->fwd_data.info.copy_redirect_port.skip_storm_igrVlan);
                    break;
                case ACL_ACTION_FWD_COPY_TO_PORTMASK:
                    diag_util_mprintf("copy multi index:%d\n", action->fwd_data.info.copy_redirect_portMsk.fwd_idx);
                    diag_util_mprintf("\t\t        cpu tag:%d\n", action->fwd_data.info.copy_redirect_portMsk.cpu_tag);
                    diag_util_mprintf("\t\t        skip ingress stp:%d\n", action->fwd_data.info.copy_redirect_portMsk.skip_igrStpDrop);
                    diag_util_mprintf("\t\t        skip ingress vlan filter:%d\n", action->fwd_data.info.copy_redirect_portMsk.skip_storm_igrVlan);
                    break;
                case ACL_ACTION_FWD_REDIRECT_TO_PORTID:
                    diag_util_mprintf("redirect uni port:%d\n", action->fwd_data.info.copy_redirect_port.fwd_port_id);
                    diag_util_mprintf("\t\t        cpu tag:%d\n", action->fwd_data.info.copy_redirect_port.cpu_tag);
                    diag_util_mprintf("\t\t        skip ingress stp:%d\n", action->fwd_data.info.copy_redirect_port.skip_igrStpDrop);
                    diag_util_mprintf("\t\t        skip storm suppression filter:%d\n", action->fwd_data.info.copy_redirect_port.skip_storm_igrVlan);
                    diag_util_mprintf("\t\t        force:%d\n", action->fwd_data.info.copy_redirect_port.force);
                    break;
                case ACL_ACTION_FWD_REDIRECT_TO_PORTMASK:
                    diag_util_mprintf("redirect multi index:%d\n", action->fwd_data.info.copy_redirect_portMsk.fwd_idx);
                    diag_util_mprintf("\t\t        cpu tag:%d\n", action->fwd_data.info.copy_redirect_portMsk.cpu_tag);
                    diag_util_mprintf("\t\t        skip ingress stp:%d\n", action->fwd_data.info.copy_redirect_portMsk.skip_igrStpDrop);
                    diag_util_mprintf("\t\t        skip storm suppression filter:%d\n", action->fwd_data.info.copy_redirect_portMsk.skip_storm_igrVlan);
                    diag_util_mprintf("\t\t        force:%d\n", action->fwd_data.info.copy_redirect_portMsk.force);
                    break;
                case ACL_ACTION_FWD_UNICAST_ROUTING:
                    diag_util_mprintf("route unicast index:%d\n", action->fwd_data.info.route.idx);
                    break;
                case ACL_ACTION_FWD_VLAN_LEAKY:
                    diag_util_mprintf("vlan leaky\n");
                    break;

                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->fwd_data.fwd_type) */
        }   /* if (ENABLED == action->fwd_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* Counter action */
        diag_util_mprintf("\tStatistic action state: ");
        if (ENABLED == action->stat_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\taction: ");
            switch (action->stat_data.stat_type)
            {
                case STAT_TYPE_PACKET_BASED_32BIT:
                    diag_util_mprintf("packet32 index:%d\n", action->stat_data.stat_idx);
                    break;
                case STAT_TYPE_BYTE_BASED_64BIT:
                    diag_util_mprintf("byte64 index:%d\n", action->stat_data.stat_idx);
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->stat_data.stat_type) */
        }   /* if (ENABLED == action->stat_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* Mirror action */
        diag_util_mprintf("\tMirror action state: ");
        if (ENABLED == action->mirror_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\tmirror index:%d\n", action->mirror_data.mirror_set_idx);
            if(ACL_ACTION_MIRROR_ORIGINAL == action->mirror_data.mirror_type)
            {
                diag_util_mprintf("\t\tmirror type: original\n");
            }
            else
            {
                diag_util_mprintf("\t\tmirror type: modified\n");
            }
        }   /* end of if (ENABLED == action->mirror_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* Meter action */
        diag_util_mprintf("\tMeter action state: ");
        if (ENABLED == action->meter_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tmeter index:%d\n", action->meter_data.meter_idx);
        }   /* end of if (ENABLED == action->meter_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* inner vlan translate action */
        diag_util_mprintf("\tInner Vlan translate action state: ");
        if (ENABLED == action->inner_vlan_assign_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\tinner vlan-xlate action: ");
            switch (action->inner_vlan_data.vid_assign_type)
            {
                case ACL_IGR_ACTION_IVLAN_ASSIGN_NEW_VID:
                    diag_util_mprintf("new vid:%d\n", action->inner_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_IVLAN_ASSIGN_SHIFT_VID:
                    diag_util_mprintf("shift vid:%d\n", action->inner_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_IVLAN_ASSIGN_SHIFT_FROM_OUTER_VID:
                    diag_util_mprintf("shift from outer vid:%d\n", action->inner_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_IVLAN_ASSIGN_PORT_BASED_INNER_VID:
                    diag_util_mprintf("shift from inner port-based vid\n");
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->inner_vlan_data.vid_assign_type) */
        }   /* end of if (ENABLED == action->inner_vlan_assign_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* outer vlan translate action */
        diag_util_mprintf("\tOuter Vlan translate action state: ");
        if (ENABLED == action->outer_vlan_assign_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\touter vlan-xlate action: ");
            switch (action->outer_vlan_data.vid_assign_type)
            {
                case ACL_IGR_ACTION_OVLAN_ASSIGN_NEW_VID:
                    diag_util_mprintf("new vid:%d\n", action->outer_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_OVLAN_ASSIGN_SHIFT_VID:
                    diag_util_mprintf("shift vid:%d\n", action->outer_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_OVLAN_ASSIGN_SHIFT_FROM_INNER_VID:
                    diag_util_mprintf("shift from inner vid:%d\n", action->outer_vlan_data.vid_value);
                    break;
                case ACL_IGR_ACTION_OVLAN_ASSIGN_PORT_BASED_OUTER_VID:
                    diag_util_mprintf("shift from outer port-based vid\n");
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->outer_vlan_data.vid_assign_type) */
        }   /* end of if (ENABLED == action->outer_vlan_assign_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* priority action */
        diag_util_mprintf("\tNormal Port Priority action state: ");
        if (ENABLED == action->pri_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tpriority:%d\n", action->pri_data.pri);
        }   /* end of if (ENABLED == action->pri_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* priority for cpu port action */
        diag_util_mprintf("\tCPU Port Priority action state: ");
        if (ENABLED == action->cpu_pri_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tpriority:%d\n", action->cpu_pri_data.pri);
        }   /* end of if (ENABLED == action->pri_en) */
        else
            diag_util_mprintf("Disabled\n");

        /*drop action */
        diag_util_mprintf("\tDrop action state: ");
        if (ACL_IGR_ACTION_DROP_DROP == action->drop_data)
        {
            diag_util_mprintf("Drop\n");
        }
        else if (ACL_IGR_ACTION_DROP_WITHDRAW == action->drop_data)
        {
            diag_util_mprintf("Withdraw Drop\n");
        }
        else
        {
            diag_util_mprintf("Disabled\n");
        }

#if 0
        /* shaper action */
        diag_util_mprintf("\tShaper action state: ");
        if (ENABLED == action->shaper_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tQueue:%d\n", action->shaper_data.shaper_act);
        }   /* end of if (ENABLED == action->shaper_en) */
        else
            diag_util_mprintf("Disabled\n");
#endif

        /* O-TPID action */
        diag_util_mprintf("\tO-TPID action state: ");
        if (ENABLED == action->otpid_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tTPID Index:%d\n", action->otpid_data.tpid_idx);
        }   /* end of if (ENABLED == action->otpid_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* I-TPID action */
        diag_util_mprintf("\tI-TPID action state: ");
        if (ENABLED == action->otpid_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tTPID Index:%d\n", action->itpid_data.tpid_idx);
        }   /* end of if (ENABLED == action->otpid_en) */
        else
            diag_util_mprintf("Disabled\n");

        /*Filter action */
        diag_util_mprintf("\tFilter action state: ");
        if (ENABLED == action->filter_en)
        {
            diag_util_mprintf("Enabled\n");
            if(ACL_IGR_ACTION_FLT_SINGLE_PORT == action->filter_data.flt_act)
            {
                diag_util_mprintf("filter singel port:%d\n", action->filter_data.flt_info);
            }
            else if(ACL_IGR_ACTION_FLT_MULTIPLE_PORTS == action->filter_data.flt_act)
            {
                diag_util_mprintf("filter multi ports, index:%d\n", action->filter_data.flt_info);
            }
        }   /* end of if (ENABLED == action->filter_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* remark action */
        diag_util_mprintf("\tRemark action state: ");
        if (ENABLED == action->remark_en)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\tremark ");

            switch (action->remark_data.rmk_act)
            {
                case ACL_ACTION_REMARK_INNER_USER_PRI:
                    diag_util_mprintf("inner tag priority %d\n", action->remark_data.rmk_info);
                    break;
                case ACL_ACTION_REMARK_OUTER_USER_PRI:
                    diag_util_mprintf("outer tag priority %d\n", action->remark_data.rmk_info);
                    break;
                case ACL_ACTION_REMARK_DSCP:
                    diag_util_mprintf("DSCP %d\n", action->remark_data.rmk_info);
                    break;
                case ACL_ACTION_REMARK_IP_PRECEDENCE:
                    diag_util_mprintf("IP percedence %d\n", action->remark_data.rmk_info);
                    break;
                case ACL_ACTION_REMARK_COPY_IPRI_TO_OPRI:
                    diag_util_mprintf("outer tag priority from inner tag priority\n");
                    break;
                case ACL_ACTION_REMARK_COPY_OPRI_TO_IPRI:
                    diag_util_mprintf("inner tag priority from outer tag priority\n");
                    break;
                case ACL_ACTION_REMARK_KEEP_INNER_USER_PRI:
                    diag_util_mprintf("keep inner 1p priority\n");
                    break;
                case ACL_ACTION_REMARK_KEEP_OUTER_USER_PRI:
                    diag_util_mprintf("keep outer 1p priority\n");
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* end of switch (action->rmk_data.rmk_type) */
        }   /* end of if (ENABLED == action->rmk_en) */
        else
            diag_util_mprintf("Disabled\n");

        /* tag status action */
        diag_util_mprintf("\tTag status action state: ");
        if (ENABLED == action->egrTagStat_en)
        {
            diag_util_mprintf("Enabled\n");

            diag_util_mprintf("\t\tinner tag status: ");
            switch (action->egrTagStat_data.itag_sts)
            {
                case ACL_ACTION_TAG_STS_UNTAG:
                    diag_util_mprintf("untagged\n");
                    break;
                case ACL_ACTION_TAG_STS_TAG:
                    diag_util_mprintf("tagged\n");
                    break;
                case ACL_ACTION_TAG_STS_KEEP_CONTENT:
                    diag_util_mprintf("keep-content\n");
                    break;
                case ACL_ACTION_TAG_STS_NOP:
                    diag_util_mprintf("reserved\n");
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* switch (action->tag_sts_data.itag_sts) */

            diag_util_mprintf("\t\touter tag status: ");
            switch (action->egrTagStat_data.otag_sts)
            {
                case ACL_ACTION_TAG_STS_UNTAG:
                    diag_util_mprintf("untagged\n");
                    break;
                case ACL_ACTION_TAG_STS_TAG:
                    diag_util_mprintf("tagged\n");
                    break;
                case ACL_ACTION_TAG_STS_KEEP_CONTENT:
                    diag_util_mprintf("keep-content\n");
                    break;
                case ACL_ACTION_TAG_STS_NOP:
                    diag_util_mprintf("reserved\n");
                    break;
                default:
                    diag_util_mprintf("\n");
            }   /* switch (action->tag_sts_data.itag_sts) */
        }   /* end of if (ENABLED == action->tag_sts_en) */
        else
            diag_util_mprintf("Disabled\n");
    }
#endif

    return ;
}   /* end of _diag_acl_igrAction_show */

#if defined(CONFIG_SDK_RTL8390)
static void
_diag_acl_egrAction_show(uint32 unit, rtk_acl_egrAction_t *action)
{
    /* forward action */
    diag_util_mprintf("\tForward action state: ");
    if (ENABLED == action->fwd_en)
    {
        diag_util_mprintf("Enabled\n");

        diag_util_mprintf("\t\taction: ");
        switch (action->fwd_data.fwd_type)
        {
            case ACL_ACTION_FWD_PERMIT:
                diag_util_mprintf("permit\n");
                break;
            case ACL_ACTION_FWD_DROP:
                diag_util_mprintf("drop\n");
                break;
            case ACL_ACTION_FWD_COPY_TO_PORTID:
                diag_util_mprintf("copy uni port:%d\n", action->fwd_data.fwd_info);
                break;
            case ACL_ACTION_FWD_COPY_TO_PORTMASK:
                diag_util_mprintf("copy multi index:%d\n", action->fwd_data.fwd_info);
                break;
            case ACL_ACTION_FWD_REDIRECT_TO_PORTID:
                diag_util_mprintf("redirect uni port:%d\n", action->fwd_data.fwd_info);
                break;
            case ACL_ACTION_FWD_REDIRECT_TO_PORTMASK:
                diag_util_mprintf("redirect multi index:%d\n", action->fwd_data.fwd_info);
                break;
            case ACL_ACTION_FWD_FILTERING:
                diag_util_mprintf("filtering index:%d\n", action->fwd_data.fwd_info);
                break;
            default:
                diag_util_mprintf("\n");
        }   /* end of switch (action->fwd_data.fwd_type) */
    }   /* if (ENABLED == action->fwd_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* Counter action */
    diag_util_mprintf("\tStatistic action state: ");
    if (ENABLED == action->stat_en)
    {
        diag_util_mprintf("Enabled\n");

        diag_util_mprintf("\t\taction: ");
        switch (action->stat_data.stat_type)
        {
            case STAT_TYPE_PACKET_BASED_32BIT:
                diag_util_mprintf("packet32 index:%d\n", action->stat_data.stat_idx);
                break;
            case STAT_TYPE_BYTE_BASED_64BIT:
                diag_util_mprintf("byte64 index:%d\n", action->stat_data.stat_idx);
                break;
            default:
                diag_util_mprintf("\n");
        }   /* end of switch (action->stat_data.stat_type) */
    }   /* if (ENABLED == action->stat_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* Mirror action */
    diag_util_mprintf("\tMirror action state: ");
    if (ENABLED == action->mirror_en)
    {
        diag_util_mprintf("Enabled\n");

        diag_util_mprintf("\t\tmirror type: ");
        if (ACL_ACTION_MIRROR_ORIGINAL == action->mirror_data.mirror_type)
            diag_util_mprintf("original\n");
        else
            diag_util_mprintf("modified\n");

        diag_util_mprintf("\t\tmirror index:%d\n", action->mirror_data.mirror_set_idx);
    }   /* end of if (ENABLED == action->mirror_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* Meter action */
    diag_util_mprintf("\tMeter action state: ");
    if (ENABLED == action->meter_en)
    {
        diag_util_mprintf("Enabled\n");
        diag_util_mprintf("\t\tmeter index:%d\n", action->meter_data.meter_idx);
    }   /* end of if (ENABLED == action->meter_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* inner vlan translate action */
    diag_util_mprintf("\tInner Vlan translate action state: ");
    if (ENABLED == action->inner_vlan_assign_en)
    {
        diag_util_mprintf("Enabled\n");

        diag_util_mprintf("\t\tinner vlan-xlate action: ");
        switch (action->inner_vlan_data.vid_assign_type)
        {
            case ACL_EGR_ACTION_IVLAN_ASSIGN_NEW_VID:
                diag_util_mprintf("new vid:%d\n", action->inner_vlan_data.vid_value);
                break;
            case ACL_EGR_ACTION_IVLAN_ASSIGN_SHIFT_VID:
                diag_util_mprintf("shift vid:%d\n", action->inner_vlan_data.vid_value);
                break;
            case ACL_EGR_ACTION_IVLAN_ASSIGN_SHIFT_FROM_OUTER_VID:
                diag_util_mprintf("shift from outer vid:%d\n", action->inner_vlan_data.vid_value);
                break;
            case ACL_ACTION_VLAN_ASSIGN_COPY_FROM_OUTER_VID:
                diag_util_mprintf("copy from outer vid:%d\n", action->inner_vlan_data.vid_value);
                break;
            default:
                diag_util_mprintf("\n");
        }   /* end of switch (action->inner_vlan_data.vid_assign_type) */

        /* inner TPID action */
        diag_util_mprintf("\tInner TPID action state: ");
        if (ENABLED == action->inner_vlan_data.tpid_assign)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\ttpid index:%d\n", action->inner_vlan_data.tpid_idx);
        }   /* end of if (ENABLED == action->tpid_assign) */
        else
            diag_util_mprintf("Disabled\n");
    }   /* end of if (ENABLED == action->inner_vlan_assign_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* outer vlan translate action */
    diag_util_mprintf("\tOuter Vlan translate action state: ");
    if (ENABLED == action->outer_vlan_assign_en)
    {
        diag_util_mprintf("Enabled\n");

        diag_util_mprintf("\t\touter vlan-xlate action: ");
        switch (action->outer_vlan_data.vid_assign_type)
        {
            case ACL_EGR_ACTION_OVLAN_ASSIGN_NEW_VID:
                diag_util_mprintf("new vid:%d\n", action->outer_vlan_data.vid_value);
                break;
            case ACL_EGR_ACTION_OVLAN_ASSIGN_SHIFT_VID:
                diag_util_mprintf("shift vid:%d\n", action->outer_vlan_data.vid_value);
                break;
            case ACL_EGR_ACTION_OVLAN_ASSIGN_SHIFT_FROM_INNER_VID:
                diag_util_mprintf("shift from inner vid:%d\n", action->outer_vlan_data.vid_value);
                break;
            case ACL_ACTION_VLAN_ASSIGN_COPY_FROM_INNER_VID:
                diag_util_mprintf("copy from inner vid:%d\n", action->outer_vlan_data.vid_value);
                break;
            default:
                diag_util_mprintf("\n");
        }   /* end of switch (action->outer_vlan_data.vid_assign_type) */

        /* outer TPID action */
        diag_util_mprintf("\tOuter TPID action state: ");
        if (ENABLED == action->outer_vlan_data.tpid_assign)
        {
            diag_util_mprintf("Enabled\n");
            diag_util_mprintf("\t\ttpid index:%d\n", action->outer_vlan_data.tpid_idx);
        }   /* end of if (ENABLED == action->tpid_assign) */
        else
            diag_util_mprintf("Disabled\n");
    }   /* end of if (ENABLED == action->outer_vlan_assign_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* priority action */
    diag_util_mprintf("\tPriority action state: ");
    if (ENABLED == action->pri_en)
    {
        diag_util_mprintf("Enabled\n");

        diag_util_mprintf("\t\tpriority type: ");
        if (0 == action->pri_data.pri_act)
            diag_util_mprintf("all ports\n");
        else
            diag_util_mprintf("cpu port only\n");

        diag_util_mprintf("\t\tpriority:%d\n", action->pri_data.pri);
    }   /* end of if (ENABLED == action->pri_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* remark action */
    diag_util_mprintf("\tRemark action state: ");
    if (ENABLED == action->rmk_en)
    {
        diag_util_mprintf("Enabled\n");
        diag_util_mprintf("\t\tremark ");

        switch (action->rmk_data.rmk_type)
        {
            case ACL_ACTION_REMARK_INNER_USER_PRI:
                diag_util_mprintf("inner tag priority %d\n", action->rmk_data.rmk_value);
                break;
            case ACL_ACTION_REMARK_OUTER_USER_PRI:
                diag_util_mprintf("outer tag priority %d\n", action->rmk_data.rmk_value);
                break;
            case ACL_ACTION_REMARK_DSCP:
                diag_util_mprintf("DSCP %d\n", action->rmk_data.rmk_value);
                break;
            case ACL_ACTION_REMARK_IP_PRECEDENCE:
                diag_util_mprintf("IP percedence %d\n", action->rmk_data.rmk_value);
                break;
            case ACL_ACTION_REMARK_COPY_IPRI_TO_OPRI:
                diag_util_mprintf("outer tag priority from inner tag priority\n");
                break;
            case ACL_ACTION_REMARK_COPY_OPRI_TO_IPRI:
                diag_util_mprintf("inner tag priority from outer tag priority\n");
                break;
#if 0
            case ACL_ACTION_REMARK_INNER_AND_DSCP:
                diag_util_mprintf("DSCP %d\n", action->rmk_data.rmk_value);
                diag_util_mprintf("inner tag priority %d\n", action->rmk_data.rmk_value_ext);
                break;
            case ACL_ACTION_REMARK_INNER_AND_OUTER:
                diag_util_mprintf("inner tag priority %d\n", action->rmk_data.rmk_value);
                diag_util_mprintf("outer tag priority %d\n", action->rmk_data.rmk_value_ext);
                break;
#endif
            default:
                diag_util_mprintf("\n");
        }   /* end of switch (action->rmk_data.rmk_type) */
    }   /* end of if (ENABLED == action->rmk_en) */
    else
        diag_util_mprintf("Disabled\n");

    /* tag status action */
    diag_util_mprintf("\tTag status action state: ");
    if (ENABLED == action->tag_sts_en)
    {
        diag_util_mprintf("Enabled\n");

        diag_util_mprintf("\t\tinner tag status: ");
        switch (action->tag_sts_data.itag_sts)
        {
            case ACL_ACTION_TAG_STS_UNTAG:
                diag_util_mprintf("untagged\n");
                break;
            case ACL_ACTION_TAG_STS_TAG:
                diag_util_mprintf("tagged\n");
                break;
            case ACL_ACTION_TAG_STS_KEEP_CONTENT:
                diag_util_mprintf("keep-content\n");
                break;
            case ACL_ACTION_TAG_STS_NOP:
                diag_util_mprintf("reserved\n");
                break;
            default:
                diag_util_mprintf("\n");
        }   /* switch (action->tag_sts_data.itag_sts) */

        diag_util_mprintf("\t\touter tag status: ");
        switch (action->tag_sts_data.otag_sts)
        {
            case ACL_ACTION_TAG_STS_UNTAG:
                diag_util_mprintf("untagged\n");
                break;
            case ACL_ACTION_TAG_STS_TAG:
                diag_util_mprintf("tagged\n");
                break;
            case ACL_ACTION_TAG_STS_KEEP_CONTENT:
                diag_util_mprintf("keep-content\n");
                break;
            case ACL_ACTION_TAG_STS_NOP:
                diag_util_mprintf("reserved\n");
                break;
            default:
                diag_util_mprintf("\n");
        }   /* switch (action->tag_sts_data.itag_sts) */
    }   /* end of if (ENABLED == action->tag_sts_en) */
    else
        diag_util_mprintf("Disabled\n");

    return ;
}   /* end of _diag_acl_egrAction_show */
#endif

/*
 * acl get entry phase <UINT:phase> entry <UINT:index> action
 */
cparser_result_t
cparser_cmd_acl_get_entry_phase_phase_entry_index_action(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32              unit;
    int32               ret;
    rtk_acl_action_t    action;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&action, 0, sizeof(rtk_acl_action_t));

    if (0 == *phase_ptr)
        diag_util_mprintf("Ingress ");
#if defined(CONFIG_SDK_RTL8390)
    else
        diag_util_mprintf("Egress ");
#endif

    diag_util_mprintf("entry index %d:\n", *index_ptr);

    ret = rtk_acl_ruleAction_get(unit, *phase_ptr, *index_ptr, &action);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
        _diag_acl_igrAction_show(unit, &action.igr_acl);
#if defined(CONFIG_SDK_RTL8390)
    else
        _diag_acl_egrAction_show(unit, &action.egr_acl);
#endif

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_entry_phase_entry_action */
#endif

#ifdef CMD_RANGE_CHECK_GET_PORT_ENTRY_INDEX_SOURCE
/*
 * range-check get port entry <UINT:index> source
 */
cparser_result_t
cparser_cmd_range_check_get_port_entry_index_source(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    uint8                           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_acl_rangeCheck_portMask_t   port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&port_range, 0, sizeof(rtk_acl_rangeCheck_portMask_t));

    ret = rtk_acl_rangeCheckSrcPort_get(unit, *index_ptr, &port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Source port range check index %d:\n", *index_ptr);
    diag_util_lPortMask2str(port_list, &port_range.port_mask);
    diag_util_mprintf("\tconfigured ports: %s\n", port_list);

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_get_entry_index_port_check_source */
#endif

#ifdef CMD_RANGE_CHECK_SET_PORT_ENTRY_INDEX_PORT_PORTS_NONE_SOURCE
/*
 * range-check set port entry <UINT:index> port ( <PORT_LIST:ports> | none ) source
 */
cparser_result_t
cparser_cmd_range_check_set_port_entry_index_port_ports_none_source(
    cparser_context_t *context,
    uint32_t *index_ptr,
    char **ports_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_portMask_t   port_range;
    rtk_portmask_t                  port_mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&port_range, 0, sizeof(rtk_acl_rangeCheck_portMask_t));
    memset(&port_mask, 0, sizeof(rtk_portmask_t));

    ret = rtk_acl_rangeCheckSrcPort_get(unit, *index_ptr, &port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('n' != TOKEN_CHAR(6,0))
    {
        memset(&port_mask, 0, sizeof(rtk_portmask_t));
        DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask((uint8 *)*ports_ptr, &port_mask), ret);
    }

    memcpy(&port_range.port_mask, &port_mask, sizeof(rtk_portmask_t));

    ret = rtk_acl_rangeCheckSrcPort_set(unit, *index_ptr, &port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_port_check_source_port_none */
#endif

#ifdef CMD_RANGE_CHECK_GET_PORT_ENTRY_INDEX_DESTINATION
/*
 * range-check get port entry <UINT:index> destination
 */
cparser_result_t
cparser_cmd_range_check_get_port_entry_index_destination(
    cparser_context_t *context,
    uint32_t *index_ptr)
{
    uint32                          unit;
    int32                           ret;
    uint8                           port_list[DIAG_UTIL_PORT_MASK_STRING_LEN];
    rtk_acl_rangeCheck_portMask_t   port_range;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&port_range, 0, sizeof(rtk_acl_rangeCheck_portMask_t));

    ret = rtk_acl_rangeCheckDstPort_get(unit, *index_ptr, &port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    diag_util_mprintf("Destination port range check index %d:\n", *index_ptr);
    diag_util_lPortMask2str(port_list, &port_range.port_mask);
    diag_util_mprintf("\tconfigured ports: %s\n", port_list);

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_get_entry_index_port_check_destination */
#endif

#ifdef CMD_RANGE_CHECK_SET_PORT_ENTRY_INDEX_PORT_PORTS_NONE_DESTINATION
/*
 * range-check set port entry <UINT:index> port ( <PORT_LIST:ports> | none ) destination
 */
cparser_result_t
cparser_cmd_range_check_set_port_entry_index_port_ports_none_destination(
    cparser_context_t *context,
    uint32_t *index_ptr,
    char **ports_ptr)
{
    uint32                          unit;
    int32                           ret;
    rtk_acl_rangeCheck_portMask_t   port_range;
    rtk_portmask_t                  port_mask;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    memset(&port_range, 0, sizeof(rtk_acl_rangeCheck_portMask_t));
    memset(&port_mask, 0, sizeof(rtk_portmask_t));

    ret = rtk_acl_rangeCheckDstPort_get(unit, *index_ptr, &port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if('n' != TOKEN_CHAR(6,0))
    {
        memset(&port_mask, 0, sizeof(rtk_portmask_t));
        DIAG_UTIL_ERR_CHK(diag_util_str2LPortMask((uint8 *)*ports_ptr, &port_mask), ret);
    }

    memcpy(&port_range.port_mask, &port_mask, sizeof(rtk_portmask_t));

    ret = rtk_acl_rangeCheckDstPort_set(unit, *index_ptr, &port_range);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_range_check_set_entry_index_port_check_destination_port_none */
#endif

#ifdef CMD_ACL_GET_TEMPLATE_FIELD_INTENT_VLAN_TAG
/*
 * acl get template field-intent vlan-tag
 */
cparser_result_t
cparser_cmd_acl_get_template_field_intent_vlan_tag(
    cparser_context_t *context)
{
    rtk_vlan_tagType_t  tagType;
    uint32              unit;
    int32               ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    ret = rtk_acl_templateFieldIntentVlanTag_get(unit, &tagType);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_OUTPUT_INIT();

    diag_util_mprintf("Template field intent VLAN tag: ");
    if (VLAN_TAG_TYPE_INNER == tagType)
        diag_util_mprintf("inner\n");
    else
        diag_util_mprintf("outer\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_template_field_intent_vlan_tag */
#endif

#ifdef CMD_ACL_SET_TEMPLATE_FIELD_INTENT_VLAN_TAG_INNER_OUTER
/*
 * acl set template field-intent vlan-tag ( inner | outer )
 */
cparser_result_t
cparser_cmd_acl_set_template_field_intent_vlan_tag_inner_outer(
    cparser_context_t *context)
{
    rtk_vlan_tagType_t  tagType;
    uint32              unit;
    int32               ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if ('i' == TOKEN_CHAR(5, 0))
        tagType = VLAN_TAG_TYPE_INNER;
    else
        tagType = VLAN_TAG_TYPE_OUTER;

    ret = rtk_acl_templateFieldIntentVlanTag_set(unit, tagType);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_template_field_intent_vlan_tag_inner_outer */
#endif

#ifdef CMD_ACL_INIT_ENTRY_BUFFER
/*
 * acl init entry buffer
 */
cparser_result_t
cparser_cmd_acl_init_entry_buffer(
    cparser_context_t *context)
{
    uint32  unit;
    uint32  entry_size;
    int32   ret;

    DIAG_OM_GET_CHIP_ID(unit);

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntrySize_get(unit, ACL_PHASE_IGR_ACL, &entry_size), ret);

    if (!diag_acl_entry_buffer)
    {
        if ((diag_acl_entry_buffer = malloc(entry_size)) == NULL)
        {
            diag_util_printf("malloc() fail!\n");
            return CPARSER_NOT_OK;
        }
    }

    memset(diag_acl_entry_buffer, 0, entry_size);

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_init_entry_buffer */
#endif

#ifdef CMD_ACL_SET_ENTRY_BUFFER_PHASE_PHASE_ENTRY_INDEX_FIELD_FIELD_NAME_DATA_DATA_MASK_MASK
/*
 * acl set entry buffer phase <UINT:phase> entry <UINT:index> field <STRING:field_name> data <STRING:data> mask <STRING:mask>
 */
cparser_result_t
cparser_cmd_acl_set_entry_buffer_phase_phase_entry_index_field_field_name_data_data_mask_mask(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr,
    char **field_name_ptr,
    char **data_ptr,
    char **mask_ptr)
{
    uint32  num_element, i;
    uint32  unit = 0;
    uint32  field_size = 0;
    int32   ret;
    uint8   field_data[RTK_MAX_SIZE_OF_ACL_USER_FIELD];
    uint8   field_mask[RTK_MAX_SIZE_OF_ACL_USER_FIELD];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (!diag_acl_entry_buffer)
    {
        diag_util_printf("Entry buffer is not init!\n");
        return CPARSER_NOT_OK;
    }

    num_element = (sizeof(diag_acl_field_list)/sizeof(diag_acl_field_t));

    for (i = 0; i < num_element; ++i)
    {
        if (!strcasecmp(*field_name_ptr, diag_acl_field_list[i].user_info))
            break;
    }

    if (num_element <= i)
    {
        diag_util_printf("Invalid field %s!\n", *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    ret = rtk_acl_ruleEntryField_check(unit, *phase_ptr,
            diag_acl_field_list[i].type);
    if (RT_ERR_OK != ret)
    {
        diag_util_printf("Field %s isn't supported in the phase!\n",
                *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntryFieldSize_get(unit, diag_acl_field_list[i].type, &field_size), ret);

    DIAG_UTIL_ACL_PARAM_LEN_CHK(field_size, *data_ptr);
    DIAG_UTIL_ACL_PARAM_LEN_CHK(field_size, *mask_ptr);
    memset(field_data, 0x0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memset(field_mask, 0x0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);

    field_size = ((field_size + 7) / 8);

    if (diag_acl_str2IntArray (field_data, (uint8 *)*data_ptr, field_size) != RT_ERR_OK)
    {
        diag_util_printf("field data error!\n");
        return CPARSER_NOT_OK;
    }

    if (diag_acl_str2IntArray (field_mask, (uint8 *)*mask_ptr, field_size) != RT_ERR_OK)
    {
        diag_util_printf("field mask error!\n");
        return CPARSER_NOT_OK;
    }

    ret = rtk_acl_ruleEntryField_set(unit, *phase_ptr, *index_ptr,
            diag_acl_entry_buffer, diag_acl_field_list[i].type, field_data, field_mask);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_set_entry_buffer_phase_phase_entry_index_field_field_name_data_data_mask_mask */
#endif

#ifdef CMD_ACL_GET_ENTRY_BUFFER
/*
 * acl get entry buffer
 */
cparser_result_t
cparser_cmd_acl_get_entry_buffer(
    cparser_context_t *context)
{
    uint32  unit;
    uint32  entry_size, entry_mask_position;
    int32   ret;
    uint8   i;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (!diag_acl_entry_buffer)
    {
        diag_util_printf("Entry buffer is not init!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntrySize_get(unit, ACL_PHASE_IGR_ACL, &entry_size), ret);

    entry_mask_position = (entry_size / 2);
    for (i = 0; i < entry_mask_position; i++)
    {
        if((i % 8) == 0)
        {
            diag_util_mprintf("\n");
        }
        diag_util_printf("byte %2d | data:%02x mask:%02x\n", i,
                *(diag_acl_entry_buffer + i),
                *(diag_acl_entry_buffer + i + entry_mask_position));
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_entry_buffer */
#endif

#ifdef CMD_ACL_GET_ENTRY_BUFFER_PHASE_PHASE_ENTRY_INDEX_FIELD_FIELD_NAME
/*
 * acl get entry buffer phase <UINT:phase> entry <UINT:index> field <STRING:field_name>
 */
cparser_result_t
cparser_cmd_acl_get_entry_buffer_phase_phase_entry_index_field_field_name(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr,
    char **field_name_ptr)
{
    uint32  num_element;
    uint32  unit = 0;
    uint32  field_size = 0;
    int32   ret;
    uint32  i;
    uint8   field_data[RTK_MAX_SIZE_OF_ACL_USER_FIELD];
    uint8   field_mask[RTK_MAX_SIZE_OF_ACL_USER_FIELD];

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);
    DIAG_UTIL_OUTPUT_INIT();

    if (!diag_acl_entry_buffer)
    {
        diag_util_printf("Entry buffer is not init!\n");
        return CPARSER_NOT_OK;
    }

    num_element = (sizeof(diag_acl_field_list)/sizeof(diag_acl_field_t));
    memset(field_data, 0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);
    memset(field_mask, 0, RTK_MAX_SIZE_OF_ACL_USER_FIELD);

    for (i = 0; i < num_element; ++i)
    {
        if (!strcasecmp(*field_name_ptr, diag_acl_field_list[i].user_info))
            break;
    }

    if (num_element <= i)
    {
        diag_util_printf("Invalid field %s!\n", *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    ret = rtk_acl_ruleEntryField_check(unit, *phase_ptr,
            diag_acl_field_list[i].type);
    if (RT_ERR_OK != ret)
    {
        diag_util_printf("Field %s isn't supported in the phase!\n",
                *field_name_ptr);
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntryFieldSize_get(unit,
            diag_acl_field_list[i].type, &field_size), ret);

    if ((field_size % 8) == 0)
    {
        field_size = (field_size / 8);
    }
    else
    {
        field_size = (field_size / 8) + 1;
    }

    ret = rtk_acl_ruleEntryField_get(unit, *phase_ptr, *index_ptr,
            diag_acl_entry_buffer, diag_acl_field_list[i].type, field_data,
            field_mask);
    if (RT_ERR_OK != ret)
    {
        DIAG_ERR_PRINT(ret);
        return CPARSER_NOT_OK;
    }

    if (0 == *phase_ptr)
        diag_util_mprintf("Ingress ");
    else
        diag_util_mprintf("Egress ");

    diag_util_mprintf("index %d field %s\n", *index_ptr, *field_name_ptr);

    diag_util_mprintf("\tdata=0x");
    for (i = 0; i < field_size ; ++i)
    {
        diag_util_mprintf("%02x", field_data[i]);
    }
    diag_util_mprintf("\n");

    diag_util_mprintf("\tmask=0x");
    for (i = 0; i < field_size ; ++i)
    {
        diag_util_mprintf("%02x", field_mask[i]);
    }
    diag_util_mprintf("\n");

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_get_entry_buffer_phase_phase_entry_index_field_field_name */
#endif

#ifdef CMD_ACL_WRITE_ENTRY_BUFFER_PHASE_PHASE_ENTRY_INDEX
/*
 * acl write entry buffer phase <UINT:phase> entry <UINT:index>
 */
cparser_result_t
cparser_cmd_acl_write_entry_buffer_phase_phase_entry_index(
    cparser_context_t *context,
    uint32_t *phase_ptr,
    uint32_t *index_ptr)
{
    uint32  unit;
    int32   ret;

    DIAG_UTIL_PARAM_CHK();
    DIAG_OM_GET_CHIP_ID(unit);

    if (!diag_acl_entry_buffer)
    {
        diag_util_printf("Entry buffer is not init!\n");
        return CPARSER_NOT_OK;
    }

    DIAG_UTIL_ERR_CHK(rtk_acl_ruleEntry_write(unit, *phase_ptr, *index_ptr,
            diag_acl_entry_buffer), ret);

    return CPARSER_OK;
}   /* end of cparser_cmd_acl_write_entry_buffer_phase_phase_entry_index */
#endif

